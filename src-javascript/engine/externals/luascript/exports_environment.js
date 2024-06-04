"use strict";

const ENVIRONMENT = "Environment";


function script_environment_get_language(L) {
    LUA.lua_pushstring(L, luascriptplatform.GetLanguage());
    return 1;
}

function script_environment_get_username(L) {
    LUA.lua_pushstring(L, luascriptplatform.GetUsername());
    return 1;
}

function script_environment_get_cmdargs(L) {
    let cmdargs = luascriptplatform.GetCmdargs();

    LUA.lua_createtable(L, cmdargs.length, 0);
    for (let i = 0; i < cmdargs.length; i++) {
        LUA.lua_pushstring(L, cmdargs[i]);
        LUA.lua_rawseti(L, -2, i + 1);
    }

    return 1;
}

function script_environment_exit(L) {
    let exit_code = LUA.luaL_checkinteger(L, 2);
    luascriptplatform.Exit(exit_code);
    return 0;
}

function script_environment_change_window_title(L) {
    let luascript = luascript_get_instance(L);
    let title = LUA.luaL_optstring(L, 2, null);

    luascriptplatform.ChangeWindowTitle(title, !luascript.is_week);

    return 0;
}

function script_environment_require_window_attention(L) {
    luascriptplatform.RequestWindowAttention();
    return 0;
}

function script_environment_open_www_link(L) {
    let url = LUA.luaL_checkstring(L, 2);
    luascriptplatform.OpenWWWLink(url);
    return 0;
}

function script_environment_get_screensize(L) {
    const screen_size = [0.0, 0.0];
    luascriptplatform.GetScreenSize(screen_size);

    LUA.lua_pushinteger(L, screen_size[0]);
    LUA.lua_pushinteger(L, screen_size[1]);
    return 2;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/**@type {luaL_Reg[]}*/const ENVIRONMENT_FUNCTIONS = [
    ["get_language", script_environment_get_language],
    ["get_username", script_environment_get_username],
    ["get_cmdargs", script_environment_get_cmdargs],
    ["exit", script_environment_exit],
    ["change_window_title", script_environment_change_window_title],
    ["require_window_attention", script_environment_require_window_attention],
    ["open_www_link", script_environment_open_www_link],
    ["get_screensize", script_environment_get_screensize],
    [null, null]
];


function script_environment_gc(L) {
    // nothing to do
    return 0;
}

function script_environment_tostring(L) {
    let language = luascriptplatform.GetLanguage();
    let username = luascriptplatform.GetUsername();
    let cmdargs = luascriptplatform.GetCmdargs().join(" ");

    LUA.lua_pushfstring(
        L,
        `{language="${language}" username="${username}" cmdargs="${cmdargs}"}`
    );
    return 0;
}


function script_environment_register(L) {
    luascript_register(L, ENVIRONMENT, script_environment_gc, script_environment_tostring, ENVIRONMENT_FUNCTIONS);
}
