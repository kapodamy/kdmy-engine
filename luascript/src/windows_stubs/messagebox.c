#include "commons.h"
#include "messagebox.h"

static Modifier_t stub_modifier = {};

void messagebox_set_buttons_text(MessageBox messagebox, const char* left_text, const char* right_text) {
    print_stub("messagebox_set_buttons_text", "messagebox=%p left_text=%s right_text=%s", messagebox, left_text, right_text);
}
void messagebox_set_button_single(MessageBox messagebox, const char* center_text) {
    print_stub("messagebox_set_button_single", "messagebox=%p center_text=%s", messagebox, center_text);
}
void messagebox_set_buttons_icons(MessageBox messagebox, const char* left_icon_name, const char* right_icon_name) {
    print_stub("messagebox_set_buttons_icons", "messagebox=%p left_icon_name=%s right_icon_name=%s", messagebox, left_icon_name, right_icon_name);
}
void messagebox_set_button_single_icon(MessageBox messagebox, const char* center_icon_name) {
    print_stub("messagebox_set_button_single_icon", "messagebox=%p center_icon_name=%s", messagebox, center_icon_name);
}
void messagebox_set_title(MessageBox messagebox, const char* text) {
    print_stub("messagebox_set_title", "messagebox=%p text=%s", messagebox, text);
}
void messagebox_set_image_background_color(MessageBox messagebox, uint32_t color_rgb8) {
    print_stub("messagebox_set_image_background_color", "messagebox=%p color_rgb8=%u", messagebox, color_rgb8);
}
void messagebox_set_image_background_color_default(MessageBox messagebox) {
    print_stub("messagebox_set_image_background_color_default", "messagebox=%p", messagebox);
}
void messagebox_set_message(MessageBox messagebox, const char* text) {
    print_stub("messagebox_set_message", "messagebox=%p text=%s", messagebox, text);
}
void messagebox_hide_image_background(MessageBox messagebox, bool hide) {
    print_stub("messagebox_hide_image_background", "messagebox=%p hide=(bool)%i", messagebox, hide);
}
void messagebox_hide_image(MessageBox messagebox, bool hide) {
    print_stub("messagebox_hide_image", "messagebox=%p hide=(bool)%i", messagebox, hide);
}
void messagebox_show_buttons_icons(MessageBox messagebox, bool show) {
    print_stub("messagebox_show_buttons_icons", "messagebox=%p show=(bool)%i", messagebox, show);
}
void messagebox_use_small_size(MessageBox messagebox, bool small_or_normal) {
    print_stub("messagebox_use_small_size", "messagebox=%p small_or_normal=(bool)%i", messagebox, small_or_normal);
}
void messagebox_set_image_sprite(MessageBox messagebox, Sprite sprite) {
    print_stub("messagebox_set_image_sprite", "messagebox=%p sprite=%p", messagebox, sprite);
}
void messagebox_set_image_from_texture(MessageBox messagebox, const char* filename) {
    print_stub("messagebox_set_image_from_texture", "messagebox=%p filename=%s", messagebox, filename);
}
void messagebox_set_image_from_atlas(MessageBox messagebox, const char* filename, const char* entry_name, bool is_animation) {
    print_stub("messagebox_set_image_from_atlas", "messagebox=%p filename=%s entry_name=%s is_animation=(bool)%i", messagebox, filename, entry_name, is_animation);
}
void messagebox_hide(MessageBox messagebox, bool animated) {
    print_stub("messagebox_hide", "messagebox=%p animated=(bool)%i", messagebox, animated);
}
void messagebox_show(MessageBox messagebox, bool animated) {
    print_stub("messagebox_show", "messagebox=%p animated=(bool)%i", messagebox, animated);
}
void messagebox_set_z_index(MessageBox messagebox, float z_index) {
    print_stub("messagebox_set_z_index", "messagebox=%p z_index=%f", messagebox, z_index);
}
Modifier messagebox_get_modifier(MessageBox messagebox) {
    print_stub("messagebox_get_modifier", "messagebox=%p", messagebox);
    return &stub_modifier;
}
