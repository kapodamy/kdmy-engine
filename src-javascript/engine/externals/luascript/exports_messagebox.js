"use strict";

const MESSAGEBOX = "MessageBox";


function script_messagebox_set_buttons_text(L) {
    let messagebox = luascript_read_userdata(L, MESSAGEBOX);

    let left_text = LUA.luaL_optstring(L, 2, null);
    let right_text = LUA.luaL_optstring(L, 3, null);

    messagebox_set_buttons_text(messagebox, left_text, right_text);

    return 0;
}

function script_messagebox_set_button_single(L) {
    let messagebox = luascript_read_userdata(L, MESSAGEBOX);

    let center_text = LUA.luaL_optstring(L, 2, null);

    messagebox_set_button_single(messagebox, center_text);

    return 0;
}

function script_messagebox_set_buttons_icons(L) {
    let messagebox = luascript_read_userdata(L, MESSAGEBOX);

    let left_icon_name = LUA.luaL_optstring(L, 2, null);
    let right_icon_name = LUA.luaL_optstring(L, 3, null);

    messagebox_set_buttons_icons(messagebox, left_icon_name, right_icon_name);

    return 0;
}

function script_messagebox_set_button_single_icon(L) {
    let messagebox = luascript_read_userdata(L, MESSAGEBOX);

    let center_icon_name = LUA.luaL_optstring(L, 2, null);

    messagebox_set_button_single_icon(messagebox, center_icon_name);

    return 0;
}

function script_messagebox_set_title(L) {
    let messagebox = luascript_read_userdata(L, MESSAGEBOX);

    let text = LUA.luaL_optstring(L, 2, null);

    messagebox_set_title(messagebox, text);

    return 0;
}

function script_messagebox_set_image_background_color(L) {
    let messagebox = luascript_read_userdata(L, MESSAGEBOX);

    let color_rgb8 = LUA.luaL_checkinteger(L, 2);

    messagebox_set_image_background_color(messagebox, color_rgb8);

    return 0;
}

function script_messagebox_set_image_background_color_default(L) {
    let messagebox = luascript_read_userdata(L, MESSAGEBOX);

    messagebox_set_image_background_color_default(messagebox);

    return 0;
}

function script_messagebox_set_message(L) {
    let messagebox = luascript_read_userdata(L, MESSAGEBOX);

    let text = LUA.luaL_optstring(L, 2, null);

    messagebox_set_message(messagebox, text);

    return 0;
}

function script_messagebox_hide_image_background(L) {
    let messagebox = luascript_read_userdata(L, MESSAGEBOX);

    let hide = LUA.lua_toboolean(L, 2);

    messagebox_hide_image_background(messagebox, hide);

    return 0;
}

function script_messagebox_hide_image(L) {
    let messagebox = luascript_read_userdata(L, MESSAGEBOX);

    let hide = LUA.lua_toboolean(L, 2);

    messagebox_hide_image(messagebox, hide);

    return 0;
}

function script_messagebox_show_buttons_icons(L) {
    let messagebox = luascript_read_userdata(L, MESSAGEBOX);

    let show = LUA.lua_toboolean(L, 2);

    messagebox_show_buttons_icons(messagebox, show);

    return 0;
}

function script_messagebox_use_small_size(L) {
    let messagebox = luascript_read_userdata(L, MESSAGEBOX);

    let small_or_normal = LUA.lua_toboolean(L, 2);

    messagebox_use_small_size(messagebox, small_or_normal);

    return 0;
}

async function script_messagebox_set_image_from_texture(L) {
    let messagebox = luascript_read_userdata(L, MESSAGEBOX);

    luascript_change_working_folder(L);
    let filename = LUA.luaL_optstring(L, 2, null);
    luascript_restore_working_folder(L);

    await messagebox_set_image_from_texture(messagebox, filename);

    return 0;
}

async function script_messagebox_set_image_from_atlas(L) {
    let messagebox = luascript_read_userdata(L, MESSAGEBOX);

    let filename = LUA.luaL_optstring(L, 2, null);
    let entry_name = LUA.luaL_optstring(L, 3, null);
    let is_animation = LUA.luaL_optstring(L, 4, null);

    luascript_change_working_folder(L);
    await messagebox_set_image_from_atlas(messagebox, filename, entry_name, is_animation);
    luascript_restore_working_folder(L);

    return 0;
}

function script_messagebox_hide(L) {
    let messagebox = luascript_read_userdata(L, MESSAGEBOX);

    let animated = LUA.lua_toboolean(L, 2);

    messagebox_hide(messagebox, animated);

    return 0;
}

function script_messagebox_show(L) {
    let messagebox = luascript_read_userdata(L, MESSAGEBOX);

    let animated = LUA.lua_toboolean(L, 2);

    messagebox_show(messagebox, animated);

    return 0;
}

function script_messagebox_set_z_index(L) {
    let messagebox = luascript_read_userdata(L, MESSAGEBOX);

    let z_index = LUA.luaL_checknumber(L, 2);

    messagebox_set_z_index(messagebox, z_index);

    return 0;
}

function script_messagebox_get_modifier(L) {
    let messagebox = luascript_read_userdata(L, MESSAGEBOX);

    let modifier = messagebox_get_modifier(messagebox);

    return script_modifier_new(L, modifier);
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/**@type {luaL_Reg[]}*/const MESSAGEBOX_FUNCTIONS = [
    ["set_buttons_text", script_messagebox_set_buttons_text],
    ["set_button_single", script_messagebox_set_button_single],
    ["set_buttons_icons", script_messagebox_set_buttons_icons],
    ["set_button_single_icon", script_messagebox_set_button_single_icon],
    ["set_title", script_messagebox_set_title],
    ["set_image_background_color", script_messagebox_set_image_background_color],
    ["set_image_background_color_default", script_messagebox_set_image_background_color_default],
    ["set_message", script_messagebox_set_message],
    ["hide_image_background", script_messagebox_hide_image_background],
    ["hide_image", script_messagebox_hide_image],
    ["show_buttons_icons", script_messagebox_show_buttons_icons],
    ["use_small_size", script_messagebox_use_small_size],
    ["set_image_from_texture", script_messagebox_set_image_from_texture],
    ["set_image_from_atlas", script_messagebox_set_image_from_atlas],
    ["hide", script_messagebox_hide],
    ["show", script_messagebox_show],
    ["set_z_index", script_messagebox_set_z_index],
    ["get_modifier", script_messagebox_get_modifier],
    [null, null]
];

function script_messagebox_new(L, messagebox) {
    return luascript_userdata_new(L, MESSAGEBOX, messagebox);
}

function script_messagebox_gc(L) {
    return luascript_userdata_gc(L, MESSAGEBOX);
}

function script_messagebox_tostring(L) {
    return luascript_userdata_tostring(L, MESSAGEBOX);
}


function script_messagebox_register(L) {
    luascript_register(L, MESSAGEBOX, script_messagebox_gc, script_messagebox_tostring, MESSAGEBOX_FUNCTIONS);
}
