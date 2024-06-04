"use strict";

const MODELHOLDER = "ModelHolder";


async function script_modelholder_init(L) {
    let src = LUA.luaL_checkstring(L, 2);

    luascript_change_working_folder(L);
    let ret = await modelholder_init(src);
    luascript_restore_working_folder(L);

    return luascript_userdata_allocnew(L, MODELHOLDER, ret);
}

function script_modelholder_init2(L) {
    let vertex_color_rgb8 = LUA.luaL_checkinteger(L, 2);
    let atlas_src = LUA.luaL_optstring(L, 3, null);
    let animlist_src = LUA.luaL_optstring(L, 4, null);

    luascript_change_working_folder(L);
    let ret = modelholder_init2(vertex_color_rgb8, atlas_src, animlist_src);
    luascript_restore_working_folder(L);

    return luascript_userdata_allocnew(L, MODELHOLDER, ret);
}

function script_modelholder_destroy(L) {
    let modelholder = luascript_read_userdata(L, MODELHOLDER);

    if (luascript_userdata_is_allocated(L, MODELHOLDER))
        modelholder_destroy(modelholder);
    else
        console.error("script_modelholder_destroy() object was not allocated by lua");

    return 0;
}

function script_modelholder_is_invalid(L) {
    let modelholder = luascript_read_userdata(L, MODELHOLDER);

    let ret = modelholder_is_invalid(modelholder);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_modelholder_has_animlist(L) {
    let modelholder = luascript_read_userdata(L, MODELHOLDER);

    let ret = modelholder_has_animlist(modelholder);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_modelholder_create_animsprite(L) {
    let modelholder = luascript_read_userdata(L, MODELHOLDER);
    let animation_name = LUA.luaL_optstring(L, 2, null);
    let fallback_static = LUA.lua_toboolean(L, 3);
    let no_return_null = LUA.lua_toboolean(L, 4);

    let ret = modelholder_create_animsprite(modelholder, animation_name, fallback_static, no_return_null);

    return luascript_userdata_allocnew(L, ANIMSPRITE, ret);
}

function script_modelholder_get_atlas(L) {
    let modelholder = luascript_read_userdata(L, MODELHOLDER);

    let ret = modelholder_get_atlas(modelholder);

    return script_atlas_new(L, ret);
}

function script_modelholder_get_vertex_color(L) {
    let modelholder = luascript_read_userdata(L, MODELHOLDER);

    let ret = modelholder_get_vertex_color(modelholder);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_modelholder_get_animlist(L) {
    let modelholder = luascript_read_userdata(L, MODELHOLDER);

    let ret = modelholder_get_animlist(modelholder);

    return script_animlist_new(L, ret);
}

function script_modelholder_get_atlas_entry(L) {
    let modelholder = luascript_read_userdata(L, MODELHOLDER);
    let atlas_entry_name = LUA.luaL_optstring(L, 2, null);

    let ret = modelholder_get_atlas_entry(modelholder, atlas_entry_name);

    return luascript_helper_push_atlas_entry(L, ret);
}

function script_modelholder_get_texture_resolution(L) {
    let modelholder = luascript_read_userdata(L, MODELHOLDER);

    const output_resolution = [0, 0];
    modelholder_get_texture_resolution(modelholder, output_resolution);

    LUA.lua_pushinteger(L, output_resolution[0]);
    LUA.lua_pushinteger(L, output_resolution[1]);
    return 2;
}

function script_modelholder_utils_is_known_extension(L) {
    let filename = LUA.luaL_checkstring(L, 2);

    let ret = modelholder_utils_is_known_extension(filename);

    LUA.lua_pushboolean(L, ret);
    return 1;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/**@type {luaL_Reg[]}*/const MODELHOLDER_FUNCTIONS = [
    ["init", script_modelholder_init],
    ["init2", script_modelholder_init2],
    ["destroy", script_modelholder_destroy],
    ["is_invalid", script_modelholder_is_invalid],
    ["has_animlist", script_modelholder_has_animlist],
    ["create_animsprite", script_modelholder_create_animsprite],
    ["get_atlas", script_modelholder_get_atlas],
    ["get_vertex_color", script_modelholder_get_vertex_color],
    ["get_animlist", script_modelholder_get_animlist],
    ["get_atlas_entry", script_modelholder_get_atlas_entry],
    ["get_texture_resolution", script_modelholder_get_texture_resolution],
    ["utils_is_known_extension", script_modelholder_utils_is_known_extension],
    [null, null]
];

function script_modelholder_new(L, modelholder) {
    return luascript_userdata_new(L, MODELHOLDER, modelholder);
}

function script_modelholder_gc(L) {
    return luascript_userdata_destroy(L, MODELHOLDER, modelholder_destroy);
}

function script_modelholder_tostring(L) {
    return luascript_userdata_tostring(L, MODELHOLDER);
}

function script_modelholder_register(L) {
    luascript_register(L, MODELHOLDER, script_modelholder_gc, script_modelholder_tostring, MODELHOLDER_FUNCTIONS);
}

