#include "luascript_internal.h"

#include "game/common/weekenumerator.h"
#include "game/modding.h"
#include "malloc_utils.h"
#include "stringutils.h"


static int script_modding_unlockdirective_create(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    Modding modding = (Modding)luascript->context;

    const char* name = luaL_optstring(L, 1, NULL);
    float64 value = luaL_checknumber(L, 2);

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

    float64 ret = modding_unlockdirective_get(modding, name);

    lua_pushnumber(L, (lua_Number)ret);
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

    luascript_change_working_folder(L);
    SoundPlayer ret = modding_replace_native_background_music(modding, music_src);
    luascript_restore_working_folder(L);

    return script_soundplayer_new(L, ret);
}

static int script_modding_spawn_screen(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    Modding modding = (Modding)luascript->context;

    const char* layout_src = luaL_optstring(L, 1, NULL);
    const char* script_src = luaL_optstring(L, 2, NULL);

    ModdingValueType arg_type;
    void* arg_value = luascript_parse_and_allocate_modding_value(L, 3, &arg_type, true);

    luascript_change_working_folder(L);
    ModdingValueType ret_type;
    void* ret = modding_spawn_screen(modding, layout_src, script_src, arg_type, arg_value, &ret_type);
    luascript_restore_working_folder(L);

    if (arg_value) free_chk(arg_value);

    return luascript_push_modding_value(L, true, ret_type, ret);
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

    size_t ret = modding_storage_get(modding, week_name, name, &data);

    if (!data)
        lua_pushnil(L);
    else
        lua_pushlstring(L, (const char*)data, ret);

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
        data = (const uint8_t*)luaL_checklstring(L, 3, &data_size);
    }

    bool ret = modding_storage_set(modding, week_name, name, data, data_size);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_modding_get_loaded_weeks(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    Modding modding = (Modding)luascript->context;

    int32_t weeks_size;
    WeekInfo* weeks = modding_get_loaded_weeks(modding, &weeks_size);

    if (!weeks || weeks_size < 1) {
        lua_createtable(L, 0, 0);
        return 1;
    }

    lua_createtable(L, weeks_size, 0);
    for (int32_t i = 0; i < weeks_size; i++) {
        int32_t songs_count = weeks[i].songs_count;
        luascript_helper_add_field_array_item_as_table(L, 0, 7, i + 1);
        {
            luascript_helper_add_field_string(L, "name", weeks[i].name);
            luascript_helper_add_field_string(L, "display_name", weeks[i].display_name);
            luascript_helper_add_field_string(L, "description", weeks[i].description);
            luascript_helper_add_field_string(L, "custom_folder", weeks[i].custom_folder);
            luascript_helper_add_field_string(L, "unlock_directive", weeks[i].unlock_directive);
            luascript_helper_add_field_string(L, "emit_directive", weeks[i].emit_directive);
            luascript_helper_add_field_table(L, "songs", weeks[i].songs_count, 0);
            {
                for (int j = 0; j < songs_count; j++) {
                    luascript_helper_add_field_array_item_as_table(L, 0, 5, j + 1);
                    {
                        luascript_helper_add_field_boolean(L, "freeplay_only", weeks[i].songs[j].freeplay_only);
                        luascript_helper_add_field_string(L, "name", weeks[i].songs[j].name);
                        luascript_helper_add_field_string(L, "freeplay_unlock_directive", weeks[i].songs[j].freeplay_unlock_directive);
                        luascript_helper_add_field_string(L, "freeplay_song_filename", weeks[i].songs[j].freeplay_song_filename);
                        luascript_helper_add_field_string(L, "freeplay_description", weeks[i].songs[j].freeplay_description);
                    }
                    lua_settable(L, -3);
                }
            }
            lua_settable(L, -3);
        }
        lua_settable(L, -3);
    }

    return 1;
}

static int script_modding_launch_week(lua_State* L) {
    Luascript luascript = luascript_get_instance(L);
    Modding modding = (Modding)luascript->context;

    const char* week_name = luaL_checkstring(L, 1);
    const char* difficult = luaL_checkstring(L, 2);
    bool alt_tracks = (bool)lua_toboolean(L, 3);
    const char* bf = luaL_optstring(L, 4, NULL);
    const char* gf = luaL_optstring(L, 5, NULL);
    const char* gameplay_manifest = luaL_checkstring(L, 6);
    int32_t song_idx = (int32_t)luaL_optinteger(L, 7, -1);
    const char* ws_label = luaL_optstring(L, 8, NULL);

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
    {"modding_get_layout", script_modding_get_layout},
    {"modding_exit", script_modding_exit},
    {"modding_set_halt", script_modding_set_halt},
    {"modding_unlockdirective_create", script_modding_unlockdirective_create},
    {"modding_unlockdirective_remove", script_modding_unlockdirective_remove},
    {"modding_unlockdirective_get", script_modding_unlockdirective_get},
    {"modding_get_active_menu", script_modding_get_active_menu},
    {"modding_choose_native_menu_option", script_modding_choose_native_menu_option},
    {"modding_get_native_menu", script_modding_get_native_menu},
    {"modding_set_active_menu", script_modding_set_active_menu},
    {"modding_get_native_background_music", script_modding_get_native_background_music},
    {"modding_replace_native_background_music", script_modding_replace_native_background_music},
    {"modding_spawn_screen", script_modding_spawn_screen},
    {"modding_set_exit_delay", script_modding_set_exit_delay},
    {"modding_get_messagebox", script_modding_get_messagebox},
    {"modding_set_menu_in_layout_placeholder", script_modding_set_menu_in_layout_placeholder},
    {"modding_storage_set", script_modding_storage_set},
    {"modding_storage_get", script_modding_storage_get},
    {"modding_get_loaded_weeks", script_modding_get_loaded_weeks},
    {"modding_launch_week", script_modding_launch_week},
    {"modding_launch_credits", script_modding_launch_credits},
    {"modding_launch_startscreen", script_modding_launch_startscreen},
    {"modding_launch_mainmenu", script_modding_launch_mainmenu},
    {"modding_launch_settings", script_modding_launch_settings},
    {"modding_launch_freeplay", script_modding_launch_freeplay},
    {"modding_launch_weekselector", script_modding_launch_weekselector},
    {NULL, NULL}
};


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void script_modding_register(lua_State* L) {
    for (size_t i = 0;; i++) {
        if (MODDING_FUNCTIONS[i].name == NULL || MODDING_FUNCTIONS[i].func == NULL) break;
        lua_pushcfunction(L, MODDING_FUNCTIONS[i].func);
        lua_setglobal(L, MODDING_FUNCTIONS[i].name);
    }
}
