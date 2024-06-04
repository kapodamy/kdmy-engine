using System;
using System.IO;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Engine.Externals.FFGraphInterop.Interop;
using Engine.Externals.SoundBridge;

namespace Engine.Externals.FFGraphInterop;

internal unsafe class IOHandler : IDisposable {

    private const int BUFFER_SIZE = 256 * 1024;// 256KiB
    private const int BUFFER_SIZE_BIG_FILES = 1024 * 1024 * 4;// 4MiB
    private const int SOURCEHANDLE_MAX_BUFFERED_FILE_LENGTH = 128 * 1024 * 1024;// 128MiB

    private GCHandle src_hnd;
    private AVIOContext* avio_ctx;

    private IOHandler() { }


    private static ISourceHandle Recover(nint ptr) {
        return (ISourceHandle)GCHandle.FromIntPtr(ptr).Target;
    }


    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    static int Read(nint opaque, byte* buf, int buf_size) {
        ISourceHandle sourcehandle = Recover(opaque);

        int readed = sourcehandle.Read(buf, buf_size);

        if (readed == 0)
            return FFmpeg.AVERROR_EOF;
        else if (readed < 0)
            return FFmpeg.AVERROR_UNKNOWN;
        else
            return readed;
    }

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    static long Seek(nint opaque, long offset, int whence) {
        ISourceHandle sourcehandle = Recover(opaque);
        if (whence == FFmpeg.AVSEEK_SIZE) return sourcehandle.Length();

        whence &= ~FFmpeg.AVSEEK_FORCE;

        int ret = sourcehandle.Seek(offset, (SeekOrigin)whence);

        if (ret > 0)
            return FFmpeg.AVERROR_UNKNOWN;
        else
            return sourcehandle.Tell();
    }


    public static IOHandler Init(ISourceHandle src_hnd) {
        // just in case
        src_hnd.Seek(0, SeekOrigin.Begin);

        int buffer_size;
        if (src_hnd.Length() > SOURCEHANDLE_MAX_BUFFERED_FILE_LENGTH)
            buffer_size = BUFFER_SIZE_BIG_FILES;
        else
            buffer_size = BUFFER_SIZE;

        byte* buffer;

        try {
            buffer = (byte*)FFmpeg.av_malloc((nuint)buffer_size);
        } catch {
            Logger.Error("IOHanlder::Init() Can not allocate the buffer.");
            return null;
        }

        IOHandler iohandler = new IOHandler();

        iohandler.src_hnd = GCHandle.Alloc(src_hnd, GCHandleType.Normal);
        nint ptr = GCHandle.ToIntPtr(iohandler.src_hnd);

        iohandler.avio_ctx = FFmpeg.avio_alloc_context(
            buffer, buffer_size, 0, ptr, &Read, null, &Seek
        );

        if (iohandler.avio_ctx == null) {
            Logger.Error("IOHanlder::Init() call to avio_alloc_context() failed.");
            iohandler.src_hnd.Free();
            return null;
        }

        return iohandler;
    }

    public void Dispose() {
        if (this.avio_ctx->buffer != null) FFmpeg.av_free(this.avio_ctx->buffer);

        fixed (AVIOContext** ptr = &avio_ctx) {
            FFmpeg.avio_context_free(ptr);
        }

        this.src_hnd.Free();
    }

    public AVIOContext* IOContext {
        get
        {
            return this.avio_ctx;
        }
    }

}

