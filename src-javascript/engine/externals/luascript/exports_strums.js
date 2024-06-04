"use strict";

const STRUMS = "Strum";


function script_strums_get_drawable(L) {
    let strums = luascript_read_userdata(L, STRUMS);

    let ret = strums_get_drawable(strums);

    return script_drawable_new(L, ret);
}

function script_strums_set_scroll_speed(L) {
    let strums = luascript_read_userdata(L, STRUMS);
    let speed = LUA.luaL_checknumber(L, 2);

    strums_set_scroll_speed(strums, speed);

    return 0;
}

function script_strums_set_scroll_direction(L) {
    let strums = luascript_read_userdata(L, STRUMS);
    let direction = luascript_helper_optenum(L, 2, LUASCRIPT_ENUMS_ScrollDirection);

    strums_set_scroll_direction(strums, direction);

    return 0;
}

function script_strums_set_marker_duration_multiplier(L) {
    let strums = luascript_read_userdata(L, STRUMS);
    let multipler = LUA.luaL_checknumber(L, 2);

    strums_set_marker_duration_multiplier(strums, multipler);

    return 0;
}

function script_strums_disable_beat_synced_idle_and_continous(L) {
    let strums = luascript_read_userdata(L, STRUMS);
    let disabled = LUA.lua_toboolean(L, 2);

    strums_disable_beat_synced_idle_and_continous(strums, disabled);

    return 0;
}

function script_strums_set_bpm(L) {
    let strums = luascript_read_userdata(L, STRUMS);
    let bpm = LUA.luaL_checknumber(L, 2);

    strums_set_bpm(strums, bpm);

    return 0;
}

function script_strums_reset(L) {
    let strums = luascript_read_userdata(L, STRUMS);
    let scroll_speed = LUA.luaL_checknumber(L, 2);
    let state_name = LUA.luaL_optstring(L, 3, null);

    strums_reset(strums, scroll_speed, state_name);

    return 0;
}

function script_strums_force_key_release(L) {
    let strums = luascript_read_userdata(L, STRUMS);

    strums_force_key_release(strums);

    return 0;
}

function script_strums_set_alpha(L) {
    let strums = luascript_read_userdata(L, STRUMS);
    let alpha = LUA.luaL_checknumber(L, 2);

    let ret = strums_set_alpha(strums, alpha);

    LUA.lua_pushnumber(L, ret);
    return 1;
}

function script_strums_enable_background(L) {
    let strums = luascript_read_userdata(L, STRUMS);
    let enable = LUA.lua_toboolean(L, 2);

    strums_enable_background(strums, enable);

    return 0;
}

function script_strums_set_keep_aspect_ratio_background(L) {
    let strums = luascript_read_userdata(L, STRUMS);
    let enable = LUA.lua_toboolean(L, 2);

    strums_set_keep_aspect_ratio_background(strums, enable);

    return 0;
}

function script_strums_set_alpha_background(L) {
    let strums = luascript_read_userdata(L, STRUMS);
    let alpha = LUA.luaL_checknumber(L, 2);

    strums_set_alpha_background(strums, alpha);

    return 0;
}

function script_strums_set_alpha_sick_effect(L) {
    let strums = luascript_read_userdata(L, STRUMS);
    let alpha = LUA.luaL_checknumber(L, 2);

    strums_set_alpha_sick_effect(strums, alpha);

    return 0;
}

function script_strums_set_draw_offset(L) {
    let strums = luascript_read_userdata(L, STRUMS);
    let offset_milliseconds = LUA.luaL_checknumber(L, 2);

    strums_set_draw_offset(strums, offset_milliseconds);

    return 0;
}

function script_strums_state_add(L) {
    let strums = luascript_read_userdata(L, STRUMS);
    let mdlhldr_mrkr = luascript_read_nullable_userdata(L, 2, MODELHOLDER);
    let mdlhldr_sck_ffct = luascript_read_nullable_userdata(L, 3, MODELHOLDER);
    let mdlhldr_bckgrnd = luascript_read_nullable_userdata(L, 4, MODELHOLDER);
    let state_name = LUA.luaL_optstring(L, 5, null);

    strums_state_add(strums, mdlhldr_mrkr, mdlhldr_sck_ffct, mdlhldr_bckgrnd, state_name);

    return 0;
}

function script_strums_state_toggle(L) {
    let strums = luascript_read_userdata(L, STRUMS);
    let state_name = LUA.luaL_optstring(L, 2, null);

    let ret = strums_state_toggle(strums, state_name);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_strums_state_toggle_notes(L) {
    let strums = luascript_read_userdata(L, STRUMS);
    let state_name = LUA.luaL_optstring(L, 2, null);

    let ret = strums_state_toggle_notes(strums, state_name);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_strums_state_toggle_marker_and_sick_effect(L) {
    let strums = luascript_read_userdata(L, STRUMS);
    let state_name = LUA.luaL_optstring(L, 2, null);

    strums_state_toggle_marker_and_sick_effect(strums, state_name);

    return 0;
}

function script_strums_get_lines_count(L) {
    let strums = luascript_read_userdata(L, STRUMS);

    let ret = strums_get_lines_count(strums);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_strums_get_strum_line(L) {
    let strums = luascript_read_userdata(L, STRUMS);
    let index = LUA.luaL_checkinteger(L, 2);

    let ret = strums_get_strum_line(strums, index);

    return script_strum_new(L, ret);
}

function script_strums_enable_post_sick_effect_draw(L) {
    let strums = luascript_read_userdata(L, STRUMS);
    let enable = LUA.lua_toboolean(L, 2);

    strums_enable_post_sick_effect_draw(strums, enable);

    return 0;
}

function script_strums_animation_set(L) {
    let strums = luascript_read_userdata(L, STRUMS);
    let animsprite = luascript_read_nullable_userdata(L, 2, ANIMSPRITE);

    strums_animation_set(strums, animsprite);

    return 0;
}

function script_strums_animation_restart(L) {
    let strums = luascript_read_userdata(L, STRUMS);

    strums_animation_restart(strums);

    return 0;
}

function script_strums_animation_end(L) {
    let strums = luascript_read_userdata(L, STRUMS);

    strums_animation_end(strums);

    return 0;
}

function script_strums_decorators_get_count(L) {
    let strums = luascript_read_userdata(L, STRUMS);

    let ret = strums_decorators_get_count(strums);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_strums_decorators_add(L) {
    let strums = luascript_read_userdata(L, STRUMS);
    let modelholder = luascript_read_nullable_userdata(L, 2, MODELHOLDER);
    let animation_name = LUA.luaL_checkstring(L, 3);
    let timestamp = LUA.luaL_checknumber(L, 4);

    let ret = strums_decorators_add(strums, modelholder, animation_name, timestamp);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_strums_decorators_add2(L) {
    let strums = luascript_read_userdata(L, STRUMS);
    let modelholder = luascript_read_nullable_userdata(L, 2, MODELHOLDER);
    let animation_name = LUA.luaL_checkstring(L, 3);
    let timestamp = LUA.luaL_checknumber(L, 4);
    let from_strum_index = LUA.luaL_checkinteger(L, 5);
    let to_strum_index = LUA.luaL_checkinteger(L, 6);

    let ret = strums_decorators_add2(strums, modelholder, animation_name, timestamp, from_strum_index, to_strum_index);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_strums_decorators_set_scroll_speed(L) {
    let strums = luascript_read_userdata(L, STRUMS);
    let speed = LUA.luaL_checknumber(L, 2);

    strums_decorators_set_scroll_speed(strums, speed);

    return 0;
}

function script_strums_decorators_set_alpha(L) {
    let strums = luascript_read_userdata(L, STRUMS);
    let alpha = LUA.luaL_checknumber(L, 2);

    strums_decorators_set_alpha(strums, alpha);

    return 0;
}

function script_strums_decorators_set_visible(L) {
    let strums = luascript_read_userdata(L, STRUMS);
    let decorator_timestamp = LUA.luaL_checknumber(L, 2);
    let visible = LUA.lua_toboolean(L, 3);

    strums_decorators_set_visible(strums, decorator_timestamp, visible);

    return 0;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/**@type {luaL_Reg[]}*/const STRUMS_FUNCTIONS = [
    ["get_drawable", script_strums_get_drawable],
    ["set_scroll_speed", script_strums_set_scroll_speed],
    ["set_scroll_direction", script_strums_set_scroll_direction],
    ["set_marker_duration_multiplier", script_strums_set_marker_duration_multiplier],
    ["disable_beat_synced_idle_and_continous", script_strums_disable_beat_synced_idle_and_continous],
    ["set_bpm", script_strums_set_bpm],
    ["reset", script_strums_reset],
    ["force_key_release", script_strums_force_key_release],
    ["set_alpha", script_strums_set_alpha],
    ["enable_background", script_strums_enable_background],
    ["set_keep_aspect_ratio_background", script_strums_set_keep_aspect_ratio_background],
    ["set_alpha_background", script_strums_set_alpha_background],
    ["set_alpha_sick_effect", script_strums_set_alpha_sick_effect],
    ["set_draw_offset", script_strums_set_draw_offset],
    ["state_add", script_strums_state_add],
    ["state_toggle", script_strums_state_toggle],
    ["state_toggle_notes", script_strums_state_toggle_notes],
    ["state_toggle_marker_and_sick_effect", script_strums_state_toggle_marker_and_sick_effect],
    ["get_lines_count", script_strums_get_lines_count],
    ["get_strum_line", script_strums_get_strum_line],
    ["enable_post_sick_effect_draw", script_strums_enable_post_sick_effect_draw],
    ["animation_set", script_strums_animation_set],
    ["animation_restart", script_strums_animation_restart],
    ["animation_end", script_strums_animation_end],
    ["decorators_get_count", script_strums_decorators_get_count],
    ["decorators_add", script_strums_decorators_add],
    ["decorators_add2", script_strums_decorators_add2],
    ["decorators_set_scroll_speed", script_strums_decorators_set_scroll_speed],
    ["decorators_set_alpha", script_strums_decorators_set_alpha],
    ["decorators_set_visible", script_strums_decorators_set_visible],
    [null, null]
];

function script_strums_new(L, strums) {
    return luascript_userdata_new(L, STRUMS, strums);
}

function script_strums_gc(L) {
    return luascript_userdata_gc(L, STRUMS);
}

function script_strums_tostring(L) {
    return luascript_userdata_tostring(L, STRUMS);
}

function script_strums_register(L) {
    luascript_register(L, STRUMS, script_strums_gc, script_strums_tostring, STRUMS_FUNCTIONS);
}

