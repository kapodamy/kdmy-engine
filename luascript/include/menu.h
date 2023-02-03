#ifndef __menu_h_
#define __menu_h_

#include <stdbool.h>
#include <stdint.h>

#include "drawable.h"
#include "menumanifest.h"

typedef struct _Menu_t {
    int dummy;
} Menu_t;

typedef Menu_t* Menu;

Menu menu_init(MenuManifest menumanifest, float x, float y, float z, float width, float height);
Menu menu_destroy(Menu* menu);
Drawable menu_get_drawable(Menu menu);
void menu_trasition_in(Menu menu);
void menu_trasition_out(Menu menu);
void menu_select_item(Menu menu, const char* name);
void menu_select_index(Menu menu, int32_t index);
bool menu_select_vertical(Menu menu, int32_t offset);
bool menu_select_horizontal(Menu menu, int32_t offset);
void menu_toggle_choosen(Menu menu, bool enable);
int32_t menu_get_selected_index(Menu menu);
int32_t menu_get_items_count(Menu menu);
bool menu_set_item_text(Menu menu, int32_t index, const char* text);
bool menu_set_item_visibility(Menu menu, int32_t index, bool visible);
bool menu_get_item_rect(Menu menu, int32_t index, float* x, float* y, float* width, float* height);
const char* menu_get_selected_item_name(Menu menu);
void menu_set_text_force_case(Menu menu, TextSpriteForceCase none_or_lowercase_or_uppercase);

#endif