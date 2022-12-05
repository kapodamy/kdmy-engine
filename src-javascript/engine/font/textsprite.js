"use strict";

const TEXTSPRITE_FORCE_NONE = 0;
const TEXTSPRITE_FORCE_LOWERCASE = 1;
const TEXTSPRITE_FORCE_UPPERCASE = 2;


const TEXTSPRITE_POOL = new Map();
var TEXTSPRITE_IDS = 0;


function textsprite_init(font, font_is_truetype, size, rbg8_color) {
    let textsprite = {
        font,
        font_from_atlas: font_is_truetype,
        font_size: size,
        color: [1, 1, 1],

        text: null,
        intern: 0,

        force_case: 0,
        last_force_case: 0,
        text_forced_case: null,

        visible: 1,
        x: 0,
        y: 0,
        z: 0,
        alpha: 1.0,
        z_offset: 0,

        align_vertical: ALIGN_START,
        align_horizontal: ALIGN_START,
        max_width: -1,
        max_height: -1,

        max_lines: -1,
        paragraph_align: ALIGN_START,

        matrix_source: {},
        matrix_corner: { x: 0, y: 0, angle: 0 },
        flip_x: 0,
        flip_y: 0,

        paragraph_array: arraylist_init(),

        modified_coords: 0,
        modified_string: 0,

        last_draw_x: 0,
        last_draw_y: 0,

        last_draw_width: 0,
        last_draw_height: 0,

        animation_external: null,
        animation_selected: null,
        animation_list: linkedlist_init(),

        border_enable: 0,
        border_size: 0,
        border_color: [1.0, 1.0, 1.0, 1.0],

        font_paragraph_separation: 0,

        antialiasing: PVR_FLAG_DEFAULT,

        wordbreak: FONT_WORDBREAK_LOOSE,

        psshader: null,

        blend_enabled: 1,
        blend_src_rgb: BLEND_DEFAULT,
        blend_dst_rgb: BLEND_DEFAULT,
        blend_src_alpha: BLEND_DEFAULT,
        blend_dst_alpha: BLEND_DEFAULT,

        id: TEXTSPRITE_IDS++
    };

    math2d_color_bytes_to_floats(rbg8_color, 0, textsprite.color);

    pvrctx_helper_clear_modifier(textsprite.matrix_source);

    TEXTSPRITE_POOL.set(textsprite.id, textsprite);
    return textsprite;
}

function textsprite_init2(fontholder, font_size, rbg8_color) {
    return textsprite_init(fontholder.font, fontholder.font_from_atlas, font_size, rbg8_color);
}

function textsprite_destroy(textsprite) {
    textsprite.font = null;// set to null only

    if (!textsprite.intern) textsprite.text = undefined;

    if (textsprite.text_forced_case) textsprite.text_forced_case = undefined;

    arraylist_destroy(textsprite.paragraph_array, 0);

    linkedlist_destroy(textsprite.animation_list);

    TEXTSPRITE_POOL.delete(textsprite.id);
    ModuleLuaScript.kdmyEngine_drop_shared_object(textsprite.matrix_source);
    ModuleLuaScript.kdmyEngine_drop_shared_object(textsprite);
    textsprite = undefined;
}


function textsprite_set_text(textsprite, text) {
    textsprite_set_text_intern(textsprite, 0, text);
}

function textsprite_set_text_intern(textsprite, intern, text) {
    if (!textsprite.intern) textsprite.text = undefined;

    textsprite.intern = intern;
    if (intern)
        textsprite.text = text;
    else
        textsprite.text = strdup(text);

    textsprite.modified_string = 1;
    textsprite.modified_coords = 1;
}

function textsprite_set_text_formated(textsprite, format, ...values) {
    let text = stringbuilder_helper_create_formatted_string(format, values);

    if (!textsprite.intern) textsprite.text = undefined;

    textsprite.intern = 0;
    textsprite.text = text;

    textsprite.modified_string = 1;
    textsprite.modified_coords = 1;
}

function textsprite_set_text_formated2(textsprite, format, va_args) {
    let text = stringbuilder_helper_create_formatted_string(format, va_args);
    textsprite_set_text_intern(textsprite, 0, text);
}

function textsprite_set_font_size(textsprite, font_size) {
    textsprite.modified_string = 1;
    textsprite.font_size = font_size;
}

function textsprite_force_case(textsprite, none_or_lowercase_or_uppercase) {
    textsprite.modified_string = 1;
    textsprite.force_case = none_or_lowercase_or_uppercase;
}

function textsprite_set_paragraph_align(textsprite, align) {
    textsprite.modified_coords = 1;
    textsprite.paragraph_align = align;
}

function textsprite_set_paragraph_space(textsprite, space) {
    textsprite.font_paragraph_separation = space;
}

function textsprite_set_maxlines(textsprite, max_lines) {
    textsprite.modified_string = 1;
    textsprite.max_lines = max_lines;
}

function textsprite_set_color_rgba8(textsprite, rbg8_color) {
    math2d_color_bytes_to_floats(rbg8_color, 0, textsprite.color);
}

function textsprite_set_color(textsprite, r, g, b) {
    if (r >= 0) textsprite.color[0] = r;
    if (g >= 0) textsprite.color[1] = g;
    if (b >= 0) textsprite.color[2] = b;
}


function textsprite_set_alpha(textsprite, alpha) {
    textsprite.alpha = alpha;
}

function textsprite_set_visible(textsprite, visible) {
    textsprite.visible = visible;
}

function textsprite_set_draw_location(textsprite, x, y) {
    textsprite.modified_coords = 1;
    if (x != null) textsprite.x = x;
    if (y != null) textsprite.y = y;
}

function textsprite_get_draw_location(textsprite, output_location) {
    output_location[0] = textsprite.x;
    output_location[1] = textsprite.y;
    return output_location;
}

function textsprite_set_z_index(textsprite, z_index) {
    textsprite.z = z_index;
}

function textsprite_set_z_offset(textsprite, offset) {
    textsprite.z_offset = offset;
}

function textsprite_get_z_index(textsprite) {
    return textsprite.z + textsprite.z_offset;
}

function textsprite_set_max_draw_size(textsprite, max_width, max_height) {
    textsprite.modified_coords = 1;
    if (Number.isFinite(max_width)) textsprite.max_width = max_width;
    if (Number.isFinite(max_height)) textsprite.max_height = max_height;
}

function textsprite_matrix_flip(textsprite, flip_x, flip_y) {
    textsprite.modified_coords = 1;
    if (flip_x != null) textsprite.flip_x = flip_x;
    if (flip_y != null) textsprite.flip_y = flip_y;
}


function textsprite_set_align(textsprite, align_vertical, align_horizontal) {
    textsprite.modified_coords = 1;
    if (align_vertical != ALIGN_NONE) {
        if (align_vertical == ALIGN_BOTH) align_vertical = ALIGN_START;
        textsprite.align_vertical = align_vertical;
    }
    if (align_horizontal != ALIGN_NONE) {
        if (align_horizontal == ALIGN_BOTH) align_horizontal = ALIGN_START;
        textsprite.align_horizontal = align_horizontal;
    }
}

function textsprite_set_align_vertical(textsprite, align) {
    textsprite.modified_coords = 1;
    textsprite.align_vertical = align;
}

function textsprite_set_align_horizontal(textsprite, align) {
    textsprite.modified_coords = 1;
    textsprite.align_horizontal = align;
}


function textsprite_matrix_get_modifier(textsprite) {
    return textsprite.matrix_source;
}

function textsprite_matrix_reset(textsprite) {
    pvrctx_helper_clear_modifier(textsprite.matrix_source);
    textsprite.flip_x = textsprite.flip_y = 0;
    textsprite.matrix_corner = { x: 0, y: 0, angle: 0 };
}

function textsprite_matrix_translate(textsprite, translate_x, translate_y) {
    if (translate_x != null)
        textsprite.matrix_source.translate_x = translate_x;
    if (translate_y != null)
        textsprite.matrix_source.translate_y = translate_y;
}

function textsprite_matrix_skew(textsprite, skew_x, skew_y) {
    if (skew_x != null)
        textsprite.matrix_source.skew_x = skew_x;
    if (skew_y != null)
        textsprite.matrix_source.skew_y = skew_y;
}

function textsprite_matrix_scale(textsprite, scale_x, scale_y) {
    if (scale_x != null)
        textsprite.matrix_source.scale_x = scale_x;
    if (scale_y != null)
        textsprite.matrix_source.scale_y = scale_y;
}

function textsprite_matrix_rotate(textsprite, degrees_angle) {
    textsprite.matrix_source.rotate = degrees_angle * MATH2D_DEG_TO_RAD;
}


function textsprite_matrix_rotate_pivot_enable(textsprite, enable) {
    textsprite.matrix_source.rotate_pivot_enabled = enable;
}

function textsprite_matrix_rotate_pivot(textsprite, u, v) {
    if (u != null) textsprite.matrix_source.rotate_pivot_u = u;
    if (v != null) textsprite.matrix_source.rotate_pivot_v = v;
}


function textsprite_matrix_scale_offset(textsprite, direction_x, direction_y) {
    if (direction_x != null) textsprite.matrix_source.scale_direction_x = direction_x;
    if (direction_y != null) textsprite.matrix_source.scale_direction_y = direction_y;
}


function textsprite_matrix_corner_rotation(textsprite, corner) {
    sh4matrix_helper_calculate_corner_rotation(textsprite.matrix_corner, corner);
}


function textsprite_matrix_calculate(textsprite, pvrctx) {
    let matrix = pvrctx.current_matrix;

    // step 1: flip text (if required)
    if (textsprite.flip_x || textsprite.flip_y) {
        sh4matrix_scale(
            matrix,
            textsprite.flip_x ? -1 : 1,
            textsprite.flip_y ? -1 : 1
        );
    }

    // step 2: apply self modifier
    sh4matrix_apply_modifier2(
        matrix,
        textsprite.matrix_source,
        textsprite.last_draw_x, textsprite.last_draw_y,
        textsprite.last_draw_width, textsprite.last_draw_height
    );

    // step 3: apply corner rotation
    if (textsprite.matrix_corner.angle != 0) {
        sh4matrix_corner_rotate(
            matrix, textsprite.matrix_corner,
            textsprite.last_draw_x, textsprite.last_draw_y,
            textsprite.last_draw_width, textsprite.last_draw_height
        );
    }

    pvr_context_flush(pvrctx);
}


function textsprite_calculate_paragraph_alignment(textsprite) {
    const grapheme = { code: 0, size: 0 };
    const lineinfo = { line_char_count: 0, last_char_width: 0, previous_codepoint: 0x0000, space_width: -1 };

    if (!textsprite.modified_string && !textsprite.modified_coords) return;

    // check if case conversion is required
    if (textsprite.modified_string || textsprite.last_force_case != textsprite.force_case) {
        textsprite.last_force_case = textsprite.force_case;

        if (textsprite.text_forced_case) textsprite.text_forced_case = undefined;

        switch (textsprite.force_case) {
            case TEXTSPRITE_FORCE_UPPERCASE:
                textsprite.text_forced_case = string_to_uppercase(textsprite.text);
                break;
            case TEXTSPRITE_FORCE_LOWERCASE:
                textsprite.text_forced_case = string_to_lowercase(textsprite.text);
                break;
            //case TEXTSPRITE_FORCE_NONE:
            default:
                textsprite.text_forced_case = null;
                break;
        }
    }

    const text = textsprite.text_forced_case ?? textsprite.text;
    arraylist_clear(textsprite.paragraph_array);

    if (!text) {
        textsprite.modified_string = 0;
        textsprite.last_draw_width = 0;
        textsprite.last_draw_height = 0;
        return;
    }

    let meansure_char_fn = textsprite.font_from_atlas ? fontglyph_meansure_char : fonttype_meansure_char;

    // step 1: count the paragraphs
    let line_count = MATH2D_MAX_INT32;
    if (textsprite.max_lines > 0) {
        line_count = string_occurrences_of_string(text, "\n") + 1;
        if (line_count > textsprite.max_lines) line_count = textsprite.max_lines;
    }

    // step 2: build paragraph info array and store in paragraph info offset the paragraph width
    let index = 0;
    let index_previous = 0;
    let index_current_line = 0;
    let accumulated_width = 0;
    let index_last_detected_break = 0;
    let last_break_was_dotcommatab = 0;
    let calculated_text_height = 0;
    let max_height = textsprite.max_height < 0 ? MATH2D_MAX_INT32 : textsprite.max_height;
    let max_width = textsprite.max_width < 0 ? MATH2D_MAX_INT32 : textsprite.max_width;
    let last_known_break_index = 0;
    let loose_index = 0;
    let last_known_break_width = 0;

    while (1) {
        let eof_reached = !string_get_character_codepoint(text, index, grapheme);

        if (grapheme.code == FONTGLYPH_LINEFEED || eof_reached || loose_index > 0) {
            let new_index = index + grapheme.size;
            if (loose_index > 0) {
                new_index = index = loose_index;
                loose_index = 0;
            }

            // note: do not ignore empty lines
            arraylist_add(textsprite.paragraph_array, {
                index: index_current_line,
                length: index - index_current_line,
                offset: accumulated_width// temporal
            });

            index_last_detected_break = index_current_line = new_index;
            last_break_was_dotcommatab = 1;

            calculated_text_height += textsprite.font_size + textsprite.font_paragraph_separation;
            if (calculated_text_height >= max_height) break;

            lineinfo.line_char_count = 0;
            lineinfo.previous_codepoint = 0x0000;
            index_previous = index;
            index = new_index;
            accumulated_width = 0;
            last_known_break_index = index;
            last_known_break_width = -1;

            if (eof_reached) break;
            continue;
        }

        // meansure char width
        meansure_char_fn(textsprite.font, grapheme.code, textsprite.font_size, lineinfo);

        // check if the current codepoit is breakable
        let current_is_break = 0;
        let break_in_index = -1;
        let break_char_count = 1;
        let break_codepoint = grapheme.code;
        let break_width = lineinfo.last_char_width;

        switch (grapheme.code) {
            case FONTGLYPH_SPACE:
                if (last_break_was_dotcommatab) {
                    last_known_break_width = accumulated_width;
                    current_is_break = 1;
                }
                last_break_was_dotcommatab = 0;
                break;
            case FONTGLYPH_TAB:
            case 0x2C:// comma
            case 0x2E:// dot
            case 0x3A:// dot dot
            case 0x3B:// dot comma
                last_known_break_width = accumulated_width;
                current_is_break = 1;
                last_break_was_dotcommatab = 1;
                break;
            default:
                last_break_was_dotcommatab = 1;
                break;
        }

        accumulated_width += lineinfo.last_char_width;

        if (accumulated_width > max_width) {
            if (current_is_break) {
                break_in_index = index;
                break_char_count = 0;
                break_codepoint = 0x0000;
                break_width = 0;
            } else {
                switch (textsprite.wordbreak) {
                    case FONT_WORDBREAK_NONE:
                        // do nothing, wait for the next line break
                        break;
                    case FONT_WORDBREAK_LOOSE:
                        // fallback to FONT_WORDBREAK_BREAK if the current word is to long
                        if (index_current_line != index_last_detected_break) {
                            // break and reparse the overflowing word
                            loose_index = last_known_break_index;
                            if (last_known_break_width >= 0) accumulated_width = last_known_break_width;
                            continue;
                        }
                    case FONT_WORDBREAK_BREAK:
                        // break in previous character
                        break_in_index = index_previous;
                        break;
                }
            }
        } else if (current_is_break) {
            index_last_detected_break = index;
            last_known_break_index = index;
        }

        // commit wordbreak (if required)
        if (break_in_index >= 0) {
            arraylist_add(textsprite.paragraph_array, {
                index: index_current_line,
                length: break_in_index - index_current_line,
                offset: accumulated_width// temporal
            });

            calculated_text_height += textsprite.font_size + textsprite.font_paragraph_separation;
            if (calculated_text_height >= max_height) break;

            index_last_detected_break = index_current_line = break_in_index;
            last_break_was_dotcommatab = 0;

            lineinfo.line_char_count = break_char_count;
            lineinfo.previous_codepoint = break_codepoint;
            accumulated_width = break_width;
            last_known_break_index = break_in_index;
            last_known_break_width = -1;
        }

        index_previous = index;
        index += grapheme.size;
    }

    if (arraylist_size(textsprite.paragraph_array) > line_count) {
        arraylist_cut_size(textsprite.paragraph_array, line_count);
    }

    // step 3: find the longest/wide paragraph
    let max_line_width = 0;
    let align_to_start = textsprite.paragraph_align == ALIGN_START;
    let align_to_center = textsprite.paragraph_align == ALIGN_CENTER;

    for (let paragraphinfo of arraylist_iterate4(textsprite.paragraph_array)) {
        if (paragraphinfo.offset > max_line_width) max_line_width = paragraphinfo.offset;
    }

    if (max_line_width == 0 || align_to_start) {
        for (let paragraphinfo of arraylist_iterate4(textsprite.paragraph_array)) {
            // ALIGN_START is used, put all offsets in zero
            paragraphinfo.offset = 0;
        }
    } else {
        // step 5: calculate paragraph offsets
        for (let paragraphinfo of arraylist_iterate4(textsprite.paragraph_array)) {
            if (max_line_width == paragraphinfo.offset) {
                paragraphinfo.offset = 0;
                continue;
            }

            paragraphinfo.offset = max_line_width - paragraphinfo.offset;// align to end
            if (align_to_center) paragraphinfo.offset /= 2;// align to center
        }
    }

    textsprite.modified_string = 0;
    textsprite.last_draw_width = max_line_width;
    textsprite.last_draw_height = calculated_text_height;
}


function textsprite_set_property(textsprite, property_id, value) {
    switch (property_id) {
        case TEXTSPRITE_PROP_STRING:
            textsprite_set_text_intern(textsprite, 0, value);
            break;
        case TEXTSPRITE_PROP_FONT_SIZE:
            textsprite_set_font_size(textsprite, value);
            break;
        case TEXTSPRITE_PROP_ALIGN_H:
            textsprite_set_align_horizontal(textsprite, value);
            break;
        case TEXTSPRITE_PROP_ALIGN_V:
            textsprite_set_align_vertical(textsprite, value);
            break;
        case TEXTSPRITE_PROP_FORCE_CASE:
            textsprite_force_case(textsprite, value);
            break;
        case TEXTSPRITE_PROP_FONT_COLOR:
            textsprite_set_color_rgba8(textsprite, value);
            break;
        case TEXTSPRITE_PROP_MAX_LINES:
            textsprite_set_maxlines(textsprite, value);
            break;
        case TEXTSPRITE_PROP_ALIGN_PARAGRAPH:
            textsprite_set_paragraph_align(textsprite, value);
            break;
        case TEXTSPRITE_PROP_BORDER_ENABLE:
            textsprite.border_enable = value == 1.0;
            break;
        case TEXTSPRITE_PROP_BORDER_SIZE:
            textsprite.border_size = value;
            break;
        case TEXTSPRITE_PROP_BORDER_COLOR_R:
            textsprite.border_color[0] = value;
            break;
        case TEXTSPRITE_PROP_BORDER_COLOR_G:
            textsprite.border_color[1] = value;
            break;
        case TEXTSPRITE_PROP_BORDER_COLOR_B:
            textsprite.border_color[2] = value;
            break;
        case TEXTSPRITE_PROP_BORDER_COLOR_A:
            textsprite.border_color[3] = value;
            break;
        /////////////////////////////////////////////////////////////////////////////////////////////////
        case SPRITE_PROP_X:
            textsprite.x = value;
            break;
        case SPRITE_PROP_Y:
            textsprite.y = value;
            break;
        //case SPRITE_PROP_WIDTH:
        case TEXTSPRITE_PROP_MAX_WIDTH:
            textsprite.max_width = value;
            break;
        //case SPRITE_PROP_HEIGHT:
        case TEXTSPRITE_PROP_MAX_HEIGHT:
            textsprite.max_height = value;
            break;
        case SPRITE_PROP_ROTATE:
            textsprite_matrix_rotate(textsprite, value);
            break;
        case SPRITE_PROP_SCALE_X:
            textsprite_matrix_scale(textsprite, value, null);
            break;
        case SPRITE_PROP_SCALE_Y:
            textsprite_matrix_scale(textsprite, null, value);
            break;
        case SPRITE_PROP_SKEW_X:
            textsprite_matrix_skew(textsprite, value, null);
            break;
        case SPRITE_PROP_SKEW_Y:
            textsprite_matrix_skew(textsprite, null, value);
            break;
        case SPRITE_PROP_TRANSLATE_X:
            textsprite_matrix_translate(textsprite, value, null);
            break;
        case SPRITE_PROP_TRANSLATE_Y:
            textsprite_matrix_translate(textsprite, null, value);
            break;
        case SPRITE_PROP_ALPHA:
            textsprite.alpha = math2d_clamp(value, 0, 1.0);
            break;
        case SPRITE_PROP_Z:
            textsprite.z = value;
            break;
        case SPRITE_PROP_VERTEX_COLOR_R:
            textsprite.color[0] = math2d_clamp(value, 0, 1.0);
            break;
        case SPRITE_PROP_VERTEX_COLOR_G:
            textsprite.color[1] = math2d_clamp(value, 0, 1.0);
            break;
        case SPRITE_PROP_VERTEX_COLOR_B:
            textsprite.color[2] = math2d_clamp(value, 0, 1.0);
            break;
        case SPRITE_PROP_ANIMATIONLOOP:
            if (textsprite.animation_selected)
                animsprite_set_loop(textsprite.animation_selected, value);
            break;
        case SPRITE_PROP_ROTATE_PIVOT_ENABLE:
            textsprite_matrix_rotate_pivot_enable(textsprite, value >= 1);
            break;
        case SPRITE_PROP_ROTATE_PIVOT_U:
            textsprite_matrix_rotate_pivot(textsprite, value, null);
            break;
        case SPRITE_PROP_ROTATE_PIVOT_V:
            textsprite_matrix_rotate_pivot(textsprite, null, value);
            break;
        case SPRITE_PROP_SCALE_DIRECTION_X:
            textsprite.matrix_source.scale_direction_x = value;
            break;
        case SPRITE_PROP_SCALE_DIRECTION_Y:
            textsprite.matrix_source.scale_direction_y = value;
            break;
        case SPRITE_PROP_TRANSLATE_ROTATION:
            textsprite.matrix_source.translate_rotation = value >= 1.0;
            break;
        case SPRITE_PROP_SCALE_SIZE:
            textsprite.matrix_source.scale_size = value >= 1.0;
            break;
        case SPRITE_PROP_SCALE_TRANSLATION:
            textsprite.matrix_source.scale_translation = value >= 1.0;
            break;
        case SPRITE_PROP_Z_OFFSET:
            textsprite.z_offset = value;
            break;
        case TEXTSPRITE_PROP_PARAGRAPH_SEPARATION:
            textsprite.font_paragraph_separation = value;
            break;
        case SPRITE_PROP_ANTIALIASING:
            textsprite.antialiasing = Math.trunc(value);
            break;
        case FONT_PROP_WORDBREAK:
            textsprite.wordbreak = Math.trunc(value);
            break;
    }

    // check if the coordinates was modified
    switch (property_id) {
        case SPRITE_PROP_X:
        case SPRITE_PROP_Y:
        case SPRITE_PROP_WIDTH:
        case SPRITE_PROP_HEIGHT:
        case TEXTSPRITE_PROP_MAX_WIDTH:
        case TEXTSPRITE_PROP_MAX_HEIGHT:
        case TEXTSPRITE_PROP_PARAGRAPH_SEPARATION:
            textsprite.modified_coords = 1;
            break;
        case FONT_PROP_WORDBREAK:
        case TEXTSPRITE_PROP_FONT_SIZE:
            textsprite.modified_string = 1;
            break;
    }

}


function textsprite_get_draw_size(textsprite, draw_size) {
    textsprite_calculate_paragraph_alignment(textsprite);
    draw_size[0] = textsprite.last_draw_width;
    draw_size[1] = textsprite.last_draw_height;
    return draw_size;
}

function textsprite_get_font_size(textsprite) {
    return textsprite.font_size;
}

function textsprite_get_max_draw_size(textsprite, max_draw_size) {
    max_draw_size[0] = textsprite.max_width;
    max_draw_size[1] = textsprite.max_height;
    return max_draw_size;
}

function textsprite_draw(textsprite, pvrctx) {
    if (textsprite.alpha <= 0) return;
    if (!textsprite.text_forced_case && !textsprite.text) return;

    // check if all calculations are up-to-date
    if (textsprite.modified_string || textsprite.modified_coords) textsprite_draw_calc(textsprite);

    textsprite_draw_internal(textsprite, pvrctx);
}


function textsprite_draw_internal(textsprite, pvrctx) {
    let alpha_fn;
    let color_fn;
    let draw_fn;
    let borderset_fn;
    let separation_fn;

    if (textsprite.font_from_atlas) {
        alpha_fn = fontglyph_set_alpha;
        color_fn = fontglyph_set_color;
        draw_fn = fontglyph_draw_text;
        borderset_fn = fontglyph_set_border;
        separation_fn = fontglyph_set_lines_separation;
    } else {
        alpha_fn = fonttype_set_alpha;
        color_fn = fonttype_set_color;
        draw_fn = fonttype_draw_text;
        borderset_fn = fonttype_set_border;
        separation_fn = fonttype_set_lines_separation;
    }
    alpha_fn(textsprite.font, textsprite.alpha);
    color_fn(textsprite.font, textsprite.color[0], textsprite.color[1], textsprite.color[2]);
    borderset_fn(
        textsprite.font, textsprite.border_enable, textsprite.border_size, textsprite.border_color
    );
    separation_fn(textsprite.font, textsprite.font_paragraph_separation);


    pvr_context_save(pvrctx);

    if (textsprite.psshader) pvr_context_add_shader(pvrctx, textsprite.psshader);

    pvr_context_set_vertex_blend(
        pvrctx,
        textsprite.blend_enabled,
        textsprite.blend_src_rgb,
        textsprite.blend_dst_rgb,
        textsprite.blend_src_alpha,
        textsprite.blend_dst_alpha
    );

    if (textsprite.antialiasing != PVR_FLAG_DEFAULT) {
        pvr_context_set_global_antialiasing(pvrctx, textsprite.antialiasing);
    }

    textsprite_matrix_calculate(textsprite, pvrctx);

    const text = textsprite.text_forced_case ?? textsprite.text;

    if (arraylist_size(textsprite.paragraph_array) < 2) {
        // let the font handle the draw
        draw_fn(
            textsprite.font,
            pvrctx,
            textsprite.font_size,
            textsprite.last_draw_x, textsprite.last_draw_y,
            0, text.length, text
        );
    } else {
        // paragraph by paragraph draw
        let y = textsprite.last_draw_y;
        let line_height = textsprite.font_size + textsprite.font_paragraph_separation;

        for (let paragraphinfo of arraylist_iterate4(textsprite.paragraph_array)) {
            if (paragraphinfo.length > 0) {
                draw_fn(
                    textsprite.font, pvrctx, textsprite.font_size,
                    textsprite.last_draw_x + paragraphinfo.offset, y,
                    paragraphinfo.index, paragraphinfo.length, text
                );
            }
            y += line_height;
        }
    }

    pvr_context_restore(pvrctx);
}

function textsprite_draw_calc(textsprite) {
    // step 1: calculate the draw size (lines and the most wide paragraph)
    let draw_size = [0, 0];
    textsprite_get_draw_size(textsprite, draw_size);

    // step 2: calculate the alignment
    let offset_x = 0;
    let offset_y = 0;
    if (textsprite.max_height >= 0 && textsprite.align_vertical != ALIGN_START) {
        offset_y = textsprite.max_height - draw_size[1];
        if (textsprite.align_vertical == ALIGN_CENTER) offset_y /= 2;
    }
    if (textsprite.max_width >= 0 && textsprite.align_horizontal != ALIGN_START) {
        offset_x = textsprite.max_width - draw_size[0];
        if (textsprite.align_horizontal == ALIGN_CENTER) offset_x /= 2;
    }

    // step 3: cache the drawing coordinates
    textsprite.last_draw_x = textsprite.x + offset_x;
    textsprite.last_draw_y = textsprite.y + offset_y;
    textsprite.modified_coords = 0;
    textsprite.modified_string = 0;
}

function textsprite_animation_set(textsprite, animsprite) {
    let old_animation = textsprite.animation_selected;
    textsprite.animation_selected = animsprite;
    return old_animation;
}

function textsprite_animation_restart(textsprite) {
    if (textsprite.animation_selected) animsprite_restart(textsprite.animation_selected);
}

function textsprite_animation_end(textsprite) {
    if (!textsprite.animation_selected) return;
    animsprite_force_end(textsprite.animation_selected);
    animsprite_update_textsprite(textsprite.animation_selected, textsprite, 1);
}

function textsprite_animation_stop(textsprite) {
    if (textsprite.animation_selected) animsprite_stop(textsprite.animation_selected);
}


function textsprite_animation_external_set(textsprite, animsprite) {
    textsprite.animation_external = animsprite;
}

function textsprite_animation_external_end(textsprite) {
    if (textsprite.animation_external) {
        animsprite_force_end(textsprite.animation_external);
        animsprite_update_textsprite(textsprite.animation_external, textsprite, 1);
    }
}

function textsprite_animation_external_restart(textsprite) {
    if (textsprite.animation_external) animsprite_restart(textsprite.animation_external);
}


function textsprite_animate(textsprite, elapsed) {
    let result = 1;

    if (textsprite.animation_selected) {
        result = animsprite_animate(textsprite.animation_selected, elapsed);
        animsprite_update_textsprite(textsprite.animation_selected, textsprite, 1);
    }

    if (textsprite.animation_external) {
        result = animsprite_animate(textsprite.animation_external, elapsed);
        animsprite_update_textsprite(textsprite.animation_external, textsprite, 0);
    }

    if (textsprite.modified_string && textsprite.modified_coords) textsprite_draw_calc(textsprite);

    return result;
}


function textsprite_border_enable(textsprite, enable) {
    textsprite.border_enable = !!enable;
}

function textsprite_border_set_size(textsprite, border_size) {
    textsprite.border_size = Number.isFinite(border_size) ? border_size : 0;
}

function textsprite_border_set_color(textsprite, r, g, b, a) {
    if (r >= 0) textsprite.border_color[0] = r;
    if (g >= 0) textsprite.border_color[1] = g;
    if (b >= 0) textsprite.border_color[2] = b;
    if (a >= 0) textsprite.border_color[3] = a;
}

function textsprite_border_set_color_rgba8(textsprite, rbga8_color) {
    math2d_color_bytes_to_floats(rbga8_color, 1, textsprite.border_color);
}


function textsprite_is_visible(textsprite) {
    return textsprite.visible;
}

function textsprite_set_antialiasing(textsprite, antialiasing) {
    textsprite.antialiasing = antialiasing;
}

function textsprite_set_wordbreak(textsprite, wordbreak) {
    textsprite.wordbreak = wordbreak;
    textsprite.modified_coords = 1;
}

function textsprite_has_font(textsprite) {
    return textsprite.font != null;
}

function textsprite_change_font(textsprite, fontholder) {
    if (fontholder == null) throw new Error("fontholder can not be null");
    textsprite.font = textsprite.font;
    textsprite.font_from_atlas = textsprite.font_from_atlas;
}

function textsprite_set_shader(textsprite, psshader) {
    textsprite.psshader = psshader;
}

function textsprite_get_shader(textsprite) {
    return textsprite.psshader;
}

function textsprite_blend_enable(textsprite, enabled) {
    textsprite.blend_enabled = enabled;
}

function textsprite_blend_set(textsprite, src_rgb, dst_rgb, src_alpha, dst_alpha) {
    textsprite.blend_src_rgb = src_rgb;
    textsprite.blend_dst_rgb = dst_rgb;
    textsprite.blend_src_alpha = src_alpha;
    textsprite.blend_dst_alpha = dst_alpha;
}

