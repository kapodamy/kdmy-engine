#ifndef __messagebox_h
#define __messagebox_h

#include <stdint.h>
#include <stdbool.h>

#include "modifier.h"
#include "sprite.h"

typedef struct _Messagebox_t {
    int dummy;
} Messagebox_t;

typedef Messagebox_t* Messagebox;

void messagebox_set_buttons_text(Messagebox messagebox, const char* left_text, const char* right_text);
void messagebox_set_button_single(Messagebox messagebox, const char* center_text);
void messagebox_set_buttons_icons(Messagebox messagebox, const char* left_icon_name, const char* right_icon_name);
void messagebox_set_button_single_icon(Messagebox messagebox, const char* center_icon_name);
void messagebox_set_title(Messagebox messagebox, const char* text);
void messagebox_set_image_background_color(Messagebox messagebox, uint32_t color_rgb8);
void messagebox_set_image_background_color_default(Messagebox messagebox);
void messagebox_set_message(Messagebox messagebox, const char* text);
void messagebox_hide_image_background(Messagebox messagebox, bool hide);
void messagebox_hide_image(Messagebox messagebox, bool hide);
void messagebox_show_buttons_icons(Messagebox messagebox, bool show);
void messagebox_use_small_size(Messagebox messagebox, bool small_or_normal);
void messagebox_set_image_sprite(Messagebox messagebox, Sprite sprite);
void messagebox_set_image_from_texture(Messagebox messagebox, const char* filename);
void messagebox_set_image_from_atlas(Messagebox messagebox, const char* filename, const char* entry_name, bool is_animation);
void messagebox_hide(Messagebox messagebox, bool animated);
void messagebox_show(Messagebox messagebox, bool animated);
void messagebox_set_z_index(Messagebox messagebox, float z_index);
Modifier messagebox_get_modifier(Messagebox messagebox);

#endif
