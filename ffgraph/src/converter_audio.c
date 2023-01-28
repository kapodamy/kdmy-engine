#include <assert.h>
#include <string.h>

#include "ffgraph.h"

#define BUFFER_LENGTH ((1024 * 1024) / 4)
#define BUFFER_SIZE_MAX (BUFFER_LENGTH / 2)
#define BUFFER_SIZE (BUFFER_LENGTH / 4)

const enum AVSampleFormat SAMPLE_FMT = AV_SAMPLE_FMT_FLT;
const int SAMPLE_RATE = 44100;
const int SAMPLE_SIZE = sizeof(float);

typedef struct {
    struct SwrContext* swr_ctx;
    float* sample_buffer;
    int sample_buffer_used;
    double seconds;
    int channels;
    int sample_rate;
} AudioConverter;



static bool audio_process(FFGraphConversor* ffgraphconv, const AVCodecContext* codec_ctx, const AVFrame* av_frame) {
    (void)codec_ctx;
    AudioConverter* obj = ffgraphconv->priv_data;
    int available = BUFFER_SIZE_MAX - obj->sample_buffer_used;
    uint8_t* buffer = (uint8_t*)(obj->sample_buffer + (obj->sample_buffer_used * obj->channels));
    int count;

    if (av_frame) {
        const uint8_t** samples = (const uint8_t**)av_frame->extended_data;
        count = swr_convert(obj->swr_ctx, &buffer, available, samples, av_frame->nb_samples);
    } else {
        count = swr_convert(obj->swr_ctx, &buffer, available, NULL, 0);
    }

    if (count == 0) {
        return false;
    } else if (count < 0) {
        printf("audio_process() call to swr_convert() failed, reason: %s\n", av_err2str(count));
        return true;
    }

    if (obj->sample_buffer_used < 1 && av_frame) {
        obj->seconds = calculate_seconds(ffgraphconv->av_stream, av_frame);
    }

    obj->sample_buffer_used += count;

    return false;
}

static void audio_flush(FFGraphConversor* ffgraphconv) {
    AudioConverter* obj = ffgraphconv->priv_data;
    obj->sample_buffer_used = 0;

    uint8_t* buffer = (uint8_t*)obj->sample_buffer;
    swr_convert(obj->swr_ctx, &buffer, BUFFER_SIZE_MAX, NULL, 0);
}

static void audio_destroy(AudioConverter* conv_ctx) {
    swr_free(&conv_ctx->swr_ctx);
    av_free(conv_ctx->sample_buffer);
    av_free(conv_ctx);
}

static void audio_destroy_from_conversor(FFGraphConversor* ffgraphconv) {
    if (!ffgraphconv || !ffgraphconv->priv_data) return;

    audio_destroy(ffgraphconv->priv_data);
    ffgraphconv->priv_data = NULL;
}

static void audio_bufferseek(FFGraphConversor* ffgraphconv, double seconds) {
    AudioConverter* obj = ffgraphconv->priv_data;
    double end_seconds = obj->seconds + (obj->sample_buffer_used / (double)obj->sample_rate);

    if (obj->sample_buffer_used < 1 || seconds < obj->seconds || seconds > end_seconds) {
        // drop buffer
        obj->sample_buffer_used = 0;
        return;
    }

    if (obj->seconds == seconds) {
        // nothing to do
        return;
    }

    // calculate the amount of samples to drop
    int32_t forward_samples = (int32_t)ceil((seconds - obj->seconds) * obj->sample_rate);
    size_t byte_offset = forward_samples * obj->channels * SAMPLE_SIZE;

    // drop samples
    int32_t remaining_samples = obj->sample_buffer_used - forward_samples;
    size_t remaining_bytes = remaining_samples * obj->channels * SAMPLE_SIZE;
    uint8_t* remaining_buffer = ((uint8_t*)obj->sample_buffer) + byte_offset;

    memmove(obj->sample_buffer, remaining_buffer, remaining_bytes);

    obj->sample_buffer_used -= forward_samples;
    obj->seconds += forward_samples / obj->sample_rate; // calc again due ceiling
}


bool audioconverter_init(const AVCodecContext* codec_ctx, FFGraphConversor* ffgraphconv) {
    AVChannelLayout out_ch_layout = {.opaque = NULL};
    int channel_count = codec_ctx->ch_layout.nb_channels > 2 ? 2 : codec_ctx->ch_layout.nb_channels;
    int sample_rate = codec_ctx->sample_rate;

    if (sample_rate != 44100 || sample_rate != 48000) sample_rate = SAMPLE_RATE;

    av_channel_layout_default(&out_ch_layout, channel_count);

    // NOTE: the input channel layout can not be changed during decoding.
    struct SwrContext* swr_ctx = swr_alloc();
    int ret = swr_alloc_set_opts2(
        &swr_ctx,
        &out_ch_layout, SAMPLE_FMT, sample_rate,
        &codec_ctx->ch_layout, codec_ctx->sample_fmt, codec_ctx->sample_rate,
        0, NULL
    );

    if (ret < 0) {
        printf("audioconverter_init() call to swr_alloc_set_opts2() failed.\n");
        swr_free(&swr_ctx);
        return false;
    }

    ret = swr_init(swr_ctx); // obligatory
    if (ret < 0) {
        printf("audioconverter_init() call to swr_init() failed.\n");
        swr_free(&swr_ctx);
        return false;
    }

    AudioConverter* obj = av_malloc(sizeof(AudioConverter));
    obj->swr_ctx = swr_ctx;
    obj->sample_buffer = av_malloc(BUFFER_LENGTH * sizeof(float));
    obj->sample_buffer_used = 0;
    obj->channels = channel_count;
    obj->sample_rate = sample_rate;
    obj->seconds = 0;

    assert(obj->sample_buffer);

#ifdef DEBUG
    memset(obj->sample_buffer, 0x00, BUFFER_SIZE_MAX * sizeof(float));
#endif

    ffgraphconv->priv_data = obj;
    ffgraphconv->convert_cb = audio_process;
    ffgraphconv->destroy_cb = audio_destroy_from_conversor;
    ffgraphconv->flush_cb = audio_flush;
    ffgraphconv->bufferseek_cb = audio_bufferseek;

    return true;
}

void audioconverter_get_stream_info(FFGraphConversor* ffgraphconv, int* out_channels, int* out_rate) {
    AudioConverter* obj = ffgraphconv->priv_data;
    *out_channels = obj->channels;
    *out_rate = obj->sample_rate;
}


int32_t audioconverter_read(FFGraphConversor* ffgraphconv, void* samples_out, int32_t max_samples_per_channel) {
    AudioConverter* obj = ffgraphconv->priv_data;

    int32_t required_samples = max_samples_per_channel;
    if (required_samples > obj->sample_buffer_used) required_samples = obj->sample_buffer_used;

    size_t required_bytes = required_samples * obj->channels * SAMPLE_SIZE;
    memcpy(samples_out, obj->sample_buffer, required_bytes);

    if (required_samples >= obj->sample_buffer_used) {
        obj->sample_buffer_used = 0;
        return required_samples;
    }

    // shift all remaining samples
    obj->sample_buffer_used -= required_samples;
    int32_t remaining_samples = obj->sample_buffer_used;
    size_t remaining_bytes = remaining_samples * obj->channels * SAMPLE_SIZE;
    memmove(obj->sample_buffer, required_bytes + (uint8_t*)obj->sample_buffer, remaining_bytes);

    // calculate time
    obj->seconds += required_samples / (double)obj->sample_rate;

    return required_samples;
}
