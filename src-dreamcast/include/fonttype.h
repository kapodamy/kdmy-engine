#ifndef _fonttype_h
#define _fonttype_h

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "fontglyph_utils.h"
#include "pvrcontext_types.h"


typedef struct FontType_s* FontType;


FontType fonttype_init(const char* src);
void fonttype_destroy(FontType* fonttype);

float fonttype_measure(FontType fonttype, FontParams* params, const char* text, int32_t text_index, size_t text_length);
void fonttype_measure_char(FontType fonttype, uint32_t codepoint, float height, FontLineInfo* lineinfo);
float fonttype_draw_text(FontType fonttype, PVRContext pvrctx, FontParams* params, float x, float y, int32_t text_index, size_t text_length, const char* text);
void fonttype_map_codepoints(FontType fonttype, const char* text, int32_t text_index, size_t text_end_index);
int32_t fonttype_animate(FontType fonttype, float elapsed);

#endif
