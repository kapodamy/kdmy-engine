#include "luascript_internal.h"

#include "game/funkin/note_types.h"
#include "game/funkin/strums.h"
#include "luascript_enums.h"


static int script_strums_get_drawable(lua_State* L) {
    Strums strums = luascript_read_userdata(L, STRUMS);

    Drawable ret = strums_get_drawable(strums);

    return script_drawable_new(L, ret);
}

static int script_strums_set_scroll_speed(lua_State* L) {
    Strums strums = luascript_read_userdata(L, STRUMS);
    float speed = (float)luaL_checknumber(L, 2);

    strums_set_scroll_speed(strums, speed);

    return 0;
}

static int script_strums_set_scroll_direction(lua_State* L) {
    Strums strums = luascript_read_userdata(L, STRUMS);
    ScrollDirection direction = luascript_helper_optenum(L, 2, LUASCRIPT_ENUMS_ScrollDirection);

    strums_set_scroll_direction(strums, direction);

    return 0;
}

static int script_strums_set_marker_duration_multiplier(lua_State* L) {
    Strums strums = luascript_read_userdata(L, STRUMS);
    float multipler = (float)luaL_checknumber(L, 2);

    strums_set_marker_duration_multiplier(strums, multipler);

    return 0;
}

static int script_strums_disable_beat_synced_idle_and_continous(lua_State* L) {
    Strums strums = luascript_read_userdata(L, STRUMS);
    bool disabled = (bool)lua_toboolean(L, 2);

    strums_disable_beat_synced_idle_and_continous(strums, disabled);

    return 0;
}

static int script_strums_set_bpm(lua_State* L) {
    Strums strums = luascript_read_userdata(L, STRUMS);
    float bpm = (float)luaL_checknumber(L, 2);

    strums_set_bpm(strums, bpm);

    return 0;
}

static int script_strums_reset(lua_State* L) {
    Strums strums = luascript_read_userdata(L, STRUMS);
    float scroll_speed = (float)luaL_checknumber(L, 2);
    const char* state_name = luaL_optstring(L, 3, NULL);

    strums_reset(strums, scroll_speed, state_name);

    return 0;
}

static int script_strums_force_key_release(lua_State* L) {
    Strums strums = luascript_read_userdata(L, STRUMS);

    strums_force_key_release(strums);

    return 0;
}

static int script_strums_set_alpha(lua_State* L) {
    Strums strums = luascript_read_userdata(L, STRUMS);
    float alpha = (float)luaL_checknumber(L, 2);

    float ret = strums_set_alpha(strums, alpha);

    lua_pushnumber(L, (lua_Number)ret);
    return 1;
}

static int script_strums_enable_background(lua_State* L) {
    Strums strums = luascript_read_userdata(L, STRUMS);
    bool enable = (bool)lua_toboolean(L, 2);

    strums_enable_background(strums, enable);

    return 0;
}

static int script_strums_set_keep_aspect_ratio_background(lua_State* L) {
    Strums strums = luascript_read_userdata(L, STRUMS);
    bool enable = (bool)lua_toboolean(L, 2);

    strums_set_keep_aspect_ratio_background(strums, enable);

    return 0;
}

static int script_strums_set_alpha_background(lua_State* L) {
    Strums strums = luascript_read_userdata(L, STRUMS);
    float alpha = (float)luaL_checknumber(L, 2);

    strums_set_alpha_background(strums, alpha);

    return 0;
}

static int script_strums_set_alpha_sick_effect(lua_State* L) {
    Strums strums = luascript_read_userdata(L, STRUMS);
    float alpha = (float)luaL_checknumber(L, 2);

    strums_set_alpha_sick_effect(strums, alpha);

    return 0;
}

static int script_strums_set_draw_offset(lua_State* L) {
    Strums strums = luascript_read_userdata(L, STRUMS);
    float offset_milliseconds = (float)luaL_checknumber(L, 2);

    strums_set_draw_offset(strums, offset_milliseconds);

    return 0;
}

static int script_strums_state_add(lua_State* L) {
    Strums strums = luascript_read_userdata(L, STRUMS);
    ModelHolder mdlhldr_mrkr = luascript_read_nullable_userdata(L, 2, MODELHOLDER);
    ModelHolder mdlhldr_sck_ffct = luascript_read_nullable_userdata(L, 3, MODELHOLDER);
    ModelHolder mdlhldr_bckgrnd = luascript_read_nullable_userdata(L, 4, MODELHOLDER);
    const char* state_name = luaL_optstring(L, 5, NULL);

    strums_state_add(strums, mdlhldr_mrkr, mdlhldr_sck_ffct, mdlhldr_bckgrnd, state_name);

    return 0;
}

static int script_strums_state_toggle(lua_State* L) {
    Strums strums = luascript_read_userdata(L, STRUMS);
    const char* state_name = luaL_optstring(L, 2, NULL);

    int32_t ret = strums_state_toggle(strums, state_name);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_strums_state_toggle_notes(lua_State* L) {
    Strums strums = luascript_read_userdata(L, STRUMS);
    const char* state_name = luaL_optstring(L, 2, NULL);

    int32_t ret = strums_state_toggle_notes(strums, state_name);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_strums_state_toggle_marker_and_sick_effect(lua_State* L) {
    Strums strums = luascript_read_userdata(L, STRUMS);
    const char* state_name = luaL_optstring(L, 2, NULL);

    strums_state_toggle_marker_and_sick_effect(strums, state_name);

    return 0;
}

static int script_strums_get_lines_count(lua_State* L) {
    Strums strums = luascript_read_userdata(L, STRUMS);

    int32_t ret = strums_get_lines_count(strums);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_strums_get_strum_line(lua_State* L) {
    Strums strums = luascript_read_userdata(L, STRUMS);
    int32_t index = (int32_t)luaL_checkinteger(L, 2);

    Strum ret = strums_get_strum_line(strums, index);

    return script_strum_new(L, ret);
}

static int script_strums_enable_post_sick_effect_draw(lua_State* L) {
    Strums strums = luascript_read_userdata(L, STRUMS);
    bool enable = (bool)lua_toboolean(L, 2);

    strums_enable_post_sick_effect_draw(strums, enable);

    return 0;
}

static int script_strums_animation_set(lua_State* L) {
    Strums strums = luascript_read_userdata(L, STRUMS);
    AnimSprite animsprite = luascript_read_nullable_userdata(L, 2, ANIMSPRITE);

    strums_animation_set(strums, animsprite);

    return 0;
}

static int script_strums_animation_restart(lua_State* L) {
    Strums strums = luascript_read_userdata(L, STRUMS);

    strums_animation_restart(strums);

    return 0;
}

static int script_strums_animation_end(lua_State* L) {
    Strums strums = luascript_read_userdata(L, STRUMS);

    strums_animation_end(strums);

    return 0;
}

static int script_strums_decorators_get_count(lua_State* L) {
    Strums strums = luascript_read_userdata(L, STRUMS);

    int32_t ret = strums_decorators_get_count(strums);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_strums_decorators_add(lua_State* L) {
    Strums strums = luascript_read_userdata(L, STRUMS);
    ModelHolder modelholder = luascript_read_nullable_userdata(L, 2, MODELHOLDER);
    const char* animation_name = luaL_checkstring(L, 3);
    float64 timestamp = luaL_checknumber(L, 4);

    bool ret = strums_decorators_add(strums, modelholder, animation_name, timestamp);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_strums_decorators_add2(lua_State* L) {
    Strums strums = luascript_read_userdata(L, STRUMS);
    ModelHolder modelholder = luascript_read_nullable_userdata(L, 2, MODELHOLDER);
    const char* animation_name = luaL_checkstring(L, 3);
    float64 timestamp = luaL_checknumber(L, 4);
    int32_t from_strum_index = (int32_t)luaL_checkinteger(L, 5);
    int32_t to_strum_index = (int32_t)luaL_checkinteger(L, 6);

    bool ret = strums_decorators_add2(strums, modelholder, animation_name, timestamp, from_strum_index, to_strum_index);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_strums_decorators_set_scroll_speed(lua_State* L) {
    Strums strums = luascript_read_userdata(L, STRUMS);
    float64 speed = luaL_checknumber(L, 2);

    strums_decorators_set_scroll_speed(strums, speed);

    return 0;
}

static int script_strums_decorators_set_alpha(lua_State* L) {
    Strums strums = luascript_read_userdata(L, STRUMS);
    float alpha = (float)luaL_checknumber(L, 2);

    strums_decorators_set_alpha(strums, alpha);

    return 0;
}

static int script_strums_decorators_set_visible(lua_State* L) {
    Strums strums = luascript_read_userdata(L, STRUMS);
    float64 decorator_timestamp = luaL_checknumber(L, 2);
    bool visible = (bool)lua_toboolean(L, 3);

    strums_decorators_set_visible(strums, decorator_timestamp, visible);

    return 0;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const luaL_Reg STRUMS_FUNCTIONS[] = {
    {"get_drawable", script_strums_get_drawable},
    {"set_scroll_speed", script_strums_set_scroll_speed},
    {"set_scroll_direction", script_strums_set_scroll_direction},
    {"set_marker_duration_multiplier", script_strums_set_marker_duration_multiplier},
    {"disable_beat_synced_idle_and_continous", script_strums_disable_beat_synced_idle_and_continous},
    {"set_bpm", script_strums_set_bpm},
    {"reset", script_strums_reset},
    {"force_key_release", script_strums_force_key_release},
    {"set_alpha", script_strums_set_alpha},
    {"enable_background", script_strums_enable_background},
    {"set_keep_aspect_ratio_background", script_strums_set_keep_aspect_ratio_background},
    {"set_alpha_background", script_strums_set_alpha_background},
    {"set_alpha_sick_effect", script_strums_set_alpha_sick_effect},
    {"set_draw_offset", script_strums_set_draw_offset},
    {"state_add", script_strums_state_add},
    {"state_toggle", script_strums_state_toggle},
    {"state_toggle_notes", script_strums_state_toggle_notes},
    {"state_toggle_marker_and_sick_effect", script_strums_state_toggle_marker_and_sick_effect},
    {"get_lines_count", script_strums_get_lines_count},
    {"get_strum_line", script_strums_get_strum_line},
    {"enable_post_sick_effect_draw", script_strums_enable_post_sick_effect_draw},
    {"animation_set", script_strums_animation_set},
    {"animation_restart", script_strums_animation_restart},
    {"animation_end", script_strums_animation_end},
    {"decorators_get_count", script_strums_decorators_get_count},
    {"decorators_add", script_strums_decorators_add},
    {"decorators_add2", script_strums_decorators_add2},
    {"decorators_set_scroll_speed", script_strums_decorators_set_scroll_speed},
    {"decorators_set_alpha", script_strums_decorators_set_alpha},
    {"decorators_set_visible", script_strums_decorators_set_visible},
    {NULL, NULL}
};

int script_strums_new(lua_State* L, Strums strums) {
    return luascript_userdata_new(L, STRUMS, strums);
}

static int script_strums_gc(lua_State* L) {
    return luascript_userdata_gc(L, STRUMS);
}

static int script_strums_tostring(lua_State* L) {
    return luascript_userdata_tostring(L, STRUMS);
}

void script_strums_register(lua_State* L) {
    luascript_register(L, STRUMS, script_strums_gc, script_strums_tostring, STRUMS_FUNCTIONS);
}
