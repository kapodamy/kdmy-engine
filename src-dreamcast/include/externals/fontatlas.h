#ifndef __fontatlas_h
#define __fontatlas_h

#include <stdbool.h>
#include <stdint.h>
#include <wchar.h>

#ifdef __wasm__
#define _FONT_STRUCTS_ATTRIB __attribute__((packed))
#else
#define _FONT_STRUCTS_ATTRIB
#endif

typedef struct _FONT_STRUCTS_ATTRIB {
    uint32_t codepoint;
    int32_t x;
} FontKerning;

typedef struct _FONT_STRUCTS_ATTRIB {
    uint16_t x;
    uint16_t y;
} FontAtlasEntry;

typedef struct _FONT_STRUCTS_ATTRIB {
    uint32_t codepoint;
    int16_t offset_x;
    int16_t offset_y;
    int16_t advancex;
    int16_t advancey;
    uint16_t width;
    uint16_t height;
    FontKerning* kernings;
    int32_t kernings_size;
    FontAtlasEntry atlas_entry;
    uint8_t has_atlas_entry;
} FontCharData;

typedef struct _FONT_STRUCTS_ATTRIB {
    FontCharData* char_array;
    int32_t char_array_size;
    uint8_t* texture; // 8BPP texture format (4BPP on dreamcast)
    uint16_t texture_width;
    uint16_t texture_height;
    int32_t texture_byte_size;
    int16_t ascender;
    int16_t line_height;
    FontKerning* kernings_array;
} FontCharMap;

typedef struct FontAtlas_s* FontAtlas;


extern const wchar_t* FONTATLAS_BASE_LIST_COMMON;
extern const wchar_t* FONTATLAS_BASE_LIST_EXTENDED;


#ifndef _arch_dreamcast
void fontatlas_enable_sdf(bool enable);
#endif

FontAtlas fontatlas_init(const uint8_t* font_data, int32_t font_data_size);
void fontatlas_destroy(FontAtlas* fontatlas);
FontCharMap* fontatlas_atlas_build(FontAtlas fontatlas, uint8_t font_height, int8_t gaps, uint32_t* characters_to_add);
FontCharMap* fontatlas_atlas_build_complete(FontAtlas fontatlas, uint8_t font_height, int8_t gaps);
void fontatlas_atlas_destroy_texture_only(FontCharMap* fontcharmap);
void fontatlas_atlas_destroy(FontCharMap* fontcharmap);
const char* fontatlas_get_version();

#endif
