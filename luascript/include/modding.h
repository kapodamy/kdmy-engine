#ifndef _modding_h

#include "layout.h"
#include "menu.h"
#include "messagebox.h"

typedef struct {
    int stub;
} Modding_t;

typedef Modding_t* Modding;

typedef int32_t BasicValueType;
#define BasicValueTypeNULL 0
#define BasicValueTypeString 1
#define BasicValueTypeBoolean 2
#define BasicValueTypeDouble 3

typedef struct {
    BasicValueType type;
    union {
        char* as_string;
        bool as_boolean;
        double as_double;
    } value;
} BasicValue;


void modding_unlockdirective_create(Modding modding, const char* name, double value);
bool modding_unlockdirective_has(Modding roundcontext, const char* name);
void modding_unlockdirective_remove(Modding modding, const char* name);
double modding_unlockdirective_get(Modding modding, const char* name);
void modding_set_ui_visibility(Modding modding, bool visible);

Layout modding_get_layout(Modding modding);
void modding_set_halt(Modding modding, bool halt);
void modding_exit(Modding modding);

Menu modding_get_active_menu(Modding modding);
bool modding_choose_native_menu_option(Modding modding, const char* name);
Menu modding_get_native_menu(Modding modding);
void modding_set_active_menu(Modding modding, Menu menu);
SoundPlayer modding_get_native_background_music(Modding modding);
SoundPlayer modding_replace_native_background_music(Modding modding, const char* music_src);
BasicValue* modding_spawn_screen(Modding modding, const char* layout_src, const char* script_src, BasicValue* value);
void modding_set_exit_delay(Modding modding, float delay_ms);
MessageBox modding_get_messagebox(Modding modding);
void modding_set_menu_in_layout_placeholder(Modding modding, const char* placeholder_name, Menu menu);

#endif

