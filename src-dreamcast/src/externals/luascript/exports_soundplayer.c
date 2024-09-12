
#include "luascript_internal.h"

#include "logger.h"
#include "soundplayer.h"


static int script_soundplayer_init(lua_State* L) {
    const char* src = luaL_checkstring(L, 2);

    luascript_change_working_folder(L);
    SoundPlayer ret = soundplayer_init(src);
    luascript_restore_working_folder(L);

    return luascript_userdata_allocnew(L, SOUNDPLAYER, ret);
}

static int script_soundplayer_destroy(lua_State* L) {
    SoundPlayer soundplayer = luascript_read_userdata(L, SOUNDPLAYER);

    if (luascript_userdata_is_allocated(L, SOUNDPLAYER))
        soundplayer_destroy(&soundplayer);
    else
        logger_warn("script_soundplayer_destroy() object was not allocated by lua");

    return 0;
}

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

static int script_soundplayer_has_fading(lua_State* L) {
    SoundPlayer soundplayer = luascript_read_userdata(L, SOUNDPLAYER);

    Fading ret = soundplayer_has_fading(soundplayer);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
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

    float64 ret = soundplayer_get_duration(soundplayer);

    lua_pushnumber(L, (lua_Number)ret);
    return 1;
}

static int script_soundplayer_get_position(lua_State* L) {
    SoundPlayer soundplayer = luascript_read_userdata(L, SOUNDPLAYER);

    float64 ret = soundplayer_get_position(soundplayer);

    lua_pushnumber(L, (lua_Number)ret);
    return 1;
}

static int script_soundplayer_seek(lua_State* L) {
    SoundPlayer soundplayer = luascript_read_userdata(L, SOUNDPLAYER);

    float64 timestamp = luaL_checknumber(L, 2);

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
    {"init", script_soundplayer_init},
    {"destroy", script_soundplayer_destroy},
    {"play", script_soundplayer_play},
    {"pause", script_soundplayer_pause},
    {"stop", script_soundplayer_stop},
    {"loop_enable", script_soundplayer_loop_enable},
    {"fade", script_soundplayer_fade},
    {"set_volume", script_soundplayer_set_volume},
    {"set_mute", script_soundplayer_set_mute},
    {"has_fading", script_soundplayer_has_fading},
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
    // if this object was allocated by lua, call the destructor
    return luascript_userdata_destroy(L, SOUNDPLAYER, (Destructor)soundplayer_destroy);
}

static int script_soundplayer_tostring(lua_State* L) {
    return luascript_userdata_tostring(L, SOUNDPLAYER);
}

void script_soundplayer_register(lua_State* L) {
    luascript_register(L, SOUNDPLAYER, script_soundplayer_gc, script_soundplayer_tostring, SOUNDPLAYER_FUNCTIONS);
}
