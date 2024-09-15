#include "textsprite.h"

#include "arraylist.h"
#include "externals/luascript.h"
#include "fontglyph.h"
#include "fontglyph_utils.h"
#include "fonttype.h"
#include "glyphrenderer.h"
#include "linkedlist.h"
#include "malloc_utils.h"
#include "map.h"
#include "pvrcontext.h"
#include "stringbuilder.h"
#include "stringutils.h"
#include "vertexprops.h"


typedef float (*MeasureFn)(void* font, FontParams* params, const char* text, int32_t text_index, size_t text_length);
typedef void (*MeasureCharFn)(void* font, uint32_t codepoint, float height, FontLineInfo* lineinfo);
typedef float (*DrawTextFn)(void* font, PVRContext pvrctx, FontParams* params, float x, float y, int32_t text_index, size_t text_length, const char* text);
typedef int32_t (*AnimateFn)(void* font, float elapsed);
typedef void (*MapCodepointsFn)(void* font, const char* text, int32_t text_index, size_t text_length);


typedef struct {
    MeasureFn measure;
    MeasureCharFn measure_char;
    DrawTextFn draw_text;
    AnimateFn animate;
    MapCodepointsFn map_codepoints;
} FontFunctions;

typedef struct {
    int32_t length;
    float offset;
    int32_t index;
} ParagraphInfo;

struct TextSprite_s {
    void* font;
    // bool font_from_atlas;
    char** text;
    bool intern;
    int32_t force_case;
    int32_t last_force_case;
    char* text_forced_case;
    bool visible;
    float x;
    float y;
    float z;
    float alpha2;
    float z_offset;
    Align align_vertical;
    Align align_horizontal;
    float max_width;
    float max_height;
    int32_t max_lines;
    Align paragraph_align;
    Modifier matrix_source;
    CornerRotation matrix_corner;
    bool flip_x;
    bool flip_y;
    ArrayList paragraph_array;
    bool modified_coords;
    bool modified_string;
    float last_draw_x;
    float last_draw_y;
    float last_draw_width;
    float last_draw_height;
    AnimSprite animation_external;
    AnimSprite animation_selected;
    PVRFlag antialiasing;
    int32_t wordbreak;

    // PSShader psshader;

    bool blend_enabled;
    Blend blend_src_rgb;
    Blend blend_dst_rgb;
    Blend blend_src_alpha;
    Blend blend_dst_alpha;
    bool background_enabled;
    float background_size;
    float background_offset_x;
    float background_offset_y;
    RGBA background_rgba;
    int32_t id;

    FontParams fontparams;
    FontFunctions fontfunctions;
    char* text_holder;
};


static Map TEXTSPRITE_POOL = NULL;
static int32_t TEXTSPRITE_IDS = 0;


void __attribute__((constructor)) __ctor_textsprite() {
    TEXTSPRITE_POOL = map_init();
}


static void textsprite_matrix_calculate(TextSprite textsprite, PVRContext pvrctx);
static void textsprite_draw_internal(TextSprite textsprite, PVRContext pvrctx);
static void textsprite_draw_calc(TextSprite textsprite);


TextSprite textsprite_init(void* font, bool font_is_truetype, bool color_by_addition, float size, uint32_t rbg8_color) {
    TextSprite textsprite = malloc_chk(sizeof(struct TextSprite_s));
    malloc_assert(textsprite, TextSprite);

    *textsprite = (struct TextSprite_s){
        .font = font,

        .text = NULL,
        .intern = false,

        .force_case = 0,
        .last_force_case = 0,
        .text_forced_case = NULL,

        .visible = true,
        .x = 0.0f,
        .y = 0.0f,
        .z = 0.0f,
        .alpha2 = 1.0f,
        .z_offset = 0.0f,

        .align_vertical = ALIGN_START,
        .align_horizontal = ALIGN_START,
        .max_width = -1.0f,
        .max_height = -1.0f,

        .max_lines = -1,
        .paragraph_align = ALIGN_START,


        .matrix_corner = {x : 0, y : 0, angle : 0},
        .flip_x = false,
        .flip_y = false,

        .paragraph_array = arraylist_init(sizeof(ParagraphInfo)),

        .modified_coords = false,
        .modified_string = false,

        .last_draw_x = 0.0f,
        .last_draw_y = 0.0f,

        .last_draw_width = 0.0f,
        .last_draw_height = 0.0f,

        .animation_external = NULL,
        .animation_selected = NULL,

        .antialiasing = PVRCTX_FLAG_DEFAULT,

        .wordbreak = FONT_WORDBREAK_LOOSE,

        //.psshader = NULL,

        .blend_enabled = true, // obligatory
        .blend_src_rgb = BLEND_DEFAULT,
        .blend_dst_rgb = BLEND_DEFAULT,
        .blend_src_alpha = BLEND_DEFAULT,
        .blend_dst_alpha = BLEND_DEFAULT,

        .background_enabled = false,
        .background_size = 0.0f,
        .background_offset_x = 0.0f,
        .background_offset_y = 0.0f,
        .background_rgba = {0.0f, 0.0f, 0.0f, 0.5f},

        .fontparams = {
            .height = size,
            .paragraph_space = 0.0f,
            .color_by_addition = false,

            .tint_color = {1.0f, 1.0f, 1.0f, 1.0f},

            .border_enable = color_by_addition,
            .border_size = 0.0f,

            .border_color = {0.0f, 0.0f, 0.0f, 1.0f},
            .border_offset_x = 0.0f,
            .border_offset_y = 0.0f,
        },

        .fontfunctions = {
            .measure = NULL,
            .measure_char = NULL,
            .draw_text = NULL,
            .animate = NULL,
            .map_codepoints = NULL,
        },

        .id = TEXTSPRITE_IDS++,

        .text_holder = NULL
    };

    textsprite->text = &textsprite->text_holder;

    math2d_color_bytes_to_floats(rbg8_color, false, textsprite->fontparams.tint_color);

    pvr_context_helper_clear_modifier(&textsprite->matrix_source);

    if (font_is_truetype) {
        textsprite->fontfunctions.measure = (MeasureFn)fonttype_measure;
        textsprite->fontfunctions.measure_char = (MeasureCharFn)fonttype_measure_char;
        textsprite->fontfunctions.draw_text = (DrawTextFn)fonttype_draw_text;
        textsprite->fontfunctions.animate = (AnimateFn)fonttype_animate;
        textsprite->fontfunctions.map_codepoints = (MapCodepointsFn)fonttype_map_codepoints;
    } else {
        textsprite->fontfunctions.measure = (MeasureFn)fontglyph_measure;
        textsprite->fontfunctions.measure_char = (MeasureCharFn)fontglyph_measure_char;
        textsprite->fontfunctions.draw_text = (DrawTextFn)fontglyph_draw_text;
        textsprite->fontfunctions.animate = (AnimateFn)fontglyph_animate;
        textsprite->fontfunctions.map_codepoints = (MapCodepointsFn)fontglyph_map_codepoints;
    }

    map_add(TEXTSPRITE_POOL, textsprite->id, textsprite);
    return textsprite;
}

TextSprite textsprite_init2(FontHolder fontholder, float font_size, uint32_t rbg8_color) {
    return textsprite_init(fontholder->font, !fontholder->font_from_atlas, fontholder->font_color_by_addition, font_size, rbg8_color);
}

void textsprite_destroy(TextSprite* textsprite_ptr) {
    TextSprite textsprite = *textsprite_ptr;
    if (!textsprite) return;

    textsprite->font = NULL; // set to NULL only

    if (!textsprite->intern) free_chk(*textsprite->text);

    if (textsprite->text_forced_case) free_chk(textsprite->text_forced_case);

    arraylist_destroy(&textsprite->paragraph_array);

    if (textsprite->animation_selected) animsprite_destroy(&textsprite->animation_selected);

    map_delete(TEXTSPRITE_POOL, textsprite->id);
    luascript_drop_shared(&textsprite->matrix_source);
    luascript_drop_shared(textsprite);

    free_chk(textsprite);
    *textsprite_ptr = NULL;
}


void textsprite_set_text(TextSprite textsprite, const char* text) {
    textsprite_set_text_intern(textsprite, false, &text);
}

void textsprite_set_text_intern(TextSprite textsprite, bool intern, const char* const* text) {
    if (!textsprite->intern) free_chk(*textsprite->text);

    textsprite->intern = intern;
    if (text == NULL) {
        // this never should happen (especially for pointer-to-pointer strings)
        textsprite->text_holder = NULL;
        textsprite->text = &textsprite->text_holder;
    } else if (intern) {
        textsprite->text = (char**)text;
    } else {
        textsprite->text_holder = string_duplicate(*text);
        textsprite->text = &textsprite->text_holder;
    }

    textsprite->modified_string = true;
}

void textsprite_set_text_formated(TextSprite textsprite, const char* format, ...) {
    va_list values;
    va_start(values, format);

    char* text = stringbuilder_helper_create_formatted_string2(format, values);
    va_end(values);

    if (!textsprite->intern) free_chk(*textsprite->text);

    textsprite->intern = false;
    textsprite->text_holder = text;
    textsprite->text = &textsprite->text_holder;

    textsprite->modified_string = true;
}

void textsprite_set_text_formated2(TextSprite textsprite, const char* format, va_list va_args) {
    char* text = stringbuilder_helper_create_formatted_string2(format, va_args);

    if (!textsprite->intern) free_chk(*textsprite->text);

    textsprite->intern = false;
    textsprite->text_holder = text;
    textsprite->text = &textsprite->text_holder;

    textsprite->modified_string = true;
}

void textsprite_set_font_size(TextSprite textsprite, float font_size) {
    textsprite->modified_coords = true;
    textsprite->fontparams.height = font_size;
}

void textsprite_force_case(TextSprite textsprite, int32_t none_or_lowercase_or_uppercase) {
    textsprite->modified_string = true;
    textsprite->force_case = none_or_lowercase_or_uppercase;
}

void textsprite_set_paragraph_align(TextSprite textsprite, Align align) {
    textsprite->modified_coords = true;
    textsprite->paragraph_align = align;
}

void textsprite_set_paragraph_space(TextSprite textsprite, float space) {
    textsprite->fontparams.paragraph_space = space;
}

void textsprite_set_maxlines(TextSprite textsprite, int32_t max_lines) {
    textsprite->modified_coords = true;
    textsprite->max_lines = max_lines;
}

void textsprite_set_color_rgba8(TextSprite textsprite, uint32_t rbg8_color) {
    math2d_color_bytes_to_floats(rbg8_color, 0, textsprite->fontparams.tint_color);
}

void textsprite_set_color(TextSprite textsprite, float r, float g, float b) {
    if (r >= 0.0f) textsprite->fontparams.tint_color[0] = r;
    if (g >= 0.0f) textsprite->fontparams.tint_color[1] = g;
    if (b >= 0.0f) textsprite->fontparams.tint_color[2] = b;
}


void textsprite_set_alpha(TextSprite textsprite, float alpha) {
    textsprite->fontparams.tint_color[3] = alpha;
}

void textsprite_set_visible(TextSprite textsprite, bool visible) {
    textsprite->visible = visible;
}

void textsprite_set_draw_location(TextSprite textsprite, float x, float y) {
    textsprite->modified_coords = true;
    if (!math2d_is_float_NaN(x)) textsprite->x = x;
    if (!math2d_is_float_NaN(y)) textsprite->y = y;
}

void textsprite_get_draw_location(TextSprite textsprite, float* x, float* y) {
    *x = textsprite->x;
    *y = textsprite->y;
}

void textsprite_set_z_index(TextSprite textsprite, float z_index) {
    textsprite->z = z_index;
}

void textsprite_set_z_offset(TextSprite textsprite, float offset) {
    textsprite->z_offset = offset;
}

float textsprite_get_z_index(TextSprite textsprite) {
    return textsprite->z + textsprite->z_offset;
}

void textsprite_set_max_draw_size(TextSprite textsprite, float max_width, float max_height) {
    textsprite->modified_coords = true;
    if (!math2d_is_float_NaN(max_width)) textsprite->max_width = max_width;
    if (!math2d_is_float_NaN(max_height)) textsprite->max_height = max_height;
}

void textsprite_matrix_flip(TextSprite textsprite, nbool flip_x, nbool flip_y) {
    textsprite->modified_coords = true;
    if (flip_x != unset) textsprite->flip_x = flip_x;
    if (flip_y != unset) textsprite->flip_y = flip_y;
}


void textsprite_set_align(TextSprite textsprite, Align align_vertical, Align align_horizontal) {
    textsprite->modified_coords = true;
    if (align_vertical != ALIGN_NONE) {
        if (align_vertical == ALIGN_BOTH) align_vertical = ALIGN_START;
        textsprite->align_vertical = align_vertical;
    }
    if (align_horizontal != ALIGN_NONE) {
        if (align_horizontal == ALIGN_BOTH) align_horizontal = ALIGN_START;
        textsprite->align_horizontal = align_horizontal;
    }
}

void textsprite_set_align_vertical(TextSprite textsprite, Align align) {
    textsprite->modified_coords = true;
    textsprite->align_vertical = align;
}

void textsprite_set_align_horizontal(TextSprite textsprite, Align align) {
    textsprite->modified_coords = true;
    textsprite->align_horizontal = align;
}


Modifier* textsprite_matrix_get_modifier(TextSprite textsprite) {
    return &textsprite->matrix_source;
}

void textsprite_matrix_reset(TextSprite textsprite) {
    pvr_context_helper_clear_modifier(&textsprite->matrix_source);
    textsprite->flip_x = textsprite->flip_y = false;
    textsprite->matrix_corner = (CornerRotation){.x = 0.0f, .y = 0.0f, .angle = 0.0f};
}

void textsprite_matrix_translate(TextSprite textsprite, float translate_x, float translate_y) {
    if (!math2d_is_float_NaN(translate_x))
        textsprite->matrix_source.translate_x = translate_x;
    if (!math2d_is_float_NaN(translate_y))
        textsprite->matrix_source.translate_y = translate_y;
}

void textsprite_matrix_skew(TextSprite textsprite, float skew_x, float skew_y) {
    if (!math2d_is_float_NaN(skew_x))
        textsprite->matrix_source.skew_x = skew_x;
    if (!math2d_is_float_NaN(skew_y))
        textsprite->matrix_source.skew_y = skew_y;
}

void textsprite_matrix_scale(TextSprite textsprite, float scale_x, float scale_y) {
    if (!math2d_is_float_NaN(scale_x))
        textsprite->matrix_source.scale_x = scale_x;
    if (!math2d_is_float_NaN(scale_y))
        textsprite->matrix_source.scale_y = scale_y;
}

void textsprite_matrix_rotate(TextSprite textsprite, float degrees_angle) {
    textsprite->matrix_source.rotate = degrees_angle * MATH2D_DEG_TO_RAD;
}


void textsprite_matrix_rotate_pivot_enable(TextSprite textsprite, bool enable) {
    textsprite->matrix_source.rotate_pivot_enabled = enable;
}

void textsprite_matrix_rotate_pivot(TextSprite textsprite, float u, float v) {
    if (!math2d_is_float_NaN(u)) textsprite->matrix_source.rotate_pivot_u = u;
    if (!math2d_is_float_NaN(v)) textsprite->matrix_source.rotate_pivot_v = v;
}


void textsprite_matrix_scale_offset(TextSprite textsprite, float direction_x, float direction_y) {
    if (!math2d_is_float_NaN(direction_x)) textsprite->matrix_source.scale_direction_x = direction_x;
    if (!math2d_is_float_NaN(direction_y)) textsprite->matrix_source.scale_direction_y = direction_y;
}


void textsprite_matrix_corner_rotation(TextSprite textsprite, Corner corner) {
    sh4matrix_helper_calculate_corner_rotation(&textsprite->matrix_corner, corner);
}


static void textsprite_matrix_calculate(TextSprite textsprite, PVRContext pvrctx) {
    float* matrix = pvrctx->current_matrix;

    // step 1: flip text (if required)
    if (textsprite->flip_x || textsprite->flip_y) {
        sh4matrix_scale(
            matrix,
            textsprite->flip_x ? -1.0f : 1.0f,
            textsprite->flip_y ? -1.0f : 1.0f
        );
    }

    // step 2: apply self modifier
    sh4matrix_apply_modifier2(
        matrix,
        &textsprite->matrix_source,
        textsprite->last_draw_x, textsprite->last_draw_y,
        textsprite->last_draw_width, textsprite->last_draw_height
    );

    // step 3: apply corner rotation
    if (textsprite->matrix_corner.angle != 0.0f) {
        sh4matrix_corner_rotate(
            matrix, textsprite->matrix_corner,
            textsprite->last_draw_x, textsprite->last_draw_y,
            textsprite->last_draw_width, textsprite->last_draw_height
        );
    }

    pvr_context_flush(pvrctx);
}


void textsprite_calculate_paragraph_alignment(TextSprite textsprite) {
    Grapheme grapheme = {.code = 0, .size = 0};
    FontLineInfo lineinfo = {
        .line_char_count = 0, .last_char_width = 0.0f, .last_char_height = 0.0f, .previous_codepoint = 0x0000, .space_width = -1.0f
    };

    if (!textsprite->modified_string && !textsprite->modified_coords) return;

    // check if case conversion is required
    if (textsprite->modified_string || textsprite->last_force_case != textsprite->force_case) {
        textsprite->last_force_case = textsprite->force_case;

        if (textsprite->text_forced_case) free_chk(textsprite->text_forced_case);

        switch (textsprite->force_case) {
            case TEXTSPRITE_FORCE_UPPERCASE:
                textsprite->text_forced_case = string_to_uppercase(*textsprite->text);
                break;
            case TEXTSPRITE_FORCE_LOWERCASE:
                textsprite->text_forced_case = string_to_lowercase(*textsprite->text);
                break;
            // case TEXTSPRITE_FORCE_NONE:
            default:
                textsprite->text_forced_case = NULL;
                break;
        }
    }

    const char* text = textsprite->text_forced_case ? textsprite->text_forced_case : *textsprite->text;
    arraylist_clear(textsprite->paragraph_array);

    if (string_is_empty(text)) {
        textsprite->modified_string = false;
        textsprite->last_draw_width = 0.0f;
        textsprite->last_draw_height = 0.0f;
        return;
    }

    size_t text_length = strlen(text);
    if (textsprite->modified_string) {
        textsprite->fontfunctions.map_codepoints(textsprite->font, text, 0, text_length);
        textsprite->fontfunctions.measure(textsprite->font, &textsprite->fontparams, text, 0, text_length);
    }

    // step 1: count the paragraphs
    int32_t line_count = MATH2D_MAX_INT32;
    if (textsprite->max_lines > 0) {
        line_count = string_occurrences_of_char(text, '\n') + 1;
        if (line_count > textsprite->max_lines) line_count = textsprite->max_lines;
    }

    // step 2: build paragraph info array and store in paragraph info offset the paragraph width
    int32_t index = 0;
    int32_t index_previous = 0;
    int32_t index_current_line = 0;
    float accumulated_width = 0.0f;
    int32_t index_last_detected_break = 0;
    bool last_break_was_dotcommatab = true;
    float calculated_text_height = 0.0f;
    float max_height = textsprite->max_height < 0.0f ? FLOAT_Inf : textsprite->max_height;
    float max_width = textsprite->max_width < 0.0f ? FLOAT_Inf : textsprite->max_width;
    int32_t last_known_break_index = 0;
    int32_t loose_index = 0;
    float last_known_break_width = 0.0f;
    float border_size = 0.0f;

    if (textsprite->fontparams.border_enable && textsprite->fontparams.border_size > 0.0f && textsprite->fontparams.border_color[3] > 0.0f) {
        border_size = textsprite->fontparams.border_size;
    }

    while (true) {
        bool eof_reached = !string_get_character_codepoint(text, index, text_length, &grapheme);

        if (grapheme.code == FONTGLYPH_LINEFEED || eof_reached || loose_index > 0) {
            int32_t new_index = index + grapheme.size;
            if (loose_index > 0) {
                new_index = index = loose_index;
                loose_index = 0;
            }

            // note: do not ignore empty lines
            arraylist_add(
                textsprite->paragraph_array,
                &(ParagraphInfo){
                    .index = index_current_line, .length = index - index_current_line,
                    .offset = accumulated_width // temporal
                }
            );

            index_last_detected_break = index_current_line = new_index;
            last_break_was_dotcommatab = true;

            calculated_text_height += lineinfo.last_char_height + textsprite->fontparams.paragraph_space;
            if ((calculated_text_height + border_size) >= max_height) break;

            lineinfo.line_char_count = 0;
            lineinfo.previous_codepoint = 0x0000;
            index_previous = index;
            index = new_index;
            accumulated_width = 0.0f;
            last_known_break_index = index;
            last_known_break_width = -1.0f;

            if (eof_reached) break;
            continue;
        }

        // measure char width
        textsprite->fontfunctions.measure_char(textsprite->font, grapheme.code, textsprite->fontparams.height, &lineinfo);

        // check if the current codepoint is breakable
        bool current_is_break = false;
        int32_t break_in_index = -1;
        int32_t break_char_count = 1;
        uint32_t break_codepoint = grapheme.code;
        float break_width = lineinfo.last_char_width;

        switch (grapheme.code) {
            case FONTGLYPH_SPACE:
                if (last_break_was_dotcommatab) {
                    last_known_break_width = accumulated_width;
                    current_is_break = true;
                }
                last_break_was_dotcommatab = false;
                break;
            case FONTGLYPH_TAB:
            case 0x2C: // comma
            case 0x2E: // dot
            case 0x3A: // dot dot
            case 0x3B: // dot comma
                last_known_break_width = accumulated_width;
                current_is_break = true;
                last_break_was_dotcommatab = true;
                break;
            default:
                last_break_was_dotcommatab = true;
                break;
        }

        accumulated_width += lineinfo.last_char_width;

        if ((accumulated_width + border_size) > max_width) {
            if (current_is_break) {
                break_in_index = index;
                break_char_count = 0;
                break_codepoint = 0x0000;
                break_width = 0.0f;
            } else {
                switch (textsprite->wordbreak) {
                    case FONT_WORDBREAK_NONE:
                        // do nothing, wait for the next line break
                        break;
                    case FONT_WORDBREAK_LOOSE:
                        // fallback to FONT_WORDBREAK_BREAK if the current word is to long
                        if (index_current_line != index_last_detected_break) {
                            // break and reparse the overflowing word
                            loose_index = last_known_break_index;
                            if (last_known_break_width >= 0.0f) accumulated_width = last_known_break_width;
                            continue;
                        }
                        __attribute__((fallthrough));
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
            arraylist_add(
                textsprite->paragraph_array,
                &(ParagraphInfo){
                    .index = index_current_line, .length = break_in_index - index_current_line,
                    .offset = accumulated_width // temporal
                }
            );

            calculated_text_height += lineinfo.last_char_height + textsprite->fontparams.paragraph_space;
            if ((calculated_text_height + border_size) >= max_height) break;

            index_last_detected_break = index_current_line = break_in_index;
            last_break_was_dotcommatab = false;

            lineinfo.line_char_count = break_char_count;
            lineinfo.previous_codepoint = break_codepoint;
            accumulated_width = break_width;
            last_known_break_index = break_in_index;
            last_known_break_width = -1.0f;
        }

        index_previous = index;
        index += grapheme.size;
    }

    if (arraylist_size(textsprite->paragraph_array) > line_count) {
        arraylist_cut_size(textsprite->paragraph_array, line_count);
    }

    // step 3: find the longest/wide paragraph
    float max_line_width = 0.0f;
    bool align_to_start = textsprite->paragraph_align == ALIGN_START;
    bool align_to_center = textsprite->paragraph_align == ALIGN_CENTER;

    foreach (ParagraphInfo*, paragraphinfo, ARRAYLIST_ITERATOR, textsprite->paragraph_array) {
        if (paragraphinfo->offset > max_line_width) max_line_width = paragraphinfo->offset;
    }

    if (max_line_width == 0.0f || align_to_start) {
        foreach (ParagraphInfo*, paragraphinfo, ARRAYLIST_ITERATOR, textsprite->paragraph_array) {
            // ALIGN_START is used, put all offsets in zero
            paragraphinfo->offset = 0.0f;
        }
    } else {
        // step 5: calculate paragraph offsets
        foreach (ParagraphInfo*, paragraphinfo, ARRAYLIST_ITERATOR, textsprite->paragraph_array) {
            if (max_line_width == paragraphinfo->offset) {
                paragraphinfo->offset = 0.0f;
                continue;
            }

            paragraphinfo->offset = max_line_width - paragraphinfo->offset; // align to end
            if (align_to_center) paragraphinfo->offset /= 2.0f;             // align to center
        }
    }

    textsprite->modified_string = false;
    textsprite->last_draw_width = max_line_width + border_size;
    textsprite->last_draw_height = calculated_text_height + border_size;
}


void textsprite_set_property(TextSprite textsprite, int32_t property_id, /*union {const */ float value /*; const char* string; } complex_value*/) {
    switch (property_id) {
        case TEXTSPRITE_PROP_STRING:
            // textsprite_set_text_intern(textsprite, false, value.string);
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
            textsprite_force_case(textsprite, (int32_t)value);
            break;
        case TEXTSPRITE_PROP_FONT_COLOR:
            textsprite_set_color_rgba8(textsprite, (uint32_t)value);
            break;
        case TEXTSPRITE_PROP_MAX_LINES:
            textsprite_set_maxlines(textsprite, (int32_t)value);
            break;
        case TEXTSPRITE_PROP_ALIGN_PARAGRAPH:
            textsprite_set_paragraph_align(textsprite, value);
            break;
        case TEXTSPRITE_PROP_BORDER_ENABLE:
            textsprite->fontparams.border_enable = value >= 1.0f;
            break;
        case TEXTSPRITE_PROP_BORDER_SIZE:
            textsprite->fontparams.border_size = value;
            break;
        case TEXTSPRITE_PROP_BORDER_COLOR_R:
            textsprite->fontparams.border_color[0] = value;
            break;
        case TEXTSPRITE_PROP_BORDER_COLOR_G:
            textsprite->fontparams.border_color[1] = value;
            break;
        case TEXTSPRITE_PROP_BORDER_COLOR_B:
            textsprite->fontparams.border_color[2] = value;
            break;
        case TEXTSPRITE_PROP_BORDER_COLOR_A:
            textsprite->fontparams.border_color[3] = value;
            break;
        case TEXTSPRITE_PROP_BORDER_OFFSET_X:
            textsprite->fontparams.border_offset_x = value;
            break;
        case TEXTSPRITE_PROP_BORDER_OFFSET_Y:
            textsprite->fontparams.border_offset_y = value;
            break;
        /////////////////////////////////////////////////////////////////////////////////////////////////
        case SPRITE_PROP_X:
            textsprite->x = value;
            break;
        case SPRITE_PROP_Y:
            textsprite->y = value;
            break;
        // case SPRITE_PROP_WIDTH:
        case TEXTSPRITE_PROP_MAX_WIDTH:
            textsprite->max_width = value;
            break;
        // case SPRITE_PROP_HEIGHT:
        case TEXTSPRITE_PROP_MAX_HEIGHT:
            textsprite->max_height = value;
            break;
        case SPRITE_PROP_ROTATE:
            textsprite_matrix_rotate(textsprite, value);
            break;
        case SPRITE_PROP_SCALE_X:
            textsprite_matrix_scale(textsprite, value, FLOAT_NaN);
            break;
        case SPRITE_PROP_SCALE_Y:
            textsprite_matrix_scale(textsprite, FLOAT_NaN, value);
            break;
        case SPRITE_PROP_SKEW_X:
            textsprite_matrix_skew(textsprite, value, FLOAT_NaN);
            break;
        case SPRITE_PROP_SKEW_Y:
            textsprite_matrix_skew(textsprite, FLOAT_NaN, value);
            break;
        case SPRITE_PROP_TRANSLATE_X:
            textsprite_matrix_translate(textsprite, value, FLOAT_NaN);
            break;
        case SPRITE_PROP_TRANSLATE_Y:
            textsprite_matrix_translate(textsprite, FLOAT_NaN, value);
            break;
        case SPRITE_PROP_ALPHA:
            textsprite->fontparams.tint_color[3] = math2d_clamp_float(value, 0.0f, 1.0f);
            break;
        case SPRITE_PROP_Z:
            textsprite->z = value;
            break;
        case SPRITE_PROP_VERTEX_COLOR_R:
            textsprite->fontparams.tint_color[0] = math2d_clamp_float(value, 0.0f, 1.0f);
            break;
        case SPRITE_PROP_VERTEX_COLOR_G:
            textsprite->fontparams.tint_color[1] = math2d_clamp_float(value, 0.0f, 1.0f);
            break;
        case SPRITE_PROP_VERTEX_COLOR_B:
            textsprite->fontparams.tint_color[2] = math2d_clamp_float(value, 0.0f, 1.0f);
            break;
        case SPRITE_PROP_ANIMATIONLOOP:
            if (textsprite->animation_selected)
                animsprite_set_loop(textsprite->animation_selected, (int32_t)value);
            break;
        case SPRITE_PROP_ROTATE_PIVOT_ENABLED:
            textsprite_matrix_rotate_pivot_enable(textsprite, value >= 1.0f);
            break;
        case SPRITE_PROP_ROTATE_PIVOT_U:
            textsprite_matrix_rotate_pivot(textsprite, value, FLOAT_NaN);
            break;
        case SPRITE_PROP_ROTATE_PIVOT_V:
            textsprite_matrix_rotate_pivot(textsprite, FLOAT_NaN, value);
            break;
        case SPRITE_PROP_SCALE_DIRECTION_X:
            textsprite->matrix_source.scale_direction_x = value;
            break;
        case SPRITE_PROP_SCALE_DIRECTION_Y:
            textsprite->matrix_source.scale_direction_y = value;
            break;
        case SPRITE_PROP_TRANSLATE_ROTATION:
            textsprite->matrix_source.translate_rotation = value >= 1.0f;
            break;
        case SPRITE_PROP_SCALE_SIZE:
            textsprite->matrix_source.scale_size = value >= 1.0f;
            break;
        case SPRITE_PROP_SCALE_TRANSLATION:
            textsprite->matrix_source.scale_translation = value >= 1.0f;
            break;
        case SPRITE_PROP_Z_OFFSET:
            textsprite->z_offset = value;
            break;
        case TEXTSPRITE_PROP_PARAGRAPH_SEPARATION:
            textsprite->fontparams.paragraph_space = value;
            break;
        case SPRITE_PROP_ANTIALIASING:
            textsprite->antialiasing = (PVRFlag)(int)value;
            break;
        case FONT_PROP_WORDBREAK:
            textsprite->wordbreak = (int32_t)value;
            break;
        case TEXTSPRITE_PROP_BACKGROUND_ENABLED:
            textsprite->background_enabled = value >= 1.0f;
            break;
        case TEXTSPRITE_PROP_BACKGROUND_SIZE:
            textsprite->background_size = value;
            break;
        case TEXTSPRITE_PROP_BACKGROUND_OFFSET_X:
            textsprite->background_offset_x = value;
            break;
        case TEXTSPRITE_PROP_BACKGROUND_OFFSET_Y:
            textsprite->background_offset_y = value;
            break;
        case TEXTSPRITE_PROP_BACKGROUND_COLOR_R:
            textsprite->background_rgba[0] = value;
            break;
        case TEXTSPRITE_PROP_BACKGROUND_COLOR_G:
            textsprite->background_rgba[1] = value;
            break;
        case TEXTSPRITE_PROP_BACKGROUND_COLOR_B:
            textsprite->background_rgba[2] = value;
            break;
        case TEXTSPRITE_PROP_BACKGROUND_COLOR_A:
            textsprite->background_rgba[3] = value;
            break;
        case SPRITE_PROP_ALPHA2:
            textsprite->alpha2 = value;
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
        case FONT_PROP_WORDBREAK:
        case TEXTSPRITE_PROP_FONT_SIZE:
            textsprite->modified_coords = true;
            break;
    }
}


void textsprite_get_draw_size(TextSprite textsprite, float* draw_width, float* draw_height) {
    textsprite_calculate_paragraph_alignment(textsprite);
    *draw_width = textsprite->last_draw_width;
    *draw_height = textsprite->last_draw_height;
}

float textsprite_get_font_size(TextSprite textsprite) {
    return textsprite->fontparams.height;
}

void textsprite_get_max_draw_size(TextSprite textsprite, float* max_draw_width, float* draw_height_max) {
    *max_draw_width = textsprite->max_width;
    *draw_height_max = textsprite->max_height;
}

void textsprite_draw(TextSprite textsprite, PVRContext pvrctx) {
    if (textsprite->fontparams.tint_color[3] <= 0.0f) return;
    if (!textsprite->text_forced_case && !*textsprite->text) return;

    // check if all calculations are up-to-date
    if (textsprite->modified_string || textsprite->modified_coords) textsprite_draw_calc(textsprite);

    textsprite_draw_internal(textsprite, pvrctx);
}


static void textsprite_draw_internal(TextSprite textsprite, PVRContext pvrctx) {
    pvr_context_save(pvrctx);
    pvr_context_set_global_alpha(pvrctx, textsprite->alpha2);

    // if (textsprite->psshader) pvr_context_add_shader(pvrctx, textsprite->psshader);

    pvr_context_set_vertex_blend(
        pvrctx,
        textsprite->blend_enabled,
        textsprite->blend_src_rgb,
        textsprite->blend_dst_rgb,
        textsprite->blend_src_alpha,
        textsprite->blend_dst_alpha
    );

    if (textsprite->antialiasing != PVRCTX_FLAG_DEFAULT) {
        pvr_context_set_global_antialiasing(pvrctx, textsprite->antialiasing);
    }

    textsprite_matrix_calculate(textsprite, pvrctx);

    const char* text = textsprite->text_forced_case ? textsprite->text_forced_case : *textsprite->text;
    size_t text_length = strlen(text);

    if (textsprite->background_enabled) {
        float size = textsprite->background_size * 2.0f;
        float x = textsprite->last_draw_x - textsprite->background_size + textsprite->background_offset_x;
        float y = textsprite->last_draw_y - textsprite->background_size + textsprite->background_offset_y;
        float width = textsprite->last_draw_width + size;
        float height = textsprite->last_draw_height + size;

        pvr_context_set_vertex_alpha(pvrctx, textsprite->background_rgba[3]);
        pvr_context_draw_solid_color(pvrctx, textsprite->background_rgba, x, y, width, height);
    }

    if (arraylist_size(textsprite->paragraph_array) < 2) {
        // let the font handle the draw
        textsprite->fontfunctions.draw_text(
            textsprite->font,
            pvrctx,
            &textsprite->fontparams,
            textsprite->last_draw_x, textsprite->last_draw_y,
            0, text_length, text
        );
    } else {
        // paragraph by paragraph draw
        float y = textsprite->last_draw_y;
        float line_height = textsprite->fontparams.height + textsprite->fontparams.paragraph_space;

        foreach (ParagraphInfo*, paragraphinfo, ARRAYLIST_ITERATOR, textsprite->paragraph_array) {
            if (paragraphinfo->length > 0) {
                textsprite->fontfunctions.draw_text(
                    textsprite->font, pvrctx, &textsprite->fontparams,
                    textsprite->last_draw_x + paragraphinfo->offset, y,
                    paragraphinfo->index, (size_t)paragraphinfo->length, text
                );
            }
            y += line_height;
        }
    }

    pvr_context_restore(pvrctx);
}

static void textsprite_draw_calc(TextSprite textsprite) {
    // step 1: calculate the draw size (lines and the most wide paragraph)
    float draw_width = 0.0f, draw_height = 0.0f;
    textsprite_get_draw_size(textsprite, &draw_width, &draw_height);

    // step 2: calculate the alignment
    float offset_x = 0.0f;
    float offset_y = 0.0f;
    if (textsprite->max_height >= 0.0f && textsprite->align_vertical != ALIGN_START) {
        offset_y = textsprite->max_height - draw_height;
        if (textsprite->align_vertical == ALIGN_CENTER) offset_y /= 2.0f;
    }
    if (textsprite->max_width >= 0.0f && textsprite->align_horizontal != ALIGN_START) {
        offset_x = textsprite->max_width - draw_width;
        if (textsprite->align_horizontal == ALIGN_CENTER) offset_x /= 2.0f;
    }

    // step 3: cache the drawing coordinates
    textsprite->last_draw_x = textsprite->x + offset_x;
    textsprite->last_draw_y = textsprite->y + offset_y;
    textsprite->modified_coords = false;
    textsprite->modified_string = false;
}

AnimSprite textsprite_animation_set(TextSprite textsprite, AnimSprite animsprite) {
    AnimSprite old_animation = textsprite->animation_selected;
    textsprite->animation_selected = animsprite;
    return old_animation;
}

void textsprite_animation_restart(TextSprite textsprite) {
    if (textsprite->animation_selected) animsprite_restart(textsprite->animation_selected);
}

void textsprite_animation_end(TextSprite textsprite) {
    if (!textsprite->animation_selected) return;
    animsprite_force_end(textsprite->animation_selected);
    animsprite_update_textsprite(textsprite->animation_selected, textsprite, true);
}

void textsprite_animation_stop(TextSprite textsprite) {
    if (textsprite->animation_selected) animsprite_stop(textsprite->animation_selected);
}


void textsprite_animation_external_set(TextSprite textsprite, AnimSprite animsprite) {
    textsprite->animation_external = animsprite;
}

void textsprite_animation_external_end(TextSprite textsprite) {
    if (textsprite->animation_external) {
        animsprite_force_end(textsprite->animation_external);
        animsprite_update_textsprite(textsprite->animation_external, textsprite, true);
    }
}

void textsprite_animation_external_restart(TextSprite textsprite) {
    if (textsprite->animation_external) animsprite_restart(textsprite->animation_external);
}


int32_t textsprite_animate(TextSprite textsprite, float elapsed) {
    int32_t result = 1;

    if (textsprite->animation_selected) {
        result = animsprite_animate(textsprite->animation_selected, elapsed);
        animsprite_update_textsprite(textsprite->animation_selected, textsprite, true);
    }

    if (textsprite->animation_external) {
        result = animsprite_animate(textsprite->animation_external, elapsed);
        animsprite_update_textsprite(textsprite->animation_external, textsprite, false);
    }

    if (textsprite->modified_string && textsprite->modified_coords) textsprite_draw_calc(textsprite);

    return result;
}


void textsprite_border_enable(TextSprite textsprite, bool enable) {
    textsprite->fontparams.border_enable = !!enable;
}

void textsprite_border_set_size(TextSprite textsprite, float border_size) {
    textsprite->fontparams.border_size = math2d_is_float_NaN(border_size) ? 0.0f : border_size;
}

void textsprite_border_set_color(TextSprite textsprite, float r, float g, float b, float a) {
    if (r >= 0.0f) textsprite->fontparams.border_color[0] = r;
    if (g >= 0.0f) textsprite->fontparams.border_color[1] = g;
    if (b >= 0.0f) textsprite->fontparams.border_color[2] = b;
    if (a >= 0.0f) textsprite->fontparams.border_color[3] = a;
}

void textsprite_border_set_color_rgba8(TextSprite textsprite, uint32_t rbga8_color) {
    math2d_color_bytes_to_floats(rbga8_color, true, textsprite->fontparams.border_color);
}

void textsprite_border_set_offset(TextSprite textsprite, float x, float y) {
    if (!math2d_is_float_NaN(x)) textsprite->fontparams.border_offset_x = x;
    if (!math2d_is_float_NaN(y)) textsprite->fontparams.border_offset_y = y;
}


bool textsprite_is_visible(TextSprite textsprite) {
    return textsprite->visible;
}

void textsprite_set_antialiasing(TextSprite textsprite, PVRFlag antialiasing) {
    textsprite->antialiasing = antialiasing;
}

void textsprite_set_wordbreak(TextSprite textsprite, int32_t wordbreak) {
    textsprite->wordbreak = wordbreak;
    textsprite->modified_coords = true;
}

bool textsprite_has_font(TextSprite textsprite) {
    return textsprite->font != NULL;
}

void textsprite_change_font(TextSprite textsprite, FontHolder fontholder) {
    if (fontholder == NULL) {
        logger_error("fontholder can not be NULL");
        assert(fontholder);
    }

    textsprite->font = fontholder->font;
    textsprite->fontparams.color_by_addition = fontholder->font_color_by_addition;

    bool font_is_truetype = !fontholder->font_from_atlas;

    if (font_is_truetype) {
        textsprite->fontfunctions.measure = (MeasureFn)fonttype_measure;
        textsprite->fontfunctions.measure_char = (MeasureCharFn)fonttype_measure_char;
        textsprite->fontfunctions.draw_text = (DrawTextFn)fonttype_draw_text;
        textsprite->fontfunctions.animate = (AnimateFn)fonttype_animate;
        textsprite->fontfunctions.map_codepoints = (MapCodepointsFn)fonttype_map_codepoints;
    } else {
        textsprite->fontfunctions.measure = (MeasureFn)fontglyph_measure;
        textsprite->fontfunctions.measure_char = (MeasureCharFn)fontglyph_measure_char;
        textsprite->fontfunctions.draw_text = (DrawTextFn)fontglyph_draw_text;
        textsprite->fontfunctions.animate = (AnimateFn)fontglyph_animate;
        textsprite->fontfunctions.map_codepoints = (MapCodepointsFn)fontglyph_map_codepoints;
    }
}

/*void textsprite_set_shader(TextSprite textsprite,PSShader psshader) {
    textsprite->psshader = psshader;
}

PSShader textsprite_get_shader(TextSprite textsprite) {
    return textsprite->psshader;
}*/

void textsprite_blend_enable(TextSprite textsprite, bool enabled) {
    textsprite->blend_enabled = enabled;
}

void textsprite_blend_set(TextSprite textsprite, Blend src_rgb, Blend dst_rgb, Blend src_alpha, Blend dst_alpha) {
    textsprite->blend_src_rgb = src_rgb;
    textsprite->blend_dst_rgb = dst_rgb;
    textsprite->blend_src_alpha = src_alpha;
    textsprite->blend_dst_alpha = dst_alpha;
}

void textsprite_background_enable(TextSprite textsprite, bool enabled) {
    textsprite->background_enabled = enabled;
}

void textsprite_background_set_size(TextSprite textsprite, float size) {
    textsprite->background_size = size;
}

void textsprite_background_set_offets(TextSprite textsprite, float offset_x, float offset_y) {
    if (!math2d_is_float_NaN(offset_x)) textsprite->background_offset_x = offset_x;
    if (!math2d_is_float_NaN(offset_y)) textsprite->background_offset_y = offset_y;
}

void textsprite_background_set_color(TextSprite textsprite, float r, float g, float b, float a) {
    if (!math2d_is_float_NaN(r)) textsprite->background_rgba[0] = r;
    if (!math2d_is_float_NaN(g)) textsprite->background_rgba[1] = g;
    if (!math2d_is_float_NaN(b)) textsprite->background_rgba[2] = b;
    if (!math2d_is_float_NaN(a)) textsprite->background_rgba[3] = a;
}

const char* const* textsprite_get_string(TextSprite textsprite) {
    if (textsprite->text_forced_case)
        return (const char* const*)&textsprite->text_forced_case;
    else
        return (const char* const*)textsprite->text;
}

void textsprite_enable_color_by_addition(TextSprite textsprite, bool enabled) {
    textsprite->fontparams.color_by_addition = enabled;
}
