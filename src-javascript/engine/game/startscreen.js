"use strict";

const STARTSCREEN_LAYOUT = "/assets/common/image/start-screen/layout.xml";
const STARTSCREEN_LAYOUT_DREAMCAST = "/assets/common/image/start-screen/layout~dreamcast.xml";
const STARTSCREEN_MODDING_SCRIPT = "/assets/data/scripts/startscreen.lua";

var startscreen_has_press_start;

async function startscreen_main() {
    let layout = await layout_init(pvrctx_is_widescreen() ? STARTSCREEN_LAYOUT : STARTSCREEN_LAYOUT_DREAMCAST);
    if (!layout) {
        console.warn("startscreen_main() can not load mainmenu layout");
        return 0;
    }

    let soundplayer_confirm = await soundplayer_init("/assets/common/sound/confirmMenu.ogg");
    let maple_pad = gamepad_init(-1);
    let modding = await modding_init(layout, STARTSCREEN_MODDING_SCRIPT);

    let total_elapsed = 0;
    let enter_pressed = 0;
    let trigger_fade_away = 1;
    let exit_to_bios = 0;

    startscreen_has_press_start = 0;
    modding.callback_private_data = null;
    modding.callback_option = startscreen_internal_handle_option;

    gamepad_clear_buttons(maple_pad);
    gamepad_set_buttons_delay(maple_pad, 200);
    await modding_helper_notify_init(modding, MODDING_NATIVE_MENU_SCREEN);
    await modding_helper_notify_event(modding, "start-screen");

    while (1) {
        let elapsed = await pvrctx_wait_ready();

        layout_animate(layout, elapsed);

        let res = await modding_helper_handle_custom_menu(modding, maple_pad, elapsed);
        if (modding.has_exit || res != MODDING_HELPER_RESULT_CONTINUE) break;

        let pressed = gamepad_has_pressed_delayed(maple_pad, MAINMENU_GAMEPAD_BUTTONS);
        if (modding.has_halt) pressed = 0x00;

        if (!enter_pressed && ((pressed & MAINMENU_GAMEPAD_OK) || startscreen_has_press_start)) {
            enter_pressed = 1;
            if (soundplayer_confirm) soundplayer_play(soundplayer_confirm);
            layout_trigger_any(layout, "start_pressed");
            modding.HelperNotifyModdingEvent("start_pressed");
        } else if (enter_pressed) {
            if (total_elapsed >= 2000) {
                if (trigger_fade_away) {
                    trigger_fade_away = 0;
                    layout_trigger_any(layout, "outro");
                    modding.HelperNotifyModdingEvent("outro");
                } else {
                    let state = layout_animation_is_completed(layout, "transition-intro-outro-effect");

                    if (state == -1) break;// infinite looped animation

                    // check if:
                    //          the animation is completed (1)
                    //          there no animation on the sprite (2)
                    //          there no sprite in the layout with that name (3)
                    else if (state != 0) break;
                }
            }

            if (!modding.has_halt) total_elapsed += elapsed;
        } else if ((pressed & MAINMENU_GAMEPAD_CANCEL) && !await modding_helper_notify_back(modding)) {
            layout_trigger_any(layout, "outro");
            enter_pressed = 1;
            exit_to_bios = 1;
        }

        pvr_context_reset(pvr_context);
        layout_draw(layout, pvr_context);
    };

    await modding_helper_notify_exit2(modding);

    await modding_destroy(modding);
    layout_destroy(layout);
    gamepad_destroy(maple_pad);
    if (soundplayer_confirm) soundplayer_destroy(soundplayer_confirm);

    if (exit_to_bios) {
        // boot dreamcast BIOS menu
        if (background_menu_music) soundplayer_stop(background_menu_music);
        arch_menu();
        return 1;
    }

    return 0;
}

function startscreen_internal_handle_option(priv_data, option_name) {
    let has_press_start = option_name == "start_pressed";
    startscreen_has_press_start = has_press_start;
    return has_press_start;
}

