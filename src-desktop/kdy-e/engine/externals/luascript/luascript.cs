using System;
using System.Collections.Generic;
using System.Diagnostics;
using CsharpWrapper;
using Engine.Externals.LuaInterop;
using Engine.Game;
using Engine.Platform;
using Engine.Sound;

namespace Engine.Externals.LuaScriptInterop;

public partial class Luascript {

    private static readonly LinkedList<Luascript> instances = new LinkedList<Luascript>();

    private readonly ManagedLuaState lua;
    private readonly LuaState L;
    private readonly bool is_week;

    private Luascript(ManagedLuaState lua, bool is_week) {
        this.lua = lua;
        this.L = lua.LuaStateHandle;
        this.is_week = is_week;
    }


    private static void RegisterObjects(ManagedLuaState lua, bool is_week) {
        lua.RegisterConstantString("ENGINE_NAME", GameMain.ENGINE_NAME);
        lua.RegisterConstantString("ENGINE_VERSION", GameMain.ENGINE_VERSION);

        lua.RegisterConstantString("StrumScriptTarget_MARKER", "marker");
        lua.RegisterConstantString("StrumScriptTarget_SICK_EFFECT", "sick_effect");
        lua.RegisterConstantString("StrumScriptTarget_BACKGROUND", "background");
        lua.RegisterConstantString("StrumScriptTarget_STRUM_LINE", "strum_line");
        lua.RegisterConstantString("StrumScriptTarget_NOTE", "note");
        lua.RegisterConstantString("StrumScriptTarget_ALL", "all");

        lua.RegisterConstantString("StrumScriptOn_HIT_DOWN", "on_hit_down");
        lua.RegisterConstantString("StrumScriptOn_HIT_UP", "on_hit_up");
        lua.RegisterConstantString("StrumScriptOn_MISS", "on_miss");
        lua.RegisterConstantString("StrumScriptOn_PENALITY", "on_penality");
        lua.RegisterConstantString("StrumScriptOn_IDLE", "on_idle");
        lua.RegisterConstantString("StrumScriptOn_ALL", "on_all");

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

        lua.RegisterConstantInteger("MouseButton_BUTTON1", 0);
        lua.RegisterConstantInteger("MouseButton_BUTTON2", 1);
        lua.RegisterConstantInteger("MouseButton_BUTTON3", 2);
        lua.RegisterConstantInteger("MouseButton_BUTTON4", 3);
        lua.RegisterConstantInteger("MouseButton_BUTTON5", 4);
        lua.RegisterConstantInteger("MouseButton_BUTTON6", 5);
        lua.RegisterConstantInteger("MouseButton_BUTTON7", 6);
        lua.RegisterConstantInteger("MouseButton_BUTTON8", 7);

        lua.RegisterConstantInteger("ModKeys_Shift", 0x0001);
        lua.RegisterConstantInteger("ModKeys_Control", 0x0002);
        lua.RegisterConstantInteger("ModKeys_Alt", 0x0004);
        lua.RegisterConstantInteger("ModKeys_Super", 0x0008);
        lua.RegisterConstantInteger("ModKeys_CapsLock", 0x0010);
        lua.RegisterConstantInteger("ModKeys_NumLock", 0x0020);

        lua.RegisterConstantInteger("Keys_Unknown", -1);
        lua.RegisterConstantInteger("Keys_Space", 32);
        lua.RegisterConstantInteger("Keys_Apostrophe", 39);
        lua.RegisterConstantInteger("Keys_Comma", 44);
        lua.RegisterConstantInteger("Keys_Minus", 45);
        lua.RegisterConstantInteger("Keys_Period", 46);
        lua.RegisterConstantInteger("Keys_Slash", 47);
        lua.RegisterConstantInteger("Keys_Numpad0", 48);
        lua.RegisterConstantInteger("Keys_Numpad1", 49);
        lua.RegisterConstantInteger("Keys_Numpad2", 50);
        lua.RegisterConstantInteger("Keys_Numpad3", 51);
        lua.RegisterConstantInteger("Keys_Numpad4", 52);
        lua.RegisterConstantInteger("Keys_Numpad5", 53);
        lua.RegisterConstantInteger("Keys_Numpad6", 54);
        lua.RegisterConstantInteger("Keys_Numpad7", 55);
        lua.RegisterConstantInteger("Keys_Numpad8", 56);
        lua.RegisterConstantInteger("Keys_Numpad9", 57);
        lua.RegisterConstantInteger("Keys_Semicolon", 59);
        lua.RegisterConstantInteger("Keys_Equal", 61);
        lua.RegisterConstantInteger("Keys_A", 65);
        lua.RegisterConstantInteger("Keys_B", 66);
        lua.RegisterConstantInteger("Keys_C", 67);
        lua.RegisterConstantInteger("Keys_D", 68);
        lua.RegisterConstantInteger("Keys_E", 69);
        lua.RegisterConstantInteger("Keys_F", 70);
        lua.RegisterConstantInteger("Keys_G", 71);
        lua.RegisterConstantInteger("Keys_H", 72);
        lua.RegisterConstantInteger("Keys_I", 73);
        lua.RegisterConstantInteger("Keys_J", 74);
        lua.RegisterConstantInteger("Keys_K", 75);
        lua.RegisterConstantInteger("Keys_L", 76);
        lua.RegisterConstantInteger("Keys_M", 77);
        lua.RegisterConstantInteger("Keys_N", 78);
        lua.RegisterConstantInteger("Keys_O", 79);
        lua.RegisterConstantInteger("Keys_P", 80);
        lua.RegisterConstantInteger("Keys_Q", 81);
        lua.RegisterConstantInteger("Keys_R", 82);
        lua.RegisterConstantInteger("Keys_S", 83);
        lua.RegisterConstantInteger("Keys_T", 84);
        lua.RegisterConstantInteger("Keys_U", 85);
        lua.RegisterConstantInteger("Keys_V", 86);
        lua.RegisterConstantInteger("Keys_W", 87);
        lua.RegisterConstantInteger("Keys_X", 88);
        lua.RegisterConstantInteger("Keys_Y", 89);
        lua.RegisterConstantInteger("Keys_Z", 90);
        lua.RegisterConstantInteger("Keys_LeftBracket", 91);
        lua.RegisterConstantInteger("Keys_Backslash", 92);
        lua.RegisterConstantInteger("Keys_RightBracket", 93);
        lua.RegisterConstantInteger("Keys_GraveAccent", 96);
        lua.RegisterConstantInteger("Keys_World_1", 161);
        lua.RegisterConstantInteger("Keys_World_2", 162);
        lua.RegisterConstantInteger("Keys_Escape", 256);
        lua.RegisterConstantInteger("Keys_Enter", 257);
        lua.RegisterConstantInteger("Keys_Tab", 258);
        lua.RegisterConstantInteger("Keys_Backspace", 259);
        lua.RegisterConstantInteger("Keys_Insert", 260);
        lua.RegisterConstantInteger("Keys_Delete", 261);
        lua.RegisterConstantInteger("Keys_Right", 262);
        lua.RegisterConstantInteger("Keys_Left", 263);
        lua.RegisterConstantInteger("Keys_Down", 264);
        lua.RegisterConstantInteger("Keys_Up", 265);
        lua.RegisterConstantInteger("Keys_PageUp", 266);
        lua.RegisterConstantInteger("Keys_PageDown", 267);
        lua.RegisterConstantInteger("Keys_Home", 268);
        lua.RegisterConstantInteger("Keys_End", 269);
        lua.RegisterConstantInteger("Keys_CapsLock", 280);
        lua.RegisterConstantInteger("Keys_ScrollLock", 281);
        lua.RegisterConstantInteger("Keys_NumLock", 282);
        lua.RegisterConstantInteger("Keys_PrintScreen", 283);
        lua.RegisterConstantInteger("Keys_Pause", 284);
        lua.RegisterConstantInteger("Keys_F1", 290);
        lua.RegisterConstantInteger("Keys_F2", 291);
        lua.RegisterConstantInteger("Keys_F3", 292);
        lua.RegisterConstantInteger("Keys_F4", 293);
        lua.RegisterConstantInteger("Keys_F5", 294);
        lua.RegisterConstantInteger("Keys_F6", 295);
        lua.RegisterConstantInteger("Keys_F7", 296);
        lua.RegisterConstantInteger("Keys_F8", 297);
        lua.RegisterConstantInteger("Keys_F9", 298);
        lua.RegisterConstantInteger("Keys_F10", 299);
        lua.RegisterConstantInteger("Keys_F11", 300);
        lua.RegisterConstantInteger("Keys_F12", 301);
        lua.RegisterConstantInteger("Keys_F13", 302);
        lua.RegisterConstantInteger("Keys_F14", 303);
        lua.RegisterConstantInteger("Keys_F15", 304);
        lua.RegisterConstantInteger("Keys_F16", 305);
        lua.RegisterConstantInteger("Keys_F17", 306);
        lua.RegisterConstantInteger("Keys_F18", 307);
        lua.RegisterConstantInteger("Keys_F19", 308);
        lua.RegisterConstantInteger("Keys_F20", 309);
        lua.RegisterConstantInteger("Keys_F21", 310);
        lua.RegisterConstantInteger("Keys_F22", 311);
        lua.RegisterConstantInteger("Keys_F23", 312);
        lua.RegisterConstantInteger("Keys_F24", 313);
        lua.RegisterConstantInteger("Keys_F25", 314);
        lua.RegisterConstantInteger("Keys_KP_0", 320);
        lua.RegisterConstantInteger("Keys_KP_1", 321);
        lua.RegisterConstantInteger("Keys_KP_2", 322);
        lua.RegisterConstantInteger("Keys_KP_3", 323);
        lua.RegisterConstantInteger("Keys_KP_4", 324);
        lua.RegisterConstantInteger("Keys_KP_5", 325);
        lua.RegisterConstantInteger("Keys_KP_6", 326);
        lua.RegisterConstantInteger("Keys_KP_7", 327);
        lua.RegisterConstantInteger("Keys_KP_8", 328);
        lua.RegisterConstantInteger("Keys_KP_9", 329);
        lua.RegisterConstantInteger("Keys_KP_Decimal", 330);
        lua.RegisterConstantInteger("Keys_KP_Divide", 331);
        lua.RegisterConstantInteger("Keys_KP_Multiply", 332);
        lua.RegisterConstantInteger("Keys_KP_Subtract", 333);
        lua.RegisterConstantInteger("Keys_KP_Add", 334);
        lua.RegisterConstantInteger("Keys_KP_Enter", 335);
        lua.RegisterConstantInteger("Keys_KP_Equal", 336);
        lua.RegisterConstantInteger("Keys_LeftShift", 340);
        lua.RegisterConstantInteger("Keys_LeftControl", 341);
        lua.RegisterConstantInteger("Keys_LeftAlt", 342);
        lua.RegisterConstantInteger("Keys_LeftSuper", 343);
        lua.RegisterConstantInteger("Keys_RightShift", 344);
        lua.RegisterConstantInteger("Keys_RightControl", 345);
        lua.RegisterConstantInteger("Keys_RightAlt", 346);
        lua.RegisterConstantInteger("Keys_RightSuper", 347);
        lua.RegisterConstantInteger("Keys_Menu", 348);

        lua.RegisterConstantInteger("StrumPressState_NONE", (int)StrumPressState.NONE);
        lua.RegisterConstantInteger("StrumPressState_HIT", (int)StrumPressState.HIT);
        lua.RegisterConstantInteger("StrumPressState_HIT_SUSTAIN", (int)StrumPressState.HIT_SUSTAIN);
        lua.RegisterConstantInteger("StrumPressState_PENALTY_NOTE", (int)StrumPressState.PENALTY_NOTE);
        lua.RegisterConstantInteger("StrumPressState_PENALTY_HIT", (int)StrumPressState.PENALTY_HIT);
        lua.RegisterConstantInteger("StrumPressState_MISS", (int)StrumPressState.MISS);

        lua.RegisterConstantInteger("Ranking_NONE", (int)Ranking.NONE);
        lua.RegisterConstantInteger("Ranking_SICK", (int)Ranking.SICK);
        lua.RegisterConstantInteger("Ranking_GOOD", (int)Ranking.GOOD);
        lua.RegisterConstantInteger("Ranking_BAD", (int)Ranking.BAD);
        lua.RegisterConstantInteger("Ranking_SHIT", (int)Ranking.SHIT);
        lua.RegisterConstantInteger("Ranking_MISS", (int)Ranking.MISS);
        lua.RegisterConstantInteger("Ranking_PENALITY", (int)Ranking.PENALITY);

        lua.RegisterConstantInteger("Fading_NONE", (int)Fading.NONE);
        lua.RegisterConstantInteger("Fading_IN", (int)Fading.IN);
        lua.RegisterConstantInteger("Fading_OUT", (int)Fading.OUT);

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
        ExportsJson.script_json_register(lua);

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

    public static Luascript Init(string lua_sourcecode, string filename, string working_folder, object context, bool is_week) {
        ManagedLuaState lua = ManagedLuaState.Init(context);

        if (lua == null) {
            Logger.Error("luascript_init() cannot create lua state, not enough memory");
            return null;
        }

        RegisterObjects(lua, is_week);
        RegisterSandbox(lua);

        int status = lua.EvaluateString(lua_sourcecode, filename);

        if (status != ManagedLuaState.LUA_OK) {
            string error_message = lua.LuaStateHandle.lua_tostring(-1);
            Logger.Error($"luascript_init() luaL_loadfile() failed: {error_message}");

            lua.Dispose();
            return null;
        }

        lua.Tag = working_folder;

        return new Luascript(lua, is_week);
    }

    public void Destroy() {
        this.lua.Dispose();
        Luascript.instances.Remove(this);
    }

    public static void DropShared(object obj) {
        foreach (Luascript luascript in Luascript.instances) {
            luascript.lua.DropShared(obj);
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


}