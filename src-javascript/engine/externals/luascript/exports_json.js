"use strict";

const JSONKDY = "JSON";


function luascript_helper_parse_json(L, json) {
    if (!json) {
        LUA.lua_pushnil(L);
        return 1;
    }

    let type = typeof (json);

    if (Array.isArray(json)) {
        luascript_helper_parse_json_array(L, json);
    } else if (type == null) {
        LUA.lua_pushnil(L);
    } else if (type === "boolean") {
        LUA.lua_pushboolean(L, json);
    } else if (type === "number") {
        LUA.lua_pushnumber(L, json);
    } else if (type === "string") {
        LUA.lua_pushstring(L, json);
    } else if (type === "object") {
        luascript_helper_parse_json_object(L, json);
    }

    return 1;
}

function luascript_helper_parse_json_object(L, obj) {
    let props = Object.getOwnPropertyNames(obj);

    LUA.lua_createtable(L, 0, props.length);

    for (let i = 0; i < props.length; i++) {
        let value = obj[props[i]];
        let type = typeof value;
        let name = props[i];

        switch (type) {
            case "number":
                LUA.lua_pushnumber(L, value);
                LUA.lua_setfield(L, -2, name);
                break;
            case "string":
                LUA.lua_pushstring(L, value);
                LUA.lua_setfield(L, -2, name);
                break;
            case "boolean":
                LUA.lua_pushboolean(L, value);
                LUA.lua_setfield(L, -2, name);
                break;
            case "object":
                if (Array.isArray(value)) {
                    LUA.lua_pushstring(L, name);
                    luascript_helper_parse_json_array(L, value);
                    LUA.lua_settable(L, -3);
                } else if (value == null) {
                    LUA.lua_pushnil(L);
                    LUA.lua_setfield(L, -2, name);
                } else {
                    LUA.lua_pushstring(L, name);
                    luascript_helper_parse_json_object(L, value);
                    LUA.lua_settable(L, -3);
                }
                break;
        }
    }

}

function luascript_helper_parse_json_array(L, array) {
    LUA.lua_createtable(L, array.length, 0);

    for (let i = 0; i < array.length; i++) {
        let value = array[i];
        let type = typeof value;

        switch (type) {
            case "number":
                LUA.lua_pushnumber(L, value);
                break;
            case "string":
                LUA.lua_pushstring(L, value);
                break;
            case "boolean":
                LUA.lua_pushboolean(L, value);
                break;
            case "object":
                if (Array.isArray(value))
                    luascript_helper_parse_json_array(L, value);
                else if (value == null)
                    LUA.lua_pushnil(L);
                else
                    luascript_helper_parse_json_object(L, value);
                break;
        }

        LUA.lua_rawseti(L, -2, i + 1);
    }
}




async function script_json_parse_from_file(L) {
    let src = LUA.luaL_checkstring(L, 2);

    let json = await json_load_from(src);

    return luascript_helper_parse_json(L, json);
}

function script_json_parse(L) {
    let json_sourcecode = LUA.luaL_checkstring(L, 2);

    let json = json_load_from_string(json_sourcecode);

    return luascript_helper_parse_json(L, json);
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/**@type {luaL_Reg[]}*/const JSON_FUNCTIONS = [
    ["parse_from_file", script_json_parse_from_file],
    ["parse", script_json_parse],
    [null, null]
];


function script_json_gc(L) {
    // nothing to do
    return 0;
}

function script_json_tostring(L) {
    LUA.lua_pushstring(L, "{JSON}");
    return 1;
}

function script_json_register(L) {
    luascript_register(L, JSONKDY, script_json_gc, script_json_tostring, JSON_FUNCTIONS);
}
