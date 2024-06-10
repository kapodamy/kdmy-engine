"use strict";
const FONTGLYPH_SPACE = 0x20;
const FONTGLYPH_HARDSPACE = 0xA0;
const FONTGLYPH_LINEFEED = 0x0A;
const FONTGLYPH_TAB = 0x09;
const FONTGLYPH_CARRIAGERETURN = 0x0D;
const FONTGLYPH_SPACE_WIDTH_RATIO = 0.8;
const FONTGLYPH_TABSTOP = 8;


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
        table: new Array(atlas.size),
        table_size: atlas.size,// temporal value
        paragraph_separation: 0.0,
        tintcolor: [1.0, 1.0, 1.0],
        alpha: 1.0,

        color_by_addition: false,

        border_tintcolor: [0.0, 0.0, 0.0, 0.0],
        border_size: 0.0,
        border_enable: false,
        border_offset_x: 0.0,
        border_offset_y: 0.0,
        frame_time: 0.0,
        frame_progress: 0.0,
    };

    if (allow_animation) {
        if (atlas.glyph_fps > 0.0)
            fontglyph.frame_time = atlas.glyph_fps;
        else
            fontglyph.frame_time = FUNKIN_DEFAULT_ANIMATIONS_FRAMERATE;

        fontglyph.frame_time = 1000.0 / fontglyph.frame_time;
    }

    pvr_context_helper_clear_offsetcolor(fontglyph.border_tintcolor);

    let table_index = 0;

    // calculate the amount of required frames
    // needs C implementation
    for (let i = 0; i < atlas.size; i++) {
        fontglyph.table[i] = { actual_frame: 0, code: 0x00, frames: null, frames_size: 0 };
        let result = fontglyph_internal_parse(
            atlas.entries[i], suffix, allow_animation, fontglyph.table, table_index
        );

        if (result == 1) table_index++;
    }

    // shrink the table if necessary
    if (table_index < fontglyph.table_size) {
        fontglyph.table_size = table_index;
        fontglyph.table = realloc(fontglyph.table, table_index);
    }

    // allocate frames array
    // needs C implementation
    for (let i = 0; i < fontglyph.table_size; i++) {
        let glyph_info = fontglyph.table[i];
        if (glyph_info.frames_size > 0) {
            glyph_info.frames = new Array(glyph_info.frames_size);
            glyph_info.frames_size = 0;
        }
    }

    // add glyph frames
    // needs C implementation
    table_index = 0;
    for (let i = 0; i < atlas.size; i++) {
        let result = fontglyph_internal_parse(
            atlas.entries[i], suffix, allow_animation, fontglyph.table, table_index
        );

        if (result == 1) table_index++;
    }

    return fontglyph;
}

function fontglyph_destroy(fontglyph) {
    if (!fontglyph) return;

    //fontglyph.frames_array = undefined;
    fontglyph.table = undefined;

    texture_destroy(fontglyph.texture);
}


function fontglyph_measure(fontglyph, height, text, text_index, text_length) {
    const grapheme = { code: 0, size: 0 };
    const text_end_index = text_index + text_length;

    console.assert(text_end_index <= text_length, "invalid text_index/text_length (overflow)");

    let width = 0.0;
    let max_width = 0.0;
    let line_chars = 0;
    let space_width = fontglyph_internal_find_space_width(fontglyph, height);

    for (let i = text_index; i < text_end_index; i++) {
        if (!string_get_character_codepoint(text, i, grapheme)) continue;
        i += grapheme.size - 1;

        if (grapheme.code == FONTGLYPH_LINEFEED) {
            if (width > max_width) max_width = width;
            width = 0.0;
            line_chars = 0;
            continue;
        }

        let found = false;

        for (let j = 0; j < fontglyph.table_size; j++) {
            if (fontglyph.table[j].code == grapheme.code) {
                let frame = fontglyph.table[j].frames[fontglyph.table[j].actual_frame];
                width += frame.glyph_width_ratio * height;
                line_chars++;
                found = true;
                break;
            }
        }

        if (found) continue;

        if (grapheme.code == FONTGLYPH_TAB) {
            let filler = fontglyph_internal_calc_tabstop(line_chars);
            if (filler > 0) {
                width += space_width * filler;
                line_chars += filler;
            }
        } else {
            // space, hard space or unknown character
            width += height * FONTGLYPH_SPACE_WIDTH_RATIO;
            line_chars++;
        }
    }

    return Math.max(width, max_width);
}

function fontglyph_measure_char(fontglyph, codepoint, height, lineinfo) {
    if (lineinfo.space_width < 0.0) {
        lineinfo.space_width = fontglyph_internal_find_space_width(fontglyph, height);
    }

    for (let j = 0; j < fontglyph.table_size; j++) {
        if (fontglyph.table[j].code == codepoint) {
            let frame = fontglyph.table[j].frames[fontglyph.table[j].actual_frame];
            lineinfo.last_char_width = frame.glyph_width_ratio * height;
            lineinfo.line_char_count++;
            return;
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
}

function fontglyph_set_lines_separation(fontglyph, height) {
    fontglyph.paragraph_separation = height;
}

function fontglyph_set_color(fontglyph, r, g, b) {
    fontglyph.tintcolor[0] = r;
    fontglyph.tintcolor[1] = g;
    fontglyph.tintcolor[2] = b;
}

function fontglyph_set_rgb8_color(fontglyph, rbg8_color) {
    math2d_color_bytes_to_floats(rbg8_color, false, fontglyph.tintcolor);
}

function fontglyph_set_alpha(fontglyph, alpha) {
    fontglyph.alpha = alpha;
}

function fontglyph_set_border_color_rgba8(fontglyph, rbga8_color) {
    math2d_color_bytes_to_floats(rbga8_color, true, fontglyph.border_tintcolor);
}

function fontglyph_set_border_color(fontglyph, r, g, b, a) {
    if (r >= 0.0) fontglyph.border_tintcolor[0] = r;
    if (g >= 0.0) fontglyph.border_tintcolor[1] = g;
    if (b >= 0.0) fontglyph.border_tintcolor[2] = b;
    if (a >= 0.0) fontglyph.border_tintcolor[3] = a;
}

function fontglyph_set_border_size(fontglyph, size) {
    fontglyph.border_size = size;
}

function fontglyph_set_border_offset(fontglyph, x, y) {
    fontglyph.border_offset_x = x;
    fontglyph.border_offset_y = y;
}

function fontglyph_enable_border(fontglyph, enable) {
    fontglyph.border_enable = enable;
}

function fontglyph_set_border(fontglyph, enable, size, rgba) {
    fontglyph.border_enable = enable;
    fontglyph.border_size = size;
    fontglyph_set_border_color(fontglyph, rgba[0], rgba[1], rgba[2], rgba[3]);
}

function fontglyph_enable_color_by_addition(fontglyph, enable) {
    fontglyph.color_by_addition = !!enable;
}


function fontglyph_draw_text(fontglyph, pvrctx, height, x, y, text_index, text_length, text) {
    const grapheme = { code: 0, size: 0 };
    const by_add = fontglyph.color_by_addition;
    const has_border = fontglyph.border_enable && fontglyph.border_tintcolor[3] > 0.0 && fontglyph.border_size >= 0.0;
    const outline_size = fontglyph.border_size * 2.0;
    const text_end_index = text_index + text_length;

    console.assert(text_end_index <= text_length, "invalid text_index/text_length (overflow)");

    let draw_x = 0.0;
    let draw_y = 0.0;
    let index = text_index;
    let total_glyphs = 0;
    let line_chars = 0;

    // get space glyph width (if present)
    let space_width = fontglyph_internal_find_space_width(fontglyph, height);

    texture_upload_to_pvr(fontglyph.texture);
    pvr_context_save(pvrctx);
    pvr_context_set_vertex_alpha(pvrctx, fontglyph.alpha);

    // count required glyphs
    while (index < text_end_index && string_get_character_codepoint(text, index, grapheme)) {
        index += grapheme.size;

        switch (grapheme.code) {
            case FONTGLYPH_CARRIAGERETURN:
            case FONTGLYPH_LINEFEED:
                continue;
        }

        for (let i = 0; i < fontglyph.table_size; i++) {
            if (fontglyph.table[i].code == grapheme.code) {
                switch (grapheme.code) {
                    case FONTGLYPH_CARRIAGERETURN:
                    case FONTGLYPH_LINEFEED:
                        continue;
                }
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
            draw_y += height + fontglyph.paragraph_separation;
            draw_x = 0.0;
            line_chars = 0;
            continue;
        }

        let frame = null;
        for (let i = 0; i < fontglyph.table_size; i++) {
            if (fontglyph.table[i].code == grapheme.code) {
                frame = fontglyph.table[i].frames[fontglyph.table[i].actual_frame];
                break;
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
                draw_x += height * FONTGLYPH_SPACE_WIDTH_RATIO;
                line_chars++;
            }
            continue;
        }

        // calculate glyph bounds
        let ratio_width, ratio_height;
        let dx = x + draw_x;
        let dy = y + draw_y;
        let dh = height;
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
            let sdx = dx - fontglyph.border_size;
            let sdy = dy - fontglyph.border_size;
            let sdw = dw + outline_size;
            let sdh = dh + outline_size;

            sdx += fontglyph.border_offset_x;
            sdy += fontglyph.border_offset_y;

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

    glyphrenderer_draw(pvrctx, fontglyph.tintcolor, fontglyph.border_tintcolor, by_add, false, fontglyph.texture, null);

    pvr_context_restore(pvrctx);
    return draw_y + height;
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


function fontglyph_internal_parse(atlas_entry, match_suffix, allow_animation, table, table_index) {
    let atlas_entry_name = atlas_entry.name;
    let atlas_entry_name_length = atlas_entry_name.length;
    let grapheme = { code: 0, size: 0 };

    // read character info
    if (!string_get_character_codepoint(atlas_entry_name, 0, grapheme)) return -1;

    let index = grapheme.size;

    if (match_suffix) {
        let match_suffix_length = match_suffix.length;
        let number_suffix_start = index + match_suffix_length + 1;

        if (number_suffix_start > atlas_entry_name_length) return 0;// suffix not present

        switch (atlas_entry_name.codePointAt(index)) {
            //case FONTGLYPH_HARDSPACE:
            case FONTGLYPH_SPACE:
                index++;
                break;
            default:
                return 0;// suffix not present
        }

        // check if the suffix matchs
        if (!atlas_entry_name.startsWith(match_suffix, index)) return 0;

        index += match_suffix_length;
    }

    // check if this atlas entry is an animation frame
    if (index < atlas_entry_name_length) {
        if (!atlas_name_has_number_suffix(atlas_entry_name, index)) return 0;// suffix not present

        // check if already exists an entry with this unicode code point
        let code_index = -1;
        for (let i = 0; i < table_index; i++) {
            if (table[i].code == grapheme.code) {
                code_index = i;
                break;
            }
        }

        if (code_index >= 0) {
            // reject, animation is disabled
            if (!allow_animation || table[code_index].frames_size > 0) return 0;
            // add another frame
            fontglyph_internal_add_frame(atlas_entry, table[code_index]);
            return 2;
        }
    }

    // create entry for this unicode code point
    // needs C implementation instead of "new Array()"
    table[table_index] = {
        code: grapheme.code, actual_frame: 0, frames: null, frames_size: 0
    };

    fontglyph_internal_add_frame(atlas_entry, table[table_index]);

    return 1;
}

function fontglyph_internal_add_frame(atlas_entry, glyph_info) {
    // needs C implementation
    if (!glyph_info.frames) {
        glyph_info.frames_size++;
        return;
    }

    let frame = glyph_info.frames[glyph_info.frames_size] = {
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

        glyph_width_ratio: 0.0
    };
    glyph_info.frames_size++;

    let height = atlas_entry.frame_height > 0.0 ? atlas_entry.frame_height : atlas_entry.height;

    if (height > 0.0) {
        // cache this frame width
        let width = atlas_entry.frame_width > 0.0 ? atlas_entry.frame_width : atlas_entry.width;
        frame.glyph_width_ratio = width / height;
    }

    return frame;
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
    return height * FONTGLYPH_SPACE_WIDTH_RATIO;;
}

