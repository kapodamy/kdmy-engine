#ifndef __wavutil_h
#define __wavutil_h

#include "decoderhandle.h"
#include "sourcehandle.h"


typedef struct __attribute__((__packed__)) {
    uint32_t name;
    uint32_t chunk_size;
} RIFFChunk;

typedef struct __attribute__((__packed__)) {
    uint16_t format;
    uint16_t channels;
    uint32_t sample_rate;
    uint32_t bitrate;
    uint16_t block_align;
    uint16_t bits_per_sample;
} WavFormat;

typedef struct __attribute__((__packed__)) {
    RIFFChunk riff;
    uint32_t riff_type;

    RIFFChunk fmt;
    WavFormat format;
} WAV;


#define WAV_PCM 0x01
#define WAV_ITU_G723_ADPCM_ANTEX 0x14
#define WAV_YAMAHA_AICA_ADPCM 0x20

#define WAV__RIFF_NAME_MAKE(s) (uint32_t)(s[0] | (s[1] << 8) | (s[2] << 16) | (s[3] << 24))

#define WAV_HDR_RIFF WAV__RIFF_NAME_MAKE("RIFF")
#define WAV_HDR_FMT WAV__RIFF_NAME_MAKE("fmt\x20")
#define WAV_HDR_DATA WAV__RIFF_NAME_MAKE("data")
#define WAV_HDR_SMPL WAV__RIFF_NAME_MAKE("smpl")
#define WAV_HDR_CUE WAV__RIFF_NAME_MAKE("cue\x20")
#define WAV_RIFF_TYPE_WAVE WAV__RIFF_NAME_MAKE("WAVE")

DecoderHandle* adpcmreader_init(SourceHandle* hnd);
bool wav_is_file_RIFF_WAVE(SourceHandle* hnd);
bool wav_read_header(SourceHandle* hnd, WavFormat* wav_format, int64_t* data_offset, int64_t* data_length, int64_t* loop_start, int64_t* loop_length);
int32_t wav_calc_samples(WavFormat* fmt, int64_t data_length);

#endif
