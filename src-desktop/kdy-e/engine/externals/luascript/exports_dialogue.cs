using Engine.Externals.LuaInterop;
using Engine.Game.Gameplay;
using Engine.Platform;
using Engine.Utils;

namespace Engine.Externals.LuaScriptInterop {

    public static class ExportsDialogue {

        private const string DIALOGUE = "Dialogue";

        static int script_dialogue_apply_state(LuaState L) {
            Dialogue dialogue = L.ReadUserdata<Dialogue>(DIALOGUE);

            string state_name = L.luaL_checkstring(2);

            bool ret = dialogue.ApplyState(state_name);

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

            string dialog_src = L.luaL_checkstring(2);

            bool ret = dialogue.ShowDialog(dialog_src);

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

            bool hidden = L.luaL_toboolean(2);

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

            PVRContextFlag antialiasing = LuascriptHelpers.ParsePVRFLAG(L, L.luaL_checkstring(2));

            dialogue.SetAntialiasing(antialiasing);

            return 0;
        }


        ////////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////

        static readonly LuaTableFunction[] DIALOGUE_FUNCTIONS = {
            new LuaTableFunction() { name = "apply_state", func = script_dialogue_apply_state },
            new LuaTableFunction() { name = "is_completed", func = script_dialogue_is_completed },
            new LuaTableFunction() { name = "is_hidden", func = script_dialogue_is_hidden },
            new LuaTableFunction() { name = "show_dialog", func = script_dialogue_show_dialog },
            new LuaTableFunction() { name = "close", func = script_dialogue_close },
            new LuaTableFunction() { name = "hide", func = script_dialogue_hide },
            new LuaTableFunction() { name = "get_modifier", func = script_dialogue_get_modifier },
            new LuaTableFunction() { name = "set_offsetcolor", func = script_dialogue_set_offsetcolor },
            new LuaTableFunction() { name = "set_alpha", func = script_dialogue_set_alpha },
            new LuaTableFunction() { name = "set_antialiasing", func = script_dialogue_set_set_antialiasing },
            new LuaTableFunction() { name = null, func = null }
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
}
