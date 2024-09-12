#include "luascript_internal.h"

#include "jsonparser.h"
#include "unused_switch_case.h"


static void luascript_helper_parse_json_object(lua_State* L, JSONToken obj);
static void luascript_helper_parse_json_array(lua_State* L, JSONToken array);


static int luascript_helper_parse_json(lua_State* L, JSONToken json) {
    if (!json) {
        lua_pushnil(L);
        return 1;
    }

    JSONTokenValue root = json_get_root_as_token_value(json);

    switch (root.value_type) {
        case JSONTokenType_Array:
            luascript_helper_parse_json_array(L, json);
            break;
        case JSONTokenType_Null:
            lua_pushnil(L);
            break;
        case JSONTokenType_Boolean:
            lua_pushboolean(L, (*root.value_boolean) ? 1 : 0);
            break;
        case JSONTokenType_NumberDouble:
            lua_pushnumber(L, (lua_Number)(*root.value_number_double));
            break;
        case JSONTokenType_NumberLong:
            lua_pushinteger(L, (lua_Integer)(*root.value_number_long));
            break;
        case JSONTokenType_String:
            lua_pushstring(L, root.value_string);
            break;
        case JSONTokenType_Object:
            luascript_helper_parse_json_object(L, json);
            break;
            CASE_UNUSED(JSONTokenType_Unknown)
    }

    return 1;
}

static void luascript_helper_parse_json_object(lua_State* L, JSONToken obj) {
    int32_t props_length = 0;
    JSONTokenIterator iter = json_iterator_get_object_properties(obj);
    JSONTokenValue value;

    while (json_iterate_object(&iter, &value)) {
        props_length++;
    }

    lua_createtable(L, 0, props_length);

    iter = json_iterator_get_object_properties(obj);
    while (json_iterate_object(&iter, &value)) {
        switch (value.value_type) {
            case JSONTokenType_NumberDouble:
                lua_pushnumber(L, (lua_Number)(*value.value_number_double));
                lua_setfield(L, -2, value.property_name);
                break;
            case JSONTokenType_NumberLong:
                lua_pushinteger(L, (lua_Integer)(*value.value_number_long));
                lua_setfield(L, -2, value.property_name);
                break;
            case JSONTokenType_String:
                lua_pushstring(L, value.value_string);
                lua_setfield(L, -2, value.property_name);
                break;
            case JSONTokenType_Boolean:
                lua_pushboolean(L, (*value.value_boolean) ? 1 : 0);
                lua_setfield(L, -2, value.property_name);
                break;
            case JSONTokenType_Array:
                lua_pushstring(L, value.property_name);
                luascript_helper_parse_json_array(L, value.token);
                lua_settable(L, -3);
                break;
            case JSONTokenType_Null:
                lua_pushnil(L);
                lua_setfield(L, -2, value.property_name);
                break;
            case JSONTokenType_Object:
                lua_pushstring(L, value.property_name);
                luascript_helper_parse_json_object(L, value.token);
                lua_settable(L, -3);
                break;
                CASE_UNUSED(JSONTokenType_Unknown)
        }
    }
}

static void luascript_helper_parse_json_array(lua_State* L, JSONToken array) {
    int32_t array_length = json_read_array_length(array);

    lua_createtable(L, array_length, 0);

    for (int32_t i = 0; i < array_length; i++) {
        JSONTokenType type = json_get_array_item_type(array, i);

        switch (type) {
            case JSONTokenType_NumberDouble:
                lua_pushnumber(L, (lua_Number)json_read_array_item_number_double(array, i, 0.0));
                break;
            case JSONTokenType_NumberLong:
                lua_pushinteger(L, (lua_Integer)json_read_array_item_number_long(array, i, 0));
                break;
            case JSONTokenType_String:
                lua_pushstring(L, json_read_array_item_string(array, i, NULL));
                break;
            case JSONTokenType_Boolean:
                lua_pushboolean(L, json_read_array_item_boolean(array, i, false) ? 1 : 0);
                break;
            case JSONTokenType_Object:
                luascript_helper_parse_json_object(L, json_read_array_item_object(array, i));
                break;
            case JSONTokenType_Array:
                luascript_helper_parse_json_array(L, json_read_array_item_array(array, i));
                break;
            case JSONTokenType_Null:
                lua_pushnil(L);
                break;
                CASE_UNUSED(JSONTokenType_Unknown)
        }

        lua_rawseti(L, -2, i + 1);
    }
}


static int script_json_parse_from_file(lua_State* L) {
    const char* src = luaL_checkstring(L, 2);

    JSONToken json = json_load_from(src);

    return luascript_helper_parse_json(L, json);
}

static int script_json_parse(lua_State* L) {
    const char* json_sourcecode = luaL_checkstring(L, 2);

    JSONToken json = json_load_from_string(json_sourcecode);

    return luascript_helper_parse_json(L, json);
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const luaL_Reg JSON_FUNCTIONS[] = {
    {"parse_from_file", script_json_parse_from_file},
    {"parse", script_json_parse},
    {NULL, NULL}
};


static int script_json_gc(lua_State* L) {
    // nothing to do
    (void)L;
    return 0;
}

static int script_json_tostring(lua_State* L) {
    lua_pushstring(L, "{JSON}");
    return 1;
}

void script_json_register(lua_State* L) {
    luascript_register(L, JSON, script_json_gc, script_json_tostring, JSON_FUNCTIONS);
}
