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
            new LuaTableFunction("modding_get_layout", script_modding_get_layout),
            new LuaTableFunction("modding_exit", script_modding_exit),
            new LuaTableFunction(null, null)
        };


        internal static void script_modding_register(ManagedLuaState lua) {
            lua.RegisterGlobalFunctions(EXPORTS_FUNCTION);
        }

    }
}
