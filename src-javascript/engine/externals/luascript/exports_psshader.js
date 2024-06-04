"use strict";

const PSSHADER = "PSShader";


function script_psshader_init(L) {
    let vertex_sourcecode = LUA.luaL_optstring(L, 2, null);
    let fragment_sourcecode = LUA.luaL_optstring(L, 3, null);

    /**@type {PSShader}*/let psshader = PSShader.BuildFromSource(pvr_context, vertex_sourcecode, fragment_sourcecode);

    if (!psshader) {
        LUA.lua_pushnil(L);
        return 1;
    }

    return luascript_userdata_allocnew(L, PSSHADER, psshader);
}

function script_psshader_destroy(L) {
    /**@type {PSShader}*/let psshader = luascript_read_userdata(L, PSSHADER);

    if (luascript_userdata_is_allocated(L, PSSHADER))
        psshader.Destroy();
    else
        console.error("script_psshader_destroy() object was not allocated by lua");

    return 0;
}

function script_psshader_set_uniform_any(L) {
    /**@type {PSShader}*/let psshader = luascript_read_userdata(L, PSSHADER);

    let name = LUA.luaL_checkstring(L, 2);
    let values_count = LUA.lua_gettop(L) - 2;

    const buffer = []
    for (let i = 0; i < values_count; i++) {
        if (i > 16) break;
        buffer[i] = LUA.luaL_checknumber(L, i + 3);
    }

    let ret = psshader.SetUniformAny(name, buffer);

    LUA.lua_pushinteger(L, ret);

    return 1;
}

function script_psshader_set_uniform1f(L) {
    /**@type {PSShader}*/let psshader = luascript_read_userdata(L, PSSHADER);

    let name = LUA.luaL_checkstring(L, 2);
    let value = LUA.luaL_checknumber(L, 3);

    let ret = psshader.SetUniform1F(name, value);

    LUA.lua_pushboolean(L, ret);

    return 1;
}

function script_psshader_set_uniform1i(L) {
    /**@type {PSShader}*/let psshader = luascript_read_userdata(L, PSSHADER);

    let name = LUA.luaL_checkstring(L, 2);
    let value = LUA.luaL_checkinteger(L, 3);

    let ret = psshader.SetUniform1I(name, value);

    LUA.lua_pushboolean(L, ret);

    return 1;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/**@type {luaL_Reg[]}*/const PSSHADER_FUNCTIONS = [
    ["init", script_psshader_init],
    ["destroy", script_psshader_destroy],
    ["set_uniform_any", script_psshader_set_uniform_any],
    ["set_uniform1f", script_psshader_set_uniform1f],
    ["set_uniform1i", script_psshader_set_uniform1i],
    [null, null]
];


function script_psshader_new(L, psshader) {
    return luascript_userdata_new(L, PSSHADER, psshader);
}

function script_psshader_gc(L) {
    function psshader_destroy(/**@type {PSShader}*/psshader) {
        psshader.Destroy();
    }

    // if this object was allocated by lua, call the destructor
    return luascript_userdata_destroy(L, PSSHADER, psshader_destroy);
}

function script_psshader_tostring(L) {
    return luascript_userdata_tostring(L, PSSHADER);
}


function script_psshader_register(L) {
    luascript_register(L, PSSHADER, script_psshader_gc, script_psshader_tostring, PSSHADER_FUNCTIONS);
}

