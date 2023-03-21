#include "luascript_internal.h"

#ifdef JAVASCRIPT
EM_ASYNC_JS_PRFX(AnimList, animlist_init, (const char* src), {
    let ret = await animlist_init(kdmyEngine_ptrToString(src));
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(void, animlist_destroy, (AnimList* animlist), {
    animlist_destroy(kdmyEngine_obtain(kdmyEngine_get_uint32(animlist)));
});
EM_JS_PRFX(AnimListItem, animlist_get_animation, (AnimList animlist, const char* animation_name), {
    let ret = animlist_get_animation(kdmyEngine_obtain(animlist), kdmyEngine_ptrToString(animation_name));
    return kdmyEngine_obtain(ret);
});
#endif


static int script_animlist_init(lua_State* L) {
    const char* src = luaL_checkstring(L, 2);

    AnimList ret = animlist_init(src);

    return luascript_userdata_allocnew(L, ANIMLIST, ret);
}

static int script_animlist_destroy(lua_State* L) {
    AnimList animlist = luascript_read_userdata(L, ANIMLIST);
    
    if (luascript_userdata_is_allocated(L, ANIMLIST))
        animlist_destroy(&animlist);
    else
        printf("script_animlist_destroy() object was not allocated by lua\n");

    return 0;
}

static int script_animlist_get_animation(lua_State* L) {
    AnimList animlist = luascript_read_userdata(L, ANIMLIST);
    const char* animation_name = luaL_optstring(L, 2, NULL);

    AnimListItem ret = animlist_get_animation(animlist, animation_name);

    return script_animlistitem_new(L, ret);
}



static const luaL_Reg ANIMLIST_FUNCTIONS[] = {
    { "init", script_animlist_init },
    { "destroy", script_animlist_destroy },
    { "get_animation", script_animlist_get_animation },
    { NULL, NULL }
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

