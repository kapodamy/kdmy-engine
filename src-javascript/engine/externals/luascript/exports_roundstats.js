"use strict";

const ROUNDSTATS = "RoundStats";


function script_roundstats_hide(L) {
    let roundstats = luascript_read_userdata(L, ROUNDSTATS);
    let hide = LUA.lua_toboolean(L, 2);

    roundstats_hide(roundstats, hide);

    return 0;
}

function script_roundstats_hide_nps(L) {
    let roundstats = luascript_read_userdata(L, ROUNDSTATS);
    let hide = LUA.lua_toboolean(L, 2);

    roundstats_hide_nps(roundstats, hide);

    return 0;
}

function script_roundstats_set_draw_y(L) {
    let roundstats = luascript_read_userdata(L, ROUNDSTATS);
    let y = LUA.luaL_checknumber(L, 2);

    roundstats_set_draw_y(roundstats, y);

    return 0;
}

function script_roundstats_reset(L) {
    let roundstats = luascript_read_userdata(L, ROUNDSTATS);

    roundstats_reset(roundstats);

    return 0;
}

function script_roundstats_get_drawable(L) {
    let roundstats = luascript_read_userdata(L, ROUNDSTATS);

    let ret = roundstats_get_drawable(roundstats);

    return script_drawable_new(L, ret);
}

function script_roundstats_tweenkeyframe_set_on_beat(L) {
    let roundstats = luascript_read_userdata(L, ROUNDSTATS);
    let tweenkeyframe = luascript_read_nullable_userdata(L, 2, TWEENKEYFRAME);
    let rollback_beats = LUA.luaL_checknumber(L, 3);
    let beat_duration = LUA.luaL_checknumber(L, 4);

    roundstats_tweenkeyframe_set_on_beat(roundstats, tweenkeyframe, rollback_beats, beat_duration);

    return 0;
}

function script_roundstats_tweenkeyframe_set_on_hit(L) {
    let roundstats = luascript_read_userdata(L, ROUNDSTATS);
    let tweenkeyframe = luascript_read_nullable_userdata(L, 2, TWEENKEYFRAME);
    let rollback_beats = LUA.luaL_checknumber(L, 3);
    let beat_duration = LUA.luaL_checknumber(L, 4);

    roundstats_tweenkeyframe_set_on_hit(roundstats, tweenkeyframe, rollback_beats, beat_duration);

    return 0;
}

function script_roundstats_tweenkeyframe_set_on_miss(L) {
    let roundstats = luascript_read_userdata(L, ROUNDSTATS);
    let tweenkeyframe = luascript_read_nullable_userdata(L, 2, TWEENKEYFRAME);
    let rollback_beats = LUA.luaL_checknumber(L, 3);
    let beat_duration = LUA.luaL_checknumber(L, 4);

    roundstats_tweenkeyframe_set_on_miss(roundstats, tweenkeyframe, rollback_beats, beat_duration);

    return 0;
}

function script_roundstats_tweenkeyframe_set_bpm(L) {
    let roundstats = luascript_read_userdata(L, ROUNDSTATS);
    let beats_per_minute = LUA.luaL_checknumber(L, 2);

    roundstats_tweenkeyframe_set_bpm(roundstats, beats_per_minute);

    return 0;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/**@type {luaL_Reg[]}*/const ROUNDSTATS_FUNCTIONS = [
    ["hide", script_roundstats_hide],
    ["hide_nps", script_roundstats_hide_nps],
    ["set_draw_y", script_roundstats_set_draw_y],
    ["reset", script_roundstats_reset],
    ["get_drawable", script_roundstats_get_drawable],
    ["tweenkeyframe_set_on_beat", script_roundstats_tweenkeyframe_set_on_beat],
    ["tweenkeyframe_set_on_hit", script_roundstats_tweenkeyframe_set_on_hit],
    ["tweenkeyframe_set_on_miss", script_roundstats_tweenkeyframe_set_on_miss],
    ["tweenkeyframe_set_bpm", script_roundstats_tweenkeyframe_set_bpm],
    [null, null]
];

function script_roundstats_new(L, roundstats) {
    return luascript_userdata_new(L, ROUNDSTATS, roundstats);
}

function script_roundstats_gc(L) {
    return luascript_userdata_gc(L, ROUNDSTATS);
}

function script_roundstats_tostring(L) {
    return luascript_userdata_tostring(L, ROUNDSTATS);
}

function script_roundstats_register(L) {
    luascript_register(L, ROUNDSTATS, script_roundstats_gc, script_roundstats_tostring, ROUNDSTATS_FUNCTIONS);
}

