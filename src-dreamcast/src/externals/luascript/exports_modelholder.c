#include "luascript_internal.h"

#include "logger.h"
#include "modelholder.h"


static int script_modelholder_init(lua_State* L) {
    const char* src = luaL_checkstring(L, 2);

    luascript_change_working_folder(L);
    ModelHolder ret = modelholder_init(src);
    luascript_restore_working_folder(L);

    return luascript_userdata_allocnew(L, MODELHOLDER, ret);
}

static int script_modelholder_init2(lua_State* L) {
    uint32_t vertex_color_rgb8 = (uint32_t)luaL_checkinteger(L, 2);
    const char* atlas_src = luaL_optstring(L, 3, NULL);
    const char* animlist_src = luaL_optstring(L, 4, NULL);

    luascript_change_working_folder(L);
    ModelHolder ret = modelholder_init2(vertex_color_rgb8, atlas_src, animlist_src);
    luascript_restore_working_folder(L);

    return luascript_userdata_allocnew(L, MODELHOLDER, ret);
}

static int script_modelholder_destroy(lua_State* L) {
    ModelHolder modelholder = luascript_read_userdata(L, MODELHOLDER);

    if (luascript_userdata_is_allocated(L, MODELHOLDER))
        modelholder_destroy(&modelholder);
    else
        logger_warn("script_modelholder_destroy() object was not allocated by lua");

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
    bool fallback_static = (bool)lua_toboolean(L, 3);
    bool no_return_null = (bool)lua_toboolean(L, 4);

    AnimSprite ret = modelholder_create_animsprite(modelholder, animation_name, fallback_static, no_return_null);

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

    const AtlasEntry* ret = modelholder_get_atlas_entry(modelholder, atlas_entry_name);

    return script_atlas_push_atlas_entry(L, ret);
}

static int script_modelholder_get_texture_resolution(lua_State* L) {
    ModelHolder modelholder = luascript_read_userdata(L, MODELHOLDER);

    int32_t resolution_width, resolution_height;
    modelholder_get_texture_resolution(modelholder, &resolution_width, &resolution_height);

    lua_pushinteger(L, (lua_Integer)resolution_width);
    lua_pushinteger(L, (lua_Integer)resolution_height);
    return 2;
}

static int script_modelholder_utils_is_known_extension(lua_State* L) {
    const char* filename = luaL_checkstring(L, 2);

    bool ret = modelholder_utils_is_known_extension(filename);

    lua_pushboolean(L, ret);
    return 1;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const luaL_Reg MODELHOLDER_FUNCTIONS[] = {
    {"init", script_modelholder_init},
    {"init2", script_modelholder_init2},
    {"destroy", script_modelholder_destroy},
    {"is_invalid", script_modelholder_is_invalid},
    {"has_animlist", script_modelholder_has_animlist},
    {"create_animsprite", script_modelholder_create_animsprite},
    {"get_atlas", script_modelholder_get_atlas},
    {"get_vertex_color", script_modelholder_get_vertex_color},
    {"get_animlist", script_modelholder_get_animlist},
    {"get_atlas_entry", script_modelholder_get_atlas_entry},
    {"get_texture_resolution", script_modelholder_get_texture_resolution},
    {"utils_is_known_extension", script_modelholder_utils_is_known_extension},
    {NULL, NULL}
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
