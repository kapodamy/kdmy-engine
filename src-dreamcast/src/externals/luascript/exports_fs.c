#include "luascript_internal.h"

#include "fs.h"
#include "malloc_utils.h"


static int script_fs_readfile(lua_State* L) {
    const char* src = luaL_checkstring(L, 1);

    luascript_change_working_folder(L);
    ArrayBuffer buffer = fs_readarraybuffer(src);
    luascript_restore_working_folder(L);

    if (buffer) {
        lua_pushlstring(L, (const char*)buffer->data, buffer->length);
        arraybuffer_destroy(&buffer);
    } else {
        lua_pushnil(L);
    }

    return 1;
}

static int script_fs_get_full_path(lua_State* L) {
    const char* path = luaL_checkstring(L, 1);

    luascript_change_working_folder(L);
    char* ret = fs_get_full_path(path);
    luascript_restore_working_folder(L);

    lua_pushstring(L, ret);
    free_chk(ret);

    return 1;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void script_fs_register(lua_State* L) {
    lua_pushcfunction(L, script_fs_readfile);
    lua_setglobal(L, "fs_readfile");
    lua_pushcfunction(L, script_fs_get_full_path);
    lua_setglobal(L, "fs_get_full_path");
}
