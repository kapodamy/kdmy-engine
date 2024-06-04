"use strict";

const CHARACTER = "Character";


function script_character_use_alternate_sing_animations(L) {
    let character = luascript_read_userdata(L, CHARACTER);

    let enable = LUA.lua_toboolean(L, 2);

    character_use_alternate_sing_animations(character, enable);

    return 0;
}

function script_character_set_draw_location(L) {
    let character = luascript_read_userdata(L, CHARACTER);

    let x = LUA.luaL_checknumber(L, 2);
    let y = LUA.luaL_checknumber(L, 3);

    character_set_draw_location(character, x, y);

    return 0;
}

function script_character_set_draw_align(L) {
    let character = luascript_read_userdata(L, CHARACTER);

    let align_vertical = luascript_helper_optenum(L, 2, LUASCRIPT_ENUMS_Align);
    let align_horizontal = luascript_helper_optenum(L, 3, LUASCRIPT_ENUMS_Align);

    character_set_draw_align(character, align_vertical, align_horizontal);

    return 0;
}

function script_character_update_reference_size(L) {
    let character = luascript_read_userdata(L, CHARACTER);

    let width = LUA.luaL_optnumber(L, 2, NaN);
    let height = LUA.luaL_optnumber(L, 3, NaN);

    character_update_reference_size(character, width, height);

    return 0;
}

function script_character_enable_reference_size(L) {
    let character = luascript_read_userdata(L, CHARACTER);

    let enable = LUA.lua_toboolean(L, 2);

    character_enable_reference_size(character, enable);

    return 0;
}

function script_character_set_offset(L) {
    let character = luascript_read_userdata(L, CHARACTER);

    let offset_x = LUA.luaL_checknumber(L, 2);
    let offset_y = LUA.luaL_checknumber(L, 3);

    character_set_offset(character, offset_x, offset_y);

    return 0;
}

function script_character_state_add(L) {
    let character = luascript_read_userdata(L, CHARACTER);

    let modelholder = LUA.luaL_testudata(L, 2, MODELHOLDER);
    let state_name = LUA.luaL_optstring(L, 3, null);

    let ret = character_state_add(character, modelholder, state_name);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_character_state_toggle(L) {
    let character = luascript_read_userdata(L, CHARACTER);

    let state_name = LUA.luaL_optstring(L, 2, null);

    let ret = character_state_toggle(character, state_name);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_character_play_hey(L) {
    let character = luascript_read_userdata(L, CHARACTER);

    let ret = character_play_hey(character);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_character_play_idle(L) {
    let character = luascript_read_userdata(L, CHARACTER);

    let ret = character_play_idle(character);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_character_play_sing(L) {
    let character = luascript_read_userdata(L, CHARACTER);

    let direction = LUA.luaL_optstring(L, 2, null);
    let prefer_sustain = LUA.lua_toboolean(L, 3);

    let ret = character_play_sing(character, direction, prefer_sustain);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_character_play_miss(L) {
    let character = luascript_read_userdata(L, CHARACTER);

    let direction = LUA.luaL_optstring(L, 2, null);
    let keep_in_hold = LUA.lua_toboolean(L, 3);

    let ret = character_play_miss(character, direction, keep_in_hold);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_character_play_extra(L) {
    let character = luascript_read_userdata(L, CHARACTER);

    let extra_animation_name = LUA.luaL_optstring(L, 2, null);
    let prefer_sustain = LUA.lua_toboolean(L, 3);

    let ret = character_play_extra(character, extra_animation_name, prefer_sustain);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_character_set_idle_speed(L) {
    let character = luascript_read_userdata(L, CHARACTER);

    let speed = LUA.luaL_checknumber(L, 2);

    character_set_idle_speed(character, speed);

    return 0;
}

function script_character_set_scale(L) {
    let character = luascript_read_userdata(L, CHARACTER);

    let scale_factor = LUA.luaL_checknumber(L, 2);

    character_set_scale(character, scale_factor);

    return 0;
}

function script_character_reset(L) {
    let character = luascript_read_userdata(L, CHARACTER);

    character_reset(character);

    return 0;
}

function script_character_enable_continuous_idle(L) {
    let character = luascript_read_userdata(L, CHARACTER);

    let enable = LUA.lua_toboolean(L, 2);

    character_enable_continuous_idle(character, enable);

    return 0;
}

function script_character_is_idle_active(L) {
    let character = luascript_read_userdata(L, CHARACTER);

    let ret = character_is_idle_active(character);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_character_enable_flip_correction(L) {
    let character = luascript_read_userdata(L, CHARACTER);

    let enable = LUA.lua_toboolean(L, 2);

    character_enable_flip_correction(character, enable);

    return 0;
}

function script_character_flip_orientation(L) {
    let character = luascript_read_userdata(L, CHARACTER);

    let enable = LUA.lua_toboolean(L, 2);

    character_flip_orientation(character, enable);

    return 0;
}

function script_character_face_as_opponent(L) {
    let character = luascript_read_userdata(L, CHARACTER);

    let face_as_opponent = LUA.lua_toboolean(L, 2);

    character_face_as_opponent(character, face_as_opponent);

    return 0;
}

function script_character_set_z_index(L) {
    let character = luascript_read_userdata(L, CHARACTER);

    let z = LUA.luaL_checknumber(L, 2);

    character_set_z_index(character, z);

    return 0;
}

function script_character_set_z_offset(L) {
    let character = luascript_read_userdata(L, CHARACTER);

    let z_offset = LUA.luaL_checknumber(L, 2);

    character_set_z_offset(character, z_offset);

    return 0;
}

function script_character_animation_set(L) {
    let character = luascript_read_userdata(L, CHARACTER);

    let animsprite = LUA.luaL_testudata(L, 2, ANIMSPRITE);

    character_animation_set(character, animsprite);

    return 0;
}

function script_character_animation_restart(L) {
    let character = luascript_read_userdata(L, CHARACTER);

    character_animation_restart(character);

    return 0;
}

function script_character_animation_end(L) {
    let character = luascript_read_userdata(L, CHARACTER);

    character_animation_end(character);

    return 0;
}

function script_character_set_color_offset(L) {
    let character = luascript_read_userdata(L, CHARACTER);

    let r = LUA.luaL_optnumber(L, 2, NaN);
    let g = LUA.luaL_optnumber(L, 3, NaN);
    let b = LUA.luaL_optnumber(L, 4, NaN);
    let a = LUA.luaL_optnumber(L, 5, NaN);

    character_set_color_offset(character, r, g, b, a);

    return 0;
}

function script_character_set_color_offset_to_default(L) {
    let character = luascript_read_userdata(L, CHARACTER);

    character_set_color_offset_to_default(character);

    return 0;
}

function script_character_set_alpha(L) {
    let character = luascript_read_userdata(L, CHARACTER);

    let alpha = LUA.luaL_checknumber(L, 2);

    character_set_alpha(character, alpha);

    return 0;
}

function script_character_set_visible(L) {
    let character = luascript_read_userdata(L, CHARACTER);

    let visible = LUA.lua_toboolean(L, 2);

    character_set_visible(character, visible);

    return 0;
}

function script_character_get_modifier(L) {
    let character = luascript_read_userdata(L, CHARACTER);

    let modifier = character_get_modifier(character);

    return script_modifier_new(L, modifier);
}

function script_character_has_direction(L) {
    let character = luascript_read_userdata(L, CHARACTER);

    let name = LUA.luaL_optstring(L, 2, null);
    let is_extra = LUA.lua_toboolean(L, 3);

    let ret = character_has_direction(character, name, is_extra);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_character_get_play_calls(L) {
    let character = luascript_read_userdata(L, CHARACTER);

    let ret = character_get_play_calls(character);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_character_get_commited_animations_count(L) {
    let character = luascript_read_userdata(L, CHARACTER);

    let ret = character_get_commited_animations_count(character);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_character_get_current_action(L) {
    let character = luascript_read_userdata(L, CHARACTER);

    let ret = character_get_current_action(character);

    luascript_helper_pushenum(L, LUASCRIPT_ENUMS_CharacterActionType, ret);
    return 1;
}

function script_character_freeze_animation(L) {
    let character = luascript_read_userdata(L, CHARACTER);

    let enabled = LUA.lua_toboolean(L, 2);

    character_freeze_animation(character, enabled);

    return 0;
}

function script_character_trailing_enabled(L) {
    let character = luascript_read_userdata(L, CHARACTER);
    let enabled = LUA.lua_toboolean(L, 2);

    character_trailing_enabled(character, enabled);

    return 0;
}

function script_character_trailing_set_params(L) {
    let character = luascript_read_userdata(L, CHARACTER);
    let length = LUA.luaL_optinteger(L, 2, -1);
    let trail_delay = LUA.luaL_optnumber(L, 3, NaN);
    let trail_alpha = LUA.luaL_optnumber(L, 4, NaN);

    let darken_colors = false;
    if (LUA.lua_isnil(L, 5))
        darken_colors = null;
    else
        darken_colors = LUA.lua_toboolean(L, 5);

    character_trailing_set_params(character, length, trail_delay, trail_alpha, darken_colors);

    return 0;
}

function script_character_trailing_set_offsetcolor(L) {
    let character = luascript_read_userdata(L, CHARACTER);
    let r = LUA.luaL_optnumber(L, 2, NaN);
    let g = LUA.luaL_optnumber(L, 3, NaN);
    let b = LUA.luaL_optnumber(L, 4, NaN);

    character_trailing_set_offsetcolor(character, r, g, b);

    return 0;
}

function script_character_schedule_idle(L) {
    let character = luascript_read_userdata(L, CHARACTER);

    character_schedule_idle(character);

    return 0;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/**@type {luaL_Reg[]}*/const CHARACTER_FUNCTIONS = [
    ["use_alternate_sing_animations", script_character_use_alternate_sing_animations],
    ["set_draw_location", script_character_set_draw_location],
    ["set_draw_align", script_character_set_draw_align],
    ["update_reference_size", script_character_update_reference_size],
    ["enable_reference_size", script_character_enable_reference_size],
    ["set_offset", script_character_set_offset],
    ["state_add", script_character_state_add],
    ["state_toggle", script_character_state_toggle],
    ["play_hey", script_character_play_hey],
    ["play_idle", script_character_play_idle],
    ["play_sing", script_character_play_sing],
    ["play_miss", script_character_play_miss],
    ["play_extra", script_character_play_extra],
    ["set_idle_speed", script_character_set_idle_speed],
    ["set_scale", script_character_set_scale],
    ["reset", script_character_reset],
    ["enable_continuous_idle", script_character_enable_continuous_idle],
    ["is_idle_active", script_character_is_idle_active],
    ["enable_flip_correction", script_character_enable_flip_correction],
    ["flip_orientation", script_character_flip_orientation],
    ["face_as_opponent", script_character_face_as_opponent],
    ["set_z_index", script_character_set_z_index],
    ["set_z_offset", script_character_set_z_offset],
    ["animation_set", script_character_animation_set],
    ["animation_restart", script_character_animation_restart],
    ["animation_end", script_character_animation_end],
    ["set_color_offset", script_character_set_color_offset],
    ["set_color_offset_to_default", script_character_set_color_offset_to_default],
    ["set_alpha", script_character_set_alpha],
    ["set_visible", script_character_set_visible],
    ["get_modifier", script_character_get_modifier],
    ["has_direction", script_character_has_direction],
    ["get_play_calls", script_character_get_play_calls],
    ["get_commited_animations_count", script_character_get_commited_animations_count],
    ["get_current_action", script_character_get_current_action],
    ["freeze_animation", script_character_freeze_animation],
    ["trailing_enabled", script_character_trailing_enabled],
    ["trailing_set_params", script_character_trailing_set_params],
    ["trailing_set_offsetcolor", script_character_trailing_set_offsetcolor],
    ["schedule_idle", script_character_schedule_idle],
    [null, null]
];


function script_character_new(L, character) {
    return luascript_userdata_new(L, CHARACTER, character);
}

function script_character_gc(L) {
    return luascript_userdata_gc(L, CHARACTER);
}

function script_character_tostring(L) {
    return luascript_userdata_tostring(L, CHARACTER);
}


function script_character_register(L) {
    luascript_register(L, CHARACTER, script_character_gc, script_character_tostring, CHARACTER_FUNCTIONS);
}
