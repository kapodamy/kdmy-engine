#ifndef _modding_h
#define _modding_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "modding_types.h"


//
//  forward references
//

typedef struct Layout_s* Layout;
typedef struct Menu_s* Menu;
typedef struct Gamepad_s* Gamepad;
typedef struct Luascript_s* WeekScript;
typedef struct MessageBox_s* MessageBox;
typedef struct ScreenMenu_s* ScreenMenu;
typedef struct SoundPlayer_s* SoundPlayer;
typedef struct WeekInfo_s WeekInfo;

typedef struct Modding_s {
    WeekScript script;
    Layout layout;
    bool has_exit;
    bool has_halt;
    Menu native_menu;
    Menu active_menu;
    ModdingCallbackOption callback_option;
    void* callback_private_data;
    float64 exit_delay_ms;
    float64 custom_menu_gamepad_delay;
    float64 custom_menu_active_gamepad_delay;
    GamepadButtons last_pressed;
    MessageBox messagebox;
}* Modding;


extern const char* MODDING_NATIVE_MENU_SCREEN;


Modding modding_init(Layout layout, const char* src_script);
void modding_destroy(Modding* modding);
Layout modding_get_layout(Modding modding);
void modding_exit(Modding modding);
void modding_set_halt(Modding modding, bool halt);

void modding_unlockdirective_create(Modding modding, const char* name, float64 value);
bool modding_unlockdirective_has(Modding modding, const char* name);
float64 modding_unlockdirective_get(Modding modding, const char* name);
void modding_unlockdirective_remove(Modding modding, const char* name);

bool modding_storage_set(Modding modding, const char* week_name, const char* name, const uint8_t* data, size_t data_size);
size_t modding_storage_get(Modding modding, const char* week_name, const char* name, uint8_t** out_data);

bool modding_choose_native_menu_option(Modding modding, const char* name);
void modding_set_active_menu(Modding modding, Menu menu);
Menu modding_get_active_menu(Modding modding);
Menu modding_get_native_menu(Modding modding);
void* modding_spawn_screen(Modding modding, const char* layout_src, const char* script_src, const ModdingValueType arg_type, const void* arg_value, ModdingValueType* ret_type);
SoundPlayer modding_get_native_background_music(Modding modding);
SoundPlayer modding_replace_native_background_music(Modding modding, const char* music_src);
void modding_set_exit_delay(Modding modding, float64 delay_ms);
MessageBox modding_get_messagebox(Modding modding);

ModdingHelperResult modding_helper_handle_custom_menu(Modding modding, Gamepad gamepad, float elapsed);

bool modding_helper_notify_option(Modding modding, bool selected_or_choosen);
bool modding_helper_notify_option2(Modding modding, bool selected_or_choosen, Menu menu, int32_t index, const char* name);
void modding_helper_notify_frame(Modding modding, float elapsed, float64 song_timestamp);
bool modding_helper_notify_back(Modding modding);
void* modding_helper_notify_exit(Modding modding, ModdingValueType* ret_type);
void modding_helper_notify_exit2(Modding modding);
void modding_helper_notify_init(Modding modding, const void* arg, const ModdingValueType arg_type);
void modding_helper_notify_event(Modding modding, const char* event_name);
void modding_helper_notify_handle_custom_option(Modding modding, const char* option_name);
void modding_set_menu_in_layout_placeholder(Modding modding, const char* placeholder_name, Menu menu);

WeekInfo* modding_get_loaded_weeks(Modding modding, int32_t* out_size);
int32_t modding_launch_week(Modding modding, const char* week_name, const char* difficult, bool alt_tracks, const char* bf, const char* gf, const char* gameplay_manifest, int32_t song_idx, const char* ws_label);
void modding_launch_credits(Modding modding);
bool modding_launch_startscreen(Modding modding);
bool modding_launch_mainmenu(Modding modding);
void modding_launch_settings(Modding modding);
void modding_launch_freeplay(Modding modding);
int32_t modding_launch_weekselector(Modding modding);

#endif
