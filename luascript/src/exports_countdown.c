#include "luascript_internal.h"

#ifdef JAVASCRIPT
EM_JS_PRFX(void, countdown_set_default_animation2, (Countdown countdown, TweenKeyframe tweenkeyframe), {
    countdown_set_default_animation2(kdmyEngine_obtain(countdown), kdmyEngine_obtain(tweenkeyframe));
});
EM_JS_PRFX(void, countdown_set_bpm, (Countdown countdown, float bpm), {
    countdown_set_bpm(kdmyEngine_obtain(countdown), bpm);
});
EM_JS_PRFX(Drawable, countdown_get_drawable, (Countdown countdown), {
    let ret = countdown_get_drawable(kdmyEngine_obtain(countdown));
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(bool, countdown_ready, (Countdown countdown), {
    let ret = countdown_ready(kdmyEngine_obtain(countdown));
    return ret ? 1 : 0;
});
EM_JS_PRFX(bool, countdown_start, (Countdown countdown), {
    let ret = countdown_start(kdmyEngine_obtain(countdown));
    return ret ? 1 : 0;
});
EM_JS_PRFX(bool, countdown_has_ended, (Countdown countdown), {
    let ret = countdown_has_ended(kdmyEngine_obtain(countdown));
    return ret ? 1 : 0;
});
#endif

static int script_countdown_set_default_animation2(lua_State* L) {
    Countdown countdown = luascript_read_userdata(L, COUNTDOWN);
    TweenKeyframe tweenkeyframe = luascript_read_nullable_userdata(L, 2, TWEENKEYFRAME);

    countdown_set_default_animation2(countdown, tweenkeyframe);

    return 0;
}

static int script_countdown_set_bpm(lua_State* L) {
    Countdown countdown = luascript_read_userdata(L, COUNTDOWN);
    float bpm = (float)luaL_checknumber(L, 2);

    countdown_set_bpm(countdown, bpm);

    return 0;
}

static int script_countdown_get_drawable(lua_State* L) {
    Countdown countdown = luascript_read_userdata(L, COUNTDOWN);

    Drawable ret = countdown_get_drawable(countdown);

    return script_drawable_new(L, ret);
}

static int script_countdown_ready(lua_State* L) {
    Countdown countdown = luascript_read_userdata(L, COUNTDOWN);

    bool ret = countdown_ready(countdown);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_countdown_start(lua_State* L) {
    Countdown countdown = luascript_read_userdata(L, COUNTDOWN);

    bool ret = countdown_start(countdown);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_countdown_has_ended(lua_State* L) {
    Countdown countdown = luascript_read_userdata(L, COUNTDOWN);

    bool ret = countdown_has_ended(countdown);

    lua_pushboolean(L, ret);
    return 1;
}




static const luaL_Reg COUNTDOWN_FUNCTIONS[] = {
    { "set_default_animation2", script_countdown_set_default_animation2 },
    { "set_bpm", script_countdown_set_bpm },
    { "get_drawable", script_countdown_get_drawable },
    { "ready", script_countdown_ready },
    { "start", script_countdown_start },
    { "has_ended", script_countdown_has_ended },
    { NULL, NULL }
};

int script_countdown_new(lua_State* L, Countdown countdown) {
    return luascript_userdata_new(L, COUNTDOWN, countdown);
}

static int script_countdown_gc(lua_State* L) {
    return luascript_userdata_gc(L, COUNTDOWN);
}

static int script_countdown_tostring(lua_State* L) {
    return luascript_userdata_tostring(L, COUNTDOWN);
}

void script_countdown_register(lua_State* L) {
    luascript_register(L, COUNTDOWN, script_countdown_gc, script_countdown_tostring, COUNTDOWN_FUNCTIONS);
}

