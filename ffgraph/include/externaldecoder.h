#ifndef _externaldecoder_h
#define _externaldecoder_h

#include <stdint.h>
#include <stdbool.h>


typedef void (*ExternalDecoderDestroyCB)(void* decoder);
typedef int32_t (*ExternalDecoderReadCB)(void* decoder, float* buffer, int32_t sample_per_channel);
typedef void (*ExternalDecoderInfoCB)(void* decoder, int32_t* rate, int32_t* channels, double* duration);
typedef bool (*ExternalDecoderSeekCB)(void* decoder, double timestamp);

//
// Important: in ExternalDecoder all callbacks are required. If necessary omit using STUBS
//

typedef struct {
    void* decoder;
    ExternalDecoderDestroyCB destroy_func;
    ExternalDecoderReadCB read_func;
    ExternalDecoderInfoCB info_func;
    ExternalDecoderSeekCB seek_func;
} ExternalDecoder;

#endif