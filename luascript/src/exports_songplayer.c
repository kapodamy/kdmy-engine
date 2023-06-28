#include "luascript_internal.h"



#ifdef JAVASCRIPT
EM_ASYNC_JS_PRFX(bool, songplayer_changesong, (SongPlayer songplayer, const char* src, bool prefer_alternative), {
    return await songplayer_changesong(kdmyEngine_obtain(songplayer), kdmyEngine_ptrToString(src), prefer_alternative);
});
EM_ASYNC_JS_PRFX(void, songplayer_play, (SongPlayer songplayer, SongInfo_t* songinfo), {
    const _songinfo = {completed : 0, timestamp : 0};
    await songplayer_play(kdmyEngine_obtain(songplayer), _songinfo);
});
EM_JS_PRFX(void, songplayer_pause, (SongPlayer songplayer), {
    songplayer_pause(kdmyEngine_obtain(songplayer));
});
EM_JS_PRFX(void, songplayer_seek, (SongPlayer songplayer, double timestamp), {
    songplayer_seek(kdmyEngine_obtain(songplayer), timestamp);
});
EM_JS_PRFX(double, songplayer_get_duration, (SongPlayer songplayer), {
    return songplayer_get_duration(kdmyEngine_obtain(songplayer));
});
EM_JS_PRFX(bool, songplayer_is_completed, (SongPlayer songplayer), {
    return songplayer_is_completed(kdmyEngine_obtain(songplayer));
});
EM_JS_PRFX(double, songplayer_get_timestamp, (SongPlayer songplayer), {
    return songplayer_get_timestamp(kdmyEngine_obtain(songplayer));
});
EM_JS_PRFX(void, songplayer_mute_track, (SongPlayer songplayer, bool vocals_or_instrumental, bool muted), {
    songplayer_mute_track(kdmyEngine_obtain(songplayer), vocals_or_instrumental, muted);
});
EM_JS_PRFX(void, songplayer_mute, (SongPlayer songplayer, bool muted), {
    songplayer_mute(kdmyEngine_obtain(songplayer), muted);
});
EM_JS_PRFX(void, songplayer_set_volume_track, (SongPlayer songplayer, bool vocals_or_instrumental, float volume), {
    songplayer_set_volume_track(kdmyEngine_obtain(songplayer), vocals_or_instrumental, volume);
});
EM_JS_PRFX(void, songplayer_set_volume, (SongPlayer songplayer, float volume), {
    songplayer_set_volume(kdmyEngine_obtain(songplayer), volume);
});
#endif



static int script_songplayer_changesong(lua_State* L) {
    SongPlayer songplayer = luascript_read_userdata(L, SONGPLAYER);

    const char* src = luaL_checkstring(L, 2);
    bool prefer_alternative = lua_toboolean(L, 3);

    bool ret = songplayer_changesong(songplayer, src, prefer_alternative);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_songplayer_play(lua_State* L) {
    SongPlayer songplayer = luascript_read_userdata(L, SONGPLAYER);

    SongInfo_t songinfo_dummy;
    songplayer_play(songplayer, &songinfo_dummy);

    return 0;
}

static int script_songplayer_pause(lua_State* L) {
    SongPlayer songplayer = luascript_read_userdata(L, SONGPLAYER);

    songplayer_pause(songplayer);

    return 0;
}

static int script_songplayer_seek(lua_State* L) {
    SongPlayer songplayer = luascript_read_userdata(L, SONGPLAYER);

    double timestamp = luaL_checknumber(L, 2);

    songplayer_seek(songplayer, timestamp);

    return 0;
}

static int script_songplayer_get_duration(lua_State* L) {
    SongPlayer songplayer = luascript_read_userdata(L, SONGPLAYER);

    double duration = songplayer_get_duration(songplayer);

    lua_pushnumber(L, duration);
    return 1;
}

static int script_songplayer_is_completed(lua_State* L) {
    SongPlayer songplayer = luascript_read_userdata(L, SONGPLAYER);

    bool ret = songplayer_is_completed(songplayer);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_songplayer_get_timestamp(lua_State* L) {
    SongPlayer songplayer = luascript_read_userdata(L, SONGPLAYER);

    double timestamp = songplayer_get_timestamp(songplayer);

    lua_pushnumber(L, timestamp);
    return 1;
}

static int script_songplayer_mute_track(lua_State* L) {
    SongPlayer songplayer = luascript_read_userdata(L, SONGPLAYER);

    bool vocals_or_instrumental = lua_toboolean(L, 2);
    bool muted = lua_toboolean(L, 3);

    songplayer_mute_track(songplayer, vocals_or_instrumental, muted);

    return 0;
}

static int script_songplayer_mute(lua_State* L) {
    SongPlayer songplayer = luascript_read_userdata(L, SONGPLAYER);

    bool muted = lua_toboolean(L, 2);

    songplayer_mute(songplayer, muted);

    return 0;
}

static int script_songplayer_set_volume_track(lua_State* L) {
    SongPlayer songplayer = luascript_read_userdata(L, SONGPLAYER);
    bool vocals_or_instrumental = (bool)lua_toboolean(L, 2);
    float volume = (float)luaL_checknumber(L, 3);

    songplayer_set_volume_track(songplayer, vocals_or_instrumental, volume);

    return 0;
}

static int script_songplayer_set_volume(lua_State* L) {
    SongPlayer songplayer = luascript_read_userdata(L, SONGPLAYER);
    float volume = (float)luaL_checknumber(L, 2);

    songplayer_set_volume(songplayer, volume);

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const luaL_Reg SONGPLAYER_FUNCTIONS[] = {
    {"changesong", script_songplayer_changesong},
    {"play", script_songplayer_play},
    {"pause", script_songplayer_pause},
    {"seek", script_songplayer_seek},
    {"get_duration", script_songplayer_get_duration},
    {"is_completed", script_songplayer_is_completed},
    {"get_timestamp", script_songplayer_get_timestamp},
    {"mute_track", script_songplayer_mute_track},
    {"mute", script_songplayer_mute},
    { "set_volume_track", script_songplayer_set_volume_track },
    { "set_volume", script_songplayer_set_volume },
    {NULL, NULL}
};


int script_songplayer_new(lua_State* L, SongPlayer songplayer) {
    return luascript_userdata_new(L, SONGPLAYER, songplayer);
}

static int script_songplayer_gc(lua_State* L) {
    return luascript_userdata_gc(L, SONGPLAYER);
}

static int script_songplayer_tostring(lua_State* L) {
    return luascript_userdata_tostring(L, SONGPLAYER);
}


void script_songplayer_register(lua_State* L) {
    luascript_register(L, SONGPLAYER, script_songplayer_gc, script_songplayer_tostring, SONGPLAYER_FUNCTIONS);
}
