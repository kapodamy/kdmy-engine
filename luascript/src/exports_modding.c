#include "luascript_internal.h"

#include "modding.h"

#ifdef JAVASCRIPT

EM_JS_PRFX(Layout, modding_get_layout, (ModdingContext moddingcontext), {
    let layout = modding_get_layout(kdmyEngine_obtain(moddingcontext));
    return kdmyEngine_obtain(layout);
    });
EM_JS_PRFX(void, modding_exit, (ModdingContext moddingcontext), {
    modding_exit(kdmyEngine_obtain(moddingcontext));
    });
#endif

static int script_modding_get_layout(lua_State* L) {
    LUASCRIPT_GET(L);

    Layout layout = modding_get_layout(luascript->context);

    return script_layout_new(L, layout);
}

static int script_modding_exit(lua_State* L) {
    LUASCRIPT_GET(L);

    modding_exit(luascript->context);

    return 0;
}


static const luaL_Reg EXPORTS_FUNCTION[] = {
    { "modding_get_layout", script_modding_get_layout },
    { "modding_exit", script_modding_exit },
    { NULL, NULL }
};


void register_modding(lua_State* L) {
    for (size_t i = 0; ; i++) {
        if (EXPORTS_FUNCTION[i].name == NULL || EXPORTS_FUNCTION[i].func == NULL) break;
        lua_pushcfunction(L, EXPORTS_FUNCTION[i].func);
        lua_setglobal(L, EXPORTS_FUNCTION[i].name);
    }
}
