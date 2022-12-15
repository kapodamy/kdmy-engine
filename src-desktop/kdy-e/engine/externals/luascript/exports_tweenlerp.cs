using System;
using Engine.Animation;
using Engine.Externals.LuaInterop;
using Engine.Utils;

namespace Engine.Externals.LuaScriptInterop {

    public static class ExportsTweenLerp {
        public const string TWEENLERP = "TweenLerp";

        static int script_tweenlerp_init(LuaState L) {
            TweenLerp tweenlerp = new TweenLerp();
            return L.CreateAllocatedUserdata(TWEENLERP, tweenlerp);
        }

        static int script_tweenlerp_destroy(LuaState L) {
            TweenLerp tweenlerp = L.ReadUserdata<TweenLerp>(TWEENLERP);

            if (L.IsUserdataAllocated(TWEENLERP))
                tweenlerp.Destroy();
            else
                Console.WriteLine("script_tweenlerp_destroy() object was not allocated by lua");

            return 0;
        }

        static int script_tweenlerp_end(LuaState L) {
            TweenLerp tweenlerp = L.ReadUserdata<TweenLerp>(TWEENLERP);

            tweenlerp.End();

            return 0;
        }

        static int script_tweenlerp_mark_as_completed(LuaState L) {
            TweenLerp tweenlerp = L.ReadUserdata<TweenLerp>(TWEENLERP);

            tweenlerp.MarkAsCompleted();

            return 0;
        }

        static int script_tweenlerp_restart(LuaState L) {
            TweenLerp tweenlerp = L.ReadUserdata<TweenLerp>(TWEENLERP);

            tweenlerp.Restart();

            return 0;
        }

        static int script_tweenlerp_animate(LuaState L) {
            TweenLerp tweenlerp = L.ReadUserdata<TweenLerp>(TWEENLERP);
            float elapsed = (float)L.luaL_checknumber(2);

            int ret = tweenlerp.Animate(elapsed);

            L.lua_pushinteger(ret);
            return 1;
        }

        static int script_tweenlerp_animate_percent(LuaState L) {
            TweenLerp tweenlerp = L.ReadUserdata<TweenLerp>(TWEENLERP);
            float percent = (float)L.luaL_checknumber(2);

            int ret = tweenlerp.AnimatePercent(percent);

            L.lua_pushinteger(ret);
            return 1;
        }

        static int script_tweenlerp_is_completed(LuaState L) {
            TweenLerp tweenlerp = L.ReadUserdata<TweenLerp>(TWEENLERP);

            bool ret = tweenlerp.IsCompleted();

            L.lua_pushboolean(ret);
            return 1;
        }

        static int script_tweenlerp_get_elapsed(LuaState L) {
            TweenLerp tweenlerp = L.ReadUserdata<TweenLerp>(TWEENLERP);

            double ret = tweenlerp.GetElapsed();

            L.lua_pushnumber(ret);
            return 1;
        }

        static int script_tweenlerp_get_entry_count(LuaState L) {
            TweenLerp tweenlerp = L.ReadUserdata<TweenLerp>(TWEENLERP);

            int ret = tweenlerp.GetEntryCount();

            L.lua_pushinteger(ret);
            return 1;
        }

        static int script_tweenlerp_peek_value(LuaState L) {
            TweenLerp tweenlerp = L.ReadUserdata<TweenLerp>(TWEENLERP);

            float ret = tweenlerp.PeekValue();

            L.lua_pushnumber(ret);
            return 1;
        }

        static int script_tweenlerp_peek_value_by_index(LuaState L) {
            TweenLerp tweenlerp = L.ReadUserdata<TweenLerp>(TWEENLERP);
            int index = (int)L.luaL_checkinteger(2);

            float ret = tweenlerp.PeekValueByIndex(index);

            L.lua_pushnumber(ret);
            return 1;
        }

        static int script_tweenlerp_peek_entry_by_index(LuaState L) {
            TweenLerp tweenlerp = L.ReadUserdata<TweenLerp>(TWEENLERP);
            int index = (int)L.luaL_checkinteger(2);

            int out_id;
            float out_value, out_duration;
            bool ret = tweenlerp.PeekEntryByIndex(index, out out_id, out out_value, out out_duration);

            if (ret) {
                L.lua_pushinteger(out_id);
                L.lua_pushnumber(out_value);
                L.lua_pushnumber(out_duration);
                return 3;
            } else {
                L.lua_pushnil();
                return 1;
            }

        }

        static int script_tweenlerp_peek_value_by_id(LuaState L) {
            TweenLerp tweenlerp = L.ReadUserdata<TweenLerp>(TWEENLERP);
            int id = (int)L.luaL_checkinteger(2);

            float ret = tweenlerp.PeekValueById(id);

            L.lua_pushnumber(ret);
            return 1;
        }

        static int script_tweenlerp_change_bounds_by_index(LuaState L) {
            TweenLerp tweenlerp = L.ReadUserdata<TweenLerp>(TWEENLERP);
            int index = (int)L.luaL_checkinteger(2);
            float new_start = (float)L.luaL_checknumber(3);
            float new_end = (float)L.luaL_checknumber(4);

            bool ret = tweenlerp.ChangeBoundsByIndex(index, new_start, new_end);

            L.lua_pushboolean(ret);
            return 1;
        }

        static int script_tweenlerp_override_start_with_end_by_index(LuaState L) {
            TweenLerp tweenlerp = L.ReadUserdata<TweenLerp>(TWEENLERP);
            int index = (int)L.luaL_checkinteger(2);

            bool ret = tweenlerp.OverrideStartWithEndByIndex(index);

            L.lua_pushboolean(ret);
            return 1;
        }

        static int script_tweenlerp_change_bounds_by_id(LuaState L) {
            TweenLerp tweenlerp = L.ReadUserdata<TweenLerp>(TWEENLERP);
            int id = (int)L.luaL_checkinteger(2);
            float new_start = (float)L.luaL_checknumber(3);
            float new_end = (float)L.luaL_checknumber(4);

            bool ret = tweenlerp.ChangeBoundsById(id, new_start, new_end);

            L.lua_pushboolean(ret);
            return 1;
        }

        static int script_tweenlerp_change_duration_by_index(LuaState L) {
            TweenLerp tweenlerp = L.ReadUserdata<TweenLerp>(TWEENLERP);
            int index = (int)L.luaL_checkinteger(2);
            float new_duration = (float)L.luaL_checknumber(3);

            bool ret = tweenlerp.ChangeDurationByIndex(index, new_duration);

            L.lua_pushboolean(ret);
            return 1;
        }

        static int script_tweenlerp_swap_bounds_by_index(LuaState L) {
            TweenLerp tweenlerp = L.ReadUserdata<TweenLerp>(TWEENLERP);
            int index = (int)L.luaL_checkinteger(2);

            bool ret = tweenlerp.SwapBoundsByIndex(index);

            L.lua_pushboolean(ret);
            return 1;
        }

        static int script_tweenlerp_add_ease(LuaState L) {
            TweenLerp tweenlerp = L.ReadUserdata<TweenLerp>(TWEENLERP);
            int id = (int)L.luaL_checkinteger(2);
            float start = (float)L.luaL_checknumber(3);
            float end = (float)L.luaL_checknumber(4);
            float duration = (float)L.luaL_checknumber(5);

            int ret = tweenlerp.AddEase(id, start, end, duration);

            L.lua_pushinteger(ret);
            return 1;
        }

        static int script_tweenlerp_add_easein(LuaState L) {
            TweenLerp tweenlerp = L.ReadUserdata<TweenLerp>(TWEENLERP);
            int id = (int)L.luaL_checkinteger(2);
            float start = (float)L.luaL_checknumber(3);
            float end = (float)L.luaL_checknumber(4);
            float duration = (float)L.luaL_checknumber(5);

            int ret = tweenlerp.AddEaseIn(id, start, end, duration);

            L.lua_pushinteger(ret);
            return 1;
        }

        static int script_tweenlerp_add_easeout(LuaState L) {
            TweenLerp tweenlerp = L.ReadUserdata<TweenLerp>(TWEENLERP);
            int id = (int)L.luaL_checkinteger(2);
            float start = (float)L.luaL_checknumber(3);
            float end = (float)L.luaL_checknumber(4);
            float duration = (float)L.luaL_checknumber(5);

            int ret = tweenlerp.AddEaseOut(id, start, end, duration);

            L.lua_pushinteger(ret);
            return 1;
        }

        static int script_tweenlerp_add_easeinout(LuaState L) {
            TweenLerp tweenlerp = L.ReadUserdata<TweenLerp>(TWEENLERP);
            int id = (int)L.luaL_checkinteger(2);
            float start = (float)L.luaL_checknumber(3);
            float end = (float)L.luaL_checknumber(4);
            float duration = (float)L.luaL_checknumber(5);

            int ret = tweenlerp.AddEaseInOut(id, start, end, duration);

            L.lua_pushinteger(ret);
            return 1;
        }

        static int script_tweenlerp_add_linear(LuaState L) {
            TweenLerp tweenlerp = L.ReadUserdata<TweenLerp>(TWEENLERP);
            int id = (int)L.luaL_checkinteger(2);
            float start = (float)L.luaL_checknumber(3);
            float end = (float)L.luaL_checknumber(4);
            float duration = (float)L.luaL_checknumber(5);

            int ret = tweenlerp.AddLinear(id, start, end, duration);

            L.lua_pushinteger(ret);
            return 1;
        }

        static int script_tweenlerp_add_steps(LuaState L) {
            TweenLerp tweenlerp = L.ReadUserdata<TweenLerp>(TWEENLERP);
            int id = (int)L.luaL_checkinteger(2);
            float start = (float)L.luaL_checknumber(3);
            float end = (float)L.luaL_checknumber(4);
            float duration = (float)L.luaL_checknumber(5);
            int steps_count = (int)L.luaL_checkinteger(6);
            Align steps_method = LuascriptHelpers.ParseAlign(L, L.luaL_optstring(7, null));

            int ret = tweenlerp.AddSteps(id, start, end, duration, steps_count, steps_method);

            L.lua_pushinteger(ret);
            return 1;
        }

        static int script_tweenlerp_add_interpolator(LuaState L) {
            TweenLerp tweenlerp = L.ReadUserdata<TweenLerp>(TWEENLERP);
            int id = (int)L.luaL_checkinteger(2);
            float start = (float)L.luaL_checknumber(3);
            float end = (float)L.luaL_checknumber(4);
            float duration = (float)L.luaL_checknumber(5);
            AnimInterpolator type = LuascriptHelpers.ParseInterpolator(L, L.luaL_optstring(6, null));

            int ret = tweenlerp.AddInterpolator(id, start, end, duration, type);

            L.lua_pushinteger(ret);
            return 1;
        }




        private static readonly LuaTableFunction[] TWEENLERP_FUNCTIONS = {
            new LuaTableFunction("init", script_tweenlerp_init),
            new LuaTableFunction("destroy", script_tweenlerp_destroy),
            new LuaTableFunction("end", script_tweenlerp_end),
            new LuaTableFunction("mark_as_completed", script_tweenlerp_mark_as_completed),
            new LuaTableFunction("restart", script_tweenlerp_restart),
            new LuaTableFunction("animate", script_tweenlerp_animate),
            new LuaTableFunction("animate_percent", script_tweenlerp_animate_percent),
            new LuaTableFunction("is_completed", script_tweenlerp_is_completed),
            new LuaTableFunction("get_elapsed", script_tweenlerp_get_elapsed),
            new LuaTableFunction("get_entry_count", script_tweenlerp_get_entry_count),
            new LuaTableFunction("peek_value", script_tweenlerp_peek_value),
            new LuaTableFunction("peek_value_by_index", script_tweenlerp_peek_value_by_index),
            new LuaTableFunction("peek_entry_by_index", script_tweenlerp_peek_entry_by_index),
            new LuaTableFunction("peek_value_by_id", script_tweenlerp_peek_value_by_id),
            new LuaTableFunction("change_bounds_by_index", script_tweenlerp_change_bounds_by_index),
            new LuaTableFunction("override_start_with_end_by_index", script_tweenlerp_override_start_with_end_by_index),
            new LuaTableFunction("change_bounds_by_id", script_tweenlerp_change_bounds_by_id),
            new LuaTableFunction("change_duration_by_index", script_tweenlerp_change_duration_by_index),
            new LuaTableFunction("swap_bounds_by_index", script_tweenlerp_swap_bounds_by_index),
            new LuaTableFunction("add_ease", script_tweenlerp_add_ease),
            new LuaTableFunction("add_easein", script_tweenlerp_add_easein),
            new LuaTableFunction("add_easeout", script_tweenlerp_add_easeout),
            new LuaTableFunction("add_easeinout", script_tweenlerp_add_easeinout),
            new LuaTableFunction("add_linear", script_tweenlerp_add_linear),
            new LuaTableFunction("add_steps", script_tweenlerp_add_steps),
            new LuaTableFunction("add_interpolator", script_tweenlerp_add_interpolator),
            new LuaTableFunction(null, null)
        };

        static int script_tweenlerp_new(LuaState L, TweenLerp tweenlerp) {
            return L.CreateUserdata(TWEENLERP, tweenlerp);
        }

        static int script_tweenlerp_gc(LuaState L) {
            return L.DestroyUserdata(TWEENLERP);
        }

        static int script_tweenlerp_tostring(LuaState L) {
            return L.ToString_userdata(TWEENLERP);
        }

        private static readonly LuaCallback delegate_gc = script_tweenlerp_gc;
        private static readonly LuaCallback delegate_tostring = script_tweenlerp_tostring;

        public static void script_tweenlerp_register(ManagedLuaState L) {
            L.RegisterMetaTable(TWEENLERP, delegate_gc, delegate_tostring, TWEENLERP_FUNCTIONS);
        }


    }
}
