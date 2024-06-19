"use strict";

const SDF_FONT = true;
const SDF_SMOOTHNESS_BY_MATRIX_SCALE_DECOMPOSITION = false;
const SDF_MIPMAPS = false;

const FONTTYPE_POOL = new Map();
var FONTTYPE_IDS = 0;
const FONTTYPE_GLYPHS_HEIGHT = 72;// in the dreamcast use 32px, 64px is enough for SDF
const FONTTYPE_GLYPHS_SDF_SIZE = FONTTYPE_GLYPHS_HEIGHT >> 2;// 25% of FONTTYPE_GLYPHS_HEIGHT
const FONTTYPE_GLYPHS_GAPS = 16;// space between glyph in pixels (must be high for SDF)
const FONTTYPE_FAKE_SPACE = 0.75;// 75% of the height


async function fonttype_init(src) {
    let full_path = await fs_get_full_path_and_override(src);

    for (let [id, instance] of FONTTYPE_POOL) {
        if (instance.instance_path == full_path) {
            full_path = undefined;
            instance.instance_references++;
            return instance;
        }
    }

    let fonttype = {
        instance_id: FONTTYPE_IDS++,
        instance_references: 1,
        instance_path: full_path,

        fontatlas: null,
        font: null,
        space_width: FONTTYPE_GLYPHS_HEIGHT * FONTTYPE_FAKE_SPACE,

        fontcharmap_primary: null,
        fontcharmap_primary_texture: null,

        fontcharmap_secondary: null,
        fontcharmap_secondary_texture: null,
        lookup_table: new Array(FONTGLYPH_LOOKUP_TABLE_LENGTH)
    };

    fonttype.lookup_table.fill(FONTGLYPH_LOOKUP_TABLE_LENGTH, 0, FONTGLYPH_LOOKUP_TABLE_LENGTH);

    // initialize FreeType library
    if (await fonttype_internal_init_freetype(fonttype, src)) {
        full_path = undefined;
        fonttype.font = undefined;
        fonttype = undefined;
        return null;
    }

    // create a texture atlas and glyphs map with all common letters, numbers and symbols
    fonttype.fontcharmap_primary = fonttype_internal_retrieve_fontcharmap(fonttype, null);
    fonttype.fontcharmap_primary_texture = fonttype_internal_upload_texture(fonttype.fontcharmap_primary);

    if (fonttype.fontcharmap_primary) {
        let char_array = fonttype.fontcharmap_primary.char_array;
        let char_array_size = fonttype.fontcharmap_primary.char_array_size;

        for (let i = 0; i < char_array_size; i++) {
            if (char_array[i].codepoint == FONTGLYPH_SPACE) {
                fonttype.space_width = char_array[i].advancex;
                break;
            }
        }

        for (let i = 0; i < char_array_size && i <= FONTGLYPH_LOOKUP_TABLE_LENGTH; i++) {
            let codepoint = char_array[i].codepoint;
            if (codepoint < FONTGLYPH_LOOKUP_TABLE_LENGTH) {
                fonttype.lookup_table[codepoint] = i;
            }
        }
    }

    FONTTYPE_POOL.set(fonttype.instance_id, fonttype);
    return fonttype;
}

function fonttype_destroy(fonttype) {
    if (!fonttype) return;

    fonttype.instance_references--;
    if (fonttype.instance_references > 0) return;
    FONTTYPE_POOL.delete(fonttype.instance_id);

    if (fonttype.fontcharmap_primary) {
        fontatlas_atlas_destroy(fonttype.fontcharmap_primary);
        texture_destroy(fonttype.fontcharmap_primary_texture);
    }

    if (fonttype.fontcharmap_secondary) {
        fontatlas_atlas_destroy(fonttype.fontcharmap_secondary);
        texture_destroy(fonttype.fontcharmap_secondary_texture);
    }

    if (fonttype.fontatlas) fontatlas_destroy(fonttype.fontatlas);

    fonttype.font = undefined;
    fonttype.instance_path = undefined;
    fonttype = undefined;
}


function fonttype_measure(fonttype, params, text, text_index, text_length) {
    const scale = params.height / FONTTYPE_GLYPHS_HEIGHT;
    const text_end_index = text_index + text_length;

    //console.assert(text_end_index <= text_length, "invalid text_index/text_length (overflow)");

    let max_width = 0.0;
    let width = 0.0;
    let index = text_index;
    let previous_codepoint = 0;
    let line_chars = 0;
    const grapheme = { code: 0, size: 0 };

    while (index < text_end_index && string_get_character_codepoint(text, index, grapheme)) {
        index += grapheme.size;

        //override hard-spaces with white-spaces
        if (grapheme.code == 0xA0) grapheme.code = 0x20;

        // ignore "\r" characters
        if (grapheme.code == FONTGLYPH_CARRIAGERETURN) {
            previous_codepoint = grapheme.code;
            continue;
        }

        if (grapheme.code == FONTGLYPH_LINEFEED) {
            if (width > max_width) max_width = width;
            width = 0.0;
            previous_codepoint = grapheme.code;
            line_chars = 0;
            continue;
        }

        let fontchardata = fonttype_internal_get_fontchardata2(fonttype.lookup_table, fonttype.fontcharmap_primary, grapheme.code);
        if (!fontchardata) {
            fontchardata = fonttype_internal_get_fontchardata(fonttype.fontcharmap_secondary, grapheme.code);
            if (!fontchardata) {
                if (grapheme.code == FONTGLYPH_TAB) {
                    let filler = fontglyph_internal_calc_tabstop(line_chars);
                    if (filler > 0) {
                        width += fonttype.space_width * filler * scale;
                        line_chars += filler;
                    }
                } else {
                    // space, hard space or unknown characters
                    width += fonttype.space_width * scale;
                    line_chars++;
                }
            }

            continue;
        }

        if (previous_codepoint) {
            // compute kerning
            for (let i = 0; i < fontchardata.kernings_size; i++) {
                if (fontchardata.kernings[i].codepoint == previous_codepoint) {
                    width += fontchardata.kernings[i].x * scale;
                    break;
                }
            }
        }

        width += fontchardata.advancex * scale;
        previous_codepoint = grapheme.code;
        line_chars++;
    }

    return Math.max(width, max_width);
}

function fonttype_measure_char(fonttype, codepoint, height, lineinfo) {
    const scale = height / FONTTYPE_GLYPHS_HEIGHT;

    //override hard-spaces with white-spaces
    if (codepoint == 0xA0) codepoint = 0x20;

    let fontchardata = fonttype_internal_get_fontchardata2(fonttype.lookup_table, fonttype.fontcharmap_primary, codepoint);
    if (!fontchardata) {
        fontchardata = fonttype_internal_get_fontchardata(fonttype.fontcharmap_secondary, codepoint);
        if (!fontchardata) {
            if (codepoint == FONTGLYPH_TAB) {
                let filler = fontglyph_internal_calc_tabstop(lineinfo.line_char_count);
                if (filler > 0) {
                    lineinfo.last_char_width = fonttype.space_width * filler * scale;
                    lineinfo.line_char_count += filler;
                }
            } else {
                // space, hard space or unknown characters
                lineinfo.last_char_width = fonttype.space_width * scale;
                lineinfo.line_char_count++;
            }
        }
        return;
    }

    if (lineinfo.previous_codepoint) {
        // compute kerning
        for (let i = 0; i < fontchardata.kernings_size; i++) {
            if (fontchardata.kernings[i].codepoint == lineinfo.previous_codepoint) {
                lineinfo.last_char_width = fontchardata.kernings[i].x * scale;
                break;
            }
        }
    }

    lineinfo.last_char_width = fontchardata.advancex * scale;
    lineinfo.previous_codepoint = codepoint;
    lineinfo.line_char_count++;
}

function fonttype_draw_text(fonttype, pvrctx, params, x, y, text_index, text_length, text) {
    if (text == null || text_length < 1) return 0.0;

    const grapheme = { code: 0, size: 0 };
    const primary = fonttype.fontcharmap_primary;
    const primary_texture = fonttype.fontcharmap_primary_texture;
    const secondary = fonttype.fontcharmap_secondary;
    const secondary_texture = fonttype.fontcharmap_secondary_texture;
    const has_border = params.border_enable && params.border_color[3] > 0.0 && params.border_size >= 0.0;
    const outline_size = params.border_size * 2.0;
    const scale = params.height / FONTTYPE_GLYPHS_HEIGHT;
    const ascender = ((primary ?? secondary).ascender / 2.0) * scale;// FIXME: ¿why does dividing by 2 works?
    const text_end_index = text_index + text_length;

    //console.assert(text_end_index <= text_length, "invalid text_index/text_length (overflow)");

    if (SDF_FONT) {
        if (has_border) {
            // calculate sdf padding
            let padding;
            padding = params.border_size / FONTTYPE_GLYPHS_SDF_SIZE;
            padding /= params.height / FONTTYPE_GLYPHS_HEIGHT;

            glyphrenderer_set_params_sdf(pvrctx, FONTTYPE_GLYPHS_SDF_SIZE, padding);
        } else {
            glyphrenderer_set_params_sdf(pvrctx, FONTTYPE_GLYPHS_SDF_SIZE, -1.0);
        }
    }

    let draw_x = 0.0;
    let draw_y = 0.0 - ascender;
    let line_chars = 0;

    let index = text_index;
    let previous_codepoint = 0x0000;
    let total_glyphs = 0;
    /**@type {FontCharData}*/let fontchardata;

    pvr_context_save(pvrctx);
    pvr_context_set_vertex_alpha(pvrctx, params.tint_color[3]);
    pvr_context_set_vertex_offsetcolor(pvrctx, PVRCTX_DEFAULT_OFFSET_COLOR);
    pvr_context_set_vertex_antialiasing(pvrctx, PVRCTX_FLAG_DEFAULT);


    // count the amount of glyph required
    while (index < text_end_index && string_get_character_codepoint(text, index, grapheme)) {
        index += grapheme.size;

        if (grapheme.code == 0xA0) continue;

        if (fontchardata = fonttype_internal_get_fontchardata2(fonttype.lookup_table, primary, grapheme.code)) {
            if (fontchardata.has_entry) total_glyphs++;
            continue;
        }
        if (fontchardata = fonttype_internal_get_fontchardata(secondary, grapheme.code)) {
            if (fontchardata.has_entry) total_glyphs++;
            continue;
        }
    }

    // prepare vertex buffer
    if (has_border) total_glyphs *= 2;
    let added = 0;
    let maximum = glyphrenderer_prepare(total_glyphs, has_border);

    // add glyphs to the vertex buffer
    index = text_index;
    while (added < maximum && index < text_end_index && string_get_character_codepoint(text, index, grapheme)) {
        index += grapheme.size;

        //override hard-spaces with white-spaces
        if (grapheme.code == 0xA0) grapheme.code = 0x20;

        // ignore "\r" characters
        if (grapheme.code == FONTGLYPH_CARRIAGERETURN) {
            previous_codepoint = grapheme.code;
            continue;
        }

        if (grapheme.code == FONTGLYPH_LINEFEED) {
            draw_x = 0.0;
            draw_y += params.height + fonttype.paragraph_space - ascender;
            previous_codepoint = grapheme.code;
            line_chars = 0;
            continue;
        }

        fontchardata = fonttype_internal_get_fontchardata2(fonttype.lookup_table, primary, grapheme.code);
        let is_secondary = false;
        let texture = primary_texture;

        if (!fontchardata) {
            fontchardata = fonttype_internal_get_fontchardata(secondary, grapheme.code);
            is_secondary = true;
            texture = secondary_texture;
        }

        if (!fontchardata) {
            // codepoint not mapped or fonttype_measure() was not called previously to map it
            if (grapheme.code == FONTGLYPH_TAB) {
                let filler = fontglyph_internal_calc_tabstop(line_chars);
                if (filler > 0) {
                    draw_x += fonttype.space_width * filler * scale;
                    line_chars += filler;
                }
            } else {
                // space, hard space or unknown characters
                draw_x += fonttype.space_width * scale;
                line_chars++;
            }

            previous_codepoint = grapheme.code;
            continue;
        }

        // apply kerking before continue
        if (previous_codepoint) {
            for (let i = 0; i < fontchardata.kernings_size; i++) {
                if (fontchardata.kernings[i].codepoint == previous_codepoint) {
                    draw_x += fontchardata.kernings[i].x * scale;
                    break;
                }
            }
        }

        if (fontchardata.has_entry) {
            // compute draw location and size
            let dx = x + draw_x + (fontchardata.offset_x * scale);
            let dy = y + draw_y + (fontchardata.offset_y * scale);
            let dw = fontchardata.width * scale;
            let dh = fontchardata.height * scale;

            if (has_border) {
                let sdx, sdy, sdw, sdh;
                if (SDF_FONT) {
                    sdx = dx;
                    sdy = dy;
                    sdw = dw;
                    sdh = dh;
                } else {
                    // compute border location and outline size
                    sdx = dx - params.border_size;
                    sdy = dy - params.border_size;
                    sdw = dw + outline_size;
                    sdh = dh + outline_size;
                }

                sdx += params.border_offset_x;
                sdy += params.border_offset_y;

                // queue outlined glyph for batch rendering
                glyphrenderer_append_glyph(
                    texture, is_secondary, true,
                    fontchardata.atlas_entry.x, fontchardata.atlas_entry.y, fontchardata.width, fontchardata.height,
                    sdx, sdy, sdw, sdh
                );
                added++;
            }

            // queue glyph for batch rendering
            glyphrenderer_append_glyph(
                texture, is_secondary, false,
                fontchardata.atlas_entry.x, fontchardata.atlas_entry.y, fontchardata.width, fontchardata.height,
                dx, dy, dw, dh
            );
            added++;
        }

        draw_x += fontchardata.advancex * scale;
        line_chars++;
    }

    // commit draw
    glyphrenderer_draw(pvrctx, params.tint_color, params.border_color, 0, 1, primary_texture, secondary_texture);

    pvr_context_restore(pvrctx);
    return draw_y + params.height;
}

function fonttype_map_codepoints(fonttype, text_index, text_end_index, text) {
    let actual = fonttype.fontcharmap_secondary ? fonttype.fontcharmap_secondary.char_array_size : 0;
    let new_codepoints = 0;
    let grapheme = { code: 0, size: 0 };
    let index = text_index;

    // step 1: count all unmapped codepoints
    while (index < text_end_index && string_get_character_codepoint(text, index, grapheme)) {
        index += grapheme.size;

        switch (grapheme.code) {
            case FONTGLYPH_LINEFEED:
            case FONTGLYPH_CARRIAGERETURN:
                continue;
        }

        if (fonttype_internal_get_fontchardata2(fonttype.lookup_table, fonttype.fontcharmap_primary, grapheme.code))
            continue;
        if (fonttype_internal_get_fontchardata(fonttype.fontcharmap_secondary, grapheme.code))
            continue;

        // not present, count it
        new_codepoints++;
    }

    if (new_codepoints < 1) return;// nothing to do

    // step 2: allocate codepoints array
    let codepoints = new Uint32Array(actual + new_codepoints + 1);
    codepoints[actual + new_codepoints] = 0x00000000;

    if (fonttype.fontcharmap_secondary) {
        // add existing secondary codepoints
        for (let i = 0; i < fonttype.fontcharmap_secondary.char_array_size; i++) {
            codepoints[i] = fonttype.fontcharmap_secondary.char_array[i].codepoint;
        }
    }

    index = text_index;
    new_codepoints = actual;
    while (index < text_end_index && string_get_character_codepoint(text, index, grapheme)) {
        index += grapheme.size;

        if (fonttype_internal_get_fontchardata2(fonttype.lookup_table, fonttype.fontcharmap_primary, grapheme.code))
            continue;
        if (fonttype_internal_get_fontchardata(fonttype.fontcharmap_secondary, grapheme.code))
            continue;

        codepoints[new_codepoints++] = grapheme.code;
    }

    // step 3: rebuild the secondary char map
    if (fonttype.fontcharmap_secondary) {
        // dispose previous instance
        fontatlas_atlas_destroy(fonttype.fontcharmap_secondary);
        fonttype.fontcharmap_secondary.char_array = fonttype.fontcharmap_secondary = null;
        texture_destroy(fonttype.fontcharmap_secondary_texture);
    }

    // build map and upload texture
    fonttype.fontcharmap_secondary = fonttype_internal_retrieve_fontcharmap(fonttype, codepoints);
    fonttype.fontcharmap_secondary_texture = fonttype_internal_upload_texture(fonttype.fontcharmap_secondary);

    // dispose secondary codepoints array
    codepoints = undefined;
}

function fonttype_animate(fonttype, elapsed) {
    void fonttype;
    void elapsed;

    // not used
    return 1;
}


async function fonttype_internal_init_freetype(fonttype, src) {
    //if (fonttype.font) throw new Error("The font is already initialized");

    let font = await fs_readarraybuffer(src);
    if (!font) return 0;

    if (SDF_FONT) fontatlas_enable_sdf(true);

    // Important: keep the font data allocated, required for FreeType library
    fonttype.font = font;
    fonttype.fontatlas = fontatlas_init(font, font.byteLength);

    return !fonttype.fontatlas;
}

function fonttype_internal_retrieve_fontcharmap(fonttype, characters_map) {
    let fontcharmap = null;

    if (characters_map) {
        fontcharmap = fontatlas_atlas_build(
            fonttype.fontatlas, FONTTYPE_GLYPHS_HEIGHT, FONTTYPE_GLYPHS_GAPS, characters_map
        );
    } else {
        fontcharmap = fontatlas_atlas_build_complete(
            fonttype.fontatlas, FONTTYPE_GLYPHS_HEIGHT, FONTTYPE_GLYPHS_GAPS
        );
    }

    return fontcharmap;
}

function fonttype_internal_upload_texture(fontcharmap) {
    if (!fontcharmap || !fontcharmap.texture) return null;

    // C only
    /*
    pvr_ptr_t vram_ptr = pvr_mem_malloc(fontcharmap->texture_byte_size);
    if (!vram_ptr) {
        // logger_error("fonttype_internal_upload_texture() not enough space in the vram for " FMT_I4 " bytes", fontcharmap->char_array_size);
        return NULL;
    }

    pvr_txr_load_ex(fontcharmap->texture, vram_ptr, fontcharmap->texture_width, fontcharmap->texture_height, PVR_TXRLOAD_8BPP);

    Texture texture = texture_init_from_raw(
        vram_ptr, fontcharmap->texture_byte_size, true,
        fontcharmap->texture_width, fontcharmap->texture_height,
        fontcharmap->texture_width, fontcharmap->texture_height
    );

    texture->format = PVR_TXRFMT_PAL8BPP | PVR_TXRFMT_TWIDDLED;

    // no longer needed
    fontatlas_atlas_destroy_texture_only(fontcharmap);
    return texture;
    */


    const gl = pvr_context.webopengl.gl;
    //const unpack_alignment = gl.getParameter(gl.UNPACK_ALIGNMENT);
    //gl.pixelStorei(gl.UNPACK_ALIGNMENT, 1);

    let texture = gl.createTexture();
    gl.bindTexture(gl.TEXTURE_2D, texture);

    if (SDF_MIPMAPS) {
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR_MIPMAP_NEAREST);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_NEAREST);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    } else {
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    }

    gl.texImage2D(
        gl.TEXTURE_2D, 0, gl.LUMINANCE,
        fontcharmap.texture_width, fontcharmap.texture_height, 0,
        gl.LUMINANCE, gl.UNSIGNED_BYTE, fontcharmap.texture
    );

    if (SDF_MIPMAPS) {
        gl.generateMipmap(gl.TEXTURE_2D);
    }

    //gl.pixelStorei(gl.UNPACK_ALIGNMENT, unpack_alignment);

    fontcharmap.texture = null;

    let tex = texture_init_from_raw(
        texture, fontcharmap.texture_byte_size, true,
        fontcharmap.texture_width, fontcharmap.texture_height,
        fontcharmap.texture_width, fontcharmap.texture_height
    );

    if (SDF_MIPMAPS) {
        tex.has_mipmaps = true;
    }

    return tex;
}

function fonttype_internal_get_fontchardata(/**@type {FontCharMap}*/ fontcharmap, codepoint) {
    if (fontcharmap) {
        for (let i = 0; i < fontcharmap.char_array_size; i++) {
            if (codepoint == fontcharmap.char_array[i].codepoint) return fontcharmap.char_array[i];
        }
    }
    return null;
}

function fonttype_internal_get_fontchardata2(lookup_table, fontcharmap, codepoint) {
    if (codepoint < FONTGLYPH_LOOKUP_TABLE_LENGTH) {
        let index = lookup_table[codepoint];
        if (index < FONTGLYPH_LOOKUP_TABLE_LENGTH) {
            return fontcharmap.char_array[index];
        }
    }
    return fonttype_internal_get_fontchardata(fontcharmap, codepoint);
}

