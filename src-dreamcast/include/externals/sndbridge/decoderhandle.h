#ifndef __decoderhandle_h
#define __decoderhandle_h

#include <stdbool.h>
#include <stdint.h>

#include "float64.h"


typedef struct DecoderHandle DecoderHandle;


#ifdef _arch_dreamcast

typedef enum {
    SampleFormat_PCM_S16LE,
    SampleFormat_PCM_U8,
    SampleFormat_ADPCM_4_YAMAHA
} SampleFormat;

struct DecoderHandle {
    int32_t (*readBytes)(DecoderHandle* decoder, void* buffer, uint32_t buffer_size);
    SampleFormat (*getInfo)(DecoderHandle* decoder, uint32_t* rate, uint32_t* channels, float64* duration_in_seconds);
    bool (*seek)(DecoderHandle* decoder, float64 seconds);
    void (*getLoopPoints)(DecoderHandle* decoder, int64_t* loop_start, int64_t* loop_length);
    void (*destroy)(DecoderHandle* decoder);
};

#else

typedef enum {
    SampleFormat_FLOAT32,
    SampleFormat_PCM_S16LE,
    SampleFormat_PCM_U8,
    SampleFormat_ADPCM_4_YAMAHA
} SampleFormat;

struct DecoderHandle {
    int32_t (*read)(DecoderHandle* decoder, void* buffer, uint32_t samples_per_channel);
    SampleFormat (*getInfo)(DecoderHandle* decoder, uint32_t* rate, uint32_t* channels, float64* duration_in_seconds);
    bool (*seek)(DecoderHandle* decoder, float64 seconds);
    void (*getLoopPoints)(DecoderHandle* decoder, int64_t* loop_start, int64_t* loop_length);
    void (*destroy)(DecoderHandle* decoder);
};

#endif

#endif
