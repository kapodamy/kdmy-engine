using Engine.Animation;
using Engine.Externals.LuaInterop;
using Engine.Game;

namespace Engine.Externals.LuaScriptInterop {

    public static class ExportsRoundStats {
        public const string ROUNDSTATS = "RoundStats";

        static int script_roundstats_hide(LuaState L) {
            RoundStats roundstats = L.ReadUserdata<RoundStats>(ROUNDSTATS);
            bool hide = L.lua_toboolean(2);

            roundstats.Hide(hide);

            return 0;
        }

        static int script_roundstats_hide_nps(LuaState L) {
            RoundStats roundstats = L.ReadUserdata<RoundStats>(ROUNDSTATS);
            bool hide = L.lua_toboolean(2);

            roundstats.HideNps(hide);

            return 0;
        }

        static int script_roundstats_set_draw_y(LuaState L) {
            RoundStats roundstats = L.ReadUserdata<RoundStats>(ROUNDSTATS);
            float y = (float)L.luaL_checknumber(2);

            roundstats.SetDrawY(y);

            return 0;
        }

        static int script_roundstats_reset(LuaState L) {
            RoundStats roundstats = L.ReadUserdata<RoundStats>(ROUNDSTATS);

            roundstats.Reset();

            return 0;
        }

        static int script_roundstats_get_drawable(LuaState L) {
            RoundStats roundstats = L.ReadUserdata<RoundStats>(ROUNDSTATS);

            Drawable ret = roundstats.GetDrawable();

            return ExportsDrawable.script_drawable_new(L, ret);
        }

        static int script_roundstats_tweenlerp_set_on_beat(LuaState L) {
            RoundStats roundstats = L.ReadUserdata<RoundStats>(ROUNDSTATS);
            TweenLerp tweenlerp = L.ReadNullableUserdata<TweenLerp>(2, ExportsTweenLerp.TWEENLERP);
            float rollback_beats = (float)L.luaL_checknumber(3);
            float beat_duration = (float)L.luaL_checknumber(4);

            roundstats.TweenlerpSetOnBeat(tweenlerp, rollback_beats, beat_duration);

            return 0;
        }

        static int script_roundstats_tweenlerp_set_on_hit(LuaState L) {
            RoundStats roundstats = L.ReadUserdata<RoundStats>(ROUNDSTATS);
            TweenLerp tweenlerp = L.ReadNullableUserdata<TweenLerp>(2, ExportsTweenLerp.TWEENLERP);
            float rollback_beats = (float)L.luaL_checknumber(3);
            float beat_duration = (float)L.luaL_checknumber(4);

            roundstats.TweenlerpSetOnHit(tweenlerp, rollback_beats, beat_duration);

            return 0;
        }

        static int script_roundstats_tweenlerp_set_on_miss(LuaState L) {
            RoundStats roundstats = L.ReadUserdata<RoundStats>(ROUNDSTATS);
            TweenLerp tweenlerp = L.ReadNullableUserdata<TweenLerp>(2, ExportsTweenLerp.TWEENLERP);
            float rollback_beats = (float)L.luaL_checknumber(3);
            float beat_duration = (float)L.luaL_checknumber(4);

            roundstats.TweenlerpSetOnMiss(tweenlerp, rollback_beats, beat_duration);

            return 0;
        }

        static int script_roundstats_tweenlerp_set_bpm(LuaState L) {
            RoundStats roundstats = L.ReadUserdata<RoundStats>(ROUNDSTATS);
            float beats_per_minute = (float)L.luaL_checknumber(2);

            roundstats.TweenlerpSetBpm(beats_per_minute);

            return 0;
        }




        static readonly LuaTableFunction[] ROUNDSTATS_FUNCTIONS = {
            new LuaTableFunction("hide", script_roundstats_hide),
            new LuaTableFunction("hide_nps", script_roundstats_hide_nps),
            new LuaTableFunction("set_draw_y", script_roundstats_set_draw_y),
            new LuaTableFunction("reset", script_roundstats_reset),
            new LuaTableFunction("get_drawable", script_roundstats_get_drawable),
            new LuaTableFunction("tweenlerp_set_on_beat", script_roundstats_tweenlerp_set_on_beat),
            new LuaTableFunction("tweenlerp_set_on_hit", script_roundstats_tweenlerp_set_on_hit),
            new LuaTableFunction("tweenlerp_set_on_miss", script_roundstats_tweenlerp_set_on_miss),
            new LuaTableFunction("tweenlerp_set_bpm", script_roundstats_tweenlerp_set_bpm),
            new LuaTableFunction(null, null)
        };

        public static int script_roundstats_new(LuaState L, RoundStats roundstats) {
            return L.CreateUserdata(ROUNDSTATS, roundstats);
        }

        static int script_roundstats_gc(LuaState L) {
            return L.GC_userdata(ROUNDSTATS);
        }

        static int script_roundstats_tostring(LuaState L) {
            return L.ToString_userdata(ROUNDSTATS);
        }

        private static readonly LuaCallback delegate_gc = script_roundstats_gc;
        private static readonly LuaCallback delegate_tostring = script_roundstats_tostring;

        public static void script_roundstats_register(ManagedLuaState L) {
            L.RegisterMetaTable(ROUNDSTATS, delegate_gc, delegate_tostring, ROUNDSTATS_FUNCTIONS);
        }

    }
}

