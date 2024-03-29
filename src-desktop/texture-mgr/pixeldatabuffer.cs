﻿using System;

namespace Engine.Platform;

public interface IPixelDataBufferBuilder {
    IPixelDataBuffer CreatePixelDataBuffer(int byte_size);

    bool CanCreatePixelDataBuffer();
}

public interface IPixelDataBuffer : IDisposable {
    nint DataPointer { get; }
    int Length { get; }
}
