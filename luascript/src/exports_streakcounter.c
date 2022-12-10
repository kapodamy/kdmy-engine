#include "luascript_internal.h"

#ifdef JAVASCRIPT
EM_JS_PRFX(void, streakcounter_reset, (StreakCounter streakcounter), {
    streakcounter_reset(kdmyEngine_obtain(streakcounter));
});
EM_JS_PRFX(void, streakcounter_hide_combo_sprite, (StreakCounter streakcounter, bool hide), {
    streakcounter_hide_combo_sprite(kdmyEngine_obtain(streakcounter), hide);
});
EM_JS_PRFX(void, streakcounter_set_combo_draw_location, (StreakCounter streakcounter, float x, float y), {
    streakcounter_set_combo_draw_location(kdmyEngine_obtain(streakcounter), x, y);
});
EM_JS_PRFX(int32_t, streakcounter_state_add, (StreakCounter streakcounter, ModelHolder combo_modelholder, ModelHolder number_modelholder, const char* state_name), {
    let ret = streakcounter_state_add(kdmyEngine_obtain(streakcounter), kdmyEngine_obtain(combo_modelholder), kdmyEngine_obtain(number_modelholder), kdmyEngine_ptrToString(state_name));
    return ret;
});
EM_JS_PRFX(bool, streakcounter_state_toggle, (StreakCounter streakcounter, const char* state_name), {
    let ret = streakcounter_state_toggle(kdmyEngine_obtain(streakcounter), kdmyEngine_ptrToString(state_name));
    return ret ? 1 : 0;
});
EM_JS_PRFX(void, streakcounter_set_alpha, (StreakCounter streakcounter, float alpha), {
    streakcounter_set_alpha(kdmyEngine_obtain(streakcounter), alpha);
});
EM_JS_PRFX(Drawable, streakcounter_get_drawable, (StreakCounter streakcounter), {
    let ret = streakcounter_get_drawable(kdmyEngine_obtain(streakcounter));
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(void, streakcounter_animation_set, (StreakCounter streakcounter, AnimSprite animsprite), {
    streakcounter_animation_set(kdmyEngine_obtain(streakcounter), kdmyEngine_obtain(animsprite));
});
EM_JS_PRFX(void, streakcounter_animation_restart, (StreakCounter streakcounter), {
    streakcounter_animation_restart(kdmyEngine_obtain(streakcounter));
});
EM_JS_PRFX(void, streakcounter_animation_end, (StreakCounter streakcounter), {
    streakcounter_animation_end(kdmyEngine_obtain(streakcounter));
});
#endif


static int script_streakcounter_reset(lua_State* L) {
    StreakCounter streakcounter = luascript_read_userdata(L, STREAKCOUNTER);

    streakcounter_reset(streakcounter);

    return 0;
}

static int script_streakcounter_hide_combo_sprite(lua_State* L) {
    StreakCounter streakcounter = luascript_read_userdata(L, STREAKCOUNTER);
    bool hide = (bool)lua_toboolean(L, 2);

    streakcounter_hide_combo_sprite(streakcounter, hide);

    return 0;
}

static int script_streakcounter_set_combo_draw_location(lua_State* L) {
    StreakCounter streakcounter = luascript_read_userdata(L, STREAKCOUNTER);
    float x = (float)luaL_checknumber(L, 2);
    float y = (float)luaL_checknumber(L, 3);

    streakcounter_set_combo_draw_location(streakcounter, x, y);

    return 0;
}

static int script_streakcounter_state_add(lua_State* L) {
    StreakCounter streakcounter = luascript_read_userdata(L, STREAKCOUNTER);
    ModelHolder combo_modelholder = luascript_read_nullable_userdata(L, 2, MODELHOLDER);
    ModelHolder number_modelholder = luascript_read_nullable_userdata(L, 3, MODELHOLDER);
    const char* state_name = luaL_optstring(L, 4, NULL);

    int32_t ret = streakcounter_state_add(streakcounter, combo_modelholder, number_modelholder, state_name);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_streakcounter_state_toggle(lua_State* L) {
    StreakCounter streakcounter = luascript_read_userdata(L, STREAKCOUNTER);
    const char* state_name = luaL_optstring(L, 2, NULL);

    bool ret = streakcounter_state_toggle(streakcounter, state_name);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_streakcounter_set_alpha(lua_State* L) {
    StreakCounter streakcounter = luascript_read_userdata(L, STREAKCOUNTER);
    float alpha = (float)luaL_checknumber(L, 2);

    streakcounter_set_alpha(streakcounter, alpha);

    return 0;
}

static int script_streakcounter_get_drawable(lua_State* L) {
    StreakCounter streakcounter = luascript_read_userdata(L, STREAKCOUNTER);

    Drawable ret = streakcounter_get_drawable(streakcounter);

    return script_drawable_new(L, ret);
}

static int script_streakcounter_animation_set(lua_State* L) {
    StreakCounter streakcounter = luascript_read_userdata(L, STREAKCOUNTER);
    AnimSprite animsprite = luascript_read_nullable_userdata(L, 2, ANIMSPRITE);

    streakcounter_animation_set(streakcounter, animsprite);

    return 0;
}

static int script_streakcounter_animation_restart(lua_State* L) {
    StreakCounter streakcounter = luascript_read_userdata(L, STREAKCOUNTER);

    streakcounter_animation_restart(streakcounter);

    return 0;
}

static int script_streakcounter_animation_end(lua_State* L) {
    StreakCounter streakcounter = luascript_read_userdata(L, STREAKCOUNTER);

    streakcounter_animation_end(streakcounter);

    return 0;
}




static const luaL_Reg STREAKCOUNTER_FUNCTIONS[] = {
    { "reset", script_streakcounter_reset },
    { "hide_combo_sprite", script_streakcounter_hide_combo_sprite },
    { "set_combo_draw_location", script_streakcounter_set_combo_draw_location },
    { "state_add", script_streakcounter_state_add },
    { "state_toggle", script_streakcounter_state_toggle },
    { "set_alpha", script_streakcounter_set_alpha },
    { "get_drawable", script_streakcounter_get_drawable },
    { "animation_set", script_streakcounter_animation_set },
    { "animation_restart", script_streakcounter_animation_restart },
    { "animation_end", script_streakcounter_animation_end },
    { NULL, NULL }
};

int script_streakcounter_new(lua_State* L, StreakCounter streakcounter) {
    return luascript_userdata_new(L, STREAKCOUNTER, streakcounter);
}

static int script_streakcounter_gc(lua_State* L) {
    return luascript_userdata_gc(L, STREAKCOUNTER);
}

static int script_streakcounter_tostring(lua_State* L) {
    return luascript_userdata_tostring(L, STREAKCOUNTER);
}

void script_streakcounter_register(lua_State* L) {
    luascript_register(L, STREAKCOUNTER, script_streakcounter_gc, script_streakcounter_tostring, STREAKCOUNTER_FUNCTIONS);
}

