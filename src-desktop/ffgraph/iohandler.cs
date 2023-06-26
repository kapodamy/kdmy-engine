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
    private const int FILEHANDLE_MAX_BUFFERED_FILE_LENGTH = 128 * 1024 * 1024;// 128MiB

    private GCHandle file_hnd;
    private AVIOContext* avio_ctx;

    private IOHandler() { }


    private static IFileSource Recover(nint ptr) {
        return (IFileSource)GCHandle.FromIntPtr(ptr).Target;
    }


    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    static int Read(nint opaque, byte* buf, int buf_size) {
        IFileSource filehandle = Recover(opaque);

        int readed = filehandle.Read(buf, buf_size);

        // check if the end-of-file was reached
        if (readed < 0) return FFmpeg.AVERROR_EOF;

        return readed;
    }

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    static long Seek(nint opaque, long offset, int whence) {
        IFileSource filehandle = Recover(opaque);
        if (whence == FFmpeg.AVSEEK_SIZE) return filehandle.Length();

        whence &= ~FFmpeg.AVSEEK_FORCE;

        long new_offset = filehandle.Seek(offset, (SeekOrigin)whence);

        return new_offset < 0 ? FFmpeg.AVERROR_UNKNOWN : new_offset;
    }


    public static IOHandler Init(IFileSource filehandle) {
        IFileSource filehnd = filehandle;

        // just in case
        filehnd.Seek(0, SeekOrigin.Begin);

        int buffer_size;
        if (filehnd.Length() > FILEHANDLE_MAX_BUFFERED_FILE_LENGTH)
            buffer_size = BUFFER_SIZE_BIG_FILES;
        else
            buffer_size = BUFFER_SIZE;

        byte* buffer;

        try {
            buffer = (byte*)FFmpeg.av_malloc((nuint)buffer_size);
        } catch {
            Console.Error.WriteLine("[ERROR] iohandler_init() Can not allocate the buffer.");
            return null;
        }

        IOHandler iohandler = new IOHandler();

        iohandler.file_hnd = GCHandle.Alloc(filehnd, GCHandleType.Normal);
        nint ptr = GCHandle.ToIntPtr(iohandler.file_hnd);

        iohandler.avio_ctx = FFmpeg.avio_alloc_context(
            buffer, buffer_size, 0, ptr, &Read, null, &Seek
        );

        if (iohandler.avio_ctx == null) {
            Console.Error.WriteLine("[ERROR] call to avio_alloc_context() failed.");
            iohandler.file_hnd.Free();
            return null;
        }

        return iohandler;
    }

    public void Dispose() {
        if (this.avio_ctx->buffer != null) FFmpeg.av_free(this.avio_ctx->buffer);

        fixed (AVIOContext** ptr = &avio_ctx) {
            FFmpeg.avio_context_free(ptr);
        }

        this.file_hnd.Free();
    }

    public AVIOContext* IOContext {
        get
        {
            return this.avio_ctx;
        }
    }

}

