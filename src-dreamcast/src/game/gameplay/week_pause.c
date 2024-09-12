#include "game/gameplay/week_pause.h"

#include "drawable.h"
#include "fs.h"
#include "game/common/menu.h"
#include "game/common/messagebox.h"
#include "game/gameplay/helpers/gameplaymanifest.h"
#include "game/gameplay/helpers/menumanifest.h"
#include "game/gameplay/weekscript.h"
#include "game/main.h"
#include "game/modding.h"
#include "layout.h"
#include "logger.h"
#include "malloc_utils.h"
#include "math2d.h"
#include "pvrcontext.h"
#include "pvrcontext_global.h"
#include "soundplayer.h"
#include "sprite.h"
#include "stringutils.h"
#include "textsprite.h"


struct WeekPause_s {
    Menu menu;
    MessageBox messagebox;
    Sprite sprite_nocontroller;
    Layout layout;
    LayoutPlaceholder* menu_placeholder;
    Menu menu_external;
    SoundPlayer background_music;
    Modding modding;
    const char* modding_choosen_option_name;
};


static struct MenuManifest_s WEEKPAUSE_MENU = (struct MenuManifest_s){
    .parameters = (MenuManifestParameters){
        .suffix_selected = NULL,  // unused
        .suffix_idle = NULL,      // unused
        .suffix_choosen = NULL,   // unused
        .suffix_discarded = NULL, // unused
        .suffix_rollback = NULL,  // unused
        .suffix_in = NULL,        // unused
        .suffix_out = NULL,       // unused

        .atlas = NULL,
        .animlist = "/assets/common/anims/week-round.xml",

        .anim_selected = "pause_item_selected",
        .anim_idle = "pause_item_idle",
        .anim_choosen = NULL,   // unused
        .anim_discarded = NULL, // unused
        .anim_rollback = NULL,  // unused
        .anim_in = NULL,        // unused
        .anim_out = NULL,       // unused

        .anim_transition_in_delay = 0.0f,  // unused
        .anim_transition_out_delay = 0.0f, // unused

        .font = NULL,
        .font_glyph_suffix = "bold",
        .font_color_by_addition = false, // unused
        .font_size = 48.0f,
        .font_color = 0xFFFFFF,
        .font_border_color = 0x00,     // unused
        .font_border_size = FLOAT_NaN, // unused

        .is_sparse = false, // unused
        .is_vertical = true,
        .is_per_page = false,
        .static_index = 1,

        .items_align = ALIGN_START,
        .items_gap = 48.0f,
        .items_dimmen = 0.0f,                     // unused
        .texture_scale = FLOAT_NaN,               // unused
        .enable_horizontal_text_correction = true // unused
    },
    .items = (MenuManifestItem[]){(MenuManifestItem){
                                      .name = "resume",
                                      .text = "RESUME",                                                 // unused
                                      .placement = {.x = 0.0f, .y = 0.0f, .dimmen = 0.0f, .gap = 0.0f}, // unused
                                      .anim_selected = NULL,                                            // unused
                                      .anim_choosen = NULL,                                             // unused
                                      .anim_discarded = NULL,                                           // unused
                                      .anim_idle = NULL,                                                // unused
                                      .anim_rollback = NULL,                                            // unused
                                      .anim_in = NULL,                                                  // unused
                                      .anim_out = NULL,                                                 // unused
                                      .hidden = false,
                                      .description = NULL, // usused
                                  },
                                  (MenuManifestItem){
                                      .name = "week-menu",
                                      .text = "WEEK MENU",                                              // unused
                                      .placement = {.x = 0.0f, .y = 0.0f, .dimmen = 0.0f, .gap = 0.0f}, // unused
                                      .anim_selected = NULL,                                            // unused
                                      .anim_choosen = NULL,                                             // unused
                                      .anim_discarded = NULL,                                           // unused
                                      .anim_idle = NULL,                                                // unused
                                      .anim_rollback = NULL,                                            // unused
                                      .anim_in = NULL,                                                  // unused
                                      .anim_out = NULL,                                                 // unused
                                      .hidden = 1,
                                      .description = NULL, // usused
                                  },
                                  {
                                      .name = "restart-song",
                                      .text = "RESTART SONG",                                           // unused
                                      .placement = {.x = 0.0f, .y = 0.0f, .dimmen = 0.0f, .gap = 0.0f}, // unused
                                      .anim_selected = NULL,                                            // unused
                                      .anim_choosen = NULL,                                             // unused
                                      .anim_discarded = NULL,                                           // unused
                                      .anim_idle = NULL,                                                // unused
                                      .anim_rollback = NULL,                                            // unused
                                      .anim_in = NULL,                                                  // unused
                                      .anim_out = NULL,                                                 // unused
                                      .hidden = false,
                                      .description = NULL, // usused
                                  },
                                  {
                                      .name = "exit-week-selector",
                                      .text = "EXIT TO WEEK SELECTOR",                                  // unused
                                      .placement = {.x = 0.0f, .y = 0.0f, .dimmen = 0.0f, .gap = 0.0f}, // unused
                                      .anim_selected = NULL,                                            // unused
                                      .anim_choosen = NULL,                                             // unused
                                      .anim_discarded = NULL,                                           // unused
                                      .anim_idle = NULL,                                                // unused
                                      .anim_rollback = NULL,                                            // unused
                                      .anim_in = NULL,                                                  // unused
                                      .anim_out = NULL,                                                 // unused
                                      .hidden = false,
                                      .description = NULL, // usused
                                  },
                                  {
                                      .name = "exit-main-menu",
                                      .text = "EXIT TO MAIN MENU",                                      // unused
                                      .placement = {.x = 0.0f, .y = 0.0f, .dimmen = 0.0f, .gap = 0.0f}, // unused
                                      .anim_selected = NULL,                                            // unused
                                      .anim_choosen = NULL,                                             // unused
                                      .anim_discarded = NULL,                                           // unused
                                      .anim_idle = NULL,                                                // unused
                                      .anim_rollback = NULL,                                            // unused
                                      .anim_in = NULL,                                                  // unused
                                      .anim_out = NULL,                                                 // unused
                                      .hidden = false,
                                      .description = NULL, // usused
                                  }},
    .items_size = 5
};
static const char* WEEKPAUSE_LAYOUT_WIDESCREEN = "/assets/common/image/week-round/pause.xml";
static const char* WEEKPAUSE_LAYOUT_DREAMCAST = "/assets/common/image/week-round/pause~dreamcast.xml";
static const char* WEEKPAUSE_NOCONTROLLER = "/assets/common/image/week-round/no_controller.png";
static const char* WEEKPAUSE_MODDING_SCRIPT = "/assets/common/data/scripts/weekpause.lua";
static const char* WEEKPAUSE_MODDING_MENU = "/assets/common/data/menus/weekpause.json";
static const GamepadButtons WEEKPAUSE_BUTTONS = GAMEPAD_START | GAMEPAD_AD_UP | GAMEPAD_AD_DOWN | GAMEPAD_A | GAMEPAD_X | GAMEPAD_B | GAMEPAD_BACK;
static const int32_t WEEKPAUSE_DELAY = 170;
static const int32_t WEEKPAUSE_ANTIBOUNCE = (int32_t)(WEEKPAUSE_DELAY * 1.25f);

// messagebox strings
static const char* WEEKPAUSE_MSGCONTROLLER = "The controller $i was disconnected, \n"
                                             "reconnect it or press START on an unused \n"
                                             "controller to replace it.";
static const char* WEEKPAUSE_MSGMENU = "The week progress will be lost, ¿return\n to the main menu?";
static const char* WEEKPAUSE_MSGWEEKSELECTOR = "The week progress will be lost, ¿return?";


static float week_pause_internal_render(WeekPause weekpause, RoundContext roundcontext);
static bool week_pause_internal_handle_modding_option(WeekPause weekpause, const char* option_name);
static int32_t week_pause_internal_return_value(WeekPause weekpause);


WeekPause week_pause_init(const char* exit_to_weekselector_label) {
    const char* src = pvr_context_is_widescreen() ? WEEKPAUSE_LAYOUT_WIDESCREEN : WEEKPAUSE_LAYOUT_DREAMCAST;
    Layout layout = layout_init(src);
    if (!layout) {
        logger_error("can not load: %s", src);
        assert(layout);
    }

    LayoutPlaceholder* menu_placeholder = layout_get_placeholder(layout, "menu");
    if (!menu_placeholder) {
        logger_error("missing 'menu' placeholder in: %s", src);
        assert(menu_placeholder);
    }

    WEEKPAUSE_MENU.parameters.font = (char*)layout_get_attached_value_string(
        layout, "menu_font_path", NULL
    );
    WEEKPAUSE_MENU.parameters.font_size = layout_get_attached_value_as_float(
        layout, "menu_font_size", 24.0f
    );
    WEEKPAUSE_MENU.parameters.items_gap = WEEKPAUSE_MENU.parameters.font_size;

    // load custom menumanifest if exists
    MenuManifest menumanifest = main_helper_init_menumanifest_suffixed(WEEKPAUSE_MODDING_MENU, true);
    if (!menumanifest) menumanifest = &WEEKPAUSE_MENU;

    Menu menu = menu_init(
        menumanifest,
        menu_placeholder->x, menu_placeholder->y, menu_placeholder->z,
        menu_placeholder->width, menu_placeholder->height
    );

    if (menumanifest != &WEEKPAUSE_MENU) menumanifest_destroy(&menumanifest);

    MessageBox messagebox = messagebox_init();
    Sprite sprite_nocontroller = sprite_init(texture_init(WEEKPAUSE_NOCONTROLLER));

    Modding modding = modding_init(layout, WEEKPAUSE_MODDING_SCRIPT);
    modding->native_menu = modding->active_menu = menu;
    modding->callback_option = (ModdingCallbackOption)week_pause_internal_handle_modding_option;

    if (string_is_not_empty(exit_to_weekselector_label)) {
        int32_t index = menu_index_of_item(menu, "exit-week-selector");
        if (index >= 0) menu_set_item_text(menu, index, exit_to_weekselector_label);
    }

    WeekPause weekpause = malloc_chk(sizeof(struct WeekPause_s));
    malloc_assert(weekpause, WeekPause);

    *weekpause = (struct WeekPause_s){
        .menu = menu,
        .messagebox = messagebox,
        .sprite_nocontroller = sprite_nocontroller,
        .layout = layout,
        .menu_placeholder = menu_placeholder,
        .menu_external = NULL,
        .background_music = NULL,
        .modding = modding,
        .modding_choosen_option_name = NULL
    };

    modding->callback_private_data = weekpause;
    week_pause_change_background_music(weekpause, "/assets/common/music/breakfast.ogg");

    return weekpause;
}

void week_pause_destroy(WeekPause* weekpause_ptr) {
    if (!weekpause_ptr || !*weekpause_ptr) return;

    WeekPause weekpause = *weekpause_ptr;

    modding_destroy(&weekpause->modding);
    layout_destroy(&weekpause->layout);
    menu_destroy(&weekpause->menu);
    messagebox_destroy(&weekpause->messagebox);
    sprite_destroy_full(&weekpause->sprite_nocontroller);
    if (weekpause->menu_external) menu_destroy(&weekpause->menu_external);
    if (weekpause->background_music) soundplayer_destroy(&weekpause->background_music);
    weekpause->modding_choosen_option_name = NULL; // do not dispose

    free_chk(weekpause);
    *weekpause_ptr = NULL;
}


void week_pause_external_set_text(WeekPause weekpause, int32_t index, const char* text) {
    if (weekpause->menu_external) menu_set_item_text(weekpause->menu_external, index, text);
}

void week_pause_external_set_visible(WeekPause weekpause, int32_t index, bool visible) {
    if (weekpause->menu_external) menu_set_item_visibility(weekpause->menu_external, index, visible);
}

void week_pause_external_set_menu(WeekPause weekpause, const char* menumanifest_src) {
    if (weekpause->menu_external) {
        menu_destroy(&weekpause->menu_external);
        weekpause->menu_external = NULL;
    }

    if (!menumanifest_src || !fs_file_exists(menumanifest_src)) {
        menu_set_item_visibility(weekpause->menu, 1, false);
        return;
    }

    menu_set_item_visibility(weekpause->menu, 1, true);

    LayoutPlaceholder* menu_placeholder = weekpause->menu_placeholder;
    MenuManifest menumanifest = menumanifest_init(menumanifest_src);
    weekpause->menu_external = menu_init(
        menumanifest,
        menu_placeholder->x, menu_placeholder->y, menu_placeholder->z,
        menu_placeholder->width, menu_placeholder->height
    );
    menumanifest_destroy(&menumanifest);
}

void week_pause_change_background_music(WeekPause weekpause, const char* filename) {
    if (weekpause->background_music) soundplayer_destroy(&weekpause->background_music);

    if (string_is_not_empty(filename)) {
        weekpause->background_music = soundplayer_init(filename);
        if (weekpause->background_music) {
            soundplayer_loop_enable(weekpause->background_music, true);
            soundplayer_set_volume(weekpause->background_music, 0.5f);
        }
    } else {
        weekpause->background_music = NULL;
    }
}

int32_t week_pause_helper_show(WeekPause weekpause, RoundContext roundcontext, int32_t dettached_index) {
    bool dettached_controller = dettached_index >= 0;
    Gamepad controller = gamepad_init(-1);
    gamepad_set_buttons_delay(controller, WEEKPAUSE_DELAY);

    messagebox_hide(weekpause->messagebox, false);
    if (weekpause->background_music) soundplayer_play(weekpause->background_music);
    gamepad_clear_buttons(controller);

    bool current_menu_is_external = false;
    Menu current_menu = weekpause->menu;
    int32_t return_value = 0;

    menu_select_index(weekpause->menu, 0);
    if (weekpause->menu_external) menu_select_index(weekpause->menu_external, 0);
    weekpause->menu_placeholder->vertex = menu_get_drawable(weekpause->menu);

    TextSprite textsprite = layout_get_textsprite(weekpause->layout, "stats");
    if (textsprite) {
        int32_t song_index = roundcontext->song_index;
        GameplayManifestSong* songmanifest = &roundcontext->initparams.gameplaymanifest->songs[song_index];
        const char* song_name = songmanifest->name;
        const char* song_difficult = roundcontext->song_difficult;
        textsprite_set_text_formated(textsprite, "$s\n$s", song_name, song_difficult);
    }

    if (roundcontext->script) weekscript_notify_pause(roundcontext->script, true);
    while (roundcontext->scriptcontext.halt_flag) week_pause_internal_render(weekpause, roundcontext);

    layout_trigger_any(weekpause->layout, NULL);

    if (dettached_controller) {
        dettached_index++; // do not use base-zero index
        messagebox_set_button_single(weekpause->messagebox, "(Waiting controller)");
        messagebox_set_title(weekpause->messagebox, "Controller disconnected");
        messagebox_set_message_formated(weekpause->messagebox, WEEKPAUSE_MSGCONTROLLER, dettached_index);
        messagebox_show_buttons_icons(weekpause->messagebox, false);
        messagebox_use_small_size(weekpause->messagebox, false);
        messagebox_hide_image(weekpause->messagebox, false);
        messagebox_set_image_sprite(weekpause->messagebox, weekpause->sprite_nocontroller);
        messagebox_show(weekpause->messagebox, true);
    }

    weekpause->modding_choosen_option_name = NULL;
    weekpause->modding->has_exit = false;
    weekpause->modding->has_halt = false;
    weekpause->modding->native_menu = weekpause->modding->active_menu;
    modding_helper_notify_init(weekpause->modding, MODDING_NATIVE_MENU_SCREEN, ModdingValueType_string);
    modding_helper_notify_option(weekpause->modding, true);

    while (!weekpause->modding->has_exit) {
        bool has_option_choosen = false;
        bool go_back = false;
        float elapsed = week_pause_internal_render(weekpause, roundcontext);
        GamepadButtons buttons = gamepad_has_pressed_delayed(controller, WEEKPAUSE_BUTTONS);

        // if the messagebox is visible, wait decision
        if (return_value != 0) {
            if (buttons & (GAMEPAD_A | GAMEPAD_X)) {
                break;
            } else if (buttons & (GAMEPAD_B | GAMEPAD_START | GAMEPAD_BACK)) {
                return_value = 0;
                if (buttons & (GAMEPAD_B | GAMEPAD_BACK)) {
                    gamepad_set_buttons_delay(controller, WEEKPAUSE_DELAY);
                    gamepad_enforce_buttons_delay(controller);
                    messagebox_hide(weekpause->messagebox, false);
                } else {
                    break;
                }
            }
            continue;
        }

        ModdingHelperResult res = modding_helper_handle_custom_menu(weekpause->modding, controller, elapsed);
        if (res != ModdingHelperResult_CONTINUE) break;
        if (weekpause->modding->has_halt || weekpause->modding->active_menu != weekpause->menu) continue;
        if (weekpause->modding_choosen_option_name != NULL) {
            buttons = 0x00;
            has_option_choosen = true;
            break;
        }

        if (dettached_controller) {
            int32_t dettached_count = 0;
            int32_t available = 0;
            for (int32_t i = 0; i < roundcontext->players_size; i++) {
                if (!roundcontext->players[i].controller) continue;
                if (gamepad_is_dettached(roundcontext->players[i].controller)) {
                    if (gamepad_pick(roundcontext->players[i].controller, true)) {
                        available++;
                    } else {
                        dettached_count++;
                    }
                } else {
                    available++;
                }
            }
            if (available < 1) continue;
            if (dettached_count < 1) {
                dettached_controller = false;
                messagebox_hide(weekpause->messagebox, false);
            }
        }

        if (buttons & GAMEPAD_START) {
            if (!current_menu_is_external && !modding_helper_notify_back(weekpause->modding)) break;
        } else if (buttons & GAMEPAD_AD_UP) {
            if (!menu_select_vertical(current_menu, -1))
                menu_select_index(current_menu, menu_get_items_count(current_menu) - 1);
            modding_helper_notify_option(weekpause->modding, true);
        } else if (buttons & GAMEPAD_AD_DOWN) {
            if (!menu_select_vertical(current_menu, 1))
                menu_select_index(current_menu, 0);
            modding_helper_notify_option(weekpause->modding, true);
        } else if (buttons & (GAMEPAD_A | GAMEPAD_X)) {
            has_option_choosen = true;
        } else if (buttons & (GAMEPAD_B | GAMEPAD_BACK)) {
            go_back = true;
        } else if (!has_option_choosen) {
            // nothing to do
            continue;
        }


        if (has_option_choosen && current_menu_is_external) {
            int32_t option_index = menu_get_selected_index(current_menu);
            weekscript_notify_pause_optionchoosen(roundcontext->script, option_index);
            has_option_choosen = false;
            go_back = true;
        } else if (has_option_choosen) {
            if (weekpause->modding_choosen_option_name == NULL) {
                weekpause->modding_choosen_option_name = menu_get_selected_item_name(current_menu);
                if (modding_helper_notify_option(weekpause->modding, false)) {
                    weekpause->modding_choosen_option_name = NULL;
                    continue;
                }
            }

            return_value = week_pause_internal_return_value(weekpause);

            if (return_value == 0) {
                // resume
                break;
            } else if (return_value == 4) {
                return_value = 0;
                if (!weekpause->menu_external || !roundcontext->script) continue;

                // display week menu
                current_menu = weekpause->menu_external;
                weekpause->modding->callback_option = NULL;
                modding_helper_notify_event(weekpause->modding, "week-custom-menu");
                menu_trasition_in(current_menu);
                weekpause->menu_placeholder->vertex = menu_get_drawable(current_menu);
                current_menu_is_external = true;
                if (roundcontext->script) weekscript_notify_pause_menuvisible(roundcontext->script, true);
            } else if (return_value == 1) {
                // restart song
                break;
            } else if (return_value == 2 || return_value == 3) {
                const char* msg = return_value == 2 ? WEEKPAUSE_MSGWEEKSELECTOR : WEEKPAUSE_MSGMENU;
                messagebox_hide_image(weekpause->messagebox, true);
                messagebox_set_buttons_icons(weekpause->messagebox, "a", "b");
                messagebox_set_buttons_text(weekpause->messagebox, "Ok", "Cancel");
                messagebox_set_title(weekpause->messagebox, "Confirm");
                messagebox_use_small_size(weekpause->messagebox, true);
                messagebox_set_message(weekpause->messagebox, msg);
                messagebox_show(weekpause->messagebox, false);
                gamepad_set_buttons_delay(controller, WEEKPAUSE_ANTIBOUNCE);
                gamepad_enforce_buttons_delay(controller);
            } else if (return_value == -1) {
                // custom option menu
                weekpause->modding->callback_option = NULL;
                modding_helper_notify_handle_custom_option(weekpause->modding, weekpause->modding_choosen_option_name);
                weekpause->modding->callback_option = (ModdingCallbackOption)week_pause_internal_handle_modding_option;
                weekpause->modding_choosen_option_name = NULL;
                return_value = 0;
            }
        }

        if (go_back && current_menu_is_external) {
            current_menu_is_external = false;
            current_menu = weekpause->menu;
            weekpause->menu_placeholder->vertex = menu_get_drawable(weekpause->menu);
            weekscript_notify_pause_menuvisible(roundcontext->script, false);
            weekpause->modding->callback_option = (ModdingCallbackOption)week_pause_internal_handle_modding_option;
            modding_helper_notify_event(weekpause->modding, "week-pause-menu");
            menu_trasition_in(weekpause->menu);
        } else if (go_back && !modding_helper_notify_back(weekpause->modding)) {
            return_value = 0;
            break;
        }
    }

    if (weekpause->background_music) soundplayer_fade(weekpause->background_music, false, 100.0f);

    if (roundcontext->script) weekscript_notify_pause(roundcontext->script, false);
    while (roundcontext->scriptcontext.halt_flag) week_pause_internal_render(weekpause, roundcontext);

    if (weekpause->background_music) soundplayer_stop(weekpause->background_music);
    messagebox_hide(weekpause->messagebox, true);

    if (return_value != 0) {
        const char* target = return_value == 1 ? "transition_fast" : "transition";
        layout_trigger_any(weekpause->layout, target);

        while (true) {
            week_pause_internal_render(weekpause, roundcontext);
            if (layout_animation_is_completed(weekpause->layout, "transition_effect")) break;
        }
    }

    modding_helper_notify_exit2(weekpause->modding);
    gamepad_destroy(&controller);

    // selected options:
    //      0 -> resume
    //      1 -> restart song
    //      2 -> back to weekselector
    //      3 -> back to mainmenu
    return return_value;
}

Layout week_pause_get_layout(WeekPause weekpause) {
    return weekpause->layout;
}


static float week_pause_internal_render(WeekPause weekpause, RoundContext roundcontext) {
    GamepadButtons buttons = 0x00;

    float elapsed;

    do {
        elapsed = pvrctx_wait_ready();
        pvr_context_reset(&pvr_context);

        for (int32_t i = 0; i < roundcontext->players_size; i++) {
            Gamepad controller = roundcontext->players[i].controller;
            if (controller && gamepad_get_managed_presses(controller, true, &buttons) && roundcontext->script) {
                weekscript_notify_buttons(roundcontext->script, i, buttons);
            }
        }

        if (roundcontext->script) weekscript_notify_frame(roundcontext->script, elapsed);

        // draw the stage+ui layout but do not animate
        layout_draw(roundcontext->layout, &pvr_context);

        layout_animate(weekpause->layout, elapsed);
        layout_draw(weekpause->layout, &pvr_context);
    } while (roundcontext->scriptcontext.halt_flag);

    messagebox_animate(weekpause->messagebox, elapsed);
    messagebox_draw(weekpause->messagebox, &pvr_context);

    return elapsed;
}

static bool week_pause_internal_handle_modding_option(WeekPause weekpause, const char* option_name) {
    if (option_name == NULL) {
        // resume
        weekpause->modding_choosen_option_name = WEEKPAUSE_MENU.items[0].name;
        menu_select_item(weekpause->menu, weekpause->modding_choosen_option_name);
        return true;
    }

    // select native option
    int32_t index = menumanifest_get_option_index(&WEEKPAUSE_MENU, option_name);
    if (index >= 0) {
        weekpause->modding_choosen_option_name = WEEKPAUSE_MENU.items[index].name;
        menu_select_item(weekpause->menu, weekpause->modding_choosen_option_name);
        return true;
    }

    // select custom option
    if (menu_has_item(weekpause->menu, option_name)) {
        menu_select_item(weekpause->menu, option_name);
        weekpause->modding_choosen_option_name = menu_get_selected_item_name(weekpause->menu);
        return true;
    }

    // unknown option
    weekpause->modding_choosen_option_name = NULL;
    return false;
}

static int32_t week_pause_internal_return_value(WeekPause weekpause) {
    const char* option_name = weekpause->modding_choosen_option_name;

    if (string_equals(option_name, NULL) || string_equals(option_name, "resume")) {
        return 0;
    } else if (string_equals(option_name, "week-menu")) {
        return 4;
    } else if (string_equals(option_name, "restart-song")) {
        return 1;
    } else if (string_equals(option_name, "exit-week-selector")) {
        return 2;
    } else if (string_equals(option_name, "exit-main-menu")) {
        return 3;
    } else {
        // custom option
        return -1;
    }
}
