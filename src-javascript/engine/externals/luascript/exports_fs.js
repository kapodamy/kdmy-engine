"use strict";


async function script_fs_readfile(L) {
    let src = LUA.luaL_checkstring(L, 1);

    luascript_change_working_folder(L);
    let buffer = await fs_readarraybuffer(src);
    luascript_restore_working_folder(L);

    if (buffer) {
        LUA.lua_pushlstring(L, buffer, buffer.length);
        free(buffer);
    } else {
        LUA.lua_pushnil(L);
    }

    return 1;
}

async function script_fs_get_full_path(L) {
    let path = LUA.luaL_checkstring(L, 1);

    luascript_change_working_folder(L);
    let ret = await fs_get_full_path(path);
    luascript_restore_working_folder(L);

    LUA.lua_pushstring(L, ret);
    free(ret);

    return 1;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

function script_fs_register(L) {
    LUA.lua_pushcfunction(L, script_fs_readfile);
    LUA.lua_setglobal(L, "fs_readfile");
    LUA.lua_pushcfunction(L, script_fs_get_full_path);
    LUA.lua_setglobal(L, "fs_get_full_path");
}
