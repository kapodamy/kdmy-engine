"use strict";

const SPRITE = "Sprite";


function script_sprite_matrix_get_modifier(L) {
    let sprite = luascript_read_userdata(L, SPRITE);

    let modifier = sprite_matrix_get_modifier(sprite);
    return script_modifier_new(L, modifier);
}

function script_sprite_set_offset_source(L) {
    let sprite = luascript_read_userdata(L, SPRITE);

    let x = LUA.luaL_checknumber(L, 2);
    let y = LUA.luaL_checknumber(L, 3);
    let width = LUA.luaL_checknumber(L, 4);
    let height = LUA.luaL_checknumber(L, 5);

    sprite_set_offset_source(sprite, x, y, width, height);

    return 0;
}

function script_sprite_set_offset_frame(L) {
    let sprite = luascript_read_userdata(L, SPRITE);

    let x = LUA.luaL_checknumber(L, 2);
    let y = LUA.luaL_checknumber(L, 3);
    let width = LUA.luaL_checknumber(L, 4);
    let height = LUA.luaL_checknumber(L, 5);

    sprite_set_offset_frame(sprite, x, y, width, height);

    return 0;
}

function script_sprite_set_offset_pivot(L) {
    let sprite = luascript_read_userdata(L, SPRITE);

    let x = LUA.luaL_checknumber(L, 2);
    let y = LUA.luaL_checknumber(L, 3);

    sprite_set_offset_pivot(sprite, x, y);
    return 0;
}

function script_sprite_matrix_reset(L) {
    let sprite = luascript_read_userdata(L, SPRITE);

    sprite_matrix_reset(sprite);

    return 0;
}

function script_sprite_set_draw_location(L) {
    let sprite = luascript_read_userdata(L, SPRITE);

    let x = LUA.luaL_checknumber(L, 2);
    let y = LUA.luaL_checknumber(L, 3);

    sprite_set_draw_location(sprite, x, y);

    return 0;
}

function script_sprite_set_draw_size(L) {
    let sprite = luascript_read_userdata(L, SPRITE);

    let width = LUA.luaL_checknumber(L, 2);
    let height = LUA.luaL_checknumber(L, 3);

    sprite_set_draw_size(sprite, width, height);

    return 0;
}

function script_sprite_set_draw_size_from_source_size(L) {
    let sprite = luascript_read_userdata(L, SPRITE);

    sprite_set_draw_size_from_source_size(sprite);

    return 0;
}

function script_sprite_set_alpha(L) {
    let sprite = luascript_read_userdata(L, SPRITE);

    let alpha = LUA.luaL_checknumber(L, 2);

    sprite_set_alpha(sprite, alpha);

    return 0;
}

function script_sprite_set_visible(L) {
    let sprite = luascript_read_userdata(L, SPRITE);

    let visible = LUA.lua_toboolean(L, 2);

    sprite_set_visible(sprite, visible);

    return 0;
}

function script_sprite_set_z_index(L) {
    let sprite = luascript_read_userdata(L, SPRITE);

    let index = LUA.luaL_checknumber(L, 2);

    sprite_set_z_index(sprite, index);

    return 0;
}

function script_sprite_set_z_offset(L) {
    let sprite = luascript_read_userdata(L, SPRITE);

    let index = LUA.luaL_checknumber(L, 2);

    sprite_set_z_offset(sprite, index);

    return 0;
}

function script_sprite_get_source_size(L) {
    let sprite = luascript_read_userdata(L, SPRITE);

    const size = [0.0, 0.0];
    sprite_get_source_size(sprite, size);

    LUA.lua_pushnumber(L, size[0]);
    LUA.lua_pushnumber(L, size[1]);

    return 2;
}

function script_sprite_set_vertex_color(L) {
    let sprite = luascript_read_userdata(L, SPRITE);

    let r = LUA.luaL_optnumber(L, 2, NaN);
    let g = LUA.luaL_optnumber(L, 3, NaN);
    let b = LUA.luaL_optnumber(L, 4, NaN);

    sprite_set_vertex_color(sprite, r, g, b);

    return 0;
}

function script_sprite_set_offsetcolor(L) {
    let sprite = luascript_read_userdata(L, SPRITE);

    let r = LUA.luaL_optnumber(L, 2, NaN);
    let g = LUA.luaL_optnumber(L, 3, NaN);
    let b = LUA.luaL_optnumber(L, 4, NaN);
    let a = LUA.luaL_optnumber(L, 5, NaN);

    sprite_set_offsetcolor(sprite, r, g, b, a);

    return 0;
}

function script_sprite_is_textured(L) {
    let sprite = luascript_read_userdata(L, SPRITE);

    let ret = sprite_is_textured(sprite);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_sprite_crop(L) {
    let sprite = luascript_read_userdata(L, SPRITE);

    let dx = LUA.luaL_optnumber(L, 2, NaN);
    let dy = LUA.luaL_optnumber(L, 3, NaN);
    let dwidth = LUA.luaL_optnumber(L, 4, NaN);
    let dheight = LUA.luaL_optnumber(L, 5, NaN);

    let ret = sprite_crop(sprite, dx, dy, dwidth, dheight);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_sprite_is_crop_enabled(L) {
    let sprite = luascript_read_userdata(L, SPRITE);

    let ret = sprite_is_crop_enabled(sprite);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_sprite_crop_enable(L) {
    let sprite = luascript_read_userdata(L, SPRITE);

    let enable = LUA.lua_toboolean(L, 2);

    sprite_crop_enable(sprite, enable);

    return 0;
}

function script_sprite_resize_draw_size(L) {
    let sprite = luascript_read_userdata(L, SPRITE);

    let max_width = LUA.luaL_checknumber(L, 2);
    let max_height = LUA.luaL_checknumber(L, 3);

    const applied_draw_size = [0.0, 0.0];
    sprite_resize_draw_size(sprite, max_width, max_height, applied_draw_size);

    LUA.lua_pushnumber(L, applied_draw_size[0]);
    LUA.lua_pushnumber(L, applied_draw_size[1]);
    return 2;
}

function script_sprite_center_draw_location(L) {
    let sprite = luascript_read_userdata(L, SPRITE);

    let x = LUA.luaL_checknumber(L, 2);
    let y = LUA.luaL_checknumber(L, 3);
    let ref_width = LUA.luaL_checknumber(L, 4);
    let ref_height = LUA.luaL_checknumber(L, 5);

    const applied_draw_size = [0.0, 0.0];
    sprite_center_draw_location(sprite, x, y, ref_width, ref_height, applied_draw_size);

    LUA.lua_pushnumber(L, applied_draw_size[0]);
    LUA.lua_pushnumber(L, applied_draw_size[1]);
    return 2;
}

function script_sprite_set_antialiasing(L) {
    let sprite = luascript_read_userdata(L, SPRITE);

    let antialiasing = luascript_helper_optenum(L, 2, LUASCRIPT_ENUMS_PVRFlag);

    sprite_set_antialiasing(sprite, antialiasing);

    return 0;
}

function script_sprite_flip_rendered_texture(L) {
    let sprite = luascript_read_userdata(L, SPRITE);

    let flip_x = luascript_helper_optnbool(L, 2);
    let flip_y = luascript_helper_optnbool(L, 3);

    sprite_flip_rendered_texture(sprite, flip_x, flip_y);

    return 0;
}

function script_sprite_flip_rendered_texture_enable_correction(L) {
    let sprite = luascript_read_userdata(L, SPRITE);

    let enabled = LUA.lua_toboolean(L, 2);

    sprite_flip_rendered_texture_enable_correction(sprite, enabled);

    return 0;
}

function script_sprite_set_shader(L) {
    let sprite = luascript_read_userdata(L, SPRITE);

    let psshader = luascript_read_nullable_userdata(L, 2, PSSHADER);

    sprite_set_shader(sprite, psshader);

    return 0;
}

function script_sprite_get_shader(L) {
    let sprite = luascript_read_userdata(L, SPRITE);

    let psshader = sprite_get_shader(sprite);

    return script_psshader_new(L, psshader);
}

function script_sprite_blend_enable(L) {
    let sprite = luascript_read_userdata(L, SPRITE);
    let enabled = LUA.lua_toboolean(L, 2);

    sprite_blend_enable(sprite, enabled);

    return 0;
}

function script_sprite_blend_set(L) {
    let sprite = luascript_read_userdata(L, SPRITE);
    let src_rgb = luascript_helper_optenum(L, 2, LUASCRIPT_ENUMS_Blend);
    let dst_rgb = luascript_helper_optenum(L, 3, LUASCRIPT_ENUMS_Blend);
    let src_alpha = luascript_helper_optenum(L, 4, LUASCRIPT_ENUMS_Blend);
    let dst_alpha = luascript_helper_optenum(L, 5, LUASCRIPT_ENUMS_Blend);

    sprite_blend_set(sprite, src_rgb, dst_rgb, src_alpha, dst_alpha);

    return 0;
}

function script_sprite_trailing_enabled(L) {
    let sprite = luascript_read_userdata(L, SPRITE);
    let enabled = LUA.lua_toboolean(L, 2);

    sprite_trailing_enabled(sprite, enabled);

    return 0;
}

function script_sprite_trailing_set_params(L) {
    let sprite = luascript_read_userdata(L, SPRITE);
    let length = LUA.luaL_checkinteger(L, 2);
    let trail_delay = LUA.luaL_checknumber(L, 3);
    let trail_alpha = LUA.luaL_checknumber(L, 4);
    let darken_colors = luascript_helper_optnbool(L, 5);

    sprite_trailing_set_params(sprite, length, trail_delay, trail_alpha, darken_colors);

    return 0;
}

function script_sprite_trailing_set_offsetcolor(L) {
    let sprite = luascript_read_userdata(L, SPRITE);
    let r = LUA.luaL_optnumber(L, 2, NaN);
    let g = LUA.luaL_optnumber(L, 3, NaN);
    let b = LUA.luaL_optnumber(L, 4, NaN);

    sprite_trailing_set_offsetcolor(sprite, r, g, b);

    return 0;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/**@type {luaL_Reg[]}*/const SPRITE_FUNCTIONS = [
    ["matrix_get_modifier", script_sprite_matrix_get_modifier],
    ["set_offset_source", script_sprite_set_offset_source],
    ["set_offset_frame", script_sprite_set_offset_frame],
    ["set_offset_pivot", script_sprite_set_offset_pivot],
    ["matrix_reset", script_sprite_matrix_reset],
    ["set_draw_location", script_sprite_set_draw_location],
    ["set_draw_size", script_sprite_set_draw_size],
    ["set_draw_size_from_source_size", script_sprite_set_draw_size_from_source_size],
    ["set_alpha", script_sprite_set_alpha],
    ["set_visible", script_sprite_set_visible],
    ["set_z_index", script_sprite_set_z_index],
    ["set_z_offset", script_sprite_set_z_offset],
    ["get_source_size", script_sprite_get_source_size],
    ["set_vertex_color", script_sprite_set_vertex_color],
    ["set_offsetcolor", script_sprite_set_offsetcolor],
    ["is_textured", script_sprite_is_textured],
    ["crop", script_sprite_crop],
    ["is_crop_enabled", script_sprite_is_crop_enabled],
    ["crop_enable", script_sprite_crop_enable],
    ["resize_draw_size", script_sprite_resize_draw_size],
    ["center_draw_location", script_sprite_center_draw_location],
    ["set_antialiasing", script_sprite_set_antialiasing],
    ["flip_rendered_texture", script_sprite_flip_rendered_texture],
    ["flip_rendered_texture_enable_correction", script_sprite_flip_rendered_texture_enable_correction],
    ["set_shader", script_sprite_set_shader],
    ["get_shader", script_sprite_get_shader],
    ["blend_enable", script_sprite_blend_enable],
    ["blend_set", script_sprite_blend_set],
    ["trailing_enabled", script_sprite_trailing_enabled],
    ["trailing_set_params", script_sprite_trailing_set_params],
    ["trailing_set_offsetcolor", script_sprite_trailing_set_offsetcolor],
    [null, null]
];


function script_sprite_new(L, sprite) {
    return luascript_userdata_new(L, SPRITE, sprite);
}

function script_sprite_gc(L) {
    return luascript_userdata_gc(L, SPRITE);
}

function script_sprite_tostring(L) {
    return luascript_userdata_tostring(L, SPRITE);
}


function script_sprite_register(L) {
    luascript_register(L, SPRITE, script_sprite_gc, script_sprite_tostring, SPRITE_FUNCTIONS);
}
