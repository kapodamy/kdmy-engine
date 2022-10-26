#include "commons.h"
#include "messagebox.h"

static Modifier_t stub_modifier = {};

void messagebox_set_buttons_text(Messagebox messagebox, const char* left_text, const char* right_text) {
    print_stub("messagebox_set_buttons_text", "messagebox=%p left_text=%s right_text=%s", messagebox, left_text, right_text);
}
void messagebox_set_button_single(Messagebox messagebox, const char* center_text) {
    print_stub("messagebox_set_button_single", "messagebox=%p center_text=%s", messagebox, center_text);
}
void messagebox_set_buttons_icons(Messagebox messagebox, const char* left_icon_name, const char* right_icon_name) {
    print_stub("messagebox_set_buttons_icons", "messagebox=%p left_icon_name=%s right_icon_name=%s", messagebox, left_icon_name, right_icon_name);
}
void messagebox_set_button_single_icon(Messagebox messagebox, const char* center_icon_name) {
    print_stub("messagebox_set_button_single_icon", "messagebox=%p center_icon_name=%s", messagebox, center_icon_name);
}
void messagebox_set_title(Messagebox messagebox, const char* text) {
    print_stub("messagebox_set_title", "messagebox=%p text=%s", messagebox, text);
}
void messagebox_set_image_background_color(Messagebox messagebox, uint32_t color_rgb8) {
    print_stub("messagebox_set_image_background_color", "messagebox=%p color_rgb8=%u", messagebox, color_rgb8);
}
void messagebox_set_image_background_color_default(Messagebox messagebox) {
    print_stub("messagebox_set_image_background_color_default", "messagebox=%p", messagebox);
}
void messagebox_set_message(Messagebox messagebox, const char* text) {
    print_stub("messagebox_set_message", "messagebox=%p text=%s", messagebox, text);
}
void messagebox_hide_image_background(Messagebox messagebox, bool hide) {
    print_stub("messagebox_hide_image_background", "messagebox=%p hide=(bool)%i", messagebox, hide);
}
void messagebox_hide_image(Messagebox messagebox, bool hide) {
    print_stub("messagebox_hide_image", "messagebox=%p hide=(bool)%i", messagebox, hide);
}
void messagebox_show_buttons_icons(Messagebox messagebox, bool show) {
    print_stub("messagebox_show_buttons_icons", "messagebox=%p show=(bool)%i", messagebox, show);
}
void messagebox_use_small_size(Messagebox messagebox, bool small_or_normal) {
    print_stub("messagebox_use_small_size", "messagebox=%p small_or_normal=(bool)%i", messagebox, small_or_normal);
}
void messagebox_set_image_sprite(Messagebox messagebox, Sprite sprite) {
    print_stub("messagebox_set_image_sprite", "messagebox=%p sprite=%p", messagebox, sprite);
}
void messagebox_set_image_from_texture(Messagebox messagebox, const char* filename) {
    print_stub("messagebox_set_image_from_texture", "messagebox=%p filename=%s", messagebox, filename);
}
void messagebox_set_image_from_atlas(Messagebox messagebox, const char* filename, const char* entry_name, bool is_animation) {
    print_stub("messagebox_set_image_from_atlas", "messagebox=%p filename=%s entry_name=%s is_animation=(bool)%i", messagebox, filename, entry_name, is_animation);
}
void messagebox_hide(Messagebox messagebox, bool animated) {
    print_stub("messagebox_hide", "messagebox=%p animated=(bool)%i", messagebox, animated);
}
void messagebox_show(Messagebox messagebox, bool animated) {
    print_stub("messagebox_show", "messagebox=%p animated=(bool)%i", messagebox, animated);
}
void messagebox_set_z_index(Messagebox messagebox, float z_index) {
    print_stub("messagebox_set_z_index", "messagebox=%p z_index=%f", messagebox, z_index);
}
Modifier messagebox_get_modifier(Messagebox messagebox) {
    print_stub("messagebox_get_modifier", "messagebox=%p", messagebox);
    return &stub_modifier;
}
