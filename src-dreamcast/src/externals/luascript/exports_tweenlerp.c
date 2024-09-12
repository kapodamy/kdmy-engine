#include "luascript_internal.h"

#include "logger.h"
#include "luascript_enums.h"
#include "tweenlerp.h"


static int script_tweenlerp_init(lua_State* L) {
    TweenLerp tweenlerp = tweenlerp_init();
    return luascript_userdata_allocnew(L, TWEENLERP, tweenlerp);
}

static int script_tweenlerp_destroy(lua_State* L) {
    TweenLerp tweenlerp = luascript_read_userdata(L, TWEENLERP);

    if (luascript_userdata_is_allocated(L, TWEENLERP))
        tweenlerp_destroy(&tweenlerp);
    else
        logger_warn("script_tweenlerp_destroy() object was not allocated by lua");

    return 0;
}

static int script_tweenlerp_end(lua_State* L) {
    TweenLerp tweenlerp = luascript_read_userdata(L, TWEENLERP);

    tweenlerp_end(tweenlerp);

    return 0;
}

static int script_tweenlerp_mark_as_completed(lua_State* L) {
    TweenLerp tweenlerp = luascript_read_userdata(L, TWEENLERP);

    tweenlerp_mark_as_completed(tweenlerp);

    return 0;
}

static int script_tweenlerp_restart(lua_State* L) {
    TweenLerp tweenlerp = luascript_read_userdata(L, TWEENLERP);

    tweenlerp_restart(tweenlerp);

    return 0;
}

static int script_tweenlerp_animate(lua_State* L) {
    TweenLerp tweenlerp = luascript_read_userdata(L, TWEENLERP);
    float elapsed = (float)luaL_checknumber(L, 2);

    int32_t ret = tweenlerp_animate(tweenlerp, elapsed);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_tweenlerp_animate_percent(lua_State* L) {
    TweenLerp tweenlerp = luascript_read_userdata(L, TWEENLERP);
    float percent = (float)luaL_checknumber(L, 2);

    int32_t ret = tweenlerp_animate_percent(tweenlerp, percent);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_tweenlerp_is_completed(lua_State* L) {
    TweenLerp tweenlerp = luascript_read_userdata(L, TWEENLERP);

    bool ret = tweenlerp_is_completed(tweenlerp);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_tweenlerp_get_elapsed(lua_State* L) {
    TweenLerp tweenlerp = luascript_read_userdata(L, TWEENLERP);

    float64 ret = tweenlerp_get_elapsed(tweenlerp);

    lua_pushnumber(L, (lua_Number)ret);
    return 1;
}

static int script_tweenlerp_get_entry_count(lua_State* L) {
    TweenLerp tweenlerp = luascript_read_userdata(L, TWEENLERP);

    int32_t ret = tweenlerp_get_entry_count(tweenlerp);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_tweenlerp_peek_value(lua_State* L) {
    TweenLerp tweenlerp = luascript_read_userdata(L, TWEENLERP);

    float ret = tweenlerp_peek_value(tweenlerp);

    lua_pushnumber(L, (lua_Number)ret);
    return 1;
}

static int script_tweenlerp_peek_value_by_index(lua_State* L) {
    TweenLerp tweenlerp = luascript_read_userdata(L, TWEENLERP);
    int32_t index = (int32_t)luaL_checkinteger(L, 2);

    float ret = tweenlerp_peek_value_by_index(tweenlerp, index);

    lua_pushnumber(L, (lua_Number)ret);
    return 1;
}

static int script_tweenlerp_peek_entry_by_index(lua_State* L) {
    TweenLerp tweenlerp = luascript_read_userdata(L, TWEENLERP);
    int32_t index = (int32_t)luaL_checkinteger(L, 2);

    int32_t out_id;
    float out_value, out_duration;
    bool ret = tweenlerp_peek_entry_by_index(tweenlerp, index, &out_id, &out_value, &out_duration);

    if (ret) {
        lua_pushinteger(L, (lua_Integer)out_id);
        lua_pushnumber(L, (lua_Number)out_value);
        lua_pushnumber(L, (lua_Number)out_duration);
        return 3;
    } else {
        lua_pushnil(L);
        return 1;
    }
}

static int script_tweenlerp_peek_value_by_id(lua_State* L) {
    TweenLerp tweenlerp = luascript_read_userdata(L, TWEENLERP);
    int32_t id = (int32_t)luaL_checkinteger(L, 2);

    float ret = tweenlerp_peek_value_by_id(tweenlerp, id);

    lua_pushnumber(L, (lua_Number)ret);
    return 1;
}

static int script_tweenlerp_change_bounds_by_index(lua_State* L) {
    TweenLerp tweenlerp = luascript_read_userdata(L, TWEENLERP);
    int32_t index = (int32_t)luaL_checkinteger(L, 2);
    float new_start = (float)luaL_checknumber(L, 3);
    float new_end = (float)luaL_checknumber(L, 4);

    bool ret = tweenlerp_change_bounds_by_index(tweenlerp, index, new_start, new_end);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_tweenlerp_override_start_with_end_by_index(lua_State* L) {
    TweenLerp tweenlerp = luascript_read_userdata(L, TWEENLERP);
    int32_t index = (int32_t)luaL_checkinteger(L, 2);

    bool ret = tweenlerp_override_start_with_end_by_index(tweenlerp, index);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_tweenlerp_change_bounds_by_id(lua_State* L) {
    TweenLerp tweenlerp = luascript_read_userdata(L, TWEENLERP);
    int32_t id = (int32_t)luaL_checkinteger(L, 2);
    float new_start = (float)luaL_checknumber(L, 3);
    float new_end = (float)luaL_checknumber(L, 4);

    bool ret = tweenlerp_change_bounds_by_id(tweenlerp, id, new_start, new_end);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_tweenlerp_change_duration_by_index(lua_State* L) {
    TweenLerp tweenlerp = luascript_read_userdata(L, TWEENLERP);
    int32_t index = (int32_t)luaL_checkinteger(L, 2);
    float new_duration = (float)luaL_checknumber(L, 3);

    bool ret = tweenlerp_change_duration_by_index(tweenlerp, index, new_duration);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_tweenlerp_swap_bounds_by_index(lua_State* L) {
    TweenLerp tweenlerp = luascript_read_userdata(L, TWEENLERP);
    int32_t index = (int32_t)luaL_checkinteger(L, 2);

    bool ret = tweenlerp_swap_bounds_by_index(tweenlerp, index);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_tweenlerp_add_ease(lua_State* L) {
    TweenLerp tweenlerp = luascript_read_userdata(L, TWEENLERP);
    int32_t id = (int32_t)luaL_checkinteger(L, 2);
    float start = (float)luaL_checknumber(L, 3);
    float end = (float)luaL_checknumber(L, 4);
    float duration = (float)luaL_checknumber(L, 5);

    int32_t ret = tweenlerp_add_ease(tweenlerp, id, start, end, duration);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_tweenlerp_add_easein(lua_State* L) {
    TweenLerp tweenlerp = luascript_read_userdata(L, TWEENLERP);
    int32_t id = (int32_t)luaL_checkinteger(L, 2);
    float start = (float)luaL_checknumber(L, 3);
    float end = (float)luaL_checknumber(L, 4);
    float duration = (float)luaL_checknumber(L, 5);

    int32_t ret = tweenlerp_add_easein(tweenlerp, id, start, end, duration);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_tweenlerp_add_easeout(lua_State* L) {
    TweenLerp tweenlerp = luascript_read_userdata(L, TWEENLERP);
    int32_t id = (int32_t)luaL_checkinteger(L, 2);
    float start = (float)luaL_checknumber(L, 3);
    float end = (float)luaL_checknumber(L, 4);
    float duration = (float)luaL_checknumber(L, 5);

    int32_t ret = tweenlerp_add_easeout(tweenlerp, id, start, end, duration);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_tweenlerp_add_easeinout(lua_State* L) {
    TweenLerp tweenlerp = luascript_read_userdata(L, TWEENLERP);
    int32_t id = (int32_t)luaL_checkinteger(L, 2);
    float start = (float)luaL_checknumber(L, 3);
    float end = (float)luaL_checknumber(L, 4);
    float duration = (float)luaL_checknumber(L, 5);

    int32_t ret = tweenlerp_add_easeinout(tweenlerp, id, start, end, duration);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_tweenlerp_add_linear(lua_State* L) {
    TweenLerp tweenlerp = luascript_read_userdata(L, TWEENLERP);
    int32_t id = (int32_t)luaL_checkinteger(L, 2);
    float start = (float)luaL_checknumber(L, 3);
    float end = (float)luaL_checknumber(L, 4);
    float duration = (float)luaL_checknumber(L, 5);

    int32_t ret = tweenlerp_add_linear(tweenlerp, id, start, end, duration);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_tweenlerp_add_steps(lua_State* L) {
    TweenLerp tweenlerp = luascript_read_userdata(L, TWEENLERP);
    int32_t id = (int32_t)luaL_checkinteger(L, 2);
    float start = (float)luaL_checknumber(L, 3);
    float end = (float)luaL_checknumber(L, 4);
    float duration = (float)luaL_checknumber(L, 5);
    int32_t steps_count = (int32_t)luaL_checkinteger(L, 6);
    Align steps_method = luascript_helper_optenum(L, 7, LUASCRIPT_ENUMS_Align);

    int32_t ret = tweenlerp_add_steps(tweenlerp, id, start, end, duration, steps_count, steps_method);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_tweenlerp_add_cubic(lua_State* L) {
    TweenLerp tweenlerp = luascript_read_userdata(L, TWEENLERP);
    int32_t id = (int32_t)luaL_checkinteger(L, 2);
    float start = (float)luaL_checknumber(L, 3);
    float end = (float)luaL_checknumber(L, 4);
    float duration = (float)luaL_checknumber(L, 5);

    int32_t ret = tweenlerp_add_cubic(tweenlerp, id, start, end, duration);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_tweenlerp_add_quad(lua_State* L) {
    TweenLerp tweenlerp = luascript_read_userdata(L, TWEENLERP);
    int32_t id = (int32_t)luaL_checkinteger(L, 2);
    float start = (float)luaL_checknumber(L, 3);
    float end = (float)luaL_checknumber(L, 4);
    float duration = (float)luaL_checknumber(L, 5);

    int32_t ret = tweenlerp_add_quad(tweenlerp, id, start, end, duration);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_tweenlerp_add_expo(lua_State* L) {
    TweenLerp tweenlerp = luascript_read_userdata(L, TWEENLERP);
    int32_t id = (int32_t)luaL_checkinteger(L, 2);
    float start = (float)luaL_checknumber(L, 3);
    float end = (float)luaL_checknumber(L, 4);
    float duration = (float)luaL_checknumber(L, 5);

    int32_t ret = tweenlerp_add_expo(tweenlerp, id, start, end, duration);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_tweenlerp_add_sin(lua_State* L) {
    TweenLerp tweenlerp = luascript_read_userdata(L, TWEENLERP);
    int32_t id = (int32_t)luaL_checkinteger(L, 2);
    float start = (float)luaL_checknumber(L, 3);
    float end = (float)luaL_checknumber(L, 4);
    float duration = (float)luaL_checknumber(L, 5);

    int32_t ret = tweenlerp_add_sin(tweenlerp, id, start, end, duration);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_tweenlerp_add_interpolator(lua_State* L) {
    TweenLerp tweenlerp = luascript_read_userdata(L, TWEENLERP);
    int32_t id = (int32_t)luaL_checkinteger(L, 2);
    float start = (float)luaL_checknumber(L, 3);
    float end = (float)luaL_checknumber(L, 4);
    float duration = (float)luaL_checknumber(L, 5);
    AnimInterpolator type = luascript_helper_optenum(L, 6, LUASCRIPT_ENUMS_AnimInterpolator);

    int32_t ret = tweenlerp_add_interpolator(tweenlerp, id, start, end, duration, type);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const luaL_Reg TWEENLERP_FUNCTIONS[] = {
    {"init", script_tweenlerp_init},
    {"destroy", script_tweenlerp_destroy},
    {"end", script_tweenlerp_end},
    {"mark_as_completed", script_tweenlerp_mark_as_completed},
    {"restart", script_tweenlerp_restart},
    {"animate", script_tweenlerp_animate},
    {"animate_percent", script_tweenlerp_animate_percent},
    {"is_completed", script_tweenlerp_is_completed},
    {"get_elapsed", script_tweenlerp_get_elapsed},
    {"get_entry_count", script_tweenlerp_get_entry_count},
    {"peek_value", script_tweenlerp_peek_value},
    {"peek_value_by_index", script_tweenlerp_peek_value_by_index},
    {"peek_entry_by_index", script_tweenlerp_peek_entry_by_index},
    {"peek_value_by_id", script_tweenlerp_peek_value_by_id},
    {"change_bounds_by_index", script_tweenlerp_change_bounds_by_index},
    {"override_start_with_end_by_index", script_tweenlerp_override_start_with_end_by_index},
    {"change_bounds_by_id", script_tweenlerp_change_bounds_by_id},
    {"change_duration_by_index", script_tweenlerp_change_duration_by_index},
    {"swap_bounds_by_index", script_tweenlerp_swap_bounds_by_index},
    {"add_ease", script_tweenlerp_add_ease},
    {"add_easein", script_tweenlerp_add_easein},
    {"add_easeout", script_tweenlerp_add_easeout},
    {"add_easeinout", script_tweenlerp_add_easeinout},
    {"add_linear", script_tweenlerp_add_linear},
    {"add_steps", script_tweenlerp_add_steps},
    {"add_cubic", script_tweenlerp_add_cubic},
    {"add_quad", script_tweenlerp_add_quad},
    {"add_expo", script_tweenlerp_add_expo},
    {"add_sin", script_tweenlerp_add_sin},
    {"add_interpolator", script_tweenlerp_add_interpolator},
    {NULL, NULL}
};

int script_tweenlerp_new(lua_State* L, TweenLerp tweenlerp) {
    return luascript_userdata_new(L, TWEENLERP, tweenlerp);
}

static int script_tweenlerp_gc(lua_State* L) {
    return luascript_userdata_destroy(L, TWEENLERP, (Destructor)tweenlerp_destroy);
}

static int script_tweenlerp_tostring(lua_State* L) {
    return luascript_userdata_tostring(L, TWEENLERP);
}

void script_tweenlerp_register(lua_State* L) {
    luascript_register(L, TWEENLERP, script_tweenlerp_gc, script_tweenlerp_tostring, TWEENLERP_FUNCTIONS);
}
