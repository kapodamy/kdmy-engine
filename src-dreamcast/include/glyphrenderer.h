#ifndef _glyphrenderer_h
#define _glyphrenderer_h

#include <stdbool.h>
#include <stdint.h>


#include "pvrcontext_types.h"
#include "texture.h"


int32_t glyphrenderer_prepare(PVRContext pvrctx, int32_t glyph_count, bool has_outlines, RGBA tint, RGBA outline, bool by_add, bool is_gryscl, Texture tex0, Texture tex1);
void glyphrenderer_draw_glyph(bool is_tex1, bool is_outline, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh);

#endif
