
// pixel formats
const PIXELFMT_ARGB1555 = 0x00;
const PIXELFMT_RGB565 = 0x01;
const PIXELFMT_ARGB4444 = 0x02;
const PIXELFMT_YUV422 = 0x03;

// compression types 
const CODECTYPE_TWIDDLED = 0x01;
const CODECTYPE_TWIDDLED_MIPMAPED = 0x02;
const CODECTYPE_TWIDDLED_VQ = 0x03;
const CODECTYPE_TWIDDLED_VQ_MIPMAPED = 0x04;
const CODECTYPE_NONE = 0x09;
const CODECTYPE_NONE_MIPMAPED = 0x0A;

// VQ codebok entry (2x2 block) with twiddled indexes
const CODEBOOK_ENTRY_TWIDDLED = new Uint8Array([0, 2, 1, 3]);


var input_file = document.querySelector("input");
var article = document.querySelector("article");
var span = document.querySelector("span");


function decode_ARGB1555(/**@type{Uint16Array}*/raw_pixels, /**@type{Uint8ClampedArray}*/output) {
    for (let i = 0, j = 0; i < raw_pixels.length; i++) {
        let texel = raw_pixels[i];

        output[j++] = (texel & 0x7C00) >>> 7;// r
        output[j++] = (texel & 0x03E0) >>> 2;// g
        output[j++] = (texel & 0x001F) << 3;// b
        output[j++] = (texel & 0x8000) ? 0xFF : 0x00;// a
    }
}

function decode_RGB565(/**@type{Uint16Array}*/raw_pixels, /**@type{Uint8ClampedArray}*/output) {
    for (let i = 0, j = 0; i < raw_pixels.length; i++) {
        let texel = raw_pixels[i];

        output[j++] = (texel & 0xF800) >>> 8;// r
        output[j++] = (texel & 0x07E0) >>> 3;// g
        output[j++] = (texel & 0x001F) << 3;// b
        output[j++] = 0xFF;// a
    }

}

function decode_ARGB4444(/**@type{Uint16Array}*/raw_pixels, /**@type{Uint8ClampedArray}*/output) {
    for (let i = 0, j = 0; i < raw_pixels.length; i++) {
        let texel = raw_pixels[i];

        output[j++] = (texel & 0x0F00) >>> 4;// r
        output[j++] = (texel & 0x00F0);// g
        output[j++] = (texel & 0x000F) << 4;// b
        output[j++] = (texel & 0xF000) >>> 8;// a
    }
}

function decode_YUV422(/**@type{Uint16Array}*/raw_pixels, /**@type{Uint8ClampedArray}*/output) {
    function yuv_to_rgb24(/**@type{number}*/y, /**@type{number}*/u, /**@type{number}*/v, /**@type{number}*/a, /**@type{Uint8ClampedArray}*/rgba, /**@type{number}*/index) {
        rgba[index + 0] = y + 1.375 * (v - 128);// r
        rgba[index + 1] = y - 0.6875 * (v - 128) - 0.34375 * (u - 128);// g
        rgba[index + 2] = y + 1.71875 * (u - 128);// b
        rgba[index + 3] = a;// a
    }

    for (let i = 0, j = 0; i < raw_pixels.length;) {
        let texel1 = raw_pixels[i++];
        let texel2 = raw_pixels[i++];

        let y1 = (texel1 & 0xFF00) >>> 8;
        let y2 = (texel2 & 0xFF00) >>> 8;
        let u = (texel1 & 0x00FF);
        let v = (texel2 & 0x00FF);

        yuv_to_rgb24(y1, u, v, 0xFF, output, j); j += 4;
        yuv_to_rgb24(y2, u, v, 0xFF, output, j); j += 4;
    }

}


function render_as_canvas(/**@type{number}*/width, /**@type{number}*/height, /**@type{number}*/pixel_format, /**@type{Uint16Array[]}*/ bitmaps) {
    for (let i = 0; i < bitmaps.length; i++) {
        let canvas = document.createElement("canvas");
        let div = document.createElement("div");
        let b = document.createElement("b");

        b.textContent = `${width}x${height}`;
        div.appendChild(document.createElement("br"));
        div.appendChild(b);
        article.appendChild(div)
        article.appendChild(canvas);


        let raw_rgba = new Uint8ClampedArray(width * height * 4);
        if (pixel_format == PIXELFMT_YUV422) {
            decode_YUV422(bitmaps[i], raw_rgba);
        } else if (pixel_format == PIXELFMT_RGB565) {
            decode_RGB565(bitmaps[i], raw_rgba);
        } else if (pixel_format == PIXELFMT_ARGB4444) {
            decode_ARGB4444(bitmaps[i], raw_rgba);
        } else if (pixel_format == PIXELFMT_ARGB1555) {
            decode_ARGB1555(bitmaps[i], raw_rgba);
        } else {
            throw "Unknown pixel format found in nTextureType: 0x" + pixel_format.toString(16);
        }

        let canvasctx = canvas.getContext("2d");
        let imagedata = new ImageData(raw_rgba, width, height);

        canvas.width = width;
        canvas.height = height;
        canvasctx.clearRect(0, 0, width, height);
        canvasctx.putImageData(imagedata, 0, 0);

        width /= 2;
        height /= 2;
    }
}

function get_texel_from_codebook(/**@type{Uint16Array}*/codebook, /**@type{number}*/entry_index, /**@type{number}*/texel_in_entry_index) {
    return codebook[(entry_index * 4) + texel_in_entry_index];
}

function copyto(/**@type {ArrayBufferView}*/dst, /**@type {ArrayBufferView}*/src, /**@type {number}*/ src_offset, /**@type {number}*/bytes) {
    let _dst = new Uint8Array(dst.buffer, dst.byteOffset, dst.byteLength);
    let _src = new Uint8Array(src.buffer, src.byteOffset + src_offset, bytes);

    _dst.set(_src, 0);
}


function parse_pvrt(/**@type{ArrayBuffer}*/ arraybuffer) {
    let dataview = new DataView(arraybuffer);
    let position = 0;

    // check "GBIX"
    if (dataview.getUint32(position, false) == 0x47424958) {
        position += 16;
        span.textContent += " [has GBIX]"
    }

    // check "PVRT"
    if (dataview.getUint32(position, false) == 0x50565254) {
        position += 4;
    } else {
        throw 'missing "PVRT" header';
    }

    let nTextureDataSize = dataview.getUint32(position, true); position += 4;
    let pixel_format = dataview.getUint8(position); position += 1
    let compression_format = dataview.getUint8(position); position += 1;
    /*let unknown = */dataview.getUint16(position, true); position += 2;
    let nWidth = dataview.getUint16(position, true); position += 2;
    let nHeight = dataview.getUint16(position, true); position += 2;

    let has_mipmaps = false;
    let mipmap_count = 1;
    let texture_data = new Uint16Array(arraybuffer, position, (nTextureDataSize - 8) / 2);

    switch (pixel_format) {
        case PIXELFMT_YUV422:
            span.textContent += ` [YUV422]`;
            break;
        case PIXELFMT_RGB565:
            span.textContent += ` [RGB565]`;
            break;
        case PIXELFMT_ARGB4444:
            span.textContent += ` [ARGB4444]`;
            break;
        case PIXELFMT_ARGB1555:
            span.textContent += ` [ARGB1555]`;
            break;
        default:
            throw "Unknown pixel format: 0x" + pixel_format.toString(16);
    }

    switch (compression_format) {
        case CODECTYPE_TWIDDLED_MIPMAPED:
        case CODECTYPE_TWIDDLED_VQ_MIPMAPED:
        case CODECTYPE_NONE_MIPMAPED:
            has_mipmaps = true;
            mipmap_count = Math.min(calc_mipmaps_count(nWidth), calc_mipmaps_count(nHeight));
            span.textContent += ` [${mipmap_count} mipmaps]`;
            break;
    }

    let bitmaps;
    if (compression_format == CODECTYPE_TWIDDLED_VQ || compression_format == CODECTYPE_TWIDDLED_VQ_MIPMAPED) {
        let codebook = new Uint16Array(arraybuffer, position, 4 * 256);
        let codebook_size = codebook.byteLength;
        let indexes = new Uint8Array(arraybuffer, position + codebook_size, nTextureDataSize - codebook_size - 8);

        span.textContent += ` [twiddled] [VQ]`;
        bitmaps = decode_twiddled_vq(nWidth, codebook, indexes, has_mipmaps);
        // let a = document.createElement("a");
        // a.download = "decoded.data";
        // a.href = URL.createObjectURL(new Blob([bitmaps[0]]));
        // a.click();
    } else if (compression_format == CODECTYPE_TWIDDLED || compression_format == CODECTYPE_TWIDDLED_MIPMAPED) {
        span.textContent += ` [twiddled]`;
        texture_data = texture_data.slice(1, texture_data.length);//skip over dummy placeholders
        bitmaps = decode_twiddled(nWidth, nHeight, texture_data, mipmap_count);
    } else if (compression_format == CODECTYPE_NONE || compression_format == CODECTYPE_NONE_MIPMAPED) {
        bitmaps = decode_non_twiddled(nWidth, nHeight, texture_data, mipmap_count);
    } else {
        throw "Unknown compression format: 0x" + compression_format.toString(16);
    }

    render_as_canvas(nWidth, nHeight, pixel_format, bitmaps);
}

function calc_mipmaps_count(/**@type{number}*/dimmen) {
    let count = 0;
    while (dimmen > 0) {
        count++;
        dimmen = Math.trunc(dimmen / 2);
    }
    return count;
}

function decode_twiddled_vq(/**@type{number}*/dimmen, /**@type{Uint16Array}*/ codebook, /**@type{Uint8Array}*/ indexes, /**@type{boolean}*/ has_mipmaps) {
    let mipmap_count = has_mipmaps ? calc_mipmaps_count(dimmen) : 1;
    let mipmaps_array = new Array(mipmap_count);
    let offset = 0;
    /**@type{TwiddleLookup}*/const lookup = { mask: 0, reverse_mask: 0, shift: 0 };

    for (let level = mipmap_count - 1; level >= 0; level--) {
        let mipmap_dimmen = dimmen >>> level;
        let mipmap = mipmaps_array[level] = new Uint16Array(mipmap_dimmen * mipmap_dimmen);

        if (mipmap_dimmen == 1) {
            // 1x1 mipmap is a single texel
            mipmap[0] = get_texel_from_codebook(codebook, indexes[offset], 0);

            offset++;
            continue;
        }

        let half_mipmap_dimmen = mipmap_dimmen >>> 1;
        let total_indexes = half_mipmap_dimmen * half_mipmap_dimmen;

        twiddle_lookup_init(lookup, half_mipmap_dimmen, half_mipmap_dimmen);

        let x = 0, y = 0;
        for (let written = 0; written < total_indexes; written++) {
            let read_index = twiddle_coords(lookup, Math.trunc(x / 2), Math.trunc(y / 2));
            // let read_index = Math.trunc(x / 2) + (Math.trunc(y / 2)* half_mipmap_dimmen);
            read_index += offset;

            // VQ codebook entry is also twiddled, unpack it
            let x_offset = 0, y_offset = 0;
            for (let i = 0; i < 4; i++) {
                let read_offset = CODEBOOK_ENTRY_TWIDDLED[i];
                // let read_offset = i;
                let write_index = (x + x_offset) + ((y + y_offset) * mipmap_dimmen);

                mipmap[write_index] = get_texel_from_codebook(codebook, indexes[read_index], read_offset);

                x_offset++;
                if (x_offset >= 2) {
                    x_offset = 0;
                    y_offset++;
                }
            }

            x += 2;
            if (x >= mipmap_dimmen) {
                x = 0;
                y += 2;
            }
        }

        offset += total_indexes;
    }

    return mipmaps_array;
}

function decode_non_twiddled(/**@type{number}*/width, /**@type{number}*/height, /**@type{Uint16Array}*/ data, /**@type{number}*/mipmap_count) {
    let mipmaps_array = new Array(mipmap_count);
    let offset_in_bytes = 0;

    for (let level = mipmap_count - 1; level >= 0; level--) {
        let mipmap_width = width >>> level;
        let mipmap_height = height >>> level;

        let total_pixels = mipmap_width * mipmap_height;
        let size_in_bytes = total_pixels * 2;

        mipmaps_array[level] = new Uint16Array(total_pixels);
        copyto(mipmaps_array[level], data, offset_in_bytes, size_in_bytes);

        offset_in_bytes += size_in_bytes;
    }

    return mipmaps_array;
}

function decode_twiddled(/**@type{number}*/width, /**@type{number}*/height, /**@type{Uint16Array}*/ data, /**@type{number}*/mipmap_count) {
    let mipmaps_array = new Array(mipmap_count);
    let offset = 0;
    /**@type{TwiddleLookup}*/const lookup = { mask: 0, reverse_mask: 0, shift: 0 };

    for (let level = mipmap_count - 1; level >= 0; level--) {
        let mipmap_width = width >>> level;
        let mipmap_height = height >>> level;
        let total_pixels = mipmap_width * mipmap_height;
        let mipmap = mipmaps_array[level] = new Uint16Array(total_pixels);

        twiddle_lookup_init(lookup, mipmap_width, mipmap_height);

        let x = 0, y = 0;
        for (let written = 0; written < total_pixels; written++) {
            let read_index = twiddle_coords(lookup, x, y) + offset;
            let write_index = (x + (y * mipmap_width));

            mipmap[write_index] = data[read_index];

            x++;
            if (x >= mipmap_width) {
                x = 0;
                y++;
            }
        }

        offset += total_pixels;
    }

    return mipmaps_array;
}


twiddle_table_init();
input_file.addEventListener("change", async function () {
    article.replaceChildren();

    try {
        let file = input_file.files[0];
        span.textContent = `${file.name} ${(file.size / 1024).toFixed(2)}KiB`;
        let arraybuffer = await file.arrayBuffer();
        parse_pvrt(arraybuffer);
    } catch (e) {
        console.error(e);
        alert("failed to read the file.\n" + e);
    }
});
