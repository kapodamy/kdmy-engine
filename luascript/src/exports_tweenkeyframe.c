#include "luascript_internal.h"

#ifdef JAVASCRIPT
EM_JS_PRFX(TweenKeyframe, tweenkeyframe_init, (), {
    let ret = tweenkeyframe_init();
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(TweenKeyframe, tweenkeyframe_init2, (AnimListItem animlist_item), {
    let ret = tweenkeyframe_init2(kdmyEngine_obtain(animlist_item));
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(void, tweenkeyframe_destroy, (TweenKeyframe* tweenkeyframe), {
    tweenkeyframe_destroy(kdmyEngine_obtain(kdmyEngine_get_uint32(tweenkeyframe)));
});
EM_JS_PRFX(void, tweenkeyframe_animate_percent, (TweenKeyframe tweenkeyframe, double percent), {
    tweenkeyframe_animate_percent(kdmyEngine_obtain(tweenkeyframe), percent);
});
EM_JS_PRFX(int32_t, tweenkeyframe_get_ids_count, (TweenKeyframe tweenkeyframe), {
    let ret = tweenkeyframe_get_ids_count(kdmyEngine_obtain(tweenkeyframe));
    return ret;
});
EM_JS_PRFX(float, tweenkeyframe_peek_value, (TweenKeyframe tweenkeyframe), {
    let ret = tweenkeyframe_peek_value(kdmyEngine_obtain(tweenkeyframe));
    return ret;
});
EM_JS_PRFX(float, tweenkeyframe_peek_value_by_index, (TweenKeyframe tweenkeyframe, int32_t index), {
    let ret = tweenkeyframe_peek_value_by_index(kdmyEngine_obtain(tweenkeyframe), index);
    return ret;
});
EM_JS_PRFX(bool, tweenkeyframe_peek_entry_by_index, (TweenKeyframe tweenkeyframe, int32_t index, int32_t* out_id,  float* out_value), {
    const values = [0, 0];
    let ret = tweenkeyframe_peek_entry_by_index(kdmyEngine_obtain(tweenkeyframe), index, values);
    kdmyEngine_set_int32(out_id, values[0]);
    kdmyEngine_set_float32(out_value, values[1]);
    return ret ? 1 : 0;
});
EM_JS_PRFX(float, tweenkeyframe_peek_value_by_id, (TweenKeyframe tweenkeyframe, int32_t id), {
    let ret = tweenkeyframe_peek_value_by_id(kdmyEngine_obtain(tweenkeyframe), id);
    return ret;
});
EM_JS_PRFX(int32_t, tweenkeyframe_add_easeout, (TweenKeyframe tweenkeyframe, float at, int32_t id, float value), {
    let ret = tweenkeyframe_add_easeout(kdmyEngine_obtain(tweenkeyframe), at, id, value);
    return ret;
});
EM_JS_PRFX(int32_t, tweenkeyframe_add_easeinout, (TweenKeyframe tweenkeyframe, float at, int32_t id, float value), {
    let ret = tweenkeyframe_add_easeinout(kdmyEngine_obtain(tweenkeyframe), at, id, value);
    return ret;
});
EM_JS_PRFX(int32_t, tweenkeyframe_add_linear, (TweenKeyframe tweenkeyframe, float at, int32_t id, float value), {
    let ret = tweenkeyframe_add_linear(kdmyEngine_obtain(tweenkeyframe), at, id, value);
    return ret;
});
EM_JS_PRFX(int32_t, tweenkeyframe_add_steps, (TweenKeyframe tweenkeyframe, float at, int32_t id, float value, int32_t steps_count, Align steps_method), {
    let ret = tweenkeyframe_add_steps(kdmyEngine_obtain(tweenkeyframe), at, id, value, steps_count, steps_method);
    return ret;
});
EM_JS_PRFX(int32_t, tweenkeyframe_add_interpolator, (TweenKeyframe tweenkeyframde, float at, int32_t id, float value, AnimInterpolator type), {
    let ret = tweenkeyframe_add_interpolator(kdmyEngine_obtain(tweenkeyframde), at, id, value, type);
    return ret;
});
#endif


static int script_tweenkeyframe_init(lua_State* L) {
    TweenKeyframe ret = tweenkeyframe_init();

    return luascript_userdata_allocnew(L, TWEENKEYFRAME, ret);
}

static int script_tweenkeyframe_init2(lua_State* L) {
    AnimListItem animlistitem = luascript_read_userdata(L, ANIMLISTITEM);

    TweenKeyframe ret = tweenkeyframe_init2(animlistitem);

    return luascript_userdata_allocnew(L, TWEENKEYFRAME, ret);
}

static int script_tweenkeyframe_destroy(lua_State* L) {
    TweenKeyframe tweenkeyframe = luascript_read_userdata(L, TWEENKEYFRAME);

    if (luascript_userdata_is_allocated(L, TWEENKEYFRAME))
        tweenkeyframe_destroy(&tweenkeyframe);
    else
        printf("script_tweenkeyframe_destroy() object was not allocated by lua\n");

    return 0;
}

static int script_tweenkeyframe_animate_percent(lua_State* L) {
    TweenKeyframe tweenkeyframe = luascript_read_userdata(L, TWEENKEYFRAME);
    double percent = (double)luaL_checknumber(L, 2);

    tweenkeyframe_animate_percent(tweenkeyframe, percent);

    return 0;
}

static int script_tweenkeyframe_get_ids_count(lua_State* L) {
    TweenKeyframe tweenkeyframe = luascript_read_userdata(L, TWEENKEYFRAME);

    int32_t ret = tweenkeyframe_get_ids_count(tweenkeyframe);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_tweenkeyframe_peek_value(lua_State* L) {
    TweenKeyframe tweenkeyframe = luascript_read_userdata(L, TWEENKEYFRAME);

    float ret = tweenkeyframe_peek_value(tweenkeyframe);

    lua_pushnumber(L, ret);
    return 1;
}

static int script_tweenkeyframe_peek_value_by_index(lua_State* L) {
    TweenKeyframe tweenkeyframe = luascript_read_userdata(L, TWEENKEYFRAME);
    int32_t index = (int32_t)luaL_checkinteger(L, 2);

    float ret = tweenkeyframe_peek_value_by_index(tweenkeyframe, index);

    lua_pushnumber(L, ret);
    return 1;
}

static int script_tweenkeyframe_peek_entry_by_index(lua_State* L) {
    TweenKeyframe tweenkeyframe = luascript_read_userdata(L, TWEENKEYFRAME);
    int32_t index = (int32_t)luaL_checkinteger(L, 2);
    
    int32_t out_id;
    float out_value;
    bool ret = tweenkeyframe_peek_entry_by_index(tweenkeyframe, index, &out_id, &out_value);

    if (!ret) {
        lua_pushnil(L);
        return 1;
    }

    lua_pushinteger(L, out_id);
    lua_pushnumber(L, out_value);
    return 2;
}

static int script_tweenkeyframe_peek_value_by_id(lua_State* L) {
    TweenKeyframe tweenkeyframe = luascript_read_userdata(L, TWEENKEYFRAME);
    int32_t id = (int32_t)luaL_checkinteger(L, 2);

    float ret = tweenkeyframe_peek_value_by_id(tweenkeyframe, id);

    lua_pushnumber(L, ret);
    return 1;
}

static int script_tweenkeyframe_add_easeout(lua_State* L) {
    TweenKeyframe tweenkeyframe = luascript_read_userdata(L, TWEENKEYFRAME);
    float at = (float)luaL_checknumber(L, 2);
    int32_t id = (int32_t)luaL_checkinteger(L, 3);
    float value = (float)luaL_checknumber(L, 4);

    int32_t ret = tweenkeyframe_add_easeout(tweenkeyframe, at, id, value);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_tweenkeyframe_add_easeinout(lua_State* L) {
    TweenKeyframe tweenkeyframe = luascript_read_userdata(L, TWEENKEYFRAME);
    float at = (float)luaL_checknumber(L, 2);
    int32_t id = (int32_t)luaL_checkinteger(L, 3);
    float value = (float)luaL_checknumber(L, 4);

    int32_t ret = tweenkeyframe_add_easeinout(tweenkeyframe, at, id, value);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_tweenkeyframe_add_linear(lua_State* L) {
    TweenKeyframe tweenkeyframe = luascript_read_userdata(L, TWEENKEYFRAME);
    float at = (float)luaL_checknumber(L, 2);
    int32_t id = (int32_t)luaL_checkinteger(L, 3);
    float value = (float)luaL_checknumber(L, 4);

    int32_t ret = tweenkeyframe_add_linear(tweenkeyframe, at, id, value);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_tweenkeyframe_add_steps(lua_State* L) {
    TweenKeyframe tweenkeyframe = luascript_read_userdata(L, TWEENKEYFRAME);
    float at = (float)luaL_checknumber(L, 2);
    int32_t id = (int32_t)luaL_checkinteger(L, 3);
    float value = (float)luaL_checknumber(L, 4);
    int32_t steps_count = (int32_t)luaL_checkinteger(L, 5);
    Align steps_method = luascript_parse_align(L, luaL_optstring(L, 6, NULL));

    int32_t ret = tweenkeyframe_add_steps(tweenkeyframe, at, id, value, steps_count, steps_method);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_tweenkeyframe_add_interpolator(lua_State* L) {
    TweenKeyframe tweenkeyframe = luascript_read_userdata(L, TWEENKEYFRAME);
    float at = (float)luaL_checknumber(L, 2);
    int32_t id = (int32_t)luaL_checkinteger(L, 3);
    float value = (float)luaL_checknumber(L, 4);
    AnimInterpolator type = luascript_parse_interpolator(L, luaL_optstring(L, 5, NULL));

    int32_t ret = tweenkeyframe_add_interpolator(tweenkeyframe, at, id, value, type);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}




static const luaL_Reg TWEENKEYFRAME_FUNCTIONS[] = {
    { "init", script_tweenkeyframe_init },
    { "init2", script_tweenkeyframe_init2 },
    { "destroy", script_tweenkeyframe_destroy },
    { "animate_percent", script_tweenkeyframe_animate_percent },
    { "get_ids_count", script_tweenkeyframe_get_ids_count },
    { "peek_value", script_tweenkeyframe_peek_value },
    { "peek_value_by_index", script_tweenkeyframe_peek_value_by_index },
    { "peek_entry_by_index", script_tweenkeyframe_peek_entry_by_index },
    { "peek_value_by_id", script_tweenkeyframe_peek_value_by_id },
    { "add_easeout", script_tweenkeyframe_add_easeout },
    { "add_easeinout", script_tweenkeyframe_add_easeinout },
    { "add_linear", script_tweenkeyframe_add_linear },
    { "add_steps", script_tweenkeyframe_add_steps },
    { "add_interpolator", script_tweenkeyframe_add_interpolator },
    { NULL, NULL }
};

int script_tweenkeyframe_new(lua_State* L, TweenKeyframe tweenkeyframe) {
    return luascript_userdata_new(L, TWEENKEYFRAME, tweenkeyframe);
}

static int script_tweenkeyframe_gc(lua_State* L) {
    return luascript_userdata_destroy(L, TWEENKEYFRAME, (Destructor)tweenkeyframe_destroy);
}

static int script_tweenkeyframe_tostring(lua_State* L) {
    return luascript_userdata_tostring(L, TWEENKEYFRAME);
}

void script_tweenkeyframe_register(lua_State* L) {
    luascript_register(L, TWEENKEYFRAME, script_tweenkeyframe_gc, script_tweenkeyframe_tostring, TWEENKEYFRAME_FUNCTIONS);
}

