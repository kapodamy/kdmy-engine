#include "luascript_internal.h"

#ifdef JAVASCRIPT
EM_ASYNC_JS_PRFX(void*, json_load_from, (const char* src), {
    let ret = await json_load_from(kdmyEngine_ptrToString(src));
    return ModuleLuaScript.kdmyEngine_obtain(ret);
});
EM_JS_PRFX(void*, json_load_from_string, (const char* json_sourcecode), {
    let ret = json_load_from_string(kdmyEngine_ptrToString(json_sourcecode));
    return ModuleLuaScript.kdmyEngine_obtain(ret);
});
EM_JS_PRFX(int, kdmyEngine_parse_json, (lua_State* L, void* json), {
    return luascript_helper_parse_json(L, ModuleLuaScript.kdmyEngine_obtain(json));
});
#endif

static int script_json_parse_from_file(lua_State* L) {
    const char* src = luaL_checkstring(L, 2);

#ifdef JAVASCRIPT
    void* json = json_load_from(src);
    int ret = kdmyEngine_parse_json(L, json);
    kdmy_forget_obtained(json);
    return ret;
#else
    #warning "json parsing directly from C is not implemented"
    (void)src;
    lua_pushnil(L);
    return 1;
#endif
}

static int script_json_parse(lua_State* L) {
    const char* json_sourcecode = luaL_checkstring(L, 2);

#ifdef JAVASCRIPT
    void* json = json_load_from_string(json_sourcecode);
    int ret = kdmyEngine_parse_json(L, json);
    kdmy_forget_obtained(json);
    return ret;
#else
    #warning "json parsing directly from C is not implemented"
    (void)json_sourcecode;
    lua_pushnil(L);
    return 1;
#endif
}


static const luaL_Reg JSON_FUNCTIONS[] = {
    {"parse_from_file", script_json_parse_from_file},
    {"parse", script_json_parse},
    {NULL, NULL}};

static int script_json_gc(lua_State* L) {
    // nothing to do
    (void)L;
    return 0;
}

static int script_json_tostring(lua_State* L) {
    lua_pushstring(L, "{JSON}");
    return 1;
}


void script_json_register(lua_State* L) {
    luascript_register(L, JSON, script_json_gc, script_json_tostring, JSON_FUNCTIONS);
}
