using Engine.Externals.LuaInterop;
using Engine.Game;

namespace Engine.Externals.LuaScriptInterop {

    public static class ExportsModding {

        static int script_modding_unlockdirective_create(LuaState L) {
            Modding moddingcontext = (Modding)L.Context;

            string name = L.luaL_optstring(1, null);
            double value = (double)L.luaL_checknumber(2);

            moddingcontext.UnlockdirectiveCreate(name, value);

            return 0;
        }

        static int script_modding_unlockdirective_remove(LuaState L) {

            Modding moddingcontext = (Modding)L.Context;

            string name = L.luaL_optstring(1, null);

            moddingcontext.UnlockdirectiveRemove(name);

            return 0;
        }

        static int script_modding_unlockdirective_get(LuaState L) {

            Modding moddingcontext = (Modding)L.Context;

            string name = L.luaL_optstring(1, null);

            if (!moddingcontext.UnlockdirectiveHas(name)) {
                L.lua_pushnil();
                return 1;
            }

            double ret = moddingcontext.UnlockdirectiveGet(name);

            L.lua_pushnumber(ret);
            return 1;
        }

        static int script_modding_set_ui_visibility(LuaState L) {

            Modding moddingcontext = (Modding)L.Context;

            bool visible = L.lua_toboolean(1);

            moddingcontext.SetUiVisibility(visible);

            return 0;
        }

        static int script_modding_get_layout(LuaState L) {

            Modding moddingcontext = (Modding)L.Context;


            Layout ret = moddingcontext.GetLayout();

            return ExportsLayout.script_layout_new(L, ret);
        }

        static int script_modding_exit(LuaState L) {

            Modding moddingcontext = (Modding)L.Context;


            moddingcontext.Exit();

            return 0;
        }

        static int script_modding_set_halt(LuaState L) {

            Modding moddingcontext = (Modding)L.Context;

            bool halt = L.lua_toboolean(1);

            moddingcontext.SetHalt(halt);

            return 0;
        }



        static readonly LuaTableFunction[] MODDING_FUNCTIONS = {
            new LuaTableFunction("modding_set_ui_visibility", script_modding_set_ui_visibility),
            new LuaTableFunction("modding_get_layout", script_modding_get_layout),
            new LuaTableFunction("modding_exit", script_modding_exit),
            new LuaTableFunction("modding_set_halt", script_modding_set_halt),
            new LuaTableFunction("modding_unlockdirective_create", script_modding_unlockdirective_create),
            new LuaTableFunction("modding_unlockdirective_remove", script_modding_unlockdirective_remove),
            new LuaTableFunction("modding_unlockdirective_get", script_modding_unlockdirective_get),
            new LuaTableFunction(null, null)
        };




        public static void script_modding_register(ManagedLuaState L) {
            L.RegisterGlobalFunctions(MODDING_FUNCTIONS);
        }

    }
}

