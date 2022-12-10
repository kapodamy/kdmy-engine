#include "luascript_internal.h"

typedef struct {
    const char* variable;
    const int32_t value;
} luaL_Reg_Integer;

#ifdef JAVASCRIPT
EM_JS_PRFX(void, week_unlockdirective_create, (RoundContext roundcontext, const char* name, bool completed_round, bool completed_week, double value), {
    week_unlockdirective_create(kdmyEngine_obtain(roundcontext), kdmyEngine_ptrToString(name), completed_round, completed_week, value);
});
EM_JS_PRFX(bool, week_unlockdirective_has, (RoundContext roundcontext, const char* name), {
    let ret = week_unlockdirective_has(kdmyEngine_obtain(roundcontext), kdmyEngine_ptrToString(name));
    return ret ? 1 : 0;
});
EM_JS_PRFX(void, week_unlockdirective_remove, (RoundContext roundcontext, const char* name, bool completed_round, bool completed_week), {
    week_unlockdirective_remove(kdmyEngine_obtain(roundcontext), kdmyEngine_ptrToString(name), completed_round, completed_week);
});
EM_JS_PRFX(double, week_unlockdirective_get, (RoundContext roundcontext, const char* name), {
    let ret = week_unlockdirective_get(kdmyEngine_obtain(roundcontext), kdmyEngine_ptrToString(name));
    return ret;
});
EM_JS_PRFX(void, week_ui_set_visibility, (RoundContext roundcontext, bool visible), {
    week_ui_set_visibility(kdmyEngine_obtain(roundcontext), visible);
});
EM_JS_PRFX(Layout, week_ui_get_layout, (RoundContext roundcontext), {
    let ret = week_ui_get_layout(kdmyEngine_obtain(roundcontext));
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(Camera, week_ui_get_camera, (RoundContext roundcontext), {
    let ret = week_ui_get_camera(kdmyEngine_obtain(roundcontext));
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(void, week_set_halt, (RoundContext roundcontext, bool halt), {
    week_set_halt(kdmyEngine_obtain(roundcontext), halt);
});
EM_JS_PRFX(int32_t, week_ui_get_strums_count, (RoundContext roundcontext), {
    let ret = week_ui_get_strums_count(kdmyEngine_obtain(roundcontext));
    return ret;
});
EM_JS_PRFX(Strums, week_ui_get_strums, (RoundContext roundcontext, int32_t strums_id), {
    let ret = week_ui_get_strums(kdmyEngine_obtain(roundcontext), strums_id);
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(RoundStats, week_ui_get_roundstats, (RoundContext roundcontext), {
    let ret = week_ui_get_roundstats(kdmyEngine_obtain(roundcontext));
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(RankingCounter, week_ui_get_rankingcounter, (RoundContext roundcontext), {
    let ret = week_ui_get_rankingcounter(kdmyEngine_obtain(roundcontext));
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(StreakCounter, week_ui_get_streakcounter, (RoundContext roundcontext), {
    let ret = week_ui_get_streakcounter(kdmyEngine_obtain(roundcontext));
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(TextSprite, week_ui_get_trackinfo, (RoundContext roundcontext), {
    let ret = week_ui_get_trackinfo(kdmyEngine_obtain(roundcontext));
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(SongProgressbar, week_ui_get_songprogressbar, (RoundContext roundcontext), {
    let ret = week_ui_get_songprogressbar(kdmyEngine_obtain(roundcontext));
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(Countdown, week_ui_get_countdown, (RoundContext roundcontext), {
    let ret = week_ui_get_countdown(kdmyEngine_obtain(roundcontext));
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(HealthBar, week_ui_get_healthbar, (RoundContext roundcontext), {
    let ret = week_ui_get_healthbar(kdmyEngine_obtain(roundcontext));
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(Layout, week_get_stage_layout, (RoundContext roundcontext), {
    let ret = week_get_stage_layout(kdmyEngine_obtain(roundcontext));
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(HealthWatcher, week_get_healthwatcher, (RoundContext roundcontext), {
    let ret = week_get_healthwatcher(kdmyEngine_obtain(roundcontext));
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(MissNoteFX, week_get_missnotefx, (RoundContext roundcontext), {
    let ret = week_get_missnotefx(kdmyEngine_obtain(roundcontext));
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(void, week_update_bpm, (RoundContext roundcontext, float bpm), {
    week_update_bpm(kdmyEngine_obtain(roundcontext), bpm);
});
EM_JS_PRFX(void, week_update_speed, (RoundContext roundcontext, float speed), {
    week_update_speed(kdmyEngine_obtain(roundcontext), speed);
});
EM_JS_PRFX(MessageBox, week_get_messagebox, (RoundContext roundcontext), {
    let ret = week_get_messagebox(kdmyEngine_obtain(roundcontext));
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(Character, week_get_girlfriend, (RoundContext roundcontext), {
    let ret = week_get_girlfriend(kdmyEngine_obtain(roundcontext));
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(int32_t, week_get_character_count, (RoundContext roundcontext), {
    let ret = week_get_character_count(kdmyEngine_obtain(roundcontext));
    return ret;
});
EM_JS_PRFX(Conductor, week_get_conductor, (RoundContext roundcontext, int32_t character_index), {
    let ret = week_get_conductor(kdmyEngine_obtain(roundcontext), character_index);
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(Character, week_get_character, (RoundContext roundcontext, int32_t character_index), {
    let ret = week_get_character(kdmyEngine_obtain(roundcontext), character_index);
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(PlayerStats, week_get_playerstats, (RoundContext roundcontext, int32_t character_index), {
    let ret = week_get_playerstats(kdmyEngine_obtain(roundcontext), character_index);
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(SongPlayer, week_get_songplayer, (RoundContext roundcontext), {
    let ret = week_get_songplayer(kdmyEngine_obtain(roundcontext));
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(void, week_get_current_chart_info, (RoundContext roundcontext, float* bpm, double* speed), {
    const values = {
        bpm: null,
        speed: null
    };
    week_get_current_chart_info(kdmyEngine_obtain(roundcontext), values);
    kdmyEngine_set_float32(bpm, values.bpm);
    kdmyEngine_set_float64(speed, values.speed)
});
EM_JS_PRFX(void, week_get_current_track_info, (RoundContext roundcontext, const char** name, const char** difficult, int32_t* index), {
    const values = {
        name: null,
        difficult: null,
        index: -1
    };
    week_get_current_track_info(kdmyEngine_obtain(roundcontext), values);
    kdmyEngine_set_uint32(name, kdmyEngine_stringToPtr(values.name));
    kdmyEngine_set_uint32(difficult, kdmyEngine_stringToPtr(values.difficult));
    kdmyEngine_set_int32(index, values.index)
});
EM_JS_PRFX(void, week_change_character_camera_name, (RoundContext roundcontext, bool opponent_or_player, const char* new_name), {
    week_change_character_camera_name(kdmyEngine_obtain(roundcontext), opponent_or_player, kdmyEngine_ptrToString(new_name));
});
EM_JS_PRFX(void, week_disable_layout_rollback, (RoundContext roundcontext, bool disable), {
    week_disable_layout_rollback(kdmyEngine_obtain(roundcontext), disable);
});
EM_JS_PRFX(void, week_override_common_folder, (RoundContext roundcontext, const char* custom_common_path), {
    week_override_common_folder(kdmyEngine_obtain(roundcontext), kdmyEngine_ptrToString(custom_common_path));
});
EM_JS_PRFX(void, week_enable_credits_on_completed, (RoundContext roundcontext), {
    week_enable_credits_on_completed(kdmyEngine_obtain(roundcontext));
});
EM_JS_PRFX(void, week_end, (RoundContext roundcontext, bool round_or_week, bool loose_or_win), {
    week_end(kdmyEngine_obtain(roundcontext), round_or_week, loose_or_win);
});
EM_JS_PRFX(void, week_get_dialogue, (RoundContext roundcontext), {
    week_get_dialogue(kdmyEngine_obtain(roundcontext));
});
EM_JS_PRFX(void, week_set_ui_shader, (RoundContext roundcontext, PSShader psshader), {
    week_set_ui_shader(kdmyEngine_obtain(roundcontext), kdmyEngine_obtain(psshader));
});
EM_ASYNC_JS_PRFX(void, week_rebuild_ui, (RoundContext roundcontext), {
    await week_rebuild_ui(kdmyEngine_obtain(roundcontext));
});
#endif


static int script_week_unlockdirective_create(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;
    
    const char* name = luaL_optstring(L, 1, NULL);
    bool completed_round = (bool)lua_toboolean(L, 2);
    bool completed_week = (bool)lua_toboolean(L, 3);
    double value = (double)luaL_checknumber(L, 4);

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

    if(!week_unlockdirective_has(roundcontext, name)) {
        lua_pushnil(L);
        return 1;
    }

    double ret = week_unlockdirective_get(roundcontext, name);

    lua_pushnumber(L, ret);
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

static int script_week_ui_get_trackinfo(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;


    TextSprite ret = week_ui_get_trackinfo(roundcontext);

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


    HealthBar ret = week_ui_get_healthbar(roundcontext);

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
    double speed;
    week_get_current_chart_info(roundcontext, &bpm, &speed);

    lua_pushnumber(L, bpm);
    lua_pushnumber(L, speed);
    return 2;
}

static int script_week_get_current_track_info(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;
    
    const char* name;
    const char* difficult;
    int32_t index;
    week_get_current_track_info(roundcontext, &name, &difficult, &index);

    lua_pushstring(L, name);
    lua_pushstring(L, difficult);
    lua_pushinteger(L, index);

#ifdef JAVASCRIPT
    free((char*)name);
    free((char*)difficult);
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

    week_override_common_folder(roundcontext, custom_common_path);

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


    week_get_dialogue(roundcontext);

    return 0;
}

static int script_week_set_ui_shader(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;
    
    PSShader psshader = luascript_read_nullable_userdata(L, 1, PSSHADER);

    week_set_ui_shader(roundcontext, psshader);

    return 0;
}

static int script_week_rebuild_ui(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    RoundContext roundcontext = (RoundContext)luascript->context;


    week_rebuild_ui(roundcontext);

    return 0;
}



static const luaL_Reg WEEK_FUNCTIONS[] = {
    { "week_ui_set_visibility", script_week_ui_set_visibility },
    { "week_ui_get_layout", script_week_ui_get_layout },
    { "week_ui_get_camera", script_week_ui_get_camera },
    { "week_set_halt", script_week_set_halt },
    { "week_ui_get_strums_count", script_week_ui_get_strums_count },
    { "week_ui_get_strums", script_week_ui_get_strums },
    { "week_ui_get_roundstats", script_week_ui_get_roundstats },
    { "week_ui_get_rankingcounter", script_week_ui_get_rankingcounter },
    { "week_ui_get_streakcounter", script_week_ui_get_streakcounter },
    { "week_ui_get_trackinfo", script_week_ui_get_trackinfo },
    { "week_ui_get_songprogressbar", script_week_ui_get_songprogressbar },
    { "week_ui_get_countdown", script_week_ui_get_countdown },
    { "week_ui_get_healthbar", script_week_ui_get_healthbar },
    { "week_get_stage_layout", script_week_get_stage_layout },
    { "week_get_healthwatcher", script_week_get_healthwatcher },
    { "week_get_missnotefx", script_week_get_missnotefx },
    { "week_update_bpm", script_week_update_bpm },
    { "week_update_speed", script_week_update_speed },
    { "week_get_messagebox", script_week_get_messagebox },
    { "week_get_girlfriend", script_week_get_girlfriend },
    { "week_get_character_count", script_week_get_character_count },
    { "week_get_conductor", script_week_get_conductor },
    { "week_get_character", script_week_get_character },
    { "week_get_playerstats", script_week_get_playerstats },
    { "week_get_songplayer", script_week_get_songplayer },
    { "week_get_current_chart_info", script_week_get_current_chart_info },
    { "week_get_current_track_info", script_week_get_current_track_info },
    { "week_change_character_camera_name", script_week_change_character_camera_name },
    { "week_disable_layout_rollback", script_week_disable_layout_rollback },
    { "week_override_common_folder", script_week_override_common_folder },
    { "week_enable_credits_on_completed", script_week_enable_credits_on_completed },
    { "week_end", script_week_end },
    { "week_get_dialogue", script_week_get_dialogue },
    { "week_set_ui_shader", script_week_set_ui_shader },
    { "week_rebuild_ui", script_week_rebuild_ui },
    { "week_unlockdirective_create", script_week_unlockdirective_create },
    { "week_unlockdirective_get", script_week_unlockdirective_get },
    { "week_unlockdirective_remove", script_week_unlockdirective_remove },
    { NULL, NULL }
};

static const luaL_Reg_Integer WEEK_GLOBALS[] = {
    {"NOTE_MISS", 0},
    {"NOTE_PENALITY", 1},
    {"NOTE_SHIT", 2},
    {"NOTE_BAD", 3},
    {"NOTE_GOOD", 4},
    {"NOTE_SICK", 5},
    {NULL, 0}
};


void script_week_register(lua_State* L) {
    for (size_t i = 0;; i++) {
        if (WEEK_FUNCTIONS[i].name == NULL || WEEK_FUNCTIONS[i].func == NULL) break;
        lua_pushcfunction(L, WEEK_FUNCTIONS[i].func);
        lua_setglobal(L, WEEK_FUNCTIONS[i].name);
    }

    for (size_t i = 0;; i++) {
        if (WEEK_GLOBALS[i].variable == NULL) break;
        lua_pushinteger(L, WEEK_GLOBALS[i].value);
        lua_setglobal(L, WEEK_GLOBALS[i].variable);
    }
}
