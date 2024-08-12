using Engine.Externals.LuaInterop;
using Engine.Platform;

namespace Engine.Externals.LuaScriptInterop;

public static class ExportsPreloadCache {

    public const string PRELOADCACHE = "PreloadCache";


    private static int script_preloadcache_clear(LuaState L) {
        PreloadCache.Clear();

        return 0;
    }

    private static int script_preloadcache_flush(LuaState L) {
        long bytes_amount = L.luaL_checkinteger(1);
        if (bytes_amount < 1) {
            return L.luaL_error("the bytes amount can not be less than 0");
        }

        bool ret = PreloadCache.Flush(bytes_amount);

        L.lua_pushboolean(ret);
        return 1;
    }


    ////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////

    private static readonly LuaCallback preloadcache_clear = script_preloadcache_clear;
    private static readonly LuaCallback preloadcache_flush = script_preloadcache_flush;

    public static void script_preloadcache_register(ManagedLuaState L) {
        L.RegisterGlobalFunction("preloadcache_clear", preloadcache_clear);
        L.RegisterGlobalFunction("preloadcache_flush", preloadcache_flush);
    }

}