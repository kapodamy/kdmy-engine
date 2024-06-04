"use strict";

const VIDEOPLAYER = "VideoPlayer";


function script_videoplayer_get_sprite(L) {
    let videoplayer = luascript_read_userdata(L, VIDEOPLAYER);

    let ret = videoplayer_get_sprite(videoplayer);

    return script_sprite_new(L, ret);
}

function script_videoplayer_replay(L) {
    let videoplayer = luascript_read_userdata(L, VIDEOPLAYER);

    videoplayer_replay(videoplayer);

    return 0;
}

function script_videoplayer_play(L) {
    let videoplayer = luascript_read_userdata(L, VIDEOPLAYER);

    videoplayer_play(videoplayer);

    return 0;
}

function script_videoplayer_pause(L) {
    let videoplayer = luascript_read_userdata(L, VIDEOPLAYER);

    videoplayer_pause(videoplayer);

    return 0;
}

function script_videoplayer_stop(L) {
    let videoplayer = luascript_read_userdata(L, VIDEOPLAYER);

    videoplayer_stop(videoplayer);

    return 0;
}

function script_videoplayer_loop_enable(L) {
    let videoplayer = luascript_read_userdata(L, VIDEOPLAYER);
    let enable = LUA.lua_toboolean(L, 2);

    videoplayer_loop_enable(videoplayer, enable);

    return 0;
}

function script_videoplayer_fade_audio(L) {
    let videoplayer = luascript_read_userdata(L, VIDEOPLAYER);
    let in_or_out = LUA.lua_toboolean(L, 2);
    let duration = LUA.luaL_checknumber(L, 3);

    videoplayer_fade_audio(videoplayer, in_or_out, duration);

    return 0;
}

function script_videoplayer_set_volume(L) {
    let videoplayer = luascript_read_userdata(L, VIDEOPLAYER);
    let volume = LUA.luaL_checknumber(L, 2);

    videoplayer_set_volume(videoplayer, volume);

    return 0;
}

function script_videoplayer_set_mute(L) {
    let videoplayer = luascript_read_userdata(L, VIDEOPLAYER);
    let muted = LUA.lua_toboolean(L, 2);

    videoplayer_set_mute(videoplayer, muted);

    return 0;
}

function script_videoplayer_seek(L) {
    let videoplayer = luascript_read_userdata(L, VIDEOPLAYER);
    let timestamp = LUA.luaL_checknumber(L, 2);

    videoplayer_seek(videoplayer, timestamp);

    return 0;
}

function script_videoplayer_is_muted(L) {
    let videoplayer = luascript_read_userdata(L, VIDEOPLAYER);

    let ret = videoplayer_is_muted(videoplayer);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_videoplayer_is_playing(L) {
    let videoplayer = luascript_read_userdata(L, VIDEOPLAYER);

    let ret = videoplayer_is_playing(videoplayer);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_videoplayer_get_duration(L) {
    let videoplayer = luascript_read_userdata(L, VIDEOPLAYER);

    let ret = videoplayer_get_duration(videoplayer);

    LUA.lua_pushnumber(L, ret);
    return 1;
}

function script_videoplayer_get_position(L) {
    let videoplayer = luascript_read_userdata(L, VIDEOPLAYER);

    let ret = videoplayer_get_position(videoplayer);

    LUA.lua_pushnumber(L, ret);
    return 1;
}

function script_videoplayer_has_ended(L) {
    let videoplayer = luascript_read_userdata(L, VIDEOPLAYER);

    let ret = videoplayer_has_ended(videoplayer);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_videoplayer_has_video_track(L) {
    let videoplayer = luascript_read_userdata(L, VIDEOPLAYER);

    let ret = videoplayer_has_video_track(videoplayer);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_videoplayer_has_audio_track(L) {
    let videoplayer = luascript_read_userdata(L, VIDEOPLAYER);

    let ret = videoplayer_has_audio_track(videoplayer);

    LUA.lua_pushboolean(L, ret);
    return 1;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/**@type {luaL_Reg[]}*/const VIDEOPLAYER_FUNCTIONS = [
    ["get_sprite", script_videoplayer_get_sprite],
    ["replay", script_videoplayer_replay],
    ["play", script_videoplayer_play],
    ["pause", script_videoplayer_pause],
    ["stop", script_videoplayer_stop],
    ["loop_enable", script_videoplayer_loop_enable],
    ["fade_audio", script_videoplayer_fade_audio],
    ["set_volume", script_videoplayer_set_volume],
    ["set_mute", script_videoplayer_set_mute],
    ["seek", script_videoplayer_seek],
    ["is_muted", script_videoplayer_is_muted],
    ["is_playing", script_videoplayer_is_playing],
    ["get_duration", script_videoplayer_get_duration],
    ["get_position", script_videoplayer_get_position],
    ["has_ended", script_videoplayer_has_ended],
    ["has_video_track", script_videoplayer_has_video_track],
    ["has_audio_track", script_videoplayer_has_audio_track],
    [null, null]
];

function script_videoplayer_new(L, videoplayer) {
    return luascript_userdata_new(L, VIDEOPLAYER, videoplayer);
}

function script_videoplayer_gc(L) {
    return luascript_userdata_gc(L, VIDEOPLAYER);
}

function script_videoplayer_tostring(L) {
    return luascript_userdata_tostring(L, VIDEOPLAYER);
}

function script_videoplayer_register(L) {
    luascript_register(L, VIDEOPLAYER, script_videoplayer_gc, script_videoplayer_tostring, VIDEOPLAYER_FUNCTIONS);
}

