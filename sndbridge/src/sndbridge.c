#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>

#include "linkedlist.h"
#include "sndbridge.h"
#include "portaudio.h"
#include "pa_win_wasapi.h"
#include "oggdecoder.h"
#include "filehandle.h"

typedef unsigned long ulong;
typedef const PaStreamCallbackTimeInfo* CbTimeInfo;
typedef PaStreamCallbackFlags CbFlags;

#define SAMPLE_BUFFER_SIZE (32 * 1024)
#define BUFFER_SIZE_BYTES (SAMPLE_BUFFER_SIZE * (int32_t)sizeof(int16_t))
#define HOLE -3// retured by ov_read and op_read, means bad/missing packets in the file
#define PI 3.141592653589793f
#define PI_HALF 1.5707963267948966f
#define DESIRED_API_NAME "DSound"
#define DESIRED_API_ENUM paDirectSound
#define MIN_DURATION_MS_KEEP_ALIVE 500// 500 milliseconds


typedef struct {
    StreamID id;
    OggDecoder* oggdecoder;
    int32_t bytes_per_channel;
    int32_t channels;
    int32_t sample_rate;

    PaStream* pastream;

    volatile float volume;
    volatile bool muted;
    volatile bool looped;
    volatile bool completed;
    volatile bool fetching;
    volatile bool keep_alive;
    volatile bool halt;
    volatile bool callback_running;

    volatile PaTime timestamp;
    volatile PaTime played_time;
    double duration;
    float milliseconds_to_samples;

    uint8_t buffer[SAMPLE_BUFFER_SIZE];
    int16_t queue[SAMPLE_BUFFER_SIZE];
    size_t buffer_used;

    float fade_duration;
    float fade_progress;
    bool fade_in_or_out;

} Stream_t;


static volatile bool backend_initialize;
static volatile StreamID STREAM_IDS;
static Linkedlist streams;
static volatile float master_volume;
static char runtime_info[256];
static __attribute__((constructor)) void sndbridge_constructor() {
    backend_initialize = true;
    STREAM_IDS = 0x0000;
    streams = linkedlist_init();
    master_volume = 1.0f;
}

#define LOG_PAERROR(fmt, err) fprintf(stderr, fmt "\n", Pa_GetErrorText(err))
#define LOG_ERROR(fmt, ...) fprintf(stderr, fmt "\n", __VA_ARGS__)
#define LOG_ERROR_NOTFMT(fmt) fprintf(stderr, fmt "\n")
#define GET_STREAM(stream_id) Stream_t* stream = sndbridge_get_stream(stream_id);

#define CLAMP_FLOAT(value) (value < -1.0f ? -1.0f : (value > 1.0f ? 1.0f : value))


static void completed_cb(void* userdata) {
    Stream_t* stream = (Stream_t*)userdata;
    stream->completed = true;
    stream->callback_running = false;

#ifdef DEBUG
    printf(
        "sndbridge: END stream_id=%i position=%f\n",
        stream->id, (Pa_GetStreamTime(stream->pastream) - stream->timestamp) * 1000.0
    );
#endif

}

static void wait_and_halt(Stream_t* stream, bool halt) {
    int32_t elapsed = 0;
    while (stream->callback_running && elapsed < 1000) {
        Pa_Sleep(10);
        elapsed += 10;
    }
    stream->halt = halt;
}

static inline float calc_s_curve(float percent) {
    if (percent <= 0.0f) return 0.0f;
    if (percent >= 1.0f) return 1.0f;
    return (sinf((PI * percent) - PI_HALF) / 2.0f) + 0.5f;
}

static inline float sndbridge_tofloat(int16_t sample) {
    float samplef = (float)sample / 32768.0f;
    if (samplef > 1.0f) samplef = 1.0f;
    if (samplef < -1.0f) samplef = -1.0f;
    return samplef;
}

static void sndbridge_dispose_backend() {
    if (linkedlist_count(streams) > 0) return;
    Pa_Terminate();
    backend_initialize = true;
}

static Stream_t* sndbridge_get_stream(StreamID stream) {
    LINKEDLIST_FOREACH(Stream_t*, item, streams) {
        if (item->id == stream) return item;
    }
    // not found
    return NULL;
}

static inline int32_t sndbridge_read_ogg_samples(Stream_t* stream, ulong frameCount, int16_t* output) {
    assert(frameCount < SAMPLE_BUFFER_SIZE);

    int32_t to_write = frameCount * stream->bytes_per_channel;
    int32_t total_readed = stream->buffer_used;
    int32_t required_bytes = to_write - total_readed;
    uint8_t* buf = stream->buffer + total_readed;

    while (required_bytes > 0) {
        int32_t readed = oggdecoder_read(stream->oggdecoder, buf, required_bytes);
        if (readed < 1) {
            if (readed == HOLE) continue;

            // if looped seek to the start
            if (stream->looped) {
                oggdecoder_seek(stream->oggdecoder, 0.0);
                stream->played_time = 0.0;
                stream->fetching = false;
                continue;
            }
            break;
        }
        required_bytes -= readed;
        buf += readed;
        total_readed += readed;
    }

    if (total_readed < to_write) {
        // EOF reached add silece at the end of the output
        memcpy(output, stream->buffer, total_readed);
        //memset(output + total_readed, 0x00, to_write - total_readed);
        return total_readed / stream->bytes_per_channel;
    }

    memcpy(output, stream->buffer, to_write);
    stream->buffer_used = total_readed - to_write;
    memmove(stream->buffer, stream->buffer + to_write, stream->buffer_used);

    return to_write / stream->bytes_per_channel;
}

static inline void sndbridge_apply_fade(Stream_t* stream, int32_t frames_total, float output_float[]) {
    if (stream->fade_duration < 1) return;


    register float fade_duration = stream->fade_duration;
    register float fade_progress = stream->fade_progress;
    register int32_t channels = stream->channels;

    if (stream->fade_in_or_out) {
        for (int32_t i = 0, j = 0;i < frames_total;i++, j++) {
#ifdef SNDBRIDGE_SCURVE
            output_float[i] *= calc_s_curve(fade_progress / fade_duration);
#else
            output_float[i] *= fade_progress / fade_duration;
#endif
            if (j >= channels) {
                j = 0;
                fade_progress++;
            }
        }
    }
    else {
        for (int32_t i = 0, j = 0;i < frames_total;i++, j++) {
            output_float[i] *= 1.0f - fade_progress / fade_duration;
            fade_progress++;
            if (j >= channels) {
                j = 0;
                fade_progress++;
            }
        }

    }

    if (fade_progress >= fade_duration) {
        stream->fade_duration = 0.0f;
        if (!stream->fade_in_or_out)  stream->volume = 0.0f;
    }
    else {
        stream->fade_progress = fade_progress;
    }
}

static int read_cb(const void* ib, void* output, ulong frameCount, CbTimeInfo ti, CbFlags sf, void* userdata) {
    (void)ib;
    (void)sf;

    Stream_t* stream = (Stream_t*)userdata;
    const int32_t frames_total = frameCount * stream->channels;
    float* output_float = (float*)output;

    stream->callback_running = true;

    if (stream->halt) {
        // playback ended, silence output to keep alive the stream
        memset(output_float, 0x00, frames_total * sizeof(float));
        stream->callback_running = false;
        return paContinue;
    }

    // read samples
    int32_t readed_frames = sndbridge_read_ogg_samples(stream, frameCount, stream->queue);

    if (stream->muted) {
        memset(output_float, 0x00, frames_total * sizeof(float));
        goto L_prepare_return;
    }

    // do sample conversion and apply volume
    register float divisor = 32768.0f / (stream->volume * master_volume);
    for (int32_t i = 0;i < frames_total;i++) {
        output_float[i] = stream->queue[i] / divisor;
    }

    // do fade if required
    sndbridge_apply_fade(stream, frames_total, output_float);

L_prepare_return:
    stream->callback_running = false;

    if ((ulong)readed_frames < frameCount) {
        if (!stream->keep_alive) return paComplete;

        if (frameCount > 0) frameCount--;
        PaTime remaing_frames = frameCount;

        stream->completed = true;
        //stream->fetching = true;
        stream->halt = true;
        stream->played_time = ti->outputBufferDacTime + (remaing_frames / stream->sample_rate);

        return paContinue;
    }

    if (!stream->fetching) {
        stream->fetching = true;
        stream->timestamp = ti->outputBufferDacTime;
    }

    return paContinue;
}

static void sndbridge_create_pastream(Stream_t* stream) {
    // step 1: adquire default output device
    bool has_desired = false;
    PaError err = paNoError;
    PaDeviceIndex output_device_index = Pa_GetDefaultOutputDevice();
    PaStream* pastream = NULL;

    stream->pastream = NULL;

    // attempt to use the desired api (DESIRED_API_ENUM)
    PaHostApiIndex count = Pa_GetHostApiCount();
    for (PaHostApiIndex index = 0; index < count; index++) {
        const PaHostApiInfo* info = Pa_GetHostApiInfo(index);

        if (info->type != DESIRED_API_ENUM) continue;
        if (info->defaultOutputDevice != paNoDevice) {
            has_desired = true;
            output_device_index = info->defaultOutputDevice;
        }
        break;
    }

L_check_device:
    if (output_device_index == paNoDevice) {
        /*if (has_desired) {
            has_desired = false;
            output_device_index = Pa_GetDefaultOutputDevice();
            LOG_ERROR_NOTFMT("sndbridge_spawn_stream() no output devices available for " DESIRED_API_NAME);
            goto L_check_device;
        }*/
        LOG_ERROR_NOTFMT("sndbridge_spawn_stream() no output devices available");
        return;
    }

    // step 2: obtain minimal latency
    const PaDeviceInfo* device_info = Pa_GetDeviceInfo(output_device_index);
    assert(device_info);
    PaTime latency = device_info->defaultLowOutputLatency;

    // step 3: prepare stream
    PaStreamParameters output_params = {
        .channelCount = stream->channels,
        .device = output_device_index,
        .hostApiSpecificStreamInfo = NULL,
        .sampleFormat = paFloat32,
        .suggestedLatency = latency
    };

    PaWasapiStreamInfo stream_info = {
        .hostApiType = paWASAPI,
        .version = 1,
        .size = sizeof(PaWasapiStreamInfo),
        .flags = paWinWasapiThreadPriority | paWinWasapiAutoConvert,
        .threadPriority = eThreadPriorityGames,
        .streamCategory = eAudioCategoryGameMedia
    };
    if (DESIRED_API_ENUM == paWASAPI && has_desired) output_params.hostApiSpecificStreamInfo = &stream_info;


    // step 4: check if the format is supported
    err = Pa_IsFormatSupported(NULL, &output_params, stream->sample_rate);
    if (err != paNoError) {
        LOG_ERROR(
            "Pa_IsFormatSupported() failed. sample_rate=%i channels=%i latency=%f " DESIRED_API_NAME "=(bool)%i\n%s",
            stream->sample_rate, stream->channels, latency, has_desired, Pa_GetErrorText(err)
        );

        if (has_desired) {
            // try again with the default audio API and output device
            has_desired = false;
            output_device_index = Pa_GetDefaultOutputDevice();
            goto L_check_device;
        }

        return;
    }

    // step 5: open the stream
    err = Pa_OpenStream(
        &pastream,
        NULL, &output_params,
        stream->sample_rate,
        paFramesPerBufferUnspecified, paNoFlag, read_cb, stream
    );

    if (err == paNoError) {
        stream->pastream = pastream;
        Pa_SetStreamFinishedCallback(pastream, completed_cb);
        return;
    }

    LOG_ERROR(
        "Pa_OpenStream() failed. sample_rate=%i channels=%i latency=%f " DESIRED_API_NAME "=(bool)%i",
        stream->sample_rate, stream->channels, latency, has_desired
    );

    if (has_desired) {
        // last attempt with the default audio API and output device
        has_desired = false;
        output_device_index = Pa_GetDefaultOutputDevice();
        goto L_check_device;
    }
}

extern StreamID sndbridge_queue_ogg(FileHandle_t* ogg_filehandle) {
    PaError err = paNoError;
    Stream_t* stream = NULL;
    OggDecoder* oggdecoder = NULL;

    if (backend_initialize) {
        err = Pa_Initialize();
        if (err != paNoError) return StreamID_BACKEND_FAILED;
        backend_initialize = false;
    }

    oggdecoder = oggdecoder_init(ogg_filehandle);
    if (!oggdecoder) {
        return StreamID_DECODER_FAILED;
    }

    int32_t sample_rate, channels;
    double duration;
    oggdecoder_get_info(oggdecoder, &sample_rate, &channels, &duration);

    stream = calloc(1, sizeof(Stream_t));
    stream->id = STREAM_IDS++;
    stream->oggdecoder = oggdecoder;
    stream->volume = 1.0f;
    stream->duration = duration;
    stream->milliseconds_to_samples = sample_rate / 1000.0f;
    stream->bytes_per_channel = channels * 2;
    stream->channels = channels;
    stream->looped = false;
    stream->completed = false;
    stream->pastream = NULL;
    stream->keep_alive = duration < MIN_DURATION_MS_KEEP_ALIVE;
    stream->callback_running = false;
    stream->sample_rate = sample_rate;

    sndbridge_create_pastream(stream);
    if (!stream->pastream) {
        free(stream);
        oggdecoder_destroy(oggdecoder);
        sndbridge_dispose_backend();
        return StreamID_BACKEND_FAILED;
    }

    // ¡success!
    linkedlist_add_item(streams, stream);
    return stream->id;
}


extern void sndbridge_dispose(StreamID stream_id) {
    GET_STREAM(stream_id);
    if (!stream) return;

    // FIXME: there a race condition which keeps the stream alive
    stream->volume = 0.0;
    Pa_Sleep(1);

    Pa_CloseStream(stream->pastream);

    linkedlist_remove_item(streams, stream);
    oggdecoder_destroy(stream->oggdecoder);
    free(stream);

    //sndbridge_dispose_backend();
}

extern double sndbridge_duration(StreamID stream_id) {
    GET_STREAM(stream_id);
    if (!stream) return -1.0;
    return stream->duration;
}

extern double sndbridge_position(StreamID stream_id) {
    GET_STREAM(stream_id);
    if (!stream) return -1.0;

    PaTime position = stream->played_time;
    PaError err = Pa_IsStreamStopped(stream->pastream);

    switch (err) {
    case 1:
        break;
    case 0:
        if (stream->fetching && !stream->halt)
            position += Pa_GetStreamTime(stream->pastream) - stream->timestamp;
        break;
    default:
        LOG_ERROR("sndbridge_position() failed on stream %i: %s", stream_id, Pa_GetErrorText(err));
        return -1.0;
    }

    position *= 1000.0;

    //if (position >= stream->duration) return stream->duration;
    return position;
}

extern void sndbridge_seek(StreamID stream_id, double milliseconds) {
    GET_STREAM(stream_id);
    if (!stream) return;

    PaError status = Pa_IsStreamActive(stream->pastream);
    if (status != 1 && status != 0) {
        LOG_ERROR("sndbridge_seek() status failed on stream %i: %s", stream_id, Pa_GetErrorText(status));
        return;
    }

    if (status == 1 && !stream->keep_alive) {
        status = Pa_AbortStream(stream->pastream);
        if (status != paNoError) {
            LOG_ERROR("sndbridge_seek() failed to abort stream %i: %s", stream_id, Pa_GetErrorText(status));
        }
    }

    if (stream->keep_alive) wait_and_halt(stream, status == 1);

    stream->buffer_used = 0;
    stream->played_time = milliseconds / 1000.0;
    oggdecoder_seek(stream->oggdecoder, milliseconds);

    if (milliseconds >= stream->duration) {
        stream->completed = true;
        stream->fetching = true;
        return;
    }

    stream->completed = false;
    stream->fetching = false;

    if (status == 1) {
        if (stream->keep_alive) {
            stream->halt = false;
            return;
        }

        status = Pa_StartStream(stream->pastream);
        if (status != paNoError) {
            LOG_ERROR("sndbridge_seek() failed to resume stream %i: %s", stream_id, Pa_GetErrorText(status));
        }
    }

}

extern void sndbridge_play(StreamID stream_id) {
    GET_STREAM(stream_id);
    if (!stream) return;

    PaError err = Pa_IsStreamActive(stream->pastream);
    switch (err) {
    case 1:
        if (stream->halt) {
            sndbridge_stop(stream_id);
            stream->halt = false;
        }
        return;
    case 0:
        break;
    default:
        LOG_ERROR("sndbridge_play() status failed on stream %i: %s", stream_id, Pa_GetErrorText(err));
        return;
    }

    // seek to the beginning if the playback has ended
    if (((stream->played_time * 1000.0) >= stream->duration) || stream->completed) {
        sndbridge_stop(stream_id);
    }

    stream->fade_duration = 0;
    stream->buffer_used = 0;
    stream->fetching = false;
    stream->completed = false;
    stream->buffer_used = 0;
    stream->halt = false;

    err = Pa_StartStream(stream->pastream);
    if (err != paNoError) {
        LOG_ERROR("sndbridge_play() start failed on stream %i: %s", stream_id, Pa_GetErrorText(err));
        return;
    }
}

extern void sndbridge_pause(StreamID stream_id) {
    GET_STREAM(stream_id);
    if (!stream) return;

    PaError err = Pa_IsStreamStopped(stream->pastream);
    switch (err) {
    case 1:
        return;
    case 0:
        if (stream->halt) return;
        break;
    default:
        LOG_ERROR("sndbridge_pause() status failed on stream %i: %s", stream_id, Pa_GetErrorText(err));
        return;
    }

    if (stream->fetching) {
        stream->played_time += Pa_GetStreamTime(stream->pastream) - stream->timestamp;
    }

    if (!stream->keep_alive) {
        err = Pa_AbortStream(stream->pastream);
        if (err != paNoError) {
            LOG_ERROR("sndbridge_pause() stop failed on stream %i: %s", stream_id, Pa_GetErrorText(err));
        }
    }

    if (stream->keep_alive) wait_and_halt(stream, true);
    stream->buffer_used = 0;
    stream->fade_duration = 0.0;
    stream->fetching = false;
    stream->completed = false;
    oggdecoder_seek(stream->oggdecoder, stream->played_time * 1000.0);
}

extern void sndbridge_stop(StreamID stream_id) {
    GET_STREAM(stream_id);
    if (!stream) return;

    PaError err = Pa_IsStreamStopped(stream->pastream);
    switch (err) {
    case 1:
        goto L_reset_stream;
    case 0:
        if (stream->keep_alive) goto L_reset_stream;
        break;
    default:
        LOG_ERROR("sndbridge_stop() status failed on stream %i: %s", stream_id, Pa_GetErrorText(err));
        return;
    }

    err = Pa_AbortStream(stream->pastream);
    if (err != paNoError) {
        LOG_ERROR("sndbridge_stop() close failed on stream %i: %s", stream_id, Pa_GetErrorText(err));
        return;
    }

L_reset_stream:
    if (stream->keep_alive) wait_and_halt(stream, true);
    stream->buffer_used = 0;
    stream->played_time = 0.0;
    stream->fade_duration = 0.0;
    stream->completed = false;
    stream->fetching = false;
    oggdecoder_seek(stream->oggdecoder, 0.0);
}


extern void sndbridge_set_volume(StreamID stream_id, float volume) {
    GET_STREAM(stream_id);
    if (!stream) return;

    stream->volume = volume;
    stream->fade_duration = 0.0f;
}

extern void sndbridge_mute(StreamID stream_id, bool muted) {
    GET_STREAM(stream_id);
    if (!stream) return;

    stream->muted = muted;
    stream->fade_duration = 0.0f;
}

extern void sndbridge_do_fade(StreamID stream_id, bool fade_in_or_out, float milliseconds) {
    GET_STREAM(stream_id);
    if (!stream) return;

    stream->fade_in_or_out = fade_in_or_out;
    stream->fade_progress = 0.0f;
    stream->fade_duration = milliseconds * stream->milliseconds_to_samples;
}

extern bool sndbridge_is_active(StreamID stream_id) {
    GET_STREAM(stream_id);
    if (!stream) return false;

    if (stream->keep_alive) return !stream->halt;

    return Pa_IsStreamActive(stream->pastream) == 0x01;
}

extern bool sndbridge_is_fade_active(StreamID stream_id) {
    GET_STREAM(stream_id);
    if (!stream) return false;
    
    if (Pa_IsStreamActive(stream->pastream) != 0x01) return false;
    return stream->fade_progress < stream->fade_duration && stream->fade_duration > 0;
}

extern bool sndbridge_has_ended(StreamID stream_id) {
    GET_STREAM(stream_id);
    if (!stream) return false;

    return stream->completed;
}

extern void sndbridge_loop(StreamID stream_id, bool enable) {
    GET_STREAM(stream_id);
    if (!stream) return;

    stream->looped = enable == 0 ? false : true;
}

extern void sndbridge_set_master_volume(float volume) {
    master_volume = CLAMP_FLOAT(volume);
}

extern const char* sndbridge_get_runtime_info() {
    if (backend_initialize) {
        backend_initialize = false;

        PaError err = Pa_Initialize();
        if (err != paNoError) {
            return Pa_GetErrorText(err);
        }
    }

    PaHostApiIndex default_api_index = Pa_GetDefaultHostApi();
    PaHostApiIndex count = Pa_GetHostApiCount();

    const char* default_name = NULL;
    const char* desired_api_name = NULL;
    const char* device_name = NULL;

    const PaHostApiInfo* default_api_info = Pa_GetHostApiInfo(default_api_index);
    if (default_api_info) default_name = default_api_info->name;
    if (default_api_info && default_api_info->defaultOutputDevice != paNoDevice) {
        device_name = Pa_GetDeviceInfo(default_api_info->defaultOutputDevice)->name;
    }

    for (PaHostApiIndex index = 0; index < count; index++) {
        const PaHostApiInfo* info = Pa_GetHostApiInfo(index);
        if (info->type != DESIRED_API_ENUM) continue;

        desired_api_name = DESIRED_API_NAME;//info->name;
        if (info->defaultOutputDevice != paNoDevice) {
            device_name = Pa_GetDeviceInfo(info->defaultOutputDevice)->name;
        }
        break;
    }

    int size = snprintf(runtime_info, sizeof(runtime_info), "%s\nAudio API: default=%s desired=%s output=%s",
        Pa_GetVersionText(), default_name, desired_api_name, device_name
    );
    runtime_info[size] = '\0';
    return runtime_info;
}

#ifdef DEBUG
int main() {
    printf("%s\n", sndbridge_get_runtime_info());

    /*FileHandle_t* mem = filehandle_init("./vorbis.ogg");
    assert(mem);
    StreamID stream_id = sndbridge_queue_ogg(mem);*/

    FileHandle_t* mem = filehandle_init("./vorbis.ogg");
    StreamID stream_id = sndbridge_queue_ogg(mem);


    /*bool a = true;
    bool b = true;
    bool c = true;
    bool d = true;
    sndbridge_play(stream_id);
    while (true) {
        double duration = sndbridge_duration(stream_id);
        double position = sndbridge_position(stream_id);
        printf("duration=%f  time=%f\n", duration, position);
        Pa_Sleep(100);
        //if (i == 6) sndbridge_pause(stream_id);
        //if (i == 10) sndbridge_play(stream_id);
        if (position >= (1000+duration)) break;
        if (position >= 5000 && a) {
            a = false;
            sndbridge_seek(stream_id, 1000.0);
        }
        if (position > 3000 && b) {
            sndbridge_pause(stream_id);
            Pa_Sleep(1000);

            duration = sndbridge_duration(stream_id);
            position = sndbridge_position(stream_id);
            printf("(paused) duration=%f  time=%f\n", duration, position);
            sndbridge_play(stream_id);
            b = false;
        }
        if (position > 13000 && c) {
            sndbridge_pause(stream_id);
            Pa_Sleep(1000);

            duration = sndbridge_duration(stream_id);
            position = sndbridge_position(stream_id);
            printf("(paused) duration=%f  time=%f\n", duration, position);
            sndbridge_play(stream_id);
            c = false;
        }
        if (position > 17000 && d) {
            sndbridge_stop(stream_id);
            Pa_Sleep(1000);

            duration = sndbridge_duration(stream_id);
            position = sndbridge_position(stream_id);
            printf("(stop) duration=%f  time=%f\n", duration, position);
            sndbridge_play(stream_id);
            d = false;
        }
    }*/

    /*while (true) {
        sndbridge_play(stream_id);
        Pa_Sleep(2000);
        sndbridge_pause(stream_id);
        Pa_Sleep(2000);
        //sndbridge_seek(stream_id, 0.0);
    }*/

    sndbridge_stop(stream_id);
    sndbridge_play(stream_id);

    while (true) {
        double duration = sndbridge_duration(stream_id);
        double position = sndbridge_position(stream_id);
        printf("duration=%f  time=%f\n", duration, position);
        Pa_Sleep(100);
        if (sndbridge_has_ended(stream_id)) break;
    }

    printf("playback done\n");

    sndbridge_dispose(stream_id);
}

#endif
