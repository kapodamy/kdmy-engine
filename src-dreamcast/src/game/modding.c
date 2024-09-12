#include "game/modding.h"

#include "externals/luascript.h"
#include "fs.h"
#include "game/common/funkin.h"
#include "game/common/funkinsave.h"
#include "game/common/menu.h"
#include "game/common/messagebox.h"
#include "game/credits.h"
#include "game/freeplaymenu.h"
#include "game/gameplay/week.h"
#include "game/gameplay/weekscript.h"
#include "game/main.h"
#include "game/mainmenu.h"
#include "game/screenmenu.h"
#include "game/startscreen.h"
#include "game/weekselector.h"
#include "layout.h"
#include "logger.h"
#include "malloc_utils.h"
#include "preloadcache.h"
#include "pvrcontext_global.h"
#include "soundplayer.h"
#include "stringutils.h"


const char* MODDING_NATIVE_MENU_SCREEN = "NATIVE_SCREEN";


Modding modding_init(Layout layout, const char* src_script) {

    Modding modding = malloc_chk(sizeof(struct Modding_s));
    malloc_assert(modding, Modding);

    *modding = (struct Modding_s){
        .script = NULL,
        .layout = layout,
        .has_exit = false,
        .has_halt = false,

        .native_menu = NULL,
        .active_menu = NULL,
        .callback_option = NULL,
        .callback_private_data = NULL,
        .exit_delay_ms = 0.0,
        .custom_menu_gamepad_delay = 200.0,
        .custom_menu_active_gamepad_delay = 0.0,
        .last_pressed = 0x00,
        .messagebox = NULL
    };

    if (src_script && fs_file_exists(src_script))
        modding->script = weekscript_init(src_script, modding, false);

    return modding;
}

void modding_destroy(Modding* modding_ptr) {
    if (!modding_ptr || !*modding_ptr) return;

    Modding modding = *modding_ptr;

    if (modding->script) weekscript_destroy(&modding->script);
    if (modding->messagebox) messagebox_destroy(&modding->messagebox);

    free_chk(modding);
    *modding_ptr = NULL;
}

Layout modding_get_layout(Modding modding) {
    return modding->layout;
}

void modding_exit(Modding modding) {
    modding->has_exit = true;
}

void modding_set_halt(Modding modding, bool halt) {
    modding->has_halt = halt;
}


void modding_unlockdirective_create(Modding modding, const char* name, float64 value) {
    (void)modding;
    funkinsave_create_unlock_directive(name, value);
}

bool modding_unlockdirective_has(Modding modding, const char* name) {
    (void)modding;
    return funkinsave_contains_unlock_directive(name);
}

float64 modding_unlockdirective_get(Modding modding, const char* name) {
    (void)modding;

    float64 value;
    funkinsave_read_unlock_directive(name, &value);
    return value;
}

void modding_unlockdirective_remove(Modding modding, const char* name) {
    (void)modding;
    funkinsave_delete_unlock_directive(name);
}


bool modding_storage_set(Modding modding, const char* week_name, const char* name, const uint8_t* data, size_t data_size) {
    (void)modding;
    return funkinsave_storage_set(week_name, name, data, data_size);
}

size_t modding_storage_get(Modding modding, const char* week_name, const char* name, uint8_t** out_data) {
    (void)modding;
    return funkinsave_storage_get(week_name, name, out_data);
}


bool modding_choose_native_menu_option(Modding modding, const char* name) {
    if (!modding->callback_option) return false;
    return modding->callback_option(modding->callback_private_data, name);
}

void modding_set_active_menu(Modding modding, Menu menu) {
    modding->active_menu = menu;
}

Menu modding_get_active_menu(Modding modding) {
    return modding->active_menu;
}

Menu modding_get_native_menu(Modding modding) {
    return modding->native_menu;
}

void* modding_spawn_screen(Modding modding, const char* layout_src, const char* script_src, const ModdingValueType arg_type, const void* arg_value, ModdingValueType* ret_type) {
    (void)modding;

    ScreenMenu screenmenu = screenmenu_init(layout_src, script_src);
    if (!screenmenu) return NULL;

    void* ret = screenmenu_display(screenmenu, &pvr_context, arg_value, arg_type, ret_type);
    screenmenu_destroy(&screenmenu);

    return ret;
}

SoundPlayer modding_get_native_background_music(Modding modding) {
    (void)modding;
    return background_menu_music;
}

SoundPlayer modding_replace_native_background_music(Modding modding, const char* music_src) {
    (void)modding;

    if (background_menu_music) {
        soundplayer_destroy(&background_menu_music);
        background_menu_music = NULL;
    }

    if (string_is_not_empty(music_src)) {
        background_menu_music = soundplayer_init(music_src);
    }

    return background_menu_music;
}

void modding_set_exit_delay(Modding modding, float64 delay_ms) {
    modding->exit_delay_ms = delay_ms;
}

MessageBox modding_get_messagebox(Modding modding) {
    if (!modding->messagebox) modding->messagebox = messagebox_init();
    return modding->messagebox;
}


ModdingHelperResult modding_helper_handle_custom_menu(Modding modding, Gamepad gamepad, float elapsed) {
    WeekScript script = modding->script ? weekscript_get_luascript(modding->script) : NULL;
    GamepadButtons pressed = gamepad_get_pressed(gamepad);

    if (script) {
        if (modding->last_pressed != pressed) {
            modding->last_pressed = pressed;
            weekscript_notify_buttons(modding->script, -1, pressed);
        }
        weekscript_notify_frame(modding->script, elapsed);
    }

    Menu menu = modding->active_menu;
    if (modding->has_halt || !menu || menu == modding->native_menu) return ModdingHelperResult_CONTINUE;

    if (modding->custom_menu_active_gamepad_delay > 0.0) {
        modding->custom_menu_active_gamepad_delay -= elapsed;
        if (modding->custom_menu_active_gamepad_delay > 0.0) return ModdingHelperResult_CONTINUE;
    }

    bool go_back = false;
    bool has_selected = false;
    bool has_choosen = false;

    if (pressed & MAINMENU_GAMEPAD_CANCEL)
        go_back = true;
    else if (pressed & GAMEPAD_DALL_LEFT)
        has_selected = menu_select_horizontal(menu, -1);
    else if (pressed & GAMEPAD_DALL_RIGHT)
        has_selected = menu_select_horizontal(menu, 1);
    else if (pressed & GAMEPAD_DALL_UP)
        has_selected = menu_select_vertical(menu, -1);
    else if (pressed & GAMEPAD_DALL_DOWN)
        has_selected = menu_select_vertical(menu, 1);
    else if (pressed & MAINMENU_GAMEPAD_OK)
        has_choosen = menu_get_selected_index(menu) >= 0;


    if (go_back) {
        if (!script) return ModdingHelperResult_BACK;
        if (!luascript_notify_modding_back(script)) return ModdingHelperResult_BACK;
        modding->custom_menu_active_gamepad_delay = modding->custom_menu_gamepad_delay;
    }

    if (has_selected) {
        if (!script) {
            int32_t index = menu_get_selected_index(menu);
            const char* name = menu_get_selected_item_name(menu);
            luascript_notify_modding_menu_option_selected(script, menu, index, name);
        }
        modding->custom_menu_active_gamepad_delay = modding->custom_menu_gamepad_delay;
    }

    if (has_choosen) {
        if (!script) return ModdingHelperResult_CHOOSEN;

        int32_t index = menu_get_selected_index(menu);
        const char* name = menu_get_selected_item_name(menu);

        if (!luascript_notify_modding_menu_option_choosen(script, menu, index, name)) {
            menu_toggle_choosen(menu, true);
            return ModdingHelperResult_CHOOSEN;
        }

        modding->custom_menu_active_gamepad_delay = modding->custom_menu_gamepad_delay;
    }

    return ModdingHelperResult_CONTINUE;
}


bool modding_helper_notify_option(Modding modding, bool selected_or_choosen) {
    if (!modding->active_menu || !modding->script) return false;

    int32_t index = menu_get_selected_index(modding->active_menu);
    const char* name = menu_get_selected_item_name(modding->active_menu);
    Luascript script = weekscript_get_luascript(modding->script);

    if (selected_or_choosen) {
        luascript_notify_modding_menu_option_selected(script, modding->active_menu, index, name);
        return false;
    }

    return luascript_notify_modding_menu_option_choosen(script, modding->active_menu, index, name);
}

bool modding_helper_notify_option2(Modding modding, bool selected_or_choosen, Menu menu, int32_t index, const char* name) {
    if (!modding->script) return false;

    Luascript script = weekscript_get_luascript(modding->script);

    if (selected_or_choosen) {
        luascript_notify_modding_menu_option_selected(script, menu, index, name);
        return false;
    }

    return luascript_notify_modding_menu_option_choosen(script, menu, index, name);
}

void modding_helper_notify_frame(Modding modding, float elapsed, float64 song_timestamp) {
    if (!modding->script) return;
    if (song_timestamp >= 0.0) weekscript_notify_timersong(modding->script, song_timestamp);
    weekscript_notify_frame(modding->script, elapsed);
}

bool modding_helper_notify_back(Modding modding) {
    if (!modding->script) return false;
    return luascript_notify_modding_back(weekscript_get_luascript(modding->script));
}

void* modding_helper_notify_exit(Modding modding, ModdingValueType* ret_type) {
    if (!modding->script) return NULL;
    return luascript_notify_modding_exit(weekscript_get_luascript(modding->script), ret_type);
}

void modding_helper_notify_exit2(Modding modding) {
    ModdingValueType ret_type;
    void* ret = modding_helper_notify_exit(modding, &ret_type);
    if (ret) free_chk(ret);
}

void modding_helper_notify_init(Modding modding, const void* arg, const ModdingValueType arg_type) {
    if (!modding->script) return;
    luascript_notify_modding_init(weekscript_get_luascript(modding->script), arg, arg_type);
}

void modding_helper_notify_event(Modding modding, const char* event_name) {
    if (!modding->script) return;
    luascript_notify_modding_event(weekscript_get_luascript(modding->script), event_name);
}

void modding_helper_notify_handle_custom_option(Modding modding, const char* option_name) {
    if (!modding->script) return;
    luascript_notify_modding_handle_custom_option(weekscript_get_luascript(modding->script), option_name);
}

void modding_set_menu_in_layout_placeholder(Modding modding, const char* placeholder_name, Menu menu) {
    if (!modding->layout) return;
    LayoutPlaceholder* placeholder = layout_get_placeholder(modding->layout, placeholder_name);
    if (!placeholder) return;
    placeholder->vertex = menu ? menu_get_drawable(menu) : NULL;
}


WeekInfo* modding_get_loaded_weeks(Modding modding, int32_t* out_size) {
    (void)modding;
    *out_size = weeks_array.size;
    return weeks_array.array;
}

int32_t modding_launch_week(Modding modding, const char* week_name, const char* difficult, bool alt_tracks, const char* bf, const char* gf, const char* gameplay_manifest, int32_t song_idx, const char* ws_label) {
    (void)modding;

    WeekInfo* gameplay_weekinfo = NULL;

    for (int32_t i = 0; i < weeks_array.size; i++) {
        if (string_equals(weeks_array.array[i].name, week_name)) {
            gameplay_weekinfo = &weeks_array.array[i];
        }
    }

    if (gameplay_weekinfo == NULL) return -1;

    funkinsave_set_last_played(gameplay_weekinfo->name, difficult);

    bool bf_allocated = string_is_empty(bf);
    bool gf_allocated = string_is_empty(gf);

    if (bf_allocated) bf = freeplaymenu_helper_get_default_character_manifest(true);
    if (gf_allocated) gf = freeplaymenu_helper_get_default_character_manifest(false);

    main_helper_draw_loading_screen();

    /*
    // before run check if necessary preload files
    char* preload_filelist = weekenumerator_get_asset(gameplay_weekinfo, PRELOADCACHE_PRELOAD_FILENAME);
    preloadcache_add_file_list(preload_filelist);
    free_chk(preload_filelist);
    */

    int32_t week_result = week_main(
        gameplay_weekinfo,
        alt_tracks,
        difficult,
        bf,
        gf,
        gameplay_manifest,
        song_idx,
        ws_label
    );

    if (bf_allocated) free_chk((char*)bf);
    if (gf_allocated) free_chk((char*)gf);

    return week_result;
}

void modding_launch_credits(Modding modding) {
    (void)modding;
    credits_main();
}

bool modding_launch_startscreen(Modding modding) {
    (void)modding;
    return startscreen_main();
}

bool modding_launch_mainmenu(Modding modding) {
    (void)modding;
    return mainmenu_main();
}

void modding_launch_settings(Modding modding) {
    (void)modding;
    logger_info("modding_launch_settingsmenu() not implemented: settingsmenu_main()");
}

void modding_launch_freeplay(Modding modding) {
    (void)modding;
    freeplaymenu_main();
}

int32_t modding_launch_weekselector(Modding modding) {
    (void)modding;
    return weekselector_main();
}
