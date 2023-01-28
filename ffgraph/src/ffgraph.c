#include "ffgraph.h"

static char INFO_BUFFER[256];

FFGraph* ffgraph_init(FileHandle_t* video_filehandle, FileHandle_t* audio_filehandle) {
    if (video_filehandle == audio_filehandle) {
        printf("ffgraph_init() error: due to design constraints both filehandles can not be equal.\n");
        return NULL;
    }

    FFGraphFormat* video = ffgraphfmt_init(video_filehandle, AVMEDIA_TYPE_VIDEO);
    FFGraphFormat* audio = ffgraphfmt_init(audio_filehandle, AVMEDIA_TYPE_AUDIO);

    if (!video && !audio) {
        printf("ffgraph_init() failed, no audio/video stream available.\n");
        return NULL;
    }

    if (video) videoconverter_init(video->codec_ctx, &video->ffgraphconv);
    if (audio) audioconverter_init(audio->codec_ctx, &audio->ffgraphconv);

    FFGraph* ffgraph = malloc(sizeof(FFGraph));
    ffgraph->video = video;
    ffgraph->audio = audio;

    return ffgraph;
}

void ffgraph_destroy(FFGraph* ffgraph) {
    if (ffgraph->audio) ffgraphfmt_destroy(ffgraph->audio);
    if (ffgraph->video) ffgraphfmt_destroy(ffgraph->video);

    free(ffgraph);
}

void ffgraph_get_streams_info(FFGraph* ffgraph, FFGraphInfo* output_info) {
    if (!output_info) return;

    output_info->audio_has_stream = ffgraph->audio != NULL;
    if (output_info->audio_has_stream) {
        output_info->audio_seconds_duration = ffgraphfmt_get_stream_duration(ffgraph->audio);
        audioconverter_get_stream_info(
            &ffgraph->audio->ffgraphconv, &output_info->audio_channels, &output_info->audio_sample_rate
        );
    }

    output_info->video_has_stream = ffgraph->video != NULL;
    if (output_info->video_has_stream) {
        output_info->video_seconds_duration = ffgraphfmt_get_stream_duration(ffgraph->video);
        videoconverter_get_stream_info(
            &ffgraph->video->ffgraphconv, &output_info->video_width, &output_info->video_height
        );
    }
}

int32_t ffgraph_read_audio_samples(FFGraph* ffgraph, void* out_samples, int32_t max_samples_per_channel) {
    if (!ffgraph->audio || ffgraph->audio->has_ended) {
        return -1;
    }

    if (!ffgraphfmt_read(ffgraph->audio)) {
        return -1;
    }

    return audioconverter_read(&ffgraph->audio->ffgraphconv, out_samples, max_samples_per_channel);
}

double ffgraph_read_video_frame(FFGraph* ffgraph, void** out_frame, int32_t* out_frame_size) {
    if (!ffgraph->video || ffgraph->video->has_ended) {
        return -2.0;
    }

    if (!ffgraphfmt_read(ffgraph->video)) {
        return -1.0;
    }

    // do not care, required for non-monotonous timetamp
    const double seconds = -1.0;

    return videoconverter_read(&ffgraph->video->ffgraphconv, seconds, out_frame, out_frame_size);
}

double ffgraph_read_video_frame2(FFGraph* ffgraph, uint8_t* buffer, int32_t buffer_size) {
    void* frame;
    int32_t frame_size;

    double ret = ffgraph_read_video_frame(ffgraph, &frame, &frame_size);
    if (ret < 0) return ret;

    int32_t to_copy = frame_size < buffer_size ? frame_size : buffer_size;
    memcpy(buffer, frame, to_copy);

    return ret;
}

void ffgraph_seek(FFGraph* ffgraph, double time_in_seconds) {
    if (ffgraph->video)
        ffgraphfmt_seek(ffgraph->video, time_in_seconds);

    if (ffgraph->audio)
        ffgraphfmt_seek(ffgraph->audio, time_in_seconds);
}

void ffgraph_seek2(FFGraph* ffgraph, double time_in_seconds, bool audio_or_video) {
    FFGraphFormat* ffgraphfmt = audio_or_video ? ffgraph->audio : ffgraph->video;
    if (!ffgraphfmt) return;
    ffgraphfmt_seek(ffgraphfmt, time_in_seconds);
}

const char* ffgraph_get_runtime_info() {
    int version_avformat = avformat_version();
    int version_avcodec = avcodec_version();
    int version_swresample = swresample_version();
    int version_swscale = swscale_version();
    int version_avutil = avutil_version();

    int ret = snprintf(
        INFO_BUFFER, sizeof(INFO_BUFFER),
        "avf=%d.%d.%d avc=%d.%d.%d swr=%d.%d.%d sws=%d.%d.%d avu=%d.%d.%d",
        (version_avformat >> 16) & 0xFFFF,
        (version_avformat >> 8) & 0x00FF,
        version_avformat & 0x00FF,
        (version_avcodec >> 16) & 0xFFFF,
        (version_avcodec >> 8) & 0x00FF,
        version_avcodec & 0x00FF,
        (version_swresample >> 16) & 0xFFFF,
        (version_swresample >> 8) & 0x00FF,
        version_swresample & 0x00FF,
        (version_swscale >> 16) & 0xFFFF,
        (version_swscale >> 8) & 0x00FF,
        version_swscale & 0x00FF,
        (version_avutil >> 16) & 0xFFFF,
        (version_avutil >> 8) & 0x00FF,
        version_avutil & 0x00FF
    );

    return ret < 0 ? NULL : INFO_BUFFER;
}