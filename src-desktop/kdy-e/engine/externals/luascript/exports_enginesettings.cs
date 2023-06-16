using CsharpWrapper;
using Engine.Externals.LuaInterop;

namespace Engine.Externals.LuaScriptInterop;

public static class ExportsEngineSettings {
    public const string ENGINESETTINGS = "EngineSettings";


    static int script_enginesettings_index(LuaState L) {
        L.ReadUserdata<object>(ENGINESETTINGS);
        string field = L.luaL_optstring(2, null);

        switch (field) {
            case "distractionsEnabled":
                L.lua_pushboolean(EngineSettings.gameplay_enabled_distractions);
                break;
            case "flahsingLightsEnabled":
                L.lua_pushboolean(EngineSettings.gameplay_enabled_flashinglights);
                break;
            case "inversedScrollEnabled":
                L.lua_pushboolean(EngineSettings.inverse_strum_scroll);
                break;
            case "songProgressbarEnabled":
                L.lua_pushboolean(EngineSettings.song_progressbar);
                break;
            default:
                return L.luaL_error($"unknown field '{field}'");
        }

        return 1;
    }

    public static int script_enginesettings_new(LuaState L, object enginesettings) {
        return L.CreateUserdata(ENGINESETTINGS, enginesettings);
    }

    static int script_enginesettings_gc(LuaState L) {
        return L.GC_userdata(ENGINESETTINGS);
    }

    static int script_enginesettings_tostring(LuaState L) {
        return L.ToString_userdata(ENGINESETTINGS);
    }

    private static readonly LuaCallback delgate_index = script_enginesettings_index;
    private static readonly LuaCallback delegate_gc = script_enginesettings_gc;
    private static readonly LuaCallback delegate_tostring = script_enginesettings_tostring;

    public static void script_enginesettings_register(ManagedLuaState L) {
        L.RegisterStructMetaTable(ENGINESETTINGS, delegate_gc, delegate_tostring, delgate_index, null);
    }

}
