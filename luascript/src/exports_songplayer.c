#include "luascript_internal.h"



#ifdef JAVASCRIPT
EM_JS_PRFX(bool, songplayer_changesong, (SongPlayer songplayer, const char* src, bool prefer_no_copyright), {
    return songplayer_changesong(kdmyEngine_obtain(songplayer), kdmyEngine_ptrToString(src), prefer_no_copyright);
});
EM_ASYNC_JS_PRFX(void, songplayer_play, (SongPlayer songplayer, SongInfo_t* songinfo), {
    const _songinfo = {completed: 0, timestamp: 0};
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
EM_JS_PRFX(void, songplayer_mute_track, (SongPlayer songplayer, bool vocals_or_instrumental, bool muted),{
    songplayer_mute_track(kdmyEngine_obtain(songplayer), vocals_or_instrumental, muted);
});
EM_JS_PRFX(void, songplayer_mute, (SongPlayer songplayer, bool muted),{
    songplayer_mute(kdmyEngine_obtain(songplayer), muted);
});
#endif



static int script_songplayer_changesong(lua_State* L) {
    READ_USERDATA(L, SongPlayer, songplayer, SONGPLAYER);

    const char* src = luaL_checkstring(L, 2);
    bool prefer_no_copyright = luaL_checkboolean(L, 3);

    bool ret = songplayer_changesong(songplayer, src, prefer_no_copyright);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_songplayer_play(lua_State* L) {
    READ_USERDATA(L, SongPlayer, songplayer, SONGPLAYER);

    SongInfo_t songinfo_dummy;
    songplayer_play(songplayer, &songinfo_dummy);

    return 0;
}

static int script_songplayer_pause(lua_State* L) {
    READ_USERDATA(L, SongPlayer, songplayer, SONGPLAYER);

    songplayer_pause(songplayer);

    return 0;
}

static int script_songplayer_seek(lua_State* L) {
    READ_USERDATA(L, SongPlayer, songplayer, SONGPLAYER);

    double timestamp = luaL_checkfloat(L, 2);

    songplayer_seek(songplayer, timestamp);

    return 0;
}

static int script_songplayer_get_duration(lua_State* L) {
    READ_USERDATA(L, SongPlayer, songplayer, SONGPLAYER);

    double duration = songplayer_get_duration(songplayer);

    lua_pushnumber(L, duration);
    return 1;
}

static int script_songplayer_is_completed(lua_State* L) {
    READ_USERDATA(L, SongPlayer, songplayer, SONGPLAYER);

    bool ret = songplayer_is_completed(songplayer);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_songplayer_get_timestamp(lua_State* L) {
    READ_USERDATA(L, SongPlayer, songplayer, SONGPLAYER);

    double timestamp = songplayer_get_timestamp(songplayer);

    lua_pushnumber(L, timestamp);
    return 1;
}

static int script_songplayer_mute_track(lua_State* L) {
    READ_USERDATA(L, SongPlayer, songplayer, SONGPLAYER);

    bool vocals_or_instrumental = luaL_checkboolean(L, 2);
    bool muted = luaL_checkboolean(L, 3);

    songplayer_mute_track(songplayer, vocals_or_instrumental, muted);

    return 0;
}

static int script_songplayer_mute(lua_State* L) {
    READ_USERDATA(L, SongPlayer, songplayer, SONGPLAYER);

    bool muted = luaL_checkboolean(L, 2);

    songplayer_mute(songplayer, muted);

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
    {NULL, NULL}
};


int script_songplayer_new(lua_State* L, SongPlayer songplayer) {
    return NEW_USERDATA(L, SONGPLAYER, NULL, songplayer, true);
}

static int script_songplayer_gc(lua_State* L) {
    READ_USERDATA_UNCHECKED(L, SongPlayer, songplayer, SONGPLAYER);
    _luascript_suppress_item(L, songplayer, true);
    return 0;
}

static int script_songplayer_tostring(lua_State* L) {
    READ_USERDATA(L, SongPlayer, songplayer, SONGPLAYER);
    lua_pushstring(L, "[SongPlayer]");
    return 1;
}


inline void register_songplayer(lua_State* L) {
    _luascript_register(
        L,
        SONGPLAYER,
        script_songplayer_gc,
        script_songplayer_tostring,
        SONGPLAYER_FUNCTIONS
    );
}

