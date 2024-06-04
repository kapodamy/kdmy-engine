"use strict";

const HEALTHBAR = "HealthBar";


function script_healthbar_enable_extra_length(L) {
    let healthbar = luascript_read_userdata(L, HEALTHBAR);
    let extra_enabled = LUA.lua_toboolean(L, 2);

    healthbar_enable_extra_length(healthbar, extra_enabled);

    return 0;
}

function script_healthbar_enable_vertical(L) {
    let healthbar = luascript_read_userdata(L, HEALTHBAR);
    let enable_vertical = LUA.lua_toboolean(L, 2);

    healthbar_enable_vertical(healthbar, enable_vertical);

    return 0;
}

function script_healthbar_state_opponent_add(L) {
    let healthbar = luascript_read_userdata(L, HEALTHBAR);
    let icon_mdlhldr = luascript_read_nullable_userdata(L, 2, MODELHOLDER);
    let bar_mdlhldr = luascript_read_nullable_userdata(L, 3, MODELHOLDER);
    let state_name = LUA.luaL_optstring(L, 4, null);

    let ret = healthbar_state_opponent_add(healthbar, icon_mdlhldr, bar_mdlhldr, state_name);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_healthbar_state_opponent_add2(L) {
    let healthbar = luascript_read_userdata(L, HEALTHBAR);
    let icon_mdlhldr = luascript_read_nullable_userdata(L, 2, MODELHOLDER);
    let bar_color_rgb8 = LUA.luaL_checkinteger(L, 3);
    let state_name = LUA.luaL_optstring(L, 4, null);

    let ret = healthbar_state_opponent_add2(healthbar, icon_mdlhldr, bar_color_rgb8, state_name);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_healthbar_state_player_add(L) {
    let healthbar = luascript_read_userdata(L, HEALTHBAR);
    let icon_mdlhldr = luascript_read_nullable_userdata(L, 2, MODELHOLDER);
    let bar_mdlhldr = luascript_read_nullable_userdata(L, 3, MODELHOLDER);
    let state_name = LUA.luaL_optstring(L, 4, null);

    let ret = healthbar_state_player_add(healthbar, icon_mdlhldr, bar_mdlhldr, state_name);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_healthbar_state_player_add2(L) {
    let healthbar = luascript_read_userdata(L, HEALTHBAR);
    let icon_modelholder = luascript_read_nullable_userdata(L, 2, MODELHOLDER);
    let bar_color_rgb8 = LUA.luaL_checkinteger(L, 3);
    let state_name = LUA.luaL_optstring(L, 4, null);

    let ret = healthbar_state_player_add2(healthbar, icon_modelholder, bar_color_rgb8, state_name);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_healthbar_state_background_add(L) {
    let healthbar = luascript_read_userdata(L, HEALTHBAR);
    let modelholder = luascript_read_nullable_userdata(L, 2, MODELHOLDER);
    let state_name = LUA.luaL_optstring(L, 3, null);

    let ret = healthbar_state_background_add(healthbar, modelholder, state_name);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_healthbar_state_background_add2(L) {
    let healthbar = luascript_read_userdata(L, HEALTHBAR);
    let color_rgb8 = LUA.luaL_checkinteger(L, 2);
    let animsprite = luascript_read_nullable_userdata(L, 3, ANIMSPRITE);
    let state_name = LUA.luaL_optstring(L, 4, null);

    let ret = healthbar_state_background_add2(healthbar, color_rgb8, animsprite, state_name);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_healthbar_load_warnings(L) {
    let healthbar = luascript_read_userdata(L, HEALTHBAR);
    let modelholder = luascript_read_nullable_userdata(L, 2, MODELHOLDER);
    let use_alt_icons = LUA.lua_toboolean(L, 3);

    let ret = healthbar_load_warnings(healthbar, modelholder, use_alt_icons);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_healthbar_set_opponent_bar_color_rgb8(L) {
    let healthbar = luascript_read_userdata(L, HEALTHBAR);
    let color_rgb8 = LUA.luaL_checkinteger(L, 2);

    healthbar_set_opponent_bar_color_rgb8(healthbar, color_rgb8);

    return 0;
}

function script_healthbar_set_opponent_bar_color(L) {
    let healthbar = luascript_read_userdata(L, HEALTHBAR);
    let r = LUA.luaL_checknumber(L, 2);
    let g = LUA.luaL_checknumber(L, 3);
    let b = LUA.luaL_checknumber(L, 4);

    healthbar_set_opponent_bar_color(healthbar, r, g, b);

    return 0;
}

function script_healthbar_set_player_bar_color_rgb8(L) {
    let healthbar = luascript_read_userdata(L, HEALTHBAR);
    let color_rgb8 = LUA.luaL_checkinteger(L, 2);

    healthbar_set_player_bar_color_rgb8(healthbar, color_rgb8);

    return 0;
}

function script_healthbar_set_player_bar_color(L) {
    let healthbar = luascript_read_userdata(L, HEALTHBAR);
    let r = LUA.luaL_checknumber(L, 2);
    let g = LUA.luaL_checknumber(L, 3);
    let b = LUA.luaL_checknumber(L, 4);

    healthbar_set_player_bar_color(healthbar, r, g, b);

    return 0;
}

function script_healthbar_state_toggle(L) {
    let healthbar = luascript_read_userdata(L, HEALTHBAR);
    let state_name = LUA.luaL_optstring(L, 2, null);

    let ret = healthbar_state_toggle(healthbar, state_name);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_healthbar_state_toggle_background(L) {
    let healthbar = luascript_read_userdata(L, HEALTHBAR);
    let state_name = LUA.luaL_optstring(L, 2, null);

    let ret = healthbar_state_toggle_background(healthbar, state_name);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_healthbar_state_toggle_player(L) {
    let healthbar = luascript_read_userdata(L, HEALTHBAR);
    let state_name = LUA.luaL_optstring(L, 2, null);

    let ret = healthbar_state_toggle_player(healthbar, state_name);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_healthbar_state_toggle_opponent(L) {
    let healthbar = luascript_read_userdata(L, HEALTHBAR);
    let state_name = LUA.luaL_optstring(L, 2, null);

    let ret = healthbar_state_toggle_opponent(healthbar, state_name);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_healthbar_set_bump_animation_opponent(L) {
    let healthbar = luascript_read_userdata(L, HEALTHBAR);
    let animsprite = luascript_read_nullable_userdata(L, 2, ANIMSPRITE);

    healthbar_set_bump_animation_opponent(healthbar, animsprite);

    return 0;
}

function script_healthbar_set_bump_animation_player(L) {
    let healthbar = luascript_read_userdata(L, HEALTHBAR);
    let animsprite = luascript_read_nullable_userdata(L, 2, ANIMSPRITE);

    healthbar_set_bump_animation_player(healthbar, animsprite);

    return 0;
}

function script_healthbar_bump_enable(L) {
    let healthbar = luascript_read_userdata(L, HEALTHBAR);
    let enable_bump = LUA.lua_toboolean(L, 2);

    healthbar_bump_enable(healthbar, enable_bump);

    return 0;
}

function script_healthbar_set_bpm(L) {
    let healthbar = luascript_read_userdata(L, HEALTHBAR);
    let beats_per_minute = LUA.luaL_checknumber(L, 2);

    healthbar_set_bpm(healthbar, beats_per_minute);

    return 0;
}

function script_healthbar_set_alpha(L) {
    let healthbar = luascript_read_userdata(L, HEALTHBAR);
    let alpha = LUA.luaL_checknumber(L, 2);

    healthbar_set_alpha(healthbar, alpha);

    return 0;
}

function script_healthbar_set_visible(L) {
    let healthbar = luascript_read_userdata(L, HEALTHBAR);
    let visible = LUA.lua_toboolean(L, 2);

    healthbar_set_visible(healthbar, visible);

    return 0;
}

function script_healthbar_get_drawable(L) {
    let healthbar = luascript_read_userdata(L, HEALTHBAR);

    let ret = healthbar_get_drawable(healthbar);

    return script_drawable_new(L, ret);
}

function script_healthbar_animation_set(L) {
    let healthbar = luascript_read_userdata(L, HEALTHBAR);
    let animsprite = luascript_read_nullable_userdata(L, 2, ANIMSPRITE);

    healthbar_animation_set(healthbar, animsprite);

    return 0;
}

function script_healthbar_animation_restart(L) {
    let healthbar = luascript_read_userdata(L, HEALTHBAR);

    healthbar_animation_restart(healthbar);

    return 0;
}

function script_healthbar_animation_end(L) {
    let healthbar = luascript_read_userdata(L, HEALTHBAR);

    healthbar_animation_end(healthbar);

    return 0;
}

function script_healthbar_disable_progress_animation(L) {
    let healthbar = luascript_read_userdata(L, HEALTHBAR);
    let disable = LUA.lua_toboolean(L, 2);

    healthbar_disable_progress_animation(healthbar, disable);

    return 0;
}

function script_healthbar_set_health_position(L) {
    let healthbar = luascript_read_userdata(L, HEALTHBAR);
    let max_health = LUA.luaL_checknumber(L, 2);
    let health = LUA.luaL_checknumber(L, 3);
    let opponent_recover = LUA.lua_toboolean(L, 4);

    let ret = healthbar_set_health_position(healthbar, max_health, health, opponent_recover);

    LUA.lua_pushnumber(L, ret);
    return 1;
}

function script_healthbar_set_health_position2(L) {
    let healthbar = luascript_read_userdata(L, HEALTHBAR);
    let percent = LUA.luaL_checknumber(L, 2);

    healthbar_set_health_position2(healthbar, percent);

    return 0;
}

function script_healthbar_disable_icon_overlap(L) {
    let healthbar = luascript_read_userdata(L, HEALTHBAR);
    let disable = LUA.lua_toboolean(L, 2);

    healthbar_disable_icon_overlap(healthbar, disable);

    return 0;
}

function script_healthbar_disable_warnings(L) {
    let healthbar = luascript_read_userdata(L, HEALTHBAR);
    let disable = LUA.lua_toboolean(L, 2);

    healthbar_disable_warnings(healthbar, disable);

    return 0;
}

function script_healthbar_enable_low_health_flash_warning(L) {
    let healthbar = luascript_read_userdata(L, HEALTHBAR);
    let enable = LUA.lua_toboolean(L, 2);

    healthbar_enable_low_health_flash_warning(healthbar, enable);

    return 0;
}

function script_healthbar_hide_warnings(L) {
    let healthbar = luascript_read_userdata(L, HEALTHBAR);

    healthbar_hide_warnings(healthbar);

    return 0;
}

function script_healthbar_show_drain_warning(L) {
    let healthbar = luascript_read_userdata(L, HEALTHBAR);
    let use_fast_drain = LUA.lua_toboolean(L, 2);

    healthbar_show_drain_warning(healthbar, use_fast_drain);

    return 0;
}

function script_healthbar_show_locked_warning(L) {
    let healthbar = luascript_read_userdata(L, HEALTHBAR);

    healthbar_show_locked_warning(healthbar);

    return 0;
}

function script_healthbar_get_bar_midpoint(L) {
    let healthbar = luascript_read_userdata(L, HEALTHBAR);
    const output_location = [0.0, 0.0];

    healthbar_get_bar_midpoint(healthbar, output_location);

    LUA.lua_pushnumber(L, output_location[0]);
    LUA.lua_pushnumber(L, output_location[1]);
    return 0;
}

function script_healthbar_get_percent(L) {
    let healthbar = luascript_read_userdata(L, HEALTHBAR);

    let ret = healthbar_get_percent(healthbar);

    LUA.lua_pushnumber(L, ret);
    return 1;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/**@type {luaL_Reg[]}*/const HEALTHBAR_FUNCTIONS = [
    ["enable_extra_length", script_healthbar_enable_extra_length],
    ["enable_vertical", script_healthbar_enable_vertical],
    ["state_opponent_add", script_healthbar_state_opponent_add],
    ["state_opponent_add2", script_healthbar_state_opponent_add2],
    ["state_player_add", script_healthbar_state_player_add],
    ["state_player_add2", script_healthbar_state_player_add2],
    ["state_background_add", script_healthbar_state_background_add],
    ["state_background_add2", script_healthbar_state_background_add2],
    ["load_warnings", script_healthbar_load_warnings],
    ["set_opponent_bar_color_rgb8", script_healthbar_set_opponent_bar_color_rgb8],
    ["set_opponent_bar_color", script_healthbar_set_opponent_bar_color],
    ["set_player_bar_color_rgb8", script_healthbar_set_player_bar_color_rgb8],
    ["set_player_bar_color", script_healthbar_set_player_bar_color],
    ["state_toggle", script_healthbar_state_toggle],
    ["state_toggle_background", script_healthbar_state_toggle_background],
    ["state_toggle_player", script_healthbar_state_toggle_player],
    ["state_toggle_opponent", script_healthbar_state_toggle_opponent],
    ["set_bump_animation_opponent", script_healthbar_set_bump_animation_opponent],
    ["set_bump_animation_player", script_healthbar_set_bump_animation_player],
    ["bump_enable", script_healthbar_bump_enable],
    ["set_bpm", script_healthbar_set_bpm],
    ["set_alpha", script_healthbar_set_alpha],
    ["set_visible", script_healthbar_set_visible],
    ["get_drawable", script_healthbar_get_drawable],
    ["animation_set", script_healthbar_animation_set],
    ["animation_restart", script_healthbar_animation_restart],
    ["animation_end", script_healthbar_animation_end],
    ["disable_progress_animation", script_healthbar_disable_progress_animation],
    ["set_health_position", script_healthbar_set_health_position],
    ["set_health_position2", script_healthbar_set_health_position2],
    ["disable_icon_overlap", script_healthbar_disable_icon_overlap],
    ["disable_warnings", script_healthbar_disable_warnings],
    ["enable_low_health_flash_warning", script_healthbar_enable_low_health_flash_warning],
    ["hide_warnings", script_healthbar_hide_warnings],
    ["show_drain_warning", script_healthbar_show_drain_warning],
    ["show_locked_warning", script_healthbar_show_locked_warning],
    ["get_bar_midpoint", script_healthbar_get_bar_midpoint],
    ["get_percent", script_healthbar_get_percent],
    [null, null]
];


function script_healthbar_new(L, healthbar) {
    return luascript_userdata_new(L, HEALTHBAR, healthbar);
}

function script_healthbar_gc(L) {
    return luascript_userdata_gc(L, HEALTHBAR);
}

function script_healthbar_tostring(L) {
    return luascript_userdata_tostring(L, HEALTHBAR);
}

function script_healthbar_register(L) {
    luascript_register(L, HEALTHBAR, script_healthbar_gc, script_healthbar_tostring, HEALTHBAR_FUNCTIONS);
}

