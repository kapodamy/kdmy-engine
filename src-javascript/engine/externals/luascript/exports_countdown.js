"use strict";

const COUNTDOWN = "Countdown";


function script_countdown_set_default_animation2(L) {
    let countdown = luascript_read_userdata(L, COUNTDOWN);
    let tweenkeyframe = luascript_read_nullable_userdata(L, 2, TWEENKEYFRAME);

    countdown_set_default_animation2(countdown, tweenkeyframe);

    return 0;
}

function script_countdown_set_bpm(L) {
    let countdown = luascript_read_userdata(L, COUNTDOWN);
    let bpm = LUA.luaL_checknumber(L, 2);

    countdown_set_bpm(countdown, bpm);

    return 0;
}

function script_countdown_get_drawable(L) {
    let countdown = luascript_read_userdata(L, COUNTDOWN);

    let ret = countdown_get_drawable(countdown);

    return script_drawable_new(L, ret);
}

function script_countdown_ready(L) {
    let countdown = luascript_read_userdata(L, COUNTDOWN);

    let ret = countdown_ready(countdown);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_countdown_start(L) {
    let countdown = luascript_read_userdata(L, COUNTDOWN);

    let ret = countdown_start(countdown);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_countdown_has_ended(L) {
    let countdown = luascript_read_userdata(L, COUNTDOWN);

    let ret = countdown_has_ended(countdown);

    LUA.lua_pushboolean(L, ret);
    return 1;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/**@type {luaL_Reg[]}*/const COUNTDOWN_FUNCTIONS = [
    ["set_default_animation2", script_countdown_set_default_animation2],
    ["set_bpm", script_countdown_set_bpm],
    ["get_drawable", script_countdown_get_drawable],
    ["ready", script_countdown_ready],
    ["start", script_countdown_start],
    ["has_ended", script_countdown_has_ended],
    [null, null]
];


function script_countdown_new(L, countdown) {
    return luascript_userdata_new(L, COUNTDOWN, countdown);
}

function script_countdown_gc(L) {
    return luascript_userdata_gc(L, COUNTDOWN);
}

function script_countdown_tostring(L) {
    return luascript_userdata_tostring(L, COUNTDOWN);
}


function script_countdown_register(L) {
    luascript_register(L, COUNTDOWN, script_countdown_gc, script_countdown_tostring, COUNTDOWN_FUNCTIONS);
}
