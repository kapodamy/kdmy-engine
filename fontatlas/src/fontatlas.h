#ifndef __fontatlas_h
#define __fontatlas_h

#include <stdint.h>

#include "ft2build.h"
#include FT_FREETYPE_H

typedef struct __attribute__((packed)) {
    uint32_t codepoint;
    int32_t x;
} FontCharDataKerning;

typedef struct __attribute__((packed)) {
    uint16_t x;
    uint16_t y;
} FontCharDataAtlasEntry;

typedef struct __attribute__((packed)) {
    uint32_t codepoint;
    int16_t offset_x;
    int16_t offset_y;
    int16_t advancex;
    int32_t width;
    int32_t height;
    FontCharDataKerning* kernings;
    int32_t kernings_size;
    FontCharDataAtlasEntry atlas_entry;
    uint8_t has_atlas_entry;
} FontCharData;


typedef struct __attribute__((packed)) {
#if JAVASCRIPT
	uint32_t _ptrsize;
#endif
    FontCharData* char_array;
    int32_t char_array_size;
    uint8_t* texture;
    uint16_t texture_width;
    uint16_t texture_height;
    uint32_t texture_byte_size;
    int16_t ascender;
} FontCharMap;

typedef struct {
    uint8_t* font;
    int32_t font_size;
    FT_Library lib;
    FT_Face face;
} FontAtlas_t;

typedef FontAtlas_t* FontAtlas;

void fontatlas_enable_sdf(bool enable);
FontAtlas fontatlas_init(uint8_t* font_data, int32_t font_data_size);
void fontatlas_destroy(FontAtlas* fontatlas);
FontCharMap* fontatlas_atlas_build(FontAtlas fontatlas, uint8_t font_height, uint8_t gaps, uint32_t* characters_to_add);
void fontatlas_atlas_destroy_texture_only(FontCharMap* fontcharmap);
void fontatlas_atlas_destroy(FontCharMap** fontcharmap);
const char* fontatlas_get_version();

#define FONTATLAS_BASE_LIST_COMMON L"  !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"
#define FONTATLAS_BASE_LIST_EXTENDED L"¿¡¢¥¦¤§¨©ª«»¬®¯°±´³²¹ºµ¶·ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßŸàáâãäåæçèéêëìíîïðñòóôõö×øùúûüýþßÿ"


#endif

