using System;

namespace Engine.Externals.SoundBridge;

public enum SampleFormat : int {
    FLOAT32,
    PCM_S16LE,
    PCM_U8,
    ADPCM_4_YAMAHA
}

public unsafe interface IDecoder : IDisposable {

    int Read(float* buffer, uint samples_per_channel);

    SampleFormat GetInfo(out uint rate, out uint channels, out double duration_in_seconds);

    bool Seek(double seconds);

    void GetLoopPoints(out long loop_start, out long loop_length);

}
