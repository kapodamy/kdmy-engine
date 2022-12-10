using System;
using Engine.Externals.LuaInterop;
using Engine.Game;

namespace Engine.Externals.LuaScriptInterop {

    public static class ExportsConductor {

        public static string CONDUCTOR = "Conductor";


        static int script_conductor_init(LuaState L) {
            Conductor conductor = new Conductor();
            return L.CreateAllocatedUserdata<Conductor>(CONDUCTOR, conductor);
        }

        static int script_conductor_destroy(LuaState L) {
            Conductor conductor = L.ReadUserdata<Conductor>(CONDUCTOR);

            if (L.IsUserdataAllocated(CONDUCTOR))
                conductor.Destroy();
            else
                Console.WriteLine("script_conductor_destroy() object was not allocated by lua");

            return 0;
        }

        static int script_conductor_poll_reset(LuaState L) {
            Conductor conductor = L.ReadUserdata<Conductor>(CONDUCTOR);

            conductor.PollReset();

            return 0;
        }

        static int script_conductor_set_character(LuaState L) {
            Conductor conductor = L.ReadUserdata<Conductor>(CONDUCTOR);
            Character character = L.ReadNullableUserdata<Character>(2, ExportsCharacter.CHARACTER);

            conductor.SetCharacter(character);

            return 0;
        }

        static int script_conductor_use_strum_line(LuaState L) {
            Conductor conductor = L.ReadUserdata<Conductor>(CONDUCTOR);
            Strum strum = L.ReadNullableUserdata<Strum>(2, ExportsStrum.STRUM);

            conductor.UseStrumLine(strum);

            return 0;
        }

        static int script_conductor_use_strums(LuaState L) {
            Conductor conductor = L.ReadUserdata<Conductor>(CONDUCTOR);
            Strums strums = L.ReadNullableUserdata<Strums>(2, ExportsStrums.STRUMS);

            conductor.UseStrums(strums);

            return 0;
        }

        static int script_conductor_disable_strum_line(LuaState L) {
            Conductor conductor = L.ReadUserdata<Conductor>(CONDUCTOR);
            Strum strum = L.ReadNullableUserdata<Strum>(2, ExportsStrum.STRUM);
            bool should_disable = L.lua_toboolean(3);

            bool ret = conductor.DisableStrumLine(strum, should_disable);

            L.lua_pushboolean(ret);
            return 1;
        }

        static int script_conductor_remove_strum(LuaState L) {
            Conductor conductor = L.ReadUserdata<Conductor>(CONDUCTOR);
            Strum strum = L.ReadNullableUserdata<Strum>(2, ExportsStrum.STRUM);

            bool ret = conductor.RemoveStrum(strum);

            L.lua_pushboolean(ret);
            return 1;
        }

        static int script_conductor_clear_mapping(LuaState L) {
            Conductor conductor = L.ReadUserdata<Conductor>(CONDUCTOR);

            conductor.ClearMapping();

            return 0;
        }

        static int script_conductor_map_strum_to_player_sing_add(LuaState L) {
            Conductor conductor = L.ReadUserdata<Conductor>(CONDUCTOR);
            Strum strum = L.ReadNullableUserdata<Strum>(2, ExportsStrum.STRUM);
            string sing_direction_name = L.luaL_optstring(3, null);

            conductor.MapStrumToPlayerSingAdd(strum, sing_direction_name);

            return 0;
        }

        static int script_conductor_map_strum_to_player_extra_add(LuaState L) {
            Conductor conductor = L.ReadUserdata<Conductor>(CONDUCTOR);
            Strum strum = L.ReadNullableUserdata<Strum>(2, ExportsStrum.STRUM);
            string extra_animation_name = L.luaL_optstring(3, null);

            conductor.MapStrumToPlayerExtraAdd(strum, extra_animation_name);

            return 0;
        }

        static int script_conductor_map_strum_to_player_sing_remove(LuaState L) {
            Conductor conductor = L.ReadUserdata<Conductor>(CONDUCTOR);
            Strum strum = L.ReadNullableUserdata<Strum>(2, ExportsStrum.STRUM);
            string sing_direction_name = L.luaL_optstring(3, null);

            conductor.MapStrumToPlayerSingRemove(strum, sing_direction_name);

            return 0;
        }

        static int script_conductor_map_strum_to_player_extra_remove(LuaState L) {
            Conductor conductor = L.ReadUserdata<Conductor>(CONDUCTOR);
            Strum strum = L.ReadNullableUserdata<Strum>(2, ExportsStrum.STRUM);
            string extra_animation_name = L.luaL_optstring(3, null);

            conductor.MapStrumToPlayerExtraRemove(strum, extra_animation_name);

            return 0;
        }

        static int script_conductor_map_automatically(LuaState L) {
            Conductor conductor = L.ReadUserdata<Conductor>(CONDUCTOR);
            bool should_map_extras = L.lua_toboolean(2);

            int ret = conductor.MapAutomatically(should_map_extras);

            L.lua_pushinteger(ret);
            return 1;
        }

        static int script_conductor_poll(LuaState L) {
            Conductor conductor = L.ReadUserdata<Conductor>(CONDUCTOR);

            conductor.Poll();

            return 0;
        }

        static int script_conductor_disable(LuaState L) {
            Conductor conductor = L.ReadUserdata<Conductor>(CONDUCTOR);
            bool disable = L.lua_toboolean(2);

            conductor.Disable(disable);

            return 0;
        }

        static int script_conductor_play_idle(LuaState L) {
            Conductor conductor = L.ReadUserdata<Conductor>(CONDUCTOR);

            conductor.PlayIdle();

            return 0;
        }

        static int script_conductor_play_hey(LuaState L) {
            Conductor conductor = L.ReadUserdata<Conductor>(CONDUCTOR);

            conductor.PlayHey();

            return 0;
        }

        static int script_conductor_get_character(LuaState L) {
            Conductor conductor = L.ReadUserdata<Conductor>(CONDUCTOR);

            Character character = conductor.GetCharacter();

            return ExportsCharacter.script_character_new(L, character);
        }




        static readonly LuaTableFunction[] CONDUCTOR_FUNCTIONS = {
            new LuaTableFunction("init", script_conductor_init),
            new LuaTableFunction("destroy", script_conductor_destroy),
            new LuaTableFunction("poll_reset", script_conductor_poll_reset),
            new LuaTableFunction("set_character", script_conductor_set_character),
            new LuaTableFunction("use_strum_line", script_conductor_use_strum_line),
            new LuaTableFunction("use_strums", script_conductor_use_strums),
            new LuaTableFunction("disable_strum_line", script_conductor_disable_strum_line),
            new LuaTableFunction("remove_strum", script_conductor_remove_strum),
            new LuaTableFunction("clear_mapping", script_conductor_clear_mapping),
            new LuaTableFunction("map_strum_to_player_sing_add", script_conductor_map_strum_to_player_sing_add),
            new LuaTableFunction("map_strum_to_player_extra_add", script_conductor_map_strum_to_player_extra_add),
            new LuaTableFunction("map_strum_to_player_sing_remove", script_conductor_map_strum_to_player_sing_remove),
            new LuaTableFunction("map_strum_to_player_extra_remove", script_conductor_map_strum_to_player_extra_remove),
            new LuaTableFunction("map_automatically", script_conductor_map_automatically),
            new LuaTableFunction("poll", script_conductor_poll),
            new LuaTableFunction("disable", script_conductor_disable),
            new LuaTableFunction("play_idle", script_conductor_play_idle),
            new LuaTableFunction("play_hey", script_conductor_play_hey),
            new LuaTableFunction("get_character", script_conductor_get_character),
            new LuaTableFunction(null, null)
        };

        public static int script_conductor_new(LuaState L, Conductor conductor) {
            return L.CreateUserdata(CONDUCTOR, conductor);
        }

        static int script_conductor_gc(LuaState L) {
            return L.DestroyUserdata(CONDUCTOR);
        }

        static int script_conductor_tostring(LuaState L) {
            return L.ToString_userdata(CONDUCTOR);
        }

        private static readonly LuaCallback delegate_gc = script_conductor_gc;
        private static readonly LuaCallback delegate_tostring = script_conductor_tostring;

        public static void script_conductor_register(ManagedLuaState L) {
            L.RegisterMetaTable(CONDUCTOR, delegate_gc, delegate_tostring, CONDUCTOR_FUNCTIONS);
        }

    }
}

