using System;
using System.Globalization;
using Engine.Animation;
using Engine.Externals.LuaInterop;
using Engine.Game;

namespace Engine.Externals.LuaScriptInterop {

    public static class ExportsEnvironment {

        public const string ENVIRONMENT = "Environment";


        static int script_environment_get_language(LuaState L) {
            L.lua_pushstring(CultureInfo.InstalledUICulture.DisplayName);
            return 1;
        }

        static int script_environment_get_username(LuaState L) {
            L.lua_pushstring(Environment.UserName);
            return 1;
        }

        static int script_environment_get_cmdargs(LuaState L) {
            L.lua_pushstring(Environment.CommandLine);
            return 1;
        }

        static int script_environment_exit(LuaState L) {
            int exit_code = (int)L.luaL_checkinteger(2);
            Environment.Exit(exit_code);
            return 0;
        }

        static int script_environment_change_window_title(LuaState L) {
            string title = L.luaL_optstring(2, null);
            LuascriptPlatform.ChangeWindowTitle(title, L.Context is Modding);
            return 0;
        }

        static int script_environment_require_window_attention(LuaState L) {
            LuascriptPlatform.RequestWindowAttention();
            return 0;
        }

        static int script_environment_open_www_link(LuaState L) {
            string url = L.luaL_checkstring(2);
            LuascriptPlatform.OpenWWWLink(url);
            return 0;
        }


        private static readonly LuaTableFunction[] ENVIRONMENT_FUNCTIONS = {
            new LuaTableFunction("get_language", script_environment_get_language),
            new LuaTableFunction("get_username", script_environment_get_username),
            new LuaTableFunction("get_cmdargs", script_environment_get_cmdargs),
            new LuaTableFunction("exit", script_environment_exit),
            new LuaTableFunction("change_window_title", script_environment_change_window_title),
            new LuaTableFunction("require_window_attention", script_environment_require_window_attention),
            new LuaTableFunction("open_www_link", script_environment_open_www_link),
            new LuaTableFunction(null, null),
        };

        static int script_enviroment_gc(LuaState L) {
            // nothind to do
            return 0;
        }

        static int script_enviroment_tostring(LuaState L) {
            string str = String.Format(
                "{{language=\"{0}\" username=\"{1}\" cmdargs=\"{2}\"}}",
                CultureInfo.InstalledUICulture.DisplayName,
                Environment.UserName,
                Environment.CommandLine
            );

            L.lua_pushstring(str);
            return 1;
        }

        private static readonly LuaCallback delegate_gc = script_enviroment_gc;
        private static readonly LuaCallback delegate_tostring = script_enviroment_tostring;

        public static void script_environment_register(ManagedLuaState L) {
            L.RegisterMetaTable(ENVIRONMENT, delegate_gc, delegate_tostring, ENVIRONMENT_FUNCTIONS);
        }

    }
}
