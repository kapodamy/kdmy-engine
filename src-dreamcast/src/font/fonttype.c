#include "fonttype.h"

#include <kos/mutex.h>

#include "color2d.h"
#include "externals/fontatlas.h"
#include "float64.h"
#include "fs.h"
#include "glyphrenderer.h"
#include "malloc_utils.h"
#include "map.h"
#include "number_format_specifiers.h"
#include "pvrcontext.h"
#include "stringutils.h"

typedef struct {
#if !defined(_arch_dreamcast) && defined(SDF_FONT)
    FontCharMap* map_outline;
    Texture texture_outline;
#endif
    FontCharMap* map;
    Texture texture;
} FCAtlas;

struct FontType_s {
    int32_t instance_id;
    int32_t instance_references;
    char* instance_path;
    ArrayBuffer font;
    FontAtlas fontatlas;
    float space_width;
    FCAtlas atlas_primary;
    FCAtlas atlas_secondary;
    uint8_t lookup_table[FONTGLYPH_LOOKUP_TABLE_LENGTH];
};


#if !defined(_arch_dreamcast) && defined(SDF_FONT)
static const uint8_t FONTTYPE_GLYPHS_HEIGHT = 72; // 72px is enough for SDF
static const int8_t FONTTYPE_GLYPHS_GAP = 16;     // space between glyphs in pixels (must be high)
static const uint8_t FONTTYPE_GLYPHS_OUTLINE_HEIGHT = GLYPHS_HEIGHT >>> 1;
static const int8_t FONTTYPE_GLYPHS_OUTLINE_GAP = GLYPHS_GAP >>> 1;
static const uint8_t FONTTYPE_GLYPHS_OUTLINE_RATIO = GLYPHS_OUTLINE_HEIGHT >>> 2; // 25% of FONTTYPE_GLYPHS_OUTLINE_HEIGHT
static const float FONTTYPE_GLYPHS_OUTLINE_THICKNESS = 1.25f;                     // 125%
#else
static const uint8_t FONTTYPE_GLYPHS_HEIGHT = 42;
static const int8_t FONTTYPE_GLYPHS_GAP = 4; // space between glyphs in pixels
#endif
static const float FONTTYPE_FAKE_SPACE = 0.9f; // 90% of the height


static Map FONTTYPE_POOL = NULL;
static int32_t FONTTYPE_IDS = 0;
static mutex_t fonttype_fontatlas_mutex = MUTEX_INITIALIZER;


void __attribute__((constructor)) __ctor_fonttype() {
    FONTTYPE_POOL = map_init();
}


static bool fonttype_internal_init_freetype(FontType fonttype, const char* src);
static FontCharMap* fonttype_internal_create_fontcharmap(FontType fonttype, uint32_t* characters_map, uint8_t glyphs_height, int8_t glyphs_gap);
static Texture fonttype_internal_upload_texture(FontCharMap* fontcharmap);
static int32_t fonttype_internal_get_fontchardata(const FCAtlas* atlas, uint32_t codepoint);
static inline int32_t fonttype_internal_get_fontchardata2(const uint8_t* lookup_table, const FCAtlas* atlas, uint32_t codepoint);
static void fonttype_internal_create_atlas(FontType fonttype, FCAtlas* atlas, uint32_t* codepoints);
static void fonttype_internal_destroy_atlas(FCAtlas* atlas);


FontType fonttype_init(const char* src) {
    char* full_path = fs_get_full_path_and_override(src);

    foreach (FontType, instance, MAP_ITERATOR, FONTTYPE_POOL) {
        if (string_equals_ignore_case(instance->instance_path, full_path)) {
            free_chk(full_path);
            instance->instance_references++;
            return instance;
        }
    }

    FontType fonttype = malloc_chk(sizeof(struct FontType_s));
    malloc_assert(fonttype, FontType);

    *fonttype = (struct FontType_s){
        .instance_id = FONTTYPE_IDS++,
        .instance_references = 1,
        .instance_path = full_path,

        .font = NULL,
        .fontatlas = NULL,
        .space_width = FONTTYPE_GLYPHS_HEIGHT * FONTTYPE_FAKE_SPACE,

#if !defined(_arch_dreamcast) && defined(SDF_FONT)
        .atlas_primary = {.map_outline = NULL, .texture_outline = NULL, .map = NULL, .texture = NULL},
        .atlas_secondary = {.map_outline = NULL, .texture_outline = NULL, .map = NULL, .texture = NULL},
#else
        .atlas_primary = {.map = NULL, .texture = NULL},
        .atlas_secondary = {.map = NULL, .texture = NULL},
#endif
    };

    memset(fonttype->lookup_table, (uint8_t)FONTGLYPH_LOOKUP_TABLE_LENGTH, FONTGLYPH_LOOKUP_TABLE_LENGTH);

    // initialize FreeType library
    if (fonttype_internal_init_freetype(fonttype, src)) {
        arraybuffer_destroy(&fonttype->font);
        free_chk(full_path);
        free_chk(fonttype);
        return NULL;
    }

    // create a texture atlas and glyphs map with all common letters, numbers and symbols
    fonttype_internal_create_atlas(fonttype, &fonttype->atlas_primary, NULL);

    if (fonttype->atlas_primary.map) {
        FontCharData* char_array = fonttype->atlas_primary.map->char_array;
        int32_t char_array_size = fonttype->atlas_primary.map->char_array_size;

        for (int32_t i = 0; i < char_array_size; i++) {
            if (char_array[i].codepoint == FONTGLYPH_SPACE) {
                fonttype->space_width = char_array[i].advancex;
                break;
            }
        }

        for (uint8_t i = 0; i < char_array_size && i <= FONTGLYPH_LOOKUP_TABLE_LENGTH; i++) {
            uint32_t codepoint = char_array[i].codepoint;
            if (codepoint < FONTGLYPH_LOOKUP_TABLE_LENGTH) {
                fonttype->lookup_table[codepoint] = i;
            }
        }
    }

    map_add(FONTTYPE_POOL, fonttype->instance_id, fonttype);
    return fonttype;
}

void fonttype_destroy(FontType* fonttype_ptr) {
    FontType fonttype = *fonttype_ptr;
    if (!fonttype) return;

    fonttype->instance_references--;
    if (fonttype->instance_references > 0) return;
    map_delete(FONTTYPE_POOL, fonttype->instance_id);

    fonttype_internal_destroy_atlas(&fonttype->atlas_primary);
    fonttype_internal_destroy_atlas(&fonttype->atlas_secondary);

    if (fonttype->fontatlas) fontatlas_destroy(&fonttype->fontatlas);
    arraybuffer_destroy(&fonttype->font);

    free_chk(fonttype->instance_path);

    free_chk(fonttype);
    *fonttype_ptr = NULL;
}


void fonttype_measure(FontType fonttype, FontParams* params, const char* text, int32_t text_index, size_t text_length, FontLinesInfo* info) {
    const float scale = params->height / FONTTYPE_GLYPHS_HEIGHT;
    const size_t text_end_index = (size_t)text_index + text_length;

    float max_width = 0.0f;
    float width = 0.0f;
    int32_t index = text_index;
    uint32_t previous_codepoint = 0;
    int32_t line_chars = 0;
    int32_t lines = 1;
    int32_t last_glyph_width_correction = 0;
    Grapheme grapheme = {.code = 0, .size = 0};

    while (index < text_end_index && string_get_character_codepoint(text, index, text_end_index, &grapheme)) {
        index += grapheme.size;

        // override hard-spaces with white-spaces
        if (grapheme.code == 0xA0) grapheme.code = 0x20;

        // ignore "\r" characters
        if (grapheme.code == FONTGLYPH_CARRIAGERETURN) {
            previous_codepoint = grapheme.code;
            continue;
        }

        if (grapheme.code == FONTGLYPH_LINEFEED) {
            if (width > max_width) max_width = width;
            width = 0.0f;
            previous_codepoint = grapheme.code;
            line_chars = 0;
            lines++;
            last_glyph_width_correction = 0;
            continue;
        }

        FontCharData* fontchardata;
        int32_t map_index = fonttype_internal_get_fontchardata2(fonttype->lookup_table, &fonttype->atlas_primary, grapheme.code);

        if (map_index < 0) {
            map_index = fonttype_internal_get_fontchardata(&fonttype->atlas_secondary, grapheme.code);
            if (map_index < 0) {
                if (grapheme.code == FONTGLYPH_TAB) {
                    int32_t filler = fontglyph_internal_calc_tabstop(line_chars);
                    if (filler > 0) {
                        width += fonttype->space_width * filler;
                        line_chars += filler;
                    }
                } else {
                    // space, hard space or unknown characters
                    width += fonttype->space_width;
                    line_chars++;
                }
                last_glyph_width_correction = 0;
                continue;
            } else {
                fontchardata = &fonttype->atlas_secondary.map->char_array[map_index];
            }
        } else {
            fontchardata = &fonttype->atlas_primary.map->char_array[map_index];
        }

        int32_t kerning_x = 0;
        if (previous_codepoint) {
            // compute kerning
            for (int32_t i = 0; i < fontchardata->kernings_size; i++) {
                if (fontchardata->kernings[i].codepoint == previous_codepoint) {
                    kerning_x = fontchardata->kernings[i].x;
                    break;
                }
            }
        }

        last_glyph_width_correction = fontchardata->width + fontchardata->offset_x - fontchardata->advancex + kerning_x;
        width += fontchardata->advancex + kerning_x;
        previous_codepoint = grapheme.code;
        line_chars++;
    }

    float line_height = fonttype_measure_line_height(fonttype, params->height);
    info->max_width = (math2d_max_float(width, max_width) + last_glyph_width_correction) * scale;
    info->total_height = (line_height * lines) + (params->paragraph_space * (lines - 1));
}

void fonttype_measure_char(FontType fonttype, uint32_t codepoint, float height, FontCharInfo* info) {
    const float scale = height / FONTTYPE_GLYPHS_HEIGHT;

    // override hard-spaces with white-spaces
    if (codepoint == 0xA0) codepoint = 0x20;

    FontCharData* fontchardata;
    int32_t map_index = fonttype_internal_get_fontchardata2(fonttype->lookup_table, &fonttype->atlas_primary, codepoint);

    if (map_index < 0) {
        map_index = fonttype_internal_get_fontchardata(&fonttype->atlas_secondary, codepoint);
        if (map_index < 0) {
            if (codepoint == FONTGLYPH_TAB) {
                int32_t filler = fontglyph_internal_calc_tabstop(info->line_char_count);
                if (filler > 0) {
                    info->last_char_width = fonttype->space_width * filler;
                    info->line_char_count += filler;
                }
            } else {
                // space, hard space or unknown characters
                info->last_char_width = fonttype->space_width;
                info->line_char_count++;
            }
            info->last_char_width *= scale;
            info->last_char_height = height;
            info->last_char_width_end = 0.0f;
            return;
        } else {
            fontchardata = &fonttype->atlas_secondary.map->char_array[map_index];
        }
    } else {
        fontchardata = &fonttype->atlas_primary.map->char_array[map_index];
    }

    int32_t kerning_x = 0;
    if (info->previous_codepoint) {
        // compute kerning
        for (int32_t i = 0; i < fontchardata->kernings_size; i++) {
            if (fontchardata->kernings[i].codepoint == info->previous_codepoint) {
                kerning_x = fontchardata->kernings[i].x;
                break;
            }
        }
    }

    info->last_char_width = (fontchardata->advancex + kerning_x) * scale;
    info->last_char_height = fontchardata->advancey * scale;
    info->last_char_width_end = (fontchardata->width + fontchardata->offset_x - fontchardata->advancex + kerning_x) * scale;
    info->previous_codepoint = codepoint;
    info->line_char_count++;
}

float fonttype_measure_line_height(FontType fonttype, float height) {
    float line_height;
    if (fonttype->atlas_primary.map)
        line_height = fonttype->atlas_primary.map->line_height;
    else
        line_height = fonttype->atlas_secondary.map->line_height;

    float scale = height / FONTTYPE_GLYPHS_HEIGHT;
    return math2d_max_float(height, (height * 2.0f) - (line_height * scale));
}

float fonttype_draw_text(FontType fonttype, PVRContext pvrctx, FontParams* params, float x, float y, int32_t text_index, size_t text_length, const char* text) {
    Grapheme grapheme = {.code = 0, .size = 0};
    const bool has_border = params->border_enable && params->border_color[3] > 0.0f && params->border_size >= 0.0f;
    const float outline_size = params->border_size * 2.0f;
    const float scale_glyph = params->height / FONTTYPE_GLYPHS_HEIGHT;
    const size_t text_end_index = (size_t)text_index + text_length;
    const float line_height = fonttype_measure_line_height(fonttype, params->height);
    const float ascender = (fonttype->atlas_primary.map ? fonttype->atlas_primary.map : fonttype->atlas_secondary.map)->ascender;

#if !defined(_arch_dreamcast) && defined(SDF_FONT)
    const float scale_outline = params->height / FONTTYPE_GLYPHS_OUTLINE_HEIGHT;

    if (has_border) {
        // calculate sdf padding
        float padding;
        padding = params->border_size / FONTTYPE_GLYPHS_OUTLINE_RATIO;
        padding /= params->height / FONTTYPE_GLYPHS_OUTLINE_HEIGHT;

        glyphrenderer_set_params_sdf(pvrctx, FONTTYPE_GLYPHS_OUTLINE_RATIO, padding, FONTTYPE_GLYPHS_OUTLINE_THICKNESS);
    } else {
        glyphrenderer_set_params_sdf(pvrctx, FONTTYPE_GLYPHS_OUTLINE_RATIO, -1.0f, -1.0f);
    }
#endif

    float draw_glyph_x = 0.0f;
    float draw_glyph_y = ascender;
    int32_t line_chars = 0;
    int32_t lines = 1;

    int32_t index = text_index;
    uint32_t previous_codepoint = 0x0000;
    int32_t total_glyphs = 0;
    int32_t map_index;

    pvr_context_save(pvrctx);
    pvr_context_set_vertex_alpha(pvrctx, params->tint_color[3]);
    pvr_context_set_vertex_offsetcolor(pvrctx, PVR_DEFAULT_OFFSET_COLOR);
    pvr_context_set_vertex_antialiasing(pvrctx, PVRCTX_FLAG_DEFAULT);


    // count the amount of glyph required
    while (index < text_end_index && string_get_character_codepoint(text, index, text_end_index, &grapheme)) {
        index += grapheme.size;

        if (grapheme.code == 0xA0) continue;

        map_index = fonttype_internal_get_fontchardata2(fonttype->lookup_table, &fonttype->atlas_primary, grapheme.code);
        if (map_index >= 0) {
            if (fonttype->atlas_primary.map->char_array[map_index].has_atlas_entry) total_glyphs++;
            continue;
        }
        map_index = fonttype_internal_get_fontchardata(&fonttype->atlas_secondary, grapheme.code);
        if (map_index >= 0) {
            if (fonttype->atlas_secondary.map->char_array[map_index].has_atlas_entry) total_glyphs++;
            continue;
        }
    }

    // prepare vertex buffer
    if (has_border) total_glyphs *= 2;
    int32_t added = 0;
    int32_t maximum = glyphrenderer_prepare(
        pvrctx, total_glyphs, has_border,
        params->tint_color, params->border_color,
        false, true,
        fonttype->atlas_primary.texture, fonttype->atlas_secondary.texture
    );

    // add glyphs to the vertex buffer
    index = text_index;
    while (added < maximum && index < text_end_index && string_get_character_codepoint(text, index, text_end_index, &grapheme)) {
        index += grapheme.size;

        // override hard-spaces with white-spaces
        if (grapheme.code == 0xA0) grapheme.code = 0x20;

        // ignore "\r" characters
        if (grapheme.code == FONTGLYPH_CARRIAGERETURN) {
            previous_codepoint = grapheme.code;
            continue;
        }

        if (grapheme.code == FONTGLYPH_LINEFEED) {
            draw_glyph_x = 0.0f;
            draw_glyph_y += line_height + params->paragraph_space;
            previous_codepoint = grapheme.code;
            line_chars = 0;
            lines++;
            continue;
        }


        FCAtlas* atlas = &fonttype->atlas_primary;
        bool is_secondary = false;
        map_index = fonttype_internal_get_fontchardata2(fonttype->lookup_table, atlas, grapheme.code);

        if (map_index < 0) {
            atlas = &fonttype->atlas_secondary;
            is_secondary = true;
            map_index = fonttype_internal_get_fontchardata(atlas, grapheme.code);
        }

        if (map_index < 0) {
            // codepoint not mapped or fonttype_measure() was not called previously to map it
            if (grapheme.code == FONTGLYPH_TAB) {
                int32_t filler = fontglyph_internal_calc_tabstop(line_chars);
                if (filler > 0) {
                    draw_glyph_x += fonttype->space_width * filler;
                    line_chars += filler;
                }
            } else {
                // space, hard space or unknown characters
                draw_glyph_x += fonttype->space_width;
                line_chars++;
            }

            previous_codepoint = grapheme.code;
            continue;
        }

        FontCharData* fontchardata_glyph = &atlas->map->char_array[map_index];

        // apply kerning before continue
        if (previous_codepoint != 0x0000) {
            for (int32_t i = 0; i < fontchardata_glyph->kernings_size; i++) {
                if (fontchardata_glyph->kernings[i].codepoint == previous_codepoint) {
                    draw_glyph_x += fontchardata_glyph->kernings[i].x;
                    break;
                }
            }
        }

        if (fontchardata_glyph->has_atlas_entry) {
            // compute draw location and size
            float dx = x + ((draw_glyph_x + fontchardata_glyph->offset_x) * scale_glyph);
            float dy = y + ((draw_glyph_y + fontchardata_glyph->offset_y) * scale_glyph);
            float dw = fontchardata_glyph->width * scale_glyph;
            float dh = fontchardata_glyph->height * scale_glyph;

            if (has_border) {
                float sdx, sdy, sdw, sdh;
#ifdef _arch_dreamcast
                // compute border location and outline size
                sdx = dx - params->border_size;
                sdy = dy - params->border_size;
                sdw = dw + outline_size;
                sdh = dh + outline_size;
#else
                Texture texture_outline;
                FontCharData* fontchardata_outline;

#ifdef SDF_FONT
                texture_outline = atlas->texture_outline;
                fontchardata_outline = &atlas->map_outline->char_array[map_index];

                sdw = dw + ((fontchardata_outline->width * scale_outline) - dw);
                sdh = dh + ((fontchardata_outline->height * scale_outline) - dh);
                sdx = dx - ((sdw - dw) / 2.0f);
                sdy = dy - ((sdh - dh) / 2.0f);
#else
                texture_outline = atlas->texture;
                fontchardata_outline = fontchardata_glyph;

                // compute border location and outline size
                sdw = dw + outline_size;
                sdh = dh + outline_size;
                sdx = dx - params->border_size;
                sdy = dy - params->border_size;
#endif
#endif
                sdx += params->border_offset_x;
                sdy += params->border_offset_y;

#ifdef _arch_dreamcast
                // queue outlined glyph for batch rendering
                glyphrenderer_draw_glyph(
                    is_secondary, true,
                    fontchardata_glyph->atlas_entry.x, fontchardata_glyph->atlas_entry.y, fontchardata_glyph->width, fontchardata_glyph->height,
                    sdx, sdy, sdw, sdh
                );
#else
                // queue outlined glyph for batch rendering
                glyphrenderer_draw(
                    texture_outline, is_secondary, true,
                    fontchardata_outline->atlas_entry.x, fontchardata_outline->atlas_entry.y,
                    fontchardata_outline->width, fontchardata_outline->height,
                    sdx, sdy, sdw, sdh
                );
#endif
                added++;
            }

#ifdef _arch_dreamcast
            // queue glyph for batch rendering
            glyphrenderer_draw_glyph(
                is_secondary, false,
                fontchardata_glyph->atlas_entry.x, fontchardata_glyph->atlas_entry.y, fontchardata_glyph->width, fontchardata_glyph->height,
                dx, dy, dw, dh
            );
            added++;
        }
#else
#ifdef SDF_FONT
            if (has_border) {
                glyphrenderer_draw(
                    pvrctx,
                    params->tint_color, params->border_color,
                    false, true,
                    fonttype->atlas_primary.texture, fonttype->atlas_secondary.texture,
                    fonttype->atlas_primary.texture_outline, fonttype->atlas_secondary.texture_outline
                );
            } else {
#else
            {
#endif
                glyphrenderer_draw(
                    pvrctx,
                    params->tint_color, params->border_color,
                    false, true,
                    fonttype->atlas_primary.texture, fonttype->atlas_secondary.texture,
                    NULL, NULL
                );
            }
#endif

        draw_glyph_x += fontchardata_glyph->advancex;
        line_chars++;
    }

    pvr_context_restore(pvrctx);
    return (line_height * lines) + (params->paragraph_space * (lines - 1));
}

void fonttype_map_codepoints(FontType fonttype, const char* text, int32_t text_index, size_t text_end_index) {
    int32_t actual = fonttype->atlas_secondary.map ? fonttype->atlas_secondary.map->char_array_size : 0;
    int32_t new_codepoints = 0;
    Grapheme grapheme = {.code = 0, .size = 0};
    int32_t index = text_index;

    // step 1: count all unmapped codepoints
    while (index < text_end_index && string_get_character_codepoint(text, index, text_end_index, &grapheme)) {
        index += grapheme.size;

        switch (grapheme.code) {
            case FONTGLYPH_LINEFEED:
            case FONTGLYPH_CARRIAGERETURN:
                continue;
        }

        if (fonttype_internal_get_fontchardata2(fonttype->lookup_table, &fonttype->atlas_primary, grapheme.code) >= 0)
            continue;
        if (fonttype_internal_get_fontchardata(&fonttype->atlas_secondary, grapheme.code) >= 0)
            continue;

        // not present, count it
        new_codepoints++;
    }

    if (new_codepoints < 1) return; // nothing to do

    // step 2: allocate codepoints array
    int32_t codepoints_size = actual + new_codepoints + 1;
    uint32_t* codepoints = malloc_for_array(uint32_t, codepoints_size);

    codepoints[actual + new_codepoints] = 0x00000000;

    if (fonttype->atlas_secondary.map) {
        // add existing secondary codepoints
        for (int32_t i = 0; i < fonttype->atlas_secondary.map->char_array_size; i++) {
            codepoints[i] = fonttype->atlas_secondary.map->char_array[i].codepoint;
        }
    }

    index = text_index;
    new_codepoints = actual;
    while (index < text_end_index && string_get_character_codepoint(text, index, text_end_index, &grapheme)) {
        index += grapheme.size;

        if (fonttype_internal_get_fontchardata2(fonttype->lookup_table, &fonttype->atlas_primary, grapheme.code) >= 0)
            continue;
        if (fonttype_internal_get_fontchardata(&fonttype->atlas_secondary, grapheme.code) >= 0)
            continue;

        codepoints[new_codepoints++] = grapheme.code;
    }

    // step 3: rebuild the secondary char map
    fonttype_internal_destroy_atlas(&fonttype->atlas_secondary);
    fonttype_internal_create_atlas(fonttype, &fonttype->atlas_secondary, codepoints);

    // dispose secondary codepoints array
    free_chk(codepoints);
}

int32_t fonttype_animate(FontType fonttype, float elapsed) {
    (void)fonttype;
    (void)elapsed;

    // not used
    return 1;
}


static bool fonttype_internal_init_freetype(FontType fonttype, const char* src) {
    ArrayBuffer font = fs_readarraybuffer(src);
    if (!font) return true;

#if !defined(_arch_dreamcast) && defined(SDF_FONT)
    fontatlas_enable_sdf(true);
#endif

    mutex_lock(&fonttype_fontatlas_mutex);

    // Important: keep the font data allocated, required for FreeType library
    fonttype->font = font;
    fonttype->fontatlas = fontatlas_init(font->data, (int32_t)font->length);

    mutex_unlock(&fonttype_fontatlas_mutex);

    return !fonttype->fontatlas;
}

static FontCharMap* fonttype_internal_create_fontcharmap(FontType fonttype, uint32_t* characters_map, uint8_t glyphs_height, int8_t glyphs_gap) {
    FontCharMap* fontcharmap = NULL;

    mutex_lock(&fonttype_fontatlas_mutex);

    if (characters_map) {
        fontcharmap = fontatlas_atlas_build(
            fonttype->fontatlas, glyphs_height, glyphs_gap, characters_map
        );
    } else {
        fontcharmap = fontatlas_atlas_build_complete(
            fonttype->fontatlas, glyphs_height, glyphs_gap
        );
    }

    mutex_unlock(&fonttype_fontatlas_mutex);

    return fontcharmap;
}

static Texture fonttype_internal_upload_texture(FontCharMap* fontcharmap) {
    if (!fontcharmap || !fontcharmap->texture) return NULL;

    pvr_ptr_t vram_ptr = pvr_mem_malloc((size_t)fontcharmap->texture_byte_size);
    if (!vram_ptr) {
        // logger_error("fonttype_internal_upload_texture() not enough space in the vram for " FMT_I4 " bytes", fontcharmap->char_array_size);
        return NULL;
    }

    pvr_txr_load_ex(fontcharmap->texture, vram_ptr, fontcharmap->texture_width, fontcharmap->texture_height, PVR_TXRLOAD_4BPP);

    Texture texture = texture_init_from_raw(
        vram_ptr, (size_t)fontcharmap->texture_byte_size, true,
        fontcharmap->texture_width, fontcharmap->texture_height,
        fontcharmap->texture_width, fontcharmap->texture_height
    );

    texture->format = PVR_TXRFMT_PAL4BPP | PVR_TXRFMT_TWIDDLED | PVR_TXRFMT_NOSTRIDE | PVR_TXRFMT_VQ_DISABLE;

    // no longer needed
    fontatlas_atlas_destroy_texture_only(fontcharmap);

    return texture;
}

static int32_t fonttype_internal_get_fontchardata(const FCAtlas* atlas, uint32_t codepoint) {
    if (atlas->map) {
        for (int32_t i = 0; i < atlas->map->char_array_size; i++) {
            if (codepoint == atlas->map->char_array[i].codepoint) return i;
        }
    }
    return -1;
}

static inline int32_t fonttype_internal_get_fontchardata2(const uint8_t* lookup_table, const FCAtlas* atlas, uint32_t codepoint) {
    if (codepoint < FONTGLYPH_LOOKUP_TABLE_LENGTH) {
        const uint8_t index = lookup_table[codepoint];
        if (index < FONTGLYPH_LOOKUP_TABLE_LENGTH) {
            return index;
        }
    }
    return fonttype_internal_get_fontchardata(atlas, codepoint);
}


static void fonttype_internal_create_atlas(FontType fonttype, FCAtlas* atlas, uint32_t* codepoints) {
    FontCharMap* fontcharmap = fonttype_internal_create_fontcharmap(fonttype, codepoints, FONTTYPE_GLYPHS_HEIGHT, FONTTYPE_GLYPHS_GAP);
    Texture texture = fonttype_internal_upload_texture(fontcharmap);

#if !defined(_arch_dreamcast) && defined(SDF_FONT)
    if (fontcharmap) {
        FontCharMap* fontcharmap_outline = fonttype_internal_create_fontcharmap(fonttype, codepoints, FONTTYPE_GLYPHS_OUTLINE_HEIGHT, FONTTYPE_GLYPHS_OUTLINE_GAP);
        Texture texture_outline = fonttype_internal_upload_texture(fontcharmap_outline);

        assert(fontcharmap_outline);
        assert(fontcharmap.char_array_size == fontcharmap_outline.char_array_size);

        for (int32_t i = 0; i < fontcharmap->char_array_size; i++) {
            FontCharData entry = fontcharmap->char_array[i];
            FontCharData entry_outline = fontcharmap_outline->char_array[i];

            assert(entry.has_atlas_entry == entry_outline.has_atlas_entry);
        }

        atlas->map_outline = fontcharmap_outline;
        atlas->texture_outline = texture_outline;
    }
#endif
    atlas->map = fontcharmap;
    atlas->texture = texture;
}

static void fonttype_internal_destroy_atlas(FCAtlas* atlas) {
#if !defined(_arch_dreamcast) && defined(SDF_FONT)
    if (atlas->map_outline) {
        fontatlas_atlas_destroy(atlas->map_outline);
        atlas->map_outline = NULL;
    }
    if (atlas->texture_outline) {
        texture_destroy(&atlas->texture_outline);
        atlas->texture_outline = NULL;
    }
#endif
    if (atlas->map) {
        fontatlas_atlas_destroy(atlas->map);
        atlas->map = NULL;
    }
    if (atlas->texture) {
        texture_destroy(&atlas->texture);
        atlas->texture = NULL;
    }
}
