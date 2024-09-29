#include "game/mainmenu.h"

#include "drawable.h"
#include "fs.h"
#include "game/common/funkinsave.h"
#include "game/common/menu.h"
#include "game/credits.h"
#include "game/freeplaymenu.h"
#include "game/gameplay/helpers/menumanifest.h"
#include "game/main.h"
#include "game/modding.h"
#include "game/savemanager.h"
#include "game/settingsmenu.h"
#include "game/weekselector.h"
#include "layout.h"
#include "logger.h"
#include "malloc_utils.h"
#include "math2d.h"
#include "pvrcontext.h"
#include "pvrcontext_global.h"
#include "soundplayer.h"
#include "stringutils.h"
#include "textsprite.h"


typedef struct {
    MenuManifest menumanifest;
    const char* choosen_name;
    bool choosen_name_is_allocated;
} ModdingHelper;


static const char* MAINMENU_LAYOUT = "/assets/common/image/main-menu/layout.xml";
static const char* MAINMENU_LAYOUT_DREAMCAST = "/assets/common/image/main-menu/layout~dreamcast.xml";
static const int32_t MAINMENU_OPTION_SELECTION_DELAY = 200; // milliseconds
static const char* MAINMENU_MODDING_SCRIPT = "/assets/common/data/scripts/mainmenu.lua";
static const char* MAINMENU_MODDING_MENU = "/assets/common/data/menus/mainmenu.json";
static const char* MAINMENU_BACK_TO_STARTSCREEN = "back-to-startscreen";


static struct MenuManifest_s MAINMENU_MENU_MANIFEST = {
    .parameters = {
        .suffix_selected = "selected",
        .suffix_choosen = "choosen",
        .suffix_discarded = "discarded",
        .suffix_idle = NULL,     // unused
        .suffix_rollback = NULL, // unused
        .suffix_in = NULL,       // unused
        .suffix_out = NULL,      // unused

        .atlas = "/assets/common/image/main-menu/FNF_main_menu_assets.xml",
        .animlist = "/assets/common/image/main-menu/animations.xml",

        .anim_selected = NULL,     // unused
        .anim_choosen = "choosen", // unused
        .anim_discarded = NULL,    // unused
        .anim_idle = NULL,         // unused
        .anim_rollback = NULL,     // unused
        .anim_in = NULL,           // unused
        .anim_out = NULL,          // unused

        .anim_transition_in_delay = 0.0f,
        .anim_transition_out_delay = -100.0f, // negative means start delays from the bottom

        .font = NULL,                    // unused
        .font_glyph_suffix = NULL,       // unused
        .font_color_by_addition = false, // unused
        .font_size = 0.0f,               // unused
        .font_color = 0x00,              // unused
        .font_border_color = 0x00,       // unused
        .font_border_size = FLOAT_NaN,   // unused

        .is_sparse = false,   // unused
        .is_vertical = false, // readed from the layout
        .is_per_page = true,

        .items_align = ALIGN_CENTER, // readed from the layout
        .items_gap = 0.0f,           // readed from the layout
        .items_dimmen = 0.0f,        // readed from the layout
        .static_index = 0,           // unused
        .texture_scale = FLOAT_NaN,  // readed from the layout
        .enable_horizontal_text_correction = true
    },
    .items = (MenuManifestItem[]){
        {
            .name = "storymode",
            .text = NULL,                                                     // unused
            .placement = {.x = 0.0f, .y = 0.0f, .dimmen = 0.0f, .gap = 0.0f}, // unused
            .anim_selected = NULL,                                            // unused
            .anim_choosen = NULL,                                             // unused
            .anim_discarded = NULL,
            .anim_idle = NULL,
            .anim_rollback = NULL, // unused
            .anim_in = "draw_even_index",
            .anim_out = "no_choosen_even_index",
            .hidden = false,
            .description = NULL // unused
        },
        {
            .name = "freeplay",
            .text = NULL,                                                     // unused
            .placement = {.x = 0.0f, .y = 0.0f, .dimmen = 0.0f, .gap = 0.0f}, // unused
            .anim_selected = NULL,                                            // unused
            .anim_choosen = NULL,                                             // unused
            .anim_discarded = NULL,
            .anim_idle = NULL,
            .anim_rollback = NULL, // unused
            .anim_in = "draw_odd_index",
            .anim_out = "no_choosen_odd_index",
            .hidden = false,
            .description = NULL // unused
        },
        {
            .name = "donate",
            .text = NULL,                                                     // unused
            .placement = {.x = 0.0f, .y = 0.0f, .dimmen = 0.0f, .gap = 0.0f}, // unused
            .anim_selected = NULL,                                            // unused
            .anim_choosen = NULL,                                             // unused
            .anim_discarded = NULL,
            .anim_idle = NULL,
            .anim_rollback = NULL, // unused
            .anim_in = "draw_even_index",
            .anim_out = "no_choosen_even_index",
            .hidden = false,
            .description = NULL // unused
        },
        {
            .name = "options",
            .text = NULL,                                                     // unused
            .placement = {.x = 0.0f, .y = 0.0f, .dimmen = 0.0f, .gap = 0.0f}, // unused
            .anim_selected = NULL,                                            // unused
            .anim_choosen = NULL,                                             // unused
            .anim_discarded = NULL,
            .anim_idle = NULL,
            .anim_rollback = NULL, // unused
            .anim_in = "draw_odd_index",
            .anim_out = "no_choosen_odd_index",
            .hidden = false,
            .description = NULL // unused
        },
    },
    .items_size = 4
};

static const GamepadButtons MAINMENU_GAMEPAD_SHOW_CREDITS = GAMEPAD_DPAD_DOWN | GAMEPAD_DPAD_UP | GAMEPAD_DPAD_LEFT | GAMEPAD_DPAD_RIGHT;


static void mainmenu_show_donate();
static void mainmenu_show_credits(Layout layout);
static bool mainmenu_handle_selected_option(int32_t selected_index);
static bool mainmenu_handle_modding_option(ModdingHelper* moddinghelper, const char* option_name);


bool mainmenu_main() {
    Layout layout = layout_init(pvr_context_is_widescreen() ? MAINMENU_LAYOUT : MAINMENU_LAYOUT_DREAMCAST);
    if (!layout) {
        logger_warn("mainmenu_main() can not load mainmenu layout");
        return mainmenu_handle_selected_option(0);
    }

    LayoutPlaceholder* menu_placeholder = layout_get_placeholder(layout, "menu");
    float delay_after_choose = layout_get_attached_value_as_float(layout, "delay_after_choose", 1200.0f);

    // default values
    MAINMENU_MENU_MANIFEST.parameters.items_align = ALIGN_CENTER;
    MAINMENU_MENU_MANIFEST.parameters.is_vertical = true;
    MAINMENU_MENU_MANIFEST.parameters.items_dimmen = 60.0f;
    MAINMENU_MENU_MANIFEST.parameters.items_gap = 24.0f;
    MAINMENU_MENU_MANIFEST.parameters.texture_scale = 0.0f;

    float x = 0.0f;
    float y = 0.0f;
    float z = 2.0f;
    float size_width = pvr_context.screen_width, size_height = pvr_context.screen_height;

    layout_get_viewport_size(layout, &size_width, &size_height);

    if (menu_placeholder) {
        x = menu_placeholder->x;
        y = menu_placeholder->y;
        z = menu_placeholder->z;
        if (menu_placeholder->width > 0.0f) size_width = menu_placeholder->width;
        if (menu_placeholder->height > 0.0f) size_height = menu_placeholder->height;

        MAINMENU_MENU_MANIFEST.parameters.is_vertical = layout_get_attached_value_boolean(
            layout, "menu_isVertical",
            MAINMENU_MENU_MANIFEST.parameters.is_vertical
        );

        if (MAINMENU_MENU_MANIFEST.parameters.is_vertical) {
            if (menu_placeholder->align_horizontal != ALIGN_NONE)
                MAINMENU_MENU_MANIFEST.parameters.items_align = menu_placeholder->align_horizontal;
        } else {
            if (menu_placeholder->align_vertical != ALIGN_NONE)
                MAINMENU_MENU_MANIFEST.parameters.items_align = menu_placeholder->align_vertical;
        }

        MAINMENU_MENU_MANIFEST.parameters.items_dimmen = (float)layout_get_attached_value_double(
            layout, "menu_itemDimmen",
            MAINMENU_MENU_MANIFEST.parameters.items_dimmen
        );

        MAINMENU_MENU_MANIFEST.parameters.texture_scale = (float)layout_get_attached_value_double(
            layout, "menu_itemScale",
            MAINMENU_MENU_MANIFEST.parameters.texture_scale
        );

        MAINMENU_MENU_MANIFEST.parameters.items_gap = (float)layout_get_attached_value_double(
            layout, "menu_itemGap",
            MAINMENU_MENU_MANIFEST.parameters.items_gap
        );
    }

    // load custom menumanifest if exists
    MenuManifest menumanifest = main_helper_init_menumanifest_for_dreamcast(MAINMENU_MODDING_MENU);
    if (!menumanifest) menumanifest = &MAINMENU_MENU_MANIFEST;

    Menu menu = menu_init(menumanifest, x, y, z, size_width, size_height);
    menu_trasition_in(menu);
    menu_select_index(menu, 0);
    main_helper_trigger_action_menu2(layout, menumanifest, 0, NULL, true, false);

    if (menu_placeholder) {
        menu_placeholder->vertex = menu_get_drawable(menu);
    } else {
        int32_t index = layout_external_create_group(layout, NULL, 0);
        layout_external_vertex_create_entries(layout, 1);
        layout_external_vertex_set_entry(layout, 0, VERTEX_DRAWABLE, menu_get_drawable(menu), index);
    }

    TextSprite info_textsprite;
    info_textsprite = layout_get_textsprite(layout, "info-engine");
    if (info_textsprite) {
        textsprite_set_text_formated(info_textsprite, "$s $s\u00A0", ENGINE_NAME, ENGINE_VERSION);
    }

    info_textsprite = layout_get_textsprite(layout, "info-vmu");
    if (info_textsprite) {
        int8_t port, unit;
        if (funkinsave_get_vmu(&port, &unit)) {
            char port_name = (char)(0x41 + port);
            char slot_name = (char)(0x30 + unit);
            const char* state = funkinsave_is_vmu_missing() ? " (disconnected)" : "";
            const char* loaded = savemanager_is_running_without_savedata() ? " (without savedata)" : "";
            textsprite_set_text_formated(info_textsprite, "VMU $c$c$s$s\u00A0", port_name, slot_name, state, loaded);
        } else {
            textsprite_set_text(info_textsprite, "No VMU available\u00A0");
        }
    }

    // attach camera animation (if was defined)
    layout_trigger_camera(layout, "camera_animation");

    SoundPlayer sound_confirm = soundplayer_init("/assets/common/sound/confirmMenu.ogg");
    SoundPlayer sound_scroll = soundplayer_init("/assets/common/sound/scrollMenu.ogg");
    SoundPlayer sound_asterik = soundplayer_init("/assets/common/sound/asterikMenu.ogg");
    SoundPlayer sound_cancel = soundplayer_init("/assets/common/sound/cancelMenu.ogg");

    Gamepad maple_pad = gamepad_init(-1);
    gamepad_set_buttons_delay(maple_pad, MAINMENU_OPTION_SELECTION_DELAY);
    gamepad_clear_buttons(maple_pad);

    ModdingHelper moddinghelper = {
        .menumanifest = menumanifest,
        .choosen_name = NULL,
        .choosen_name_is_allocated = false
    };

    Modding modding = modding_init(layout, MAINMENU_MODDING_SCRIPT);
    modding->native_menu = modding->active_menu = menu;
    modding->callback_private_data = &moddinghelper;
    modding->callback_option = (ModdingCallbackOption)mainmenu_handle_modding_option;
    modding_helper_notify_init(modding, MODDING_NATIVE_MENU_SCREEN, ModdingValueType_string);
    modding_helper_notify_option(modding, true);

    int32_t last_selected_index = 0;

    while (!modding->has_exit) {
        if (moddinghelper.choosen_name != NULL) {
            if (/* pointer equals */ moddinghelper.choosen_name == MAINMENU_BACK_TO_STARTSCREEN)
                moddinghelper.choosen_name = NULL;
            else
                menu_select_item(menu, moddinghelper.choosen_name);
            break;
        }

        int32_t selection_offset_x = 0;
        int32_t selection_offset_y = 0;
        float elapsed = pvrctx_wait_ready();
        GamepadButtons buttons = gamepad_has_pressed_delayed(maple_pad, MAINMENU_GAMEPAD_BUTTONS);

        pvr_context_reset(&pvr_context);

        ModdingHelperResult res = modding_helper_handle_custom_menu(modding, maple_pad, elapsed);
        if (res != ModdingHelperResult_CONTINUE) break;

        layout_animate(layout, elapsed);
        layout_draw(layout, &pvr_context);

        if (modding->has_halt || modding->active_menu != menu) continue;

        if (buttons & MAINMENU_GAMEPAD_OK) {
            if (menu_has_valid_selection(menu)) {
                moddinghelper.choosen_name = menu_get_selected_item_name(menu);
                if (modding_helper_notify_option(modding, false)) {
                    moddinghelper.choosen_name = NULL;
                    continue;
                }
                // Note: the modding script can override the choosen option
                break;
            }
        } else if ((buttons & MAINMENU_GAMEPAD_CANCEL) && !modding_helper_notify_back(modding))
            break;
        else if ((buttons & MAINMENU_GAMEPAD_SHOW_CREDITS) == MAINMENU_GAMEPAD_SHOW_CREDITS) {
            mainmenu_show_credits(layout);
            gamepad_clear_buttons(maple_pad);
        } else if (buttons & GAMEPAD_AD_DOWN)
            selection_offset_y++;
        else if (buttons & GAMEPAD_AD_UP)
            selection_offset_y--;
        else if (buttons & GAMEPAD_AD_LEFT)
            selection_offset_x--;
        else if (buttons & GAMEPAD_AD_RIGHT)
            selection_offset_x++;

        if (selection_offset_x == 0 && selection_offset_y == 0) continue;

        bool success = false;

        if (selection_offset_x != 0 && menu_select_horizontal(menu, selection_offset_x)) success = true;
        if (selection_offset_y != 0 && menu_select_vertical(menu, selection_offset_y)) success = true;

        if (success) {
            if (sound_asterik) soundplayer_stop(sound_asterik);
            if (sound_scroll) soundplayer_replay(sound_scroll);

            main_helper_trigger_action_menu2(layout, menumanifest, last_selected_index, NULL, false, false);
            last_selected_index = menu_get_selected_index(menu);
            main_helper_trigger_action_menu2(layout, menumanifest, last_selected_index, NULL, true, false);
            modding_helper_notify_option(modding, true);
        } else {
            if (sound_scroll) soundplayer_stop(sound_scroll);
            if (sound_asterik) soundplayer_replay(sound_asterik);
        }
    }

    int32_t choosen_option_index = 0;
    bool has_choosen_option = moddinghelper.choosen_name != NULL;

    // apply choosen transition
    if (sound_scroll) soundplayer_stop(sound_scroll);
    SoundPlayer target_sound = has_choosen_option ? sound_confirm : sound_cancel;
    if (target_sound) soundplayer_replay(target_sound);
    layout_trigger_any(layout, has_choosen_option ? "option_selected" : "return");
    if (has_choosen_option)
        menu_toggle_choosen(menu, true);
    else
        menu_trasition_out(menu);

    if (has_choosen_option) {
        main_helper_trigger_action_menu(layout, NULL, moddinghelper.choosen_name, false, true);
    }

    // animate choosen option
    if (has_choosen_option) {
        float total_elapsed = 0.0f;
        while (total_elapsed < delay_after_choose && !modding->has_exit) {
            float elapsed = pvrctx_wait_ready();
            if (!modding->has_halt) total_elapsed += elapsed;

            layout_animate(layout, elapsed);
            layout_draw(layout, &pvr_context);
        }
    }

    // trigger outro transition
    layout_trigger_action(layout, NULL, "outro");
    menu_trasition_out(menu);

    while (true) {
        float elapsed = pvrctx_wait_ready();

        layout_animate(layout, elapsed);
        layout_draw(layout, &pvr_context);

        if (layout_animation_is_completed(layout, "transition_effect")) {
            // flush framebuffer again with last fade frame
            pvrctx_wait_ready();
            break;
        }
    }

    if (has_choosen_option) {
        // obtain the native option index
        choosen_option_index = menumanifest_get_option_index(&MAINMENU_MENU_MANIFEST, moddinghelper.choosen_name);

        // special case for "story mode"
        if (choosen_option_index < 0 && string_equals(moddinghelper.choosen_name, "story mode")) {
            choosen_option_index = menumanifest_get_option_index(&MAINMENU_MENU_MANIFEST, "storymode");
        }

        if (choosen_option_index < 0) {
            modding->callback_option = NULL;
            modding_helper_notify_handle_custom_option(modding, moddinghelper.choosen_name);
        }
    }

    if (moddinghelper.choosen_name_is_allocated) free_chk((char*)moddinghelper.choosen_name);

    modding_helper_notify_exit2(modding);

    menu_destroy(&menu);
    gamepad_destroy(&maple_pad);
    if (sound_confirm) soundplayer_destroy(&sound_confirm);
    if (sound_scroll) soundplayer_destroy(&sound_scroll);
    if (sound_cancel) soundplayer_destroy(&sound_cancel);
    if (sound_asterik) soundplayer_destroy(&sound_asterik);
    layout_destroy(&layout);
    modding_destroy(&modding);

    if (menumanifest != &MAINMENU_MENU_MANIFEST) menumanifest_destroy(&menumanifest);

    savemanager_check_and_save_changes();

    // if no option was selected, jump to the start screen
    if (!has_choosen_option) return false;

    // handle user action
    return mainmenu_handle_selected_option(choosen_option_index);
}

static void mainmenu_show_donate() {
    Layout layout = layout_init("/assets/common/credits/donation.xml");
    if (!layout) return;

    bool pause_background_menu_music = layout_get_attached_value_boolean(
        layout, "pause_background_menu_music", false
    );
    float timeout = (float)layout_get_attached_value_double(
        layout, "timeout", -1.0
    );
    const char* donate_url = layout_get_attached_value_string(
        layout, "donate_url", NULL
    );

    if (pause_background_menu_music) soundplayer_pause(background_menu_music);

    Gamepad gamepad = gamepad_init(-1);
    gamepad_set_buttons_delay(gamepad, 250);
    gamepad_clear_buttons(gamepad);

    float progress = 0.0f;
    if (timeout < 0.0f) timeout = FLOAT_Inf;

    while (progress < timeout) {
        float elapsed = pvrctx_wait_ready();
        pvr_context_reset(&pvr_context);

        progress += elapsed;

        layout_animate(layout, elapsed);
        layout_draw(layout, &pvr_context);

        GamepadButtons buttons = gamepad_has_pressed_delayed(gamepad, CREDITS_BUTTONS);
        if (buttons) {
            if ((buttons & (GAMEPAD_A | GAMEPAD_START)) && donate_url) {
                gamepad_clear_buttons2(gamepad, GAMEPAD_A | GAMEPAD_START);
                logger_info("www link opening is not supported in the current platform. url=%s", donate_url);
            }
            break;
        }
    }

    layout_destroy(&layout);
    gamepad_destroy(&gamepad);

    if (pause_background_menu_music) soundplayer_play(background_menu_music);
}

static void mainmenu_show_credits(Layout layout) {
    layout_trigger_any(layout, "outro");

    bool has_bg_music = background_menu_music && soundplayer_is_playing(background_menu_music);
    if (has_bg_music) soundplayer_fade(background_menu_music, false, 500.0f);

    while (true) {
        float elapsed = pvrctx_wait_ready();

        layout_animate(layout, elapsed);
        layout_draw(layout, &pvr_context);

        if (layout_animation_is_completed(layout, "transition_effect") > 0) {
            // flush framebuffer again with last fade frame
            pvrctx_wait_ready();
            break;
        }
    }

    credits_main();

    layout_trigger_any(layout, "intro");

    if (has_bg_music) {
        soundplayer_play(background_menu_music);
        soundplayer_fade(background_menu_music, true, 500.0f);
    }
}

static bool mainmenu_handle_selected_option(int32_t selected_index) {
    //
    // all main menu options are handled here
    //
    // return 0 to go back to the start-screen, otherwise, reload the main-menu
    //
    switch (selected_index) {
        case -1:
            // from custom menu
            return true;
        case 0: // storymode
            while (weekselector_main() > 0);
            return true; // main-menu
        case 1:
            freeplaymenu_main();
            return true; // main-menu
        case 2:
            mainmenu_show_donate();
            return true; // main-menu
        case 3:
            settingsmenu_main();
            return true; // main-menu
        default:
            logger_log("unimplemented selected option: " FMT_I4, selected_index);
            return false;
    }
}

static bool mainmenu_handle_modding_option(ModdingHelper* moddinghelper, const char* option_name) {
    if (moddinghelper->choosen_name_is_allocated) {
        free_chk((char*)moddinghelper->choosen_name);
        moddinghelper->choosen_name_is_allocated = false;
    }

    if (option_name == NULL || string_equals(option_name, MAINMENU_BACK_TO_STARTSCREEN)) {
        // assume is going back
        moddinghelper->choosen_name = MAINMENU_BACK_TO_STARTSCREEN;
        return true;
    }

    // check if the option is present on the menu (native or custom)
    int32_t index = menumanifest_get_option_index(moddinghelper->menumanifest, option_name);
    if (index >= 0) {
        moddinghelper->choosen_name = moddinghelper->menumanifest->items[index].name;
        return true;
    }

    // check if the option is native
    index = menumanifest_get_option_index(&MAINMENU_MENU_MANIFEST, option_name);
    if (index >= 0) {
        moddinghelper->choosen_name = MAINMENU_MENU_MANIFEST.items[index].name;
        return true;
    }

    // unknown option
    moddinghelper->choosen_name = string_duplicate(option_name);
    moddinghelper->choosen_name_is_allocated = true;
    return false;
}
