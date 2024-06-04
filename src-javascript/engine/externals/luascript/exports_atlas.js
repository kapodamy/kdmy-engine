"use strict";

const ATLAS = "Atlas";


function luascript_helper_push_atlas_entry(L, entry) {
    if (!entry) {
        LUA.lua_pushnil(L);
        return 1;
    }

    LUA.lua_createtable(L, 0, 11);

    luascript_helper_add_table_field(L, "name", LUA.lua_pushstring, entry.name);
    luascript_helper_add_table_field(L, "x", LUA.lua_pushnumber, entry.x);
    luascript_helper_add_table_field(L, "y", LUA.lua_pushnumber, entry.y);
    luascript_helper_add_table_field(L, "width", LUA.lua_pushnumber, entry.width);
    luascript_helper_add_table_field(L, "height", LUA.lua_pushnumber, entry.height);
    luascript_helper_add_table_field(L, "frameX", LUA.lua_pushnumber, entry.frame_x);
    luascript_helper_add_table_field(L, "frameY", LUA.lua_pushnumber, entry.frame_y);
    luascript_helper_add_table_field(L, "frameWidth", LUA.lua_pushnumber, entry.frame_width);
    luascript_helper_add_table_field(L, "frameHeight", LUA.lua_pushnumber, entry.frame_height);
    luascript_helper_add_table_field(L, "pivotX", LUA.lua_pushnumber, entry.pivot_x);
    luascript_helper_add_table_field(L, "pivotY", LUA.lua_pushnumber, entry.pivot_y);

    return 1;
}


async function script_atlas_init(L) {
    let src = LUA.luaL_checkstring(L, 2);

    luascript_change_working_folder(L);
    let ret = await atlas_init(src);
    luascript_restore_working_folder(L);

    return luascript_userdata_allocnew(L, ATLAS, ret);
}

function script_atlas_destroy(L) {
    let atlas = luascript_read_userdata(L, ATLAS);

    if (luascript_userdata_is_allocated(L, ATLAS))
        atlas_destroy(atlas);
    else
        console.error("script_atlas_destroy() object was not allocated by lua");

    return 0;
}

function script_atlas_get_index_of(L) {
    let atlas = luascript_read_userdata(L, ATLAS);
    let name = LUA.luaL_optstring(L, 2, null);

    let ret = atlas_get_index_of(atlas, name);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_atlas_get_entry(L) {
    let atlas = luascript_read_userdata(L, ATLAS);
    let name = LUA.luaL_optstring(L, 2, null);

    let ret = atlas_get_entry(atlas, name);

    return luascript_helper_push_atlas_entry(L, ret);
}

function script_atlas_get_entry_with_number_suffix(L) {
    let atlas = luascript_read_userdata(L, ATLAS);
    let name_prefix = LUA.luaL_optstring(L, 2, null);

    let ret = atlas_get_entry_with_number_suffix(atlas, name_prefix);

    return luascript_helper_push_atlas_entry(L, ret);
}

function script_atlas_get_glyph_fps(L) {
    let atlas = luascript_read_userdata(L, ATLAS);

    let ret = atlas_get_glyph_fps(atlas);

    LUA.lua_pushnumber(L, ret);
    return 1;
}

function script_atlas_get_texture_resolution(L) {
    let atlas = luascript_read_userdata(L, ATLAS);
    const resolution = [0.0, 0.0];

    atlas_get_texture_resolution(atlas, resolution);

    LUA.lua_pushinteger(L, resolution[0]);
    LUA.lua_pushinteger(L, resolution[1]);
    return 2;
}

function script_atlas_utils_is_known_extension(L) {
    let src = LUA.luaL_checkstring(L, 1);

    let ret = atlas_utils_is_known_extension(src);

    LUA.lua_pushboolean(L, ret);
    return 1;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/**@type {luaL_Reg[]}*/const ATLAS_FUNCTIONS = [
    ["init", script_atlas_init],
    ["destroy", script_atlas_destroy],
    ["get_index_of", script_atlas_get_index_of],
    ["get_entry", script_atlas_get_entry],
    ["get_entry_with_number_suffix", script_atlas_get_entry_with_number_suffix],
    ["get_glyph_fps", script_atlas_get_glyph_fps],
    ["get_texture_resolution", script_atlas_get_texture_resolution],
    ["utils_is_known_extension", script_atlas_utils_is_known_extension],
    [null, null]
];


function script_atlas_new(L, atlas) {
    return luascript_userdata_new(L, ATLAS, atlas);
}

function script_atlas_gc(L) {
    return luascript_userdata_destroy(L, ATLAS, atlas_destroy);
}

function script_atlas_tostring(L) {
    return luascript_userdata_tostring(L, ATLAS);
}

function script_atlas_register(L) {
    luascript_register(L, ATLAS, script_atlas_gc, script_atlas_tostring, ATLAS_FUNCTIONS);
}
