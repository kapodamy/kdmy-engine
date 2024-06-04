"use strict";


function script_math2d_random(L) {
    let ret;

    if (LUA.lua_gettop(L) == 0) {
        ret = math2d_random_float();
    } else {
        let start = LUA.luaL_checknumber(L, 1);
        let end = LUA.luaL_checknumber(L, 2);
        ret = math2d_random_float_range(start, end);
    }

    LUA.lua_pushnumber(L, ret);
    return 1;
}

function script_math2d_lerp(L) {
    let start = LUA.luaL_checknumber(L, 1);
    let end = LUA.luaL_checknumber(L, 2);
    let step = LUA.luaL_checknumber(L, 3);

    let result = math2d_lerp(start, end, step);
    LUA.lua_pushnumber(L, result);

    return 1;
}

function script_math2d_inverselerp(L) {
    let start = LUA.luaL_checknumber(L, 1);
    let end = LUA.luaL_checknumber(L, 2);
    let value = LUA.luaL_checknumber(L, 3);

    let result = math2d_inverselerp(start, end, value);
    LUA.lua_pushnumber(L, result);

    return 1;
}

function script_math2d_nearestdown(L) {
    let value = LUA.luaL_checknumber(L, 1);
    let step = LUA.luaL_checknumber(L, 2);

    let result = math2d_nearestdown(value, step);
    LUA.lua_pushnumber(L, result);

    return 1;
}

function script_math2d_rotate_point_by_degs(L) {
    let degs = LUA.luaL_checknumber(L, 1);
    let x = LUA.luaL_checknumber(L, 2);
    let y = LUA.luaL_checknumber(L, 3);

    const point = [x, y];
    math2d_rotate_point(degs * MATH2D_DEG_TO_RAD, point);
    LUA.lua_pushnumber(L, point[0]);
    LUA.lua_pushnumber(L, point[1]);

    return 2;
}

function script_math2d_points_distance(L) {
    let x1 = LUA.luaL_checknumber(L, 1);
    let y1 = LUA.luaL_checknumber(L, 2);
    let x2 = LUA.luaL_checknumber(L, 3);
    let y2 = LUA.luaL_checknumber(L, 4);

    let result = math2d_points_distance(x1, y1, x2, y2);
    LUA.lua_pushnumber(L, result);

    return 1;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/**@type {luaL_Reg[]}*/const EXPORTS_FUNCTION = [
    ["math2d_random", script_math2d_random],
    ["math2d_lerp", script_math2d_lerp],
    ["math2d_inverselerp", script_math2d_inverselerp],
    ["math2d_nearestdown", script_math2d_nearestdown],
    ["math2d_rotate_point_by_degs", script_math2d_rotate_point_by_degs],
    ["math2d_points_distance", script_math2d_points_distance],
    [null, null]
];

function script_math2d_register(L) {
    for (let i = 0; ; i++) {
        if (EXPORTS_FUNCTION[i][0] == null || EXPORTS_FUNCTION[i][1] == null) break;
        LUA.lua_pushcfunction(L, EXPORTS_FUNCTION[i][1]);
        LUA.lua_setglobal(L, EXPORTS_FUNCTION[i][0]);
    }

    let ret = LUA.luaL_dostring(L, "function math2d_random_boolean(chance)\n" +
        "local value = math.random(0, 100)\n" +
        "return value < chance\n" +
        "end\n");

    console.assert(ret == LUA.LUA_OK);
}
