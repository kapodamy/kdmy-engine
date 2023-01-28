#ifdef SNDBRIDGE_OPUS_DECODING
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "oggdecoders.h"


static int read_func(void* stream, unsigned char* ptr, int nbytes) {
    return (int)filehandle_read((FileHandle_t*)stream, ptr, nbytes);
}
static int seek_func(void* stream, opus_int64 offset, int whence) {
    int ret;
    if (filehandle_seek((FileHandle_t*)stream, (_off64_t)offset, whence) < 0)
        ret = -1;
    else
        ret = 0;
    return ret;
}
static opus_int64 tell_func(void* stream) {
    return (opus_int64)filehandle_tell((FileHandle_t*)stream);
}


static const int32_t OPUS_RATE = 48000;
static const double OPUS_RATE_CONV = 1000.0 / 48000;

static struct OpusFileCallbacks callbacks = {
    .read = read_func,
    .seek = seek_func,
    .close = NULL,
    .tell = tell_func};


OggOpusDecoder oggopusdecoder_init(FileHandle_t* file_hnd) {
    OggOpusDecoder oggopusdecoder = malloc(sizeof(OggOpusDecoder_t));
    assert(oggopusdecoder);

    OggOpusFile* op = op_open_callbacks((void*)file_hnd, &callbacks, NULL, 0, NULL);
    if (!op) {
        free(oggopusdecoder);
        return NULL;
    }

    oggopusdecoder->op = op;
    oggopusdecoder->duration = op_pcm_total(oggopusdecoder->op, -1) * OPUS_RATE_CONV;
    oggopusdecoder->channels = op_channel_count(oggopusdecoder->op, -1);
    oggopusdecoder->file_hnd = file_hnd;
    oggopusdecoder->bytes_per_channel = op_channel_count(oggopusdecoder->op, -1) * 2;

    return oggopusdecoder;
}

void oggopusdecoder_destroy(OggOpusDecoder oggopusdecoder) {
    op_free(oggopusdecoder->op);
    free(oggopusdecoder);
}


int32_t oggopusdecoder_read(OggOpusDecoder oggopusdecoder, float* buffer, int32_t samples_per_channel) {
    int buf_size = samples_per_channel * oggopusdecoder->channels;
    return op_read_float(oggopusdecoder->op, buffer, buf_size, NULL);
}

void oggopusdecoder_get_info(OggOpusDecoder oggopusdecoder, int32_t* rate, int32_t* channels, double* duration) {
    *rate = OPUS_RATE;
    *channels = oggopusdecoder->channels;
    *duration = oggopusdecoder->duration;
}

bool oggopusdecoder_seek(OggOpusDecoder oggopusdecoder, double timestamp) {
    ogg_int64_t pcm_offset = (ogg_int64_t)(timestamp / OPUS_RATE_CONV);
    return op_pcm_seek(oggopusdecoder->op, pcm_offset);
}
#endif