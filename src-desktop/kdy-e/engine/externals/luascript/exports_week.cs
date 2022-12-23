using Engine.Externals.LuaInterop;
using Engine.Font;
using Engine.Game;
using Engine.Game.Common;
using Engine.Game.Gameplay;
using Engine.Platform;
using Engine.Sound;

namespace Engine.Externals.LuaScriptInterop {

    public static class ExportsWeek {

        static int script_week_unlockdirective_create(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            string name = L.luaL_optstring(1, null);
            bool completed_round = L.lua_toboolean(2);
            bool completed_week = L.lua_toboolean(3);
            double value = L.luaL_checknumber(4);

            Week.UnlockDirectiveCreate(roundcontext, name, completed_round, completed_week, value);

            return 0;
        }

        static int script_week_unlockdirective_remove(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            string name = L.luaL_optstring(1, null);
            bool completed_round = L.lua_toboolean(2);
            bool completed_week = L.lua_toboolean(3);

            Week.UnlockdirectiveRemove(roundcontext, name, completed_round, completed_week);

            return 0;
        }

        static int script_week_unlockdirective_get(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            string name = L.luaL_optstring(1, null);

            if (!Week.UnlockdirectiveHas(roundcontext, name)) {
                L.lua_pushnil();
                return 1;
            }

            double ret = Week.UnlockdirectiveGet(roundcontext, name);

            L.lua_pushnumber(ret);
            return 1;
        }

        static int script_week_ui_set_visibility(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            bool visible = L.lua_toboolean(1);

            Week.UISetVisibility(roundcontext, visible);

            return 0;
        }

        static int script_week_ui_get_layout(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            Layout ret = Week.UIGetLayout(roundcontext);

            return ExportsLayout.script_layout_new(L, ret);
        }

        static int script_week_ui_get_camera(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            Camera ret = Week.UIGetCamera(roundcontext);

            return ExportsCamera.script_camera_new(L, ret);
        }

        static int script_week_set_halt(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            bool halt = L.lua_toboolean(1);

            Week.SetHalt(roundcontext, halt);

            return 0;
        }

        static int script_week_ui_get_strums_count(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            int ret = Week.UiGetStrumsCount(roundcontext);

            L.lua_pushinteger(ret);
            return 1;
        }

        static int script_week_ui_get_strums(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            int strums_id = (int)L.luaL_checkinteger(1);

            Strums ret = Week.UIGetStrums(roundcontext, strums_id);

            return ExportsStrums.script_strums_new(L, ret);
        }

        static int script_week_ui_get_roundstats(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            RoundStats ret = Week.UIGetroundstats(roundcontext);

            return ExportsRoundStats.script_roundstats_new(L, ret);
        }

        static int script_week_ui_get_rankingcounter(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            RankingCounter ret = Week.UIGetRankingCounter(roundcontext);

            return ExportsRankingCounter.script_rankingcounter_new(L, ret);
        }

        static int script_week_ui_get_streakcounter(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            StreakCounter ret = Week.UIGetStreakcounter(roundcontext);

            return ExportsStreakCounter.script_streakcounter_new(L, ret);
        }

        static int script_week_ui_get_trackinfo(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            TextSprite ret = Week.UIGetTrackinfo(roundcontext);

            return ExportsTextSprite.script_textsprite_new(L, ret);
        }

        static int script_week_ui_get_songprogressbar(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            SongProgressbar ret = Week.UIGetSongprogressbar(roundcontext);

            return ExportsSongProgressbar.script_songprogressbar_new(L, ret);
        }

        static int script_week_ui_get_countdown(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            Countdown ret = Week.UIGetcountdown(roundcontext);

            return ExportsCountdown.script_countdown_new(L, ret);
        }

        static int script_week_ui_get_healthbar(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            HealthBar ret = Week.UIGetHealthbar(roundcontext);

            return ExportsHealthBar.script_healthbar_new(L, ret);
        }

        static int script_week_get_stage_layout(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            Layout ret = Week.GetStageLayout(roundcontext);

            return ExportsLayout.script_layout_new(L, ret);
        }

        static int script_week_get_healthwatcher(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            HealthWatcher ret = Week.GetHealthwatcher(roundcontext);

            return ExportsHealthWatcher.script_healthwatcher_new(L, ret);
        }

        static int script_week_get_missnotefx(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            MissNoteFX ret = Week.GetMissnotefx(roundcontext);

            return ExportsMissNoteFX.script_missnotefx_new(L, ret);
        }

        static int script_week_update_bpm(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            float bpm = (float)L.luaL_checknumber(1);

            Week.UpdateBpm(roundcontext, bpm);

            return 0;
        }

        static int script_week_update_speed(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            float speed = (float)L.luaL_checknumber(1);

            Week.UpdateSpeed(roundcontext, speed);

            return 0;
        }

        static int script_week_get_messagebox(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            MessageBox ret = Week.GetMessagebox(roundcontext);

            return ExportsMessageBox.script_messagebox_new(L, ret);
        }

        static int script_week_get_girlfriend(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            Character ret = Week.GetGirlfriend(roundcontext);

            return ExportsCharacter.script_character_new(L, ret);
        }

        static int script_week_get_character_count(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            int ret = Week.GetCharacterCount(roundcontext);

            L.lua_pushinteger(ret);
            return 1;
        }

        static int script_week_get_conductor(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            int character_index = (int)L.luaL_checkinteger(1);

            Conductor ret = Week.GetConductor(roundcontext, character_index);

            return ExportsConductor.script_conductor_new(L, ret);
        }

        static int script_week_get_character(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            int character_index = (int)L.luaL_checkinteger(1);

            Character ret = Week.GetCharacter(roundcontext, character_index);

            return ExportsCharacter.script_character_new(L, ret);
        }

        static int script_week_get_playerstats(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            int character_index = (int)L.luaL_checkinteger(1);

            PlayerStats ret = Week.GetPlayerstats(roundcontext, character_index);

            return ExportsPlayerStats.script_playerstats_new(L, ret);
        }

        static int script_week_get_songplayer(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            SongPlayer ret = Week.GetSongplayer(roundcontext);

            return ExportsSongPlayer.script_songplayer_new(L, ret);
        }

        static int script_week_get_current_chart_info(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            float bpm;
            double speed;
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

        static int script_week_change_character_camera_name(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            bool opponent_or_player = L.lua_toboolean(1);
            string new_name = L.luaL_optstring(2, null);

            Week.ChangeCharacterCameraName(roundcontext, opponent_or_player, new_name);

            return 0;
        }

        static int script_week_disable_layout_rollback(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            bool disable = L.lua_toboolean(1);

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

            bool round_or_week = L.lua_toboolean(1);
            bool loose_or_win = L.lua_toboolean(2);

            Week.End(roundcontext, round_or_week, loose_or_win);

            return 0;
        }

        static int script_week_get_dialogue(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            Dialogue dialogue = Week.GetDialogue(roundcontext);

            return ExportsDialogue.script_dialogue_new(L, dialogue);
        }

        static int script_week_set_ui_shader(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            PSShader psshader = L.ReadNullableUserdata<PSShader>(1, ExportsPSShader.PSSHADER);

            Week.SeUIShader(roundcontext, psshader);

            return 0;
        }

        static int script_week_rebuild_ui(LuaState L) {
            RoundContext roundcontext = (RoundContext)L.Context;

            Week.RebuildUI(roundcontext);

            return 0;
        }



        static readonly LuaTableFunction[] EXPORTS_FUNCTION = {
            new LuaTableFunction("week_ui_set_visibility", script_week_ui_set_visibility),
            new LuaTableFunction("week_ui_get_layout", script_week_ui_get_layout),
            new LuaTableFunction("week_ui_get_camera", script_week_ui_get_camera),
            new LuaTableFunction("week_set_halt", script_week_set_halt),
            new LuaTableFunction("week_ui_get_strums_count", script_week_ui_get_strums_count),
            new LuaTableFunction("week_ui_get_strums", script_week_ui_get_strums),
            new LuaTableFunction("week_ui_get_roundstats", script_week_ui_get_roundstats),
            new LuaTableFunction("week_ui_get_rankingcounter", script_week_ui_get_rankingcounter),
            new LuaTableFunction("week_ui_get_streakcounter", script_week_ui_get_streakcounter),
            new LuaTableFunction("week_ui_get_trackinfo", script_week_ui_get_trackinfo),
            new LuaTableFunction("week_ui_get_songprogressbar", script_week_ui_get_songprogressbar),
            new LuaTableFunction("week_ui_get_countdown", script_week_ui_get_countdown),
            new LuaTableFunction("week_ui_get_healthbar", script_week_ui_get_healthbar),
            new LuaTableFunction("week_get_stage_layout", script_week_get_stage_layout),
            new LuaTableFunction("week_get_healthwatcher", script_week_get_healthwatcher),
            new LuaTableFunction("week_get_missnotefx", script_week_get_missnotefx),
            new LuaTableFunction("week_update_bpm", script_week_update_bpm),
            new LuaTableFunction("week_update_speed", script_week_update_speed),
            new LuaTableFunction("week_get_messagebox", script_week_get_messagebox),
            new LuaTableFunction("week_get_girlfriend", script_week_get_girlfriend),
            new LuaTableFunction("week_get_character_count", script_week_get_character_count),
            new LuaTableFunction("week_get_conductor", script_week_get_conductor),
            new LuaTableFunction("week_get_character", script_week_get_character),
            new LuaTableFunction("week_get_playerstats", script_week_get_playerstats),
            new LuaTableFunction("week_get_songplayer", script_week_get_songplayer),
            new LuaTableFunction("week_get_current_chart_info", script_week_get_current_chart_info),
            new LuaTableFunction("week_get_current_track_info", script_week_get_current_track_info),
            new LuaTableFunction("week_change_character_camera_name", script_week_change_character_camera_name),
            new LuaTableFunction("week_disable_layout_rollback", script_week_disable_layout_rollback),
            new LuaTableFunction("week_override_common_folder", script_week_override_common_folder),
            new LuaTableFunction("week_enable_credits_on_completed", script_week_enable_credits_on_completed),
            new LuaTableFunction("week_end", script_week_end),
            new LuaTableFunction("week_get_dialogue", script_week_get_dialogue),
            new LuaTableFunction("week_set_ui_shader", script_week_set_ui_shader),
            new LuaTableFunction("week_rebuild_ui", script_week_rebuild_ui),
            new LuaTableFunction("week_unlockdirective_create", script_week_unlockdirective_create),
            new LuaTableFunction("week_unlockdirective_get", script_week_unlockdirective_get),
            new LuaTableFunction("week_unlockdirective_remove", script_week_unlockdirective_remove),
            new LuaTableFunction(null, null)
        };

        static readonly LuaIntegerConstant[] EXPORTS_GLOBAL = {
            new LuaIntegerConstant("NOTE_MISS", 0),
            new LuaIntegerConstant("NOTE_PENALITY", 1),
            new LuaIntegerConstant("NOTE_SHIT", 2),
            new LuaIntegerConstant("NOTE_BAD", 3),
            new LuaIntegerConstant("NOTE_GOOD", 4),
            new LuaIntegerConstant("NOTE_SICK", 5),
            new LuaIntegerConstant(null, -1)
        };


        internal static void script_week_register(ManagedLuaState lua) {
            lua.RegisterGlobalFunctions(EXPORTS_FUNCTION);
            lua.RegisterIntegerConstants(EXPORTS_GLOBAL);
        }

    }
}

