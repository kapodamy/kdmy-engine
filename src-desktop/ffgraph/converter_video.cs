using System;
using Engine.Externals.FFGraphInterop.Interop;

namespace Engine.Externals.FFGraphInterop;

internal unsafe class VideoConverter : IFFGraphFormatConverter {

    private const AVPixelFormat PIX_FMT = AVPixelFormat.AV_PIX_FMT_RGB24;

    private SwsContext* sws_ctx;
    private AVFrame* frame;
    private byte* buffer;
    private int buffer_size;
    private uint width;
    private uint height;
    private double seconds;
    private double frame_duration;


    public bool Process(AVCodecContext* codec_ctx, AVFrame* av_frame, AVStream* av_stream) {
        if (av_frame == null) return false;

        FFmpeg.sws_scale(
            this.sws_ctx,
            (byte**)&av_frame->data, av_frame->linesize,
            0, codec_ctx->height,
            (byte**)&this.frame->data, this.frame->linesize
        );

        this.seconds = FFGraph.CalculateSeconds(av_stream, av_frame);
        this.frame_duration = av_frame->duration;

        return false;
    }

    public void Flush() { }

    public void Seek(double seconds) { }

    public void Dispose() {
        FFmpeg.sws_freeContext(this.sws_ctx);
        if (this.buffer != null) FFmpeg.av_free(this.buffer);
        if (this.frame != null) {
            fixed (AVFrame** ptr = &this.frame) {
                FFmpeg.av_frame_free(ptr);
            }
        }
    }


    public static VideoConverter Init(AVCodecContext* codec_ctx) {
        if (codec_ctx == null) throw new ArgumentNullException(nameof(codec_ctx));

        int size = FFmpeg.av_image_get_buffer_size(PIX_FMT, codec_ctx->width, codec_ctx->height, 1);
        if (size < 0) {
            Console.Error.WriteLine("[ERROR] videoconverter_init() call to av_image_get_buffer_size() failed.");
            return null;
        }
        byte* buffer = (byte*)FFmpeg.av_malloc((nuint)size);
        if (buffer == null) {
            Console.Error.WriteLine("[ERROR] videoconverter_init() failed to allocate the buffer.");
            return null;
        }

        VideoConverter converter = new VideoConverter() {
            frame = null,
            sws_ctx = null,
            buffer = buffer,
            buffer_size = size,
            width = checked((uint)codec_ctx->width),
            height = checked((uint)codec_ctx->height),
            seconds = -1.0,
            frame_duration = codec_ctx->framerate.den / (double)codec_ctx->framerate.num
        };

        if (codec_ctx->framerate.num == 0 && codec_ctx->framerate.den == 1) {
            converter.frame_duration = 0.0;
        }

        converter.sws_ctx = FFmpeg.sws_getContext(
            codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt,
            codec_ctx->width, codec_ctx->height, PIX_FMT,
            FFmpeg.SWS_BILINEAR,
            null, null, null
        );
        if (converter.sws_ctx == null) {
            Console.Error.WriteLine("[ERROR] videoconverter_init() call to sws_getContext() failed.");
            goto L_failed;
        }

        converter.frame = FFmpeg.av_frame_alloc();
        if (converter.frame == null) {
            Console.Error.WriteLine("[ERROR] videoconverter_init() call to av_frame_alloc() failed.");
            goto L_failed;
        }

        FFmpeg.av_image_fill_arrays(
           (byte**)&converter.frame->data, converter.frame->linesize, buffer, PIX_FMT, codec_ctx->width, codec_ctx->height, 1
        );

        return converter;

L_failed:
        converter.Dispose();
        return null;
    }

    public void GetStreamInfo(out uint out_width, out uint out_height) {
        out_width = this.width;
        out_height = this.height;
    }

    public double Read(double seconds, void** frame_out, int* frame_size_out) {
        // check if there no frame at all
        if (this.seconds < 0) return -1.0;

        // check if the buffered frame is future
        if (seconds >= 0 && this.seconds > 0 && this.seconds > seconds) {
            return -1.0;
        }

        *frame_out = this.buffer;
        *frame_size_out = this.buffer_size;

        return this.seconds;
    }

}
