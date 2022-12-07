#include "luascript_internal.h"

#ifdef JAVASCRIPT
EM_JS_PRFX(AnimSprite, animsprite_init_from_atlas, (float frame_rate, int32_t loop, Atlas atlas, const char* prefix, bool has_number_suffix), {
    let ret = animsprite_init_from_atlas(frame_rate, loop, kdmyEngine_obtain(atlas), kdmyEngine_ptrToString(prefix), has_number_suffix);
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(AnimSprite, animsprite_init_from_animlist, (AnimList animlist, const char* animation_name), {
    let ret = animsprite_init_from_animlist(kdmyEngine_obtain(animlist), kdmyEngine_ptrToString(animation_name));
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(AnimSprite, animsprite_init_as_empty, (const char* name), {
    let ret = animsprite_init_as_empty(kdmyEngine_ptrToString(name));
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(AnimSprite, animsprite_init, (AnimListItem animlist_item), {
    let ret = animsprite_init(kdmyEngine_obtain(animlist_item));
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(void, animsprite_destroy, (AnimSprite* animsprite), {
    animsprite_destroy(kdmyEngine_obtain(kdmyEngine_get_uint32(animsprite)));
});
EM_JS_PRFX(void, animsprite_set_loop, (AnimSprite animsprite, int32_t loop), {
    animsprite_set_loop(kdmyEngine_obtain(animsprite), loop);
});
EM_JS_PRFX(void, animsprite_restart, (AnimSprite animsprite), {
    animsprite_restart(kdmyEngine_obtain(animsprite));
});
EM_JS_PRFX(const char*, animsprite_get_name, (AnimSprite animsprite), {
    let ret = animsprite_get_name(kdmyEngine_obtain(animsprite));
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(bool, animsprite_is_frame_animation, (AnimSprite animsprite), {
    let ret = animsprite_is_frame_animation(kdmyEngine_obtain(animsprite));
    return ret ? 1 : 0;
});
EM_JS_PRFX(void, animsprite_set_delay, (AnimSprite animsprite, float delay_milliseconds), {
    animsprite_set_delay(kdmyEngine_obtain(animsprite), delay_milliseconds);
});
#endif


static int script_animsprite_init_from_atlas(lua_State* L) {
    float frame_rate = (float)luaL_checknumber(L, 1);
    int32_t loop = (int32_t)luaL_checkinteger(L, 2);
    Atlas atlas = luascript_read_nullable_userdata(L, 3, ATLAS);
    const char* prefix = luaL_optstring(L, 4, NULL);
    bool has_number_suffix = (bool)lua_toboolean(L, 5);

    AnimSprite ret = animsprite_init_from_atlas(frame_rate, loop, atlas, prefix, has_number_suffix);

    return luascript_userdata_allocnew(L, ANIMSPRITE, ret);
}

static int script_animsprite_init_from_animlist(lua_State* L) {
    AnimList animlist = luascript_read_nullable_userdata(L, 1, ANIMLIST);
    const char* animation_name = luaL_optstring(L, 2, NULL);

    AnimSprite ret = animsprite_init_from_animlist(animlist, animation_name);

    return luascript_userdata_allocnew(L, ANIMSPRITE, ret);
}

static int script_animsprite_init_as_empty(lua_State* L) {
    const char* name = luaL_checkstring(L, 1);

    AnimSprite ret = animsprite_init_as_empty(name);

    return luascript_userdata_allocnew(L, ANIMSPRITE, ret);
}

static int script_animsprite_init(lua_State* L) {
    AnimListItem animlist_item = luascript_read_nullable_userdata(L, 1, ANIMLISTITEM);

    AnimSprite ret = animsprite_init(animlist_item);

    return luascript_userdata_allocnew(L, ANIMSPRITE, ret);
}

static int script_animsprite_destroy(lua_State* L) {
    AnimSprite animsprite = luascript_read_userdata(L, ANIMSPRITE);

    if (luascript_userdata_is_allocated(L, ANIMSPRITE))
        animsprite_destroy(&animsprite);
    else
        printf("script_animsprite_destroy() object was not allocated by lua\n");

    return 0;
}

static int script_animsprite_set_loop(lua_State* L) {
    AnimSprite animsprite = luascript_read_userdata(L, ANIMSPRITE);
    int32_t loop = (int32_t)luaL_checkinteger(L, 2);

    animsprite_set_loop(animsprite, loop);

    return 0;
}

static int script_animsprite_get_name(lua_State* L) {
    AnimSprite animsprite = luascript_read_userdata(L, ANIMSPRITE);

    const char* ret = animsprite_get_name(animsprite);

    lua_pushstring(L, ret);

#ifdef JAVASCRIPT
    free((char*)ret);
#endif
    return 1;
}

static int script_animsprite_is_frame_animation(lua_State* L) {
    AnimSprite animsprite = luascript_read_userdata(L, ANIMSPRITE);

    bool ret = animsprite_is_frame_animation(animsprite);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_animsprite_set_delay(lua_State* L) {
    AnimSprite animsprite = luascript_read_userdata(L, ANIMSPRITE);
    float delay_milliseconds = (float)luaL_checknumber(L, 2);

    animsprite_set_delay(animsprite, delay_milliseconds);

    return 0;
}




static const luaL_Reg ANIMSPRITE_FUNCTIONS[] = {
    { "init_from_atlas", script_animsprite_init_from_atlas },
    { "init_from_animlist", script_animsprite_init_from_animlist },
    { "init_as_empty", script_animsprite_init_as_empty },
    { "init", script_animsprite_init },
    { "destroy", script_animsprite_destroy },
    { "set_loop", script_animsprite_set_loop },
    { "get_name", script_animsprite_get_name },
    { "is_frame_animation", script_animsprite_is_frame_animation },
    { "set_delay", script_animsprite_set_delay },
    { NULL, NULL }
};

int script_animsprite_new(lua_State* L, AnimSprite animsprite) {
    return luascript_userdata_new(L, ANIMSPRITE, animsprite);
}

static int script_animsprite_gc(lua_State* L) {
    return luascript_userdata_destroy(L, ANIMSPRITE, (Destructor)animlist_destroy);
}

static int script_animsprite_tostring(lua_State* L) {
    return luascript_userdata_tostring(L, ANIMSPRITE);
}

void script_animsprite_register(lua_State* L) {
    luascript_register(L, ANIMSPRITE, script_animsprite_gc, script_animsprite_tostring, ANIMSPRITE_FUNCTIONS);
}

