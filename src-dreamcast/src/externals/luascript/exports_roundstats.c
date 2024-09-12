#include "luascript_internal.h"

#include "game/funkin/roundstats.h"


static int script_roundstats_hide(lua_State* L) {
    RoundStats roundstats = luascript_read_userdata(L, ROUNDSTATS);
    bool hide = (bool)lua_toboolean(L, 2);

    roundstats_hide(roundstats, hide);

    return 0;
}

static int script_roundstats_hide_nps(lua_State* L) {
    RoundStats roundstats = luascript_read_userdata(L, ROUNDSTATS);
    bool hide = (bool)lua_toboolean(L, 2);

    roundstats_hide_nps(roundstats, hide);

    return 0;
}

static int script_roundstats_set_draw_y(lua_State* L) {
    RoundStats roundstats = luascript_read_userdata(L, ROUNDSTATS);
    float y = (float)luaL_checknumber(L, 2);

    roundstats_set_draw_y(roundstats, y);

    return 0;
}

static int script_roundstats_reset(lua_State* L) {
    RoundStats roundstats = luascript_read_userdata(L, ROUNDSTATS);

    roundstats_reset(roundstats);

    return 0;
}

static int script_roundstats_get_drawable(lua_State* L) {
    RoundStats roundstats = luascript_read_userdata(L, ROUNDSTATS);

    Drawable ret = roundstats_get_drawable(roundstats);

    return script_drawable_new(L, ret);
}

static int script_roundstats_tweenkeyframe_set_on_beat(lua_State* L) {
    RoundStats roundstats = luascript_read_userdata(L, ROUNDSTATS);
    TweenKeyframe tweenkeyframe = luascript_read_nullable_userdata(L, 2, TWEENKEYFRAME);
    float rollback_beats = (float)luaL_checknumber(L, 3);
    float beat_duration = (float)luaL_checknumber(L, 4);

    roundstats_tweenkeyframe_set_on_beat(roundstats, tweenkeyframe, rollback_beats, beat_duration);

    return 0;
}

static int script_roundstats_tweenkeyframe_set_on_hit(lua_State* L) {
    RoundStats roundstats = luascript_read_userdata(L, ROUNDSTATS);
    TweenKeyframe tweenkeyframe = luascript_read_nullable_userdata(L, 2, TWEENKEYFRAME);
    float rollback_beats = (float)luaL_checknumber(L, 3);
    float beat_duration = (float)luaL_checknumber(L, 4);

    roundstats_tweenkeyframe_set_on_hit(roundstats, tweenkeyframe, rollback_beats, beat_duration);

    return 0;
}

static int script_roundstats_tweenkeyframe_set_on_miss(lua_State* L) {
    RoundStats roundstats = luascript_read_userdata(L, ROUNDSTATS);
    TweenKeyframe tweenkeyframe = luascript_read_nullable_userdata(L, 2, TWEENKEYFRAME);
    float rollback_beats = (float)luaL_checknumber(L, 3);
    float beat_duration = (float)luaL_checknumber(L, 4);

    roundstats_tweenkeyframe_set_on_miss(roundstats, tweenkeyframe, rollback_beats, beat_duration);

    return 0;
}

static int script_roundstats_tweenkeyframe_set_bpm(lua_State* L) {
    RoundStats roundstats = luascript_read_userdata(L, ROUNDSTATS);
    float beats_per_minute = (float)luaL_checknumber(L, 2);

    roundstats_tweenkeyframe_set_bpm(roundstats, beats_per_minute);

    return 0;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const luaL_Reg ROUNDSTATS_FUNCTIONS[] = {
    {"hide", script_roundstats_hide},
    {"hide_nps", script_roundstats_hide_nps},
    {"set_draw_y", script_roundstats_set_draw_y},
    {"reset", script_roundstats_reset},
    {"get_drawable", script_roundstats_get_drawable},
    {"tweenkeyframe_set_on_beat", script_roundstats_tweenkeyframe_set_on_beat},
    {"tweenkeyframe_set_on_hit", script_roundstats_tweenkeyframe_set_on_hit},
    {"tweenkeyframe_set_on_miss", script_roundstats_tweenkeyframe_set_on_miss},
    {"tweenkeyframe_set_bpm", script_roundstats_tweenkeyframe_set_bpm},
    {NULL, NULL}
};

int script_roundstats_new(lua_State* L, RoundStats roundstats) {
    return luascript_userdata_new(L, ROUNDSTATS, roundstats);
}

static int script_roundstats_gc(lua_State* L) {
    return luascript_userdata_gc(L, ROUNDSTATS);
}

static int script_roundstats_tostring(lua_State* L) {
    return luascript_userdata_tostring(L, ROUNDSTATS);
}

void script_roundstats_register(lua_State* L) {
    luascript_register(L, ROUNDSTATS, script_roundstats_gc, script_roundstats_tostring, ROUNDSTATS_FUNCTIONS);
}
