#include "luascript_internal.h"

#include <string.h>

#include "game/gameplay/week.h"
#include "game/gameplay/week_gameover.h"
#include "game/gameplay/week_result.h"
#include "game/gameplay/week_types.h"
#include "math2d.h"


static int script_week_unlockdirective_create(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;

    const char* name = luaL_optstring(L, 1, NULL);
    bool completed_round = (bool)lua_toboolean(L, 2);
    bool completed_week = (bool)lua_toboolean(L, 3);
    float64 value = luaL_checknumber(L, 4);

    week_unlockdirective_create(roundcontext, name, completed_round, completed_week, value);

    return 0;
}

static int script_week_unlockdirective_remove(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;

    const char* name = luaL_optstring(L, 1, NULL);
    bool completed_round = (bool)lua_toboolean(L, 2);
    bool completed_week = (bool)lua_toboolean(L, 3);

    week_unlockdirective_remove(roundcontext, name, completed_round, completed_week);

    return 0;
}

static int script_week_unlockdirective_get(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;

    const char* name = luaL_optstring(L, 1, NULL);

    if (!week_unlockdirective_has(roundcontext, name)) {
        lua_pushnil(L);
        return 1;
    }

    float64 ret = week_unlockdirective_get(roundcontext, name);

    lua_pushnumber(L, (lua_Number)ret);
    return 1;
}

static int script_week_ui_set_visibility(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;

    bool visible = (bool)lua_toboolean(L, 1);

    week_ui_set_visibility(roundcontext, visible);

    return 0;
}

static int script_week_ui_get_layout(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;


    Layout ret = week_ui_get_layout(roundcontext);

    return script_layout_new(L, ret);
}

static int script_week_ui_get_camera(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;


    Camera ret = week_ui_get_camera(roundcontext);

    return script_camera_new(L, ret);
}

static int script_week_set_halt(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;

    bool halt = (bool)lua_toboolean(L, 1);

    week_set_halt(roundcontext, halt);

    return 0;
}

static int script_week_disable_week_end_results(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;

    bool disable = (bool)lua_toboolean(L, 1);

    week_disable_week_end_results(roundcontext, disable);

    return 0;
}

static int script_week_disable_girlfriend_cry(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;

    bool disable = (bool)lua_toboolean(L, 1);

    week_disable_girlfriend_cry(roundcontext, disable);

    return 0;
}

static int script_week_disable_ask_ready(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;

    bool disable = (bool)lua_toboolean(L, 1);

    week_disable_ask_ready(roundcontext, disable);

    return 0;
}

static int script_week_disable_countdown(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;

    bool disable = (bool)lua_toboolean(L, 1);

    week_disable_countdown(roundcontext, disable);

    return 0;
}

static int script_week_disable_camera_bumping(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;

    bool disable = (bool)lua_toboolean(L, 1);

    week_disable_camera_bumping(roundcontext, disable);

    return 0;
}

static int script_week_ui_get_strums_count(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;


    int32_t ret = week_ui_get_strums_count(roundcontext);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_week_ui_get_strums(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;

    int32_t strums_id = (int32_t)luaL_checkinteger(L, 1);

    Strums ret = week_ui_get_strums(roundcontext, strums_id);

    return script_strums_new(L, ret);
}

static int script_week_ui_get_roundstats(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;


    RoundStats ret = week_ui_get_roundstats(roundcontext);

    return script_roundstats_new(L, ret);
}

static int script_week_ui_get_rankingcounter(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;


    RankingCounter ret = week_ui_get_rankingcounter(roundcontext);

    return script_rankingcounter_new(L, ret);
}

static int script_week_ui_get_streakcounter(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;


    StreakCounter ret = week_ui_get_streakcounter(roundcontext);

    return script_streakcounter_new(L, ret);
}

static int script_week_ui_get_round_textsprite(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;


    TextSprite ret = week_ui_get_round_textsprite(roundcontext);

    return script_textsprite_new(L, ret);
}

static int script_week_ui_get_songprogressbar(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;


    SongProgressbar ret = week_ui_get_songprogressbar(roundcontext);

    return script_songprogressbar_new(L, ret);
}

static int script_week_ui_get_countdown(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;


    Countdown ret = week_ui_get_countdown(roundcontext);

    return script_countdown_new(L, ret);
}

static int script_week_ui_get_healthbar(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;


    Healthbar ret = week_ui_get_healthbar(roundcontext);

    return script_healthbar_new(L, ret);
}

static int script_week_get_stage_layout(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;


    Layout ret = week_get_stage_layout(roundcontext);

    return script_layout_new(L, ret);
}

static int script_week_get_healthwatcher(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;


    HealthWatcher ret = week_get_healthwatcher(roundcontext);

    return script_healthwatcher_new(L, ret);
}

static int script_week_get_missnotefx(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;


    MissNoteFX ret = week_get_missnotefx(roundcontext);

    return script_missnotefx_new(L, ret);
}

static int script_week_update_bpm(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;

    float bpm = (float)luaL_checknumber(L, 1);

    week_update_bpm(roundcontext, bpm);

    return 0;
}

static int script_week_update_speed(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;

    float speed = (float)luaL_checknumber(L, 1);

    week_update_speed(roundcontext, speed);

    return 0;
}

static int script_week_get_messagebox(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;


    MessageBox ret = week_get_messagebox(roundcontext);

    return script_messagebox_new(L, ret);
}

static int script_week_get_girlfriend(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;


    Character ret = week_get_girlfriend(roundcontext);

    return script_character_new(L, ret);
}

static int script_week_get_character_count(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;


    int32_t ret = week_get_character_count(roundcontext);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_week_get_conductor(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;

    int32_t character_index = (int32_t)luaL_checkinteger(L, 1);

    Conductor ret = week_get_conductor(roundcontext, character_index);

    return script_conductor_new(L, ret);
}

static int script_week_get_character(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;

    int32_t character_index = (int32_t)luaL_checkinteger(L, 1);

    Character ret = week_get_character(roundcontext, character_index);

    return script_character_new(L, ret);
}

static int script_week_get_playerstats(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;

    int32_t character_index = (int32_t)luaL_checkinteger(L, 1);

    PlayerStats ret = week_get_playerstats(roundcontext, character_index);

    return script_playerstats_new(L, ret);
}

static int script_week_get_songplayer(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;


    SongPlayer ret = week_get_songplayer(roundcontext);

    return script_songplayer_new(L, ret);
}

static int script_week_get_current_chart_info(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;

    float bpm;
    float64 speed;
    week_get_current_chart_info(roundcontext, &bpm, &speed);

    lua_pushnumber(L, (lua_Number)bpm);
    lua_pushnumber(L, (lua_Number)speed);
    return 2;
}

static int script_week_get_current_song_info(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;

    const char* name;
    const char* difficult;
    int32_t index;
    week_get_current_song_info(roundcontext, &name, &difficult, &index);

    lua_pushstring(L, name);
    lua_pushstring(L, difficult);
    lua_pushinteger(L, (lua_Integer)index);

#ifdef JAVASCRIPT
    free_chk((char*)name);
    free_chk((char*)difficult);
#endif

    return 3;
}

static int script_week_change_character_camera_name(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;

    bool opponent_or_player = (bool)lua_toboolean(L, 1);
    const char* new_name = luaL_optstring(L, 2, NULL);

    week_change_character_camera_name(roundcontext, opponent_or_player, new_name);

    return 0;
}

static int script_week_disable_layout_rollback(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;

    bool disable = (bool)lua_toboolean(L, 1);

    week_disable_layout_rollback(roundcontext, disable);

    return 0;
}

static int script_week_override_common_folder(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;

    const char* custom_common_path = luaL_optstring(L, 1, NULL);

    luascript_change_working_folder(L);
    week_override_common_folder(roundcontext, custom_common_path);
    luascript_restore_working_folder(L);

    return 0;
}

static int script_week_enable_credits_on_completed(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;


    week_enable_credits_on_completed(roundcontext);

    return 0;
}

static int script_week_end(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;

    bool round_or_week = (bool)lua_toboolean(L, 1);
    bool loose_or_win = (bool)lua_toboolean(L, 2);

    week_end(roundcontext, round_or_week, loose_or_win);

    return 0;
}

static int script_week_get_dialogue(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;


    Dialogue dialogue = week_get_dialogue(roundcontext);

    return script_dialogue_new(L, dialogue);
}

#ifndef _arch_dreamcast
static int script_week_set_ui_shader(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;

    PSShader psshader = luascript_read_nullable_userdata(L, 1, PSSHADER);

    week_set_ui_shader(roundcontext, psshader);

    return 0;
}
#endif

static int script_week_rebuild_ui(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;


    week_rebuild_ui(roundcontext);

    return 0;
}

static int script_week_storage_get(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;

    const char* name = luaL_optstring(L, 1, NULL);
    uint8_t* data;

    size_t ret = week_storage_get(roundcontext, name, &data);

    if (!data)
        lua_pushnil(L);
    else
        lua_pushlstring(L, (const char*)data, ret);

    return 1;
}

static int script_week_storage_set(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;

    const char* name = luaL_optstring(L, 1, NULL);
    const uint8_t* data;
    size_t data_size;

    if (lua_isnil(L, 2)) {
        data = NULL;
        data_size = 0;
    } else {
        data = (const uint8_t*)luaL_checklstring(L, 3, &data_size);
    }

    bool ret = week_storage_set(roundcontext, name, data, data_size);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_week_gameover_no_music(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;

    week_set_gameover_option(roundcontext, WEEK_GAMEOVER_NOMUSIC, FLOAT_NaN, NULL);

    return 0;
}

static int script_week_gameover_no_sfx_die(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;

    week_set_gameover_option(roundcontext, WEEK_GAMEOVER_NOSFXDIE, FLOAT_NaN, NULL);

    return 0;
}

static int script_week_gameover_no_sfx_retry(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;

    week_set_gameover_option(roundcontext, WEEK_GAMEOVER_NOSFXRETRY, FLOAT_NaN, NULL);

    return 0;
}

static int script_week_gameover_set_die_anim_duration(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;

    float duration_ms = (float)luaL_checknumber(L, 1);

    week_set_gameover_option(roundcontext, WEEK_GAMEOVER_ANIMDURATIONDIE, duration_ms, NULL);

    return 0;
}

static int script_week_gameover_set_retry_anim_duration(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;

    float duration_ms = (float)luaL_checknumber(L, 1);

    week_set_gameover_option(roundcontext, WEEK_GAMEOVER_ANIMDURATIONRETRY, duration_ms, NULL);

    return 0;
}

static int script_week_gameover_set_giveup_anim_duration(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;

    float duration_ms = (float)luaL_checknumber(L, 1);

    week_set_gameover_option(roundcontext, WEEK_GAMEOVER_ANIMDURATIONGIVEUP, duration_ms, NULL);

    return 0;
}

static int script_week_gameover_set_before_anim_duration(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;

    float duration_ms = (float)luaL_checknumber(L, 1);

    week_set_gameover_option(roundcontext, WEEK_GAMEOVER_ANIMDURATIONBEFORE, duration_ms, NULL);

    return 0;
}

static int script_week_gameover_set_before_force_end_anim_duration(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;

    float duration_ms = (float)luaL_checknumber(L, 1);

    week_set_gameover_option(roundcontext, WEEK_GAMEOVER_ANIMDURATIONBEFOREFORCEEND, duration_ms, NULL);

    return 0;
}

static int script_week_gameover_set_music(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;

    const char* filename = luaL_optstring(L, 1, NULL);

    week_set_gameover_option(roundcontext, WEEK_GAMEOVER_SETMUSIC, FLOAT_NaN, filename);

    return 0;
}

static int script_week_gameover_set_sfx_die(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;

    const char* filename = luaL_optstring(L, 1, NULL);

    week_set_gameover_option(roundcontext, WEEK_GAMEOVER_SETSFXDIE, FLOAT_NaN, filename);

    return 0;
}

static int script_week_gameover_set_sfx_confirm(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;

    const char* filename = luaL_optstring(L, 1, NULL);

    week_set_gameover_option(roundcontext, WEEK_GAMEOVER_SETSFXRETRY, FLOAT_NaN, filename);

    return 0;
}

static int script_week_get_accumulated_stats(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;

    WeekResult_Stats stats;
    memset(&stats, 0x00, sizeof(WeekResult_Stats));

    week_get_accumulated_stats(roundcontext, &stats);

    lua_createtable(L, 0, 11);
    luascript_helper_add_table_field(L, "sick", lua_pushinteger, (lua_Integer)stats.sick);
    luascript_helper_add_table_field(L, "good", lua_pushinteger, (lua_Integer)stats.good);
    luascript_helper_add_table_field(L, "bads", lua_pushinteger, (lua_Integer)stats.bads);
    luascript_helper_add_table_field(L, "shits", lua_pushinteger, (lua_Integer)stats.shits);
    luascript_helper_add_table_field(L, "miss", lua_pushinteger, (lua_Integer)stats.miss);
    luascript_helper_add_table_field(L, "penalties", lua_pushinteger, (lua_Integer)stats.penalties);
    luascript_helper_add_table_field(L, "score", lua_pushinteger, (lua_Integer)stats.score);
    luascript_helper_add_table_field(L, "accuracy", lua_pushnumber, (lua_Number)stats.accuracy);
    luascript_helper_add_table_field(L, "notesPerSeconds", lua_pushinteger, (lua_Integer)stats.notesperseconds);
    luascript_helper_add_table_field(L, "comboBreaks", lua_pushinteger, (lua_Integer)stats.combobreaks);
    luascript_helper_add_table_field(L, "highestStreak", lua_pushinteger, (lua_Integer)stats.higheststreak);

    return 1;
}

static int script_week_get_gameover_layout(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;

    Layout ret = week_get_layout_of(roundcontext, 'g');

    return script_layout_new(L, ret);
}

static int script_week_get_pause_layout(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;

    Layout ret = week_get_layout_of(roundcontext, 'p');

    return script_layout_new(L, ret);
}

static int script_week_get_results_layout(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;

    Layout ret = week_get_layout_of(roundcontext, 'r');

    return script_layout_new(L, ret);
}

static int script_week_set_pause_background_music(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;

    const char* filename = luaL_optstring(L, 1, NULL);

    week_set_pause_background_music(roundcontext, filename);

    return 0;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const luaL_Reg WEEK_FUNCTIONS[] = {
    {"week_ui_set_visibility", script_week_ui_set_visibility},
    {"week_ui_get_layout", script_week_ui_get_layout},
    {"week_ui_get_camera", script_week_ui_get_camera},
    {"week_set_halt", script_week_set_halt},
    {"week_disable_week_end_results", script_week_disable_week_end_results},
    {"week_disable_girlfriend_cry", script_week_disable_girlfriend_cry},
    {"week_disable_ask_ready", script_week_disable_ask_ready},
    {"week_disable_countdown", script_week_disable_countdown},
    {"week_disable_camera_bumping", script_week_disable_camera_bumping},
    {"week_ui_get_strums_count", script_week_ui_get_strums_count},
    {"week_ui_get_strums", script_week_ui_get_strums},
    {"week_ui_get_roundstats", script_week_ui_get_roundstats},
    {"week_ui_get_rankingcounter", script_week_ui_get_rankingcounter},
    {"week_ui_get_streakcounter", script_week_ui_get_streakcounter},
    {"week_ui_get_round_textsprite", script_week_ui_get_round_textsprite},
    {"week_ui_get_songprogressbar", script_week_ui_get_songprogressbar},
    {"week_ui_get_countdown", script_week_ui_get_countdown},
    {"week_ui_get_healthbar", script_week_ui_get_healthbar},
    {"week_get_stage_layout", script_week_get_stage_layout},
    {"week_get_healthwatcher", script_week_get_healthwatcher},
    {"week_get_missnotefx", script_week_get_missnotefx},
    {"week_update_bpm", script_week_update_bpm},
    {"week_update_speed", script_week_update_speed},
    {"week_get_messagebox", script_week_get_messagebox},
    {"week_get_girlfriend", script_week_get_girlfriend},
    {"week_get_character_count", script_week_get_character_count},
    {"week_get_conductor", script_week_get_conductor},
    {"week_get_character", script_week_get_character},
    {"week_get_playerstats", script_week_get_playerstats},
    {"week_get_songplayer", script_week_get_songplayer},
    {"week_get_current_chart_info", script_week_get_current_chart_info},
    {"week_get_current_song_info", script_week_get_current_song_info},
    {"week_change_character_camera_name", script_week_change_character_camera_name},
    {"week_disable_layout_rollback", script_week_disable_layout_rollback},
    {"week_override_common_folder", script_week_override_common_folder},
    {"week_enable_credits_on_completed", script_week_enable_credits_on_completed},
    {"week_end", script_week_end},
    {"week_get_dialogue", script_week_get_dialogue},
#ifndef _arch_dreamcast
    {"week_set_ui_shader", script_week_set_ui_shader},
#endif
    {"week_rebuild_ui", script_week_rebuild_ui},
    {"week_unlockdirective_create", script_week_unlockdirective_create},
    {"week_unlockdirective_get", script_week_unlockdirective_get},
    {"week_unlockdirective_remove", script_week_unlockdirective_remove},
    {"week_storage_set", script_week_storage_set},
    {"week_storage_get", script_week_storage_get},
    {"week_gameover_no_music", script_week_gameover_no_music},
    {"week_gameover_no_sfx_die", script_week_gameover_no_sfx_die},
    {"week_gameover_no_sfx_retry", script_week_gameover_no_sfx_retry},
    {"week_gameover_set_die_anim_duration", script_week_gameover_set_die_anim_duration},
    {"week_gameover_set_retry_anim_duration", script_week_gameover_set_retry_anim_duration},
    {"week_gameover_set_giveup_anim_duration", script_week_gameover_set_giveup_anim_duration},
    {"week_gameover_set_music", script_week_gameover_set_music},
    {"week_gameover_set_sfx_die", script_week_gameover_set_sfx_die},
    {"week_gameover_set_sfx_confirm", script_week_gameover_set_sfx_confirm},
    {"week_gameover_set_before_anim_duration", script_week_gameover_set_before_anim_duration},
    {"week_gameover_set_before_force_end_anim_duration", script_week_gameover_set_before_force_end_anim_duration},
    {"week_get_accumulated_stats", script_week_get_accumulated_stats},
    {"week_get_gameover_layout", script_week_get_gameover_layout},
    {"week_get_pause_layout", script_week_get_pause_layout},
    {"week_get_results_layout", script_week_get_results_layout},
    {"week_set_pause_background_music", script_week_set_pause_background_music},
    {NULL, NULL}
};

void script_week_register(lua_State* L) {
    for (size_t i = 0;; i++) {
        if (WEEK_FUNCTIONS[i].name == NULL || WEEK_FUNCTIONS[i].func == NULL) break;
        lua_pushcfunction(L, WEEK_FUNCTIONS[i].func);
        lua_setglobal(L, WEEK_FUNCTIONS[i].name);
    }
}
