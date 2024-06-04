using System;
using Engine.Externals.FFGraphInterop.Interop;
using Engine.Externals.SoundBridge;

namespace Engine.Externals.FFGraphInterop;


internal unsafe interface IFFGraphFormatConverter : IDisposable {
    bool Process(AVCodecContext* codec_ctx, AVFrame* av_frame, AVStream* av_stream);
    void Flush();
    void Seek(double seconds);
}

internal unsafe class FFGraphFormat : IDisposable {

    private AVPacket* packet;
    private AVFormatContext* fmt_ctx;
    private IOHandler iohandle;
    public AVCodecContext* codec_ctx;
    private AVCodec* av_codec;
    private AVFrame* av_frame;
    private AVStream* av_stream;
    public bool has_ended;
    public bool has_decoded;
    public IFFGraphFormatConverter ffgraphconv;
    private int stream_idx;


    public static FFGraphFormat Init(ISourceHandle sourcehandle, AVMediaType required_type) {
        switch (required_type) {
            case AVMediaType.AVMEDIA_TYPE_AUDIO:
            case AVMediaType.AVMEDIA_TYPE_VIDEO:
                break;
            default:
                Logger.Error($"FFGraphFormat::Init() unknown required_type: {required_type}");
                return null;
        }

        AVFormatContext* fmt_ctx = FFmpeg.avformat_alloc_context();
        FFGraphFormat ffgraphfmt = new FFGraphFormat() {
            packet = FFmpeg.av_packet_alloc(),
            fmt_ctx = fmt_ctx,
            iohandle = IOHandler.Init(sourcehandle),
            codec_ctx = FFmpeg.avcodec_alloc_context3(null),
            av_frame = FFmpeg.av_frame_alloc(),

            ffgraphconv = null,
            av_codec = null,
            av_stream = null,

            stream_idx = -1,
            has_ended = false,
            has_decoded = false
        };

        if (ffgraphfmt.packet == null) {
            Logger.Error("FFGraphFormat::Init() AVPacket allocation failed.");
            ffgraphfmt.Dispose();
            return null;
        }
        if (ffgraphfmt.fmt_ctx == null) {
            Logger.Error("FFGraphFormat::Init() AVFormatContext allocation failed.");
            ffgraphfmt.Dispose();
            return null;
        }
        if (ffgraphfmt.iohandle == null) {
            Logger.Error("FFGraphFormat::Init() AVIOContext allocation failed.");
            ffgraphfmt.Dispose();
            return null;
        }
        if (ffgraphfmt.codec_ctx == null) {
            Logger.Error("FFGraphFormat::Init() AVCodecContext allocation failed.");
            ffgraphfmt.Dispose();
            return null;
        }
        if (ffgraphfmt.av_frame == null) {
            Logger.Error("FFGraphFormat::Init() AVFrame allocation failed.");
            ffgraphfmt.Dispose();
            return null;
        }

        // set custom AVIO handler
        fmt_ctx->flags = FFmpeg.AVFMT_FLAG_CUSTOM_IO;
        fmt_ctx->pb = ffgraphfmt.iohandle.IOContext;

        int ret = FFmpeg.avformat_open_input(&fmt_ctx, null, null, null);
        if (ret < 0) {
            string e = FFmpeg.av_err2str(ret);
            Logger.Error($"FFGraphFormat::Init() open input failed, reason: {e}");
            ffgraphfmt.Dispose();
            return null;
        }

        ret = FFmpeg.avformat_find_stream_info(ffgraphfmt.fmt_ctx, null);
        if (ret < 0) {
            string e = FFmpeg.av_err2str(ret);
            Logger.Error($"FFGraphFormat::Init() failed to find the stream info, reason: {e}");
            ffgraphfmt.Dispose();
            return null;
        }

        AVCodec* av_codec;
        ffgraphfmt.stream_idx = FFmpeg.av_find_best_stream(ffgraphfmt.fmt_ctx, required_type, -1, -1, &av_codec, 0x00);
        if (ffgraphfmt.stream_idx < 0) {
            //string e = FFmpeg.av_err2str(ret);
            //Logger.Error($"FFGraphFormat::Init() failed to find the best/default stream, reason: {e}");
            ffgraphfmt.Dispose();
            return null;
        } else {
            ffgraphfmt.av_codec = av_codec;
        }

        AVStream* av_stream = ffgraphfmt.fmt_ctx->streams[ffgraphfmt.stream_idx];
        ffgraphfmt.av_stream = av_stream;

        FFmpeg.avcodec_parameters_to_context(ffgraphfmt.codec_ctx, av_stream->codecpar);
        ffgraphfmt.codec_ctx->pkt_timebase = av_stream->time_base;

        ret = FFmpeg.avcodec_open2(ffgraphfmt.codec_ctx, ffgraphfmt.av_codec, null);
        if (ret < 0) {
            string e = FFmpeg.av_err2str(ret);
            Logger.Error($"FFGraphFormat::Init() failed to open the codec, reason: {e}");
            ffgraphfmt.Dispose();
            return null;
        }

        if (required_type == AVMediaType.AVMEDIA_TYPE_AUDIO) {
            ffgraphfmt.ffgraphconv = AudioConverter.Init(ffgraphfmt.codec_ctx);
        } else if (required_type == AVMediaType.AVMEDIA_TYPE_VIDEO) {
            ffgraphfmt.ffgraphconv = VideoConverter.Init(ffgraphfmt.codec_ctx);
        }

        if (ffgraphfmt.ffgraphconv == null) {
            ffgraphfmt.Dispose();
            return null;
        }

        return ffgraphfmt;
    }

    public void Dispose() {
        AVPacket* packet = this.packet;
        AVCodecContext* codec_ctx = this.codec_ctx;
        AVFrame* av_frame = this.av_frame;

        if (this.packet != null) FFmpeg.av_packet_free(&packet);
        if (this.fmt_ctx != null) FFmpeg.avformat_free_context(this.fmt_ctx);
        if (this.iohandle != null) this.iohandle.Dispose();
        if (this.codec_ctx != null) FFmpeg.avcodec_free_context(&codec_ctx);
        if (this.av_frame != null) FFmpeg.av_frame_free(&av_frame);
        if (this.ffgraphconv != null) this.ffgraphconv.Dispose();
    }


    public double GetStreamDuration() {
        AVStream* av_stream = this.av_stream;

        if (av_stream->duration == FFmpeg.AV_NOPTS_VALUE) {
            // unknown duration use AVFormatContext instead
            return this.fmt_ctx->duration / (double)FFmpeg.AV_TIME_BASE;
        }

        double seconds = av_stream->duration * (av_stream->time_base.num / (double)av_stream->time_base.den);
        return seconds;
    }


    public bool ReadAndSeek(long seek_to_pts) {
        bool running = true;
        bool data_readed = false;
        bool seek_request = seek_to_pts >= 0;
        int ret;
        AVPacket* av_packet = this.packet;

L_process:
        while (running) {
            ret = FFmpeg.av_read_frame(this.fmt_ctx, this.packet);

            if (ret == FFmpeg.AVERROR_EOF) {
                av_packet = null;
                this.has_ended = true;
            } else if (ret < 0) {
                FFmpeg.av_packet_unref(av_packet);
                return false;
            }

            if (av_packet != null && av_packet->stream_index != this.stream_idx) {
                goto L_drop_packet;
            }

            ret = FFmpeg.avcodec_send_packet(this.codec_ctx, av_packet);
            if (ret < 0) {
                if (ret == FFmpeg.AVERROR_EOF) {
                    running = false;
                } else if (ret != FFmpeg.AVERROR(FFmpeg.EAGAIN)) {
                    string e = FFmpeg.av_err2str(ret);
                    Logger.Error($"FFGraphFormat::ReadAndSeek() call to avcodec_send_packet() failed: {e}");
                }
                goto L_drop_packet;
            }

L_receive_frame:
            if (ret >= 0 && av_packet != null) av_packet->size = 0;
            ret = FFmpeg.avcodec_receive_frame(this.codec_ctx, this.av_frame);

            if (seek_to_pts >= 0 && av_packet != null) {
                long next_pts = av_packet->pts + av_packet->duration;
                if (next_pts < seek_to_pts) {
                    FFmpeg.av_packet_unref(av_packet);
                    goto L_process;
                }

                seek_to_pts = -1;
            }

            if (ret >= 0) {
                data_readed = true;
                bool failed = this.ffgraphconv.Process(this.codec_ctx, this.av_frame, this.av_stream);

                if (failed) {
                    FFmpeg.av_packet_unref(this.packet);
                    return false;
                }

                if (seek_request) {
                    this.has_decoded = true;
                }

                // call again avcodec_receive_frame() which is required for audio samples
                goto L_receive_frame;
            } else if (ret == FFmpeg.AVERROR(FFmpeg.EAGAIN)) {
                // try adquire another packet (if possible)
                if (data_readed) running = false;
            } else if (ret == FFmpeg.AVERROR_EOF) {
                running = false;
            } else {
#if DEBUG
                string e = FFmpeg.av_err2str(ret);
                Logger.Error($"FFGraphFormat::ReadAndSeek() call to avcodec_receive_frame() failed, reason: {e}");
#endif
            }

L_drop_packet:
            FFmpeg.av_packet_unref(this.packet);
        }

        if (this.has_ended) {
            // flush converter
            this.ffgraphconv.Process(this.codec_ctx, null, this.av_stream);
        }

        return true;
    }


    public bool Read() {
        if (this.has_decoded) {
            this.has_decoded = false;
            return true;
        }
        return ReadAndSeek(-1L);
    }

    public void Seek(double seconds) {
        AVStream* av_stream = this.av_stream;
        double timebase = av_stream->time_base.den / (double)av_stream->time_base.num;

        long timestamp = (long)(seconds * timebase);

        // seek to the nearest previous timestamp
        int err = FFmpeg.avformat_seek_file(this.fmt_ctx, this.stream_idx, 0, timestamp, timestamp, 0x00);
        if (err < 0) {
            string e = FFmpeg.av_err2str(err);
            Logger.Error($"FFGraphFormat::Seek() seeking failed, reason: {e}.");
            return;
        }

        // flush decoder
        FFmpeg.avcodec_flush_buffers(this.codec_ctx);

        // flush converter
        this.ffgraphconv.Flush();

        // read until the nearest upper timestamp is found
        this.has_ended = false;
        this.has_decoded = false;
        ReadAndSeek(timestamp);

        // seek converter buffer (if applicable)
        this.ffgraphconv.Seek(seconds);
    }

}
