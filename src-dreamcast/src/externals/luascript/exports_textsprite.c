#include "luascript_internal.h"

#include "luascript_enums.h"
#include "math2d.h"
#include "textsprite.h"


static int script_textsprite_set_text(lua_State* L) {
    TextSprite textsprite = luascript_read_userdata(L, TEXTSPRITE);

    const char* text = luaL_optstring(L, 2, NULL);

    textsprite_set_text_intern(textsprite, false, &text);

    return 0;
}

static int script_textsprite_set_font_size(lua_State* L) {
    TextSprite textsprite = luascript_read_userdata(L, TEXTSPRITE);

    float font_size = (float)luaL_checknumber(L, 2);

    textsprite_set_font_size(textsprite, font_size);

    return 0;
}

static int script_textsprite_force_case(lua_State* L) {
    TextSprite textsprite = luascript_read_userdata(L, TEXTSPRITE);

    int32_t uppercase_or_lowecase_or_none = (int32_t)luascript_helper_optenum(L, 2, LUASCRIPT_ENUMS_TextSpriteForceCase);

    textsprite_force_case(textsprite, uppercase_or_lowecase_or_none);

    return 0;
}

static int script_textsprite_set_paragraph_align(lua_State* L) {
    TextSprite textsprite = luascript_read_userdata(L, TEXTSPRITE);

    Align align = luascript_helper_optenum(L, 2, LUASCRIPT_ENUMS_Align);

    textsprite_set_paragraph_align(textsprite, align);

    return 0;
}

static int script_textsprite_set_paragraph_space(lua_State* L) {
    TextSprite textsprite = luascript_read_userdata(L, TEXTSPRITE);

    float space = (float)luaL_checknumber(L, 2);

    textsprite_set_paragraph_space(textsprite, space);

    return 0;
}

static int script_textsprite_set_maxlines(lua_State* L) {
    TextSprite textsprite = luascript_read_userdata(L, TEXTSPRITE);

    int32_t max_lines = (int32_t)luaL_checkinteger(L, 2);

    textsprite_set_maxlines(textsprite, max_lines);

    return 0;
}

static int script_textsprite_set_color_rgba8(lua_State* L) {
    TextSprite textsprite = luascript_read_userdata(L, TEXTSPRITE);

    uint64_t number = (uint64_t)luaL_checkinteger(L, 2);
    uint32_t rbga8_color = (uint32_t)number;

    textsprite_set_color_rgba8(textsprite, rbga8_color);

    return 0;
}

static int script_textsprite_set_color(lua_State* L) {
    TextSprite textsprite = luascript_read_userdata(L, TEXTSPRITE);

    float r = (float)luaL_optnumber(L, 2, FLOAT_NaN);
    float g = (float)luaL_optnumber(L, 3, FLOAT_NaN);
    float b = (float)luaL_optnumber(L, 4, FLOAT_NaN);

    textsprite_set_color(textsprite, r, g, b);

    return 0;
}

static int script_textsprite_set_alpha(lua_State* L) {
    TextSprite textsprite = luascript_read_userdata(L, TEXTSPRITE);

    float alpha = (float)luaL_checknumber(L, 2);

    textsprite_set_alpha(textsprite, alpha);

    return 0;
}

static int script_textsprite_set_visible(lua_State* L) {
    TextSprite textsprite = luascript_read_userdata(L, TEXTSPRITE);

    bool visible = lua_toboolean(L, 2);

    textsprite_set_visible(textsprite, visible);

    return 0;
}

static int script_textsprite_set_draw_location(lua_State* L) {
    TextSprite textsprite = luascript_read_userdata(L, TEXTSPRITE);

    float x = (float)luaL_checknumber(L, 2);
    float y = (float)luaL_checknumber(L, 3);

    textsprite_set_draw_location(textsprite, x, y);

    return 0;
}

static int script_textsprite_set_z_index(lua_State* L) {
    TextSprite textsprite = luascript_read_userdata(L, TEXTSPRITE);

    float z_index = (float)luaL_checknumber(L, 2);

    textsprite_set_z_index(textsprite, z_index);

    return 0;
}

static int script_textsprite_set_z_offset(lua_State* L) {
    TextSprite textsprite = luascript_read_userdata(L, TEXTSPRITE);

    float offset = (float)luaL_checknumber(L, 2);

    textsprite_set_z_offset(textsprite, offset);

    return 0;
}

static int script_textsprite_set_max_draw_size(lua_State* L) {
    TextSprite textsprite = luascript_read_userdata(L, TEXTSPRITE);

    float max_width = (float)luaL_checknumber(L, 2);
    float max_height = (float)luaL_checknumber(L, 3);

    textsprite_set_max_draw_size(textsprite, max_width, max_height);

    return 0;
}

static int script_textsprite_matrix_flip(lua_State* L) {
    TextSprite textsprite = luascript_read_userdata(L, TEXTSPRITE);

    nbool flip_x = luascript_helper_optnbool(L, 2);
    nbool flip_y = luascript_helper_optnbool(L, 3);

    textsprite_matrix_flip(textsprite, flip_x, flip_y);

    return 0;
}

static int script_textsprite_set_align(lua_State* L) {
    TextSprite textsprite = luascript_read_userdata(L, TEXTSPRITE);

    Align align_vertical = luascript_helper_optenum(L, 2, LUASCRIPT_ENUMS_Align);
    Align align_horizontal = luascript_helper_optenum(L, 3, LUASCRIPT_ENUMS_Align);

    textsprite_set_align(textsprite, align_vertical, align_horizontal);

    return 0;
}

static int script_textsprite_matrix_get_modifier(lua_State* L) {
    TextSprite textsprite = luascript_read_userdata(L, TEXTSPRITE);

    Modifier* modifier = textsprite_matrix_get_modifier(textsprite);

    return script_modifier_new(L, modifier);
}

static int script_textsprite_matrix_reset(lua_State* L) {
    TextSprite textsprite = luascript_read_userdata(L, TEXTSPRITE);

    textsprite_matrix_reset(textsprite);

    return 0;
}

static int script_textsprite_get_font_size(lua_State* L) {
    TextSprite textsprite = luascript_read_userdata(L, TEXTSPRITE);

    float font_size = textsprite_get_font_size(textsprite);

    lua_pushnumber(L, (lua_Number)font_size);
    return 1;
}

static int script_textsprite_get_draw_size(lua_State* L) {
    TextSprite textsprite = luascript_read_userdata(L, TEXTSPRITE);

    float draw_width, draw_height;
    textsprite_get_draw_size(textsprite, &draw_width, &draw_height);

    lua_pushnumber(L, (lua_Number)draw_width);
    lua_pushnumber(L, (lua_Number)draw_height);
    return 2;
}

static int script_textsprite_border_enable(lua_State* L) {
    TextSprite textsprite = luascript_read_userdata(L, TEXTSPRITE);

    bool enable = lua_toboolean(L, 2);

    textsprite_border_enable(textsprite, enable);

    return 0;
}

static int script_textsprite_border_set_size(lua_State* L) {
    TextSprite textsprite = luascript_read_userdata(L, TEXTSPRITE);

    float border_size = (float)luaL_checknumber(L, 2);

    textsprite_border_set_size(textsprite, border_size);

    return 0;
}

static int script_textsprite_border_set_color(lua_State* L) {
    TextSprite textsprite = luascript_read_userdata(L, TEXTSPRITE);

    float r = (float)luaL_optnumber(L, 2, FLOAT_NaN);
    float g = (float)luaL_optnumber(L, 3, FLOAT_NaN);
    float b = (float)luaL_optnumber(L, 4, FLOAT_NaN);
    float a = (float)luaL_optnumber(L, 5, FLOAT_NaN);

    textsprite_border_set_color(textsprite, r, g, b, a);

    return 0;
}

static int script_textsprite_border_set_offset(lua_State* L) {
    TextSprite textsprite = luascript_read_userdata(L, TEXTSPRITE);

    float x = (float)luaL_optnumber(L, 2, FLOAT_NaN);
    float y = (float)luaL_optnumber(L, 3, FLOAT_NaN);

    textsprite_border_set_offset(textsprite, x, y);

    return 0;
}

static int script_textsprite_set_antialiasing(lua_State* L) {
    TextSprite textsprite = luascript_read_userdata(L, TEXTSPRITE);

    PVRFlag antialiasing = luascript_helper_optenum(L, 2, LUASCRIPT_ENUMS_PVRFlag);

    textsprite_set_antialiasing(textsprite, antialiasing);

    return 0;
}

static int script_textsprite_set_wordbreak(lua_State* L) {
    TextSprite textsprite = luascript_read_userdata(L, TEXTSPRITE);

    int32_t wordbreak = (int32_t)luascript_helper_optenum(L, 2, LUASCRIPT_ENUMS_FontWordBreak);

    textsprite_set_wordbreak(textsprite, wordbreak);

    return 0;
}

#ifndef _arch_dreamcast
static int script_textsprite_set_shader(lua_State* L) {
    TextSprite textsprite = luascript_read_userdata(L, TEXTSPRITE);

    PSShader psshader = luascript_read_nullable_userdata(L, 2, PSSHADER);

    textsprite_set_shader(textsprite, psshader);

    return 0;
}

static int script_textsprite_get_shader(lua_State* L) {
    TextSprite textsprite = luascript_read_userdata(L, TEXTSPRITE);

    PSShader psshader = textsprite_get_shader(textsprite);

    return script_psshader_new(L, psshader);
}
#endif

static int script_textsprite_blend_enable(lua_State* L) {
    TextSprite textsprite = luascript_read_userdata(L, TEXTSPRITE);
    bool enabled = (bool)lua_toboolean(L, 2);

    textsprite_blend_enable(textsprite, enabled);

    return 0;
}

static int script_textsprite_blend_set(lua_State* L) {
    TextSprite textsprite = luascript_read_userdata(L, TEXTSPRITE);
    Blend src_rgb = luascript_helper_optenum(L, 2, LUASCRIPT_ENUMS_Blend);
    Blend dst_rgb = luascript_helper_optenum(L, 3, LUASCRIPT_ENUMS_Blend);
    Blend src_alpha = luascript_helper_optenum(L, 4, LUASCRIPT_ENUMS_Blend);
    Blend dst_alpha = luascript_helper_optenum(L, 5, LUASCRIPT_ENUMS_Blend);

    textsprite_blend_set(textsprite, src_rgb, dst_rgb, src_alpha, dst_alpha);

    return 0;
}

static int script_textsprite_background_enable(lua_State* L) {
    TextSprite textsprite = luascript_read_userdata(L, TEXTSPRITE);
    bool enabled = (bool)lua_toboolean(L, 2);

    textsprite_background_enable(textsprite, enabled);

    return 0;
}

static int script_textsprite_background_set_size(lua_State* L) {
    TextSprite textsprite = luascript_read_userdata(L, TEXTSPRITE);
    float size = (float)luaL_checknumber(L, 2);

    textsprite_background_set_size(textsprite, size);

    return 0;
}

static int script_textsprite_background_set_offets(lua_State* L) {
    TextSprite textsprite = luascript_read_userdata(L, TEXTSPRITE);
    float offset_x = (float)luaL_checknumber(L, 2);
    float offset_y = (float)luaL_checknumber(L, 3);

    textsprite_background_set_offets(textsprite, offset_x, offset_y);

    return 0;
}

static int script_textsprite_background_set_color(lua_State* L) {
    TextSprite textsprite = luascript_read_userdata(L, TEXTSPRITE);
    float r = (float)luaL_checknumber(L, 2);
    float g = (float)luaL_checknumber(L, 3);
    float b = (float)luaL_checknumber(L, 4);
    float a = (float)luaL_checknumber(L, 5);

    textsprite_background_set_color(textsprite, r, g, b, a);

    return 0;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const luaL_Reg TEXTSPRITE_FUNCTIONS[] = {
    {"set_text", script_textsprite_set_text},
    {"set_font_size", script_textsprite_set_font_size},
    {"force_case", script_textsprite_force_case},
    {"set_paragraph_align", script_textsprite_set_paragraph_align},
    {"set_paragraph_space", script_textsprite_set_paragraph_space},
    {"set_maxlines", script_textsprite_set_maxlines},
    {"set_color_rgba8", script_textsprite_set_color_rgba8},
    {"set_color", script_textsprite_set_color},
    {"set_alpha", script_textsprite_set_alpha},
    {"set_visible", script_textsprite_set_visible},
    {"set_draw_location", script_textsprite_set_draw_location},
    {"set_z_index", script_textsprite_set_z_index},
    {"set_z_offset", script_textsprite_set_z_offset},
    {"set_max_draw_size", script_textsprite_set_max_draw_size},
    {"matrix_flip", script_textsprite_matrix_flip},
    {"set_align", script_textsprite_set_align},
    {"matrix_get_modifier", script_textsprite_matrix_get_modifier},
    {"matrix_reset", script_textsprite_matrix_reset},
    {"get_font_size", script_textsprite_get_font_size},
    {"get_draw_size", script_textsprite_get_draw_size},
    {"border_enable", script_textsprite_border_enable},
    {"border_set_size", script_textsprite_border_set_size},
    {"border_set_color", script_textsprite_border_set_color},
    {"border_set_offset", script_textsprite_border_set_offset},
    {"set_antialiasing", script_textsprite_set_antialiasing},
    {"set_wordbreak", script_textsprite_set_wordbreak},
#ifndef _arch_dreamcast
    {"set_shader", script_textsprite_set_shader},
    {"get_shader", script_textsprite_get_shader},
#endif
    {"blend_enable", script_textsprite_blend_enable},
    {"blend_set", script_textsprite_blend_set},
    {"background_enable", script_textsprite_background_enable},
    {"background_set_size", script_textsprite_background_set_size},
    {"background_set_offets", script_textsprite_background_set_offets},
    {"background_set_color", script_textsprite_background_set_color},
    {NULL, NULL}
};

int script_textsprite_new(lua_State* L, TextSprite textsprite) {
    return luascript_userdata_new(L, TEXTSPRITE, textsprite);
}

static int script_textsprite_gc(lua_State* L) {
    return luascript_userdata_gc(L, TEXTSPRITE);
}

static int script_textsprite_tostring(lua_State* L) {
    return luascript_userdata_tostring(L, TEXTSPRITE);
}

void script_textsprite_register(lua_State* L) {
    luascript_register(L, TEXTSPRITE, script_textsprite_gc, script_textsprite_tostring, TEXTSPRITE_FUNCTIONS);
}
