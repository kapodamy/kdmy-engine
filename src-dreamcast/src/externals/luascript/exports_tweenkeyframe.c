#include "luascript_internal.h"

#include "animlist.h"
#include "logger.h"
#include "luascript_enums.h"
#include "tweenkeyframe.h"


static int script_tweenkeyframe_init(lua_State* L) {
    TweenKeyframe ret = tweenkeyframe_init();

    return luascript_userdata_allocnew(L, TWEENKEYFRAME, ret);
}

static int script_tweenkeyframe_init2(lua_State* L) {
    const AnimListItem* animlistitem = luascript_read_nullable_userdata(L, 2, ANIMLISTITEM);

    if (!animlistitem) {
        return luaL_error(L, "animlistitem was nil (null)");
    }

    TweenKeyframe ret = tweenkeyframe_init2(animlistitem);

    return luascript_userdata_allocnew(L, TWEENKEYFRAME, ret);
}

static int script_tweenkeyframe_destroy(lua_State* L) {
    TweenKeyframe tweenkeyframe = luascript_read_userdata(L, TWEENKEYFRAME);

    if (luascript_userdata_is_allocated(L, TWEENKEYFRAME))
        tweenkeyframe_destroy(&tweenkeyframe);
    else
        logger_warn("script_tweenkeyframe_destroy() object was not allocated by lua");

    return 0;
}

static int script_tweenkeyframe_animate_percent(lua_State* L) {
    TweenKeyframe tweenkeyframe = luascript_read_userdata(L, TWEENKEYFRAME);
    float64 percent = luaL_checknumber(L, 2);

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

    lua_pushnumber(L, (lua_Number)ret);
    return 1;
}

static int script_tweenkeyframe_peek_value_by_index(lua_State* L) {
    TweenKeyframe tweenkeyframe = luascript_read_userdata(L, TWEENKEYFRAME);
    int32_t index = (int32_t)luaL_checkinteger(L, 2);

    float ret = tweenkeyframe_peek_value_by_index(tweenkeyframe, index);

    lua_pushnumber(L, (lua_Number)ret);
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

    lua_pushinteger(L, (lua_Integer)out_id);
    lua_pushnumber(L, (lua_Number)out_value);
    return 2;
}

static int script_tweenkeyframe_peek_value_by_id(lua_State* L) {
    TweenKeyframe tweenkeyframe = luascript_read_userdata(L, TWEENKEYFRAME);
    int32_t id = (int32_t)luaL_checkinteger(L, 2);

    float ret = tweenkeyframe_peek_value_by_id(tweenkeyframe, id);

    lua_pushnumber(L, (lua_Number)ret);
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
    Align steps_method = luascript_helper_optenum(L, 6, LUASCRIPT_ENUMS_Align);

    int32_t ret = tweenkeyframe_add_steps(tweenkeyframe, at, id, value, steps_count, steps_method);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_tweenkeyframe_add_ease(lua_State* L) {
    TweenKeyframe tweenkeyframe = luascript_read_userdata(L, TWEENKEYFRAME);
    float at = (float)luaL_checknumber(L, 2);
    int32_t id = (int32_t)luaL_checkinteger(L, 3);
    float value = (float)luaL_checknumber(L, 4);

    int32_t ret = tweenkeyframe_add_ease(tweenkeyframe, at, id, value);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_tweenkeyframe_add_easein(lua_State* L) {
    TweenKeyframe tweenkeyframe = luascript_read_userdata(L, TWEENKEYFRAME);
    float at = (float)luaL_checknumber(L, 2);
    int32_t id = (int32_t)luaL_checkinteger(L, 3);
    float value = (float)luaL_checknumber(L, 4);

    int32_t ret = tweenkeyframe_add_easein(tweenkeyframe, at, id, value);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_tweenkeyframe_add_cubic(lua_State* L) {
    TweenKeyframe tweenkeyframe = luascript_read_userdata(L, TWEENKEYFRAME);
    float at = (float)luaL_checknumber(L, 2);
    int32_t id = (int32_t)luaL_checkinteger(L, 3);
    float value = (float)luaL_checknumber(L, 4);

    int32_t ret = tweenkeyframe_add_cubic(tweenkeyframe, at, id, value);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_tweenkeyframe_add_quad(lua_State* L) {
    TweenKeyframe tweenkeyframe = luascript_read_userdata(L, TWEENKEYFRAME);
    float at = (float)luaL_checknumber(L, 2);
    int32_t id = (int32_t)luaL_checkinteger(L, 3);
    float value = (float)luaL_checknumber(L, 4);

    int32_t ret = tweenkeyframe_add_quad(tweenkeyframe, at, id, value);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_tweenkeyframe_add_expo(lua_State* L) {
    TweenKeyframe tweenkeyframe = luascript_read_userdata(L, TWEENKEYFRAME);
    float at = (float)luaL_checknumber(L, 2);
    int32_t id = (int32_t)luaL_checkinteger(L, 3);
    float value = (float)luaL_checknumber(L, 4);

    int32_t ret = tweenkeyframe_add_expo(tweenkeyframe, at, id, value);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_tweenkeyframe_add_sin(lua_State* L) {
    TweenKeyframe tweenkeyframe = luascript_read_userdata(L, TWEENKEYFRAME);
    float at = (float)luaL_checknumber(L, 2);
    int32_t id = (int32_t)luaL_checkinteger(L, 3);
    float value = (float)luaL_checknumber(L, 4);

    int32_t ret = tweenkeyframe_add_sin(tweenkeyframe, at, id, value);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_tweenkeyframe_add_interpolator(lua_State* L) {
    TweenKeyframe tweenkeyframe = luascript_read_userdata(L, TWEENKEYFRAME);
    float at = (float)luaL_checknumber(L, 2);
    int32_t id = (int32_t)luaL_checkinteger(L, 3);
    float value = (float)luaL_checknumber(L, 4);
    AnimInterpolator type = luascript_helper_optenum(L, 5, LUASCRIPT_ENUMS_AnimInterpolator);

    int32_t ret = tweenkeyframe_add_interpolator(tweenkeyframe, at, id, value, type);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const luaL_Reg TWEENKEYFRAME_FUNCTIONS[] = {
    {"init", script_tweenkeyframe_init},
    {"init2", script_tweenkeyframe_init2},
    {"destroy", script_tweenkeyframe_destroy},
    {"animate_percent", script_tweenkeyframe_animate_percent},
    {"get_ids_count", script_tweenkeyframe_get_ids_count},
    {"peek_value", script_tweenkeyframe_peek_value},
    {"peek_value_by_index", script_tweenkeyframe_peek_value_by_index},
    {"peek_entry_by_index", script_tweenkeyframe_peek_entry_by_index},
    {"peek_value_by_id", script_tweenkeyframe_peek_value_by_id},
    {"add_easeout", script_tweenkeyframe_add_easeout},
    {"add_easeinout", script_tweenkeyframe_add_easeinout},
    {"add_linear", script_tweenkeyframe_add_linear},
    {"add_steps", script_tweenkeyframe_add_steps},
    {"add_ease", script_tweenkeyframe_add_ease},
    {"add_easein", script_tweenkeyframe_add_easein},
    {"add_cubic", script_tweenkeyframe_add_cubic},
    {"add_quad", script_tweenkeyframe_add_quad},
    {"add_expo", script_tweenkeyframe_add_expo},
    {"add_sin", script_tweenkeyframe_add_sin},
    {"add_interpolator", script_tweenkeyframe_add_interpolator},
    {NULL, NULL}
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
