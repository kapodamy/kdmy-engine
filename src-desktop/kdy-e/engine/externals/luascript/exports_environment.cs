using System;
using System.Globalization;
using Engine.Animation;
using Engine.Externals.LuaInterop;

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
            int exit_code = (int)L.luaL_checkinteger(1);
            Environment.Exit(exit_code);
            return 0;
        }


        private static readonly LuaTableFunction[] ENVIRONMENT_FUNCTIONS = {
            new LuaTableFunction("get_language", script_environment_get_language),
            new LuaTableFunction("get_username", script_environment_get_username),
            new LuaTableFunction("get_cmdargs", script_environment_get_cmdargs),
            new LuaTableFunction("exit", script_environment_exit),
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
