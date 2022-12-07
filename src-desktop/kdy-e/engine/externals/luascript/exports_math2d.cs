using Engine.Externals.LuaInterop;
using Engine.Utils;

namespace Engine.Externals.LuaScriptInterop {

    public static class ExportsMath2D {

        static int script_math2d_lerp(LuaState L) {
            float start = (float)L.luaL_checknumber(1);
            float end = (float)L.luaL_checknumber(2);
            float step = (float)L.luaL_checknumber(3);

            float result = Math2D.Lerp(start, end, step);
            L.lua_pushnumber(result);

            return 1;
        }

        static int script_math2d_inverselerp(LuaState L) {
            float start = (float)L.luaL_checknumber(1);
            float end = (float)L.luaL_checknumber(2);
            float value = (float)L.luaL_checknumber(3);

            float result = Math2D.InverseLerp(start, end, value);
            L.lua_pushnumber(result);

            return 1;
        }

        static int script_math2d_nearestdown(LuaState L) {
            float value = (float)L.luaL_checknumber(1);
            float step = (float)L.luaL_checknumber(2);

            float result = Math2D.NearestDown(value, step);
            L.lua_pushnumber(result);

            return 1;
        }

        static int script_math2d_rotate_point_by_degs(LuaState L) {
            float radians = (float)L.luaL_checknumber(1);
            float x = (float)L.luaL_checknumber(2);
            float y = (float)L.luaL_checknumber(3);

            Math2D.RotatePoint(radians, ref x, ref y);
            L.lua_pushnumber(x);
            L.lua_pushnumber(y);

            return 2;
        }

        static int script_math2d_points_distance(LuaState L) {
            float x1 = (float)L.luaL_checknumber(1);
            float y1 = (float)L.luaL_checknumber(2);
            float x2 = (float)L.luaL_checknumber(3);
            float y2 = (float)L.luaL_checknumber(4);

            float result = Math2D.PointsDistance(x1, y1, x2, y2);
            L.lua_pushnumber(result);

            return 1;
        }


        static readonly LuaTableFunction[] EXPORTS_FUNCTION = {
            new LuaTableFunction("math2d_lerp", script_math2d_lerp),
            new LuaTableFunction("math2d_inverselerp", script_math2d_inverselerp),
            new LuaTableFunction("math2d_nearestdown", script_math2d_nearestdown),
            new LuaTableFunction("math2d_rotate_point_by_degs", script_math2d_rotate_point_by_degs),
            new LuaTableFunction("math2d_points_distance", script_math2d_points_distance),
            new LuaTableFunction(null, null)
        };

        internal static void script_math2d_register(ManagedLuaState lua) {
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
