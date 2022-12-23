#include "engine_string.h"
#include "luascript_internal.h"
#include "vertexprops.h"



#ifdef JAVASCRIPT
EM_JS_PRFX(void, textsprite_set_text_intern, (TextSprite textsprite, bool intern, const char* text), {
    textsprite_set_text_intern(kdmyEngine_obtain(textsprite), intern, kdmyEngine_ptrToString(text));
});
EM_JS_PRFX(void, textsprite_set_font_size, (TextSprite textsprite, float font_size), {
    textsprite_set_font_size(kdmyEngine_obtain(textsprite), font_size);
});
EM_JS_PRFX(void, textsprite_force_case, (TextSprite textsprite, TextSpriteForceCase none_or_lowercase_or_uppercase), {
    textsprite_force_case(kdmyEngine_obtain(textsprite), none_or_lowercase_or_uppercase);
});
EM_JS_PRFX(void, textsprite_set_paragraph_align, (TextSprite textsprite, Align align), {
    textsprite_set_paragraph_align(kdmyEngine_obtain(textsprite), align);
});
EM_JS_PRFX(void, textsprite_set_paragraph_space, (TextSprite textsprite, float space), {
    textsprite_set_paragraph_space(kdmyEngine_obtain(textsprite), space);
});
EM_JS_PRFX(void, textsprite_set_maxlines, (TextSprite textsprite, int32_t max_lines), {
    textsprite_set_maxlines(kdmyEngine_obtain(textsprite), max_lines);
});
EM_JS_PRFX(void, textsprite_set_color_rgba8, (TextSprite textsprite, uint32_t rbga8_color), {
    textsprite_set_color_rgba8(kdmyEngine_obtain(textsprite), rbga8_color);
});
EM_JS_PRFX(void, textsprite_set_color, (TextSprite textsprite, float r, float g, float b), {
    textsprite_set_color(kdmyEngine_obtain(textsprite), r, g, b);
});
EM_JS_PRFX(void, textsprite_set_alpha, (TextSprite textsprite, float alpha), {
    textsprite_set_alpha(kdmyEngine_obtain(textsprite), alpha);
});
EM_JS_PRFX(void, textsprite_set_visible, (TextSprite textsprite, bool visible), {
    textsprite_set_visible(kdmyEngine_obtain(textsprite), visible);
});
EM_JS_PRFX(void, textsprite_set_draw_location, (TextSprite textsprite, float x, float y), {
    textsprite_set_draw_location(kdmyEngine_obtain(textsprite), x, y);
});
EM_JS_PRFX(void, textsprite_set_z_index, (TextSprite textsprite, float z_index), {
    textsprite_set_z_index(kdmyEngine_obtain(textsprite), z_index);
});
EM_JS_PRFX(void, textsprite_set_z_offset, (TextSprite textsprite, float offset), {
    textsprite_set_z_offset(kdmyEngine_obtain(textsprite), offset);
});
EM_JS_PRFX(void, textsprite_set_max_draw_size, (TextSprite textsprite, float max_width, float max_height), {
    textsprite_set_max_draw_size(kdmyEngine_obtain(textsprite), max_width, max_height);
});
EM_JS_PRFX(void, textsprite_matrix_flip, (TextSprite textsprite, bool flip_x, bool flip_y), {
    textsprite_matrix_flip(kdmyEngine_obtain(textsprite), flip_x, flip_y);
});
EM_JS_PRFX(void, textsprite_set_align, (TextSprite textsprite, Align align_vertical, Align align_horizontal), {
    textsprite_set_align(kdmyEngine_obtain(textsprite), align_vertical, align_horizontal);
});
EM_JS_PRFX(Modifier, textsprite_matrix_get_modifier, (TextSprite textsprite), {
    const modifier = textsprite_matrix_get_modifier(kdmyEngine_obtain(textsprite));
    return kdmyEngine_obtain(modifier);
});
EM_JS_PRFX(void, textsprite_matrix_reset, (TextSprite textsprite), {
    textsprite_matrix_reset(kdmyEngine_obtain(textsprite));
});
EM_JS_PRFX(float, textsprite_get_font_size, (TextSprite textsprite), {
    return textsprite_get_font_size(kdmyEngine_obtain(textsprite));
});
EM_JS_PRFX(void, textsprite_get_draw_size, (TextSprite textsprite, float* draw_width, float* draw_height), {
    const values = [ 0, 0 ];
    textsprite_get_draw_size(kdmyEngine_obtain(textsprite), values);

    kdmyEngine_set_float32(draw_width, values[0]);
    kdmyEngine_set_float32(draw_height, values[1]);
});
EM_JS_PRFX(void, textsprite_border_enable, (TextSprite textsprite, bool enable), {
    textsprite_border_enable(kdmyEngine_obtain(textsprite), enable);
});
EM_JS_PRFX(void, textsprite_border_set_size, (TextSprite textsprite, float border_size), {
    textsprite_border_set_size(kdmyEngine_obtain(textsprite), border_size);
});
EM_JS_PRFX(void, textsprite_border_set_color, (TextSprite textsprite, float r, float g, float b, float a), {
    textsprite_border_set_color(kdmyEngine_obtain(textsprite), r, g, b, a);
});
EM_JS_PRFX(void, textsprite_border_set_offset, (TextSprite textsprite, float x, float y), {
    textsprite_border_set_offset(kdmyEngine_obtain(textsprite), x, y);
});
EM_JS_PRFX(void, textsprite_set_antialiasing, (TextSprite textsprite, PVRFlag antialiasing), {
    textsprite_set_antialiasing(kdmyEngine_obtain(textsprite), antialiasing);
});
EM_JS_PRFX(void, textsprite_set_wordbreak, (TextSprite textsprite, FontWordBreak wordbreak), {
    textsprite_set_wordbreak(kdmyEngine_obtain(textsprite), wordbreak);
});
EM_JS_PRFX(void, textsprite_set_shader, (TextSprite textsprite, PSShader psshader), {
    textsprite_set_shader(kdmyEngine_obtain(textsprite), kdmyEngine_obtain(psshader));
});
EM_JS_PRFX(PSShader, textsprite_get_shader, (TextSprite textsprite), {
    let psshader = textsprite_get_shader(kdmyEngine_obtain(textsprite));
    return kdmyEngine_obtain(psshader);
});
EM_JS_PRFX(void, textsprite_background_enable, (TextSprite textsprite, bool enabled), {
    textsprite_background_enable(kdmyEngine_obtain(textsprite), enabled);
});
EM_JS_PRFX(void, textsprite_background_set_size, (TextSprite textsprite, float size), {
    textsprite_background_set_size(kdmyEngine_obtain(textsprite), size);
});
EM_JS_PRFX(void, textsprite_background_set_offets, (TextSprite textsprite, float offset_x, float offset_y), {
    textsprite_background_set_offets(kdmyEngine_obtain(textsprite), offset_x, offset_y);
});
EM_JS_PRFX(void, textsprite_background_set_color, (TextSprite textsprite, float r, float g, float b, float a), {
    textsprite_background_set_color(kdmyEngine_obtain(textsprite), r, g, b, a);
});
#endif



static int script_textsprite_set_text(lua_State* L) {
    TextSprite textsprite = luascript_read_userdata(L, TEXTSPRITE);

    const char* text = luaL_optstring(L, 2, NULL);

    textsprite_set_text_intern(textsprite, false, text);

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

    TextSpriteForceCase uppercase_or_lowecase_or_none = luascript_parse_forcecase(L, luaL_optstring(L, 2, NULL));

    textsprite_force_case(textsprite, uppercase_or_lowecase_or_none);

    return 0;
}

static int script_textsprite_set_paragraph_align(lua_State* L) {
    TextSprite textsprite = luascript_read_userdata(L, TEXTSPRITE);

    const char* str = luaL_optstring(L, 2, NULL);

    Align align = luascript_parse_align(L, str);
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

    float r = (float)luaL_optnumber(L, 2, NAN);
    float g = (float)luaL_optnumber(L, 3, NAN);
    float b = (float)luaL_optnumber(L, 4, NAN);

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

    bool flip_x = lua_toboolean(L, 2);
    bool flip_y = lua_toboolean(L, 3);

    textsprite_matrix_flip(textsprite, flip_x, flip_y);

    return 0;
}

static int script_textsprite_set_align(lua_State* L) {
    TextSprite textsprite = luascript_read_userdata(L, TEXTSPRITE);

    const char* str1 = luaL_optstring(L, 2, NULL);
    const char* str2 = luaL_optstring(L, 3, NULL);

    Align align_vertical = luascript_parse_align(L, str1);
    Align align_horizontal = luascript_parse_align(L, str2);

    textsprite_set_align(textsprite, align_vertical, align_horizontal);

    return 0;
}

static int script_textsprite_matrix_get_modifier(lua_State* L) {
    TextSprite textsprite = luascript_read_userdata(L, TEXTSPRITE);

    Modifier modifier = textsprite_matrix_get_modifier(textsprite);

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

    lua_pushnumber(L, (double)font_size);
    return 1;
}

static int script_textsprite_get_draw_size(lua_State* L) {
    TextSprite textsprite = luascript_read_userdata(L, TEXTSPRITE);

    float draw_width, draw_height;
    textsprite_get_draw_size(textsprite, &draw_width, &draw_height);

    lua_pushnumber(L, draw_width);
    lua_pushnumber(L, draw_height);
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

    float r = (float)luaL_optnumber(L, 2, NAN);
    float g = (float)luaL_optnumber(L, 3, NAN);
    float b = (float)luaL_optnumber(L, 4, NAN);
    float a = (float)luaL_optnumber(L, 5, NAN);

    textsprite_border_set_color(textsprite, r, g, b, a);

    return 0;
}

static int script_textsprite_border_set_offset(lua_State* L) {
    TextSprite textsprite = luascript_read_userdata(L, TEXTSPRITE);

    float x = (float)luaL_optnumber(L, 2, NAN);
    float y = (float)luaL_optnumber(L, 3, NAN);

    textsprite_border_set_offset(textsprite, x, y);

    return 0;
}

static int script_textsprite_set_antialiasing(lua_State* L) {
    TextSprite textsprite = luascript_read_userdata(L, TEXTSPRITE);

    PVRFlag antialiasing = luascript_parse_align(L, luaL_checkstring(L, 2));

    textsprite_set_antialiasing(textsprite, antialiasing);

    return 0;
}

static int script_textsprite_set_wordbreak(lua_State* L) {
    TextSprite textsprite = luascript_read_userdata(L, TEXTSPRITE);

    FontWordBreak wordbreak = luascript_parse_wordbreak(L, luaL_optstring(L, 2, NULL));

    textsprite_set_wordbreak(textsprite, wordbreak);

    return 0;
}

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
    {"set_shader", script_textsprite_set_shader},
    {"get_shader", script_textsprite_get_shader},
    { "background_enable", script_textsprite_background_enable },
    { "background_set_size", script_textsprite_background_set_size },
    { "background_set_offets", script_textsprite_background_set_offets },
    { "background_set_color", script_textsprite_background_set_color },
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
