#include "game/startscreen.h"

#include <arch/arch.h>

#include "game/main.h"
#include "game/mainmenu.h"
#include "game/modding.h"
#include "gamepad.h"
#include "layout.h"
#include "logger.h"
#include "pvrcontext.h"
#include "pvrcontext_global.h"
#include "soundplayer.h"
#include "stringutils.h"


typedef struct {
    bool start_pressed;
    bool exit_to_bios;
} ModdingHelper;


static const char* STARTSCREEN_LAYOUT = "/assets/common/image/start-screen/layout.xml";
static const char* STARTSCREEN_LAYOUT_DREAMCAST = "/assets/common/image/start-screen/layout~dreamcast.xml";
static const char* STARTSCREEN_MODDING_SCRIPT = "/assets/common/data/scripts/startscreen.lua";


static bool startscreen_internal_handle_modding_option(ModdingHelper* moddinghelper, const char* option_name);


bool startscreen_main() {
    Layout layout = layout_init(pvr_context_is_widescreen() ? STARTSCREEN_LAYOUT : STARTSCREEN_LAYOUT_DREAMCAST);
    if (!layout) {
        logger_warn("startscreen_main() can not load mainmenu layout");
        return false;
    }

    float delay_after_start = layout_get_attached_value_as_float(layout, "delay_after_start", 2000.0f);
    SoundPlayer soundplayer_confirm = soundplayer_init("/assets/common/sound/confirmMenu.ogg");
    Gamepad maple_pad = gamepad_init(-1);
    Modding modding = modding_init(layout, STARTSCREEN_MODDING_SCRIPT);

    float total_elapsed = 0.0f;
    bool enter_pressed = false;
    bool trigger_fade_away = true;
    bool exit_to_bios = false;

    ModdingHelper moddinghelper = {.start_pressed = false, .exit_to_bios = false};
    modding->callback_private_data = &moddinghelper;
    modding->callback_option = (ModdingCallbackOption)startscreen_internal_handle_modding_option;

    gamepad_clear_buttons(maple_pad);
    gamepad_set_buttons_delay(maple_pad, 200);
    modding_helper_notify_init(modding, MODDING_NATIVE_MENU_SCREEN, ModdingValueType_string);
    modding_helper_notify_event(modding, "start_screen");

    while (true) {
        float elapsed = pvrctx_wait_ready();
        layout_animate(layout, elapsed);

        ModdingHelperResult res = modding_helper_handle_custom_menu(modding, maple_pad, elapsed);
        if (modding->has_exit || res != ModdingHelperResult_CONTINUE) break;

        GamepadButtons pressed = gamepad_has_pressed_delayed(maple_pad, MAINMENU_GAMEPAD_BUTTONS);
        if (modding->has_halt) pressed = 0x00;

        if ((pressed & MAINMENU_GAMEPAD_OK) || moddinghelper.start_pressed) {
            if (enter_pressed) {
                break;
            } else {
                enter_pressed = true;
                moddinghelper.start_pressed = false;
                if (soundplayer_confirm) soundplayer_play(soundplayer_confirm);
                layout_trigger_any(layout, "start_pressed");
                modding_helper_notify_event(modding, "start_pressed");
                gamepad_clear_buttons2(maple_pad, MAINMENU_GAMEPAD_OK);
            }
        } else if (enter_pressed) {
            if (total_elapsed >= delay_after_start) {
                if (trigger_fade_away && !exit_to_bios) {
                    trigger_fade_away = false;
                    layout_trigger_any(layout, "outro");
                    modding_helper_notify_event(modding, "outro");
                } else {
                    int32_t state = layout_animation_is_completed(layout, "transition-effect");

                    if (state == -1) break; // infinite looped animation

                    // check if:
                    //          the animation is completed (1)
                    //          there no animation on the sprite (2)
                    //          there no sprite in the layout with that name (3)
                    else if (state != 0)
                        break;
                }
            }

            if (!modding->has_halt) total_elapsed += elapsed;
        } else if (((pressed & MAINMENU_GAMEPAD_CANCEL) && !modding_helper_notify_back(modding)) || moddinghelper.exit_to_bios) {
            layout_trigger_any(layout, "outro");
            modding_helper_notify_event(modding, "exit_to_bios");
            if (background_menu_music) soundplayer_fade(background_menu_music, false, 1000.0f);
            enter_pressed = true;
            exit_to_bios = true;
        }

        pvr_context_reset(&pvr_context);
        layout_draw(layout, &pvr_context);
    };

    modding_helper_notify_exit2(modding);

    modding_destroy(&modding);
    layout_destroy(&layout);
    gamepad_destroy(&maple_pad);
    if (soundplayer_confirm) soundplayer_destroy(&soundplayer_confirm);

    if (exit_to_bios) {
        // boot dreamcast BIOS menu
        if (background_menu_music) soundplayer_stop(background_menu_music);
        arch_menu();
        return true;
    }

    return false;
}

static bool startscreen_internal_handle_modding_option(ModdingHelper* moddinghelper, const char* option_name) {
    if (option_name == NULL || string_equals(option_name, "exit_to_bios")) {
        moddinghelper->start_pressed = true;
    } else if (string_equals(option_name, "start_pressed")) {
        moddinghelper->exit_to_bios = true;
    } else {
        return false;
    }
    return true;
}
