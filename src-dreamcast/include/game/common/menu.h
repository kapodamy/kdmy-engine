#ifndef _menu_h
#define _menu_h

#include <stdbool.h>

#include "game/gameplay/helpers/menumanifest.h"
#include "pvrcontext_types.h"


//
//  forward reference
//

typedef struct Drawable_s* Drawable;
typedef struct ModelHolder_s* ModelHolder;


typedef struct Menu_s* Menu;
typedef bool (*MenuDrawCallback)(void* privatedata, PVRContext pvrctx, Menu menu, int32_t index, float item_x, float item_y, float item_width, float item_height);


Menu menu_init(MenuManifest menumanifest, float x, float y, float z, float width, float height);
void menu_destroy(Menu* menu);

Drawable menu_get_drawable(Menu menu);
void menu_trasition_in(Menu menu);
void menu_trasition_out(Menu menu);

bool menu_select_item(Menu menu, const char* name);
void menu_select_index(Menu menu, int32_t index);
bool menu_select_vertical(Menu menu, int32_t offset);
bool menu_select_horizontal(Menu menu, int32_t offset);
void menu_toggle_choosen(Menu menu, bool enable);
int32_t menu_get_selected_index(Menu menu);
int32_t menu_get_items_count(Menu menu);
bool menu_set_item_text(Menu menu, int32_t index, const char* text);
bool menu_set_item_visibility(Menu menu, int32_t index, bool visible);
bool menu_has_valid_selection(Menu menu);

int32_t menu_animate(Menu menu, float elapsed);
void menu_draw(Menu menu, PVRContext pvrctx);

bool menu_get_item_rect(Menu menu, int32_t index, float* x, float* y, float* width, float* height);
bool menu_get_selected_item_rect(Menu menu, float* x, float* y, float* width, float* height);
const char* menu_get_selected_item_name(Menu menu);
void menu_set_text_force_case(Menu menu, int32_t none_or_lowercase_or_uppercase);
void menu_set_draw_callback(Menu menu, bool before_or_after, MenuDrawCallback callback, void* privatedata);
bool menu_has_item(Menu menu, const char* name);
int32_t menu_index_of_item(Menu menu, const char* name);
void menu_set_item_image(Menu menu, int32_t index, ModelHolder modelholder, const char* atlas_or_animlist_entry_name);

#endif
