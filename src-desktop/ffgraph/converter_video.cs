using System;
using Engine.Externals.FFGraphInterop.Interop;

namespace Engine.Externals.FFGraphInterop;

internal unsafe class VideoConverter : IFFGraphFormatConverter {

    private const AVPixelFormat FALLBACK_PIX_FMT = AVPixelFormat.AV_PIX_FMT_RGB24;

    public static bool force_rgb24 = false;

    private SwsContext* sws_ctx;
    private int width;
    private int height;
    private double seconds;
    private double frame_duration;
    private AVFrame* frame;
    private byte* rgb24_buffer;
    private AVPixelFormat pixel_format;
    private FFGraphFrame result;


    public bool Process(AVCodecContext* codec_ctx, AVFrame* av_frame, AVStream* av_stream) {
        if (av_frame == null) {
            this.seconds = -2.0;
            return true;
        }

        if (this.sws_ctx != null) {
            FFmpeg.sws_scale(
                this.sws_ctx,
                (byte**)&av_frame->data, av_frame->linesize,
                0, codec_ctx->height,
                (byte**)&this.frame->data, this.frame->linesize
            );
        } else {
            this.result.width = av_frame->width;
            this.result.height = av_frame->height;
            this.result.y = av_frame->data[0];
            this.result.u = av_frame->data[1];
            this.result.v = av_frame->data[2];
            this.result.y_stride = av_frame->linesize[0];
            this.result.u_stride = av_frame->linesize[1];
            this.result.v_stride = av_frame->linesize[2];
        }

        if (av_frame->pts != FFmpeg.AV_NOPTS_VALUE) {
            this.seconds = FFGraph.CalculateSeconds(av_stream, av_frame);
        }
        this.frame_duration = av_frame->duration;

        return false;
    }

    public void Flush() { }

    public void Seek(double seconds) { }

    public void Dispose() {
        if (this.sws_ctx != null)
            FFmpeg.sws_freeContext(this.sws_ctx);
        if (this.rgb24_buffer != null)
            FFmpeg.av_free(this.rgb24_buffer);
        if (this.frame != null)
            fixed (AVFrame** ptr = &this.frame) FFmpeg.av_frame_free(ptr);
    }


    public static VideoConverter Init(AVCodecContext* codec_ctx) {
        if (codec_ctx == null) throw new ArgumentNullException(nameof(codec_ctx));

        AVPixelFormat pixel_format = codec_ctx->pix_fmt;
        byte* rgb24_buffer = null;
        AVFrame* frame = null;

        if (force_rgb24 || pixel_format != AVPixelFormat.AV_PIX_FMT_YUV420P) {
            pixel_format = VideoConverter.FALLBACK_PIX_FMT;

            int size = FFmpeg.av_image_get_buffer_size(pixel_format, codec_ctx->width, codec_ctx->height, 1);
            if (size < 0) {
                Logger.Error("VideoConverter::Init() call to av_image_get_buffer_size() failed.");
                return null;
            }
            rgb24_buffer = (byte*)FFmpeg.av_malloc((nuint)size);
            if (rgb24_buffer == null) {
                Logger.Error("VideoConverter::Init() failed to allocate the buffer.");
                return null;
            }

            frame = FFmpeg.av_frame_alloc();
            if (frame == null) {
                Logger.Error("VideoConverter::Init() call to av_frame_alloc() failed.");
                FFmpeg.av_free(rgb24_buffer);
                return null;
            }

            frame->width = codec_ctx->width;
            frame->height = codec_ctx->height;
            frame->format = (int)pixel_format;

            FFmpeg.av_image_fill_arrays(
               (byte**)&frame->data,
               frame->linesize, rgb24_buffer,
               pixel_format,
               codec_ctx->width, codec_ctx->height,
               1
            );
        }

        VideoConverter converter = new VideoConverter() {
            sws_ctx = null,
            width = codec_ctx->width,
            height = codec_ctx->height,
            seconds = -1.0,
            frame_duration = codec_ctx->framerate.den / (double)codec_ctx->framerate.num,
            frame = frame,
            rgb24_buffer = rgb24_buffer,
            pixel_format = pixel_format,
            result = new FFGraphFrame()
        };

        if (codec_ctx->framerate.num == 0 && codec_ctx->framerate.den == 1) {
            converter.frame_duration = 0.0;
        }

        converter.result.width = codec_ctx->width;
        converter.result.height = codec_ctx->height;

        if (pixel_format == VideoConverter.FALLBACK_PIX_FMT) {
            converter.sws_ctx = FFmpeg.sws_getContext(
                codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt,
                codec_ctx->width, codec_ctx->height, pixel_format,
                0x00,
                null, null, null
            );
            if (converter.sws_ctx == null) {
                Logger.Error("VideoConverter::Init() call to sws_getContext() failed.");
                converter.Dispose();
                return null;
            }

            converter.result.rgb24 = (nint)rgb24_buffer;
        }

        return converter;
    }

    public void GetStreamInfo(ref FFGraphInfo info) {
        info.video_width = this.width;
        info.video_height = this.height;
        info.video_is_yuv420p_or_rgb24 = this.pixel_format != VideoConverter.FALLBACK_PIX_FMT;
    }

    public double Read(out FFGraphFrame frame) {
        // check if there no frame at all
        if (this.seconds < 0.0)
            frame = default;
        else
            frame = this.result;

        return this.seconds;
    }

}
