using Engine.Externals.LuaInterop;
using Engine.Platform;

namespace Engine.Externals.LuaScriptInterop {

    public static class ExportsFS {

        static int script_fs_readfile(LuaState L) {
            string path = L.luaL_checkstring(1);

            byte[] buffer = FS.ReadArrayBuffer(path);

            if (buffer != null) {
                L.lua_pushlstring(buffer, buffer.Length);
                //free(buffer);
            } else {
                L.lua_pushnil();
            }

            return 1;
        }


        private static readonly LuaCallback readfile = script_fs_readfile;

        internal static void register_fs(ManagedLuaState lua) {
            lua.RegisterGlobalFunction("fs_readfile", readfile);
        }

    }
}
