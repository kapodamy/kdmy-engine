"use strict";

/**  @readonly @enum {number} */
const DDSCompression = {
    Unknown: 0,
    DXT1: 1,
    DXT3: 2,
    DXT5: 3
};

/**
 * @typedef {object} DDSBitmap
 * @property {DataView} pixels;
 * @property {number} length
 * @property {number} width
 * @property {number} height
*/

class DDS {
    static #DDS_PIXELFORMAT = {
        dwSize: 0,
        dwFlags: 4,
        dwFourCC: 8,
        dwRGBBitCount: 12,
        dwRBitMask: 16,
        dwGBitMask: 20,
        dwBBitMask: 24,
        dwABitMask: 28,
    };

    static #DDS_HEADER = {
        //
        // Note: signature is not declared in the original DDS_HEADER struct
        // see https://learn.microsoft.com/windows/win32/direct3ddds/dds-header
        //
        signature: 0,

        dwSize: 4,
        dwFlags: 8,
        dwHeight: 12,
        dwWidth: 16,
        dwPitchOrLinearSize: 20,
        dwDepth: 24,
        dwMipMapCount: 28,
        dwReserved1: 32,
        ddspf: 76,
        dwCaps: 76 + /*sizeof(DDS_PIXELFORMAT)*/32 + 0,
        dwCaps2: 76 + /*sizeof(DDS_PIXELFORMAT)*/32 + 4,
        dwCaps3: 76 + /*sizeof(DDS_PIXELFORMAT)*/32 + 8,
        dwCaps4: 76 + /*sizeof(DDS_PIXELFORMAT)*/32 + 12,
        dwReserved2: 76 + /*sizeof(DDS_PIXELFORMAT)*/32 + 16,
    };


    static #DDSD_CAPS = 0x1;
    static #DDSD_HEIGHT = 0x2;
    static #DDSD_WIDTH = 0x4;
    static #DDSD_PITCH = 0x8;
    static #DDSD_PIXELFORMAT = 0x1000;
    static #DDSD_MIPMAPCOUNT = 0x20000;
    static #DDSD_LINEARSIZE = 0x80000;
    static #DDSD_DEPTH = 0x800000;

    static #DDSCAPS_COMPLEX = 0x8;
    static #DDSCAPS_MIPMAP = 0x400000;
    static #DDSCAPS_TEXTURE = 0x1000;

    static #DDSCAPS2_NOTHING = 0x000;
    static #DDSCAPS2_CUBEMAP = 0x200;
    static #DDSCAPS2_CUBEMAP_POSITIVEX = 0x400;
    static #DDSCAPS2_CUBEMAP_NEGATIVEX = 0x800;
    static #DDSCAPS2_CUBEMAP_POSITIVEY = 0x1000;
    static #DDSCAPS2_CUBEMAP_NEGATIVEY = 0x2000;
    static #DDSCAPS2_CUBEMAP_POSITIVEZ = 0x4000;
    static #DDSCAPS2_CUBEMAP_NEGATIVEZ = 0x8000;
    static #DDSCAPS2_VOLUME = 0x200000;

    static #DDPF_ALPHAPIXELS = 0x1;
    static #DDPF_ALPHA = 0x2;
    static #DDPF_FOURCC = 0x4;
    static #DDPF_RGB = 0x40;
    static #DDPF_YUV = 0x200;
    static #DDPF_LUMINANCE = 0x20000;



    static #SIGNATURE = 0x20534444;
    static #FOURCC_DXT1 = 0x31545844;
    static #FOURCC_DXT3 = 0x33545844;
    static #FOURCC_DXT5 = 0x35545844;
    static #FOURCC_DX10 = 0x30315844;
    static #HEADER_SIZE = 124;
    static #PIXELFORMAT_SIZE = 32;
    static #BITMAPS_OFFSET = 4 + DDS.#HEADER_SIZE;

    static #DDS_HEADER_FLAGS_TEXTURE = DDS.#DDSD_CAPS | DDS.#DDSD_HEIGHT | DDS.#DDSD_WIDTH | DDS.#DDSD_PIXELFORMAT;
    static #DDS_SURFACE_FLAGS_MIPMAP = DDS.#DDSCAPS_COMPLEX | DDS.#DDSCAPS_MIPMAP;

    /**@type {DDSBitmap[]}*/Mipmaps;
    /**@type {DDSCompression}*/Compression;

    /**@type {DataView}*/pixels;
    /**@type {number}*/size;
    /**@type {number}*/width;
    /**@type {number}*/height;

    constructor(/**@type {DDSBitmap[]}*/bitmaps, /**@type {DDSCompression}*/compression) {
        this.Mipmaps = bitmaps;
        this.Compression = compression;
        this.width = bitmaps[0].width;
        this.height = bitmaps[0].height;
        this.pixels = bitmaps[0].pixels;
        this.size = bitmaps[0].length;

        this.Mipmaps.shift();
    }

    close() {
        // stub
    }


    static IsDDS(/**@type {ArrayBuffer | string}*/ filename_or_buffer) {
        if (typeof (filename_or_buffer) === "string") {
            return filename_or_buffer.toLowerCase().endsWith(".dds");
        } else if (filename_or_buffer instanceof ArrayBuffer) {
            let dataview = new DataView(filename_or_buffer);
            let signature = dataview.getUint32(DDS.#DDS_HEADER.signature, true);
            return signature == DDS.#SIGNATURE;
        }
        return false;
    }

    static Parse(/**@type {ArrayBuffer}*/ buffer) {
        let compression = [DDSCompression.Unknown];

        let bitmaps = DDS.#Parse(buffer, buffer.byteLength, compression);
        if (bitmaps == null) return null;

        //
        // disposing DDSPixelDataBuffer releases the GCHandle
        //
        return new DDS(bitmaps, compression[0]);
    }

    static #Parse(data_ptr, data_size, out_compression) {
        let dds = new DataView(data_ptr, 0, data_size);
        let ddspf = new DataView(data_ptr, DDS.#DDS_HEADER.ddspf);
        let mipmap_count = 1;
        let block_size = 16;

        out_compression[0] = DDSCompression.Unknown;

        if (dds.getUint32(DDS.#DDS_HEADER.signature, true) != DDS.#SIGNATURE) {
            console.error("DDS::Parse() invalid file siganture (not a .DDS file)");
            return null;
        }

        if (dds.getUint32(DDS.#DDS_HEADER.dwSize, true) != DDS.#HEADER_SIZE) {
            console.error("DDS::Parse() invalid header size");
            return null;
        }

        //
        // here should check DDS_HEADER_FLAGS_TEXTURE instead of REQUIRED_FLAGS but in
        // MSDN warns "you should not rely on those flags being set because some writers
        //            of such a file might not set these flags."
        //
        if (!(dds.getUint32(DDS.#DDS_HEADER.dwFlags, true) & (DDS.#DDSD_WIDTH | DDS.#DDSD_HEIGHT))) {
            console.error("DDS::Parse() missing one or more required flags in the header");
            return null;
        }

        if (dds.getUint32(DDS.#DDS_HEADER.dwFlags, true) & DDS.#DDSD_MIPMAPCOUNT) {
            if (dds.getUint32(DDS.#DDS_HEADER.dwMipMapCount, true) < 1) {
                console.error("DDS::Parse() invalid mipmap count");
                return null;
            }
            mipmap_count = dds.getUint32(DDS.#DDS_HEADER.dwMipMapCount, true);
            console.assert(!!(dds.getUint32(DDS.#DDS_HEADER.dwCaps, true) & DDS.#DDS_SURFACE_FLAGS_MIPMAP));
        }

        if (!(dds.getUint32(DDS.#DDS_HEADER.dwCaps, true) & DDS.#DDSCAPS_TEXTURE)) {
            console.error("DDS::Parse() missing DDSCAPS_TEXTURE flag in the header");
            return null;
        }

        if (dds.getUint32(DDS.#DDS_HEADER.dwCaps2, true) != DDS.#DDSCAPS2_NOTHING) {
            console.error("DDS::Parse() expected texture, found CUBEMAP or VOLUME");
            return null;
        }

        if (ddspf.getUint32(DDS.#DDS_PIXELFORMAT.dwSize, true) != DDS.#PIXELFORMAT_SIZE) {
            console.error("DDS::Parse() invalid PIXELFORMAT header size");
            return null;
        }

        if (!(ddspf.getUint32(DDS.#DDS_PIXELFORMAT.dwFlags, true) & DDS.#DDPF_FOURCC)) {
            console.error("DDS::Parse() uncompressed texture load is not implemented");
            return null;
        }

        switch (ddspf.getUint32(DDS.#DDS_PIXELFORMAT.dwFourCC, true)) {
            case DDS.#FOURCC_DXT1:
                block_size = 8;
                out_compression[0] = DDSCompression.DXT1;
                break;
            case DDS.#FOURCC_DXT3:
                out_compression[0] = DDSCompression.DXT3;
                break;
            case DDS.#FOURCC_DXT5:
                out_compression[0] = DDSCompression.DXT5;
                break;
            case DDS.#FOURCC_DX10:
                console.error("DDS::Parse() DX10 and DXGI pixel formats are not supported");
                return null;
            default:
                let txtdec = new TextDecoder("ascii", { fatal: true, ignoreBOM: true });
                let fourcc = txtdec.decode(
                    new Uint8Array(data_ptr, ddspf.byteOffset + DDS.#DDS_PIXELFORMAT.dwFourCC, 4),
                    { stream: false }
                );
                console.error(`DDS::Parse() unknown fourCC '${fourcc}'`);
                return null;
        }


        // check the amout of bytes required
        let expected_size = 0;

        for (let i = 0, width = dds.getUint32(DDS.#DDS_HEADER.dwWidth, true), height = dds.getUint32(DDS.#DDS_HEADER.dwHeight, true); i < mipmap_count; i++) {
            expected_size += block_size * Math.trunc((width + 3) / 4) * Math.trunc((height + 3) / 4);

            width /= 2;
            height /= 2;

            if (width < 1 || height < 1) {
                i++;
                if (i < mipmap_count) {
                    mipmap_count = i;
                    console.warn("DDS::Parse() invalid mipmap count or dimens are not pow-of-two");
                }
                break;
            }
        }

        if (expected_size > (data_size - DDS.#BITMAPS_OFFSET)) {
            console.error("DDS::Parse() the .DDS file is truncated");
            return null;
        }


        // finally, read all bitmaps
        let offset = DDS.#BITMAPS_OFFSET;
        /**@type{DDSBitmap[]}*/let bitmaps = new Array(mipmap_count);

        for (let i = 0, width = dds.getUint32(DDS.#DDS_HEADER.dwWidth, true), height = dds.getUint32(DDS.#DDS_HEADER.dwHeight, true); i < mipmap_count; i++) {
            let length = block_size * Math.trunc((width + 3) / 4) * Math.trunc((height + 3) / 4);

            bitmaps[i] = {
                pixels: new DataView(data_ptr, offset, length),
                width: width,
                height: height,
                length: length
            };

            offset += length;
            width /= 2;
            height /= 2;
        }


        return bitmaps;
    }

}
