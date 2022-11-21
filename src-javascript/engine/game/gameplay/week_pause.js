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

        anim_transition_in_delay: 0,// unused
        anim_transition_out_delay: 0,// unused

        font: null,
        font_glyph_suffix: "bold",
        font_color_by_difference: 0,// unused
        font_size: 48,
        font_color: 0xFFFFFF,
        font_border_color: 0x00,// unused
        font_border_size: NaN,// unused

        is_sparse: 0,// unused
        is_vertical: 1,
        is_per_page: 0,
        static_index: 1,

        items_align: ALIGN_START,
        items_gap: 48,
        items_dimmen: 0,// unused
        texture_scale: NaN,// unused
        enable_horizontal_text_correction: 1// unused
    },
    items: [
        {
            name: null,
            text: "RESUME",// unused
            placement: { x: 0, y: 0, dimmen: 0, gap: 0 },// unused
            anim_selected: null,// unused
            anim_choosen: null,// unused
            anim_discarded: null,// unused
            anim_idle: null,// unused
            anim_rollback: null,// unused
            anim_in: null,// unused
            anim_out: null,// unused
            hidden: 0
        },
        {
            name: null,
            text: "WEEK MENU",// unused
            placement: { x: 0, y: 0, dimmen: 0, gap: 0 },// unused
            anim_selected: null,// unused
            anim_choosen: null,// unused
            anim_discarded: null,// unused
            anim_idle: null,// unused
            anim_rollback: null,// unused
            anim_in: null,// unused
            anim_out: null,// unused
            hidden: 1
        },
        {
            name: null,
            text: "RESTART SONG",// unused
            placement: { x: 0, y: 0, dimmen: 0, gap: 0 },// unused
            anim_selected: null,// unused
            anim_choosen: null,// unused
            anim_discarded: null,// unused
            anim_idle: null,// unused
            anim_rollback: null,// unused
            anim_in: null,// unused
            anim_out: null,// unused
            hidden: 0
        },
        {
            name: null,
            text: "EXIT TO WEEK SELECTOR",// unused
            placement: { x: 0, y: 0, dimmen: 0, gap: 0 },// unused
            anim_selected: null,// unused
            anim_choosen: null,// unused
            anim_discarded: null,// unused
            anim_idle: null,// unused
            anim_rollback: null,// unused
            anim_in: null,// unused
            anim_out: null,// unused
            hidden: 0
        },
        {
            name: null,
            text: "EXIT TO MAIN MENU",// unused
            placement: { x: 0, y: 0, dimmen: 0, gap: 0 },// unused
            anim_selected: null,// unused
            anim_choosen: null,// unused
            anim_discarded: null,// unused
            anim_idle: null,// unused
            anim_rollback: null,// unused
            anim_in: null,// unused
            anim_out: null,// unused
            hidden: 0
        }
    ],
    items_size: 5
};
const WEEKPAUSE_LAYOUT_WIDESCREEN = "/assets/common/image/week-round/pause.xml";
const WEEKPAUSE_LAYOUT_DREAMCAST = "/assets/common/image/week-round/pause~dreamcast.xml";
const WEEKPAUSE_NOCONTROLLER = "/assets/common/image/week-round/no_controller.png";
const WEEKPAUSE_ANTIBOUNCE = 400;

// messagebox strings
const WEEKPAUSE_MSGCONTROLLER = "The controller $i was discconected, \n" +
    "reconnect it or press START on an unused \n" +
    "controller to replace it.";
const WEEKPAUSE_MSGMENU = "The week progress will be lost, ¿return\n to the main menu?";
const WEEKPAUSE_MSGWEEKSELECTOR = "The week progress will be lost, ¿return\n to the week selector?";


async function week_pause_init() {
    let src = pvrctx_is_widescreen() ? WEEKPAUSE_LAYOUT_WIDESCREEN : WEEKPAUSE_LAYOUT_DREAMCAST;
    let layout = await layout_init(src);
    if (!layout) throw new Error("can not load: " + src);

    let menu_placeholder = layout_get_placeholder(layout, "menu");
    if (!menu_placeholder) throw new Error("missing 'menu' placeholder in: " + src);

    WEEKPAUSE_MENU.parameters.font = layout_get_attached_value(
        layout, "menu_font_path", LAYOUT_TYPE_STRING, null
    );
    WEEKPAUSE_MENU.parameters.font_size = layout_get_attached_value_as_float(
        layout, "menu_font_size", 24
    );
    WEEKPAUSE_MENU.parameters.items_gap = WEEKPAUSE_MENU.parameters.font_size;

    let menu = await menu_init(
        WEEKPAUSE_MENU,
        menu_placeholder.x, menu_placeholder.y, menu_placeholder.z,
        menu_placeholder.width, menu_placeholder.height
    );

    let messagebox = await messagebox_init();
    let sprite_nocontroller = sprite_init(await texture_init(WEEKPAUSE_NOCONTROLLER));

    return {
        menu,
        messagebox,
        sprite_nocontroller,
        layout,
        menu_placeholder,
        menu_external: null,
        background_menu_music: null
    };
}

function week_pause_destroy(weekpause) {
    layout_destroy(weekpause.layout);
    menu_destroy(weekpause.menu);
    messagebox_destroy(weekpause.messagebox);
    sprite_destroy_full(weekpause.sprite_nocontroller);
    if (weekpause.menu_external) menu_destroy(weekpause.menu_external);
    if (weekpause.background_menu_music) soundplayer_destroy(weekpause.background_menu_music);
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
        menu_set_item_visibility(weekpause.menu, 1, 0);
        return;
    }

    menu_set_item_visibility(weekpause.menu, 1, 1);

    let menu_placeholder = weekpause.menu_placeholder;
    let menumanifest = await menumanifest_init(menumanifest_src);
    weekpause.menu_external = await menu_init(
        menumanifest,
        menu_placeholder.x, menu_placeholder.y, menu_placeholder.z,
        menu_placeholder.width, menu_placeholder.height
    );
    menumanifest_destroy(menumanifest);
}

async function week_pause_prepare(weekpause) {
    if (weekpause.background_menu_music) soundplayer_destroy(weekpause.background_menu_music);
    weekpause.background_menu_music = await soundplayer_init("/assets/common/music/breakfast.ogg");
    if (weekpause.background_menu_music) soundplayer_loop_enable(weekpause.background_menu_music, 1);
}

async function week_pause_helper_show(weekpause,/**@type {RoundContext} */ roundcontext, dettached_index) {
    let dettached_controller = dettached_index >= 0;
    let controller = gamepad_init(-1);
    let antibounce = dettached_controller ? 1000 : WEEKPAUSE_ANTIBOUNCE;
    gamepad_set_buttons_delay(controller, WEEKPAUSE_ANTIBOUNCE / 2);

    messagebox_hide(weekpause.messagebox, 0);
    if (weekpause.background_menu_music) {
        soundplayer_set_volume(weekpause.background_menu_music, 0.5);
        soundplayer_replay(weekpause.background_menu_music);
    }
    gamepad_clear_buttons(controller);

    let current_menu_external = 0;
    let current_menu = weekpause.menu;
    let selected_option = 0;

    menu_select_index(weekpause.menu, 0);
    if (weekpause.menu_external) menu_select_index(weekpause.menu_external, 0);
    weekpause.menu_placeholder.vertex = menu_get_drawable(weekpause.menu);

    let textsprite = layout_get_textsprite(weekpause.layout, "stats");
    if (textsprite) {
        const track_index = roundcontext.track_index;
        const trackmanifest = roundcontext.initparams.gameplaymanifest.tracks[track_index];
        const track_name = trackmanifest.name;
        const track_difficult = roundcontext.track_difficult;
        textsprite_set_text_formated(textsprite, "$s\n$s", track_name, track_difficult);
    }

    if (roundcontext.script) await weekscript_notify_pause(roundcontext.script, 1);
    while (roundcontext.scriptcontext.halt_flag) await week_pause_internal_render(roundcontext, weekpause);

    layout_trigger_any(weekpause.layout, null);

    if (dettached_controller) {
        dettached_index++;// do not use base-zero index
        messagebox_set_button_single(weekpause.messagebox, "(Waiting controller)");
        messagebox_set_title(weekpause.messagebox, "Controller disconnected");
        messagebox_set_message_formated(weekpause.messagebox, WEEKPAUSE_MSGCONTROLLER, dettached_index);
        messagebox_show_buttons_icons(weekpause.messagebox, 0);
        messagebox_use_small_size(weekpause.messagebox, 0);
        messagebox_hide_image(weekpause.messagebox, 0);
        messagebox_set_image_sprite(weekpause.messagebox, weekpause.sprite_nocontroller);
        messagebox_show(weekpause.messagebox, 1);
    }

    while (1) {
        let to_external_menu = 0;
        let elapsed = await week_pause_internal_render(roundcontext, weekpause);

        let buttons = gamepad_has_pressed_delayed(
            controller,
            GAMEPAD_START | GAMEPAD_AD_UP | GAMEPAD_AD_DOWN | GAMEPAD_A | GAMEPAD_X | GAMEPAD_B | GAMEPAD_BACK
        );

        if (antibounce > 0) {
            if (buttons)
                antibounce -= elapsed;
            else
                antibounce = 0;
            continue;
        }

        if (selected_option != 0) {
            if (buttons & (GAMEPAD_A | GAMEPAD_X)) {
                break;
            } else if (buttons & (GAMEPAD_B | GAMEPAD_START | GAMEPAD_BACK)) {
                selected_option = 0;
                if (buttons & (GAMEPAD_B | GAMEPAD_BACK)) {
                    antibounce = 200;
                    messagebox_hide(weekpause.messagebox, 0);
                } else {
                    break;
                }
            }
            continue;
        }

        if (dettached_controller) {
            let dettached_count = 0;
            let available = 0;
            for (let i = 0; i < roundcontext.players_size; i++) {
                if (!roundcontext.players[i].controller) continue;
                if (gamepad_is_dettached(roundcontext.players[i].controller)) {
                    if (gamepad_pick(roundcontext.players[i].controller, 1)) {
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
                messagebox_hide(weekpause.messagebox, 0);
            }
        }

        if (buttons & GAMEPAD_START) {
            if (!current_menu_external) break;
        } else if (buttons & GAMEPAD_AD_UP) {
            if (!menu_select_vertical(current_menu, -1))
                menu_select_index(current_menu, menu_get_items_count(current_menu) - 1);
        } else if (buttons & GAMEPAD_AD_DOWN) {
            if (!menu_select_vertical(current_menu, 1))
                menu_select_index(current_menu, 0);
        } else if (buttons & (GAMEPAD_A | GAMEPAD_X)) {
            if (current_menu_external) {
                to_external_menu = 1;
                buttons = gamepad_get_pressed(controller);
            } else {
                let option_index = menu_get_selected_index(current_menu);
                if (option_index == 0) {
                    selected_option = 0;// resume
                    break;
                } else if (option_index == 1 && weekpause.menu_external) {
                    // display week menu
                    menu_trasition_in(weekpause.menu_external);
                    weekpause.menu_placeholder.vertex = menu_get_drawable(weekpause.menu_external);
                    current_menu_external = 1;
                    if (roundcontext.script) await weekscript_notify_pause_menuvisible(roundcontext.script, 1);
                } else if (option_index == 2) {
                    selected_option = 1;// restart song
                    break;
                } else if (option_index == 3) {
                    selected_option = 2;// back to weekselector
                } else if (option_index == 4) {
                    selected_option = 3;// back to mainmenu
                }

                if (selected_option != 0) {
                    let msg = selected_option == 2 ? WEEKPAUSE_MSGWEEKSELECTOR : WEEKPAUSE_MSGMENU;
                    messagebox_hide_image(weekpause.messagebox, 1);
                    messagebox_set_buttons_icons(weekpause.messagebox, "a", "b");
                    messagebox_set_buttons_text(weekpause.messagebox, "Ok", "Cancel");
                    messagebox_set_title(weekpause.messagebox, "Confirm");
                    messagebox_use_small_size(weekpause.messagebox, 1);
                    messagebox_set_message(weekpause.messagebox, msg);
                    messagebox_show(weekpause.messagebox, 0);
                    gamepad_clear_buttons(controller);
                }
            }
        } else if (buttons & (GAMEPAD_B | GAMEPAD_BACK)) {
            if (current_menu_external) {
                current_menu_external = 0;
                weekpause.menu_placeholder.vertex = menu_get_drawable(weekpause.menu);
                if (roundcontext.script) await weekscript_notify_pause_menuvisible(roundcontext.script, 0);
                menu_trasition_in(weekpause.menu);
            } else {
                selected_option = 0;
                break;
            }
        } else if (current_menu_external) {
            buttons = gamepad_get_pressed(controller);
            to_external_menu = buttons;// notify script if has buttons pressed
        }

        if (to_external_menu) {
            let option_index = menu_get_selected_index(weekpause.menu_external);
            let buttons = gamepad_get_pressed(controller);
            if (roundcontext.script) {
                await weekscript_notify_pause_optionselected(roundcontext.script, option_index, buttons);
            }
            while (roundcontext.scriptcontext.halt_flag) await week_pause_internal_render(roundcontext, weekpause);
        }
    }

    if (weekpause.background_menu_music) soundplayer_fade(weekpause.background_menu_music, 0, 100);

    if (roundcontext.script) await weekscript_notify_pause(roundcontext.script, 0);
    while (roundcontext.scriptcontext.halt_flag) await week_pause_internal_render(roundcontext, weekpause);

    if (weekpause.background_menu_music) soundplayer_stop(weekpause.background_menu_music);
    messagebox_hide(weekpause.messagebox, 1);

    if (selected_option == 0) {
        for (let i = 0; i < roundcontext.players_size; i++) {
            if (roundcontext.players[i].controller) gamepad_clear_buttons(roundcontext.players[i].controller);
        }
    } else {
        let target = selected_option == 1 ? "transition_fast" : "transition";
        layout_trigger_any(weekpause.layout, target);

        while (1) {
            let elapsed = await week_pause_internal_render(roundcontext, weekpause);

            if (layout_animation_is_completed(weekpause.layout, "transition_effect")) break;
        }
    }

    // selected options:
    //      0 -> resume
    //      1 -> restart song
    //      2 -> back to weekselector
    //      3 -> back to mainmenu
    return selected_option;
}

async function week_pause_internal_render(roundcontext, weekpause) {
    const buttons = [0x00];

    let elapsed = await pvrctx_wait_ready();
    pvr_context_reset(pvr_context);

    for (let i = 0; i < roundcontext.players_size; i++) {
        let controller = roundcontext.players[i].controller;
        if (controller && gamepad_get_managed_presses(controller, 1, buttons) && roundcontext.script) {
            await weekscript_notify_buttons(roundcontext.script, i, buttons[0]);
        }
    }

    if (roundcontext.script) await weekscript_notify_frame(roundcontext.script, elapsed);

    // draw the stage+ui layout but do not animate
    layout_draw(roundcontext.layout, pvr_context);

    layout_animate(weekpause.layout, elapsed);
    layout_draw(weekpause.layout, pvr_context);

    messagebox_animate(weekpause.messagebox, elapsed);
    messagebox_draw(weekpause.messagebox, pvr_context);

    return elapsed;
}

