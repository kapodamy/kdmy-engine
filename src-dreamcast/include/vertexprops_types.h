#ifndef _vertexprops_types_h
#define _vertexprops_types_h


#define FONT_WORDBREAK_LOOSE 0
#define FONT_WORDBREAK_NONE 1
#define FONT_WORDBREAK_BREAK 2


typedef enum {
    ALIGN_INVALID = -1,
    ALIGN_START = 0,
    ALIGN_CENTER = 1,
    ALIGN_END = 2,
    ALIGN_BOTH = 3,
    ALIGN_NONE = 4
} Align;

// Note: some blending modes are not supported
typedef enum {
    BLEND_DEFAULT = 0,
    BLEND_ZERO = 1,
    BLEND_ONE = 2,
    //BLEND_SRC_COLOR = 3,
    //BLEND_ONE_MINUS_SRC_COLOR = 4,
    BLEND_DST_COLOR = 5,
    BLEND_ONE_MINUS_DST_COLOR = 6,
    BLEND_SRC_ALPHA = 7,
    BLEND_ONE_MINUS_SRC_ALPHA = 8,
    BLEND_DST_ALPHA = 9,
    BLEND_ONE_MINUS_DST_ALPHA = 10,
    //BLEND_CONSTANT_COLOR = 11,
    //BLEND_ONE_MINUS_CONSTANT_COLOR = 12,
    //BLEND_CONSTANT_ALPHA = 13,
    //BLEND_ONE_MINUS_CONSTANT_ALPHA = 14,
    //BLEND_SRC_ALPHA_SATURATE = 15
} Blend;

#endif
