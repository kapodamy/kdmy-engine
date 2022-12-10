#include "luascript_internal.h"

#ifdef JAVASCRIPT
EM_JS_PRFX(Ranking, playerstats_add_hit, (PlayerStats playerstats, float multiplier, float base_note_duration, double hit_time_difference), {
    let ret = playerstats_add_hit(kdmyEngine_obtain(playerstats), multiplier, base_note_duration, hit_time_difference);
    return ret;
});
EM_JS_PRFX(void, playerstats_add_sustain, (PlayerStats playerstats, int32_t quarters, bool is_released), {
    playerstats_add_sustain(kdmyEngine_obtain(playerstats), quarters, is_released);
});
EM_JS_PRFX(Ranking, playerstats_add_sustain_delayed_hit, (PlayerStats playerstats, float multiplier, float hit_time_difference), {
    let ret = playerstats_add_sustain_delayed_hit(kdmyEngine_obtain(playerstats), multiplier, hit_time_difference);
    return ret;
});
EM_JS_PRFX(void, playerstats_add_penality, (PlayerStats playerstats, bool on_empty_strum), {
    playerstats_add_penality(kdmyEngine_obtain(playerstats), on_empty_strum);
});
EM_JS_PRFX(void, playerstats_add_miss, (PlayerStats playerstats, float multiplier), {
    playerstats_add_miss(kdmyEngine_obtain(playerstats), multiplier);
});
EM_JS_PRFX(void, playerstats_reset, (PlayerStats playerstats), {
    playerstats_reset(kdmyEngine_obtain(playerstats));
});
EM_JS_PRFX(void, playerstats_reset_notes_per_seconds, (PlayerStats playerstats), {
    playerstats_reset_notes_per_seconds(kdmyEngine_obtain(playerstats));
});
EM_JS_PRFX(void, playerstats_add_extra_health, (PlayerStats playerstats, float multiplier), {
    playerstats_add_extra_health(kdmyEngine_obtain(playerstats), multiplier);
});
EM_JS_PRFX(void, playerstats_enable_penality_on_empty_strum, (PlayerStats playerstats, bool enable), {
    playerstats_enable_penality_on_empty_strum(kdmyEngine_obtain(playerstats), enable);
});
EM_JS_PRFX(double, playerstats_get_maximum_health, (PlayerStats playerstats), {
    let ret = playerstats_get_maximum_health(kdmyEngine_obtain(playerstats));
    return ret;
});
EM_JS_PRFX(double, playerstats_get_health, (PlayerStats playerstats), {
    let ret = playerstats_get_health(kdmyEngine_obtain(playerstats));
    return ret;
});
EM_JS_PRFX(double, playerstats_get_accuracy, (PlayerStats playerstats), {
    let ret = playerstats_get_accuracy(kdmyEngine_obtain(playerstats));
    return ret;
});
EM_JS_PRFX(double, playerstats_get_last_accuracy, (PlayerStats playerstats), {
    let ret = playerstats_get_last_accuracy(kdmyEngine_obtain(playerstats));
    return ret;
});
EM_JS_PRFX(Ranking, playerstats_get_last_ranking, (PlayerStats playerstats), {
    let ret = playerstats_get_last_ranking(kdmyEngine_obtain(playerstats));
    return ret;
});
EM_JS_PRFX(double, playerstats_get_last_difference, (PlayerStats playerstats), {
    let ret = playerstats_get_last_difference(kdmyEngine_obtain(playerstats));
    return ret;
});
EM_JS_PRFX(int32_t, playerstats_get_combo_streak, (PlayerStats playerstats), {
    let ret = playerstats_get_combo_streak(kdmyEngine_obtain(playerstats));
    return ret;
});
EM_JS_PRFX(int32_t, playerstats_get_highest_combo_streak, (PlayerStats playerstats), {
    let ret = playerstats_get_highest_combo_streak(kdmyEngine_obtain(playerstats));
    return ret;
});
EM_JS_PRFX(int32_t, playerstats_get_combo_breaks, (PlayerStats playerstats), {
    let ret = playerstats_get_combo_breaks(kdmyEngine_obtain(playerstats));
    return ret;
});
EM_JS_PRFX(int32_t, playerstats_get_notes_per_seconds, (PlayerStats playerstats), {
    let ret = playerstats_get_notes_per_seconds(kdmyEngine_obtain(playerstats));
    return ret;
});
EM_JS_PRFX(int32_t, playerstats_get_notes_per_seconds_highest, (PlayerStats playerstats), {
    let ret = playerstats_get_notes_per_seconds_highest(kdmyEngine_obtain(playerstats));
    return ret;
});
EM_JS_PRFX(int32_t, playerstats_get_iterations, (PlayerStats playerstats), {
    let ret = playerstats_get_iterations(kdmyEngine_obtain(playerstats));
    return ret;
});
EM_JS_PRFX(int64_t, playerstats_get_score, (PlayerStats playerstats), {
    let ret = playerstats_get_score(kdmyEngine_obtain(playerstats));
    return ret;
});
EM_JS_PRFX(int32_t, playerstats_get_hits, (PlayerStats playerstats), {
    let ret = playerstats_get_hits(kdmyEngine_obtain(playerstats));
    return ret;
});
EM_JS_PRFX(int32_t, playerstats_get_misses, (PlayerStats playerstats), {
    let ret = playerstats_get_misses(kdmyEngine_obtain(playerstats));
    return ret;
});
EM_JS_PRFX(int32_t, playerstats_get_penalties, (PlayerStats playerstats), {
    let ret = playerstats_get_penalties(kdmyEngine_obtain(playerstats));
    return ret;
});
EM_JS_PRFX(int32_t, playerstats_get_shits, (PlayerStats playerstats), {
    let ret = playerstats_get_shits(kdmyEngine_obtain(playerstats));
    return ret;
});
EM_JS_PRFX(int32_t, playerstats_get_bads, (PlayerStats playerstats), {
    let ret = playerstats_get_bads(kdmyEngine_obtain(playerstats));
    return ret;
});
EM_JS_PRFX(int32_t, playerstats_get_goods, (PlayerStats playerstats), {
    let ret = playerstats_get_goods(kdmyEngine_obtain(playerstats));
    return ret;
});
EM_JS_PRFX(int32_t, playerstats_get_sicks, (PlayerStats playerstats), {
    let ret = playerstats_get_sicks(kdmyEngine_obtain(playerstats));
    return ret;
});
EM_JS_PRFX(void, playerstats_set_health, (PlayerStats playerstats, double health), {
    playerstats_set_health(kdmyEngine_obtain(playerstats), health);
});
EM_JS_PRFX(double, playerstats_add_health, (PlayerStats playerstats, double health, bool die_if_negative), {
    let ret = playerstats_add_health(kdmyEngine_obtain(playerstats), health, die_if_negative);
    return ret;
});
EM_JS_PRFX(void, playerstats_raise, (PlayerStats playerstats, bool with_full_health), {
    playerstats_raise(kdmyEngine_obtain(playerstats), with_full_health);
});
EM_JS_PRFX(void, playerstats_kill, (PlayerStats playerstats), {
    playerstats_kill(kdmyEngine_obtain(playerstats));
});
EM_JS_PRFX(void, playerstats_kill_if_negative_health, (PlayerStats playerstats), {
    playerstats_kill_if_negative_health(kdmyEngine_obtain(playerstats));
});
EM_JS_PRFX(bool, playerstats_is_dead, (PlayerStats playerstats), {
    let ret = playerstats_is_dead(kdmyEngine_obtain(playerstats));
    return ret ? 1 : 0;
});
#endif


static int script_playerstats_add_hit(lua_State* L) {
    PlayerStats playerstats = luascript_read_userdata(L, PLAYERSTATS);
    float multiplier = (float)luaL_checknumber(L, 2);
    float base_note_duration = (float)luaL_checknumber(L, 3);
    double hit_time_difference = (double)luaL_checknumber(L, 4);

    Ranking ret = playerstats_add_hit(playerstats, multiplier, base_note_duration, hit_time_difference);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_playerstats_add_sustain(lua_State* L) {
    PlayerStats playerstats = luascript_read_userdata(L, PLAYERSTATS);
    int32_t quarters = (int32_t)luaL_checkinteger(L, 2);
    bool is_released = (bool)lua_toboolean(L, 3);

    playerstats_add_sustain(playerstats, quarters, is_released);

    return 0;
}

static int script_playerstats_add_sustain_delayed_hit(lua_State* L) {
    PlayerStats playerstats = luascript_read_userdata(L, PLAYERSTATS);
    float multiplier = (float)luaL_checknumber(L, 2);
    float hit_time_difference = (float)luaL_checknumber(L, 3);

    Ranking ret = playerstats_add_sustain_delayed_hit(playerstats, multiplier, hit_time_difference);

    lua_pushinteger(L, ret);
    return 1;
}

static int script_playerstats_add_penality(lua_State* L) {
    PlayerStats playerstats = luascript_read_userdata(L, PLAYERSTATS);
    bool on_empty_strum = (bool)lua_toboolean(L, 2);

    playerstats_add_penality(playerstats, on_empty_strum);

    return 0;
}

static int script_playerstats_add_miss(lua_State* L) {
    PlayerStats playerstats = luascript_read_userdata(L, PLAYERSTATS);
    float multiplier = (float)luaL_checknumber(L, 2);

    playerstats_add_miss(playerstats, multiplier);

    return 0;
}

static int script_playerstats_reset(lua_State* L) {
    PlayerStats playerstats = luascript_read_userdata(L, PLAYERSTATS);

    playerstats_reset(playerstats);

    return 0;
}

static int script_playerstats_reset_notes_per_seconds(lua_State* L) {
    PlayerStats playerstats = luascript_read_userdata(L, PLAYERSTATS);

    playerstats_reset_notes_per_seconds(playerstats);

    return 0;
}

static int script_playerstats_add_extra_health(lua_State* L) {
    PlayerStats playerstats = luascript_read_userdata(L, PLAYERSTATS);
    float multiplier = (float)luaL_checknumber(L, 2);

    playerstats_add_extra_health(playerstats, multiplier);

    return 0;
}

static int script_playerstats_enable_penality_on_empty_strum(lua_State* L) {
    PlayerStats playerstats = luascript_read_userdata(L, PLAYERSTATS);
    bool enable = (bool)lua_toboolean(L, 2);

    playerstats_enable_penality_on_empty_strum(playerstats, enable);

    return 0;
}

static int script_playerstats_get_maximum_health(lua_State* L) {
    PlayerStats playerstats = luascript_read_userdata(L, PLAYERSTATS);

    double ret = playerstats_get_maximum_health(playerstats);

    lua_pushnumber(L, ret);
    return 1;
}

static int script_playerstats_get_health(lua_State* L) {
    PlayerStats playerstats = luascript_read_userdata(L, PLAYERSTATS);

    double ret = playerstats_get_health(playerstats);

    lua_pushnumber(L, ret);
    return 1;
}

static int script_playerstats_get_accuracy(lua_State* L) {
    PlayerStats playerstats = luascript_read_userdata(L, PLAYERSTATS);

    double ret = playerstats_get_accuracy(playerstats);

    lua_pushnumber(L, ret);
    return 1;
}

static int script_playerstats_get_last_accuracy(lua_State* L) {
    PlayerStats playerstats = luascript_read_userdata(L, PLAYERSTATS);

    double ret = playerstats_get_last_accuracy(playerstats);

    lua_pushnumber(L, ret);
    return 1;
}

static int script_playerstats_get_last_ranking(lua_State* L) {
    PlayerStats playerstats = luascript_read_userdata(L, PLAYERSTATS);

    Ranking ret = playerstats_get_last_ranking(playerstats);

    lua_pushinteger(L, ret);
    return 1;
}

static int script_playerstats_get_last_difference(lua_State* L) {
    PlayerStats playerstats = luascript_read_userdata(L, PLAYERSTATS);

    double ret = playerstats_get_last_difference(playerstats);

    lua_pushnumber(L, ret);
    return 1;
}

static int script_playerstats_get_combo_streak(lua_State* L) {
    PlayerStats playerstats = luascript_read_userdata(L, PLAYERSTATS);

    int32_t ret = playerstats_get_combo_streak(playerstats);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_playerstats_get_highest_combo_streak(lua_State* L) {
    PlayerStats playerstats = luascript_read_userdata(L, PLAYERSTATS);

    int32_t ret = playerstats_get_highest_combo_streak(playerstats);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_playerstats_get_combo_breaks(lua_State* L) {
    PlayerStats playerstats = luascript_read_userdata(L, PLAYERSTATS);

    int32_t ret = playerstats_get_combo_breaks(playerstats);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_playerstats_get_notes_per_seconds(lua_State* L) {
    PlayerStats playerstats = luascript_read_userdata(L, PLAYERSTATS);

    int32_t ret = playerstats_get_notes_per_seconds(playerstats);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_playerstats_get_notes_per_seconds_highest(lua_State* L) {
    PlayerStats playerstats = luascript_read_userdata(L, PLAYERSTATS);

    int32_t ret = playerstats_get_notes_per_seconds_highest(playerstats);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_playerstats_get_iterations(lua_State* L) {
    PlayerStats playerstats = luascript_read_userdata(L, PLAYERSTATS);

    int32_t ret = playerstats_get_iterations(playerstats);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_playerstats_get_score(lua_State* L) {
    PlayerStats playerstats = luascript_read_userdata(L, PLAYERSTATS);

    int64_t ret = playerstats_get_score(playerstats);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_playerstats_get_hits(lua_State* L) {
    PlayerStats playerstats = luascript_read_userdata(L, PLAYERSTATS);

    int32_t ret = playerstats_get_hits(playerstats);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_playerstats_get_misses(lua_State* L) {
    PlayerStats playerstats = luascript_read_userdata(L, PLAYERSTATS);

    int32_t ret = playerstats_get_misses(playerstats);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_playerstats_get_penalties(lua_State* L) {
    PlayerStats playerstats = luascript_read_userdata(L, PLAYERSTATS);

    int32_t ret = playerstats_get_penalties(playerstats);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_playerstats_get_shits(lua_State* L) {
    PlayerStats playerstats = luascript_read_userdata(L, PLAYERSTATS);

    int32_t ret = playerstats_get_shits(playerstats);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_playerstats_get_bads(lua_State* L) {
    PlayerStats playerstats = luascript_read_userdata(L, PLAYERSTATS);

    int32_t ret = playerstats_get_bads(playerstats);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_playerstats_get_goods(lua_State* L) {
    PlayerStats playerstats = luascript_read_userdata(L, PLAYERSTATS);

    int32_t ret = playerstats_get_goods(playerstats);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_playerstats_get_sicks(lua_State* L) {
    PlayerStats playerstats = luascript_read_userdata(L, PLAYERSTATS);

    int32_t ret = playerstats_get_sicks(playerstats);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_playerstats_set_health(lua_State* L) {
    PlayerStats playerstats = luascript_read_userdata(L, PLAYERSTATS);
    double health = (double)luaL_checknumber(L, 2);

    playerstats_set_health(playerstats, health);

    return 0;
}

static int script_playerstats_add_health(lua_State* L) {
    PlayerStats playerstats = luascript_read_userdata(L, PLAYERSTATS);
    double health = (double)luaL_checknumber(L, 2);
    bool die_if_negative = (bool)lua_toboolean(L, 3);

    double ret = playerstats_add_health(playerstats, health, die_if_negative);

    lua_pushnumber(L, ret);
    return 1;
}

static int script_playerstats_raise(lua_State* L) {
    PlayerStats playerstats = luascript_read_userdata(L, PLAYERSTATS);
    bool with_full_health = (bool)lua_toboolean(L, 2);

    playerstats_raise(playerstats, with_full_health);

    return 0;
}

static int script_playerstats_kill(lua_State* L) {
    PlayerStats playerstats = luascript_read_userdata(L, PLAYERSTATS);

    playerstats_kill(playerstats);

    return 0;
}

static int script_playerstats_kill_if_negative_health(lua_State* L) {
    PlayerStats playerstats = luascript_read_userdata(L, PLAYERSTATS);

    playerstats_kill_if_negative_health(playerstats);

    return 0;
}

static int script_playerstats_is_dead(lua_State* L) {
    PlayerStats playerstats = luascript_read_userdata(L, PLAYERSTATS);

    bool ret = playerstats_is_dead(playerstats);

    lua_pushboolean(L, ret);
    return 1;
}




static const luaL_Reg PLAYERSTATS_FUNCTIONS[] = {
    { "add_hit", script_playerstats_add_hit },
    { "add_sustain", script_playerstats_add_sustain },
    { "add_sustain_delayed_hit", script_playerstats_add_sustain_delayed_hit },
    { "add_penality", script_playerstats_add_penality },
    { "add_miss", script_playerstats_add_miss },
    { "reset", script_playerstats_reset },
    { "reset_notes_per_seconds", script_playerstats_reset_notes_per_seconds },
    { "add_extra_health", script_playerstats_add_extra_health },
    { "enable_penality_on_empty_strum", script_playerstats_enable_penality_on_empty_strum },
    { "get_maximum_health", script_playerstats_get_maximum_health },
    { "get_health", script_playerstats_get_health },
    { "get_accuracy", script_playerstats_get_accuracy },
    { "get_last_accuracy", script_playerstats_get_last_accuracy },
    { "get_last_ranking", script_playerstats_get_last_ranking },
    { "get_last_difference", script_playerstats_get_last_difference },
    { "get_combo_streak", script_playerstats_get_combo_streak },
    { "get_highest_combo_streak", script_playerstats_get_highest_combo_streak },
    { "get_combo_breaks", script_playerstats_get_combo_breaks },
    { "get_notes_per_seconds", script_playerstats_get_notes_per_seconds },
    { "get_notes_per_seconds_highest", script_playerstats_get_notes_per_seconds_highest },
    { "get_iterations", script_playerstats_get_iterations },
    { "get_score", script_playerstats_get_score },
    { "get_hits", script_playerstats_get_hits },
    { "get_misses", script_playerstats_get_misses },
    { "get_penalties", script_playerstats_get_penalties },
    { "get_shits", script_playerstats_get_shits },
    { "get_bads", script_playerstats_get_bads },
    { "get_goods", script_playerstats_get_goods },
    { "get_sicks", script_playerstats_get_sicks },
    { "set_health", script_playerstats_set_health },
    { "add_health", script_playerstats_add_health },
    { "raise", script_playerstats_raise },
    { "kill", script_playerstats_kill },
    { "kill_if_negative_health", script_playerstats_kill_if_negative_health },
    { "is_dead", script_playerstats_is_dead },
    { NULL, NULL }
};

int script_playerstats_new(lua_State* L, PlayerStats playerstats) {
    return luascript_userdata_new(L, PLAYERSTATS, playerstats);
}

static int script_playerstats_gc(lua_State* L) {
    return luascript_userdata_gc(L, PLAYERSTATS);
}

static int script_playerstats_tostring(lua_State* L) {
    return luascript_userdata_tostring(L, PLAYERSTATS);
}

void script_playerstats_register(lua_State* L) {
    luascript_register(L, PLAYERSTATS, script_playerstats_gc, script_playerstats_tostring, PLAYERSTATS_FUNCTIONS);
}

