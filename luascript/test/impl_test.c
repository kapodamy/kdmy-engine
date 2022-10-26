#include <stdlib.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

typedef struct {
    int width; int height; void* somebuffer;
} SomeStruct;

const char FOOBAR[] = "FOOBAR";



static int script_new_object(lua_State* L) {
    SomeStruct* ptr = (SomeStruct*)lua_newuserdata(L, sizeof(SomeStruct));
    ptr->width = 640;
    ptr->height = 480;
    ptr->somebuffer = malloc(12345);

    luaL_getmetatable(L, FOOBAR);
    lua_setmetatable(L, -2);

    return 1;
}

static int script_method(lua_State* L) {
    if (lua_isnil(L, 1)) return luaL_error(L, "null instance");

    SomeStruct* ptr = (SomeStruct*)luaL_checkudata(L, 1, FOOBAR);
    int density = (int)luaL_checkinteger(L, 2);

    int result = ptr->width * ptr->height * density;

    lua_pushinteger(L, (lua_Integer)result);
    return 1;
}

static int script_gc(lua_State* L) {
    printf("__gc was called!\n");
    SomeStruct* userdata = (SomeStruct*)luaL_checkudata(L, 1, FOOBAR);
    free(userdata->somebuffer);
    return 0;
}

static int script_tostring(lua_State* L) {
    luaL_checkudata(L, 1, FOOBAR);
    lua_pushstring(L, "[this is a FOOBAR instance]");
    return 1;
}


static const luaL_Reg OBJECT_METHODS[] = {
    { "test_method", script_method},
    {NULL, NULL}
};

static const luaL_Reg OBJECT_METAFUNCTIONS[] = {
    { "__gc", script_gc},
    { "__tostring", script_tostring},
    {NULL, NULL}
};


static inline void register_object(lua_State* L) {
    // replacement of "luaL_openlib(L, FOOBAR, OBJECT_METHODS, 0);"
    lua_newtable(L);
    luaL_setfuncs(L, OBJECT_METHODS, 0);
    lua_pushvalue(L, -1);// this does the trick
    lua_setglobal(L, FOOBAR);

    luaL_newmetatable(L, FOOBAR);

    // replacement of "luaL_openlib(L, 0, OBJECT_METAFUNCTIONS, 0);"
    luaL_setfuncs(L, OBJECT_METAFUNCTIONS, 0);

    lua_pushliteral(L, "__index");
    lua_pushvalue(L, -3);
    lua_rawset(L, -3);

    lua_pushliteral(L, "__metatable");
    lua_pushvalue(L, -3);
    lua_rawset(L, -3);

    lua_pop(L, 1);
}


int main(int argc, char* argv[]) {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    // register FOOBAR
    register_object(L);

    // expose "script_new_object" as global function
    lua_pushcfunction(L, script_new_object);
    lua_setglobal(L, "create_object");

    int status = luaL_dofile(L, "testscript.lua");

    if (status != LUA_OK) {
        const char* error_message = lua_tostring(L, -1);
        fprintf(stderr, "luaL_dofile() failed: %s\n", error_message);

        lua_close(L);
        return 1;
    }

    lua_close(L);
    return 0;
}
