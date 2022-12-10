using Engine.Animation;
using Engine.Externals.LuaInterop;
using Engine.Game;

namespace Engine.Externals.LuaScriptInterop {

    public static class ExportsRankingCounter {
        public const string RANKINGCOUNTER = "RankingCounter";

        static int script_rankingcounter_add_state(LuaState L) {
            RankingCounter rankingcounter = L.ReadUserdata<RankingCounter>(RANKINGCOUNTER);
            ModelHolder modelholder = L.ReadNullableUserdata<ModelHolder>(2, ExportsModelHolder.MODELHOLDER);
            string state_name = L.luaL_optstring(3, null);

            int ret = rankingcounter.AddState(modelholder, state_name);

            L.lua_pushinteger(ret);
            return 1;
        }

        static int script_rankingcounter_toggle_state(LuaState L) {
            RankingCounter rankingcounter = L.ReadUserdata<RankingCounter>(RANKINGCOUNTER);
            string state_name = L.luaL_optstring(2, null);

            rankingcounter.ToggleState(state_name);

            return 0;
        }

        static int script_rankingcounter_reset(LuaState L) {
            RankingCounter rankingcounter = L.ReadUserdata<RankingCounter>(RANKINGCOUNTER);

            rankingcounter.Reset();

            return 0;
        }

        static int script_rankingcounter_hide_accuracy(LuaState L) {
            RankingCounter rankingcounter = L.ReadUserdata<RankingCounter>(RANKINGCOUNTER);
            bool hide = L.lua_toboolean(2);

            rankingcounter.HideAccuracy(hide);

            return 0;
        }

        static int script_rankingcounter_use_percent_instead(LuaState L) {
            RankingCounter rankingcounter = L.ReadUserdata<RankingCounter>(RANKINGCOUNTER);
            bool use_accuracy_percenter = L.lua_toboolean(2);

            rankingcounter.UsePercentInstead(use_accuracy_percenter);

            return 0;
        }

        static int script_rankingcounter_set_default_ranking_animation2(LuaState L) {
            RankingCounter rankingcounter = L.ReadUserdata<RankingCounter>(RANKINGCOUNTER);
            AnimSprite animsprite = L.ReadNullableUserdata<AnimSprite>(2, ExportsAnimSprite.ANIMSPRITE);

            rankingcounter.SetDefaultRankingAnimation2(animsprite);

            return 0;
        }

        static int script_rankingcounter_set_default_ranking_text_animation2(LuaState L) {
            RankingCounter rankingcounter = L.ReadUserdata<RankingCounter>(RANKINGCOUNTER);
            AnimSprite animsprite = L.ReadNullableUserdata<AnimSprite>(2, ExportsAnimSprite.ANIMSPRITE);

            rankingcounter.SetDefaultRankingTextAnimation2(animsprite);

            return 0;
        }

        static int script_rankingcounter_set_alpha(LuaState L) {
            RankingCounter rankingcounter = L.ReadUserdata<RankingCounter>(RANKINGCOUNTER);
            float alpha = (float)L.luaL_checknumber(2);

            rankingcounter.SetAlpha(alpha);

            return 0;
        }

        static int script_rankingcounter_animation_set(LuaState L) {
            RankingCounter rankingcounter = L.ReadUserdata<RankingCounter>(RANKINGCOUNTER);
            AnimSprite animsprite = L.ReadNullableUserdata<AnimSprite>(2, ExportsAnimSprite.ANIMSPRITE);

            rankingcounter.AnimationSet(animsprite);

            return 0;
        }

        static int script_rankingcounter_animation_restart(LuaState L) {
            RankingCounter rankingcounter = L.ReadUserdata<RankingCounter>(RANKINGCOUNTER);

            rankingcounter.AnimationRestart();

            return 0;
        }

        static int script_rankingcounter_animation_end(LuaState L) {
            RankingCounter rankingcounter = L.ReadUserdata<RankingCounter>(RANKINGCOUNTER);

            rankingcounter.AnimationEnd();

            return 0;
        }




        static readonly LuaTableFunction[] RANKINGCOUNTER_FUNCTIONS = {
            new LuaTableFunction("add_state", script_rankingcounter_add_state),
            new LuaTableFunction("toggle_state", script_rankingcounter_toggle_state),
            new LuaTableFunction("reset", script_rankingcounter_reset),
            new LuaTableFunction("hide_accuracy", script_rankingcounter_hide_accuracy),
            new LuaTableFunction("use_percent_instead", script_rankingcounter_use_percent_instead),
            new LuaTableFunction("set_default_ranking_animation2", script_rankingcounter_set_default_ranking_animation2),
            new LuaTableFunction("set_default_ranking_text_animation2", script_rankingcounter_set_default_ranking_text_animation2),
            new LuaTableFunction("set_alpha", script_rankingcounter_set_alpha),
            new LuaTableFunction("animation_set", script_rankingcounter_animation_set),
            new LuaTableFunction("animation_restart", script_rankingcounter_animation_restart),
            new LuaTableFunction("animation_end", script_rankingcounter_animation_end),
            new LuaTableFunction(null, null)
        };

        public static int script_rankingcounter_new(LuaState L, RankingCounter rankingcounter) {
            return L.CreateUserdata(RANKINGCOUNTER, rankingcounter);
        }

        static int script_rankingcounter_gc(LuaState L) {
            return L.GC_userdata(RANKINGCOUNTER);
        }

        static int script_rankingcounter_tostring(LuaState L) {
            return L.ToString_userdata(RANKINGCOUNTER);
        }

        private static readonly LuaCallback delegate_gc = script_rankingcounter_gc;
        private static readonly LuaCallback delegate_tostring = script_rankingcounter_tostring;

        public static void script_rankingcounter_register(ManagedLuaState L) {
            L.RegisterMetaTable(RANKINGCOUNTER, delegate_gc, delegate_tostring, RANKINGCOUNTER_FUNCTIONS);
        }


    }
}

