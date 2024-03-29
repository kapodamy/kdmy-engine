"use strict";

const SCREENMENU_BUTTONS_DELAY = 200;
const SCREENMENU_BACK_BUTTONS = GAMEPAD_BACK | GAMEPAD_B;

async function screenmenu_init(layout_src, script_src) {
    let layout = await layout_init(layout_src);
    if (!layout) {
        console.error("screenmenu_init() failed to load layout: " + layout_src);
        return null;
    }

    let modding = await modding_init(layout, script_src);
    modding.exit_delay_ms = layout_get_attached_value_as_float(layout, "exit_delay", 0.0);

    return { modding, layout };
}

async function screenmenu_destroy(screenmenu) {
    layout_destroy(screenmenu.layout);
    await modding_destroy(screenmenu.modding);
    screenmenu = undefined;
}

async function screenmenu_display(screenmenu, pvrctx, script_arg) {
    const script = screenmenu.modding.script != null ? weekscript_get_luascript(screenmenu.modding.script) : null;
    const layout = screenmenu.layout;

    if (script != null) {
        await luascript_notify_modding_init(script, script_arg);
        if (screenmenu.modding.has_exit) return await luascript_notify_modding_exit(script);
    }

    let active_gamepad_delay = 0.0;
    let last_pressed = 0x00;
    let gamepad = gamepad_init(-1);
    gamepad_clear_buttons(gamepad);

    while (!screenmenu.modding.has_exit) {
        let elapsed = await pvrctx_wait_ready();
        pvr_context_reset(pvrctx);

        let pressed = gamepad_get_pressed(gamepad);
        let back_pressed = (pressed & SCREENMENU_BACK_BUTTONS) != 0;

        // ignore back buttons if halt flag not is signalated and call "f_modding_back" instead
        if (back_pressed && !screenmenu.modding.has_halt) pressed &= ~SCREENMENU_BACK_BUTTONS;

        if (script != null) {
            if (last_pressed != pressed) {
                last_pressed = pressed;
                await weekscript_notify_buttons(screenmenu.modding.script, -1, pressed);
            }
            await weekscript_notify_frame(screenmenu.modding.script, elapsed);
        }

        L_process_gamepad: {
            if (screenmenu.modding.has_halt) break L_process_gamepad;

            if (active_gamepad_delay > 0.0) {
                active_gamepad_delay -= elapsed;
                if (active_gamepad_delay > 0.0) break L_process_gamepad;
            }

            let menu = screenmenu.modding.active_menu;

            let go_back = 0;
            let has_selected = 0;
            let has_choosen = false;

            if (back_pressed)
                go_back = 1;
            else if (menu == null) {
                break L_process_gamepad;
            } else if ((pressed & GAMEPAD_DALL_LEFT) != 0)
                has_selected = menu_select_horizontal(menu, -1);
            else if ((pressed & GAMEPAD_DALL_RIGHT) != 0)
                has_selected = menu_select_horizontal(menu, 1);
            else if ((pressed & GAMEPAD_DALL_UP) != 0)
                has_selected = menu_select_vertical(menu, -1);
            else if ((pressed & GAMEPAD_DALL_DOWN) != 0)
                has_selected = menu_select_vertical(menu, 1);
            else if ((pressed & MAINMENU_GAMEPAD_OK) != 0)
                has_choosen = menu_get_selected_index(menu) >= 0;


            if (go_back) {
                if (script == null) break;
                if (!await luascript_notify_modding_back(script)) break;
                active_gamepad_delay = SCREENMENU_BUTTONS_DELAY;
            }

            if (has_selected) {
                if (script == null) {
                    let index = menu_get_selected_index(menu);
                    let name = menu_get_selected_item_name(menu);
                    await luascript_notify_modding_menu_option_selected(script, menu, index, name);
                }
                active_gamepad_delay = SCREENMENU_BUTTONS_DELAY;
            }

            if (has_choosen) {
                if (script == null) break;

                let index = menu_get_selected_index(menu);
                let name = menu_get_selected_item_name(menu);

                if (!await luascript_notify_modding_menu_option_choosen(script, menu, index, name)) {
                    menu_toggle_choosen(menu, 1);
                    break;
                }

                active_gamepad_delay = SCREENMENU_BUTTONS_DELAY;
            }
        }

        layout_animate(layout, elapsed);
        layout_draw(layout, pvrctx);
    }

    // if there no script ¿exit_value should be null or pick the menu selectd option name?
    let exit_value = null;
    if (script != null) exit_value = await luascript_notify_modding_exit(script);

    // delay exit (if applicable)
    let exit_delay = screenmenu.modding.exit_delay_ms;
    while (exit_delay > 0) {
        let elapsed = await pvrctx_wait_ready();

        exit_delay -= elapsed;
        pvr_context_reset(pvrctx);

        if (script != null) {
            await weekscript_notify_frame(screenmenu.modding.script, elapsed);
        }

        layout_animate(layout, elapsed);
        layout_draw(layout, pvrctx);
    }

    // flush framebuffer again with last fade frame
    await pvrctx_wait_ready();

    gamepad_destroy(gamepad);
    return exit_value;
}
