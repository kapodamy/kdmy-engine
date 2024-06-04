"use strict";

const MISSNOTEFX = "MissNoteFX";


function script_missnotefx_stop(L) {
    let missnotefx = luascript_read_userdata(L, MISSNOTEFX);

    missnotefx_stop(missnotefx);

    return 0;
}

function script_missnotefx_disable(L) {
    let missnotefx = luascript_read_userdata(L, MISSNOTEFX);
    let disabled = LUA.lua_toboolean(L, 2);

    missnotefx_disable(missnotefx, disabled);

    return 0;
}

function script_missnotefx_play_effect(L) {
    let missnotefx = luascript_read_userdata(L, MISSNOTEFX);

    missnotefx_play_effect(missnotefx);

    return 0;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/**@type {luaL_Reg[]}*/const MISSNOTEFX_FUNCTIONS = [
    ["missnotefx_stop", script_missnotefx_stop],
    ["missnotefx_disable", script_missnotefx_disable],
    ["missnotefx_play_effect", script_missnotefx_play_effect],
    [null, null]
];

function script_missnotefx_new(L, missnotefx) {
    return luascript_userdata_new(L, MISSNOTEFX, missnotefx);
}

function script_missnotefx_gc(L) {
    return luascript_userdata_gc(L, MISSNOTEFX);
}

function script_missnotefx_tostring(L) {
    return luascript_userdata_tostring(L, MISSNOTEFX);
}

function script_missnotefx_register(L) {
    luascript_register(L, MISSNOTEFX, script_missnotefx_gc, script_missnotefx_tostring, MISSNOTEFX_FUNCTIONS);
}

