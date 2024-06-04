"use strict";


const ANIMLIST = "AnimList";


async function script_animlist_init(L) {
    let src = LUA.luaL_checkstring(L, 2);

    luascript_change_working_folder(L);
    let ret = await animlist_init(src);
    luascript_restore_working_folder(L);

    return luascript_userdata_allocnew(L, ANIMLIST, ret);
}

function script_animlist_destroy(L) {
    let animlist = luascript_read_userdata(L, ANIMLIST);

    if (luascript_userdata_is_allocated(L, ANIMLIST))
        animlist_destroy(animlist);
    else
        console.error("script_animlist_destroy() object was not allocated by lua\n");

    return 0;
}

function script_animlist_get_animation(L) {
    let animlist = luascript_read_userdata(L, ANIMLIST);
    let animation_name = LUA.luaL_optstring(L, 2, null);

    let ret = animlist_get_animation(animlist, animation_name);

    return script_animlistitem_new(L, ret);
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/**@type {luaL_Reg[]}*/const ANIMLIST_FUNCTIONS = [
    ["init", script_animlist_init],
    ["destroy", script_animlist_destroy],
    ["get_animation", script_animlist_get_animation],
    [null, null]
];


function script_animlist_new(L, animlist) {
    return luascript_userdata_new(L, ANIMLIST, animlist);
}

function script_animlist_gc(L) {
    return luascript_userdata_destroy(L, ANIMLIST, animlist_destroy);
}

function script_animlist_tostring(L) {
    return luascript_userdata_tostring(L, ANIMLIST);
}

function script_animlist_register(L) {
    luascript_register(L, ANIMLIST, script_animlist_gc, script_animlist_tostring, ANIMLIST_FUNCTIONS);
}

