#include "luascript_internal.h"
#include "engine_string.h"

#ifdef JAVASCRIPT
EM_JS_PRFX(bool, animlist_is_item_macro_animation, (AnimListItem animlist_item), {
    let ret = animlist_is_item_macro_animation(kdmyEngine_obtain(animlist_item));
    return ret ? 1 : 0;
});
EM_JS_PRFX(bool, animlist_is_item_frame_animation, (AnimListItem animlist_item), {
    let ret = animlist_is_item_frame_animation(kdmyEngine_obtain(animlist_item));
    return ret ? 1 : 0;
});
#endif


int script_animlistitem_index(lua_State* L) {
    AnimListItem animlistitem = luascript_read_userdata(L, ANIMLISTITEM);
    const char* field = luaL_optstring(L, 2, NULL);


    if (string_equals(field, "name")) {
#ifdef JAVASCRIPT
        char* name = kdmy_read_prop_string(animlistitem, "name");
        lua_pushstring(L, name);
        free(name);
#else
        lua_pushstring(L, animlistitem->name);
#endif
    } else if (string_equals(field, "isFrameAnimation")) {
        lua_pushboolean(
            L, animlist_is_item_frame_animation(animlistitem)
        );
    } else if (string_equals(field, "isItemMacroAnimation")) {
        lua_pushboolean(
            L, animlist_is_item_macro_animation(animlistitem)
        );
    } else if (string_equals(field, "isItemTweenlerpAnimation")){
        lua_pushboolean(
            L, !animlist_is_item_frame_animation(animlistitem) && !animlist_is_item_macro_animation(animlistitem)
        );
    } else {
        return luaL_error(L, "unknown field '%s'", field);
    }
    
    return 1;
}

int script_animlistitem_new(lua_State* L, AnimListItem animlistitem) {
    return luascript_userdata_new(L, ANIMLISTITEM, animlistitem);
}

static int script_animlistitem_gc(lua_State* L) {
    return luascript_userdata_gc(L, ANIMLISTITEM);
}

static int script_animlistitem_tostring(lua_State* L) {
    return luascript_userdata_tostring(L, ANIMLISTITEM);
}

void script_animlistitem_register(lua_State* L) {
    luaL_newmetatable(L, ANIMLISTITEM);

    lua_pushcfunction(L, script_animlistitem_gc);
    lua_setfield(L, -2, "__gc");

    lua_pushcfunction(L, script_animlistitem_tostring);
    lua_setfield(L, -2, "__tostring");

    lua_pushcfunction(L, script_animlistitem_index);
    lua_setfield(L, -2, "__index");
}

