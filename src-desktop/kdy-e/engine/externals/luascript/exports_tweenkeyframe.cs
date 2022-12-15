using System;
using Engine.Animation;
using Engine.Externals.LuaInterop;
using Engine.Utils;

namespace Engine.Externals.LuaScriptInterop {

    public static class ExportsTweenKeyframe {
        public const string TWEENKEYFRAME = "TweenKeyframe";

        static int script_tweenkeyframe_init(LuaState L) {
            TweenKeyframe ret = TweenKeyframe.Init();

            return L.CreateAllocatedUserdata(TWEENKEYFRAME, ret);
        }

        static int script_tweenkeyframe_init2(LuaState L) {
            AnimListItem animlistitem = L.ReadUserdata<AnimListItem>(ExportsAnimListItem.ANIMLISTITEM);

            TweenKeyframe ret = TweenKeyframe.Init2(animlistitem);

            return L.CreateAllocatedUserdata(TWEENKEYFRAME, ret);
        }

        static int script_tweenkeyframe_destroy(LuaState L) {
            TweenKeyframe tweenkeyframe = L.ReadUserdata<TweenKeyframe>(TWEENKEYFRAME);

            if (L.IsUserdataAllocated(TWEENKEYFRAME))
                tweenkeyframe.Destroy();
            else
                Console.WriteLine("script_tweenkeyframe_destroy() object was not allocated by lua");

            return 0;
        }

        static int script_tweenkeyframe_animate_percent(LuaState L) {
            TweenKeyframe tweenkeyframe = L.ReadUserdata<TweenKeyframe>(TWEENKEYFRAME);
            double percent = (double)L.luaL_checknumber(2);

            tweenkeyframe.AnimatePercent(percent);

            return 0;
        }

        static int script_tweenkeyframe_get_ids_count(LuaState L) {
            TweenKeyframe tweenkeyframe = L.ReadUserdata<TweenKeyframe>(TWEENKEYFRAME);

            int ret = tweenkeyframe.GetIdsCount();

            L.lua_pushinteger(ret);
            return 1;
        }

        static int script_tweenkeyframe_peek_value(LuaState L) {
            TweenKeyframe tweenkeyframe = L.ReadUserdata<TweenKeyframe>(TWEENKEYFRAME);

            float ret = tweenkeyframe.PeekValue();

            L.lua_pushnumber(ret);
            return 1;
        }

        static int script_tweenkeyframe_peek_value_by_index(LuaState L) {
            TweenKeyframe tweenkeyframe = L.ReadUserdata<TweenKeyframe>(TWEENKEYFRAME);
            int index = (int)L.luaL_checkinteger(2);

            float ret = tweenkeyframe.PeekValueByIndex(index);

            L.lua_pushnumber(ret);
            return 1;
        }

        static int script_tweenkeyframe_peek_entry_by_index(LuaState L) {
            TweenKeyframe tweenkeyframe = L.ReadUserdata<TweenKeyframe>(TWEENKEYFRAME);
            int index = (int)L.luaL_checkinteger(2);

            int out_id;
            float out_value;
            bool ret = tweenkeyframe.PeekEntryByIndex(index, out out_id, out out_value);

            if (!ret) {
                L.lua_pushnil();
                return 1;
            }

            L.lua_pushinteger(out_id);
            L.lua_pushnumber(out_value);
            return 2;
        }

        static int script_tweenkeyframe_peek_value_by_id(LuaState L) {
            TweenKeyframe tweenkeyframe = L.ReadUserdata<TweenKeyframe>(TWEENKEYFRAME);
            int id = (int)L.luaL_checkinteger(2);

            float ret = tweenkeyframe.PeekValueById(id);

            L.lua_pushnumber(ret);
            return 1;
        }

        static int script_tweenkeyframe_add_easeout(LuaState L) {
            TweenKeyframe tweenkeyframe = L.ReadUserdata<TweenKeyframe>(TWEENKEYFRAME);
            float at = (float)L.luaL_checknumber(2);
            int id = (int)L.luaL_checkinteger(3);
            float value = (float)L.luaL_checknumber(4);

            int ret = tweenkeyframe.AddEaseOut(at, id, value);

            L.lua_pushinteger(ret);
            return 1;
        }

        static int script_tweenkeyframe_add_easeinout(LuaState L) {
            TweenKeyframe tweenkeyframe = L.ReadUserdata<TweenKeyframe>(TWEENKEYFRAME);
            float at = (float)L.luaL_checknumber(2);
            int id = (int)L.luaL_checkinteger(3);
            float value = (float)L.luaL_checknumber(4);

            int ret = tweenkeyframe.AddEaseInOut(at, id, value);

            L.lua_pushinteger(ret);
            return 1;
        }

        static int script_tweenkeyframe_add_linear(LuaState L) {
            TweenKeyframe tweenkeyframe = L.ReadUserdata<TweenKeyframe>(TWEENKEYFRAME);
            float at = (float)L.luaL_checknumber(2);
            int id = (int)L.luaL_checkinteger(3);
            float value = (float)L.luaL_checknumber(4);

            int ret = tweenkeyframe.AddLinear(at, id, value);

            L.lua_pushinteger(ret);
            return 1;
        }

        static int script_tweenkeyframe_add_steps(LuaState L) {
            TweenKeyframe tweenkeyframe = L.ReadUserdata<TweenKeyframe>(TWEENKEYFRAME);
            float at = (float)L.luaL_checknumber(2);
            int id = (int)L.luaL_checkinteger(3);
            float value = (float)L.luaL_checknumber(4);
            int steps_count = (int)L.luaL_checkinteger(5);
            Align steps_method = LuascriptHelpers.ParseAlign(L, L.luaL_optstring(6, null));

            int ret = tweenkeyframe.AddSteps(at, id, value, steps_count, steps_method);

            L.lua_pushinteger(ret);
            return 1;
        }

        static int script_tweenkeyframe_add_interpolator(LuaState L) {
            TweenKeyframe tweenkeyframe = L.ReadUserdata<TweenKeyframe>(TWEENKEYFRAME);
            float at = (float)L.luaL_checknumber(2);
            int id = (int)L.luaL_checkinteger(3);
            float value = (float)L.luaL_checknumber(4);
            AnimInterpolator type = LuascriptHelpers.ParseInterpolator(L, L.luaL_optstring(5, null));

            int ret = tweenkeyframe.AddInterpolator(at, id, value, type);

            L.lua_pushinteger(ret);
            return 1;
        }




        private static readonly LuaTableFunction[] TWEENKEYFRAME_FUNCTIONS = {
            new LuaTableFunction("init", script_tweenkeyframe_init),
            new LuaTableFunction("init2", script_tweenkeyframe_init2),
            new LuaTableFunction("destroy", script_tweenkeyframe_destroy),
            new LuaTableFunction("animate_percent", script_tweenkeyframe_animate_percent),
            new LuaTableFunction("get_ids_count", script_tweenkeyframe_get_ids_count),
            new LuaTableFunction("peek_value", script_tweenkeyframe_peek_value),
            new LuaTableFunction("peek_value_by_index", script_tweenkeyframe_peek_value_by_index),
            new LuaTableFunction("peek_entry_by_index", script_tweenkeyframe_peek_entry_by_index),
            new LuaTableFunction("peek_value_by_id", script_tweenkeyframe_peek_value_by_id),
            new LuaTableFunction("add_easeout", script_tweenkeyframe_add_easeout),
            new LuaTableFunction("add_easeinout", script_tweenkeyframe_add_easeinout),
            new LuaTableFunction("add_linear", script_tweenkeyframe_add_linear),
            new LuaTableFunction("add_steps", script_tweenkeyframe_add_steps),
            new LuaTableFunction("add_interpolator", script_tweenkeyframe_add_interpolator),
            new LuaTableFunction(null, null)
        };

        public static int script_tweenkeyframe_new(LuaState L, TweenKeyframe tweenkeyframe) {
            return L.CreateUserdata(TWEENKEYFRAME, tweenkeyframe);
        }

        static int script_tweenkeyframe_gc(LuaState L) {
            return L.DestroyUserdata(TWEENKEYFRAME);
        }

        static int script_tweenkeyframe_tostring(LuaState L) {
            return L.ToString_userdata(TWEENKEYFRAME);
        }

        private static readonly LuaCallback delegate_gc = script_tweenkeyframe_gc;
        private static readonly LuaCallback delegate_tostring = script_tweenkeyframe_tostring;

        public static void script_tweenkeyframe_register(ManagedLuaState L) {
            L.RegisterMetaTable(TWEENKEYFRAME, delegate_gc, delegate_tostring, TWEENKEYFRAME_FUNCTIONS);
        }

    }
}

