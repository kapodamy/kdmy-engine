"use strict";

const ANIMSPRITE = "AnimSprite";


function script_animsprite_init_from_atlas(L) {
    let frame_rate = LUA.luaL_checknumber(L, 2);
    let loop = LUA.luaL_checkinteger(L, 3);
    let atlas = luascript_read_nullable_userdata(L, 4, ATLAS);
    let prefix = LUA.luaL_optstring(L, 5, null);
    let has_number_suffix = LUA.lua_toboolean(L, 6);

    let ret = animsprite_init_from_atlas(frame_rate, loop, atlas, prefix, has_number_suffix);

    return luascript_userdata_allocnew(L, ANIMSPRITE, ret);
}

function script_animsprite_init_from_animlist(L) {
    let animlist = luascript_read_nullable_userdata(L, 2, ANIMLIST);
    let animation_name = LUA.luaL_optstring(L, 3, null);

    if (!animlist) {
        return LUA.luaL_error(L, "animlist was nil (null)");
    }

    let ret = animsprite_init_from_animlist(animlist, animation_name);

    return luascript_userdata_allocnew(L, ANIMSPRITE, ret);
}

function script_animsprite_init_from_tweenlerp(L) {
    let name = LUA.luaL_optstring(L, 2, null);
    let loop = LUA.luaL_checkinteger(L, 3);
    let tweenlerp = luascript_read_nullable_userdata(L, 4, TWEENLERP);

    if (!tweenlerp) {
        return LUA.luaL_error(L, "tweenlerp was nil (null)");
    }

    let ret = animsprite_init_from_tweenlerp(name, loop, tweenlerp);

    return luascript_userdata_allocnew(L, ANIMSPRITE, ret);
}

function script_animsprite_init_as_empty(L) {
    let name = LUA.luaL_checkstring(L, 2);

    let ret = animsprite_init_as_empty(name);

    return luascript_userdata_allocnew(L, ANIMSPRITE, ret);
}

function script_animsprite_init(L) {
    let animlist_item = luascript_read_nullable_userdata(L, 2, ANIMLISTITEM);

    if (!animlist_item) {
        return LUA.luaL_error(L, "animlist_item was nil (null)");
    }

    let ret = animsprite_init(animlist_item);

    return luascript_userdata_allocnew(L, ANIMSPRITE, ret);
}

function script_animsprite_destroy(L) {
    let animsprite = luascript_read_userdata(L, ANIMSPRITE);

    if (luascript_userdata_is_allocated(L, ANIMSPRITE))
        animsprite_destroy(animsprite);
    else
        console.error("script_animsprite_destroy() object was not allocated by lua");

    return 0;
}

function script_animsprite_set_loop(L) {
    let animsprite = luascript_read_userdata(L, ANIMSPRITE);
    let loop = LUA.luaL_checkinteger(L, 2);

    animsprite_set_loop(animsprite, loop);

    return 0;
}

function script_animsprite_get_name(L) {
    let animsprite = luascript_read_userdata(L, ANIMSPRITE);

    let ret = animsprite_get_name(animsprite);

    LUA.lua_pushstring(L, ret);

    return 1;
}

function script_animsprite_is_frame_animation(L) {
    let animsprite = luascript_read_userdata(L, ANIMSPRITE);

    let ret = animsprite_is_frame_animation(animsprite);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_animsprite_set_delay(L) {
    let animsprite = luascript_read_userdata(L, ANIMSPRITE);
    let delay_milliseconds = LUA.luaL_checknumber(L, 2);

    animsprite_set_delay(animsprite, delay_milliseconds);

    return 0;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/**@type {luaL_Reg[]}*/const ANIMSPRITE_FUNCTIONS = [
    ["init_from_atlas", script_animsprite_init_from_atlas],
    ["init_from_animlist", script_animsprite_init_from_animlist],
    ["init_from_tweenlerp", script_animsprite_init_from_tweenlerp],
    ["init_as_empty", script_animsprite_init_as_empty],
    ["init", script_animsprite_init],
    ["destroy", script_animsprite_destroy],
    ["set_loop", script_animsprite_set_loop],
    ["get_name", script_animsprite_get_name],
    ["is_frame_animation", script_animsprite_is_frame_animation],
    ["set_delay", script_animsprite_set_delay],
    [null, null]
];


function script_animsprite_new(L, animsprite) {
    return luascript_userdata_new(L, ANIMSPRITE, animsprite);
}

function script_animsprite_gc(L) {
    return luascript_userdata_destroy(L, ANIMSPRITE, animsprite_destroy);
}

function script_animsprite_tostring(L) {
    return luascript_userdata_tostring(L, ANIMSPRITE);
}

function script_animsprite_register(L) {
    luascript_register(L, ANIMSPRITE, script_animsprite_gc, script_animsprite_tostring, ANIMSPRITE_FUNCTIONS);
}

