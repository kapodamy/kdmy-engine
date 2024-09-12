#include <assert.h>

#include "externals/ffgraph.h"
#include "externals/sndbridge/decoderhandle.h"
#include "malloc_utils.h"


typedef struct {
    DecoderHandle dechnd;
    FFGraph ffgraph;
} FFGraphDecoderHandle;


static void stub_dec_destroy(DecoderHandle* decoder) {
    (void)decoder;
}

static bool stub_dec_seek(DecoderHandle* decoder, float64 timestamp) {
    (void)decoder;
    (void)timestamp;
    return false;
}

static void stub_dec_loop(DecoderHandle* decoder, int64_t* loop_start, int64_t* loop_length) {
    (void)decoder;
    *loop_start = -1;
    *loop_length = -1;
}

static int32_t dec_readBytes(DecoderHandle* decoder, void* buffer, uint32_t buffer_size) {
    FFGraph ffgraph = ((FFGraphDecoderHandle*)decoder)->ffgraph;
    int32_t ret = ffgraph_read_audio_samples(ffgraph, buffer, (int32_t)buffer_size);
    return ret;
}

static SampleFormat dec_info(DecoderHandle* decoder, uint32_t* rate, uint32_t* channels, float64* duration) {
    FFGraph ffgraph = ((FFGraphDecoderHandle*)decoder)->ffgraph;
    FFGraphInfo info;
    ffgraph_get_streams_info(ffgraph, &info);

    *rate = (uint32_t)info.audio_sample_rate;
    *channels = (uint32_t)info.audio_channels;
    *duration = info.estimated_duration_seconds;

    return SampleFormat_ADPCM_4_YAMAHA;
}

static bool dec_seek(DecoderHandle* decoder, float64 seconds) {
    FFGraph ffgraph = ((FFGraphDecoderHandle*)decoder)->ffgraph;
    ffgraph_seek(ffgraph, seconds);
    return true;
}

static void dec_destroy(DecoderHandle* decoder) {
    FFGraph ffgraph = ((FFGraphDecoderHandle*)decoder)->ffgraph;
    ffgraph_destroy(ffgraph);
}


DecoderHandle* ffgraph_sndbridge_create_helper(FFGraph ffgraph, bool allow_seek, bool allow_destroy) {
    FFGraphInfo info;
    ffgraph_get_streams_info(ffgraph, &info);

    if (!info.audio_has_stream) {
        return NULL;
    }

    FFGraphDecoderHandle* ffgraph_sndbridge = malloc_chk(sizeof(FFGraphDecoderHandle));
    malloc_assert(ffgraph_sndbridge, FFGraphDecoderHandle);

    ffgraph_sndbridge->dechnd.destroy = allow_destroy ? dec_destroy : stub_dec_destroy;
    ffgraph_sndbridge->dechnd.getInfo = dec_info;
    ffgraph_sndbridge->dechnd.readBytes = dec_readBytes;
    ffgraph_sndbridge->dechnd.seek = allow_seek ? dec_seek : stub_dec_seek;
    ffgraph_sndbridge->dechnd.getLoopPoints = stub_dec_loop;
    ffgraph_sndbridge->ffgraph = ffgraph;

    return (DecoderHandle*)ffgraph_sndbridge;
}

void ffgraph_sndbridge_destroy_helper(DecoderHandle* ffgraph_sndbridge) {
    if (!ffgraph_sndbridge) return;
    free_chk(ffgraph_sndbridge);
}
