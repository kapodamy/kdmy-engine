using Engine.Externals.LuaInterop;
using Engine.Utils;

namespace Engine.Externals.LuaScriptInterop {

    internal static class ExportsModifier {

        private const string MODIFIER = "Modifier";
        private const string TOSTRING_FORMAT = "{ " +
                                                "translateX: $6f, " +
                                                "translateY: $6f, " +
                                                "rotate: $6f, " +
                                                "skewX: $6f, " +
                                                "skewY: $6f, " +
                                                "scaleX: $6f, " +
                                                "scaleY: $6f, " +
                                                "scaleDirectionX: $6f, " +
                                                "scaleDirectionY: $6f, " +
                                                "rotatePivotEnabled: $b, " +
                                                "rotatePivotU: $6f, " +
                                                "rotatePivotV: $6f, " +
                                                "translateRotation: $b, " +
                                                "scaleSize: $b, " +
                                                "scaleTranslation: $b, " +
                                                "translateRotation: $b, " +
                                                "x: $6f, " +
                                                "y: $6f, " +
                                                "width: $6f, " +
                                                "height: $6f, " +
                                                "}";


        internal static int script_modifier_new(LuaState L, Modifier modifier) {
            return L.CreateUserdata(MODIFIER, modifier);
        }

        static int script_modifier_gc(LuaState L) {
            return L.NullifyUserdata(MODIFIER);
        }

        static int script_modifier_tostring(LuaState L) {
            Modifier modifier = L.ReadUserdata<Modifier>(MODIFIER);

            L.lua_pushstring(StringUtils.CreateFormattedString(
                TOSTRING_FORMAT,
                modifier.translate_x,
                modifier.translate_y,
                modifier.rotate,
                modifier.skew_x,
                modifier.skew_y,
                modifier.scale_x,
                modifier.scale_y,
                modifier.scale_direction_x,
                modifier.scale_direction_y,
                modifier.rotate_pivot_enabled,
                modifier.rotate_pivot_u,
                modifier.rotate_pivot_v,
                modifier.translate_rotation,
                modifier.scale_size,
                modifier.scale_translation,
                modifier.x,
                modifier.y,
                modifier.width,
                modifier.height
            ));

            return 1;
        }

        static int script_modifier_index(LuaState L) {
            Modifier modifier = L.ReadUserdata<Modifier>(MODIFIER);
            string field = L.luaL_optstring(2, null);

            if (field == "translateX")
                L.lua_pushnumber(modifier.translate_x);
            else if (field == "translateY")
                L.lua_pushnumber(modifier.translate_y);
            else if (field == "rotate")
                L.lua_pushnumber(modifier.rotate);
            else if (field == "skewX")
                L.lua_pushnumber(modifier.skew_x);
            else if (field == "skewY")
                L.lua_pushnumber(modifier.skew_y);
            else if (field == "scaleX")
                L.lua_pushnumber(modifier.scale_x);
            else if (field == "scaleY")
                L.lua_pushnumber(modifier.scale_y);
            else if (field == "scaleDirectionX")
                L.lua_pushnumber(modifier.scale_direction_x);
            else if (field == "scaleDirectionY")
                L.lua_pushnumber(modifier.scale_direction_y);
            else if (field == "rotatePivotEnabled")
                L.lua_pushboolean(modifier.rotate_pivot_enabled == true);
            else if (field == "rotatePivotU")
                L.lua_pushnumber(modifier.rotate_pivot_u);
            else if (field == "rotatePivotV")
                L.lua_pushnumber(modifier.rotate_pivot_v);
            else if (field == "translateRotation")
                L.lua_pushboolean(modifier.translate_rotation == true);
            else if (field == "scaleSize")
                L.lua_pushboolean(modifier.scale_size == true);
            else if (field == "scaleTranslation")
                L.lua_pushboolean(modifier.scale_translation == true);
            else if (field == "x")
                L.lua_pushnumber(modifier.x);
            else if (field == "y")
                L.lua_pushnumber(modifier.y);
            else if (field == "width")
                L.lua_pushnumber(modifier.width);
            else if (field == "height")
                L.lua_pushnumber(modifier.height);
            else
                L.lua_pushnil();

            return 1;
        }

        static int script_modifier_newindex(LuaState L) {
            Modifier modifier = L.ReadUserdata<Modifier>(MODIFIER);

            string field = L.luaL_optstring(2, null);

            if (field == "translateX")
                modifier.translate_x = L.luaL_checkfloat(3);
            else if (field == "translateY")
                modifier.translate_y = L.luaL_checkfloat(3);
            else if (field == "rotate")
                modifier.rotate = L.luaL_checkfloat(3);
            else if (field == "skewX")
                modifier.skew_x = L.luaL_checkfloat(3);
            else if (field == "skewY")
                modifier.skew_y = L.luaL_checkfloat(3);
            else if (field == "scaleX")
                modifier.scale_x = L.luaL_checkfloat(3);
            else if (field == "scaleY")
                modifier.scale_y = L.luaL_checkfloat(3);
            else if (field == "scaleDirectionX")
                modifier.scale_direction_x = L.luaL_checkfloat(3);
            else if (field == "scaleDirectionY")
                modifier.scale_direction_y = L.luaL_checkfloat(3);
            else if (field == "rotatePivotEnabled")
                modifier.rotate_pivot_enabled = L.luaL_checkboolean(3);
            else if (field == "rotatePivotU")
                modifier.rotate_pivot_u = L.luaL_checkfloat(3);
            else if (field == "rotatePivotV")
                modifier.rotate_pivot_v = L.luaL_checkfloat(3);
            else if (field == "translateRotation")
                modifier.translate_rotation = L.luaL_checkboolean(3);
            else if (field == "scaleSize")
                modifier.scale_size = L.luaL_checkboolean(3);
            else if (field == "scaleTranslation")
                modifier.scale_translation = L.luaL_checkboolean(3);
            else if (field == "x")
                modifier.x = L.luaL_checkfloat(3);
            else if (field == "y")
                modifier.y = L.luaL_checkfloat(3);
            else if (field == "width")
                modifier.width = L.luaL_checkfloat(3);
            else if (field == "height")
                modifier.height = L.luaL_checkfloat(3);
            else
                return L.luaL_argerror(2, $"unknown Modifier field '{field}'");

            return 0;
        }


        private static readonly LuaCallback delegate_gc = script_modifier_gc;
        private static readonly LuaCallback delegate_tostring = script_modifier_tostring;
        private static readonly LuaCallback delegate_index = script_modifier_index;
        private static readonly LuaCallback delegate_newindex = script_modifier_newindex;

        internal static void register_modifier(ManagedLuaState lua) {
            lua.RegisterStructMetaTable(
                MODIFIER,
                delegate_gc,
                delegate_tostring,
                delegate_index,
                delegate_newindex
            );
        }

    }
}
