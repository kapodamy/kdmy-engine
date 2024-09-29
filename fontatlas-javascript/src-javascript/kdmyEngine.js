

class DataReader {
    /**
     * @param {ArrayBuffer} buffer 
     * @param {number} ptr 
     */
    constructor(buffer, ptr) {
        this._dataview = new DataView(buffer, ptr);
        this._little_endian = (new Uint16Array((new Uint8Array([0xFF, 0x00])).buffer))[0] == 0xFF;
        this._offset = 0;
    }

    getPointer() {
        // 32bit wasm memory
        return this.getUint32();
    }

    getFloat32() {
        let value = this._dataview.getFloat32(this._offset, this._little_endian);
        this._offset += Float32Array.BYTES_PER_ELEMENT;
        return value;
    }

    getUint32() {
        let value = this._dataview.getUint32(this._offset, this._little_endian);
        this._offset += Uint32Array.BYTES_PER_ELEMENT;
        return value;
    }

    getInt32() {
        let value = this._dataview.getInt32(this._offset, this._little_endian);
        this._offset += Int32Array.BYTES_PER_ELEMENT;
        return value;
    }

    getUint16() {
        let value = this._dataview.getUint16(this._offset, this._little_endian);
        this._offset += Uint16Array.BYTES_PER_ELEMENT;
        return value;
    }

    getInt16() {
        let value = this._dataview.getInt16(this._offset, this._little_endian);
        this._offset += Int16Array.BYTES_PER_ELEMENT;
        return value;
    }

    getUint8() {
        let value = this._dataview.getUint8(this._offset);
        this._offset += Uint8Array.BYTES_PER_ELEMENT;
        return value;
    }

}

/**
 * @param {number} ptr 
 * @returns {FontCharMap}
 */
function kdmyEngine_parseFontCharMap(ptr) {
    let dataReader = new DataReader(wasmMemory.buffer, ptr);

    let fontcharmap_char_array = new DataReader(wasmMemory.buffer, dataReader.getPointer());
    let fontcharmap_char_array_size = dataReader.getInt32();
    let fontcharmap_texture = dataReader.getPointer();
    let fontcharmap_texture_width = dataReader.getUint16();
    let fontcharmap_texture_height = dataReader.getUint16();
    let fontcharmap_texture_byte_size = dataReader.getUint32();
    let fontcharmap_ascender = dataReader.getFloat32();
    let fontcharmap_line_height = dataReader.getInt16();
    dataReader.getPointer();// kernings_array

    let texture = HEAPU8.subarray(fontcharmap_texture, fontcharmap_texture + fontcharmap_texture_byte_size);

    /**@type {FontCharData[]} */
    let char_array = malloc_for_array(fontcharmap_char_array_size);

    for (let i = 0; i < fontcharmap_char_array_size; i++) {
        let field_codepoint = fontcharmap_char_array.getUint32();
        let field_offset_x = fontcharmap_char_array.getInt16();
        let field_offset_y = fontcharmap_char_array.getInt16();
        let field_advancex = fontcharmap_char_array.getInt16();
        let field_advancey = fontcharmap_char_array.getInt16();
        let field_width = fontcharmap_char_array.getUint16();
        let field_height = fontcharmap_char_array.getUint16();
        let field_kernings = fontcharmap_char_array.getPointer();
        let field_kernings_size = fontcharmap_char_array.getInt32();
        let atlas_entry_x = fontcharmap_char_array.getUint16();
        let atlas_entry_y = fontcharmap_char_array.getUint16();
        let field_has_atlas_entry = fontcharmap_char_array.getUint8();

        char_array[i] = {
            codepoint: field_codepoint,
            offset_x: field_offset_x,
            offset_y: field_offset_y,
            advancex: field_advancex,
            advancey: field_advancey,
            width: field_width,
            height: field_height,
            kernings: malloc_for_array(field_kernings_size),
            kernings_size: field_kernings_size,
            atlas_entry: {
                x: atlas_entry_x,
                y: atlas_entry_y
            },
            has_atlas_entry: field_has_atlas_entry != 0
        };

        if (field_kernings != 0) {
            let kernings_dataReader = new DataReader(wasmMemory.buffer, field_kernings);
            for (let j = 0; j < field_kernings_size; j++) {
                char_array[i].kernings[j] = {
                    codepoint: kernings_dataReader.getUint32(),
                    x: kernings_dataReader.getInt32(),
                };
            }
        }
    }

    return {
        char_array_size: fontcharmap_char_array_size,
        char_array: char_array,
        texture: texture,
        texture_width: fontcharmap_texture_width,
        texture_height: fontcharmap_texture_height,
        texture_byte_size: fontcharmap_texture_byte_size,
        ascender: fontcharmap_ascender,
        line_height: fontcharmap_line_height,
        __ptr: ptr
    };
}
function kdmyEngine_ptrToString(/**@type {number}*/ptr) {
    if (ptr == 0) return null;

    const textDecoder = new TextDecoder("UTF-8", { ignoreBOM: true, fatal: true });

    let end = HEAPU8.indexOf(0x00, ptr);
    if (end < 0) end = HEAPU8.byteLength;// this never should happen

    let char_array = HEAPU8.subarray(ptr, end);
    return textDecoder.decode(char_array, { stream: false });
}

/**@type {ModuleFontAtlas} */
const FONTATLAS_EXPORTS = {
    enable_sdf: function (enable) {
        _fontatlas_enable_sdf(enable ? 1 : 0);
    },
    init: function (font_data, font_data_size) {
        if (font_data == null) throw new Error("The parameter 'font_data' can not be null");
        if (!(font_data_size > 0)) throw new Error("Invalid parameter 'font_data_size'");

        font_data_size |= 0;

        // copy the buffer into wasm memory
        let font_data_ptr = _malloc(font_data_size);
        if (font_data_ptr == 0) throw new Error(`Failed to allocate ${font_data_size} bytes`);

        HEAPU8.set(new Uint8Array(font_data, 0, font_data_size), font_data_ptr);

        let ptr = _fontatlas_init(font_data_ptr, font_data_size);
        if (!ptr) {
            _free(font_data_ptr);
            return null;
        }

        return {
            fontatlas_ptr: ptr,
            font_data_ptr: font_data_ptr
        };
    },
    destroy: function (fontatlas) {
        _fontatlas_destroy_JS(fontatlas.fontatlas_ptr);
        _free(fontatlas.font_data_ptr);

        fontatlas.fontatlas_ptr = fontatlas.font_data_ptr = undefined;
    },
    atlas_build: function (fontatlas, font_height, gaps, characters_to_add) {
        // copy the buffer into wasm memory
        let size = characters_to_add.indexOf(0x00);
        if (size < 0) throw new Error("Missing null-terminator on 'characters_to_add' parameter");

        size *= Uint32Array.BYTES_PER_ELEMENT;
        let characters_to_add_ptr = _malloc(size);

        if (characters_to_add_ptr == 0) throw new Error(`Failed to allocate ${size} bytes`);
        HEAPU8.set(new Uint8Array(characters_to_add.buffer, characters_to_add.byteOffset, size), characters_to_add_ptr);

        let ptr = _fontatlas_atlas_build(fontatlas.fontatlas_ptr, font_height, gaps, characters_to_add_ptr);
        _free(characters_to_add_ptr);

        if (ptr == 0) {
            return null;
        }

        return kdmyEngine_parseFontCharMap(ptr);
    },
    atlas_build_complete: function (fontatlas, font_height, gaps) {
        let ptr = _fontatlas_atlas_build_complete(fontatlas.fontatlas_ptr, font_height, gaps);
        return kdmyEngine_parseFontCharMap(ptr);
    },
    atlas_destroy_texture_only: function (fontcharmap) {
        _fontatlas_atlas_destroy_texture_only(fontcharmap.__ptr);
    },
    atlas_destroy: function (fontcharmap) {
        _fontatlas_atlas_destroy(fontcharmap.__ptr);
    },
    get_version: function () {
        let ptr = _fontatlas_get_version();
        return kdmyEngine_ptrToString(ptr);
    }
};
const PRINT_EXPORTS = {
    print: function (text) {
        console.log("%c[FontAtlas-stdout]%c", "font-weight: bold", "font-weight: normal", text);
    },
    printErr: function (text) {
        console.error("%c[FontAtlas-stderr]%c", "font-weight: bold", "font-weight: normal", text);
    }
};

Object.assign(Module, FONTATLAS_EXPORTS);
Object.assign(Module, PRINT_EXPORTS);