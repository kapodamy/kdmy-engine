#include "commons.h"
#include "drawable.h"

static PSShader_t psshader = {};
static Modifier_t modifier = {};

void drawable_set_z_index(Drawable drawable, float z_index) {
    print_stub("drawable_set_z_index", "drawable=%p z_index=%f", drawable, z_index);
}
float drawable_get_z_index(Drawable drawable) {
    print_stub("drawable_get_z_index", "drawable=%p", drawable);
    return 0;
}
void drawable_set_z_offset(Drawable drawable, float offset) {
    print_stub("drawable_set_z_offset", "drawable=%p offset=%f", drawable, offset);
}
void drawable_set_alpha(Drawable drawable, float alpha) {
    print_stub("drawable_set_alpha", "drawable=%p alpha=%f", drawable, alpha);
}
float drawable_get_alpha(Drawable drawable) {
    print_stub("drawable_get_alpha", "drawable=%p", drawable);
    return 0;
}
void drawable_set_offsetcolor(Drawable drawable, float r, float g, float b, float a) {
    print_stub("drawable_set_offsetcolor", "drawable=%p r=%f g=%f b=%f a=%f", drawable, r, g, b, a);
}
void drawable_set_offsetcolor_to_default(Drawable drawable) {
    print_stub("drawable_set_offsetcolor_to_default", "drawable=%p", drawable);
}
Modifier drawable_get_modifier(Drawable drawable) {
    print_stub("drawable_get_modifier", "drawable=%p", drawable);
    return &modifier;
}
void drawable_set_antialiasing(Drawable drawable, PVRFlag antialiasing) {
    print_stub("drawable_set_antialiasing", "drawable=%p antialiasing=%i", drawable, antialiasing);
}
void drawable_set_shader(Drawable drawable, PSShader psshader) {
    print_stub("drawable_set_shader", "drawable=%p psshader=%p", drawable, psshader);
}
PSShader drawable_get_shader(Drawable drawable) {
    print_stub("drawable_get_shader", "drawable=%p", drawable);
    return &psshader;
}
void drawable_blend_enable(Drawable drawable, bool enabled) {
    print_stub("drawable_blend_enable", "drawable=%p enabled=(bool)%i", drawable, enabled);
}
void drawable_blend_set(Drawable drawable, Blend src_rgb, Blend dst_rgb, Blend src_alpha, Blend dst_alpha) {
    print_stub("drawable_blend_set", "drawable=%p src_rgb=%i dst_rgb=%i src_alpha=%i dst_alpha=%i", drawable, src_rgb, dst_rgb, src_alpha, dst_alpha);
}

