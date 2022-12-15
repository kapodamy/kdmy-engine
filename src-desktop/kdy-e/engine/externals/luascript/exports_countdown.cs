using Engine.Animation;
using Engine.Externals.LuaInterop;
using Engine.Game;

namespace Engine.Externals.LuaScriptInterop {

    public static class ExportsCountdown {
        public const string COUNTDOWN = "Countdown";


        static int script_countdown_set_default_animation2(LuaState L) {
            Countdown countdown = L.ReadUserdata<Countdown>(COUNTDOWN);
            TweenKeyframe tweenkeyframe = L.ReadNullableUserdata<TweenKeyframe>(2, ExportsTweenKeyframe.TWEENKEYFRAME);

            countdown.SetDefaultAnimation2(tweenkeyframe);

            return 0;
        }

        static int script_countdown_set_bpm(LuaState L) {
            Countdown countdown = L.ReadUserdata<Countdown>(COUNTDOWN);
            float bpm = (float)L.luaL_checknumber(2);

            countdown.SetBpm(bpm);

            return 0;
        }

        static int script_countdown_get_drawable(LuaState L) {
            Countdown countdown = L.ReadUserdata<Countdown>(COUNTDOWN);

            Drawable ret = countdown.GetDrawable();

            return ExportsDrawable.script_drawable_new(L, ret);
        }

        static int script_countdown_ready(LuaState L) {
            Countdown countdown = L.ReadUserdata<Countdown>(COUNTDOWN);

            bool ret = countdown.Ready();

            L.lua_pushboolean(ret);
            return 1;
        }

        static int script_countdown_start(LuaState L) {
            Countdown countdown = L.ReadUserdata<Countdown>(COUNTDOWN);

            bool ret = countdown.Start();

            L.lua_pushboolean(ret);
            return 1;
        }

        static int script_countdown_has_ended(LuaState L) {
            Countdown countdown = L.ReadUserdata<Countdown>(COUNTDOWN);

            bool ret = countdown.HasEnded();

            L.lua_pushboolean(ret);
            return 1;
        }




        static readonly LuaTableFunction[] COUNTDOWN_FUNCTIONS = {
            new LuaTableFunction("set_default_animation2", script_countdown_set_default_animation2),
            new LuaTableFunction("get_drawable", script_countdown_get_drawable),
            new LuaTableFunction("ready", script_countdown_ready),
            new LuaTableFunction("start", script_countdown_start),
            new LuaTableFunction("has_ended", script_countdown_has_ended),
            new LuaTableFunction(null, null)
        };

        public static int script_countdown_new(LuaState L, Countdown countdown) {
            return L.CreateUserdata(COUNTDOWN, countdown);
        }

        static int script_countdown_gc(LuaState L) {
            return L.GC_userdata(COUNTDOWN);
        }

        static int script_countdown_tostring(LuaState L) {
            return L.ToString_userdata(COUNTDOWN);
        }

        private static readonly LuaCallback delegate_gc = script_countdown_gc;
        private static readonly LuaCallback delegate_tostring = script_countdown_tostring;

        public static void script_countdown_register(ManagedLuaState L) {
            L.RegisterMetaTable(COUNTDOWN, delegate_gc, delegate_tostring, COUNTDOWN_FUNCTIONS);
        }

    }

}

