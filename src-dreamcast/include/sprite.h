#ifndef _sprite_h
#define _sprite_h

#include <stdbool.h>
#include <stdint.h>

#include "animsprite.h"
#include "atlas.h"
#include "modifier.h"
#include "pvrcontext.h"
#include "sh4matrix.h"
#include "texture.h"


typedef struct Sprite_s* Sprite;


void sprite_draw(Sprite sprite, PVRContext pvrctx);
Sprite sprite_init_from_rgb8(uint32_t solid_rgb8_color);
Sprite sprite_init(Texture src_texture);
void sprite_destroy(Sprite* sprite);
void sprite_destroy_all_animations(Sprite sprite);
void sprite_destroy_texture(Sprite sprite);
void sprite_destroy_full(Sprite* sprite);
Texture sprite_set_texture(Sprite sprite, Texture texture, bool update_offset_source_size);
Modifier* sprite_matrix_get_modifier(Sprite sprite);
void sprite_matrix_scale(Sprite sprite, float scale_x, float scale_y);
void sprite_matrix_rotate(Sprite sprite, float degrees_angle);
void sprite_matrix_skew(Sprite sprite, float skew_x, float skew_y);
void sprite_matrix_translate(Sprite sprite, float translate_x, float translate_y);
void sprite_set_offset_source(Sprite sprite, float x, float y, float width, float height);
void sprite_set_offset_frame(Sprite sprite, float x, float y, float width, float height);
void sprite_set_offset_pivot(Sprite sprite, float x, float y);
void sprite_flip_rendered_texture(Sprite sprite, nbool flip_x, nbool flip_y);
void sprite_flip_rendered_texture_enable_correction(Sprite sprite, bool enabled);
void sprite_matrix_reset(Sprite sprite);
void sprite_set_draw_location(Sprite sprite, float x, float y);
void sprite_set_draw_size(Sprite sprite, float width, float height);
void sprite_set_draw_size_from_source_size(Sprite sprite);
void sprite_set_alpha(Sprite sprite, float alpha);
float sprite_get_alpha(Sprite sprite);
void sprite_set_visible(Sprite sprite, bool visible);
void sprite_set_z_index(Sprite sprite, float index);
void sprite_set_z_offset(Sprite sprite, float offset);
float sprite_get_z_index(Sprite sprite);
void sprite_animation_add(Sprite sprite, AnimSprite animsprite);
void sprite_animation_remove(Sprite sprite, const char* animation_name);
void sprite_animation_restart(Sprite sprite);
bool sprite_animation_play(Sprite sprite, const char* animation_name);
void sprite_animation_play_by_index(Sprite sprite, int32_t index);
bool sprite_animation_play_by_animsprite(Sprite sprite, AnimSprite animsprite, bool only_if_attached);
void sprite_animation_play_first(Sprite sprite);
void sprite_animation_stop(Sprite sprite);
void sprite_animation_end(Sprite sprite);
int32_t sprite_animate(Sprite sprite, float elapsed);
AnimSprite sprite_animation_get_attached(Sprite sprite, const char* animation_name);

void sprite_get_source_size(Sprite sprite, float* width, float* height);
void sprite_get_draw_size(Sprite sprite, float* width, float* height);
void sprite_get_draw_location(Sprite sprite, float* x, float* y);
void sprite_set_property(Sprite sprite, int32_t property_id, float value);
void sprite_set_vertex_color_rgb8(Sprite sprite, uint32_t rbg8_color);
void sprite_set_vertex_color_rgba8(Sprite sprite, uint32_t rbga8_color);
void sprite_set_vertex_color(Sprite sprite, float r, float g, float b);
uint32_t sprite_get_vertex_color_rgb8(Sprite sprite);

void sprite_set_offsetcolor_rgba8(Sprite sprite, uint32_t rgba8_color);
void sprite_set_offsetcolor(Sprite sprite, float r, float g, float b, float a);

AnimSprite sprite_external_animation_set(Sprite sprite, AnimSprite animsprite);
void sprite_external_animation_restart(Sprite sprite);
void sprite_external_animation_end(Sprite sprite);
void sprite_resize_draw_size(Sprite sprite, float max_width, float max_height, float* applied_width, float* applied_height);
void sprite_center_draw_location(Sprite sprite, float x, float y, float ref_width, float ref_height, float* applied_x, float* applied_y);
bool sprite_is_textured(Sprite sprite);
bool sprite_is_visible(Sprite sprite);

void sprite_matrix_rotate_pivot_enable(Sprite sprite, bool enable);
void sprite_matrix_rotate_pivot(Sprite sprite, float u, float v);
void sprite_matrix_scale_offset(Sprite sprite, float direction_x, float direction_y);
void sprite_matrix_translate_rotation(Sprite sprite, bool enable_translate_rotation);
void sprite_matrix_scale_size(Sprite sprite, bool enable_scale_size);
void sprite_matrix_corner_rotation(Sprite sprite, Corner corner);
void sprite_set_draw_x(Sprite sprite, float value);
void sprite_set_draw_y(Sprite sprite, float value);

bool sprite_crop(Sprite sprite, bool dx, bool dy, bool dwidth, bool dheight);
bool sprite_is_crop_enabled(Sprite sprite);
void sprite_crop_enable(Sprite sprite, bool enable);
void sprite_fill_atlas_entry(Sprite sprite, AtlasEntry* target_atlas_entry);
Texture sprite_get_texture(Sprite sprite);
void sprite_set_antialiasing(Sprite sprite, PVRFlag antialiasing);

/*void sprite_set_shader(Sprite sprite, PSShader psshader);
PSShader sprite_get_shader(Sprite sprite);*/

void sprite_blend_enable(Sprite sprite, bool enabled);
void sprite_blend_set(Sprite sprite, Blend src_rgb, Blend dst_rgb, Blend src_alpha, Blend dst_alpha);
void sprite_trailing_enabled(Sprite sprite, bool enabled);
void sprite_trailing_set_params(Sprite sprite, int32_t length, float trail_delay, float trail_alpha, nbool darken_colors);
void sprite_trailing_set_offsetcolor(Sprite sprite, float r, float g, float b);

#endif