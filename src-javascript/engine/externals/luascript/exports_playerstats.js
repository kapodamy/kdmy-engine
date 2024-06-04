"use strict";

const PLAYERSTATS = "PlayerStats";


function script_playerstats_add_hit(L) {
    let playerstats = luascript_read_userdata(L, PLAYERSTATS);
    let multiplier = LUA.luaL_checknumber(L, 2);
    let base_note_duration = LUA.luaL_checknumber(L, 3);
    let hit_time_difference = LUA.luaL_checknumber(L, 4);

    let ret = playerstats_add_hit(playerstats, multiplier, base_note_duration, hit_time_difference);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_playerstats_add_sustain(L) {
    let playerstats = luascript_read_userdata(L, PLAYERSTATS);
    let quarters = LUA.luaL_checkinteger(L, 2);
    let is_released = LUA.lua_toboolean(L, 3);

    playerstats_add_sustain(playerstats, quarters, is_released);

    return 0;
}

function script_playerstats_add_sustain_delayed_hit(L) {
    let playerstats = luascript_read_userdata(L, PLAYERSTATS);
    let multiplier = LUA.luaL_checknumber(L, 2);
    let hit_time_difference = LUA.luaL_checknumber(L, 3);

    let ret = playerstats_add_sustain_delayed_hit(playerstats, multiplier, hit_time_difference);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_playerstats_add_penality(L) {
    let playerstats = luascript_read_userdata(L, PLAYERSTATS);
    let on_empty_strum = LUA.lua_toboolean(L, 2);

    playerstats_add_penality(playerstats, on_empty_strum);

    return 0;
}

function script_playerstats_add_miss(L) {
    let playerstats = luascript_read_userdata(L, PLAYERSTATS);
    let multiplier = LUA.luaL_checknumber(L, 2);

    playerstats_add_miss(playerstats, multiplier);

    return 0;
}

function script_playerstats_reset(L) {
    let playerstats = luascript_read_userdata(L, PLAYERSTATS);

    playerstats_reset(playerstats);

    return 0;
}

function script_playerstats_reset_notes_per_seconds(L) {
    let playerstats = luascript_read_userdata(L, PLAYERSTATS);

    playerstats_reset_notes_per_seconds(playerstats);

    return 0;
}

function script_playerstats_add_extra_health(L) {
    let playerstats = luascript_read_userdata(L, PLAYERSTATS);
    let multiplier = LUA.luaL_checknumber(L, 2);

    playerstats_add_extra_health(playerstats, multiplier);

    return 0;
}

function script_playerstats_enable_penality_on_empty_strum(L) {
    let playerstats = luascript_read_userdata(L, PLAYERSTATS);
    let enable = LUA.lua_toboolean(L, 2);

    playerstats_enable_penality_on_empty_strum(playerstats, enable);

    return 0;
}

function script_playerstats_get_maximum_health(L) {
    let playerstats = luascript_read_userdata(L, PLAYERSTATS);

    let ret = playerstats_get_maximum_health(playerstats);

    LUA.lua_pushnumber(L, ret);
    return 1;
}

function script_playerstats_get_health(L) {
    let playerstats = luascript_read_userdata(L, PLAYERSTATS);

    let ret = playerstats_get_health(playerstats);

    LUA.lua_pushnumber(L, ret);
    return 1;
}

function script_playerstats_get_accuracy(L) {
    let playerstats = luascript_read_userdata(L, PLAYERSTATS);

    let ret = playerstats_get_accuracy(playerstats);

    LUA.lua_pushnumber(L, ret);
    return 1;
}

function script_playerstats_get_last_accuracy(L) {
    let playerstats = luascript_read_userdata(L, PLAYERSTATS);

    let ret = playerstats_get_last_accuracy(playerstats);

    LUA.lua_pushnumber(L, ret);
    return 1;
}

function script_playerstats_get_last_ranking(L) {
    let playerstats = luascript_read_userdata(L, PLAYERSTATS);

    let ret = playerstats_get_last_ranking(playerstats);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_playerstats_get_last_difference(L) {
    let playerstats = luascript_read_userdata(L, PLAYERSTATS);

    let ret = playerstats_get_last_difference(playerstats);

    LUA.lua_pushnumber(L, ret);
    return 1;
}

function script_playerstats_get_combo_streak(L) {
    let playerstats = luascript_read_userdata(L, PLAYERSTATS);

    let ret = playerstats_get_combo_streak(playerstats);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_playerstats_get_highest_combo_streak(L) {
    let playerstats = luascript_read_userdata(L, PLAYERSTATS);

    let ret = playerstats_get_highest_combo_streak(playerstats);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_playerstats_get_combo_breaks(L) {
    let playerstats = luascript_read_userdata(L, PLAYERSTATS);

    let ret = playerstats_get_combo_breaks(playerstats);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_playerstats_get_notes_per_seconds(L) {
    let playerstats = luascript_read_userdata(L, PLAYERSTATS);

    let ret = playerstats_get_notes_per_seconds(playerstats);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_playerstats_get_notes_per_seconds_highest(L) {
    let playerstats = luascript_read_userdata(L, PLAYERSTATS);

    let ret = playerstats_get_notes_per_seconds_highest(playerstats);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_playerstats_get_iterations(L) {
    let playerstats = luascript_read_userdata(L, PLAYERSTATS);

    let ret = playerstats_get_iterations(playerstats);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_playerstats_get_score(L) {
    let playerstats = luascript_read_userdata(L, PLAYERSTATS);

    let ret = playerstats_get_score(playerstats);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_playerstats_get_hits(L) {
    let playerstats = luascript_read_userdata(L, PLAYERSTATS);

    let ret = playerstats_get_hits(playerstats);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_playerstats_get_misses(L) {
    let playerstats = luascript_read_userdata(L, PLAYERSTATS);

    let ret = playerstats_get_misses(playerstats);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_playerstats_get_penalties(L) {
    let playerstats = luascript_read_userdata(L, PLAYERSTATS);

    let ret = playerstats_get_penalties(playerstats);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_playerstats_get_shits(L) {
    let playerstats = luascript_read_userdata(L, PLAYERSTATS);

    let ret = playerstats_get_shits(playerstats);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_playerstats_get_bads(L) {
    let playerstats = luascript_read_userdata(L, PLAYERSTATS);

    let ret = playerstats_get_bads(playerstats);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_playerstats_get_goods(L) {
    let playerstats = luascript_read_userdata(L, PLAYERSTATS);

    let ret = playerstats_get_goods(playerstats);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_playerstats_get_sicks(L) {
    let playerstats = luascript_read_userdata(L, PLAYERSTATS);

    let ret = playerstats_get_sicks(playerstats);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_playerstats_set_health(L) {
    let playerstats = luascript_read_userdata(L, PLAYERSTATS);
    let health = LUA.luaL_checknumber(L, 2);

    playerstats_set_health(playerstats, health);

    return 0;
}

function script_playerstats_add_health(L) {
    let playerstats = luascript_read_userdata(L, PLAYERSTATS);
    let health = LUA.luaL_checknumber(L, 2);
    let die_if_negative = LUA.lua_toboolean(L, 3);

    let ret = playerstats_add_health(playerstats, health, die_if_negative);

    LUA.lua_pushnumber(L, ret);
    return 1;
}

function script_playerstats_raise(L) {
    let playerstats = luascript_read_userdata(L, PLAYERSTATS);
    let with_full_health = LUA.lua_toboolean(L, 2);

    playerstats_raise(playerstats, with_full_health);

    return 0;
}

function script_playerstats_kill(L) {
    let playerstats = luascript_read_userdata(L, PLAYERSTATS);

    playerstats_kill(playerstats);

    return 0;
}

function script_playerstats_kill_if_negative_health(L) {
    let playerstats = luascript_read_userdata(L, PLAYERSTATS);

    playerstats_kill_if_negative_health(playerstats);

    return 0;
}

function script_playerstats_is_dead(L) {
    let playerstats = luascript_read_userdata(L, PLAYERSTATS);

    let ret = playerstats_is_dead(playerstats);

    LUA.lua_pushboolean(L, ret);
    return 1;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/**@type {luaL_Reg[]}*/const PLAYERSTATS_FUNCTIONS = [
    ["add_hit", script_playerstats_add_hit],
    ["add_sustain", script_playerstats_add_sustain],
    ["add_sustain_delayed_hit", script_playerstats_add_sustain_delayed_hit],
    ["add_penality", script_playerstats_add_penality],
    ["add_miss", script_playerstats_add_miss],
    ["reset", script_playerstats_reset],
    ["reset_notes_per_seconds", script_playerstats_reset_notes_per_seconds],
    ["add_extra_health", script_playerstats_add_extra_health],
    ["enable_penality_on_empty_strum", script_playerstats_enable_penality_on_empty_strum],
    ["get_maximum_health", script_playerstats_get_maximum_health],
    ["get_health", script_playerstats_get_health],
    ["get_accuracy", script_playerstats_get_accuracy],
    ["get_last_accuracy", script_playerstats_get_last_accuracy],
    ["get_last_ranking", script_playerstats_get_last_ranking],
    ["get_last_difference", script_playerstats_get_last_difference],
    ["get_combo_streak", script_playerstats_get_combo_streak],
    ["get_highest_combo_streak", script_playerstats_get_highest_combo_streak],
    ["get_combo_breaks", script_playerstats_get_combo_breaks],
    ["get_notes_per_seconds", script_playerstats_get_notes_per_seconds],
    ["get_notes_per_seconds_highest", script_playerstats_get_notes_per_seconds_highest],
    ["get_iterations", script_playerstats_get_iterations],
    ["get_score", script_playerstats_get_score],
    ["get_hits", script_playerstats_get_hits],
    ["get_misses", script_playerstats_get_misses],
    ["get_penalties", script_playerstats_get_penalties],
    ["get_shits", script_playerstats_get_shits],
    ["get_bads", script_playerstats_get_bads],
    ["get_goods", script_playerstats_get_goods],
    ["get_sicks", script_playerstats_get_sicks],
    ["set_health", script_playerstats_set_health],
    ["add_health", script_playerstats_add_health],
    ["raise", script_playerstats_raise],
    ["kill", script_playerstats_kill],
    ["kill_if_negative_health", script_playerstats_kill_if_negative_health],
    ["is_dead", script_playerstats_is_dead],
    [null, null]
];

function script_playerstats_new(L, playerstats) {
    return luascript_userdata_new(L, PLAYERSTATS, playerstats);
}

function script_playerstats_gc(L) {
    return luascript_userdata_gc(L, PLAYERSTATS);
}

function script_playerstats_tostring(L) {
    return luascript_userdata_tostring(L, PLAYERSTATS);
}

function script_playerstats_register(L) {
    luascript_register(L, PLAYERSTATS, script_playerstats_gc, script_playerstats_tostring, PLAYERSTATS_FUNCTIONS);
}

