using System;
using Engine.Externals.LuaInterop;
using Engine.Image;
using Engine.Platform;

namespace Engine.Externals.LuaScriptInterop {

    public static class ExportsSprite {
        private const string SPRITE = "Sprite";


        static int script_sprite_matrix_get_modifier(LuaState L) {
            Sprite sprite = L.ReadUserdata<Sprite>(SPRITE);

            Modifier modifier = sprite.MatrixGetModifier();
            return ExportsModifier.script_modifier_new(L, modifier);
        }

        static int script_sprite_set_offset_source(LuaState L) {
            Sprite sprite = L.ReadUserdata<Sprite>(SPRITE);

            float x = (float)L.luaL_checknumber(2);
            float y = (float)L.luaL_checknumber(3);
            float width = (float)L.luaL_checknumber(4);
            float height = (float)L.luaL_checknumber(5);

            sprite.SetOffsetSource(x, y, width, height);

            return 0;
        }

        static int script_sprite_set_offset_frame(LuaState L) {
            Sprite sprite = L.ReadUserdata<Sprite>(SPRITE);

            float x = (float)L.luaL_checknumber(2);
            float y = (float)L.luaL_checknumber(3);
            float width = (float)L.luaL_checknumber(4);
            float height = (float)L.luaL_checknumber(5);

            sprite.SetOffsetFrame(x, y, width, height);

            return 0;
        }

        static int script_sprite_set_offset_pivot(LuaState L) {
            Sprite sprite = L.ReadUserdata<Sprite>(SPRITE);

            float x = (float)L.luaL_checknumber(2);
            float y = (float)L.luaL_checknumber(3);

            sprite.SetOffsetPivot(x, y);
            return 0;
        }

        static int script_sprite_matrix_reset(LuaState L) {
            Sprite sprite = L.ReadUserdata<Sprite>(SPRITE);

            sprite.MatrixReset();

            return 0;
        }

        static int script_sprite_set_draw_location(LuaState L) {
            Sprite sprite = L.ReadUserdata<Sprite>(SPRITE);

            float x = (float)L.luaL_checknumber(2);
            float y = (float)L.luaL_checknumber(3);

            sprite.SetDrawLocation(x, y);

            return 0;
        }

        static int script_sprite_set_draw_size(LuaState L) {
            Sprite sprite = L.ReadUserdata<Sprite>(SPRITE);

            float width = (float)L.luaL_checknumber(2);
            float height = (float)L.luaL_checknumber(3);

            sprite.SetDrawSize(width, height);

            return 0;
        }

        static int script_sprite_set_draw_size_from_source_size(LuaState L) {
            Sprite sprite = L.ReadUserdata<Sprite>(SPRITE);

            sprite.SetDrawSizeFromSourceSize();

            return 0;
        }

        static int script_sprite_set_alpha(LuaState L) {
            Sprite sprite = L.ReadUserdata<Sprite>(SPRITE);

            float alpha = (float)L.luaL_checknumber(2);

            sprite.SetAlpha(alpha);

            return 0;
        }

        static int script_sprite_set_visible(LuaState L) {
            Sprite sprite = L.ReadUserdata<Sprite>(SPRITE);

            bool visible = L.lua_toboolean(2);

            sprite.SetVisible(visible);

            return 0;
        }

        static int script_sprite_set_z_index(LuaState L) {
            Sprite sprite = L.ReadUserdata<Sprite>(SPRITE);

            float index = (float)L.luaL_checknumber(2);

            sprite.SetZIndex(index);

            return 0;
        }

        static int script_sprite_set_z_offset(LuaState L) {
            Sprite sprite = L.ReadUserdata<Sprite>(SPRITE);

            float index = (float)L.luaL_checknumber(2);

            sprite.SetZOffset(index);

            return 0;
        }

        static int script_sprite_get_source_size(LuaState L) {
            Sprite sprite = L.ReadUserdata<Sprite>(SPRITE);

            float size_width, size_height;
            sprite.GetSourceSize(out size_width, out size_height);

            L.lua_pushnumber(size_width);
            L.lua_pushnumber(size_height);

            return 2;
        }

        static int script_sprite_set_vertex_color(LuaState L) {
            Sprite sprite = L.ReadUserdata<Sprite>(SPRITE);

            float r = (float)L.luaL_optnumber(2, Double.NaN);
            float g = (float)L.luaL_optnumber(3, Double.NaN);
            float b = (float)L.luaL_optnumber(4, Double.NaN);

            sprite.SetVertexColor(r, g, b);

            return 0;
        }

        static int script_sprite_set_offsetcolor(LuaState L) {
            Sprite sprite = L.ReadUserdata<Sprite>(SPRITE);

            float r = (float)L.luaL_optnumber(2, Double.NaN);
            float g = (float)L.luaL_optnumber(3, Double.NaN);
            float b = (float)L.luaL_optnumber(4, Double.NaN);
            float a = (float)L.luaL_optnumber(5, Double.NaN);

            sprite.SetOffsetColor(r, g, b, a);

            return 0;
        }

        static int script_sprite_is_textured(LuaState L) {
            Sprite sprite = L.ReadUserdata<Sprite>(SPRITE);

            bool ret = sprite.IsTextured();

            L.lua_pushboolean(ret);
            return 1;
        }

        static int script_sprite_crop(LuaState L) {
            Sprite sprite = L.ReadUserdata<Sprite>(SPRITE);

            float dx = (float)L.luaL_optnumber(2, Double.NaN);
            float dy = (float)L.luaL_optnumber(3, Double.NaN);
            float dwidth = (float)L.luaL_optnumber(4, Double.NaN);
            float dheight = (float)L.luaL_optnumber(5, Double.NaN);

            bool ret = sprite.Crop(dx, dy, dwidth, dheight);

            L.lua_pushboolean(ret);
            return 1;
        }

        static int script_sprite_is_crop_enabled(LuaState L) {
            Sprite sprite = L.ReadUserdata<Sprite>(SPRITE);

            bool ret = sprite.IsCropEnabled();

            L.lua_pushboolean(ret);
            return 1;
        }

        static int script_sprite_crop_enable(LuaState L) {
            Sprite sprite = L.ReadUserdata<Sprite>(SPRITE);

            bool enable = L.lua_toboolean(2);

            sprite.CropEnable(enable);

            return 0;
        }

        static int script_sprite_resize_draw_size(LuaState L) {
            Sprite sprite = L.ReadUserdata<Sprite>(SPRITE);

            float max_width = (float)L.luaL_checknumber(2);
            float max_height = (float)L.luaL_checknumber(3);

            float applied_draw_width, applied_draw_height;
            sprite.ResizeDrawSize(max_width, max_height, out applied_draw_width, out applied_draw_height);

            L.lua_pushnumber(applied_draw_width);
            L.lua_pushnumber(applied_draw_height);
            return 2;
        }

        static int script_sprite_center_draw_location(LuaState L) {
            Sprite sprite = L.ReadUserdata<Sprite>(SPRITE);

            float x = (float)L.luaL_checknumber(2);
            float y = (float)L.luaL_checknumber(3);
            float ref_width = (float)L.luaL_checknumber(4);
            float ref_height = (float)L.luaL_checknumber(5);

            float applied_draw_x, applied_draw_y;
            sprite.CenterDrawLocation(x, y, ref_width, ref_height, out applied_draw_x, out applied_draw_y);

            L.lua_pushnumber(applied_draw_x);
            L.lua_pushnumber(applied_draw_y);
            return 2;
        }

        static int script_sprite_set_antialiasing(LuaState L) {
            Sprite sprite = L.ReadUserdata<Sprite>(SPRITE);

            PVRContextFlag antialiasing = LuascriptHelpers.ParsePVRFLAG(L, L.luaL_checkstring(2));

            sprite.SetAntialiasing(antialiasing);

            return 0;
        }

        static int script_sprite_set_shader(LuaState L) {
            Sprite sprite = L.ReadUserdata<Sprite>(SPRITE);

            PSShader psshader = L.ReadNullableUserdata<PSShader>(2, ExportsPSShader.PSSHADER);

            sprite.SetShader(psshader);

            return 0;
        }

        static int script_sprite_get_shader(LuaState L) {
            Sprite sprite = L.ReadUserdata<Sprite>(SPRITE);

            PSShader psshader = sprite.GetShader();

            if (psshader == null) {
                L.lua_pushnil();
                return 1;
            }

            return ExportsPSShader.script_psshader_new(L, psshader);
        }


        static int script_sprite_flip_rendered_texture(LuaState L) {
            Sprite sprite = L.ReadUserdata<Sprite>(SPRITE);

            bool? flip_x = null, flip_y = null;

            if (!L.lua_isnil(2)) flip_x = L.lua_toboolean(2);
            if (!L.lua_isnil(3)) flip_y = L.lua_toboolean(3);

            sprite.FlipRenderedTexture(flip_x, flip_y);

            return 0;
        }

        static int script_sprite_flip_rendered_texture_enable_correction(LuaState L) {
            Sprite sprite = L.ReadUserdata<Sprite>(SPRITE);

            bool enabled = L.lua_toboolean(2);

            sprite.FlipRenderedTextureEnableCorrection(enabled);

            return 0;
        }

        ////////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////

        static readonly LuaTableFunction[] SPRITE_FUNCTIONS = {
            new LuaTableFunction("matrix_get_modifier", script_sprite_matrix_get_modifier),
            new LuaTableFunction("set_offset_source", script_sprite_set_offset_source),
            new LuaTableFunction("set_offset_frame", script_sprite_set_offset_frame),
            new LuaTableFunction("set_offset_pivot", script_sprite_set_offset_pivot),
            new LuaTableFunction("matrix_reset", script_sprite_matrix_reset),
            new LuaTableFunction("set_draw_location", script_sprite_set_draw_location),
            new LuaTableFunction("set_draw_size", script_sprite_set_draw_size),
            new LuaTableFunction("set_draw_size_from_source_size", script_sprite_set_draw_size_from_source_size),
            new LuaTableFunction("set_alpha", script_sprite_set_alpha),
            new LuaTableFunction("set_visible", script_sprite_set_visible),
            new LuaTableFunction("set_z_index", script_sprite_set_z_index),
            new LuaTableFunction("set_z_offset", script_sprite_set_z_offset),
            new LuaTableFunction("get_source_size", script_sprite_get_source_size),
            new LuaTableFunction("set_vertex_color", script_sprite_set_vertex_color),
            new LuaTableFunction("set_offsetcolor", script_sprite_set_offsetcolor),
            new LuaTableFunction("is_textured", script_sprite_is_textured),
            new LuaTableFunction("crop", script_sprite_crop),
            new LuaTableFunction("is_crop_enabled", script_sprite_is_crop_enabled),
            new LuaTableFunction("crop_enable", script_sprite_crop_enable),
            new LuaTableFunction("resize_draw_size", script_sprite_resize_draw_size),
            new LuaTableFunction("center_draw_location", script_sprite_center_draw_location),
            new LuaTableFunction("set_antialiasing", script_sprite_set_antialiasing),
            new LuaTableFunction("flip_rendered_texture", script_sprite_flip_rendered_texture),
            new LuaTableFunction("flip_rendered_texture_enable_correction", script_sprite_flip_rendered_texture_enable_correction),
            new LuaTableFunction("set_shader", script_sprite_set_shader),
            new LuaTableFunction("get_shader", script_sprite_get_shader),
            new LuaTableFunction(null, null)
        };


        internal static int script_sprite_new(LuaState L, Sprite sprite) {
            return L.CreateUserdata(SPRITE, sprite);
        }

        static int script_sprite_gc(LuaState L) {
            return L.GC_userdata(SPRITE);
        }

        static int script_sprite_tostring(LuaState L) {
            return L.ToString_userdata(SPRITE);
        }


        private static readonly LuaCallback delegate_gc = script_sprite_gc;
        private static readonly LuaCallback delegate_tostring = script_sprite_tostring;

        internal static void script_sprite_register(ManagedLuaState lua) {
            lua.RegisterMetaTable(SPRITE, delegate_gc, delegate_tostring, SPRITE_FUNCTIONS);
        }

    }
}
