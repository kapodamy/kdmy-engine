#ifndef __textsprite_h
#define __textsprite_h

#include <stdint.h>
#include <stdbool.h>

#include "vertexprops.h"
#include "modifier.h"
#include "pvrctx.h"
#include "psshader.h"

typedef struct TextSprite_t {
    int dummy;
} TextSprite_t;

typedef TextSprite_t* TextSprite;

void textsprite_set_text_intern(TextSprite textsprite, bool intern, const char* text);
void textsprite_set_font_size(TextSprite textsprite, float font_size);
void textsprite_force_case(TextSprite textsprite, TextSpriteForceCase none_or_lowercase_or_uppercase);
void textsprite_set_paragraph_align(TextSprite textsprite, Align align);
void textsprite_set_paragraph_space(TextSprite textsprite, float space);
void textsprite_set_maxlines(TextSprite textsprite, int32_t max_lines);
void textsprite_set_color_rgba8(TextSprite textsprite, uint32_t rbg8_color);
void textsprite_set_color(TextSprite textsprite, float r, float g, float b);
void textsprite_set_alpha(TextSprite textsprite, float alpha);
void textsprite_set_visible(TextSprite textsprite, bool visible);
void textsprite_set_draw_location(TextSprite textsprite, float x, float y);
void textsprite_set_z_index(TextSprite textsprite, float z_index);
void textsprite_set_z_offset(TextSprite textsprite, float offset);
void textsprite_set_max_draw_size(TextSprite textsprite, float max_width, float max_height);
void textsprite_matrix_flip(TextSprite textsprite, bool flip_x, bool flip_y);
void textsprite_set_align(TextSprite textsprite, Align align_vertical, Align align_horizontal);
Modifier textsprite_matrix_get_modifier(TextSprite textsprite);
void textsprite_matrix_reset(TextSprite textsprite);
float textsprite_get_font_size(TextSprite textsprite);
void textsprite_get_draw_size(TextSprite textsprite, float* draw_width, float* draw_height);
void textsprite_border_enable(TextSprite textsprite, bool enable);
void textsprite_border_set_size(TextSprite textsprite, float border_size);
void textsprite_border_set_color(TextSprite textsprite, float r, float g, float b, float a);
void textsprite_set_antialiasing(TextSprite textsprite, PVRFlag antialiasing);
void textsprite_set_wordbreak(TextSprite textsprite, FontWordBreak wordbreak);
void textsprite_set_shader(TextSprite textsprite, PSShader psshader);
PSShader textsprite_get_shader(TextSprite textsprite);
void textsprite_background_enable(TextSprite textsprite, bool enabled);
void textsprite_background_set_size(TextSprite textsprite, float size);
void textsprite_background_set_offets(TextSprite textsprite, float offset_x, float offset_y);
void textsprite_background_set_color(TextSprite textsprite, float r, float g, float b, float a);
#endif

