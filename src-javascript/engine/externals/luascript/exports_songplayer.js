"use strict";

const SONGPLAYER = "SongPlayer";


async function script_songplayer_changesong(L) {
    let songplayer = luascript_read_userdata(L, SONGPLAYER);

    let src = LUA.luaL_checkstring(L, 2);
    let prefer_alternative = LUA.lua_toboolean(L, 3);

    luascript_change_working_folder(L);
    let ret = await songplayer_changesong(songplayer, src, prefer_alternative);
    luascript_restore_working_folder(L);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

async function script_songplayer_play(L) {
    let songplayer = luascript_read_userdata(L, SONGPLAYER);

    const songinfo_dummy = { timestamp: 0, completed: false };
    await songplayer_play(songplayer, songinfo_dummy);

    return 0;
}

function script_songplayer_pause(L) {
    let songplayer = luascript_read_userdata(L, SONGPLAYER);

    songplayer_pause(songplayer);

    return 0;
}

function script_songplayer_seek(L) {
    let songplayer = luascript_read_userdata(L, SONGPLAYER);

    let timestamp = LUA.luaL_checknumber(L, 2);

    songplayer_seek(songplayer, timestamp);

    return 0;
}

function script_songplayer_get_duration(L) {
    let songplayer = luascript_read_userdata(L, SONGPLAYER);

    let duration = songplayer_get_duration(songplayer);

    LUA.lua_pushnumber(L, duration);
    return 1;
}

function script_songplayer_is_completed(L) {
    let songplayer = luascript_read_userdata(L, SONGPLAYER);

    let ret = songplayer_is_completed(songplayer);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_songplayer_get_timestamp(L) {
    let songplayer = luascript_read_userdata(L, SONGPLAYER);

    let timestamp = songplayer_get_timestamp(songplayer);

    LUA.lua_pushnumber(L, timestamp);
    return 1;
}

function script_songplayer_mute_track(L) {
    let songplayer = luascript_read_userdata(L, SONGPLAYER);

    let vocals_or_instrumental = LUA.lua_toboolean(L, 2);
    let muted = LUA.lua_toboolean(L, 3);

    songplayer_mute_track(songplayer, vocals_or_instrumental, muted);

    return 0;
}

function script_songplayer_mute(L) {
    let songplayer = luascript_read_userdata(L, SONGPLAYER);

    let muted = LUA.lua_toboolean(L, 2);

    songplayer_mute(songplayer, muted);

    return 0;
}

function script_songplayer_set_volume_track(L) {
    let songplayer = luascript_read_userdata(L, SONGPLAYER);
    let vocals_or_instrumental = LUA.lua_toboolean(L, 2);
    let volume = LUA.luaL_checknumber(L, 3);

    songplayer_set_volume_track(songplayer, vocals_or_instrumental, volume);

    return 0;
}

function script_songplayer_set_volume(L) {
    let songplayer = luascript_read_userdata(L, SONGPLAYER);
    let volume = LUA.luaL_checknumber(L, 2);

    songplayer_set_volume(songplayer, volume);

    return 0;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/**@type {luaL_Reg[]}*/const SONGPLAYER_FUNCTIONS = [
    ["changesong", script_songplayer_changesong],
    ["play", script_songplayer_play],
    ["pause", script_songplayer_pause],
    ["seek", script_songplayer_seek],
    ["get_duration", script_songplayer_get_duration],
    ["is_completed", script_songplayer_is_completed],
    ["get_timestamp", script_songplayer_get_timestamp],
    ["mute_track", script_songplayer_mute_track],
    ["mute", script_songplayer_mute],
    ["set_volume_track", script_songplayer_set_volume_track],
    ["set_volume", script_songplayer_set_volume],
    [null, null]
];


function script_songplayer_new(L, songplayer) {
    return luascript_userdata_new(L, SONGPLAYER, songplayer);
}

function script_songplayer_gc(L) {
    return luascript_userdata_gc(L, SONGPLAYER);
}

function script_songplayer_tostring(L) {
    return luascript_userdata_tostring(L, SONGPLAYER);
}


function script_songplayer_register(L) {
    luascript_register(L, SONGPLAYER, script_songplayer_gc, script_songplayer_tostring, SONGPLAYER_FUNCTIONS);
}
