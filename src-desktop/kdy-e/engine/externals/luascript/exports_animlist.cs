using System;
using Engine.Animation;
using Engine.Externals.LuaInterop;

namespace Engine.Externals.LuaScriptInterop; 

public static class ExportsAnimList {

    public const string ANIMLIST = "AnimList";


    static int script_animlist_init(LuaState L) {
        string src = L.luaL_checkstring(2);

        LuascriptHelpers.ChangeWorkingFolder(L);
        AnimList ret = AnimList.Init(src);
        LuascriptHelpers.RestoreWorkingFolder(L);

        return L.CreateAllocatedUserdata(ANIMLIST, ret);
    }

    static int script_animlist_destroy(LuaState L) {
        AnimList animlist = L.ReadUserdata<AnimList>(ANIMLIST);

        if (L.IsUserdataAllocated(ANIMLIST))
            animlist.Destroy();
        else
            Logger.Warn("script_animlist_destroy() object was not allocated by lua");

        return 0;
    }

    static int script_animlist_get_animation(LuaState L) {
        AnimList animlist = L.ReadUserdata<AnimList>(ANIMLIST);
        string animation_name = L.luaL_optstring(2, null);

        AnimListItem ret = animlist.GetAnimation(animation_name);

        return ExportsAnimListItem.script_animlistitem_new(L, ret);
    }



    static readonly LuaTableFunction[] ANIMLIST_FUNCTIONS = {
        new LuaTableFunction("init", script_animlist_init),
        new LuaTableFunction("destroy", script_animlist_destroy),
        new LuaTableFunction("get_animation", script_animlist_get_animation),
        new LuaTableFunction(null, null)
    };

    public static int script_animlist_new(LuaState L, AnimList animlist) {
        return L.CreateUserdata(ANIMLIST, animlist);
    }

    static int script_animlist_gc(LuaState L) {
        return L.DestroyUserdata(ANIMLIST);
    }

    static int script_animlist_tostring(LuaState L) {
        return L.ToString_userdata(ANIMLIST);
    }

    private static readonly LuaCallback delegate_gc = script_animlist_gc;
    private static readonly LuaCallback delegate_tostring = script_animlist_tostring;

    public static void script_animlist_register(ManagedLuaState L) {
        L.RegisterMetaTable(ANIMLIST, delegate_gc, delegate_tostring, ANIMLIST_FUNCTIONS);
    }

}
