using Engine.Externals.LuaInterop;
using Engine.Utils;

namespace Engine.Externals.LuaScriptInterop;

public static class ExportsJson {

    public const string JSON = "JSON";

    #region "JSON --> Lua table"

    private static int luascript_helper_parse_json(LuaState L, JSONToken json) {
        if (json == null) {
            L.lua_pushnil();
            return 1;
        }

        object value = JSONParser.GetTokenValue(json);

        if (JSONParser.IsArray(json)) {
            luascript_helper_parse_json_array(L, json);
        } else if (value is null) {
            L.lua_pushnil();
        } else if (value is bool) {
            L.lua_pushboolean((bool)value);
        } else if (value is long) {
            L.lua_pushinteger((long)value);
        } else if (value is double) {
            L.lua_pushnumber((double)value);
        } else if (value is string) {
            L.lua_pushstring((string)value);
        } else {
            luascript_helper_parse_json_object(L, json);
        }

        return 1;
    }

    private static void luascript_helper_parse_json_object(LuaState L, JSONToken obj) {
        JSONProperty[] props = JSONParser.GetObjectProperties(obj);

        L.lua_createtable(0, props.Length);

        for (int i = 0 ; i < props.Length ; i++) {
            switch (props[i].type) {
                case JSONValueType.NumberDouble:
                    L.lua_pushnumber(JSONParser.ReadNumberDouble(obj, props[i].name, 0.0));
                    L.lua_setfield(-2, props[i].name);
                    break;
                case JSONValueType.NumberLong:
                    L.lua_pushinteger(JSONParser.ReadNumberLong(obj, props[i].name, 0L));
                    L.lua_setfield(-2, props[i].name);
                    break;
                case JSONValueType.String:
                    L.lua_pushstring(JSONParser.ReadString(obj, props[i].name, null));
                    L.lua_setfield(-2, props[i].name);
                    break;
                case JSONValueType.Boolean:
                    L.lua_pushboolean(JSONParser.ReadBoolean(obj, props[i].name, false));
                    L.lua_setfield(-2, props[i].name);
                    break;
                case JSONValueType.Array:
                    L.lua_pushstring(props[i].name);
                    luascript_helper_parse_json_array(L, JSONParser.ReadArray(obj, props[i].name));
                    L.lua_settable(-3);
                    break;
                case JSONValueType.Null:
                    L.lua_pushnil();
                    L.lua_setfield(-2, props[i].name);
                    break;
                case JSONValueType.Object:
                    L.lua_pushstring(props[i].name);
                    luascript_helper_parse_json_object(L, JSONParser.ReadObject(obj, props[i].name));
                    L.lua_settable(-3);
                    break;
            }
        }
    }

    private static void luascript_helper_parse_json_array(LuaState L, JSONToken array) {
        int length = JSONParser.ReadArrayLength(array);

        L.lua_createtable(length, 0);

        for (int i = 0 ; i < length ; i++) {
            JSONValueType type = JSONParser.GetArrayItemType(array, i);

            switch (type) {
                case JSONValueType.NumberDouble:
                    L.lua_pushnumber(JSONParser.ReadArrayItemNumberDouble(array, i, 0.0));
                    break;
                case JSONValueType.NumberLong:
                    L.lua_pushinteger(JSONParser.ReadArrayItemNumberLong(array, i, 0L));
                    break;
                case JSONValueType.String:
                    L.lua_pushstring(JSONParser.ReadArrayItemString(array, i, null));
                    break;
                case JSONValueType.Boolean:
                    L.lua_pushboolean(JSONParser.ReadArrayItemBoolean(array, i, false));
                    break;
                case JSONValueType.Array:
                    luascript_helper_parse_json_array(L, JSONParser.ReadArrayItemArray(array, i));
                    break;
                case JSONValueType.Object:
                    luascript_helper_parse_json_object(L, JSONParser.ReadArrayItemObject(array, i));
                    break;
                case JSONValueType.Null:
                    L.lua_pushnil();
                    break;
            }

            L.lua_rawseti(-2, i + 1);
        }
    }

    #endregion


    static int script_json_parse_from_file(LuaState L) {
        string src = L.luaL_checkstring(2);

        JSONToken json = JSONParser.LoadFrom(src);
        int ret = luascript_helper_parse_json(L, json);
        JSONParser.Destroy(json);

        return ret;
    }

    static int script_json_parse(LuaState L) {
        string json_sourcecode = L.luaL_checkstring(2);

        JSONToken json = JSONParser.LoadFromString(json_sourcecode);
        int ret = luascript_helper_parse_json(L, json);
        JSONParser.Destroy(json);

        return ret;
    }


    static readonly LuaTableFunction[] JSON_FUNCTIONS = {
        new LuaTableFunction("parse_from_file", script_json_parse_from_file),
        new LuaTableFunction("parse", script_json_parse),
        new LuaTableFunction( null, null)
    };

    static int script_json_gc(LuaState L) {
        // nothing to do
        return 0;
    }

    static int script_json_tostring(LuaState L) {
        L.lua_pushstring("{JSON}");
        return 1;
    }

    private static readonly LuaCallback gc = script_json_gc;
    private static readonly LuaCallback tostring = script_json_tostring;

    public static void script_json_register(ManagedLuaState L) {
        L.RegisterMetaTable(JSON, gc, tostring, JSON_FUNCTIONS);
    }
}
