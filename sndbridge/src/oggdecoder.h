#ifndef _oggdecoder_h
#define _oggdecoder_h

#include <stdint.h>
#include <stdbool.h>
#include "filehandle.h"

typedef void* (*cb_init)(FileHandle_t* file_hnd);
typedef void (*cb_destroy)(void* decoder);
typedef int32_t(*cb_read)(void* decoder, uint8_t* buffer, int32_t buffer_size);
typedef void (*cb_get_info)(void* decoder, int32_t* rate, int32_t* channels, double* duration);
typedef bool (*cb_seek)(void* decoder, double timestamp);

typedef struct {
    void* decoder_handle;

    FileHandle_t* filehandle;

    cb_destroy destroy;
    cb_read read;
    cb_get_info get_info;
    cb_seek seek;
} OggDecoder;

extern OggDecoder* oggdecoder_init(FileHandle_t* filehandle);
extern void oggdecoder_destroy(OggDecoder* oggdecoder);

extern int32_t oggdecoder_read(OggDecoder* oggdecoder, uint8_t* buffer, int32_t buffer_size);
extern void oggdecoder_get_info(OggDecoder* oggdecoder, int32_t* rate, int32_t* channels, double* duration);
extern bool oggdecoder_seek(OggDecoder* oggdecoder, double timestamp);

#endif
