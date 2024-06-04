"use strict";


const TWEENKEYFRAME = "TweenKeyframe";


function script_tweenkeyframe_init(L) {
    let ret = tweenkeyframe_init();

    return luascript_userdata_allocnew(L, TWEENKEYFRAME, ret);
}

function script_tweenkeyframe_init2(L) {
    let animlistitem = luascript_read_nullable_userdata(L, 2, ANIMLISTITEM);

    if (!animlistitem) {
        return LUA.luaL_error(L, "animlistitem was nil (null)");
    }

    let ret = tweenkeyframe_init2(animlistitem);

    return luascript_userdata_allocnew(L, TWEENKEYFRAME, ret);
}

function script_tweenkeyframe_destroy(L) {
    let tweenkeyframe = luascript_read_userdata(L, TWEENKEYFRAME);

    if (luascript_userdata_is_allocated(L, TWEENKEYFRAME))
        tweenkeyframe_destroy(tweenkeyframe);
    else
        console.error("script_tweenkeyframe_destroy() object was not allocated by lua");

    return 0;
}

function script_tweenkeyframe_animate_percent(L) {
    let tweenkeyframe = luascript_read_userdata(L, TWEENKEYFRAME);
    let percent = LUA.luaL_checknumber(L, 2);

    tweenkeyframe_animate_percent(tweenkeyframe, percent);

    return 0;
}

function script_tweenkeyframe_get_ids_count(L) {
    let tweenkeyframe = luascript_read_userdata(L, TWEENKEYFRAME);

    let ret = tweenkeyframe_get_ids_count(tweenkeyframe);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_tweenkeyframe_peek_value(L) {
    let tweenkeyframe = luascript_read_userdata(L, TWEENKEYFRAME);

    let ret = tweenkeyframe_peek_value(tweenkeyframe);

    LUA.lua_pushnumber(L, ret);
    return 1;
}

function script_tweenkeyframe_peek_value_by_index(L) {
    let tweenkeyframe = luascript_read_userdata(L, TWEENKEYFRAME);
    let index = LUA.luaL_checkinteger(L, 2);

    let ret = tweenkeyframe_peek_value_by_index(tweenkeyframe, index);

    LUA.lua_pushnumber(L, ret);
    return 1;
}

function script_tweenkeyframe_peek_entry_by_index(L) {
    let tweenkeyframe = luascript_read_userdata(L, TWEENKEYFRAME);
    let index = LUA.luaL_checkinteger(L, 2);

    const output_id_value_pair = [0, 0.0];
    let ret = tweenkeyframe_peek_entry_by_index(tweenkeyframe, index, output_id_value_pair);

    if (!ret) {
        LUA.lua_pushnil(L);
        return 1;
    }

    LUA.lua_pushinteger(L, output_id_value_pair[0]);
    LUA.lua_pushnumber(L, output_id_value_pair[1]);
    return 2;
}

function script_tweenkeyframe_peek_value_by_id(L) {
    let tweenkeyframe = luascript_read_userdata(L, TWEENKEYFRAME);
    let id = LUA.luaL_checkinteger(L, 2);

    let ret = tweenkeyframe_peek_value_by_id(tweenkeyframe, id);

    LUA.lua_pushnumber(L, ret);
    return 1;
}

function script_tweenkeyframe_add_easeout(L) {
    let tweenkeyframe = luascript_read_userdata(L, TWEENKEYFRAME);
    let at = LUA.luaL_checknumber(L, 2);
    let id = LUA.luaL_checkinteger(L, 3);
    let value = LUA.luaL_checknumber(L, 4);

    let ret = tweenkeyframe_add_easeout(tweenkeyframe, at, id, value);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_tweenkeyframe_add_easeinout(L) {
    let tweenkeyframe = luascript_read_userdata(L, TWEENKEYFRAME);
    let at = LUA.luaL_checknumber(L, 2);
    let id = LUA.luaL_checkinteger(L, 3);
    let value = LUA.luaL_checknumber(L, 4);

    let ret = tweenkeyframe_add_easeinout(tweenkeyframe, at, id, value);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_tweenkeyframe_add_linear(L) {
    let tweenkeyframe = luascript_read_userdata(L, TWEENKEYFRAME);
    let at = LUA.luaL_checknumber(L, 2);
    let id = LUA.luaL_checkinteger(L, 3);
    let value = LUA.luaL_checknumber(L, 4);

    let ret = tweenkeyframe_add_linear(tweenkeyframe, at, id, value);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_tweenkeyframe_add_steps(L) {
    let tweenkeyframe = luascript_read_userdata(L, TWEENKEYFRAME);
    let at = LUA.luaL_checknumber(L, 2);
    let id = LUA.luaL_checkinteger(L, 3);
    let value = LUA.luaL_checknumber(L, 4);
    let steps_count = LUA.luaL_checkinteger(L, 5);
    let steps_method = luascript_helper_optenum(L, 6, LUASCRIPT_ENUMS_Align);

    let ret = tweenkeyframe_add_steps(tweenkeyframe, at, id, value, steps_count, steps_method);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_tweenkeyframe_add_ease(L) {
    let tweenkeyframe = luascript_read_userdata(L, TWEENKEYFRAME);
    let at = LUA.luaL_checknumber(L, 2);
    let id = LUA.luaL_checkinteger(L, 3);
    let value = LUA.luaL_checknumber(L, 4);

    let ret = tweenkeyframe_add_ease(tweenkeyframe, at, id, value);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_tweenkeyframe_add_easein(L) {
    let tweenkeyframe = luascript_read_userdata(L, TWEENKEYFRAME);
    let at = LUA.luaL_checknumber(L, 2);
    let id = LUA.luaL_checkinteger(L, 3);
    let value = LUA.luaL_checknumber(L, 4);

    let ret = tweenkeyframe_add_easein(tweenkeyframe, at, id, value);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_tweenkeyframe_add_cubic(L) {
    let tweenkeyframe = luascript_read_userdata(L, TWEENKEYFRAME);
    let at = LUA.luaL_checknumber(L, 2);
    let id = LUA.luaL_checkinteger(L, 3);
    let value = LUA.luaL_checknumber(L, 4);

    let ret = tweenkeyframe_add_cubic(tweenkeyframe, at, id, value);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_tweenkeyframe_add_quad(L) {
    let tweenkeyframe = luascript_read_userdata(L, TWEENKEYFRAME);
    let at = LUA.luaL_checknumber(L, 2);
    let id = LUA.luaL_checkinteger(L, 3);
    let value = LUA.luaL_checknumber(L, 4);

    let ret = tweenkeyframe_add_quad(tweenkeyframe, at, id, value);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_tweenkeyframe_add_expo(L) {
    let tweenkeyframe = luascript_read_userdata(L, TWEENKEYFRAME);
    let at = LUA.luaL_checknumber(L, 2);
    let id = LUA.luaL_checkinteger(L, 3);
    let value = LUA.luaL_checknumber(L, 4);

    let ret = tweenkeyframe_add_expo(tweenkeyframe, at, id, value);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_tweenkeyframe_add_sin(L) {
    let tweenkeyframe = luascript_read_userdata(L, TWEENKEYFRAME);
    let at = LUA.luaL_checknumber(L, 2);
    let id = LUA.luaL_checkinteger(L, 3);
    let value = LUA.luaL_checknumber(L, 4);

    let ret = tweenkeyframe_add_sin(tweenkeyframe, at, id, value);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_tweenkeyframe_add_interpolator(L) {
    let tweenkeyframe = luascript_read_userdata(L, TWEENKEYFRAME);
    let at = LUA.luaL_checknumber(L, 2);
    let id = LUA.luaL_checkinteger(L, 3);
    let value = LUA.luaL_checknumber(L, 4);
    let type = luascript_helper_optenum(L, 5, LUASCRIPT_ENUMS_AnimInterpolator);

    let ret = tweenkeyframe_add_interpolator(tweenkeyframe, at, id, value, type);

    LUA.lua_pushinteger(L, ret);
    return 1;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/**@type {luaL_Reg[]}*/const TWEENKEYFRAME_FUNCTIONS = [
    ["init", script_tweenkeyframe_init],
    ["init2", script_tweenkeyframe_init2],
    ["destroy", script_tweenkeyframe_destroy],
    ["animate_percent", script_tweenkeyframe_animate_percent],
    ["get_ids_count", script_tweenkeyframe_get_ids_count],
    ["peek_value", script_tweenkeyframe_peek_value],
    ["peek_value_by_index", script_tweenkeyframe_peek_value_by_index],
    ["peek_entry_by_index", script_tweenkeyframe_peek_entry_by_index],
    ["peek_value_by_id", script_tweenkeyframe_peek_value_by_id],
    ["add_easeout", script_tweenkeyframe_add_easeout],
    ["add_easeinout", script_tweenkeyframe_add_easeinout],
    ["add_linear", script_tweenkeyframe_add_linear],
    ["add_steps", script_tweenkeyframe_add_steps],
    ["add_ease", script_tweenkeyframe_add_ease],
    ["add_easein", script_tweenkeyframe_add_easein],
    ["add_cubic", script_tweenkeyframe_add_cubic],
    ["add_quad", script_tweenkeyframe_add_quad],
    ["add_expo", script_tweenkeyframe_add_expo],
    ["add_sin", script_tweenkeyframe_add_sin],
    ["add_interpolator", script_tweenkeyframe_add_interpolator],
    [null, null]
];

function script_tweenkeyframe_new(L, tweenkeyframe) {
    return luascript_userdata_new(L, TWEENKEYFRAME, tweenkeyframe);
}

function script_tweenkeyframe_gc(L) {
    return luascript_userdata_destroy(L, TWEENKEYFRAME, tweenkeyframe_destroy);
}

function script_tweenkeyframe_tostring(L) {
    return luascript_userdata_tostring(L, TWEENKEYFRAME);
}

function script_tweenkeyframe_register(L) {
    luascript_register(L, TWEENKEYFRAME, script_tweenkeyframe_gc, script_tweenkeyframe_tostring, TWEENKEYFRAME_FUNCTIONS);
}

