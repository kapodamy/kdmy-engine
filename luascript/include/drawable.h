#ifndef __drawable_h
#define __drawable_h

#include <stdint.h>
#include <stdbool.h>

#include "psshader.h"
#include "pvrctx.h"
#include "vertexprops.h"
#include "modifier.h"

typedef struct _Drawable_t {
    int dummy;
} Drawable_t;

typedef Drawable_t* Drawable;

void drawable_set_z_index(Drawable drawable, float z_index);
float drawable_get_z_index(Drawable drawable);
void drawable_set_z_offset(Drawable drawable, float offset);
void drawable_set_alpha(Drawable drawable, float alpha);
float drawable_get_alpha(Drawable drawable);
void drawable_set_offsetcolor(Drawable drawable, float r, float g, float b, float a);
void drawable_set_offsetcolor_to_default(Drawable drawable);
Modifier drawable_get_modifier(Drawable drawable);
void drawable_set_antialiasing(Drawable drawable, PVRFlag antialiasing);
void drawable_set_shader(Drawable drawable, PSShader psshader);
PSShader drawable_get_shader(Drawable drawable);
void drawable_blend_enable(Drawable drawable, bool enabled);
void drawable_blend_set(Drawable drawable, Blend src_rgb, Blend dst_rgb, Blend src_alpha, Blend dst_alpha);

#endif

