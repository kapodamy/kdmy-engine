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
            object arg = LuascriptHelpers.ParseModdingValue(L, 3);

            object ret = modding.SpawnScreen(layout_src, script_src, arg);

            LuascriptHelpers.PushModdingValue(L, ret);
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

        static int script_modding_storage_get_blob(LuaState L) {
            Modding modding = (Modding)L.Context;

            string week_name = L.luaL_optstring(1, null);
            string name = L.luaL_checkstring(2);

            byte[] data;
            uint ret = modding.StorageGet(week_name, name, out data);

            if (data == null)
                L.lua_pushnil();
            else
                L.lua_pushlstring(data, (int)ret);

            return 1;
        }

        static int script_modding_storage_set_blob(LuaState L) {
            Modding modding = (Modding)L.Context;

            string week_name = L.luaL_optstring(1, null);
            string name = L.luaL_checkstring(2);
            byte[] data;
            uint data_size;

            if (L.lua_isnil(3)) {
                data = null;
                data_size = 0;
            } else {
                data = L.luaL_checklstring(3);
                data_size = data == null ? 0 : (uint)data.Length;
            }

            bool ret = modding.StorageSet(week_name, name, data, data_size);

            L.lua_pushboolean(ret);
            return 1;
        }

        static int script_modding_get_loaded_weeks(LuaState L) {
            Modding modding = (Modding)L.Context;

            int weeks_size;
            WeekInfo[] weeks = modding.GetLoadedWeeks(out weeks_size);

            if (weeks == null) {
                L.lua_pushnil();
                return 1;
            }

            L.lua_createtable(weeks_size, 0);
            for (int i = 0 ; i < weeks_size ; i++) {
                int songs_count = weeks[i].songs_count;
                LuaTableHelpers.AddFieldArrayItemAsTable(L, 0, 7, i + 1);
                {
                    LuaTableHelpers.AddFieldString(L, "name", weeks[i].name);
                    LuaTableHelpers.AddFieldString(L, "display_name", weeks[i].display_name);
                    LuaTableHelpers.AddFieldString(L, "description", weeks[i].description);
                    LuaTableHelpers.AddFieldString(L, "custom_folder", weeks[i].custom_folder);
                    LuaTableHelpers.AddFieldString(L, "unlock_directive", weeks[i].unlock_directive);
                    LuaTableHelpers.AddFieldString(L, "emit_directive", weeks[i].emit_directive);
                    LuaTableHelpers.AddFieldTable(L, "songs", songs_count, 0);
                    {
                        for (int j = 0 ; j < songs_count ; j++) {
                            LuaTableHelpers.AddFieldArrayItemAsTable(L, 0, 5, j + 1);
                            {
                                LuaTableHelpers.AddFieldBoolean(L, "freeplay_only", weeks[i].songs[j].freeplay_only);
                                LuaTableHelpers.AddFieldString(L, "name", weeks[i].songs[j].name);
                                LuaTableHelpers.AddFieldString(L, "freeplay_unlock_directive", weeks[i].songs[j].freeplay_unlock_directive);
                                LuaTableHelpers.AddFieldString(L, "freeplay_song_filename", weeks[i].songs[j].freeplay_song_filename);
                                LuaTableHelpers.AddFieldString(L, "freeplay_description", weeks[i].songs[j].freeplay_description);
                            }
                            L.lua_settable(-3);
                        }
                    }
                    L.lua_settable(-3);
                }
                L.lua_settable(-3);
            }

            return 1;
        }

        static int script_modding_launch_week(LuaState L) {
            Modding modding = (Modding)L.Context;

            string week_name = L.luaL_checkstring(1);
            string difficult = L.luaL_checkstring(2);
            bool alt_tracks = L.lua_toboolean(3);
            string bf = L.luaL_optstring(4, null);
            string gf = L.luaL_optstring(5, null);
            string gameplay_manifest = L.luaL_optstring(6, null);
            int song_idx = (int)L.luaL_optinteger(7, -1);

            int ret = modding.LaunchWeek(week_name, difficult, alt_tracks, bf, gf, gameplay_manifest, song_idx);

            L.lua_pushinteger(ret);
            return 1;
        }

        static int script_modding_launch_credits(LuaState L) {
            Modding modding = (Modding)L.Context;

            modding.LaunchCredits();

            return 0;
        }

        static int script_modding_launch_startscreen(LuaState L) {
            Modding modding = (Modding)L.Context;

            bool ret = modding.LaunchStartScreen();

            L.lua_pushboolean(ret);
            return 1;
        }

        static int script_modding_launch_mainmenu(LuaState L) {
            Modding modding = (Modding)L.Context;

            bool ret = modding.LaunchMainMenu();

            L.lua_pushboolean(ret);
            return 1;
        }

        static int script_modding_launch_settings(LuaState L) {
            Modding modding = (Modding)L.Context;

            modding.LaunchSettings();

            return 0;
        }

        static int script_modding_launch_freeplay(LuaState L) {
            Modding modding = (Modding)L.Context;

            modding.LaunchFreeplay();

            return 0;
        }

        static int script_modding_launch_weekselector(LuaState L) {
            Modding modding = (Modding)L.Context;

            modding.LaunchWeekSelector();

            return 0;
        }



        static readonly LuaTableFunction[] MODDING_FUNCTIONS = {
            new LuaTableFunction("modding_get_layout", script_modding_get_layout),
            new LuaTableFunction("modding_exit", script_modding_exit),
            new LuaTableFunction("modding_set_halt", script_modding_set_halt),
            new LuaTableFunction("modding_unlockdirective_create", script_modding_unlockdirective_create),
            new LuaTableFunction("modding_unlockdirective_remove", script_modding_unlockdirective_remove),
            new LuaTableFunction("modding_unlockdirective_get", script_modding_unlockdirective_get),
            new LuaTableFunction("modding_storage_get_blob", script_modding_storage_get_blob),
            new LuaTableFunction("modding_storage_set_blob", script_modding_storage_set_blob),
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
            new LuaTableFunction("modding_get_loaded_weeks", script_modding_get_loaded_weeks),
            new LuaTableFunction("modding_launch_week", script_modding_launch_week),
            new LuaTableFunction("modding_launch_credits", script_modding_launch_credits),
            new LuaTableFunction("modding_launch_startscreen", script_modding_launch_startscreen),
            new LuaTableFunction("modding_launch_mainmenu", script_modding_launch_mainmenu),
            new LuaTableFunction("modding_launch_settings", script_modding_launch_settings),
            new LuaTableFunction("modding_launch_freeplay", script_modding_launch_freeplay),
            new LuaTableFunction("modding_launch_weekselector", script_modding_launch_weekselector),
            new LuaTableFunction(null, null)
        };




        public static void script_modding_register(ManagedLuaState L) {
            L.RegisterGlobalFunctions(MODDING_FUNCTIONS);
        }

    }
}

