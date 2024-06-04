using Engine.Externals.LuaInterop;
using Engine.Game.Gameplay;
using Engine.Platform;

namespace Engine.Externals.LuaScriptInterop;

public static class ExportsDialogue {

    private const string DIALOGUE = "Dialogue";

    static int script_dialogue_apply_state(LuaState L) {
        Dialogue dialogue = L.ReadUserdata<Dialogue>(DIALOGUE);

        string state_name = L.luaL_checkstring(2);

        bool ret = dialogue.ApplyState(state_name);

        L.lua_pushboolean(ret);
        return 1;
    }

    static int script_dialogue_apply_state2(LuaState L) {
        Dialogue dialogue = L.ReadUserdata<Dialogue>(DIALOGUE);

        string state_name = L.luaL_checkstring(2);
        string if_line_label = L.luaL_checkstring(3);

        bool ret = dialogue.ApplyState2(state_name, if_line_label);

        L.lua_pushboolean(ret);
        return 1;
    }

    static int script_dialogue_is_completed(LuaState L) {
        Dialogue dialogue = L.ReadUserdata<Dialogue>(DIALOGUE);

        bool ret = dialogue.IsCompleted();

        L.lua_pushboolean(ret);
        return 1;
    }

    static int script_dialogue_is_hidden(LuaState L) {
        Dialogue dialogue = L.ReadUserdata<Dialogue>(DIALOGUE);

        bool ret = dialogue.IsHidden();

        L.lua_pushboolean(ret);
        return 1;
    }

    static int script_dialogue_show_dialog(LuaState L) {
        Dialogue dialogue = L.ReadUserdata<Dialogue>(DIALOGUE);

        LuascriptHelpers.ChangeWorkingFolder(L);
        string dialog_src = L.luaL_checkstring(2);
        LuascriptHelpers.RestoreWorkingFolder(L);

        bool ret = dialogue.ShowDialog(dialog_src);

        L.lua_pushboolean(ret);
        return 1;
    }

    static int script_dialogue_show_dialog2(LuaState L) {
        Dialogue dialogue = L.ReadUserdata<Dialogue>(DIALOGUE);

        string text_dialog_content = L.luaL_checkstring(2);

        bool ret = dialogue.ShowDialog2(text_dialog_content);

        L.lua_pushboolean(ret);
        return 1;
    }

    static int script_dialogue_close(LuaState L) {
        Dialogue dialogue = L.ReadUserdata<Dialogue>(DIALOGUE);

        dialogue.Close();

        return 0;
    }

    static int script_dialogue_hide(LuaState L) {
        Dialogue dialogue = L.ReadUserdata<Dialogue>(DIALOGUE);

        bool hidden = L.lua_toboolean(2);

        dialogue.Hide(hidden);

        return 0;
    }

    static int script_dialogue_get_modifier(LuaState L) {
        Dialogue dialogue = L.ReadUserdata<Dialogue>(DIALOGUE);

        Modifier modifier = dialogue.GetModifier();

        return ExportsModifier.script_modifier_new(L, modifier);
    }

    static int script_dialogue_set_offsetcolor(LuaState L) {
        Dialogue dialogue = L.ReadUserdata<Dialogue>(DIALOGUE);

        float r = (float)L.luaL_checknumber(2);
        float g = (float)L.luaL_checknumber(3);
        float b = (float)L.luaL_checknumber(4);
        float a = (float)L.luaL_checknumber(5);

        dialogue.SetOffsetColor(r, g, b, a);

        return 0;
    }

    static int script_dialogue_set_alpha(LuaState L) {
        Dialogue dialogue = L.ReadUserdata<Dialogue>(DIALOGUE);

        float alpha = (float)L.luaL_checknumber(2);

        dialogue.SetAlpha(alpha);

        return 0;
    }

    static int script_dialogue_set_set_antialiasing(LuaState L) {
        Dialogue dialogue = L.ReadUserdata<Dialogue>(DIALOGUE);

        PVRFlag antialiasing = (PVRFlag)LuascriptHelpers.optenum(L, 2, LuascriptEnums.PVRFlag);

        dialogue.SetAntialiasing(antialiasing);

        return 0;
    }


    ////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////

    static readonly LuaTableFunction[] DIALOGUE_FUNCTIONS = {
        new LuaTableFunction("apply_state", script_dialogue_apply_state),
        new LuaTableFunction("apply_state2", script_dialogue_apply_state2),
        new LuaTableFunction("is_completed", script_dialogue_is_completed),
        new LuaTableFunction("is_hidden", script_dialogue_is_hidden),
        new LuaTableFunction("show_dialog", script_dialogue_show_dialog),
        new LuaTableFunction("show_dialog2", script_dialogue_show_dialog2),
        new LuaTableFunction("close", script_dialogue_close),
        new LuaTableFunction("hide", script_dialogue_hide),
        new LuaTableFunction("get_modifier", script_dialogue_get_modifier),
        new LuaTableFunction("set_offsetcolor", script_dialogue_set_offsetcolor),
        new LuaTableFunction("set_alpha", script_dialogue_set_alpha),
        new LuaTableFunction("set_antialiasing", script_dialogue_set_set_antialiasing),
        new LuaTableFunction(null, null)
    };

    internal static int script_dialogue_new(LuaState L, Dialogue dialogue) {
        return L.CreateUserdata(DIALOGUE, dialogue);
    }

    static int script_dialoge_gc(LuaState L) {
        return L.GC_userdata(DIALOGUE);
    }

    static int script_dialogue_tostring(LuaState L) {
        return L.ToString_userdata(DIALOGUE);
    }


    private static readonly LuaCallback delegate_gc = script_dialoge_gc;
    private static readonly LuaCallback delegate_tostring = script_dialogue_tostring;

    internal static void script_dialogue_register(ManagedLuaState L) {
        L.RegisterMetaTable(DIALOGUE, delegate_gc, delegate_tostring, DIALOGUE_FUNCTIONS);
    }

}
