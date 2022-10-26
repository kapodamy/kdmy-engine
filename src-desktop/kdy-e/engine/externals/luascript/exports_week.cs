using System;
using Engine.Externals.LuaInterop;
using Engine.Font;
using Engine.Game;
using Engine.Game.Common;
using Engine.Game.Gameplay;
using Engine.Sound;

namespace Engine.Externals.LuaScriptInterop {

    public static class ExportsWeek {


        private static int NotImplemented(LuaState L) {
            Console.Error.WriteLine("[ERROR] lua function not implemented");
            L.lua_pushnil();
            return 1;
        }


        static int script_week_unlockdirective_create(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;
            string name = L.luaL_checkstring(1);
            bool completed_round = L.luaL_checkboolean(2);
            bool completed_week = L.luaL_checkboolean(3);
            double value = L.luaL_checknumber(4);

            if (String.IsNullOrEmpty(name)) {
                return L.luaL_argerror(1, "the directive name cannot be empty.");
            }

            Week.UnlockDirectiveCreate(roundcontext, name, completed_round, completed_week, value);

            return 0;
        }

        static int script_week_unlockdirective_remove(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;
            string name = L.luaL_checkstring(1);
            bool completed_round = L.luaL_checkboolean(2);
            bool completed_week = L.luaL_checkboolean(3);

            if (String.IsNullOrEmpty(name)) {
                L.luaL_argerror(1, "the directive name cannot be null or empty.");
            }

            Week.UnlockdirectiveRemove(roundcontext, name, completed_round, completed_week);

            return 0;
        }

        static int script_week_unlockdirective_get(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;
            string name = L.luaL_optstring(1, null);

            if (String.IsNullOrEmpty(name)) {
                L.luaL_argerror(1, "the directive name cannot be null or empty.");
            }

            if (Week.UnlockdirectiveHas(roundcontext, name)) {
                double value = Week.UnlockdirectiveGet(roundcontext, name);
                L.lua_pushnumber(value);
            } else {
                L.lua_pushnil();
            }

            return 1;
        }

        static int script_week_halt(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;
            bool should_halt = L.luaL_checkboolean(1);

            Week.SetHalt(roundcontext, should_halt);

            return 0;
        }

        static int script_week_ui_set_visibility(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            bool visible = L.luaL_checkboolean(1);

            Week.UISetVisibility(roundcontext, visible);

            return 0;
        }

        static int script_week_ui_get_layout(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            Layout layout = Week.UIGetLayout(roundcontext);

            return ExportsLayout.script_layout_new(L, layout);
        }

        static int script_week_get_stage_layout(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            Layout layout = Week.GetStageLayout(roundcontext);

            return ExportsLayout.script_layout_new(L, layout);
        }

        static int script_week_ui_get_camera(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            Layout layout = Week.UIGetLayout(roundcontext);
            Camera camera = Week.UIGetCamera(roundcontext);

            return ExportsCamera.script_camera_new(L, camera);
        }

        static int script_week_ui_get_strums_count(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            int ret = Week.UiGetStrumsCount(roundcontext);

            L.lua_pushinteger(ret);
            return 1;
        }

        static int script_week_ui_get_strums(LuaState L) {
            return NotImplemented(L);

            /*RoundContext roundcontext = (RoundContext)L.Context;
            int strums_id = (int)L.luaL_checkinteger( 1);

            Strums strums = Week.UIGetStrums(roundcontext, strums_id);

            return ExportsStrums.script_week_strums_new(L, strums);*/
        }

        static int script_week_ui_get_roundstats(LuaState L) {
            return NotImplemented(L);

            /*RoundContext roundcontext = (RoundContext)L.Context;
            Roundstats roundstats = Week.UIGetRoundStats(roundcontext);
            return ExportsRoundStats.script_week_roundstats_new(L, roundstats);*/
        }

        static int script_week_ui_get_rankingcounter(LuaState L) {
            return NotImplemented(L);

            /*RoundContext roundcontext = (RoundContext)L.Context;
            Rankingcounter rankingcounter = Week.UIGetRankingcounter(roundcontext);
            return ExportsRankingcounter.script_week_rankingcounter_new(L, rankingcounter);*/
        }

        static int script_week_ui_get_streakcounter(LuaState L) {
            return NotImplemented(L);

            /*RoundContext roundcontext = (RoundContext)L.Context;
            StreakCounter streakcounter = Week.UIGetStreakCounter(roundcontext);
            return ExportsStreakCounter.script_week_streakcounter_new(L, streakcounter);*/
        }

        static int script_week_ui_get_trackinfo(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            TextSprite textsprite = Week.UIGetTrackinfo(roundcontext);

            return ExportsTextSprite.script_textsprite_new(L, textsprite);
        }

        static int script_week_ui_get_songprogressbar(LuaState L) {
            return NotImplemented(L);

            /*RoundContext roundcontext = (RoundContext)L.Context;
            SongProgressbar songprogressbar = Week.UIGetSongProgressbar(roundcontext);
            return ExportsSongProgressbar.script_week_songprogressbar_new(L, songprogressbar);*/
        }

        static int script_week_set_bpm(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;
            float bpm = L.luaL_checkfloat(1);

            Week.UpdateBpm(roundcontext, bpm);
            return 0;
        }

        static int script_week_set_speed(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;
            double speed = L.luaL_checknumber(1);

            Week.UpdateSpeed(roundcontext, speed);
            return 0;
        }

        static int script_week_ui_get_messagebox(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            MessageBox messagebox = Week.UIGetMessagebox(roundcontext);

            return ExportsMessageBox.script_messagebox_new(L, messagebox);
        }

        static int script_week_get_girlfriend(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            Character character = Week.GetGirlfriend(roundcontext);

            return ExportsCharacter.script_character_new(L, character);
        }

        static int script_week_get_character_count(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;
            int count = Week.GetCharacterCount(roundcontext);

            L.lua_pushinteger(count);
            return 1;
        }

        static int script_week_get_character(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;
            int index = (int)L.luaL_checkinteger(1);

            Character character = Week.GetCharacter(roundcontext, index);

            return ExportsCharacter.script_character_new(L, character);
        }

        static int script_week_get_songplayer(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            SongPlayer songplayer = Week.GetSongplayer(roundcontext);

            return ExportsSongPlayer.script_songplayer_new(L, songplayer);
        }

        static int script_week_get_current_chart_info(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            float bpm; double speed;

            Week.GetCurrentChartInfo(roundcontext, out bpm, out speed);

            L.lua_pushnumber(bpm);
            L.lua_pushnumber(speed);

            return 2;
        }

        static int script_week_get_current_track_info(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            string name;
            string difficult;
            int index;

            Week.GetCurrentTrackInfo(roundcontext, out name, out difficult, out index);

            L.lua_pushstring(name);
            L.lua_pushstring(difficult);
            L.lua_pushinteger(index);

            return 3;
        }

        static int script_week_change_charecter_camera_name(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            bool opponent_or_player = L.luaL_checkboolean(1);
            string new_name = L.luaL_optstring(2, null);

            Week.ChangeCharacterCameraName(roundcontext, opponent_or_player, new_name);

            return 0;
        }

        static int script_week_disable_layout_rollback(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            bool disable = L.luaL_checkboolean(1);

            Week.DisableLayoutRollback(roundcontext, disable);

            return 0;
        }

        static int script_week_override_common_folder(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            string custom_common_path = L.luaL_optstring(1, null);

            Week.OverrideCommonFolder(roundcontext, custom_common_path);

            return 0;
        }

        static int script_week_enable_credits_on_completed(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            Week.EnableCreditsOnCompleted(roundcontext);

            return 0;
        }

        static int script_week_end(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            bool round_or_week = L.luaL_checkboolean(1);
            bool loose_or_win = L.luaL_checkboolean(2);

            Week.End(roundcontext, round_or_week, loose_or_win);

            return 0;
        }

        static readonly LuaTableFunction[] EXPORTS_FUNCTION = {
            new LuaTableFunction() { name = "unlockdirective_create", func = script_week_unlockdirective_create },
            new LuaTableFunction() { name = "unlockdirective_remove", func = script_week_unlockdirective_remove },
            new LuaTableFunction() { name = "unlockdirective_get", func = script_week_unlockdirective_get },
            new LuaTableFunction() { name = "week_set_halt", func = script_week_halt },
            new LuaTableFunction() { name = "ui_set_visibility", func = script_week_ui_set_visibility },
            new LuaTableFunction() { name = "ui_get_layout", func = script_week_ui_get_layout },
            new LuaTableFunction() { name = "week_get_stage_layout", func = new LuaCallback(script_week_get_stage_layout) },
            new LuaTableFunction() { name = "ui_get_camera", func = script_week_ui_get_camera },
            new LuaTableFunction() { name = "ui_get_strums_count", func = script_week_ui_get_strums_count },
            new LuaTableFunction() { name = "ui_get_strums", func = script_week_ui_get_strums },
            new LuaTableFunction() { name = "ui_get_roundstats", func = script_week_ui_get_roundstats },
            new LuaTableFunction() { name = "ui_get_rankingcounter", func = script_week_ui_get_rankingcounter },
            new LuaTableFunction() { name = "ui_get_streakcounter", func = script_week_ui_get_streakcounter },
            new LuaTableFunction() { name = "ui_get_trackinfo", func = script_week_ui_get_trackinfo },
            new LuaTableFunction() { name = "ui_get_songprogressbar", func = script_week_ui_get_songprogressbar },
            new LuaTableFunction() { name = "week_set_bpm", func = script_week_set_bpm },
            new LuaTableFunction() { name = "week_set_speed", func = script_week_set_speed },
            new LuaTableFunction() { name = "ui_get_messagebox", func = script_week_ui_get_messagebox },
            new LuaTableFunction() { name = "week_get_girlfriend", func = script_week_get_girlfriend },
            new LuaTableFunction() { name = "week_get_character_count", func = script_week_get_character_count },
            new LuaTableFunction() { name = "week_get_character", func = script_week_get_character },
            new LuaTableFunction() { name = "week_get_songplayer", func = script_week_get_songplayer },
            new LuaTableFunction() { name = "week_get_current_chart_info", func = script_week_get_current_chart_info },
            new LuaTableFunction() { name = "week_get_current_track_info", func = script_week_get_current_track_info },
            new LuaTableFunction() { name = "week_change_charecter_camera_name", func = script_week_change_charecter_camera_name },
            new LuaTableFunction() { name = "week_disable_layout_rollback", func = script_week_disable_layout_rollback },
            new LuaTableFunction() { name = "week_override_common_folder", func = script_week_override_common_folder },
            new LuaTableFunction() { name = "week_enable_credits_on_completed", func = script_week_enable_credits_on_completed },
            new LuaTableFunction() { name = "week_end", func = script_week_end },
            new LuaTableFunction() { name = null, func = null }
        };

        static readonly LuaIntegerConstant[] EXPORTS_GLOBAL = {
            new LuaIntegerConstant() { variable = "NOTE_MISS",value = 0},
            new LuaIntegerConstant() { variable = "NOTE_PENALITY",value = 1},
            new LuaIntegerConstant() { variable = "NOTE_SHIT",value = 2},
            new LuaIntegerConstant() { variable = "NOTE_BAD",value = 3},
            new LuaIntegerConstant() { variable = "NOTE_GOOD",value = 4},
            new LuaIntegerConstant() { variable = "NOTE_SICK",value = 5},
            new LuaIntegerConstant() { variable = null, value = -1}
        };


        internal static void register_week(ManagedLuaState lua) {
            lua.RegisterGlobalFunctions(EXPORTS_FUNCTION);
            lua.RegisterIntegerConstants(EXPORTS_GLOBAL);
        }

    }
}
