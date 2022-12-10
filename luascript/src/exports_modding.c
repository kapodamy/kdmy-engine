#include "luascript_internal.h"

#ifdef JAVASCRIPT
EM_JS_PRFX(void, modding_set_ui_visibility, (ModdingContext moddingcontext, bool visible), {
    modding_set_ui_visibility(kdmyEngine_obtain(moddingcontext), visible);
});
EM_JS_PRFX(Layout, modding_get_layout, (ModdingContext moddingcontext), {
    let ret = modding_get_layout(kdmyEngine_obtain(moddingcontext));
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(void, modding_exit, (ModdingContext moddingcontext), {
    modding_exit(kdmyEngine_obtain(moddingcontext));
});
EM_JS_PRFX(void, modding_set_halt, (ModdingContext moddingcontext, bool halt), {
    modding_set_halt(kdmyEngine_obtain(moddingcontext), halt);
});
EM_JS_PRFX(void, modding_unlockdirective_create, (ModdingContext moddingcontext, const char* name, double value), {
    modding_unlockdirective_create(kdmyEngine_obtain(moddingcontext), kdmyEngine_ptrToString(name), value);
});
EM_JS_PRFX(double, modding_unlockdirective_get, (ModdingContext moddingcontext, const char* name), {
    let ret = modding_unlockdirective_get(kdmyEngine_obtain(moddingcontext), kdmyEngine_ptrToString(name));
    return ret;
});
EM_JS_PRFX(void, modding_unlockdirective_remove, (ModdingContext moddingcontext, const char* name), {
    modding_unlockdirective_remove(kdmyEngine_obtain(moddingcontext), kdmyEngine_ptrToString(name));
});
EM_JS_PRFX(bool, modding_unlockdirective_has, (ModdingContext moddingcontext, const char* name), {
    let ret = modding_unlockdirective_has(kdmyEngine_obtain(moddingcontext), kdmyEngine_ptrToString(name));
    return ret ? 1 : 0;
});
#endif


static int script_modding_unlockdirective_create(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    ModdingContext moddingcontext = (ModdingContext)luascript->context;
    
    const char* name = luaL_optstring(L, 1, NULL);
    double value = (double)luaL_checknumber(L, 2);

    modding_unlockdirective_create(moddingcontext, name, value);

    return 0;
}

static int script_modding_unlockdirective_remove(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    ModdingContext moddingcontext = (ModdingContext)luascript->context;
    
    const char* name = luaL_optstring(L, 1, NULL);

    modding_unlockdirective_remove(moddingcontext, name);

    return 0;
}

static int script_modding_unlockdirective_get(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    ModdingContext moddingcontext = (ModdingContext)luascript->context;
    
    const char* name = luaL_optstring(L, 1, NULL);

    if(!modding_unlockdirective_has(moddingcontext, name)) {
        lua_pushnil(L);
        return 1;
    }

    double ret = modding_unlockdirective_get(moddingcontext, name);

    lua_pushnumber(L, ret);
    return 1;
}

static int script_modding_set_ui_visibility(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    ModdingContext moddingcontext = (ModdingContext)luascript->context;
    
    bool visible = (bool)lua_toboolean(L, 1);

    modding_set_ui_visibility(moddingcontext, visible);

    return 0;
}

static int script_modding_get_layout(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    ModdingContext moddingcontext = (ModdingContext)luascript->context;


    Layout ret = modding_get_layout(moddingcontext);

    return script_layout_new(L, ret);
}

static int script_modding_exit(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    ModdingContext moddingcontext = (ModdingContext)luascript->context;


    modding_exit(moddingcontext);

    return 0;
}

static int script_modding_set_halt(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    ModdingContext moddingcontext = (ModdingContext)luascript->context;
    
    bool halt = (bool)lua_toboolean(L, 1);

    modding_set_halt(moddingcontext, halt);

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
    { NULL, NULL }
};



void script_modding_register(lua_State* L) {
    for (size_t i = 0; ; i++) {
        if (MODDING_FUNCTIONS[i].name == NULL || MODDING_FUNCTIONS[i].func == NULL) break;
        lua_pushcfunction(L, MODDING_FUNCTIONS[i].func);
        lua_setglobal(L, MODDING_FUNCTIONS[i].name);
    }
}
