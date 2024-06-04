using System;
using System.IO;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Engine.Externals.FFGraphInterop.Interop;
using Engine.Externals.SoundBridge;

namespace Engine.Externals.FFGraphInterop;

public struct FFGraphInfo {
    public bool audio_has_stream;
    public bool video_has_stream;

    public uint audio_channels;
    public uint audio_sample_rate;

    public int video_width;
    public int video_height;

    public double video_seconds_duration;
    public double audio_seconds_duration;
}

public unsafe class FFGraph {

    internal FFGraphFormat video_fmt;
    internal FFGraphFormat audio_fmt;
    internal VideoConverter video_converter;
    internal AudioConverter audio_converter;


    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    internal static double CalculateSeconds(AVStream* av_stream, AVFrame* av_frame) {
        long start_pts = av_stream->start_time;
        if (start_pts == FFmpeg.AV_NOPTS_VALUE) start_pts = 0;

        double time_base = av_stream->time_base.num / (double)av_stream->time_base.den;
        double seconds = (av_frame->pts - start_pts) * time_base;

        return seconds;
    }

    public static FFGraph Init(ISourceHandle video_sourcehandle, ISourceHandle audio_sourcehandle) {
        if (video_sourcehandle == audio_sourcehandle) {
            Logger.Error("FFGraph::Init() error: due to design constraints both sourcehandles can not be equal.");
            return null;
        }

        FFGraphFormat video = FFGraphFormat.Init(video_sourcehandle, AVMediaType.AVMEDIA_TYPE_VIDEO);
        FFGraphFormat audio = FFGraphFormat.Init(audio_sourcehandle, AVMediaType.AVMEDIA_TYPE_AUDIO);

        if (video == null && audio == null) {
            Logger.Error("FFGraph::Init() failed, no audio/video stream available.");
            return null;
        }

        FFGraph ffgraph = new FFGraph() {
            video_fmt = video,
            audio_fmt = audio,
            video_converter = (VideoConverter)(video?.ffgraphconv),
            audio_converter = (AudioConverter)(audio?.ffgraphconv)
        };

        return ffgraph;
    }

    public void Dispose() {
        if (this.audio_fmt != null) this.audio_fmt.Dispose();
        if (this.video_fmt != null) this.video_fmt.Dispose();
    }


    public FFGraphInfo GetStreamsInfo() {
        FFGraphInfo output_info = new FFGraphInfo();

        output_info.audio_has_stream = this.audio_fmt != null;
        if (output_info.audio_has_stream) {
            output_info.audio_seconds_duration = this.audio_fmt.GetStreamDuration();
            this.audio_converter.GetStreamInfo(
                out output_info.audio_channels, out output_info.audio_sample_rate
            );
        }

        output_info.video_has_stream = this.video_fmt != null;
        if (output_info.video_has_stream) {
            uint video_width, video_height;
            output_info.video_seconds_duration = this.video_fmt.GetStreamDuration();
            this.video_converter.GetStreamInfo(
                out video_width, out video_height
            );
            output_info.video_width = (int)video_width;
            output_info.video_height = (int)video_height;
        }

        return output_info;
    }

    public int ReadAudioSamples(void* out_samples, uint max_samples_per_channel) {
        if (this.audio_fmt == null) {
            return -1;
        }
        if (max_samples_per_channel < 1) {
            return 0;
        }

        uint ret = this.audio_converter.Read(out_samples, max_samples_per_channel);

        if (ret > 0) {
            return (int)ret;
        } else if (ret < 1 && this.audio_fmt.has_ended) {
            return -1;
        }

        // ignore return value
        this.audio_fmt.Read();

        return (int)this.audio_converter.Read(out_samples, max_samples_per_channel);
    }

    public double ReadVideoFrame(void** out_frame, int* out_frame_size) {
        if (this.video_fmt == null || this.video_fmt.has_ended) {
            return -2.0;
        }

        if (!this.video_fmt.Read()) {
            return -1.0;
        }

        // do not care, required for non-monotonous timetamp
        const double seconds = -1.0;

        return this.video_converter.Read(seconds, out_frame, out_frame_size);
    }

    public double ReadVideoFrame2(nint buffer, int buffer_size) {
        void* frame;
        int frame_size;

        double ret = ReadVideoFrame(&frame, &frame_size);
        if (ret < 0) return ret;

        int to_copy = frame_size < buffer_size ? frame_size : buffer_size;
        NativeMemory.Copy(frame, (void*)buffer, (nuint)to_copy);

        return ret;
    }

    public void Seek(double time_in_seconds) {
        if (this.video_fmt != null)
            this.video_fmt.Seek(time_in_seconds);

        if (this.audio_fmt != null)
            this.audio_fmt.Seek(time_in_seconds);
    }

    public void Seek2(double time_in_seconds, bool audio_or_video) {
        FFGraphFormat ffgraphfmt = audio_or_video ? this.audio_fmt : this.video_fmt;
        if (ffgraphfmt == null) return;
        ffgraphfmt.Seek(time_in_seconds);
    }

    public static string GetRuntimeInfo() {
        try {
            uint version_avformat = FFmpeg.avformat_version();
            uint version_avcodec = FFmpeg.avcodec_version();
            uint version_swresample = FFmpeg.swresample_version();
            uint version_swscale = FFmpeg.swscale_version();
            uint version_avutil = FFmpeg.avutil_version();

            return String.Format(
                "avf={0}.{1}.{2} avc={3}.{4}.{5} swr={6}.{7}.{8} sws={9}.{10}.{11} avu={12}.{13}.{14}",
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
        } catch {
            // one or more FFmpeg dll files are missing
            return null;
        }
    }

#if DEBUG

    public static int Main() {
        Logger.Info("version: " + Marshal.PtrToStringUTF8((nint)FFmpeg.av_version_info()));
        Logger.Info("runtime: " + GetRuntimeInfo());

        int video_frame_index = 0;
        int samples_max = 1024 * 1024;
        float* samples = (float*)NativeMemory.Alloc((nuint)samples_max * sizeof(float));
        FileStream audio_raw = new FileStream("./_audio.data", FileMode.Create, FileAccess.Write);

        ISourceHandle audio_sourcehandle = FileHandleUtil.Init("./a.webm", false);
        ISourceHandle video_sourcehandle = FileHandleUtil.Init("./a.webm", false);

        FFGraph ffgraph = FFGraph.Init(video_sourcehandle, audio_sourcehandle);

        FFGraphInfo info = ffgraph.GetStreamsInfo();

        int frame_video_stop = 40;
        int samples_audio_stop = 106;
        int iters = 0;
        bool test_seek = true;
L_read_streams:

        while (!ffgraph.video_fmt.has_ended) {
            void* frame;
            int frame_size;
            double seconds = ffgraph.ReadVideoFrame(&frame, &frame_size);
            if (seconds < 0 || iters++ >= frame_video_stop) {
                Logger.Log("break: seconds < 0");
                break;
            }

            // dump video frame
            string name = $"_frame_{video_frame_index++}.data";
            using (FileStream video_raw = new FileStream(name, FileMode.Create, FileAccess.Write)) {
                video_raw.Write(new Span<byte>(frame, frame_size));
            }
        }

        iters = 0;
        while (!ffgraph.audio_fmt.has_ended) {
            int readed = ffgraph.ReadAudioSamples(samples, (uint)(samples_max / info.audio_channels));
            if (readed < 0 || iters++ >= samples_audio_stop) {
                Logger.Log("break: readed < 0");
                break;
            }

            // dump audio frames
            audio_raw.Write(new Span<byte>(samples, (int)(sizeof(float) * readed * info.audio_channels)));
        }

        if (test_seek) {
            samples_audio_stop = frame_video_stop = Int32.MaxValue;
            test_seek = false;

            ffgraph.Seek(0.458);
            audio_raw.Flush();
            video_frame_index += 1000;
            audio_raw.Seek(0, SeekOrigin.Begin);

            goto L_read_streams;
        }

        ffgraph.Dispose();
        audio_raw.Dispose();
        video_sourcehandle.Dispose();
        audio_sourcehandle.Dispose();

        return 0;
    }
#endif

}
