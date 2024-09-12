#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arch/timer.h>
#include <dc/sound/stream.h>
#include <dc/spu.h>
#include <kos/mutex.h>
#include <kos/thread.h>

#include "condevt.h"
#include "externals/sndbridge/oggutil.h"
#include "externals/sndbridge/sndbridge.h"
#include "externals/sndbridge/wavutil.h"
#include "logger.h"
#include "malloc_utils.h"
#include "math2d.h"
#include "snd_stream2.h"


#ifndef _arch_dreamcast
#error "dreamcast version of SoundBridge only works in milliseconds"
#endif

static const int32_t HOLE = -3;                  // retured by ov_read and op_read, means bad/missing ogg packets
static const unsigned int BACKEND_INTERVAL = 50; // 50ms
static const char BACKEND_NAME[] = "KallistiOS AICA streaming driver";
static const int32_t MINIMUM_READ = 128;

#ifdef SNDBRIDGE_DREAMCAST_STUB
static const uint32_t MIN_DURATION_KEEP_ALIVE = 500; // 500 milliseconds
static const uint32_t WAIT_AND_HALT_TIMEOUT = 1000;  // 1000 milliseconds

static void wait_and_halt(Stream* stream, bool halt);
#endif

static volatile int32_t STREAM_IDS;
static volatile bool master_mute;
static volatile float master_volume;
static bool backend_available;


static inline int32_t dec_readBytes(DecoderHandle* dec, void* buffer, uint32_t buffer_size) {
    return dec->readBytes(dec, buffer, buffer_size);
}
static inline SampleFormat dec_getInfo(DecoderHandle* dec, uint32_t* rate, uint32_t* channels, float64* duration_sec) {
    return dec->getInfo(dec, rate, channels, duration_sec);
}
static inline bool dec_seek(DecoderHandle* dec, float64 seconds) {
    return dec->seek(dec, seconds);
}
static inline void dec_getLoopPoints(DecoderHandle* dec, int64_t* loop_start, int64_t* loop_length) {
    return dec->getLoopPoints(dec, loop_start, loop_length);
}
static inline void dec_destroy(DecoderHandle* dec) {
    return dec->destroy(dec);
}
static inline void srchnd_suspend(SourceHandle* srchnd) {
    if (srchnd) {
        srchnd->suspend(srchnd);
    }
}
static inline void print_stream_error1(const char* str, int32_t stream_id, int err) {
    logger_error("sndbridge: id=" FMT_I4 " %s%i", stream_id, str, err);
}
static inline void print_stream_error2(const char* str, int err) {
    logger_error("sndbridge: %s%i", str, err);
}

static void* sndbridge_thread(void* arg);
static void* read_cb(snd_stream_hnd_t hnd, int bytes_req, int* bytes_recv);


struct Stream {
    int32_t id;
    DecoderHandle* decoder;
    int32_t channels;
    uint32_t sample_rate;
    bool is_adpcm_or_pcm16;

    SourceHandle* sourcehandle;

    volatile snd_stream_hnd_t stream_hnd;
    uint64_t stream_ms;
    bool stream_running;

    int32_t played_ms;
    int32_t duration_ms;
    uint32_t position_in_bytes;
    int32_t buffer_size_in_bytes;

    volatile float volume;
    volatile bool muted;
    volatile bool looped;
    volatile bool completed;
#ifdef SNDBRIDGE_DREAMCAST_STUB
    volatile bool keep_alive;
    volatile bool halt;
    volatile bool callback_running;
#endif
    volatile bool looped_needs_time_reset;
    volatile bool fetching;

    volatile bool queueing_enabled;

    float fade_duration_ms;
    int32_t fade_progress;
    bool fade_out;

    bool has_loop_points;
    uint32_t loop_end_bytes;
    uint32_t loop_start_bytes;
    float64 loop_start_seconds;

    uint8_t* buffer;
    int32_t buffer_bytes_used;
    int32_t buffer_last_bytes_readed;

    kthread_t* backend_thread;
    condevt_t backend_barrier;
    volatile bool backend_run;
    mutex_t backend_mutex;
    uint64_t backend_end_ms;
    float last_master_volume;
};


static inline uint32_t milliseconds_to_bytes(Stream* stream, int64_t milliseconds) {
    int64_t result = (milliseconds * stream->sample_rate);

    uint32_t rate = 0;
    uint32_t channels = 0;
    float64 duration_sec = DOUBLE_NaN;

    SampleFormat fmt = dec_getInfo(stream->decoder, &rate, &channels, &duration_sec);

    switch (fmt) {
        case SampleFormat_PCM_U8:
            result *= channels;
            break;
        case SampleFormat_PCM_S16LE:
            result *= sizeof(int16_t) * channels;
            break;
        case SampleFormat_ADPCM_4_YAMAHA:
            result = (result * channels) / 2;
            break;
    }

    return result / (int64_t)1000;
}

static void set_stream_volume(Stream* stream, float volume_modifier) {
    if (master_mute || stream->muted) {
        snd_stream_volume(stream->stream_hnd, 0);
        return;
    }

    int vol = (int)(stream->volume * master_volume * volume_modifier * 255.0f);

    if (vol < 0)
        vol = 0;
    else if (vol > 255)
        vol = 255;

    stream->last_master_volume = master_volume;

    snd_stream_volume(stream->stream_hnd, vol);
}

static bool start_stream(Stream* stream) {
    if (stream->stream_hnd == SND_STREAM_INVALID) {
        int bufsize = stream->is_adpcm_or_pcm16 ? SND_STREAM2_BUFFER_ADPCM_MAX : SND_STREAM_BUFFER_MAX;
        stream->stream_hnd = snd_stream_alloc(read_cb, bufsize);
        if (stream->stream_hnd == SND_STREAM_INVALID) {
            logger_error("sndbridge: the snd_stream_alloc() has failed");
            return false;
        }
    }

    snd_stream_set_userdata(stream->stream_hnd, stream);

    uint32_t rate, channels;
    float64 duration_in_seconds;

    dec_getInfo(stream->decoder, &rate, &channels, &duration_in_seconds);

    if (stream->queueing_enabled) snd_stream_queue_enable(stream->stream_hnd);

    if (stream->is_adpcm_or_pcm16)
        snd_stream_start_adpcm(stream->stream_hnd, rate, channels > 1);
    else
        snd_stream_start(stream->stream_hnd, rate, channels > 1);

    set_stream_volume(stream, 1.0f);

    stream->stream_running = true;
    stream->fetching = false;
    stream->backend_end_ms = 0;

    if (stream->queueing_enabled) snd_stream_queue_disable(stream->stream_hnd);

    condevt_signal(&stream->backend_barrier);

    return true;
}

static void stop_stream(Stream* stream) {
    if (stream->stream_hnd == SND_STREAM_INVALID) {
        return;
    }

    mutex_lock(&stream->backend_mutex);
    {
        condevt_reset(&stream->backend_barrier);

        snd_stream_stop(stream->stream_hnd);
        snd_stream_destroy(stream->stream_hnd);

        stream->stream_running = false;
        stream->stream_hnd = SND_STREAM_INVALID;
        stream->backend_end_ms = 0;
        stream->fetching = false;
    }
    mutex_unlock(&stream->backend_mutex);
}


///////////////////
// Stream methods
//////////////////

int32_t sndbridge_stream_duration(Stream* stream) {
    return stream->duration_ms;
}

int32_t sndbridge_stream_position(Stream* stream) {
    if (!stream->stream_running || !stream->fetching) {
        return stream->played_ms;
    }

    int32_t position = stream->played_ms;

    position += (int32_t)(timer_ms_gettime64() - stream->stream_ms);

    return position;
}


void sndbridge_stream_seek(Stream* stream, int32_t milliseconds) {
    if (milliseconds < 0) milliseconds = 0;

    bool running = stream->stream_running;

#ifdef SNDBRIDGE_DREAMCAST_STUB
    if (running && !stream->keep_alive) {
        stop_stream(stream);
    }

    bool has_halt = stream->halt;
    if (stream->keep_alive) wait_and_halt(stream, running);
#else
    if (running) {
        stop_stream(stream);
    }
#endif

    stream->buffer_bytes_used = 0;
    stream->buffer_last_bytes_readed = 0;
    stream->backend_end_ms = 0;
    stream->played_ms = milliseconds;
    stream->looped_needs_time_reset = false;
    stream->position_in_bytes = milliseconds_to_bytes(stream, milliseconds);

    dec_seek(stream->decoder, (float64)milliseconds / 1000.0);

    if (milliseconds >= stream->duration_ms) {
        stream->completed = true;
        return;
    }

    stream->completed = false;

    if (running) {
#ifdef SNDBRIDGE_DREAMCAST_STUB
        if (stream->keep_alive) {
            if (!has_halt) stream->halt = false;
            return;
        }
#endif

        start_stream(stream);
    }
}

void sndbridge_stream_play(Stream* stream) {
    if (stream->stream_running) {
#ifdef SNDBRIDGE_DREAMCAST_STUB
        if (stream->halt) {
            sndbridge_stream_stop(stream);
            stream->looped_needs_time_reset = false;
            stream->halt = false;
        }
#endif
        return;
    }

    // seek to the beginning if the playback has ended
    if (stream->played_ms >= stream->duration_ms || stream->completed) {
        sndbridge_stream_stop(stream);
    }

    stream->fade_duration_ms = 0.0f;
    stream->buffer_bytes_used = 0;
    stream->buffer_last_bytes_readed = 0;
    stream->backend_end_ms = 0;
    stream->completed = false;
#ifdef SNDBRIDGE_DREAMCAST_STUB
    stream->halt = false;
#endif

    start_stream(stream);
}

void sndbridge_stream_pause(Stream* stream) {
#ifdef SNDBRIDGE_DREAMCAST_STUB
    if (!stream->stream_running || stream->halt) {
        return;
    }
#else
    if (!stream->stream_running) {
        return;
    }
#endif

    stream->played_ms += (int32_t)(timer_ms_gettime64() - stream->stream_ms);

#ifdef SNDBRIDGE_DREAMCAST_STUB
    if (!stream->keep_alive) {
        stop_stream(stream);
    }
    if (stream->keep_alive) wait_and_halt(stream, true);
#else
    stop_stream(stream);
#endif

    stream->buffer_bytes_used = 0;
    stream->buffer_last_bytes_readed = 0;
    stream->backend_end_ms = 0;
    stream->fade_duration_ms = 0.0f;
    stream->completed = false;
    stream->looped_needs_time_reset = false;
    stream->position_in_bytes = milliseconds_to_bytes(stream, stream->played_ms);
    dec_seek(stream->decoder, stream->played_ms / 1000.0);
    srchnd_suspend(stream->sourcehandle);
}

void sndbridge_stream_stop(Stream* stream) {
#ifdef SNDBRIDGE_DREAMCAST_STUB
    if (!stream->stream_running || stream->keep_alive) {
        goto L_reset_stream;
    }
#else
    if (!stream->stream_running) {
        goto L_reset_stream;
    }
#endif

    stop_stream(stream);

L_reset_stream:
#ifdef SNDBRIDGE_DREAMCAST_STUB
    if (stream->keep_alive) wait_and_halt(stream, true);
#endif
    stream->buffer_bytes_used = 0;
    stream->buffer_last_bytes_readed = 0;
    stream->backend_end_ms = 0;
    stream->played_ms = 0;
    stream->fade_duration_ms = 0.0f;
    stream->completed = false;
    stream->looped_needs_time_reset = false;
    stream->position_in_bytes = 0;
    dec_seek(stream->decoder, 0.0);
    srchnd_suspend(stream->sourcehandle);
}


void sndbridge_stream_set_volume(Stream* stream, float volume) {
    stream->volume = math2d_clamp_float(volume, 0.0f, 1.0f);
    stream->fade_duration_ms = 0.0f;

    if (stream->stream_running)
        set_stream_volume(stream, 1.0f);
}

void sndbridge_stream_mute(Stream* stream, bool muted) {
    stream->muted = muted;
    stream->fade_duration_ms = 0.0f;

    if (stream->stream_running)
        set_stream_volume(stream, 1.0f);
}

void sndbridge_stream_do_fade(Stream* stream, bool fade_in_or_out, float seconds) {
    if (seconds <= FLT_MIN) {
        // cancel any active fade
        stream->fade_duration_ms = 0.0f;
        return;
    }

    stream->fade_out = !fade_in_or_out;
    stream->fade_progress = 0;
    stream->fade_duration_ms = seconds * 1000.0f;
}

bool sndbridge_stream_is_active(Stream* stream) {
#ifdef SNDBRIDGE_DREAMCAST_STUB
    if (stream->keep_alive) return !stream->halt;
#endif

    return stream->stream_running;
}

StreamFading sndbridge_stream_active_fade(Stream* stream) {
    if (!stream->stream_running) return STREAMFADING_NONE;
    if (stream->fade_duration_ms < 1) return STREAMFADING_NONE;

    return stream->fade_out ? STREAMFADING_OUT : STREAMFADING_IN;
}

bool sndbridge_stream_has_ended(Stream* stream) {
    return stream->completed;
}

void sndbridge_stream_set_looped(Stream* stream, bool enable) {
    stream->looped = enable;
}

void sndbridge_stream_set_queueing(Stream* stream, bool enable) {
    stream->queueing_enabled = enable;
}

void sndbridge_stream_force_resync(Stream* stream) {
    stream->stream_ms = timer_ms_gettime64();
}

void sndbridge_stream_destroy(Stream* stream) {
    if (stream->backend_thread) {
        stream->backend_run = false;
        condevt_signal(&stream->backend_barrier);
        thd_join(stream->backend_thread, NULL);
    }

    if (stream->stream_hnd != SND_STREAM_INVALID) {
        snd_stream_destroy(stream->stream_hnd);
        stream->stream_hnd = SND_STREAM_INVALID;
    }

    dec_destroy(stream->decoder);
    free_chk(stream->buffer);
    condevt_destroy(&stream->backend_barrier);
    mutex_destroy(&stream->backend_mutex);
    free_chk(stream);
}



void sndbridge_init() {
    STREAM_IDS = 0;
    master_mute = false;
    master_volume = 1.0f;
    backend_available = snd_stream_init() == 0;
}

#ifdef SNDBRIDGE_DREAMCAST_STUB
static void wait_and_halt(Stream* stream, bool halt) {
    int32_t elapsed = 0;
    while (stream->callback_running && elapsed < WAIT_AND_HALT_TIMEOUT) {
        thd_sleep(10);
        elapsed += 10;
    }
    stream->halt = halt;
}
#endif


static inline int32_t processing_read_samples(Stream* stream, int bytes_required) {
    // assert(bytes_required < stream->buffer_size_in_samples);

    int32_t buffer_bytes_used = stream->buffer_bytes_used;
    int32_t buffer_last_bytes_readed = stream->buffer_last_bytes_readed;
    uint8_t* buffer_offset = stream->buffer + buffer_bytes_used;

    int32_t available = stream->buffer_size_in_bytes - buffer_bytes_used;

    if (stream->looped_needs_time_reset) {
        stream->played_ms = (int32_t)(stream->loop_start_seconds / (float64)1000.0); // this is zero if loop points are absent
        stream->looped_needs_time_reset = false;
    }

    if (buffer_last_bytes_readed > 0 /*&& buffer_bytes_used > 0*/) {
        uint8_t* buffer = stream->buffer;
        memmove(buffer, buffer + buffer_last_bytes_readed, (size_t)buffer_bytes_used);
    }

    while (available >= MINIMUM_READ) {
        int32_t readed_bytes = dec_readBytes(stream->decoder, buffer_offset, (uint32_t)available);
        if (readed_bytes < 1) {
            if (readed_bytes == HOLE) continue;

            // EOF reached, if looped seek to the start
            if (stream->looped) {
                if (stream->has_loop_points) {
                    // this never should happen
                    dec_seek(stream->decoder, stream->loop_start_seconds);
                    stream->position_in_bytes = stream->loop_start_bytes;
                    stream->looped_needs_time_reset = true;
                    continue;
                }

                dec_seek(stream->decoder, 0.0);
                stream->looped_needs_time_reset = true;
                continue;
            }
            break;
        }

        // check loop points
        if (stream->looped && stream->has_loop_points && readed_bytes > 0) {
            uint32_t total_readed = stream->position_in_bytes + (uint32_t)readed_bytes;

            if (total_readed < stream->loop_end_bytes) {
                stream->position_in_bytes = total_readed;
            } else {
                // discard last bytes
                int32_t to_discard = (int32_t)((int64_t)total_readed - (int64_t)stream->loop_end_bytes);
                if (to_discard > 0) {
                    if (to_discard > readed_bytes)
                        readed_bytes = 0;
                    else
                        readed_bytes -= to_discard;
                }

                // seek to the loop start position
                dec_seek(stream->decoder, stream->loop_start_seconds);
                stream->position_in_bytes = stream->loop_start_bytes;
                stream->looped_needs_time_reset = true;
            }
        }

        buffer_offset += readed_bytes;
        available -= readed_bytes;
        buffer_bytes_used += readed_bytes;
    }

    if (buffer_bytes_used < bytes_required) bytes_required = buffer_bytes_used;

    stream->buffer_bytes_used = buffer_bytes_used - bytes_required;
    stream->buffer_last_bytes_readed = bytes_required;

    return bytes_required;
}

static inline void completed_cb(Stream* stream) {
#ifdef SNDBRIDGE_DREAMCAST_STUB
    stream->callback_running = false;
#endif

    stream->completed = true;
    stream->stream_running = false;
    stream->fetching = false;
    stream->played_ms = stream->played_ms + (int32_t)(timer_ms_gettime64() - stream->stream_ms);

    condevt_reset(&stream->backend_barrier);

    if (stream->stream_hnd != SND_STREAM_INVALID) {
        snd_stream_stop(stream->stream_hnd);
        snd_stream_destroy(stream->stream_hnd);

        stream->stream_hnd = SND_STREAM_INVALID;
    }

    srchnd_suspend(stream->sourcehandle);

#if DEBUG && SNDBRIDGE_DEBUG_COMPLETE
    int32_t position = (int32_t)(timer_ms_gettime64() - stream->stream_ms);
    logger_info("sndbridge: END stream_id=" FMT_I4 " position_ms=" FMT_I4 "", stream->id, position);
#endif
}

static void* read_cb(snd_stream_hnd_t hnd, int bytes_req, int* bytes_recv) {
    Stream* stream = (Stream*)snd_stream_get_userdata(hnd);

#ifdef SNDBRIDGE_DREAMCAST_STUB
    stream->callback_running = true;

    if (stream->halt) {
        if (stream->looped_needs_time_reset) stream->looped_needs_time_reset = false;

        // playback ended, silence output to keep alive the stream
        stream->callback_running = false;

        *bytes_recv = bytes_req;
        memset(stream->buffer, 0, stream->buffer_size_in_bytes);
        return stream->buffer;
    }
#endif

    /*if (!stream->stream_running) {
        memset(stream->buffer, 0x00, stream->buffer_size_in_bytes);
        *bytes_recv = bytes_req;
        return stream->buffer;
    }*/

    if (stream->backend_end_ms > 0) {
        *bytes_recv = 0;
        return NULL;
    }

    // read samples
    int32_t readed_bytes = processing_read_samples(stream, bytes_req);

    if (readed_bytes < bytes_req) {
        // EOF reached in the decoder, estimate the amount of milliseconds remaining to play
        uint64_t now = timer_ms_gettime64();
        uint64_t position_ms = now - stream->stream_ms;
        position_ms += (uint64_t)stream->played_ms;

        stream->backend_end_ms = ((uint64_t)stream->duration_ms - position_ms) + now;

        // add padding silence
        memset(stream->buffer + readed_bytes, 0x00, (size_t)(bytes_req - readed_bytes));
        readed_bytes = bytes_req;
    }

    if (!stream->fetching && stream->stream_running) {
        stream->fetching = true;
        stream->stream_ms = timer_ms_gettime64();
    }

#ifdef SNDBRIDGE_DREAMCAST_STUB
    if (readed_bytes < bytes_req && stream->keep_alive) {
        stream->completed = true;
        stream->halt = true;
        stream->looped_needs_time_reset = true;
    }
#endif

    *bytes_recv = readed_bytes;
    return stream->buffer;
}


static inline float calc_s_curve(float percent) {
    if (percent <= 0.0f) return 0.0f;
    if (percent >= 1.0f) return 1.0f;
    return (math2d_sin((MATH2D_PI * percent) - MATH2D_HALF_PI) / 2.0f) + 0.5f;
}


void sndbridge_dispose_backend() {
    snd_stream_shutdown();
    backend_available = false;
}


StreamResult sndbridge_enqueue1(SourceHandle* source_handle, Stream** created_stream) {
    DecoderHandle* decoder;

    if (wav_is_file_RIFF_WAVE(source_handle))
        decoder = adpcmreader_init(source_handle);
    else
        decoder = oggutil_init_ogg_decoder(source_handle);

    if (decoder == NULL) {
        *created_stream = NULL;
        return STREAMRESULT_DecoderFailed;
    }

    StreamResult res = sndbridge_enqueue2(decoder, created_stream);

    if (res != STREAMRESULT_Success) {
        dec_destroy(decoder);
    }

    (*created_stream)->sourcehandle = source_handle;
    srchnd_suspend(source_handle);

    return res;
}

StreamResult sndbridge_enqueue2(DecoderHandle* external_decoder, Stream** created_stream) {
    *created_stream = NULL;

    if (!backend_available) return STREAMRESULT_BackendFailed;

    uint32_t sample_rate, channels;
    float64 duration;
    SampleFormat sample_format = dec_getInfo(external_decoder, &sample_rate, &channels, &duration);

    if (sample_format != SampleFormat_ADPCM_4_YAMAHA && sample_format != SampleFormat_PCM_S16LE) {
        logger_error("sndbridge: sndbridge_enqueue2() failed, expected sample format adpcm_4_yamaha or pcm_s16le");
        return STREAMRESULT_CreateFailed;
    }

    Stream* stream = *created_stream = malloc_chk(sizeof(Stream));
    if (!stream) {
        logger_error("sndbridge: sndbridge_enqueue2() out-of-memory");
        return STREAMRESULT_CreateFailed;
    }

    *stream = (Stream){
        //.id = -1,
        .decoder = external_decoder,
        .channels = (int32_t)channels,
        .sample_rate = sample_rate,
        .is_adpcm_or_pcm16 = sample_format == SampleFormat_ADPCM_4_YAMAHA,

        .stream_hnd = SND_STREAM_INVALID,
        .stream_ms = 0,
        .stream_running = false,

        .played_ms = 0,
        .duration_ms = (int32_t)(duration * (float64)1000.0),
        .position_in_bytes = 0,
        .buffer_size_in_bytes = 0,

        .volume = 1.0f,
        .muted = false,
        .looped = false,
        .completed = false,
#ifdef SNDBRIDGE_DREAMCAST_STUB
        .keep_alive = false,
        .halt = false,
        .callback_running = false,
#endif
        .looped_needs_time_reset = false,
        .fetching = false,

        .queueing_enabled = false,

        .fade_duration_ms = 0.0f,
        .fade_progress = 0,
        .fade_out = false,

        .has_loop_points = false,
        .loop_end_bytes = 0,
        .loop_start_bytes = 0,
        .loop_start_seconds = 0.0,

        .buffer = NULL,
        .buffer_bytes_used = 0,
        .buffer_last_bytes_readed = 0,

        .backend_run = true,
        .backend_end_ms = 0,
        .last_master_volume = master_volume
    };

    if (sample_format == SampleFormat_ADPCM_4_YAMAHA)
        stream->buffer_size_in_bytes = SND_STREAM2_BUFFER_ADPCM_MAX + 1024;
    else
        stream->buffer_size_in_bytes = SND_STREAM_BUFFER_MAX + 1024;

    int64_t loop_start, loop_length;
    dec_getLoopPoints(external_decoder, &loop_start, &loop_length);

    // check if the loop points are valid
    if (loop_start < 0 || loop_start == loop_length) {
        stream->has_loop_points = false;
    } else {
        stream->loop_start_bytes = (uint32_t)loop_start;

        if (loop_length < 0)
            stream->loop_end_bytes = (uint32_t)((stream->duration_ms / (float64)1000.0) * stream->sample_rate);
        else
            stream->loop_end_bytes = (uint32_t)(loop_length + loop_start);
    }

    if (stream->has_loop_points) {
        stream->loop_start_seconds = loop_start / (float64)sample_rate;
        stream->looped = true; // enable looping by default
        stream->has_loop_points = true;

        switch (sample_format) {
            case SampleFormat_PCM_U8:
                stream->loop_start_bytes *= channels;
                stream->loop_end_bytes *= channels;
                break;
            case SampleFormat_PCM_S16LE:
                uint32_t amount = sizeof(int16_t) * channels;
                stream->loop_start_bytes *= amount;
                stream->loop_end_bytes *= amount;
                break;
            case SampleFormat_ADPCM_4_YAMAHA:
                stream->loop_start_bytes = (stream->loop_start_bytes * channels) / 2;
                stream->loop_end_bytes = (stream->loop_end_bytes * channels) / 2;
                break;
        }
    }


    stream->id = STREAM_IDS++;
    stream->buffer = memalign_chk(32, (size_t)stream->buffer_size_in_bytes);

#ifdef SNDBRIDGE_DREAMCAST_STUB
    stream->keep_alive = stream->duration_ms < MIN_DURATION_KEEP_ALIVE;
#endif

    if (!stream->buffer) {
        logger_error("soundbridge: buffer allocation failed on stream id " FMT_I4, stream->id);
        sndbridge_stream_destroy(stream);
        return STREAMRESULT_CreateFailed;
    }

    // create backend thread to poll the stream data
    bool condevt_ret = condevt_init(&stream->backend_barrier, false, false);
    int mutex_ret = mutex_init(&stream->backend_mutex, MUTEX_TYPE_DEFAULT);
    stream->backend_thread = thd_create(0, sndbridge_thread, stream);

    if (!stream->backend_thread || !condevt_ret || mutex_ret != 0) {
        logger_error(
            "soundbridge: backend failed {id=" FMT_I4 " thd=%i condevt=%i mutex=%i}",
            stream->id, !!stream->backend_thread, condevt_ret, mutex_ret
        );
        sndbridge_stream_destroy(stream);
        return STREAMRESULT_CreateFailed;
    }

    return STREAMRESULT_Success;
}


void sndbridge_set_master_volume(float volume) {
    if (volume > 1.0f)
        master_volume = 1.0f;
    else if (volume < 0.0f)
        master_volume = 0.0f;
    else
        master_volume = volume;
}

void sndbridge_set_master_muted(bool muted) {
    master_mute = muted;
}


const char* sndbridge_get_runtime_info() {
    return BACKEND_NAME;
}



static void process_fade(Stream* stream, int32_t elapsed) {
    float fade_duration_ms = stream->fade_duration_ms;
    if (stream->fade_duration_ms < 1.0f) {
        // if the master volume has changed still do volume update
        if (master_volume != stream->last_master_volume) {
            set_stream_volume(stream, 1.0f);
        }
        return;
    }

    int32_t fade_progress = stream->fade_progress;
    if (fade_progress >= fade_duration_ms) {
        set_stream_volume(stream, stream->fade_out ? 0.0f : 1.0f);
        stream->fade_duration_ms = 0.0f;
        return;
    }

    float fade_percent = fade_progress / fade_duration_ms;
#ifdef SNDBRIDGE_SCURVE
    fade_percent = calc_s_curve(fade_percent);
#endif

    if (stream->fade_out) fade_percent = 1.0f - fade_percent;
    set_stream_volume(stream, fade_percent);

    fade_progress += elapsed;
    stream->fade_progress = fade_progress < fade_duration_ms ? fade_progress : (int32_t)fade_duration_ms;
}

static void* sndbridge_thread(void* arg) {
    Stream* stream = (Stream*)arg;

    uint64_t fade_last_timestamp = 0;
    int32_t elapsed;
    uint64_t now;

    while (stream->backend_run) {
        condevt_wait(&stream->backend_barrier);
        if (!stream->backend_run) break;
        if (!stream->stream_running) continue;

        mutex_lock(&stream->backend_mutex);
        if (!stream->backend_run) break;
        if (!stream->stream_running) continue;

        now = timer_ms_gettime64();

        // process fade
        if (fade_last_timestamp == 0) {
            elapsed = 0;
            fade_last_timestamp = timer_ms_gettime64();
        } else {
            elapsed = (int32_t)(now - fade_last_timestamp);
            fade_last_timestamp = now;
        }
        process_fade(stream, elapsed);

        // poll whatever ended or not
        snd_stream_poll(stream->stream_hnd);

        // wait until the AICA buffers are empty (if has ended)
        uint64_t backend_end_ms = stream->backend_end_ms;
        if (backend_end_ms > 0 && now > backend_end_ms) {
            completed_cb(stream);
            mutex_unlock(&stream->backend_mutex);
            fade_last_timestamp = 0;
            continue;
        }

        // process fade again (in case of high I/O latency)
        now = timer_ms_gettime64();
        elapsed = (int32_t)(now - fade_last_timestamp);
        fade_last_timestamp = now;
        process_fade(stream, elapsed);

        mutex_unlock(&stream->backend_mutex);

        if (!stream->backend_run) break;
        if (!stream->stream_running) continue;
        thd_sleep(BACKEND_INTERVAL);
    }

    return NULL;
}
