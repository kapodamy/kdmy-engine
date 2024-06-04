"use strict";

const TWEENLERP = "TweenLerp";


function script_tweenlerp_init(L) {
    let tweenlerp = tweenlerp_init();
    return luascript_userdata_allocnew(L, TWEENLERP, tweenlerp);
}

function script_tweenlerp_destroy(L) {
    let tweenlerp = luascript_read_userdata(L, TWEENLERP);

    if (luascript_userdata_is_allocated(L, TWEENLERP))
        tweenlerp_destroy(tweenlerp);
    else
        console.error("script_tweenlerp_destroy() object was not allocated by lua");

    return 0;
}

function script_tweenlerp_end(L) {
    let tweenlerp = luascript_read_userdata(L, TWEENLERP);

    tweenlerp_end(tweenlerp);

    return 0;
}

function script_tweenlerp_mark_as_completed(L) {
    let tweenlerp = luascript_read_userdata(L, TWEENLERP);

    tweenlerp_mark_as_completed(tweenlerp);

    return 0;
}

function script_tweenlerp_restart(L) {
    let tweenlerp = luascript_read_userdata(L, TWEENLERP);

    tweenlerp_restart(tweenlerp);

    return 0;
}

function script_tweenlerp_animate(L) {
    let tweenlerp = luascript_read_userdata(L, TWEENLERP);
    let elapsed = LUA.luaL_checknumber(L, 2);

    let ret = tweenlerp_animate(tweenlerp, elapsed);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_tweenlerp_animate_percent(L) {
    let tweenlerp = luascript_read_userdata(L, TWEENLERP);
    let percent = LUA.luaL_checknumber(L, 2);

    let ret = tweenlerp_animate_percent(tweenlerp, percent);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_tweenlerp_is_completed(L) {
    let tweenlerp = luascript_read_userdata(L, TWEENLERP);

    let ret = tweenlerp_is_completed(tweenlerp);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_tweenlerp_get_elapsed(L) {
    let tweenlerp = luascript_read_userdata(L, TWEENLERP);

    let ret = tweenlerp_get_elapsed(tweenlerp);

    LUA.lua_pushnumber(L, ret);
    return 1;
}

function script_tweenlerp_get_entry_count(L) {
    let tweenlerp = luascript_read_userdata(L, TWEENLERP);

    let ret = tweenlerp_get_entry_count(tweenlerp);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_tweenlerp_peek_value(L) {
    let tweenlerp = luascript_read_userdata(L, TWEENLERP);

    let ret = tweenlerp_peek_value(tweenlerp);

    LUA.lua_pushnumber(L, ret);
    return 1;
}

function script_tweenlerp_peek_value_by_index(L) {
    let tweenlerp = luascript_read_userdata(L, TWEENLERP);
    let index = LUA.luaL_checkinteger(L, 2);

    let ret = tweenlerp_peek_value_by_index(tweenlerp, index);

    LUA.lua_pushnumber(L, ret);
    return 1;
}

function script_tweenlerp_peek_entry_by_index(L) {
    let tweenlerp = luascript_read_userdata(L, TWEENLERP);
    let index = LUA.luaL_checkinteger(L, 2);

    const output_id_value_duration_pair = [0, 0.0, 0.0];
    let ret = tweenlerp_peek_entry_by_index(tweenlerp, index, output_id_value_duration_pair);

    if (ret) {
        LUA.lua_pushinteger(L, output_id_value_duration_pair[0]);
        LUA.lua_pushnumber(L, output_id_value_duration_pair[1]);
        LUA.lua_pushnumber(L, output_id_value_duration_pair[2]);
        return 3;
    } else {
        LUA.lua_pushnil(L);
        return 1;
    }

}

function script_tweenlerp_peek_value_by_id(L) {
    let tweenlerp = luascript_read_userdata(L, TWEENLERP);
    let id = LUA.luaL_checkinteger(L, 2);

    let ret = tweenlerp_peek_value_by_id(tweenlerp, id);

    LUA.lua_pushnumber(L, ret);
    return 1;
}

function script_tweenlerp_change_bounds_by_index(L) {
    let tweenlerp = luascript_read_userdata(L, TWEENLERP);
    let index = LUA.luaL_checkinteger(L, 2);
    let new_start = LUA.luaL_checknumber(L, 3);
    let new_end = LUA.luaL_checknumber(L, 4);

    let ret = tweenlerp_change_bounds_by_index(tweenlerp, index, new_start, new_end);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_tweenlerp_override_start_with_end_by_index(L) {
    let tweenlerp = luascript_read_userdata(L, TWEENLERP);
    let index = LUA.luaL_checkinteger(L, 2);

    let ret = tweenlerp_override_start_with_end_by_index(tweenlerp, index);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_tweenlerp_change_bounds_by_id(L) {
    let tweenlerp = luascript_read_userdata(L, TWEENLERP);
    let id = LUA.luaL_checkinteger(L, 2);
    let new_start = LUA.luaL_checknumber(L, 3);
    let new_end = LUA.luaL_checknumber(L, 4);

    let ret = tweenlerp_change_bounds_by_id(tweenlerp, id, new_start, new_end);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_tweenlerp_change_duration_by_index(L) {
    let tweenlerp = luascript_read_userdata(L, TWEENLERP);
    let index = LUA.luaL_checkinteger(L, 2);
    let new_duration = LUA.luaL_checknumber(L, 3);

    let ret = tweenlerp_change_duration_by_index(tweenlerp, index, new_duration);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_tweenlerp_swap_bounds_by_index(L) {
    let tweenlerp = luascript_read_userdata(L, TWEENLERP);
    let index = LUA.luaL_checkinteger(L, 2);

    let ret = tweenlerp_swap_bounds_by_index(tweenlerp, index);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_tweenlerp_add_ease(L) {
    let tweenlerp = luascript_read_userdata(L, TWEENLERP);
    let id = LUA.luaL_checkinteger(L, 2);
    let start = LUA.luaL_checknumber(L, 3);
    let end = LUA.luaL_checknumber(L, 4);
    let duration = LUA.luaL_checknumber(L, 5);

    let ret = tweenlerp_add_ease(tweenlerp, id, start, end, duration);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_tweenlerp_add_easein(L) {
    let tweenlerp = luascript_read_userdata(L, TWEENLERP);
    let id = LUA.luaL_checkinteger(L, 2);
    let start = LUA.luaL_checknumber(L, 3);
    let end = LUA.luaL_checknumber(L, 4);
    let duration = LUA.luaL_checknumber(L, 5);

    let ret = tweenlerp_add_easein(tweenlerp, id, start, end, duration);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_tweenlerp_add_easeout(L) {
    let tweenlerp = luascript_read_userdata(L, TWEENLERP);
    let id = LUA.luaL_checkinteger(L, 2);
    let start = LUA.luaL_checknumber(L, 3);
    let end = LUA.luaL_checknumber(L, 4);
    let duration = LUA.luaL_checknumber(L, 5);

    let ret = tweenlerp_add_easeout(tweenlerp, id, start, end, duration);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_tweenlerp_add_easeinout(L) {
    let tweenlerp = luascript_read_userdata(L, TWEENLERP);
    let id = LUA.luaL_checkinteger(L, 2);
    let start = LUA.luaL_checknumber(L, 3);
    let end = LUA.luaL_checknumber(L, 4);
    let duration = LUA.luaL_checknumber(L, 5);

    let ret = tweenlerp_add_easeinout(tweenlerp, id, start, end, duration);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_tweenlerp_add_linear(L) {
    let tweenlerp = luascript_read_userdata(L, TWEENLERP);
    let id = LUA.luaL_checkinteger(L, 2);
    let start = LUA.luaL_checknumber(L, 3);
    let end = LUA.luaL_checknumber(L, 4);
    let duration = LUA.luaL_checknumber(L, 5);

    let ret = tweenlerp_add_linear(tweenlerp, id, start, end, duration);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_tweenlerp_add_steps(L) {
    let tweenlerp = luascript_read_userdata(L, TWEENLERP);
    let id = LUA.luaL_checkinteger(L, 2);
    let start = LUA.luaL_checknumber(L, 3);
    let end = LUA.luaL_checknumber(L, 4);
    let duration = LUA.luaL_checknumber(L, 5);
    let steps_count = LUA.luaL_checkinteger(L, 6);
    let steps_method = luascript_helper_optenum(L, 7, LUASCRIPT_ENUMS_Align);

    let ret = tweenlerp_add_steps(tweenlerp, id, start, end, duration, steps_count, steps_method);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_tweenlerp_add_cubic(L) {
    let tweenlerp = luascript_read_userdata(L, TWEENLERP);
    let id = LUA.luaL_checkinteger(L, 2);
    let start = LUA.luaL_checknumber(L, 3);
    let end = LUA.luaL_checknumber(L, 4);
    let duration = LUA.luaL_checknumber(L, 5);

    let ret = tweenlerp_add_cubic(tweenlerp, id, start, end, duration);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_tweenlerp_add_quad(L) {
    let tweenlerp = luascript_read_userdata(L, TWEENLERP);
    let id = LUA.luaL_checkinteger(L, 2);
    let start = LUA.luaL_checknumber(L, 3);
    let end = LUA.luaL_checknumber(L, 4);
    let duration = LUA.luaL_checknumber(L, 5);

    let ret = tweenlerp_add_quad(tweenlerp, id, start, end, duration);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_tweenlerp_add_expo(L) {
    let tweenlerp = luascript_read_userdata(L, TWEENLERP);
    let id = LUA.luaL_checkinteger(L, 2);
    let start = LUA.luaL_checknumber(L, 3);
    let end = LUA.luaL_checknumber(L, 4);
    let duration = LUA.luaL_checknumber(L, 5);

    let ret = tweenlerp_add_expo(tweenlerp, id, start, end, duration);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_tweenlerp_add_sin(L) {
    let tweenlerp = luascript_read_userdata(L, TWEENLERP);
    let id = LUA.luaL_checkinteger(L, 2);
    let start = LUA.luaL_checknumber(L, 3);
    let end = LUA.luaL_checknumber(L, 4);
    let duration = LUA.luaL_checknumber(L, 5);

    let ret = tweenlerp_add_sin(tweenlerp, id, start, end, duration);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_tweenlerp_add_interpolator(L) {
    let tweenlerp = luascript_read_userdata(L, TWEENLERP);
    let id = LUA.luaL_checkinteger(L, 2);
    let start = LUA.luaL_checknumber(L, 3);
    let end = LUA.luaL_checknumber(L, 4);
    let duration = LUA.luaL_checknumber(L, 5);
    let type = luascript_helper_optenum(L, 6, LUASCRIPT_ENUMS_AnimInterpolator);

    let ret = tweenlerp_add_interpolator(tweenlerp, id, start, end, duration, type);

    LUA.lua_pushinteger(L, ret);
    return 1;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/**@type {luaL_Reg[]}*/const TWEENLERP_FUNCTIONS = [
    ["init", script_tweenlerp_init],
    ["destroy", script_tweenlerp_destroy],
    ["end", script_tweenlerp_end],
    ["mark_as_completed", script_tweenlerp_mark_as_completed],
    ["restart", script_tweenlerp_restart],
    ["animate", script_tweenlerp_animate],
    ["animate_percent", script_tweenlerp_animate_percent],
    ["is_completed", script_tweenlerp_is_completed],
    ["get_elapsed", script_tweenlerp_get_elapsed],
    ["get_entry_count", script_tweenlerp_get_entry_count],
    ["peek_value", script_tweenlerp_peek_value],
    ["peek_value_by_index", script_tweenlerp_peek_value_by_index],
    ["peek_entry_by_index", script_tweenlerp_peek_entry_by_index],
    ["peek_value_by_id", script_tweenlerp_peek_value_by_id],
    ["change_bounds_by_index", script_tweenlerp_change_bounds_by_index],
    ["override_start_with_end_by_index", script_tweenlerp_override_start_with_end_by_index],
    ["change_bounds_by_id", script_tweenlerp_change_bounds_by_id],
    ["change_duration_by_index", script_tweenlerp_change_duration_by_index],
    ["swap_bounds_by_index", script_tweenlerp_swap_bounds_by_index],
    ["add_ease", script_tweenlerp_add_ease],
    ["add_easein", script_tweenlerp_add_easein],
    ["add_easeout", script_tweenlerp_add_easeout],
    ["add_easeinout", script_tweenlerp_add_easeinout],
    ["add_linear", script_tweenlerp_add_linear],
    ["add_steps", script_tweenlerp_add_steps],
    ["add_cubic", script_tweenlerp_add_cubic],
    ["add_quad", script_tweenlerp_add_quad],
    ["add_expo", script_tweenlerp_add_expo],
    ["add_sin", script_tweenlerp_add_sin],
    ["add_interpolator", script_tweenlerp_add_interpolator],
    [null, null]
];

function script_tweenlerp_new(L, tweenlerp) {
    return luascript_userdata_new(L, TWEENLERP, tweenlerp);
}

function script_tweenlerp_gc(L) {
    return luascript_userdata_destroy(L, TWEENLERP, tweenlerp_destroy);
}

function script_tweenlerp_tostring(L) {
    return luascript_userdata_tostring(L, TWEENLERP);
}

function script_tweenlerp_register(L) {
    luascript_register(L, TWEENLERP, script_tweenlerp_gc, script_tweenlerp_tostring, TWEENLERP_FUNCTIONS);
}

