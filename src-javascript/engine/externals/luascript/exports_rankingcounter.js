"use strict";

const RANKINGCOUNTER = "RankingCounter";


function script_rankingcounter_add_state(L) {
    let rankingcounter = luascript_read_userdata(L, RANKINGCOUNTER);
    let modelholder = luascript_read_nullable_userdata(L, 2, MODELHOLDER);
    let state_name = LUA.luaL_optstring(L, 3, null);

    let ret = rankingcounter_add_state(rankingcounter, modelholder, state_name);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_rankingcounter_toggle_state(L) {
    let rankingcounter = luascript_read_userdata(L, RANKINGCOUNTER);
    let state_name = LUA.luaL_optstring(L, 2, null);

    rankingcounter_toggle_state(rankingcounter, state_name);

    return 0;
}

function script_rankingcounter_reset(L) {
    let rankingcounter = luascript_read_userdata(L, RANKINGCOUNTER);

    rankingcounter_reset(rankingcounter);

    return 0;
}

function script_rankingcounter_hide_accuracy(L) {
    let rankingcounter = luascript_read_userdata(L, RANKINGCOUNTER);
    let hide = LUA.lua_toboolean(L, 2);

    rankingcounter_hide_accuracy(rankingcounter, hide);

    return 0;
}

function script_rankingcounter_use_percent_instead(L) {
    let rankingcounter = luascript_read_userdata(L, RANKINGCOUNTER);
    let use_accuracy_percenter = LUA.lua_toboolean(L, 2);

    rankingcounter_use_percent_instead(rankingcounter, use_accuracy_percenter);

    return 0;
}

function script_rankingcounter_set_default_ranking_animation2(L) {
    let rankingcounter = luascript_read_userdata(L, RANKINGCOUNTER);
    let animsprite = luascript_read_nullable_userdata(L, 2, ANIMSPRITE);

    rankingcounter_set_default_ranking_animation2(rankingcounter, animsprite);

    return 0;
}

function script_rankingcounter_set_default_ranking_text_animation2(L) {
    let rankingcounter = luascript_read_userdata(L, RANKINGCOUNTER);
    let animsprite = luascript_read_nullable_userdata(L, 2, ANIMSPRITE);

    rankingcounter_set_default_ranking_text_animation2(rankingcounter, animsprite);

    return 0;
}

function script_rankingcounter_set_alpha(L) {
    let rankingcounter = luascript_read_userdata(L, RANKINGCOUNTER);
    let alpha = LUA.luaL_checknumber(L, 2);

    rankingcounter_set_alpha(rankingcounter, alpha);

    return 0;
}

function script_rankingcounter_animation_set(L) {
    let rankingcounter = luascript_read_userdata(L, RANKINGCOUNTER);
    let animsprite = luascript_read_nullable_userdata(L, 2, ANIMSPRITE);

    rankingcounter_animation_set(rankingcounter, animsprite);

    return 0;
}

function script_rankingcounter_animation_restart(L) {
    let rankingcounter = luascript_read_userdata(L, RANKINGCOUNTER);

    rankingcounter_animation_restart(rankingcounter);

    return 0;
}

function script_rankingcounter_animation_end(L) {
    let rankingcounter = luascript_read_userdata(L, RANKINGCOUNTER);

    rankingcounter_animation_end(rankingcounter);

    return 0;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/**@type {luaL_Reg[]}*/const RANKINGCOUNTER_FUNCTIONS = [
    ["add_state", script_rankingcounter_add_state],
    ["toggle_state", script_rankingcounter_toggle_state],
    ["reset", script_rankingcounter_reset],
    ["hide_accuracy", script_rankingcounter_hide_accuracy],
    ["use_percent_instead", script_rankingcounter_use_percent_instead],
    ["set_default_ranking_animation2", script_rankingcounter_set_default_ranking_animation2],
    ["set_default_ranking_text_animation2", script_rankingcounter_set_default_ranking_text_animation2],
    ["set_alpha", script_rankingcounter_set_alpha],
    ["animation_set", script_rankingcounter_animation_set],
    ["animation_restart", script_rankingcounter_animation_restart],
    ["animation_end", script_rankingcounter_animation_end],
    [null, null]
];

function script_rankingcounter_new(L, rankingcounter) {
    return luascript_userdata_new(L, RANKINGCOUNTER, rankingcounter);
}

function script_rankingcounter_gc(L) {
    return luascript_userdata_gc(L, RANKINGCOUNTER);
}

function script_rankingcounter_tostring(L) {
    return luascript_userdata_tostring(L, RANKINGCOUNTER);
}

function script_rankingcounter_register(L) {
    luascript_register(L, RANKINGCOUNTER, script_rankingcounter_gc, script_rankingcounter_tostring, RANKINGCOUNTER_FUNCTIONS);
}

