using Engine.Externals.LuaInterop;
using Engine.Game;
using Engine.Game.Common;
using Engine.Sound;
using Engine.Utils;

namespace Engine.Externals.LuaScriptInterop {

    public static class ExportsModding {

        static int script_modding_unlockdirective_create(LuaState L) {
            Modding modding = (Modding)L.Context;

            string name = L.luaL_optstring(1, null);
            double value = (double)L.luaL_checknumber(2);

            modding.UnlockdirectiveCreate(name, value);

            return 0;
        }

        static int script_modding_unlockdirective_remove(LuaState L) {

            Modding modding = (Modding)L.Context;

            string name = L.luaL_optstring(1, null);

            modding.UnlockdirectiveRemove(name);

            return 0;
        }

        static int script_modding_unlockdirective_get(LuaState L) {

            Modding modding = (Modding)L.Context;

            string name = L.luaL_optstring(1, null);

            if (!modding.UnlockdirectiveHas(name)) {
                L.lua_pushnil();
                return 1;
            }

            double ret = modding.UnlockdirectiveGet(name);

            L.lua_pushnumber(ret);
            return 1;
        }

        static int script_modding_set_ui_visibility(LuaState L) {

            Modding modding = (Modding)L.Context;

            bool visible = L.lua_toboolean(1);

            modding.SetUiVisibility(visible);

            return 0;
        }

        static int script_modding_get_layout(LuaState L) {

            Modding modding = (Modding)L.Context;


            Layout ret = modding.GetLayout();

            return ExportsLayout.script_layout_new(L, ret);
        }

        static int script_modding_exit(LuaState L) {

            Modding modding = (Modding)L.Context;


            modding.Exit();

            return 0;
        }

        static int script_modding_set_halt(LuaState L) {

            Modding modding = (Modding)L.Context;

            bool halt = L.lua_toboolean(1);

            modding.SetHalt(halt);

            return 0;
        }

        static int script_modding_get_active_menu(LuaState L) {
            Modding modding = (Modding)L.Context;

            return ExportsMenu.script_menu_new(L, modding.GetActiveMenu());
        }

        static int script_modding_choose_native_menu_option(LuaState L) {
            Modding modding = (Modding)L.Context;

            string name = L.luaL_optstring(1, null);

            bool ret = modding.ChooseNativeMenuOption(name);
            L.lua_pushboolean(ret);

            return 1;
        }

        static int script_modding_get_native_menu(LuaState L) {
            Modding modding = (Modding)L.Context;

            return ExportsMenu.script_menu_new(L, modding.GetNativeMenu());
        }

        static int script_modding_set_active_menu(LuaState L) {
            Modding modding = (Modding)L.Context;

            Menu menu = L.ReadNullableUserdata<Menu>(1, ExportsMenu.MENU);

            modding.SetActiveMenu(menu);

            return 0;
        }

        static int script_modding_get_native_background_music(LuaState L) {
            Modding modding = (Modding)L.Context;

            SoundPlayer ret = modding.GetNativeBackgroundMusic();

            return ExportsSoundPlayer.script_soundplayer_new(L, ret);
        }

        static int script_modding_replace_native_background_music(LuaState L) {
            Modding modding = (Modding)L.Context;

            string music_src = L.luaL_optstring(1, null);

            SoundPlayer ret = modding.ReplaceNativeBackgroundMusic(music_src);

            return ExportsSoundPlayer.script_soundplayer_new(L, ret);
        }

        static int script_modding_spawn_screen(LuaState L) {
            Modding modding = (Modding)L.Context;

            string layout_src = L.lua_tostring(1);
            string script_src = L.luaL_optstring(2, null);
            object arg = LuascriptHelpers.ToBasicValue(L, 3);

            object ret = modding.SpawnScreen(layout_src, script_src, arg);

            LuascriptHelpers.PushBasicValue(L, ret);
            return 1;
        }

        static int script_modding_set_exit_delay(LuaState L) {
            Modding modding = (Modding)L.Context;

            double delay_ms = L.lua_tonumber(1);

            modding.SetExitDelay(delay_ms);

            return 0;
        }

        static int script_modding_get_messagebox(LuaState L) {
            Modding modding = (Modding)L.Context;

            MessageBox ret = modding.GetMessageBox();

            return ExportsMessageBox.script_messagebox_new(L, ret);
        }

        static int script_modding_set_menu_in_layout_placeholder(LuaState L) {
            Modding modding = (Modding)L.Context;

            string placeholder_name = L.lua_tostring(1);
            Menu menu = L.ReadNullableUserdata<Menu>(2, ExportsMenu.MENU);

            modding.SetMenuInLayoutPlaceholder(placeholder_name, menu);

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
            new LuaTableFunction("modding_get_active_menu", script_modding_get_active_menu),
            new LuaTableFunction("modding_choose_native_menu_option", script_modding_choose_native_menu_option),
            new LuaTableFunction("modding_get_native_menu", script_modding_get_native_menu),
            new LuaTableFunction("modding_set_active_menu", script_modding_set_active_menu),
            new LuaTableFunction("modding_get_native_background_music", script_modding_get_native_background_music),
            new LuaTableFunction("modding_replace_native_background_music", script_modding_replace_native_background_music),
            new LuaTableFunction("modding_spawn_screen", script_modding_spawn_screen),
            new LuaTableFunction("modding_set_exit_delay", script_modding_set_exit_delay),
            new LuaTableFunction("modding_get_messagebox", script_modding_get_messagebox),
            new LuaTableFunction("modding_set_menu_in_layout_placeholder", script_modding_set_menu_in_layout_placeholder),
            new LuaTableFunction(null, null)
        };




        public static void script_modding_register(ManagedLuaState L) {
            L.RegisterGlobalFunctions(MODDING_FUNCTIONS);
        }

    }
}

