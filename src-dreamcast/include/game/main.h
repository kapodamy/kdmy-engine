#ifndef _main_h
#define _main_h

#include <stdbool.h>
#include <stdint.h>


#define SETTINGS_ID_INPUT_OFFSET 1
#define SETTINGS_ID_PENALITY_ON_EMPTY_STRUM 2
#define SETTINGS_ID_INVERSE_STRUM_SCROLL 3
#define SETTINGS_ID_SONG_PROGRESSBAR 4
#define SETTINGS_ID_SONG_PROGRESSBAR_REMAINING 5
#define SETTINGS_ID_USE_FUNKIN_MARKER_DURATION 6
#define SETTINGS_ID_GAMEPLAY_ENABLED_DISTRACTIONS 7
#define SETTINGS_ID_GAMEPLAY_ENABLED_FLASHINGLIGHTS 8
#define SETTINGS_ID_GAMEPLAY_ENABLED_UICOSMETICS 9

#ifndef _arch_dreamcast
#define SETTINGS_ID_PIXELBUFFEROBJECTS 10
#define SETTINGS_ID_SHOW_FPS 11
#define SETTINGS_ID_FPS_LIMIT 12
#define SETTINGS_ID_FULLSCREEN 13
#define SETTINGS_ID_SAVESLOTS 14
#define SETTINGS_ID_AUTOHIDE_CURSOR 15
#define SETTINGS_ID_MUTE_ON_MINIMIZE 16
#define SETTINGS_ID_MASTER_VOLUME 17
#define SETTINGS_ID_SHOW_LOADING_SCREEN 18

#define SETTINGS_ID_FPS_LIMIT_VALUE_VSYNC 1
#define SETTINGS_ID_FPS_LIMIT_VALUE_DETERMINISTIC 2
#define SETTINGS_ID_FPS_LIMIT_VALUE_OFF 3
#endif

//
//  forward reference
//

typedef struct SoundPlayer_s* SoundPlayer;
typedef struct Layout_s* Layout;
typedef struct WeekInfo_s WeekInfo;
typedef struct MenuManifest_s* MenuManifest;
typedef struct kthread kthread_t;


typedef void* (*AsyncThread)(void* param);

typedef struct EngineSettings_s {
    bool input_offset;
    bool inverse_strum_scroll;
    bool penality_on_empty_strum;
    bool use_funkin_marker_duration;
    bool song_progressbar;
    bool song_progressbar_remaining;
    bool gameplay_enabled_distractions;
    bool gameplay_enabled_flashinglights;
    bool gameplay_enabled_uicosmetics;
    // char* style_from_week_name;

    void (*reload_settings)();
    void (*save_settings)();
} EngineSettings;


extern const char* ENGINE_NAME;
extern const char* ENGINE_VERSION;
extern SoundPlayer background_menu_music;
extern EngineSettings SETTINGS;

extern int main_argc;
extern char** main_argv;


kthread_t* main_thd_helper_spawn(bool detached, AsyncThread routine, void* param);

void* main_spawn_coroutine(Layout background_layout, AsyncThread function_routine, void* argument_routine);

void main_helper_draw_loading_screen();

void main_helper_trigger_action_menu(Layout layout, const char* prefix, const char* name, bool selected, bool choosen);
void main_helper_trigger_action_menu2(Layout layout, MenuManifest menu_manifest, int32_t index, const char* prefix, bool selected, bool choosen);

Layout main_helper_init_layout_suffixed(const char* src, bool check_exists);
MenuManifest main_helper_init_menumanifest_suffixed(const char* src, bool check_exists);

#endif
