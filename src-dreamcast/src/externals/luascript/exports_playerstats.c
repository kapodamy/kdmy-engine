#include "luascript_internal.h"

#include "game/funkin/playerstats.h"


static int script_playerstats_add_hit(lua_State* L) {
    PlayerStats playerstats = luascript_read_userdata(L, PLAYERSTATS);
    float multiplier = (float)luaL_checknumber(L, 2);
    float base_note_duration = (float)luaL_checknumber(L, 3);
    float64 hit_time_difference = luaL_checknumber(L, 4);

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

    lua_pushinteger(L, (lua_Integer)ret);
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

    float64 ret = playerstats_get_maximum_health(playerstats);

    lua_pushnumber(L, (lua_Number)ret);
    return 1;
}

static int script_playerstats_get_health(lua_State* L) {
    PlayerStats playerstats = luascript_read_userdata(L, PLAYERSTATS);

    float64 ret = playerstats_get_health(playerstats);

    lua_pushnumber(L, (lua_Number)ret);
    return 1;
}

static int script_playerstats_get_accuracy(lua_State* L) {
    PlayerStats playerstats = luascript_read_userdata(L, PLAYERSTATS);

    float64 ret = playerstats_get_accuracy(playerstats);

    lua_pushnumber(L, (lua_Number)ret);
    return 1;
}

static int script_playerstats_get_last_accuracy(lua_State* L) {
    PlayerStats playerstats = luascript_read_userdata(L, PLAYERSTATS);

    float64 ret = playerstats_get_last_accuracy(playerstats);

    lua_pushnumber(L, (lua_Number)ret);
    return 1;
}

static int script_playerstats_get_last_ranking(lua_State* L) {
    PlayerStats playerstats = luascript_read_userdata(L, PLAYERSTATS);

    Ranking ret = playerstats_get_last_ranking(playerstats);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_playerstats_get_last_difference(lua_State* L) {
    PlayerStats playerstats = luascript_read_userdata(L, PLAYERSTATS);

    float64 ret = playerstats_get_last_difference(playerstats);

    lua_pushnumber(L, (lua_Number)ret);
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
    float64 health = luaL_checknumber(L, 2);

    playerstats_set_health(playerstats, health);

    return 0;
}

static int script_playerstats_add_health(lua_State* L) {
    PlayerStats playerstats = luascript_read_userdata(L, PLAYERSTATS);
    float64 health = luaL_checknumber(L, 2);
    bool die_if_negative = (bool)lua_toboolean(L, 3);

    float64 ret = playerstats_add_health(playerstats, health, die_if_negative);

    lua_pushnumber(L, (lua_Number)ret);
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


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const luaL_Reg PLAYERSTATS_FUNCTIONS[] = {
    {"add_hit", script_playerstats_add_hit},
    {"add_sustain", script_playerstats_add_sustain},
    {"add_sustain_delayed_hit", script_playerstats_add_sustain_delayed_hit},
    {"add_penality", script_playerstats_add_penality},
    {"add_miss", script_playerstats_add_miss},
    {"reset", script_playerstats_reset},
    {"reset_notes_per_seconds", script_playerstats_reset_notes_per_seconds},
    {"add_extra_health", script_playerstats_add_extra_health},
    {"enable_penality_on_empty_strum", script_playerstats_enable_penality_on_empty_strum},
    {"get_maximum_health", script_playerstats_get_maximum_health},
    {"get_health", script_playerstats_get_health},
    {"get_accuracy", script_playerstats_get_accuracy},
    {"get_last_accuracy", script_playerstats_get_last_accuracy},
    {"get_last_ranking", script_playerstats_get_last_ranking},
    {"get_last_difference", script_playerstats_get_last_difference},
    {"get_combo_streak", script_playerstats_get_combo_streak},
    {"get_highest_combo_streak", script_playerstats_get_highest_combo_streak},
    {"get_combo_breaks", script_playerstats_get_combo_breaks},
    {"get_notes_per_seconds", script_playerstats_get_notes_per_seconds},
    {"get_notes_per_seconds_highest", script_playerstats_get_notes_per_seconds_highest},
    {"get_iterations", script_playerstats_get_iterations},
    {"get_score", script_playerstats_get_score},
    {"get_hits", script_playerstats_get_hits},
    {"get_misses", script_playerstats_get_misses},
    {"get_penalties", script_playerstats_get_penalties},
    {"get_shits", script_playerstats_get_shits},
    {"get_bads", script_playerstats_get_bads},
    {"get_goods", script_playerstats_get_goods},
    {"get_sicks", script_playerstats_get_sicks},
    {"set_health", script_playerstats_set_health},
    {"add_health", script_playerstats_add_health},
    {"raise", script_playerstats_raise},
    {"kill", script_playerstats_kill},
    {"kill_if_negative_health", script_playerstats_kill_if_negative_health},
    {"is_dead", script_playerstats_is_dead},
    {NULL, NULL}
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
