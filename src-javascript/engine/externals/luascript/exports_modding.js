"use strict";


function script_modding_unlockdirective_create(L) {
    let luascript = luascript_get_instance(L);
    let modding = luascript.context;

    let name = LUA.luaL_optstring(L, 1, null);
    let value = LUA.luaL_checknumber(L, 2);

    modding_unlockdirective_create(modding, name, value);

    return 0;
}

function script_modding_unlockdirective_remove(L) {
    let luascript = luascript_get_instance(L);
    let modding = luascript.context;

    let name = LUA.luaL_optstring(L, 1, null);

    modding_unlockdirective_remove(modding, name);

    return 0;
}

function script_modding_unlockdirective_get(L) {
    let luascript = luascript_get_instance(L);
    let modding = luascript.context;

    let name = LUA.luaL_optstring(L, 1, null);

    if (!modding_unlockdirective_has(modding, name)) {
        LUA.lua_pushnil(L);
        return 1;
    }

    let ret = modding_unlockdirective_get(modding, name);

    LUA.lua_pushnumber(L, ret);
    return 1;
}

function script_modding_get_layout(L) {
    let luascript = luascript_get_instance(L);
    let modding = luascript.context;


    let ret = modding_get_layout(modding);

    return script_layout_new(L, ret);
}

function script_modding_exit(L) {
    let luascript = luascript_get_instance(L);
    let modding = luascript.context;


    modding_exit(modding);

    return 0;
}

function script_modding_set_halt(L) {
    let luascript = luascript_get_instance(L);
    let modding = luascript.context;

    let halt = LUA.lua_toboolean(L, 1);

    modding_set_halt(modding, halt);

    return 0;
}

function script_modding_get_active_menu(L) {
    let luascript = luascript_get_instance(L);
    let modding = luascript.context;

    let ret = modding_get_active_menu(modding);

    return script_menu_new(L, ret);
}

function script_modding_choose_native_menu_option(L) {
    let luascript = luascript_get_instance(L);
    let modding = luascript.context;

    let name = LUA.luaL_optstring(L, 1, null);

    let ret = modding_choose_native_menu_option(modding, name);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_modding_get_native_menu(L) {
    let luascript = luascript_get_instance(L);
    let modding = luascript.context;

    let ret = modding_get_native_menu(modding);

    return script_menu_new(L, ret);
}

function script_modding_set_active_menu(L) {
    let luascript = luascript_get_instance(L);
    let modding = luascript.context;

    let menu = luascript_read_nullable_userdata(L, 1, MENU);

    modding_set_active_menu(modding, menu);

    return 0;
}

function script_modding_get_native_background_music(L) {
    let luascript = luascript_get_instance(L);
    let modding = luascript.context;

    let ret = modding_get_native_background_music(modding);

    return script_soundplayer_new(L, ret);
}

async function script_modding_replace_native_background_music(L) {
    let luascript = luascript_get_instance(L);
    let modding = luascript.context;

    let music_src = LUA.luaL_optstring(L, 1, null);

    luascript_change_working_folder(L);
    let ret = await modding_replace_native_background_music(modding, music_src);
    luascript_restore_working_folder(L);

    return script_soundplayer_new(L, ret);
}

async function script_modding_spawn_screen(L) {
    let luascript = luascript_get_instance(L);
    let modding = luascript.context;

    let layout_src = LUA.luaL_optstring(L, 1, null);
    let script_src = LUA.luaL_optstring(L, 2, null);

    let arg_value = luascript_parse_and_allocate_modding_value(L, 3, true);

    luascript_change_working_folder(L);
    let ret = await modding_spawn_screen(modding, layout_src, script_src, arg_value);
    luascript_restore_working_folder(L);

    return luascript_push_modding_value(L, true, ret);
}

function script_modding_set_exit_delay(L) {
    let luascript = luascript_get_instance(L);
    let modding = luascript.context;

    let delay_ms = LUA.luaL_checknumber(L, 1);

    modding_set_exit_delay(modding, delay_ms);

    return 0;
}

async function script_modding_get_messagebox(L) {
    let luascript = luascript_get_instance(L);
    let modding = luascript.context;

    let ret = await modding_get_messagebox(modding);

    return script_messagebox_new(L, ret);
}

function script_modding_set_menu_in_layout_placeholder(L) {
    let luascript = luascript_get_instance(L);
    let modding = luascript.context;

    let placeholder_name = LUA.lua_tostring(L, 1);
    let menu = luascript_read_nullable_userdata(L, 2, MENU);

    modding_set_menu_in_layout_placeholder(modding, placeholder_name, menu);

    return 0;
}

function script_modding_storage_get(L) {
    let luascript = luascript_get_instance(L);
    let modding = luascript.context;

    let week_name = LUA.luaL_optstring(L, 1, null);
    let name = LUA.luaL_optstring(L, 2, null);
    const out_data = [null];

    let ret = modding_storage_get(modding, week_name, name, out_data);

    if (!out_data[0])
        LUA.lua_pushnil(L);
    else
        LUA.lua_pushlstring(L, out_data[0], ret);

    return 1;
}

function script_modding_storage_set(L) {
    let luascript = luascript_get_instance(L);
    let modding = luascript.context;

    let week_name = LUA.luaL_optstring(L, 1, null);
    let name = LUA.luaL_optstring(L, 2, null);
    let data;
    let data_size = [0];

    if (LUA.lua_isnil(L, 3)) {
        data = null;
        data_size[0] = 0;
    } else {
        data = LUA.luaL_checklstring(L, 3, data_size);
    }

    let ret = modding_storage_set(modding, week_name, name, data, data_size[0]);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_modding_get_loaded_weeks(L) {
    let luascript = luascript_get_instance(L);
    let modding = luascript.context;

    const weeks_size = [0];
    let weeks = modding_get_loaded_weeks(modding, weeks_size);

    if (!weeks || weeks_size[0] < 1) {
        LUA.lua_createtable(L, 0, 0);
        return 1;
    }

    LUA.lua_createtable(L, weeks_size[0], 0);
    for (let i = 0; i < weeks_size[0]; i++) {
        let songs_count = weeks[i].songs_count;
        luascript_helper_add_field_array_item_as_table(L, 0, 7, i + 1);
        {
            luascript_helper_get_field_string(L, "name", weeks[i].name);
            luascript_helper_get_field_string(L, "display_name", weeks[i].display_name);
            luascript_helper_get_field_string(L, "description", weeks[i].description);
            luascript_helper_get_field_string(L, "custom_folder", weeks[i].custom_folder);
            luascript_helper_get_field_string(L, "unlock_directive", weeks[i].unlock_directive);
            luascript_helper_get_field_string(L, "emit_directive", weeks[i].emit_directive);
            luascript_helper_add_field_table(L, "songs", weeks[i].songs_count, 0);
            {
                for (let j = 0; j < songs_count; j++) {
                    luascript_helper_add_field_array_item_as_table(L, 0, 5, j + 1);
                    {
                        luascript_helper_add_field_boolean(L, "freeplay_only", weeks[i].songs[j].freeplay_only);
                        luascript_helper_get_field_string(L, "name", weeks[i].songs[j].name);
                        luascript_helper_get_field_string(L, "freeplay_unlock_directive", weeks[i].songs[j].freeplay_unlock_directive);
                        luascript_helper_get_field_string(L, "freeplay_song_filename", weeks[i].songs[j].freeplay_song_filename);
                        luascript_helper_get_field_string(L, "freeplay_description", weeks[i].songs[j].freeplay_description);
                    }
                    LUA.lua_settable(L, -3);
                }
            }
            LUA.lua_settable(L, -3);
        }
        LUA.lua_settable(L, -3);
    }

    return 1;
}

async function script_modding_launch_week(L) {
    let luascript = luascript_get_instance(L);
    let modding = luascript.context;

    let week_name = LUA.luaL_checkstring(L, 1);
    let difficult = LUA.luaL_checkstring(L, 2);
    let alt_tracks = LUA.lua_toboolean(L, 3);
    let bf = LUA.luaL_checkstring(L, 4);
    let gf = LUA.luaL_checkstring(L, 5);
    let gameplay_manifest = LUA.luaL_checkstring(L, 6);
    let song_idx = LUA.luaL_optinteger(L, 7, -1);
    let ws_label = LUA.luaL_optstring(L, 8, null);

    let ret = await modding_launch_week(modding, week_name, difficult, alt_tracks, bf, gf, gameplay_manifest, song_idx, ws_label);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

async function script_modding_launch_credits(L) {
    let luascript = luascript_get_instance(L);
    let modding = luascript.context;


    await modding_launch_credits(modding);

    return 0;
}

async function script_modding_launch_startscreen(L) {
    let luascript = luascript_get_instance(L);
    let modding = luascript.context;


    let ret = await modding_launch_startscreen(modding);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

async function script_modding_launch_mainmenu(L) {
    let luascript = luascript_get_instance(L);
    let modding = luascript.context;


    let ret = await modding_launch_mainmenu(modding);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

async function script_modding_launch_settings(L) {
    let luascript = luascript_get_instance(L);
    let modding = luascript.context;


    await modding_launch_settings(modding);

    return 0;
}

async function script_modding_launch_freeplay(L) {
    let luascript = luascript_get_instance(L);
    let modding = luascript.context;


    await modding_launch_freeplay(modding);

    return 0;
}

async function script_modding_launch_weekselector(L) {
    let luascript = luascript_get_instance(L);
    let modding = luascript.context;


    let ret = await modding_launch_weekselector(modding);

    LUA.lua_pushinteger(L, ret);
    return 1;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/**@type {luaL_Reg[]}*/const MODDING_FUNCTIONS = [
    ["modding_get_layout", script_modding_get_layout],
    ["modding_exit", script_modding_exit],
    ["modding_set_halt", script_modding_set_halt],
    ["modding_unlockdirective_create", script_modding_unlockdirective_create],
    ["modding_unlockdirective_remove", script_modding_unlockdirective_remove],
    ["modding_unlockdirective_get", script_modding_unlockdirective_get],
    ["modding_get_active_menu", script_modding_get_active_menu],
    ["modding_choose_native_menu_option", script_modding_choose_native_menu_option],
    ["modding_get_native_menu", script_modding_get_native_menu],
    ["modding_set_active_menu", script_modding_set_active_menu],
    ["modding_get_native_background_music", script_modding_get_native_background_music],
    ["modding_replace_native_background_music", script_modding_replace_native_background_music],
    ["modding_spawn_screen", script_modding_spawn_screen],
    ["modding_set_exit_delay", script_modding_set_exit_delay],
    ["modding_get_messagebox", script_modding_get_messagebox],
    ["modding_set_menu_in_layout_placeholder", script_modding_set_menu_in_layout_placeholder],
    ["modding_storage_set", script_modding_storage_set],
    ["modding_storage_get", script_modding_storage_get],
    ["modding_get_loaded_weeks", script_modding_get_loaded_weeks],
    ["modding_launch_week", script_modding_launch_week],
    ["modding_launch_credits", script_modding_launch_credits],
    ["modding_launch_startscreen", script_modding_launch_startscreen],
    ["modding_launch_mainmenu", script_modding_launch_mainmenu],
    ["modding_launch_settings", script_modding_launch_settings],
    ["modding_launch_freeplay", script_modding_launch_freeplay],
    ["modding_launch_weekselector", script_modding_launch_weekselector],
    [null, null]
];



function script_modding_register(L) {
    for (let i = 0; ; i++) {
        if (MODDING_FUNCTIONS[i][0] == null || MODDING_FUNCTIONS[i][1] == null) break;
        LUA.lua_pushcfunction(L, MODDING_FUNCTIONS[i][1]);
        LUA.lua_setglobal(L, MODDING_FUNCTIONS[i][0]);
    }
}
