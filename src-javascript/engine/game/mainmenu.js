"use strict";

const MAINMENU_LAYOUT = "/assets/common/image/main-menu/layout.xml";
const MAINMENU_LAYOUT_DREAMCAST = "/assets/common/image/main-menu/layout~dreamcast.xml";
const MAINMENU_OPTION_SELECTION_DELAY = 200;// milliseconds

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
            hidden: 0
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
            hidden: 0
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
            hidden: 0
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
            hidden: 0
        }
    ],
    items_size: 4
};

const MAINMENU_GAMEPAD_OK = GAMEPAD_A | GAMEPAD_X | GAMEPAD_START;
const MAINMENU_GAMEPAD_CANCEL = GAMEPAD_B | GAMEPAD_Y | GAMEPAD_BACK;
const MAINMENU_GAMEPAD_BUTTONS = MAINMENU_GAMEPAD_OK | MAINMENU_GAMEPAD_CANCEL | GAMEPAD_AD;

async function mainmenu_main() {
    let layout = await layout_init(pvrctx_output_is_widescreen() ? MAINMENU_LAYOUT : MAINMENU_LAYOUT_DREAMCAST);
    let menu_placeholder = layout_get_placeholder(layout, "menu");

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

    let menu = await menu_init(MAINMENU_MENU_MANIFEST, x, y, z, size[0], size[1]);
    menu_trasition_in(menu);
    menu_select_index(menu, 0);

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

    let option_was_selected = 0;
    let selected_index = -1;

    while (1) {
        let selection_offset_x = 0;
        let selection_offset_y = 0;
        let elapsed = await pvrctx_wait_ready();
        let buttons = gamepad_has_pressed_delayed(maple_pad, MAINMENU_GAMEPAD_BUTTONS);

        pvr_context_reset(pvr_context);
        layout_animate(layout, elapsed);
        layout_draw(layout, pvr_context);

        if (buttons & MAINMENU_GAMEPAD_OK) {
            selected_index = menu_get_selected_index(menu);
            if (selected_index >= 0 && selected_index < menu_get_items_count(menu)) {
                option_was_selected = 1;
                break;
            }
        } else if (buttons & MAINMENU_GAMEPAD_CANCEL)
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
        } else {
            if (sound_scroll) soundplayer_stop(sound_scroll);
            if (sound_asterik) soundplayer_replay(sound_asterik);
        }
    }

    // apply choosen transition
    if (sound_scroll) soundplayer_stop(sound_scroll);
    let target_sound = option_was_selected ? sound_confirm : sound_cancel;
    if (target_sound) soundplayer_replay(target_sound);
    layout_trigger_any(layout, option_was_selected ? "option_selected" : "return");
    if (option_was_selected) menu_toggle_choosen(menu, 1);
    else menu_trasition_out(menu);


    let total_elapsed = 0;

    // animate selected option
    if (option_was_selected) {
        while (total_elapsed < 1200) {
            let elapsed = await pvrctx_wait_ready();
            total_elapsed += elapsed;

            layout_animate(layout, elapsed);
            layout_draw(layout, pvr_context);
        }

        total_elapsed = 0;
    }

    // trigger outro transition
    layout_trigger_action(layout, null, "outro");
    menu_trasition_out(menu);

    while (total_elapsed < 1200) {
        let elapsed = await pvrctx_wait_ready();

        layout_animate(layout, elapsed);
        layout_draw(layout, pvr_context);

        if (layout_animation_is_completed(layout, "transition_effect"))
            break;

        total_elapsed += elapsed;
    }

    menu_destroy(menu);
    gamepad_destroy(maple_pad);
    if (sound_confirm) soundplayer_destroy(sound_confirm);
    if (sound_scroll) soundplayer_destroy(sound_scroll);
    if (sound_cancel) soundplayer_destroy(sound_cancel);
    if (sound_asterik) soundplayer_destroy(sound_asterik);
    layout_destroy(layout);

    // if no option was selected, jump to the start screen
    if (!option_was_selected) return 0;

    // handle user action
    return await mainmenu_handle_selected_option(selected_index);
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

        if (gamepad_has_pressed_delayed(gamepad, CREDITS_BUTTONS)) break;
    }

    layout_destroy(layout);
    gamepad_clear_buttons(gamepad);
    gamepad_destroy(gamepad);

    if (pause_background_menu_music) soundplayer_play(background_menu_music);
}

async function mainmenu_handle_selected_option(selected_index) {
    //
    // we handle all menu options here
    // all functions called here are async in JS
    //
    // return 0 to go back to the start-screen, otherwise, reload the main-menu
    //
    switch (selected_index) {
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
