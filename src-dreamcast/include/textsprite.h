#ifndef _textsprite_h
#define _textsprite_h

#include <stdarg.h>
#include <stdint.h>

#include "animsprite.h"
#include "fontholder.h"
#include "pvrcontext_types.h"
#include "sh4matrix_types.h"


#define TEXTSPRITE_FORCE_NONE 0
#define TEXTSPRITE_FORCE_LOWERCASE 1
#define TEXTSPRITE_FORCE_UPPERCASE 2


typedef struct TextSprite_s* TextSprite;


TextSprite textsprite_init(void* font, bool font_is_truetype, bool color_by_addition, float size, uint32_t rbg8_color);
TextSprite textsprite_init2(FontHolder fontholder, float font_size, uint32_t rbg8_color);
void textsprite_destroy(TextSprite* textsprite);

void textsprite_set_text(TextSprite textsprite, const char* text);
void textsprite_set_text_intern(TextSprite textsprite, bool intern, const char* const* text);
void textsprite_set_text_formated(TextSprite textsprite, const char* format, ...);
void textsprite_set_text_formated2(TextSprite textsprite, const char* format, va_list va_args);
void textsprite_set_font_size(TextSprite textsprite, float font_size);
void textsprite_force_case(TextSprite textsprite, int32_t none_or_lowercase_or_uppercase);
void textsprite_set_paragraph_align(TextSprite textsprite, Align align);
void textsprite_set_paragraph_space(TextSprite textsprite, float space);
void textsprite_set_maxlines(TextSprite textsprite, int32_t max_lines);
void textsprite_set_color_rgba8(TextSprite textsprite, uint32_t rbg8_color);
void textsprite_set_color(TextSprite textsprite, float r, float g, float b);

void textsprite_set_alpha(TextSprite textsprite, float alpha);
void textsprite_set_visible(TextSprite textsprite, bool visible);
void textsprite_set_draw_location(TextSprite textsprite, float x, float y);
void textsprite_get_draw_location(TextSprite textsprite, float* x, float* y);
void textsprite_set_z_index(TextSprite textsprite, float z_index);
void textsprite_set_z_offset(TextSprite textsprite, float offset);
float textsprite_get_z_index(TextSprite textsprite);
void textsprite_set_max_draw_size(TextSprite textsprite, float max_width, float max_height);
void textsprite_matrix_flip(TextSprite textsprite, nbool flip_x, nbool flip_y);

void textsprite_set_align(TextSprite textsprite, Align align_vertical, Align align_horizontal);
void textsprite_set_align_vertical(TextSprite textsprite, Align align);
void textsprite_set_align_horizontal(TextSprite textsprite, Align align);

Modifier* textsprite_matrix_get_modifier(TextSprite textsprite);
void textsprite_matrix_reset(TextSprite textsprite);
void textsprite_matrix_translate(TextSprite textsprite, float translate_x, float translate_y);
void textsprite_matrix_skew(TextSprite textsprite, float skew_x, float skew_y);
void textsprite_matrix_scale(TextSprite textsprite, float scale_x, float scale_y);
void textsprite_matrix_rotate(TextSprite textsprite, float degrees_angle);

void textsprite_matrix_rotate_pivot_enable(TextSprite textsprite, bool enable);
void textsprite_matrix_rotate_pivot(TextSprite textsprite, float u, float v);

void textsprite_matrix_scale_offset(TextSprite textsprite, float direction_x, float direction_y);

void textsprite_matrix_corner_rotation(TextSprite textsprite, Corner corner);

void textsprite_calculate_paragraph_alignment(TextSprite textsprite);

void textsprite_set_property(TextSprite textsprite, int32_t property_id, /*union;const */ float value /*; const char* string; } complex_value*/);

void textsprite_get_draw_size(TextSprite textsprite, float* draw_width, float* draw_height);
float textsprite_get_font_size(TextSprite textsprite);
void textsprite_get_max_draw_size(TextSprite textsprite, float* max_draw_width, float* draw_height_max);
void textsprite_draw(TextSprite textsprite, PVRContext pvrctx);

AnimSprite textsprite_animation_set(TextSprite textsprite, AnimSprite animsprite);
void textsprite_animation_restart(TextSprite textsprite);
void textsprite_animation_end(TextSprite textsprite);
void textsprite_animation_stop(TextSprite textsprite);

void textsprite_animation_external_set(TextSprite textsprite, AnimSprite animsprite);
void textsprite_animation_external_end(TextSprite textsprite);
void textsprite_animation_external_restart(TextSprite textsprite);

int32_t textsprite_animate(TextSprite textsprite, float elapsed);

void textsprite_border_enable(TextSprite textsprite, bool enable);
void textsprite_border_set_size(TextSprite textsprite, float border_size);
void textsprite_border_set_color(TextSprite textsprite, float r, float g, float b, float a);
void textsprite_border_set_color_rgba8(TextSprite textsprite, uint32_t rbga8_color);
void textsprite_border_set_offset(TextSprite textsprite, float x, float y);

bool textsprite_is_visible(TextSprite textsprite);
void textsprite_set_antialiasing(TextSprite textsprite, PVRFlag antialiasing);
void textsprite_set_wordbreak(TextSprite textsprite, int32_t wordbreak);
bool textsprite_has_font(TextSprite textsprite);
void textsprite_change_font(TextSprite textsprite, FontHolder fontholder);

// void textsprite_set_shader(TextSprite textsprite, PSShader psshader);
// PSShader textsprite_get_shader(TextSprite textsprite);

void textsprite_blend_enable(TextSprite textsprite, bool enabled);
void textsprite_blend_set(TextSprite textsprite, Blend src_rgb, Blend dst_rgb, Blend src_alpha, Blend dst_alpha);
void textsprite_background_enable(TextSprite textsprite, bool enabled);
void textsprite_background_set_size(TextSprite textsprite, float size);
void textsprite_background_set_offets(TextSprite textsprite, float offset_x, float offset_y);
void textsprite_background_set_color(TextSprite textsprite, float r, float g, float b, float a);
const char* const* textsprite_get_string(TextSprite textsprite);
void textsprite_enable_color_by_addition(TextSprite textsprite, bool enabled);

#endif
