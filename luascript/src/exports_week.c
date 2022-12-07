#include "engine_string.h"
#include "luascript_internal.h"
#include "week.h"



#ifdef JAVASCRIPT

EM_JS_PRFX(void, week_unlockdirective_create, (RoundContext roundcontext, const char* name, bool completed_round, bool completed_week, double value), {
    week_unlockdirective_create(kdmyEngine_obtain(roundcontext), kdmyEngine_ptrToString(name), completed_round, completed_week, value);
});
EM_JS_PRFX(void, week_unlockdirective_remove, (RoundContext roundcontext, const char* name, bool completed_round, bool completed_week), {
    week_unlockdirective_remove(kdmyEngine_obtain(roundcontext), kdmyEngine_ptrToString(name), completed_round, completed_week);
});
EM_JS_PRFX(bool, week_unlockdirective_has, (RoundContext roundcontext, const char* name), {
    return week_unlockdirective_has(kdmyEngine_obtain(roundcontext), kdmyEngine_ptrToString(name));
});
EM_JS_PRFX(double, week_unlockdirective_get, (RoundContext roundcontext, const char* name), {
    let value = week_unlockdirective_get(kdmyEngine_obtain(roundcontext), kdmyEngine_ptrToString(name));
    return value;
});
EM_JS_PRFX(void, week_set_halt, (RoundContext roundcontext, bool halt), {
    week_set_halt(kdmyEngine_obtain(roundcontext), halt);
});
EM_JS_PRFX(void, week_ui_set_visibility, (RoundContext roundcontext, bool visible), {
    week_ui_set_visibility(kdmyEngine_obtain(roundcontext), visible);
});
EM_JS_PRFX(Camera, week_ui_get_camera, (RoundContext roundcontext), {
    const camera = week_ui_get_camera(kdmyEngine_obtain(roundcontext));
    return kdmyEngine_obtain(camera);
});
EM_JS_PRFX(Layout, week_ui_get_layout, (RoundContext roundcontext), {
    const layout = week_ui_get_layout(kdmyEngine_obtain(roundcontext));
    return kdmyEngine_obtain(layout);
});
EM_JS_PRFX(Layout, week_get_stage_layout, (RoundContext roundcontext), {
    const layout = week_get_stage_layout(kdmyEngine_obtain(roundcontext));
    return kdmyEngine_obtain(layout);
});
EM_JS_PRFX(int32_t, week_ui_get_strums_count, (RoundContext roundcontext), {
    return week_ui_get_strums_count(kdmyEngine_obtain(roundcontext));
});
/*EM_JS_PRFX(Strums, week_ui_get_strums, (RoundContext roundcontext, int32_t strums_id), {
    const strums = week_ui_get_strums(kdmyEngine_obtain(roundcontext), strums_id);
    return kdmyEngine_obtain(strums);
});
EM_JS_PRFX(Roundstats, week_ui_get_roundstats, (RoundContext roundcontext), {
    const roundstats = week_ui_get_roundstats(kdmyEngine_obtain(roundcontext));
    return kdmyEngine_obtain(roundstats);
});
EM_JS_PRFX(Rankingcounter, week_ui_get_rankingcounter, (RoundContext roundcontext), {
    const rankingcounter = week_ui_get_rankingcounter(kdmyEngine_obtain(roundcontext));
    return kdmyEngine_obtain(rankingcounter);
});
EM_JS_PRFX(Streakcounter, week_ui_get_streakcounter, (RoundContext roundcontext), {
    const streakcounter = week_ui_get_streakcounter(kdmyEngine_obtain(roundcontext));
    return kdmyEngine_obtain(streakcounter);
});*/
EM_JS_PRFX(TextSprite, week_ui_get_trackinfo, (RoundContext roundcontext), {
    const textsprite = week_ui_get_trackinfo(kdmyEngine_obtain(roundcontext));
    return kdmyEngine_obtain(textsprite);
});
/*EM_JS_PRFX(SongProgressbar, week_ui_get_songprogressbar, (RoundContext roundcontext), {
   const songprogressbar = week_ui_get_songprogressbar(kdmyEngine_obtain(roundcontext));
   return kdmyEngine_obtain(songprogressbar);
});*/
EM_JS_PRFX(void, week_update_bpm, (RoundContext roundcontext, float bpm), {
    week_update_bpm(kdmyEngine_obtain(roundcontext), bpm);
});
EM_JS_PRFX(void, week_update_speed, (RoundContext roundcontext, double speed), {
    week_update_speed(kdmyEngine_obtain(roundcontext), speed);
});
EM_JS_PRFX(Messagebox, week_ui_get_messagebox, (RoundContext roundcontext), {
    const messagebox = week_ui_get_messagebox(kdmyEngine_obtain(roundcontext));
    return kdmyEngine_obtain(messagebox);
});
EM_JS_PRFX(Character, week_get_girlfriend, (RoundContext roundcontext), {
    const character = week_get_girlfriend(kdmyEngine_obtain(roundcontext));
    return kdmyEngine_obtain(character);
});
EM_JS_PRFX(int32_t, week_get_character_count, (RoundContext roundcontext), {
    return week_get_character_count(kdmyEngine_obtain(roundcontext));
});
EM_JS_PRFX(Character, week_get_character, (RoundContext roundcontext, int32_t index), {
    const character = week_get_character(kdmyEngine_obtain(roundcontext), index);
    return kdmyEngine_obtain(character);
});
EM_JS_PRFX(SongPlayer, week_get_songplayer, (RoundContext roundcontext), {
    const songplayer = week_get_songplayer(kdmyEngine_obtain(roundcontext));
    return kdmyEngine_obtain(songplayer);
});
EM_JS_PRFX(void, week_get_current_chart_info, (RoundContext roundcontext, float* bpm, double* speed), {
    const values = {bpm : null, speed : null};
    week_get_current_chart_info(kdmyEngine_obtain(roundcontext), values);

    kdmyEngine_set_float32(bpm, values.bpm);
    kdmyEngine_set_float32(speed, values.speed);
});
EM_JS_PRFX(void, week_get_current_track_info, (RoundContext roundcontext, const char** name, const char** difficult, int32_t* index), {
    const values = {name : null, difficult : null, index : -1};
    week_get_current_track_info(kdmyEngine_obtain(roundcontext), values);

    kdmyEngine_set_uint32(name, kdmyEngine_stringToPtr(values.name));
    kdmyEngine_set_uint32(difficult, kdmyEngine_stringToPtr(values.difficult));
    kdmyEngine_set_int32(index, values.index);
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
EM_JS_PRFX(void, week_enable_credits_on_completed, (RoundContext roundcontext), {week_enable_credits_on_completed(kdmyEngine_obtain(roundcontext))});
EM_JS_PRFX(void, week_end, (RoundContext roundcontext, bool round_or_week, bool loose_or_win), {
    week_end(kdmyEngine_obtain(roundcontext), round_or_week, loose_or_win);
});
EM_JS_PRFX(Dialogue, week_get_dialogue, (RoundContext roundcontext), {
    const dialogue = week_get_dialogue(kdmyEngine_obtain(roundcontext));
    return kdmyEngine_obtain(dialogue);
});
EM_JS_PRFX(void, week_set_ui_shader, (RoundContext roundcontext, PSShader psshader), {
    week_set_ui_shader(kdmyEngine_obtain(roundcontext), kdmyEngine_obtain(psshader));
});

#endif

typedef struct {
    const char* variable;
    const int32_t value;
} luaL_Reg_Integer;



static inline int not_implemented(lua_State* L) {
    printf("lua function not implemented");
    lua_pushnil(L);
    return 1;
}


static int script_week_unlockdirective_create(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    const char* name = luaL_checkstring(L, 1);
    bool completed_round = lua_toboolean(L, 2);
    bool completed_week = lua_toboolean(L, 3);
    double value = luaL_checknumber(L, 4);

    if (string_lengthbytes(name) < 1) {
        return luaL_error(L, "the directive name cannot be empty.");
    }

    week_unlockdirective_create(luascript->context, name, completed_round, completed_week, value);

    return 0;
}

static int script_week_unlockdirective_remove(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    const char* name = luaL_checkstring(L, 1);
    bool completed_round = lua_toboolean(L, 2);
    bool completed_week = lua_toboolean(L, 3);

    if (string_lengthbytes(name) < 1) {
        return luaL_error(L, "the directive name cannot be null or empty.");
    }

    week_unlockdirective_remove(luascript->context, name, completed_round, completed_week);

    return 0;
}

static int script_week_unlockdirective_get(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    const char* name = luaL_optstring(L, 1, NULL);

    if (string_lengthbytes(name) < 1) {
        return luaL_error(L, "the directive name cannot be null or empty.");
    }

    if (week_unlockdirective_has(luascript->context, name)) {
        double value = week_unlockdirective_get(luascript->context, name);
        lua_pushnumber(L, value);
    } else {
        lua_pushnil(L);
    }

    return 1;
}

static int script_week_halt(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    bool should_halt = lua_toboolean(L, 1);

    week_set_halt(luascript->context, should_halt);

    return 0;
}

static int script_week_ui_set_visibility(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);

    bool visible = lua_toboolean(L, 1);

    week_ui_set_visibility(luascript->context, visible);

    return 0;
}

static int script_week_ui_get_layout(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);

    Layout layout = week_ui_get_layout(luascript->context);

    return script_layout_new(L, layout);
}

static int script_week_get_stage_layout(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);

    Layout layout = week_get_stage_layout(luascript->context);

    return script_layout_new(L, layout);
}

static int script_week_ui_get_camera(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);

    Camera camera = week_ui_get_camera(luascript->context);

    return script_camera_new(L, camera);
}

static int script_week_ui_get_strums_count(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);

    int32_t ret = week_ui_get_strums_count(luascript->context);

    lua_pushinteger(L, ret);
    return 1;
}

static int script_week_ui_get_strums(lua_State* L) {
    return not_implemented(L);

    /*Luascript luascript = luascript_get_instance(L);
    int32_t strums_id = (int32_t)luaL_checkinteger(L, 1);

    Strums strums = week_ui_get_strums(luascript->context, strums_id);

    return script_week_strums_new(L, strums);*/
}

static int script_week_ui_get_roundstats(lua_State* L) {
    return not_implemented(L);

    /*Luascript luascript = luascript_get_instance(L);
    Roundstats roundstats = week_ui_get_roundstats(luascript->context);
    return script_week_roundstats_new(L, roundstats);*/
}

static int script_week_ui_get_rankingcounter(lua_State* L) {
    return not_implemented(L);

    /*Luascript luascript = luascript_get_instance(L);
    Rankingcounter rankingcounter = week_ui_get_rankingcounter(luascript->context);
    return script_week_rankingcounter_new(L, rankingcounter);*/
}

static int script_week_ui_get_streakcounter(lua_State* L) {
    return not_implemented(L);

    /*Luascript luascript = luascript_get_instance(L);
    Streakcounter streakcounter = week_ui_get_streakcounter(luascript->context);
    return script_week_streakcounter_new(L, streakcounter);*/
}

static int script_week_ui_get_trackinfo(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);

    TextSprite textsprite = week_ui_get_trackinfo(luascript->context);

    return script_textsprite_new(L, textsprite);
}

static int script_week_ui_get_songprogressbar(lua_State* L) {
    return not_implemented(L);

    /*Luascript luascript = luascript_get_instance(L);
    SongProgressbar songprogressbar = week_ui_get_songprogressbar(luascript->context);
    return script_week_songprogressbar_new(L, songprogressbar);*/
}

static int script_week_set_bpm(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    float bpm = (float)luaL_checknumber(L, 1);

    week_update_bpm(luascript->context, bpm);
    return 0;
}

static int script_week_set_speed(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    double speed = luaL_checknumber(L, 1);

    week_update_speed(luascript->context, speed);
    return 0;
}

static int script_week_ui_get_messagebox(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);

    Messagebox messagebox = week_ui_get_messagebox(luascript->context);

    return script_messagebox_new(L, messagebox);
}

static int script_week_get_girlfriend(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);

    Character character = week_get_girlfriend(luascript->context);

    return script_character_new(L, character);
}

static int script_week_get_character_count(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    int32_t count = week_get_character_count(luascript->context);

    lua_pushinteger(L, count);
    return 1;
}

static int script_week_get_character(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    int32_t index = (int32_t)luaL_checkinteger(L, 1);

    Character character = week_get_character(luascript->context, index);

    return script_character_new(L, character);
}

static int script_week_get_songplayer(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);

    SongPlayer songplayer = week_get_songplayer(luascript->context);

    return script_songplayer_new(L, songplayer);
}

static int script_week_get_current_chart_info(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);

    float bpm;
    double speed;

    week_get_current_chart_info(luascript->context, &bpm, &speed);

    lua_pushnumber(L, bpm);
    lua_pushnumber(L, speed);

    return 2;
}

static int script_week_get_current_track_info(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);

    const char* name;
    const char* difficult;
    int32_t index;

    week_get_current_track_info(luascript->context, &name, &difficult, &index);

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

    bool opponent_or_player = lua_toboolean(L, 1);
    const char* new_name = luaL_optstring(L, 2, NULL);

    week_change_character_camera_name(luascript->context, opponent_or_player, new_name);

    return 0;
}

static int script_week_disable_layout_rollback(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);

    bool disable = lua_toboolean(L, 1);

    week_disable_layout_rollback(luascript->context, disable);

    return 0;
}

static int script_week_override_common_folder(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);

    const char* custom_common_path = luaL_optstring(L, 1, NULL);

    week_override_common_folder(luascript->context, custom_common_path);

    return 0;
}

static int script_week_enable_credits_on_completed(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);

    week_enable_credits_on_completed(luascript->context);

    return 0;
}

static int script_week_end(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);

    bool round_or_week = lua_toboolean(L, 1);
    bool loose_or_win = lua_toboolean(L, 2);

    week_end(luascript->context, round_or_week, loose_or_win);

    return 0;
}

static int script_week_get_dialogue(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);

    Dialogue dialogue = week_get_dialogue(luascript->context);

    return script_dialogue_new(L, dialogue);
}

static int script_week_set_ui_shader(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);

    PSShader psshader = luascript_read_nullable_userdata(L, 1, PSSHADER);

    week_set_ui_shader(luascript->context, psshader);

    return 0;
}


static const luaL_Reg EXPORTS_FUNCTION[] = {
    {"unlockdirective_create", script_week_unlockdirective_create},
    {"unlockdirective_remove", script_week_unlockdirective_remove},
    {"unlockdirective_get", script_week_unlockdirective_get},
    {"week_set_halt", script_week_halt},
    {"ui_set_visibility", script_week_ui_set_visibility},
    {"ui_get_layout", script_week_ui_get_layout},
    {"week_get_stage_layout", script_week_get_stage_layout},
    {"ui_get_camera", script_week_ui_get_camera},
    {"ui_get_strums_count", script_week_ui_get_strums_count},
    {"ui_get_strums", script_week_ui_get_strums},
    {"ui_get_roundstats", script_week_ui_get_roundstats},
    {"ui_get_rankingcounter", script_week_ui_get_rankingcounter},
    {"ui_get_streakcounter", script_week_ui_get_streakcounter},
    {"ui_get_trackinfo", script_week_ui_get_trackinfo},
    {"ui_get_songprogressbar", script_week_ui_get_songprogressbar},
    {"week_set_bpm", script_week_set_bpm},
    {"week_set_speed", script_week_set_speed},
    {"ui_get_messagebox", script_week_ui_get_messagebox},
    {"week_get_girlfriend", script_week_get_girlfriend},
    {"week_get_character_count", script_week_get_character_count},
    {"week_get_character", script_week_get_character},
    {"week_get_songplayer", script_week_get_songplayer},
    {"week_get_current_chart_info", script_week_get_current_chart_info},
    {"week_get_current_track_info", script_week_get_current_track_info},
    {"week_change_character_camera_name", script_week_change_character_camera_name},
    {"week_disable_layout_rollback", script_week_disable_layout_rollback},
    {"week_override_common_folder", script_week_override_common_folder},
    {"week_enable_credits_on_completed", script_week_enable_credits_on_completed},
    {"week_end", script_week_end},
    {"week_get_dialogue", script_week_get_dialogue},
    {"week_set_ui_shader", script_week_set_ui_shader},
    {NULL, NULL}
};

static const luaL_Reg_Integer EXPORTS_GLOBAL[] = {
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
        if (EXPORTS_FUNCTION[i].name == NULL || EXPORTS_FUNCTION[i].func == NULL) break;
        lua_pushcfunction(L, EXPORTS_FUNCTION[i].func);
        lua_setglobal(L, EXPORTS_FUNCTION[i].name);
    }

    for (size_t i = 0;; i++) {
        if (EXPORTS_GLOBAL[i].variable == NULL) break;
        lua_pushinteger(L, EXPORTS_GLOBAL[i].value);
        lua_setglobal(L, EXPORTS_GLOBAL[i].variable);
    }
}
