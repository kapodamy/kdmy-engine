"use strict";

const DRAWABLE = "Drawable";


function script_drawable_set_z_index(L) {
    let drawable = luascript_read_userdata(L, DRAWABLE);
    let z_index = LUA.luaL_checknumber(L, 2);

    drawable_set_z_index(drawable, z_index);

    return 0;
}

function script_drawable_get_z_index(L) {
    let drawable = luascript_read_userdata(L, DRAWABLE);

    let ret = drawable_get_z_index(drawable);

    LUA.lua_pushnumber(L, ret);
    return 1;
}

function script_drawable_set_z_offset(L) {
    let drawable = luascript_read_userdata(L, DRAWABLE);
    let offset = LUA.luaL_checknumber(L, 2);

    drawable_set_z_offset(drawable, offset);

    return 0;
}

function script_drawable_set_alpha(L) {
    let drawable = luascript_read_userdata(L, DRAWABLE);
    let alpha = LUA.luaL_checknumber(L, 2);

    drawable_set_alpha(drawable, alpha);

    return 0;
}

function script_drawable_get_alpha(L) {
    let drawable = luascript_read_userdata(L, DRAWABLE);

    let ret = drawable_get_alpha(drawable);

    LUA.lua_pushnumber(L, ret);
    return 1;
}

function script_drawable_set_offsetcolor(L) {
    let drawable = luascript_read_userdata(L, DRAWABLE);
    let r = LUA.luaL_checknumber(L, 2);
    let g = LUA.luaL_checknumber(L, 3);
    let b = LUA.luaL_checknumber(L, 4);
    let a = LUA.luaL_checknumber(L, 5);

    drawable_set_offsetcolor(drawable, r, g, b, a);

    return 0;
}

function script_drawable_set_offsetcolor_to_default(L) {
    let drawable = luascript_read_userdata(L, DRAWABLE);

    drawable_set_offsetcolor_to_default(drawable);

    return 0;
}

function script_drawable_get_modifier(L) {
    let drawable = luascript_read_userdata(L, DRAWABLE);

    let ret = drawable_get_modifier(drawable);

    return script_modifier_new(L, ret);
}

function script_drawable_set_antialiasing(L) {
    let drawable = luascript_read_userdata(L, DRAWABLE);
    let antialiasing = luascript_helper_optenum(L, 2, LUASCRIPT_ENUMS_PVRFlag);

    drawable_set_antialiasing(drawable, antialiasing);

    return 0;
}

function script_drawable_set_shader(L) {
    let drawable = luascript_read_userdata(L, DRAWABLE);
    let psshader = luascript_read_nullable_userdata(L, 2, PSSHADER);

    drawable_set_shader(drawable, psshader);

    return 0;
}

function script_drawable_get_shader(L) {
    let drawable = luascript_read_userdata(L, DRAWABLE);

    let ret = drawable_get_shader(drawable);

    return script_psshader_new(L, ret);
}

function script_drawable_blend_enable(L) {
    let drawable = luascript_read_userdata(L, DRAWABLE);
    let enabled = LUA.lua_toboolean(L, 2);

    drawable_blend_enable(drawable, enabled);

    return 0;
}

function script_drawable_blend_set(L) {
    let drawable = luascript_read_userdata(L, DRAWABLE);
    let src_rgb = luascript_helper_optenum(L, 2, LUASCRIPT_ENUMS_Blend);
    let dst_rgb = luascript_helper_optenum(L, 3, LUASCRIPT_ENUMS_Blend);
    let src_alpha = luascript_helper_optenum(L, 4, LUASCRIPT_ENUMS_Blend);
    let dst_alpha = luascript_helper_optenum(L, 5, LUASCRIPT_ENUMS_Blend);

    drawable_blend_set(drawable, src_rgb, dst_rgb, src_alpha, dst_alpha);

    return 0;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/**@type {luaL_Reg[]}*/const DRAWABLE_FUNCTIONS = [
    ["set_z_index", script_drawable_set_z_index],
    ["get_z_index", script_drawable_get_z_index],
    ["set_z_offset", script_drawable_set_z_offset],
    ["set_alpha", script_drawable_set_alpha],
    ["get_alpha", script_drawable_get_alpha],
    ["set_offsetcolor", script_drawable_set_offsetcolor],
    ["set_offsetcolor_to_default", script_drawable_set_offsetcolor_to_default],
    ["get_modifier", script_drawable_get_modifier],
    ["set_antialiasing", script_drawable_set_antialiasing],
    ["set_shader", script_drawable_set_shader],
    ["get_shader", script_drawable_get_shader],
    ["blend_enable", script_drawable_blend_enable],
    ["blend_set", script_drawable_blend_set],
    [null, null]
];


function script_drawable_new(L, drawable) {
    return luascript_userdata_new(L, DRAWABLE, drawable);
}

function script_drawable_gc(L) {
    return luascript_userdata_gc(L, DRAWABLE);
}

function script_drawable_tostring(L) {
    return luascript_userdata_tostring(L, DRAWABLE);
}

function script_drawable_register(L) {
    luascript_register(L, DRAWABLE, script_drawable_gc, script_drawable_tostring, DRAWABLE_FUNCTIONS);
}
