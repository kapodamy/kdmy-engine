#include "luascript_internal.h"
#include "math2d.h"


static int script_math2d_lerp(lua_State* L) {
    float start = luaL_checkfloat(L, 1);
    float end = luaL_checkfloat(L, 2);
    float step = luaL_checkfloat(L, 3);

    float result = math2d_lerp(start, end, step);
    lua_pushnumber(L, (lua_Number)result);

    return 1;
}

static int script_math2d_inverselerp(lua_State* L) {
    float start = luaL_checkfloat(L, 1);
    float end = luaL_checkfloat(L, 2);
    float value = luaL_checkfloat(L, 3);

    float result = math2d_inverselerp(start, end, value);
    lua_pushnumber(L, (lua_Number)result);

    return 1;
}

static int script_math2d_nearestdown(lua_State* L) {
    float value = luaL_checkfloat(L, 1);
    float step = luaL_checkfloat(L, 2);

    float result = math2d_nearestdown(value, step);
    lua_pushnumber(L, (lua_Number)result);

    return 1;
}

static int script_math2d_rotate_point_by_degs(lua_State* L) {
    float radians = luaL_checkfloat(L, 1);
    float x = luaL_checkfloat(L, 2);
    float y = luaL_checkfloat(L, 3);

    math2d_rotate_point(radians, &x, &y);
    lua_pushnumber(L, (lua_Number)x);
    lua_pushnumber(L, (lua_Number)y);

    return 2;
}

static int script_math2d_points_distance(lua_State* L) {
    float x1 = luaL_checkfloat(L, 1);
    float y1 = luaL_checkfloat(L, 2);
    float x2 = luaL_checkfloat(L, 3);
    float y2 = luaL_checkfloat(L, 4);

    float result = math2d_points_distance(x1, y1, x2, y2);
    lua_pushnumber(L, (lua_Number)result);

    return 1;
}


static const luaL_Reg EXPORTS_FUNCTION[] = {
    { "math2d_lerp", script_math2d_lerp },
    { "math2d_inverselerp", script_math2d_inverselerp },
    { "math2d_nearestdown", script_math2d_nearestdown },
    { "math2d_rotate_point_by_degs", script_math2d_rotate_point_by_degs },
    { "math2d_points_distance", script_math2d_points_distance },
    { NULL, NULL }
};

void register_math2d(lua_State* L) {
    for (size_t i = 0; ; i++) {
        if (EXPORTS_FUNCTION[i].name == NULL || EXPORTS_FUNCTION[i].func == NULL) break;
        lua_pushcfunction(L, EXPORTS_FUNCTION[i].func);
        lua_setglobal(L, EXPORTS_FUNCTION[i].name);
    }

    luaL_dostring(L, 
        "function math2d_random_boolean(chance)\n"
        "local value = math.random(0, 100)\n"
        "return value < chance\n"
        "end\n"
    );
}

