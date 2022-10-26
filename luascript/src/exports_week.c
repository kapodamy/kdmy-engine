#include "luascript_internal.h"
#include "engine_string.h"
#include "week.h"



#ifdef JAVASCRIPT

EM_JS_PRFX(void, week_unlockdirective_create_JS, (RoundContext roundcontext, const char* name, bool completed_round, bool completed_week, double* value_ptr), {
    let dataView = new DataView(buffer);
    const ENDIANESS = true;
    let val = dataView.getFloat64(value_ptr, ENDIANESS);

    week_unlockdirective_create(kdmyEngine_obtain(roundcontext), kdmyEngine_ptrToString(name), completed_round, completed_week, val);
    });
EM_JS_PRFX(void, week_unlockdirective_remove, (RoundContext roundcontext, const char* name, bool completed_round, bool completed_week), {
    week_unlockdirective_remove(kdmyEngine_obtain(roundcontext), kdmyEngine_ptrToString(name), completed_round, completed_week);
    });
EM_JS_PRFX(bool, week_unlockdirective_has, (RoundContext roundcontext, const char* name), {
    return week_unlockdirective_has(kdmyEngine_obtain(roundcontext), kdmyEngine_ptrToString(name));
    });
EM_JS_PRFX(void, week_unlockdirective_get_JS, (RoundContext roundcontext, const char* name, double* value_ptr), {
    let dataView = new DataView(buffer);
    const ENDIANESS = true;

    let value = week_unlockdirective_get(kdmyEngine_obtain(roundcontext), kdmyEngine_ptrToString(name));
    dataView.setFloat64(value_ptr, BigInt(value), ENDIANESS);
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
EM_JS_PRFX(void, week_get_current_chart_info, (RoundContext roundcontext, WeekChartInfo_t* output_info), {
  const values = { bmp: null, speed : null };
  const dataView = new DataView(buffer);
  const HEAP_ENDIANESS = true;

  week_get_current_chart_info(kdmyEngine_obtain(roundcontext), values);

  dataView.setFloat32(output_info + 0, values.bmp, HEAP_ENDIANESS);
  dataView.setFloat64(output_info + 4, values.speed, HEAP_ENDIANESS);
    });
EM_JS_PRFX(void, week_get_current_track_info, (RoundContext roundcontext, WeekTrackInfo_t* output_info), {
    const values = { name: null, difficult : null, index : -1 };
    const dataView = new DataView(buffer);
    const HEAP_ENDIANESS = true;
    const ptrsize = dataView.getUint32(output_info, HEAP_ENDIANESS);

    function setPointer(offset, ptr) {
      if (ptrsize == 4) {
        dataView.setUint32(offset, ptr, HEAP_ENDIANESS);
      }
 else {
dataView.setUint32(offset, ptr, HEAP_ENDIANESS);
dataView.setUint32(offset + 4, 0x00, HEAP_ENDIANESS);
}
};

week_get_current_track_info(kdmyEngine_obtain(roundcontext), values);
const ptr_name = kdmyEngine_stringToPtr(values.name);
const ptr_difficult = kdmyEngine_stringToPtr(values.difficult);

let offset = output_info + 4;// add _ptrsize field size

setPointer(offset, ptr_name); offset += ptrsize;
setPointer(offset, ptr_difficult); offset += ptrsize;
dataView.setInt32(offset, values.index, HEAP_ENDIANESS);
    });
EM_JS_PRFX(void, week_change_charecter_camera_name, (RoundContext roundcontext, bool opponent_or_player, const char* new_name), {
    week_change_charecter_camera_name(kdmyEngine_obtain(roundcontext), opponent_or_player, kdmyEngine_ptrToString(new_name));
    });
EM_JS_PRFX(void, week_disable_layout_rollback, (RoundContext roundcontext, bool disable), {
    week_disable_layout_rollback(kdmyEngine_obtain(roundcontext), disable);
    });
EM_JS_PRFX(void, week_override_common_folder, (RoundContext roundcontext, const char* custom_common_path), {
    week_override_common_folder(kdmyEngine_obtain(roundcontext), kdmyEngine_ptrToString(custom_common_path));
    });
EM_JS_PRFX(void, week_enable_credits_on_completed, (RoundContext roundcontext), {
    week_enable_credits_on_completed(kdmyEngine_obtain(roundcontext))
    });
EM_JS_PRFX(void, week_end, (RoundContext roundcontext, bool round_or_week, bool loose_or_win), {
    week_end(kdmyEngine_obtain(roundcontext), round_or_week, loose_or_win);
    });

void week_unlockdirective_create(RoundContext roundcontext, const char* name, bool completed_round, bool completed_week, double value) {
    week_unlockdirective_create_JS(roundcontext, name, completed_round, completed_week, &value);
}

double week_unlockdirective_get(RoundContext roundcontext, const char* name) {
    double value;
    week_unlockdirective_get_JS(roundcontext, name, &value);
    return value;
}

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
    LUASCRIPT_GET(L);
    const char* name = luaL_checkstring(L, 1);
    bool completed_round = luaL_checkboolean(L, 2);
    bool completed_week = luaL_checkboolean(L, 3);
    double value = luaL_checknumber(L, 4);

    if (string_lengthbytes(name) < 1) {
        LUA_THROW_ERROR(L, "the directive name cannot be empty.");
    }

    week_unlockdirective_create(luascript->context, name, completed_round, completed_week, value);

    return 0;
}

static int script_week_unlockdirective_remove(lua_State* L) {
    LUASCRIPT_GET(L);
    const char* name = luaL_checkstring(L, 1);
    bool completed_round = luaL_checkboolean(L, 2);
    bool completed_week = luaL_checkboolean(L, 3);

    if (string_lengthbytes(name) < 1) {
        LUA_THROW_ERROR(L, "the directive name cannot be null or empty.");
    }

    week_unlockdirective_remove(luascript->context, name, completed_round, completed_week);

    return 0;
}

static int script_week_unlockdirective_get(lua_State* L) {
    LUASCRIPT_GET(L);
    const char* name = luaL_optstring(L, 1, NULL);

    if (string_lengthbytes(name) < 1) {
        LUA_THROW_ERROR(L, "the directive name cannot be null or empty.");
    }

    if (week_unlockdirective_has(luascript->context, name)) {
        double value = week_unlockdirective_get(luascript->context, name);
        lua_pushnumber(L, value);
    }
    else {
        lua_pushnil(L);
    }

    return 1;
}

static int script_week_halt(lua_State* L) {
    LUASCRIPT_GET(L);
    bool should_halt = luaL_checkboolean(L, 1);

    week_set_halt(luascript->context, should_halt);

    return 0;
}

static int script_week_ui_set_visibility(lua_State* L) {
    LUASCRIPT_GET(L);

    bool visible = luaL_checkboolean(L, 1);

    week_ui_set_visibility(luascript->context, visible);

    return 0;
}

static int script_week_ui_get_layout(lua_State* L) {
    LUASCRIPT_GET(L);

    Layout layout = week_ui_get_layout(luascript->context);

    return script_layout_new(L, layout);
}

static int script_week_get_stage_layout(lua_State* L) {
    LUASCRIPT_GET(L);

    Layout layout = week_get_stage_layout(luascript->context);

    return script_layout_new(L, layout);
}

static int script_week_ui_get_camera(lua_State* L) {
    LUASCRIPT_GET(L);

    Layout layout = week_ui_get_layout(luascript->context);
    Camera camera = week_ui_get_camera(luascript->context);

    return script_camera_new(L, layout, camera);
}

static int script_week_ui_get_strums_count(lua_State* L) {
    LUASCRIPT_GET(L);

    int32_t ret = week_ui_get_strums_count(luascript->context);

    lua_pushinteger(L, ret);
    return 1;
}

static int script_week_ui_get_strums(lua_State* L) {
    return not_implemented(L);

    /*LUASCRIPT_GET(L);
    int32_t strums_id = (int32_t)luaL_checkinteger(L, 1);

    Strums strums = week_ui_get_strums(luascript->context, strums_id);

    return script_week_strums_new(L, strums);*/
}

static int script_week_ui_get_roundstats(lua_State* L) {
    return not_implemented(L);

    /*LUASCRIPT_GET(L);
    Roundstats roundstats = week_ui_get_roundstats(luascript->context);
    return script_week_roundstats_new(L, roundstats);*/
}

static int script_week_ui_get_rankingcounter(lua_State* L) {
    return not_implemented(L);

    /*LUASCRIPT_GET(L);
    Rankingcounter rankingcounter = week_ui_get_rankingcounter(luascript->context);
    return script_week_rankingcounter_new(L, rankingcounter);*/
}

static int script_week_ui_get_streakcounter(lua_State* L) {
    return not_implemented(L);

    /*LUASCRIPT_GET(L);
    Streakcounter streakcounter = week_ui_get_streakcounter(luascript->context);
    return script_week_streakcounter_new(L, streakcounter);*/
}

static int script_week_ui_get_trackinfo(lua_State* L) {
    LUASCRIPT_GET(L);

    TextSprite textsprite = week_ui_get_trackinfo(luascript->context);

    return script_textsprite_new(L, NULL, textsprite);
}

static int script_week_ui_get_songprogressbar(lua_State* L) {
    return not_implemented(L);

    /*LUASCRIPT_GET(L);
    SongProgressbar songprogressbar = week_ui_get_songprogressbar(luascript->context);
    return script_week_songprogressbar_new(L, songprogressbar);*/
}

static int script_week_set_bpm(lua_State* L) {
    LUASCRIPT_GET(L);
    float bpm = luaL_checkfloat(L, 1);

    week_update_bpm(luascript->context, bpm);
    return 0;
}

static int script_week_set_speed(lua_State* L) {
    LUASCRIPT_GET(L);
    double speed = luaL_checknumber(L, 1);

    week_update_speed(luascript->context, speed);
    return 0;
}

static int script_week_ui_get_messagebox(lua_State* L) {
    LUASCRIPT_GET(L);

    Messagebox messagebox = week_ui_get_messagebox(luascript->context);

    return script_messagebox_new(L, messagebox);
}

static int script_week_get_girlfriend(lua_State* L) {
    LUASCRIPT_GET(L);

    Character character = week_get_girlfriend(luascript->context);

    return script_character_new(L, character);
}

static int script_week_get_character_count(lua_State* L) {
    LUASCRIPT_GET(L);
    int32_t count = week_get_character_count(luascript->context);

    lua_pushinteger(L, count);
    return 1;
}

static int script_week_get_character(lua_State* L) {
    LUASCRIPT_GET(L);
    int32_t index = (int32_t)luaL_checkinteger(L, 1);

    Character character = week_get_character(luascript->context, index);

    return script_character_new(L, character);
}

static int script_week_get_songplayer(lua_State* L) {
    LUASCRIPT_GET(L);

    SongPlayer songplayer = week_get_songplayer(luascript->context);

    return script_songplayer_new(L, songplayer);
}

static int script_week_get_current_chart_info(lua_State* L) {
    LUASCRIPT_GET(L);

    WeekChartInfo_t chartinfo = { .bpm = 0.0f, .speed = 1.0 };

    week_get_current_chart_info(luascript->context, &chartinfo);

    lua_pushnumber(L, (lua_Number)chartinfo.bpm);
    lua_pushnumber(L, (lua_Number)chartinfo.speed);

    return 2;
}

static int script_week_get_current_track_info(lua_State* L) {
    LUASCRIPT_GET(L);

    WeekTrackInfo_t trackinfo;
#ifdef JAVASCRIPT
    trackinfo._ptrsize = sizeof(const char*);
#endif
    trackinfo.name = NULL;
    trackinfo.difficult = NULL;
    trackinfo.index = -1;

    week_get_current_track_info(luascript->context, &trackinfo);

    lua_pushstring(L, trackinfo.name);
    lua_pushstring(L, trackinfo.difficult);
    lua_pushinteger(L, trackinfo.index);

#ifdef JAVASCRIPT
    if (trackinfo.name != NULL) free(trackinfo.name);
#endif

    return 3;
}

static int script_week_change_charecter_camera_name(lua_State* L) {
    LUASCRIPT_GET(L);

    bool opponent_or_player = luaL_checkboolean(L, 1);
    const char* new_name = luaL_optstring(L, 2, NULL);

    week_change_charecter_camera_name(luascript->context, opponent_or_player, new_name);

    return 0;
}

static int script_week_disable_layout_rollback(lua_State* L) {
    LUASCRIPT_GET(L);

    bool disable = luaL_checkboolean(L, 1);

    week_disable_layout_rollback(luascript->context, disable);

    return 0;
}

static int script_week_override_common_folder(lua_State* L) {
    LUASCRIPT_GET(L);

    const char* custom_common_path = luaL_optstring(L, 1, NULL);

    week_override_common_folder(luascript->context, custom_common_path);

    return 0;
}

static int script_week_enable_credits_on_completed(lua_State* L) {
    LUASCRIPT_GET(L);

    week_enable_credits_on_completed(luascript->context);

    return 0;
}

static int script_week_end(lua_State* L) {
    LUASCRIPT_GET(L);

    bool round_or_week = luaL_checkboolean(L, 1);
    bool loose_or_win = luaL_checkboolean(L, 2);

    week_end(luascript->context, round_or_week, loose_or_win);

    return 0;
}

static const luaL_Reg EXPORTS_FUNCTION[] = {
    { "unlockdirective_create", script_week_unlockdirective_create },
    { "unlockdirective_remove", script_week_unlockdirective_remove },
    { "unlockdirective_get", script_week_unlockdirective_get },
    { "week_set_halt", script_week_halt },
    { "ui_set_visibility", script_week_ui_set_visibility },
    { "ui_get_layout", script_week_ui_get_layout },
    { "week_get_stage_layout", script_week_get_stage_layout },
    { "ui_get_camera", script_week_ui_get_camera },
    { "ui_get_strums_count", script_week_ui_get_strums_count },
    { "ui_get_strums", script_week_ui_get_strums },
    { "ui_get_roundstats", script_week_ui_get_roundstats },
    { "ui_get_rankingcounter", script_week_ui_get_rankingcounter },
    { "ui_get_streakcounter", script_week_ui_get_streakcounter },
    { "ui_get_trackinfo", script_week_ui_get_trackinfo },
    { "ui_get_songprogressbar", script_week_ui_get_songprogressbar },
    { "week_set_bpm", script_week_set_bpm },
    { "week_set_speed", script_week_set_speed },
    { "ui_get_messagebox", script_week_ui_get_messagebox },
    { "week_get_girlfriend", script_week_get_girlfriend },
    { "week_get_character_count", script_week_get_character_count },
    { "week_get_character", script_week_get_character },
    { "week_get_songplayer", script_week_get_songplayer },
    { "week_get_current_chart_info", script_week_get_current_chart_info },
    { "week_get_current_track_info", script_week_get_current_track_info },
    { "week_change_charecter_camera_name", script_week_change_charecter_camera_name },
    { "week_disable_layout_rollback", script_week_disable_layout_rollback },
    { "week_override_common_folder", script_week_override_common_folder },
    { "week_enable_credits_on_completed", script_week_enable_credits_on_completed },
    { "week_end", script_week_end },
    { NULL, NULL }
};

static const luaL_Reg_Integer EXPORTS_GLOBAL[] = {
    { "NOTE_MISS", 0},
    { "NOTE_PENALITY", 1},
    { "NOTE_SHIT", 2},
    { "NOTE_BAD", 3},
    { "NOTE_GOOD", 4},
    { "NOTE_SICK", 5},
    { NULL, 0 }
};


void register_week(lua_State* L) {
    for (size_t i = 0; ; i++) {
        if (EXPORTS_FUNCTION[i].name == NULL || EXPORTS_FUNCTION[i].func == NULL) break;
        lua_pushcfunction(L, EXPORTS_FUNCTION[i].func);
        lua_setglobal(L, EXPORTS_FUNCTION[i].name);
    }

    for (size_t i = 0; ; i++) {
        if (EXPORTS_GLOBAL[i].variable == NULL) break;
        lua_pushinteger(L, EXPORTS_GLOBAL[i].value);
        lua_setglobal(L, EXPORTS_GLOBAL[i].variable);
    }
}

