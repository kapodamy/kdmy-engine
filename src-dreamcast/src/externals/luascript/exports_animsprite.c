#include "luascript_internal.h"

#include "animsprite.h"
#include "logger.h"


static int script_animsprite_init_from_atlas(lua_State* L) {
    float frame_rate = (float)luaL_checknumber(L, 2);
    int32_t loop = (int32_t)luaL_checkinteger(L, 3);
    Atlas atlas = luascript_read_nullable_userdata(L, 4, ATLAS);
    const char* prefix = luaL_optstring(L, 5, NULL);
    bool has_number_suffix = (bool)lua_toboolean(L, 6);

    AnimSprite ret = animsprite_init_from_atlas(frame_rate, loop, atlas, prefix, has_number_suffix);

    return luascript_userdata_allocnew(L, ANIMSPRITE, ret);
}

static int script_animsprite_init_from_animlist(lua_State* L) {
    AnimList animlist = luascript_read_nullable_userdata(L, 2, ANIMLIST);
    const char* animation_name = luaL_optstring(L, 3, NULL);

    if (!animlist) {
        return luaL_error(L, "animlist was nil (null)");
    }

    AnimSprite ret = animsprite_init_from_animlist(animlist, animation_name);

    return luascript_userdata_allocnew(L, ANIMSPRITE, ret);
}

static int script_animsprite_init_from_tweenlerp(lua_State* L) {
    const char* name = luaL_optstring(L, 2, NULL);
    int32_t loop = (int32_t)luaL_checkinteger(L, 3);
    TweenLerp tweenlerp = luascript_read_nullable_userdata(L, 4, TWEENLERP);

    if (!tweenlerp) {
        return luaL_error(L, "tweenlerp was nil (null)");
    }

    AnimSprite ret = animsprite_init_from_tweenlerp(name, loop, tweenlerp);

    return luascript_userdata_allocnew(L, ANIMSPRITE, ret);
}

static int script_animsprite_init_as_empty(lua_State* L) {
    const char* name = luaL_checkstring(L, 2);

    AnimSprite ret = animsprite_init_as_empty(name);

    return luascript_userdata_allocnew(L, ANIMSPRITE, ret);
}

static int script_animsprite_init(lua_State* L) {
    const AnimListItem* animlist_item = luascript_read_nullable_userdata(L, 2, ANIMLISTITEM);

    if (!animlist_item) {
        return luaL_error(L, "animlist_item was nil (null)");
    }

    AnimSprite ret = animsprite_init(animlist_item);

    return luascript_userdata_allocnew(L, ANIMSPRITE, ret);
}

static int script_animsprite_destroy(lua_State* L) {
    AnimSprite animsprite = luascript_read_userdata(L, ANIMSPRITE);

    if (luascript_userdata_is_allocated(L, ANIMSPRITE))
        animsprite_destroy(&animsprite);
    else
        logger_warn("script_animsprite_destroy() object was not allocated by lua");

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


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const luaL_Reg ANIMSPRITE_FUNCTIONS[] = {
    {"init_from_atlas", script_animsprite_init_from_atlas},
    {"init_from_animlist", script_animsprite_init_from_animlist},
    {"init_from_tweenlerp", script_animsprite_init_from_tweenlerp},
    {"init_as_empty", script_animsprite_init_as_empty},
    {"init", script_animsprite_init},
    {"destroy", script_animsprite_destroy},
    {"set_loop", script_animsprite_set_loop},
    {"get_name", script_animsprite_get_name},
    {"is_frame_animation", script_animsprite_is_frame_animation},
    {"set_delay", script_animsprite_set_delay},
    {NULL, NULL}
};


int script_animsprite_new(lua_State* L, AnimSprite animsprite) {
    return luascript_userdata_new(L, ANIMSPRITE, animsprite);
}

static int script_animsprite_gc(lua_State* L) {
    return luascript_userdata_destroy(L, ANIMSPRITE, (Destructor)animsprite_destroy);
}

static int script_animsprite_tostring(lua_State* L) {
    return luascript_userdata_tostring(L, ANIMSPRITE);
}

void script_animsprite_register(lua_State* L) {
    luascript_register(L, ANIMSPRITE, script_animsprite_gc, script_animsprite_tostring, ANIMSPRITE_FUNCTIONS);
}
