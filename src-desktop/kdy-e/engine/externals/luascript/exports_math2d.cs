using Engine.Externals.LuaInterop;
using Engine.Utils;

namespace Engine.Externals.LuaScriptInterop {

    public static class ExportsMath2D {

        static int script_math2d_lerp(LuaState L) {
            float start = L.luaL_checkfloat(1);
            float end = L.luaL_checkfloat(2);
            float step = L.luaL_checkfloat(3);

            float result = Math2D.Lerp(start, end, step);
            L.lua_pushnumber(result);

            return 1;
        }

        static int script_math2d_inverselerp(LuaState L) {
            float start = L.luaL_checkfloat(1);
            float end = L.luaL_checkfloat(2);
            float value = L.luaL_checkfloat(3);

            float result = Math2D.InverseLerp(start, end, value);
            L.lua_pushnumber(result);

            return 1;
        }

        static int script_math2d_nearestdown(LuaState L) {
            float value = L.luaL_checkfloat(1);
            float step = L.luaL_checkfloat(2);

            float result = Math2D.NearestDown(value, step);
            L.lua_pushnumber(result);

            return 1;
        }

        static int script_math2d_rotate_point_by_degs(LuaState L) {
            float radians = L.luaL_checkfloat(1);
            float x = L.luaL_checkfloat(2);
            float y = L.luaL_checkfloat(3);

            Math2D.RotatePoint(radians, ref x, ref y);
            L.lua_pushnumber(x);
            L.lua_pushnumber(y);

            return 2;
        }

        static int script_math2d_points_distance(LuaState L) {
            float x1 = L.luaL_checkfloat(1);
            float y1 = L.luaL_checkfloat(2);
            float x2 = L.luaL_checkfloat(3);
            float y2 = L.luaL_checkfloat(4);

            float result = Math2D.PointsDistance(x1, y1, x2, y2);
            L.lua_pushnumber(result);

            return 1;
        }


        static readonly LuaTableFunction[] EXPORTS_FUNCTION = {
            new LuaTableFunction() { name = "math2d_lerp",  func = script_math2d_lerp },
            new LuaTableFunction() { name = "math2d_inverselerp",  func = script_math2d_inverselerp },
            new LuaTableFunction() { name = "math2d_nearestdown",  func = script_math2d_nearestdown },
            new LuaTableFunction() { name = "math2d_rotate_point_by_degs",  func = script_math2d_rotate_point_by_degs },
            new LuaTableFunction() { name = "math2d_points_distance",  func = script_math2d_points_distance },
            new LuaTableFunction() { name = null, func = null }
        };

        internal static void register_math2d(ManagedLuaState lua) {
            lua.RegisterGlobalFunctions(EXPORTS_FUNCTION);

            lua.EvaluateString(
                "function math2d_random_boolean(chance)\n" +
                "local value = math.random(0, 100)\n" +
                "return value < chance\n" +
                "end\n"
            );
        }

    }
}
