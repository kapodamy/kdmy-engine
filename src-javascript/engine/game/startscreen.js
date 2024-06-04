"use strict";

const STARTSCREEN_LAYOUT = "/assets/common/image/start-screen/layout.xml";
const STARTSCREEN_LAYOUT_DREAMCAST = "/assets/common/image/start-screen/layout~dreamcast.xml";
const STARTSCREEN_MODDING_SCRIPT = "/assets/common/data/scripts/startscreen.lua";


async function startscreen_main() {
    let layout = await layout_init(pvr_context_is_widescreen() ? STARTSCREEN_LAYOUT : STARTSCREEN_LAYOUT_DREAMCAST);
    if (!layout) {
        console.warn("startscreen_main() can not load mainmenu layout");
        return 0;
    }

    let delay_after_start = layout_get_attached_value_as_float(layout, "delay_after_start", 2000.0);
    let soundplayer_confirm = await soundplayer_init("/assets/common/sound/confirmMenu.ogg");
    let maple_pad = gamepad_init(-1);
    let modding = await modding_init(layout, STARTSCREEN_MODDING_SCRIPT);

    let total_elapsed = 0;
    let enter_pressed = 0;
    let trigger_fade_away = 1;
    let exit_to_bios = 0;

    const moddinghelper = { start_pressed: 0, exit_to_bios: 0 };;
    modding.callback_private_data = moddinghelper;
    modding.callback_option = startscreen_internal_handle_modding_option;

    gamepad_clear_buttons(maple_pad);
    gamepad_set_buttons_delay(maple_pad, 200);
    await modding_helper_notify_init(modding, MODDING_NATIVE_MENU_SCREEN);
    await modding_helper_notify_event(modding, "start_screen");

    while (1) {
        let elapsed = await pvrctx_wait_ready();
        layout_animate(layout, elapsed);

        let res = await modding_helper_handle_custom_menu(modding, maple_pad, elapsed);
        if (modding.has_exit || res != MODDING_HELPER_RESULT_CONTINUE) break;

        let pressed = gamepad_has_pressed_delayed(maple_pad, MAINMENU_GAMEPAD_BUTTONS);
        if (modding.has_halt) pressed = 0x00;

        if ((pressed & MAINMENU_GAMEPAD_OK) || moddinghelper.start_pressed) {
            if (enter_pressed) {
                break;
            } else {
                enter_pressed = 1;
                moddinghelper.start_pressed = 0;
                if (soundplayer_confirm) soundplayer_play(soundplayer_confirm);
                layout_trigger_any(layout, "start_pressed");
                await modding_helper_notify_event(modding, "start_pressed");
                gamepad_clear_buttons2(maple_pad, MAINMENU_GAMEPAD_OK);
            }
        } else if (enter_pressed) {
            if (total_elapsed >= delay_after_start) {
                if (trigger_fade_away && !exit_to_bios) {
                    trigger_fade_away = 0;
                    layout_trigger_any(layout, "outro");
                    await modding_helper_notify_event(modding, "outro");
                } else {
                    let state = layout_animation_is_completed(layout, "transition-effect");

                    if (state == -1) break;// infinite looped animation

                    // check if:
                    //          the animation is completed (1)
                    //          there no animation on the sprite (2)
                    //          there no sprite in the layout with that name (3)
                    else if (state != 0) break;
                }
            }

            if (!modding.has_halt) total_elapsed += elapsed;
        } else if (((pressed & MAINMENU_GAMEPAD_CANCEL) && !await modding_helper_notify_back(modding)) || moddinghelper.exit_to_bios) {
            layout_trigger_any(layout, "outro");
            await modding_helper_notify_event(modding, "exit_to_bios");
            if (background_menu_music) soundplayer_fade(background_menu_music, false, 1000.0);
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

function startscreen_internal_handle_modding_option(moddinghelper, option_name) {
    switch (option_name) {
        case null:
        case "exit_to_bios":
            moddinghelper.start_pressed = 1;
            break;
        case "start_pressed":
            moddinghelper.exit_to_bios = 1;
            break;
        default:
            return 0;
    }
    return 1;
}

