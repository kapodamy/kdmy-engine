"use strict";

const SOUNDPLAYER = "SoundPlayer";


async function script_soundplayer_init(L) {
    let src = LUA.luaL_checkstring(L, 2);

    luascript_change_working_folder(L);
    let ret = await soundplayer_init(src);
    luascript_restore_working_folder(L);

    return luascript_userdata_allocnew(L, SOUNDPLAYER, ret);
}

function script_soundplayer_destroy(L) {
    let soundplayer = luascript_read_userdata(L, SOUNDPLAYER);

    if (luascript_userdata_is_allocated(L, SOUNDPLAYER))
        soundplayer_destroy(soundplayer);
    else
        console.error("script_soundplayer_destroy() object was not allocated by lua");

    return 0;
}

function script_soundplayer_play(L) {
    let soundplayer = luascript_read_userdata(L, SOUNDPLAYER);

    soundplayer_play(soundplayer);

    return 0;
}

function script_soundplayer_pause(L) {
    let soundplayer = luascript_read_userdata(L, SOUNDPLAYER);

    soundplayer_pause(soundplayer);

    return 0;
}

function script_soundplayer_stop(L) {
    let soundplayer = luascript_read_userdata(L, SOUNDPLAYER);

    soundplayer_stop(soundplayer);
    return 0;
}

function script_soundplayer_loop_enable(L) {
    let soundplayer = luascript_read_userdata(L, SOUNDPLAYER);

    let enable = LUA.lua_toboolean(L, 2);

    soundplayer_loop_enable(soundplayer, enable);

    return 0;
}

function script_soundplayer_fade(L) {
    let soundplayer = luascript_read_userdata(L, SOUNDPLAYER);

    let in_or_out = LUA.lua_toboolean(L, 2);
    let duration = LUA.luaL_checknumber(L, 3);

    soundplayer_fade(soundplayer, in_or_out, duration);

    return 0;
}

function script_soundplayer_set_volume(L) {
    let soundplayer = luascript_read_userdata(L, SOUNDPLAYER);

    let volume = LUA.luaL_checknumber(L, 2);

    soundplayer_set_volume(soundplayer, volume);

    return 0;
}

function script_soundplayer_set_mute(L) {
    let soundplayer = luascript_read_userdata(L, SOUNDPLAYER);

    let muted = LUA.lua_toboolean(L, 2);

    soundplayer_set_mute(soundplayer, muted);

    return 0;
}

function script_soundplayer_has_fading(L) {
    let soundplayer = luascript_read_userdata(L, SOUNDPLAYER);

    let ret = soundplayer_has_fading(soundplayer);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_soundplayer_is_muted(L) {
    let soundplayer = luascript_read_userdata(L, SOUNDPLAYER);

    let ret = soundplayer_is_muted(soundplayer);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_soundplayer_is_playing(L) {
    let soundplayer = luascript_read_userdata(L, SOUNDPLAYER);

    let ret = soundplayer_is_playing(soundplayer);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_soundplayer_get_duration(L) {
    let soundplayer = luascript_read_userdata(L, SOUNDPLAYER);

    let ret = soundplayer_get_duration(soundplayer);

    LUA.lua_pushnumber(L, ret);
    return 1;
}

function script_soundplayer_get_position(L) {
    let soundplayer = luascript_read_userdata(L, SOUNDPLAYER);

    let ret = soundplayer_get_position(soundplayer);

    LUA.lua_pushnumber(L, ret);
    return 1;
}

function script_soundplayer_seek(L) {
    let soundplayer = luascript_read_userdata(L, SOUNDPLAYER);

    let timestamp = LUA.luaL_checknumber(L, 2);

    soundplayer_seek(soundplayer, timestamp);

    return 0;
}

function script_soundplayer_has_ended(L) {
    let soundplayer = luascript_read_userdata(L, SOUNDPLAYER);

    let ret = soundplayer_has_ended(soundplayer);

    LUA.lua_pushboolean(L, ret);
    return 1;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/**@type {luaL_Reg[]}*/const SOUNDPLAYER_FUNCTIONS = [
    ["init", script_soundplayer_init],
    ["destroy", script_soundplayer_destroy],
    ["play", script_soundplayer_play],
    ["pause", script_soundplayer_pause],
    ["stop", script_soundplayer_stop],
    ["loop_enable", script_soundplayer_loop_enable],
    ["fade", script_soundplayer_fade],
    ["set_volume", script_soundplayer_set_volume],
    ["set_mute", script_soundplayer_set_mute],
    ["has_fading", script_soundplayer_has_fading],
    ["is_muted", script_soundplayer_is_muted],
    ["is_playing", script_soundplayer_is_playing],
    ["get_duration", script_soundplayer_get_duration],
    ["get_position", script_soundplayer_get_position],
    ["seek", script_soundplayer_seek],
    ["has_ended", script_soundplayer_has_ended],
    [null, null]
];


function script_soundplayer_new(L, soundplayer) {
    return luascript_userdata_new(L, SOUNDPLAYER, soundplayer);
}

function script_soundplayer_gc(L) {
    // if this object was allocated by lua, call the destructor
    return luascript_userdata_destroy(L, SOUNDPLAYER, soundplayer_destroy);
}

function script_soundplayer_tostring(L) {
    return luascript_userdata_tostring(L, SOUNDPLAYER);
}

function script_soundplayer_register(L) {
    luascript_register(L, SOUNDPLAYER, script_soundplayer_gc, script_soundplayer_tostring, SOUNDPLAYER_FUNCTIONS);
}

