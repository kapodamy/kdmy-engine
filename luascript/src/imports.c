#include "luascript_internal.h"
#include "timer.h"
#include "string.h"

#define FUNCTION(luascript, function_name)               \
    lua_State* lua = luascript->L;                       \
    const char* FUNCTION = function_name;                \
    if (lua_getglobal(lua, FUNCTION) != LUA_TFUNCTION) { \
        lua_pop(lua, 1);                                 \
        return;                                          \
    }

#define FUNCTION_WITH_DEFAULT_RETURN(luascript, function_name, def_ret)               \
    lua_State* lua = luascript->L;                       \
    const char* FUNCTION = function_name;                \
    if (lua_getglobal(lua, FUNCTION) != LUA_TFUNCTION) { \
        lua_pop(lua, 1);                                 \
        return def_ret;                                          \
    }


static int lua_imported_fn(lua_State* L, const char* fn_name, int arguments_count) {
    int result = luascript_pcallk(L, arguments_count, 0);
    if (result == LUA_OK) return 0;

    const char* error_message = lua_tostring(L, -1);
    lua_pop(L, 1);

    fprintf(stderr, "lua_imported_fn() call to '%s' failed.\n%s\n", fn_name, error_message);
    // fflush(stdout);

    return 1;
}

static int lua_imported_fn_with_return(lua_State* L, const char* fn_name, int arguments_count, int* return_lua_type, LuaValue* return_lua_value) {
    int result = luascript_pcallk(L, arguments_count, 1);
    if (result == LUA_OK) {
        switch (lua_type(L, -1)) {
            case LUA_TBOOLEAN:
                return_lua_value->as_boolean = lua_toboolean(L, -1);
                *return_lua_type = LUA_TBOOLEAN;
                break;
            case LUA_TNUMBER:
                return_lua_value->as_number = lua_tonumber(L, -1);
                *return_lua_type = LUA_TNUMBER;
                break;
            case LUA_TSTRING:
                return_lua_value->as_string = strdup(lua_tostring(L, -1));
                *return_lua_type = LUA_TSTRING;
                break;
            case LUA_TNIL:
            case LUA_TNONE:
                return_lua_value->as_string = NULL;
                *return_lua_type = LUA_TNIL;
                break;
            default:
                printf("lua_imported_fn_with_return() unknown lua return type");
                break;
        }
        return 0;
    }

    const char* error_message = lua_tostring(L, -1);
    lua_pop(L, 1);

    fprintf(stderr, "lua_imported_fn() call to '%s' failed.\n%s\n", fn_name, error_message);
    // fflush(stdout);

    return 1;
}


void luascript_notify_weekinit(Luascript luascript, int32_t freeplay_index) {
    FUNCTION(luascript, "f_weekinit");

    lua_pushinteger(lua, freeplay_index);

    lua_imported_fn(lua, FUNCTION, 1);
}

void luascript_notify_beforeready(Luascript luascript, bool from_retry) {
    FUNCTION(luascript, "f_beforeready");

    lua_pushboolean(lua, from_retry);

    lua_imported_fn(lua, FUNCTION, 1);
}

void luascript_notify_ready(Luascript luascript) {
    FUNCTION(luascript, "f_ready");
    lua_imported_fn(lua, FUNCTION, 0);
}

void luascript_notify_aftercountdown(Luascript luascript) {
    FUNCTION(luascript, "f_aftercountdown");
    lua_imported_fn(lua, FUNCTION, 0);
}

void luascript_notify_frame(Luascript luascript, float elapsed) {
    FUNCTION(luascript, "f_frame");

    lua_pushnumber(lua, (double)elapsed);

    lua_imported_fn(lua, FUNCTION, 1);
}

void luascript_notify_roundend(Luascript luascript, bool loose) {
    FUNCTION(luascript, "f_roundend");

    lua_pushboolean(lua, loose);

    lua_imported_fn(lua, FUNCTION, 1);
}

void luascript_notify_weekend(Luascript luascript, bool giveup) {
    FUNCTION(luascript, "f_weekend");

    lua_pushboolean(lua, giveup);

    lua_imported_fn(lua, FUNCTION, 1);
}

void luascript_notify_diedecision(Luascript luascript, bool retry_or_giveup) {
    FUNCTION(luascript, "f_diedecision");

    lua_pushboolean(lua, retry_or_giveup);

    lua_imported_fn(lua, FUNCTION, 1);
}

void luascript_notify_pause(Luascript luascript, bool pause_or_resume) {
    FUNCTION(luascript, "f_pause");

    lua_pushboolean(lua, pause_or_resume);

    lua_imported_fn(lua, FUNCTION, 1);
}

void luascript_notify_weekleave(Luascript luascript) {
    FUNCTION(luascript, "f_weekleave");
    lua_imported_fn(lua, FUNCTION, 0);
}

void luascript_notify_afterresults(Luascript luascript) {
    FUNCTION(luascript, "f_afterresults");
    lua_imported_fn(lua, FUNCTION, 0);
}

void luascript_notify_scriptchange(Luascript luascript) {
    FUNCTION(luascript, "f_scriptchange");
    lua_imported_fn(lua, FUNCTION, 0);
}

void luascript_notify_pause_optionselected(Luascript luascript, int32_t option_index, uint32_t buttons) {
    FUNCTION(luascript, "f_pause_optionselected");

    lua_pushinteger(lua, option_index);
    lua_pushinteger(lua, (lua_Integer)buttons);

    lua_imported_fn(lua, FUNCTION, 2);
}

void luascript_notify_pause_menuvisible(Luascript luascript, bool shown_or_hidden) {
    FUNCTION(luascript, "f_pause_menuvisible");

    lua_pushboolean(lua, shown_or_hidden);

    lua_imported_fn(lua, FUNCTION, 1);
}

void luascript_notify_note(Luascript luascript, double timestamp, int32_t id, double duration, double data, bool special, int32_t player_id, uint32_t state) {
    FUNCTION(luascript, "f_note");

    lua_pushnumber(lua, timestamp);
    lua_pushinteger(lua, id);
    lua_pushnumber(lua, duration);
    lua_pushnumber(lua, data);
    lua_pushboolean(lua, special);
    lua_pushinteger(lua, player_id);
    lua_pushinteger(lua, state);

    lua_imported_fn(lua, FUNCTION, 7);
}

void luascript_notify_buttons(Luascript luascript, int32_t player_id, uint32_t buttons) {
    FUNCTION(luascript, "f_buttons");

    lua_pushinteger(lua, player_id);
    lua_pushinteger(lua, (lua_Integer)buttons);

    lua_imported_fn(lua, FUNCTION, 2);
}

void luascript_notify_unknownnote(Luascript luascript, int32_t player_id, double timestamp, int32_t id, double duration, double data) {
    FUNCTION(luascript, "f_unknownnote");

    lua_pushinteger(lua, player_id);
    lua_pushnumber(lua, timestamp);
    lua_pushinteger(lua, id);
    lua_pushnumber(lua, duration);
    lua_pushnumber(lua, data);

    lua_imported_fn(lua, FUNCTION, 5);
}

void luascript_notify_beat(Luascript luascript, int32_t current_beat, float since) {
    FUNCTION(luascript, "f_beat");

    lua_pushinteger(lua, current_beat);
    lua_pushnumber(lua, (lua_Number)since);

    lua_imported_fn(lua, FUNCTION, 2);
}

void luascript_notify_quarter(Luascript luascript, int32_t current_quarter, float since) {
    FUNCTION(luascript, "f_quarter");

    lua_pushinteger(lua, current_quarter);
    lua_pushnumber(lua, (lua_Number)since);

    lua_imported_fn(lua, FUNCTION, 2);
}

void luascript_notify_timer_run(Luascript luascript, double kos_timestamp) {
    FUNCTION(luascript, "______kdmyEngine_timer_run");

    lua_pushnumber(lua, kos_timestamp);
    lua_pushboolean(lua, false);

    lua_imported_fn(lua, FUNCTION, 2);
}

void luascript_notify_timersong_run(Luascript luascript, double song_timestamp) {
    FUNCTION(luascript, "______kdmyEngine_timer_run");

    lua_pushnumber(lua, song_timestamp);
    lua_pushboolean(lua, true);

    lua_imported_fn(lua, FUNCTION, 2);
}

void luascript_notify_dialogue_builtin_open(Luascript luascript, const char* dialog_src) {
    FUNCTION(luascript, "f_dialogue_builtin_open");

    lua_pushstring(lua, dialog_src);

#ifdef JAVASCRIPT
    free((char*)dialog_src);
#endif

    lua_imported_fn(lua, FUNCTION, 1);
}

void luascript_notify_dialogue_line_starts(Luascript luascript, int line_index, const char* state_name, const char* text) {
    FUNCTION(luascript, "f_dialogue_line_starts");

    lua_pushinteger(lua, line_index);
    lua_pushstring(lua, state_name);
    lua_pushstring(lua, text);

#ifdef JAVASCRIPT
    free((char*)state_name);
    free((char*)text);
#endif

    lua_imported_fn(lua, FUNCTION, 3);
}

void luascript_notify_dialogue_line_ends(Luascript luascript, int line_index, const char* state_name, const char* text) {
    FUNCTION(luascript, "f_dialogue_line_ends");

    lua_pushinteger(lua, line_index);
    lua_pushstring(lua, state_name);
    lua_pushstring(lua, text);

#ifdef JAVASCRIPT
    free((char*)state_name);
    free((char*)text);
#endif

    lua_imported_fn(lua, FUNCTION, 3);
}

void luascript_notify_after_strum_scroll(Luascript luascript) {
    FUNCTION(luascript, "f_after_strum_scroll");

    lua_imported_fn(lua, FUNCTION, 0);
}

void luascript_notify_modding_menu_option_selected(Luascript luascript, void* menu, int index, const char* name) {
    FUNCTION(luascript, "f_modding_menu_option_selected");

    luascript_create_userdata(luascript, menu, MENU, false);
    lua_pushinteger(luascript->L, index);
    lua_pushstring(luascript->L, name);

#ifdef JAVASCRIPT
    free((char*)name);
#endif

    lua_imported_fn(lua, FUNCTION, 3);
}

bool luascript_notify_modding_menu_option_choosen(Luascript luascript, void* menu, int index, const char* name) {
    FUNCTION_WITH_DEFAULT_RETURN(luascript, "f_modding_menu_option_choosen", false);

    luascript_create_userdata(luascript, menu, MENU, false);
    lua_pushinteger(luascript->L, index);
    lua_pushstring(luascript->L, name);

#ifdef JAVASCRIPT
    free((char*)name);
#endif

    int ret_type;
    LuaValue ret_value;

    if (!lua_imported_fn_with_return(luascript->L, FUNCTION, 3, &ret_type, &ret_value)) {
        return false;
    }

    switch (ret_type) {
        case LUA_TNUMBER:
            return ret_value.as_number != 1.0;
        case LUA_TSTRING: {
            bool ret = ret_value.as_string || ret_value.as_string[0] != '\0';
            free(ret_value.as_string);
            return ret;
        }
        default:
            return false;
    }
}

bool luascript_notify_modding_back(Luascript luascript) {
    FUNCTION_WITH_DEFAULT_RETURN(luascript, "f_modding_back", false);

    int ret_type;
    LuaValue ret_value;

    if (!lua_imported_fn_with_return(luascript->L, FUNCTION, 0, &ret_type, &ret_value)) {
        return false;
    }

    switch (ret_type) {
        case LUA_TNUMBER:
            return ret_value.as_number != 1.0;
        case LUA_TSTRING: {
            bool ret = ret_value.as_string || ret_value.as_string[0] != '\0';
            free(ret_value.as_string);
            return ret;
        }
        default:
            return false;
    }
}

void luascript_notify_modding_exit(Luascript luascript, int* return_lua_type, LuaValue* return_lua_value) {
    FUNCTION(luascript, "f_modding_exit");

    lua_imported_fn_with_return(luascript->L, FUNCTION, 0, return_lua_type, return_lua_value);
}

void luascript_notify_modding_init(Luascript luascript, int value_type, LuaValue* value) {
    FUNCTION(luascript, "f_modding_init");

    switch (value_type) {
        case LUA_TNUMBER:
            lua_pushnumber(luascript->L, value->as_number);
            break;
        case LUA_TSTRING:
            lua_pushstring(luascript->L, value->as_string);
            free(value->as_string);
            break;
        case LUA_TBOOLEAN:
            lua_pushboolean(luascript->L, value->as_boolean);
            break;
        default:
            lua_pushnil(luascript->L);
            break;
    }

    lua_imported_fn(luascript->L, FUNCTION, 1);
}

void luascript_call_function(Luascript luascript, const char* function_name) {
    if (!function_name || !function_name[0]) return;
    FUNCTION(luascript, function_name);

    lua_imported_fn(lua, function_name, 0);

#ifdef JAVASCRIPT
    free((char*)function_name);
#endif
}
