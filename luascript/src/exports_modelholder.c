#include "luascript_internal.h"

#ifdef JAVASCRIPT
EM_ASYNC_JS_PRFX(ModelHolder, modelholder_init, (const char* src), {
    let ret = await modelholder_init(kdmyEngine_ptrToString(src));
    return kdmyEngine_obtain(ret);
});
EM_ASYNC_JS_PRFX(ModelHolder, modelholder_init2, (uint32_t vertex_color_rgb8, const char* atlas_src, const char* animlist_src), {
    let ret = await modelholder_init2(vertex_color_rgb8, kdmyEngine_ptrToString(atlas_src), kdmyEngine_ptrToString(animlist_src));
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(void, modelholder_destroy, (ModelHolder* modelholder), {
    modelholder_destroy(kdmyEngine_obtain(modelholder));
});
EM_JS_PRFX(bool, modelholder_is_invalid, (ModelHolder modelholder), {
    let ret = modelholder_is_invalid(kdmyEngine_obtain(modelholder));
    return ret ? 1 : 0;
});
EM_JS_PRFX(bool, modelholder_has_animlist, (ModelHolder modelholder), {
    let ret = modelholder_has_animlist(kdmyEngine_obtain(modelholder));
    return ret ? 1 : 0;
});
EM_JS_PRFX(AnimSprite, modelholder_create_animsprite, (ModelHolder modelholder, const char* animation_name, bool fallback_static, bool no_return_null), {
    let ret = modelholder_create_animsprite(kdmyEngine_obtain(modelholder), kdmyEngine_ptrToString(animation_name), fallback_static, no_return_null);
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(Atlas, modelholder_get_atlas, (ModelHolder modelholder), {
    let ret = modelholder_get_atlas(kdmyEngine_obtain(modelholder));
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(uint32_t, modelholder_get_vertex_color, (ModelHolder modelholder), {
    let ret = modelholder_get_vertex_color(kdmyEngine_obtain(modelholder));
    return ret;
});
EM_JS_PRFX(AnimList, modelholder_get_animlist, (ModelHolder modelholder), {
    let ret = modelholder_get_animlist(kdmyEngine_obtain(modelholder));
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(AtlasEntry, modelholder_get_atlas_entry, (ModelHolder modelholder, const char* atlas_entry_name, bool return_copy), {
    let ret = modelholder_get_atlas_entry(kdmyEngine_obtain(modelholder), kdmyEngine_ptrToString(atlas_entry_name), return_copy);
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(AtlasEntry, modelholder_get_atlas_entry2, (ModelHolder modelholder, const char* atlas_entry_name, bool return_copy), {
    let ret = modelholder_get_atlas_entry2(kdmyEngine_obtain(modelholder), kdmyEngine_ptrToString(atlas_entry_name), return_copy);
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(void, modelholder_get_texture_resolution, (ModelHolder modelholder, int32_t* resolution_width, int32_t* resolution_height), {
    const values = [0, 0];
    modelholder_get_texture_resolution(kdmyEngine_obtain(modelholder), values);
    kdmyEngine_set_int32(resolution_width, values[0]);
    kdmyEngine_set_int32(resolution_height, values[1]);
});
EM_JS_PRFX(bool, modelholder_utils_is_known_extension, (const char* filename), {
    let ret = modelholder_utils_is_known_extension(kdmyEngine_ptrToString(filename));
    return ret ? 1 : 0;
});
#endif


static int script_modelholder_init(lua_State* L) {
    const char* src = luaL_checkstring(L, 2);

    ModelHolder ret = modelholder_init(src);

    return luascript_userdata_allocnew(L, MODELHOLDER, ret);
}

static int script_modelholder_init2(lua_State* L) {
    uint32_t vertex_color_rgb8 = (uint32_t)luaL_checkinteger(L, 2);
    const char* atlas_src = luaL_optstring(L, 3, NULL);
    const char* animlist_src = luaL_optstring(L, 4, NULL);

    ModelHolder ret = modelholder_init2(vertex_color_rgb8, atlas_src, animlist_src);

    return luascript_userdata_allocnew(L, MODELHOLDER, ret);
}

static int script_modelholder_destroy(lua_State* L) {
    ModelHolder modelholder = luascript_read_userdata(L, MODELHOLDER);

    if (luascript_userdata_is_allocated(L, MODELHOLDER))
        modelholder_destroy(&modelholder);
    else
        printf("script_modelholder_destroy() object was not allocated by lua\n");

    return 0;
}

static int script_modelholder_is_invalid(lua_State* L) {
    ModelHolder modelholder = luascript_read_userdata(L, MODELHOLDER);

    bool ret = modelholder_is_invalid(modelholder);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_modelholder_has_animlist(lua_State* L) {
    ModelHolder modelholder = luascript_read_userdata(L, MODELHOLDER);

    bool ret = modelholder_has_animlist(modelholder);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_modelholder_create_animsprite(lua_State* L) {
    ModelHolder modelholder = luascript_read_userdata(L, MODELHOLDER);
    const char* animation_name = luaL_optstring(L, 2, NULL);
    bool fallback_ = (bool)lua_toboolean(L, 3);
    bool no_return_null = (bool)lua_toboolean(L, 4);

    AnimSprite ret = modelholder_create_animsprite(modelholder, animation_name, fallback_, no_return_null);

    return luascript_userdata_allocnew(L, ANIMSPRITE, ret);
}

static int script_modelholder_get_atlas(lua_State* L) {
    ModelHolder modelholder = luascript_read_userdata(L, MODELHOLDER);

    Atlas ret = modelholder_get_atlas(modelholder);

    return script_atlas_new(L, ret);
}

static int script_modelholder_get_vertex_color(lua_State* L) {
    ModelHolder modelholder = luascript_read_userdata(L, MODELHOLDER);

    uint32_t ret = modelholder_get_vertex_color(modelholder);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_modelholder_get_animlist(lua_State* L) {
    ModelHolder modelholder = luascript_read_userdata(L, MODELHOLDER);

    AnimList ret = modelholder_get_animlist(modelholder);

    return script_animlist_new(L, ret);
}

static int script_modelholder_get_atlas_entry(lua_State* L) {
    ModelHolder modelholder = luascript_read_userdata(L, MODELHOLDER);
    const char* atlas_entry_name = luaL_optstring(L, 2, NULL);

    AtlasEntry ret = modelholder_get_atlas_entry(modelholder, atlas_entry_name, false);

    return script_atlas_push_atlas_entry(L, ret);
}

static int script_modelholder_get_atlas_entry2(lua_State* L) {
    ModelHolder modelholder = luascript_read_userdata(L, MODELHOLDER);
    const char* atlas_entry_name = luaL_optstring(L, 2, NULL);

    AtlasEntry ret = modelholder_get_atlas_entry2(modelholder, atlas_entry_name, false);

    return script_atlas_push_atlas_entry(L, ret);
}

static int script_modelholder_get_texture_resolution(lua_State* L) {
    ModelHolder modelholder = luascript_read_userdata(L, MODELHOLDER);

    int32_t resolution_width, resolution_height;
    modelholder_get_texture_resolution(modelholder, &resolution_width, &resolution_height);

    lua_pushinteger(L, resolution_width);
    lua_pushinteger(L, resolution_height);
    return 2;
}

static int script_modelholder_utils_is_known_extension(lua_State* L) {
    const char* filename = luaL_checkstring(L, 2);

    bool ret = modelholder_utils_is_known_extension(filename);

    lua_pushboolean(L, ret);
    return 1;
}




static const luaL_Reg MODELHOLDER_FUNCTIONS[] = {
    { "init", script_modelholder_init },
    { "init2", script_modelholder_init2 },
    { "destroy", script_modelholder_destroy },
    { "is_invalid", script_modelholder_is_invalid },
    { "has_animlist", script_modelholder_has_animlist },
    { "create_animsprite", script_modelholder_create_animsprite },
    { "get_atlas", script_modelholder_get_atlas },
    { "get_vertex_color", script_modelholder_get_vertex_color },
    { "get_animlist", script_modelholder_get_animlist },
    { "get_atlas_entry", script_modelholder_get_atlas_entry },
    { "get_atlas_entry2", script_modelholder_get_atlas_entry2 },
    { "get_texture_resolution", script_modelholder_get_texture_resolution },
    { "utils_is_known_extension", script_modelholder_utils_is_known_extension },
    { NULL, NULL }
};

int script_modelholder_new(lua_State* L, ModelHolder modelholder) {
    return luascript_userdata_new(L, MODELHOLDER, modelholder);
}

static int script_modelholder_gc(lua_State* L) {
    return luascript_userdata_destroy(L, MODELHOLDER, (Destructor)modelholder_destroy);
}

static int script_modelholder_tostring(lua_State* L) {
    return luascript_userdata_tostring(L, MODELHOLDER);
}

void script_modelholder_register(lua_State* L) {
    luascript_register(L, MODELHOLDER, script_modelholder_gc, script_modelholder_tostring, MODELHOLDER_FUNCTIONS);
}

