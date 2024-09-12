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


struct FontType_s {
    int32_t instance_id;
    int32_t instance_references;
    char* instance_path;
    ArrayBuffer font;
    FontAtlas fontatlas;
    float space_width;
    FontCharMap* fontcharmap_primary;
    Texture fontcharmap_primary_texture;
    FontCharMap* fontcharmap_secondary;
    Texture fontcharmap_secondary_texture;
    uint8_t lookup_table[FONTGLYPH_LOOKUP_TABLE_LENGTH];
};


static const uint8_t FONTTYPE_GLYPHS_HEIGHT = 42; // original as 32px, 64px is enough for SDF
#ifndef _arch_dreamcast
const float FONTTYPE_GLYPHS_SDF_SIZE = FONTTYPE_GLYPHS_HEIGHT >> 2; // 25% of FONTTYPE_GLYPHS_HEIGHT
const float FONTTYPE_GLYPHS_GAPS = 16.0f;                           // space between glyph in pixels (must be high for SDF)
#endif
static const int8_t FONTTYPE_GLYPHS_GAPS = 4;   // space between glyph in pixels
static const float FONTTYPE_FAKE_SPACE = 0.75f; // 75% of the height

static Map FONTTYPE_POOL = NULL;
static int32_t FONTTYPE_IDS = 0;
static mutex_t fonttype_fontatlas_mutex = MUTEX_INITIALIZER;


void __attribute__((constructor)) __ctor_fonttype() {
    FONTTYPE_POOL = map_init();
}


static bool fonttype_internal_init_freetype(FontType fonttype, const char* src);
static FontCharMap* fonttype_internal_retrieve_fontcharmap(FontType fonttype, uint32_t* characters_map);
static Texture fonttype_internal_upload_texture(FontCharMap* fontcharmap);
static FontCharData* fonttype_internal_get_fontchardata(const FontCharMap* fontcharmap, uint32_t codepoint);
static inline FontCharData* fonttype_internal_get_fontchardata2(const uint8_t* lookup_table, const FontCharMap* fontcharmap, uint32_t codepoint);
#ifndef _arch_dreamcast
static float fonttype_internal_calc_smoothing(PVRContext pvrctx, float height);
#endif


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

        .fontcharmap_primary = NULL,
        .fontcharmap_primary_texture = NULL,

        .fontcharmap_secondary = NULL,
        .fontcharmap_secondary_texture = NULL
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
    fonttype->fontcharmap_primary = fonttype_internal_retrieve_fontcharmap(fonttype, NULL);
    fonttype->fontcharmap_primary_texture = fonttype_internal_upload_texture(fonttype->fontcharmap_primary);

    if (fonttype->fontcharmap_primary) {
        FontCharData* char_array = fonttype->fontcharmap_primary->char_array;
        int32_t char_array_size = fonttype->fontcharmap_primary->char_array_size;

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

    if (fonttype->fontcharmap_primary) {
        fontatlas_atlas_destroy(fonttype->fontcharmap_primary);
        if (fonttype->fontcharmap_primary_texture) texture_destroy(&fonttype->fontcharmap_primary_texture);
    }

    if (fonttype->fontcharmap_secondary) {
        fontatlas_atlas_destroy(fonttype->fontcharmap_secondary);
        if (fonttype->fontcharmap_secondary_texture) texture_destroy(&fonttype->fontcharmap_secondary_texture);
    }

    if (fonttype->fontatlas) fontatlas_destroy(&fonttype->fontatlas);
    arraybuffer_destroy(&fonttype->font);

    free_chk(fonttype->instance_path);

    free_chk(fonttype);
    *fonttype_ptr = NULL;
}


float fonttype_measure(FontType fonttype, FontParams* params, const char* text, int32_t text_index, size_t text_length) {
    const float scale = params->height / FONTTYPE_GLYPHS_HEIGHT;
    const size_t text_end_index = (size_t)text_index + text_length;

    float max_width = 0.0f;
    float width = 0.0f;
    int32_t index = text_index;
    uint32_t previous_codepoint = 0;
    int32_t line_chars = 0;
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
            continue;
        }

        FontCharData* fontchardata = fonttype_internal_get_fontchardata2(fonttype->lookup_table, fonttype->fontcharmap_primary, grapheme.code);
        if (!fontchardata) {
            fontchardata = fonttype_internal_get_fontchardata(fonttype->fontcharmap_secondary, grapheme.code);
            if (!fontchardata) {
                if (grapheme.code == FONTGLYPH_TAB) {
                    int32_t filler = fontglyph_internal_calc_tabstop(line_chars);
                    if (filler > 0) {
                        width += fonttype->space_width * filler * scale;
                        line_chars += filler;
                    }
                } else {
                    // space, hard space or unknown characters
                    width += fonttype->space_width * scale;
                    line_chars++;
                }
            }

            continue;
        }

        if (previous_codepoint) {
            // compute kerning
            for (int32_t i = 0; i < fontchardata->kernings_size; i++) {
                if (fontchardata->kernings[i].codepoint == previous_codepoint) {
                    width += fontchardata->kernings[i].x * scale;
                    break;
                }
            }
        }

        width += fontchardata->advancex * scale;
        previous_codepoint = grapheme.code;
        line_chars++;
    }

    return math2d_max_float(width, max_width);
}

void fonttype_measure_char(FontType fonttype, uint32_t codepoint, float height, FontLineInfo* lineinfo) {
    const float scale = height / FONTTYPE_GLYPHS_HEIGHT;

    // override hard-spaces with white-spaces
    if (codepoint == 0xA0) codepoint = 0x20;

    FontCharData* fontchardata = fonttype_internal_get_fontchardata2(fonttype->lookup_table, fonttype->fontcharmap_primary, codepoint);
    if (!fontchardata) {
        fontchardata = fonttype_internal_get_fontchardata(fonttype->fontcharmap_secondary, codepoint);
        if (!fontchardata) {
            if (codepoint == FONTGLYPH_TAB) {
                int32_t filler = fontglyph_internal_calc_tabstop(lineinfo->line_char_count);
                if (filler > 0) {
                    lineinfo->last_char_width = fonttype->space_width * filler * scale;
                    lineinfo->line_char_count += filler;
                }
            } else {
                // space, hard space or unknown characters
                lineinfo->last_char_width = fonttype->space_width * scale;
                lineinfo->line_char_count++;
            }
        }
        return;
    }

    if (lineinfo->previous_codepoint) {
        // compute kerning
        for (int32_t i = 0; i < fontchardata->kernings_size; i++) {
            if (fontchardata->kernings[i].codepoint == lineinfo->previous_codepoint) {
                lineinfo->last_char_width = fontchardata->kernings[i].x * scale;
                break;
            }
        }
    }

    lineinfo->last_char_width = fontchardata->advancex * scale;
    lineinfo->previous_codepoint = codepoint;
    lineinfo->line_char_count++;
}

float fonttype_draw_text(FontType fonttype, PVRContext pvrctx, FontParams* params, float x, float y, int32_t text_index, size_t text_length, const char* text) {
    if (text == NULL || text_length < 1) return 0.0f;

    Grapheme grapheme = {.code = 0, .size = 0};
    const FontCharMap* primary = fonttype->fontcharmap_primary;
    const FontCharMap* secondary = fonttype->fontcharmap_secondary;
    const bool has_border = params->border_enable && params->border_color[3] > 0.0f && params->border_size >= 0.0f;
    const float outline_size = params->border_size * 2.0f;
    const float scale = params->height / FONTTYPE_GLYPHS_HEIGHT;
    const float ascender = ((primary ? primary : secondary)->ascender / 2.0f) * scale; // FIXME: ¿why does dividing by 2 works?
    const size_t text_end_index = (size_t)text_index + text_length;

#ifndef _arch_dreamcast
#ifdef SDF_FONT
    if (has_border) {
        // calculate sdf padding
        float padding;
        padding = params->border_size / FONTTYPE_GLYPHS_SDF_SIZE;
        padding /= params->height / FONTTYPE_GLYPHS_HEIGHT;

        glyphrenderer_set_params_sdf(pvrctx, FONTTYPE_GLYPHS_SDF_SIZE, padding);
    } else {
        glyphrenderer_set_params_sdf(pvrctx, FONTTYPE_GLYPHS_SDF_SIZE, -1.0f);
    }
#endif
#endif

    float draw_x = 0.0f;
    float draw_y = 0.0f; // in dreamcast do not use "0.0f - ascender"
    int32_t line_chars = 0;

    int32_t index = text_index;
    uint32_t previous_codepoint = 0x0000;
    int32_t total_glyphs = 0;
    FontCharData* fontchardata;

    pvr_context_save(pvrctx);
    pvr_context_set_vertex_alpha(pvrctx, params->tint_color[3]);
    pvr_context_set_vertex_offsetcolor(pvrctx, PVR_DEFAULT_OFFSET_COLOR);
    pvr_context_set_vertex_antialiasing(pvrctx, PVRCTX_FLAG_DEFAULT);


    // count the amount of glyph required
    while (index < text_end_index && string_get_character_codepoint(text, index, text_end_index, &grapheme)) {
        index += grapheme.size;

        if (grapheme.code == 0xA0) continue;

        if ((fontchardata = fonttype_internal_get_fontchardata2(fonttype->lookup_table, primary, grapheme.code))) {
            if (fontchardata->has_atlas_entry) total_glyphs++;
            continue;
        }
        if ((fontchardata = fonttype_internal_get_fontchardata(secondary, grapheme.code))) {
            if (fontchardata->has_atlas_entry) total_glyphs++;
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
        fonttype->fontcharmap_primary_texture, fonttype->fontcharmap_secondary_texture
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
            draw_x = 0.0f;
            draw_y += params->height + params->paragraph_space - ascender;
            previous_codepoint = grapheme.code;
            line_chars = 0;
            continue;
        }

        fontchardata = fonttype_internal_get_fontchardata2(fonttype->lookup_table, primary, grapheme.code);
        bool is_secondary = false;

        if (!fontchardata) {
            fontchardata = fonttype_internal_get_fontchardata(secondary, grapheme.code);
            is_secondary = true;
        }

        if (!fontchardata) {
            // codepoint not mapped or fonttype_measure() was not called previously to map it
            if (grapheme.code == FONTGLYPH_TAB) {
                int32_t filler = fontglyph_internal_calc_tabstop(line_chars);
                if (filler > 0) {
                    draw_x += fonttype->space_width * filler * scale;
                    line_chars += filler;
                }
            } else {
                // space, hard space or unknown characters
                draw_x += fonttype->space_width * scale;
                line_chars++;
            }

            previous_codepoint = grapheme.code;
            continue;
        }

        // apply kerking before continue
        if (previous_codepoint) {
            for (int32_t i = 0; i < fontchardata->kernings_size; i++) {
                if (fontchardata->kernings[i].codepoint == previous_codepoint) {
                    draw_x += fontchardata->kernings[i].x * scale;
                    break;
                }
            }
        }

        if (fontchardata->has_atlas_entry) {
            // compute draw location and size
            float dx = x + draw_x + (fontchardata->offset_x * scale);
            float dy = y + draw_y + (fontchardata->offset_y * scale);
            float dw = fontchardata->width * scale;
            float dh = fontchardata->height * scale;

            if (has_border) {
                float sdx, sdy, sdw, sdh;
#ifdef _arch_dreamcast
                // compute border location and outline size
                sdx = dx - params->border_size;
                sdy = dy - params->border_size;
                sdw = dw + outline_size;
                sdh = dh + outline_size;
#else
#ifdef SDF_FONT
                sdx = dx;
                sdy = dy;
                sdw = dw;
                sdh = dh;
#else
                // compute border location and outline size
                sdx = dx - params->border_size;
                sdy = dy - params->border_size;
                sdw = dw + outline_size;
                sdh = dh + outline_size;
#endif
#endif
                sdx += params->border_offset_x;
                sdy += params->border_offset_y;

                // queue outlined glyph for batch rendering
                glyphrenderer_draw_glyph(
                    is_secondary, true,
                    fontchardata->atlas_entry.x, fontchardata->atlas_entry.y, fontchardata->width, fontchardata->height,
                    sdx, sdy, sdw, sdh
                );
                added++;
            }

            // queue glyph for batch rendering
            glyphrenderer_draw_glyph(
                is_secondary, false,
                fontchardata->atlas_entry.x, fontchardata->atlas_entry.y, fontchardata->width, fontchardata->height,
                dx, dy, dw, dh
            );
            added++;
        }

        draw_x += fontchardata->advancex * scale;
        line_chars++;
    }

    pvr_context_restore(pvrctx);
    return draw_y + params->height;
}

void fonttype_map_codepoints(FontType fonttype, const char* text, int32_t text_index, size_t text_end_index) {
    int32_t actual = fonttype->fontcharmap_secondary ? fonttype->fontcharmap_secondary->char_array_size : 0;
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

        if (fonttype_internal_get_fontchardata2(fonttype->lookup_table, fonttype->fontcharmap_primary, grapheme.code))
            continue;
        if (fonttype_internal_get_fontchardata(fonttype->fontcharmap_secondary, grapheme.code))
            continue;

        // not present, count it
        new_codepoints++;
    }

    if (new_codepoints < 1) return; // nothing to do

    // step 2: allocate codepoints array
    int32_t codepoints_size = actual + new_codepoints + 1;
    uint32_t* codepoints = malloc_for_array(uint32_t, codepoints_size);

    codepoints[actual + new_codepoints] = 0x00000000;

    if (fonttype->fontcharmap_secondary) {
        // add existing secondary codepoints
        for (int32_t i = 0; i < fonttype->fontcharmap_secondary->char_array_size; i++) {
            codepoints[i] = fonttype->fontcharmap_secondary->char_array[i].codepoint;
        }
    }

    index = text_index;
    new_codepoints = actual;
    while (index < text_end_index && string_get_character_codepoint(text, index, text_end_index, &grapheme)) {
        index += grapheme.size;

        if (fonttype_internal_get_fontchardata2(fonttype->lookup_table, fonttype->fontcharmap_primary, grapheme.code))
            continue;
        if (fonttype_internal_get_fontchardata(fonttype->fontcharmap_secondary, grapheme.code))
            continue;

        codepoints[new_codepoints++] = grapheme.code;
    }

    // step 3: rebuild the secondary char map
    if (fonttype->fontcharmap_secondary) {
        // dispose previous instance
        fontatlas_atlas_destroy(fonttype->fontcharmap_secondary);
        texture_destroy(&fonttype->fontcharmap_secondary_texture);
    }

    // build map and upload texture
    fonttype->fontcharmap_secondary = fonttype_internal_retrieve_fontcharmap(fonttype, codepoints);
    fonttype->fontcharmap_secondary_texture = fonttype_internal_upload_texture(fonttype->fontcharmap_secondary);

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
    if (!font) return false;

#ifndef _arch_dreamcast
#ifdef SDF_FONT
    fontatlas_enable_sdf(true);
#endif
#endif

    mutex_lock(&fonttype_fontatlas_mutex);

    // Important: keep the font data allocated, required for FreeType library
    fonttype->font = font;
    fonttype->fontatlas = fontatlas_init(font->data, (int32_t)font->length);

    mutex_unlock(&fonttype_fontatlas_mutex);

    return !fonttype->fontatlas;
}

static FontCharMap* fonttype_internal_retrieve_fontcharmap(FontType fonttype, uint32_t* characters_map) {
    FontCharMap* fontcharmap = NULL;

    mutex_lock(&fonttype_fontatlas_mutex);

    if (characters_map) {
        fontcharmap = fontatlas_atlas_build(
            fonttype->fontatlas, FONTTYPE_GLYPHS_HEIGHT, FONTTYPE_GLYPHS_GAPS, characters_map
        );
    } else {
        fontcharmap = fontatlas_atlas_build_complete(
            fonttype->fontatlas, FONTTYPE_GLYPHS_HEIGHT, FONTTYPE_GLYPHS_GAPS
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

static FontCharData* fonttype_internal_get_fontchardata(const FontCharMap* fontcharmap, uint32_t codepoint) {
    if (fontcharmap) {
        for (int32_t i = 0; i < fontcharmap->char_array_size; i++) {
            if (codepoint == fontcharmap->char_array[i].codepoint) return &fontcharmap->char_array[i];
        }
    }
    return NULL;
}

static inline FontCharData* fonttype_internal_get_fontchardata2(const uint8_t* lookup_table, const FontCharMap* fontcharmap, uint32_t codepoint) {
    if (codepoint < FONTGLYPH_LOOKUP_TABLE_LENGTH) {
        const uint8_t index = lookup_table[codepoint];
        if (index < FONTGLYPH_LOOKUP_TABLE_LENGTH) {
            return &fontcharmap->char_array[index];
        }
    }
    return fonttype_internal_get_fontchardata(fontcharmap, codepoint);
}
