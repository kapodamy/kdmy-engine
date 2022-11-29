#include "luascript_internal.h"

#include "string.h"

static double buffer[16];


#ifdef JAVASCRIPT
EM_JS_PRFX(PSShader, psshader_init, (const char* vertex_sourcecode, const char* fragment_sourcecode), {
    let psshader = PostprocessingShader.BuildFromSource(
        pvr_context,
        kdmyEngine_ptrToString(vertex_sourcecode),
        kdmyEngine_ptrToString(fragment_sourcecode)
    );
    return kdmyEngine_obtain(psshader);
    });

EM_JS_PRFX(void, psshader_destroy, (PSShader psshader), {
    kdmyEngine_obtain(psshader).Destroy();
    kdmyEngine_forget(psshader);
    });

EM_JS_PRFX(int32_t, psshader_set_uniform_any, (PSShader psshader, const char* name, const double* values), {
    const val = new Float32Array(buffer, values, 128);
    kdmyEngine_obtain(psshader).SetUniformAny(kdmyEngine_ptrToString(name), val);
    });

EM_JS_PRFX(bool, psshader_set_uniform1f, (PSShader psshader, const char* name, float value), {
    kdmyEngine_obtain(psshader).SetUniform1F(kdmyEngine_ptrToString(name), value);
    });

EM_JS_PRFX(bool, psshader_set_uniform1i, (PSShader psshader, const char* name, int32_t value), {
    kdmyEngine_obtain(psshader).SetUniform1I(kdmyEngine_ptrToString(name), value);
    });
#endif



static int script_psshader_init(lua_State* L) {
    const char* vertex_sourcecode = luaL_optstring(L, 1, NULL);
    const char* fragment_sourcecode = luaL_optstring(L, 2, NULL);

    PSShader psshader = psshader_init(vertex_sourcecode, fragment_sourcecode);

    if (!psshader) {
        lua_pushnil(L);
        return 1;
    }

    return NEW_USERDATA(L, PSSHADER, NULL, psshader, false);
}

static int script_psshader_destroy(lua_State* L) {
    READ_USERDATA(L, PSShader, psshader, PSSHADER);

    if (_luascript_has_item(L, psshader, true)) {
        return luaL_error(L, "this object was not allocated by lua");
    }

    _luascript_suppress_item(L, psshader, false);
    psshader_destroy(psshader);
    NULLIFY_USERDATA(L);

    return 0;
}

static int script_psshader_set_uniform_any(lua_State* L) {
    READ_USERDATA(L, PSShader, psshader, PSSHADER);

    const char* name = luaL_checkstring(L, 2);
    int values_count = lua_gettop(L) - 2;

    memset(buffer, 0x00, sizeof(buffer));
    for (int i = 0;i < values_count;i++) {
        if (i > 16) break;
        buffer[i] = luaL_checknumber(L, i + 3);
    }

    int32_t ret = psshader_set_uniform_any(psshader, name, buffer);

    lua_pushinteger(L, ret);

    return 1;
}

static int script_psshader_set_uniform1f(lua_State* L) {
    READ_USERDATA(L, PSShader, psshader, PSSHADER);

    const char* name = luaL_checkstring(L, 2);
    float value = (float)luaL_checknumber(L, 3);

    bool ret = psshader_set_uniform1f(psshader, name, value);

    lua_pushboolean(L, ret);

    return 1;
}

static int script_psshader_set_uniform1i(lua_State* L) {
    READ_USERDATA(L, PSShader, psshader, PSSHADER);

    const char* name = luaL_checkstring(L, 2);
    int32_t value = (int32_t)luaL_checkinteger(L, 3);

    bool ret = psshader_set_uniform1i(psshader, name, value);

    lua_pushboolean(L, ret);

    return 1;
}




////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const luaL_Reg PSSHADER_FUNCTIONS[] = {
    { "destroy", script_psshader_destroy },
    { "set_uniform_any", script_psshader_set_uniform_any },
    { "set_uniform1f", script_psshader_set_uniform1f },
    { "set_uniform1i", script_psshader_set_uniform1i },
    {NULL, NULL}
};


int script_psshader_new(lua_State* L, void* vertex, PSShader psshader) {
    bool is_shared = _luascript_has_item(L, psshader, true);
    return NEW_USERDATA(L, PSSHADER, vertex, psshader, is_shared);
}

static int script_psshader_gc(lua_State* L) {
    READ_USERDATA_UNCHECKED(L, PSShader, psshader, PSSHADER);

    bool is_allocated = _luascript_has_item(L, psshader, false);

    if (is_allocated) {
        NULLIFY_USERDATA(L);
        psshader_destroy(psshader);
    }

    _luascript_suppress_item(L, psshader, !is_allocated);
    return 0;
}

static int script_psshader_tostring(lua_State* L) {
    READ_USERDATA(L, PSShader, psshader, PSSHADER);
    lua_pushstring(L, "[PSShader]");
    return 1;
}


inline void register_psshader(lua_State* L) {
    // register constructor
    lua_pushcfunction(L, script_psshader_init);
    lua_setglobal(L, "engine_create_shader");

    _luascript_register(
        L,
        PSSHADER,
        script_psshader_gc,
        script_psshader_tostring,
        PSSHADER_FUNCTIONS
    );
}

