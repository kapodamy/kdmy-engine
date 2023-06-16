using System;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;

namespace Engine.Platform;

public enum DDSCompression {
    DXT1,
    DXT3,
    DXT5,
    Unknown
}

public struct DDSBitmap {
    public nint ptr;
    public uint length;
    public uint width;
    public uint height;
}

public class DDSPixelDataBuffer : IPixelDataBuffer {
    private readonly nint dds_ptr;
    private readonly GCHandle dds_buffer_handle;

    private readonly nint first_bitmap_ptr;
    private readonly int first_bitmap_size;

    public unsafe DDSPixelDataBuffer(GCHandle hnd, void* ptr, DDSBitmap[] bitmaps, DDSCompression compression) {
        if (ptr == null) throw new ArgumentNullException(nameof(dds_ptr));
        if (bitmaps == null) throw new ArgumentNullException(nameof(bitmaps));
        if (bitmaps.Length < 1) throw new ArgumentException($"invalid {nameof(bitmaps)} length");

        this.dds_buffer_handle = hnd;
        this.dds_ptr = (int)ptr;

        this.first_bitmap_ptr = bitmaps[0].ptr;
        this.first_bitmap_size = (int)bitmaps[0].length;

        this.Mipmaps = new DDSBitmap[bitmaps.Length - 1];
        this.Compression = compression;

        for (int i = 0 ; i < this.Mipmaps.Length ; i++) {
            this.Mipmaps[i] = bitmaps[i + 1];
        }
    }

    public nint DataPointer { get => first_bitmap_ptr; }

    public int Length { get => first_bitmap_size; }

    public readonly DDSBitmap[] Mipmaps;

    public readonly DDSCompression Compression;

    public void Dispose() {
        if (this.dds_buffer_handle.IsAllocated)
            this.dds_buffer_handle.Free();
        else
            Marshal.FreeHGlobal(this.dds_ptr);

        for (int i = 0 ; i < this.Mipmaps.Length ; i++) {
            this.Mipmaps[i].ptr = 0x00;
        }
    }
}


[StructLayout(LayoutKind.Sequential)]
internal struct DDS_PIXELFORMAT {
    public uint dwSize;
    public DDPF dwFlags;
    public uint dwFourCC;
    public uint dwRGBBitCount;
    public uint dwRBitMask;
    public uint dwGBitMask;
    public uint dwBBitMask;
    public uint dwABitMask;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct DDS_HEADER {
    //
    // Note: signature is not declared in the original DDS_HEADER struct
    // see https://learn.microsoft.com/windows/win32/direct3ddds/dds-header
    //
    public uint signature;

    public uint dwSize;
    public DDSD dwFlags;
    public uint dwHeight;
    public uint dwWidth;
    public uint dwPitchOrLinearSize;
    public uint dwDepth;
    public uint dwMipMapCount;
    public fixed uint dwReserved1[11];
    public DDS_PIXELFORMAT ddspf;
    public DDSCAPS dwCaps;
    public DDSCAPS2 dwCaps2;
    public uint dwCaps3;
    public uint dwCaps4;
    public uint dwReserved2;
}

[Flags]
internal enum DDSD : uint {
    CAPS = 0x1,
    HEIGHT = 0x2,
    WIDTH = 0x4,
    PITCH = 0x8,
    PIXELFORMAT = 0x1000,
    MIPMAPCOUNT = 0x20000,
    LINEARSIZE = 0x80000,
    DEPTH = 0x800000
}

[Flags]
internal enum DDSCAPS : uint {
    COMPLEX = 0x8,
    MIPMAP = 0x400000,
    TEXTURE = 0x1000
}

[Flags]
internal enum DDSCAPS2 : uint {
    NOTHING = 0x000,

    CUBEMAP = 0x200,
    CUBEMAP_POSITIVEX = 0x400,
    CUBEMAP_NEGATIVEX = 0x800,
    CUBEMAP_POSITIVEY = 0x1000,
    CUBEMAP_NEGATIVEY = 0x2000,
    CUBEMAP_POSITIVEZ = 0x4000,
    CUBEMAP_NEGATIVEZ = 0x8000,
    VOLUME = 0x200000
}

[Flags]
internal enum DDPF : uint {
    ALPHAPIXELS = 0x1,
    ALPHA = 0x2,
    FOURCC = 0x4,
    RGB = 0x40,
    YUV = 0x200,
    LUMINANCE = 0x20000
}

internal unsafe class DDS {

    private const uint SIGNATURE = 0x20534444;
    private const uint FOURCC_DXT1 = 0x31545844;
    private const uint FOURCC_DXT3 = 0x33545844;
    private const uint FOURCC_DXT5 = 0x35545844;
    private const uint FOURCC_DX10 = 0x30315844;
    private const uint HEADER_SIZE = 124;
    private const uint PIXELFORMAT_SIZE = 32;
    private const uint BITMAPS_OFFSET = 4 + HEADER_SIZE;

    private const DDSD DDS_HEADER_FLAGS_TEXTURE = DDSD.CAPS | DDSD.HEIGHT | DDSD.WIDTH | DDSD.PIXELFORMAT;
    private const DDSCAPS DDS_SURFACE_FLAGS_MIPMAP = DDSCAPS.COMPLEX | DDSCAPS.MIPMAP;

    public static bool IsDDS(byte[] buffer) {
        if (!BitConverter.IsLittleEndian) {
            throw new NotImplementedException("DDS:IsDDS() big-endian arch not supported");
        }
        fixed (byte* ptr = buffer) {
            DDS_HEADER* dds = (DDS_HEADER*)ptr;
            return dds->signature == SIGNATURE;
        }
    }

    public static bool IsDDS(string filename) {
        // check only the filename
        return filename.EndsWith(".dds", StringComparison.InvariantCultureIgnoreCase);
    }

    public static ImageData Parse(string filename) {
        void* ptr = null;
        int size;

        try {
            // copy file contents outside of the heap memory
            using (FileStream fs = new FileStream(filename, FileMode.Open, FileAccess.Read, FileShare.Read)) {
                size = (int)fs.Length;
                ptr = (void*)Marshal.AllocHGlobal(size);

                byte* data_ptr = (byte*)ptr;
                byte[] buffer = new byte[8 * 1024];
                int read;
                while ((read = fs.Read(buffer, 0, buffer.Length)) != 0) {
                    Marshal.Copy(buffer, 0, (nint)data_ptr, read);
                    data_ptr += read;
                }
            }
        } catch (Exception e) {
            Console.Error.WriteLine($"[ERROR] DDS::Parse() can not open {filename}:\n {e.Message}");
            if (ptr != null) Marshal.FreeHGlobal((nint)ptr);
            return null;
        }

        DDSCompression compression;
        DDSBitmap[] bitmaps = Parse(ptr, (uint)size, out compression);
        if (bitmaps == null) return null;

        ref DDSBitmap first = ref bitmaps[0];
        DDSPixelDataBuffer obj = new DDSPixelDataBuffer(default, ptr, bitmaps, compression);

        ImageData texture = new ImageData(
            obj,
            (int)first.width, (int)first.height,
            (int)first.width, (int)first.height
        );

        //
        // disposing DDSPixelDataBuffer releases the GCHandle
        //
        return texture;
    }

    public static ImageData Parse(byte[] buffer) {
        GCHandle handle = GCHandle.Alloc(buffer, GCHandleType.Pinned);
        void* ptr = handle.AddrOfPinnedObject().ToPointer();
        DDSCompression compression;

        DDSBitmap[] bitmaps = Parse(ptr, (uint)buffer.Length, out compression);
        if (bitmaps == null) return null;

        ref DDSBitmap first = ref bitmaps[0];
        ImageData texture = new ImageData(
            new DDSPixelDataBuffer(handle, null, bitmaps, compression),
            (int)first.width, (int)first.height,
            (int)first.width, (int)first.height
        );

        //
        // disposing DDSPixelDataBuffer releases the GCHandle
        //
        return texture;
    }

    public static DDSBitmap[] Parse(void* data_ptr, uint data_size, out DDSCompression compression) {
        if (!BitConverter.IsLittleEndian) {
            throw new NotImplementedException("DDS:Parse() big-endian arch not supported");
        }

        DDS_HEADER* dds = (DDS_HEADER*)data_ptr;
        uint mipmap_count = 1;
        uint block_size = 16;

        compression = DDSCompression.Unknown;

        if (dds->signature != SIGNATURE) {
            Console.WriteLine("[ERROR] DDS::Parse() invalid file siganture (not a .DDS file)");
            return null;
        }

        if (dds->dwSize != HEADER_SIZE) {
            Console.WriteLine("[ERROR] DDS::Parse() invalid header size");
            return null;
        }

        //
        // here should check DDS_HEADER_FLAGS_TEXTURE instead of REQUIRED_FLAGS but in
        // MSDN warns "you should not rely on those flags being set because some writers
        //            of such a file might not set these flags."
        //
        if (!dds->dwFlags.HasFlag(DDSD.WIDTH | DDSD.HEIGHT)) {
            Console.WriteLine("[ERROR] DDS::Parse() missing one or more required flags in the header");
            return null;
        }

        if (dds->dwFlags.HasFlag(DDSD.MIPMAPCOUNT)) {
            if (dds->dwMipMapCount < 1) {
                Console.WriteLine("[ERROR] DDS::Parse() invalid mipmap count");
                return null;
            }
            mipmap_count = dds->dwMipMapCount;
            Debug.Assert(dds->dwCaps.HasFlag(DDS_SURFACE_FLAGS_MIPMAP));
        }

        if (!dds->dwCaps.HasFlag(DDSCAPS.TEXTURE)) {
            Console.WriteLine("[ERROR] DDS::Parse() missing DDSCAPS_TEXTURE flag in the header");
            return null;
        }

        if (dds->dwCaps2 != DDSCAPS2.NOTHING) {
            Console.WriteLine("[ERROR] DDS::Parse() expected texture, found CUBEMAP or VOLUME");
            return null;
        }

        if (dds->ddspf.dwSize != PIXELFORMAT_SIZE) {
            Console.WriteLine("[ERROR] DDS::Parse() invalid PIXELFORMAT header size");
            return null;
        }

        if (!dds->ddspf.dwFlags.HasFlag(DDPF.FOURCC)) {
            Console.WriteLine("[ERROR] DDS::Parse() uncompressed texture load is not implemented");
            return null;
        }

        switch (dds->ddspf.dwFourCC) {
            case FOURCC_DXT1:
                block_size = 8;
                compression = DDSCompression.DXT1;
                break;
            case FOURCC_DXT3:
                compression = DDSCompression.DXT3;
                break;
            case FOURCC_DXT5:
                compression = DDSCompression.DXT5;
                break;
            case FOURCC_DX10:
                Console.WriteLine($"[ERROR] DDS::Parse() DX10 and DXGI pixel formats are not supported");
                return null;
            default:
                string fourcc = Encoding.ASCII.GetString((byte*)&dds->ddspf.dwFourCC, 4);
                Console.WriteLine($"[ERROR] DDS::Parse() unknown fourCC '{fourcc}'");
                return null;
        }


        // check the amout of bytes required
        uint expected_size = 0;

        for (uint i = 0, width = dds->dwWidth, height = dds->dwHeight ; i < mipmap_count ; i++) {
            expected_size += block_size * ((width + 3) / 4) * ((height + 3) / 4);

            width /= 2;
            height /= 2;

            if (width < 1 || height < 1) {
                i++;
                if (i < mipmap_count) {
                    mipmap_count = i;
                    Console.WriteLine("[WARN] DDS::Parse() invalid mipmap count or dimens are not pow-of-two");
                }
                break;
            }
        }

        if (expected_size > (data_size - BITMAPS_OFFSET)) {
            Console.WriteLine("[ERROR] DDS::Parse() the .DDS file is truncated");
            return null;
        }


        // finally, read all bitmaps
        uint offset = BITMAPS_OFFSET;
        DDSBitmap[] bitmaps = new DDSBitmap[mipmap_count];

        for (uint i = 0, width = dds->dwWidth, height = dds->dwHeight ; i < mipmap_count ; i++) {
            bitmaps[i].ptr = (nint)(offset + (byte*)data_ptr);
            bitmaps[i].length = block_size * ((width + 3) / 4) * ((height + 3) / 4);
            bitmaps[i].width = width;
            bitmaps[i].height = height;

            offset += bitmaps[i].length;
            width /= 2;
            height /= 2;
        }


        return bitmaps;
    }

}

