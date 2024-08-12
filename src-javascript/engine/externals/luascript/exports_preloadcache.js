"use strict";

const PRELOADCACHE = "PreloadCache";


function script_preloadcache_clear(L) {
    // C & C# only
    // preloadcache_clear();

    return 0;
}

function script_preloadcache_flush(L) {
    let bytes_amount = LUA.luaL_checkinteger(L, 1);
    if (bytes_amount < 1) {
        return LUA.luaL_error(L, "the bytes amount can not be less than 0");
    }

    /*
    // C & C# only
    let ret = preloadcache_flush(bytes_amount);

    LUA.lua_pushboolean(L, ret);
    return 1;
    */

    LUA.lua_pushboolean(L, true);
    return 1;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

function script_preloadcache_register(L) {
    LUA.lua_pushcfunction(L, script_preloadcache_clear);
    LUA.lua_setglobal(L, "preloadcache_clear");
    LUA.lua_pushcfunction(L, script_preloadcache_flush);
    LUA.lua_setglobal(L, "preloadcache_flush");
}

