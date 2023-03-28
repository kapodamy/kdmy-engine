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
Layout modding_get_layout(Modding modding) {
    print_stub("modding_get_layout", "modding=%p", modding);
    return &stub_layout;
}
void modding_set_halt(Modding modding, bool halt) {
    print_stub("modding_set_halt", "modding=%p halt=(bool)%i", modding, halt);
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
void* modding_spawn_screen(Modding modding, const char* layout_src, const char* script_src, const ModdingValueType arg_type, const void* arg_value, ModdingValueType* ret_type) {
    print_stub("modding_spawn_screen", "modding=%p layout_src=%s script_src=%s arg_type=%i arg_value=%p ret_type=%p", modding, layout_src, script_src, arg_type, arg_value, ret_type);
    *ret_type = ModdingValueType_null;
    return NULL;
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
bool modding_storage_set(Modding modding, const char* week_name, const char* name, const uint8_t* data, uint32_t data_size) {
    print_stub("modding_storage_set", "modding=%p week_name=%s name=%s data=%p data_size=%u", modding, week_name, name, data, data_size);
    return 0;
}
uint32_t modding_storage_get(Modding modding, const char* week_name, const char* name, uint8_t** data) {
    print_stub("modding_storage_get", "modding=%p week_name=%s name=%s data=%p", modding, week_name, name, data);
    data = NULL;
    return 0;
}
WeekInfo* modding_get_loaded_weeks(Modding modding, int32_t* out_size) {
    print_stub("modding_get_loaded_weeks", "modding=%p out_size=%p", modding, out_size);
    return NULL;
}
int32_t modding_launch_week(Modding modding, const char* week_name, char* difficult, bool alt_tracks, char* bf, char* gf, char* gameplay_manifest, int32_t song_idx, const char* ws_label) {
    print_stub("modding_launch_week", "modding=%p week_name=%s difficult=%s alt_tracks=(bool)%i bf=%s gf=%s gameplay_manifest=%s song_idx=%i ws_label=%s", modding, week_name, difficult, alt_tracks, bf, gf, gameplay_manifest, song_idx, ws_label);
    free(difficult);
    free(bf);
    free(gf);
    free(gameplay_manifest);
    return 0;
}
void modding_launch_credits(Modding modding) {
    print_stub("modding_launch_credits", "modding=%p", modding);
}
bool modding_launch_startscreen(Modding modding) {
    print_stub("modding_launch_startscreen", "modding=%p", modding);
    return 0;
}
bool modding_launch_mainmenu(Modding modding) {
    print_stub("modding_launch_mainmenu", "modding=%p", modding);
    return 0;
}
void modding_launch_settings(Modding modding) {
    print_stub("modding_launch_settings", "modding=%p", modding);
}
void modding_launch_freeplay(Modding modding) {
    print_stub("modding_launch_freeplay", "modding=%p", modding);
}
int32_t modding_launch_weekselector(Modding modding) {
    print_stub("modding_launch_weekselector", "modding=%p", modding);
    return 0;
}
