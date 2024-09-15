"use strict";
const FONTGLYPH_SPACE = 0x20;
const FONTGLYPH_HARDSPACE = 0xA0;
const FONTGLYPH_LINEFEED = 0x0A;
const FONTGLYPH_TAB = 0x09;
const FONTGLYPH_CARRIAGERETURN = 0x0D;
const FONTGLYPH_SPACE_WIDTH_RATIO = 0.9;
const FONTGLYPH_TABSTOP = 8;
const FONTGLYPH_LOOKUP_TABLE_LENGTH = 128; // this covers all ascii characters


async function fontglyph_init(src_atlas, suffix, allow_animation) {
    let atlas = await atlas_init(src_atlas);
    if (!atlas || !atlas.texture_filename) {
        if (atlas) atlas_destroy(atlas);
        console.error("fontglyph_init() missing atlas file or texture filename not specified on " + src_atlas);
        return null;
    }

    let texture_path = atlas_get_texture_path(atlas);
    let fontglyph;

    let texture = await texture_init(texture_path);
    if (texture) {
        fontglyph = fontglyph_init2(texture, atlas, suffix, allow_animation);
        if (!fontglyph) console.warn(`fontglyph_init() failed for ${src_atlas}`);
    } else {
        fontglyph = null;
        console.error("fontglyph_init() texture specified by atlas not found: " + texture_path);
    }

    atlas_destroy(atlas);

    return fontglyph;
}

function fontglyph_init2(texture, atlas, suffix, allow_animation) {
    let fontglyph = {
        texture: texture,
        table: malloc_for_array(atlas.size),
        table_size: atlas.size,// temporal value

        frame_time: 0.0,
        frame_progress: 0.0,
        lookup_table: new Array(FONTGLYPH_LOOKUP_TABLE_LENGTH)
    };

    fontglyph.lookup_table.fill(FONTGLYPH_LOOKUP_TABLE_LENGTH, 0, FONTGLYPH_LOOKUP_TABLE_LENGTH);

    if (allow_animation) {
        if (atlas.glyph_fps > 0.0)
            fontglyph.frame_time = atlas.glyph_fps;
        else
            fontglyph.frame_time = FUNKIN_DEFAULT_ANIMATIONS_FRAMERATE;

        fontglyph.frame_time = 1000.0 / fontglyph.frame_time;
    }

    let table_index = 0;

    // calculate the amount of matching glyphs in the atlas
    for (let i = 0; i < atlas.size; i++) {
        let result = fontglyph_internal_parse(
            atlas.entries[i], suffix, allow_animation, fontglyph.table, table_index
        );
        if (result) table_index++;
    }

    if (table_index < 1) {
        console.warn(`fontglyph_init2() failed, there no usable glyphs in the atlas suffix=${suffix}`);
        fontglyph.table = undefined;
        fontglyph = undefined;
        return null;
    }

    // shrink the table if necessary
    if (table_index < fontglyph.table_size) {
        fontglyph.table_size = table_index;
        fontglyph.table = realloc_for_array(fontglyph.table, table_index);
    }

    // count frames of every added glpyh
    let frame_total = 0;
    for (let i = 0; i < fontglyph.table_size; i++) {
        frame_total += fontglyph.table[i].frames_size;
    }

    // allocate frames array and set to zero each glyph frame count
    // C only
    //fontglyph->frames_array = malloc_for_array(GlyphFrame, frame_total);
    for (let i = 0, j = 0; i < fontglyph.table_size; i++) {
        let frames_size = fontglyph.table[i].frames_size;
        if (frames_size > 0) {
            // C only
            //fontglyph->table[i].frames = &fontglyph->frames_array[j];
            // JS & C# only
            fontglyph.table[i].frames = malloc_for_array(frames_size);
            fontglyph.table[i].frames_size = 0;
            j += frames_size;
        }
    }

    // add frames to each glyph
    for (let i = 0; i < atlas.size; i++) {
        fontglyph_internal_parse(
            atlas.entries[i], suffix, allow_animation, fontglyph.table, table_index
        );
    }

    // sort table, place ascii characters first
    qsort(fontglyph.table, fontglyph.table_size, 16/*sizeof(GlyphInfo)*/, fontglyph_internal_table_sort);

    // populate lookup table
    for (let i = 0; i < fontglyph.table_size && i <= FONTGLYPH_LOOKUP_TABLE_LENGTH; i++) {
        let code = fontglyph.table[i].code;
        if (code < FONTGLYPH_LOOKUP_TABLE_LENGTH) {
            fontglyph.lookup_table[code] = i;
        }
    }

    return fontglyph;
}

function fontglyph_destroy(fontglyph) {
    if (!fontglyph) return;

    //fontglyph.frames_array = undefined;
    fontglyph.table = undefined;

    texture_destroy(fontglyph.texture);
}


function fontglyph_measure(fontglyph, params, text, text_index, text_length) {
    const grapheme = { code: 0, size: 0 };
    const text_end_index = text_index + text_length;

    let width = 0.0;
    let max_width = 0.0;
    let line_chars = 0;
    let space_width = fontglyph_internal_find_space_width(fontglyph, params.height);

    for (let i = text_index; i < text_end_index; i++) {
        if (!string_get_character_codepoint(text, i, grapheme)) continue;
        i += grapheme.size - 1;

        if (grapheme.code == FONTGLYPH_LINEFEED) {
            if (width > max_width) max_width = width;
            width = 0.0;
            line_chars = 0;
            continue;
        }

        let info = null;

        L_find_glyph_info: {
            if (grapheme.code < FONTGLYPH_LOOKUP_TABLE_LENGTH) {
                if (fontglyph.lookup_table[grapheme.code] != FONTGLYPH_LOOKUP_TABLE_LENGTH) {
                    let index = fontglyph.lookup_table[grapheme.code];
                    info = fontglyph.table[index];
                    break L_find_glyph_info;
                }
            }

            for (let j = 0; j < fontglyph.table_size; j++) {
                if (fontglyph.table[j].code == grapheme.code) {
                    info = fontglyph.table[j];
                    break L_find_glyph_info;
                }
            }

            if (grapheme.code == FONTGLYPH_TAB) {
                let filler = fontglyph_internal_calc_tabstop(line_chars);
                if (filler > 0) {
                    width += space_width * filler;
                    line_chars += filler;
                }
            } else {
                // space, hard space or unknown character
                width += params.height * FONTGLYPH_SPACE_WIDTH_RATIO;
                line_chars++;
            }

            continue;
        }

        let frame = info.frames[info.actual_frame];
        width += frame.glyph_width_ratio * params.height;
        line_chars++;
    }

    return Math.max(width, max_width);
}

function fontglyph_measure_char(fontglyph, codepoint, height, lineinfo) {
    if (lineinfo.space_width < 0.0) {
        lineinfo.space_width = fontglyph_internal_find_space_width(fontglyph, height);
    }

    lineinfo.last_char_height = height;

    let info = null;

    L_find_glyph_info: {
        if (codepoint < FONTGLYPH_LOOKUP_TABLE_LENGTH) {
            if (fontglyph.lookup_table[codepoint] != FONTGLYPH_LOOKUP_TABLE_LENGTH) {
                let index = fontglyph.lookup_table[codepoint];
                info = fontglyph.table[index];
                break L_find_glyph_info;
            }
        }

        for (let i = 0; i < fontglyph.table_size; i++) {
            if (fontglyph.table[i].code == codepoint) {
                info = fontglyph.table[i];
                break L_find_glyph_info;
            }
        }

        if (codepoint == FONTGLYPH_TAB) {
            let filler = fontglyph_internal_calc_tabstop(lineinfo.line_char_count);
            if (filler > 0) {
                lineinfo.last_char_width = lineinfo.space_width * filler;
                lineinfo.line_char_count += filler;
            }
        } else {
            // space, hard space or unknown character
            lineinfo.last_char_width = height * FONTGLYPH_SPACE_WIDTH_RATIO;
            lineinfo.line_char_count++;
        }

        return;
    }

    let frame = info.frames[info.actual_frame];
    lineinfo.last_char_width = frame.glyph_width_ratio * height;
    lineinfo.line_char_count++;
}

function fontglyph_draw_text(fontglyph, pvrctx, params, x, y, text_index, text_length, text) {
    const grapheme = { code: 0, size: 0 };
    const by_add = params.color_by_addition;
    const has_border = params.border_enable && params.border_color[3] > 0.0 && params.border_size >= 0.0;
    const outline_size = params.border_size * 2.0;
    const text_end_index = text_index + text_length;

    let draw_x = 0.0;
    let draw_y = 0.0;
    let index = text_index;
    let total_glyphs = 0;
    let line_chars = 0;

    // get space glyph width (if present)
    let space_width = fontglyph_internal_find_space_width(fontglyph, params.height);

    texture_upload_to_pvr(fontglyph.texture);
    pvr_context_save(pvrctx);
    pvr_context_set_vertex_alpha(pvrctx, params.tint_color[3]);

    // count required glyphs
    while (index < text_end_index && string_get_character_codepoint(text, index, grapheme)) {
        index += grapheme.size;

        switch (grapheme.code) {
            case FONTGLYPH_CARRIAGERETURN:
            case FONTGLYPH_LINEFEED:
                continue;
        }

        if (grapheme.code < FONTGLYPH_LOOKUP_TABLE_LENGTH && fontglyph.lookup_table[grapheme.code] != FONTGLYPH_LOOKUP_TABLE_LENGTH) {
            total_glyphs++;
            continue;
        }

        for (let i = 0; i < fontglyph.table_size; i++) {
            if (fontglyph.table[i].code == grapheme.code) {
                total_glyphs++;
                break;
            }
        }
    }

    // prepare vertex buffer
    if (has_border) total_glyphs *= 2;
    let added = 0;
    let maximum = glyphrenderer_prepare(total_glyphs, has_border);

    index = text_index;
    while (added < maximum && index < text_end_index && string_get_character_codepoint(text, index, grapheme)) {
        index += grapheme.size;

        // ignore "\r" characters
        if (grapheme.code == FONTGLYPH_CARRIAGERETURN) continue;

        if (grapheme.code == FONTGLYPH_LINEFEED) {
            draw_y += params.height + params.paragraph_space;
            draw_x = 0.0;
            line_chars = 0;
            continue;
        }

        let frame = null;

        if (grapheme.code < FONTGLYPH_LOOKUP_TABLE_LENGTH && fontglyph.lookup_table[grapheme.code] != FONTGLYPH_LOOKUP_TABLE_LENGTH) {
            let in_table_index = fontglyph.lookup_table[grapheme.code];
            frame = fontglyph.table[in_table_index].frames[fontglyph.table[in_table_index].actual_frame];
        } else {
            for (let i = 0; i < fontglyph.table_size; i++) {
                if (fontglyph.table[i].code == grapheme.code) {
                    frame = fontglyph.table[i].frames[fontglyph.table[i].actual_frame];
                    break;
                }
            }
        }

        if (!frame) {
            if (grapheme.code == FONTGLYPH_TAB) {
                let filler = fontglyph_internal_calc_tabstop(line_chars);
                if (filler > 0) {
                    draw_x += space_width * filler;
                    line_chars += filler;
                }
            } else {
                // space, hard space or unknown characters
                draw_x += params.height * FONTGLYPH_SPACE_WIDTH_RATIO;
                line_chars++;
            }
            continue;
        }

        // calculate glyph bounds
        let ratio_width, ratio_height;
        let dx = x + draw_x;
        let dy = y + draw_y;
        let dh = params.height;
        let dw = dh * frame.glyph_width_ratio;

        if (frame.frame_width > 0.0) {
            ratio_width = dw / frame.frame_width;
            dw = frame.width * ratio_width;
        } else {
            ratio_width = dw / frame.width;
        }

        if (frame.frame_height > 0.0) {
            ratio_height = dh / frame.frame_height;
            dh = frame.height * ratio_height;
        } else {
            ratio_height = dh / frame.height;
        }

        dx += (frame.pivot_x + frame.frame_x) * ratio_width;
        dy += (frame.pivot_y + frame.frame_y) * ratio_height;

        if (has_border) {
            let sdx = dx - params.border_size;
            let sdy = dy - params.border_size;
            let sdw = dw + outline_size;
            let sdh = dh + outline_size;

            sdx += params.border_offset_x;
            sdy += params.border_offset_y;

            glyphrenderer_append_glyph(
                fontglyph.texture, false, true,
                frame.x, frame.y, frame.width, frame.height,
                sdx, sdy, sdw, sdh
            );
        }

        glyphrenderer_append_glyph(
            fontglyph.texture, false, false,
            frame.x, frame.y, frame.width, frame.height,
            dx, dy, dw, dh
        );

        draw_x += dw;
        line_chars++;
    }

    glyphrenderer_draw(
        pvrctx, params.tint_color, params.border_color, by_add, false,
        fontglyph.texture, null, null, null
    );

    pvr_context_restore(pvrctx);
    return draw_y + params.height;
}

function fontglyph_animate(fontglyph, elapsed) {
    if (fontglyph.frame_time <= 0.0) return 1;

    let frame_index = fontglyph.frame_progress / fontglyph.frame_time;

    for (let i = 0; i < fontglyph.table_size; i++) {
        if (fontglyph.table[i].frames_size < 2) continue;
        fontglyph.table[i].actual_frame = Math.trunc(frame_index % fontglyph.table[i].frames_size);
    }

    fontglyph.frame_progress += elapsed;
    return 0;
}

function fontglyph_map_codepoints(fontglyph, text, text_index, text_length) {
    void fontglyph;
    void text_index;
    void text_length;
    void text;
    // unused
}


function fontglyph_internal_parse(atlas_entry, match_suffix, allow_animation, table, table_index) {
    let atlas_entry_name = atlas_entry.name;
    let atlas_entry_name_length = atlas_entry_name.length;
    let grapheme = { code: 0, size: 0 };

    // read character info
    if (!string_get_character_codepoint(atlas_entry_name, 0, grapheme)) {
        // eof reached
        return false;
    }

    let index = grapheme.size;

    if (match_suffix) {
        let match_suffix_length = match_suffix.length;
        let number_suffix_start = index + match_suffix_length + 1;

        if (number_suffix_start > atlas_entry_name_length) {
            // suffix not present
            return false;
        }

        switch (atlas_entry_name.codePointAt(index)) {
            //case FONTGLYPH_HARDSPACE:
            case FONTGLYPH_SPACE:
                index++;
                break;
            default:
                // suffix not present
                return 0;
        }

        // check if the suffix matchs
        if (!atlas_entry_name.startsWith(match_suffix, index)) return 0;

        index += match_suffix_length;
    }

    // check if this atlas entry is an animation frame
    if (index < atlas_entry_name_length && !atlas_name_has_number_suffix(atlas_entry_name, index)) {
        // missing number suffix
        return false;
    }

    // check if already exists an entry with this unicode code point
    let codepoint_index = -1;
    for (let i = 0; i < table_index; i++) {
        if (table[i].code == grapheme.code) {
            if (!allow_animation && table[i].frames_size > 0) {
                // glyph animation is disabled, do not add more frames
                return false;
            }
            codepoint_index = i;
            break;
        }
    }

    if (codepoint_index < 0) {
        // create entry for this unicode code point
        table[table_index] = {
            code: grapheme.code, actual_frame: 0, frames: null, frames_size: 0
        };
    } else {
        table_index = codepoint_index;
    }

    fontglyph_internal_add_frame(atlas_entry, table[table_index]);
    table[table_index].frames_size++;

    // returns true if an entry was added to the table
    return codepoint_index < 0;
}

function fontglyph_internal_add_frame(atlas_entry, glyph_info) {
    if (!glyph_info.frames) return;

    let height = atlas_entry.frame_height > 0.0 ? atlas_entry.frame_height : atlas_entry.height;
    let glyph_width_ratio = 0.0;
    if (height > 0.0) {
        // cache this frame width
        let width = atlas_entry.frame_width > 0.0 ? atlas_entry.frame_width : atlas_entry.width;
        glyph_width_ratio = width / height;
    }

    glyph_info.frames[glyph_info.frames_size] = {
        x: atlas_entry.x,
        y: atlas_entry.y,
        width: atlas_entry.width,
        height: atlas_entry.height,
        frame_x: atlas_entry.frame_x,
        frame_y: atlas_entry.frame_y,
        frame_width: atlas_entry.frame_width,
        frame_height: atlas_entry.frame_height,
        pivot_x: atlas_entry.pivot_x,
        pivot_y: atlas_entry.pivot_y,

        glyph_width_ratio: glyph_width_ratio
    };
}

function fontglyph_internal_calc_tabstop(characters_in_the_line) {
    let space = characters_in_the_line % FONTGLYPH_TABSTOP;
    if (space == 0) return 0;
    return FONTGLYPH_TABSTOP - space;
}

function fontglyph_internal_find_space_width(fontglyph, height) {
    for (let i = 0; i < fontglyph.table_size; i++) {
        if (fontglyph.table[i].code == FONTGLYPH_SPACE || fontglyph.table[i].code == FONTGLYPH_HARDSPACE) {
            let frame = fontglyph.table[i].frames[fontglyph.table[i].actual_frame];
            return height * frame.glyph_width_ratio;
        }
    }
    return height * FONTGLYPH_SPACE_WIDTH_RATIO;
}

function fontglyph_internal_table_sort(x, y) {
    let value_x = x.code;
    let value_y = y.code;

    if (value_x < value_y)
        return -1;
    if (value_x > value_y)
        return 1;
    else
        return 0;
}

