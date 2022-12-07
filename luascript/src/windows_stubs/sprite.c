#include "commons.h"
#include "sprite.h"
#include "modifier.h"
#include "pvrctx.h"

static Modifier_t stub_modifier = {};
static PSShader_t stub_psshader = {};

Modifier sprite_matrix_get_modifier(Sprite sprite) {
    print_stub("sprite_matrix_get_modifier", "sprite=%p", sprite);
    return &stub_modifier;
}
void sprite_set_offset_source(Sprite sprite, float x, float y, float width, float height) {
    print_stub("sprite_set_offset_source", "sprite=%p x=%f y=%f width=%f height=%f", sprite, x, y, width, height);
}
void sprite_set_offset_frame(Sprite sprite, float x, float y, float width, float height) {
    print_stub("sprite_set_offset_frame", "sprite=%p x=%f y=%f width=%f height=%f", sprite, x, y, width, height);
}
void sprite_set_offset_pivot(Sprite sprite, float x, float y) {
    print_stub("sprite_set_offset_pivot", "sprite=%p x=%f y=%f", sprite, x, y);
}
void sprite_matrix_reset(Sprite sprite) {
    print_stub("sprite_matrix_reset", "sprite=%p", sprite);
}
void sprite_set_draw_location(Sprite sprite, float x, float y) {
    print_stub("sprite_set_draw_location", "sprite=%p x=%f y=%f", sprite, x, y);
}
void sprite_set_draw_size(Sprite sprite, float width, float height) {
    print_stub("sprite_set_draw_size", "sprite=%p width=%f height=%f", sprite, width, height);
}
void sprite_set_draw_size_from_source_size(Sprite sprite) {
    print_stub("sprite_set_draw_size_from_source_size", "sprite=%p", sprite);
}
void sprite_set_alpha(Sprite sprite, float alpha) {
    print_stub("sprite_set_alpha", "sprite=%p alpha=%f", sprite, alpha);
}
void sprite_set_visible(Sprite sprite, bool visible) {
    print_stub("sprite_set_visible", "sprite=%p visible=(bool)%i", sprite, visible);
}
void sprite_set_z_index(Sprite sprite, float index) {
    print_stub("sprite_set_z_index", "sprite=%p index=%f", sprite, index);
}
void sprite_set_z_offset(Sprite sprite, float offset) {
    print_stub("sprite_set_z_offset", "sprite=%p offset=%f", sprite, offset);
}
void sprite_get_source_size(Sprite sprite, float* source_width, float* source_height) {
    *source_width = 123;
    *source_height = 456;
    print_stub("sprite_get_source_size", "sprite=%p source_width=%p source_height=%p", sprite, source_width, source_height);
}
void sprite_set_vertex_color(Sprite sprite, float r, float g, float b) {
    print_stub("sprite_set_vertex_color", "sprite=%p r=%f g=%f b=%f", sprite, r, g, b);
}
void sprite_set_offsetcolor(Sprite sprite, float r, float g, float b, float a) {
    print_stub("sprite_set_offsetcolor", "sprite=%p r=%f g=%f b=%f a=%f", sprite, r, g, b, a);
}
bool sprite_is_textured(Sprite sprite) {
    print_stub("sprite_is_textured", "sprite=%p", sprite);
    return 0;
}
bool sprite_crop(Sprite sprite, float dx, float dy, float dwidth, float dheight) {
    print_stub("sprite_crop", "sprite=%p dx=%f dy=%f dwidth=%f dheight=%f", sprite, dx, dy, dwidth, dheight);
    return 0;
}
bool sprite_is_crop_enabled(Sprite sprite) {
    print_stub("sprite_is_crop_enabled", "sprite=%p", sprite);
    return 0;
}
void sprite_crop_enable(Sprite sprite, bool enable) {
    print_stub("sprite_crop_enable", "sprite=%p enable=(bool)%i", sprite, enable);
}
void sprite_resize_draw_size(Sprite sprite, float max_width, float max_height, float* applied_draw_width, float* applied_draw_height) {
    *applied_draw_width = 789;
    *applied_draw_height = 456;
    print_stub("sprite_resize_draw_size", "sprite=%p max_width=%f max_height=%f applied_draw_width=%p applied_draw_height=%p", sprite, max_width, max_height, applied_draw_width, applied_draw_height);
}
void sprite_center_draw_location(Sprite sprite, float x, float y, float ref_width, float ref_height, float* applied_draw_x, float* applied_draw_y) {
    *applied_draw_x = 321;
    *applied_draw_y = 876;
    print_stub("sprite_center_draw_location", "sprite=%p x=%f y=%f ref_width=%f ref_height=%f applied_draw_x=%p applied_draw_y=%p", sprite, x, y, ref_width, ref_height, applied_draw_x, applied_draw_y);
}
void sprite_set_antialiasing(Sprite sprite, PVRFlag antialiasing) {
    print_stub("sprite_set_antialiasing", "sprite=%p antialiasing=%i", sprite, antialiasing);
}
void sprite_flip_rendered_texture(Sprite sprite, int flip_x, int flip_y) {
    print_stub("sprite_flip_rendered_texture", "sprite=%p flip_x=%i flip_y=%i", sprite, flip_x, flip_y);
}
void sprite_flip_rendered_texture_enable_correction(Sprite sprite, bool enabled) {
    print_stub("sprite_flip_rendered_texture_enable_correction", "sprite=%p enabled=(bool)%i", sprite, enabled);
}
void sprite_set_shader(Sprite sprite, PSShader psshader) {
    print_stub("sprite_set_shader", "sprite=%p psshader=%p", sprite, psshader);
}
PSShader sprite_get_shader(Sprite sprite) {
    print_stub("sprite_get_shader", "sprite=%p", sprite);
    return &stub_psshader;
}

