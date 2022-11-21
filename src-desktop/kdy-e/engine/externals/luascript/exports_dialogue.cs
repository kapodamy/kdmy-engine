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

            bool hidden = L.luaL_checkboolean(2);

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

            float r = L.luaL_checkfloat(2);
            float g = L.luaL_checkfloat(3);
            float b = L.luaL_checkfloat(4);
            float a = L.luaL_checkfloat(5);

            dialogue.SetOffsetColor(r, g, b, a);

            return 0;
        }

        static int script_dialogue_set_alpha(LuaState L) {
            Dialogue dialogue = L.ReadUserdata<Dialogue>(DIALOGUE);

            float alpha = L.luaL_checkfloat(2);

            dialogue.SetAlpha(alpha);

            return 0;
        }

        static int script_dialogue_set_set_antialiasing(LuaState L) {
            Dialogue dialogue = L.ReadUserdata<Dialogue>(DIALOGUE);

            PVRContextFlag antialiasing = VertexProps.ParseFlag2(L.luaL_checkstring(2), PVRContextFlag.INVALID_VALUE);
            if (antialiasing == PVRContextFlag.INVALID_VALUE) return L.luaL_argerror(2, "invalid pvrflag");

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
            return L.CreateUserdata(DIALOGUE, dialogue, true);
        }

        static int script_dialoge_gc(LuaState L) {
            return L.DestroyUserdata(DIALOGUE);
        }

        static int script_dialogue_tostring(LuaState L) {
            L.lua_pushstring("[Dialogue]");
            return 1;
        }


        private static readonly LuaCallback delegate_gc = script_dialoge_gc;
        private static readonly LuaCallback delegate_tostring = script_dialogue_tostring;

        internal static void register_dialogue(ManagedLuaState L) {
            L.RegisterMetaTable(
                DIALOGUE,
                delegate_gc,
                delegate_tostring,
                DIALOGUE_FUNCTIONS
            );
        }

    }
}
