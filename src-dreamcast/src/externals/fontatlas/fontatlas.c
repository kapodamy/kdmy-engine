#include "externals/fontatlas.h"

#include <assert.h>
#include <stdbool.h>

//
// Note:
//      freetype header files are loaded from "${env:DREAMSDK_HOME}/msys/1.0/opt/toolchains/dc/kos-ports"
//      if not present use DreamSDK Manager to download it
//
#include "freetype2/ft2build.h"
#include FT_FREETYPE_H
#include FT_BITMAP_H
#include FT_IMAGE_H
#include FT_GLYPH_H
#include FT_ERRORS_H
#include FT_OUTLINE_H

#include "logger.h"
#include "malloc_utils.h"
#include "math2d.h"
#include "number_format_specifiers.h"


typedef struct {
    uint8_t* texture;
    int32_t texture_byte_size;
    int32_t width;
    int32_t height;
} TextureAtlas;

typedef struct {
    unsigned char* bitmap;
    FT_UInt glyph_index;
} CharInfo;

struct FontAtlas_s {
    FT_Library lib;
    FT_Face face;
};

const wchar_t* FONTATLAS_BASE_LIST_COMMON = L"  !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
const wchar_t* FONTATLAS_BASE_LIST_EXTENDED = L"¿¡¢¥¦¤§¨©ª«»¬®¯°±´³²¹ºµ¶·ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßŸàáâãäåæçèéêëìíîïðñòóôõö×øùúûüýþßÿ";


#ifdef _arch_dreamcast
const int32_t FONTATLAS_MAX_TEXTURE_DIMMEN = 1024;
#else
const int32_t FONTATLAS_MAX_TEXTURE_DIMMEN = 2048;
#endif

#ifndef _arch_dreamcast
static bool sdf_enabled = false;
void fontatlas_enable_sdf(bool enable) {
    sdf_enabled = enable;
}
#endif


static int sort(const void* a, const void* b) {
    const FontCharData** a_ptr = (const FontCharData**)a;
    const FontCharData** b_ptr = (const FontCharData**)b;

    return (*a_ptr)->height - (*b_ptr)->height;
}

static int sort_codepoints(const void* a, const void* b) {
    const uint32_t* a_ptr = (const uint32_t*)a;
    const uint32_t* b_ptr = (const uint32_t*)b;

    const uint32_t value_a = *a_ptr;
    const uint32_t value_b = *b_ptr;

    if (value_a < value_b)
        return -1;
    if (value_a > value_b)
        return 1;
    else
        return 0;
}


FontAtlas fontatlas_init(const uint8_t* font_data, int32_t font_data_size) {
    if (font_data_size < 1 || !font_data) return NULL;

    // the "glyph_index" must fit into "codepoint" field
    _Static_assert(sizeof(FT_UInt) >= sizeof(uint32_t), "sizeof(FT_UInt) >= sizeof(uint32_t)");

    FontAtlas fontatlas = malloc_chk(sizeof(struct FontAtlas_s));
    malloc_assert(fontatlas, FontAtlas);

    FT_Error error;
    if ((error = FT_Init_FreeType(&fontatlas->lib))) {
        logger_error("fontatlas_init() could not init FreeType Library: %s\n", FT_Error_String(error));
        goto L_failed;
    }

    if ((error = FT_New_Memory_Face(fontatlas->lib, font_data, font_data_size, 0, &fontatlas->face))) {
        FT_Done_FreeType(fontatlas->lib);
        logger_error("fontatlas_init() failed to load font: %s\n", FT_Error_String(error));
        goto L_failed;
    }

    FT_Select_Charmap(fontatlas->face, FT_ENCODING_UNICODE);

    return fontatlas;

L_failed:
    free_chk(fontatlas);
    return NULL;
}

void fontatlas_destroy(FontAtlas* fontatlas_ptr) {
    FontAtlas fontatlas = *fontatlas_ptr;
    if (!fontatlas) return;

    FT_Done_Face(fontatlas->face);
    FT_Done_FreeType(fontatlas->lib);

    free_chk(fontatlas);
    *fontatlas_ptr = NULL;
}

#ifdef __wasm__
void fontatlas_destroy_JS(FontAtlas fontatlas) {
    fontatlas_destroy(&fontatlas);
}
#endif

#ifdef _arch_dreamcast
static inline int32_t convert_8bpp_to_4bpp(uint8_t* texture_8bpp, int32_t texture_byte_size) {
    texture_byte_size /= 2;

    uint8_t* ptr_4bpp = texture_8bpp;
    uint8_t* ptr_4bpp_end = ptr_4bpp + texture_byte_size;

    for (; ptr_4bpp < ptr_4bpp_end; ptr_4bpp++) {
        uint8_t texel1 = *texture_8bpp++;
        uint8_t texel2 = *texture_8bpp++;
        *ptr_4bpp = (texel1 & 0xF0) | (texel2 >> 4);
    }

    return texture_byte_size;
}
#endif

static bool pick_glyph(FT_Face face, FontCharData* chardata, uint32_t codepoint) {
    FT_ULong charcode;
    FT_UInt glyph_index = 0x0000;

    charcode = FT_Get_First_Char(face, &glyph_index);
    while (glyph_index != 0) {
        if (charcode == codepoint) break;
        charcode = FT_Get_Next_Char(face, charcode, &glyph_index);
    }

    if (glyph_index == 0) {
        // no glyph for the codepoint
        return true;
    }

    FT_Error error = FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER | FT_LOAD_IGNORE_TRANSFORM);
    if (error) {
        logger_error(
            "fontatlas_atlas_build() Failed to load glyph for codepoint " FMT_U4 ", error: %s\n",
            codepoint,
            FT_Error_String(error)
        );

        return true;
    }

    //
    // Notes:
    //          * the glyph data is loaded in "fontatlas->face->glyph" field
    //          * "chardata->codepoint" will hold temporarily the glyph index
    //
    chardata->codepoint = glyph_index;
    return false;
}

static unsigned char* render_glyph_bitmap(FT_GlyphSlot glyph) {
    FT_Error error;

#ifdef _arch_dreamcast
    error = FT_Render_Glyph(glyph, FT_RENDER_MODE_NORMAL);
#else
    if (sdf_enabled) {
        // 2 pass rendering. This use "bsdf" renderer which offers better quality
        error = FT_Render_Glyph(glyph, FT_RENDER_MODE_NORMAL);
        if (!error) {
            error = FT_Render_Glyph(glyph, FT_RENDER_MODE_SDF);
        }
    } else {
        error = FT_Render_Glyph(glyph, FT_RENDER_MODE_NORMAL);
    }
#endif

    if (error != 0) {
        /*logger_error(
            "fontatlas_atlas_build() Failed to pick the glyph bitmap of glyph index: " FMT_U4 ". Reason: %s\n",
            glyph->glyph_index,
            FT_Error_String(error)
        );*/
        return NULL;
    }

    unsigned char* buffer = glyph->bitmap.buffer;
    if (!buffer) {
        // invisible glyph
        return NULL;
    }

    unsigned int width = glyph->bitmap.width;
    unsigned int height = glyph->bitmap.rows;
    if (width < 1 || height < 1) {
        // this never should happen
        return NULL;
    }

    if (glyph->bitmap.pixel_mode != FT_PIXEL_MODE_GRAY) {
        logger_warn(
            "fontatlas_atlas_build() the glyph pixel_mode is not FT_PIXEL_MODE_GRAY for glyph index: %u\n",
            glyph->glyph_index
        );
    }

    return buffer;
}

static void pick_metrics(FT_GlyphSlot glyph, FontCharData* chardata) {
    FT_BBox acbox;
    FT_Outline_Get_CBox(&glyph->outline, &acbox);

    *chardata = (FontCharData){
        .codepoint = chardata->codepoint,
        .width = glyph->bitmap.width,
        .height = glyph->bitmap.rows,
        .offset_x = (int16_t)glyph->bitmap_left,
        .offset_y = (int16_t)(-(acbox.yMax >> 6)),
        .advancex = glyph->advance.x >> 6,
        .advancey = glyph->advance.y >> 6,
        .kernings = NULL,
        .kernings_size = 0,
        .has_atlas_entry = false
    };
}


static inline FontKerning* allocate_kernings(FT_Face face, FontCharData* chardata_array, int32_t size) {
    FT_Vector kerning;
    size_t kernings_total = 0;

    // Note: "chardata_array[].codepoint" is holding the "glyph index" not the actual codepoint
    for (int32_t i = 0; i < size; i++) {
        FT_UInt current = chardata_array[i].codepoint;

        for (int32_t j = 0; j < size; j++) {
            FT_UInt previous = chardata_array[j].codepoint;

            FT_Error error = FT_Get_Kerning(face, previous, current, FT_KERNING_DEFAULT, &kerning);
            if (error != 0) {
                /*logger_error(
                    "fontatlas_atlas_build() cannot load kernigs of previous=" FMT_U4 " current=" FMT_U4 ": %s",
                    previous,
                    current,
                    FT_Error_String(error);
                );*/
                continue;
            }

            FT_Pos kerning_x = kerning.x >> 6;
            if (kerning_x == 0) continue;

            kernings_total++;
        }
    }

    FontKerning* kernings_array = malloc_for_array(FontKerning, kernings_total);
    return kernings_array;
}

static inline void pick_kernings(FT_Face face, FontCharData* array, int32_t size, FontKerning* kerns_ptr, uint32_t* codepoints) {
    FT_Vector kerning;

    //
    // Notes:
    //          * "chardata_array[].codepoint" is holding the "glyph index" not the actual codepoint.
    //          * use "codepoints[]" to retrieve the glyph codepoint
    //
    for (int32_t i = 0; i < size; i++) {
        FT_UInt current = array[i].codepoint;
        int32_t kernings_count = 0;

        array[i].kernings = kerns_ptr;

        for (int32_t j = 0; j < size; j++) {
            FT_UInt previous = array[j].codepoint;

            if (FT_Get_Kerning(face, previous, current, FT_KERNING_DEFAULT, &kerning) != 0) continue;

            FT_Pos kerning_x = kerning.x >> 6;
            if (kerning_x == 0) continue;

            *kerns_ptr = (FontKerning){.codepoint = codepoints[j], .x = kerning_x};
            kerns_ptr++;
            kernings_count++;
        }

        array[i].kernings_size = kernings_count;
        if (kernings_count < 1) array[i].kernings = NULL;
    }
}

static inline void build_atlas(FontCharData* array, int32_t array_size, int32_t gaps, int32_t max_dimmen, TextureAtlas* atlas) {
    FontCharData** rects = malloc_for_array(FontCharData*, array_size);

    int32_t surface_needed = gaps * array_size * 2;

    for (int32_t i = 0; i < array_size; i++) {
        array[i].has_atlas_entry = false;
        array[i].atlas_entry.x = 0;
        array[i].atlas_entry.y = 0;
        rects[i] = array + i;
        surface_needed += array[i].width * array[i].height;
    }

    // FIXME: try create an square atlas
    /*for (int32_t pow2_dimmen = 64; pow2_dimmen < max_dimmen; pow2_dimmen *= 2) {
        if (surface_needed < (pow2_dimmen * pow2_dimmen)) {
            max_dimmen = pow2_dimmen;
            break;
        }
    }*/
    (void)surface_needed;

    qsort(rects, (size_t)array_size, sizeof(FontCharData**), sort);

    int32_t x = 0;
    int32_t y = 0;
    int32_t max_row_height = 0;
    int32_t max_width = 0;
    int32_t max_height = 0;

    for (int32_t i = 0; i < array_size; i++) {
        FontCharData* chardata = rects[i];

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

    free_chk(rects);

    atlas->width = max_width;
    atlas->height = max_height;
}

static inline void place_in_texture(FontCharData* chardata, TextureAtlas* atlas, unsigned char* glyph_bitmap, int pitch) {
    if (!atlas->texture || !chardata->has_atlas_entry || !glyph_bitmap) return;

    size_t char_width = (size_t)chardata->width;
    int32_t char_height = chardata->height;
    int32_t atlas_y = chardata->atlas_entry.y;

    for (int32_t y = 0; y < char_height; y++) {
        int32_t offset = chardata->atlas_entry.x + (atlas_y * atlas->width);
        if (offset >= atlas->texture_byte_size) break;

        memcpy(atlas->texture + offset, glyph_bitmap, char_width);

        glyph_bitmap += pitch;
        atlas_y++;
    }
}


FontCharMap* fontatlas_atlas_build(FontAtlas fontatlas, uint8_t font_height, int8_t gaps, uint32_t* codepoints_to_add) {
    if (!codepoints_to_add || !*codepoints_to_add) return NULL;

    FontCharMap* charmap;
    FontKerning* kernings_array = NULL;

    FT_Error error;
    if ((error = FT_Set_Pixel_Sizes(fontatlas->face, 0, font_height))) {
        logger_error(
            "fontatlas_atlas_build() Failed to use font_height " FMT_I4 "px, using 64px instead. Reason: %s\n",
            (int32_t)font_height,
            FT_Error_String(error)
        );

        return NULL;
    }

    int32_t codepoints_count = 0;
    for (size_t i = 0; codepoints_to_add[i]; i++) codepoints_count++;

    const FT_GlyphSlot glyph = fontatlas->face->glyph;
    TextureAtlas atlas = (TextureAtlas){.texture_byte_size = 0, .texture = NULL, .width = 0, .height = 0};

    FontCharData* chardata = malloc_for_array(FontCharData, codepoints_count);

    int32_t codepoints_parsed = 0;
    for (int32_t i = 0; i < codepoints_count; i++) {
        if (pick_glyph(fontatlas->face, &chardata[i], codepoints_to_add[i])) {
            // glyph not available
            chardata[i].width = chardata[i].height = 0;
            chardata[i].codepoint = 0x0000;
            continue;
        }

        // render the glyph and later pick the metrics
        render_glyph_bitmap(glyph);
        pick_metrics(glyph, &chardata[i]);
        codepoints_parsed++;
    }
    if (codepoints_parsed < 1) goto L_build_map;

    // pick kernings of each codepoint, the "kernings_array" will hold all kernings
    kernings_array = allocate_kernings(fontatlas->face, chardata, codepoints_count);
    pick_kernings(fontatlas->face, chardata, codepoints_count, kernings_array, codepoints_to_add);

    // calculate texture dimmensions and do bin packing
    build_atlas(chardata, codepoints_count, gaps, FONTATLAS_MAX_TEXTURE_DIMMEN, &atlas);
    if (atlas.width < 1 || atlas.height < 1) goto L_build_map;

    // calculate power-of-two texture dimmension
    atlas.width = math2d_poweroftwo_calc(atlas.width);
    atlas.height = math2d_poweroftwo_calc(atlas.height);
    if (atlas.width > FONTATLAS_MAX_TEXTURE_DIMMEN) atlas.width = FONTATLAS_MAX_TEXTURE_DIMMEN;
    if (atlas.height > FONTATLAS_MAX_TEXTURE_DIMMEN) atlas.height = FONTATLAS_MAX_TEXTURE_DIMMEN;

    // allocate texture
    atlas.texture_byte_size = atlas.width * atlas.height;
    atlas.texture = calloc_for_array(atlas.texture_byte_size, uint8_t);

    // place glyph bitmaps in the texture
    for (int32_t i = 0; i < codepoints_count; i++) {
        if (pick_glyph(fontatlas->face, &chardata[i], codepoints_to_add[i])) continue;

        // render again
        unsigned char* glyph_bitmap = render_glyph_bitmap(glyph);
        if (glyph_bitmap) {
            place_in_texture(&chardata[i], &atlas, glyph_bitmap, glyph->bitmap.pitch);
        }

        // replace the "glyph index" with the actual codepoint
        chardata[i].codepoint = codepoints_to_add[i];
    }

#if _arch_dreamcast
    // 8BPP --> 4BPP conversion
    atlas.texture_byte_size = convert_8bpp_to_4bpp(atlas.texture, atlas.texture_byte_size);
    atlas.texture = realloc_chk(atlas.texture, (size_t)atlas.texture_byte_size);
#endif

L_build_map:
    charmap = malloc_chk(sizeof(FontCharMap));
    malloc_assert(charmap, FontCharMap);

    *charmap = (FontCharMap){
        .char_array_size = codepoints_count,
        .char_array = chardata,
        .texture_byte_size = atlas.texture_byte_size,
        .texture = atlas.texture,
        .texture_width = atlas.width,
        .texture_height = atlas.height,
        .ascender = 0.0f,
        .line_height = font_height,
        .kernings_array = kernings_array,
    };

    FT_Size_Metrics* metrics = &fontatlas->face->size->metrics;
    charmap->ascender = math2d_max_int(metrics->ascender + metrics->descender, fontatlas->face->bbox.yMax) / 64.0f;

#ifndef _arch_dreamcast
    if (sdf_enabled) {
        // FIXME: offset of 10% ¿but why?
        charmap->ascender -= font_height * 0.10f;
    }
#endif

    return charmap;
}

void fontatlas_atlas_destroy_texture_only(FontCharMap* fontcharmap) {
    if (!fontcharmap || !fontcharmap->texture) return;

    free_chk(fontcharmap->texture);
    fontcharmap->texture = NULL;
    fontcharmap->texture_byte_size = 0;
    fontcharmap->texture_width = 0;
    fontcharmap->texture_height = 0;
}

void fontatlas_atlas_destroy(FontCharMap* fontcharmap) {
    if (!fontcharmap) return;

    free_chk(fontcharmap->texture);
    free_chk(fontcharmap->char_array);
    free_chk(fontcharmap->kernings_array);

    free_chk(fontcharmap);
}

FontCharMap* fontatlas_atlas_build_complete(FontAtlas fontatlas, uint8_t font_height, int8_t gaps) {
    size_t size_common = wcslen(FONTATLAS_BASE_LIST_COMMON);
    size_t size_extended = wcslen(FONTATLAS_BASE_LIST_EXTENDED);
    size_t size_total = size_common + size_extended;

    size_t list_complete_size = size_total + 1;
    uint32_t* list_complete = malloc_for_array(uint32_t, list_complete_size);

    uint32_t* list_ptr = list_complete;
    const wchar_t* common_ptr = FONTATLAS_BASE_LIST_COMMON;
    const wchar_t* extended_ptr = FONTATLAS_BASE_LIST_EXTENDED;

    for (; size_common > 0; size_common--) *list_ptr++ = (uint32_t)*common_ptr++;
    for (; size_extended > 0; size_extended--) *list_ptr++ = (uint32_t)*extended_ptr++;
    *list_ptr = 0;

    qsort(list_complete, size_total, sizeof(uint32_t), sort_codepoints);

    FontCharMap* charmap = fontatlas_atlas_build(fontatlas, font_height, gaps, list_complete);
    free_chk(list_complete);

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
