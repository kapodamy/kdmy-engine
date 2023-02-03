#ifndef __vertexprops_h
#define __vertexprops_h

#define ALIGN_INVALID -1
#define ALIGN_START 0
#define ALIGN_CENTER 1
#define ALIGN_END 2
#define ALIGN_BOTH 3
#define ALIGN_NONE 4
typedef int32_t Align;

#define TEXTSPRITE_FORCECASE_NONE 0
#define TEXTSPRITE_FORCECASE_LOWER 1
#define TEXTSPRITE_FORCECASE_UPPER 2
typedef int32_t TextSpriteForceCase;

#define FONT_WORDBREAK_LOOSE 0
#define FONT_WORDBREAK_NONE 1
#define FONT_WORDBREAK_BREAK 2
typedef int32_t FontWordBreak;

#define BLEND_DEFAULT 0
#define BLEND_ZERO 1
#define BLEND_ONE 2
#define BLEND_SRC_COLOR 3
#define BLEND_ONE_MINUS_SRC_COLOR 4
#define BLEND_DST_COLOR 5
#define BLEND_ONE_MINUS_DST_COLOR 6
#define BLEND_SRC_ALPHA 7
#define BLEND_ONE_MINUS_SRC_ALPHA 8
#define BLEND_DST_ALPHA 9
#define BLEND_ONE_MINUS_DST_ALPHA 0
#define BLEND_CONSTANT_COLOR 1
#define BLEND_ONE_MINUS_CONSTANT_COLOR 2
#define BLEND_CONSTANT_ALPHA 3
#define BLEND_ONE_MINUS_CONSTANT_ALPHA 4
#define BLEND_SRC_ALPHA_SATURATE 5
typedef int32_t Blend;

#endif
