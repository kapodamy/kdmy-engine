#include "luascript_internal.h"
#include "math2d.h"

#ifdef JAVASCRIPT
EM_JS_PRFX(double, math2d_rand, (), {
    window.crypto.getRandomValues(MATH2D_PRNG);
    let percent = MATH2D_PRNG[0] / 0xffffffff;
    return percent;
});
#else
static inline double math2d_rand() {
    return rand() / (double)RAND_MAX;
}
#endif

static double math2d_random(double min, double max) {
    return math2d_rand() * (max - min + 1) + min;
}


static int script_math2d_random(lua_State* L) {
    double ret;
    
    if (lua_gettop(L) == 0) {
        ret = math2d_rand();
    } else {
        double start = luaL_checknumber(L, 1);
        double end = luaL_checknumber(L, 2);
        ret = math2d_random(start, end);
    }

    lua_pushnumber(L, ret);
    return 1;
}

static int script_math2d_lerp(lua_State* L) {
    float start = (float)luaL_checknumber(L, 1);
    float end = (float)luaL_checknumber(L, 2);
    float step = (float)luaL_checknumber(L, 3);

    float result = math2d_lerp(start, end, step);
    lua_pushnumber(L, (lua_Number)result);

    return 1;
}

static int script_math2d_inverselerp(lua_State* L) {
    float start = (float)luaL_checknumber(L, 1);
    float end = (float)luaL_checknumber(L, 2);
    float value = (float)luaL_checknumber(L, 3);

    float result = math2d_inverselerp(start, end, value);
    lua_pushnumber(L, (lua_Number)result);

    return 1;
}

static int script_math2d_nearestdown(lua_State* L) {
    float value = (float)luaL_checknumber(L, 1);
    float step = (float)luaL_checknumber(L, 2);

    float result = math2d_nearestdown(value, step);
    lua_pushnumber(L, (lua_Number)result);

    return 1;
}

static int script_math2d_rotate_point_by_degs(lua_State* L) {
    float radians = (float)luaL_checknumber(L, 1);
    float x = (float)luaL_checknumber(L, 2);
    float y = (float)luaL_checknumber(L, 3);

    math2d_rotate_point(radians, &x, &y);
    lua_pushnumber(L, (lua_Number)x);
    lua_pushnumber(L, (lua_Number)y);

    return 2;
}

static int script_math2d_points_distance(lua_State* L) {
    float x1 = (float)luaL_checknumber(L, 1);
    float y1 = (float)luaL_checknumber(L, 2);
    float x2 = (float)luaL_checknumber(L, 3);
    float y2 = (float)luaL_checknumber(L, 4);

    float result = math2d_points_distance(x1, y1, x2, y2);
    lua_pushnumber(L, (lua_Number)result);

    return 1;
}


static const luaL_Reg EXPORTS_FUNCTION[] = {
    { "math2d_random", script_math2d_random },
    { "math2d_lerp", script_math2d_lerp },
    { "math2d_inverselerp", script_math2d_inverselerp },
    { "math2d_nearestdown", script_math2d_nearestdown },
    { "math2d_rotate_point_by_degs", script_math2d_rotate_point_by_degs },
    { "math2d_points_distance", script_math2d_points_distance },
    { NULL, NULL }
};

void script_math2d_register(lua_State* L) {
    for (size_t i = 0; ; i++) {
        if (EXPORTS_FUNCTION[i].name == NULL || EXPORTS_FUNCTION[i].func == NULL) break;
        lua_pushcfunction(L, EXPORTS_FUNCTION[i].func);
        lua_setglobal(L, EXPORTS_FUNCTION[i].name);
    }

    int ret = luaL_dostring(L, 
        "function math2d_random_boolean(chance)\n"
        "local value = math.random(0, 100)\n"
        "return value < chance\n"
        "end\n"
    );

    assert(ret == LUA_OK);
}

