#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#include "oggdecoder.h"
#include "oggdecoders.h"
#include "oggutil.h"


typedef struct {
    void* oggdecoder;
    ExternalDecoder extdec;
    ExternalDecoderDestroyCB destroy_func;
    ExternalDecoderReadCB read_func;
    ExternalDecoderInfoCB info_func;
    ExternalDecoderSeekCB seek_func;
    ExternalDecoderLoopCB loop_func;
} WrapperOggDecoder;

typedef void* (*InitFn)(FileHandle_t* file_hnd);


static void wrapper_destroy(WrapperOggDecoder* wrapper) {
    wrapper->destroy_func(wrapper->oggdecoder);
    free(wrapper);
}

static int32_t wrapper_read(WrapperOggDecoder* wrapper, float* buffer, int32_t samples_per_channel) {
    return wrapper->read_func(wrapper->oggdecoder, buffer, samples_per_channel);
}

static void wrapper_info(WrapperOggDecoder* wrapper, int32_t* rate, int32_t* channels, double* duration) {
    wrapper->info_func(wrapper->oggdecoder, rate, channels, duration);
}

static bool wrapper_seek(WrapperOggDecoder* wrapper, double timestamp) {
    return wrapper->seek_func(wrapper->oggdecoder, timestamp);
}

static void wrapper_loop(WrapperOggDecoder* wrapper, int64_t* loop_start, int64_t* loop_length) {
    wrapper->loop_func(wrapper->oggdecoder, loop_start, loop_length);
}


ExternalDecoder* oggdecoder_init(FileHandle_t* file_hnd) {
    assert(file_hnd);
    assert(file_hnd->read);
    assert(file_hnd->seek);
    assert(file_hnd->tell);

    WrapperOggDecoder* wrapper = malloc(sizeof(WrapperOggDecoder));
    assert(wrapper);

    int result = oggutil_get_ogg_codec(file_hnd);
    InitFn init = NULL;

    switch (result) {
        case OGGUTIL_CODEC_VORBIS:;
            init = (InitFn)oggvorbisdecoder_init;
            wrapper->destroy_func = (ExternalDecoderDestroyCB)oggvorbisdecoder_destroy;
            wrapper->info_func = (ExternalDecoderInfoCB)oggvorbisdecoder_get_info;
            wrapper->read_func = (ExternalDecoderReadCB)oggvorbisdecoder_read;
            wrapper->seek_func = (ExternalDecoderSeekCB)oggvorbisdecoder_seek;
            wrapper->loop_func = (ExternalDecoderLoopCB)oggvorbisdecoder_get_loop_points;
            break;
#ifdef SNDBRIDGE_OPUS_DECODING
        case OGGUTIL_CODEC_OPUS:;
            init = (InitFn)oggopusdecoder_init;
            wrapper->destroy_func = (ExternalDecoderDestroyCB)oggopusdecoder_destructor;
            wrapper->info_func = (ExternalDecoderInfoCB)oggopusdecoder_get_info;
            wrapper->read_func = (ExternalDecoderReadCB)oggopusdecoder_read;
            wrapper->seek_func = (ExternalDecoderSeekCB)oggopusdecoder_seek;
            wrapper->loop_func = (ExternalDecoderLoopCB)oggopusdecoder_get_loop_points;
            break;
#endif
        case OGGUTIL_CODEC_ERROR:
            fprintf(stderr, "oggdecoder_init() can not identify the audio codec\n");
            goto L_failed;
        case OGGUTIL_CODEC_UNKNOWN:
            fprintf(stderr, "oggdecoder_init() unknown audio codec\n");
            goto L_failed;
    }

    wrapper->oggdecoder = init(file_hnd);
    if (!wrapper->oggdecoder) {
        fprintf(
            stderr,
            "oggdecoder_init() can not initialize the decoder for: %s\n",
            result == OGGUTIL_CODEC_OPUS ? "opus" : "vorbis"
        );
        goto L_failed;
    }

    wrapper->extdec.decoder = wrapper;
    wrapper->extdec.destroy_func = (ExternalDecoderDestroyCB)wrapper_destroy;
    wrapper->extdec.info_func = (ExternalDecoderInfoCB)wrapper_info;
    wrapper->extdec.read_func = (ExternalDecoderReadCB)wrapper_read;
    wrapper->extdec.seek_func = (ExternalDecoderSeekCB)wrapper_seek;
    wrapper->extdec.loop_func = (ExternalDecoderLoopCB)wrapper_loop;

    // mimic IDisposable interface
    return &wrapper->extdec;

L_failed:
    free(wrapper);
    return NULL;
}
