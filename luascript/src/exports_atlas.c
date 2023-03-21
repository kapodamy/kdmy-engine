#include "luascript_internal.h"

#ifdef JAVASCRIPT
EM_ASYNC_JS_PRFX(Atlas, atlas_init, (const char* src), {
    let ret = await atlas_init(kdmyEngine_ptrToString(src));
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(void, atlas_destroy, (Atlas* atlas), {
    atlas_destroy(kdmyEngine_obtain(kdmyEngine_get_uint32(atlas)));
});
EM_JS_PRFX(int32_t, atlas_get_index_of, (Atlas atlas, const char* name), {
    let ret = atlas_get_index_of(kdmyEngine_obtain(atlas), kdmyEngine_ptrToString(name));
    return ret;
});
EM_JS_PRFX(AtlasEntry, atlas_get_entry, (Atlas atlas, const char* name), {
    let ret = atlas_get_entry(kdmyEngine_obtain(atlas), kdmyEngine_ptrToString(name));
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(AtlasEntry, atlas_get_entry_with_number_suffix, (Atlas atlas, const char* name_prefix), {
    let ret = atlas_get_entry_with_number_suffix(kdmyEngine_obtain(atlas), kdmyEngine_ptrToString(name_prefix));
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(float, atlas_get_glyph_fps, (Atlas atlas), {
    let ret = atlas_get_glyph_fps(kdmyEngine_obtain(atlas));
    return ret;
});
EM_JS_PRFX(void, atlas_get_texture_resolution, (Atlas atlas, int32_t* resolution_width, int32_t* resolution_height), {
    const values = [0, 0];
    atlas_get_texture_resolution(kdmyEngine_obtain(atlas), values);
    kdmyEngine_set_int32(resolution_width, values[0]);
    kdmyEngine_set_int32(resolution_height, values[1]);
});
EM_JS_PRFX(bool, atlas_utils_is_known_extension, (const char* src), {
    let ret = atlas_utils_is_known_extension(kdmyEngine_ptrToString(src));
    return ret ? 1 : 0;
});
#endif


int script_atlas_push_atlas_entry(lua_State* L, AtlasEntry entry) {
    if(!entry) {
        lua_pushnil(L);
        return 1;
    }
    
#ifdef JAVASCRIPT
    AtlasEntry_t _entry = {
        .name = kdmy_read_prop_string(entry, "name"),
        .x = kdmy_read_prop_float(entry, "x"),
        .y = kdmy_read_prop_float(entry, "y"),
        .width = kdmy_read_prop_float(entry, "width"),
        .height = kdmy_read_prop_float(entry, "height"),
        .frame_x = kdmy_read_prop_float(entry, "frame_x"),
        .frame_y = kdmy_read_prop_float(entry, "frame_y"),
        .frame_width = kdmy_read_prop_float(entry, "frame_width"),
        .frame_height = kdmy_read_prop_float(entry, "frame_height"),
        .pivot_x = kdmy_read_prop_float(entry, "pivot_x"),
        .pivot_y = kdmy_read_prop_float(entry, "pivot_y")
    };
    kdmy_forget_obtained(entry);
    entry = &_entry;
#endif
    
    lua_createtable(L, 0, 11);
    LuaL_add_table_field(L, "name", lua_pushstring, entry->name);
    LuaL_add_table_field(L, "x", lua_pushnumber, entry->x);
    LuaL_add_table_field(L, "y", lua_pushnumber, entry->y);
    LuaL_add_table_field(L, "width", lua_pushnumber, entry->width);
    LuaL_add_table_field(L, "height", lua_pushnumber, entry->height);
    LuaL_add_table_field(L, "frameX", lua_pushnumber, entry->frame_x);
    LuaL_add_table_field(L, "frameY", lua_pushnumber, entry->frame_y);
    LuaL_add_table_field(L, "frameWidth", lua_pushnumber, entry->frame_width);
    LuaL_add_table_field(L, "frameHeight", lua_pushnumber, entry->frame_height);
    LuaL_add_table_field(L, "pivotX", lua_pushnumber, entry->pivot_x);
    LuaL_add_table_field(L, "pivotY", lua_pushnumber, entry->pivot_y);

#ifdef JAVASCRIPT
    free(_entry.name);
#endif

    return 1;
}


static int script_atlas_init(lua_State* L) {
    const char* src = luaL_checkstring(L, 2);

    Atlas ret = atlas_init(src);
    return luascript_userdata_allocnew(L, ATLAS, ret);
}

static int script_atlas_destroy(lua_State* L) {
    Atlas atlas = luascript_read_userdata(L, ATLAS);

    if (luascript_userdata_is_allocated(L, ATLAS))
        atlas_destroy(&atlas);
    else
        printf("script_atlas_destroy() object was not allocated by lua\n");

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

    AtlasEntry ret = atlas_get_entry(atlas, name);

    return script_atlas_push_atlas_entry(L, ret);
}

static int script_atlas_get_entry_with_number_suffix(lua_State* L) {
    Atlas atlas = luascript_read_userdata(L, ATLAS);
    const char* name_prefix = luaL_optstring(L, 2, NULL);

    AtlasEntry ret = atlas_get_entry_with_number_suffix(atlas, name_prefix);

    return script_atlas_push_atlas_entry(L, ret);
}

static int script_atlas_get_glyph_fps(lua_State* L) {
    Atlas atlas = luascript_read_userdata(L, ATLAS);

    float ret = atlas_get_glyph_fps(atlas);

    lua_pushnumber(L, ret);
    return 1;
}

static int script_atlas_get_texture_resolution(lua_State* L) {
    Atlas atlas = luascript_read_userdata(L, ATLAS);
    int32_t resolution_width, resolution_height;

    atlas_get_texture_resolution(atlas, &resolution_width, &resolution_height);

    lua_pushinteger(L, resolution_width);
    lua_pushinteger(L, resolution_height);
    return 2;
}

static int script_atlas_utils_is_known_extension(lua_State* L) {
    const char* src = luaL_checkstring(L, 1);

    bool ret = atlas_utils_is_known_extension(src);

    lua_pushboolean(L, ret);
    return 1;
}




static const luaL_Reg ATLAS_FUNCTIONS[] = {
    { "init", script_atlas_init },
    { "destroy", script_atlas_destroy },
    { "get_index_of", script_atlas_get_index_of },
    { "get_entry", script_atlas_get_entry },
    { "get_entry_with_number_suffix", script_atlas_get_entry_with_number_suffix },
    { "get_glyph_fps", script_atlas_get_glyph_fps },
    { "get_texture_resolution", script_atlas_get_texture_resolution },
    { "utils_is_known_extension", script_atlas_utils_is_known_extension },
    { NULL, NULL }
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

