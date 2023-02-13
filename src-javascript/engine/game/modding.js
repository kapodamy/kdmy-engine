"use strict";

const MODDING_HELPER_RESULT_CONTINUE = 0;
const MODDING_HELPER_RESULT_BACK = 1;
const MODDING_HELPER_RESULT_CHOOSEN = 2;

const MODDING_VALUE_TYPE_NULL = 0;
const MODDING_VALUE_TYPE_STRING = 1;
const MODDING_VALUE_TYPE_BOOLEAN = 2;
const MODDING_VALUE_TYPE_DOUBLE = 3;

const MODDING_NATIVE_MENU_SCREEN = "NATIVE_SCREEN";

async function modding_init(layout, src_script) {
    let modding = {
        script: null,
        layout,
        has_exit: 0,
        has_halt: 0,
        has_funkinsave_changes: 0,

        native_menu: null,
        active_menu: null,
        callback_option: null,
        callback_private_data: null,
        exit_delay_ms: 0.0,
        custom_menu_gamepad_delay: 200.0,
        custom_menu_active_gamepad_delay: 0.0,
        last_pressed: 0x00,
        messagebox: null
    };

    if (src_script && await fs_file_exists(src_script))
        modding.script = await weekscript_init(src_script, modding, 0);

    return modding;
}

async function modding_destroy(modding) {
    if (modding.has_funkinsave_changes) funkinsave_write_to_vmu();
    if (modding.script != null) weekscript_destroy(modding.script);
    if (modding.messagebox) messagebox_destroy(modding.messagebox);
    modding = undefined;
}

function modding_get_layout(modding) {
    return modding.layout;
}

function modding_exit(modding) {
    modding.has_exit = 1;
}

function modding_set_halt(modding, halt) {
    modding.has_halt = !!halt;
}


function modding_unlockdirective_create(modding, name, value) {
    modding.has_funkinsave_changes = 1;
    funkinsave_create_unlock_directive(name, value);
}

function modding_unlockdirective_has(modding, name) {
    return funkinsave_contains_unlock_directive(name);
}

function modding_unlockdirective_get(modding, name) {
    let value = [0.0];
    funkinsave_read_unlock_directive(name, value);
    return value[0];
}

function modding_unlockdirective_remove(modding, name) {
    modding.has_funkinsave_changes = 1;
    funkinsave_delete_unlock_directive(name);
}


function modding_storage_set(modding, week_name, name, data, data_size) {
    let ret = funkinsave_storge_set(week_name, name, data, data_size);
    if (ret) modding.has_funkinsave_changes = true;
    return ret;
}

function modding_storage_get(modding, week_name, name, out_data) {
    return funkinsave_storge_get(week_name, name, out_data);
}


function modding_choose_native_menu_option(modding, name) {
    if (!modding.callback_option) return 0;
    return modding.callback_option(modding.callback_private_data, name);
}

function modding_set_active_menu(modding, menu) {
    modding.active_menu = menu;
}

function modding_get_active_menu(modding) {
    return modding.active_menu;
}

function modding_get_native_menu(modding) {
    return modding.native_menu;
}

async function modding_spawn_screen(modding, layout_src, script_src, arg) {
    let screenmenu = await screenmenu_init(layout_src, script_src);
    if (!screenmenu) return null;

    let ret = await screenmenu_display(screenmenu, pvr_context, arg);
    await screenmenu_destroy(screenmenu);

    return ret;
}

function modding_get_native_background_music(modding) {
    return background_menu_music;
}

async function modding_replace_native_background_music(modding, music_src) {
    if (background_menu_music != null) {
        soundplayer_destroy(background_menu_music);
        background_menu_music = null;
    }

    if (music_src) {
        background_menu_music = await songplayer_init(music_src);
    }

    return background_menu_music;
}

function modding_set_exit_delay(modding, delay_ms) {
    modding.exit_delay_ms = delay_ms;
}

async function modding_get_messagebox(modding) {
    if (!modding.messagebox) modding.messagebox = await messagebox_init();
    return modding.messagebox;
}


async function modding_helper_handle_custom_menu(modding, gamepad, elapsed) {
    const script = modding.script != null ? weekscript_get_luascript(modding.script) : null;
    let pressed = gamepad_get_pressed(gamepad);

    if (script != null) {
        if (modding.last_pressed != pressed) {
            modding.last_pressed = pressed;
            await weekscript_notify_buttons(modding.script, -1, pressed);
        }
        await weekscript_notify_frame(modding.script, elapsed);
    }

    const menu = modding.active_menu;
    if (modding.has_halt || !menu || menu == modding.native_menu) return MODDING_HELPER_RESULT_CONTINUE;

    if (modding.custom_menu_active_gamepad_delay > 0.0) {
        modding.custom_menu_active_gamepad_delay -= elapsed;
        if (modding.custom_menu_active_gamepad_delay > 0.0) return MODDING_HELPER_RESULT_CONTINUE;
    }

    let go_back = 0;
    let has_selected = 0;
    let has_choosen = false;

    if (pressed & MAINMENU_GAMEPAD_CANCEL)
        go_back = 1;
    else if (pressed & GAMEPAD_DALL_LEFT)
        has_selected = menu_select_horizontal(menu, -1);
    else if (pressed & GAMEPAD_DALL_RIGHT)
        has_selected = menu_select_horizontal(menu, 1);
    else if (pressed & GAMEPAD_DALL_UP)
        has_selected = menu_select_vertical(menu, -1);
    else if (pressed & GAMEPAD_DALL_DOWN)
        has_selected = menu_select_vertical(menu, 1);
    else if (pressed & MAINMENU_GAMEPAD_OK)
        has_choosen = menu_get_selected_index(menu) >= 0;


    if (go_back) {
        if (script == null) return MODDING_HELPER_RESULT_BACK;
        if (!await luascript_notify_modding_back(script)) return MODDING_HELPER_RESULT_BACK;
        modding.custom_menu_active_gamepad_delay = modding.custom_menu_gamepad_delay;
    }

    if (has_selected) {
        if (script == null) {
            let index = menu_get_selected_index(menu);
            let name = menu_get_selected_item_name(menu);
            await luascript_notify_modding_menu_option_selected(script, menu, index, name);
        }
        modding.custom_menu_active_gamepad_delay = modding.custom_menu_gamepad_delay;
    }

    if (has_choosen) {
        if (script == null) return MODDING_HELPER_RESULT_CHOOSEN;

        let index = menu_get_selected_index(menu);
        let name = menu_get_selected_item_name(menu);

        if (!await luascript_notify_modding_menu_option_choosen(script, menu, index, name)) {
            menu_toggle_choosen(menu, 1);
            return MODDING_HELPER_RESULT_CHOOSEN;
        }

        modding.custom_menu_active_gamepad_delay = modding.custom_menu_gamepad_delay;
    }

    return MODDING_HELPER_RESULT_CONTINUE;
}


async function modding_helper_notify_option(modding, selected_or_choosen) {
    if (!modding.active_menu || modding.script == null) return 0;

    let index = menu_get_selected_index(modding.active_menu);
    let name = menu_get_selected_item_name(modding.active_menu);
    let script = weekscript_get_luascript(modding.script);

    if (selected_or_choosen) {
        await luascript_notify_modding_menu_option_selected(script, modding.active_menu, index, name);
        return 0;
    }

    return await luascript_notify_modding_menu_option_choosen(script, modding.active_menu, index, name);
}

async function modding_helper_notify_option2(modding, selected_or_choosen, menu, index, name) {
    if (modding.script == null) return 0;

    let script = weekscript_get_luascript(modding.script);

    if (selected_or_choosen) {
        await luascript_notify_modding_menu_option_selected(script, menu, index, name);
        return 0;
    }

    return await luascript_notify_modding_menu_option_choosen(script, menu, index, name);
}

async function modding_helper_notify_frame(modding, elapsed, song_timestamp) {
    if (modding.script == null) return;
    if (song_timestamp >= 0.0) await weekscript_notify_timersong(modding.script, song_timestamp);
    await weekscript_notify_frame(modding.script, elapsed);
}

async function modding_helper_notify_back(modding) {
    if (modding.script == null) return false;
    return await luascript_notify_modding_back(weekscript_get_luascript(modding.script));
}

async function modding_helper_notify_exit(modding) {
    if (modding.script == null) return null;
    return await luascript_notify_modding_exit(weekscript_get_luascript(modding.script));
}

async function modding_helper_notify_exit2(modding) {
    let ret = modding_helper_notify_exit(modding);
    if (ret) ret = undefined;
}

async function modding_helper_notify_init(modding, arg) {
    if (modding.script == null) return;
    await luascript_notify_modding_init(weekscript_get_luascript(modding.script), arg);
}

async function modding_helper_notify_event(modding, event_name) {
    if (modding.script == null) return;
    await luascript_notify_modding_event(weekscript_get_luascript(modding.script), event_name);
}

async function modding_helper_handle_custom_option(modding, option_name) {
    if (modding.script == null) return;
    await luascript_notify_modding_handle_custom_option(weekscript_get_luascript(modding.script), option_name);
}

function modding_set_menu_in_layout_placeholder(modding, placeholder_name, menu) {
    if (!modding.layout) return;
    const placeholder = layout_get_placeholder(modding.layout, placeholder_name);
    if (!placeholder) return;
    placeholder.vertex = menu ? menu_get_drawable(menu) : null;
}


function modding_get_loaded_weeks(modding, out_size) {
    out_size[0] = weeks_array.size;
    return weeks_array.array;
}

async function modding_launch_week(modding, week_name, difficult, alt_tracks, bf, gf, gameplay_manifest, track_idx) {
    let gameplay_weekinfo = null;

    for (let i = 0; i < weeks_array.size; i++) {
        if (weeks_array.array[i].name == week_name) {
            gameplay_weekinfo = weeks_array.array[i];
        }
    }

    if (gameplay_weekinfo == null) return -1;

    funkinsave_set_last_played(gameplay_weekinfo.name, difficult);

    if (!bf) bf = await freeplaymenu_helper_get_default_character_manifest(true);
    if (!gf) gf = await freeplaymenu_helper_get_default_character_manifest(false);


    let week_result = await week_main(
        gameplay_weekinfo,
        alt_tracks,
        difficult,
        bf,
        gf,
        gameplay_manifest,
        track_idx
    );

    return week_result;
}

async function modding_launch_credits(modding) {
    return await credits_main();
}

async function modding_launch_startscreen(modding) {
    return await startscreen_main();
}

async function modding_launch_mainmenu(modding) {
    return await mainmenu_main();
}

async function modding_launch_settings(modding) {
    console.info("modding_launch_settingsmenu() not implemented: settingsmenu_main()");
}

async function modding_launch_freeplay(modding) {
    return await freeplaymenu_main();
}

async function modding_launch_weekselector(modding) {
    return await weekselector_main();
}
