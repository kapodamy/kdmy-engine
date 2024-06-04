"use strict";

const ENGINESETTINGS = "EngineSettings";


function script_enginesettings_index(L) {
    let enginesettings = luascript_read_userdata(L, ENGINESETTINGS);
    let field = LUA.luaL_checkstring(L, 2);

    switch (field) {
        case "distractionsEnabled":
            LUA.lua_pushboolean(L, enginesettings.gameplay_enabled_distractions);
            break;
        case "flahsingLightsEnabled":
            LUA.lua_pushboolean(L, enginesettings.gameplay_enabled_flashinglights);
            break;
        case "inversedScrollEnabled":
            LUA.lua_pushboolean(L, enginesettings.inverse_strum_scroll);
            break;
        case "songProgressbarEnabled":
            LUA.lua_pushboolean(L, enginesettings.song_progressbar);
            break;
        default:
            enginesettings = null;
    }


    if (!enginesettings) {
        return LUA.luaL_error(L, `unknown field '${field}'`);
    }

    return 1;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

function script_enginesettings_new(L, enginesettings) {
    return luascript_userdata_new(L, ENGINESETTINGS, enginesettings);
}

function script_enginesettings_gc(L) {
    return luascript_userdata_gc(L, ENGINESETTINGS);
}

function script_enginesettings_tostring(L) {
    return luascript_userdata_tostring(L, ENGINESETTINGS);
}

function script_enginesettings_register(lua) {
    LUA.luaL_newmetatable(lua, ENGINESETTINGS);

    LUA.lua_pushcfunction(lua, script_enginesettings_gc);
    LUA.lua_setfield(lua, -2, "__gc");

    LUA.lua_pushcfunction(lua, script_enginesettings_tostring);
    LUA.lua_setfield(lua, -2, "__tostring");

    LUA.lua_pushcfunction(lua, script_enginesettings_index);
    LUA.lua_setfield(lua, -2, "__index");

    /*LUA.lua_pushcfunction(lua, script_enginesettings_newindex);
    LUA.lua_setfield(lua, -2, "__newindex");*/

    LUA.lua_pop(lua, 1);
}

