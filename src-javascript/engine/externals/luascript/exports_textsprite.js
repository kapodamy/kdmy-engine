"use strict";

const TEXTSPRITE = "TextSprite";


function script_textsprite_set_text(L) {
    let textsprite = luascript_read_userdata(L, TEXTSPRITE);

    let text = LUA.luaL_optstring(L, 2, null);

    textsprite_set_text_intern(textsprite, false, text);

    return 0;
}

function script_textsprite_set_font_size(L) {
    let textsprite = luascript_read_userdata(L, TEXTSPRITE);

    let font_size = LUA.luaL_checknumber(L, 2);

    textsprite_set_font_size(textsprite, font_size);

    return 0;
}

function script_textsprite_force_case(L) {
    let textsprite = luascript_read_userdata(L, TEXTSPRITE);

    let uppercase_or_lowecase_or_none = luascript_helper_optenum(L, 2, LUASCRIPT_ENUMS_TextSpriteForceCase);

    textsprite_force_case(textsprite, uppercase_or_lowecase_or_none);

    return 0;
}

function script_textsprite_set_paragraph_align(L) {
    let textsprite = luascript_read_userdata(L, TEXTSPRITE);
    let align = luascript_helper_optenum(L, 2, LUASCRIPT_ENUMS_Align);

    textsprite_set_paragraph_align(textsprite, align);

    return 0;
}

function script_textsprite_set_paragraph_space(L) {
    let textsprite = luascript_read_userdata(L, TEXTSPRITE);

    let space = LUA.luaL_checknumber(L, 2);

    textsprite_set_paragraph_space(textsprite, space);

    return 0;
}

function script_textsprite_set_maxlines(L) {
    let textsprite = luascript_read_userdata(L, TEXTSPRITE);

    let max_lines = LUA.luaL_checkinteger(L, 2);

    textsprite_set_maxlines(textsprite, max_lines);

    return 0;
}

function script_textsprite_set_color_rgba8(L) {
    let textsprite = luascript_read_userdata(L, TEXTSPRITE);

    let rbga8_color = LUA.luaL_checkinteger(L, 2);

    textsprite_set_color_rgba8(textsprite, rbga8_color);

    return 0;
}

function script_textsprite_set_color(L) {
    let textsprite = luascript_read_userdata(L, TEXTSPRITE);

    let r = LUA.luaL_optnumber(L, 2, NaN);
    let g = LUA.luaL_optnumber(L, 3, NaN);
    let b = LUA.luaL_optnumber(L, 4, NaN);

    textsprite_set_color(textsprite, r, g, b);

    return 0;
}

function script_textsprite_set_alpha(L) {
    let textsprite = luascript_read_userdata(L, TEXTSPRITE);

    let alpha = LUA.luaL_checknumber(L, 2);

    textsprite_set_alpha(textsprite, alpha);

    return 0;
}

function script_textsprite_set_visible(L) {
    let textsprite = luascript_read_userdata(L, TEXTSPRITE);

    let visible = LUA.lua_toboolean(L, 2);

    textsprite_set_visible(textsprite, visible);

    return 0;
}

function script_textsprite_set_draw_location(L) {
    let textsprite = luascript_read_userdata(L, TEXTSPRITE);

    let x = LUA.luaL_checknumber(L, 2);
    let y = LUA.luaL_checknumber(L, 3);

    textsprite_set_draw_location(textsprite, x, y);

    return 0;
}

function script_textsprite_set_z_index(L) {
    let textsprite = luascript_read_userdata(L, TEXTSPRITE);

    let z_index = LUA.luaL_checknumber(L, 2);

    textsprite_set_z_index(textsprite, z_index);

    return 0;
}

function script_textsprite_set_z_offset(L) {
    let textsprite = luascript_read_userdata(L, TEXTSPRITE);

    let offset = LUA.luaL_checknumber(L, 2);

    textsprite_set_z_offset(textsprite, offset);

    return 0;
}

function script_textsprite_set_max_draw_size(L) {
    let textsprite = luascript_read_userdata(L, TEXTSPRITE);

    let max_width = LUA.luaL_checknumber(L, 2);
    let max_height = LUA.luaL_checknumber(L, 3);

    textsprite_set_max_draw_size(textsprite, max_width, max_height);

    return 0;
}

function script_textsprite_matrix_flip(L) {
    let textsprite = luascript_read_userdata(L, TEXTSPRITE);

    let flip_x = luascript_helper_optnbool(L, 2);
    let flip_y = luascript_helper_optnbool(L, 3);

    textsprite_matrix_flip(textsprite, flip_x, flip_y);

    return 0;
}

function script_textsprite_set_align(L) {
    let textsprite = luascript_read_userdata(L, TEXTSPRITE);
    let align_vertical = luascript_helper_optenum(L, 2, LUASCRIPT_ENUMS_Align);
    let align_horizontal = luascript_helper_optenum(L, 3, LUASCRIPT_ENUMS_Align);

    textsprite_set_align(textsprite, align_vertical, align_horizontal);

    return 0;
}

function script_textsprite_matrix_get_modifier(L) {
    let textsprite = luascript_read_userdata(L, TEXTSPRITE);

    let modifier = textsprite_matrix_get_modifier(textsprite);

    return script_modifier_new(L, modifier);
}

function script_textsprite_matrix_reset(L) {
    let textsprite = luascript_read_userdata(L, TEXTSPRITE);

    textsprite_matrix_reset(textsprite);

    return 0;
}

function script_textsprite_get_font_size(L) {
    let textsprite = luascript_read_userdata(L, TEXTSPRITE);

    let font_size = textsprite_get_font_size(textsprite);

    LUA.lua_pushnumber(L, font_size);
    return 1;
}

function script_textsprite_get_draw_size(L) {
    let textsprite = luascript_read_userdata(L, TEXTSPRITE);

    const draw_size = [0.0, 0.0];
    textsprite_get_draw_size(textsprite, draw_size);

    LUA.lua_pushnumber(L, draw_size[0]);
    LUA.lua_pushnumber(L, draw_size[1]);
    return 2;
}

function script_textsprite_border_enable(L) {
    let textsprite = luascript_read_userdata(L, TEXTSPRITE);

    let enable = LUA.lua_toboolean(L, 2);

    textsprite_border_enable(textsprite, enable);

    return 0;
}

function script_textsprite_border_set_size(L) {
    let textsprite = luascript_read_userdata(L, TEXTSPRITE);

    let border_size = LUA.luaL_checknumber(L, 2);

    textsprite_border_set_size(textsprite, border_size);

    return 0;
}

function script_textsprite_border_set_color(L) {
    let textsprite = luascript_read_userdata(L, TEXTSPRITE);

    let r = LUA.luaL_optnumber(L, 2, NaN);
    let g = LUA.luaL_optnumber(L, 3, NaN);
    let b = LUA.luaL_optnumber(L, 4, NaN);
    let a = LUA.luaL_optnumber(L, 5, NaN);

    textsprite_border_set_color(textsprite, r, g, b, a);

    return 0;
}

function script_textsprite_border_set_offset(L) {
    let textsprite = luascript_read_userdata(L, TEXTSPRITE);

    let x = LUA.luaL_optnumber(L, 2, NaN);
    let y = LUA.luaL_optnumber(L, 3, NaN);

    textsprite_border_set_offset(textsprite, x, y);

    return 0;
}

function script_textsprite_set_antialiasing(L) {
    let textsprite = luascript_read_userdata(L, TEXTSPRITE);
    let antialiasing = luascript_helper_optenum(L, 2, LUASCRIPT_ENUMS_PVRFlag);

    textsprite_set_antialiasing(textsprite, antialiasing);

    return 0;
}

function script_textsprite_set_wordbreak(L) {
    let textsprite = luascript_read_userdata(L, TEXTSPRITE);
    let wordbreak = luascript_helper_optenum(L, 2, LUASCRIPT_ENUMS_FontWordBreak);

    textsprite_set_wordbreak(textsprite, wordbreak);

    return 0;
}

function script_textsprite_set_shader(L) {
    let textsprite = luascript_read_userdata(L, TEXTSPRITE);

    let psshader = luascript_read_nullable_userdata(L, 2, PSSHADER);

    textsprite_set_shader(textsprite, psshader);

    return 0;
}

function script_textsprite_get_shader(L) {
    let textsprite = luascript_read_userdata(L, TEXTSPRITE);

    let psshader = textsprite_get_shader(textsprite);

    return script_psshader_new(L, psshader);
}

function script_textsprite_blend_enable(L) {
    let textsprite = luascript_read_userdata(L, TEXTSPRITE);
    let enabled = LUA.lua_toboolean(L, 2);

    textsprite_blend_enable(textsprite, enabled);

    return 0;
}

function script_textsprite_blend_set(L) {
    let textsprite = luascript_read_userdata(L, TEXTSPRITE);
    let src_rgb = luascript_helper_optenum(L, 2, LUASCRIPT_ENUMS_Blend);
    let dst_rgb = luascript_helper_optenum(L, 3, LUASCRIPT_ENUMS_Blend);
    let src_alpha = luascript_helper_optenum(L, 4, LUASCRIPT_ENUMS_Blend);
    let dst_alpha = luascript_helper_optenum(L, 5, LUASCRIPT_ENUMS_Blend);

    textsprite_blend_set(textsprite, src_rgb, dst_rgb, src_alpha, dst_alpha);

    return 0;
}

function script_textsprite_background_enable(L) {
    let textsprite = luascript_read_userdata(L, TEXTSPRITE);
    let enabled = LUA.lua_toboolean(L, 2);

    textsprite_background_enable(textsprite, enabled);

    return 0;
}

function script_textsprite_background_set_size(L) {
    let textsprite = luascript_read_userdata(L, TEXTSPRITE);
    let size = LUA.luaL_checknumber(L, 2);

    textsprite_background_set_size(textsprite, size);

    return 0;
}

function script_textsprite_background_set_offets(L) {
    let textsprite = luascript_read_userdata(L, TEXTSPRITE);
    let offset_x = LUA.luaL_checknumber(L, 2);
    let offset_y = LUA.luaL_checknumber(L, 3);

    textsprite_background_set_offets(textsprite, offset_x, offset_y);

    return 0;
}

function script_textsprite_background_set_color(L) {
    let textsprite = luascript_read_userdata(L, TEXTSPRITE);
    let r = LUA.luaL_checknumber(L, 2);
    let g = LUA.luaL_checknumber(L, 3);
    let b = LUA.luaL_checknumber(L, 4);
    let a = LUA.luaL_checknumber(L, 5);

    textsprite_background_set_color(textsprite, r, g, b, a);

    return 0;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/**@type {luaL_Reg[]}*/const TEXTSPRITE_FUNCTIONS = [
    ["set_text", script_textsprite_set_text],
    ["set_font_size", script_textsprite_set_font_size],
    ["force_case", script_textsprite_force_case],
    ["set_paragraph_align", script_textsprite_set_paragraph_align],
    ["set_paragraph_space", script_textsprite_set_paragraph_space],
    ["set_maxlines", script_textsprite_set_maxlines],
    ["set_color_rgba8", script_textsprite_set_color_rgba8],
    ["set_color", script_textsprite_set_color],
    ["set_alpha", script_textsprite_set_alpha],
    ["set_visible", script_textsprite_set_visible],
    ["set_draw_location", script_textsprite_set_draw_location],
    ["set_z_index", script_textsprite_set_z_index],
    ["set_z_offset", script_textsprite_set_z_offset],
    ["set_max_draw_size", script_textsprite_set_max_draw_size],
    ["matrix_flip", script_textsprite_matrix_flip],
    ["set_align", script_textsprite_set_align],
    ["matrix_get_modifier", script_textsprite_matrix_get_modifier],
    ["matrix_reset", script_textsprite_matrix_reset],
    ["get_font_size", script_textsprite_get_font_size],
    ["get_draw_size", script_textsprite_get_draw_size],
    ["border_enable", script_textsprite_border_enable],
    ["border_set_size", script_textsprite_border_set_size],
    ["border_set_color", script_textsprite_border_set_color],
    ["border_set_offset", script_textsprite_border_set_offset],
    ["set_antialiasing", script_textsprite_set_antialiasing],
    ["set_wordbreak", script_textsprite_set_wordbreak],
    ["set_shader", script_textsprite_set_shader],
    ["get_shader", script_textsprite_get_shader],
    ["blend_enable", script_textsprite_blend_enable],
    ["blend_set", script_textsprite_blend_set],
    ["background_enable", script_textsprite_background_enable],
    ["background_set_size", script_textsprite_background_set_size],
    ["background_set_offets", script_textsprite_background_set_offets],
    ["background_set_color", script_textsprite_background_set_color],
    [null, null]
];

function script_textsprite_new(L, textsprite) {
    return luascript_userdata_new(L, TEXTSPRITE, textsprite);
}

function script_textsprite_gc(L) {
    return luascript_userdata_gc(L, TEXTSPRITE);
}

function script_textsprite_tostring(L) {
    return luascript_userdata_tostring(L, TEXTSPRITE);
}


function script_textsprite_register(L) {
    luascript_register(L, TEXTSPRITE, script_textsprite_gc, script_textsprite_tostring, TEXTSPRITE_FUNCTIONS);
}
