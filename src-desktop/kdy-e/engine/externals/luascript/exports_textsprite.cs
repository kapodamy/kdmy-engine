using Engine.Externals.LuaInterop;
using Engine.Font;
using Engine.Platform;
using Engine.Utils;

namespace Engine.Externals.LuaScriptInterop {

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

            float font_size = L.luaL_checkfloat(2);

            textsprite.SetFontSize(font_size);

            return 0;
        }

        static int script_textsprite_force_case(LuaState L) {
            TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);

            string str = L.luaL_optstring(2, null);

            int uppercase_or_lowecase_or_none;
            if (str == "lower")
                uppercase_or_lowecase_or_none = VertexProps.TEXTSPRITE_FORCE_LOWERCASE;
            else if (str == "upper")
                uppercase_or_lowecase_or_none = VertexProps.TEXTSPRITE_FORCE_UPPERCASE;
            else
                uppercase_or_lowecase_or_none = VertexProps.TEXTSPRITE_FORCE_NONE;

            textsprite.ForceCase(uppercase_or_lowecase_or_none);

            return 0;
        }

        static int script_textsprite_set_paragraph_align(LuaState L) {
            TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);

            string str = L.luaL_optstring(2, null);

            Align align = VertexProps.ParseAlign2(str);
            if (align == Align.INVALID) return L.luaL_argerror(2, "invalid align");

            textsprite.SetParagraphAlign(align);

            return 0;
        }

        static int script_textsprite_set_paragraph_space(LuaState L) {
            TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);

            float space = L.luaL_checkfloat(2);

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

            float r = L.luaL_optionalfloat(2);
            float g = L.luaL_optionalfloat(3);
            float b = L.luaL_optionalfloat(4);

            textsprite.SetColor(r, g, b);

            return 0;
        }

        static int script_textsprite_set_alpha(LuaState L) {
            TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);

            float alpha = L.luaL_checkfloat(2);

            textsprite.SetAlpha(alpha);

            return 0;
        }

        static int script_textsprite_set_visible(LuaState L) {
            TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);

            bool visible = L.luaL_checkboolean(2);

            textsprite.SetVisible(visible);

            return 0;
        }

        static int script_textsprite_set_draw_location(LuaState L) {
            TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);

            float x = L.luaL_checkfloat(2);
            float y = L.luaL_checkfloat(3);

            textsprite.SetDrawLocation(x, y);

            return 0;
        }

        static int script_textsprite_set_z_index(LuaState L) {
            TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);

            float z_index = L.luaL_checkfloat(2);

            textsprite.SetZIndex(z_index);

            return 0;
        }

        static int script_textsprite_set_z_offset(LuaState L) {
            TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);

            float offset = L.luaL_checkfloat(2);

            textsprite.SetZOffset(offset);

            return 0;
        }

        static int script_textsprite_set_max_draw_size(LuaState L) {
            TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);

            float max_width = L.luaL_checkfloat(2);
            float max_height = L.luaL_checkfloat(3);

            textsprite.SetMaxDrawSize(max_width, max_height);

            return 0;
        }

        static int script_textsprite_matrix_flip(LuaState L) {
            TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);

            bool flip_x = L.luaL_checkboolean(2);
            bool flip_y = L.luaL_checkboolean(3);

            textsprite.MatrixFlip(flip_x, flip_y);

            return 0;
        }

        static int script_textsprite_set_align(LuaState L) {
            TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);

            string str1 = L.luaL_optstring(2, null);
            string str2 = L.luaL_optstring(3, null);

            Align align_vertical = VertexProps.ParseAlign2(str1);
            Align align_horizontal = VertexProps.ParseAlign2(str2);

            if (align_vertical == Align.INVALID) return L.luaL_argerror(2, "invalid align");
            if (align_horizontal == Align.INVALID) return L.luaL_argerror(3, "invalid align");

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

            bool enable = L.luaL_checkboolean(2);

            textsprite.BorderEnable(enable);

            return 0;
        }

        static int script_textsprite_border_set_size(LuaState L) {
            TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);

            float border_size = L.luaL_checkfloat(2);

            textsprite.BorderSetSize(border_size);

            return 0;
        }

        static int script_textsprite_border_set_color(LuaState L) {
            TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);

            float r = L.luaL_optionalfloat(2);
            float g = L.luaL_optionalfloat(3);
            float b = L.luaL_optionalfloat(4);
            float a = L.luaL_optionalfloat(5);

            textsprite.BorderSetColor(r, g, b, a);

            return 0;
        }

        static int script_textsprite_set_antialiasing(LuaState L) {
            TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);

            PVRContextFlag antialiasing = VertexProps.ParseFlag2(L.luaL_checkstring(2), PVRContextFlag.INVALID_VALUE);
            if (antialiasing == PVRContextFlag.INVALID_VALUE) return L.luaL_argerror(2, "invalid pvrflag");

            textsprite.SetAntialiasing(antialiasing);

            return 0;
        }

        static int script_textsprite_set_wordbreak(LuaState L) {
            TextSprite textsprite = L.ReadUserdata<TextSprite>(TEXTSPRITE);

            int wordbreak;
            string wordbreak_str = L.luaL_optstring(2, null);

            if (wordbreak_str == "loose") {
                wordbreak = VertexProps.FONT_WORDBREAK_LOOSE;
            } else if (wordbreak_str == "none") {
                wordbreak = VertexProps.FONT_WORDBREAK_NONE;
            } else if (wordbreak_str == "break") {
                wordbreak = VertexProps.FONT_WORDBREAK_BREAK;
            } else {
                return L.luaL_argerror(2, $"invalid wordbreak: {wordbreak_str}");
            }

            textsprite.SetWordbreak(wordbreak);

            return 0;
        }




        ////////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////

        static readonly LuaTableFunction[] TEXTSPRITE_FUNCTIONS = {
            new LuaTableFunction() { name = "set_text", func = script_textsprite_set_text },
            new LuaTableFunction() { name = "set_font_size", func = script_textsprite_set_font_size },
            new LuaTableFunction() { name = "force_case", func = script_textsprite_force_case },
            new LuaTableFunction() { name = "set_paragraph_align", func = script_textsprite_set_paragraph_align },
            new LuaTableFunction() { name = "set_paragraph_space", func = script_textsprite_set_paragraph_space },
            new LuaTableFunction() { name = "set_maxlines", func = script_textsprite_set_maxlines },
            new LuaTableFunction() { name = "set_color_rgba8", func = script_textsprite_set_color_rgba8 },
            new LuaTableFunction() { name = "set_color", func = script_textsprite_set_color },
            new LuaTableFunction() { name = "set_alpha", func = script_textsprite_set_alpha },
            new LuaTableFunction() { name = "set_visible", func = script_textsprite_set_visible },
            new LuaTableFunction() { name = "set_draw_location", func = script_textsprite_set_draw_location },
            new LuaTableFunction() { name = "set_z_index", func = script_textsprite_set_z_index },
            new LuaTableFunction() { name = "set_z_offset", func = script_textsprite_set_z_offset },
            new LuaTableFunction() { name = "set_max_draw_size", func = script_textsprite_set_max_draw_size },
            new LuaTableFunction() { name = "matrix_flip", func = script_textsprite_matrix_flip },
            new LuaTableFunction() { name = "set_align", func = script_textsprite_set_align },
            new LuaTableFunction() { name = "matrix_get_modifier", func = script_textsprite_matrix_get_modifier },
            new LuaTableFunction() { name = "matrix_reset", func = script_textsprite_matrix_reset },
            new LuaTableFunction() { name = "get_font_size", func = script_textsprite_get_font_size },
            new LuaTableFunction() { name = "get_draw_size", func = script_textsprite_get_draw_size },
            new LuaTableFunction() { name = "border_enable", func = script_textsprite_border_enable },
            new LuaTableFunction() { name = "border_set_size", func = script_textsprite_border_set_size },
            new LuaTableFunction() { name = "border_set_color", func = script_textsprite_border_set_color },
            new LuaTableFunction() { name = "set_antialiasing", func = script_textsprite_set_antialiasing },
            new LuaTableFunction() { name = "set_wordbreak", func = script_textsprite_set_wordbreak },
            new LuaTableFunction() { name = null, func = null }
        };

        internal static int script_textsprite_new(LuaState L, TextSprite textsprite) {
            return L.CreateUserdata(TEXTSPRITE, textsprite, true);
        }

        static int script_textsprite_gc(LuaState L) {
            return L.DestroyUserdata(TEXTSPRITE);
        }

        static int script_textsprite_tostring(LuaState L) {
            L.lua_pushstring("[TextSprite]");
            return 1;
        }


        private static readonly LuaCallback gc = script_textsprite_gc;
        private static readonly LuaCallback tostring = script_textsprite_tostring;

        internal static void register_textsprite(ManagedLuaState lua) {
            lua.RegisterMetaTable(
                TEXTSPRITE,
                gc,
                tostring,
                TEXTSPRITE_FUNCTIONS
            );
        }

    }
}
