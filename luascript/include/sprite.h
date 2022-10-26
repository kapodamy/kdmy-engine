#ifndef __sprite_h
#define __sprite_h

#include <stdbool.h>
#include "modifier.h"
#include "pvrctx.h"

typedef struct _Sprite_t {
    int dummy;
} Sprite_t;

typedef Sprite_t *Sprite;

Modifier sprite_matrix_get_modifier(Sprite sprite);
void sprite_set_offset_source(Sprite sprite, float x, float y, float width, float height);
void sprite_set_offset_frame(Sprite sprite, float x, float y, float width, float height);
void sprite_set_offset_pivot(Sprite sprite, float x, float y);
void sprite_matrix_reset(Sprite sprite);
void sprite_set_draw_location(Sprite sprite, float x, float y);
void sprite_set_draw_size(Sprite sprite, float width, float height);
void sprite_set_draw_size_from_source_size(Sprite sprite);
void sprite_set_alpha(Sprite sprite, float alpha);
void sprite_set_visible(Sprite sprite, bool visible);
void sprite_set_z_index(Sprite sprite, float index);
void sprite_set_z_offset(Sprite sprite, float offset);
float* sprite_get_source_size(Sprite sprite, float* size);
void sprite_set_vertex_color(Sprite sprite, float r, float g, float b);
void sprite_set_offsetcolor(Sprite sprite, float r, float g, float b, float a);
bool sprite_is_textured(Sprite sprite);
bool sprite_crop(Sprite sprite, float dx, float dy, float dwidth, float dheight);
bool sprite_is_crop_enabled(Sprite sprite);
void sprite_crop_enable(Sprite sprite, bool enable);
float* sprite_resize_draw_size(Sprite sprite, float max_width, float max_height, float* applied_draw_size);
float* sprite_center_draw_location(Sprite sprite, float x, float y, float ref_width, float ref_height, float* applied_draw_location);
void sprite_set_antialiasing(Sprite sprite, PVRFLAG antialiasing);
void sprite_flip_rendered_texture(Sprite sprite, int flip_x, int flip_y);
void sprite_flip_rendered_texture_enable_correction(Sprite sprite, bool enabled);
#endif