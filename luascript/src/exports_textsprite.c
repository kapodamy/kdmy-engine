#include "luascript_internal.h"
#include "engine_string.h"
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
EM_JS_PRFX(float*, textsprite_get_draw_size, (TextSprite textsprite, float* size), {
    const HEAP_ENDIANESS = true;
    const dataView = new DataView(buffer);
    const values = [0, 0];

    textsprite_get_draw_size(kdmyEngine_obtain(textsprite), values);
    dataView.setFloat32(size + 0, values[0], HEAP_ENDIANESS);
    dataView.setFloat32(size + 4, values[1], HEAP_ENDIANESS);

    return size;
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
EM_JS_PRFX(void, textsprite_set_antialiasing, (TextSprite textsprite, PVRFLAG antialiasing), {
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
#endif



static int script_textsprite_set_text(lua_State* L) {
    READ_USERDATA(L, TextSprite, textsprite, TEXTSPRITE);

    const char* text = luaL_optstring(L, 2, NULL);

    textsprite_set_text_intern(textsprite, false, text);

    return 0;
}

static int script_textsprite_set_font_size(lua_State* L) {
    READ_USERDATA(L, TextSprite, textsprite, TEXTSPRITE);

    float font_size = luaL_checkfloat(L, 2);

    textsprite_set_font_size(textsprite, font_size);

    return 0;
}

static int script_textsprite_force_case(lua_State* L) {
    READ_USERDATA(L, TextSprite, textsprite, TEXTSPRITE);

    const char* str = luaL_optstring(L, 2, NULL);

    TextSpriteForceCase uppercase_or_lowecase_or_none;
    if (string_equals(str, "lower"))
        uppercase_or_lowecase_or_none = TEXTSPRITE_FORCECASE_LOWER;
    else if (string_equals(str, "upper"))
        uppercase_or_lowecase_or_none = TEXTSPRITE_FORCECASE_UPPER;
    else
        uppercase_or_lowecase_or_none = TEXTSPRITE_FORCECASE_NONE;

    textsprite_force_case(textsprite, uppercase_or_lowecase_or_none);

    return 0;
}

static int script_textsprite_set_paragraph_align(lua_State* L) {
    READ_USERDATA(L, TextSprite, textsprite, TEXTSPRITE);

    const char* str = luaL_optstring(L, 2, NULL);

    Align align = _parse_align(L, str);
    textsprite_set_paragraph_align(textsprite, align);

    return 0;
}

static int script_textsprite_set_paragraph_space(lua_State* L) {
    READ_USERDATA(L, TextSprite, textsprite, TEXTSPRITE);

    float space = luaL_checkfloat(L, 2);

    textsprite_set_paragraph_space(textsprite, space);

    return 0;
}

static int script_textsprite_set_maxlines(lua_State* L) {
    READ_USERDATA(L, TextSprite, textsprite, TEXTSPRITE);

    int32_t max_lines = (int32_t)luaL_checkinteger(L, 2);

    textsprite_set_maxlines(textsprite, max_lines);

    return 0;
}

static int script_textsprite_set_color_rgba8(lua_State* L) {
    READ_USERDATA(L, TextSprite, textsprite, TEXTSPRITE);

    uint64_t number = (uint64_t)luaL_checkinteger(L, 2);
    uint32_t rbga8_color = (uint32_t)number;

    textsprite_set_color_rgba8(textsprite, rbga8_color);

    return 0;
}

static int script_textsprite_set_color(lua_State* L) {
    READ_USERDATA(L, TextSprite, textsprite, TEXTSPRITE);

    float r = luaL_optionalfloat(L, 2);
    float g = luaL_optionalfloat(L, 3);
    float b = luaL_optionalfloat(L, 4);

    textsprite_set_color(textsprite, r, g, b);

    return 0;
}

static int script_textsprite_set_alpha(lua_State* L) {
    READ_USERDATA(L, TextSprite, textsprite, TEXTSPRITE);

    float alpha = luaL_checkfloat(L, 2);

    textsprite_set_alpha(textsprite, alpha);

    return 0;
}

static int script_textsprite_set_visible(lua_State* L) {
    READ_USERDATA(L, TextSprite, textsprite, TEXTSPRITE);

    bool visible = luaL_checkboolean(L, 2);

    textsprite_set_visible(textsprite, visible);

    return 0;
}

static int script_textsprite_set_draw_location(lua_State* L) {
    READ_USERDATA(L, TextSprite, textsprite, TEXTSPRITE);

    float x = luaL_checkfloat(L, 2);
    float y = luaL_checkfloat(L, 3);

    textsprite_set_draw_location(textsprite, x, y);

    return 0;
}

static int script_textsprite_set_z_index(lua_State* L) {
    READ_USERDATA(L, TextSprite, textsprite, TEXTSPRITE);

    float z_index = luaL_checkfloat(L, 2);

    textsprite_set_z_index(textsprite, z_index);

    return 0;
}

static int script_textsprite_set_z_offset(lua_State* L) {
    READ_USERDATA(L, TextSprite, textsprite, TEXTSPRITE);

    float offset = luaL_checkfloat(L, 2);

    textsprite_set_z_offset(textsprite, offset);

    return 0;
}

static int script_textsprite_set_max_draw_size(lua_State* L) {
    READ_USERDATA(L, TextSprite, textsprite, TEXTSPRITE);

    float max_width = luaL_checkfloat(L, 2);
    float max_height = luaL_checkfloat(L, 3);

    textsprite_set_max_draw_size(textsprite, max_width, max_height);

    return 0;
}

static int script_textsprite_matrix_flip(lua_State* L) {
    READ_USERDATA(L, TextSprite, textsprite, TEXTSPRITE);

    bool flip_x = luaL_checkboolean(L, 2);
    bool flip_y = luaL_checkboolean(L, 3);

    textsprite_matrix_flip(textsprite, flip_x, flip_y);

    return 0;
}

static int script_textsprite_set_align(lua_State* L) {
    READ_USERDATA(L, TextSprite, textsprite, TEXTSPRITE);

    const char* str1 = luaL_optstring(L, 2, NULL);
    const char* str2 = luaL_optstring(L, 3, NULL);

    Align align_vertical = _parse_align(L, str1);
    Align align_horizontal = _parse_align(L, str2);

    textsprite_set_align(textsprite, align_vertical, align_horizontal);

    return 0;
}

static int script_textsprite_matrix_get_modifier(lua_State* L) {
    READ_USERDATA(L, TextSprite, textsprite, TEXTSPRITE);

    Modifier modifier = textsprite_matrix_get_modifier(textsprite);

    return script_modifier_new(L, textsprite, modifier);
}

static int script_textsprite_matrix_reset(lua_State* L) {
    READ_USERDATA(L, TextSprite, textsprite, TEXTSPRITE);

    textsprite_matrix_reset(textsprite);

    return 0;
}

static int script_textsprite_get_font_size(lua_State* L) {
    READ_USERDATA(L, TextSprite, textsprite, TEXTSPRITE);

    float font_size = textsprite_get_font_size(textsprite);

    lua_pushnumber(L, (double)font_size);
    return 1;
}

static int script_textsprite_get_draw_size(lua_State* L) {
    READ_USERDATA(L, TextSprite, textsprite, TEXTSPRITE);

    float size[2];
    textsprite_get_draw_size(textsprite, size);

    lua_pushnumber(L, (double)size[0]);
    lua_pushnumber(L, (double)size[1]);
    return 2;
}

static int script_textsprite_border_enable(lua_State* L) {
    READ_USERDATA(L, TextSprite, textsprite, TEXTSPRITE);

    bool enable = luaL_checkboolean(L, 2);

    textsprite_border_enable(textsprite, enable);

    return 0;
}

static int script_textsprite_border_set_size(lua_State* L) {
    READ_USERDATA(L, TextSprite, textsprite, TEXTSPRITE);

    float border_size = luaL_checkfloat(L, 2);

    textsprite_border_set_size(textsprite, border_size);

    return 0;
}

static int script_textsprite_border_set_color(lua_State* L) {
    READ_USERDATA(L, TextSprite, textsprite, TEXTSPRITE);

    float r = luaL_optionalfloat(L, 2);
    float g = luaL_optionalfloat(L, 3);
    float b = luaL_optionalfloat(L, 4);
    float a = luaL_optionalfloat(L, 5);

    textsprite_border_set_color(textsprite, r, g, b, a);

    return 0;
}

static int script_textsprite_set_antialiasing(lua_State* L) {
    READ_USERDATA(L, TextSprite, textsprite, TEXTSPRITE);

    PVRFLAG antialiasing = _parse_pvrflag(L, luaL_checkstring(L, 2));

    textsprite_set_antialiasing(textsprite, antialiasing);

    return 0;
}

static int script_textsprite_set_wordbreak(lua_State* L) {
    READ_USERDATA(L, TextSprite, textsprite, TEXTSPRITE);

    FontWordBreak wordbreak;
    const char* wordbreak_str = luaL_optstring(L, 2, NULL);

    if (string_equals(wordbreak_str, "loose")) {
        wordbreak = FONT_WORDBREAK_LOOSE;
    }
    else if (string_equals(wordbreak_str, "none")) {
        wordbreak = FONT_WORDBREAK_NONE;
    }
    else if (string_equals(wordbreak_str, "break")) {
        wordbreak = FONT_WORDBREAK_BREAK;
    }
    else {
        lua_pushfstring(L, "invalid wordbreak: %s", wordbreak_str);
        return lua_error(L);
    }

    textsprite_set_wordbreak(textsprite, wordbreak);

    return 0;
}

static int script_textsprite_set_shader(lua_State* L) {
    READ_USERDATA(L, TextSprite, textsprite, TEXTSPRITE);

    PSShader psshader = LUA_TOUSERDATA_OR_NULL(L, 2, PSShader, PSSHADER);

    textsprite_set_shader(textsprite, psshader);

    return 0;
}

static int script_textsprite_get_shader(lua_State* L) {
    READ_USERDATA(L, TextSprite, textsprite, TEXTSPRITE);

    PSShader psshader = textsprite_get_shader(textsprite);

    if (!psshader) {
        lua_pushnil(L);
        return 1;
    }

    return script_psshader_new(L, textsprite, psshader);
}





////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const luaL_Reg TEXTSPRITE_FUNCTIONS[] = {
    { "set_text", script_textsprite_set_text },
    { "set_font_size", script_textsprite_set_font_size },
    { "force_case", script_textsprite_force_case },
    { "set_paragraph_align", script_textsprite_set_paragraph_align },
    { "set_paragraph_space", script_textsprite_set_paragraph_space },
    { "set_maxlines", script_textsprite_set_maxlines },
    { "set_color_rgba8", script_textsprite_set_color_rgba8 },
    { "set_color", script_textsprite_set_color },
    { "set_alpha", script_textsprite_set_alpha },
    { "set_visible", script_textsprite_set_visible },
    { "set_draw_location", script_textsprite_set_draw_location },
    { "set_z_index", script_textsprite_set_z_index },
    { "set_z_offset", script_textsprite_set_z_offset },
    { "set_max_draw_size", script_textsprite_set_max_draw_size },
    { "matrix_flip", script_textsprite_matrix_flip },
    { "set_align", script_textsprite_set_align },
    { "matrix_get_modifier", script_textsprite_matrix_get_modifier },
    { "matrix_reset", script_textsprite_matrix_reset },
    { "get_font_size", script_textsprite_get_font_size },
    { "get_draw_size", script_textsprite_get_draw_size },
    { "border_enable", script_textsprite_border_enable },
    { "border_set_size", script_textsprite_border_set_size },
    { "border_set_color", script_textsprite_border_set_color },
    { "set_antialiasing", script_textsprite_set_antialiasing },
    { "set_wordbreak", script_textsprite_set_wordbreak },
    { "set_shader", script_textsprite_set_shader },
    { "get_shader", script_textsprite_get_shader },
    {NULL, NULL}
};

int script_textsprite_new(lua_State* L, Layout layout, TextSprite textsprite) {
    return NEW_USERDATA(L, TEXTSPRITE, layout, textsprite, true);
}

static int script_textsprite_gc(lua_State* L) {
    READ_USERDATA_UNCHECKED(L, TextSprite, textsprite, TEXTSPRITE);
    _luascript_suppress_item(L, textsprite, true);
    return 0;
}

static int script_textsprite_tostring(lua_State* L) {
    READ_USERDATA(L, TextSprite, textsprite, TEXTSPRITE);
    lua_pushstring(L, "[TextSprite]");
    return 1;
}


inline void register_textsprite(lua_State* L) {
    _luascript_register(
        L,
        TEXTSPRITE,
        script_textsprite_gc,
        script_textsprite_tostring,
        TEXTSPRITE_FUNCTIONS
    );
}

