#include "fontglyph.h"

#include "float64.h"
#include "fontholder.h"
#include "game/common/funkin.h"
#include "glyphrenderer.h"
#include "malloc_utils.h"
#include "stringutils.h"


typedef struct {
    float x;
    float y;
    float width;
    float height;
    float frame_x;
    float frame_y;
    float frame_width;
    float frame_height;
    float pivot_x;
    float pivot_y;
    float glyph_width_ratio;
} GlyphFrame;

typedef struct {
    GlyphFrame* frames;
    int32_t frames_size;
    uint32_t code;
    int32_t actual_frame;
} GlyphInfo;

struct FontGlyph_s {
    Texture texture;
    GlyphFrame* frames_array;
    GlyphInfo* table;
    int32_t table_size;
    float64 frame_time;
    float64 frame_progress;
    float space_scale;
    uint8_t lookup_table[FONTGLYPH_LOOKUP_TABLE_LENGTH];
};


static const float FONTGLYPH_SPACE_WIDTH_RATIO = 0.625f;


static bool fontglyph_internal_parse(const AtlasEntry* atlas_entry, const char* match_suffix, size_t match_suffix_length, bool allow_animation, GlyphInfo* table, int32_t table_index);
static void fontglyph_internal_add_frame(const AtlasEntry* atlas_entry, GlyphInfo* glyph);
static void fontglyph_internal_calc_space_scale(FontGlyph fontglyph);
static int fontglyph_internal_table_sort(const void* x, const void* y);


FontGlyph fontglyph_init(const char* src_atlas, const char* suffix, bool allow_animation) {
    Atlas atlas = atlas_init(src_atlas);
    if (!atlas || !atlas->texture_filename) {
        if (atlas) atlas_destroy(&atlas);
        logger_error("fontglyph_init() missing atlas file or texture filename not specified on %s", src_atlas);
        return NULL;
    }

    const char* texture_path = atlas_get_texture_path(atlas);
    FontGlyph fontglyph;

    Texture texture = texture_init(texture_path);
    if (texture) {
        fontglyph = fontglyph_init2(texture, atlas, suffix, allow_animation);
        if (!fontglyph) logger_warn("fontglyph_init() failed for %s", src_atlas);
    } else {
        fontglyph = NULL;
        logger_error("fontglyph_init() texture specified by atlas not found: %s", texture_path);
    }

    atlas_destroy(&atlas);

    return fontglyph;
}

FontGlyph fontglyph_init2(Texture texture, Atlas atlas, const char* suffix, bool allow_animation) {
#ifdef DEBUG
    assert(atlas->size >= 0);
#endif

    FontGlyph fontglyph = malloc_chk(sizeof(struct FontGlyph_s));
    malloc_assert(fontglyph, FontGlyph);

    *fontglyph = (struct FontGlyph_s){
        .texture = texture,
        .frames_array = NULL,
        .table = calloc_for_array(atlas->size, GlyphInfo),
        .table_size = atlas->size, // temporal value

        .frame_time = 0.0,
        .frame_progress = 0.0,
        .space_scale = FONTGLYPH_SPACE_WIDTH_RATIO,
    };

    memset(fontglyph->lookup_table, (uint8_t)FONTGLYPH_LOOKUP_TABLE_LENGTH, FONTGLYPH_LOOKUP_TABLE_LENGTH);

    if (allow_animation) {
        if (atlas->glyph_fps > 0.0f)
            fontglyph->frame_time = atlas->glyph_fps;
        else
            fontglyph->frame_time = FUNKIN_DEFAULT_ANIMATIONS_FRAMERATE;

        fontglyph->frame_time = 1000.0 / fontglyph->frame_time;
    }

    int32_t table_index = 0;
    size_t suffix_length = suffix ? strlen(suffix) : 0;

    // calculate the amount of matching glyphs in the atlas
    for (int32_t i = 0; i < atlas->size; i++) {
        bool result = fontglyph_internal_parse(
            &atlas->entries[i], suffix, suffix_length, allow_animation, fontglyph->table, table_index
        );
        if (result) table_index++;
    }

    if (table_index < 1) {
        logger_warn("fontglyph_init2() failed, there no usable glyphs in the atlas suffix=%s", suffix);
        free_chk(fontglyph->table);
        free_chk(fontglyph);
        return NULL;
    }

    // shrink the table if necessary
    if (table_index < fontglyph->table_size) {
        fontglyph->table_size = table_index;
        fontglyph->table = realloc_for_array(fontglyph->table, table_index, GlyphInfo);
    }

    // count frames of every added glpyh
    int32_t frame_total = 0;
    for (int32_t i = 0; i < fontglyph->table_size; i++) {
        frame_total += fontglyph->table[i].frames_size;
    }

    // allocate frames array and set to zero each glyph frame count
    fontglyph->frames_array = malloc_for_array(GlyphFrame, frame_total);
    for (int32_t i = 0, j = 0; i < fontglyph->table_size; i++) {
        int32_t frames_size = fontglyph->table[i].frames_size;
        if (frames_size > 0) {
            fontglyph->table[i].frames = &fontglyph->frames_array[j];
            fontglyph->table[i].frames_size = 0;
            j += frames_size;
        }
    }

    // add frames to each glyph
    for (int32_t i = 0; i < atlas->size; i++) {
        fontglyph_internal_parse(
            &atlas->entries[i], suffix, suffix_length, allow_animation, fontglyph->table, table_index
        );
    }

    // sort table, place ascii characters first
    qsort(fontglyph->table, (size_t)fontglyph->table_size, sizeof(GlyphInfo), fontglyph_internal_table_sort);

    // populate lookup table
    for (uint8_t i = 0; i < fontglyph->table_size && i <= FONTGLYPH_LOOKUP_TABLE_LENGTH; i++) {
        uint32_t code = fontglyph->table[i].code;
        if (code < FONTGLYPH_LOOKUP_TABLE_LENGTH) {
            fontglyph->lookup_table[code] = i;
        }
    }

    // find space width scale (normally only applies to the first frame)
    fontglyph_internal_calc_space_scale(fontglyph);

    return fontglyph;
}

void fontglyph_destroy(FontGlyph* fontglyph_ptr) {
    FontGlyph fontglyph = *fontglyph_ptr;
    if (!fontglyph) return;

    free_chk(fontglyph->frames_array);
    free_chk(fontglyph->table);

    texture_destroy(&fontglyph->texture);

    free_chk(fontglyph);
    *fontglyph_ptr = NULL;
}


void fontglyph_measure(FontGlyph fontglyph, FontParams* params, const char* text, int32_t text_index, size_t text_length, FontLinesInfo* lines_info) {
    Grapheme grapheme = {.code = 0, .size = 0};
    const size_t text_end_index = (size_t)text_index + text_length;

    float width = 0.0f;
    float max_width = 0.0f;
    int32_t line_chars = 0;
    int32_t lines = 1;

    for (int32_t i = text_index; i < text_end_index; i++) {
        if (!string_get_character_codepoint(text, i, text_end_index, &grapheme)) continue;
        i += grapheme.size - 1;

        if (grapheme.code == FONTGLYPH_LINEFEED) {
            if (width > max_width) max_width = width;
            width = 0.0f;
            line_chars = 0;
            lines++;
            continue;
        }

        GlyphInfo* info = NULL;

        if (grapheme.code < FONTGLYPH_LOOKUP_TABLE_LENGTH) {
            if (fontglyph->lookup_table[grapheme.code] != FONTGLYPH_LOOKUP_TABLE_LENGTH) {
                uint8_t index = fontglyph->lookup_table[grapheme.code];
                info = &fontglyph->table[index];
                goto L_measure;
            }
        }

        for (int32_t j = 0; j < fontglyph->table_size; j++) {
            if (fontglyph->table[j].code == grapheme.code) {
                info = &fontglyph->table[j];
                goto L_measure;
            }
        }

        if (grapheme.code == FONTGLYPH_TAB) {
            int32_t filler = fontglyph_internal_calc_tabstop(line_chars);
            if (filler > 0) {
                width += fontglyph->space_scale * filler;
                line_chars += filler;
            }
        } else {
            // space, hard space or unknown character
            width += params->height * fontglyph->space_scale;
            line_chars++;
        }

        continue;

    L_measure:
        GlyphFrame* frame = &info->frames[info->actual_frame];
        width += frame->glyph_width_ratio * params->height;
        line_chars++;
    }

    lines_info->max_width = math2d_max_float(width, max_width);
    lines_info->total_height = (params->height + params->paragraph_space) * lines;
}

void fontglyph_measure_char(FontGlyph fontglyph, uint32_t codepoint, float height, FontCharInfo* char_info) {
    GlyphInfo* info;

    char_info->last_char_height = height;
    char_info->previous_codepoint = codepoint;
    char_info->last_char_width_end = 0.0f;

    if (codepoint < FONTGLYPH_LOOKUP_TABLE_LENGTH) {
        if (fontglyph->lookup_table[codepoint] != FONTGLYPH_LOOKUP_TABLE_LENGTH) {
            uint8_t index = fontglyph->lookup_table[codepoint];
            info = &fontglyph->table[index];
            goto L_measure;
        }
    }

    for (int32_t i = 0; i < fontglyph->table_size; i++) {
        if (fontglyph->table[i].code == codepoint) {
            info = &fontglyph->table[i];
            goto L_measure;
        }
    }

    if (codepoint == FONTGLYPH_TAB) {
        int32_t filler = fontglyph_internal_calc_tabstop(char_info->line_char_count);
        if (filler > 0) {
            char_info->last_char_width = height * fontglyph->space_scale * filler;
            char_info->line_char_count += filler;
        }
    } else {
        // space, hard space or unknown character
        char_info->last_char_width = height * fontglyph->space_scale;
        char_info->line_char_count++;
    }

    return;

L_measure:
    GlyphFrame* frame = &info->frames[info->actual_frame];
    char_info->last_char_width = frame->glyph_width_ratio * height;
    char_info->line_char_count++;
}

float fontglyph_measure_line_height(FontGlyph fontglyph, float height) {
    (void)fontglyph;
    return height;
}

float fontglyph_draw_text(FontGlyph fontglyph, PVRContext pvrctx, FontParams* params, float x, float y, int32_t text_index, size_t text_length, const char* text) {
    Grapheme grapheme = {.code = 0, .size = 0};
    const bool by_add = params->color_by_addition;
    const bool has_border = params->border_enable && params->border_color[3] > 0.0f && params->border_size >= 0.0f;
    const float outline_size = params->border_size * 2.0f;
    const size_t text_end_index = (size_t)text_index + text_length;

    float draw_x = 0.0f;
    float draw_y = 0.0f;
    int32_t index = text_index;
    int32_t total_glyphs = 0;
    int32_t line_chars = 0;
    int32_t lines = 1;

    texture_upload_to_pvr(fontglyph->texture);
    pvr_context_save(pvrctx);
    pvr_context_set_vertex_alpha(pvrctx, params->tint_color[3]);

    // count required glyphs
    while (index < text_end_index && string_get_character_codepoint(text, index, text_end_index, &grapheme)) {
        index += grapheme.size;

        switch (grapheme.code) {
            case FONTGLYPH_CARRIAGERETURN:
            case FONTGLYPH_LINEFEED:
                continue;
        }

        if (grapheme.code < FONTGLYPH_LOOKUP_TABLE_LENGTH && fontglyph->lookup_table[grapheme.code] != FONTGLYPH_LOOKUP_TABLE_LENGTH) {
            total_glyphs++;
            continue;
        }

        for (int32_t i = 0; i < fontglyph->table_size; i++) {
            if (fontglyph->table[i].code == grapheme.code) {
                total_glyphs++;
                break;
            }
        }
    }

    // prepare vertex buffer
    if (has_border) total_glyphs *= 2;
    int32_t added = 0;
    int32_t maximum = glyphrenderer_prepare(
        pvrctx, total_glyphs, has_border,
        params->tint_color, params->border_color,
        by_add, false,
        fontglyph->texture, NULL
    );

    index = text_index;
    while (added < maximum && index < text_end_index && string_get_character_codepoint(text, index, text_end_index, &grapheme)) {
        index += grapheme.size;

        // ignore "\r" characters
        if (grapheme.code == FONTGLYPH_CARRIAGERETURN) continue;

        if (grapheme.code == FONTGLYPH_LINEFEED) {
            draw_y += params->height + params->paragraph_space;
            draw_x = 0.0f;
            line_chars = 0;
            lines++;
            continue;
        }

        GlyphFrame* frame = NULL;

        if (grapheme.code < FONTGLYPH_LOOKUP_TABLE_LENGTH && fontglyph->lookup_table[grapheme.code] != FONTGLYPH_LOOKUP_TABLE_LENGTH) {
            uint8_t in_table_index = fontglyph->lookup_table[grapheme.code];
            frame = &fontglyph->table[in_table_index].frames[fontglyph->table[in_table_index].actual_frame];
        } else {
            for (int32_t i = 0; i < fontglyph->table_size; i++) {
                if (fontglyph->table[i].code == grapheme.code) {
                    frame = &fontglyph->table[i].frames[fontglyph->table[i].actual_frame];
                    break;
                }
            }
        }

        if (!frame) {
            if (grapheme.code == FONTGLYPH_TAB) {
                int32_t filler = fontglyph_internal_calc_tabstop(line_chars);
                if (filler > 0) {
                    draw_x += params->height * fontglyph->space_scale * filler;
                    line_chars += filler;
                }
            } else {
                // space, hard space or unknown characters
                draw_x += params->height * fontglyph->space_scale;
                line_chars++;
            }
            continue;
        }

        // calculate glyph bounds
        float ratio_width, ratio_height;
        float dx = x + draw_x;
        float dy = y + draw_y;
        float dh = params->height;
        float dw = dh * frame->glyph_width_ratio;

        if (frame->frame_width > 0.0f) {
            ratio_width = dw / frame->frame_width;
            dw = frame->width * ratio_width;
        } else {
            ratio_width = dw / frame->width;
        }

        if (frame->frame_height > 0.0f) {
            ratio_height = dh / frame->frame_height;
            dh = frame->height * ratio_height;
        } else {
            ratio_height = dh / frame->height;
        }

        dx += (frame->pivot_x + frame->frame_x) * ratio_width;
        dy += (frame->pivot_y + frame->frame_y) * ratio_height;

        if (has_border) {
            float sdx = dx - params->border_size;
            float sdy = dy - params->border_size;
            float sdw = dw + outline_size;
            float sdh = dh + outline_size;

            sdx += params->border_offset_x;
            sdy += params->border_offset_y;

            glyphrenderer_draw_glyph(
                false, true,
                frame->x, frame->y, frame->width, frame->height,
                sdx, sdy, sdw, sdh
            );
        }

        glyphrenderer_draw_glyph(
            false, false,
            frame->x, frame->y, frame->width, frame->height,
            dx, dy, dw, dh
        );

        draw_x += dw;
        line_chars++;
    }

    pvr_context_restore(pvrctx);

    return (params->height + params->paragraph_space) * lines;
}

int32_t fontglyph_animate(FontGlyph fontglyph, float elapsed) {
    if (fontglyph->frame_time <= 0.0f) return 1;

    int32_t frame_index = (int32_t)(fontglyph->frame_progress / fontglyph->frame_time);

    for (int32_t i = 0; i < fontglyph->table_size; i++) {
        if (fontglyph->table[i].frames_size < 2) continue;
        fontglyph->table[i].actual_frame = frame_index % fontglyph->table[i].frames_size;
    }

    fontglyph_internal_calc_space_scale(fontglyph);

    fontglyph->frame_progress += elapsed;
    return 0;
}

void fontglyph_map_codepoints(FontGlyph fontglyph, const char* text, int32_t text_index, size_t text_length) {
    (void)fontglyph;
    (void)text_index;
    (void)text_length;
    (void)text;
    // unused
}


static bool fontglyph_internal_parse(const AtlasEntry* atlas_entry, const char* match_suffix, size_t match_suffix_length, bool allow_animation, GlyphInfo* table, int32_t table_index) {
    const char* atlas_entry_name = atlas_entry->name;
    size_t atlas_entry_name_length = strlen(atlas_entry_name);
    Grapheme grapheme = {.code = 0, .size = 0};

    // read character info
    if (!string_get_character_codepoint(atlas_entry_name, 0, atlas_entry_name_length, &grapheme)) {
        // eof reached
        return false;
    }

    uint8_t index = grapheme.size;

    if (match_suffix) {
        size_t number_suffix_start = index + match_suffix_length + 1;

        if (number_suffix_start > atlas_entry_name_length) {
            // suffix not present
            return false;
        }

        switch (atlas_entry_name[index]) {
            // case FONTGLYPH_HARDSPACE:
            case FONTGLYPH_SPACE:
                index++;
                break;
            default:
                // suffix not present
                return false;
        }

        // check if the suffix matchs
        if (!string_starts_with(atlas_entry_name + index, match_suffix)) {
            // suffix not present
            return false;
        }

        index += match_suffix_length;
    }

    // check if this atlas entry is an animation frame
    if (index < atlas_entry_name_length && !atlas_name_has_number_suffix(atlas_entry_name, index)) {
        // missing number suffix
        return false;
    }

    // check if already exists an entry with this unicode code point
    int32_t codepoint_index = -1;
    for (int32_t i = 0; i < table_index; i++) {
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
        table[table_index] = (GlyphInfo){
            .code = grapheme.code, .actual_frame = 0, .frames = NULL, .frames_size = 0
        };
    } else {
        table_index = codepoint_index;
    }

    fontglyph_internal_add_frame(atlas_entry, &table[table_index]);
    table[table_index].frames_size++;

    // returns true if an entry was added to the table
    return codepoint_index < 0;
}

static void fontglyph_internal_add_frame(const AtlasEntry* atlas_entry, GlyphInfo* glyph_info) {
    if (!glyph_info->frames) return;

    float height = atlas_entry->frame_height > 0.0f ? atlas_entry->frame_height : atlas_entry->height;
    float glyph_width_ratio = 0.0f;
    if (height > 0.0f) {
        // cache this frame width
        float width = atlas_entry->frame_width > 0.0f ? atlas_entry->frame_width : atlas_entry->width;
        glyph_width_ratio = width / height;
    }

    glyph_info->frames[glyph_info->frames_size] = (GlyphFrame){
        .x = atlas_entry->x,
        .y = atlas_entry->y,
        .width = atlas_entry->width,
        .height = atlas_entry->height,
        .frame_x = atlas_entry->frame_x,
        .frame_y = atlas_entry->frame_y,
        .frame_width = atlas_entry->frame_width,
        .frame_height = atlas_entry->frame_height,
        .pivot_x = atlas_entry->pivot_x,
        .pivot_y = atlas_entry->pivot_y,

        .glyph_width_ratio = glyph_width_ratio
    };
}

int32_t fontglyph_internal_calc_tabstop(int32_t characters_in_the_line) {
    int32_t space = characters_in_the_line % FONTGLYPH_TABSTOP;
    if (space == 0) return 0;
    return FONTGLYPH_TABSTOP - space;
}

static void fontglyph_internal_calc_space_scale(FontGlyph fontglyph) {
    for (int32_t i = 0; i < fontglyph->table_size; i++) {
        if (fontglyph->table[i].code == FONTGLYPH_SPACE || fontglyph->table[i].code == FONTGLYPH_HARDSPACE) {
            const GlyphFrame* frame = &fontglyph->table[i].frames[fontglyph->table[i].actual_frame];
            fontglyph->space_scale = frame->glyph_width_ratio;
            break;
        }
    }
}

static int fontglyph_internal_table_sort(const void* x, const void* y) {
    const GlyphInfo* x_ptr = (const GlyphInfo*)x;
    const GlyphInfo* y_ptr = (const GlyphInfo*)y;

    const uint32_t value_x = x_ptr->code;
    const uint32_t value_y = y_ptr->code;

    if (value_x < value_y)
        return -1;
    if (value_x > value_y)
        return 1;
    else
        return 0;
}
