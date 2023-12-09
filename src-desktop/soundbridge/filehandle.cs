using System;
using System.IO;
using System.Runtime.InteropServices;

namespace Engine.Externals.SoundBridge;

internal unsafe class UnmanagedBufferHandle : ISourceHandle {
    internal const int EINVAL = 22;
    internal const int ESPIPE = 29;
    
    private long size;
    private long offset;
    private byte* data;
    private readonly bool allow_dispose;

    public UnmanagedBufferHandle(void* data, long size, bool allow_dispose) {
        this.data = (byte*)data;
        this.size = size;
        this.allow_dispose = allow_dispose;
    }


    public int Read(void* buffer, int buffer_size) {
        if (buffer_size < 0) return -1;
        if (this.offset >= size) return 0;

        long end = this.offset + buffer_size;
        if (end > this.size) buffer_size = (int)(this.size - this.offset);

        NativeMemory.Copy(this.data + this.offset, buffer, (nuint)buffer_size);
        this.offset += buffer_size;

        return buffer_size;
    }

    public int Seek(long offset, SeekOrigin whence) {
        long newoffset;

        switch (whence) {
            case SeekOrigin.Begin:
                newoffset = offset;
                break;
            case SeekOrigin.Current:
                newoffset = this.offset + offset;
                break;
            case SeekOrigin.End:
                newoffset = this.size - offset;
                break;
            default:
                return UnmanagedBufferHandle.EINVAL;
        }

        if (newoffset < 0 || newoffset > this.size) return UnmanagedBufferHandle.ESPIPE;
        this.offset = newoffset;
        return 0;
    }

    public long Tell() {
        return this.offset;
    }

    public long Length() {
        return this.size;
    }

    public void Dispose() {
        if (this.data == null) return;
        if (this.allow_dispose) NativeMemory.Free(this.data);

        this.data = null;
        this.size = this.offset = -1;
    }

}

internal unsafe class ManagedBufferHandle : ISourceHandle {
    private readonly int base_offset;
    private int base_length;
    private int offset;
    private byte[] data;

    public ManagedBufferHandle(byte[] data, int base_offset, int base_length) {
        this.data = data;
        this.base_offset = base_offset;
        this.base_length = base_length;
    }


    public int Read(void* buffer, int buffer_size) {
        if (buffer_size < 0) return -1;
        if (this.offset >= this.base_length) return 0;

        long end = this.offset + buffer_size;
        if (end > this.base_length) buffer_size = (int)(this.base_length - this.offset);

        fixed (byte* buff = data) {
            NativeMemory.Copy(buff + this.offset + this.base_offset, buffer, (nuint)buffer_size);
        }

        this.offset += buffer_size;

        return buffer_size;
    }

    public int Seek(long offset, SeekOrigin whence) {
        long newoffset;

        switch (whence) {
            case SeekOrigin.Begin:
                newoffset = offset;
                break;
            case SeekOrigin.Current:
                newoffset = this.offset + offset;
                break;
            case SeekOrigin.End:
                newoffset = this.base_length - offset;
                break;
            default:
                return UnmanagedBufferHandle.EINVAL;
        }

        if (newoffset < 0 || newoffset > this.base_length) return UnmanagedBufferHandle.ESPIPE;
        this.offset = (int)newoffset;
        return 0;
    }

    public long Tell() {
        return this.offset;
    }

    public long Length() {
        return this.base_length;
    }

    public void Dispose() {
        this.data = null;
        this.base_length = this.offset = -1;
    }
}

internal unsafe class FileHandle : ISourceHandle {
    private const int EIO = 5;

    private readonly FileStream file;
    private readonly bool allow_dispose;

    public FileHandle(FileStream file, bool allow_dispose) {
        this.file = file;
        this.allow_dispose = allow_dispose;
    }

    public int Read(void* buffer, int buffer_size) {
        if (buffer_size < 0)
            return -1;
        else
            return this.file.Read(new Span<byte>(buffer, buffer_size));
    }
    
    public int Seek(long offset, SeekOrigin whence) {
        try {
            this.file.Seek(offset, whence);
            return 0;
        } catch {
            return FileHandle.EIO;
        }
    }

    public long Tell() {
        return this.file.Position;
    }

    public long Length() {
        return this.file.Length;
    }

    public void Dispose() {
        if (this.file == null) return;
        if (this.allow_dispose) this.file.Dispose();
    }
}

public unsafe class FileHandleUtil {

    public const int MAX_SIZE_IN_MEMORY = 128 * 1024 * 1024;// 128MiB

    public static ISourceHandle Init(string fullpath, bool try_load_in_ram) {
        FileStream file;

        try {
            file = new FileStream(fullpath, FileMode.Open, FileAccess.Read);
        } catch {
            return null;
        }

        long length = file.Length;

        if (length > MAX_SIZE_IN_MEMORY || !try_load_in_ram) {
            return new FileHandle(file, true);
        }

        void* buffer;

        try {
            buffer = NativeMemory.Alloc((nuint)length);
        } catch {
            file.Dispose();
            return null;
        }



        int readed = file.Read(new Span<byte>(buffer, (int)length));
        if (readed < length) {
            buffer = NativeMemory.Realloc(buffer, (nuint)readed);
        }

        file.Dispose();

        return new UnmanagedBufferHandle(buffer, readed, true);
    }

    public static ISourceHandle Init(nint buffer, int size) {
        if (buffer == 0x00) return null;

        return new UnmanagedBufferHandle((void*)buffer, size, false);
    }

    public static ISourceHandle Init(byte[] buffer, int offset, int size) {
        if (buffer == null) return null;
        if (offset >= buffer.Length || offset > size || offset < 0) throw new ArgumentOutOfRangeException("offset");
        if (size > buffer.Length || size < offset || size < 0) throw new ArgumentOutOfRangeException("size");

        return new ManagedBufferHandle(buffer, offset, size);
    }

}
