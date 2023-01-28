#include "ffgraph.h"


FFGraphFormat* ffgraphfmt_init(const FileHandle_t* filehandle, const enum AVMediaType required_type) {
    FFGraphFormat* ffgraphfmt = av_malloc(sizeof(FFGraphFormat));

    ffgraphfmt->packet = av_packet_alloc();
    ffgraphfmt->fmt_ctx = avformat_alloc_context();
    ffgraphfmt->iohandle = iohandler_init(filehandle);
    ffgraphfmt->codec_ctx = avcodec_alloc_context3(NULL);
    ffgraphfmt->av_frame = av_frame_alloc();
    ffgraphfmt->has_ended = false;

    if (!ffgraphfmt->packet) {
        printf("ffgraphfmt_init() AVPacket allocation failed.\n");
        ffgraphfmt_destroy(ffgraphfmt);
        return NULL;
    }
    if (!ffgraphfmt->fmt_ctx) {
        printf("ffgraphfmt_init() AVFormatContext allocation failed.\n");
        ffgraphfmt_destroy(ffgraphfmt);
        return NULL;
    }
    if (!ffgraphfmt->iohandle) {
        printf("ffgraphfmt_init() AVIOContext allocation failed.\n");
        ffgraphfmt_destroy(ffgraphfmt);
        return NULL;
    }
    if (!ffgraphfmt->codec_ctx) {
        printf("ffgraphfmt_init() AVCodecContext allocation failed.\n");
        ffgraphfmt_destroy(ffgraphfmt);
        return NULL;
    }
    if (!ffgraphfmt->av_frame) {
        printf("ffgraphfmt_init() AVFrame allocation failed.\n");
        ffgraphfmt_destroy(ffgraphfmt);
        return NULL;
    }

    // set custom AVIO handler
    ffgraphfmt->fmt_ctx->flags = AVFMT_FLAG_CUSTOM_IO;
    ffgraphfmt->fmt_ctx->pb = ffgraphfmt->iohandle;

    int ret = avformat_open_input(&ffgraphfmt->fmt_ctx, NULL, NULL, NULL);
    if (ret < 0) {
        printf("ffgraphfmt_init() open input failed, reason: %s\n", av_err2str(ret));
        ffgraphfmt_destroy(ffgraphfmt);
        return NULL;
    }

    ret = avformat_find_stream_info(ffgraphfmt->fmt_ctx, NULL);
    if (ret < 0) {
        printf("ffgraphfmt_init() failed to find then stream info, reason: %s\n", av_err2str(ret));
        ffgraphfmt_destroy(ffgraphfmt);
        return NULL;
    }

    const AVCodec** av_codec = &ffgraphfmt->av_codec;
    ffgraphfmt->stream_idx = av_find_best_stream(ffgraphfmt->fmt_ctx, required_type, -1, -1, av_codec, 0x00);
    if (ffgraphfmt->stream_idx < 0) {
        printf("ffgraphfmt_init() open input failed, reason: %s\n", av_err2str(ffgraphfmt->stream_idx));
        ffgraphfmt_destroy(ffgraphfmt);
        return NULL;
    }

    const AVStream* av_stream = ffgraphfmt->fmt_ctx->streams[ffgraphfmt->stream_idx];
    avcodec_parameters_to_context(ffgraphfmt->codec_ctx, av_stream->codecpar);
    ffgraphfmt->ffgraphconv.av_stream = av_stream;

    ret = avcodec_open2(ffgraphfmt->codec_ctx, ffgraphfmt->av_codec, NULL);
    if (ret < 0) {
        printf("ffgraphfmt_init() open input failed, reason: %s\n", av_err2str(ret));
        ffgraphfmt_destroy(ffgraphfmt);
        return NULL;
    }

    return ffgraphfmt;
}

void ffgraphfmt_destroy(FFGraphFormat* ffgraphfmt) {
    if (ffgraphfmt->packet) av_packet_free(&ffgraphfmt->packet);
    if (ffgraphfmt->fmt_ctx) avformat_free_context(ffgraphfmt->fmt_ctx);
    if (ffgraphfmt->iohandle) iohandler_destroy(&ffgraphfmt->iohandle);
    if (ffgraphfmt->codec_ctx) avcodec_free_context(&ffgraphfmt->codec_ctx);
    if (ffgraphfmt->av_frame) av_frame_free(&ffgraphfmt->av_frame);
    if (ffgraphfmt->ffgraphconv.destroy_cb) ffgraphfmt->ffgraphconv.destroy_cb(&ffgraphfmt->ffgraphconv);
    av_free(ffgraphfmt);
}

static bool ffgraphfmt_read_and_seek(FFGraphFormat* ffgraphfmt, int64_t seek_to_pts) {
    bool running = true;
    bool data_readed = false;
    int ret;
    AVPacket* av_packet = ffgraphfmt->packet;

L_process:
    while (running) {
        ret = av_read_frame(ffgraphfmt->fmt_ctx, ffgraphfmt->packet);

        if (ret == AVERROR_EOF) {
            av_packet = NULL;
            ffgraphfmt->has_ended = true;
        } else if (ret < 0) {
            av_packet_unref(av_packet);
            return false;
        }

        if (av_packet && av_packet->stream_index != ffgraphfmt->stream_idx) {
            goto L_drop_packet;
        }

        ret = avcodec_send_packet(ffgraphfmt->codec_ctx, av_packet);
        if (ret < 0) {
            if (ret == AVERROR_EOF) {
                running = false;
            } else if (ret != AVERROR(EAGAIN)) {
                printf("ffgraphfmt_read_and_seek() error in avcodec_send_packet(): %s", av_err2str(ret));
            }
            goto L_drop_packet;
        }

    L_receive_frame:
        if (ret >= 0 && av_packet) av_packet->size = 0;
        ret = avcodec_receive_frame(ffgraphfmt->codec_ctx, ffgraphfmt->av_frame);

        if (seek_to_pts >= 0 && av_packet) {
            int64_t next_pts = av_packet->pts + av_packet->duration;
            if (next_pts < seek_to_pts) {
                av_packet_unref(av_packet);
                goto L_process;
            }

            seek_to_pts = -1;
        }

        if (ret >= 0) {
            data_readed = true;
            bool failed = ffgraphfmt->ffgraphconv.convert_cb(
                &ffgraphfmt->ffgraphconv, ffgraphfmt->codec_ctx, ffgraphfmt->av_frame
            );

            if (failed) {
                av_packet_unref(ffgraphfmt->packet);
                return false;
            }

            // call again avcodec_receive_frame() which is required for audio samples
            goto L_receive_frame;
        } else if (ret == AVERROR(EAGAIN)) {
            // try adquire another packet (if possible)
            if (data_readed) running = false;
        } else if (ret == AVERROR_EOF) {
            running = false;
        } else {
#ifdef DEBUG
            printf("ffgraphfmt_read_and_seek() call to avcodec_receive_frame() failed, reason: %s", av_err2str(ret));
#endif
        }

    L_drop_packet:
        av_packet_unref(ffgraphfmt->packet);
    }

    if (ffgraphfmt->has_ended) {
        // flush converter
        ffgraphfmt->ffgraphconv.convert_cb(&ffgraphfmt->ffgraphconv, ffgraphfmt->codec_ctx, NULL);
    }

    return true;
}


double ffgraphfmt_get_stream_duration(FFGraphFormat* ffgraphfmt) {
    const AVStream* av_stream = ffgraphfmt->ffgraphconv.av_stream;

    if (av_stream->duration == AV_NOPTS_VALUE) {
        // unknown duration use AVFormatContext instead
        return ffgraphfmt->fmt_ctx->duration / (double)AV_TIME_BASE;
    }

    double seconds = (double)av_stream->duration * (av_stream->time_base.num / (double)av_stream->time_base.den);
    return seconds;
}

bool ffgraphfmt_read(FFGraphFormat* ffgraphfmt) {
    return ffgraphfmt_read_and_seek(ffgraphfmt, -1);
}

void ffgraphfmt_seek(FFGraphFormat* ffgraphfmt, double seconds) {
    const AVStream* av_stream = ffgraphfmt->ffgraphconv.av_stream;
    double timebase = av_stream->time_base.den / (double)av_stream->time_base.num;

    int64_t timestamp = (int64_t)(seconds * timebase);

    // seek to the nearest previous timestamp
    int err = avformat_seek_file(ffgraphfmt->fmt_ctx, ffgraphfmt->stream_idx, 0, timestamp, timestamp, 0x00);
    if (err < 0) {
        printf("ffgraphfmt_seek() seeking failed, reason: %s.\n", av_err2str(err));
        return;
    }

    // flush decoder
    avcodec_flush_buffers(ffgraphfmt->codec_ctx);

    // flush converter
    if (ffgraphfmt->ffgraphconv.flush_cb) ffgraphfmt->ffgraphconv.flush_cb(&ffgraphfmt->ffgraphconv);

    // read until the nearest upper timestamp is found
    ffgraphfmt->has_ended = false;
    ffgraphfmt_read_and_seek(ffgraphfmt, timestamp);

    // seek converter buffer (if applicable)
    if (ffgraphfmt->ffgraphconv.bufferseek_cb) {
        ffgraphfmt->ffgraphconv.bufferseek_cb(&ffgraphfmt->ffgraphconv, seconds);
    }
}
