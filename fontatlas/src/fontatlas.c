#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_BITMAP_H
#include FT_IMAGE_H
#include FT_GLYPH_H
#include FT_ERRORS_H

#include <stdbool.h>
#include <assert.h>
#include <wchar.h>

#include "fontatlas.h"

#define MATH_MIN(a, b) a < b ? a : b

typedef struct {
    uint8_t* texture;
    int32_t texture_byte_size;
    int32_t width;
    int32_t height;
} TextureAtlas;

typedef struct {
    uint32_t codepoint;
    int16_t offset_x;
    int16_t offset_y;
    int16_t advancex;
    int32_t width;
    int32_t height;
    FontCharDataKerning* kernings;
    int32_t kernings_size;
    FontCharDataAtlasEntry atlas_entry;
    bool has_atlas_entry;
    unsigned char* bitmap;
    FT_UInt glyph_index;
} CharData;


const int32_t MAX_TEXTURE_DIMMEN = 2048;

static bool sdf_enabled = false;
void fontatlas_enable_sdf(bool enable) {
    sdf_enabled = enable;
}


static int sort(const void* a, const void* b) {
    return (*((CharData**)a))->height - (*((CharData**)b))->height;
}

static int32_t math2d_poweroftwo_calc(int32_t dimmen) {
    int32_t size = 2;
    while (size < 0x80000) {
        if (size >= dimmen) break;
        size *= 2;
    }

    if (size >= 0x80000) {
        fprintf(stderr, "math2d_poweroftwo_calc() failed for: %i\n", dimmen);
        exit(1);
    }
    return size;
}


FontAtlas fontatlas_init(uint8_t* font_data, int32_t font_data_size) {
    if (font_data_size < 1 || !font_data) return NULL;

    FontAtlas fontatlas = malloc(sizeof(FontAtlas_t));
    fontatlas->font = font_data;
    fontatlas->font_size = font_data_size;

    FT_Error error;

    if ((error = FT_Init_FreeType(&fontatlas->lib))) {
        fprintf(stderr, "fontatlas_init() could not init FreeType Library: %s\n", FT_Error_String(error));
        goto L_failed;
    }

    if ((error = FT_New_Memory_Face(fontatlas->lib, font_data, font_data_size, 0, &fontatlas->face))) {
        fprintf(stderr, "fontatlas_init() failed to load font: %s\n", FT_Error_String(error));
        goto L_failed;
    }

    FT_Select_Charmap(fontatlas->face, FT_ENCODING_UNICODE);

    return fontatlas;

L_failed:
    free(fontatlas);
    return NULL;
}

void fontatlas_destroy(FontAtlas* fontatlas) {
    FontAtlas obj = *fontatlas;

    FT_Done_Face(obj->face);
    FT_Done_FreeType(obj->lib);

    free(obj);
    *fontatlas = NULL;
}


static inline bool pick_glyph(FT_Face face, CharData* chardata, uint32_t codepoint) {
    FT_ULong charcode;
    FT_UInt glyph_index = 0x0000;

    charcode = FT_Get_First_Char(face, &glyph_index);
    while (glyph_index != 0) {
        if (charcode == codepoint) break;
        charcode = FT_Get_Next_Char(face, charcode, &glyph_index);
    }

    if (glyph_index == 0) {
        // no glyph for the codepoint
        goto L_failed;
    }

    FT_Error error = FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER);
    if (error) {
        fprintf(
            stderr,
            "fontatlas_atlas_build() Failed to load glyph for codepoint %u, error: %s\n",
            codepoint,
            FT_Error_String(error)
        );

        goto L_failed;
    }

    // Note: the glyph data is loaded in "fontatlas->face->glyph" field
    chardata->glyph_index = glyph_index;
    chardata->codepoint = codepoint;
    return false;

L_failed:
    chardata->width = chardata->height = 0;
    chardata->codepoint = 0x0000;
    return true;
}

static inline void pick_bitmap(FT_Face face, CharData* chardata, FT_GlyphSlot glyph) {
    FT_Render_Mode render_mode = sdf_enabled ? FT_RENDER_MODE_SDF : FT_RENDER_MODE_NORMAL;
    FT_Error error = FT_Render_Glyph(glyph, render_mode);

    if (error) {
        /*fprintf(
            stderr,
            "fontatlas_atlas_build() Failed to pick the glyph bitmap of codepoint %u, error:%s\n",
            chardata->codepoint,
            FT_Error_String(error)
        );*/
        chardata->bitmap = NULL;
        return;
    }

    unsigned char* buffer = glyph->bitmap.buffer;
    unsigned int width = glyph->bitmap.width;
    unsigned int height = glyph->bitmap.rows;
    size_t texture_size = width * height;

    if (texture_size < 1 || !buffer) {
        chardata->bitmap = NULL;
        return;
    }

    chardata->bitmap = malloc(texture_size);
    assert(chardata->bitmap);

    memcpy(chardata->bitmap, buffer, texture_size);

    if (glyph->bitmap.pixel_mode != FT_PIXEL_MODE_GRAY) {
        fprintf(
            stderr,
            "fontatlas_atlas_build() Warning: glyph pixel_mode is not FT_PIXEL_MODE_GRAY in  codepoint %u\n",
            chardata->codepoint
        );
    }
}

static inline void pick_metrics(FT_GlyphSlot glyph, CharData* chardata, int32_t font_height) {
    // just in case, this never should happen
    assert(glyph);

    chardata->width = glyph->bitmap.width;// glyph->metrics.width >> 6;
    chardata->height = glyph->bitmap.rows;// glyph->metrics.height >> 6;
    chardata->offset_x = glyph->bitmap_left;
    chardata->offset_y = font_height - glyph->bitmap_top;
    chardata->advancex = glyph->metrics.horiAdvance >> 6;
    chardata->kernings = NULL;
    chardata->kernings_size = 0;
    chardata->has_atlas_entry = false;
}

static inline void pick_kernings(FT_Face face, CharData* chardata_array, int32_t chardata_array_size) {
    FT_Vector kerning;

    for (size_t i = 0; i < chardata_array_size; i++) {
        FT_UInt current = chardata_array[i].glyph_index;
        size_t kernings_count = 0;

        for (size_t j = 0; j < chardata_array_size; j++) {
            FT_UInt previous = chardata_array[j].glyph_index;

            FT_Error error = FT_Get_Kerning(face, previous, current, FT_KERNING_DEFAULT, &kerning);
            if (error) {
                /*fprintf(
                    stderr,
                    "fontatlas_atlas_build() cannot load kernigs of previous=%u current=%u: %s",
                    previous,
                    current,
                    FT_Error_String(error);
                );*/
                continue;
            }

            FT_Pos kerning_x = kerning.x >> 6;
            if (kerning_x == 0) continue;

            kernings_count++;
        }

        if (kernings_count < 1) {
            chardata_array[i].kernings_size = 0;
            chardata_array[i].kernings = NULL;
            continue;
        }

        size_t kerning_index = 0;
        FontCharDataKerning* kernings = malloc(sizeof(FontCharDataKerning) * kernings_count);
        assert(kernings);

        for (size_t j = 0; j < chardata_array_size; j++) {
            FT_UInt previous = chardata_array[j].glyph_index;

            if (FT_Get_Kerning(face, previous, current, FT_KERNING_DEFAULT, &kerning)) continue;

            FT_Pos kerning_x = kerning.x >> 6;
            if (kerning_x == 0) continue;

            kernings[kerning_index].codepoint = chardata_array[j].codepoint;
            kernings[kerning_index].x = kerning.x >> 6;
            kerning_index++;
        }

        chardata_array[i].kernings_size = kernings_count;
        chardata_array[i].kernings = kernings;
    }
}

static inline void build_atlas(CharData* array, int32_t array_size, int32_t gaps, int32_t max_dimmen, TextureAtlas* atlas) {
    CharData** rects = malloc(sizeof(void*) * array_size);

    for (size_t i = 0; i < array_size; i++) {
        array[i].has_atlas_entry = false;
        array[i].atlas_entry.x = 0;
        array[i].atlas_entry.y = 0;
        rects[i] = array + i;
    }

    qsort(rects, array_size, sizeof(CharData**), sort);

    int32_t x = 0;
    int32_t y = 0;
    int32_t max_row_height = 0;
    int32_t max_width = 0;
    int32_t max_height = 0;

    for (size_t i = 0; i < array_size;i++) {
        CharData* chardata = rects[i];

        if (!chardata->codepoint) continue;
        if (chardata->width < 1 || chardata->height < 1) continue;

        int32_t rect_width = chardata->width + gaps;
        int32_t rect_height = chardata->height + gaps;

        int32_t right = x + rect_width;
        if (right > max_dimmen) {
            y += max_row_height;
            x = 0;
            max_row_height = 0;
        }

        int32_t bottom = y + rect_height;
        if (bottom > max_dimmen) break;

        chardata->atlas_entry.x = x;
        chardata->atlas_entry.y = y;
        x += rect_width;

        if (rect_height > max_row_height) max_row_height = rect_height;
        if (right > max_width) max_width = right;
        if (bottom > max_height) max_height = bottom;

        chardata->has_atlas_entry = true;
    }

    free(rects);

    atlas->width = max_width;
    atlas->height = max_height;
}

static inline void put_in_atlas_texture(CharData* chardata, TextureAtlas* atlas) {
    if (!atlas->texture || !chardata->has_atlas_entry || !chardata->bitmap) return;

    unsigned char* buffer = chardata->bitmap;
    unsigned int char_width = chardata->width;
    unsigned int char_height = chardata->height;
    int32_t atlas_y = chardata->atlas_entry.y;

    for (unsigned int y = 0; y < char_height; y++) {
        int32_t offset = chardata->atlas_entry.x + (atlas_y * atlas->width);
        if (offset >= atlas->texture_byte_size) break;

        memcpy(atlas->texture + offset, buffer, char_width);

        buffer += char_width;
        atlas_y++;
    }

    free(chardata->bitmap);
    chardata->bitmap = NULL;
}


FontCharMap* fontatlas_atlas_build(FontAtlas fontatlas, uint8_t font_height, uint8_t gaps, uint32_t* codepoints_to_add) {
    if (!codepoints_to_add || !*codepoints_to_add) return NULL;

    int32_t codepoints_parsed = 0;
    int32_t codepoints_count = 0;
    for (size_t i = 0;codepoints_to_add[i]; i++) codepoints_count++;

    CharData* chardata = malloc(sizeof(CharData) * codepoints_count);
    FT_Error error;
    TextureAtlas atlas = { .texture_byte_size = 0, .texture = NULL , .width = 0, .height = 0 };
    const FT_GlyphSlot glyph = fontatlas->face->glyph;

    assert(chardata);

    if ((error = FT_Set_Pixel_Sizes(fontatlas->face, 0, font_height))) {
        fprintf(
            stderr,
            "fontatlas_atlas_build() Failed to use font_height %u using 64 instead, error: %s\n",
            font_height,
            FT_Error_String(error)
        );

        // fallback to 64px
        FT_Set_Pixel_Sizes(fontatlas->face, 0, 64);
    }

    for (size_t i = 0; i < codepoints_count; i++) {
        if (pick_glyph(fontatlas->face, &chardata[i], codepoints_to_add[i])) continue;
        pick_bitmap(fontatlas->face, &chardata[i], glyph);
        pick_metrics(glyph, &chardata[i], font_height);
        codepoints_parsed++;
    }

    for (size_t i = 0; i < codepoints_count; i++) {
        pick_kernings(fontatlas->face, chardata, codepoints_count);
    }

    if (codepoints_parsed < 1) goto L_build_map;

    // calculate texture dimmensions
    build_atlas(chardata, codepoints_count, gaps, MAX_TEXTURE_DIMMEN, &atlas);
    if (atlas.width < 1 || atlas.height < 1) goto L_build_map;

    // calculate power of two dimmensions
    atlas.width = math2d_poweroftwo_calc(atlas.width);
    atlas.height = math2d_poweroftwo_calc(atlas.height);
    if (atlas.width > MAX_TEXTURE_DIMMEN) atlas.width = MAX_TEXTURE_DIMMEN;
    if (atlas.height > MAX_TEXTURE_DIMMEN) atlas.height = MAX_TEXTURE_DIMMEN;

    // allocate texture
    atlas.texture_byte_size = atlas.width * atlas.height;
    atlas.texture = calloc(1, atlas.texture_byte_size);
    assert(/* Texture creation failed, not enough memory */atlas.texture);

    // place glyph bitmaps in the texture
    for (size_t i = 0; i < codepoints_count; i++) {
        if (!chardata[i].codepoint || chardata[i].width < 1 || chardata[i].height < 1) continue;
        put_in_atlas_texture(&chardata[i], &atlas);
    }


L_build_map:
    {};// workaround for emscripten (wont compile if the label is on top of variable declaration)
    FontCharMap* obj = malloc(sizeof(FontCharMap));
    obj->char_array_size = codepoints_parsed;
    obj->char_array = malloc(sizeof(FontCharData) * codepoints_parsed);
    obj->texture_byte_size = atlas.texture_byte_size;
    obj->texture = atlas.texture;
    obj->texture_width = atlas.width;
    obj->texture_height = atlas.height;
    obj->ascender = fontatlas->face->ascender / 64;

#if JAVASCRIPT
    obj->_ptrsize = sizeof(void*);
#endif

    if (codepoints_parsed > 0) assert(obj->char_array);

    // place character data in the map
    for (size_t i = 0, j = 0; i < codepoints_count; i++) {
        if (!chardata[i].codepoint) continue;

        obj->char_array[j].advancex = chardata[i].advancex;
        obj->char_array[j].atlas_entry = chardata[i].atlas_entry;
        obj->char_array[j].codepoint = chardata[i].codepoint;
        obj->char_array[j].has_atlas_entry = chardata[i].has_atlas_entry;
        obj->char_array[j].height = chardata[i].height;
        obj->char_array[j].kernings = chardata[i].kernings;
        obj->char_array[j].kernings_size = chardata[i].kernings_size;
        obj->char_array[j].offset_x = chardata[i].offset_x;
        obj->char_array[j].offset_y = chardata[i].offset_y;
        obj->char_array[j].width = chardata[i].width;
        j++;
    }

    free(chardata);

    return obj;
}

void fontatlas_atlas_destroy_texture_only(FontCharMap* fontcharmap) {
    if (!fontcharmap || !fontcharmap->texture) return;

    free(fontcharmap->texture);
    fontcharmap->texture = NULL;
    fontcharmap->texture_byte_size = 0;
    fontcharmap->texture_width = 0;
    fontcharmap->texture_height = 0;
}

void fontatlas_atlas_destroy(FontCharMap** fontcharmap) {
    if (!fontcharmap || !*fontcharmap) return;

    FontCharMap* obj = *fontcharmap;

    for (size_t i = 0;i < obj->char_array_size;i++) {
        if (!obj->char_array[i].kernings) continue;
        free(obj->char_array[i].kernings);
    }
    if (obj->texture) free(obj->texture);
    free(obj->char_array);

    free(obj);
    *fontcharmap = NULL;
}

FontCharMap* fontatlas_atlas_build_complete(FontAtlas fontatlas, uint8_t font_height, int8_t gaps) {
    uint32_t size_common = wcslen(FONTATLAS_BASE_LIST_COMMON);
    uint32_t size_extended = wcslen(FONTATLAS_BASE_LIST_EXTENDED);
    uint32_t size_total = size_extended + size_common;
    uint32_t* list_complete = malloc((size_total + 1) * sizeof(uint32_t));
    uint32_t j = 0;

    for (size_t i = 0; i < size_common; i++)  list_complete[j++] = FONTATLAS_BASE_LIST_COMMON[i];
    for (size_t i = 0; i < size_extended; i++) list_complete[j++] = FONTATLAS_BASE_LIST_EXTENDED[i];
    list_complete[size_total] = 0;

    FontCharMap* charmap = fontatlas_atlas_build(fontatlas, font_height, gaps, list_complete);
    free(list_complete);

    return charmap;
}

static char version[16];
const char* fontatlas_get_version() {
    FT_Library lib;
    FT_Int amajor, aminor, apatch;

    int error = FT_Init_FreeType(&lib);
    if (error != 0) return NULL;
    FT_Library_Version(lib, &amajor, &aminor, &apatch);
    FT_Done_FreeType(lib);

    int size = snprintf(version, sizeof(version), "%i.%i.%i", amajor, aminor, apatch);
    version[size] = '\0';

    return version;
}

#ifdef JAVASCRIPT
void fontatlas_destroy_JS(FontAtlas fontatlas) {
    FontAtlas obj = fontatlas;
    fontatlas_destroy(&obj);
}
void fontatlas_atlas_destroy_JS(FontCharMap* fontcharmap) {
    FontCharMap* obj = fontcharmap;
    fontatlas_atlas_destroy(&obj);
}
#endif

