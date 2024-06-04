"use strict";

const CAMERA = "Camera";


function script_camera_set_interpolator_type(L) {
    let camera = luascript_read_userdata(L, CAMERA);

    let type = luascript_helper_optenum(L, 2, LUASCRIPT_ENUMS_AnimInterpolator);

    camera_set_interpolator_type(camera, type);

    return 0;
}

function script_camera_set_transition_duration(L) {
    let camera = luascript_read_userdata(L, CAMERA);

    let expresed_in_beats = LUA.lua_toboolean(L, 2);
    let value = LUA.luaL_checknumber(L, 3);

    camera_set_transition_duration(camera, expresed_in_beats, value);

    return 0;
}

function script_camera_set_absolute_zoom(L) {
    let camera = luascript_read_userdata(L, CAMERA);

    let z = LUA.luaL_checknumber(L, 2);

    camera_set_absolute_zoom(camera, z);

    return 0;
}

function script_camera_set_absolute_position(L) {
    let camera = luascript_read_userdata(L, CAMERA);

    let x = LUA.luaL_optnumber(L, 2, NaN);
    let y = LUA.luaL_optnumber(L, 3, NaN);

    camera_set_absolute_position(camera, x, y);

    return 0;
}

function script_camera_set_offset(L) {
    let camera = luascript_read_userdata(L, CAMERA);

    let x = LUA.luaL_optnumber(L, 2, NaN);
    let y = LUA.luaL_optnumber(L, 3, NaN);
    let z = LUA.luaL_optnumber(L, 4, NaN);

    camera_set_offset(camera, x, y, z);

    return 0;
}

function script_camera_get_offset(L) {
    let camera = luascript_read_userdata(L, CAMERA);

    const xyz = [0.0, 0.0, 0.0];

    camera_get_offset(camera, xyz);

    LUA.lua_pushnumber(L, xyz[0]);
    LUA.lua_pushnumber(L, xyz[1]);
    LUA.lua_pushnumber(L, xyz[2]);

    return 3;
}

function script_camera_get_modifier(L) {
    let camera = luascript_read_userdata(L, CAMERA);

    let modifier = camera_get_modifier(camera);
    return script_modifier_new(L, modifier);
}

function script_camera_move(L) {
    let camera = luascript_read_userdata(L, CAMERA);

    let x = LUA.luaL_optnumber(L, 2, NaN);
    let y = LUA.luaL_optnumber(L, 3, NaN);
    let z = LUA.luaL_optnumber(L, 4, NaN);

    camera_move(camera, x, y, z);

    return 0;
}

function script_camera_slide(L) {
    let camera = luascript_read_userdata(L, CAMERA);

    let start_x = LUA.luaL_optnumber(L, 2, NaN);
    let start_y = LUA.luaL_optnumber(L, 3, NaN);
    let start_z = LUA.luaL_optnumber(L, 4, NaN);
    let end_x = LUA.luaL_optnumber(L, 5, NaN);
    let end_y = LUA.luaL_optnumber(L, 6, NaN);
    let end_z = LUA.luaL_optnumber(L, 7, NaN);

    camera_slide(camera, start_x, start_y, start_z, end_x, end_y, end_z);

    return 0;
}

function script_camera_slide_x(L) {
    let camera = luascript_read_userdata(L, CAMERA);

    let start = LUA.luaL_optnumber(L, 2, NaN);
    let end = LUA.luaL_optnumber(L, 3, NaN);

    camera_slide_x(camera, start, end);

    return 0;
}

function script_camera_slide_y(L) {
    let camera = luascript_read_userdata(L, CAMERA);

    let start = LUA.luaL_optnumber(L, 2, NaN);
    let end = LUA.luaL_optnumber(L, 3, NaN);

    camera_slide_y(camera, start, end);

    return 0;
}

function script_camera_slide_z(L) {
    let camera = luascript_read_userdata(L, CAMERA);

    let start = LUA.luaL_optnumber(L, 2, NaN);
    let end = LUA.luaL_optnumber(L, 3, NaN);

    camera_slide_z(camera, start, end);

    return 0;
}

function script_camera_slide_to(L) {
    let camera = luascript_read_userdata(L, CAMERA);

    let x = LUA.luaL_optnumber(L, 2, NaN);
    let y = LUA.luaL_optnumber(L, 3, NaN);
    let z = LUA.luaL_optnumber(L, 4, NaN);

    camera_slide_to(camera, x, y, z);

    return 0;
}

function script_camera_from_layout(L) {
    let camera = luascript_read_userdata(L, CAMERA);

    let camera_name = LUA.luaL_optstring(L, 2, null);
    let layout = camera_get_parent_layout(camera);

    if (!layout) {
        LUA.lua_pushboolean(L, false);
        return 1;
    }

    let ret = camera_from_layout(camera, layout, camera_name);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_camera_disable_offset_zoom(L) {
    let camera = luascript_read_userdata(L, CAMERA);

    let disable = LUA.lua_toboolean(L, 2);

    camera_disable_offset_zoom(camera, disable);

    return 0;
}

function script_camera_to_origin(L) {
    let camera = luascript_read_userdata(L, CAMERA);

    let should_slide = LUA.lua_toboolean(L, 2);

    camera_to_origin(camera, should_slide);

    return 0;
}

function script_camera_repeat(L) {
    let camera = luascript_read_userdata(L, CAMERA);

    camera_repeat(camera);

    return 0;
}

function script_camera_stop(L) {
    let camera = luascript_read_userdata(L, CAMERA);

    camera_stop(camera);

    return 0;
}

function script_camera_end(L) {
    let camera = luascript_read_userdata(L, CAMERA);

    camera_end(camera);

    return 0;
}

function script_camera_is_completed(L) {
    let camera = luascript_read_userdata(L, CAMERA);

    let ret = camera_is_completed(camera);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_camera_debug_log_info(L) {
    let camera = luascript_read_userdata(L, CAMERA);

    camera_debug_log_info(camera);

    return 0;
}

function script_camera_apply(L) {
    let camera = luascript_read_userdata(L, CAMERA);

    camera_apply(camera, null);

    return 0;
}

function script_camera_move_offset(L) {
    let camera = luascript_read_userdata(L, CAMERA);

    let end_x = LUA.luaL_optnumber(L, 2, NaN);
    let end_y = LUA.luaL_optnumber(L, 3, NaN);
    let end_z = LUA.luaL_optnumber(L, 4, NaN);

    camera_move_offset(camera, end_x, end_y, end_z);

    return 0;
}

function script_camera_slide_offset(L) {
    let camera = luascript_read_userdata(L, CAMERA);

    let start_x = LUA.luaL_optnumber(L, 2, NaN);
    let start_y = LUA.luaL_optnumber(L, 3, NaN);
    let start_z = LUA.luaL_optnumber(L, 4, NaN);
    let end_x = LUA.luaL_optnumber(L, 5, NaN);
    let end_y = LUA.luaL_optnumber(L, 6, NaN);
    let end_z = LUA.luaL_optnumber(L, 7, NaN);

    camera_slide_offset(camera, start_x, start_y, start_z, end_x, end_y, end_z);

    return 0;
}

function script_camera_slide_x_offset(L) {
    let camera = luascript_read_userdata(L, CAMERA);

    let start = LUA.luaL_optnumber(L, 2, NaN);
    let end = LUA.luaL_optnumber(L, 3, NaN);

    camera_slide_x_offset(camera, start, end);

    return 0;
}

function script_camera_slide_y_offset(L) {
    let camera = luascript_read_userdata(L, CAMERA);

    let start = LUA.luaL_optnumber(L, 2, NaN);
    let end = LUA.luaL_optnumber(L, 3, NaN);

    camera_slide_y_offset(camera, start, end);

    return 0;
}

function script_camera_slide_z_offset(L) {
    let camera = luascript_read_userdata(L, CAMERA);

    let start = LUA.luaL_optnumber(L, 2, NaN);
    let end = LUA.luaL_optnumber(L, 3, NaN);

    camera_slide_z_offset(camera, start, end);

    return 0;
}

function script_camera_slide_to_offset(L) {
    let camera = luascript_read_userdata(L, CAMERA);

    let x = LUA.luaL_optnumber(L, 2, NaN);
    let y = LUA.luaL_optnumber(L, 3, NaN);
    let z = LUA.luaL_optnumber(L, 4, NaN);

    camera_slide_to_offset(camera, x, y, z);

    return 0;
}

function script_camera_to_origin_offset(L) {
    let camera = luascript_read_userdata(L, CAMERA);

    let should_slide = LUA.lua_toboolean(L, 2);

    camera_to_origin_offset(camera, should_slide);

    return 0;
}

function script_camera_set_animation(L) {
    let camera = luascript_read_userdata(L, CAMERA);

    let animsprite = luascript_read_nullable_userdata(L, 2, ANIMSPRITE);

    camera_set_animation(camera, animsprite);

    return 0;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/**@type {luaL_Reg[]}*/const CAMERA_FUNCTIONS = [
    ["set_interpolator_type", script_camera_set_interpolator_type],
    ["set_transition_duration", script_camera_set_transition_duration],
    ["set_absolute_zoom", script_camera_set_absolute_zoom],
    ["set_absolute_position", script_camera_set_absolute_position],
    ["set_offset", script_camera_set_offset],
    ["get_offset", script_camera_get_offset],
    ["get_modifier", script_camera_get_modifier],
    ["move", script_camera_move],
    ["slide", script_camera_slide],
    ["slide_x", script_camera_slide_x],
    ["slide_y", script_camera_slide_y],
    ["slide_z", script_camera_slide_z],
    ["slide_to", script_camera_slide_to],
    ["from_layout", script_camera_from_layout],
    ["disable_offset_zoom", script_camera_disable_offset_zoom],
    ["to_origin", script_camera_to_origin],
    ["repeat", script_camera_repeat],
    ["stop", script_camera_stop],
    ["end", script_camera_end],
    ["is_completed", script_camera_is_completed],
    ["debug_log_info", script_camera_debug_log_info],
    ["apply", script_camera_apply],
    ["move_offset", script_camera_move_offset],
    ["slide_offset", script_camera_slide_offset],
    ["slide_x_offset", script_camera_slide_x_offset],
    ["slide_y_offset", script_camera_slide_y_offset],
    ["slide_z_offset", script_camera_slide_z_offset],
    ["slide_to_offset", script_camera_slide_to_offset],
    ["to_origin_offset", script_camera_to_origin_offset],
    ["set_animation", script_camera_set_animation],
    [null, null]
];


function script_camera_new(L, camera) {
    return luascript_userdata_new(L, CAMERA, camera);
}

function script_camera_gc(L) {
    return luascript_userdata_gc(L, CAMERA);
}

function script_camera_tostring(L) {
    return luascript_userdata_tostring(L, CAMERA);
}


function script_camera_register(L) {
    luascript_register(L, CAMERA, script_camera_gc, script_camera_tostring, CAMERA_FUNCTIONS);
}
