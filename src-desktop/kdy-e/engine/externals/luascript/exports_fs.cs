using Engine.Externals.LuaInterop;
using Engine.Platform;

namespace Engine.Externals.LuaScriptInterop;

public static class ExportsFS {

    static int script_fs_readfile(LuaState L) {
        string src = L.luaL_checkstring(1);

        LuascriptHelpers.ChangeWorkingFolder(L);
        byte[] buffer = FS.ReadArrayBuffer(src);
        LuascriptHelpers.RestoreWorkingFolder(L);

        if (buffer != null) {
            L.lua_pushlstring(buffer, buffer.Length);
            //free(buffer);
        } else {
            L.lua_pushnil();
        }

        return 1;
    }

    static int script_fs_get_full_path(LuaState L) {
        string path = L.luaL_checkstring(1);

        LuascriptHelpers.ChangeWorkingFolder(L);
        string ret = FS.GetFullPath(path);
        LuascriptHelpers.RestoreWorkingFolder(L);

        L.lua_pushstring(ret);

        return 1;
    }

    private static readonly LuaCallback readfile = script_fs_readfile;
    private static readonly LuaCallback get_full_path = script_fs_get_full_path;

    internal static void script_fs_register(ManagedLuaState lua) {
        lua.RegisterGlobalFunction("fs_readfile", readfile);
        lua.RegisterGlobalFunction("fs_get_full_path", get_full_path);
    }

}
