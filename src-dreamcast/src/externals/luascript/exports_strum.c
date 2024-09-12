#include "luascript_internal.h"

#include "game/funkin/note_types.h"
#include "game/funkin/strum.h"
#include "luascript_enums.h"


static int script_strum_update_draw_location(lua_State* L) {
    Strum strum = luascript_read_userdata(L, STRUM);
    float x = (float)luaL_checknumber(L, 2);
    float y = (float)luaL_checknumber(L, 3);

    strum_update_draw_location(strum, x, y);

    return 0;
}

static int script_strum_set_scroll_speed(lua_State* L) {
    Strum strum = luascript_read_userdata(L, STRUM);
    float speed = (float)luaL_checknumber(L, 2);

    strum_set_scroll_speed(strum, speed);

    return 0;
}

static int script_strum_set_scroll_direction(lua_State* L) {
    Strum strum = luascript_read_userdata(L, STRUM);
    ScrollDirection direction = luascript_helper_optenum(L, 2, LUASCRIPT_ENUMS_ScrollDirection);

    strum_set_scroll_direction(strum, direction);

    return 0;
}

static int script_strum_set_marker_duration_multiplier(lua_State* L) {
    Strum strum = luascript_read_userdata(L, STRUM);
    float multipler = (float)luaL_checknumber(L, 2);

    strum_set_marker_duration_multiplier(strum, multipler);

    return 0;
}

static int script_strum_reset(lua_State* L) {
    Strum strum = luascript_read_userdata(L, STRUM);
    float scroll_speed = (float)luaL_checknumber(L, 2);
    const char* state_name = luaL_optstring(L, 3, NULL);

    strum_reset(strum, scroll_speed, state_name);

    return 0;
}

static int script_strum_force_key_release(lua_State* L) {
    Strum strum = luascript_read_userdata(L, STRUM);

    strum_force_key_release(strum);

    return 0;
}

static int script_strum_get_press_state_changes(lua_State* L) {
    Strum strum = luascript_read_userdata(L, STRUM);

    int32_t ret = strum_get_press_state_changes(strum);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_strum_get_press_state(lua_State* L) {
    Strum strum = luascript_read_userdata(L, STRUM);

    StrumPressState ret = strum_get_press_state(strum);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_strum_get_press_state_use_alt_anim(lua_State* L) {
    Strum strum = luascript_read_userdata(L, STRUM);

    bool ret = strum_get_press_state_use_alt_anim(strum);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_strum_get_name(lua_State* L) {
    Strum strum = luascript_read_userdata(L, STRUM);

    const char* ret = strum_get_name(strum);

    lua_pushstring(L, ret);
    return 1;
}

static int script_strum_get_marker_duration(lua_State* L) {
    Strum strum = luascript_read_userdata(L, STRUM);

    float ret = strum_get_marker_duration(strum);

    lua_pushnumber(L, (lua_Number)ret);
    return 1;
}

static int script_strum_set_player_id(lua_State* L) {
    Strum strum = luascript_read_userdata(L, STRUM);
    int32_t player_id = (int32_t)luaL_checkinteger(L, 2);

    strum_set_player_id(strum, player_id);

    return 0;
}

static int script_strum_enable_background(lua_State* L) {
    Strum strum = luascript_read_userdata(L, STRUM);
    bool enable = (bool)lua_toboolean(L, 2);

    strum_enable_background(strum, enable);

    return 0;
}

static int script_strum_enable_sick_effect(lua_State* L) {
    Strum strum = luascript_read_userdata(L, STRUM);
    bool enable = (bool)lua_toboolean(L, 2);

    strum_enable_sick_effect(strum, enable);

    return 0;
}

static int script_strum_state_add(lua_State* L) {
    Strum strum = luascript_read_userdata(L, STRUM);
    ModelHolder mdlhldr_mrkr = luascript_read_nullable_userdata(L, 2, MODELHOLDER);
    ModelHolder mdlhldr_sck_ffct = luascript_read_nullable_userdata(L, 3, MODELHOLDER);
    ModelHolder mdlhldr_bckgrnd = luascript_read_nullable_userdata(L, 4, MODELHOLDER);
    const char* state_name = luaL_optstring(L, 5, NULL);

    strum_state_add(strum, mdlhldr_mrkr, mdlhldr_sck_ffct, mdlhldr_bckgrnd, state_name);

    return 0;
}

static int script_strum_state_toggle(lua_State* L) {
    Strum strum = luascript_read_userdata(L, STRUM);
    const char* state_name = luaL_optstring(L, 2, NULL);

    int32_t ret = strum_state_toggle(strum, state_name);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_strum_state_toggle_notes(lua_State* L) {
    Strum strum = luascript_read_userdata(L, STRUM);
    const char* state_name = luaL_optstring(L, 2, NULL);

    int32_t ret = strum_state_toggle_notes(strum, state_name);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_strum_state_toggle_sick_effect(lua_State* L) {
    Strum strum = luascript_read_userdata(L, STRUM);
    const char* state_name = luaL_optstring(L, 2, NULL);

    bool ret = strum_state_toggle_sick_effect(strum, state_name);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_strum_state_toggle_marker(lua_State* L) {
    Strum strum = luascript_read_userdata(L, STRUM);
    const char* state_name = luaL_optstring(L, 2, NULL);

    int32_t ret = strum_state_toggle_marker(strum, state_name);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_strum_state_toggle_background(lua_State* L) {
    Strum strum = luascript_read_userdata(L, STRUM);
    const char* state_name = luaL_optstring(L, 2, NULL);

    bool ret = strum_state_toggle_background(strum, state_name);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_strum_set_alpha_background(lua_State* L) {
    Strum strum = luascript_read_userdata(L, STRUM);
    float alpha = (float)luaL_checknumber(L, 2);

    float ret = strum_set_alpha_background(strum, alpha);

    lua_pushnumber(L, (lua_Number)ret);
    return 1;
}

static int script_strum_set_alpha_sick_effect(lua_State* L) {
    Strum strum = luascript_read_userdata(L, STRUM);
    float alpha = (float)luaL_checknumber(L, 2);

    float ret = strum_set_alpha_sick_effect(strum, alpha);

    lua_pushnumber(L, (lua_Number)ret);
    return 1;
}

static int script_strum_set_keep_aspect_ratio_background(lua_State* L) {
    Strum strum = luascript_read_userdata(L, STRUM);
    bool enable = (bool)lua_toboolean(L, 2);

    strum_set_keep_aspect_ratio_background(strum, enable);

    return 0;
}

static int script_strum_draw_sick_effect_apart(lua_State* L) {
    Strum strum = luascript_read_userdata(L, STRUM);
    bool enable = (bool)lua_toboolean(L, 2);

    strum_draw_sick_effect_apart(strum, enable);

    return 0;
}

static int script_strum_set_extra_animation(lua_State* L) {
    Strum strum = luascript_read_userdata(L, STRUM);
    StrumScriptTarget strum_script_target = luascript_helper_optenum(L, 2, LUASCRIPT_ENUMS_StrumScriptTarget);
    StrumScriptOn strum_script_on = luascript_helper_optenum(L, 3, LUASCRIPT_ENUMS_StrumScriptOn);
    bool undo = (bool)lua_toboolean(L, 4);
    AnimSprite animsprite = luascript_read_nullable_userdata(L, 5, ANIMSPRITE);

    strum_set_extra_animation(strum, strum_script_target, strum_script_on, undo, animsprite);

    return 0;
}

static int script_strum_set_extra_animation_continuous(lua_State* L) {
    Strum strum = luascript_read_userdata(L, STRUM);
    StrumScriptTarget strum_script_target = luascript_helper_optenum(L, 2, LUASCRIPT_ENUMS_StrumScriptTarget);
    AnimSprite animsprite = luascript_read_nullable_userdata(L, 3, ANIMSPRITE);

    strum_set_extra_animation_continuous(strum, strum_script_target, animsprite);

    return 0;
}

static int script_strum_disable_beat_synced_idle_and_continous(lua_State* L) {
    Strum strum = luascript_read_userdata(L, STRUM);
    bool disabled = (bool)lua_toboolean(L, 2);

    strum_disable_beat_synced_idle_and_continous(strum, disabled);

    return 0;
}

static int script_strum_set_bpm(lua_State* L) {
    Strum strum = luascript_read_userdata(L, STRUM);
    float bpm = (float)luaL_checknumber(L, 2);

    strum_set_bpm(strum, bpm);

    return 0;
}

static int script_strum_set_note_tweenkeyframe(lua_State* L) {
    Strum strum = luascript_read_userdata(L, STRUM);
    TweenKeyframe tweenkeyframe = luascript_read_nullable_userdata(L, 2, TWEENKEYFRAME);

    strum_set_note_tweenkeyframe(strum, tweenkeyframe);

    return 0;
}

static int script_strum_set_sickeffect_size_ratio(lua_State* L) {
    Strum strum = luascript_read_userdata(L, STRUM);
    float size_ratio = (float)luaL_checknumber(L, 2);

    strum_set_sickeffect_size_ratio(strum, size_ratio);

    return 0;
}

static int script_strum_set_alpha(lua_State* L) {
    Strum strum = luascript_read_userdata(L, STRUM);
    float alpha = (float)luaL_checknumber(L, 2);

    strum_set_alpha(strum, alpha);

    return 0;
}

static int script_strum_set_visible(lua_State* L) {
    Strum strum = luascript_read_userdata(L, STRUM);
    bool visible = (bool)lua_toboolean(L, 2);

    strum_set_visible(strum, visible);

    return 0;
}

static int script_strum_set_draw_offset(lua_State* L) {
    Strum strum = luascript_read_userdata(L, STRUM);
    float offset_milliseconds = (float)luaL_checknumber(L, 2);

    strum_set_draw_offset(strum, offset_milliseconds);

    return 0;
}

static int script_strum_get_modifier(lua_State* L) {
    Strum strum = luascript_read_userdata(L, STRUM);

    Modifier* ret = strum_get_modifier(strum);

    return script_modifier_new(L, ret);
}

static int script_strum_get_duration(lua_State* L) {
    Strum strum = luascript_read_userdata(L, STRUM);

    float64 ret = strum_get_duration(strum);

    lua_pushnumber(L, (lua_Number)ret);
    return 1;
}

static int script_strum_animation_restart(lua_State* L) {
    Strum strum = luascript_read_userdata(L, STRUM);

    strum_animation_restart(strum);

    return 0;
}

static int script_strum_animation_end(lua_State* L) {
    Strum strum = luascript_read_userdata(L, STRUM);

    strum_animation_end(strum);

    return 0;
}

static int script_strum_get_drawable(lua_State* L) {
    Strum strum = luascript_read_userdata(L, STRUM);

    Drawable ret = strum_get_drawable(strum);

    return script_drawable_new(L, ret);
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const luaL_Reg STRUM_FUNCTIONS[] = {
    {"update_draw_location", script_strum_update_draw_location},
    {"set_scroll_speed", script_strum_set_scroll_speed},
    {"set_scroll_direction", script_strum_set_scroll_direction},
    {"set_marker_duration_multiplier", script_strum_set_marker_duration_multiplier},
    {"reset", script_strum_reset},
    {"force_key_release", script_strum_force_key_release},
    {"get_press_state_changes", script_strum_get_press_state_changes},
    {"get_press_state", script_strum_get_press_state},
    {"get_press_state_use_alt_anim", script_strum_get_press_state_use_alt_anim},
    {"get_name", script_strum_get_name},
    {"get_marker_duration", script_strum_get_marker_duration},
    {"set_player_id", script_strum_set_player_id},
    {"enable_background", script_strum_enable_background},
    {"enable_sick_effect", script_strum_enable_sick_effect},
    {"state_add", script_strum_state_add},
    {"state_toggle", script_strum_state_toggle},
    {"state_toggle_notes", script_strum_state_toggle_notes},
    {"state_toggle_sick_effect", script_strum_state_toggle_sick_effect},
    {"state_toggle_marker", script_strum_state_toggle_marker},
    {"state_toggle_background", script_strum_state_toggle_background},
    {"set_alpha_background", script_strum_set_alpha_background},
    {"set_alpha_sick_effect", script_strum_set_alpha_sick_effect},
    {"set_keep_aspect_ratio_background", script_strum_set_keep_aspect_ratio_background},
    {"draw_sick_effect_apart", script_strum_draw_sick_effect_apart},
    {"set_extra_animation", script_strum_set_extra_animation},
    {"set_extra_animation_continuous", script_strum_set_extra_animation_continuous},
    {"disable_beat_synced_idle_and_continous", script_strum_disable_beat_synced_idle_and_continous},
    {"set_bpm", script_strum_set_bpm},
    {"set_note_tweenkeyframe", script_strum_set_note_tweenkeyframe},
    {"set_sickeffect_size_ratio", script_strum_set_sickeffect_size_ratio},
    {"set_alpha", script_strum_set_alpha},
    {"set_visible", script_strum_set_visible},
    {"set_draw_offset", script_strum_set_draw_offset},
    {"get_modifier", script_strum_get_modifier},
    {"get_duration", script_strum_get_duration},
    {"animation_restart", script_strum_animation_restart},
    {"animation_end", script_strum_animation_end},
    {"get_drawable", script_strum_get_drawable},
    {NULL, NULL}
};

int script_strum_new(lua_State* L, Strum strum) {
    return luascript_userdata_new(L, STRUM, strum);
}

static int script_strum_gc(lua_State* L) {
    return luascript_userdata_gc(L, STRUM);
}

static int script_strum_tostring(lua_State* L) {
    return luascript_userdata_tostring(L, STRUM);
}

void script_strum_register(lua_State* L) {
    luascript_register(L, STRUM, script_strum_gc, script_strum_tostring, STRUM_FUNCTIONS);
}
