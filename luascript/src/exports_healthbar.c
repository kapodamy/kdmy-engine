#include "luascript_internal.h"

#ifdef JAVASCRIPT
EM_JS_PRFX(void, healthbar_enable_extra_length, (HealthBar healthbar, bool extra_enabled), {
    healthbar_enable_extra_length(kdmyEngine_obtain(healthbar), extra_enabled);
});
EM_JS_PRFX(void, healthbar_enable_vertical, (HealthBar healthbar, bool enable_vertical), {
    healthbar_enable_vertical(kdmyEngine_obtain(healthbar), enable_vertical);
});
EM_JS_PRFX(int32_t, healthbar_state_opponent_add, (HealthBar healthbar, ModelHolder icon_mdlhldr, ModelHolder bar_mdlhldr, const char* state_name), {
    let ret = healthbar_state_opponent_add(kdmyEngine_obtain(healthbar), kdmyEngine_obtain(icon_mdlhldr), kdmyEngine_obtain(bar_mdlhldr), kdmyEngine_ptrToString(state_name));
    return ret;
});
EM_JS_PRFX(int32_t, healthbar_state_opponent_add2, (HealthBar healthbar, ModelHolder icon_mdlhldr, uint32_t bar_color_rgb8, const char* state_name), {
    let ret = healthbar_state_opponent_add2(kdmyEngine_obtain(healthbar), kdmyEngine_obtain(icon_mdlhldr), bar_color_rgb8, kdmyEngine_ptrToString(state_name));
    return ret;
});
EM_JS_PRFX(int32_t, healthbar_state_player_add, (HealthBar healthbar, ModelHolder icon_mdlhldr, ModelHolder bar_mdlhldr, const char* state_name), {
    let ret = healthbar_state_player_add(kdmyEngine_obtain(healthbar), kdmyEngine_obtain(icon_mdlhldr), kdmyEngine_obtain(bar_mdlhldr), kdmyEngine_ptrToString(state_name));
    return ret;
});
EM_JS_PRFX(int32_t, healthbar_state_player_add2, (HealthBar healthbar, ModelHolder icon_modelholder, uint32_t bar_color_rgb8, const char* state_name), {
    let ret = healthbar_state_player_add2(kdmyEngine_obtain(healthbar), kdmyEngine_obtain(icon_modelholder), bar_color_rgb8, kdmyEngine_ptrToString(state_name));
    return ret;
});
EM_JS_PRFX(bool, healthbar_state_background_add, (HealthBar healthbar, ModelHolder modelholder, const char* state_name), {
    let ret = healthbar_state_background_add(kdmyEngine_obtain(healthbar), kdmyEngine_obtain(modelholder), kdmyEngine_ptrToString(state_name));
    return ret ? 1 : 0;
});
EM_JS_PRFX(bool, healthbar_state_background_add2, (HealthBar healthbar, uint32_t color_rgb8, AnimSprite animsprite, const char* state_name), {
    let ret = healthbar_state_background_add2(kdmyEngine_obtain(healthbar), color_rgb8, kdmyEngine_obtain(animsprite), kdmyEngine_ptrToString(state_name));
    return ret ? 1 : 0;
});
EM_JS_PRFX(bool, healthbar_load_warnings, (HealthBar healthbar, ModelHolder modelholder, bool use_alt_icons), {
    let ret = healthbar_load_warnings(kdmyEngine_obtain(healthbar), kdmyEngine_obtain(modelholder), use_alt_icons);
    return ret ? 1 : 0;
});
EM_JS_PRFX(void, healthbar_set_opponent_bar_color_rgb8, (HealthBar healthbar, uint32_t color_rgb8), {
    healthbar_set_opponent_bar_color_rgb8(kdmyEngine_obtain(healthbar), color_rgb8);
});
EM_JS_PRFX(void, healthbar_set_opponent_bar_color, (HealthBar healthbar, float r, float g, float b), {
    healthbar_set_opponent_bar_color(kdmyEngine_obtain(healthbar), r, g, b);
});
EM_JS_PRFX(void, healthbar_set_player_bar_color_rgb8, (HealthBar healthbar, uint32_t color_rgb8), {
    healthbar_set_player_bar_color_rgb8(kdmyEngine_obtain(healthbar), color_rgb8);
});
EM_JS_PRFX(void, healthbar_set_player_bar_color, (HealthBar healthbar, float r, float g, float b), {
    healthbar_set_player_bar_color(kdmyEngine_obtain(healthbar), r, g, b);
});
EM_JS_PRFX(int32_t, healthbar_state_toggle, (HealthBar healthbar, const char* state_name), {
    let ret = healthbar_state_toggle(kdmyEngine_obtain(healthbar), kdmyEngine_ptrToString(state_name));
    return ret;
});
EM_JS_PRFX(bool, healthbar_state_toggle_background, (HealthBar healthbar, const char* state_name), {
    let ret = healthbar_state_toggle_background(kdmyEngine_obtain(healthbar), kdmyEngine_ptrToString(state_name));
    return ret ? 1 : 0;
});
EM_JS_PRFX(bool, healthbar_state_toggle_player, (HealthBar healthbar, const char* state_name), {
    let ret = healthbar_state_toggle_player(kdmyEngine_obtain(healthbar), kdmyEngine_ptrToString(state_name));
    return ret ? 1 : 0;
});
EM_JS_PRFX(bool, healthbar_state_toggle_opponent, (HealthBar healthbar, const char* state_name), {
    let ret = healthbar_state_toggle_opponent(kdmyEngine_obtain(healthbar), kdmyEngine_ptrToString(state_name));
    return ret ? 1 : 0;
});
EM_JS_PRFX(void, healthbar_set_bump_animation_opponent, (HealthBar healthbar, AnimSprite animsprite), {
    healthbar_set_bump_animation_opponent(kdmyEngine_obtain(healthbar), kdmyEngine_obtain(animsprite));
});
EM_JS_PRFX(void, healthbar_set_bump_animation_player, (HealthBar healthbar, AnimSprite animsprite), {
    healthbar_set_bump_animation_player(kdmyEngine_obtain(healthbar), kdmyEngine_obtain(animsprite));
});
EM_JS_PRFX(void, healthbar_bump_enable, (HealthBar healthbar, bool enable_bump), {
    healthbar_bump_enable(kdmyEngine_obtain(healthbar), enable_bump);
});
EM_JS_PRFX(void, healthbar_set_bpm, (HealthBar healthbar, float beats_per_minute), {
    healthbar_set_bpm(kdmyEngine_obtain(healthbar), beats_per_minute);
});
EM_JS_PRFX(void, healthbar_set_alpha, (HealthBar healthbar, float alpha), {
    healthbar_set_alpha(kdmyEngine_obtain(healthbar), alpha);
});
EM_JS_PRFX(void, healthbar_set_visible, (HealthBar healthbar, bool visible), {
    healthbar_set_visible(kdmyEngine_obtain(healthbar), visible);
});
EM_JS_PRFX(Drawable, healthbar_get_drawable, (HealthBar healthbar), {
    let ret = healthbar_get_drawable(kdmyEngine_obtain(healthbar));
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(void, healthbar_animation_set, (HealthBar healthbar, AnimSprite animsprite), {
    healthbar_animation_set(kdmyEngine_obtain(healthbar), kdmyEngine_obtain(animsprite));
});
EM_JS_PRFX(void, healthbar_animation_restart, (HealthBar healthbar), {
    healthbar_animation_restart(kdmyEngine_obtain(healthbar));
});
EM_JS_PRFX(void, healthbar_animation_end, (HealthBar healthbar), {
    healthbar_animation_end(kdmyEngine_obtain(healthbar));
});
EM_JS_PRFX(void, healthbar_disable_progress_animation, (HealthBar healthbar, bool disable), {
    healthbar_disable_progress_animation(kdmyEngine_obtain(healthbar), disable);
});
EM_JS_PRFX(float, healthbar_set_health_position, (HealthBar healthbar, float max_health, float health, bool opponent_recover), {
    let ret = healthbar_set_health_position(kdmyEngine_obtain(healthbar), max_health, health, opponent_recover);
    return ret;
});
EM_JS_PRFX(void, healthbar_set_health_position2, (HealthBar healthbar, float percent), {
    healthbar_set_health_position2(kdmyEngine_obtain(healthbar), percent);
});
EM_JS_PRFX(void, healthbar_disable_icon_overlap, (HealthBar healthbar, bool disable), {
    healthbar_disable_icon_overlap(kdmyEngine_obtain(healthbar), disable);
});
EM_JS_PRFX(void, healthbar_disable_warnings, (HealthBar healthbar, bool disable), {
    healthbar_disable_warnings(kdmyEngine_obtain(healthbar), disable);
});
EM_JS_PRFX(void, healthbar_enable_low_health_flash_warning, (HealthBar healthbar, bool enable), {
    healthbar_enable_low_health_flash_warning(kdmyEngine_obtain(healthbar), enable);
});
EM_JS_PRFX(void, healthbar_hide_warnings, (HealthBar healthbar), {
    healthbar_hide_warnings(kdmyEngine_obtain(healthbar));
});
EM_JS_PRFX(void, healthbar_show_drain_warning, (HealthBar healthbar, bool use_fast_drain), {
    healthbar_show_drain_warning(kdmyEngine_obtain(healthbar), use_fast_drain);
});
EM_JS_PRFX(void, healthbar_show_locked_warning, (HealthBar healthbar), {
    healthbar_show_locked_warning(kdmyEngine_obtain(healthbar));
});
EM_JS_PRFX(void, healthbar_get_bar_midpoint, (HealthBar healthbar, float* x, float* y), {
    const values = [0, 0];
    healthbar_get_bar_midpoint(kdmyEngine_obtain(healthbar), values);
    
    kdmyEngine_set_float32(x, values[0]);
    kdmyEngine_set_float32(y, values[1]);
});
EM_JS_PRFX(float, healthbar_get_percent, (HealthBar healthbar), {
    let ret = healthbar_get_percent(kdmyEngine_obtain(healthbar));
    return ret;
});
#endif


static int script_healthbar_enable_extra_length(lua_State* L) {
    HealthBar healthbar = luascript_read_userdata(L, HEALTHBAR);
    bool extra_enabled = (bool)lua_toboolean(L, 2);

    healthbar_enable_extra_length(healthbar, extra_enabled);

    return 0;
}

static int script_healthbar_enable_vertical(lua_State* L) {
    HealthBar healthbar = luascript_read_userdata(L, HEALTHBAR);
    bool enable_vertical = (bool)lua_toboolean(L, 2);

    healthbar_enable_vertical(healthbar, enable_vertical);

    return 0;
}

static int script_healthbar_state_opponent_add(lua_State* L) {
    HealthBar healthbar = luascript_read_userdata(L, HEALTHBAR);
    ModelHolder icon_mdlhldr = luascript_read_nullable_userdata(L, 2, MODELHOLDER);
    ModelHolder bar_mdlhldr = luascript_read_nullable_userdata(L, 3, MODELHOLDER);
    const char* state_name = luaL_optstring(L, 4, NULL);

    int32_t ret = healthbar_state_opponent_add(healthbar, icon_mdlhldr, bar_mdlhldr, state_name);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_healthbar_state_opponent_add2(lua_State* L) {
    HealthBar healthbar = luascript_read_userdata(L, HEALTHBAR);
    ModelHolder icon_mdlhldr = luascript_read_nullable_userdata(L, 2, MODELHOLDER);
    uint32_t bar_color_rgb8 = (uint32_t)luaL_checkinteger(L, 3);
    const char* state_name = luaL_optstring(L, 4, NULL);

    int32_t ret = healthbar_state_opponent_add2(healthbar, icon_mdlhldr, bar_color_rgb8, state_name);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_healthbar_state_player_add(lua_State* L) {
    HealthBar healthbar = luascript_read_userdata(L, HEALTHBAR);
    ModelHolder icon_mdlhldr = luascript_read_nullable_userdata(L, 2, MODELHOLDER);
    ModelHolder bar_mdlhldr = luascript_read_nullable_userdata(L, 3, MODELHOLDER);
    const char* state_name = luaL_optstring(L, 4, NULL);

    int32_t ret = healthbar_state_player_add(healthbar, icon_mdlhldr, bar_mdlhldr, state_name);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_healthbar_state_player_add2(lua_State* L) {
    HealthBar healthbar = luascript_read_userdata(L, HEALTHBAR);
    ModelHolder icon_modelholder = luascript_read_nullable_userdata(L, 2, MODELHOLDER);
    uint32_t bar_color_rgb8 = (uint32_t)luaL_checkinteger(L, 3);
    const char* state_name = luaL_optstring(L, 4, NULL);

    int32_t ret = healthbar_state_player_add2(healthbar, icon_modelholder, bar_color_rgb8, state_name);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_healthbar_state_background_add(lua_State* L) {
    HealthBar healthbar = luascript_read_userdata(L, HEALTHBAR);
    ModelHolder modelholder = luascript_read_nullable_userdata(L, 2, MODELHOLDER);
    const char* state_name = luaL_optstring(L, 3, NULL);

    bool ret = healthbar_state_background_add(healthbar, modelholder, state_name);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_healthbar_state_background_add2(lua_State* L) {
    HealthBar healthbar = luascript_read_userdata(L, HEALTHBAR);
    uint32_t color_rgb8 = (uint32_t)luaL_checkinteger(L, 2);
    AnimSprite animsprite = luascript_read_nullable_userdata(L, 3, ANIMSPRITE);
    const char* state_name = luaL_optstring(L, 4, NULL);

    bool ret = healthbar_state_background_add2(healthbar, color_rgb8, animsprite, state_name);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_healthbar_load_warnings(lua_State* L) {
    HealthBar healthbar = luascript_read_userdata(L, HEALTHBAR);
    ModelHolder modelholder = luascript_read_nullable_userdata(L, 2, MODELHOLDER);
    bool use_alt_icons = (bool)lua_toboolean(L, 3);

    bool ret = healthbar_load_warnings(healthbar, modelholder, use_alt_icons);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_healthbar_set_opponent_bar_color_rgb8(lua_State* L) {
    HealthBar healthbar = luascript_read_userdata(L, HEALTHBAR);
    uint32_t color_rgb8 = (uint32_t)luaL_checkinteger(L, 2);

    healthbar_set_opponent_bar_color_rgb8(healthbar, color_rgb8);

    return 0;
}

static int script_healthbar_set_opponent_bar_color(lua_State* L) {
    HealthBar healthbar = luascript_read_userdata(L, HEALTHBAR);
    float r = (float)luaL_checknumber(L, 2);
    float g = (float)luaL_checknumber(L, 3);
    float b = (float)luaL_checknumber(L, 4);

    healthbar_set_opponent_bar_color(healthbar, r, g, b);

    return 0;
}

static int script_healthbar_set_player_bar_color_rgb8(lua_State* L) {
    HealthBar healthbar = luascript_read_userdata(L, HEALTHBAR);
    uint32_t color_rgb8 = (uint32_t)luaL_checkinteger(L, 2);

    healthbar_set_player_bar_color_rgb8(healthbar, color_rgb8);

    return 0;
}

static int script_healthbar_set_player_bar_color(lua_State* L) {
    HealthBar healthbar = luascript_read_userdata(L, HEALTHBAR);
    float r = (float)luaL_checknumber(L, 2);
    float g = (float)luaL_checknumber(L, 3);
    float b = (float)luaL_checknumber(L, 4);

    healthbar_set_player_bar_color(healthbar, r, g, b);

    return 0;
}

static int script_healthbar_state_toggle(lua_State* L) {
    HealthBar healthbar = luascript_read_userdata(L, HEALTHBAR);
    const char* state_name = luaL_optstring(L, 2, NULL);

    int32_t ret = healthbar_state_toggle(healthbar, state_name);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_healthbar_state_toggle_background(lua_State* L) {
    HealthBar healthbar = luascript_read_userdata(L, HEALTHBAR);
    const char* state_name = luaL_optstring(L, 2, NULL);

    bool ret = healthbar_state_toggle_background(healthbar, state_name);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_healthbar_state_toggle_player(lua_State* L) {
    HealthBar healthbar = luascript_read_userdata(L, HEALTHBAR);
    const char* state_name = luaL_optstring(L, 2, NULL);

    bool ret = healthbar_state_toggle_player(healthbar, state_name);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_healthbar_state_toggle_opponent(lua_State* L) {
    HealthBar healthbar = luascript_read_userdata(L, HEALTHBAR);
    const char* state_name = luaL_optstring(L, 2, NULL);

    bool ret = healthbar_state_toggle_opponent(healthbar, state_name);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_healthbar_set_bump_animation_opponent(lua_State* L) {
    HealthBar healthbar = luascript_read_userdata(L, HEALTHBAR);
    AnimSprite animsprite = luascript_read_nullable_userdata(L, 2, ANIMSPRITE);

    healthbar_set_bump_animation_opponent(healthbar, animsprite);

    return 0;
}

static int script_healthbar_set_bump_animation_player(lua_State* L) {
    HealthBar healthbar = luascript_read_userdata(L, HEALTHBAR);
    AnimSprite animsprite = luascript_read_nullable_userdata(L, 2, ANIMSPRITE);

    healthbar_set_bump_animation_player(healthbar, animsprite);

    return 0;
}

static int script_healthbar_bump_enable(lua_State* L) {
    HealthBar healthbar = luascript_read_userdata(L, HEALTHBAR);
    bool enable_bump = (bool)lua_toboolean(L, 2);

    healthbar_bump_enable(healthbar, enable_bump);

    return 0;
}

static int script_healthbar_set_bpm(lua_State* L) {
    HealthBar healthbar = luascript_read_userdata(L, HEALTHBAR);
    float beats_per_minute = (float)luaL_checknumber(L, 2);

    healthbar_set_bpm(healthbar, beats_per_minute);

    return 0;
}

static int script_healthbar_set_alpha(lua_State* L) {
    HealthBar healthbar = luascript_read_userdata(L, HEALTHBAR);
    float alpha = (float)luaL_checknumber(L, 2);

    healthbar_set_alpha(healthbar, alpha);

    return 0;
}

static int script_healthbar_set_visible(lua_State* L) {
    HealthBar healthbar = luascript_read_userdata(L, HEALTHBAR);
    bool visible = (bool)lua_toboolean(L, 2);

    healthbar_set_visible(healthbar, visible);

    return 0;
}

static int script_healthbar_get_drawable(lua_State* L) {
    HealthBar healthbar = luascript_read_userdata(L, HEALTHBAR);

    Drawable ret = healthbar_get_drawable(healthbar);

    return script_drawable_new(L, ret);
}

static int script_healthbar_animation_set(lua_State* L) {
    HealthBar healthbar = luascript_read_userdata(L, HEALTHBAR);
    AnimSprite animsprite = luascript_read_nullable_userdata(L, 2, ANIMSPRITE);

    healthbar_animation_set(healthbar, animsprite);

    return 0;
}

static int script_healthbar_animation_restart(lua_State* L) {
    HealthBar healthbar = luascript_read_userdata(L, HEALTHBAR);

    healthbar_animation_restart(healthbar);

    return 0;
}

static int script_healthbar_animation_end(lua_State* L) {
    HealthBar healthbar = luascript_read_userdata(L, HEALTHBAR);

    healthbar_animation_end(healthbar);

    return 0;
}

static int script_healthbar_disable_progress_animation(lua_State* L) {
    HealthBar healthbar = luascript_read_userdata(L, HEALTHBAR);
    bool disable = (bool)lua_toboolean(L, 2);

    healthbar_disable_progress_animation(healthbar, disable);

    return 0;
}

static int script_healthbar_set_health_position(lua_State* L) {
    HealthBar healthbar = luascript_read_userdata(L, HEALTHBAR);
    float max_health = (float)luaL_checknumber(L, 2);
    float health = (float)luaL_checknumber(L, 3);
    bool opponent_recover = (bool)lua_toboolean(L, 4);

    float ret = healthbar_set_health_position(healthbar, max_health, health, opponent_recover);

    lua_pushnumber(L, ret);
    return 1;
}

static int script_healthbar_set_health_position2(lua_State* L) {
    HealthBar healthbar = luascript_read_userdata(L, HEALTHBAR);
    float percent = (float)luaL_checknumber(L, 2);

    healthbar_set_health_position2(healthbar, percent);

    return 0;
}

static int script_healthbar_disable_icon_overlap(lua_State* L) {
    HealthBar healthbar = luascript_read_userdata(L, HEALTHBAR);
    bool disable = (bool)lua_toboolean(L, 2);

    healthbar_disable_icon_overlap(healthbar, disable);

    return 0;
}

static int script_healthbar_disable_warnings(lua_State* L) {
    HealthBar healthbar = luascript_read_userdata(L, HEALTHBAR);
    bool disable = (bool)lua_toboolean(L, 2);

    healthbar_disable_warnings(healthbar, disable);

    return 0;
}

static int script_healthbar_enable_low_health_flash_warning(lua_State* L) {
    HealthBar healthbar = luascript_read_userdata(L, HEALTHBAR);
    bool enable = (bool)lua_toboolean(L, 2);

    healthbar_enable_low_health_flash_warning(healthbar, enable);

    return 0;
}

static int script_healthbar_hide_warnings(lua_State* L) {
    HealthBar healthbar = luascript_read_userdata(L, HEALTHBAR);

    healthbar_hide_warnings(healthbar);

    return 0;
}

static int script_healthbar_show_drain_warning(lua_State* L) {
    HealthBar healthbar = luascript_read_userdata(L, HEALTHBAR);
    bool use_fast_drain = (bool)lua_toboolean(L, 2);

    healthbar_show_drain_warning(healthbar, use_fast_drain);

    return 0;
}

static int script_healthbar_show_locked_warning(lua_State* L) {
    HealthBar healthbar = luascript_read_userdata(L, HEALTHBAR);

    healthbar_show_locked_warning(healthbar);

    return 0;
}

static int script_healthbar_get_bar_midpoint(lua_State* L) {
    HealthBar healthbar = luascript_read_userdata(L, HEALTHBAR);
    float x, y;

    healthbar_get_bar_midpoint(healthbar, &x, &y);

    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    return 0;
}

static int script_healthbar_get_percent(lua_State* L) {
    HealthBar healthbar = luascript_read_userdata(L, HEALTHBAR);

    float ret = healthbar_get_percent(healthbar);

    lua_pushnumber(L, ret);
    return 1;
}




static const luaL_Reg HEALTHBAR_FUNCTIONS[] = {
    { "enable_extra_length", script_healthbar_enable_extra_length },
    { "enable_vertical", script_healthbar_enable_vertical },
    { "state_opponent_add", script_healthbar_state_opponent_add },
    { "state_opponent_add2", script_healthbar_state_opponent_add2 },
    { "state_player_add", script_healthbar_state_player_add },
    { "state_player_add2", script_healthbar_state_player_add2 },
    { "state_background_add", script_healthbar_state_background_add },
    { "state_background_add2", script_healthbar_state_background_add2 },
    { "load_warnings", script_healthbar_load_warnings },
    { "set_opponent_bar_color_rgb8", script_healthbar_set_opponent_bar_color_rgb8 },
    { "set_opponent_bar_color", script_healthbar_set_opponent_bar_color },
    { "set_player_bar_color_rgb8", script_healthbar_set_player_bar_color_rgb8 },
    { "set_player_bar_color", script_healthbar_set_player_bar_color },
    { "state_toggle", script_healthbar_state_toggle },
    { "state_toggle_background", script_healthbar_state_toggle_background },
    { "state_toggle_player", script_healthbar_state_toggle_player },
    { "state_toggle_opponent", script_healthbar_state_toggle_opponent },
    { "set_bump_animation_opponent", script_healthbar_set_bump_animation_opponent },
    { "set_bump_animation_player", script_healthbar_set_bump_animation_player },
    { "bump_enable", script_healthbar_bump_enable },
    { "set_bpm", script_healthbar_set_bpm },
    { "set_alpha", script_healthbar_set_alpha },
    { "set_visible", script_healthbar_set_visible },
    { "get_drawable", script_healthbar_get_drawable },
    { "animation_set", script_healthbar_animation_set },
    { "animation_restart", script_healthbar_animation_restart },
    { "animation_end", script_healthbar_animation_end },
    { "disable_progress_animation", script_healthbar_disable_progress_animation },
    { "set_health_position", script_healthbar_set_health_position },
    { "set_health_position2", script_healthbar_set_health_position2 },
    { "disable_icon_overlap", script_healthbar_disable_icon_overlap },
    { "disable_warnings", script_healthbar_disable_warnings },
    { "enable_low_health_flash_warning", script_healthbar_enable_low_health_flash_warning },
    { "hide_warnings", script_healthbar_hide_warnings },
    { "show_drain_warning", script_healthbar_show_drain_warning },
    { "show_locked_warning", script_healthbar_show_locked_warning },
    { "get_bar_midpoint", script_healthbar_get_bar_midpoint },
    { "get_percent", script_healthbar_get_percent },
    { NULL, NULL }
};

int script_healthbar_new(lua_State* L, HealthBar healthbar) {
    return luascript_userdata_new(L, HEALTHBAR, healthbar);
}

static int script_healthbar_gc(lua_State* L) {
    return luascript_userdata_gc(L, HEALTHBAR);
}

static int script_healthbar_tostring(lua_State* L) {
    return luascript_userdata_tostring(L, HEALTHBAR);
}

void script_healthbar_register(lua_State* L) {
    luascript_register(L, HEALTHBAR, script_healthbar_gc, script_healthbar_tostring, HEALTHBAR_FUNCTIONS);
}

