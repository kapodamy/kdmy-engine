"use strict";

const WEEKPAUSE_MENU = {
    parameters: {
        suffix_selected: null,// unused
        suffix_idle: null,// unused
        suffix_choosen: null,// unused
        suffix_discarded: null,// unused
        suffix_rollback: null,// unused
        suffix_in: null,// unused
        suffix_out: null,// unused

        atlas: null,
        animlist: "/assets/common/anims/week-round.xml",

        anim_selected: "pause_item_selected",
        anim_idle: "pause_item_idle",
        anim_choosen: null,// unused
        anim_discarded: null,// unused
        anim_rollback: null,// unused
        anim_in: null,// unused
        anim_out: null,// unused

        anim_transition_in_delay: 0.0,// unused
        anim_transition_out_delay: 0.0,// unused

        font: null,
        font_glyph_suffix: "bold",
        font_color_by_addition: false,// unused
        font_size: 48.0,
        font_color: 0xFFFFFF,
        font_border_color: 0x00,// unused
        font_border_size: NaN,// unused

        is_sparse: false,// unused
        is_vertical: true,
        is_per_page: false,
        static_index: 1,

        items_align: ALIGN_START,
        items_gap: 48.0,
        items_dimmen: 0.0,// unused
        texture_scale: NaN,// unused
        enable_horizontal_text_correction: true// unused
    },
    items: [
        {
            name: "resume",
            text: "RESUME",// unused
            placement: { x: 0.0, y: 0.0, dimmen: 0.0, gap: 0.0 },// unused
            anim_selected: null,// unused
            anim_choosen: null,// unused
            anim_discarded: null,// unused
            anim_idle: null,// unused
            anim_rollback: null,// unused
            anim_in: null,// unused
            anim_out: null,// unused
            hidden: false,
            description: null,// usused
        },
        {
            name: "week-menu",
            text: "WEEK MENU",// unused
            placement: { x: 0.0, y: 0.0, dimmen: 0.0, gap: 0.0 },// unused
            anim_selected: null,// unused
            anim_choosen: null,// unused
            anim_discarded: null,// unused
            anim_idle: null,// unused
            anim_rollback: null,// unused
            anim_in: null,// unused
            anim_out: null,// unused
            hidden: true,
            description: null,// usused
        },
        {
            name: "restart-song",
            text: "RESTART SONG",// unused
            placement: { x: 0.0, y: 0.0, dimmen: 0.0, gap: 0.0 },// unused
            anim_selected: null,// unused
            anim_choosen: null,// unused
            anim_discarded: null,// unused
            anim_idle: null,// unused
            anim_rollback: null,// unused
            anim_in: null,// unused
            anim_out: null,// unused
            hidden: false,
            description: null,// usused
        },
        {
            name: "exit-week-selector",
            text: "EXIT TO WEEK SELECTOR",// unused
            placement: { x: 0.0, y: 0.0, dimmen: 0.0, gap: 0.0 },// unused
            anim_selected: null,// unused
            anim_choosen: null,// unused
            anim_discarded: null,// unused
            anim_idle: null,// unused
            anim_rollback: null,// unused
            anim_in: null,// unused
            anim_out: null,// unused
            hidden: false,
            description: null,// usused
        },
        {
            name: "exit-main-menu",
            text: "EXIT TO MAIN MENU",// unused
            placement: { x: 0.0, y: 0.0, dimmen: 0.0, gap: 0.0 },// unused
            anim_selected: null,// unused
            anim_choosen: null,// unused
            anim_discarded: null,// unused
            anim_idle: null,// unused
            anim_rollback: null,// unused
            anim_in: null,// unused
            anim_out: null,// unused
            hidden: false,
            description: null,// usused
        }
    ],
    items_size: 5
};
const WEEKPAUSE_LAYOUT_WIDESCREEN = "/assets/common/image/week-round/pause.xml";
const WEEKPAUSE_LAYOUT_DREAMCAST = "/assets/common/image/week-round/pause~dreamcast.xml";
const WEEKPAUSE_NOCONTROLLER = "/assets/common/image/week-round/no_controller.png";
const WEEKPAUSE_MODDING_SCRIPT = "/assets/common/data/scripts/weekpause.lua";
const WEEKPAUSE_MODDING_MENU = "/assets/common/data/menus/weekpause.json";
const WEEKPAUSE_BUTTONS = GAMEPAD_START | GAMEPAD_AD_UP | GAMEPAD_AD_DOWN | GAMEPAD_A | GAMEPAD_X | GAMEPAD_B | GAMEPAD_BACK;
const WEEKPAUSE_DELAY = 170;
const WEEKPAUSE_ANTIBOUNCE = WEEKPAUSE_DELAY * 1.25;

// messagebox strings
const WEEKPAUSE_MSGCONTROLLER = "The controller $i was disconnected, \n" +
    "reconnect it or press START on an unused \n" +
    "controller to replace it.";
const WEEKPAUSE_MSGMENU = "The week progress will be lost, ¿return\n to the main menu?";
const WEEKPAUSE_MSGWEEKSELECTOR = "The week progress will be lost, ¿return?";


async function week_pause_init(exit_to_weekselector_label) {
    let src = pvr_context_is_widescreen() ? WEEKPAUSE_LAYOUT_WIDESCREEN : WEEKPAUSE_LAYOUT_DREAMCAST;
    let layout = await layout_init(src);
    if (!layout) throw new Error("can not load: " + src);

    let menu_placeholder = layout_get_placeholder(layout, "menu");
    if (!menu_placeholder) throw new Error("missing 'menu' placeholder in: " + src);

    WEEKPAUSE_MENU.parameters.font = layout_get_attached_value(
        layout, "menu_font_path", LAYOUT_TYPE_STRING, null
    );
    WEEKPAUSE_MENU.parameters.font_size = layout_get_attached_value_as_float(
        layout, "menu_font_size", 24.0
    );
    WEEKPAUSE_MENU.parameters.items_gap = WEEKPAUSE_MENU.parameters.font_size;

    let menumanifest = WEEKPAUSE_MENU;
    if (await fs_file_exists(WEEKPAUSE_MODDING_MENU)) {
        menumanifest = await menumanifest_init(WEEKPAUSE_MODDING_MENU);
        if (!menumanifest) throw new Error("failed to load " + WEEKPAUSE_MODDING_MENU);
    }

    let menu = await menu_init(
        menumanifest,
        menu_placeholder.x, menu_placeholder.y, menu_placeholder.z,
        menu_placeholder.width, menu_placeholder.height
    );

    if (menumanifest != WEEKPAUSE_MENU) menumanifest_destroy(menumanifest);

    let messagebox = await messagebox_init();
    let sprite_nocontroller = sprite_init(await texture_init(WEEKPAUSE_NOCONTROLLER));

    let modding = await modding_init(layout, WEEKPAUSE_MODDING_SCRIPT);
    modding.native_menu = modding.active_menu = menu;
    modding.callback_option = week_pause_internal_handle_modding_option;

    if (exit_to_weekselector_label) {
        let index = menu_index_of_item(menu, "exit-week-selector");
        if (index >= 0) menu_set_item_text(menu, index, exit_to_weekselector_label);
    }

    let weekpause = {
        menu,
        messagebox,
        sprite_nocontroller,
        layout,
        menu_placeholder,
        menu_external: null,
        background_music: null,
        modding,
        modding_choosen_option_name: null
    };

    modding.callback_private_data = weekpause;
    await week_pause_change_background_music(weekpause, "/assets/common/music/breakfast.ogg");

    return weekpause;
}

async function week_pause_destroy(weekpause) {
    modding_destroy(weekpause.modding);
    layout_destroy(weekpause.layout);
    menu_destroy(weekpause.menu);
    messagebox_destroy(weekpause.messagebox);
    sprite_destroy_full(weekpause.sprite_nocontroller);
    if (weekpause.menu_external) menu_destroy(weekpause.menu_external);
    if (weekpause.background_music) soundplayer_destroy(weekpause.background_music);
    weekpause.modding_choosen_option_name = null;// do not dispose
    weekpause = undefined;
}


function week_pause_external_set_text(weekpause, index, text) {
    if (weekpause.menu_external) menu_set_item_text(weekpause.menu_external, index, text);
}

function week_pause_external_set_visible(weekpause, index, visible) {
    if (weekpause.menu_external) menu_set_item_visibility(weekpause.menu_external, index, visible);
}

async function week_pause_external_set_menu(weekpause, menumanifest_src) {
    if (weekpause.menu_external) {
        menu_destroy(weekpause.menu_external);
        weekpause.menu_external = null;
    }

    if (!menumanifest_src || !await fs_file_exists(menumanifest_src)) {
        menu_set_item_visibility(weekpause.menu, 1, false);
        return;
    }

    menu_set_item_visibility(weekpause.menu, 1, true);

    let menu_placeholder = weekpause.menu_placeholder;
    let menumanifest = await menumanifest_init(menumanifest_src);
    weekpause.menu_external = await menu_init(
        menumanifest,
        menu_placeholder.x, menu_placeholder.y, menu_placeholder.z,
        menu_placeholder.width, menu_placeholder.height
    );
    menumanifest_destroy(menumanifest);
}


async function week_pause_change_background_music(weekpause, filename) {
    if (weekpause.background_music) soundplayer_destroy(weekpause.background_music);

    if (filename) {
        weekpause.background_music = await soundplayer_init(filename);
        if (weekpause.background_music) {
            soundplayer_loop_enable(weekpause.background_music, true);
            soundplayer_set_volume(weekpause.background_music, 0.5);
        }
    } else {
        weekpause.background_music = null;
    }
}

async function week_pause_helper_show(weekpause,/**@type {RoundContext} */ roundcontext, dettached_index) {
    let dettached_controller = dettached_index >= 0;
    let controller = gamepad_init(-1);
    gamepad_set_buttons_delay(controller, WEEKPAUSE_DELAY);

    messagebox_hide(weekpause.messagebox, false);
    if (weekpause.background_music) soundplayer_play(weekpause.background_music);
    gamepad_clear_buttons(controller);

    let current_menu_is_external = false;
    let current_menu = weekpause.menu;
    let return_value = 0;

    menu_select_index(weekpause.menu, 0);
    if (weekpause.menu_external) menu_select_index(weekpause.menu_external, 0);
    weekpause.menu_placeholder.vertex = menu_get_drawable(weekpause.menu);

    let textsprite = layout_get_textsprite(weekpause.layout, "stats");
    if (textsprite) {
        const song_index = roundcontext.song_index;
        const songmanifest = roundcontext.initparams.gameplaymanifest.songs[song_index];
        const song_name = songmanifest.name;
        const song_difficult = roundcontext.song_difficult;
        textsprite_set_text_formated(textsprite, "$s\n$s", song_name, song_difficult);
    }

    if (roundcontext.script) await weekscript_notify_pause(roundcontext.script, true);
    while (roundcontext.scriptcontext.halt_flag) await week_pause_internal_render(weekpause, roundcontext);

    layout_trigger_any(weekpause.layout, null);

    if (dettached_controller) {
        dettached_index++;// do not use base-zero index
        messagebox_set_button_single(weekpause.messagebox, "(Waiting controller)");
        messagebox_set_title(weekpause.messagebox, "Controller disconnected");
        messagebox_set_message_formated(weekpause.messagebox, WEEKPAUSE_MSGCONTROLLER, dettached_index);
        messagebox_show_buttons_icons(weekpause.messagebox, false);
        messagebox_use_small_size(weekpause.messagebox, false);
        messagebox_hide_image(weekpause.messagebox, false);
        messagebox_set_image_sprite(weekpause.messagebox, weekpause.sprite_nocontroller);
        messagebox_show(weekpause.messagebox, true);
    }

    weekpause.modding_choosen_option_name = null;
    weekpause.modding.has_exit = false;
    weekpause.modding.has_halt = false;
    weekpause.modding.native_menu = weekpause.modding.active_menu;
    await modding_helper_notify_init(weekpause.modding, MODDING_NATIVE_MENU_SCREEN);
    await await modding_helper_notify_option(weekpause.modding, true);

    while (!weekpause.modding.has_exit) {
        let has_option_choosen = false;
        let go_back = false;
        let elapsed = await week_pause_internal_render(weekpause, roundcontext);
        let buttons = gamepad_has_pressed_delayed(controller, WEEKPAUSE_BUTTONS);

        // if the messagebox is visible, wait decision
        if (return_value != 0) {
            if (buttons & (GAMEPAD_A | GAMEPAD_X)) {
                break;
            } else if (buttons & (GAMEPAD_B | GAMEPAD_START | GAMEPAD_BACK)) {
                return_value = 0;
                if (buttons & (GAMEPAD_B | GAMEPAD_BACK)) {
                    gamepad_set_buttons_delay(controller, WEEKPAUSE_DELAY);
                    gamepad_enforce_buttons_delay(controller);
                    messagebox_hide(weekpause.messagebox, false);
                } else {
                    break;
                }
            }
            continue;
        }

        let res = await modding_helper_handle_custom_menu(weekpause.modding, controller, elapsed);
        if (res != MODDING_HELPER_RESULT_CONTINUE) break;
        if (weekpause.modding.has_halt || weekpause.modding.active_menu != weekpause.menu) continue;
        if (weekpause.modding_choosen_option_name != null) {
            buttons = 0x00;
            has_option_choosen = true;
            break;
        }

        if (dettached_controller) {
            let dettached_count = 0;
            let available = 0;
            for (let i = 0; i < roundcontext.players_size; i++) {
                if (!roundcontext.players[i].controller) continue;
                if (gamepad_is_dettached(roundcontext.players[i].controller)) {
                    if (gamepad_pick(roundcontext.players[i].controller, true)) {
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
                messagebox_hide(weekpause.messagebox, false);
            }
        }

        if (buttons & GAMEPAD_START) {
            if (!current_menu_is_external && !await modding_helper_notify_back(weekpause.modding)) break;
        } else if (buttons & GAMEPAD_AD_UP) {
            if (!menu_select_vertical(current_menu, -1))
                menu_select_index(current_menu, menu_get_items_count(current_menu) - 1);
            await modding_helper_notify_option(weekpause.modding, true);
        } else if (buttons & GAMEPAD_AD_DOWN) {
            if (!menu_select_vertical(current_menu, 1))
                menu_select_index(current_menu, 0);
            await modding_helper_notify_option(weekpause.modding, true);
        } else if (buttons & (GAMEPAD_A | GAMEPAD_X)) {
            has_option_choosen = true;
        } else if (buttons & (GAMEPAD_B | GAMEPAD_BACK)) {
            go_back = true;
        } else if (!has_option_choosen) {
            // nothing to do
            continue;
        }


        if (has_option_choosen && current_menu_is_external) {
            let option_index = menu_get_selected_index(current_menu);
            await weekscript_notify_pause_optionchoosen(roundcontext.script, option_index);
            has_option_choosen = false;
            go_back = true;
        } else if (has_option_choosen) {
            if (weekpause.modding_choosen_option_name == null) {
                weekpause.modding_choosen_option_name = menu_get_selected_item_name(current_menu);
                if (await modding_helper_notify_option(weekpause.modding, false)) {
                    weekpause.modding_choosen_option_name = null;
                    continue;
                }
            }

            return_value = week_pause_internal_return_value(weekpause);

            if (return_value == 0) {
                // resume
                break;
            } else if (return_value == 4) {
                return_value = 0;
                if (!weekpause.menu_external || !roundcontext.script) continue;

                // display week menu
                current_menu = weekpause.menu_external;
                weekpause.modding.callback_option = null;
                await modding_helper_notify_event(weekpause.modding, "week-custom-menu");
                menu_trasition_in(current_menu);
                weekpause.menu_placeholder.vertex = menu_get_drawable(current_menu);
                current_menu_is_external = true;
                if (roundcontext.script) await weekscript_notify_pause_menuvisible(roundcontext.script, true);
            } else if (return_value == 1) {
                // restart song
                break;
            } else if (return_value == 2 || return_value == 3) {
                const msg = return_value == 2 ? WEEKPAUSE_MSGWEEKSELECTOR : WEEKPAUSE_MSGMENU;
                messagebox_hide_image(weekpause.messagebox, true);
                messagebox_set_buttons_icons(weekpause.messagebox, "a", "b");
                messagebox_set_buttons_text(weekpause.messagebox, "Ok", "Cancel");
                messagebox_set_title(weekpause.messagebox, "Confirm");
                messagebox_use_small_size(weekpause.messagebox, true);
                messagebox_set_message(weekpause.messagebox, msg);
                messagebox_show(weekpause.messagebox, false);
                gamepad_set_buttons_delay(controller, WEEKPAUSE_ANTIBOUNCE);
                gamepad_enforce_buttons_delay(controller);
            } else if (return_value == -1) {
                // custom option menu
                weekpause.modding.callback_option = null;
                await modding_helper_notify_handle_custom_option(weekpause.modding, weekpause.modding_choosen_option_name);
                weekpause.modding.callback_option = week_pause_internal_handle_modding_option;
                weekpause.modding_choosen_option_name = null;
                return_value = 0;
            }
        }

        if (go_back && current_menu_is_external) {
            current_menu_is_external = false;
            current_menu = weekpause.menu;
            weekpause.menu_placeholder.vertex = menu_get_drawable(weekpause.menu);
            await weekscript_notify_pause_menuvisible(roundcontext.script, false);
            weekpause.modding.callback_option = week_pause_internal_handle_modding_option;
            await modding_helper_notify_event(weekpause.modding, "week-pause-menu");
            menu_trasition_in(weekpause.menu);
        } else if (go_back && !await modding_helper_notify_back(weekpause.modding)) {
            return_value = 0;
            break;
        }

    }

    if (weekpause.background_music) soundplayer_fade(weekpause.background_music, false, 100.0);

    if (roundcontext.script) await weekscript_notify_pause(roundcontext.script, false);
    while (roundcontext.scriptcontext.halt_flag) await week_pause_internal_render(weekpause, roundcontext);

    if (weekpause.background_music) soundplayer_stop(weekpause.background_music);
    messagebox_hide(weekpause.messagebox, true);

    if (return_value != 0) {
        const target = return_value == 1 ? "transition_fast" : "transition";
        layout_trigger_any(weekpause.layout, target);

        while (true) {
            await week_pause_internal_render(weekpause, roundcontext);
            if (layout_animation_is_completed(weekpause.layout, "transition_effect")) break;
        }
    }

    await modding_helper_notify_exit2(weekpause.modding);
    gamepad_destroy(controller);

    // selected options:
    //      0 -> resume
    //      1 -> restart song
    //      2 -> back to weekselector
    //      3 -> back to mainmenu
    return return_value;
}

function week_pause_get_layout(weekpause) {
    return weekpause.layout;
}


async function week_pause_internal_render(weekpause, roundcontext) {
    const buttons = [0x00];

    let elapsed;

    do {
        elapsed = await pvrctx_wait_ready();
        pvr_context_reset(pvr_context);

        for (let i = 0; i < roundcontext.players_size; i++) {
            let controller = roundcontext.players[i].controller;
            if (controller && gamepad_get_managed_presses(controller, true, buttons) && roundcontext.script) {
                await weekscript_notify_buttons(roundcontext.script, i, buttons[0]);
            }
        }

        if (roundcontext.script) await weekscript_notify_frame(roundcontext.script, elapsed);

        // draw the stage+ui layout but do not animate
        layout_draw(roundcontext.layout, pvr_context);

        layout_animate(weekpause.layout, elapsed);
        layout_draw(weekpause.layout, pvr_context);
    } while (roundcontext.scriptcontext.halt_flag);

    messagebox_animate(weekpause.messagebox, elapsed);
    messagebox_draw(weekpause.messagebox, pvr_context);

    return elapsed;
}

function week_pause_internal_handle_modding_option(weekpause, option_name) {
    if (option_name == null) {
        // resume
        weekpause.modding_choosen_option_name = WEEKPAUSE_MENU.items[0].name;
        menu_select_item(weekpause.menu, weekpause.modding_choosen_option_name);
        return true;
    }

    // select native option
    let index = menumanifest_get_option_index(WEEKPAUSE_MENU, option_name);
    if (index >= 0) {
        weekpause.modding_choosen_option_name = WEEKPAUSE_MENU.items[index].name;
        menu_select_item(weekpause.menu, weekpause.modding_choosen_option_name);
        return true;
    }

    // select custom option
    if (menu_has_item(weekpause.menu, option_name)) {
        menu_select_item(weekpause.menu, option_name);
        weekpause.modding_choosen_option_name = menu_get_selected_item_name(weekpause.menu);
        return true;
    }

    // unknown option
    weekpause.modding_choosen_option_name = null;
    return false;
}

function week_pause_internal_return_value(weekpause) {
    switch (weekpause.modding_choosen_option_name) {
        case null:
        case "resume":
            return 0;
        case "week-menu":
            return 4;
        case "restart-song":
            return 1;
        case "exit-week-selector":
            return 2;
        case "exit-main-menu":
            return 3;
        default:
            // custom option
            return -1;
    }
}

