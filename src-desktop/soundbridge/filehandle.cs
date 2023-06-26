using System;
using System.IO;
using System.Runtime.InteropServices;

namespace Engine.Externals.SoundBridge;

internal unsafe class UnmanagedBufferHandle : IFileSource {
    private long size;
    private long offset;
    private void* data;
    private readonly bool allow_dispose;

    public UnmanagedBufferHandle(void* data, long size, bool allow_dispose) {
        this.data = data;
        this.size = size;
        this.allow_dispose = allow_dispose;
    }


    public int Read(void* buffer, int buffer_size) {
        if (offset >= size) return -1;

        long end = offset + buffer_size;
        if (end > size) buffer_size = (int)(size - offset);

        NativeMemory.Copy((byte*)data + offset, buffer, (nuint)buffer_size);
        offset += buffer_size;

        return buffer_size;
    }

    public long Seek(long offset, SeekOrigin whence) {
        long newoffset;

        switch (whence) {
            case SeekOrigin.Begin:
                newoffset = offset;
                break;
            case SeekOrigin.Current:
                newoffset = this.offset + offset;
                break;
            case SeekOrigin.End:
                newoffset = size - offset;
                break;
            default:
                return -1;
        }

        if (newoffset < 0 || newoffset > size) return -2;
        this.offset = newoffset;
        return newoffset;
    }

    public long Tell() {
        return offset;
    }

    public long Length() {
        return size;
    }

    public void Dispose() {
        if (data == null) return;
        if (allow_dispose) NativeMemory.Free(data);

        data = null;
        size = offset = -1;
    }

}

internal unsafe class ManagedBufferHandle : IFileSource {
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
        if (offset >= base_length) return -1;

        long end = offset + buffer_size;
        if (end > base_length) buffer_size = (int)(base_length - offset);

        fixed (byte* buff = data) {
            NativeMemory.Copy(buff + offset + base_offset, buffer, (nuint)buffer_size);
        }

        offset += buffer_size;

        return buffer_size;
    }

    public long Seek(long offset, SeekOrigin whence) {
        long newoffset;

        switch (whence) {
            case SeekOrigin.Begin:
                newoffset = offset;
                break;
            case SeekOrigin.Current:
                newoffset = this.offset + offset;
                break;
            case SeekOrigin.End:
                newoffset = base_length - offset;
                break;
            default:
                return -1;
        }

        if (newoffset < 0 || newoffset > base_length) return -2;
        this.offset = (int)newoffset;
        return newoffset;
    }

    public long Tell() {
        return offset;
    }

    public long Length() {
        return base_length;
    }

    public void Dispose() {
        data = null;
        base_length = offset = -1;
    }
}

internal unsafe class FileHandle : IFileSource {

    private readonly FileStream file;
    private readonly bool allow_dispose;

    public FileHandle(FileStream file, bool allow_dispose) {
        this.file = file;
        this.allow_dispose = allow_dispose;
    }

    public int Read(void* buffer, int buffer_size) {
        if (file.Position >= file.Length)
            return -1;
        else
            return file.Read(new Span<byte>(buffer, buffer_size));
    }
    
    public long Seek(long offset, SeekOrigin whence) {
        return file.Seek(offset, whence);
    }

    public long Tell() {
        return file.Position;
    }

    public long Length() {
        return file.Length;
    }

    public void Dispose() {
        if (file == null) return;
        if (allow_dispose) file.Dispose();
    }
}

public unsafe class FileHandleUtil {

    public const int MAX_SIZE_IN_MEMORY = 128 * 1024 * 1024;// 128MiB

    public static IFileSource Init(string fullpath, bool try_load_in_ram) {
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

    public static IFileSource Init(nint buffer, int size) {
        if (buffer == 0x00) return null;

        return new UnmanagedBufferHandle((void*)buffer, size, false);
    }

    public static IFileSource Init(byte[] buffer, int offset, int size) {
        if (buffer == null) return null;
        if (offset >= buffer.Length || offset > size || offset < 0) throw new ArgumentOutOfRangeException("offset");
        if (size > buffer.Length || size < offset || size < 0) throw new ArgumentOutOfRangeException("size");

        return new ManagedBufferHandle(buffer, offset, size);
    }

}
