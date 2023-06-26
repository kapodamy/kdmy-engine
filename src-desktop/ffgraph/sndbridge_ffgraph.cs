using System;
using Engine.Externals.SoundBridge;

namespace Engine.Externals.FFGraphInterop;

public class FFGraphSoundBridgeDecoder : IDecoder {

    private readonly bool allow_seek, allow_dispose;
    private FFGraph ffgraph;

    public FFGraphSoundBridgeDecoder(FFGraph ffgraph, bool allow_seek, bool allow_dispose) {
        if (ffgraph == null) throw new ArgumentNullException(nameof(ffgraph));
        if (ffgraph.audio_fmt == null) throw new ArgumentException(nameof(ffgraph));

        this.ffgraph = ffgraph;
        this.allow_seek = allow_seek;
        this.allow_dispose = allow_dispose;
    }


    public unsafe int Read(float* buffer, uint samples_per_channel) {
        int ret = this.ffgraph.ReadAudioSamples(buffer, samples_per_channel);
        return ret;
    }

    public void GetInfo(out uint rate, out uint channels, out double duration_in_seconds) {
        FFGraphInfo info = this.ffgraph.GetStreamsInfo();

        rate = info.audio_sample_rate;
        channels = info.audio_channels;
        duration_in_seconds = info.audio_seconds_duration * 1000.0;
    }

    public bool Seek(double seconds) {
        if (this.allow_seek) {
            this.ffgraph.audio_fmt.Seek(seconds);
            return true;
        } else {
            return false;
        }
    }

    public void GetLoopPoints(out long loop_start, out long loop_length) {
        loop_start = -1;
        loop_length = -1;
    }

    public void Dispose() {
        if (!this.allow_dispose) return;

        this.ffgraph.Dispose();
        this.ffgraph = null;
    }

}

