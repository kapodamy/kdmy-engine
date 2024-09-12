#include "luascript_internal.h"

#include "preloadcache.h"


static int script_preloadcache_clear(lua_State* L) {
    (void)L;

    preloadcache_clear();

    return 0;
}

static int script_preloadcache_flush(lua_State* L) {
    int32_t bytes_amount = luaL_checkinteger(L, 1);
    if (bytes_amount < 1) {
        return luaL_error(L, "the bytes amount can not be less than 0");
    }

    bool ret = preloadcache_flush((size_t)bytes_amount);

    lua_pushboolean(L, ret);
    return 1;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void script_preloadcache_register(lua_State* L) {
    lua_pushcfunction(L, script_preloadcache_clear);
    lua_setglobal(L, "preloadcache_clear");
    lua_pushcfunction(L, script_preloadcache_flush);
    lua_setglobal(L, "preloadcache_flush");
}
