#include "luascript_internal.h"

#include "atlas.h"
#include "logger.h"


int script_atlas_push_atlas_entry(lua_State* L, const AtlasEntry* entry) {
    if (!entry) {
        lua_pushnil(L);
        return 1;
    }

    lua_createtable(L, 0, 11);

    luascript_helper_add_table_field(L, "name", lua_pushstring, entry->name);
    luascript_helper_add_table_field(L, "x", lua_pushnumber, (lua_Number)entry->x);
    luascript_helper_add_table_field(L, "y", lua_pushnumber, (lua_Number)entry->y);
    luascript_helper_add_table_field(L, "width", lua_pushnumber, (lua_Number)entry->width);
    luascript_helper_add_table_field(L, "height", lua_pushnumber, (lua_Number)entry->height);
    luascript_helper_add_table_field(L, "frameX", lua_pushnumber, (lua_Number)entry->frame_x);
    luascript_helper_add_table_field(L, "frameY", lua_pushnumber, (lua_Number)entry->frame_y);
    luascript_helper_add_table_field(L, "frameWidth", lua_pushnumber, (lua_Number)entry->frame_width);
    luascript_helper_add_table_field(L, "frameHeight", lua_pushnumber, (lua_Number)entry->frame_height);
    luascript_helper_add_table_field(L, "pivotX", lua_pushnumber, (lua_Number)entry->pivot_x);
    luascript_helper_add_table_field(L, "pivotY", lua_pushnumber, (lua_Number)entry->pivot_y);

    return 1;
}


static int script_atlas_init(lua_State* L) {
    const char* src = luaL_checkstring(L, 2);

    luascript_change_working_folder(L);
    Atlas ret = atlas_init(src);
    luascript_restore_working_folder(L);

    return luascript_userdata_allocnew(L, ATLAS, ret);
}

static int script_atlas_destroy(lua_State* L) {
    Atlas atlas = luascript_read_userdata(L, ATLAS);

    if (luascript_userdata_is_allocated(L, ATLAS))
        atlas_destroy(&atlas);
    else
        logger_warn("script_atlas_destroy() object was not allocated by lua");

    return 0;
}

static int script_atlas_get_index_of(lua_State* L) {
    Atlas atlas = luascript_read_userdata(L, ATLAS);
    const char* name = luaL_optstring(L, 2, NULL);

    int32_t ret = atlas_get_index_of(atlas, name);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_atlas_get_entry(lua_State* L) {
    Atlas atlas = luascript_read_userdata(L, ATLAS);
    const char* name = luaL_optstring(L, 2, NULL);

    const AtlasEntry* ret = atlas_get_entry(atlas, name);

    return script_atlas_push_atlas_entry(L, ret);
}

static int script_atlas_get_entry_with_number_suffix(lua_State* L) {
    Atlas atlas = luascript_read_userdata(L, ATLAS);
    const char* name_prefix = luaL_optstring(L, 2, NULL);

    const AtlasEntry* ret = atlas_get_entry_with_number_suffix(atlas, name_prefix);

    return script_atlas_push_atlas_entry(L, ret);
}

static int script_atlas_get_glyph_fps(lua_State* L) {
    Atlas atlas = luascript_read_userdata(L, ATLAS);

    float ret = atlas_get_glyph_fps(atlas);

    lua_pushnumber(L, (lua_Number)ret);
    return 1;
}

static int script_atlas_get_texture_resolution(lua_State* L) {
    Atlas atlas = luascript_read_userdata(L, ATLAS);
    int32_t resolution_width, resolution_height;

    atlas_get_texture_resolution(atlas, &resolution_width, &resolution_height);

    lua_pushinteger(L, (lua_Integer)resolution_width);
    lua_pushinteger(L, (lua_Integer)resolution_height);
    return 2;
}

static int script_atlas_utils_is_known_extension(lua_State* L) {
    const char* src = luaL_checkstring(L, 1);

    bool ret = atlas_utils_is_known_extension(src);

    lua_pushboolean(L, ret);
    return 1;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const luaL_Reg ATLAS_FUNCTIONS[] = {
    {"init", script_atlas_init},
    {"destroy", script_atlas_destroy},
    {"get_index_of", script_atlas_get_index_of},
    {"get_entry", script_atlas_get_entry},
    {"get_entry_with_number_suffix", script_atlas_get_entry_with_number_suffix},
    {"get_glyph_fps", script_atlas_get_glyph_fps},
    {"get_texture_resolution", script_atlas_get_texture_resolution},
    {"utils_is_known_extension", script_atlas_utils_is_known_extension},
    {NULL, NULL}
};


int script_atlas_new(lua_State* L, Atlas atlas) {
    return luascript_userdata_new(L, ATLAS, atlas);
}

static int script_atlas_gc(lua_State* L) {
    return luascript_userdata_destroy(L, ATLAS, (Destructor)atlas_destroy);
}

static int script_atlas_tostring(lua_State* L) {
    return luascript_userdata_tostring(L, ATLAS);
}

void script_atlas_register(lua_State* L) {
    luascript_register(L, ATLAS, script_atlas_gc, script_atlas_tostring, ATLAS_FUNCTIONS);
}
