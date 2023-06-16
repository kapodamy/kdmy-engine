using Engine.Externals.LuaInterop;
using Engine.Game;

namespace Engine.Externals.LuaScriptInterop;

public static class ExportsPlayerStats {
    public const string PLAYERSTATS = "PlayerStats";


    static int script_playerstats_add_hit(LuaState L) {
        PlayerStats playerstats = L.ReadUserdata<PlayerStats>(PLAYERSTATS);
        float multiplier = (float)L.luaL_checknumber(2);
        float base_note_duration = (float)L.luaL_checknumber(3);
        double hit_time_difference = (double)L.luaL_checknumber(4);

        Ranking ret = playerstats.AddHit(multiplier, base_note_duration, hit_time_difference);

        L.lua_pushinteger((int)ret);
        return 1;
    }

    static int script_playerstats_add_sustain(LuaState L) {
        PlayerStats playerstats = L.ReadUserdata<PlayerStats>(PLAYERSTATS);
        int quarters = (int)L.luaL_checkinteger(2);
        bool is_released = L.lua_toboolean(3);

        playerstats.AddSustain(quarters, is_released);

        return 0;
    }

    static int script_playerstats_add_sustain_delayed_hit(LuaState L) {
        PlayerStats playerstats = L.ReadUserdata<PlayerStats>(PLAYERSTATS);
        float multiplier = (float)L.luaL_checknumber(2);
        float hit_time_difference = (float)L.luaL_checknumber(3);

        Ranking ret = playerstats.AddSustainDelayedHit(multiplier, hit_time_difference);

        L.lua_pushinteger((int)ret);
        return 1;
    }

    static int script_playerstats_add_penality(LuaState L) {
        PlayerStats playerstats = L.ReadUserdata<PlayerStats>(PLAYERSTATS);
        bool on_empty_strum = L.lua_toboolean(2);

        playerstats.AddPenality(on_empty_strum);

        return 0;
    }

    static int script_playerstats_add_miss(LuaState L) {
        PlayerStats playerstats = L.ReadUserdata<PlayerStats>(PLAYERSTATS);
        float multiplier = (float)L.luaL_checknumber(2);

        playerstats.AddMiss(multiplier);

        return 0;
    }

    static int script_playerstats_reset(LuaState L) {
        PlayerStats playerstats = L.ReadUserdata<PlayerStats>(PLAYERSTATS);

        playerstats.Reset();

        return 0;
    }

    static int script_playerstats_reset_notes_per_seconds(LuaState L) {
        PlayerStats playerstats = L.ReadUserdata<PlayerStats>(PLAYERSTATS);

        playerstats.ResetNotesPerSeconds();

        return 0;
    }

    static int script_playerstats_add_extra_health(LuaState L) {
        PlayerStats playerstats = L.ReadUserdata<PlayerStats>(PLAYERSTATS);
        float multiplier = (float)L.luaL_checknumber(2);

        playerstats.AddExtraHealth(multiplier);

        return 0;
    }

    static int script_playerstats_enable_penality_on_empty_strum(LuaState L) {
        PlayerStats playerstats = L.ReadUserdata<PlayerStats>(PLAYERSTATS);
        bool enable = L.lua_toboolean(2);

        playerstats.EnablePenalityOnEmptyStrum(enable);

        return 0;
    }

    static int script_playerstats_get_maximum_health(LuaState L) {
        PlayerStats playerstats = L.ReadUserdata<PlayerStats>(PLAYERSTATS);

        double ret = playerstats.GetMaximumHealth();

        L.lua_pushnumber(ret);
        return 1;
    }

    static int script_playerstats_get_health(LuaState L) {
        PlayerStats playerstats = L.ReadUserdata<PlayerStats>(PLAYERSTATS);

        double ret = playerstats.GetHealth();

        L.lua_pushnumber(ret);
        return 1;
    }

    static int script_playerstats_get_accuracy(LuaState L) {
        PlayerStats playerstats = L.ReadUserdata<PlayerStats>(PLAYERSTATS);

        double ret = playerstats.GetAccuracy();

        L.lua_pushnumber(ret);
        return 1;
    }

    static int script_playerstats_get_last_accuracy(LuaState L) {
        PlayerStats playerstats = L.ReadUserdata<PlayerStats>(PLAYERSTATS);

        double ret = playerstats.GetLastAccuracy();

        L.lua_pushnumber(ret);
        return 1;
    }

    static int script_playerstats_get_last_ranking(LuaState L) {
        PlayerStats playerstats = L.ReadUserdata<PlayerStats>(PLAYERSTATS);

        Ranking ret = playerstats.GetLastRanking();

        L.lua_pushinteger((int)ret);
        return 1;
    }

    static int script_playerstats_get_last_difference(LuaState L) {
        PlayerStats playerstats = L.ReadUserdata<PlayerStats>(PLAYERSTATS);

        double ret = playerstats.GetLastDifference();

        L.lua_pushnumber(ret);
        return 1;
    }

    static int script_playerstats_get_combo_streak(LuaState L) {
        PlayerStats playerstats = L.ReadUserdata<PlayerStats>(PLAYERSTATS);

        int ret = playerstats.GetComboBreaks();

        L.lua_pushinteger(ret);
        return 1;
    }

    static int script_playerstats_get_highest_combo_streak(LuaState L) {
        PlayerStats playerstats = L.ReadUserdata<PlayerStats>(PLAYERSTATS);

        int ret = playerstats.GetHighestComboStreak();

        L.lua_pushinteger(ret);
        return 1;
    }

    static int script_playerstats_get_combo_breaks(LuaState L) {
        PlayerStats playerstats = L.ReadUserdata<PlayerStats>(PLAYERSTATS);

        int ret = playerstats.GetComboBreaks();

        L.lua_pushinteger(ret);
        return 1;
    }

    static int script_playerstats_get_notes_per_seconds(LuaState L) {
        PlayerStats playerstats = L.ReadUserdata<PlayerStats>(PLAYERSTATS);

        int ret = playerstats.GetNotesPerSeconds();

        L.lua_pushinteger(ret);
        return 1;
    }

    static int script_playerstats_get_notes_per_seconds_highest(LuaState L) {
        PlayerStats playerstats = L.ReadUserdata<PlayerStats>(PLAYERSTATS);

        int ret = playerstats.GetNotesPerSecondsHighest();

        L.lua_pushinteger(ret);
        return 1;
    }

    static int script_playerstats_get_iterations(LuaState L) {
        PlayerStats playerstats = L.ReadUserdata<PlayerStats>(PLAYERSTATS);

        int ret = playerstats.GetIterations();

        L.lua_pushinteger(ret);
        return 1;
    }

    static int script_playerstats_get_score(LuaState L) {
        PlayerStats playerstats = L.ReadUserdata<PlayerStats>(PLAYERSTATS);

        long ret = playerstats.GetScore();

        L.lua_pushinteger(ret);
        return 1;
    }

    static int script_playerstats_get_hits(LuaState L) {
        PlayerStats playerstats = L.ReadUserdata<PlayerStats>(PLAYERSTATS);

        int ret = playerstats.GetHits();

        L.lua_pushinteger(ret);
        return 1;
    }

    static int script_playerstats_get_misses(LuaState L) {
        PlayerStats playerstats = L.ReadUserdata<PlayerStats>(PLAYERSTATS);

        int ret = playerstats.GetMisses();

        L.lua_pushinteger(ret);
        return 1;
    }

    static int script_playerstats_get_penalties(LuaState L) {
        PlayerStats playerstats = L.ReadUserdata<PlayerStats>(PLAYERSTATS);

        int ret = playerstats.GetPenalties();

        L.lua_pushinteger(ret);
        return 1;
    }

    static int script_playerstats_get_shits(LuaState L) {
        PlayerStats playerstats = L.ReadUserdata<PlayerStats>(PLAYERSTATS);

        int ret = playerstats.GetShits();

        L.lua_pushinteger(ret);
        return 1;
    }

    static int script_playerstats_get_bads(LuaState L) {
        PlayerStats playerstats = L.ReadUserdata<PlayerStats>(PLAYERSTATS);

        int ret = playerstats.GetBads();

        L.lua_pushinteger(ret);
        return 1;
    }

    static int script_playerstats_get_goods(LuaState L) {
        PlayerStats playerstats = L.ReadUserdata<PlayerStats>(PLAYERSTATS);

        int ret = playerstats.GetGoods();

        L.lua_pushinteger(ret);
        return 1;
    }

    static int script_playerstats_get_sicks(LuaState L) {
        PlayerStats playerstats = L.ReadUserdata<PlayerStats>(PLAYERSTATS);

        int ret = playerstats.GetSicks();

        L.lua_pushinteger(ret);
        return 1;
    }

    static int script_playerstats_set_health(LuaState L) {
        PlayerStats playerstats = L.ReadUserdata<PlayerStats>(PLAYERSTATS);
        double health = (double)L.luaL_checknumber(2);

        playerstats.SetHealth(health);

        return 0;
    }

    static int script_playerstats_add_health(LuaState L) {
        PlayerStats playerstats = L.ReadUserdata<PlayerStats>(PLAYERSTATS);
        double health = (double)L.luaL_checknumber(2);
        bool die_if_negative = L.lua_toboolean(3);

        double ret = playerstats.AddHealth(health, die_if_negative);

        L.lua_pushnumber(ret);
        return 1;
    }

    static int script_playerstats_raise(LuaState L) {
        PlayerStats playerstats = L.ReadUserdata<PlayerStats>(PLAYERSTATS);
        bool with_full_health = L.lua_toboolean(2);

        playerstats.Raise(with_full_health);

        return 0;
    }

    static int script_playerstats_kill(LuaState L) {
        PlayerStats playerstats = L.ReadUserdata<PlayerStats>(PLAYERSTATS);

        playerstats.Kill();

        return 0;
    }

    static int script_playerstats_kill_if_negative_health(LuaState L) {
        PlayerStats playerstats = L.ReadUserdata<PlayerStats>(PLAYERSTATS);

        playerstats.KillIfNegativeHealth();

        return 0;
    }

    static int script_playerstats_is_dead(LuaState L) {
        PlayerStats playerstats = L.ReadUserdata<PlayerStats>(PLAYERSTATS);

        bool ret = playerstats.IsDead();

        L.lua_pushboolean(ret);
        return 1;
    }




    static readonly LuaTableFunction[] PLAYERSTATS_FUNCTIONS = {
        new LuaTableFunction("add_hit", script_playerstats_add_hit),
        new LuaTableFunction("add_sustain", script_playerstats_add_sustain),
        new LuaTableFunction("add_sustain_delayed_hit", script_playerstats_add_sustain_delayed_hit),
        new LuaTableFunction("add_penality", script_playerstats_add_penality),
        new LuaTableFunction("add_miss", script_playerstats_add_miss),
        new LuaTableFunction("reset", script_playerstats_reset),
        new LuaTableFunction("reset_notes_per_seconds", script_playerstats_reset_notes_per_seconds),
        new LuaTableFunction("add_extra_health", script_playerstats_add_extra_health),
        new LuaTableFunction("enable_penality_on_empty_strum", script_playerstats_enable_penality_on_empty_strum),
        new LuaTableFunction("get_maximum_health", script_playerstats_get_maximum_health),
        new LuaTableFunction("get_health", script_playerstats_get_health),
        new LuaTableFunction("get_accuracy", script_playerstats_get_accuracy),
        new LuaTableFunction("get_last_accuracy", script_playerstats_get_last_accuracy),
        new LuaTableFunction("get_last_ranking", script_playerstats_get_last_ranking),
        new LuaTableFunction("get_last_difference", script_playerstats_get_last_difference),
        new LuaTableFunction("get_combo_streak", script_playerstats_get_combo_streak),
        new LuaTableFunction("get_highest_combo_streak", script_playerstats_get_highest_combo_streak),
        new LuaTableFunction("get_combo_breaks", script_playerstats_get_combo_breaks),
        new LuaTableFunction("get_notes_per_seconds", script_playerstats_get_notes_per_seconds),
        new LuaTableFunction("get_notes_per_seconds_highest", script_playerstats_get_notes_per_seconds_highest),
        new LuaTableFunction("get_iterations", script_playerstats_get_iterations),
        new LuaTableFunction("get_score", script_playerstats_get_score),
        new LuaTableFunction("get_hits", script_playerstats_get_hits),
        new LuaTableFunction("get_misses", script_playerstats_get_misses),
        new LuaTableFunction("get_penalties", script_playerstats_get_penalties),
        new LuaTableFunction("get_shits", script_playerstats_get_shits),
        new LuaTableFunction("get_bads", script_playerstats_get_bads),
        new LuaTableFunction("get_goods", script_playerstats_get_goods),
        new LuaTableFunction("get_sicks", script_playerstats_get_sicks),
        new LuaTableFunction("set_health", script_playerstats_set_health),
        new LuaTableFunction("add_health", script_playerstats_add_health),
        new LuaTableFunction("raise", script_playerstats_raise),
        new LuaTableFunction("kill", script_playerstats_kill),
        new LuaTableFunction("kill_if_negative_health", script_playerstats_kill_if_negative_health),
        new LuaTableFunction("is_dead", script_playerstats_is_dead),
        new LuaTableFunction(null, null)
    };

    public static int script_playerstats_new(LuaState L, PlayerStats playerstats) {
        return L.CreateUserdata(PLAYERSTATS, playerstats);
    }

    static int script_playerstats_gc(LuaState L) {
        return L.GC_userdata(PLAYERSTATS);
    }

    static int script_playerstats_tostring(LuaState L) {
        return L.ToString_userdata(PLAYERSTATS);
    }

    private static readonly LuaCallback delegate_gc = script_playerstats_gc;
    private static readonly LuaCallback delegate_tostring = script_playerstats_tostring;

    public static void script_playerstats_register(ManagedLuaState L) {
        L.RegisterMetaTable(PLAYERSTATS, delegate_gc, delegate_tostring, PLAYERSTATS_FUNCTIONS);
    }


}

