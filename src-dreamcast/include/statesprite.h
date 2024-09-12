#ifndef _statesprite_h
#define _statesprite_h

#include <stdbool.h>
#include <stdint.h>

#include "animsprite.h"
#include "atlas.h"
#include "modelholder.h"
#include "modifier.h"
#include "pvrcontext.h"
#include "sh4matrix.h"
#include "texture.h"
#include "linkedlist.h"


typedef struct {
    Texture texture;
    AnimSprite animation;
    int32_t statesprite_id;
    const char* state_name;
    uint32_t vertex_color;
    AtlasEntry* frame_info;
    CornerRotation corner_rotation;
    float offset_x;
    float draw_width;
    float draw_height;
    float offset_y;
} StateSpriteState;

typedef struct StateSprite_s* StateSprite;


void statesprite_draw(StateSprite statesprite, PVRContext pvrctx);

StateSprite statesprite_init_from_vertex_color(uint32_t rgb8_color);
StateSprite statesprite_init_from_texture(Texture texture);
bool statesprite_destroy_texture_if_stateless(StateSprite statesprite);
void statesprite_destroy(StateSprite* statesprite);

Texture statesprite_set_texture(StateSprite statesprite, Texture texture, bool update_offset_source_size);
void statesprite_set_offset_source(StateSprite statesprite, float x, float y, float width, float height);
void statesprite_set_offset_frame(StateSprite statesprite, float x, float y, float width, float height);
void statesprite_set_offset_pivot(StateSprite statesprite, float x, float y);

void statesprite_set_draw_location(StateSprite statesprite, float x, float y);
void statesprite_set_draw_size(StateSprite statesprite, float width, float height);
void statesprite_set_alpha(StateSprite statesprite, float alpha);
void statesprite_set_visible(StateSprite statesprite, bool visible);
void statesprite_set_z_index(StateSprite statesprite, float index);
void statesprite_set_z_offset(StateSprite statesprite, float offset);
void statesprite_set_property(StateSprite statesprite, int32_t property_id, float value);
void statesprite_set_offsetcolor(StateSprite statesprite, float r, float g, float b, float a);
void statesprite_set_vertex_color(StateSprite statesprite, float r, float g, float b);
void statesprite_set_vertex_color_rgb8(StateSprite statesprite, uint32_t rbg8_color);

AnimSprite statesprite_animation_set(StateSprite statesprite, AnimSprite animsprite);
void statesprite_animation_restart(StateSprite statesprite);
void statesprite_animation_end(StateSprite statesprite);
bool statesprite_animation_completed(StateSprite statesprite);
int32_t statesprite_animate(StateSprite statesprite, float elapsed);

float statesprite_get_z_index(StateSprite statesprite);
Modifier* statesprite_get_modifier(StateSprite statesprite);
void statesprite_get_draw_size(StateSprite statesprite, float* width, float* height);
void statesprite_get_draw_location(StateSprite statesprite, float* x, float* y);
float statesprite_get_alpha(StateSprite statesprite);
void statesprite_get_offset_source_size(StateSprite statesprite, float* width, float* height);
bool statesprite_is_visible(StateSprite statesprite);

void statesprite_matrix_reset(StateSprite statesprite);
void statesprite_matrix_scale(StateSprite statesprite, float scale_x, float scale_y);
void statesprite_matrix_rotate(StateSprite statesprite, float degrees_angle);
void statesprite_matrix_skew(StateSprite statesprite, float skew_x, float skew_y);
void statesprite_matrix_translate(StateSprite statesprite, float translate_x, float translate_y);
void statesprite_matrix_rotate_pivot_enable(StateSprite statesprite, bool enable);
void statesprite_matrix_rotate_pivot(StateSprite statesprite, float u, float v);
void statesprite_matrix_scale_offset(StateSprite statesprite, float direction_x, float direction_y);
void statesprite_matrix_translate_rotation(StateSprite statesprite, bool enable_translate_rotation);

StateSpriteState* statesprite_state_add(StateSprite statesprite, ModelHolder modelholder, const char* animation_name, const char* state_name);
StateSpriteState* statesprite_state_add2(StateSprite statesprite, Texture texture, AnimSprite animsprt, const AtlasEntry* atlas_entry, uint32_t rgb8_color, const char* state_name);
bool statesprite_state_toggle(StateSprite statesprite, const char* state_name);
LinkedList statesprite_state_list(StateSprite statesprite);
const char* statesprite_state_name(StateSprite statesprite);
bool statesprite_state_has(StateSprite statesprite, const char* state_name);
StateSpriteState* statesprite_state_get(StateSprite statesprite);
bool statesprite_state_apply(StateSprite statesprite, const StateSpriteState* statesprite_state);
void statesprite_state_get_offsets(StateSprite statesprite, float* offset_x, float* offset_y);
void statesprite_state_set_offsets(StateSprite statesprite, float offset_x, float offset_y, bool update_state);
void statesprite_state_remove(StateSprite statesprite, const char* state_name);

void statesprite_resize_draw_size(StateSprite statesprite, float max_width, float max_height, float* applied_width, float* applied_height);
void statesprite_center_draw_location(StateSprite statesprite, float x, float y, float ref_width, float ref_height, float* applied_x, float* applied_y);
void statesprite_corner_rotate(StateSprite statesprite, Corner corner);

bool statesprite_crop(StateSprite statesprite, float dx, float dy, float dwidth, float dheight);
bool statesprite_is_crop_enabled(StateSprite statesprite);
void statesprite_crop_enable(StateSprite statesprite, bool enable);
void statesprite_flip_texture(StateSprite statesprite, nbool flip_x, nbool flip_y);
void statesprite_flip_texture_enable_correction(StateSprite statesprite, bool enabled);
void statesprite_change_draw_size_in_atlas_apply(StateSprite statesprite, bool enable, float scale_factor);
void statesprite_set_antialiasing(StateSprite statesprite, PVRFlag antialiasing);

// void statesprite_set_shader(StateSprite statesprite, PSShader psshader);
// PSShader statesprite_get_shader(StateSprite statesprite);

void statesprite_blend_enable(StateSprite statesprite, bool enabled);
void statesprite_blend_set(StateSprite statesprite, Blend src_rgb, Blend dst_rgb, Blend src_alpha, Blend dst_alpha);
void statesprite_trailing_enabled(StateSprite statesprite, bool enabled);
void statesprite_trailing_set_params(StateSprite statesprite, int32_t length, float trail_delay, float trail_alpha, nbool darken_colors);
void statesprite_trailing_set_offsetcolor(StateSprite statesprite, float r, float g, float b);

#endif
