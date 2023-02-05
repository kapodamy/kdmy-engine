using System;
using System.Collections.Generic;
using System.Diagnostics;
using CsharpWrapper;
using Engine.Externals.LuaInterop;
using Engine.Game;
using Engine.Platform;

namespace Engine.Externals.LuaScriptInterop {

    public class Luascript {

        private static readonly LinkedList<Luascript> instances = new LinkedList<Luascript>();

        private readonly ManagedLuaState lua;
        private readonly LuaState L;

        private Luascript(ManagedLuaState lua) {
            this.lua = lua;
            this.L = lua.LuaStateHandle;
        }


        private static void RegisterObjects(ManagedLuaState lua, bool is_week) {
            lua.RegisterConstantString("ENGINE_NAME", GameMain.ENGINE_NAME);
            lua.RegisterConstantString("ENGINE_VERSION", GameMain.ENGINE_VERSION);

            lua.RegisterConstantInteger("GAMEPAD_A", (uint)GamepadButtons.A);
            lua.RegisterConstantInteger("GAMEPAD_B", (uint)GamepadButtons.B);
            lua.RegisterConstantInteger("GAMEPAD_X", (uint)GamepadButtons.X);
            lua.RegisterConstantInteger("GAMEPAD_Y", (uint)GamepadButtons.Y);
            lua.RegisterConstantInteger("GAMEPAD_DPAD_UP", (uint)GamepadButtons.DPAD_UP);
            lua.RegisterConstantInteger("GAMEPAD_DPAD_DOWN", (uint)GamepadButtons.DPAD_DOWN);
            lua.RegisterConstantInteger("GAMEPAD_DPAD_RIGHT", (uint)GamepadButtons.DPAD_RIGHT);
            lua.RegisterConstantInteger("GAMEPAD_DPAD_LEFT", (uint)GamepadButtons.DPAD_LEFT);
            lua.RegisterConstantInteger("GAMEPAD_START", (uint)GamepadButtons.START);
            lua.RegisterConstantInteger("GAMEPAD_SELECT", (uint)GamepadButtons.SELECT);
            lua.RegisterConstantInteger("GAMEPAD_TRIGGER_LEFT", (uint)GamepadButtons.TRIGGER_LEFT);
            lua.RegisterConstantInteger("GAMEPAD_TRIGGER_RIGHT", (uint)GamepadButtons.TRIGGER_RIGHT);
            lua.RegisterConstantInteger("GAMEPAD_BUMPER_LEFT", (uint)GamepadButtons.BUMPER_LEFT);
            lua.RegisterConstantInteger("GAMEPAD_BUMPER_RIGHT", (uint)GamepadButtons.BUMPER_RIGHT);
            lua.RegisterConstantInteger("GAMEPAD_APAD_UP", (uint)GamepadButtons.APAD_UP);
            lua.RegisterConstantInteger("GAMEPAD_APAD_DOWN", (uint)GamepadButtons.APAD_DOWN);
            lua.RegisterConstantInteger("GAMEPAD_APAD_RIGHT", (uint)GamepadButtons.APAD_RIGHT);
            lua.RegisterConstantInteger("GAMEPAD_APAD_LEFT", (uint)GamepadButtons.APAD_LEFT);
            lua.RegisterConstantInteger("GAMEPAD_DPAD2_UP", (uint)GamepadButtons.DPAD2_UP);
            lua.RegisterConstantInteger("GAMEPAD_DPAD2_DOWN", (uint)GamepadButtons.DPAD2_DOWN);
            lua.RegisterConstantInteger("GAMEPAD_DPAD2_RIGHT", (uint)GamepadButtons.DPAD2_RIGHT);
            lua.RegisterConstantInteger("GAMEPAD_DPAD2_LEFT", (uint)GamepadButtons.DPAD2_LEFT);
            lua.RegisterConstantInteger("GAMEPAD_DPAD3_UP", (uint)GamepadButtons.DPAD3_UP);
            lua.RegisterConstantInteger("GAMEPAD_DPAD3_DOWN", (uint)GamepadButtons.DPAD3_DOWN);
            lua.RegisterConstantInteger("GAMEPAD_DPAD3_RIGHT", (uint)GamepadButtons.DPAD3_RIGHT);
            lua.RegisterConstantInteger("GAMEPAD_DPAD3_LEFT", (uint)GamepadButtons.DPAD3_LEFT);
            lua.RegisterConstantInteger("GAMEPAD_DPAD4_UP", (uint)GamepadButtons.DPAD4_UP);
            lua.RegisterConstantInteger("GAMEPAD_DPAD4_DOWN", (uint)GamepadButtons.DPAD4_DOWN);
            lua.RegisterConstantInteger("GAMEPAD_DPAD4_RIGHT", (uint)GamepadButtons.DPAD4_RIGHT);
            lua.RegisterConstantInteger("GAMEPAD_DPAD4_LEFT", (uint)GamepadButtons.DPAD4_LEFT);
            lua.RegisterConstantInteger("GAMEPAD_BACK", (uint)GamepadButtons.BACK);


            // register all objects (metatables) and functions
            ExportsCamera.script_camera_register(lua);
            ExportsCharacter.script_character_register(lua);
            ExportsConductor.script_conductor_register(lua);
            ExportsLayout.script_layout_register(lua);
            ExportsLayoutPlaceholder.script_layoutplaceholder_register(lua);
            ExportsMessageBox.script_messagebox_register(lua);
            ExportsModifier.script_modifier_register(lua);
            ExportsSoundPlayer.script_soundplayer_register(lua);
            ExportsVideoPlayer.script_videoplayer_register(lua);
            ExportsSprite.script_sprite_register(lua);
            ExportsTextSprite.script_textsprite_register(lua);
            ExportsPSShader.script_psshader_register(lua);
            ExportsTweenLerp.script_tweenlerp_register(lua);
            ExportsTweenKeyframe.script_tweenkeyframe_register(lua);
            ExportsAtlas.script_atlas_register(lua);
            ExportsAnimList.script_animlist_register(lua);
            ExportsAnimListItem.script_animlistitem_register(lua);
            ExportsAnimSprite.script_animsprite_register(lua);
            ExportsModelHolder.script_modelholder_register(lua);

            if (is_week) {
                ExportsSongPlayer.script_songplayer_register(lua);
                ExportsDialogue.script_dialogue_register(lua);
                ExportsCountdown.script_countdown_register(lua);
                ExportsHealthBar.script_healthbar_register(lua);
                ExportsHealthWatcher.script_healthwatcher_register(lua);
                ExportsMissNoteFX.script_missnotefx_register(lua);
                ExportsPlayerStats.script_playerstats_register(lua);
                ExportsRankingCounter.script_rankingcounter_register(lua);
                ExportsRoundStats.script_roundstats_register(lua);
                ExportsSongProgressbar.script_songprogressbar_register(lua);
                ExportsStreakCounter.script_streakcounter_register(lua);
                ExportsStrum.script_strum_register(lua);
                ExportsStrums.script_strums_register(lua);
                ExportsWeek.script_week_register(lua);
            } else {
                ExportsMenuManifest.script_menumanifest_register(lua);
                ExportsMenu.script_menu_register(lua);
                ExportsModding.script_modding_register(lua);
            }

            ExportsMath2D.script_math2d_register(lua);
            ExportsTimer.script_timer_register(lua);
            ExportsFS.script_fs_register(lua);
            ExportsScript.script_script_register(lua);
            ExportsEngineSettings.script_enginesettings_register(lua);
            ExportsEnvironment.script_environment_register(lua);

            // set engine settings
            lua.RegisterConstantUserdata<object>("Settings", ExportsEngineSettings.script_enginesettings_new, EngineSettings.ini);
        }

        private static void RegisterSandbox(ManagedLuaState lua) {
            const string SANDBOX =
                "debug.debug = nil\n" +
                "debug.getfenv = getfenv\n" +
                "debug.getregistry = nil\n" +
                "debug = nil\n" +
                "dofile = nil\n" +
                "io = nil\n" +
                //"load = nil\n" +
                "loadfile = nil\n" +
                "dofile = nil\n" +
                "os.execute = nil\n" +
                "os.getenv = nil\n" +
                "os.remove = nil\n" +
                "os.tmpname = nil\n" +
                "os.setlocale = nil\n" +
                "os.rename = nil\n" +
                //"os.exit = nil\n" +
                //"loadstring = nil\n" +
                "package.loaded.io = nil\n" +
                "package.loaded.package = nil\n" +
                "package.cpath = nil\n" +
                "package.loaded = nil\n" +
                "package.loaders= nil\n" +
                "package.loadlib= nil\n" +
                "package.path= nil\n" +
                "package.preload= nil\n" +
                "package.seeall= nil\n" +
                "package.searchpath= nil\n" +
                "package.searchers= nil\n" +
                "package = nil\n" +
                //"require = nil\n" +
                "newproxy = nil\n";


            bool result = lua.EvaluateString(SANDBOX);
            Debug.Assert(result, "luascript_register_sandbox() failed");
        }

        public static Luascript Init(string lua_sourcecode, string filename, object context, bool is_week) {
            ManagedLuaState lua = ManagedLuaState.Init(context);

            if (lua == null) {
                Console.Error.WriteLine("[ERROR] luascript_init() cannot create lua state, not enough memory");
                return null;
            }

            RegisterObjects(lua, is_week);
            RegisterSandbox(lua);

            int status = lua.EvaluateString(lua_sourcecode, filename);

            if (status != ManagedLuaState.LUA_OK) {
                string error_message = lua.LuaStateHandle.lua_tostring(-1);
                Console.Error.WriteLine($"luascript_init() luaL_loadfile() failed: {error_message}");

                lua.Dispose();
                return null;
            }

            return new Luascript(lua);
        }

        public void Destroy() {
            this.lua.Dispose();
            Luascript.instances.Remove(this);
        }

        public static void DropShared(object obj) {
            foreach (Luascript luascript in Luascript.instances) {
                luascript.lua.DropSharedObject(obj);
            }
        }

        public bool Eval(string eval_string) {
            if (String.IsNullOrEmpty(eval_string)) return true;
            return this.lua.EvaluateString(eval_string);
        }

        public void CallFunction(string function_name) {
            if (lua.PushGlobalFunction(function_name)) return;
            lua.CallPushedGlobalFunction(0);
        }



        public void notify_weekinit(int freeplay_track_index) {
            const string FUNCTION = "f_weekinit";
            if (lua.PushGlobalFunction(FUNCTION)) return;


            L.lua_pushinteger(freeplay_track_index);

            lua.CallPushedGlobalFunction(1);
        }

        public void notify_beforeready(bool from_retry) {
            const string FUNCTION = "f_beforeready";
            if (lua.PushGlobalFunction(FUNCTION)) return;


            L.lua_pushboolean(from_retry);

            lua.CallPushedGlobalFunction(1);
        }

        public void notify_ready() {
            const string FUNCTION = "f_ready";
            if (lua.PushGlobalFunction(FUNCTION)) return;

            lua.CallPushedGlobalFunction(0);
        }

        public void notify_aftercountdown() {
            const string FUNCTION = "f_aftercountdown";
            if (lua.PushGlobalFunction(FUNCTION)) return;

            lua.CallPushedGlobalFunction(0);
        }

        public void notify_frame(float elapsed) {
            const string FUNCTION = "f_frame";
            if (lua.PushGlobalFunction(FUNCTION)) return;


            L.lua_pushnumber(elapsed);

            lua.CallPushedGlobalFunction(1);
        }

        public void notify_roundend(bool loose) {
            const string FUNCTION = "f_roundend";
            if (lua.PushGlobalFunction(FUNCTION)) return;


            L.lua_pushboolean(loose);

            lua.CallPushedGlobalFunction(1);
        }

        public void notify_weekend(bool giveup) {
            const string FUNCTION = "f_weekend";
            if (lua.PushGlobalFunction(FUNCTION)) return;


            L.lua_pushboolean(giveup);

            lua.CallPushedGlobalFunction(1);
        }

        public void notify_diedecision(bool retry_or_giveup) {
            const string FUNCTION = "f_diedecision";
            if (lua.PushGlobalFunction(FUNCTION)) return;


            L.lua_pushboolean(retry_or_giveup);

            lua.CallPushedGlobalFunction(1);
        }

        public void notify_pause(bool pause_or_resume) {
            const string FUNCTION = "f_pause";
            if (lua.PushGlobalFunction(FUNCTION)) return;


            L.lua_pushboolean(pause_or_resume);

            lua.CallPushedGlobalFunction(1);
        }

        public void notify_weekleave() {
            const string FUNCTION = "f_weekleave";
            if (lua.PushGlobalFunction(FUNCTION)) return;

            lua.CallPushedGlobalFunction(0);
        }

        public void notify_afterresults() {
            const string FUNCTION = "f_afterresults";
            if (lua.PushGlobalFunction(FUNCTION)) return;

            lua.CallPushedGlobalFunction(0);
        }

        public void notify_scriptchange() {
            const string FUNCTION = "f_scriptchange";
            if (lua.PushGlobalFunction(FUNCTION)) return;

            lua.CallPushedGlobalFunction(0);
        }

        public void notify_pause_optionselected(int option_index, uint buttons) {
            const string FUNCTION = "f_pause_optionselected";
            if (lua.PushGlobalFunction(FUNCTION)) return;


            L.lua_pushinteger(option_index);
            L.lua_pushinteger(buttons);

            lua.CallPushedGlobalFunction(2);
        }

        public void notify_pause_menuvisible(bool shown_or_hidden) {
            const string FUNCTION = "f_pause_menuvisible";
            if (lua.PushGlobalFunction(FUNCTION)) return;


            L.lua_pushboolean(shown_or_hidden);

            lua.CallPushedGlobalFunction(1);
        }

        public void notify_note(double timestamp, int id, double duration, double data, bool special, int player_id, uint state) {
            const string FUNCTION = "f_note";
            if (lua.PushGlobalFunction(FUNCTION)) return;


            L.lua_pushnumber(timestamp);
            L.lua_pushinteger(id);
            L.lua_pushnumber(duration);
            L.lua_pushnumber(data);
            L.lua_pushboolean(special);
            L.lua_pushinteger(player_id);
            L.lua_pushinteger(state);

            lua.CallPushedGlobalFunction(7);
        }

        public void notify_buttons(int player_id, uint buttons) {
            const string FUNCTION = "f_buttons";
            if (lua.PushGlobalFunction(FUNCTION)) return;


            L.lua_pushinteger(player_id);
            L.lua_pushinteger(buttons);

            lua.CallPushedGlobalFunction(2);
        }

        public void notify_unknownnote(int player_id, double timestamp, int id, double duration, double data) {
            const string FUNCTION = "f_unknownnote";
            if (lua.PushGlobalFunction(FUNCTION)) return;


            L.lua_pushinteger(player_id);
            L.lua_pushnumber(timestamp);
            L.lua_pushinteger(id);
            L.lua_pushnumber(duration);
            L.lua_pushnumber(data);

            lua.CallPushedGlobalFunction(5);
        }

        public void notify_beat(int current_beat, float since) {
            const string FUNCTION = "f_beat";
            if (lua.PushGlobalFunction(FUNCTION)) return;


            L.lua_pushinteger(current_beat);
            L.lua_pushnumber(since);

            lua.CallPushedGlobalFunction(2);
        }

        public void notify_quarter(int current_quarter, float since) {
            const string FUNCTION = "f_quarter";
            if (lua.PushGlobalFunction(FUNCTION)) return;


            L.lua_pushinteger(current_quarter);
            L.lua_pushnumber(since);

            lua.CallPushedGlobalFunction(2);
        }

        public void notify_timer_run(double kos_timestamp) {
            const string FUNCTION = "______kdmyEngine_timer_run";
            if (lua.PushGlobalFunction(FUNCTION)) return;


            L.lua_pushnumber(kos_timestamp);
            L.lua_pushboolean(false);

            lua.CallPushedGlobalFunction(2);
        }

        public void notify_timersong_run(double song_timestamp) {
            const string FUNCTION = "______kdmyEngine_timer_run";
            if (lua.PushGlobalFunction(FUNCTION)) return;


            L.lua_pushnumber(song_timestamp);
            L.lua_pushboolean(true);

            lua.CallPushedGlobalFunction(2);
        }

        public void notify_dialogue_builtin_open(string dialog_src) {
            const string FUNCTION = "f_dialogue_line_starts";
            if (lua.PushGlobalFunction(FUNCTION)) return;


            L.lua_pushstring(dialog_src);

            lua.CallPushedGlobalFunction(1);
        }

        public void notify_dialogue_line_starts(int line_index, string state_name, string text) {
            const string FUNCTION = "f_dialogue_line_starts";
            if (lua.PushGlobalFunction(FUNCTION)) return;


            L.lua_pushinteger(line_index);
            L.lua_pushstring(state_name);
            L.lua_pushstring(text);

            lua.CallPushedGlobalFunction(3);
        }

        public void notify_dialogue_line_ends(int line_index, string state_name, string text) {
            const string FUNCTION = "f_dialogue_line_ends";
            if (lua.PushGlobalFunction(FUNCTION)) return;


            L.lua_pushinteger(line_index);
            L.lua_pushstring(state_name);
            L.lua_pushstring(text);

            lua.CallPushedGlobalFunction(3);
        }

        public void notify_after_strum_scroll() {
            const string FUNCTION = "f_after_strum_scroll";
            if (lua.PushGlobalFunction(FUNCTION)) return;

            lua.CallPushedGlobalFunction(0);
        }

        public void notify_modding_menu_option_selected(object menu, int index, string name) {
            const string FUNCTION = "f_modding_menu_option_selected";
            if (lua.PushGlobalFunction(FUNCTION)) return;

            L.CreateUserdata(ExportsMenu.MENU, menu);
            L.lua_pushinteger(index);
            L.lua_pushstring(name);

            lua.CallPushedGlobalFunction(3);
        }

        public bool notify_modding_menu_option_choosen(object menu, int index, string name) {
            const string FUNCTION = "f_modding_menu_option_choosen";
            if (lua.PushGlobalFunction(FUNCTION)) return false;

            L.CreateUserdata(ExportsMenu.MENU, menu);
            L.lua_pushinteger(index);
            L.lua_pushstring(name);

            return (bool)lua.CallPushedGlobalFunctionWithReturn(3);
        }

        public bool notify_modding_back() {
            const string FUNCTION = "f_modding_back";
            if (lua.PushGlobalFunction(FUNCTION)) return false;

            return (bool)lua.CallPushedGlobalFunctionWithReturn(0);
        }

        public object notify_modding_exit() {
            const string FUNCTION = "f_modding_exit";
            if (lua.PushGlobalFunction(FUNCTION)) return false;

            return lua.CallPushedGlobalFunctionWithReturn(0);
        }

        public void notify_modding_init(object value) {
            const string FUNCTION = "f_modding_init";
            if (lua.PushGlobalFunction(FUNCTION)) return;

            LuascriptHelpers.PushModdingValue(lua.LuaStateHandle, value);

            lua.CallPushedGlobalFunction(1);
        }

        public void notify_modding_event(string event_name) {
            const string FUNCTION = "f_modding_event";
            if (lua.PushGlobalFunction(FUNCTION)) return;

            lua.LuaStateHandle.lua_pushstring(event_name);

            lua.CallPushedGlobalFunction(1);
        }
    }

}