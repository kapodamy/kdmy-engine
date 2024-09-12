#ifndef _imgutils_h
#define _imgutils_h

#include "sprite.h"
#include "statesprite.h"
#include "texture.h"
#include "vertexprops_types.h"

void imgutils_calc_size(float orig_width, float orig_height, float max_width, float max_height, float* result_width, float* result_height);
void imgutils_calc_size2(Texture texture, const AtlasEntry* atlas_entry, float max_width, float max_height, float* result_width, float* result_height);
void imgutils_calc_centered_location(float x, float y, float width, float height, float ref_width, float ref_height, float* location_x, float* location_y);
void imgutils_get_statesprite_original_size(StateSpriteState* statesprite_state, float* output_width, float* output_height);
void imgutils_calc_rectangle(float x, float y, float max_width, float max_height, float src_width, float src_height, Align align_hrzntl, Align align_vrtcl, float* d_width, float* d_height, float* d_x, float* d_y);
void imgutils_calc_rectangle_in_sprite(float x, float y, float max_width, float max_height, Align align_hrzntl, Align align_vrtcl, Sprite sprite);
void imgutils_calc_rectangle_in_statesprite_state(float off_x, float off_y, float max_width, float max_height, Align align_hrzntl, Align align_vrtcl, StateSpriteState* statesprite_state);
void imgutils_calc_resize_sprite(Sprite sprite, float max_width, float max_height, bool cover, bool center);

#endif
