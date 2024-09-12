#include "luascript_internal.h"

#include "externals/luascript_platform.h"
#include "game/main.h"
#include "stringbuilder.h"


static int script_environment_get_language(lua_State* L) {
    (void)L;
    lua_pushstring(L, luascript_platform_get_language());
    return 1;
}

static int script_environment_get_username(lua_State* L) {
    (void)L;
    lua_pushstring(L, luascript_platform_get_username());
    return 1;
}

static int script_environment_get_cmdargs(lua_State* L) {
    const int argc = main_argc;
    char** argv = main_argv;

    lua_createtable(L, argc, 0);
    for (int32_t i = 0; i < argc; i++) {
        lua_pushstring(L, argv[i]);
        lua_rawseti(L, -2, i + 1);
    }

    return 1;
}

static int script_environment_exit(lua_State* L) {
    int exit_code = (int)luaL_checkinteger(L, 2);
    luascript_platform_exit(exit_code);
    return 0;
}

static int script_environment_change_window_title(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    const char* title = luaL_optstring(L, 2, NULL);

    luascript_platform_change_window_title(title, !luascript->is_week);

    return 0;
}

static int script_environment_require_window_attention(lua_State* L) {
    (void)L;
    luascript_platform_request_window_attention();
    return 0;
}

static int script_environment_open_www_link(lua_State* L) {
    const char* url = luaL_checkstring(L, 2);
    luascript_platform_open_www_link(url);
    return 0;
}

static int script_environment_get_screensize(lua_State* L) {
    int32_t screen_width, screen_height;
    luascript_platform_get_screen_size(&screen_width, &screen_height);

    lua_pushinteger(L, (lua_Integer)screen_width);
    lua_pushinteger(L, (lua_Integer)screen_height);
    return 2;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const luaL_Reg ENVIRONMENT_FUNCTIONS[] = {
    {"get_language", script_environment_get_language},
    {"get_username", script_environment_get_username},
    {"get_cmdargs", script_environment_get_cmdargs},
    {"exit", script_environment_exit},
    {"change_window_title", script_environment_change_window_title},
    {"require_window_attention", script_environment_require_window_attention},
    {"open_www_link", script_environment_open_www_link},
    {"get_screensize", script_environment_get_screensize},
    {NULL, NULL}
};


static int script_environment_gc(lua_State* L) {
    // nothing to do
    (void)L;
    return 0;
}

static int script_environment_tostring(lua_State* L) {
    const char* language = luascript_platform_get_language();
    const char* username = luascript_platform_get_username();
    StringBuilder args = stringbuilder_init(64);

    const int argc = main_argc;
    char** argv = main_argv;
    for (int i = 0; i < argc; i++) {
        if (i > 0) stringbuilder_add_char(args, ' ');
        stringbuilder_add(args, argv[i]);
    }

    lua_pushfstring(
        L,
        "{language=\"%s\" username=\"%s\" cmdargs=\"%s\"}",
        language,
        username,
        *stringbuilder_intern(args)
    );

    stringbuilder_destroy(&args);

    return 0;
}


void script_environment_register(lua_State* L) {
    luascript_register(L, ENVIRONMENT, script_environment_gc, script_environment_tostring, ENVIRONMENT_FUNCTIONS);
}
