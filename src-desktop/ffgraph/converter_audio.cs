using System;
using System.Runtime.InteropServices;
using Engine.Externals.FFGraphInterop.Interop;

namespace Engine.Externals.FFGraphInterop;

internal unsafe class AudioConverter : IFFGraphFormatConverter {

    private const int BUFFER_LENGTH = (1024 * 1024) / 4;
    private const int BUFFER_SIZE_MAX = BUFFER_LENGTH / 2;
    private const uint BUFFER_SIZE = BUFFER_LENGTH / 4;

    private const AVSampleFormat SAMPLE_FMT = AVSampleFormat.AV_SAMPLE_FMT_FLT;
    private const uint SAMPLE_RATE = 44100;
    private const uint SAMPLE_SIZE = sizeof(float);


    private SwrContext* swr_ctx;
    private float* sample_buffer;
    private uint sample_buffer_used;
    private double seconds;
    private uint channels;
    private uint sample_rate;


    private AudioConverter() { }


    public bool Process(AVCodecContext* codec_ctx, AVFrame* av_frame, AVStream* av_stream) {
        int available = (int)(BUFFER_SIZE_MAX - this.sample_buffer_used);
        byte* buffer = (byte*)(this.sample_buffer + (this.sample_buffer_used * this.channels));
        byte** samples;
        int samples_count;

        if (av_frame != null) {
            samples = av_frame->extended_data;
            samples_count = av_frame->nb_samples;
        } else {
            samples = null;
            samples_count = 0;
        }

        int count = FFmpeg.swr_convert(this.swr_ctx, &buffer, available, samples, samples_count);

        if (count == 0) {
            return false;
        } else if (count < 0) {
            string e = FFmpeg.av_err2str(count);
            Logger.Error($"AudioConverter::Process() call to swr_convert() failed, reason: {e}");
            return true;
        }

        if (this.sample_buffer_used < 1 && av_frame != null) {
            this.seconds = FFGraph.CalculateSeconds(av_stream, av_frame);
        }

        this.sample_buffer_used += (uint)count;

        return false;
    }

    public void Flush() {
        this.sample_buffer_used = 0;

        byte* buffer = (byte*)this.sample_buffer;
        FFmpeg.swr_convert(this.swr_ctx, &buffer, BUFFER_SIZE_MAX, null, 0);
    }

    public void Seek(double seconds) {
        double end_seconds = this.seconds + (this.sample_buffer_used / (double)this.sample_rate);

        if (this.sample_buffer_used < 1 || seconds < this.seconds || seconds > end_seconds) {
            // drop buffer
            this.sample_buffer_used = 0;
            return;
        }

        if (this.seconds == seconds) {
            // nothing to do
            return;
        }

        // calculate the amount of samples to drop
        uint forward_samples = (uint)Math.Ceiling((seconds - this.seconds) * this.sample_rate);
        uint byte_offset = forward_samples * this.channels * SAMPLE_SIZE;

        // drop samples
        uint remaining_samples = this.sample_buffer_used - forward_samples;
        nuint remaining_bytes = remaining_samples * this.channels * SAMPLE_SIZE;
        byte* remaining_buffer = ((byte*)this.sample_buffer) + byte_offset;

        NativeMemory.Copy(remaining_buffer, this.sample_buffer, remaining_bytes);

        this.sample_buffer_used -= forward_samples;
        this.seconds += (double)forward_samples / this.sample_rate; // calc again due ceiling
    }

    public void Dispose() {
        fixed (SwrContext** ptr = &this.swr_ctx) {
            FFmpeg.swr_free(ptr);
        }
        FFmpeg.av_free(this.sample_buffer);
    }


    public static AudioConverter Init(AVCodecContext* codec_ctx) {
        if (codec_ctx == null) throw new ArgumentNullException(nameof(codec_ctx));

        AVChannelLayout out_ch_layout = new AVChannelLayout() { opaque = 0x00 };
        int channel_count = Math.Max(codec_ctx->ch_layout.nb_channels, 2);
        uint sample_rate = checked((uint)codec_ctx->sample_rate);

        if (sample_rate != 44100 || sample_rate != 48000) sample_rate = SAMPLE_RATE;

        FFmpeg.av_channel_layout_default(&out_ch_layout, channel_count);

        // NOTE: the input channel layout can not be changed during decoding.
        SwrContext* swr_ctx = FFmpeg.swr_alloc();
        int ret = FFmpeg.swr_alloc_set_opts2(
            &swr_ctx,
            &out_ch_layout, SAMPLE_FMT, (int)sample_rate,
            &codec_ctx->ch_layout, codec_ctx->sample_fmt, codec_ctx->sample_rate,
            0, null
        );

        if (ret < 0) {
            Logger.Error("AudioConverter::Init() call to swr_alloc_set_opts2() failed.");
            FFmpeg.swr_free(&swr_ctx);
            return null;
        }

        ret = FFmpeg.swr_init(swr_ctx); // obligatory
        if (ret < 0) {
            Logger.Error("AudioConverter::Init() call to swr_init() failed.");
            FFmpeg.swr_free(&swr_ctx);
            return null;
        }

        AudioConverter converter = new AudioConverter() {
            swr_ctx = swr_ctx,
            sample_buffer = (float*)FFmpeg.av_malloc(BUFFER_LENGTH * sizeof(float)),
            sample_buffer_used = 0,
            channels = (uint)channel_count,
            sample_rate = sample_rate,
            seconds = 0.0
        };

        if (converter.sample_buffer == null) {
            FFmpeg.swr_free(&swr_ctx);
            throw new OutOfMemoryException("failed to allocate AudioConverter sample_buffer");
        }

#if DEBUG
        NativeMemory.Clear(converter.sample_buffer, BUFFER_SIZE_MAX * sizeof(float));
#endif

        return converter;
    }

    public void GetStreamInfo(out uint out_channels, out uint out_rate) {
        out_channels = this.channels;
        out_rate = this.sample_rate;
    }


    public uint Read(void* samples_out, uint max_samples_per_channel) {
        uint required_samples = max_samples_per_channel;
        if (required_samples > this.sample_buffer_used) required_samples = this.sample_buffer_used;

        uint required_bytes = required_samples * this.channels * SAMPLE_SIZE;
        NativeMemory.Copy(this.sample_buffer, samples_out, required_bytes);

        if (required_samples >= this.sample_buffer_used) {
            this.sample_buffer_used = 0;
            return required_samples;
        }

        // shift all remaining samples
        this.sample_buffer_used -= required_samples;
        uint remaining_samples = this.sample_buffer_used;
        uint remaining_bytes = remaining_samples * this.channels * SAMPLE_SIZE;
        NativeMemory.Copy(required_bytes + (byte*)this.sample_buffer, this.sample_buffer, remaining_bytes);

        // calculate time
        this.seconds += required_samples / (double)this.sample_rate;

        return required_samples;
    }

}