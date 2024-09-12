#include "luascript_internal.h"

#include "luascript_enums.h"
#include "videoplayer.h"


static int script_videoplayer_get_sprite(lua_State* L) {
    VideoPlayer videoplayer = luascript_read_userdata(L, VIDEOPLAYER);

    Sprite ret = videoplayer_get_sprite(videoplayer);

    return script_sprite_new(L, ret);
}

static int script_videoplayer_replay(lua_State* L) {
    VideoPlayer videoplayer = luascript_read_userdata(L, VIDEOPLAYER);

    videoplayer_replay(videoplayer);

    return 0;
}

static int script_videoplayer_play(lua_State* L) {
    VideoPlayer videoplayer = luascript_read_userdata(L, VIDEOPLAYER);

    videoplayer_play(videoplayer);

    return 0;
}

static int script_videoplayer_pause(lua_State* L) {
    VideoPlayer videoplayer = luascript_read_userdata(L, VIDEOPLAYER);

    videoplayer_pause(videoplayer);

    return 0;
}

static int script_videoplayer_stop(lua_State* L) {
    VideoPlayer videoplayer = luascript_read_userdata(L, VIDEOPLAYER);

    videoplayer_stop(videoplayer);

    return 0;
}

static int script_videoplayer_loop_enable(lua_State* L) {
    VideoPlayer videoplayer = luascript_read_userdata(L, VIDEOPLAYER);
    bool enable = (bool)lua_toboolean(L, 2);

    videoplayer_loop_enable(videoplayer, enable);

    return 0;
}

static int script_videoplayer_fade_audio(lua_State* L) {
    VideoPlayer videoplayer = luascript_read_userdata(L, VIDEOPLAYER);
    bool in_or_out = (bool)lua_toboolean(L, 2);
    float duration = (float)luaL_checknumber(L, 3);

    videoplayer_fade_audio(videoplayer, in_or_out, duration);

    return 0;
}

static int script_videoplayer_set_volume(lua_State* L) {
    VideoPlayer videoplayer = luascript_read_userdata(L, VIDEOPLAYER);
    float volume = (float)luaL_checknumber(L, 2);

    videoplayer_set_volume(videoplayer, volume);

    return 0;
}

static int script_videoplayer_set_mute(lua_State* L) {
    VideoPlayer videoplayer = luascript_read_userdata(L, VIDEOPLAYER);
    bool muted = (bool)lua_toboolean(L, 2);

    videoplayer_set_mute(videoplayer, muted);

    return 0;
}

static int script_videoplayer_seek(lua_State* L) {
    VideoPlayer videoplayer = luascript_read_userdata(L, VIDEOPLAYER);
    float64 timestamp = luaL_checknumber(L, 2);

    videoplayer_seek(videoplayer, timestamp);

    return 0;
}

static int script_videoplayer_is_muted(lua_State* L) {
    VideoPlayer videoplayer = luascript_read_userdata(L, VIDEOPLAYER);

    bool ret = videoplayer_is_muted(videoplayer);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_videoplayer_is_playing(lua_State* L) {
    VideoPlayer videoplayer = luascript_read_userdata(L, VIDEOPLAYER);

    bool ret = videoplayer_is_playing(videoplayer);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_videoplayer_get_duration(lua_State* L) {
    VideoPlayer videoplayer = luascript_read_userdata(L, VIDEOPLAYER);

    float64 ret = videoplayer_get_duration(videoplayer);

    lua_pushnumber(L, (lua_Number)ret);
    return 1;
}

static int script_videoplayer_get_position(lua_State* L) {
    VideoPlayer videoplayer = luascript_read_userdata(L, VIDEOPLAYER);

    float64 ret = videoplayer_get_position(videoplayer);

    lua_pushnumber(L, (lua_Number)ret);
    return 1;
}

static int script_videoplayer_has_ended(lua_State* L) {
    VideoPlayer videoplayer = luascript_read_userdata(L, VIDEOPLAYER);

    bool ret = videoplayer_has_ended(videoplayer);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_videoplayer_has_video_track(lua_State* L) {
    VideoPlayer videoplayer = luascript_read_userdata(L, VIDEOPLAYER);

    bool ret = videoplayer_has_video_track(videoplayer);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_videoplayer_has_audio_track(lua_State* L) {
    VideoPlayer videoplayer = luascript_read_userdata(L, VIDEOPLAYER);

    bool ret = videoplayer_has_audio_track(videoplayer);

    lua_pushboolean(L, ret);
    return 1;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const luaL_Reg VIDEOPLAYER_FUNCTIONS[] = {
    {"get_sprite", script_videoplayer_get_sprite},
    {"replay", script_videoplayer_replay},
    {"play", script_videoplayer_play},
    {"pause", script_videoplayer_pause},
    {"stop", script_videoplayer_stop},
    {"loop_enable", script_videoplayer_loop_enable},
    {"fade_audio", script_videoplayer_fade_audio},
    {"set_volume", script_videoplayer_set_volume},
    {"set_mute", script_videoplayer_set_mute},
    {"seek", script_videoplayer_seek},
    {"is_muted", script_videoplayer_is_muted},
    {"is_playing", script_videoplayer_is_playing},
    {"get_duration", script_videoplayer_get_duration},
    {"get_position", script_videoplayer_get_position},
    {"has_ended", script_videoplayer_has_ended},
    {"has_video_track", script_videoplayer_has_video_track},
    {"has_audio_track", script_videoplayer_has_audio_track},
    {NULL, NULL}
};

int script_videoplayer_new(lua_State* L, VideoPlayer videoplayer) {
    return luascript_userdata_new(L, VIDEOPLAYER, videoplayer);
}

static int script_videoplayer_gc(lua_State* L) {
    return luascript_userdata_gc(L, VIDEOPLAYER);
}

static int script_videoplayer_tostring(lua_State* L) {
    return luascript_userdata_tostring(L, VIDEOPLAYER);
}

void script_videoplayer_register(lua_State* L) {
    luascript_register(L, VIDEOPLAYER, script_videoplayer_gc, script_videoplayer_tostring, VIDEOPLAYER_FUNCTIONS);
}
