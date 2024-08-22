using System;
using Engine.Externals.LuaInterop;
using Engine.Font;
using Engine.Game;
using Engine.Game.Common;
using Engine.Game.Gameplay;
using Engine.Platform;
using Engine.Sound;

namespace Engine.Externals.LuaScriptInterop;

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

    static int script_week_disable_week_end_results(LuaState L) {
        RoundContext roundcontext = (RoundContext)L.Context;

        bool disable = L.lua_toboolean(1);

        Week.DisableWeekEndResults(roundcontext, disable);

        return 0;
    }

    static int script_week_disable_girlfriend_cry(LuaState L) {
        RoundContext roundcontext = (RoundContext)L.Context;

        bool disable = L.lua_toboolean(1);

        Week.DisableGirlfriendCry(roundcontext, disable);

        return 0;
    }

    static int script_week_disable_ask_ready(LuaState L) {
        RoundContext roundcontext = (RoundContext)L.Context;

        bool disable = L.lua_toboolean(1);

        Week.DisableAskReady(roundcontext, disable);

        return 0;
    }

    static int script_week_disable_countdown(LuaState L) {
        RoundContext roundcontext = (RoundContext)L.Context;

        bool disable = L.lua_toboolean(1);

        Week.DisableCountdown(roundcontext, disable);

        return 0;
    }

    static int script_week_disable_camera_bumping(LuaState L) {
        RoundContext roundcontext = (RoundContext)L.Context;

        bool disable = L.lua_toboolean(1);

        Week.DisableCameraBumping(roundcontext, disable);

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

    static int script_week_ui_get_round_textsprite(LuaState L) {
        RoundContext roundcontext = (RoundContext)L.Context;

        TextSprite ret = Week.UIGetRoundTextsprite(roundcontext);

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

    static int script_week_get_current_song_info(LuaState L) {
        RoundContext roundcontext = (RoundContext)L.Context;

        string name;
        string difficult;
        int index;
        Week.GetCurrentSongInfo(roundcontext, out name, out difficult, out index);

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

        LuascriptHelpers.ChangeWorkingFolder(L);
        Week.OverrideCommonFolder(roundcontext, custom_common_path);
        LuascriptHelpers.RestoreWorkingFolder(L);

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

    static int script_week_storage_get_blob(LuaState L) {
        RoundContext roundcontext = (RoundContext)L.Context;

        string name = L.luaL_optstring(1, null);

        byte[] data;
        uint ret = Week.StorageGet(roundcontext, name, out data);

        if (data == null)
            L.lua_pushnil();
        else
            L.lua_pushlstring(data, (int)ret);

        return 1;
    }

    static int script_week_storage_set_blob(LuaState L) {
        RoundContext roundcontext = (RoundContext)L.Context;

        string name = L.luaL_checkstring(1);
        ReadOnlySpan<byte> data;
        uint data_size;

        if (L.lua_isnil(2)) {
            data = null;
            data_size = 0;
        } else {
            data = L.luaL_checklstring(2);
            data_size = data == null ? 0 : (uint)data.Length;
        }

        bool ret = Week.StorageSet(roundcontext, name, data, data_size);

        L.lua_pushboolean(ret);
        return 1;
    }

    static int script_week_gameover_no_music(LuaState L) {
        RoundContext roundcontext = (RoundContext)L.Context;

        Week.SetGameoverOption(roundcontext, WeekGameOverOption.NoMusic, Single.NaN, null);

        return 0;
    }

    static int script_week_gameover_no_sfx_die(LuaState L) {
        RoundContext roundcontext = (RoundContext)L.Context;

        Week.SetGameoverOption(roundcontext, WeekGameOverOption.NoSfxDie, Single.NaN, null);

        return 0;
    }

    static int script_week_gameover_no_sfx_retry(LuaState L) {
        RoundContext roundcontext = (RoundContext)L.Context;

        Week.SetGameoverOption(roundcontext, WeekGameOverOption.NoSfxRetry, Single.NaN, null);

        return 0;
    }

    static int script_week_gameover_set_die_anim_duration(LuaState L) {
        RoundContext roundcontext = (RoundContext)L.Context;

        float duration_ms = (float)L.luaL_checknumber(1);

        Week.SetGameoverOption(roundcontext, WeekGameOverOption.AnimDurationDie, duration_ms, null);

        return 0;
    }

    static int script_week_gameover_set_retry_anim_duration(LuaState L) {
        RoundContext roundcontext = (RoundContext)L.Context;

        float duration_ms = (float)L.luaL_checknumber(1);

        Week.SetGameoverOption(roundcontext, WeekGameOverOption.AnimDurationRetry, duration_ms, null);

        return 0;
    }

    static int script_week_gameover_set_giveup_anim_duration(LuaState L) {
        RoundContext roundcontext = (RoundContext)L.Context;

        float duration_ms = (float)L.luaL_checknumber(1);

        Week.SetGameoverOption(roundcontext, WeekGameOverOption.AnimDurationGiveup, duration_ms, null);

        return 0;
    }

    static int script_week_gameover_set_before_anim_duration(LuaState L) {
        RoundContext roundcontext = (RoundContext)L.Context;

        float duration_ms = (float)L.luaL_checknumber(1);

        Week.SetGameoverOption(roundcontext, WeekGameOverOption.AnimDurationBefore, duration_ms, null);

        return 0;
    }

    static int script_week_gameover_set_before_force_end_anim_duration(LuaState L) {
        RoundContext roundcontext = (RoundContext)L.Context;

        float duration_ms = (float)L.luaL_checknumber(1);

        Week.SetGameoverOption(roundcontext, WeekGameOverOption.AnimDurationBeforeForceEnd, duration_ms, null);

        return 0;
    }

    static int script_week_gameover_set_music(LuaState L) {
        RoundContext roundcontext = (RoundContext)L.Context;

        string filename = L.luaL_optstring(1, null);

        Week.SetGameoverOption(roundcontext, WeekGameOverOption.SetMusic, Single.NaN, filename);

        return 0;
    }

    static int script_week_gameover_set_sfx_die(LuaState L) {
        RoundContext roundcontext = (RoundContext)L.Context;

        string filename = L.luaL_optstring(1, null);

        Week.SetGameoverOption(roundcontext, WeekGameOverOption.SetSfxDie, Single.NaN, filename);

        return 0;
    }

    static int script_week_gameover_set_sfx_confirm(LuaState L) {
        RoundContext roundcontext = (RoundContext)L.Context;

        string filename = L.luaL_optstring(1, null);

        Week.SetGameoverOption(roundcontext, WeekGameOverOption.SetSfxRetry, Single.NaN, filename);

        return 0;
    }

    static int script_week_get_accumulated_stats(LuaState L) {
        RoundContext roundcontext = (RoundContext)L.Context;

        WeekResult_Stats stats = new WeekResult_Stats();

        Week.GetAccumulatedStats(roundcontext, ref stats);

        LuaTableBuilder table = new LuaTableBuilder(11);
        table.AddInteger("sick", stats.sick);
        table.AddInteger("good", stats.good);
        table.AddInteger("bads", stats.bads);
        table.AddInteger("shits", stats.shits);
        table.AddInteger("miss", stats.miss);
        table.AddInteger("penalties", stats.penalties);
        table.AddInteger("score", stats.score);
        table.AddNumber("accuracy", stats.accuracy);
        table.AddInteger("notesPerSeconds", stats.notesperseconds);
        table.AddInteger("comboBreaks", stats.combobreaks);
        table.AddInteger("highestStreak", stats.higheststreak);

        table.PushTable(L);
        return 1;
    }

    static int script_week_get_gameover_layout(LuaState L) {
        RoundContext roundcontext = (RoundContext)L.Context;

        Layout ret = Week.GetLayoutOf(roundcontext, 'g');

        return ExportsLayout.script_layout_new(L, ret);
    }

    static int script_week_get_pause_layout(LuaState L) {
        RoundContext roundcontext = (RoundContext)L.Context;

        Layout ret = Week.GetLayoutOf(roundcontext, 'p');

        return ExportsLayout.script_layout_new(L, ret);
    }

    static int script_week_get_results_layout(LuaState L) {
        RoundContext roundcontext = (RoundContext)L.Context;

        Layout ret = Week.GetLayoutOf(roundcontext, 'r');

        return ExportsLayout.script_layout_new(L, ret);
    }

    static int script_week_set_pause_background_music(LuaState L) {
        RoundContext roundcontext = (RoundContext)L.Context;

        string filename = L.luaL_optstring(1, null);

        Week.SetPauseBackgroundMusic(roundcontext, filename);

        return 0;
    }


    static readonly LuaTableFunction[] EXPORTS_FUNCTION = {
        new LuaTableFunction("week_ui_set_visibility", script_week_ui_set_visibility),
        new LuaTableFunction("week_ui_get_layout", script_week_ui_get_layout),
        new LuaTableFunction("week_ui_get_camera", script_week_ui_get_camera),
        new LuaTableFunction("week_set_halt", script_week_set_halt),
        new LuaTableFunction("week_disable_week_end_results", script_week_disable_week_end_results),
        new LuaTableFunction("week_disable_girlfriend_cry", script_week_disable_girlfriend_cry),
        new LuaTableFunction("week_disable_ask_ready", script_week_disable_ask_ready),
        new LuaTableFunction("week_disable_countdown", script_week_disable_countdown),
        new LuaTableFunction("week_disable_camera_bumping", script_week_disable_camera_bumping),
        new LuaTableFunction("week_ui_get_strums_count", script_week_ui_get_strums_count),
        new LuaTableFunction("week_ui_get_strums", script_week_ui_get_strums),
        new LuaTableFunction("week_ui_get_roundstats", script_week_ui_get_roundstats),
        new LuaTableFunction("week_ui_get_rankingcounter", script_week_ui_get_rankingcounter),
        new LuaTableFunction("week_ui_get_streakcounter", script_week_ui_get_streakcounter),
        new LuaTableFunction("week_ui_get_round_textsprite", script_week_ui_get_round_textsprite),
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
        new LuaTableFunction("week_get_current_song_info", script_week_get_current_song_info),
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
        new LuaTableFunction("week_storage_get_blob", script_week_storage_get_blob),
        new LuaTableFunction("week_storage_set_blob", script_week_storage_set_blob),
        new LuaTableFunction("week_gameover_no_music", script_week_gameover_no_music),
        new LuaTableFunction("week_gameover_no_sfx_die", script_week_gameover_no_sfx_die),
        new LuaTableFunction("week_gameover_no_sfx_retry", script_week_gameover_no_sfx_retry),
        new LuaTableFunction("week_gameover_set_die_anim_duration", script_week_gameover_set_die_anim_duration),
        new LuaTableFunction("week_gameover_set_retry_anim_duration", script_week_gameover_set_retry_anim_duration),
        new LuaTableFunction("week_gameover_set_giveup_anim_duration", script_week_gameover_set_giveup_anim_duration),
        new LuaTableFunction("week_gameover_set_before_anim_duration", script_week_gameover_set_before_anim_duration),
        new LuaTableFunction("week_gameover_set_before_force_end_anim_duration", script_week_gameover_set_before_force_end_anim_duration),
        new LuaTableFunction("week_gameover_set_music", script_week_gameover_set_music),
        new LuaTableFunction("week_gameover_set_sfx_die", script_week_gameover_set_sfx_die),
        new LuaTableFunction("week_gameover_set_sfx_confirm", script_week_gameover_set_sfx_confirm),
        new LuaTableFunction("week_get_accumulated_stats", script_week_get_accumulated_stats),
        new LuaTableFunction("week_get_gameover_layout", script_week_get_gameover_layout),
        new LuaTableFunction("week_get_pause_layout", script_week_get_pause_layout),
        new LuaTableFunction("week_get_results_layout", script_week_get_results_layout),
        new LuaTableFunction("week_set_pause_background_music", script_week_set_pause_background_music),
        new LuaTableFunction(null, null)
    };


    internal static void script_week_register(ManagedLuaState lua) {
        lua.RegisterGlobalFunctions(EXPORTS_FUNCTION);
    }

}

