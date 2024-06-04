"use strict";

const STRUM = "Strum";


function script_strum_update_draw_location(L) {
    let strum = luascript_read_userdata(L, STRUM);
    let x = LUA.luaL_checknumber(L, 2);
    let y = LUA.luaL_checknumber(L, 3);

    strum_update_draw_location(strum, x, y);

    return 0;
}

function script_strum_set_scroll_speed(L) {
    let strum = luascript_read_userdata(L, STRUM);
    let speed = LUA.luaL_checknumber(L, 2);

    strum_set_scroll_speed(strum, speed);

    return 0;
}

function script_strum_set_scroll_direction(L) {
    let strum = luascript_read_userdata(L, STRUM);
    let direction = luascript_helper_optenum(L, 2, LUASCRIPT_ENUMS_ScrollDirection);

    strum_set_scroll_direction(strum, direction);

    return 0;
}

function script_strum_set_marker_duration_multiplier(L) {
    let strum = luascript_read_userdata(L, STRUM);
    let multipler = LUA.luaL_checknumber(L, 2);

    strum_set_marker_duration_multiplier(strum, multipler);

    return 0;
}

function script_strum_reset(L) {
    let strum = luascript_read_userdata(L, STRUM);
    let scroll_speed = LUA.luaL_checknumber(L, 2);
    let state_name = LUA.luaL_optstring(L, 3, null);

    strum_reset(strum, scroll_speed, state_name);

    return 0;
}

function script_strum_force_key_release(L) {
    let strum = luascript_read_userdata(L, STRUM);

    strum_force_key_release(strum);

    return 0;
}

function script_strum_get_press_state_changes(L) {
    let strum = luascript_read_userdata(L, STRUM);

    let ret = strum_get_press_state_changes(strum);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_strum_get_press_state(L) {
    let strum = luascript_read_userdata(L, STRUM);

    let ret = strum_get_press_state(strum);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_strum_get_press_state_use_alt_anim(L) {
    let strum = luascript_read_userdata(L, STRUM);

    let ret = strum_get_press_state_use_alt_anim(strum);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_strum_get_name(L) {
    let strum = luascript_read_userdata(L, STRUM);

    let ret = strum_get_name(strum);

    LUA.lua_pushstring(L, ret);

    return 1;
}

function script_strum_get_marker_duration(L) {
    let strum = luascript_read_userdata(L, STRUM);

    let ret = strum_get_marker_duration(strum);

    LUA.lua_pushnumber(L, ret);
    return 1;
}

function script_strum_set_player_id(L) {
    let strum = luascript_read_userdata(L, STRUM);
    let player_id = LUA.luaL_checkinteger(L, 2);

    strum_set_player_id(strum, player_id);

    return 0;
}

function script_strum_enable_background(L) {
    let strum = luascript_read_userdata(L, STRUM);
    let enable = LUA.lua_toboolean(L, 2);

    strum_enable_background(strum, enable);

    return 0;
}

function script_strum_enable_sick_effect(L) {
    let strum = luascript_read_userdata(L, STRUM);
    let enable = LUA.lua_toboolean(L, 2);

    strum_enable_sick_effect(strum, enable);

    return 0;
}

function script_strum_state_add(L) {
    let strum = luascript_read_userdata(L, STRUM);
    let mdlhldr_mrkr = luascript_read_nullable_userdata(L, 2, MODELHOLDER);
    let mdlhldr_sck_ffct = luascript_read_nullable_userdata(L, 3, MODELHOLDER);
    let mdlhldr_bckgrnd = luascript_read_nullable_userdata(L, 4, MODELHOLDER);
    let state_name = LUA.luaL_optstring(L, 5, null);

    strum_state_add(strum, mdlhldr_mrkr, mdlhldr_sck_ffct, mdlhldr_bckgrnd, state_name);

    return 0;
}

function script_strum_state_toggle(L) {
    let strum = luascript_read_userdata(L, STRUM);
    let state_name = LUA.luaL_optstring(L, 2, null);

    let ret = strum_state_toggle(strum, state_name);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_strum_state_toggle_notes(L) {
    let strum = luascript_read_userdata(L, STRUM);
    let state_name = LUA.luaL_optstring(L, 2, null);

    let ret = strum_state_toggle_notes(strum, state_name);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_strum_state_toggle_sick_effect(L) {
    let strum = luascript_read_userdata(L, STRUM);
    let state_name = LUA.luaL_optstring(L, 2, null);

    let ret = strum_state_toggle_sick_effect(strum, state_name);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_strum_state_toggle_marker(L) {
    let strum = luascript_read_userdata(L, STRUM);
    let state_name = LUA.luaL_optstring(L, 2, null);

    let ret = strum_state_toggle_marker(strum, state_name);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_strum_state_toggle_background(L) {
    let strum = luascript_read_userdata(L, STRUM);
    let state_name = LUA.luaL_optstring(L, 2, null);

    let ret = strum_state_toggle_background(strum, state_name);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_strum_set_alpha_background(L) {
    let strum = luascript_read_userdata(L, STRUM);
    let alpha = LUA.luaL_checknumber(L, 2);

    let ret = strum_set_alpha_background(strum, alpha);

    LUA.lua_pushnumber(L, ret);
    return 1;
}

function script_strum_set_alpha_sick_effect(L) {
    let strum = luascript_read_userdata(L, STRUM);
    let alpha = LUA.luaL_checknumber(L, 2);

    let ret = strum_set_alpha_sick_effect(strum, alpha);

    LUA.lua_pushnumber(L, ret);
    return 1;
}

function script_strum_set_keep_aspect_ratio_background(L) {
    let strum = luascript_read_userdata(L, STRUM);
    let enable = LUA.lua_toboolean(L, 2);

    strum_set_keep_aspect_ratio_background(strum, enable);

    return 0;
}

function script_strum_draw_sick_effect_apart(L) {
    let strum = luascript_read_userdata(L, STRUM);
    let enable = LUA.lua_toboolean(L, 2);

    strum_draw_sick_effect_apart(strum, enable);

    return 0;
}

function script_strum_set_extra_animation(L) {
    let strum = luascript_read_userdata(L, STRUM);
    let strum_script_target = luascript_helper_optenum(L, 2, LUASCRIPT_ENUMS_StrumScriptTarget);
    let strum_script_on = luascript_helper_optenum(L, 3, LUASCRIPT_ENUMS_StrumScriptOn);
    let undo = LUA.lua_toboolean(L, 4);
    let animsprite = luascript_read_nullable_userdata(L, 5, ANIMSPRITE);

    strum_set_extra_animation(strum, strum_script_target, strum_script_on, undo, animsprite);

    return 0;
}

function script_strum_set_extra_animation_continuous(L) {
    let strum = luascript_read_userdata(L, STRUM);
    let strum_script_target = luascript_helper_optenum(L, 2, LUASCRIPT_ENUMS_StrumScriptTarget);
    let animsprite = luascript_read_nullable_userdata(L, 3, ANIMSPRITE);

    strum_set_extra_animation_continuous(strum, strum_script_target, animsprite);

    return 0;
}

function script_strum_disable_beat_synced_idle_and_continous(L) {
    let strum = luascript_read_userdata(L, STRUM);
    let disabled = LUA.lua_toboolean(L, 2);

    strum_disable_beat_synced_idle_and_continous(strum, disabled);

    return 0;
}

function script_strum_set_bpm(L) {
    let strum = luascript_read_userdata(L, STRUM);
    let bpm = LUA.luaL_checknumber(L, 2);

    strum_set_bpm(strum, bpm);

    return 0;
}

function script_strum_set_note_tweenkeyframe(L) {
    let strum = luascript_read_userdata(L, STRUM);
    let tweenkeyframe = luascript_read_nullable_userdata(L, 2, TWEENKEYFRAME);

    strum_set_note_tweenkeyframe(strum, tweenkeyframe);

    return 0;
}

function script_strum_set_sickeffect_size_ratio(L) {
    let strum = luascript_read_userdata(L, STRUM);
    let size_ratio = LUA.luaL_checknumber(L, 2);

    strum_set_sickeffect_size_ratio(strum, size_ratio);

    return 0;
}

function script_strum_set_alpha(L) {
    let strum = luascript_read_userdata(L, STRUM);
    let alpha = LUA.luaL_checknumber(L, 2);

    strum_set_alpha(strum, alpha);

    return 0;
}

function script_strum_set_visible(L) {
    let strum = luascript_read_userdata(L, STRUM);
    let visible = LUA.lua_toboolean(L, 2);

    strum_set_visible(strum, visible);

    return 0;
}

function script_strum_set_draw_offset(L) {
    let strum = luascript_read_userdata(L, STRUM);
    let offset_milliseconds = LUA.luaL_checknumber(L, 2);

    strum_set_draw_offset(strum, offset_milliseconds);

    return 0;
}

function script_strum_get_modifier(L) {
    let strum = luascript_read_userdata(L, STRUM);

    let ret = strum_get_modifier(strum);

    return script_modifier_new(L, ret);
}

function script_strum_get_duration(L) {
    let strum = luascript_read_userdata(L, STRUM);

    let ret = strum_get_duration(strum);

    LUA.lua_pushnumber(L, ret);
    return 1;
}

function script_strum_animation_restart(L) {
    let strum = luascript_read_userdata(L, STRUM);

    strum_animation_restart(strum);

    return 0;
}

function script_strum_animation_end(L) {
    let strum = luascript_read_userdata(L, STRUM);

    strum_animation_end(strum);

    return 0;
}

function script_strum_get_drawable(L) {
    let strum = luascript_read_userdata(L, STRUM);

    let ret = strum_get_drawable(strum);

    return script_drawable_new(L, ret);
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/**@type {luaL_Reg[]}*/const STRUM_FUNCTIONS = [
    ["update_draw_location", script_strum_update_draw_location],
    ["set_scroll_speed", script_strum_set_scroll_speed],
    ["set_scroll_direction", script_strum_set_scroll_direction],
    ["set_marker_duration_multiplier", script_strum_set_marker_duration_multiplier],
    ["reset", script_strum_reset],
    ["force_key_release", script_strum_force_key_release],
    ["get_press_state_changes", script_strum_get_press_state_changes],
    ["get_press_state", script_strum_get_press_state],
    ["get_press_state_use_alt_anim", script_strum_get_press_state_use_alt_anim],
    ["get_name", script_strum_get_name],
    ["get_marker_duration", script_strum_get_marker_duration],
    ["set_player_id", script_strum_set_player_id],
    ["enable_background", script_strum_enable_background],
    ["enable_sick_effect", script_strum_enable_sick_effect],
    ["state_add", script_strum_state_add],
    ["state_toggle", script_strum_state_toggle],
    ["state_toggle_notes", script_strum_state_toggle_notes],
    ["state_toggle_sick_effect", script_strum_state_toggle_sick_effect],
    ["state_toggle_marker", script_strum_state_toggle_marker],
    ["state_toggle_background", script_strum_state_toggle_background],
    ["set_alpha_background", script_strum_set_alpha_background],
    ["set_alpha_sick_effect", script_strum_set_alpha_sick_effect],
    ["set_keep_aspect_ratio_background", script_strum_set_keep_aspect_ratio_background],
    ["draw_sick_effect_apart", script_strum_draw_sick_effect_apart],
    ["set_extra_animation", script_strum_set_extra_animation],
    ["set_extra_animation_continuous", script_strum_set_extra_animation_continuous],
    ["disable_beat_synced_idle_and_continous", script_strum_disable_beat_synced_idle_and_continous],
    ["set_bpm", script_strum_set_bpm],
    ["set_note_tweenkeyframe", script_strum_set_note_tweenkeyframe],
    ["set_sickeffect_size_ratio", script_strum_set_sickeffect_size_ratio],
    ["set_alpha", script_strum_set_alpha],
    ["set_visible", script_strum_set_visible],
    ["set_draw_offset", script_strum_set_draw_offset],
    ["get_modifier", script_strum_get_modifier],
    ["get_duration", script_strum_get_duration],
    ["animation_restart", script_strum_animation_restart],
    ["animation_end", script_strum_animation_end],
    ["get_drawable", script_strum_get_drawable],
    [null, null]
];

function script_strum_new(L, strum) {
    return luascript_userdata_new(L, STRUM, strum);
}

function script_strum_gc(L) {
    return luascript_userdata_gc(L, STRUM);
}

function script_strum_tostring(L) {
    return luascript_userdata_tostring(L, STRUM);
}

function script_strum_register(L) {
    luascript_register(L, STRUM, script_strum_gc, script_strum_tostring, STRUM_FUNCTIONS);
}

