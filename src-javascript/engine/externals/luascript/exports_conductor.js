"use strict";

const CONDUCTOR = "Conductor";


function script_conductor_init(L) {
    let conductor = conductor_init();
    return luascript_userdata_allocnew(L, CONDUCTOR, conductor);
}

function script_conductor_destroy(L) {
    let conductor = luascript_read_userdata(L, CONDUCTOR);

    if (luascript_userdata_is_allocated(L, CONDUCTOR))
        conductor_destroy(conductor);
    else
        console.error("script_conductor_destroy() object was not allocated by lua\n");

    return 0;
}

function script_conductor_poll_reset(L) {
    let conductor = luascript_read_userdata(L, CONDUCTOR);

    conductor_poll_reset(conductor);

    return 0;
}

function script_conductor_set_character(L) {
    let conductor = luascript_read_userdata(L, CONDUCTOR);
    let character = luascript_read_nullable_userdata(L, 2, CHARACTER);

    conductor_set_character(conductor, character);

    return 0;
}

function script_conductor_use_strum_line(L) {
    let conductor = luascript_read_userdata(L, CONDUCTOR);
    let strum = luascript_read_nullable_userdata(L, 2, STRUM);

    conductor_use_strum_line(conductor, strum);

    return 0;
}

function script_conductor_use_strums(L) {
    let conductor = luascript_read_userdata(L, CONDUCTOR);
    let strums = luascript_read_nullable_userdata(L, 2, STRUMS);

    conductor_use_strums(conductor, strums);

    return 0;
}

function script_conductor_disable_strum_line(L) {
    let conductor = luascript_read_userdata(L, CONDUCTOR);
    let strum = luascript_read_nullable_userdata(L, 2, STRUM);
    let should_disable = LUA.lua_toboolean(L, 3);

    let ret = conductor_disable_strum_line(conductor, strum, should_disable);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_conductor_remove_strum(L) {
    let conductor = luascript_read_userdata(L, CONDUCTOR);
    let strum = luascript_read_nullable_userdata(L, 2, STRUM);

    let ret = conductor_remove_strum(conductor, strum);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_conductor_clear_mapping(L) {
    let conductor = luascript_read_userdata(L, CONDUCTOR);

    conductor_clear_mapping(conductor);

    return 0;
}

function script_conductor_map_strum_to_player_sing_add(L) {
    let conductor = luascript_read_userdata(L, CONDUCTOR);
    let strum = luascript_read_nullable_userdata(L, 2, STRUM);
    let sing_direction_name = LUA.luaL_optstring(L, 3, null);

    conductor_map_strum_to_player_sing_add(conductor, strum, sing_direction_name);

    return 0;
}

function script_conductor_map_strum_to_player_extra_add(L) {
    let conductor = luascript_read_userdata(L, CONDUCTOR);
    let strum = luascript_read_nullable_userdata(L, 2, STRUM);
    let extra_animation_name = LUA.luaL_optstring(L, 3, null);

    conductor_map_strum_to_player_extra_add(conductor, strum, extra_animation_name);

    return 0;
}

function script_conductor_map_strum_to_player_sing_remove(L) {
    let conductor = luascript_read_userdata(L, CONDUCTOR);
    let strum = luascript_read_nullable_userdata(L, 2, STRUM);
    let sing_direction_name = LUA.luaL_optstring(L, 3, null);

    conductor_map_strum_to_player_sing_remove(conductor, strum, sing_direction_name);

    return 0;
}

function script_conductor_map_strum_to_player_extra_remove(L) {
    let conductor = luascript_read_userdata(L, CONDUCTOR);
    let strum = luascript_read_nullable_userdata(L, 2, STRUM);
    let extra_animation_name = LUA.luaL_optstring(L, 3, null);

    conductor_map_strum_to_player_extra_remove(conductor, strum, extra_animation_name);

    return 0;
}

function script_conductor_map_automatically(L) {
    let conductor = luascript_read_userdata(L, CONDUCTOR);
    let should_map_extras = LUA.lua_toboolean(L, 2);

    let ret = conductor_map_automatically(conductor, should_map_extras);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_conductor_poll(L) {
    let conductor = luascript_read_userdata(L, CONDUCTOR);

    conductor_poll(conductor);

    return 0;
}

function script_conductor_disable(L) {
    let conductor = luascript_read_userdata(L, CONDUCTOR);
    let disable = LUA.lua_toboolean(L, 2);

    conductor_disable(conductor, disable);

    return 0;
}

function script_conductor_play_idle(L) {
    let conductor = luascript_read_userdata(L, CONDUCTOR);

    conductor_play_idle(conductor);

    return 0;
}

function script_conductor_play_hey(L) {
    let conductor = luascript_read_userdata(L, CONDUCTOR);

    conductor_play_hey(conductor);

    return 0;
}

function script_conductor_get_character(L) {
    let conductor = luascript_read_userdata(L, CONDUCTOR);

    let character = conductor_get_character(conductor);

    return script_character_new(L, character);
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/**@type {luaL_Reg[]}*/const CONDUCTOR_FUNCTIONS = [
    ["init", script_conductor_init],
    ["destroy", script_conductor_destroy],
    ["poll_reset", script_conductor_poll_reset],
    ["set_character", script_conductor_set_character],
    ["use_strum_line", script_conductor_use_strum_line],
    ["use_strums", script_conductor_use_strums],
    ["disable_strum_line", script_conductor_disable_strum_line],
    ["remove_strum", script_conductor_remove_strum],
    ["clear_mapping", script_conductor_clear_mapping],
    ["map_strum_to_player_sing_add", script_conductor_map_strum_to_player_sing_add],
    ["map_strum_to_player_extra_add", script_conductor_map_strum_to_player_extra_add],
    ["map_strum_to_player_sing_remove", script_conductor_map_strum_to_player_sing_remove],
    ["map_strum_to_player_extra_remove", script_conductor_map_strum_to_player_extra_remove],
    ["map_automatically", script_conductor_map_automatically],
    ["poll", script_conductor_poll],
    ["disable", script_conductor_disable],
    ["play_idle", script_conductor_play_idle],
    ["play_hey", script_conductor_play_hey],
    ["get_character", script_conductor_get_character],
    [null, null]
];


function script_conductor_new(L, conductor) {
    return luascript_userdata_new(L, CONDUCTOR, conductor);
}

function script_conductor_gc(L) {
    return luascript_userdata_destroy(L, CONDUCTOR, conductor_destroy);
}

function script_conductor_tostring(L) {
    return luascript_userdata_tostring(L, CONDUCTOR);
}

function script_conductor_register(L) {
    luascript_register(L, CONDUCTOR, script_conductor_gc, script_conductor_tostring, CONDUCTOR_FUNCTIONS);
}
