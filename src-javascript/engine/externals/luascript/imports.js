"use strict";


/**
 * @param {Luascript} luascript
 * @param {string} function_name
 * @returns {boolean}
 */
function luascript_internal_get_function(luascript, function_name) {
    let lua = luascript.L;

    if (LUA.lua_getglobal(lua, function_name) != LUA.LUA_TFUNCTION) {
        LUA.lua_pop(lua, 1);
        return true;
    }

    return false;
}

/**
 *
 * @param {lua_State} L
 * @param {string} fn_name
 * @param {number} arguments_count
 * @returns {Promise<number>}
 */
async function luascript_internal_lua_imported_fn(L, fn_name, arguments_count) {
    let result = await luascript_pcallk(L, arguments_count, 0);
    if (result == LUA.LUA_OK) return 0;

    let error_message = LUA.lua_tostring(L, -1);
    LUA.lua_pop(L, 1);

    console.error(`lua_imported_fn() call to '${fn_name}' failed.\n`, error_message);

    return 1;
}

/**
 *
 * @param {lua_State} L
 * @param {string} fn_name
 * @param {number} arguments_count
 * @param {ModdingValue[]} output_ret_value
 * @returns {Promise<boolean>}
 */
async function luascript_internal_lua_imported_fn_with_return(L, fn_name, arguments_count, output_ret_value) {
    let result = await luascript_pcallk(L, arguments_count, 1);
    if (result == LUA.LUA_OK) {
        output_ret_value[0] = luascript_parse_and_allocate_modding_value(L, -1, false);
        return false;
    }

    let error_message = LUA.lua_tostring(L, -1);
    LUA.lua_pop(L, 1);

    console.error(`lua_imported_fn() call to '${fn_name}' failed.\n`, error_message);

    output_ret_value[0] = null;
    return true;
}

/**
 * @param {lua_State} L
 * @param {string} fn_name
 * @param {number} arguments_count
 * @param {boolean[]} output_ret
 */
async function luascript_internal_lua_imported_fn_with_return_bool(L, fn_name, arguments_count, output_ret) {
    let result = await luascript_pcallk(L, arguments_count, 1);
    if (result == LUA.LUA_OK) {
        output_ret[0] = LUA.lua_toboolean(L, -1);
        return;
    }

    let error_message = LUA.lua_tostring(L, -1);
    LUA.lua_pop(L, 1);

    console.error(`lua_imported_fn() call to '${fn_name}' failed.\n`, error_message);

    output_ret[0] = false;
}


/**
 * @param {Luascript} luascript
 * @param {number} freeplay_index
 */
async function luascript_notify_weekinit(luascript, freeplay_index) {
    const FUNCTION = "f_weekinit";

    let lua = luascript.L;
    if (luascript_internal_get_function(luascript, FUNCTION)) {
        return;// function not found
    }

    LUA.lua_pushinteger(lua, freeplay_index);

    await luascript_internal_lua_imported_fn(lua, FUNCTION, 1);
}

/**
 * @param {Luascript} luascript
 * @param {boolean} from_retry
 */
async function luascript_notify_beforeready(luascript, from_retry) {
    const FUNCTION = "f_beforeready";

    let lua = luascript.L;
    if (luascript_internal_get_function(luascript, FUNCTION)) {
        return;// function not found
    }

    LUA.lua_pushboolean(lua, from_retry);

    await luascript_internal_lua_imported_fn(lua, FUNCTION, 1);
}

/**
 * @param {Luascript} luascript
 */
async function luascript_notify_ready(luascript) {
    const FUNCTION = "f_ready";

    let lua = luascript.L;
    if (luascript_internal_get_function(luascript, FUNCTION)) {
        return;// function not found
    }
    await luascript_internal_lua_imported_fn(lua, FUNCTION, 0);
}

/**
 * @param {Luascript} luascript
 */
async function luascript_notify_aftercountdown(luascript) {
    const FUNCTION = "f_aftercountdown";

    let lua = luascript.L;
    if (luascript_internal_get_function(luascript, FUNCTION)) {
        return;// function not found
    }
    await luascript_internal_lua_imported_fn(lua, FUNCTION, 0);
}

/**
 * @param {Luascript} luascript
 * @param {number} elapsed
 */
async function luascript_notify_frame(luascript, elapsed) {
    const FUNCTION = "f_frame";

    let lua = luascript.L;
    if (luascript_internal_get_function(luascript, FUNCTION)) {
        return;// function not found
    }

    LUA.lua_pushnumber(lua, elapsed);

    await luascript_internal_lua_imported_fn(lua, FUNCTION, 1);
}

/**
 * @param {Luascript} luascript
 * @param {boolean} loose
 */
async function luascript_notify_roundend(luascript, loose) {
    const FUNCTION = "f_roundend";

    let lua = luascript.L;
    if (luascript_internal_get_function(luascript, FUNCTION)) {
        return;// function not found
    }

    LUA.lua_pushboolean(lua, loose);

    await luascript_internal_lua_imported_fn(lua, FUNCTION, 1);
}

/**
 * @param {Luascript} luascript
 * @param {boolean} giveup
 */
async function luascript_notify_weekend(luascript, giveup) {
    const FUNCTION = "f_weekend";

    let lua = luascript.L;
    if (luascript_internal_get_function(luascript, FUNCTION)) {
        return;// function not found
    }

    LUA.lua_pushboolean(lua, giveup);

    await luascript_internal_lua_imported_fn(lua, FUNCTION, 1);
}

/**
 * @param {Luascript} luascript
 */
async function luascript_notify_gameoverloop(luascript) {
    const FUNCTION = "f_gameoverloop";

    let lua = luascript.L;
    if (luascript_internal_get_function(luascript, FUNCTION)) {
        return;// function not found
    }

    await luascript_internal_lua_imported_fn(lua, FUNCTION, 0);
}

/**
 * @param {Luascript} luascript
 * @param {boolean} retry_or_giveup
 * @param {string} difficult_changed
 */
async function luascript_notify_gameoverdecision(luascript, retry_or_giveup, difficult_changed) {
    const FUNCTION = "f_gameoverdecision";

    let lua = luascript.L;
    if (luascript_internal_get_function(luascript, FUNCTION)) {
        return;// function not found
    }

    LUA.lua_pushboolean(lua, retry_or_giveup);
    LUA.lua_pushstring(lua, difficult_changed);

    await luascript_internal_lua_imported_fn(lua, FUNCTION, 2);
}

/**
 * @param {Luascript} luascript
 */
async function luascript_notify_gameoverended(luascript) {
    const FUNCTION = "f_gameoverended";

    let lua = luascript.L;
    if (luascript_internal_get_function(luascript, FUNCTION)) {
        return;// function not found
    }

    await luascript_internal_lua_imported_fn(lua, FUNCTION, 0);
}

/**
 * @param {Luascript} luascript
 * @param {boolean} pause_or_resume
 */
async function luascript_notify_pause(luascript, pause_or_resume) {
    const FUNCTION = "f_pause";

    let lua = luascript.L;
    if (luascript_internal_get_function(luascript, FUNCTION)) {
        return;// function not found
    }

    LUA.lua_pushboolean(lua, pause_or_resume);

    await luascript_internal_lua_imported_fn(lua, FUNCTION, 1);
}

/**
 * @param {Luascript} luascript
 */
async function luascript_notify_weekleave(luascript) {
    const FUNCTION = "f_weekleave";

    let lua = luascript.L;
    if (luascript_internal_get_function(luascript, FUNCTION)) {
        return;// function not found
    }
    await luascript_internal_lua_imported_fn(lua, FUNCTION, 0);
}

/**
 * @param {Luascript} luascript
 */
async function luascript_notify_beforeresults(luascript) {
    const FUNCTION = "f_beforeresults";

    let lua = luascript.L;
    if (luascript_internal_get_function(luascript, FUNCTION)) {
        return;// function not found
    }

    await luascript_internal_lua_imported_fn(lua, FUNCTION, 0);
}

/**
 * @param {Luascript} luascript
 * @param {number} total_attempts
 * @param {number} songs_count
 * @param {boolean} reject_completed
 */
async function luascript_notify_afterresults(luascript, total_attempts, songs_count, reject_completed) {
    const FUNCTION = "f_afterresults";

    let lua = luascript.L;
    if (luascript_internal_get_function(luascript, FUNCTION)) {
        return;// function not found
    }

    LUA.lua_pushinteger(lua, total_attempts);
    LUA.lua_pushinteger(lua, songs_count);
    LUA.lua_pushboolean(lua, reject_completed);

    await luascript_internal_lua_imported_fn(lua, FUNCTION, 3);
}

/**
 * @param {Luascript} luascript
 */
async function luascript_notify_scriptchange(luascript) {
    const FUNCTION = "f_scriptchange";

    let lua = luascript.L;
    if (luascript_internal_get_function(luascript, FUNCTION)) {
        return;// function not found
    }

    await luascript_internal_lua_imported_fn(lua, FUNCTION, 0);
}

/**
 * @param {Luascript} luascript
 * @param {number} option_index
 */
async function luascript_notify_pause_optionchoosen(luascript, option_index) {
    const FUNCTION = "f_pause_optionchoosen";

    let lua = luascript.L;
    if (luascript_internal_get_function(luascript, FUNCTION)) {
        return;// function not found
    }

    LUA.lua_pushinteger(lua, option_index);

    await luascript_internal_lua_imported_fn(lua, FUNCTION, 1);
}

/**
 *
 * @param {Luascript} luascript
 * @param {boolean} shown_or_hidden
 */
async function luascript_notify_pause_menuvisible(luascript, shown_or_hidden) {
    const FUNCTION = "f_pause_menuvisible";

    let lua = luascript.L;
    if (luascript_internal_get_function(luascript, FUNCTION)) {
        return;// function not found
    }

    LUA.lua_pushboolean(lua, shown_or_hidden);

    await luascript_internal_lua_imported_fn(lua, FUNCTION, 1);
}

/**
 * @param {Luascript} luascript
 * @param {number} timestamp
 * @param {number} id
 * @param {number} duration
 * @param {number} data
 * @param {boolean} special
 * @param {number} player_id
 * @param {number} ranking
 */
async function luascript_notify_note(luascript, timestamp, id, duration, data, special, player_id, ranking) {
    const FUNCTION = "f_note";

    let lua = luascript.L;
    if (luascript_internal_get_function(luascript, FUNCTION)) {
        return;// function not found
    }

    LUA.lua_pushnumber(lua, timestamp);
    LUA.lua_pushinteger(lua, id);
    LUA.lua_pushnumber(lua, duration);
    LUA.lua_pushnumber(lua, data);
    LUA.lua_pushboolean(lua, special);
    LUA.lua_pushinteger(lua, player_id);
    LUA.lua_pushinteger(lua, ranking);

    await luascript_internal_lua_imported_fn(lua, FUNCTION, 7);
}

/**
 * @param {Luascript} luascript
 * @param {number} player_id
 * @param {number} buttons
 */
async function luascript_notify_buttons(luascript, player_id, buttons) {
    const FUNCTION = "f_buttons";

    let lua = luascript.L;
    if (luascript_internal_get_function(luascript, FUNCTION)) {
        return;// function not found
    }

    LUA.lua_pushinteger(lua, player_id);
    LUA.lua_pushinteger(lua, buttons);

    await luascript_internal_lua_imported_fn(lua, FUNCTION, 2);
}

/**
 * @param {Luascript} luascript
 * @param {number} player_id
 * @param {number} timestamp
 * @param {number} id
 * @param {number} duration
 * @param {number} data
 */
async function luascript_notify_unknownnote(luascript, player_id, timestamp, id, duration, data) {
    const FUNCTION = "f_unknownnote";

    let lua = luascript.L;
    if (luascript_internal_get_function(luascript, FUNCTION)) {
        return;// function not found
    }

    LUA.lua_pushinteger(lua, player_id);
    LUA.lua_pushnumber(lua, timestamp);
    LUA.lua_pushinteger(lua, id);
    LUA.lua_pushnumber(lua, duration);
    LUA.lua_pushnumber(lua, data);

    await luascript_internal_lua_imported_fn(lua, FUNCTION, 5);
}

/**
 * @param {Luascript} luascript
 * @param {number} current_beat
 * @param {number} since
 */
async function luascript_notify_beat(luascript, current_beat, since) {
    const FUNCTION = "f_beat";

    let lua = luascript.L;
    if (luascript_internal_get_function(luascript, FUNCTION)) {
        return;// function not found
    }

    LUA.lua_pushinteger(lua, current_beat);
    LUA.lua_pushnumber(lua, since);

    await luascript_internal_lua_imported_fn(lua, FUNCTION, 2);
}

/**
 * @param {Luascript} luascript
 * @param {number} current_quarter
 * @param {number} since
 */
async function luascript_notify_quarter(luascript, current_quarter, since) {
    const FUNCTION = "f_quarter";

    let lua = luascript.L;
    if (luascript_internal_get_function(luascript, FUNCTION)) {
        return;// function not found
    }

    LUA.lua_pushinteger(lua, current_quarter);
    LUA.lua_pushnumber(lua, since);

    await luascript_internal_lua_imported_fn(lua, FUNCTION, 2);
}

/**
 * @param {Luascript} luascript
 * @param {number} kos_timestamp
 */
async function luascript_notify_timer_run(luascript, kos_timestamp) {
    const FUNCTION = "______kdmyEngine_timer_run";

    let lua = luascript.L;
    if (luascript_internal_get_function(luascript, FUNCTION)) {
        return;// function not found
    }

    LUA.lua_pushnumber(lua, kos_timestamp);
    LUA.lua_pushboolean(lua, false);

    await luascript_internal_lua_imported_fn(lua, FUNCTION, 2);
}

/**
 * @param {Luascript} luascript
 * @param {number} song_timestamp
 */
async function luascript_notify_timersong_run(luascript, song_timestamp) {
    const FUNCTION = "______kdmyEngine_timer_run";

    let lua = luascript.L;
    if (luascript_internal_get_function(luascript, FUNCTION)) {
        return;// function not found
    }

    LUA.lua_pushnumber(lua, song_timestamp);
    LUA.lua_pushboolean(lua, true);

    await luascript_internal_lua_imported_fn(lua, FUNCTION, 2);
}

/**
 * @param {Luascript} luascript
 * @param {string} dialog_src
 */
async function luascript_notify_dialogue_builtin_open(luascript, dialog_src) {
    const FUNCTION = "f_dialogue_builtin_open";

    let lua = luascript.L;
    if (luascript_internal_get_function(luascript, FUNCTION)) {
        return;// function not found
    }

    LUA.lua_pushstring(lua, dialog_src);

    await luascript_internal_lua_imported_fn(lua, FUNCTION, 1);
}

/**
 * @param {Luascript} luascript
 * @param {number} line_index
 * @param {string} state_name
 * @param {string} text
 */
async function luascript_notify_dialogue_line_starts(luascript, line_index, state_name, text) {
    const FUNCTION = "f_dialogue_line_starts";

    let lua = luascript.L;
    if (luascript_internal_get_function(luascript, FUNCTION)) {
        return;// function not found
    }

    LUA.lua_pushinteger(lua, line_index);
    LUA.lua_pushstring(lua, state_name);
    LUA.lua_pushstring(lua, text);

    await luascript_internal_lua_imported_fn(lua, FUNCTION, 3);
}

/**
 * @param {Luascript} luascript
 * @param {number} line_index
 * @param {string} state_name
 * @param {string} text
 */
async function luascript_notify_dialogue_line_ends(luascript, line_index, state_name, text) {
    const FUNCTION = "f_dialogue_line_ends";

    let lua = luascript.L;
    if (luascript_internal_get_function(luascript, FUNCTION)) {
        return;// function not found
    }

    LUA.lua_pushinteger(lua, line_index);
    LUA.lua_pushstring(lua, state_name);
    LUA.lua_pushstring(lua, text);

    await luascript_internal_lua_imported_fn(lua, FUNCTION, 3);
}

/**
 * @param {Luascript} luascript
 */
async function luascript_notify_after_strum_scroll(luascript) {
    const FUNCTION = "f_after_strum_scroll";

    let lua = luascript.L;
    if (luascript_internal_get_function(luascript, FUNCTION)) {
        return;// function not found
    }

    await luascript_internal_lua_imported_fn(lua, FUNCTION, 0);
}

/**
 * @param {Luascript} luascript
 * @param {object} menu
 * @param {number} index
 * @param {string} name
 */
async function luascript_notify_modding_menu_option_selected(luascript, menu, index, name) {
    const FUNCTION = "f_modding_menu_option_selected";

    let lua = luascript.L;
    if (luascript_internal_get_function(luascript, FUNCTION)) {
        return;// function not found
    }

    luascript_create_userdata(luascript, menu, MENU, false);
    LUA.lua_pushinteger(lua, index);
    LUA.lua_pushstring(lua, name);

    await luascript_internal_lua_imported_fn(lua, FUNCTION, 3);
}

/**
 * @param {Luascript} luascript
 * @param {object} menu
 * @param {number} index
 * @param {string} name
 * @returns {Promise<boolean>}
 */
async function luascript_notify_modding_menu_option_choosen(luascript, menu, index, name) {
    const FUNCTION = "f_modding_menu_option_choosen";

    let lua = luascript.L;
    if (luascript_internal_get_function(luascript, FUNCTION)) {
        return false;// function not found
    }

    luascript_create_userdata(luascript, menu, MENU, false);
    LUA.lua_pushinteger(lua, index);
    LUA.lua_pushstring(lua, name);

    const ret = [false];
    await luascript_internal_lua_imported_fn_with_return_bool(lua, FUNCTION, 3, ret);

    return ret[0];
}

/**
 * @param {Luascript} luascript
 * @returns {Promise<boolean>}
 */
async function luascript_notify_modding_back(luascript) {
    const FUNCTION = "f_modding_back";

    let lua = luascript.L;
    if (luascript_internal_get_function(luascript, FUNCTION)) {
        return false;// function not found
    }

    const ret = [false];
    await luascript_internal_lua_imported_fn_with_return_bool(lua, FUNCTION, 0, ret);

    return ret[0];
}

/**
 * @param {Luascript} luascript
 * @returns {Promise<ModdingValue>}
 */
async function luascript_notify_modding_exit(luascript) {
    const FUNCTION = "f_modding_exit";

    let lua = luascript.L;
    if (luascript_internal_get_function(luascript, FUNCTION)) {
        return null;// function not found
    }

    const ret_value = [null];

    if (await luascript_internal_lua_imported_fn_with_return(lua, FUNCTION, 0, ret_value)) {
        // call failed
        return null;
    }

    return ret_value[0];
}

/**
 * @param {Luascript} luascript
 * @param {ModdingValue} arg_value
 */
async function luascript_notify_modding_init(luascript, arg_value) {
    const FUNCTION = "f_modding_init";

    let lua = luascript.L;
    if (luascript_internal_get_function(luascript, FUNCTION)) {
        return;// function not found
    }

    luascript_push_modding_value(lua, false, arg_value);

    await luascript_internal_lua_imported_fn(lua, FUNCTION, 1);
}

/**
 * @param {Luascript} luascript
 * @param {string} function_name
 */
async function luascript_call_function(luascript, function_name) {
    if (!function_name) return;

    let lua = luascript.L;
    if (luascript_internal_get_function(luascript, function_name)) {
        return;// function not found
    }

    await luascript_internal_lua_imported_fn(lua, function_name, 0);
}

/**
 * @param {Luascript} luascript
 * @param {string} event_name
 */
async function luascript_notify_modding_event(luascript, event_name) {
    const FUNCTION = "f_modding_event";

    let lua = luascript.L;
    if (luascript_internal_get_function(luascript, FUNCTION)) {
        return;// function not found
    }

    LUA.lua_pushstring(lua, event_name);

    await luascript_internal_lua_imported_fn(lua, FUNCTION, 1);
}

/**
 * @param {Luascript} luascript
 * @param {string} option_name
 */
async function luascript_notify_modding_handle_custom_option(luascript, option_name) {
    const FUNCTION = "f_modding_handle_custom_option";

    let lua = luascript.L;
    if (luascript_internal_get_function(luascript, FUNCTION)) {
        return;// function not found
    }

    LUA.lua_pushstring(lua, option_name);

    await luascript_internal_lua_imported_fn(lua, FUNCTION, 1);
}

/**
 * @param {Luascript} luascript
 * @param {boolean} focused
 */
async function luascript_notify_modding_window_focus(luascript, focused) {
    if (luascript.is_week) return;

    const FUNCTION = "f_modding_window_focus";

    let lua = luascript.L;
    if (luascript_internal_get_function(luascript, FUNCTION)) {
        return;// function not found
    }

    LUA.lua_pushboolean(lua, focused);

    await luascript_internal_lua_imported_fn(lua, FUNCTION, 1);
}

/**
 * @param {Luascript} luascript
 * @param {boolean} minimized
 */
async function luascript_notify_modding_window_minimized(luascript, minimized) {
    if (luascript.is_week) return;

    const FUNCTION = "f_modding_window_minimized";

    let lua = luascript.L;
    if (luascript_internal_get_function(luascript, FUNCTION)) {
        return;// function not found
    }

    LUA.lua_pushboolean(lua, minimized);

    await luascript_internal_lua_imported_fn(lua, FUNCTION, 1);
}

/**
 * @param {Luascript} luascript
 * @param {number} screen_width
 * @param {number} screen_height
 */
async function luascript_notify_window_size_changed(luascript, screen_width, screen_height) {
    const FUNCTION = "f_window_size_changed";

    let lua = luascript.L;
    if (luascript_internal_get_function(luascript, FUNCTION)) {
        return;// function not found
    }

    LUA.lua_pushinteger(lua, screen_width);
    LUA.lua_pushinteger(lua, screen_height);

    await luascript_internal_lua_imported_fn(lua, FUNCTION, 2);
}

/**
 * @param {Luascript} luascript
 * @param {number} key
 * @param {number} scancode
 * @param {boolean} is_pressed
 * @param {number} mods
 */
async function luascript_notify_input_keyboard(luascript, key, scancode, is_pressed, mods) {
    const FUNCTION = "f_input_keyboard";

    let lua = luascript.L;
    if (luascript_internal_get_function(luascript, FUNCTION)) {
        return;// function not found
    }

    LUA.lua_pushinteger(lua, key);
    LUA.lua_pushinteger(lua, scancode);
    LUA.lua_pushboolean(lua, is_pressed);
    LUA.lua_pushinteger(lua, mods);

    await luascript_internal_lua_imported_fn(lua, FUNCTION, 4);
}

/**
 * @param {Luascript} luascript
 * @param {number} x
 * @param {number} y
 */
async function luascript_notify_input_mouse_position(luascript, x, y) {
    const FUNCTION = "f_input_mouse_position";

    let lua = luascript.L;
    if (luascript_internal_get_function(luascript, FUNCTION)) {
        return;// function not found
    }

    LUA.lua_pushnumber(lua, x);
    LUA.lua_pushnumber(lua, y);

    await luascript_internal_lua_imported_fn(lua, FUNCTION, 2);
}

/**
 * @param {Luascript} luascript
 * @param {boolean} entering
 */
async function luascript_notify_input_mouse_enter(luascript, entering) {
    const FUNCTION = "f_input_mouse_enter";

    let lua = luascript.L;
    if (luascript_internal_get_function(luascript, FUNCTION)) {
        return;// function not found
    }

    LUA.lua_pushboolean(lua, entering);

    await luascript_internal_lua_imported_fn(lua, FUNCTION, 1);
}

/**
 * @param {Luascript} luascript
 * @param {number} button
 * @param {boolean} is_pressed
 * @param {number} mods
 */
async function luascript_notify_input_mouse_button(luascript, button, is_pressed, mods) {
    const FUNCTION = "f_input_mouse_button";

    let lua = luascript.L;
    if (luascript_internal_get_function(luascript, FUNCTION)) {
        return;// function not found
    }

    LUA.lua_pushinteger(lua, button);
    LUA.lua_pushboolean(lua, is_pressed);
    LUA.lua_pushinteger(lua, mods);

    await luascript_internal_lua_imported_fn(lua, FUNCTION, 3);
}

/**
 * @param {Luascript} luascript
 * @param {number} x
 * @param {number} y
 */
async function luascript_notify_input_mouse_scroll(luascript, x, y) {
    const FUNCTION = "f_input_mouse_scroll";

    let lua = luascript.L;
    if (luascript_internal_get_function(luascript, FUNCTION)) {
        return;// function not found
    }

    LUA.lua_pushnumber(lua, x);
    LUA.lua_pushnumber(lua, y);

    await luascript_internal_lua_imported_fn(lua, FUNCTION, 2);
}

