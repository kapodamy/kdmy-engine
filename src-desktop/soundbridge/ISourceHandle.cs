using System;
using System.IO;

namespace Engine.Externals.SoundBridge;

public unsafe interface ISourceHandle : IDisposable {
    int Read(void* buffer, int buffer_size);

    int Seek(long offset, SeekOrigin whence);

    long Tell();

    long Length();
}

