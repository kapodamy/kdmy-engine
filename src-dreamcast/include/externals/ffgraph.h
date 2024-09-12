#ifndef __ffgraph_mvqa_h
#define __ffgraph_mvqa_h

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "float64.h"
#include "externals/ffgraph_frame.h"
#include "externals/sndbridge/decoderhandle.h"
#include "externals/sndbridge/sourcehandle.h"


typedef struct FFGraph_s* FFGraph;

typedef struct {
    bool audio_has_stream;
    bool video_has_stream;

    uint8_t audio_channels;
    uint16_t audio_sample_rate;

    uint16_t video_original_width;
    uint16_t video_original_height;

    uint16_t video_encoded_width;
    uint16_t video_encoded_height;

    float64 estimated_duration_seconds;
} FFGraphInfo;


FFGraph ffgraph_init(const SourceHandle* media_sourcehandle);
void ffgraph_destroy(FFGraph ffgraph);
void ffgraph_get_streams_info(FFGraph ffgraph, FFGraphInfo* output_info);
int32_t ffgraph_read_audio_samples(FFGraph ffgraph, void* out_samples, int32_t buffer_size);
float64 ffgraph_read_video_frame(FFGraph ffgraph, YUVFrame* out_frame);
float64 ffgraph_seek(FFGraph ffgraph, float64 time_in_seconds);

DecoderHandle* ffgraph_sndbridge_create_helper(FFGraph ffgraph, bool allow_seek, bool allow_destroy);
void ffgraph_sndbridge_destroy_helper(DecoderHandle* ffgraph_sndbridge);

const char* ffgraph_get_runtime_info();
void ffgraph_get_yuv_sizes(FFGraph ffgraph, size_t* luma_size, size_t* chroma_size);

#endif
