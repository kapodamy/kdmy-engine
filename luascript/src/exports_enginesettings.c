#include "engine_string.h"
#include "luascript_internal.h"

static int script_enginesettings_index(lua_State* L) {
    EngineSettings_t* enginesettings = luascript_read_userdata(L, ENGINESETTINGS);
    const char* field = luaL_checkstring(L, 2);

#ifdef JAVASCRIPT
    if (string_equals(field, "distractionsEnabled"))
        lua_pushboolean(L, kdmy_read_prop_boolean(enginesettings, "gameplay_enabled_distractions"));
    else if (string_equals(field, "flahsingLightsEnabled"))
        lua_pushboolean(L, kdmy_read_prop_boolean(enginesettings, "gameplay_enabled_flashinglights"));
    else if (string_equals(field, "inversedScrollEnabled"))
        lua_pushboolean(L, kdmy_read_prop_boolean(enginesettings, "inverse_strum_scroll"));
    else if (string_equals(field, "songProgressbarEnabled"))
        lua_pushboolean(L, kdmy_read_prop_boolean(enginesettings, "song_progressbar"));
#else
    if (string_equals(field, "distractionsEnabled"))
        lua_pushboolean(L, enginesettings->gameplay_enabled_distractions);
    else if (string_equals(field, "flahsingLightsEnabled"))
        lua_pushboolean(L, enginesettings->gameplay_enabled_flashinglights);
    else if (string_equals(field, "inversedScrollEnabled"))
        lua_pushboolean(L, enginesettings->inverse_strum_scroll);
    else if (string_equals(field, "songProgressbarEnabled"))
        lua_pushboolean(L, enginesettings->song_progressbar);
#endif
    else
        return luaL_error(L, "unknown field '%s'", field);

    return 1;
}

int script_enginesettings_new(lua_State* L, EngineSettings_t* enginesettings) {
    return luascript_userdata_new(L, ENGINESETTINGS, enginesettings);
}

static int script_enginesettings_gc(lua_State* L) {
    return luascript_userdata_gc(L, ENGINESETTINGS);
}

static int script_enginesettings_tostring(lua_State* L) {
    return luascript_userdata_tostring(L, ENGINESETTINGS);
}

void script_enginesettings_register(lua_State* lua) {
    luaL_newmetatable(lua, ENGINESETTINGS);

    lua_pushcfunction(lua, script_enginesettings_gc);
    lua_setfield(lua, -2, "__gc");

    lua_pushcfunction(lua, script_enginesettings_tostring);
    lua_setfield(lua, -2, "__tostring");

    lua_pushcfunction(lua, script_enginesettings_index);
    lua_setfield(lua, -2, "__index");

    /*lua_pushcfunction(lua, script_enginesettings_newindex);
    lua_setfield(lua, -2, "__newindex");*/

    lua_pop(lua, 1);
}