
#include "luascript_internal.h"



#ifdef JAVASCRIPT
EM_JS_PRFX(void, soundplayer_play, (SoundPlayer soundplayer), {
    soundplayer_play(kdmyEngine_obtain(soundplayer));
});
EM_JS_PRFX(void, soundplayer_pause, (SoundPlayer soundplayer), {
    soundplayer_pause(kdmyEngine_obtain(soundplayer));
});
EM_JS_PRFX(void, soundplayer_stop, (SoundPlayer soundplayer), {
    soundplayer_stop(kdmyEngine_obtain(soundplayer));
});
EM_JS_PRFX(void, soundplayer_loop_enable, (SoundPlayer soundplayer, bool enable), {
    soundplayer_loop_enable(kdmyEngine_obtain(soundplayer), enable);
});
EM_JS_PRFX(void, soundplayer_fade, (SoundPlayer soundplayer, bool in_or_out, float duration), {
    soundplayer_fade(kdmyEngine_obtain(soundplayer), in_or_out, duration);
});
EM_JS_PRFX(void, soundplayer_set_volume, (SoundPlayer soundplayer, float volume), {
    soundplayer_set_volume(kdmyEngine_obtain(soundplayer), volume);
});
EM_JS_PRFX(void, soundplayer_set_mute, (SoundPlayer soundplayer, bool muted), {
    soundplayer_set_mute(kdmyEngine_obtain(soundplayer), muted);
});
EM_JS_PRFX(bool, soundplayer_is_muted, (SoundPlayer soundplayer), {
    return soundplayer_is_muted(kdmyEngine_obtain(soundplayer));
});
EM_JS_PRFX(bool, soundplayer_is_playing, (SoundPlayer soundplayer), {
    return soundplayer_is_playing(kdmyEngine_obtain(soundplayer));
});
EM_JS_PRFX(double, soundplayer_get_duration, (SoundPlayer soundplayer), {
    return soundplayer_get_duration(kdmyEngine_obtain(soundplayer));
});
EM_JS_PRFX(double, soundplayer_get_position, (SoundPlayer soundplayer), {
    return soundplayer_get_position(kdmyEngine_obtain(soundplayer));
});
EM_JS_PRFX(void, soundplayer_seek, (SoundPlayer soundplayer, double timestamp), {
    return soundplayer_seek(kdmyEngine_obtain(soundplayer), timestamp);
});
EM_JS_PRFX(bool, soundplayer_has_ended, (SoundPlayer soundplayer), {
    return soundplayer_has_ended(kdmyEngine_obtain(soundplayer));
});
#endif



static int script_soundplayer_play(lua_State* L) {
    SoundPlayer soundplayer = luascript_read_userdata(L, SOUNDPLAYER);

    soundplayer_play(soundplayer);

    return 0;
}

static int script_soundplayer_pause(lua_State* L) {
    SoundPlayer soundplayer = luascript_read_userdata(L, SOUNDPLAYER);

    soundplayer_pause(soundplayer);

    return 0;
}

static int script_soundplayer_stop(lua_State* L) {
    SoundPlayer soundplayer = luascript_read_userdata(L, SOUNDPLAYER);

    soundplayer_stop(soundplayer);
    return 0;
}

static int script_soundplayer_loop_enable(lua_State* L) {
    SoundPlayer soundplayer = luascript_read_userdata(L, SOUNDPLAYER);

    bool enable = lua_toboolean(L, 2);

    soundplayer_loop_enable(soundplayer, enable);

    return 0;
}

static int script_soundplayer_fade(lua_State* L) {
    SoundPlayer soundplayer = luascript_read_userdata(L, SOUNDPLAYER);

    bool in_or_out = lua_toboolean(L, 2);
    float duration = (float)luaL_checknumber(L, 3);

    soundplayer_fade(soundplayer, in_or_out, duration);

    return 0;
}

static int script_soundplayer_set_volume(lua_State* L) {
    SoundPlayer soundplayer = luascript_read_userdata(L, SOUNDPLAYER);

    float volume = (float)luaL_checknumber(L, 2);

    soundplayer_set_volume(soundplayer, volume);

    return 0;
}

static int script_soundplayer_set_mute(lua_State* L) {
    SoundPlayer soundplayer = luascript_read_userdata(L, SOUNDPLAYER);

    bool muted = lua_toboolean(L, 2);

    soundplayer_set_mute(soundplayer, muted);

    return 0;
}

static int script_soundplayer_is_muted(lua_State* L) {
    SoundPlayer soundplayer = luascript_read_userdata(L, SOUNDPLAYER);

    bool ret = soundplayer_is_muted(soundplayer);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_soundplayer_is_playing(lua_State* L) {
    SoundPlayer soundplayer = luascript_read_userdata(L, SOUNDPLAYER);

    bool ret = soundplayer_is_playing(soundplayer);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_soundplayer_get_duration(lua_State* L) {
    SoundPlayer soundplayer = luascript_read_userdata(L, SOUNDPLAYER);

    double ret = soundplayer_get_duration(soundplayer);

    lua_pushnumber(L, ret);
    return 1;
}

static int script_soundplayer_get_position(lua_State* L) {
    SoundPlayer soundplayer = luascript_read_userdata(L, SOUNDPLAYER);

    double ret = soundplayer_get_position(soundplayer);

    lua_pushnumber(L, ret);
    return 1;
}

static int script_soundplayer_seek(lua_State* L) {
    SoundPlayer soundplayer = luascript_read_userdata(L, SOUNDPLAYER);

    double timestamp = luaL_checknumber(L, 2);

    soundplayer_seek(soundplayer, timestamp);

    return 0;
}

static int script_soundplayer_has_ended(lua_State* L) {
    SoundPlayer soundplayer = luascript_read_userdata(L, SOUNDPLAYER);

    bool ret = soundplayer_has_ended(soundplayer);

    lua_pushboolean(L, ret);
    return 1;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const luaL_Reg SOUNDPLAYER_FUNCTIONS[] = {
    {"play", script_soundplayer_play},
    {"pause", script_soundplayer_pause},
    {"stop", script_soundplayer_stop},
    {"loop_enable", script_soundplayer_loop_enable},
    {"fade", script_soundplayer_fade},
    {"set_volume", script_soundplayer_set_volume},
    {"set_mute", script_soundplayer_set_mute},
    {"is_muted", script_soundplayer_is_muted},
    {"is_playing", script_soundplayer_is_playing},
    {"get_duration", script_soundplayer_get_duration},
    {"get_position", script_soundplayer_get_position},
    {"seek", script_soundplayer_seek},
    {"has_ended", script_soundplayer_has_ended},
    {NULL, NULL}
};


int script_soundplayer_new(lua_State* L, SoundPlayer soundplayer) {
    return luascript_userdata_new(L, SOUNDPLAYER, soundplayer);
}

static int script_soundplayer_gc(lua_State* L) {
    return luascript_userdata_gc(L, SOUNDPLAYER);
}

static int script_soundplayer_tostring(lua_State* L) {
    return luascript_userdata_tostring(L, SOUNDPLAYER);
}

void script_soundplayer_register(lua_State* L) {
    luascript_register(L, SOUNDPLAYER, script_soundplayer_gc, script_soundplayer_tostring, SOUNDPLAYER_FUNCTIONS);
}

