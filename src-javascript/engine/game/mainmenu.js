"use strict";

const MAINMENU_LAYOUT = "/assets/common/image/main-menu/layout.xml";
const MAINMENU_LAYOUT_DREAMCAST = "/assets/common/image/main-menu/layout~dreamcast.xml";
const MAINMENU_OPTION_SELECTION_DELAY = 200;// milliseconds
const MAINMENU_MODDING_SCRIPT = "/assets/common/data/scripts/mainmenu.lua";
const MAINMENU_MODDING_MENU = "/assets/common/data/menus/mainmenu.json";
const MAINMENU_BACK_TO_STARTSCREEN = "back-to-startscreen";

const MAINMENU_MENU_MANIFEST = {
    parameters: {
        suffix_selected: "selected",
        suffix_choosen: "choosen",
        suffix_discarded: "discarded",
        suffix_idle: null,// unused
        suffix_rollback: null,// unused
        suffix_in: null,// unused
        suffix_out: null,// unused

        atlas: "/assets/common/image/main-menu/FNF_main_menu_assets.xml",
        animlist: "/assets/common/image/main-menu/animations.xml",

        anim_selected: null,// unused
        anim_choosen: "choosen",// unused
        anim_discarded: null,// unused
        anim_idle: null,// unused
        anim_rollback: null,// unused
        anim_in: null,// unused
        anim_out: null,// unused

        anim_transition_in_delay: 0,
        anim_transition_out_delay: -100,// negative means start delays from the bottom

        font: null,// unused
        font_glyph_suffix: null,// unused
        font_color_by_difference: 0,// unused
        font_size: 0,// unused
        font_color: 0x00,// unused
        font_border_color: 0x00,// unused
        font_border_size: NaN,// unused

        is_sparse: 0,// unused
        is_vertical: 0,// readed from the layout
        is_per_page: 1,

        items_align: ALIGN_CENTER,// readed from the layout
        items_gap: 0,// readed from the layout
        items_dimmen: 0,// readed from the layout
        static_index: 0,// unused
        texture_scale: NaN,// unused
        enable_horizontal_text_correction: 1
    },
    items: [
        {
            name: "storymode",
            text: null,// unused
            placement: { x: 0, y: 0, dimmen: 0, gap: 0 },// unused
            anim_selected: null,// unused
            anim_choosen: null,// unused
            anim_discarded: null,
            anim_idle: null,
            anim_rollback: null,// unused
            anim_in: "draw_even_index",
            anim_out: "no_choosen_even_index",
            hidden: 0,
            description: null// unused
        },
        {
            name: "freeplay",
            text: null,// unused
            placement: { x: 0, y: 0, dimmen: 0, gap: 0 },// unused
            anim_selected: null,// unused
            anim_choosen: null,// unused
            anim_discarded: null,
            anim_idle: null,
            anim_rollback: null,// unused
            anim_in: "draw_odd_index",
            anim_out: "no_choosen_odd_index",
            hidden: 0,
            description: null// unused
        },
        {
            name: "donate",
            text: null,// unused
            placement: { x: 0, y: 0, dimmen: 0, gap: 0 },// unused
            anim_selected: null,// unused
            anim_choosen: null,// unused
            anim_discarded: null,
            anim_idle: null,
            anim_rollback: null,// unused
            anim_in: "draw_even_index",
            anim_out: "no_choosen_even_index",
            hidden: 0,
            description: null// unused
        },
        {
            name: "options",
            text: null,// unused
            placement: { x: 0, y: 0, dimmen: 0, gap: 0 },// unused
            anim_selected: null,// unused
            anim_choosen: null,// unused
            anim_discarded: null,
            anim_idle: null,
            anim_rollback: null,// unused
            anim_in: "draw_odd_index",
            anim_out: "no_choosen_odd_index",
            hidden: 0,
            description: null// unused
        }
    ],
    items_size: 4
};

const MAINMENU_GAMEPAD_OK = GAMEPAD_A | GAMEPAD_X | GAMEPAD_START;
const MAINMENU_GAMEPAD_CANCEL = GAMEPAD_B | GAMEPAD_Y | GAMEPAD_BACK;
const MAINMENU_GAMEPAD_BUTTONS = MAINMENU_GAMEPAD_OK | MAINMENU_GAMEPAD_CANCEL | GAMEPAD_AD;


async function mainmenu_main() {
    let layout = await layout_init(pvrctx_is_widescreen() ? MAINMENU_LAYOUT : MAINMENU_LAYOUT_DREAMCAST);
    if (!layout) {
        console.warn("mainmenu_main() can not load mainmenu layout");
        return await mainmenu_handle_selected_option(0);
    }

    let menu_placeholder = layout_get_placeholder(layout, "menu");
    let delay_after_choose = layout_get_attached_value_as_float(layout, "delay_after_choose", 1200.0);

    // default values
    MAINMENU_MENU_MANIFEST.parameters.items_align = ALIGN_CENTER;
    MAINMENU_MENU_MANIFEST.parameters.is_vertical = 1;
    MAINMENU_MENU_MANIFEST.parameters.items_dimmen = 60;
    MAINMENU_MENU_MANIFEST.parameters.items_gap = 24;
    MAINMENU_MENU_MANIFEST.parameters.texture_scale = 0;

    let x = 0;
    let y = 0;
    let z = 2;
    let size = [pvr_context.screen_width, pvr_context.screen_height];

    layout_get_viewport_size(layout, size);

    if (menu_placeholder) {
        x = menu_placeholder.x;
        y = menu_placeholder.y;
        z = menu_placeholder.z;
        if (menu_placeholder.width > 0) size[0] = menu_placeholder.width;
        if (menu_placeholder.height > 0) size[1] = menu_placeholder.height;

        MAINMENU_MENU_MANIFEST.parameters.is_vertical = layout_get_attached_value(
            layout, "menu_isVertical", LAYOUT_TYPE_BOOLEAN,
            MAINMENU_MENU_MANIFEST.parameters.is_vertical
        );

        if (MAINMENU_MENU_MANIFEST.parameters.is_vertical) {
            if (menu_placeholder.align_horizontal != ALIGN_NONE)
                MAINMENU_MENU_MANIFEST.parameters.items_align = menu_placeholder.align_horizontal;
        } else {
            if (menu_placeholder.align_vertical != ALIGN_NONE)
                MAINMENU_MENU_MANIFEST.parameters.items_align = menu_placeholder.align_vertical;
        }

        MAINMENU_MENU_MANIFEST.parameters.items_dimmen = layout_get_attached_value(
            layout, "menu_itemDimmen", LAYOUT_TYPE_FLOAT,
            MAINMENU_MENU_MANIFEST.parameters.items_dimmen
        );

        MAINMENU_MENU_MANIFEST.parameters.texture_scale = layout_get_attached_value(
            layout, "menu_itemScale", LAYOUT_TYPE_FLOAT,
            MAINMENU_MENU_MANIFEST.parameters.texture_scale
        );

        MAINMENU_MENU_MANIFEST.parameters.items_gap = layout_get_attached_value(
            layout, "menu_itemGap", LAYOUT_TYPE_FLOAT,
            MAINMENU_MENU_MANIFEST.parameters.items_gap
        );
    }

    let menumanifest = MAINMENU_MENU_MANIFEST;
    if (await fs_file_exists(MAINMENU_MODDING_MENU)) {
        menumanifest = await menumanifest_init(MAINMENU_MODDING_MENU);
        if (!menumanifest) throw new Error("failed to load " + MAINMENU_MODDING_MENU);
    }

    let menu = await menu_init(menumanifest, x, y, z, size[0], size[1]);
    menu_trasition_in(menu);
    menu_select_index(menu, 0);
    main_helper_trigger_action_menu2(layout, menumanifest, 0, null, 1, 0);

    if (menu_placeholder) {
        menu_placeholder.vertex = menu_get_drawable(menu);
    } else {
        let index = layout_external_create_group(layout, null, 0);
        layout_external_vertex_create_entries(layout, 1);
        layout_external_vertex_set_entry(layout, 0, VERTEX_DRAWABLE, menu_get_drawable(menu), index);
    }

    // attach camera animtion (if was defined)
    layout_trigger_camera(layout, "camera_animation");

    let sound_confirm = await soundplayer_init("/assets/common/sound/confirmMenu.ogg");
    let sound_scroll = await soundplayer_init("/assets/common/sound/scrollMenu.ogg");
    let sound_asterik = await soundplayer_init("/assets/common/sound/asterikMenu.ogg");
    let sound_cancel = await soundplayer_init("/assets/common/sound/cancelMenu.ogg");

    let maple_pad = gamepad_init(-1);
    gamepad_set_buttons_delay(maple_pad, MAINMENU_OPTION_SELECTION_DELAY);
    gamepad_clear_buttons(maple_pad);

    const moddinghelper = {
        menumanifest: menumanifest,
        choosen_name: null,
        choosen_name_is_allocated: 0
    };

    let modding = await modding_init(layout, MAINMENU_MODDING_SCRIPT);
    modding.native_menu = modding.active_menu = menu;
    modding.callback_private_data = moddinghelper;
    modding.callback_option = mainmenu_handle_modding_option;
    await modding_helper_notify_init(modding, MODDING_NATIVE_MENU_SCREEN);
    await modding_helper_notify_option(modding, 1);

    let last_selected_index = 0;

    while (!modding.has_exit) {
        if (moddinghelper.choosen_name != null) {
            if (moddinghelper.choosen_name == MAINMENU_BACK_TO_STARTSCREEN)
                moddinghelper.choosen_name = null;
            else
                menu_select_item(menu, moddinghelper.choosen_name);
            break;
        }

        let selection_offset_x = 0;
        let selection_offset_y = 0;
        let elapsed = await pvrctx_wait_ready();
        let buttons = gamepad_has_pressed_delayed(maple_pad, MAINMENU_GAMEPAD_BUTTONS);

        pvr_context_reset(pvr_context);

        let res = await modding_helper_handle_custom_menu(modding, maple_pad, elapsed);
        if (res != MODDING_HELPER_RESULT_CONTINUE) break;

        layout_animate(layout, elapsed);
        layout_draw(layout, pvr_context);

        if (modding.has_halt || modding.active_menu != menu) continue;

        if (buttons & MAINMENU_GAMEPAD_OK) {
            if (menu_has_valid_selection(menu)) {
                moddinghelper.choosen_name = menu_get_selected_item_name(menu);
                if (await modding_helper_notify_option(modding, 0)) {
                    moddinghelper.choosen_name = null;
                    continue;
                }
                // Note: the modding script can override the choosen option
                break;
            }
        } else if ((buttons & MAINMENU_GAMEPAD_CANCEL) && !await modding_helper_notify_back(modding))
            break;
        else if (buttons & GAMEPAD_AD_DOWN)
            selection_offset_y++;
        else if (buttons & GAMEPAD_AD_UP)
            selection_offset_y--;
        else if (buttons & GAMEPAD_AD_LEFT)
            selection_offset_x--;
        else if (buttons & GAMEPAD_AD_RIGHT)
            selection_offset_x++;

        if (selection_offset_x == 0 && selection_offset_y == 0) continue;

        let success = 0;

        if (selection_offset_x != 0 && menu_select_horizontal(menu, selection_offset_x)) success = 1;
        if (selection_offset_y != 0 && menu_select_vertical(menu, selection_offset_y)) success = 1;

        if (success > 0) {
            if (sound_asterik) soundplayer_stop(sound_asterik);
            if (sound_scroll) soundplayer_replay(sound_scroll);

            main_helper_trigger_action_menu2(layout, menumanifest, last_selected_index, null, 0, 0);
            last_selected_index = menu_get_selected_index(menu);
            main_helper_trigger_action_menu2(layout, menumanifest, last_selected_index, null, 1, 0);
            await modding_helper_notify_option(modding, 1);
        } else {
            if (sound_scroll) soundplayer_stop(sound_scroll);
            if (sound_asterik) soundplayer_replay(sound_asterik);
        }
    }

    let choosen_option_index = 0;
    let has_choosen_option = moddinghelper.choosen_name != null;

    // apply choosen transition
    if (sound_scroll) soundplayer_stop(sound_scroll);
    let target_sound = has_choosen_option ? sound_confirm : sound_cancel;
    if (target_sound) soundplayer_replay(target_sound);
    layout_trigger_any(layout, has_choosen_option ? "option_selected" : "return");
    if (has_choosen_option) menu_toggle_choosen(menu, 1);
    else menu_trasition_out(menu);

    if (has_choosen_option) {
        main_helper_trigger_action_menu(layout, null, moddinghelper.choosen_name, 0, 1);
    }

    // animate choosen option
    if (has_choosen_option) {
        let total_elapsed = 0;
        while (total_elapsed < delay_after_choose && !modding.has_exit) {
            let elapsed = await pvrctx_wait_ready();
            if (!modding.has_halt) total_elapsed += elapsed;

            layout_animate(layout, elapsed);
            layout_draw(layout, pvr_context);
        }
    }

    // trigger outro transition
    layout_trigger_action(layout, null, "outro");
    menu_trasition_out(menu);

    while (1) {
        let elapsed = await pvrctx_wait_ready();

        layout_animate(layout, elapsed);
        layout_draw(layout, pvr_context);

        if (layout_animation_is_completed(layout, "transition_effect")) {
            // flush framebuffer again with last fade frame
            await pvrctx_wait_ready();
            break;
        }
    }

    if (has_choosen_option) {
        // obtain the native option index
        choosen_option_index = menumanifest_get_option_index(MAINMENU_MENU_MANIFEST, moddinghelper.choosen_name);

        // special case for "story mode"
        if (choosen_option_index < 0 && moddinghelper.choosen_name == "story mode") {
            choosen_option_index = menumanifest_get_option_index(MAINMENU_MENU_MANIFEST, "storymode");
        }

        if (choosen_option_index < 0) {
            modding.callback_option = null;
            await modding_helper_handle_custom_option(modding, moddinghelper.choosen_name);
            if (moddinghelper.choosen_name_is_allocated) moddinghelper.choosen_name = undefined;
        }
    }

    await modding_helper_notify_exit2(modding);

    menu_destroy(menu);
    gamepad_destroy(maple_pad);
    if (sound_confirm) soundplayer_destroy(sound_confirm);
    if (sound_scroll) soundplayer_destroy(sound_scroll);
    if (sound_cancel) soundplayer_destroy(sound_cancel);
    if (sound_asterik) soundplayer_destroy(sound_asterik);
    layout_destroy(layout);
    await modding_destroy(modding);

    if (menumanifest != MAINMENU_MENU_MANIFEST) menumanifest_destroy(menumanifest);

    // if no option was selected, jump to the start screen
    if (!has_choosen_option) return 0;

    // handle user action
    return await mainmenu_handle_selected_option(choosen_option_index);
}

async function mainmenu_show_donate() {
    let layout = await layout_init("/assets/common/credits/donation.xml");
    if (!layout) return;

    let pause_background_menu_music = layout_get_attached_value(
        layout, "pause_background_menu_music", LAYOUT_TYPE_BOOLEAN, false
    );
    let timeout = layout_get_attached_value(
        layout, "timeout", LAYOUT_TYPE_FLOAT, -1
    );
    let donate_url = layout_get_attached_value(
        layout, "donate_url", LAYOUT_TYPE_STRING, null
    );

    if (pause_background_menu_music) soundplayer_pause(background_menu_music);

    let gamepad = gamepad_init(-1);
    gamepad_set_buttons_delay(gamepad, 250);
    gamepad_clear_buttons(gamepad);

    let progress = 0;
    if (timeout < 0) timeout = Infinity;

    while (progress < timeout) {
        let elapsed = await pvrctx_wait_ready();
        pvr_context_reset(pvr_context);

        progress += elapsed;

        layout_animate(layout, elapsed);
        layout_draw(layout, pvr_context);

        let buttons = gamepad_has_pressed_delayed(gamepad, CREDITS_BUTTONS);
        if (buttons) {
            if ((buttons & (GAMEPAD_A | GAMEPAD_START)) && donate_url) {
                window.open(donate_url, "_blank", "noopener,noreferrer");
                continue;
            }
            break;
        }
    }


    layout_destroy(layout);
    gamepad_destroy(gamepad);

    if (pause_background_menu_music) soundplayer_play(background_menu_music);
}

async function mainmenu_handle_selected_option(selected_index) {
    //
    // all main menu options are handled here
    // all functions called here are async in JS
    //
    // return 0 to go back to the start-screen, otherwise, reload the main-menu
    //
    switch (selected_index) {
        case -1:
            // from custom menu
            return true;
        case 0:// storymode
            while (await weekselector_main()) { }
            return 1;// main-menu
        case 1:
            await freeplaymenu_main();
            return 1;// main-menu
        case 2:
            await mainmenu_show_donate();
            return 1;// main-menu
        default:
            console.log("unimplemented selected option:" + selected_index);
            return 1;
    }
}

function mainmenu_handle_modding_option(moddinghelper, option_name) {
    if (moddinghelper.choosen_name_is_allocated) {
        moddinghelper.choosen_name = undefined;
        moddinghelper.choosen_name_is_allocated = 0;
    }

    if (option_name == null || option_name == MAINMENU_BACK_TO_STARTSCREEN) {
        // assume is going back
        moddinghelper.choosen_name = MAINMENU_BACK_TO_STARTSCREEN;
        return 1;
    }

    // check if the option is present on the menu (native or custom)
    let index = menumanifest_get_option_index(moddinghelper.menumanifest, option_name);
    if (index >= 0) {
        moddinghelper.choosen_name = moddinghelper.menumanifest.items[index].name;
        return 1;
    }

    // check if the option is native
    index = menumanifest_get_option_index(MAINMENU_MENU_MANIFEST, option_name);
    if (index >= 0) {
        moddinghelper.choosen_name = MAINMENU_MENU_MANIFEST.items[index].name;
        return 1;
    }

    // unknown option
    moddinghelper.choosen_name = option_name;
    moddinghelper.choosen_name_is_allocated = 1;
    return 0;
}

