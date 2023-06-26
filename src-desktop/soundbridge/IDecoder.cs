using System;

namespace Engine.Externals.SoundBridge;

public unsafe interface IDecoder : IDisposable {

    int Read(float* buffer, uint samples_per_channel);

    void GetInfo(out uint rate, out uint channels, out double duration_in_seconds);

    bool Seek(double seconds);

    void GetLoopPoints(out long loop_start, out long loop_length);

}
