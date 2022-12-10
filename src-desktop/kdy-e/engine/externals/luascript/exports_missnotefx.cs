using Engine.Externals.LuaInterop;
using Engine.Game;

namespace Engine.Externals.LuaScriptInterop {

    public static class ExportsMissNoteFX {
        public const string MISSNOTEFX = "MissNoteFX";


        static int script_missnotefx_play_effect(LuaState L) {
            MissNoteFX missnotefx = L.ReadUserdata<MissNoteFX>(MISSNOTEFX);

            missnotefx.PlayEffect();

            return 0;
        }




        static readonly LuaTableFunction[] MISSNOTEFX_FUNCTIONS = {
            new LuaTableFunction("missnotefx_play_effect", script_missnotefx_play_effect),
            new LuaTableFunction(null, null)
        };

        public static int script_missnotefx_new(LuaState L, MissNoteFX missnotefx) {
            return L.CreateUserdata(MISSNOTEFX, missnotefx);
        }

        static int script_missnotefx_gc(LuaState L) {
            return L.GC_userdata(MISSNOTEFX);
        }

        static int script_missnotefx_tostring(LuaState L) {
            return L.ToString_userdata(MISSNOTEFX);
        }

        private static readonly LuaCallback delegate_gc = script_missnotefx_gc;
        private static readonly LuaCallback delegate_tostring = script_missnotefx_tostring;

        public static void script_missnotefx_register(ManagedLuaState L) {
            L.RegisterMetaTable(MISSNOTEFX, delegate_gc, delegate_tostring, MISSNOTEFX_FUNCTIONS);
        }


    }
}

