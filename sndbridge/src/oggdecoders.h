#ifndef _oggdecoders_h
#define _oggdecoders_h

#include <stdbool.h>
#include <stdint.h>

// exclude OGG vorbis static callbacks
#define OV_EXCLUDE_STATIC_CALLBACKS

#include "vorbis/codec.h"
#include "vorbis/vorbisfile.h"

#include "filehandle.h"

#define OGGDECODERS_LOOPSTART "LOOPSTART="
#define OGGDECODERS_LOOPLENGTH "LOOPLENGTH="

typedef struct {
    OggVorbis_File vf;
    int current_section;

    int32_t channels;
    double rate;
    double duration;

    int64_t loop_start;
    int64_t loop_length;

    FileHandle_t* file_hnd;
} OggVorbisDecoder_t;

typedef OggVorbisDecoder_t* OggVorbisDecoder;

OggVorbisDecoder oggvorbisdecoder_init(FileHandle_t* file_hnd);
void oggvorbisdecoder_destroy(OggVorbisDecoder oggvorbisdecoder);

int32_t oggvorbisdecoder_read(OggVorbisDecoder oggvorbisdecoder, float* buffer, int32_t samples_per_channel);
void oggvorbisdecoder_get_info(OggVorbisDecoder oggvorbisdecoder, int32_t* rate, int32_t* channels, double* duration);
bool oggvorbisdecoder_seek(OggVorbisDecoder oggvorbisdecoder, double timestamp);
void oggvorbisdecoder_get_loop_points(OggVorbisDecoder oggvorbisdecoder, int64_t* loop_start, int64_t* loop_length);


#ifdef SNDBRIDGE_OPUS_DECODING

#include <opus/opus_multistream.h>
#include <opus/opus_types.h>
#include <opus/opusfile.h>

typedef struct {
    OggOpusFile* op;

    double duration;
    int channels;

    int64_t loop_start;
    int64_t loop_length;

    FileHandle_t* file_hnd;
} OggOpusDecoder_t;

typedef OggOpusDecoder_t* OggOpusDecoder;

OggOpusDecoder oggopusdecoder_init(FileHandle_t* file_hnd);
void oggopusdecoder_destroy(OggOpusDecoder oggopusdecoder);

int32_t oggopusdecoder_read(OggOpusDecoder oggopusdecoder, float* buffer, int32_t samples_per_channel);
void oggopusdecoder_get_info(OggOpusDecoder oggopusdecoder, int32_t* rate, int32_t* channels, double* duration);
bool oggopusdecoder_seek(OggOpusDecoder oggopusdecoder, double timestamp);
void oggopusdecoder_get_loop_points(OggOpusDecoder oggvorbisdecoder, int64_t* loop_start, int64_t* loop_length);
#endif

#endif
