#ifndef _fontglyph_h
#define _fontglyph_h

#include "atlas.h"
#include "fontglyph_utils.h"
#include "pvrcontext.h"


typedef struct FontGlyph_s* FontGlyph;


FontGlyph fontglyph_init(const char* src_atlas, const char* suffix, bool allow_animation);
FontGlyph fontglyph_init2(Texture texture, Atlas atlas, const char* suffix, bool allow_animation);
void fontglyph_destroy(FontGlyph* fontglyph);

float fontglyph_measure(FontGlyph fontglyph, FontParams* params, const char* text, int32_t text_index, size_t text_length);
void fontglyph_measure_char(FontGlyph fontglyph, uint32_t codepoint, float height, FontLineInfo* lineinfo);
float fontglyph_draw_text(FontGlyph fontglyph, PVRContext pvrctx, FontParams* params, float x, float y, int32_t text_index, size_t text_length, const char* text);
int32_t fontglyph_animate(FontGlyph fontglyph, float elapsed);
void fontglyph_map_codepoints(FontGlyph fontglyph, const char* text, int32_t text_index, size_t text_length);

#endif
