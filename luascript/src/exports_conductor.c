#include "luascript_internal.h"

#ifdef JAVASCRIPT
EM_JS_PRFX(Conductor, conductor_init, (), {
    let ret = conductor_init();
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(void, conductor_destroy, (Conductor* conductor), {
    conductor_destroy(kdmyEngine_obtain(kdmyEngine_get_uint32(conductor)));
});
EM_JS_PRFX(void, conductor_poll_reset, (Conductor conductor), {
    conductor_poll_reset(kdmyEngine_obtain(conductor));
});
EM_JS_PRFX(void, conductor_set_character, (Conductor conductor, Character character), {
    conductor_set_character(kdmyEngine_obtain(conductor), kdmyEngine_obtain(character));
});
EM_JS_PRFX(void, conductor_use_strum_line, (Conductor conductor, Strum strum), {
    conductor_use_strum_line(kdmyEngine_obtain(conductor), kdmyEngine_obtain(strum));
});
EM_JS_PRFX(void, conductor_use_strums, (Conductor conductor, Strums strums), {
    conductor_use_strums(kdmyEngine_obtain(conductor), kdmyEngine_obtain(strums));
});
EM_JS_PRFX(bool, conductor_disable_strum_line, (Conductor conductor, Strum strum, bool should_disable), {
    let ret = conductor_disable_strum_line(kdmyEngine_obtain(conductor), kdmyEngine_obtain(strum), should_disable);
    return ret ? 1 : 0;
});
EM_JS_PRFX(bool, conductor_remove_strum, (Conductor conductor, Strum strum), {
    let ret = conductor_remove_strum(kdmyEngine_obtain(conductor), kdmyEngine_obtain(strum));
    return ret ? 1 : 0;
});
EM_JS_PRFX(void, conductor_clear_mapping, (Conductor conductor), {
    conductor_clear_mapping(kdmyEngine_obtain(conductor));
});
EM_JS_PRFX(void, conductor_map_strum_to_player_sing_add, (Conductor conductor, Strum strum, const char* sing_direction_name), {
    conductor_map_strum_to_player_sing_add(kdmyEngine_obtain(conductor), kdmyEngine_obtain(strum), kdmyEngine_ptrToString(sing_direction_name));
});
EM_JS_PRFX(void, conductor_map_strum_to_player_extra_add, (Conductor conductor, Strum strum, const char* extra_animation_name), {
    conductor_map_strum_to_player_extra_add(kdmyEngine_obtain(conductor), kdmyEngine_obtain(strum), kdmyEngine_ptrToString(extra_animation_name));
});
EM_JS_PRFX(void, conductor_map_strum_to_player_sing_remove, (Conductor conductor, Strum strum, const char* sing_direction_name), {
    conductor_map_strum_to_player_sing_remove(kdmyEngine_obtain(conductor), kdmyEngine_obtain(strum), kdmyEngine_ptrToString(sing_direction_name));
});
EM_JS_PRFX(void, conductor_map_strum_to_player_extra_remove, (Conductor conductor, Strum strum, const char* extra_animation_name), {
    conductor_map_strum_to_player_extra_remove(kdmyEngine_obtain(conductor), kdmyEngine_obtain(strum), kdmyEngine_ptrToString(extra_animation_name));
});
EM_JS_PRFX(int32_t, conductor_map_automatically, (Conductor conductor, bool should_map_extras), {
    let ret = conductor_map_automatically(kdmyEngine_obtain(conductor), should_map_extras);
    return ret;
});
EM_JS_PRFX(void, conductor_poll, (Conductor conductor), {
    conductor_poll(kdmyEngine_obtain(conductor));
});
EM_JS_PRFX(void, conductor_disable, (Conductor conductor, bool disable), {
    conductor_disable(kdmyEngine_obtain(conductor), disable);
});
EM_JS_PRFX(void, conductor_play_idle, (Conductor conductor), {
    conductor_play_idle(kdmyEngine_obtain(conductor));
});
EM_JS_PRFX(void, conductor_play_hey, (Conductor conductor), {
    conductor_play_hey(kdmyEngine_obtain(conductor));
});
EM_JS_PRFX(Character, conductor_get_character, (Conductor conductor), {
    let ret = conductor_get_character(kdmyEngine_obtain(conductor));
    return kdmyEngine_obtain(ret);
});
#endif


static int script_conductor_init(lua_State* L) {
    Conductor conductor = conductor_init();
    return luascript_userdata_allocnew(L, CONDUCTOR, conductor);
}

static int script_conductor_destroy(lua_State* L) {
    Conductor conductor = luascript_read_userdata(L, CONDUCTOR);

    if (luascript_userdata_is_allocated(L, CONDUCTOR))
        conductor_destroy(&conductor);
    else
        printf("script_conductor_destroy() object was not allocated by lua\n");

    return 0;
}

static int script_conductor_poll_reset(lua_State* L) {
    Conductor conductor = luascript_read_userdata(L, CONDUCTOR);

    conductor_poll_reset(conductor);

    return 0;
}

static int script_conductor_set_character(lua_State* L) {
    Conductor conductor = luascript_read_userdata(L, CONDUCTOR);
    Character character = luascript_read_nullable_userdata(L, 2, CHARACTER);

    conductor_set_character(conductor, character);

    return 0;
}

static int script_conductor_use_strum_line(lua_State* L) {
    Conductor conductor = luascript_read_userdata(L, CONDUCTOR);
    Strum strum = luascript_read_nullable_userdata(L, 2, STRUM);

    conductor_use_strum_line(conductor, strum);

    return 0;
}

static int script_conductor_use_strums(lua_State* L) {
    Conductor conductor = luascript_read_userdata(L, CONDUCTOR);
    Strums strums = luascript_read_nullable_userdata(L, 2, STRUMS);

    conductor_use_strums(conductor, strums);

    return 0;
}

static int script_conductor_disable_strum_line(lua_State* L) {
    Conductor conductor = luascript_read_userdata(L, CONDUCTOR);
    Strum strum = luascript_read_nullable_userdata(L, 2, STRUM);
    bool should_disable = (bool)lua_toboolean(L, 3);

    bool ret = conductor_disable_strum_line(conductor, strum, should_disable);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_conductor_remove_strum(lua_State* L) {
    Conductor conductor = luascript_read_userdata(L, CONDUCTOR);
    Strum strum = luascript_read_nullable_userdata(L, 2, STRUM);

    bool ret = conductor_remove_strum(conductor, strum);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_conductor_clear_mapping(lua_State* L) {
    Conductor conductor = luascript_read_userdata(L, CONDUCTOR);

    conductor_clear_mapping(conductor);

    return 0;
}

static int script_conductor_map_strum_to_player_sing_add(lua_State* L) {
    Conductor conductor = luascript_read_userdata(L, CONDUCTOR);
    Strum strum = luascript_read_nullable_userdata(L, 2, STRUM);
    const char* sing_direction_name = luaL_optstring(L, 3, NULL);

    conductor_map_strum_to_player_sing_add(conductor, strum, sing_direction_name);

    return 0;
}

static int script_conductor_map_strum_to_player_extra_add(lua_State* L) {
    Conductor conductor = luascript_read_userdata(L, CONDUCTOR);
    Strum strum = luascript_read_nullable_userdata(L, 2, STRUM);
    const char* extra_animation_name = luaL_optstring(L, 3, NULL);

    conductor_map_strum_to_player_extra_add(conductor, strum, extra_animation_name);

    return 0;
}

static int script_conductor_map_strum_to_player_sing_remove(lua_State* L) {
    Conductor conductor = luascript_read_userdata(L, CONDUCTOR);
    Strum strum = luascript_read_nullable_userdata(L, 2, STRUM);
    const char* sing_direction_name = luaL_optstring(L, 3, NULL);

    conductor_map_strum_to_player_sing_remove(conductor, strum, sing_direction_name);

    return 0;
}

static int script_conductor_map_strum_to_player_extra_remove(lua_State* L) {
    Conductor conductor = luascript_read_userdata(L, CONDUCTOR);
    Strum strum = luascript_read_nullable_userdata(L, 2, STRUM);
    const char* extra_animation_name = luaL_optstring(L, 3, NULL);

    conductor_map_strum_to_player_extra_remove(conductor, strum, extra_animation_name);

    return 0;
}

static int script_conductor_map_automatically(lua_State* L) {
    Conductor conductor = luascript_read_userdata(L, CONDUCTOR);
    bool should_map_extras = (bool)lua_toboolean(L, 2);

    int32_t ret = conductor_map_automatically(conductor, should_map_extras);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_conductor_poll(lua_State* L) {
    Conductor conductor = luascript_read_userdata(L, CONDUCTOR);

    conductor_poll(conductor);

    return 0;
}

static int script_conductor_disable(lua_State* L) {
    Conductor conductor = luascript_read_userdata(L, CONDUCTOR);
    bool disable = (bool)lua_toboolean(L, 2);

    conductor_disable(conductor, disable);

    return 0;
}

static int script_conductor_play_idle(lua_State* L) {
    Conductor conductor = luascript_read_userdata(L, CONDUCTOR);

    conductor_play_idle(conductor);

    return 0;
}

static int script_conductor_play_hey(lua_State* L) {
    Conductor conductor = luascript_read_userdata(L, CONDUCTOR);

    conductor_play_hey(conductor);

    return 0;
}

static int script_conductor_get_character(lua_State* L) {
    Conductor conductor = luascript_read_userdata(L, CONDUCTOR);

    Character character = conductor_get_character(conductor);

    return script_character_new(L, character);
}




static const luaL_Reg CONDUCTOR_FUNCTIONS[] = {
    { "init", script_conductor_init },
    { "destroy", script_conductor_destroy },
    { "poll_reset", script_conductor_poll_reset },
    { "set_character", script_conductor_set_character },
    { "use_strum_line", script_conductor_use_strum_line },
    { "use_strums", script_conductor_use_strums },
    { "disable_strum_line", script_conductor_disable_strum_line },
    { "remove_strum", script_conductor_remove_strum },
    { "clear_mapping", script_conductor_clear_mapping },
    { "map_strum_to_player_sing_add", script_conductor_map_strum_to_player_sing_add },
    { "map_strum_to_player_extra_add", script_conductor_map_strum_to_player_extra_add },
    { "map_strum_to_player_sing_remove", script_conductor_map_strum_to_player_sing_remove },
    { "map_strum_to_player_extra_remove", script_conductor_map_strum_to_player_extra_remove },
    { "map_automatically", script_conductor_map_automatically },
    { "poll", script_conductor_poll },
    { "disable", script_conductor_disable },
    { "play_idle", script_conductor_play_idle },
    { "play_hey", script_conductor_play_hey },
    { "get_character", script_conductor_get_character },
    { NULL, NULL }
};

int script_conductor_new(lua_State* L, Conductor conductor) {
    return luascript_userdata_new(L, CONDUCTOR, conductor);
}

static int script_conductor_gc(lua_State* L) {
    return luascript_userdata_destroy(L, CONDUCTOR, (Destructor)conductor_destroy);
}

static int script_conductor_tostring(lua_State* L) {
    return luascript_userdata_tostring(L, CONDUCTOR);
}

void script_conductor_register(lua_State* L) {
    luascript_register(L, CONDUCTOR, script_conductor_gc, script_conductor_tostring, CONDUCTOR_FUNCTIONS);
}

