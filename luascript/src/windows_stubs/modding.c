#include "commons.h"
#include "modding.h"

static Layout_t stub_layout;
static Menu_t stub_menu;
static Menu_t stub_menu2;
static SoundPlayer_t stub_soundplayer;
static Messagebox_t stub_messagebox;

void modding_unlockdirective_create(Modding modding, const char* name, double value) {
    print_stub("modding_unlockdirective_create", "modding=%p name=%s value=%f", modding, name, value);
}
bool modding_unlockdirective_has(Modding modding, const char* name) {
    print_stub("modding_unlockdirective_has", "modding=%p name=%s", modding, name);
    return 1;
}
void modding_unlockdirective_remove(Modding modding, const char* name) {
    print_stub("modding_unlockdirective_remove", "modding=%p name=%s", modding, name);
}
double modding_unlockdirective_get(Modding modding, const char* name) {
    print_stub("modding_unlockdirective_get", "modding=%p name=%s", modding, name);
    return 123;
}
void modding_set_ui_visibility(Modding modding, bool visible) {
    print_stub("modding_set_ui_visibility", "modding=%p visible=(bool)%i", modding, visible);
}
Layout modding_get_layout(Modding modding) {
    print_stub("modding_get_layout", "modding=%p", modding);
    return &stub_layout;
}
void modding_set_halt(Modding modding, bool halt) {
    print_stub("modding_set_halt", "modding=%p halt=(bool)%i", halt);
}
void modding_exit(Modding modding) {
    print_stub("modding_exit", "modding=%p", modding);
}
Menu modding_get_active_menu(Modding modding) {
    print_stub("modding_get_active_menu", "modding=%p", modding);
    return &stub_menu;
}
bool modding_choose_native_menu_option(Modding modding, const char* name) {
    print_stub("modding_choose_native_menu_option", "modding=%p name=%s", modding, name);
    return 0;
}
Menu modding_get_native_menu(Modding modding) {
    print_stub("modding_get_native_menu", "modding=%p", modding);
    return &stub_menu2;
}
void modding_set_active_menu(Modding modding, Menu menu) {
    print_stub("modding_set_active_menu", "modding=%p menu=%p", modding, menu);
}
SoundPlayer modding_get_native_background_music(Modding modding) {
    print_stub("modding_get_native_background_music", "modding=%p", modding);
    return &stub_soundplayer;
}
SoundPlayer modding_replace_native_background_music(Modding modding, const char* music_src) {
    print_stub("modding_replace_native_background_music", "modding=%p music_src=%s", modding, music_src);
    return NULL;
}
BasicValue* modding_spawn_screen(Modding modding, const char* layout_src, const char* script_src, BasicValue* value) {
    print_stub("modding_spawn_screen", "modding=%p layout_src=%s script_src=%s value=%p", modding, layout_src, script_src, value);
    return value;
}
void modding_set_exit_delay(Modding modding, float delay_ms) {
    print_stub("modding_set_exit_delay", "modding=%p delay_ms=%f", modding, delay_ms);
}
MessageBox modding_get_messagebox(Modding modding) {
    print_stub("modding_get_messagebox", "modding=%p", modding);
    return &stub_messagebox;
}
void modding_set_menu_in_layout_placeholder(Modding modding, const char* placeholder_name, Menu menu) {
    print_stub("modding_set_menu_in_layout_placeholder", "modding=%p placeholder_name=%s menu=%p", modding, placeholder_name, menu);
}
