#include "ffgraph.h"
#include <assert.h>
#include <string.h>

static const enum AVPixelFormat PIX_FMT = AV_PIX_FMT_RGB24;

typedef struct {
    struct SwsContext* sws_ctx;
    AVFrame* frame;
    uint8_t* buffer;
    int32_t buffer_size;
    int width;
    int height;
    double seconds;
    double frame_duration;
} VideoConverter;


static bool video_process(FFGraphConversor* ffgraphconv, const AVCodecContext* codec_ctx, const AVFrame* av_frame) {
    if (!av_frame) return false;

    VideoConverter* obj = ffgraphconv->priv_data;
    const uint8_t* const* src_data = (const uint8_t* const*)av_frame->data;

    sws_scale(
        obj->sws_ctx,
        src_data, av_frame->linesize,
        0, codec_ctx->height,
        obj->frame->data, obj->frame->linesize
    );

    obj->seconds = calculate_seconds(ffgraphconv->av_stream, av_frame);
    obj->frame_duration = av_frame->duration;

    return false;
}

static void video_destroy(VideoConverter* conv_ctx) {
    sws_freeContext(conv_ctx->sws_ctx);
    if (conv_ctx->buffer) av_free(conv_ctx->buffer);
    if (conv_ctx->frame) av_frame_free(&conv_ctx->frame);
    av_free(conv_ctx);
}

static void video_destroy_from_conversor(FFGraphConversor* ffgraphconv) {
    if (!ffgraphconv || !ffgraphconv->priv_data) return;

    video_destroy(ffgraphconv->priv_data);
    ffgraphconv->priv_data = NULL;
}


bool videoconverter_init(const AVCodecContext* codec_ctx, FFGraphConversor* ffgraphconv) {
    assert(codec_ctx);

    int size = av_image_get_buffer_size(PIX_FMT, codec_ctx->width, codec_ctx->height, 1);
    if (size < 0) {
        printf("videoconverter_init() call to av_image_get_buffer_size() failed.\n");
        return false;
    }
    uint8_t* buffer = av_malloc(size);
    if (!buffer) {
        printf("videoconverter_init() failed to allocate the buffer.\n");
        return false;
    }

    VideoConverter* obj = av_malloc(sizeof(VideoConverter));
    obj->frame = NULL;
    obj->sws_ctx = NULL;
    obj->buffer = buffer;
    obj->buffer_size = size;
    obj->width = codec_ctx->width;
    obj->height = codec_ctx->height;
    obj->seconds = -1.0;
    obj->frame_duration = codec_ctx->framerate.den / (double)codec_ctx->framerate.num;

    if (codec_ctx->framerate.num == 0 && codec_ctx->framerate.den == 1) obj->frame_duration = 0.0;

    obj->sws_ctx = sws_getContext(
        codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt,
        codec_ctx->width, codec_ctx->height, PIX_FMT,
        SWS_BILINEAR,
        NULL, NULL, NULL
    );
    if (!obj->sws_ctx) {
        printf("video_process() call to sws_getContext() failed.\n");
        goto L_failed;
    }

    obj->frame = av_frame_alloc();
    if (!obj->frame) {
        printf("videoconverter_init() call to av_frame_alloc() failed.\n");
        goto L_failed;
    }

    av_image_fill_arrays(
        obj->frame->data, obj->frame->linesize, buffer, PIX_FMT, codec_ctx->width, codec_ctx->height, 1
    );

    ffgraphconv->priv_data = obj;
    ffgraphconv->convert_cb = video_process;
    ffgraphconv->destroy_cb = video_destroy_from_conversor;
    ffgraphconv->flush_cb = NULL;
    ffgraphconv->bufferseek_cb = NULL;
    return true;

L_failed:
    video_destroy(obj);
    return false;
}

void videoconverter_get_stream_info(FFGraphConversor* ffgraphconv, int* out_width, int* out_height) {
    VideoConverter* obj = ffgraphconv->priv_data;
    *out_width = obj->width;
    *out_height = obj->height;
}

double videoconverter_read(FFGraphConversor* ffgraphconv, double seconds, void** frame_out, int32_t* frame_size_out) {
    VideoConverter* obj = ffgraphconv->priv_data;

    // check if there no frame at all
    if (obj->seconds < 0) return -1.0;

    // check if the buffered frame is future
    if (seconds >= 0 && obj->seconds > 0 && obj->seconds > seconds) {
        return -1.0;
    }

    *frame_out = obj->buffer;
    *frame_size_out = obj->buffer_size;

    return obj->seconds;
}
