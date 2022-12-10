#ifndef ___messagebox_h
#define ___messagebox_h

#include <stdint.h>
#include <stdbool.h>

#include "modifier.h"
#include "sprite.h"

typedef struct _Messagebox_t {
    int dummy;
} Messagebox_t;

typedef Messagebox_t* MessageBox;

void messagebox_set_buttons_text(MessageBox messagebox, const char* left_text, const char* right_text);
void messagebox_set_button_single(MessageBox messagebox, const char* center_text);
void messagebox_set_buttons_icons(MessageBox messagebox, const char* left_icon_name, const char* right_icon_name);
void messagebox_set_button_single_icon(MessageBox messagebox, const char* center_icon_name);
void messagebox_set_title(MessageBox messagebox, const char* text);
void messagebox_set_image_background_color(MessageBox messagebox, uint32_t color_rgb8);
void messagebox_set_image_background_color_default(MessageBox messagebox);
void messagebox_set_message(MessageBox messagebox, const char* text);
void messagebox_hide_image_background(MessageBox messagebox, bool hide);
void messagebox_hide_image(MessageBox messagebox, bool hide);
void messagebox_show_buttons_icons(MessageBox messagebox, bool show);
void messagebox_use_small_size(MessageBox messagebox, bool small_or_normal);
void messagebox_set_image_sprite(MessageBox messagebox, Sprite sprite);
void messagebox_set_image_from_texture(MessageBox messagebox, const char* filename);
void messagebox_set_image_from_atlas(MessageBox messagebox, const char* filename, const char* entry_name, bool is_animation);
void messagebox_hide(MessageBox messagebox, bool animated);
void messagebox_show(MessageBox messagebox, bool animated);
void messagebox_set_z_index(MessageBox messagebox, float z_index);
Modifier messagebox_get_modifier(MessageBox messagebox);

#endif
