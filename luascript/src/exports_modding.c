#include "luascript_internal.h"
#include "string.h"

#ifdef JAVASCRIPT
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
EM_ASYNC_JS_PRFX(void*, modding_spawn_screen, (Modding modding, const char* layout_src, const char* script_src, const ModdingValueType arg_type, const void* arg_value, ModdingValueType* ret_type), {
    // parse arg_type/arg_value
    let arg = null;
    switch(arg_type) {
        default:
        case MODDING_VALUE_TYPE_NULL:
            arg = null;
            break;
        case MODDING_VALUE_TYPE_STRING:
            arg = kdmyEngine_ptrToString(arg_value);
            break;
        case MODDING_VALUE_TYPE_BOOLEAN:
            arg = kdmyEngine_get_uint32(arg_value) != 0;
            break;
        case MODDING_VALUE_TYPE_DOUBLE:
            arg = kdmyEngine_get_float64(arg_value);
            break;
    }
    if (arg_value != 0x00) _free(arg_value);

    let ret = await modding_spawn_screen(kdmyEngine_obtain(modding), kdmyEngine_ptrToString(layout_src), kdmyEngine_ptrToString(script_src), arg);
    
    let ret_ptr = 0x00;
    switch(typeof(ret)) {
        default:
        case "object":
            kdmyEngine_set_uint32(ret_type, MODDING_VALUE_TYPE_NULL);
            break;
        case "string":
            kdmyEngine_set_uint32(ret_type, MODDING_VALUE_TYPE_STRING);
            ret_ptr = kdmyEngine_stringToPtr(ret);
            break;
        case "boolean":
            kdmyEngine_set_uint32(ret_type, MODDING_VALUE_TYPE_BOOLEAN);
            ret_ptr = _malloc(4); kdmyEngine_set_uint32(ret_ptr, ret ? 1 : 0);
            break;
        case "number":
            kdmyEngine_set_uint32(ret_type, MODDING_VALUE_TYPE_DOUBLE);
            ret_ptr = _malloc(8); kdmyEngine_set_float64(ret_ptr, ret);
            break;
    }

    return ret_ptr;
});
EM_JS_PRFX(void, modding_set_exit_delay, (Modding modding, float delay_ms), {
    modding_set_exit_delay(kdmyEngine_obtain(modding), delay_ms);
});
EM_ASYNC_JS_PRFX(MessageBox, modding_get_messagebox, (Modding modding), {
    let ret = await modding_get_messagebox(kdmyEngine_obtain(modding));
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(void, modding_set_menu_in_layout_placeholder, (Modding modding, const char* placeholder_name, Menu menu), {
    modding_set_menu_in_layout_placeholder(kdmyEngine_obtain(modding), kdmyEngine_ptrToString(placeholder_name), kdmyEngine_obtain(menu));
});
EM_JS_PRFX(bool, modding_storage_set, (Modding modding, const char* week_name, const char* name, const uint8_t* data, uint32_t data_size), {
    let arraybuffer = data == 0x00 ? null : new ArrayBuffer(data_size);
    if (arraybuffer) {
        new Uint8Array(arraybuffer).set(HEAPU8.subarray(data, data + data_size), 0);
    }

    let ret = modding_storage_set(kdmyEngine_obtain(modding), kdmyEngine_ptrToString(week_name), kdmyEngine_ptrToString(name), arraybuffer, data_size);
    return ret ? 1 : 0;
});
EM_JS_PRFX(uint32_t, modding_storage_get, (Modding modding, const char* week_name, const char* name, uint8_t** data), {
    let arraybuffer = [null];

    let ret = modding_storage_get(kdmyEngine_obtain(modding), kdmyEngine_ptrToString(week_name), kdmyEngine_ptrToString(name), arraybuffer);

    let ptr;
    if (arraybuffer[0]) {
        ptr = _malloc(ret);
        if (ptr != 0x00) HEAPU8.set(new Uint8Array(arraybuffer[0]), ptr);
    } else {
        ptr = 0x00;
    }

    kdmyEngine_set_uint32(data, ptr);
    return ret;
});
EM_JS_PRFX(WeekInfo*, modding_get_loaded_weeks, (Modding modding, int32_t* out_size), {
    const size = [0];
    let ret = modding_get_loaded_weeks(kdmyEngine_obtain(modding), size);

    kdmyEngine_set_uint32(out_size, size[0]);
    return kdmyEngine_obtain(ret);
});
EM_ASYNC_JS_PRFX(int32_t, modding_launch_week, (Modding modding, const char* week_name, char* difficult, bool alt_tracks, char* bf, char* gf, char* gameplay_manifest, int32_t song_idx, const char* ws_label), {
    let ret = modding_launch_week(kdmyEngine_obtain(modding), kdmyEngine_ptrToString(week_name), kdmyEngine_ptrToString(difficult), alt_tracks, kdmyEngine_ptrToString(bf), kdmyEngine_ptrToString(gf), kdmyEngine_ptrToString(gameplay_manifest), song_idx, kdmyEngine_ptrToString(ws_label));
    _free(difficult);
    _free(bf);
    _free(gf);
    _free(gameplay_manifest);
    return ret;
});
EM_ASYNC_JS_PRFX(void, modding_launch_credits, (Modding modding), {
    modding_launch_credits(kdmyEngine_obtain(modding));
});
EM_ASYNC_JS_PRFX(bool, modding_launch_startscreen, (Modding modding), {
    let ret = modding_launch_startscreen(kdmyEngine_obtain(modding));
    return ret ? 1 : 0;
});
EM_ASYNC_JS_PRFX(bool, modding_launch_mainmenu, (Modding modding), {
    let ret = modding_launch_mainmenu(kdmyEngine_obtain(modding));
    return ret ? 1 : 0;
});
EM_ASYNC_JS_PRFX(void, modding_launch_settings, (Modding modding), {
    modding_launch_settings(kdmyEngine_obtain(modding));
});
EM_ASYNC_JS_PRFX(void, modding_launch_freeplay, (Modding modding), {
    modding_launch_freeplay(kdmyEngine_obtain(modding));
});
EM_ASYNC_JS_PRFX(int32_t, modding_launch_weekselector, (Modding modding), {
    let ret = modding_launch_weekselector(kdmyEngine_obtain(modding));
    return ret;
});
#endif


#define ADD_FIELD_INTEGER(L, name, integer) \
    {                                       \
        lua_pushinteger(L, integer);        \
        lua_setfield(L, -2, name);          \
    }

#define ADD_FIELD_STRING(L, name, string) \
    {                                     \
        lua_pushstring(L, (string));      \
        lua_setfield(L, -2, name);        \
    }

#define ADD_FIELD_NUMBER(L, name, number) \
    {                                     \
        lua_pushnumber(L, number);        \
        lua_setfield(L, -2, name);        \
    }

#define ADD_FIELD_BOOLEAN(L, name, boolean) \
    {                                       \
        lua_pushboolean(L, boolean);        \
        lua_setfield(L, -2, name);          \
    }

#define ADD_FIELD_NULL(L, name)    \
    {                              \
        lua_pushnil(L);            \
        lua_setfield(L, -2, name); \
    }

#define ADD_FIELD_TABLE(L, name, narr, nrec) \
    {                                        \
        lua_pushstring(L, name);             \
        lua_createtable(L, narr, nrec);      \
    }

#define ADD_FIELD_ARRAY_ITEM_AS_TABLE(L, narr, nrec, index_in_table) \
    {                                                                \
        lua_pushinteger(L, index_in_table);                          \
        lua_createtable(L, narr, nrec);                              \
    }

#ifdef JAVASCRIPT

static void ADD_FIELD_STRING_JS(lua_State* L, const char* name, void* js_obj) {
    char* str = kdmy_read_prop_string(js_obj, name);
    ADD_FIELD_STRING(L, name, str);
    free(str);
}

static void ADD_FIELD_BOOLEAN_JS(lua_State* L, const char* name, void* js_obj) {
    bool bl = kdmy_read_prop_boolean(js_obj, name);
    ADD_FIELD_BOOLEAN(L, name, bl);
}

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

    ModdingValueType arg_type;
    void* arg_value = luascript_parse_and_allocate_modding_value(L, 3, &arg_type, true);

    ModdingValueType ret_type;
    void* ret = modding_spawn_screen(modding, layout_src, script_src, arg_type, arg_value, &ret_type);

    return luascript_push_and_deallocate_modding_value(L, ret_type, ret);
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

static int script_modding_storage_get(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    Modding modding = (Modding)luascript->context;

    const char* week_name = luaL_optstring(L, 1, NULL);
    const char* name = luaL_optstring(L, 2, NULL);
    uint8_t* data;

    uint32_t ret = modding_storage_get(modding, week_name, name, &data);

    if (!data)
        lua_pushnil(L);
    else
        lua_pushlstring(L, (const char*)data, (size_t)ret);

#ifdef JAVASCRIPT
    free(data);
#endif

    return 1;
}

static int script_modding_storage_set(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    Modding modding = (Modding)luascript->context;

    const char* week_name = luaL_optstring(L, 1, NULL);
    const char* name = luaL_optstring(L, 2, NULL);
    const uint8_t* data;
    size_t data_size;

    if (lua_isnil(L, 3)) {
        data = NULL;
        data_size = 0;
    } else {
        data = (const uint8_t*) luaL_checklstring(L, 3, &data_size);
    }

    bool ret = modding_storage_set(modding, week_name, name, data, (uint32_t)data_size);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_modding_get_loaded_weeks(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    Modding modding = (Modding)luascript->context;

    int32_t weeks_size;
    WeekInfo* weeks = modding_get_loaded_weeks(modding, &weeks_size);

    if (!weeks) {
        lua_pushnil(L);
        return 1;
    }

#ifdef JAVASCRIPT
    lua_createtable(L, weeks_size, 0);
    for (int i = 0 ; i < weeks_size ; i++) {
        void* weekinfo = kdmy_read_array_item_object(weeks, i);
        int songs_count = kdmy_read_prop_integer(weekinfo, "songs_count");
        ADD_FIELD_ARRAY_ITEM_AS_TABLE(L, 0, 7, i + 1);
        {
            ADD_FIELD_STRING_JS(L, "name", weekinfo);
            ADD_FIELD_STRING_JS(L, "display_name", weekinfo);
            ADD_FIELD_STRING_JS(L, "description", weekinfo);
            ADD_FIELD_STRING_JS(L, "custom_folder", weekinfo);
            ADD_FIELD_STRING_JS(L, "unlock_directive", weekinfo);
            ADD_FIELD_STRING_JS(L, "emit_directive", weekinfo);

            void* songs = kdmy_read_prop_object(weekinfo, "songs");
            ADD_FIELD_TABLE(L, "songs", songs_count, 0);
            {
                for (int j = 0 ; j < songs_count ; j++) {
                    void* song = kdmy_read_array_item_object(songs, j);
                    ADD_FIELD_ARRAY_ITEM_AS_TABLE(L, 0, 5, j + 1);
                    {
                        ADD_FIELD_BOOLEAN_JS(L, "freeplay_only", song);
                        ADD_FIELD_STRING_JS(L, "name", song);
                        ADD_FIELD_STRING_JS(L, "freeplay_unlock_directive", song);
                        ADD_FIELD_STRING_JS(L, "freeplay_song_filename", song);
                        ADD_FIELD_STRING_JS(L, "freeplay_description", song);
                    }
                    lua_settable(L, -3);
                    kdmy_forget_obtained(song);
                }
            }
            lua_settable(L, -3);
            kdmy_forget_obtained(songs);
        }
        lua_settable(L, -3);
        kdmy_forget_obtained(weekinfo);
    }
    kdmy_forget_obtained(weeks);
#else
    lua_createtable(L, weeks_size, 0);
    for (int i = 0 ; i < weeks_size ; i++) {
        int songs_count = weeks[i].songs_count;
        ADD_FIELD_ARRAY_ITEM_AS_TABLE(L, 0, 7, i + 1);
        {
            ADD_FIELD_STRING(L, "name", weeks[i].name);
            ADD_FIELD_STRING(L, "display_name", weeks[i].display_name);
            ADD_FIELD_STRING(L, "description", weeks[i].description);
            ADD_FIELD_STRING(L, "custom_folder", weeks[i].custom_folder);
            ADD_FIELD_STRING(L, "unlock_directive", weeks[i].unlock_directive);
            ADD_FIELD_STRING(L, "emit_directive", weeks[i].emit_directive);
            ADD_FIELD_TABLE(L, "songs", weeks[i].songs_count, 0);
            {
                for (int j = 0 ; j < songs_count ; j++) {
                    ADD_FIELD_ARRAY_ITEM_AS_TABLE(L, 0, 5, j + 1);
                    {
                        ADD_FIELD_BOOLEAN(L, "freeplay_only", weeks[i].songs[j].freeplay_only);
                        ADD_FIELD_STRING(L, "name", weeks[i].songs[j].name);
                        ADD_FIELD_STRING(L, "freeplay_unlock_directive", weeks[i].songs[j].freeplay_unlock_directive);
                        ADD_FIELD_STRING(L, "freeplay_song_filename", weeks[i].songs[j].freeplay_song_filename);
                        ADD_FIELD_STRING(L, "freeplay_description", weeks[i].songs[j].freeplay_description);
                    }
                    lua_settable(L, -3);
                }
            }
            lua_settable(L, -3);
        }
        lua_settable(L, -3);
    }
#endif

    return 1;
}

static int script_modding_launch_week(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    Modding modding = (Modding)luascript->context;

    const char* week_name = luaL_checkstring(L, 1);
    char* difficult = luascript_get_string_copy(L, 2, NULL);
    bool alt_tracks = (bool)lua_toboolean(L, 3);
    char* bf = luascript_get_string_copy(L, 4, NULL);
    char* gf = luascript_get_string_copy(L, 5, NULL);
    char* gameplay_manifest = luascript_get_string_copy(L, 6, NULL);
    int32_t song_idx = (int32_t)luaL_optinteger(L, 7, -1);
    const char* ws_label = luaL_optstring(L, 8, NULL);

    if (!difficult || difficult[0] == '\0') {
        free(difficult);
        return luaL_error(L, "difficult must be provided");
    }

    int32_t ret = modding_launch_week(modding, week_name, difficult, alt_tracks, bf, gf, gameplay_manifest, song_idx, ws_label);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_modding_launch_credits(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    Modding modding = (Modding)luascript->context;


    modding_launch_credits(modding);

    return 0;
}

static int script_modding_launch_startscreen(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    Modding modding = (Modding)luascript->context;


    bool ret = modding_launch_startscreen(modding);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_modding_launch_mainmenu(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    Modding modding = (Modding)luascript->context;


    bool ret = modding_launch_mainmenu(modding);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_modding_launch_settings(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    Modding modding = (Modding)luascript->context;


    modding_launch_settings(modding);

    return 0;
}

static int script_modding_launch_freeplay(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    Modding modding = (Modding)luascript->context;


    modding_launch_freeplay(modding);

    return 0;
}

static int script_modding_launch_weekselector(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    Modding modding = (Modding)luascript->context;


    int32_t ret = modding_launch_weekselector(modding);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}



static const luaL_Reg MODDING_FUNCTIONS[] = {
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
    { "modding_storage_set", script_modding_storage_set },
    { "modding_storage_get", script_modding_storage_get },
    { "modding_get_loaded_weeks", script_modding_get_loaded_weeks },
    { "modding_launch_week", script_modding_launch_week },
    { "modding_launch_credits", script_modding_launch_credits },
    { "modding_launch_startscreen", script_modding_launch_startscreen },
    { "modding_launch_mainmenu", script_modding_launch_mainmenu },
    { "modding_launch_settings", script_modding_launch_settings },
    { "modding_launch_freeplay", script_modding_launch_freeplay },
    { "modding_launch_weekselector", script_modding_launch_weekselector },
    { NULL, NULL }
};



void script_modding_register(lua_State* L) {
    for (size_t i = 0;; i++) {
        if (MODDING_FUNCTIONS[i].name == NULL || MODDING_FUNCTIONS[i].func == NULL) break;
        lua_pushcfunction(L, MODDING_FUNCTIONS[i].func);
        lua_setglobal(L, MODDING_FUNCTIONS[i].name);
    }
}
