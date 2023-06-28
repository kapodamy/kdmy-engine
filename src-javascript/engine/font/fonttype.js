"use strict";

const SDF_FONT = true;
const SDF_SMOOTHNESS_BY_MATRIX_SCALE_DECOMPOSITION = false;
const SDF_MIPMAPS = false;

const FONTTYPE_POOL = new Map();
var FONTTYPE_IDS = 0;
const FONTTYPE_GLYPHS_HEIGHT = 72;// in the dreamcast use 64px
const FONTTYPE_GLYPHS_OUTLINE_RATIO = 0.086;// ~6px of outline @ 72px (used in SDF)
const FONTTYPE_GLYPHS_SMOOTHING_COEFF = 0.245;// used in SDF, idk how its works
const FONTTYPE_GLYPHS_GAPS = 4;// space between glyph in pixels
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
        lines_separation: 0,

        color: [0.0, 0.0, 0.0],
        alpha: 1.0,

        border_color: [1.0, 1.0, 1.0, 1.0],
        border_size: 0,
        border_enable: 0,
        border_offset_x: 0,
        border_offset_y: 0,

        instance_id: FONTTYPE_IDS++,
        instance_references: 1,
        instance_path: full_path,

        fontatlas: 0,
        font_ptr: 0,
        space_width: FONTTYPE_GLYPHS_HEIGHT * FONTTYPE_FAKE_SPACE,

        fontcharmap_primary: null,
        fontcharmap_primary_ptr: 0,
        fontcharmap_primary_texture: null,

        fontcharmap_secondary: null,
        fontcharmap_secondary_ptr: 0,
        fontcharmap_secondary_texture: 0
    };

    // initialize FreeType library
    if (await fonttype_internal_init_freetype(fonttype, src)) {
        full_path = undefined;
        fonttype = undefined;
        return null;
    }

    // create a texture atlas and glyphs map with all common letters, numbers and symbols
    fonttype.fontcharmap_primary_ptr = fonttype_internal_retrieve_fontcharmap(fonttype, null);
    fonttype.fontcharmap_primary = ModuleFontAtlas.kdmyEngine_parseFontCharMap(fonttype.fontcharmap_primary_ptr);
    fonttype.fontcharmap_primary_texture = fonttype_internal_upload_texture(fonttype.fontcharmap_primary);

    if (fonttype.fontcharmap_primary) {
        for (let i = 0; i < fonttype.fontcharmap_primary.char_array_size; i++) {
            if (fonttype.fontcharmap_primary.char_array[i].codepoint == FONTGLYPH_SPACE) {
                fonttype.space_width = fonttype.fontcharmap_primary.char_array[i].advancex;
                break;
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
        ModuleFontAtlas._fontatlas_atlas_destroy_JS(fonttype.fontcharmap_primary_ptr);
        texture_destroy(fonttype.fontcharmap_primary_texture);
    }

    if (fonttype.fontcharmap_secondary) {
        ModuleFontAtlas._fontatlas_atlas_destroy_JS(fonttype.fontcharmap_secondary_ptr);
        texture_destroy(fonttype.fontcharmap_secondary_texture);
    }

    if (fonttype.fontatlas) ModuleFontAtlas._fontatlas_destroy_JS(fonttype.fontatlas);
    if (fonttype.font_ptr) ModuleFontAtlas.kdmyEngine_deallocate(fonttype.font_ptr);

    fonttype.instance_path = undefined;
    fonttype = undefined;
}


function fonttype_measure(fonttype, height, text, text_index, text_size) {
    const scale = height / FONTTYPE_GLYPHS_HEIGHT;
    const text_end_index = text_index + text_size;
    const text_length = text.length;

    console.assert(text_end_index <= text_length, "invalid text_index/text_size (overflow)");

    // check for unmapped characters and them to the secondary map
    fonttype_internal_find_unmaped_codepoints_to_secondary(fonttype, text_index, text_end_index, text);

    let max_width = 0;
    let width = 0;
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
            width = 0;
            previous_codepoint = grapheme.code;
            line_chars = 0;
            continue;
        }

        let fontchardata = fonttype_internal_get_fontchardata(fonttype.fontcharmap_primary, grapheme.code);
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

function fonttype_meansure_char(fonttype, codepoint, height, lineinfo) {
    const scale = height / FONTTYPE_GLYPHS_HEIGHT;

    //override hard-spaces with white-spaces
    if (codepoint == 0xA0) codepoint = 0x20;

    let fontchardata = fonttype_internal_get_fontchardata(fonttype.fontcharmap_primary, codepoint);
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

function fonttype_set_lines_separation(fonttype, height) {
    fonttype.lines_separation = height;
}

function fonttype_set_color(fonttype, r, g, b) {
    fonttype.color[0] = r;
    fonttype.color[1] = g;
    fonttype.color[2] = b;
}

function fonttype_set_rgb8_color(fonttype, rbg8_color) {
    math2d_color_bytes_to_floats(rbg8_color, 0, fonttype.color);
}

function fonttype_set_alpha(fonttype, alpha) {
    fonttype.alpha = alpha;
}

function fonttype_set_border_size(fonttype, border_size) {
    //if (SDF_FONT) border_size /= 2;
    fonttype.border_size = border_size;
}

function fonttype_set_border_color(fonttype, r, g, b, a) {
    if (r >= 0) fonttype.border_color[0] = r;
    if (g >= 0) fonttype.border_color[1] = g;
    if (b >= 0) fonttype.border_color[2] = b;
    if (a >= 0) fonttype.border_color[3] = a;
}

function fonttype_set_border_color_rgba8(fonttype, rbga8_color) {
    math2d_color_bytes_to_floats(rbga8_color, 1, fonttype.border_color);
}

function fonttype_set_border_offset(fonttype, x, y) {
    fonttype.border_offset_x = x;
    fonttype.border_offset_y = y;
}

function fonttype_enable_border(fonttype, enable) {
    fonttype.border_enable = enable;
}

function fonttype_set_border(fonttype, enable, size, rgba) {
    //if (SDF_FONT) size /= 2;
    fonttype.border_enable = enable;
    fonttype.border_size = size;
    fonttype_set_border_color(fonttype, rgba[0], rgba[1], rgba[2], rgba[3]);
}

function fonttype_draw_text(fonttype, pvrctx, height, x, y, text_index, text_size, text) {
    if (text == null || text.length < 1) return 0;

    const grapheme = { code: 0, size: 0 };
    const primary = fonttype.fontcharmap_primary;
    const primary_texture = fonttype.fontcharmap_primary_texture;
    const secondary = fonttype.fontcharmap_secondary;
    const secondary_texture = fonttype.fontcharmap_secondary_texture;
    const has_border = fonttype.border_enable && fonttype.border_color[3] > 0 && fonttype.border_size >= 0;
    const outline_size = fonttype.border_size * 2;
    const scale = height / FONTTYPE_GLYPHS_HEIGHT;
    const ascender = ((primary ?? secondary).ascender / 2.0) * scale;// FIXME: ¿why does dividing by 2 works?
    const text_end_index = text_index + text_size;
    const text_length = text.length;
    let border_padding1 = 0.0, border_padding2 = 0.0;

    console.assert(text_end_index <= text_length, "invalid text_index/text_size (overflow)");

    if (SDF_FONT) {
        // calculate sdf thickness
        if (has_border) {
            if (fonttype.border_size > 0.0) {
                let max_border_size = height * FONTTYPE_GLYPHS_OUTLINE_RATIO;
                let border_size = Math.min(fonttype.border_size, max_border_size);
                let thickness = (1.0 - (border_size / max_border_size)) / 2.0;
                glyphrenderer_set_sdf_thickness(pvrctx, thickness);

                /*if (border_size < fonttype.border_size && height > 8.0) {
                    // add some padding
                    border_padding1 = fonttype.border_size - border_size;
                    border_padding2 = border_padding1 * 2.0;
                }*/
            } else {
                glyphrenderer_set_sdf_thickness(pvrctx, -1.0);
            }
        }
    }

    let draw_x = 0;
    let draw_y = 0 - ascender;
    let line_chars = 0;

    let index = text_index;
    let previous_codepoint = 0x0000;
    let total_glyphs = 0;
    /**@type {FontCharData}*/let fontchardata;

    pvr_context_save(pvrctx);
    pvr_context_set_vertex_alpha(pvrctx, fonttype.alpha);
    pvr_context_set_vertex_offsetcolor(pvrctx, PVR_DEFAULT_OFFSET_COLOR);
    pvr_context_set_vertex_antialiasing(pvrctx, PVR_FLAG_DEFAULT);


    // count the amount of glyph required
    while (index < text_end_index && string_get_character_codepoint(text, index, grapheme)) {
        index += grapheme.size;

        if (grapheme.code == 0xA0) continue;

        if (fontchardata = fonttype_internal_get_fontchardata(primary, grapheme.code)) {
            if (primary_texture && fontchardata.has_entry) total_glyphs++;
            continue;
        }
        if (fontchardata = fonttype_internal_get_fontchardata(secondary, grapheme.code)) {
            if (secondary_texture && fontchardata.has_entry) total_glyphs++;
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
            draw_x = 0;
            draw_y += height + fonttype.lines_separation - ascender;
            previous_codepoint = grapheme.code;
            line_chars = 0;
            continue;
        }

        fontchardata = fonttype_internal_get_fontchardata(primary, grapheme.code);
        let is_secondary = 0;
        let texture = primary_texture;

        if (!fontchardata) {
            fontchardata = fonttype_internal_get_fontchardata(secondary, grapheme.code);
            is_secondary = 1;
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

        if (texture && fontchardata.has_entry) {
            // compute draw location and size
            let dx = x + draw_x + (fontchardata.offset_x * scale);
            let dy = y + draw_y + (fontchardata.offset_y * scale);
            let dw = fontchardata.width * scale;
            let dh = fontchardata.height * scale;

            if (has_border) {
                let sdx, sdy, sdw, sdh;
                if (SDF_FONT) {
                    sdx = dx - border_padding1;
                    sdy = dy - border_padding1;
                    sdw = dw + border_padding2;
                    sdh = dh + border_padding2;
                } else {
                    // compute border location and outline size
                    sdx = dx - fonttype.border_size;
                    sdy = dy - fonttype.border_size;
                    sdw = dw + outline_size;
                    sdh = dh + outline_size;
                }

                sdx += fonttype.border_offset_x;
                sdy += fonttype.border_offset_y;

                // queue outlined glyph for batch rendering
                glyphrenderer_append_glyph(
                    texture, is_secondary, 1,
                    fontchardata.atlas_entry.x, fontchardata.atlas_entry.y, fontchardata.width, fontchardata.height,
                    sdx, sdy, sdw, sdh
                );
                added++;
            }

            // queue glyph for batch rendering
            glyphrenderer_append_glyph(
                texture, is_secondary, 0,
                fontchardata.atlas_entry.x, fontchardata.atlas_entry.y, fontchardata.width, fontchardata.height,
                dx, dy, dw, dh
            );
            added++;
        }

        draw_x += fontchardata.advancex * scale;
        line_chars++;
    }

    if (SDF_FONT) {
        let smoothing = fonttype_internal_calc_smoothing(pvrctx, height);
        glyphrenderer_set_sdf_smoothing(pvrctx, smoothing);
    }

    // commit draw
    glyphrenderer_draw(pvrctx, fonttype.color, fonttype.border_color, 0, 1, primary_texture, secondary_texture);

    pvr_context_restore(pvrctx);
    return draw_y + height;
}


async function fonttype_internal_init_freetype(fonttype, src) {
    //if (fonttype.font_ptr) throw new Error("The font is already initialized");

    let font = await fs_readarraybuffer(src);
    if (!font) return 0;

    if (SDF_FONT) ModuleFontAtlas._fontatlas_enable_sdf(1);

    // Important: keep the font data allocated, required for FreeType library
    fonttype.font_ptr = ModuleFontAtlas.kdmyEngine_allocate(font);
    fonttype.fontatlas = ModuleFontAtlas._fontatlas_init(fonttype.font_ptr, font.byteLength);

    return !fonttype.fontatlas;
}

function fonttype_internal_retrieve_fontcharmap(fonttype, characters_map) {
    let fontcharmap_ptr = 0;

    if (characters_map) {
        let characters_map_ptr = ModuleFontAtlas.kdmyEngine_allocate(characters_map);
        fontcharmap_ptr = ModuleFontAtlas._fontatlas_atlas_build(
            fonttype.fontatlas, FONTTYPE_GLYPHS_HEIGHT, FONTTYPE_GLYPHS_GAPS, characters_map_ptr
        );
        ModuleFontAtlas.kdmyEngine_deallocate(characters_map_ptr);
    } else {
        fontcharmap_ptr = ModuleFontAtlas._fontatlas_atlas_build_complete(
            fonttype.fontatlas, FONTTYPE_GLYPHS_HEIGHT, FONTTYPE_GLYPHS_GAPS
        );
    }

    return fontcharmap_ptr;
}

function fonttype_internal_upload_texture(fontcharmap) {
    if (!fontcharmap) return null;

    // C only
    /*
    Texture texture = texture_init_from_raw(
        fontcharmap.texture, fontcharmap->texture_byte_size, false,
        fontcharmap->texture_width, fontcharmap->texture_height,
        fontcharmap->texture_width, fontcharmap->texture_height
    );

    fontcharmap->texture = null;
    fontcharmap->texture_byte_size = 0;
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

function fonttype_internal_find_unmaped_codepoints_to_secondary(fonttype, text_index, text_end_index, text) {
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

        if (fonttype_internal_get_fontchardata(fonttype.fontcharmap_primary, grapheme.code)) continue;
        if (fonttype_internal_get_fontchardata(fonttype.fontcharmap_secondary, grapheme.code)) continue;

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

        if (fonttype_internal_get_fontchardata(fonttype.fontcharmap_primary, grapheme.code)) continue;
        if (fonttype_internal_get_fontchardata(fonttype.fontcharmap_secondary, grapheme.code)) continue;

        codepoints[new_codepoints++] = grapheme.code;
    }

    // step 3: rebuild the secondary char map
    if (fonttype.fontcharmap_secondary) {
        // dispose previous instance
        ModuleFontAtlas._fontatlas_atlas_destroy_JS(fonttype.fontcharmap_secondary_ptr);
        fonttype.fontcharmap_secondary.char_array = undefined;
        fonttype.fontcharmap_secondary = undefined;
        texture_destroy(fonttype.fontcharmap_secondary_texture);
    }

    // build map and upload texture
    fonttype.fontcharmap_secondary_ptr = fonttype_internal_retrieve_fontcharmap(fonttype, codepoints);
    fonttype.fontcharmap_secondary = ModuleFontAtlas.kdmyEngine_parseFontCharMap(fonttype.fontcharmap_secondary_ptr);
    fonttype.fontcharmap_secondary_texture = fonttype_internal_upload_texture(fonttype.fontcharmap_secondary);

    // dispose secondary codepoints array
    codepoints = undefined;
}

function fonttype_internal_get_fontchardata(/**@type {FontCharMap}*/ fontcharmap, codepoint) {
    if (fontcharmap) {
        for (let i = 0; i < fontcharmap.char_array_size; i++) {
            if (codepoint == fontcharmap.char_array[i].codepoint) return fontcharmap.char_array[i];
        }
    }
    return null;
}

function fonttype_internal_calc_smoothing(pvrctx, height) {
    const matrix = pvrctx.current_matrix;

    if (SDF_SMOOTHNESS_BY_MATRIX_SCALE_DECOMPOSITION) {
        let x = matrix[15] * Math.sqrt(
            (matrix[0] * matrix[0]) +
            (matrix[1] * matrix[1]) +
            (matrix[2] * matrix[2])
        );
        let y = matrix[15] * Math.sqrt(
            (matrix[4] * matrix[4]) +
            (matrix[5] * matrix[5]) +
            (matrix[6] * matrix[6])
        );

        let scale = (Math.abs((x + y) / 2.0) * height) / FONTTYPE_GLYPHS_HEIGHT;
        let smoothing = FONTTYPE_GLYPHS_SMOOTHING_COEFF / (FONTTYPE_GLYPHS_HEIGHT * scale);

        return smoothing;
    } else {
        let coord = [1.0, 1.0];
        sh4matrix_multiply_point(matrix, coord);

        let scale = (Math.abs((coord[0] + coord[1]) / 2.0) * height) / FONTTYPE_GLYPHS_HEIGHT;
        let smoothness = FONTTYPE_GLYPHS_SMOOTHING_COEFF / (FONTTYPE_GLYPHS_HEIGHT * scale);

        return smoothness;
    }
}

