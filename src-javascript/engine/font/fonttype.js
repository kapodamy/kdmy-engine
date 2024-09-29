"use strict";

const SDF_FONT = true;
const SDF_MIPMAPS = false;

const FONTTYPE_GLYPHS_HEIGHT = SDF_FONT ? 72 : 42;// in the dreamcast use 42px, 72px is enough for SDF
const FONTTYPE_GLYPHS_SDF_SIZE = FONTTYPE_GLYPHS_HEIGHT >>> 2;// 25% of FONTTYPE_GLYPHS_HEIGHT
const FONTTYPE_GLYPHS_GAP = SDF_FONT ? 16 : 4;// space between glyph in pixels (must be high for SDF)
const FONTTYPE_GLYPHS_OUTLINE_HEIGHT = FONTTYPE_GLYPHS_HEIGHT >>> 1;
const FONTTYPE_GLYPHS_OUTLINE_GAP = FONTTYPE_GLYPHS_GAP >>> 1;
const FONTTYPE_GLYPHS_OUTLINE_RATIO = FONTTYPE_GLYPHS_OUTLINE_HEIGHT >>> 2;// 25% of FONTTYPE_GLYPHS_OUTLINE_HEIGHT
const FONTTYPE_GLYPHS_OUTLINE_THICKNESS = 1.25;// 125%
const FONTTYPE_FAKE_SPACE = 0.9;// 90% of the height


var FONTTYPE_POOL = new Map();
var FONTTYPE_IDS = 0;
var fonttype_fontatlas_mutex = MUTEX_INITIALIZER();


async function fonttype_init(src) {
    let full_path = await fs_get_full_path_and_override(src);

    for (let [id, instance] of FONTTYPE_POOL) {
        if (string_equals_ignore_case(instance.instance_path, full_path)) {
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

        atlas_primary: { map_outline: null, texture_outline: null, map: null, texture: null },
        atlas_secondary: { map_outline: null, texture_outline: null, map: null, texture: null },

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
    fonttype_internal_create_atlas(fonttype, fonttype.atlas_primary, null);

    if (fonttype.atlas_primary.map != null) {
        let char_array = fonttype.atlas_primary.map.char_array;
        let char_array_size = fonttype.atlas_primary.map.char_array_size;

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

    fonttype_internal_destroy_atlas(fonttype.atlas_primary);
    fonttype_internal_destroy_atlas(fonttype.atlas_secondary);

    if (fonttype.fontatlas) fontatlas_destroy(fonttype.fontatlas);

    fonttype.font = undefined;
    fonttype.instance_path = undefined;
    fonttype = undefined;
}


function fonttype_measure(fonttype, params, text, text_index, text_length, info) {
    const scale = params.height / FONTTYPE_GLYPHS_HEIGHT;
    const text_end_index = text_index + text_length;

    let max_width = 0.0;
    let width = 0.0;
    let index = text_index;
    let previous_codepoint = 0;
    let line_chars = 0;
    let lines = 1;
    let last_glyph_width_correction = 0;
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
            lines++;
            last_glyph_width_correction = 0;
            continue;
        }

        let fontchardata;
        let map_index = fonttype_internal_get_fontchardata2(fonttype.lookup_table, fonttype.atlas_primary, grapheme.code);

        if (map_index < 0) {
            map_index = fonttype_internal_get_fontchardata(fonttype.atlas_secondary, grapheme.code);
            if (map_index < 0) {
                if (grapheme.code == FONTGLYPH_TAB) {
                    let filler = fontglyph_internal_calc_tabstop(line_chars);
                    if (filler > 0) {
                        width += fonttype.space_width * filler;
                        line_chars += filler;
                    }
                } else {
                    // space, hard space or unknown characters
                    width += fonttype.space_width;
                    line_chars++;
                }
                last_glyph_width_correction = 0;
                continue;
            } else {
                fontchardata = fonttype.atlas_secondary.map.char_array[map_index];
            }
        } else {
            fontchardata = fonttype.atlas_primary.map.char_array[map_index];
        }

        let kerning_x = 0;
        if (previous_codepoint) {
            // compute kerning
            for (let i = 0; i < fontchardata.kernings_size; i++) {
                if (fontchardata.kernings[i].codepoint == previous_codepoint) {
                    kerning_x = fontchardata.kernings[i].x;
                    break;
                }
            }
        }

        last_glyph_width_correction = fontchardata.width + fontchardata.offset_x - fontchardata.advancex + kerning_x;
        width += fontchardata.advancex + kerning_x;
        previous_codepoint = grapheme.code;
        line_chars++;
    }

    let line_height = fonttype_measure_line_height(fonttype, params.height);
    info.max_width = (Math.max(width, max_width) + last_glyph_width_correction) * scale;
    info.total_height = (line_height + params.paragraph_space) * lines;
}

function fonttype_measure_char(fonttype, codepoint, height, info) {
    const scale = height / FONTTYPE_GLYPHS_HEIGHT;

    //override hard-spaces with white-spaces
    if (codepoint == 0xA0) codepoint = 0x20;

    let fontchardata;
    let map_index = fonttype_internal_get_fontchardata2(fonttype.lookup_table, fonttype.atlas_primary, codepoint);

    if (map_index < 0) {
        map_index = fonttype_internal_get_fontchardata(fonttype.atlas_secondary, codepoint);
        if (map_index < 0) {
            if (codepoint == FONTGLYPH_TAB) {
                let filler = fontglyph_internal_calc_tabstop(info.line_char_count);
                if (filler > 0) {
                    info.last_char_width = fonttype.space_width * filler;
                    info.line_char_count += filler;
                }
            } else {
                // space, hard space or unknown characters
                info.last_char_width = fonttype.space_width;
                info.line_char_count++;
            }
            info.last_char_width *= scale;
            info.last_char_height = height;
            info.last_char_width_end = 0.0;
            return;
        } else {
            fontchardata = fonttype.atlas_secondary.map.char_array[map_index];
        }
    } else {
        fontchardata = fonttype.atlas_primary.map.char_array[map_index];
    }

    let kerning_x = 0;
    if (info.previous_codepoint) {
        // compute kerning
        for (let i = 0; i < fontchardata.kernings_size; i++) {
            if (fontchardata.kernings[i].codepoint == info.previous_codepoint) {
                kerning_x = fontchardata.kernings[i].x;
                break;
            }
        }
    }

    info.last_char_width = (fontchardata.advancex + kerning_x) * scale;
    info.last_char_height = fontchardata.advancey * scale;
    info.last_char_width_end = (fontchardata.width + fontchardata.offset_x - fontchardata.advancex + kerning_x) * scale;
    info.previous_codepoint = codepoint;
    info.line_char_count++;
}

function fonttype_measure_line_height(fonttype, height) {
    let line_height = (fonttype.atlas_primary.map ?? fonttype.atlas_secondary.map).line_height;

    let scale = height / FONTTYPE_GLYPHS_HEIGHT;
    return line_height * scale;
}

function fonttype_draw_text(fonttype, pvrctx, params, x, y, text_index, text_length, text) {
    const grapheme = { code: 0, size: 0 };
    const has_border = params.border_enable && params.border_color[3] > 0.0 && params.border_size >= 0.0;
    const outline_size = params.border_size * 2.0;
    const scale_glyph = params.height / FONTTYPE_GLYPHS_HEIGHT;
    const scale_outline = params.height / FONTTYPE_GLYPHS_OUTLINE_HEIGHT;
    const text_end_index = text_index + text_length;
    const ascender = (fonttype.atlas_primary.map ?? fonttype.atlas_secondary.map).ascender;
    const line_height = (fonttype.atlas_primary.map ?? fonttype.atlas_secondary.map).line_height;

    if (SDF_FONT) {
        if (has_border) {
            // calculate sdf padding
            let padding;
            padding = params.border_size / FONTTYPE_GLYPHS_OUTLINE_RATIO;
            padding /= params.height / FONTTYPE_GLYPHS_OUTLINE_HEIGHT;

            glyphrenderer_set_params_sdf(pvrctx, FONTTYPE_GLYPHS_OUTLINE_RATIO, padding, FONTTYPE_GLYPHS_OUTLINE_THICKNESS);
        } else {
            glyphrenderer_set_params_sdf(pvrctx, FONTTYPE_GLYPHS_OUTLINE_RATIO, -1.0, -1.0);
        }
    }

    let draw_glyph_x = 0.0;
    let draw_glyph_y = ascender;
    let max_draw_y = 0.0;
    let line_chars = 0;
    let lines = 1;

    let index = text_index;
    let previous_codepoint = 0x0000;
    let total_glyphs = 0;
    let map_index;

    pvr_context_save(pvrctx);
    pvr_context_set_vertex_alpha(pvrctx, params.tint_color[3]);
    pvr_context_set_vertex_offsetcolor(pvrctx, PVRCTX_DEFAULT_OFFSET_COLOR);
    pvr_context_set_vertex_antialiasing(pvrctx, PVRCTX_FLAG_DEFAULT);

    // count the amount of glyph required
    while (index < text_end_index && string_get_character_codepoint(text, index, grapheme)) {
        index += grapheme.size;

        if (grapheme.code == 0xA0) continue;

        map_index = fonttype_internal_get_fontchardata2(fonttype.lookup_table, fonttype.atlas_primary, grapheme.code);
        if (map_index >= 0) {
            if (fonttype.atlas_primary.map.char_array[map_index].has_atlas_entry) total_glyphs++;
            continue;
        }
        map_index = fonttype_internal_get_fontchardata(fonttype.atlas_secondary, grapheme.code);
        if (map_index >= 0) {
            if (fonttype.atlas_secondary.map.char_array[map_index].has_atlas_entry) total_glyphs++;
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
            draw_glyph_x = 0.0;
            draw_glyph_y += line_height + (params.paragraph_space / scale_glyph);
            previous_codepoint = grapheme.code;
            line_chars = 0;
            lines++;
            continue;
        }

        let atlas = fonttype.atlas_primary;
        let is_secondary = false;
        map_index = fonttype_internal_get_fontchardata2(fonttype.lookup_table, atlas, grapheme.code);

        if (map_index < 0) {
            atlas = fonttype.atlas_secondary;
            is_secondary = true;
            map_index = fonttype_internal_get_fontchardata(atlas, grapheme.code);
        }

        if (map_index < 0) {
            // codepoint not mapped or fonttype_measure() was not called previously to map it
            if (grapheme.code == FONTGLYPH_TAB) {
                let filler = fontglyph_internal_calc_tabstop(line_chars);
                if (filler > 0) {
                    draw_glyph_x += fonttype.space_width * filler;
                    line_chars += filler;
                }
            } else {
                // space, hard space or unknown characters
                draw_glyph_x += fonttype.space_width;
                line_chars++;
            }

            previous_codepoint = grapheme.code;
            continue;
        }

        let fontchardata_glyph = atlas.map.char_array[map_index];

        // apply kerning before continue
        if (previous_codepoint != 0x0000) {
            for (let i = 0; i < fontchardata_glyph.kernings_size; i++) {
                if (fontchardata_glyph.kernings[i].codepoint == previous_codepoint) {
                    draw_glyph_x += fontchardata_glyph.kernings[i].x;
                    break;
                }
            }
        }

        if (fontchardata_glyph.has_atlas_entry) {
            // compute draw location and size
            let dx = x + ((draw_glyph_x + fontchardata_glyph.offset_x) * scale_glyph);
            let dy = y + ((draw_glyph_y + fontchardata_glyph.offset_y) * scale_glyph);
            let dw = fontchardata_glyph.width * scale_glyph;
            let dh = fontchardata_glyph.height * scale_glyph;

            if (has_border) {
                let sdx, sdy, sdw, sdh;
                let texture_outline;
                let fontchardata_outline;

                if (SDF_FONT) {
                    texture_outline = atlas.texture_outline;
                    fontchardata_outline = atlas.map_outline.char_array[map_index];

                    sdw = dw + ((fontchardata_outline.width * scale_outline) - dw);
                    sdh = dh + ((fontchardata_outline.height * scale_outline) - dh);
                    sdx = dx - ((sdw - dw) / 2.0);
                    sdy = dy - ((sdh - dh) / 2.0);
                } else {
                    texture_outline = atlas.texture;
                    fontchardata_outline = fontchardata_glyph;

                    // compute border location and outline size
                    sdw = dw + outline_size;
                    sdh = dh + outline_size;
                    sdx = dx - params.border_size;
                    sdy = dy - params.border_size;
                }

                sdx += params.border_offset_x;
                sdy += params.border_offset_y;

                // queue outlined glyph for batch rendering
                glyphrenderer_append_glyph(
                    texture_outline, is_secondary, true,
                    fontchardata_outline.atlas_entry.x, fontchardata_outline.atlas_entry.y,
                    fontchardata_outline.width, fontchardata_outline.height,
                    sdx, sdy, sdw, sdh
                );
                added++;
            }

            // queue glyph for batch rendering
            glyphrenderer_append_glyph(
                atlas.texture, is_secondary, false,
                fontchardata_glyph.atlas_entry.x, fontchardata_glyph.atlas_entry.y,
                fontchardata_glyph.width, fontchardata_glyph.height,
                dx, dy, dw, dh
            );
            added++;
        }

        draw_glyph_x += fontchardata_glyph.advancex;
        line_chars++;
    }

    // commit draw
    if (SDF_FONT && has_border) {
        glyphrenderer_draw(
            pvrctx,
            params.tint_color, params.border_color,
            false, true,
            fonttype.atlas_primary.texture, fonttype.atlas_secondary.texture,
            fonttype.atlas_primary.texture_outline, fonttype.atlas_secondary.texture_outline
        );
    } else {
        glyphrenderer_draw(
            pvrctx,
            params.tint_color, params.border_color,
            false, true,
            fonttype.atlas_primary.texture, fonttype.atlas_secondary.texture,
            null, null
        );
    }

    pvr_context_restore(pvrctx);
    return ((line_height * scale_glyph) + params.paragraph_space) * lines;
}

function fonttype_map_codepoints(fonttype, text, text_index, text_end_index) {
    const grapheme = { code: 0, size: 0 };

    let codepoints_length;
    let codepoints_used;
    let codepoints;
    let existing_count;

    if (fonttype.atlas_secondary.map) {
        codepoints_used = fonttype.atlas_secondary.map.char_array_size;
        codepoints_length = codepoints_used + 32;
        codepoints = malloc_for_array(codepoints_length);
        existing_count = codepoints_used;

        // add existing secondary codepoints
        for (let i = 0; i < fonttype.atlas_secondary.map.char_array_size; i++) {
            codepoints[i] = fonttype.atlas_secondary.map.char_array[i].codepoint;
        }
    } else {
        codepoints_used = 0;
        codepoints_length = 64;
        codepoints = malloc_for_array(codepoints_length);
        existing_count = 0;
    }

    // step 1: add all unmapped codepoints
    L_find_unmaped_codepoints:
    while (text_index < text_end_index && string_get_character_codepoint(text, text_index, grapheme)) {
        text_index += grapheme.size;

        switch (grapheme.code) {
            case FONTGLYPH_LINEFEED:
            case FONTGLYPH_CARRIAGERETURN:
                continue;
        }

        if (fonttype_internal_get_fontchardata2(fonttype.lookup_table, fonttype.atlas_primary, grapheme.code) >= 0)
            continue;
        if (fonttype_internal_get_fontchardata(fonttype.atlas_secondary, grapheme.code) >= 0)
            continue;

        // check if the codepoint is already added
        for (let i = 0; i < codepoints_used; i++) {
            if (codepoints[i] == grapheme.code) continue L_find_unmaped_codepoints;
        }

        codepoints[codepoints_used++] = grapheme.code;

        if ((codepoints_used + 1) >= codepoints_length) {
            codepoints_length += 16;
            realloc_for_array(codepoints, codepoints_length);
        }
    }

    if (codepoints_used <= existing_count) {
        // nothing to do
        codepoints = undefined;
        return;
    }

    codepoints[codepoints_used++] = 0x00000000;

    // step 3: rebuild the secondary char map
    fonttype_internal_destroy_atlas(fonttype.atlas_secondary);
    fonttype_internal_create_atlas(fonttype, fonttype.atlas_secondary, codepoints);
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
    if (!font) return true;

    if (SDF_FONT) {
        fontatlas_enable_sdf(true);
    }

    mutex_lock(fonttype_fontatlas_mutex);

    // Important: keep the font data allocated, required for FreeType library
    fonttype.font = font;
    fonttype.fontatlas = fontatlas_init(font, font.byteLength);

    mutex_unlock(fonttype_fontatlas_mutex);

    return !fonttype.fontatlas;
}

function fonttype_internal_create_fontcharmap(fonttype, characters_map, glyphs_height, glyphs_gap) {
    let fontcharmap = null;

    mutex_lock(fonttype_fontatlas_mutex);

    if (characters_map) {
        fontcharmap = fontatlas_atlas_build(
            fonttype.fontatlas, glyphs_height, glyphs_gap, characters_map
        );
    } else {
        fontcharmap = fontatlas_atlas_build_complete(
            fonttype.fontatlas, glyphs_height, glyphs_gap
        );
    }

    mutex_unlock(fonttype_fontatlas_mutex);

    return fontcharmap;
}

function fonttype_internal_upload_texture(fonttype, fontcharmap) {
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

    let texture = gl.createTexture();
    gl.activeTexture(gl.TEXTURE0);
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

    gl.bindTexture(gl.TEXTURE_2D, null);

    // FIXME: calling this function also deallocates the vram texture on desktop
    // now deallocate texture data
    //fontatlas_atlas_destroy_texture_only(fontcharmap);

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

function fonttype_internal_get_fontchardata(atlas, codepoint) {
    if (atlas.map != null) {
        for (let i = 0; i < atlas.map.char_array_size; i++) {
            if (codepoint == atlas.map.char_array[i].codepoint) return i;
        }
    }
    return -1;
}

function fonttype_internal_get_fontchardata2(lookup_table, atlas, codepoint) {
    if (codepoint < FONTGLYPH_LOOKUP_TABLE_LENGTH) {
        let index = lookup_table[codepoint];
        if (index < FONTGLYPH_LOOKUP_TABLE_LENGTH) {
            return index;
        }
    }
    return fonttype_internal_get_fontchardata(atlas, codepoint);
}

function fonttype_internal_create_atlas(fonttype, atlas, codepoints) {
    let fontcharmap = fonttype_internal_create_fontcharmap(fonttype, codepoints, FONTTYPE_GLYPHS_HEIGHT, FONTTYPE_GLYPHS_GAP);
    let texture = fonttype_internal_upload_texture(fonttype, fontcharmap);

    if (SDF_FONT) {
        if (fontcharmap != null) {
            let fontcharmap_outline = fonttype_internal_create_fontcharmap(fonttype, codepoints, FONTTYPE_GLYPHS_OUTLINE_HEIGHT, FONTTYPE_GLYPHS_OUTLINE_GAP);
            let texture_outline = fonttype_internal_upload_texture(fonttype, fontcharmap_outline);

            if (fontcharmap_outline == null) throw new Error("fontcharmap_outline was null");

            if (fontcharmap.char_array_size != fontcharmap_outline.char_array_size) {
                throw new Error("char_array_size fontcharmap != fontcharmap_outline");
            }
            for (let i = 0; i < fontcharmap.char_array_size; i++) {
                let entry = fontcharmap.char_array[i];
                let entry_outline = fontcharmap_outline.char_array[i];

                if (entry.has_atlas_entry != entry_outline.has_atlas_entry) {
                    throw new Error("entry.has_atlas_entry != entry_outline.has_atlas_entry");
                }
            }

            atlas.map_outline = fontcharmap_outline;
            atlas.texture_outline = texture_outline;
        }
    }
    atlas.map = fontcharmap;
    atlas.texture = texture;
}

function fonttype_internal_destroy_atlas(atlas) {
    if (SDF_FONT) {
        if (atlas.map_outline != null) {
            fontatlas_atlas_destroy(atlas.map_outline);
            atlas.map_outline = null;
        }
        if (atlas.texture_outline != null) {
            texture_destroy(atlas.texture_outline);
            atlas.texture_outline = null;
        }
    }
    if (atlas.map != null) {
        fontatlas_atlas_destroy(atlas.map);
        atlas.map = null;
    }
    if (atlas.texture != null) {
        texture_destroy(atlas.texture);
        atlas.texture = null;
    }
}

