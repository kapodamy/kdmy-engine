using Engine.Externals.LuaInterop;
using Engine.Game;

namespace Engine.Externals.LuaScriptInterop {

    public static class ExportsModding {

        static int script_modding_get_layout(LuaState L) {
            Modding modding = (Modding)L.Context;

            Layout layout = modding.layout;

            return ExportsLayout.script_layout_new(L, layout);
        }

        static int script_modding_exit(LuaState L) {
            Modding modding = (Modding)L.Context;

            modding.Exit();

            return 0;
        }


        static readonly LuaTableFunction[] EXPORTS_FUNCTION = {
            new LuaTableFunction() { name = "modding_get_layout", func = script_modding_get_layout },
            new LuaTableFunction() { name = "modding_exit", func = script_modding_exit },
            new LuaTableFunction() { name = null, func =  null }
        };


        internal static void register_modding(ManagedLuaState lua) {
            lua.RegisterGlobalFunctions(EXPORTS_FUNCTION);
        }

    }
}
