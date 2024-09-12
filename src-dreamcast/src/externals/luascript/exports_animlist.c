#include "luascript_internal.h"

#include "animlist.h"
#include "logger.h"


static int script_animlist_init(lua_State* L) {
    const char* src = luaL_checkstring(L, 2);

    luascript_change_working_folder(L);
    AnimList ret = animlist_init(src);
    luascript_restore_working_folder(L);

    return luascript_userdata_allocnew(L, ANIMLIST, ret);
}

static int script_animlist_destroy(lua_State* L) {
    AnimList animlist = luascript_read_userdata(L, ANIMLIST);

    if (luascript_userdata_is_allocated(L, ANIMLIST))
        animlist_destroy(&animlist);
    else
        logger_warn("script_animlist_destroy() object was not allocated by lua");

    return 0;
}

static int script_animlist_get_animation(lua_State* L) {
    AnimList animlist = luascript_read_userdata(L, ANIMLIST);
    const char* animation_name = luaL_optstring(L, 2, NULL);

    const AnimListItem* ret = animlist_get_animation(animlist, animation_name);

    return script_animlistitem_new(L, ret);
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const luaL_Reg ANIMLIST_FUNCTIONS[] = {
    {"init", script_animlist_init},
    {"destroy", script_animlist_destroy},
    {"get_animation", script_animlist_get_animation},
    {NULL, NULL}
};


int script_animlist_new(lua_State* L, AnimList animlist) {
    return luascript_userdata_new(L, ANIMLIST, animlist);
}

static int script_animlist_gc(lua_State* L) {
    return luascript_userdata_destroy(L, ANIMLIST, (Destructor)animlist_destroy);
}

static int script_animlist_tostring(lua_State* L) {
    return luascript_userdata_tostring(L, ANIMLIST);
}

void script_animlist_register(lua_State* L) {
    luascript_register(L, ANIMLIST, script_animlist_gc, script_animlist_tostring, ANIMLIST_FUNCTIONS);
}
