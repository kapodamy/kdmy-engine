using System;
using Engine.Externals.LuaInterop;
using Engine.Platform;

namespace Engine.Externals.LuaScriptInterop;

public static class LuascriptHelpers {

    public static object ParseModdingValue(LuaState L, int idx) {
        object value = null;
        switch (L.lua_type(-1)) {
            case LuaType.TBOOLEAN:
                value = L.lua_toboolean(-1);
                break;
            case LuaType.TNUMBER:
                value = L.lua_tonumber(-1);
                break;
            case LuaType.TSTRING:
                value = L.lua_tostring(-1);
                break;
            case LuaType.TNIL:
            case LuaType.TNONE:
                break;
            default:
                Logger.Error($"luascripthelpers_to_basic_value() invalid value at idx={idx}");
                break;
        }

        return value;
    }

    public static void PushModdingValue(LuaState L, object value) {
        if (value is string)
            L.lua_pushstring((string)value);
        else if (value is long)
            L.lua_pushnumber((long)value);
        else if (value is double)
            L.lua_pushnumber((double)value);
        else if (value is bool)
            L.lua_pushboolean((bool)value);
        else
            L.lua_pushnil();
    }


    public static void ChangeWorkingFolder(LuaState L) {
        string working_folder = (string)L.Tag;
        if (working_folder != null) {
            FS.FolderStackPush();
            FS.SetWorkingFolder(working_folder, false);
        }
    }

    public static void RestoreWorkingFolder(LuaState L) {
        string working_folder = (string)L.Tag;
        if (working_folder != null) {
            FS.FolderStackPop();
        }
    }


    public static string EnumsStringify(LuascriptEnums source, int value) {

        foreach (LuascriptEnumsPair pair in source.pairs) {
            if (pair.name != null && pair.value == value) {
                return pair.name;
            }
        }

        Logger.Warn($"unknown enum value {value} for {source.enum_name}");

        // default name (this never should happen)
        return source.pairs[0].name;
    }

    public static int optenum(LuaState L, int idx, LuascriptEnums source) {
        string unparsed_value = L.luaL_optstring(idx, null);

        if (String.IsNullOrEmpty(unparsed_value)) {
            if (source.reject_on_null_or_empty)
                return L.luaL_error($"the {source.enum_name} enum must have a value");
            else
                return source.pairs[0].value; // default value
        }

        foreach (LuascriptEnumsPair pair in source.pairs) {
            if (pair.name == unparsed_value) {
                return pair.value;
            }
        }

        return L.luaL_error($"invalid {source.enum_name} enum value: {unparsed_value}");
    }

    public static void pushenum(LuaState L, LuascriptEnums source, int value) {
        string string_value = LuascriptHelpers.EnumsStringify(source, value);
        L.lua_pushstring(string_value);
    }


    public static bool? optnbool(LuaState L, int idx) {
        switch (L.lua_type(idx)) {
            case LuaType.TNONE:
            case LuaType.TNIL:
                return null;
            default:
                return L.lua_toboolean(idx);
        }
    }

    public static void pushnbool(LuaState L, bool? value) {
        if (value == null)
            L.lua_pushnil();
        else if (value == true)
            L.lua_pushboolean(true);
        else
            L.lua_pushboolean(false);
    }

}

