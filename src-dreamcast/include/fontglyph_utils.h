#ifndef _fontglyph_utils_h
#define _fontglyph_utils_h

#include <stdint.h>

#include "color2d.h"

#define FONTGLYPH_SPACE 0x20
#define FONTGLYPH_HARDSPACE 0xA0
#define FONTGLYPH_LINEFEED 0x0A
#define FONTGLYPH_TAB 0x09
#define FONTGLYPH_CARRIAGERETURN 0x0D
#define FONTGLYPH_TABSTOP 8

#define FONTGLYPH_LOOKUP_TABLE_LENGTH 128 // this covers all ascii characters


typedef struct FontLineInfo_s {
    float space_width;
    float last_char_width;
    int32_t line_char_count;
    uint32_t previous_codepoint;
} FontLineInfo;

typedef struct FontParams_s {
    float height;
    float paragraph_space;
    bool color_by_addition;

    RGBA tint_color;

    bool border_enable;
    float border_size;

    RGBA border_color;

    float border_offset_x;
    float border_offset_y;

} FontParams;


int32_t fontglyph_internal_calc_tabstop(int32_t);


#endif
