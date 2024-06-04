"use strict";

const STREAKCOUNTER = "StreakCounter";


function script_streakcounter_reset(L) {
    let streakcounter = luascript_read_userdata(L, STREAKCOUNTER);

    streakcounter_reset(streakcounter);

    return 0;
}

function script_streakcounter_hide_combo_sprite(L) {
    let streakcounter = luascript_read_userdata(L, STREAKCOUNTER);
    let hide = LUA.lua_toboolean(L, 2);

    streakcounter_hide_combo_sprite(streakcounter, hide);

    return 0;
}

function script_streakcounter_set_combo_draw_location(L) {
    let streakcounter = luascript_read_userdata(L, STREAKCOUNTER);
    let x = LUA.luaL_checknumber(L, 2);
    let y = LUA.luaL_checknumber(L, 3);

    streakcounter_set_combo_draw_location(streakcounter, x, y);

    return 0;
}

function script_streakcounter_state_add(L) {
    let streakcounter = luascript_read_userdata(L, STREAKCOUNTER);
    let combo_modelholder = luascript_read_nullable_userdata(L, 2, MODELHOLDER);
    let number_modelholder = luascript_read_nullable_userdata(L, 3, MODELHOLDER);
    let state_name = LUA.luaL_optstring(L, 4, null);

    let ret = streakcounter_state_add(streakcounter, combo_modelholder, number_modelholder, state_name);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_streakcounter_state_toggle(L) {
    let streakcounter = luascript_read_userdata(L, STREAKCOUNTER);
    let state_name = LUA.luaL_optstring(L, 2, null);

    let ret = streakcounter_state_toggle(streakcounter, state_name);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_streakcounter_set_alpha(L) {
    let streakcounter = luascript_read_userdata(L, STREAKCOUNTER);
    let alpha = LUA.luaL_checknumber(L, 2);

    streakcounter_set_alpha(streakcounter, alpha);

    return 0;
}

function script_streakcounter_get_drawable(L) {
    let streakcounter = luascript_read_userdata(L, STREAKCOUNTER);

    let ret = streakcounter_get_drawable(streakcounter);

    return script_drawable_new(L, ret);
}

function script_streakcounter_animation_set(L) {
    let streakcounter = luascript_read_userdata(L, STREAKCOUNTER);
    let animsprite = luascript_read_nullable_userdata(L, 2, ANIMSPRITE);

    streakcounter_animation_set(streakcounter, animsprite);

    return 0;
}

function script_streakcounter_animation_restart(L) {
    let streakcounter = luascript_read_userdata(L, STREAKCOUNTER);

    streakcounter_animation_restart(streakcounter);

    return 0;
}

function script_streakcounter_animation_end(L) {
    let streakcounter = luascript_read_userdata(L, STREAKCOUNTER);

    streakcounter_animation_end(streakcounter);

    return 0;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/**@type {luaL_Reg[]}*/const STREAKCOUNTER_FUNCTIONS = [
    ["reset", script_streakcounter_reset],
    ["hide_combo_sprite", script_streakcounter_hide_combo_sprite],
    ["set_combo_draw_location", script_streakcounter_set_combo_draw_location],
    ["state_add", script_streakcounter_state_add],
    ["state_toggle", script_streakcounter_state_toggle],
    ["set_alpha", script_streakcounter_set_alpha],
    ["get_drawable", script_streakcounter_get_drawable],
    ["animation_set", script_streakcounter_animation_set],
    ["animation_restart", script_streakcounter_animation_restart],
    ["animation_end", script_streakcounter_animation_end],
    [null, null]
];

function script_streakcounter_new(L, streakcounter) {
    return luascript_userdata_new(L, STREAKCOUNTER, streakcounter);
}

function script_streakcounter_gc(L) {
    return luascript_userdata_gc(L, STREAKCOUNTER);
}

function script_streakcounter_tostring(L) {
    return luascript_userdata_tostring(L, STREAKCOUNTER);
}

function script_streakcounter_register(L) {
    luascript_register(L, STREAKCOUNTER, script_streakcounter_gc, script_streakcounter_tostring, STREAKCOUNTER_FUNCTIONS);
}

