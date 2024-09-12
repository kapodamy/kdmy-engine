#ifndef _drawable_h
#define _drawable_h

#include "drawable_types.h"
#include "layout_types.h"
#include "modifier.h"


typedef struct Drawable_s* Drawable;


Drawable drawable_init(float z, void* private_data, DelegateDraw callback_draw, DelegateAnimate callback_animate);
void drawable_destroy(Drawable* drawable);

void drawable_set_z_index(Drawable drawable, float z_index);
float drawable_get_z_index(Drawable drawable);
void drawable_set_z_offset(Drawable drawable, float offset);

void drawable_set_alpha(Drawable drawable, float alpha);
float drawable_get_alpha(Drawable drawable);
void drawable_set_offsetcolor(Drawable drawable, float r, float g, float b, float a);
void drawable_set_offsetcolor_to_default(Drawable drawable);
void drawable_set_visible(Drawable drawable, bool visible);
Modifier* drawable_get_modifier(Drawable drawable);
void drawable_draw(Drawable drawable, PVRContext pvrctx);
void drawable_change_private_data(Drawable drawable, void* private_data);
int32_t drawable_animate(Drawable drawable, float elapsed);
void drawable_set_property(Drawable drawable, int32_t property_id, float value);
void drawable_helper_apply_in_context(Drawable drawable, PVRContext pvrctx);
void drawable_helper_update_from_placeholder(Drawable drawable, const LayoutPlaceholder* layout_placeholder);
bool drawable_is_visible(Drawable drawable);
void drawable_get_draw_location(Drawable drawable, float* x, float* y);
void drawable_set_antialiasing(Drawable drawable, PVRFlag antialiasing);

//void drawable_set_shader(Drawable drawable, PSShader psshader);
//PSShader drawable_get_shader(Drawable drawable);

void drawable_blend_enable(Drawable drawable, bool enabled);
void drawable_blend_set(Drawable drawable, Blend src_rgb, Blend dst_rgb, Blend src_alpha, Blend dst_alpha);

#endif
