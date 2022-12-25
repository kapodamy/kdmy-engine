#include "commons.h"
#include "textsprite.h"
#include "pvrctx.h"

static Modifier_t stub_modifier = {};
static PSShader_t stub_psshader = {};

void textsprite_set_text_intern(TextSprite textsprite, bool intern, const char* text) {
    print_stub("textsprite_set_text_intern", "textsprite=%p intern=(bool)%i text=%s", textsprite, intern, text);
}
void textsprite_set_font_size(TextSprite textsprite, float font_size) {
    print_stub("textsprite_set_font_size", "textsprite=%p font_size=%f", textsprite, font_size);
}
void textsprite_force_case(TextSprite textsprite, TextSpriteForceCase none_or_lowercase_or_uppercase) {
    print_stub("textsprite_force_case", "textsprite=%p none_or_lowercase_or_uppercase=%p", textsprite, none_or_lowercase_or_uppercase);
}
void textsprite_set_paragraph_align(TextSprite textsprite, Align align) {
    print_stub("textsprite_set_paragraph_align", "textsprite=%p align=%hhu", textsprite, align);
}
void textsprite_set_paragraph_space(TextSprite textsprite, float space) {
    print_stub("textsprite_set_paragraph_space", "textsprite=%p space=%f", textsprite, space);
}
void textsprite_set_maxlines(TextSprite textsprite, int32_t max_lines) {
    print_stub("textsprite_set_maxlines", "textsprite=%p max_lines=%i", textsprite, max_lines);
}
void textsprite_set_color_rgba8(TextSprite textsprite, uint32_t rbg8_color) {
    print_stub("textsprite_set_color_rgba8", "textsprite=%p rbg8_color=%u", textsprite, rbg8_color);
}
void textsprite_set_color(TextSprite textsprite, float r, float g, float b) {
    print_stub("textsprite_set_color", "textsprite=%p r=%f g=%f b=%f", textsprite, r, g, b);
}
void textsprite_set_alpha(TextSprite textsprite, float alpha) {
    print_stub("textsprite_set_alpha", "textsprite=%p alpha=%f", textsprite, alpha);
}
void textsprite_set_visible(TextSprite textsprite, bool visible) {
    print_stub("textsprite_set_visible", "textsprite=%p visible=(bool)%i", textsprite, visible);
}
void textsprite_set_draw_location(TextSprite textsprite, float x, float y) {
    print_stub("textsprite_set_draw_location", "textsprite=%p x=%f y=%f", textsprite, x, y);
}
void textsprite_set_z_index(TextSprite textsprite, float z_index) {
    print_stub("textsprite_set_z_index", "textsprite=%p z_index=%f", textsprite, z_index);
}
void textsprite_set_z_offset(TextSprite textsprite, float offset) {
    print_stub("textsprite_set_z_offset", "textsprite=%p offset=%f", textsprite, offset);
}
void textsprite_set_max_draw_size(TextSprite textsprite, float max_width, float max_height) {
    print_stub("textsprite_set_max_draw_size", "textsprite=%p max_width=%f max_height=%f", textsprite, max_width, max_height);
}
void textsprite_matrix_flip(TextSprite textsprite, bool flip_x, bool flip_y) {
    print_stub("textsprite_matrix_flip", "textsprite=%p flip_x=(bool)%i flip_y=(bool)%i", textsprite, flip_x, flip_y);
}
void textsprite_set_align(TextSprite textsprite, Align align_vertical, Align align_horizontal) {
    print_stub("textsprite_set_align", "textsprite=%p align_vertical=%hhu align_horizontal=%hhu", textsprite, align_vertical, align_horizontal);
}
Modifier textsprite_matrix_get_modifier(TextSprite textsprite) {
    print_stub("textsprite_matrix_get_modifier", "textsprite=%p", textsprite);
    return &stub_modifier;
}
void textsprite_matrix_reset(TextSprite textsprite) {
    print_stub("textsprite_matrix_reset", "textsprite=%p", textsprite);
}
float textsprite_get_font_size(TextSprite textsprite) {
    print_stub("textsprite_get_font_size", "textsprite=%p", textsprite);
    return 123;
}
void textsprite_get_draw_size(TextSprite textsprite, float* draw_width, float* draw_height) {
    *draw_width = 123;
    *draw_height = 456;
    print_stub("textsprite_get_draw_size", "textsprite=%p draw_width=%p draw_height=%p", textsprite, draw_width, draw_height);
}
void textsprite_border_enable(TextSprite textsprite, bool enable) {
    print_stub("textsprite_border_enable", "textsprite=%p enable=(bool)%i", textsprite, enable);
}
void textsprite_border_set_size(TextSprite textsprite, float border_size) {
    print_stub("textsprite_border_set_size", "textsprite=%p border_size=%f", textsprite, border_size);
}
void textsprite_border_set_color(TextSprite textsprite, float r, float g, float b, float a) {
    print_stub("textsprite_border_set_color", "textsprite=%p r=%f g=%f b=%f a=%f", textsprite, r, g, b, a);
}
void textsprite_border_set_offset(TextSprite textsprite, float x, float y) {
    print_stub("textsprite_border_set_offset", "textsprite=%p x=%f y=%f b=%f a=%f", textsprite, x, y);
}
void textsprite_set_antialiasing(TextSprite textsprite, PVRFlag antialiasing) {
    print_stub("textsprite_set_antialiasing", "textsprite=%p antialiasing=%i", textsprite, antialiasing);
}
void textsprite_set_wordbreak(TextSprite textsprite, FontWordBreak wordbreak) {
    print_stub("textsprite_set_wordbreak", "textsprite=%p wordbreak=%i", textsprite, wordbreak);
}
void textsprite_set_shader(TextSprite textsprite, PSShader psshader) {
    print_stub("textsprite_set_shader", "textsprite=%p psshader=%p", textsprite, psshader);
}
PSShader textsprite_get_shader(TextSprite textsprite) {
    print_stub("textsprite_get_shader", "textsprite=%p", textsprite);
    return &stub_psshader;
}
void textsprite_blend_enable(TextSprite textsprite, bool enabled) {
    print_stub("textsprite_blend_enable", "textsprite=%p enabled=(bool)%i", textsprite, enabled);
}
void textsprite_blend_set(TextSprite textsprite, Blend src_rgb, Blend dst_rgb, Blend src_alpha, Blend dst_alpha) {
    print_stub("textsprite_blend_set", "textsprite=%p src_rgb=%i dst_rgb=%i src_alpha=%i dst_alpha=%i", textsprite, src_rgb, dst_rgb, src_alpha, dst_alpha);
}
void textsprite_background_enable(TextSprite textsprite, bool enabled) {
    print_stub("textsprite_background_enable", "textsprite=%p enabled=(bool)%i", textsprite, enabled);
}
void textsprite_background_set_size(TextSprite textsprite, float size) {
    print_stub("textsprite_background_set_size", "textsprite=%p size=%f", textsprite, size);
}
void textsprite_background_set_offets(TextSprite textsprite, float offset_x, float offset_y) {
    print_stub("textsprite_background_set_offets", "textsprite=%p offset_x=%f offset_y=%f", textsprite, offset_x, offset_y);
}
void textsprite_background_set_color(TextSprite textsprite, float r, float g, float b, float a) {
    print_stub("textsprite_background_set_color", "textsprite=%p r=%f g=%f b=%f a=%f", textsprite, r, g, b, a);
}
