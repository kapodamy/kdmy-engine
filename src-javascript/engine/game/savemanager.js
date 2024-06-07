"use strict";

const SAVEMANAGER_EMPTY = "empty";
const SAVEMANAGER_FUNKINSAVE = "funkinsave";
const SAVEMANAGER_SCAN_INTERVAL = 1000;

const SAVEMANAGER_LAYOUT = "/assets/common/image/save-manager/layout.xml";
const SAVEMANAGER_LAYOUT_DREAMCAST = "/assets/common/image/save-manager/layout~dreamcast.xml";
const SAVEMANAGER_MODDING_SCRIPT = "/assets/common/data/scripts/savemanager.lua";

const SAVEMANAGER_MENU_MANIFEST = {
    parameters: {
        suffix_selected: null,
        suffix_choosen: null,
        suffix_discarded: null,
        suffix_idle: null,// unused
        suffix_rollback: null,// unused
        suffix_in: null,// unused
        suffix_out: null,// unused

        atlas: "/assets/common/image/save-manager/vmu.xml",
        animlist: "/assets/common/image/save-manager/anims.xml",

        anim_selected: "selected",// unused
        anim_choosen: null,// unused
        anim_discarded: null,// unused
        anim_idle: "idle",// unused
        anim_rollback: null,// unused
        anim_in: null,// unused
        anim_out: null,// unused

        anim_transition_in_delay: -50.0,
        anim_transition_out_delay: -50.0,// negative means start delays from the bottom

        font: null,// unused
        font_glyph_suffix: null,// unused
        font_color_by_addition: false,// unused
        font_size: 0,// unused
        font_color: 0x00,// unused
        font_border_color: 0x00,// unused
        font_border_size: NaN,// unused

        is_sparse: false,// unused
        is_vertical: false,
        is_per_page: true,

        items_align: ALIGN_CENTER,// readed from the layout
        items_gap: 40.0,// readed from the layout
        items_dimmen: 80.0,// readed from the layout
        static_index: 0,// unused
        texture_scale: NaN,// unused
        enable_horizontal_text_correction: true// unused
    },
    items: null,
    items_size: 0
};

/**
 * This variable is set to true if the user does not want load and/or save
 * the progress. The {@link savemanager_should_show} function will always return -1 if true
 */
var savemanager_game_withoutsavedata = false;

async function savemanager_init(save_only, error_code) {
    let layout = await layout_init(pvr_context_is_widescreen() ? SAVEMANAGER_LAYOUT : SAVEMANAGER_LAYOUT_DREAMCAST);
    if (!layout) throw new Error("missing savemanager layout");

    let label_height = layout_get_attached_value(layout, "label_height", LAYOUT_TYPE_DOUBLE, 24.0);
    let selected_bg_color = layout_get_attached_value(layout, "selected_background_color", LAYOUT_TYPE_HEX, 0x000000);
    let selected_lbl_color = layout_get_attached_value(layout, "selected_label_color", LAYOUT_TYPE_HEX, 0xFFFFFF);

    let font = layout_get_attached_font(layout, "font");
    let selected_background = sprite_init_from_rgb8(selected_bg_color);
    let selected_label = textsprite_init2(font, label_height, selected_lbl_color);
    let maple_pad = gamepad_init(-1);

    let dimmen = layout_get_attached_value(layout, "menu_itemDimmen", LAYOUT_TYPE_DOUBLE, 80.0);
    let gap = layout_get_attached_value(layout, "menu_itemGap", LAYOUT_TYPE_DOUBLE, 40.0);
    let scale = layout_get_attached_value(layout, "menu_itemScale", LAYOUT_TYPE_DOUBLE, 0.0);
    let padding = dimmen * 0.1;

    gamepad_set_buttons_delay(maple_pad, 200);
    textsprite_set_align(selected_label, ALIGN_NONE, ALIGN_CENTER);
    textsprite_set_max_draw_size(selected_label, dimmen * 1.2, -1.0);

    SAVEMANAGER_MENU_MANIFEST.parameters.items_dimmen = dimmen;
    SAVEMANAGER_MENU_MANIFEST.parameters.items_gap = gap;
    SAVEMANAGER_MENU_MANIFEST.parameters.texture_scale = scale;

    let placeholder = layout_get_placeholder(layout, "menu");
    console.assert(placeholder, "missing 'menu' placeholder");

    // for caching only
    let modelholder = await modelholder_init(SAVEMANAGER_MENU_MANIFEST.parameters.atlas);
    let animlist = await animlist_init(SAVEMANAGER_MENU_MANIFEST.parameters.animlist);

    let messagebox = await messagebox_init();
    messagebox_hide_image(messagebox, true);
    messagebox_use_small_size(messagebox, true);
    messagebox_show_buttons_icons(messagebox, false);

    let button_icons = await modelholder_init(WEEKSELECTOR_BUTTONS_MODEL);
    if (!button_icons) throw new Error("can not load " + WEEKSELECTOR_BUTTONS_MODEL);

    let help_cancel = weekselector_helptext_init(
        button_icons, layout, 2, false, "b", "Continue without save", null
    );
    let help_ok = weekselector_helptext_init(
        button_icons, layout, 4, false, "a", "Choose VMU", null
    );
    modelholder_destroy(button_icons);

    let savemanager = {
        error_code: error_code < 1 ? 0 : error_code,
        save_only,
        drawable_wrapper: null,
        vmu_array: null,
        vmu_size: 0,
        menu: null,
        messagebox,

        modelholder,
        animlist,
        layout,
        placeholder,
        selected_background,
        selected_label,
        maple_pad,
        padding,

        help_cancel,
        help_ok
    };

    placeholder.vertex = savemanager.drawable_wrapper = drawable_init(
        placeholder.z, savemanager, savemanager_internal_draw, savemanager_internal_animate
    );

    layout_trigger_any(layout, save_only ? "save-to" : "load-from");

    layout_external_vertex_create_entries(layout, 2);
    layout_external_vertex_set_entry(
        layout, 0, VERTEX_DRAWABLE, weekselector_helptext_get_drawable(help_ok), 0
    );
    layout_external_vertex_set_entry(
        layout, 1, VERTEX_DRAWABLE, weekselector_helptext_get_drawable(help_cancel), 0
    );

    return savemanager;
}

function savemanager_destroy(savemanager) {
    menu_destroy(savemanager.menu);
    drawable_destroy(savemanager.drawable_wrapper);
    savemanager.vmu_array = undefined;
    layout_destroy(savemanager.layout);
    sprite_destroy(savemanager.selected_background);
    textsprite_destroy(savemanager.selected_label);
    gamepad_destroy(savemanager.maple_pad);
    messagebox_destroy(savemanager.messagebox);
    modelholder_destroy(savemanager.modelholder);
    animlist_destroy(savemanager.animlist);
    weekselector_helptext_destroy(savemanager.help_ok);
    weekselector_helptext_destroy(savemanager.help_cancel);

    savemanager = undefined;
}

async function savemanager_show(savemanager) {
    const location = [0.0, 0.0];
    const size = [0.0, 0.0];

    let selected_index = -1;
    let save_or_load_success = false;
    let next_scan = 0.0;
    let last_saved_selected = false;
    let confirm_leave = false;

    let modding = await modding_init(savemanager.layout, SAVEMANAGER_MODDING_SCRIPT);
    modding.native_menu = modding.active_menu = savemanager.menu;
    modding.callback_private_data = null;
    modding.callback_option = null;
    await modding_helper_notify_init(modding, MODDING_NATIVE_MENU_SCREEN);
    await modding_helper_notify_event(modding, savemanager.save_only ? "do-save" : "do-load");

    while (!modding.has_exit) {
        let selection_offset_x = 0;
        let selection_offset_y = 0;
        let elapsed = await pvrctx_wait_ready();
        let buttons = gamepad_has_pressed_delayed(savemanager.maple_pad, MAINMENU_GAMEPAD_BUTTONS);

        // check for inserted VMUs
        next_scan -= elapsed;
        if (next_scan <= 0.0) {
            let last_vmu_size = savemanager.vmu_size;
            next_scan = SAVEMANAGER_SCAN_INTERVAL;

            if (await savemanager_internal_find_changes(savemanager)) {
                await savemanager_internal_build_list(savemanager);
                menu_select_index(savemanager.menu, -1);
                if (last_saved_selected) {
                    layout_trigger_any(savemanager.layout, "save-not-selected");
                    last_saved_selected = false;
                }

                if (modding.active_menu == modding.native_menu) modding.active_menu = savemanager.menu;
                modding.native_menu = savemanager.menu;
            }

            if ((last_vmu_size > 0) != (savemanager.vmu_size > 0)) {
                if (savemanager.vmu_size > 0)
                    layout_trigger_any(savemanager.layout, "no-detected-hide");
                else
                    layout_trigger_any(savemanager.layout, "no-detected-show");
            }
        }

        if (menu_get_selected_item_rect(savemanager.menu, location, size)) {
            location[0] -= savemanager.padding;
            location[1] -= savemanager.padding;
            size[0] += savemanager.padding * 2.0;
            size[1] += savemanager.padding * 2.0;

            sprite_set_draw_location(savemanager.selected_background, location[0], location[1]);
            sprite_set_draw_size(savemanager.selected_background, size[0], size[1]);
            sprite_set_visible(savemanager.selected_background, true);

            let label_height = textsprite_get_font_size(savemanager.selected_label);
            textsprite_set_draw_location(savemanager.selected_label, location[0], location[1] - label_height);
            textsprite_set_visible(savemanager.selected_label, true);
        } else {
            sprite_set_visible(savemanager.selected_background, false);
            textsprite_set_visible(savemanager.selected_label, false);
        }

        let res = await modding_helper_handle_custom_menu(modding, savemanager.maple_pad, elapsed);
        if (res != MODDING_HELPER_RESULT_CONTINUE) break;
        if (modding.has_halt || modding.active_menu != savemanager.menu) buttons = 0x00;

        pvr_context_reset(pvr_context);
        layout_animate(savemanager.layout, elapsed);
        layout_draw(savemanager.layout, pvr_context);

        if (confirm_leave) {
            messagebox_animate(savemanager.messagebox, elapsed);
            messagebox_draw(savemanager.messagebox, pvr_context);

            if (buttons & MAINMENU_GAMEPAD_OK) {
                savemanager_game_withoutsavedata = true;
                break;
            }
            if (buttons & MAINMENU_GAMEPAD_CANCEL) confirm_leave = false;
            continue;
        }

        if (savemanager.error_code) {
            messagebox_hide_buttons(savemanager.messagebox);
            messagebox_show(savemanager.messagebox, false);

            await savemanager_internal_show_error(savemanager, savemanager.error_code);

            savemanager.error_code = 0;
            continue;
        }

        if (buttons & MAINMENU_GAMEPAD_OK) {
            selected_index = menu_get_selected_index(savemanager.menu);
            if (selected_index >= 0 && selected_index < menu_get_items_count(savemanager.menu)) {
                save_or_load_success = await savemanager_internal_commit(savemanager, selected_index);
                savemanager_game_withoutsavedata = !save_or_load_success;
                if (save_or_load_success) break;
            }
        } else if (buttons & MAINMENU_GAMEPAD_CANCEL && !await modding_helper_notify_back(modding)) {
            confirm_leave = true;
            messagebox_set_buttons_icons(savemanager.messagebox, "a", "b");
            messagebox_set_buttons_text(savemanager.messagebox, "Yes", "No");
            messagebox_set_title(savemanager.messagebox, "Confirm");
            messagebox_set_message(
                savemanager.messagebox, savemanager.save_only ? "¿Leave without saving?" : "¿Continue without load?"
            );
            messagebox_show(savemanager.messagebox, true);
            continue;
        }
        else if (buttons & GAMEPAD_AD_DOWN)
            selection_offset_y++;
        else if (buttons & GAMEPAD_AD_UP)
            selection_offset_y--;
        else if (buttons & GAMEPAD_AD_LEFT)
            selection_offset_x--;
        else if (buttons & GAMEPAD_AD_RIGHT)
            selection_offset_x++;

        if (selection_offset_x == 0 && selection_offset_y == 0) continue;

        let success = false;

        if (selection_offset_x != 0 && menu_select_horizontal(savemanager.menu, selection_offset_x)) success = true;
        if (selection_offset_y != 0 && menu_select_vertical(savemanager.menu, selection_offset_y)) success = true;

        selected_index = menu_get_selected_index(savemanager.menu);
        if ((!success || selected_index < 0) && selected_index >= savemanager.vmu_size) continue;

        const vmu = savemanager.vmu_array[selected_index];

        /*
        // C only
        static char label_text[] = "VMU pu";
        label_text[4] = 0x41 + vmu.port;
        label_text[5] = 0x30 + vmu.unit;
        */

        // JS only
        let port_name = String.fromCodePoint(0x41 + vmu.port);
        let slot_name = String.fromCodePoint(0x30 + vmu.unit);
        const label_text = `VMU ${port_name}${slot_name}`;

        textsprite_set_text_intern(savemanager.selected_label, true, label_text);

        if (last_saved_selected != vmu.has_savedata) {
            last_saved_selected = vmu.has_savedata;
            layout_trigger_any(savemanager.layout, vmu.has_savedata ? "save-selected" : "save-not-selected");
        }
    }

    layout_trigger_any(savemanager.layout, "outro");
    await modding_helper_notify_event(modding, "outro");

    if (save_or_load_success) {
        await pvrctx_wait_ready();
        modding.has_exit = modding.has_halt = false;
        while (!modding.has_exit) {
            let elapsed = await pvrctx_wait_ready();
            pvr_context_reset(pvr_context);

            let res = await modding_helper_handle_custom_menu(modding, savemanager.maple_pad, elapsed);
            if (res != MODDING_HELPER_RESULT_CONTINUE) break;

            layout_animate(savemanager.layout, elapsed);
            layout_draw(savemanager.layout, pvr_context);

            if (modding.has_halt) continue;
            if (layout_animation_is_completed(savemanager.layout, "transition_effect")) {
                // flush framebuffer again with last fade frame
                await pvrctx_wait_ready();
                break;
            }
        }
    }

    await modding_helper_notify_exit2(modding);
    await modding_destroy(modding);
}

async function savemanager_should_show(attempt_to_save_or_load) {
    if (savemanager_game_withoutsavedata) return -1;
    if (funkinsave_is_vmu_missing()) return -1;

    // attempt to automatically load/save
    let result;
    if (attempt_to_save_or_load) {
        result = await funkinsave_write_to_vmu();
    } else {
        result = await funkinsave_read_from_vmu();
        if (result == 1 || result == 2) result = -1;
    }

    savemanager_game_withoutsavedata = result != 0;

    return result;
}

async function savemanager_check_and_save_changes() {
    if (!funkinsave_has_changes) return;

    let ret = await savemanager_should_show(true);
    if (ret == 0) return;

    let savemanager = await savemanager_init(true, ret);
    savemanager_show(savemanager);
    savemanager_destroy(savemanager);
}


async function savemanager_internal_build_list(savemanager) {
    if (savemanager.menu) menu_destroy(savemanager.menu);

    SAVEMANAGER_MENU_MANIFEST.items = new Array(savemanager.vmu_size);
    SAVEMANAGER_MENU_MANIFEST.items_size = savemanager.vmu_size;

    for (let i = 0; i < savemanager.vmu_size; i++) {
        SAVEMANAGER_MENU_MANIFEST.items[i] = {
            name: null,
            text: null,
            placement: { x: 0, y: 0, dimmen: 0, gap: 0 },
            anim_selected: null,
            anim_choosen: null,
            anim_discarded: null,
            anim_idle: null,
            anim_rollback: null,
            anim_in: null,
            anim_out: null,
            hidden: 0,
            description: null,
            texture_scale: 0,
        };

        if (savemanager.vmu_array[i].has_savedata)
            SAVEMANAGER_MENU_MANIFEST.items[i].name = SAVEMANAGER_FUNKINSAVE;
        else
            SAVEMANAGER_MENU_MANIFEST.items[i].name = SAVEMANAGER_EMPTY;
    }

    savemanager.menu = await menu_init(
        SAVEMANAGER_MENU_MANIFEST,
        savemanager.placeholder.x,
        savemanager.placeholder.y,
        savemanager.placeholder.z,
        savemanager.placeholder.width,
        savemanager.placeholder.height
    );

    // note: nothing is allocated inside of items[]
    SAVEMANAGER_MENU_MANIFEST.items = undefined;
    SAVEMANAGER_MENU_MANIFEST.items_size = 0;
}

async function savemanager_internal_find_changes(savemanager) {
    let index = 0;
    let count = 0;

    // count all attached VMUs
    while (true) {
        let dev = maple_enum_type(index++, MAPLE_FUNC_MEMCARD);
        if (!dev || !dev.valid) break;
        count++;
    }

    // build new scan
    let new_vmu_array = new Array(count);
    for (let i = 0; i < count; i++) {
        let dev = maple_enum_type(i, MAPLE_FUNC_MEMCARD);
        if (!dev.valid) continue;

        let found = await funkinsave_has_savedata_in_vmu(dev.port, dev.unit);
        new_vmu_array[i] = { has_savedata: found, port: dev.port, unit: dev.unit };
    }

    // check changes with the previous scan
    let changes = Math.max(count, savemanager.vmu_size);
    for (let i = 0; i < savemanager.vmu_size; i++) {
        let old_vmu = savemanager.vmu_array[i];
        for (let j = 0; j < count; j++) {
            if (
                old_vmu.port == new_vmu_array[j].port &&
                old_vmu.unit == new_vmu_array[j].unit &&
                old_vmu.has_savedata == new_vmu_array[j].has_savedata
            ) {
                changes--;
            }
        }
    }

    if (changes < 1) {
        new_vmu_array = undefined;
        return false;
    }

    // drop old VMU scan
    savemanager.vmu_array = undefined;
    savemanager.vmu_array = new_vmu_array;
    savemanager.vmu_size = count;

    return true;
}

async function savemanager_internal_animate(savemanager, elapsed) {
    let completed = 0;
    completed += sprite_animate(savemanager.selected_background, elapsed);
    if (savemanager.menu) completed += menu_animate(savemanager.menu, elapsed);
    completed += textsprite_animate(savemanager.selected_label, elapsed);

    return completed;
}

async function savemanager_internal_draw(savemanager, pvrctx) {
    if (!savemanager.menu) return;
    if (sprite_is_visible(savemanager.selected_background)) sprite_draw(savemanager.selected_background, pvrctx);
    menu_draw(savemanager.menu, pvrctx);
    if (textsprite_is_visible(savemanager.selected_label)) textsprite_draw(savemanager.selected_label, pvrctx);
}

async function savemanager_internal_commit(savemanager, selected_index) {
    let vmu = savemanager.vmu_array[selected_index];

    if (!savemanager.save_only && !vmu.has_savedata) {
        messagebox_set_title(savemanager.messagebox, textsprite_get_string(savemanager.selected_label));
        messagebox_set_message(savemanager.messagebox, "This vmu is empty ¿Create a new save?");
        messagebox_set_buttons_icons(savemanager.messagebox, "a", "b");
        messagebox_set_buttons_text(savemanager.messagebox, "Yes", "Pick another");
        messagebox_show(savemanager.messagebox, true);

        gamepad_clear_buttons(savemanager.maple_pad);

        while (true) {
            let elapsed = await pvrctx_wait_ready();
            let buttons = gamepad_has_pressed_delayed(savemanager.maple_pad, MAINMENU_GAMEPAD_BUTTONS);

            pvr_context_reset(pvr_context);
            layout_animate(savemanager.layout, elapsed);
            layout_draw(savemanager.layout, pvr_context);
            messagebox_animate(savemanager.messagebox, elapsed);
            messagebox_draw(savemanager.messagebox, pvr_context);

            if (buttons & MAINMENU_GAMEPAD_CANCEL) return false;
            if (buttons & MAINMENU_GAMEPAD_OK) break;
        }
    }

    funkinsave_set_vmu(vmu.port, vmu.unit);
    if (!savemanager.save_only && !vmu.has_savedata) {
        messagebox_set_title(savemanager.messagebox, "Creating new save...");
        savemanager.save_only = true;
    } else {
        messagebox_set_title(savemanager.messagebox, savemanager.save_only ? "Storing save..." : "Loading save...");
    }

    messagebox_hide_buttons(savemanager.messagebox);
    messagebox_use_full_title(savemanager.messagebox, true);
    messagebox_set_message(savemanager.messagebox, null);
    messagebox_show(savemanager.messagebox, false);

    pvr_context_reset(pvr_context);
    layout_draw(savemanager.layout, pvr_context);
    messagebox_draw(savemanager.messagebox, pvr_context);
    await pvrctx_wait_ready();

    messagebox_use_full_title(savemanager.messagebox, false);

    let result;
    if (savemanager.save_only)
        result = await funkinsave_write_to_vmu();
    else
        result = await funkinsave_read_from_vmu();

    return await savemanager_internal_show_error(savemanager, result);
}

async function savemanager_internal_show_error(savemanager, error_code) {
    if (savemanager.save_only) {
        switch (error_code) {
            case 0:
                savemanager_game_withoutsavedata = false;
                return true;
            case 1:
                messagebox_set_message(
                    savemanager.messagebox, "The VMU was removed"
                );
                break;
            case 2:
                messagebox_set_message(
                    savemanager.messagebox, "No enough free space"
                );
                break;
            case 3:
                messagebox_set_message(
                    savemanager.messagebox, "Error while writing the save,\nthe old save could be lost."
                );
                break;
            case 4:
                messagebox_set_message(
                    savemanager.messagebox, "The write could not be completed successfully"
                );
                break;
            case 5:
                messagebox_set_message(
                    savemanager.messagebox, "Can not delete the previous save"
                );
                break;
        }
    } else {
        switch (error_code) {
            case 0:
                // success
                savemanager_game_withoutsavedata = false;
                return true;
            case 1:
                messagebox_set_message(
                    savemanager.messagebox, "The VMU was removed"
                );
                break;
            case 2:
                messagebox_set_message(
                    savemanager.messagebox, "The VMU has changed and the save is missing"
                );
                break;
            case 3:
                messagebox_set_message(
                    savemanager.messagebox, "Error while reading the VMU"
                );
                break;
            case 4:
                messagebox_set_message(
                    savemanager.messagebox, "This save is damaged and can not be loaded"
                );
                break;
            case 5:
                messagebox_set_message(
                    savemanager.messagebox, "The version on the save is different,\nCan not be loaded."
                );
                break;
        }
    }

    messagebox_set_title(savemanager.messagebox, "Error");
    messagebox_set_button_single(savemanager.messagebox, "OK");

    while (true) {
        let elapsed = await pvrctx_wait_ready();
        let buttons = gamepad_has_pressed_delayed(savemanager.maple_pad, MAINMENU_GAMEPAD_BUTTONS);

        pvr_context_reset(pvr_context);
        layout_animate(savemanager.layout, elapsed);
        layout_draw(savemanager.layout, pvr_context);
        messagebox_animate(savemanager.messagebox, elapsed);
        messagebox_draw(savemanager.messagebox, pvr_context);

        if (buttons) break;
    }

    return false;
}

