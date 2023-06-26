using System;
using System.IO;

namespace Engine.Externals.SoundBridge;

public unsafe interface IFileSource : IDisposable {
    int Read(void* buffer, int buffer_size);

    long Seek(long offset, SeekOrigin whence);

    long Tell();

    long Length();
}

