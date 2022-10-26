#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>

#include "oggdecoders.h"
#include "oggdecoder.h"
#include "oggutil.h"


extern OggDecoder* oggdecoder_init(FileHandle_t* filehandle) {
    assert(filehandle);
    assert(filehandle->read);
    assert(filehandle->seek);
    assert(filehandle->tell);

    OggDecoder* oggdecoder = malloc(sizeof(OggDecoder));
    oggdecoder->filehandle = filehandle;
    assert(oggdecoder);

    int result = oggutil_get_ogg_codec(filehandle);
    cb_init init = NULL;

    switch (result) {
    case OGGUTIL_CODEC_VORBIS:;
        init = (cb_init)oggvorbisdecoder_init;
        oggdecoder->destroy = (cb_destroy)oggvorbisdecoder_destroy;
        oggdecoder->get_info = (cb_get_info)oggvorbisdecoder_get_info;
        oggdecoder->read = (cb_read)oggvorbisdecoder_read;
        oggdecoder->seek = (cb_seek)oggvorbisdecoder_seek;
        break;
#ifdef SNDBRIDGE_OPUS_DECODING
    case OGGUTIL_CODEC_OPUS:;
        init = (cb_init)oggopusdecoder_init;
        oggdecoder->destroy = (cb_destroy)oggopusdecoder_destroy;
        oggdecoder->get_info = (cb_get_info)oggopusdecoder_get_info;
        oggdecoder->read = (cb_read)oggopusdecoder_read;
        oggdecoder->seek = (cb_seek)oggopusdecoder_seek;
        break;
#endif
    case OGGUTIL_CODEC_ERROR:
        fprintf(stderr, "oggdecoder_init() can not identify the audio codec\n");
        goto L_failed;
    case OGGUTIL_CODEC_UNKNOWN:
        fprintf(stderr, "oggdecoder_init() unknown audio codec\n");
        goto L_failed;
    }

    oggdecoder->decoder_handle = init(filehandle);
    if (!oggdecoder->decoder_handle) {
        fprintf(
            stderr,
            "oggdecoder_init() can not initialize the decoder for: %s\n",
            result == OGGUTIL_CODEC_OPUS ? "opus" : "vorbis"
        );
        goto L_failed;
    }

    return oggdecoder;

L_failed:
    free(oggdecoder);
    return NULL;
}

extern void oggdecoder_destroy(OggDecoder* oggdecoder) {
    oggdecoder->destroy(oggdecoder->decoder_handle);
    free(oggdecoder);
}


extern int32_t oggdecoder_read(OggDecoder* oggdecoder, uint8_t* buffer, int32_t buffer_size) {
    return oggdecoder->read(oggdecoder->decoder_handle, buffer, buffer_size);
}

extern void oggdecoder_get_info(OggDecoder* oggdecoder, int32_t* rate, int32_t* channels, double* duration) {
    return oggdecoder->get_info(oggdecoder->decoder_handle, rate, channels, duration);
}

extern bool oggdecoder_seek(OggDecoder* oggdecoder, double timestamp) {
    if (timestamp < 0.0) timestamp = 0.0;
    return oggdecoder->seek(oggdecoder->decoder_handle, timestamp);
}
