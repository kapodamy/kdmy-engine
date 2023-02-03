#include "commons.h"
#include "menu.h"

static Menu_t menu = {};

Menu menu_init(MenuManifest menumanifest, float x, float y, float z, float width, float height) {
    print_stub("menu_init", "menumanifest=%p x=%f y=%f z=%f width=%f height=%f", menumanifest, x, y, z, width, height);
    return &menu;
}
Menu menu_destroy(Menu* menu) {
    print_stub("menu_destroy", "menu=%p", menu);
    return NULL;
}
Drawable menu_get_drawable(Menu menu) {
    print_stub("menu_get_drawable", "menu=%p", menu);
    return NULL;
}
void menu_trasition_in(Menu menu) {
    print_stub("menu_trasition_in", "menu=%p", menu);
}
void menu_trasition_out(Menu menu) {
    print_stub("menu_trasition_out", "menu=%p", menu);
}
void menu_select_item(Menu menu, const char* name) {
    print_stub("menu_select_item", "menu=%p name=%s", menu, name);
}
void menu_select_index(Menu menu, int32_t index) {
    print_stub("menu_select_index", "menu=%p index=%i", menu, index);
}
bool menu_select_vertical(Menu menu, int32_t offset) {
    print_stub("menu_select_vertical", "menu=%p offset=%i", menu, offset);
    return 0;
}
bool menu_select_horizontal(Menu menu, int32_t offset) {
    print_stub("menu_select_horizontal", "menu=%p offset=%i", menu, offset);
    return 0;
}
void menu_toggle_choosen(Menu menu, bool enable) {
    print_stub("menu_toggle_choosen", "menu=%p enable=(bool)%i", menu, enable);
}
int32_t menu_get_selected_index(Menu menu) {
    print_stub("menu_get_selected_index", "menu=%p", menu);
    return 0;
}
int32_t menu_get_items_count(Menu menu) {
    print_stub("menu_get_items_count", "menu=%p", menu);
    return 0;
}
bool menu_set_item_text(Menu menu, int32_t index, const char* text) {
    print_stub("menu_set_item_text", "menu=%p index=%i text=%s", menu, index, text);
    return 0;
}
bool menu_set_item_visibility(Menu menu, int32_t index, bool visible) {
    print_stub("menu_set_item_visibility", "menu=%p index=%i visible=(bool)%i", menu, index, visible);
    return 0;
}
bool menu_get_item_rect(Menu menu, int32_t index, float* x, float* y, float* width, float* height) {
    print_stub("menu_get_item_rect", "menu=%p index=%i x=%p y=%p width=%p height=%p", menu, index, x, y, width, height);
    return 0;
}
const char* menu_get_selected_item_name(Menu menu) {
    print_stub("menu_get_selected_item_name", "menu=%p", menu);
    return NULL;
}
void menu_set_text_force_case(Menu menu, TextSpriteForceCase none_or_lowercase_or_uppercase) {
    print_stub("menu_set_text_force_case", "menu=%p none_or_lowercase_or_uppercase=%i", menu, none_or_lowercase_or_uppercase);
}
