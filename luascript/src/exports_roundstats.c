#include "luascript_internal.h"

#ifdef JAVASCRIPT
EM_JS_PRFX(void, roundstats_hide, (RoundStats roundstats, bool hide), {
    roundstats_hide(kdmyEngine_obtain(roundstats), hide);
});
EM_JS_PRFX(void, roundstats_hide_nps, (RoundStats roundstats, bool hide), {
    roundstats_hide_nps(kdmyEngine_obtain(roundstats), hide);
});
EM_JS_PRFX(void, roundstats_set_draw_y, (RoundStats roundstats, float y), {
    roundstats_set_draw_y(kdmyEngine_obtain(roundstats), y);
});
EM_JS_PRFX(void, roundstats_reset, (RoundStats roundstats), {
    roundstats_reset(kdmyEngine_obtain(roundstats));
});
EM_JS_PRFX(Drawable, roundstats_get_drawable, (RoundStats roundstats), {
    let ret = roundstats_get_drawable(kdmyEngine_obtain(roundstats));
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(void, roundstats_tweenlerp_set_on_beat, (RoundStats roundstats, TweenLerp tweenlerp, float rollback_beats, float beat_duration), {
    roundstats_tweenlerp_set_on_beat(kdmyEngine_obtain(roundstats), kdmyEngine_obtain(tweenlerp), rollback_beats, beat_duration);
});
EM_JS_PRFX(void, roundstats_tweenlerp_set_on_hit, (RoundStats roundstats, TweenLerp tweenlerp, float rollback_beats, float beat_duration), {
    roundstats_tweenlerp_set_on_hit(kdmyEngine_obtain(roundstats), kdmyEngine_obtain(tweenlerp), rollback_beats, beat_duration);
});
EM_JS_PRFX(void, roundstats_tweenlerp_set_on_miss, (RoundStats roundstats, TweenLerp tweenlerp, float rollback_beats, float beat_duration), {
    roundstats_tweenlerp_set_on_miss(kdmyEngine_obtain(roundstats), kdmyEngine_obtain(tweenlerp), rollback_beats, beat_duration);
});
EM_JS_PRFX(void, roundstats_tweenlerp_set_bpm, (RoundStats roundstats, float beats_per_minute), {
    roundstats_tweenlerp_set_bpm(kdmyEngine_obtain(roundstats), beats_per_minute);
});
#endif


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

static int script_roundstats_tweenlerp_set_on_beat(lua_State* L) {
    RoundStats roundstats = luascript_read_userdata(L, ROUNDSTATS);
    TweenLerp tweenlerp = luascript_read_nullable_userdata(L, 2, TWEENLERP);
    float rollback_beats = (float)luaL_checknumber(L, 3);
    float beat_duration = (float)luaL_checknumber(L, 4);

    roundstats_tweenlerp_set_on_beat(roundstats, tweenlerp, rollback_beats, beat_duration);

    return 0;
}

static int script_roundstats_tweenlerp_set_on_hit(lua_State* L) {
    RoundStats roundstats = luascript_read_userdata(L, ROUNDSTATS);
    TweenLerp tweenlerp = luascript_read_nullable_userdata(L, 2, TWEENLERP);
    float rollback_beats = (float)luaL_checknumber(L, 3);
    float beat_duration = (float)luaL_checknumber(L, 4);

    roundstats_tweenlerp_set_on_hit(roundstats, tweenlerp, rollback_beats, beat_duration);

    return 0;
}

static int script_roundstats_tweenlerp_set_on_miss(lua_State* L) {
    RoundStats roundstats = luascript_read_userdata(L, ROUNDSTATS);
    TweenLerp tweenlerp = luascript_read_nullable_userdata(L, 2, TWEENLERP);
    float rollback_beats = (float)luaL_checknumber(L, 3);
    float beat_duration = (float)luaL_checknumber(L, 4);

    roundstats_tweenlerp_set_on_miss(roundstats, tweenlerp, rollback_beats, beat_duration);

    return 0;
}

static int script_roundstats_tweenlerp_set_bpm(lua_State* L) {
    RoundStats roundstats = luascript_read_userdata(L, ROUNDSTATS);
    float beats_per_minute = (float)luaL_checknumber(L, 2);

    roundstats_tweenlerp_set_bpm(roundstats, beats_per_minute);

    return 0;
}




static const luaL_Reg ROUNDSTATS_FUNCTIONS[] = {
    { "hide", script_roundstats_hide },
    { "hide_nps", script_roundstats_hide_nps },
    { "set_draw_y", script_roundstats_set_draw_y },
    { "reset", script_roundstats_reset },
    { "get_drawable", script_roundstats_get_drawable },
    { "tweenlerp_set_on_beat", script_roundstats_tweenlerp_set_on_beat },
    { "tweenlerp_set_on_hit", script_roundstats_tweenlerp_set_on_hit },
    { "tweenlerp_set_on_miss", script_roundstats_tweenlerp_set_on_miss },
    { "tweenlerp_set_bpm", script_roundstats_tweenlerp_set_bpm },
    { NULL, NULL }
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

