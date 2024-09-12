#ifndef _messagebox_h
#define _messagebox_h

#include <stdbool.h>

#include "pvrcontext_types.h"
#include "drawable_types.h"


//
//  forward references
//

typedef struct Modifier_s Modifier;
typedef struct Sprite_s* Sprite;


typedef struct MessageBox_s* MessageBox;


MessageBox messagebox_init();
void messagebox_destroy(MessageBox* messagebox_ptr);

void messagebox_set_buttons_text(MessageBox messagebox, const char* left_text, const char* right_text);
void messagebox_set_button_single(MessageBox messagebox, const char* center_text);
void messagebox_set_buttons_icons(MessageBox messagebox, const char* left_icon_name, const char* right_icon_name);
void messagebox_set_button_single_icon(MessageBox messagebox, const char* center_icon_name);
void messagebox_set_title(MessageBox messagebox, const char* text);
void messagebox_set_image_background_color(MessageBox messagebox, uint8_t color_rgb8);
void messagebox_set_image_background_color_default(MessageBox messagebox);
void messagebox_set_message(MessageBox messagebox, const char* text);
void messagebox_set_message_formated(MessageBox messagebox, const char* format, ...);
void messagebox_hide_image_background(MessageBox messagebox, bool hide);
void messagebox_hide_image(MessageBox messagebox, bool hide);
void messagebox_hide_buttons(MessageBox messagebox);
void messagebox_show_buttons_icons(MessageBox messagebox, bool show);
void messagebox_use_small_size(MessageBox messagebox, bool small_or_normal);
void messagebox_use_full_title(MessageBox messagebox, bool enable);
void messagebox_set_image_sprite(MessageBox messagebox, Sprite sprite);
void messagebox_set_image_from_texture(MessageBox messagebox, const char* filename);
void messagebox_set_image_from_atlas(MessageBox messagebox, const char* filename, const char* entry_name, bool is_animation);
void messagebox_hide(MessageBox messagebox, bool animated);
void messagebox_show(MessageBox messagebox, bool animated);
void messagebox_set_z_index(MessageBox messagebox, float z_index);

Drawable messagebox_get_drawable(MessageBox messagebox);
Modifier* messagebox_get_modifier(MessageBox messagebox);
int32_t messagebox_animate(MessageBox messagebox, float elapsed);
void messagebox_draw(MessageBox messagebox, PVRContext pvrctx);

#endif
