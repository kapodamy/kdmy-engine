#ifndef _modding_h
#define _modding_h

#include "layout.h"
#include "menu.h"
#include "messagebox.h"
#include "weekenumerator.h"

typedef struct {
    int stub;
} Modding_t;

typedef Modding_t* Modding;

typedef int32_t ModdingValueType;
#define ModdingValueType_null 0
#define ModdingValueType_string 1
#define ModdingValueType_boolean 2
#define ModdingValueType_double 3


void modding_unlockdirective_create(Modding modding, const char* name, double value);
bool modding_unlockdirective_has(Modding roundcontext, const char* name);
void modding_unlockdirective_remove(Modding modding, const char* name);
double modding_unlockdirective_get(Modding modding, const char* name);

Layout modding_get_layout(Modding modding);
void modding_set_halt(Modding modding, bool halt);
void modding_exit(Modding modding);

Menu modding_get_active_menu(Modding modding);
bool modding_choose_native_menu_option(Modding modding, const char* name);
Menu modding_get_native_menu(Modding modding);
void modding_set_active_menu(Modding modding, Menu menu);
SoundPlayer modding_get_native_background_music(Modding modding);
SoundPlayer modding_replace_native_background_music(Modding modding, const char* music_src);
void* modding_spawn_screen(Modding modding, const char* layout_src, const char* script_src, const ModdingValueType arg_type, const void* arg_value, ModdingValueType* ret_type);
void modding_set_exit_delay(Modding modding, float delay_ms);
MessageBox modding_get_messagebox(Modding modding);
void modding_set_menu_in_layout_placeholder(Modding modding, const char* placeholder_name, Menu menu);

bool modding_storage_set(Modding modding, const char* week_name, const char* name, const uint8_t* data, uint32_t data_size);
uint32_t modding_storage_get(Modding modding, const char* week_name, const char* name, uint8_t** data);

WeekInfo* modding_get_loaded_weeks(Modding modding, int32_t* out_size);
int32_t modding_launch_week(Modding modding, const char* week_name, char* difficult, bool alt_tracks, char* bf, char* gf, char* gameplay_manifest, int32_t song_idx);
void modding_launch_credits(Modding modding);
bool modding_launch_startscreen(Modding modding);
bool modding_launch_mainmenu(Modding modding);
void modding_launch_settings(Modding modding);
void modding_launch_freeplay(Modding modding);
int32_t modding_launch_weekselector(Modding modding);


#endif

