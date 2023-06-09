#include "externaldecoder.h"
#include "ffgraph.h"


static void stub_destroy(void* decoder) {
    (void)decoder;
}

static bool stub_seek(void* decoder, double timestamp) {
    (void)decoder;
    (void)timestamp;
    return false;
}

static void stub_loop(void* decoder, int64_t* loop_start, int64_t* loop_length) {
    (void)decoder;
    *loop_start = -1;
    *loop_length = -1;
}


static int32_t read(FFGraph* ffgraph, float* buffer, int32_t sample_per_channel) {
    int32_t ret = ffgraph_read_audio_samples(ffgraph, buffer, sample_per_channel);
    return ret;
}

static void info(FFGraph* ffgraph, int32_t* rate, int32_t* channels, double* duration) {
    FFGraphInfo info;
    ffgraph_get_streams_info(ffgraph, &info);

    *rate = info.audio_sample_rate;
    *channels = info.audio_channels;
    *duration = info.audio_seconds_duration * 1000.0;
}

static bool seek(FFGraph* ffgraph, double timestamp) {
    ffgraphfmt_seek(ffgraph->audio, timestamp / 1000.0);
    return true;
}


ExternalDecoder* ffgraph_sndbridge_create_helper(FFGraph* ffgraph, bool allow_seek, bool allow_destroy) {
    if (!ffgraph->audio) return NULL;

    ExternalDecoder* ffgraph_sndbridge = malloc(sizeof(ExternalDecoder));

    ffgraph_sndbridge->decoder = ffgraph;
    ffgraph_sndbridge->destroy_func = allow_destroy ? (ExternalDecoderDestroyCB)ffgraph_destroy : stub_destroy;
    ffgraph_sndbridge->info_func = (ExternalDecoderInfoCB)info;
    ffgraph_sndbridge->read_func = (ExternalDecoderReadCB)read;
    ffgraph_sndbridge->seek_func = allow_seek ? (ExternalDecoderSeekCB)seek : stub_seek;
    ffgraph_sndbridge->loop_func = stub_loop;

    return ffgraph_sndbridge;
}

void ffgraph_sndbridge_destroy_helper(ExternalDecoder* ffgraph_sndbridge) {
    if (!ffgraph_sndbridge) return;
    free(ffgraph_sndbridge);
}
