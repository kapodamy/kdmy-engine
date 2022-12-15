using Engine.Animation;
using Engine.Externals.LuaInterop;

namespace Engine.Externals.LuaScriptInterop {

    public static class ExportsAnimListItem {

        public const string ANIMLISTITEM = "AnimListItem";


        static int script_animlistitem_index(LuaState L) {
            AnimListItem animlistitem = L.ReadUserdata<AnimListItem>(ANIMLISTITEM);
            string field = L.luaL_optstring(2, null);

            switch (field) {
                case "name":
                    L.lua_pushstring(animlistitem.name);
                    break;
                case "isFrameAnimation":
                    L.lua_pushboolean(AnimList.IsItemFrameAnimation(animlistitem));
                    break;
                case "isItemMacroAnimation":
                    L.lua_pushboolean(AnimList.IsItemMacroAnimation(animlistitem));
                    break;
                case "isItemTweenKeyframeAnimation":
                    L.lua_pushboolean(AnimList.IsItemTweenkeyframeAnimation(animlistitem));
                    break;
                default:
                    return L.luaL_error($"unknown field '{field}'");
            }

            return 1;
        }

        public static int script_animlistitem_new(LuaState L, AnimListItem animlistitem) {
            return L.CreateUserdata(ANIMLISTITEM, animlistitem);
        }

        static int script_animlistitem_gc(LuaState L) {
            return L.GC_userdata(ANIMLISTITEM);
        }

        static int script_animlistitem_tostring(LuaState L) {
            return L.ToString_userdata(ANIMLISTITEM);
        }

        private static readonly LuaCallback delgate_index = script_animlistitem_index;
        private static readonly LuaCallback delegate_gc = script_animlistitem_gc;
        private static readonly LuaCallback delegate_tostring = script_animlistitem_tostring;

        public static void script_animlistitem_register(ManagedLuaState L) {
            L.RegisterStructMetaTable(ANIMLISTITEM, delegate_gc, delegate_tostring, delgate_index, null);
        }

    }
}
