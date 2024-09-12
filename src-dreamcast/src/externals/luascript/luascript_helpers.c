#include "luascript_internal.h"

#include <assert.h>
#include <string.h>

#include "externals/luascript.h"
#include "fs.h"
#include "luascript_enums.h"
#include "malloc_utils.h"
#include "stringutils.h"


static void* luascript_key_ptr = &luascript_get_instance;


static int luascript_internal_get_lua_reference(Luascript luascript, void* obj) {
    const LuascriptObject* shared_array = luascript->shared_array;
    const size_t shared_size = luascript->shared_size;

    for (size_t i = 0; i < shared_size; i++) {
        if (shared_array[i].obj_ptr == obj) {
            return shared_array[i].lua_ref;
        }
    }

    return LUA_NOREF;
}

static LuascriptObject* luascript_internal_add_lua_reference(Luascript luascript, void* obj, int ref, bool allocated) {
    size_t index;

    // find and empty slot
    const LuascriptObject* shared_array = luascript->shared_array;
    const size_t shared_size = luascript->shared_size;

    for (size_t i = 0; i < shared_size; i++) {
        if (shared_array[i].obj_ptr == NULL) {
            index = i;
            goto L_store_and_return;
        }
    }

    // no empty slots, grow the array and add it
    index = luascript->shared_size;
    luascript->shared_size += LUASCRIPT_SHARED_ARRAY_CHUNK_SIZE;

    luascript->shared_array = realloc_for_array(luascript->shared_array, luascript->shared_size, LuascriptObject);

    for (size_t i = 0; i < LUASCRIPT_SHARED_ARRAY_CHUNK_SIZE; i++) {
        LuascriptObject* entry = &luascript->shared_array[i + index];
        entry->lua_ref = LUA_NOREF;
        entry->obj_ptr = NULL;
        entry->was_allocated_by_lua = false;
    }

L_store_and_return:
    luascript->shared_array[index].obj_ptr = obj;
    luascript->shared_array[index].lua_ref = ref;
    luascript->shared_array[index].was_allocated_by_lua = allocated;

    return &luascript->shared_array[index];
}

static int luascript_internal_remove_lua_reference(Luascript luascript, void* obj) {
    LuascriptObject* shared_array = luascript->shared_array;
    const size_t shared_size = luascript->shared_size;

    for (size_t i = 0; i < shared_size; i++) {
        if (shared_array[i].obj_ptr == obj) {
            int ref = shared_array[i].lua_ref;
            shared_array[i].obj_ptr = NULL;
            shared_array[i].lua_ref = LUA_REFNIL;
            shared_array[i].was_allocated_by_lua = false;
            return ref;
        }
    }

    return LUA_NOREF;
}

static LuascriptObject* luascript_internal_read_luascript_object(lua_State* L, int ud, const char* tname) {
    LuascriptObject* udata = luaL_checkudata(L, ud, tname);

    if (!udata || !udata->obj_ptr || udata->lua_ref == LUA_REFNIL || udata->lua_ref == LUA_NOREF) {
        return NULL;
    }

    return udata;
}

static void* luascript_internal_read_userdata(lua_State* L, int ud, const char* tname) {
    LuascriptObject* udata = luascript_internal_read_luascript_object(L, ud, tname);

    if (!udata) {
        luaL_error(L, "%s object was destroyed.", tname);
        return NULL;
    }

    return udata->obj_ptr;
}

/*static int luascript_internal_is_userdata_equals(lua_State* L) {
    bool equals;

    int type_a = lua_type(L, 1);
    int type_b = lua_type(L, 2);

    if (type_a != LUA_TUSERDATA || type_b != LUA_TUSERDATA) {
        equals = false;
    } else {
        void* a = lua_touserdata(L, 1);
        void* b = lua_touserdata(L, 2);

        equals = (a == NULL && b == NULL) || (a != b);
    }

    lua_pushboolean(L, equals);

    return 1;
}*/

static char* luascript_internal_get_string_copy(lua_State* L, int idx, const char* def) {
    const char* str = luaL_optstring(L, idx, def);
    return string_duplicate(str);
}



int luascript_create_userdata(Luascript luascript, void* obj, const char* metatable_name, bool allocated) {
    lua_State* L = luascript->L;

    if (!obj) {
        lua_pushnil(L);
        return 1;
    }

    int ref = luascript_internal_get_lua_reference(luascript, obj);
    if (ref != LUA_NOREF) {
        // recover userdata back into stack
        lua_pushinteger(L, ref);
        lua_gettable(L, LUA_REGISTRYINDEX);
    } else {
        LuascriptObject* udata = (LuascriptObject*)lua_newuserdata(L, sizeof(LuascriptObject));
        malloc_assert(udata, LuascriptObject);

        luaL_getmetatable(L, metatable_name);
        lua_setmetatable(L, -2);

        lua_pushvalue(L, -1);
        ref = luaL_ref(L, LUA_REGISTRYINDEX);

        udata->lua_ref = ref;
        udata->obj_ptr = obj;
        udata->was_allocated_by_lua = allocated;

        luascript_internal_add_lua_reference(luascript, obj, ref, allocated);
    }

    return 1;
}

void luascript_remove_userdata(Luascript luascript, void* obj) {
    assert(obj);

    lua_State* L = luascript->L;

    // remove object from shared array
    int ref = luascript_internal_remove_lua_reference(luascript, obj);
    if (ref == LUA_NOREF) return;

    // adquire lua userdata
    lua_pushinteger(L, ref);
    lua_gettable(L, LUA_REGISTRYINDEX);

    // nullify userdata, this is already in the stack if was called by lua GC
    LuascriptObject* udata = lua_touserdata(L, lua_gettop(L));
    if (udata) {
        udata->lua_ref = LUA_REFNIL;
        udata->obj_ptr = NULL;
        udata->was_allocated_by_lua = false;
    }
    lua_pop(L, 1);

    // remove from lua registry
    luaL_unref(L, LUA_REGISTRYINDEX, ref);
}


void* luascript_read_userdata(lua_State* L, const char* check_metatable_name) {
    if (lua_isnil(L, 1)) {
        luaL_error(L, "%s was null (nil in lua).", check_metatable_name);
        return NULL;
    }

    return luascript_internal_read_userdata(L, 1, check_metatable_name);
}

void* luascript_read_nullable_userdata(lua_State* L, int idx, const char* check_metatable_name) {
    if (lua_isnil(L, idx))
        return NULL;
    else
        return luascript_internal_read_userdata(L, idx, check_metatable_name);
}


Luascript luascript_get_instance(lua_State* L) {
    lua_pushlightuserdata(L, luascript_key_ptr);
    lua_gettable(L, LUA_REGISTRYINDEX);

    Luascript luascript = lua_touserdata(L, -1);
    assert(luascript);
    lua_pop(L, 1);

    return luascript;
}

void luascript_set_instance(Luascript luascript) {
    lua_State* L = luascript->L;

    lua_pushlightuserdata(L, luascript_key_ptr);
    lua_pushlightuserdata(L, luascript);
    lua_settable(L, LUA_REGISTRYINDEX);
}


int luascript_userdata_new(lua_State* L, const char* check_metatable_name, void* obj) {
    return luascript_create_userdata(luascript_get_instance(L), obj, check_metatable_name, false);
}

int luascript_userdata_allocnew(lua_State* L, const char* check_metatable_name, void* obj) {
    return luascript_create_userdata(luascript_get_instance(L), obj, check_metatable_name, true);
}

int luascript_userdata_tostring(lua_State* L, const char* check_metatable_name) {
    void* udata = luascript_read_userdata(L, check_metatable_name);
    lua_pushfstring(L, "{%s %p}", check_metatable_name, udata);
    return 1;
}

int luascript_userdata_gc(lua_State* L, const char* check_metatable_name) {
    LuascriptObject* udata = luaL_checkudata(L, 1, check_metatable_name);

    if (udata && (udata->lua_ref != LUA_REFNIL && udata->lua_ref != LUA_NOREF)) {
        void* obj = udata->obj_ptr;
        luascript_remove_userdata(luascript_get_instance(L), obj);
    }

    return 0;
}

int luascript_userdata_destroy(lua_State* L, const char* check_metatable_name, Destructor destructor) {
    LuascriptObject* udata = luascript_internal_read_luascript_object(L, 1, check_metatable_name);

    if (!udata) return 0;

    void* obj_ptr = udata->obj_ptr;
    bool was_allocated_by_lua = udata->was_allocated_by_lua;

    luascript_remove_userdata(luascript_get_instance(L), obj_ptr);
    udata->obj_ptr = NULL;
    udata->lua_ref = LUA_NOREF;
    udata->was_allocated_by_lua = false;

    if (was_allocated_by_lua) {
        assert(destructor);
        destructor(&obj_ptr);
    }

    return 0;
}

bool luascript_userdata_is_allocated(lua_State* L, const char* check_metatable_name) {
    LuascriptObject* udata = luascript_internal_read_luascript_object(L, 1, check_metatable_name);

    if (!udata) return false;

    return udata->was_allocated_by_lua;
}


void luascript_register(lua_State* lua, const char* name, const lua_CFunction gc, const lua_CFunction tostring, const luaL_Reg fns[]) {
    luaL_Reg OBJECT_METAMETHODS[] = {
        {"__gc", gc},
        {"__tostring", tostring},
        //{"__eq", luascript_internal_is_userdata_equals},
        {NULL, NULL}
    };

    lua_newtable(lua);
    luaL_setfuncs(lua, fns, 0);
    lua_pushvalue(lua, -1);
    lua_setglobal(lua, name);

    luaL_newmetatable(lua, name);

    luaL_setfuncs(lua, OBJECT_METAMETHODS, 0);

    lua_pushliteral(lua, "__index");
    lua_pushvalue(lua, -3);
    lua_rawset(lua, -3);

    lua_pushliteral(lua, "__metatable");
    lua_pushvalue(lua, -3);
    lua_rawset(lua, -3);

    lua_pop(lua, 1);
}


void* luascript_parse_and_allocate_modding_value(lua_State* L, int idx, ModdingValueType* output_type, bool throw_error) {
    void* ptr;

    switch (lua_type(L, idx)) {
        case LUA_TNONE:
        case LUA_TNIL:
            ptr = NULL;
            *output_type = ModdingValueType_null;
            break;
        case LUA_TBOOLEAN:
            ptr = malloc_chk(sizeof(bool));
            *((bool*)ptr) = lua_toboolean(L, idx) != 0;
            *output_type = ModdingValueType_boolean;
            break;
        case LUA_TSTRING:
            ptr = luascript_internal_get_string_copy(L, idx, NULL);
            *output_type = ModdingValueType_string;
            break;
        case LUA_TNUMBER:
            ptr = malloc_chk(sizeof(float64));
            *((float64*)ptr) = lua_tonumber(L, idx);
            *output_type = ModdingValueType_double;
            break;
        default:
            if (throw_error) {
                luaL_error(L, "invalid value type at %i, expected: string, nil, number or boolean", idx);
                return NULL;
            }

            printf(
                "luascript_parse_and_allocate_modding_value() "
                "invalid value type at %i, expected: string, nil, number or boolean",
                idx
            );
            ptr = NULL;
            break;
    }

    return ptr;
}

int luascript_push_modding_value(lua_State* L, bool deallocate, ModdingValueType type, void* value) {
    union {
        bool as_boolean;
        float64 as_double;
        void* as_pointer;
    } val;

    val.as_pointer = value;

    switch (type) {
        default:
        case ModdingValueType_null:
            lua_pushnil(L);
            break;
        case ModdingValueType_boolean:
            lua_pushboolean(L, val.as_boolean);
            break;
        case ModdingValueType_string:
            lua_pushstring(L, (char*)value);
            break;
        case ModdingValueType_double:
            lua_pushnumber(L, (LUA_NUMBER)val.as_double);
            break;
    }

    if (deallocate && value) free_chk(value);

    return 1;
}

static int luascript_handle_error(lua_State* L) {
    const char* msg = lua_tostring(L, -1);
    luaL_traceback(L, L, msg, 2);
    lua_remove(L, -2);

    // keep the error (message & traceback)  in the stack
    return 1;
}


void luascript_change_working_folder(lua_State* L) {
    const char* working_folder = luascript_get_instance(L)->working_folder;
    if (working_folder) {
        fs_folder_stack_push();
        fs_set_working_folder(working_folder, false);
    }
}

void luascript_restore_working_folder(lua_State* L) {
    const char* working_folder = luascript_get_instance(L)->working_folder;
    if (working_folder) {
        fs_folder_stack_pop();
    }
}


int luascript_pcallk(lua_State* L, int arguments_count, int results_count) {
    // push error handler
    lua_pushcfunction(L, luascript_handle_error);
    lua_insert(L, 1);

    int result = lua_pcallk(L, arguments_count, results_count, 1, 0, NULL);

    // remove error handler
    lua_remove(L, 1);

    return result;
}


const char* luascript_helper_enums_stringify(const LuascriptEnums source, const int32_t value) {
    assert(source);

    for (const LuascriptEnumsPair* ptr = source->pairs; ptr->name; ptr++) {
        if (ptr->value == value) {
            return ptr->name;
        }
    }

    // logger_warn("unknown enum value %ul for %s\n", value, source->enum_name);

    // default name (this never should happen)
    return source->pairs[0].name;
}

int32_t luascript_helper_optenum(lua_State* L, int idx, const LuascriptEnums source) {
    const char* unparsed_value = luaL_optstring(L, idx, NULL);

    if (string_is_empty(unparsed_value)) {
        if (source->reject_on_null_or_empty)
            return luaL_error(L, "the %s enum must have a value", source->enum_name);
        else
            return source->pairs[0].value; // default value
    }

    for (const LuascriptEnumsPair* ptr = source->pairs; ptr->name; ptr++) {
        if (string_equals(ptr->name, unparsed_value)) {
            return ptr->value;
        }
    }

    return luaL_error(L, "invalid %s enum value: %s", source->enum_name, unparsed_value);
}

void luascript_helper_pushenum(lua_State* L, const LuascriptEnums source, const int32_t value) {
    const char* string_value = luascript_helper_enums_stringify(source, value);
    lua_pushstring(L, string_value);
}



void luascript_helper_add_field_integer(lua_State* L, const char* name, const lua_Integer integer) {
    lua_pushinteger(L, integer);
    lua_setfield(L, -2, name);
}

void luascript_helper_add_field_string(lua_State* L, const char* name, const char* string) {
    lua_pushstring(L, string);
    lua_setfield(L, -2, name);
}

void luascript_helper_add_field_number(lua_State* L, const char* name, const lua_Number number) {
    lua_pushnumber(L, number);
    lua_setfield(L, -2, name);
}

void luascript_helper_add_field_boolean(lua_State* L, const char* name, const bool boolean) {
    lua_pushboolean(L, boolean);
    lua_setfield(L, -2, name);
}

void luascript_helper_add_field_null(lua_State* L, const char* name) {
    lua_pushnil(L);
    lua_setfield(L, -2, name);
}

void luascript_helper_add_field_table(lua_State* L, const char* name, const int32_t narr, const int32_t nrec) {
    lua_pushstring(L, name);
    lua_createtable(L, (int)narr, (int)nrec);
}

void luascript_helper_add_field_enum(lua_State* L, const char* name, const LuascriptEnums source, const int32_t value) {
    luascript_helper_pushenum(L, source, value);
    lua_setfield(L, -2, name);
}

void luascript_helper_add_field_array_item_as_table(lua_State* L, const int32_t narr, const int32_t nrec, const int32_t index_in_table) {
    lua_pushinteger(L, (lua_Integer)index_in_table);
    lua_createtable(L, (int)narr, (int)nrec);
}


lua_Integer luascript_helper_get_field_integer(lua_State* L, const int32_t idx, const char* name, const lua_Integer def_value) {
    lua_getfield(L, idx, name);
    lua_Integer ret = luaL_optinteger(L, lua_gettop(L), def_value);
    lua_pop(L, 1);
    return ret;
}

char* luascript_helper_get_field_string(lua_State* L, const int32_t idx, const char* name, const char* def_value) {
    lua_getfield(L, idx, name);
    char* ret = luascript_internal_get_string_copy(L, lua_gettop(L), def_value);
    lua_pop(L, 1);
    return ret;
}

int32_t luascript_helper_get_field_align(lua_State* L, const int32_t idx, const char* name, const int32_t def_value) {
    lua_getfield(L, idx, name);

    int32_t ret = luascript_helper_optenum(L, lua_gettop(L), LUASCRIPT_ENUMS_Align);

    if (ret == LUASCRIPT_ENUMS_Align->pairs[0].value) {
        ret = def_value;
    }

    lua_pop(L, 1);
    return ret;
}

lua_Number luascript_helper_get_field_number(lua_State* L, const int32_t idx, const char* name, const lua_Number def_value) {
    lua_getfield(L, idx, name);
    lua_Number ret = luaL_optnumber(L, lua_gettop(L), def_value);
    lua_pop(L, 1);
    return ret;
}

bool luascript_helper_get_field_boolean(lua_State* L, const int32_t idx, const char* name, const bool def_value) {
    int type = lua_getfield(L, idx, name);

    bool ret;
    if (type == LUA_TNONE)
        ret = def_value;
    else
        ret = lua_toboolean(L, lua_gettop(L)) ? true : false;

    lua_pop(L, 1);
    return ret;
}

bool luascript_helper_get_field_table(lua_State* L, const int32_t idx, const char* name) {
    int type = lua_getfield(L, idx, name);
    if (type == LUA_TTABLE) return true;

    lua_pop(L, 1);
    return false;
}

bool luascript_helper_get_array_item_as_table(lua_State* L, const int32_t idx, const int32_t index_in_table) {
    // this performs "local item = array[index_in_table]"
    lua_pushinteger(L, (lua_Integer)index_in_table);
    int type = lua_gettable(L, idx);

    if (type == LUA_TTABLE) return true;

    lua_pop(L, 1);
    return false;
}

bool luascript_helper_has_table_field(lua_State* L, const int32_t idx, const char* name, const int desired_type) {
    int type = lua_getfield(L, idx, name);
    lua_pop(L, 1);
    return type == desired_type;
}


nbool luascript_helper_optnbool(lua_State* L, int idx) {
    switch (lua_type(L, idx)) {
        case LUA_TNONE:
        case LUA_TNIL:
            return unset;
        default:
            return lua_toboolean(L, idx);
    }
}

void luascript_helper_pushnbool(lua_State* L, nbool value) {
    if (value == unset)
        lua_pushnil(L);
    else if (value)
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);
}
