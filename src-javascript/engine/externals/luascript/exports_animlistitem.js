"use strict";

const ANIMLISTITEM = "AnimListItem";


function script_animlistitem_index(L) {
    const animlistitem = luascript_read_userdata(L, ANIMLISTITEM);
    const field = LUA.luaL_optstring(L, 2, null);

    switch (field) {
        case "name":
            LUA.lua_pushstring(L, animlistitem.name);
            break;
        case "isFrameAnimation":
            LUA.lua_pushboolean(
                L, animlist_is_item_frame_animation(animlistitem)
            );
            break;
        case "isMacroAnimation":
            LUA.lua_pushboolean(
                L, animlist_is_item_macro_animation(animlistitem)
            );
            break;
        case "isTweenKeyframeAnimation":
            LUA.lua_pushboolean(
                L, animlist_is_item_tweenkeyframe_animation(animlistitem)
            );
        default:
            return LUA.luaL_error(L, `unknown field '${field}'`);
    }

    return 1;
}

function script_animlistitem_new(L, animlistitem) {
    return luascript_userdata_new(L, ANIMLISTITEM, animlistitem);
}

function script_animlistitem_gc(L) {
    return luascript_userdata_gc(L, ANIMLISTITEM);
}

function script_animlistitem_tostring(L) {
    return luascript_userdata_tostring(L, ANIMLISTITEM);
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

function script_animlistitem_register(L) {
    LUA.luaL_newmetatable(L, ANIMLISTITEM);

    LUA.lua_pushcfunction(L, script_animlistitem_gc);
    LUA.lua_setfield(L, -2, "__gc");

    LUA.lua_pushcfunction(L, script_animlistitem_tostring);
    LUA.lua_setfield(L, -2, "__tostring");

    LUA.lua_pushcfunction(L, script_animlistitem_index);
    LUA.lua_setfield(L, -2, "__index");
}

