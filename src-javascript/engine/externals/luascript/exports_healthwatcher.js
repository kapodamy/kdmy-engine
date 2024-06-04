"use strict";

const HEALTHWATCHER = "HealthbarWatcher";


function script_healthwatcher_add_opponent(L) {
    let healthwatcher = luascript_read_userdata(L, HEALTHWATCHER);
    let playerstats = luascript_read_nullable_userdata(L, 2, PLAYERSTATS);
    let can_recover = LUA.lua_toboolean(L, 3);
    let can_die = LUA.lua_toboolean(L, 4);

    let ret = healthwatcher_add_opponent(healthwatcher, playerstats, can_recover, can_die);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_healthwatcher_add_player(L) {
    let healthwatcher = luascript_read_userdata(L, HEALTHWATCHER);
    let playerstats = luascript_read_nullable_userdata(L, 2, PLAYERSTATS);
    let can_recover = LUA.lua_toboolean(L, 3);
    let can_die = LUA.lua_toboolean(L, 4);

    let ret = healthwatcher_add_player(healthwatcher, playerstats, can_recover, can_die);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_healthwatcher_has_deads(L) {
    let healthwatcher = luascript_read_userdata(L, HEALTHWATCHER);
    let in_players_or_opponents = LUA.lua_toboolean(L, 2);

    let ret = healthwatcher_has_deads(healthwatcher, in_players_or_opponents);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_healthwatcher_enable_dead(L) {
    let healthwatcher = luascript_read_userdata(L, HEALTHWATCHER);
    let playerstats = luascript_read_nullable_userdata(L, 2, PLAYERSTATS);
    let can_die = LUA.lua_toboolean(L, 3);

    let ret = healthwatcher_enable_dead(healthwatcher, playerstats, can_die);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_healthwatcher_enable_recover(L) {
    let healthwatcher = luascript_read_userdata(L, HEALTHWATCHER);
    let playerstats = luascript_read_nullable_userdata(L, 2, PLAYERSTATS);
    let can_recover = LUA.lua_toboolean(L, 3);

    let ret = healthwatcher_enable_recover(healthwatcher, playerstats, can_recover);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_healthwatcher_clear(L) {
    let healthwatcher = luascript_read_userdata(L, HEALTHWATCHER);

    healthwatcher_clear(healthwatcher);

    return 0;
}

function script_healthwatcher_balance(L) {
    let healthwatcher = luascript_read_userdata(L, HEALTHWATCHER);
    let healthbar = luascript_read_nullable_userdata(L, 2, HEALTHBAR);

    healthwatcher_balance(healthwatcher, healthbar);

    return 0;
}

function script_healthwatcher_reset_opponents(L) {
    let healthwatcher = luascript_read_userdata(L, HEALTHWATCHER);

    healthwatcher_reset_opponents(healthwatcher);

    return 0;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/**@type {luaL_Reg[]}*/const HEALTHWATCHER_FUNCTIONS = [
    ["add_opponent", script_healthwatcher_add_opponent],
    ["add_player", script_healthwatcher_add_player],
    ["has_deads", script_healthwatcher_has_deads],
    ["enable_dead", script_healthwatcher_enable_dead],
    ["enable_recover", script_healthwatcher_enable_recover],
    ["clear", script_healthwatcher_clear],
    ["balance", script_healthwatcher_balance],
    ["reset_opponents", script_healthwatcher_reset_opponents],
    [null, null]
];


function script_healthwatcher_new(L, healthwatcher) {
    return luascript_userdata_new(L, HEALTHWATCHER, healthwatcher);
}

function script_healthwatcher_gc(L) {
    return luascript_userdata_gc(L, HEALTHWATCHER);
}

function script_healthwatcher_tostring(L) {
    return luascript_userdata_tostring(L, HEALTHWATCHER);
}

function script_healthwatcher_register(L) {
    luascript_register(L, HEALTHWATCHER, script_healthwatcher_gc, script_healthwatcher_tostring, HEALTHWATCHER_FUNCTIONS);
}
