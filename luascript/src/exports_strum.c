#include "luascript_internal.h"

#ifdef JAVASCRIPT
EM_JS_PRFX(void, strum_update_draw_location, (Strum strum, float x, float y), {
    strum_update_draw_location(kdmyEngine_obtain(strum), x, y);
});
EM_JS_PRFX(void, strum_set_scroll_speed, (Strum strum, float speed), {
    strum_set_scroll_speed(kdmyEngine_obtain(strum), speed);
});
EM_JS_PRFX(void, strum_set_scroll_direction, (Strum strum, ScrollDirection direction), {
    strum_set_scroll_direction(kdmyEngine_obtain(strum), direction);
});
EM_JS_PRFX(void, strum_set_marker_duration_multiplier, (Strum strum, float multipler), {
    strum_set_marker_duration_multiplier(kdmyEngine_obtain(strum), multipler);
});
EM_JS_PRFX(void, strum_reset, (Strum strum, float scroll_speed, const char* state_name), {
    strum_reset(kdmyEngine_obtain(strum), scroll_speed, kdmyEngine_ptrToString(state_name));
});
EM_JS_PRFX(void, strum_force_key_release, (Strum strum), {
    strum_force_key_release(kdmyEngine_obtain(strum));
});
EM_JS_PRFX(int32_t, strum_get_press_state_changes, (Strum strum), {
    let ret = strum_get_press_state_changes(kdmyEngine_obtain(strum));
    return ret;
});
EM_JS_PRFX(StrumPressState, strum_get_press_state, (Strum strum), {
    let ret = strum_get_press_state(kdmyEngine_obtain(strum));
    return ret;
});
EM_JS_PRFX(const char*, strum_get_name, (Strum strum), {
    let ret = strum_get_name(kdmyEngine_obtain(strum));
    return kdmyEngine_stringToPtr(ret);
});
EM_JS_PRFX(float, strum_get_marker_duration, (Strum strum), {
    let ret = strum_get_marker_duration(kdmyEngine_obtain(strum));
    return ret;
});
EM_JS_PRFX(void, strum_set_player_id, (Strum strum, int32_t player_id), {
    strum_set_player_id(kdmyEngine_obtain(strum), player_id);
});
EM_JS_PRFX(void, strum_enable_background, (Strum strum, bool enable), {
    strum_enable_background(kdmyEngine_obtain(strum), enable);
});
EM_JS_PRFX(void, strum_enable_sick_effect, (Strum strum, bool enable), {
    strum_enable_sick_effect(kdmyEngine_obtain(strum), enable);
});
EM_JS_PRFX(void, strum_state_add, (Strum strum, ModelHolder mdlhldr_mrkr, ModelHolder mdlhldr_sck_ffct, ModelHolder mdlhldr_bckgrnd, const char* state_name), {
    strum_state_add(kdmyEngine_obtain(strum), kdmyEngine_obtain(mdlhldr_mrkr), kdmyEngine_obtain(mdlhldr_sck_ffct), kdmyEngine_obtain(mdlhldr_bckgrnd), kdmyEngine_ptrToString(state_name));
});
EM_JS_PRFX(int32_t, strum_state_toggle, (Strum strum, const char* state_name), {
    let ret = strum_state_toggle(kdmyEngine_obtain(strum), kdmyEngine_ptrToString(state_name));
    return ret;
});
EM_JS_PRFX(int32_t, strum_state_toggle_notes, (Strum strum, const char* state_name), {
    let ret = strum_state_toggle_notes(kdmyEngine_obtain(strum), kdmyEngine_ptrToString(state_name));
    return ret;
});
EM_JS_PRFX(bool, strum_state_toggle_sick_effect, (Strum strum, const char* state_name), {
    let ret = strum_state_toggle_sick_effect(kdmyEngine_obtain(strum), kdmyEngine_ptrToString(state_name));
    return ret ? 1 : 0;
});
EM_JS_PRFX(int32_t, strum_state_toggle_marker, (Strum strum, const char* state_name), {
    let ret = strum_state_toggle_marker(kdmyEngine_obtain(strum), kdmyEngine_ptrToString(state_name));
    return ret;
});
EM_JS_PRFX(bool, strum_state_toggle_background, (Strum strum, const char* state_name), {
    let ret = strum_state_toggle_background(kdmyEngine_obtain(strum), kdmyEngine_ptrToString(state_name));
    return ret ? 1 : 0;
});
EM_JS_PRFX(float, strum_set_alpha_background, (Strum strum, float alpha), {
    let ret = strum_set_alpha_background(kdmyEngine_obtain(strum), alpha);
    return ret;
});
EM_JS_PRFX(float, strum_set_alpha_sick_effect, (Strum strum, float alpha), {
    let ret = strum_set_alpha_sick_effect(kdmyEngine_obtain(strum), alpha);
    return ret;
});
EM_JS_PRFX(void, strum_set_keep_aspect_ratio_background, (Strum strum, bool enable), {
    strum_set_keep_aspect_ratio_background(kdmyEngine_obtain(strum), enable);
});
EM_JS_PRFX(void, strum_draw_sick_effect_apart, (Strum strum, bool enable), {
    strum_draw_sick_effect_apart(kdmyEngine_obtain(strum), enable);
});
EM_JS_PRFX(void, strum_set_extra_animation, (Strum strum, StrumScriptTarget strum_script_target, StrumScriptOn strum_script_on, bool undo, AnimSprite animsprite), {
    strum_set_extra_animation(kdmyEngine_obtain(strum), strum_script_target, strum_script_on, undo, kdmyEngine_obtain(animsprite));
});
EM_JS_PRFX(void, strum_set_extra_animation_continuous, (Strum strum, StrumScriptTarget strum_script_target, AnimSprite animsprite), {
    strum_set_extra_animation_continuous(kdmyEngine_obtain(strum), strum_script_target, kdmyEngine_obtain(animsprite));
});
EM_JS_PRFX(void, strum_set_notesmaker_tweenlerp, (Strum strum, TweenLerp tweenlerp, bool apply_to_marker_too), {
    strum_set_notesmaker_tweenlerp(kdmyEngine_obtain(strum), kdmyEngine_obtain(tweenlerp), apply_to_marker_too);
});
EM_JS_PRFX(void, strum_set_sickeffect_size_ratio, (Strum strum, float size_ratio), {
    strum_set_sickeffect_size_ratio(kdmyEngine_obtain(strum), size_ratio);
});
EM_JS_PRFX(void, strum_set_alpha, (Strum strum, float alpha), {
    strum_set_alpha(kdmyEngine_obtain(strum), alpha);
});
EM_JS_PRFX(void, strum_set_visible, (Strum strum, bool visible), {
    strum_set_visible(kdmyEngine_obtain(strum), visible);
});
EM_JS_PRFX(void, strum_set_draw_offset, (Strum strum, float offset_milliseconds), {
    strum_set_draw_offset(kdmyEngine_obtain(strum), offset_milliseconds);
});
EM_JS_PRFX(Modifier, strum_get_modifier, (Strum strum), {
    let ret = strum_get_modifier(kdmyEngine_obtain(strum));
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(double, strum_get_duration, (Strum strum), {
    let ret = strum_get_duration(kdmyEngine_obtain(strum));
    return ret;
});
EM_JS_PRFX(void, strum_animation_restart, (Strum strum), {
    strum_animation_restart(kdmyEngine_obtain(strum));
});
EM_JS_PRFX(void, strum_animation_end, (Strum strum), {
    strum_animation_end(kdmyEngine_obtain(strum));
});
EM_JS_PRFX(Drawable, strum_get_drawable, (Strum strum), {
    let ret = strum_get_drawable(kdmyEngine_obtain(strum));
    return kdmyEngine_obtain(ret);
});
#endif


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
    ScrollDirection direction = luascript_parse_scrolldirection(L, luaL_optstring(L, 2, NULL));

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

    lua_pushinteger(L, ret);
    return 1;
}

static int script_strum_get_name(lua_State* L) {
    Strum strum = luascript_read_userdata(L, STRUM);

    const char* ret = strum_get_name(strum);

    lua_pushstring(L, ret);

#ifdef JAVASCRIPT
    free((char*)ret);
#endif
    return 1;
}

static int script_strum_get_marker_duration(lua_State* L) {
    Strum strum = luascript_read_userdata(L, STRUM);

    float ret = strum_get_marker_duration(strum);

    lua_pushnumber(L, ret);
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

    lua_pushnumber(L, ret);
    return 1;
}

static int script_strum_set_alpha_sick_effect(lua_State* L) {
    Strum strum = luascript_read_userdata(L, STRUM);
    float alpha = (float)luaL_checknumber(L, 2);

    float ret = strum_set_alpha_sick_effect(strum, alpha);

    lua_pushnumber(L, ret);
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
    StrumScriptTarget strum_script_target = luascript_parse_strumscripttarget(L, luaL_optstring(L, 2, NULL));
    StrumScriptOn strum_script_on = luascript_parse_strumscripton(L, luaL_optstring(L, 3, NULL));
    bool undo = (bool)lua_toboolean(L, 4);
    AnimSprite animsprite = luascript_read_nullable_userdata(L, 5, ANIMSPRITE);

    strum_set_extra_animation(strum, strum_script_target, strum_script_on, undo, animsprite);

    return 0;
}

static int script_strum_set_extra_animation_continuous(lua_State* L) {
    Strum strum = luascript_read_userdata(L, STRUM);
    StrumScriptTarget strum_script_target = luascript_parse_strumscripttarget(L, luaL_optstring(L, 2, NULL));
    AnimSprite animsprite = luascript_read_nullable_userdata(L, 3, ANIMSPRITE);

    strum_set_extra_animation_continuous(strum, strum_script_target, animsprite);

    return 0;
}

static int script_strum_set_notesmaker_tweenlerp(lua_State* L) {
    Strum strum = luascript_read_userdata(L, STRUM);
    TweenLerp tweenlerp = luascript_read_nullable_userdata(L, 2, TWEENLERP);
    bool apply_to_marker_too = (bool)lua_toboolean(L, 3);

    strum_set_notesmaker_tweenlerp(strum, tweenlerp, apply_to_marker_too);

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

    Modifier ret = strum_get_modifier(strum);

    return script_modifier_new(L, ret);
}

static int script_strum_get_duration(lua_State* L) {
    Strum strum = luascript_read_userdata(L, STRUM);

    double ret = strum_get_duration(strum);

    lua_pushnumber(L, ret);
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




static const luaL_Reg STRUM_FUNCTIONS[] = {
    { "update_draw_location", script_strum_update_draw_location },
    { "set_scroll_speed", script_strum_set_scroll_speed },
    { "set_scroll_direction", script_strum_set_scroll_direction },
    { "set_marker_duration_multiplier", script_strum_set_marker_duration_multiplier },
    { "reset", script_strum_reset },
    { "force_key_release", script_strum_force_key_release },
    { "get_press_state_changes", script_strum_get_press_state_changes },
    { "get_press_state", script_strum_get_press_state },
    { "get_name", script_strum_get_name },
    { "get_marker_duration", script_strum_get_marker_duration },
    { "set_player_id", script_strum_set_player_id },
    { "enable_background", script_strum_enable_background },
    { "enable_sick_effect", script_strum_enable_sick_effect },
    { "state_add", script_strum_state_add },
    { "state_toggle", script_strum_state_toggle },
    { "state_toggle_notes", script_strum_state_toggle_notes },
    { "state_toggle_sick_effect", script_strum_state_toggle_sick_effect },
    { "state_toggle_marker", script_strum_state_toggle_marker },
    { "state_toggle_background", script_strum_state_toggle_background },
    { "set_alpha_background", script_strum_set_alpha_background },
    { "set_alpha_sick_effect", script_strum_set_alpha_sick_effect },
    { "set_keep_aspect_ratio_background", script_strum_set_keep_aspect_ratio_background },
    { "draw_sick_effect_apart", script_strum_draw_sick_effect_apart },
    { "set_extra_animation", script_strum_set_extra_animation },
    { "set_extra_animation_continuous", script_strum_set_extra_animation_continuous },
    { "set_notesmaker_tweenlerp", script_strum_set_notesmaker_tweenlerp },
    { "set_sickeffect_size_ratio", script_strum_set_sickeffect_size_ratio },
    { "set_alpha", script_strum_set_alpha },
    { "set_visible", script_strum_set_visible },
    { "set_draw_offset", script_strum_set_draw_offset },
    { "get_modifier", script_strum_get_modifier },
    { "get_duration", script_strum_get_duration },
    { "animation_restart", script_strum_animation_restart },
    { "animation_end", script_strum_animation_end },
    { "get_drawable", script_strum_get_drawable },
    { NULL, NULL }
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

