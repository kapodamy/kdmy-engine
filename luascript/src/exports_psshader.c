#include "luascript_internal.h"

#include "string.h"

static double buffer[16];


#ifdef JAVASCRIPT
EM_JS_PRFX(PSShader, psshader_init, (const char* vertex_sourcecode, const char* fragment_sourcecode), {
    let psshader = PSShader.BuildFromSource(
        pvr_context,
        kdmyEngine_ptrToString(vertex_sourcecode),
        kdmyEngine_ptrToString(fragment_sourcecode)
    );
    return kdmyEngine_obtain(psshader);
});

EM_JS_PRFX(void, psshader_destroy, (PSShader* psshader), {
    kdmyEngine_obtain(kdmyEngine_get_uint32(psshader)).Destroy();
});

EM_JS_PRFX(int32_t, psshader_set_uniform_any, (PSShader psshader, const char* name, const double* values), {
    const val = new Float32Array(wasmMemory.buffer, values, 128);
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
    const char* vertex_sourcecode = luaL_optstring(L, 2, NULL);
    const char* fragment_sourcecode = luaL_optstring(L, 3, NULL);

    PSShader psshader = psshader_init(vertex_sourcecode, fragment_sourcecode);

    if (!psshader) {
        lua_pushnil(L);
        return 1;
    }

    return luascript_userdata_allocnew(L, PSSHADER, psshader);
}

static int script_psshader_destroy(lua_State* L) {
    PSShader psshader = luascript_read_userdata(L, PSSHADER);

    if (luascript_userdata_is_allocated(L, PSSHADER))
        psshader_destroy(&psshader);
    else
        printf("script_psshader_destroy() object was not allocated by lua\n");

    return 0;
}

static int script_psshader_set_uniform_any(lua_State* L) {
    PSShader psshader = luascript_read_userdata(L, PSSHADER);

    const char* name = luaL_checkstring(L, 2);
    int values_count = lua_gettop(L) - 2;

    memset(buffer, 0x00, sizeof(buffer));
    for (int i = 0; i < values_count; i++) {
        if (i > 16) break;
        buffer[i] = luaL_checknumber(L, i + 3);
    }

    int32_t ret = psshader_set_uniform_any(psshader, name, buffer);

    lua_pushinteger(L, ret);

    return 1;
}

static int script_psshader_set_uniform1f(lua_State* L) {
    PSShader psshader = luascript_read_userdata(L, PSSHADER);

    const char* name = luaL_checkstring(L, 2);
    float value = (float)luaL_checknumber(L, 3);

    bool ret = psshader_set_uniform1f(psshader, name, value);

    lua_pushboolean(L, ret);

    return 1;
}

static int script_psshader_set_uniform1i(lua_State* L) {
    PSShader psshader = luascript_read_userdata(L, PSSHADER);

    const char* name = luaL_checkstring(L, 2);
    int32_t value = (int32_t)luaL_checkinteger(L, 3);

    bool ret = psshader_set_uniform1i(psshader, name, value);

    lua_pushboolean(L, ret);

    return 1;
}




////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const luaL_Reg PSSHADER_FUNCTIONS[] = {
    {"init", script_psshader_init},
    {"destroy", script_psshader_destroy},
    {"set_uniform_any", script_psshader_set_uniform_any},
    {"set_uniform1f", script_psshader_set_uniform1f},
    {"set_uniform1i", script_psshader_set_uniform1i},
    {NULL, NULL}
};


int script_psshader_new(lua_State* L, PSShader psshader) {
    return luascript_userdata_new(L, PSSHADER, psshader);
}

static int script_psshader_gc(lua_State* L) {
    // if this object was allocated by lua, call the destructor
    return luascript_userdata_destroy(L, PSSHADER, (Destructor)psshader_destroy);
}

static int script_psshader_tostring(lua_State* L) {
    return luascript_userdata_tostring(L, PSSHADER);
}


void script_psshader_register(lua_State* L) {
    luascript_register(L, PSSHADER, script_psshader_gc, script_psshader_tostring, PSSHADER_FUNCTIONS);
}

