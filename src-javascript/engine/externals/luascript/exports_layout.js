"use strict";

const LAYOUT = "Layout";


function script_layout_trigger_any(L) {
    let layout = luascript_read_userdata(L, LAYOUT);
    let action_triger_camera_trigger_name = LUA.luaL_optstring(L, 2, null);

    let ret = layout_trigger_any(layout, action_triger_camera_trigger_name);
    LUA.lua_pushinteger(L, ret);

    return 1;
}

function script_layout_trigger_action(L) {
    let layout = luascript_read_userdata(L, LAYOUT);
    let target_name = LUA.luaL_optstring(L, 2, null);
    let action_name = LUA.luaL_optstring(L, 3, null);

    let ret = layout_trigger_action(layout, target_name, action_name);
    LUA.lua_pushinteger(L, ret);

    return 1;
}

function script_layout_trigger_camera(L) {
    let layout = luascript_read_userdata(L, LAYOUT);
    let camera_name = LUA.luaL_optstring(L, 2, null);

    let ret = layout_trigger_camera(layout, camera_name);
    LUA.lua_pushboolean(L, ret);

    return 1;
}

function script_layout_trigger_trigger(L) {
    let layout = luascript_read_userdata(L, LAYOUT);
    let trigger_name = LUA.luaL_optstring(L, 2, null);

    let ret = layout_trigger_trigger(layout, trigger_name);
    LUA.lua_pushinteger(L, ret);

    return 1;
}

function script_layout_contains_action(L) {
    let layout = luascript_read_userdata(L, LAYOUT);
    let target_name = LUA.luaL_optstring(L, 2, null);
    let action_name = LUA.luaL_optstring(L, 3, null);

    let ret = layout_contains_action(layout, target_name, action_name);
    LUA.lua_pushinteger(L, ret);

    return 1;
}

function script_layout_stop_all_triggers(L) {
    let layout = luascript_read_userdata(L, LAYOUT);
    layout_stop_all_triggers(layout);
    return 0;
}

function script_layout_stop_trigger(L) {
    let layout = luascript_read_userdata(L, LAYOUT);
    let trigger_name = LUA.luaL_optstring(L, 2, null);

    layout_stop_trigger(layout, trigger_name);

    return 0;
}

function script_layout_animation_is_completed(L) {
    let layout = luascript_read_userdata(L, LAYOUT);
    let item_name = LUA.luaL_optstring(L, 2, null);

    let ret = layout_animation_is_completed(layout, item_name);
    LUA.lua_pushinteger(L, ret);

    return 1;
}

function script_layout_camera_set_view(L) {
    let layout = luascript_read_userdata(L, LAYOUT);
    let x = LUA.luaL_checknumber(L, 2);
    let y = LUA.luaL_checknumber(L, 3);
    let z = LUA.luaL_checknumber(L, 4);

    layout_camera_set_view(layout, x, y, z);

    return 0;
}

function script_layout_camera_is_completed(L) {
    let layout = luascript_read_userdata(L, LAYOUT);

    let ret = layout_camera_is_completed(layout);
    LUA.lua_pushboolean(L, ret);

    return 1;
}

function script_layout_get_camera_helper(L) {
    let layout = luascript_read_userdata(L, LAYOUT);

    let camera = layout_get_camera_helper(layout);

    return script_camera_new(L, camera);
}

function script_layout_get_secondary_camera_helper(L) {
    let layout = luascript_read_userdata(L, LAYOUT);

    let camera = layout_get_secondary_camera_helper(layout);

    return script_camera_new(L, camera);
}

function script_layout_get_textsprite(L) {
    let layout = luascript_read_userdata(L, LAYOUT);

    let name = LUA.luaL_optstring(L, 2, null);

    let textsprite = layout_get_textsprite(layout, name);

    return script_textsprite_new(L, textsprite);
}

function script_layout_get_soundplayer(L) {
    let layout = luascript_read_userdata(L, LAYOUT);

    let name = LUA.luaL_optstring(L, 2, null);

    let soundplayer = layout_get_soundplayer(layout, name);

    return script_soundplayer_new(L, soundplayer);
}

function script_layout_get_videoplayer(L) {
    let layout = luascript_read_userdata(L, LAYOUT);
    let name = LUA.luaL_optstring(L, 2, null);

    let ret = layout_get_videoplayer(layout, name);

    return script_videoplayer_new(L, ret);
}

function script_layout_get_sprite(L) {
    let layout = luascript_read_userdata(L, LAYOUT);

    let name = LUA.luaL_optstring(L, 2, null);

    let sprite = layout_get_sprite(layout, name);

    return script_sprite_new(L, sprite);
}

function script_layout_get_viewport_size(L) {
    let layout = luascript_read_userdata(L, LAYOUT);

    const size = [0.0, 0.0];
    layout_get_viewport_size(layout, size);

    LUA.lua_pushnumber(L, size[0]);
    LUA.lua_pushnumber(L, size[1]);

    return 2;
}

function script_layout_get_attached_value(L) {
    let layout = luascript_read_userdata(L, LAYOUT);
    let name = LUA.luaL_optstring(L, 2, null);

    if (name == null) {
        LUA.lua_pushnil(L);
        return 1;
    }

    /**@type {[any]}*/let value = [null];
    let value_type = layout_get_attached_value2(layout, name, value);

    switch (value_type) {
        case LAYOUT_TYPE_STRING:
            LUA.lua_pushstring(L, value[0]);
            break;
        case LAYOUT_TYPE_DOUBLE:
            LUA.lua_pushnumber(L, value[0]);
            break;
        case LAYOUT_TYPE_INTEGER:
            LUA.lua_pushinteger(L, value[0]);
            break;
        case LAYOUT_TYPE_HEX:
            LUA.lua_pushinteger(L, value[0]);
            break;
        case LAYOUT_TYPE_BOOLEAN:
            LUA.lua_pushboolean(L, value[0]);
            break;
        case LAYOUT_TYPE_NOTFOUND:
        default:
            LUA.lua_pushvalue(L, 3); // default value (taken from the arguments)
            break;
    }

    return 1;
}

function script_layout_set_group_visibility(L) {
    let layout = luascript_read_userdata(L, LAYOUT);
    let group_name = LUA.luaL_optstring(L, 2, null);
    let visible = LUA.lua_toboolean(L, 3);

    layout_set_group_visibility(layout, group_name, visible);

    return 0;
}

function script_layout_set_group_alpha(L) {
    let layout = luascript_read_userdata(L, LAYOUT);
    let group_name = LUA.luaL_optstring(L, 2, null);
    let alpha = LUA.luaL_checknumber(L, 3);

    layout_set_group_alpha(layout, group_name, alpha);

    return 0;
}

function script_layout_set_group_offsetcolor(L) {
    let layout = luascript_read_userdata(L, LAYOUT);
    let group_name = LUA.luaL_optstring(L, 2, null);
    let r = LUA.luaL_optnumber(L, 3, NaN);
    let g = LUA.luaL_optnumber(L, 4, NaN);
    let b = LUA.luaL_optnumber(L, 5, NaN);
    let a = LUA.luaL_optnumber(L, 6, NaN);

    layout_set_group_offsetcolor(layout, group_name, r, g, b, a);

    return 0;
}

function script_layout_get_group_visibility(L) {
    let layout = luascript_read_userdata(L, LAYOUT);
    let group_name = LUA.luaL_optstring(L, 2, null);

    let ret = layout_get_group_visibility(layout, group_name);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_layout_suspend(L) {
    let layout = luascript_read_userdata(L, LAYOUT);
    layout_suspend(layout);
    return 0;
}

function script_layout_resume(L) {
    let layout = luascript_read_userdata(L, LAYOUT);
    layout_resume(layout);
    return 0;
}

function script_layout_get_placeholder(L) {
    let layout = luascript_read_userdata(L, LAYOUT);
    let name = LUA.luaL_optstring(L, 2, null);

    let layoutplaceholder = layout_get_placeholder(layout, name);

    return script_layoutplaceholder_new(L, layoutplaceholder);
}

function script_layout_disable_antialiasing(L) {
    let layout = luascript_read_userdata(L, LAYOUT);

    let disable = LUA.lua_toboolean(L, 3);

    layout_disable_antialiasing(layout, disable);

    return 0;
}

function script_layout_set_group_antialiasing(L) {
    let layout = luascript_read_userdata(L, LAYOUT);

    let group_name = LUA.luaL_optstring(L, 2, null);
    let antialiasing = luascript_helper_optenum(L, 3, LUASCRIPT_ENUMS_PVRFlag);

    layout_set_group_antialiasing(layout, group_name, antialiasing);

    return 0;
}

function script_layout_get_group_modifier(L) {
    let layout = luascript_read_userdata(L, LAYOUT);

    let group_name = LUA.luaL_optstring(L, 2, null);

    let modifier = layout_get_group_modifier(layout, group_name);

    return script_modifier_new(L, modifier);
}

function script_layout_get_group_shader(L) {
    let layout = luascript_read_userdata(L, LAYOUT);

    let name = LUA.luaL_optstring(L, 2, null);

    let psshader = layout_get_group_shader(layout, name);

    return script_psshader_new(L, psshader);
}

function script_layout_set_group_shader(L) {
    let layout = luascript_read_userdata(L, LAYOUT);

    let name = LUA.luaL_optstring(L, 2, null);
    let psshader = luascript_read_nullable_userdata(L, 3, PSSHADER);

    let ret = layout_set_group_shader(layout, name, psshader);

    LUA.lua_pushboolean(L, ret);

    return 1;
}

function script_layout_screen_to_layout_coordinates(L) {
    let layout = luascript_read_userdata(L, LAYOUT);

    let screen_x = LUA.luaL_checknumber(L, 2);
    let screen_y = LUA.luaL_checknumber(L, 3);
    let calc_with_camera = LUA.lua_toboolean(L, 4);

    const output_coords = [0.0, 0.0];

    layout_screen_to_layout_coordinates(layout, screen_x, screen_y, calc_with_camera, output_coords);

    LUA.lua_pushnumber(L, output_coords[0]);
    LUA.lua_pushnumber(L, output_coords[1]);
    return 2;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/**@type {luaL_Reg[]}*/const LAYOUT_FUNCTIONS = [
    ["trigger_any", script_layout_trigger_any],
    ["trigger_action", script_layout_trigger_action],
    ["trigger_camera", script_layout_trigger_camera],
    ["trigger_trigger", script_layout_trigger_trigger],
    ["contains_action", script_layout_contains_action],
    ["stop_all_triggers", script_layout_stop_all_triggers],
    ["stop_trigger", script_layout_stop_trigger],
    ["animation_is_completed", script_layout_animation_is_completed],
    ["camera_set_view", script_layout_camera_set_view],
    ["camera_is_completed", script_layout_camera_is_completed],
    ["get_camera_helper", script_layout_get_camera_helper],
    ["get_secondary_camera_helper", script_layout_get_secondary_camera_helper],
    ["get_textsprite", script_layout_get_textsprite],
    ["get_sprite", script_layout_get_sprite],
    ["get_soundplayer", script_layout_get_soundplayer],
    ["get_videoplayer", script_layout_get_videoplayer],
    ["get_viewport_size", script_layout_get_viewport_size],
    ["get_attached_value", script_layout_get_attached_value],
    ["set_group_visibility", script_layout_set_group_visibility],
    ["set_group_alpha", script_layout_set_group_alpha],
    ["set_group_offsetcolor", script_layout_set_group_offsetcolor],
    ["get_group_visibility", script_layout_get_group_visibility],
    ["suspend", script_layout_suspend],
    ["resume", script_layout_resume],
    ["get_placeholder", script_layout_get_placeholder],
    ["disable_antialiasing", script_layout_disable_antialiasing],
    ["set_group_antialiasing", script_layout_set_group_antialiasing],
    ["get_group_modifier", script_layout_get_group_modifier],
    ["get_group_shader", script_layout_get_group_shader],
    ["set_group_shader", script_layout_set_group_shader],
    ["screen_to_layout_coordinates", script_layout_screen_to_layout_coordinates],
    [null, null]
];


function script_layout_new(L, layout) {
    return luascript_userdata_new(L, LAYOUT, layout);
}

function script_layout_gc(L) {
    return luascript_userdata_gc(L, LAYOUT);
}

function script_layout_tostring(L) {
    return luascript_userdata_tostring(L, LAYOUT);
}


function script_layout_register(L) {
    luascript_register(L, LAYOUT, script_layout_gc, script_layout_tostring, LAYOUT_FUNCTIONS);
}
