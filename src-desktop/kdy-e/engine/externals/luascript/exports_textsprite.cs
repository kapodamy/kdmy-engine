using System;
using Engine.Externals.LuaInterop;
using Engine.Font;
using Engine.Platform;
using Engine.Utils;

namespace Engine.Externals.LuaScriptInterop;

public static class ExportsTextSprite {
    private const string TEXTSPRITE = "TextSprite";



    static int script_textsprite_set_text(LuaState L) {
        TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);

        string text = L.luaL_optstring(2, null);

        textsprite.SetTextIntern(false, text);

        return 0;
    }

    static int script_textsprite_set_font_size(LuaState L) {
        TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);

        float font_size = (float)L.luaL_checknumber(2);

        textsprite.SetFontSize(font_size);

        return 0;
    }

    static int script_textsprite_force_case(LuaState L) {
        TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);

        int uppercase_or_lowecase_or_none = LuascriptHelpers.ParseForcecase(L, L.luaL_optstring(2, null));

        textsprite.ForceCase(uppercase_or_lowecase_or_none);

        return 0;
    }

    static int script_textsprite_set_paragraph_align(LuaState L) {
        TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);

        Align align = LuascriptHelpers.ParseAlign(L, L.luaL_optstring(2, null));

        textsprite.SetParagraphAlign(align);

        return 0;
    }

    static int script_textsprite_set_paragraph_space(LuaState L) {
        TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);

        float space = (float)L.luaL_checknumber(2);

        textsprite.SetParagraphSpace(space);

        return 0;
    }

    static int script_textsprite_set_maxlines(LuaState L) {
        TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);

        int max_lines = (int)L.luaL_checkinteger(2);

        textsprite.SetMaxlines(max_lines);

        return 0;
    }

    static int script_textsprite_set_color_rgba8(LuaState L) {
        TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);

        long number = L.luaL_checkinteger(2);
        uint rbga8_color = (uint)number;

        textsprite.SetColorRGBA8(rbga8_color);

        return 0;
    }

    static int script_textsprite_set_color(LuaState L) {
        TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);

        float r = (float)L.luaL_optnumber(2, Double.NaN);
        float g = (float)L.luaL_optnumber(3, Double.NaN);
        float b = (float)L.luaL_optnumber(4, Double.NaN);

        textsprite.SetColor(r, g, b);

        return 0;
    }

    static int script_textsprite_set_alpha(LuaState L) {
        TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);

        float alpha = (float)L.luaL_checknumber(2);

        textsprite.SetAlpha(alpha);

        return 0;
    }

    static int script_textsprite_set_visible(LuaState L) {
        TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);

        bool visible = L.lua_toboolean(2);

        textsprite.SetVisible(visible);

        return 0;
    }

    static int script_textsprite_set_draw_location(LuaState L) {
        TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);

        float x = (float)L.luaL_checknumber(2);
        float y = (float)L.luaL_checknumber(3);

        textsprite.SetDrawLocation(x, y);

        return 0;
    }

    static int script_textsprite_set_z_index(LuaState L) {
        TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);

        float z_index = (float)L.luaL_checknumber(2);

        textsprite.SetZIndex(z_index);

        return 0;
    }

    static int script_textsprite_set_z_offset(LuaState L) {
        TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);

        float offset = (float)L.luaL_checknumber(2);

        textsprite.SetZOffset(offset);

        return 0;
    }

    static int script_textsprite_set_max_draw_size(LuaState L) {
        TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);

        float max_width = (float)L.luaL_checknumber(2);
        float max_height = (float)L.luaL_checknumber(3);

        textsprite.SetMaxDrawSize(max_width, max_height);

        return 0;
    }

    static int script_textsprite_matrix_flip(LuaState L) {
        TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);

        bool flip_x = L.lua_toboolean(2);
        bool flip_y = L.lua_toboolean(3);

        textsprite.MatrixFlip(flip_x, flip_y);

        return 0;
    }

    static int script_textsprite_set_align(LuaState L) {
        TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);

        Align align_vertical = LuascriptHelpers.ParseAlign(L, L.luaL_optstring(2, null));
        Align align_horizontal = LuascriptHelpers.ParseAlign(L, L.luaL_optstring(3, null));

        textsprite.SetAlign(align_vertical, align_horizontal);

        return 0;
    }

    static int script_textsprite_matrix_get_modifier(LuaState L) {
        TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);

        Modifier modifier = textsprite.MatrixGetModifier();

        return ExportsModifier.script_modifier_new(L, modifier);
    }

    static int script_textsprite_matrix_reset(LuaState L) {
        TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);

        textsprite.MatrixReset();

        return 0;
    }

    static int script_textsprite_get_font_size(LuaState L) {
        TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);

        float font_size = textsprite.GetFontSize();

        L.lua_pushnumber(font_size);
        return 1;
    }

    static int script_textsprite_get_draw_size(LuaState L) {
        TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);

        float size_width, size_height;
        textsprite.GetDrawSize(out size_width, out size_height);

        L.lua_pushnumber(size_width);
        L.lua_pushnumber(size_height);
        return 2;
    }

    static int script_textsprite_border_enable(LuaState L) {
        TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);

        bool enable = L.lua_toboolean(2);

        textsprite.BorderEnable(enable);

        return 0;
    }

    static int script_textsprite_border_set_size(LuaState L) {
        TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);

        float border_size = (float)L.luaL_checknumber(2);

        textsprite.BorderSetSize(border_size);

        return 0;
    }

    static int script_textsprite_border_set_color(LuaState L) {
        TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);

        float r = (float)L.luaL_optnumber(2, Double.NaN);
        float g = (float)L.luaL_optnumber(3, Double.NaN);
        float b = (float)L.luaL_optnumber(4, Double.NaN);
        float a = (float)L.luaL_optnumber(5, Double.NaN);

        textsprite.BorderSetColor(r, g, b, a);

        return 0;
    }

    static int script_textsprite_border_set_offset(LuaState L) {
        TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);

        float x = (float)L.luaL_optnumber(2, Double.NaN);
        float y = (float)L.luaL_optnumber(3, Double.NaN);

        textsprite.BorderSetOffset(x, y);

        return 0;
    }

    static int script_textsprite_set_antialiasing(LuaState L) {
        TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);

        PVRContextFlag antialiasing = LuascriptHelpers.ParsePVRFLAG(L, L.luaL_checkstring(2));

        textsprite.SetAntialiasing(antialiasing);

        return 0;
    }

    static int script_textsprite_set_wordbreak(LuaState L) {
        TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);

        int wordbreak = LuascriptHelpers.ParseWordbreak(L, L.luaL_optstring(2, null));

        textsprite.SetWordbreak(wordbreak);

        return 0;
    }

    static int script_textsprite_set_shader(LuaState L) {
        TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);

        PSShader psshader = L.ReadNullableUserdata<PSShader>(2, ExportsPSShader.PSSHADER);

        textsprite.SetShader(psshader);

        return 0;
    }

    static int script_textsprite_get_shader(LuaState L) {
        TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);

        PSShader psshader = textsprite.GetShader();

        if (psshader == null) {
            L.lua_pushnil();
            return 1;
        }

        return ExportsPSShader.script_psshader_new(L, psshader);
    }
    
    static int script_textsprite_blend_enable(LuaState L) {
        TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);
        bool enabled = L.lua_toboolean(2);

        textsprite.BlendEnable(enabled);

        return 0;
    }

    static int script_textsprite_blend_set(LuaState L) {
        TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);
        Blend src_rgb = LuascriptHelpers.ParseBlend(L, L.luaL_optstring(2, null));
        Blend dst_rgb = LuascriptHelpers.ParseBlend(L, L.luaL_optstring(3, null));
        Blend src_alpha = LuascriptHelpers.ParseBlend(L, L.luaL_optstring(4, null));
        Blend dst_alpha = LuascriptHelpers.ParseBlend(L, L.luaL_optstring(5, null));

        textsprite.BlendSet(src_rgb, dst_rgb, src_alpha, dst_alpha);

        return 0;
    }

    static int script_textsprite_background_enable(LuaState L) {
        TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);
        bool enabled = L.lua_toboolean(2);

        textsprite.BackgroundEnable(enabled);

        return 0;
    }

    static int script_textsprite_background_set_size(LuaState L) {
        TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);
        float size = (float)L.luaL_checknumber(2);

        textsprite.BackgroundSetSize(size);

        return 0;
    }

    static int script_textsprite_background_set_offets(LuaState L) {
        TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);
        float offset_x = (float)L.luaL_checknumber(2);
        float offset_y = (float)L.luaL_checknumber(3);

        textsprite.BackgroundSetOffets(offset_x, offset_y);

        return 0;
    }

    static int script_textsprite_background_set_color(LuaState L) {
        TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);
        float r = (float)L.luaL_checknumber(2);
        float g = (float)L.luaL_checknumber(3);
        float b = (float)L.luaL_checknumber(4);
        float a = (float)L.luaL_checknumber(5);

        textsprite.BackgroundSetColor(r, g, b, a);

        return 0;
    }




    ////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////

    static readonly LuaTableFunction[] TEXTSPRITE_FUNCTIONS = {
        new LuaTableFunction("set_text", script_textsprite_set_text),
        new LuaTableFunction("set_font_size", script_textsprite_set_font_size),
        new LuaTableFunction("force_case", script_textsprite_force_case),
        new LuaTableFunction("set_paragraph_align", script_textsprite_set_paragraph_align),
        new LuaTableFunction("set_paragraph_space", script_textsprite_set_paragraph_space),
        new LuaTableFunction("set_maxlines", script_textsprite_set_maxlines),
        new LuaTableFunction("set_color_rgba8", script_textsprite_set_color_rgba8),
        new LuaTableFunction("set_color", script_textsprite_set_color),
        new LuaTableFunction("set_alpha", script_textsprite_set_alpha),
        new LuaTableFunction("set_visible", script_textsprite_set_visible),
        new LuaTableFunction("set_draw_location", script_textsprite_set_draw_location),
        new LuaTableFunction("set_z_index", script_textsprite_set_z_index),
        new LuaTableFunction("set_z_offset", script_textsprite_set_z_offset),
        new LuaTableFunction("set_max_draw_size", script_textsprite_set_max_draw_size),
        new LuaTableFunction("matrix_flip", script_textsprite_matrix_flip),
        new LuaTableFunction("set_align", script_textsprite_set_align),
        new LuaTableFunction("matrix_get_modifier", script_textsprite_matrix_get_modifier),
        new LuaTableFunction("matrix_reset", script_textsprite_matrix_reset),
        new LuaTableFunction("get_font_size", script_textsprite_get_font_size),
        new LuaTableFunction("get_draw_size", script_textsprite_get_draw_size),
        new LuaTableFunction("border_enable", script_textsprite_border_enable),
        new LuaTableFunction("border_set_size", script_textsprite_border_set_size),
        new LuaTableFunction("border_set_color", script_textsprite_border_set_color),
        new LuaTableFunction("border_set_offset", script_textsprite_border_set_offset),
        new LuaTableFunction("set_antialiasing", script_textsprite_set_antialiasing),
        new LuaTableFunction("set_wordbreak", script_textsprite_set_wordbreak),
        new LuaTableFunction("set_shader", script_textsprite_set_shader),
        new LuaTableFunction("get_shader", script_textsprite_get_shader),
        new LuaTableFunction("blend_enable", script_textsprite_blend_enable),
        new LuaTableFunction("blend_set", script_textsprite_blend_set),
        new LuaTableFunction("background_enable", script_textsprite_background_enable),
        new LuaTableFunction("background_set_size", script_textsprite_background_set_size),
        new LuaTableFunction("background_set_offets", script_textsprite_background_set_offets),
        new LuaTableFunction("background_set_color", script_textsprite_background_set_color),
        new LuaTableFunction(null, null)
    };

    internal static int script_textsprite_new(LuaState L, TextSprite textsprite) {
        return L.CreateUserdata(TEXTSPRITE, textsprite);
    }

    static int script_textsprite_gc(LuaState L) {
        return L.GC_userdata(TEXTSPRITE);
    }

    static int script_textsprite_tostring(LuaState L) {
        return L.ToString_userdata(TEXTSPRITE);
    }


    private static readonly LuaCallback delegate_gc = script_textsprite_gc;
    private static readonly LuaCallback delegate_tostring = script_textsprite_tostring;

    internal static void script_textsprite_register(ManagedLuaState lua) {
        lua.RegisterMetaTable(TEXTSPRITE, delegate_gc, delegate_tostring, TEXTSPRITE_FUNCTIONS);
    }

}
