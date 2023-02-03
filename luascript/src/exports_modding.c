#include "luascript_internal.h"
#include "string.h"

#ifdef JAVASCRIPT
EM_JS_PRFX(void, modding_set_ui_visibility, (Modding modding, bool visible), {
    modding_set_ui_visibility(kdmyEngine_obtain(modding), visible);
});
EM_JS_PRFX(Layout, modding_get_layout, (Modding modding), {
    let ret = modding_get_layout(kdmyEngine_obtain(modding));
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(void, modding_exit, (Modding modding), {
    modding_exit(kdmyEngine_obtain(modding));
});
EM_JS_PRFX(void, modding_set_halt, (Modding modding, bool halt), {
    modding_set_halt(kdmyEngine_obtain(modding), halt);
});
EM_JS_PRFX(void, modding_unlockdirective_create, (Modding modding, const char* name, double value), {
    modding_unlockdirective_create(kdmyEngine_obtain(modding), kdmyEngine_ptrToString(name), value);
});
EM_JS_PRFX(double, modding_unlockdirective_get, (Modding modding, const char* name), {
    let ret = modding_unlockdirective_get(kdmyEngine_obtain(modding), kdmyEngine_ptrToString(name));
    return ret;
});
EM_JS_PRFX(void, modding_unlockdirective_remove, (Modding modding, const char* name), {
    modding_unlockdirective_remove(kdmyEngine_obtain(modding), kdmyEngine_ptrToString(name));
});
EM_JS_PRFX(bool, modding_unlockdirective_has, (Modding modding, const char* name), {
    let ret = modding_unlockdirective_has(kdmyEngine_obtain(modding), kdmyEngine_ptrToString(name));
    return ret ? 1 : 0;
});
EM_JS_PRFX(Menu, modding_get_active_menu, (Modding modding), {
    let ret = modding_get_active_menu(kdmyEngine_obtain(modding));
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(bool, modding_choose_native_menu_option, (Modding modding, const char* name), {
    let ret = modding_choose_native_menu_option(kdmyEngine_obtain(modding), kdmyEngine_ptrToString(name));
    return ret ? 1 : 0;
});
EM_JS_PRFX(Menu, modding_get_native_menu, (Modding modding), {
    let ret = modding_get_native_menu(kdmyEngine_obtain(modding));
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(void, modding_set_active_menu, (Modding modding, Menu menu), {
    modding_set_active_menu(kdmyEngine_obtain(modding), kdmyEngine_obtain(menu));
});
EM_JS_PRFX(SoundPlayer, modding_get_native_background_music, (Modding modding), {
    let ret = modding_get_native_background_music(kdmyEngine_obtain(modding));
    return kdmyEngine_obtain(ret);
});
EM_ASYNC_JS_PRFX(SoundPlayer, modding_replace_native_background_music, (Modding modding, const char* music_src), {
    let ret = await modding_replace_native_background_music(kdmyEngine_obtain(modding), kdmyEngine_ptrToString(music_src));
    return kdmyEngine_obtain(ret);
});
EM_ASYNC_JS_PRFX(BasicValue*, modding_spawn_screen, (Modding modding, const char* layout_src, const char* script_src, BasicValue* value), {
    let val = {type : kdmyEngine_get_uint32(value), value : null};
    let value_value_ptr = value + 4;
    switch (val.type) {
        case BASIC_VALUE_TYPE_DOUBLE:
            val.value = kdmyEngine_dataView.getFloat64(value_value_ptr, kdmyEngine_endianess);
            break;
        case BASIC_VALUE_TYPE_NULL:
            break;
        case BASIC_VALUE_TYPE_STRING:
            let string_ptr = kdmyEngine_get_uint32(value_value_ptr);
            val.value = kdmyEngine_ptrToString(string_ptr);
            _free(string_ptr);
            break;
        case BASIC_VALUE_TYPE_BOOLEAN:
            val.value = kdmyEngine_get_uint32(value) ? true : false;
            break;
        default:
            throw new Error("value type " + val.type + "not implemented");
    }
    _free(value);

    let ret = await modding_spawn_screen(kdmyEngine_obtain(modding), kdmyEngine_ptrToString(layout_src), kdmyEngine_ptrToString(script_src), val);

    // allocate return BasicValue
    let r = _malloc(4 + 8);
    HEAPU32[r + 0] = ret.type;

    let r_value_ptr = r + 4;
    switch (ret.type) {
        case BASIC_VALUE_TYPE_DOUBLE:
            kdmyEngine_set_float64(r_value_ptr, ret.value);
            break;
        case BASIC_VALUE_TYPE_NULL:
            kdmyEngine_set_uint32(r_value_ptr, 0x00);
            break;
        case BASIC_VALUE_TYPE_STRING:
            kdmyEngine_set_float64(r_value_ptr, kdmyEngine_stringToPtr(ret.value));
            break;
        case BASIC_VALUE_TYPE_BOOLEAN:
            kdmyEngine_set_uint32(r_value_ptr, ret.value ? 0x01 : 0x00);
            break;
        default:
            throw new Error("value type " + ret.type + "not implemented");
    }

    return r;
});
EM_JS_PRFX(void, modding_set_exit_delay, (Modding modding, float delay_ms), {
    modding_set_exit_delay(kdmyEngine_obtain(modding), delay_ms);
});
EM_ASYNC_JS_PRFX(MessageBox, modding_get_messagebox, (Modding modding), {
    let ret = await modding_get_messagebox(kdmyEngine_obtain(modding));
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(void, modding_set_menu_in_layout_placeholder, (Modding modding, const char* placeholder_name, Menu menu), {
    modding_set_menu_in_layout_placeholder(kdmyEngine_obtain(modding), kdmyEngine_ptrToString(placeholder_names), kdmyEngine_obtain(menu));
});
#endif


static int script_modding_unlockdirective_create(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    Modding modding = (Modding)luascript->context;

    const char* name = luaL_optstring(L, 1, NULL);
    double value = (double)luaL_checknumber(L, 2);

    modding_unlockdirective_create(modding, name, value);

    return 0;
}

static int script_modding_unlockdirective_remove(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    Modding modding = (Modding)luascript->context;

    const char* name = luaL_optstring(L, 1, NULL);

    modding_unlockdirective_remove(modding, name);

    return 0;
}

static int script_modding_unlockdirective_get(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    Modding modding = (Modding)luascript->context;

    const char* name = luaL_optstring(L, 1, NULL);

    if (!modding_unlockdirective_has(modding, name)) {
        lua_pushnil(L);
        return 1;
    }

    double ret = modding_unlockdirective_get(modding, name);

    lua_pushnumber(L, ret);
    return 1;
}

static int script_modding_set_ui_visibility(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    Modding modding = (Modding)luascript->context;

    bool visible = (bool)lua_toboolean(L, 1);

    modding_set_ui_visibility(modding, visible);

    return 0;
}

static int script_modding_get_layout(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    Modding modding = (Modding)luascript->context;


    Layout ret = modding_get_layout(modding);

    return script_layout_new(L, ret);
}

static int script_modding_exit(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    Modding modding = (Modding)luascript->context;


    modding_exit(modding);

    return 0;
}

static int script_modding_set_halt(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    Modding modding = (Modding)luascript->context;

    bool halt = (bool)lua_toboolean(L, 1);

    modding_set_halt(modding, halt);

    return 0;
}

static int script_modding_get_active_menu(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    Modding modding = (Modding)luascript->context;

    Menu ret = modding_get_active_menu(modding);

    return script_menu_new(L, ret);
}

static int script_modding_choose_native_menu_option(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    Modding modding = (Modding)luascript->context;

    const char* name = luaL_optstring(L, 1, NULL);

    bool ret = modding_choose_native_menu_option(modding, name);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_modding_get_native_menu(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    Modding modding = (Modding)luascript->context;

    Menu ret = modding_get_native_menu(modding);

    return script_menu_new(L, ret);
}

static int script_modding_set_active_menu(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    Modding modding = (Modding)luascript->context;

    Menu menu = luascript_read_nullable_userdata(L, 1, MENU);

    modding_set_active_menu(modding, menu);

    return 0;
}

static int script_modding_get_native_background_music(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    Modding modding = (Modding)luascript->context;

    SoundPlayer ret = modding_get_native_background_music(modding);

    return script_soundplayer_new(L, ret);
}

static int script_modding_replace_native_background_music(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    Modding modding = (Modding)luascript->context;

    const char* music_src = luaL_optstring(L, 1, NULL);

    SoundPlayer ret = modding_replace_native_background_music(modding, music_src);

    return script_soundplayer_new(L, ret);
}

static int script_modding_spawn_screen(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    Modding modding = (Modding)luascript->context;

    const char* layout_src = luaL_optstring(L, 1, NULL);
    const char* script_src = luaL_optstring(L, 2, NULL);

    BasicValue basicvalue;
    switch (lua_type(L, 4)) {
        case LUA_TNONE:
        case LUA_TNIL:
            basicvalue.type = BasicValueTypeNULL;
            basicvalue.value.as_string = NULL;
            break;
        case LUA_TBOOLEAN:
            basicvalue.type = BasicValueTypeBoolean;
            basicvalue.value.as_boolean = lua_toboolean(L, 4);
            break;
        case LUA_TSTRING:
            basicvalue.type = BasicValueTypeString;
            basicvalue.value.as_string = strdup(lua_tostring(L, 4));
            break;
        case LUA_TNUMBER:
            basicvalue.type = BasicValueTypeDouble;
            basicvalue.value.as_double = lua_tonumber(L, 4);
            break;
        default:
            return luaL_error(L, "invalid arg value type, expected: string, nil, number or boolean");
    }

    BasicValue* value = malloc(sizeof(BasicValue));
    memcpy(value, &basicvalue, sizeof(BasicValue));

    BasicValue* ret = modding_spawn_screen(modding, layout_src, script_src, value);

    switch (ret->type) {
        case LUA_TBOOLEAN:
            lua_pushboolean(L, ret->value.as_boolean);
            break;
        case LUA_TSTRING:
            lua_pushstring(L, ret->value.as_string);
            free(ret->value.as_string);
        case LUA_TNUMBER:
            lua_pushnumber(L, ret->value.as_double);
            break;
        default:
            lua_pushnil(L);
    }
    free(ret);

    return 1;
}

static int script_modding_set_exit_delay(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    Modding modding = (Modding)luascript->context;

    float delay_ms = (float)luaL_checknumber(L, 1);

    modding_set_exit_delay(modding, delay_ms);

    return 0;
}

static int script_modding_get_messagebox(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    Modding modding = (Modding)luascript->context;

    MessageBox ret = modding_get_messagebox(modding);

    return script_messagebox_new(L, ret);
}

static int script_modding_set_menu_in_layout_placeholder(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    Modding modding = (Modding)luascript->context;

    const char* placeholder_name = lua_tostring(L, 1);
    Menu menu = luascript_read_nullable_userdata(L, 2, MENU);

    modding_set_menu_in_layout_placeholder(modding, placeholder_name, menu);

    return 0;
}




static const luaL_Reg MODDING_FUNCTIONS[] = {
    { "modding_set_ui_visibility", script_modding_set_ui_visibility },
    { "modding_get_layout", script_modding_get_layout },
    { "modding_exit", script_modding_exit },
    { "modding_set_halt", script_modding_set_halt },
    { "modding_unlockdirective_create", script_modding_unlockdirective_create },
    { "modding_unlockdirective_remove", script_modding_unlockdirective_remove },
    { "modding_unlockdirective_get", script_modding_unlockdirective_get },
    { "modding_get_active_menu", script_modding_get_active_menu },
    { "modding_choose_native_menu_option", script_modding_choose_native_menu_option },
    { "modding_get_native_menu", script_modding_get_native_menu },
    { "modding_set_active_menu", script_modding_set_active_menu },
    { "modding_get_native_background_music", script_modding_get_native_background_music },
    { "modding_replace_native_background_music", script_modding_replace_native_background_music },
    { "modding_spawn_screen", script_modding_spawn_screen },
    { "modding_set_exit_delay", script_modding_set_exit_delay },
    { "modding_get_messagebox", script_modding_get_messagebox },
    { "modding_set_menu_in_layout_placeholder", script_modding_set_menu_in_layout_placeholder },
    { NULL, NULL }
};



void script_modding_register(lua_State* L) {
    for (size_t i = 0;; i++) {
        if (MODDING_FUNCTIONS[i].name == NULL || MODDING_FUNCTIONS[i].func == NULL) break;
        lua_pushcfunction(L, MODDING_FUNCTIONS[i].func);
        lua_setglobal(L, MODDING_FUNCTIONS[i].name);
    }
}
