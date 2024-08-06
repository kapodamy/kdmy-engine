"use strict";

// in C use "&luascript_get_instance()" instead
const luascript_key_ptr = function () { };


/**
 * @param {Luascript} luascript
 * @param {object} obj
 * @returns {number}
 */
function luascript_internal_get_lua_reference(luascript, obj) {
    const shared_array = luascript.shared_array;
    const shared_size = luascript.shared_size;

    for (let i = 0; i < shared_size; i++) {
        if (shared_array[i].obj_ptr == obj) {
            return shared_array[i].lua_ref;
        }
    }

    return LUA.LUA_NOREF;
}

/**
 *
 * @param {Luascript} luascript
 * @param {object} obj
 * @param {number} ref
 * @param {boolean} allocated
 * @returns {LuascriptObject}
 */
function luascript_internal_add_lua_reference(luascript, obj, ref, allocated) {
    let index;

    // find and empty slot
    const shared_array = luascript.shared_array;
    const shared_size = luascript.shared_size;

    L_find_empty_slot: {
        for (let i = 0; i < shared_size; i++) {
            if (shared_array[i].obj_ptr == null) {
                index = i;
                break L_find_empty_slot;
            }
        }

        // no empty slots, grow the array and add it
        index = luascript.shared_size;
        luascript.shared_size += LUASCRIPT_SHARED_ARRAY_CHUNK_SIZE;
        luascript.shared_array = realloc_for_array(luascript.shared_array, luascript.shared_size);

        for (let i = 0; i < LUASCRIPT_SHARED_ARRAY_CHUNK_SIZE; i++) {
            luascript.shared_array[i + index] = {
                lua_ref: LUA.LUA_NOREF,
                obj_ptr: null,
                was_allocated_by_lua: false,
            };
        }
    }

    luascript.shared_array[index].obj_ptr = obj;
    luascript.shared_array[index].lua_ref = ref;
    luascript.shared_array[index].was_allocated_by_lua = allocated;

    return luascript.shared_array[index];
}

/**
 * @param {Luascript} luascript
 * @param {object} obj
 * @returns {number}
 */
function luascript_internal_remove_lua_reference(luascript, obj) {
    let shared_array = luascript.shared_array;
    const shared_size = luascript.shared_size;

    for (let i = 0; i < shared_size; i++) {
        if (shared_array[i].obj_ptr == obj) {
            let ref = shared_array[i].lua_ref;
            shared_array[i].obj_ptr = null;
            shared_array[i].lua_ref = LUA.LUA_REFNIL;
            shared_array[i].was_allocated_by_lua = false;
            return ref;
        }
    }

    return LUA.LUA_NOREF;
}

/**
 * @param {lua_State} L
 * @param {number} ud
 * @param {string} tname
 * @returns {LuascriptObject}
 */
function luascript_internal_read_luascript_object(L, ud, tname) {
    let udata = LUA.luaL_checkudata(L, ud, tname);

    if (!udata || !udata.obj_ptr || udata.lua_ref == LUA.LUA_REFNIL || udata.lua_ref == LUA.LUA_NOREF) {
        return null;
    }

    return udata;
}

/**
 * @param {lua_State} L
 * @param {number} ud
 * @param {string} tname
 * @returns {object}
 */
function luascript_internal_read_userdata(L, ud, tname) {
    let udata = luascript_internal_read_luascript_object(L, ud, tname);

    if (!udata) {
        LUA.luaL_error(L, `${tname} object was destroyed.`);
        return null;
    }

    return udata.obj_ptr;
}

/**
 * @param {lua_State} L
 * @returns {number}
 */
/*function luascript_internal_is_userdata_equals(L) {
    let equals;

    let type_a = LUA.lua_type(L, 1);
    let type_b = LUA.lua_type(L, 2);

    if (type_a != LUA.LUA_TUSERDATA || type_b != LUA.LUA_TUSERDATA) {
        equals = false;
    } else {
        let a = LUA.lua_touserdata(L, 1);
        let b = LUA.lua_touserdata(L, 2);

        equals = (a == null && b == null) || (a != b);
    }

    LUA.lua_pushboolean(L, equals);

    return 1;
}*/


/**
 * @param {lua_State} L
 * @param {number} idx
 * @param {string} def
 * @returns {string}
 */
function luascript_internal_get_string_copy(L, idx, def) {
    let str = LUA.luaL_optstring(L, idx, def);
    return strdup(str);
}


/**
 * @param {Luascript} luascript
 * @param {object} obj
 * @param {string} metatable_name
 * @param {boolean} allocated
 * @returns {number}
 */
function luascript_create_userdata(luascript, obj, metatable_name, allocated) {
    let L = luascript.L;

    if (!obj) {
        LUA.lua_pushnil(L);
        return 1;
    }

    let ref = luascript_internal_get_lua_reference(luascript, obj);
    if (ref != LUA.LUA_NOREF) {
        // recover userdata back into stack
        LUA.lua_pushinteger(L, ref);
        LUA.lua_gettable(L, LUA.LUA_REGISTRYINDEX);
    } else {
        /**@type {LuascriptObject} */
        let udata = LUA.lua_newuserdata(L, 12);//lua_newuserdata(L, sizeof(LuascriptObject));
        console.assert(udata, "can not create the lua userdata");

        LUA.luaL_getmetatable(L, metatable_name);
        LUA.lua_setmetatable(L, -2);

        LUA.lua_pushvalue(L, -1);
        ref = LUA.luaL_ref(L, LUA.LUA_REGISTRYINDEX);

        udata.lua_ref = ref;
        udata.obj_ptr = obj;
        udata.was_allocated_by_lua = allocated;

        luascript_internal_add_lua_reference(luascript, obj, ref, allocated);
    }

    return 1;
}

/**
 * @param {Luascript} luascript
 * @param {object} obj
 * @returns {void}
 */
function luascript_remove_userdata(luascript, obj) {
    console.assert(obj, "obj can not be null");

    let L = luascript.L;

    // aquire lua reference and nullify the object
    let ref = luascript_internal_remove_lua_reference(luascript, obj);
    if (ref == LUA.LUA_NOREF) return;

    // LUA.lua_pushinteger(L, ref);
    // LUA.lua_pushnil(L);
    // LUA.lua_settable(L, LUA.LUA_REGISTRYINDEX);
    // return;

    // (JS only) allow object to get garbage collected
    LUA.kdmyEngine_forget(L, obj);

    // remove from lua registry
    LUA.luaL_unref(L, LUA.LUA_REGISTRYINDEX, ref);
}

/**
 * @param {lua_State} L
 * @param {string} check_metatable_name
 * @returns {object}
 */
function luascript_read_userdata(L, check_metatable_name) {
    if (LUA.lua_isnil(L, 1)) {
        LUA.luaL_error(L, `${check_metatable_name} was null (nil in lua).`);
        return null;
    }

    return luascript_internal_read_userdata(L, 1, check_metatable_name);
}

/**
 * @param {lua_State} L
 * @param {number} idx
 * @param {string} check_metatable_name
 * @returns {object}
 */
function luascript_read_nullable_userdata(L, idx, check_metatable_name) {
    if (LUA.lua_isnil(L, idx))
        return null;
    else
        return luascript_internal_read_userdata(L, idx, check_metatable_name);
}


/**
 * @param {lua_State} L
 * @returns {Luascript}
 */
function luascript_get_instance(L) {
    LUA.lua_pushlightuserdata(L, luascript_key_ptr);
    LUA.lua_gettable(L, LUA.LUA_REGISTRYINDEX);

    /**@type {Luascript}*/let luascript = LUA.lua_touserdata(L, -1);
    console.assert(luascript, "luascript instance was null");
    LUA.lua_pop(L, 1);

    return luascript;
}

/**
 * @param {Luascript} luascript
 */
function luascript_set_instance(luascript) {
    let L = luascript.L;

    LUA.lua_pushlightuserdata(L, luascript_key_ptr);
    LUA.lua_pushlightuserdata(L, luascript);
    LUA.lua_settable(L, LUA.LUA_REGISTRYINDEX);
}


/**
 * @param {lua_State} L
 * @param {string} check_metatable_name
 * @param {object} obj
 * @returns {number}
 */
function luascript_userdata_new(L, check_metatable_name, obj) {
    return luascript_create_userdata(luascript_get_instance(L), obj, check_metatable_name, false);
}

/**
 * @param {lua_State} L
 * @param {string} check_metatable_name
 * @param {object} obj
 * @returns {number}
 */
function luascript_userdata_allocnew(L, check_metatable_name, obj) {
    return luascript_create_userdata(luascript_get_instance(L), obj, check_metatable_name, true);
}

/**
 * @param {lua_State} L
 * @param {string} check_metatable_name
 * @returns {number}
 */
function luascript_userdata_tostring(L, check_metatable_name) {
    let udata = luascript_read_userdata(L, check_metatable_name);

    // javascript only
    udata = LUA.kdmyEngine_obtain(L, udata).toString(16);

    LUA.lua_pushfstring(L, `{${check_metatable_name} 0x${udata}`);
    return 1;
}

/**
 * @param {lua_State} L
 * @param {string} check_metatable_name
 * @returns {number}
 */
function luascript_userdata_gc(L, check_metatable_name) {
    let udata = luascript_read_userdata(L, check_metatable_name);
    luascript_remove_userdata(luascript_get_instance(L), udata);
    return 0;
}

/**
 * @param {lua_State} L
 * @param {string} check_metatable_name
 * @param {Destructor} destructor
 * @returns {number}
 */
function luascript_userdata_destroy(L, check_metatable_name, destructor) {
    let udata = luascript_internal_read_luascript_object(L, 1, check_metatable_name);

    if (!udata) return 0;

    let obj_ptr = udata.obj_ptr;
    let was_allocated_by_lua = udata.was_allocated_by_lua;

    luascript_remove_userdata(luascript_get_instance(L), obj_ptr);
    udata.obj_ptr = null;
    udata.lua_ref = LUA.LUA_NOREF;
    udata.was_allocated_by_lua = false;

    if (was_allocated_by_lua) {
        console.assert(destructor, "destructor callback was null");
        destructor(obj_ptr);
    }

    return 0;
}

/**
 * @param {lua_State} L
 * @param {string} check_metatable_name
 * @returns {boolean}
 */
function luascript_userdata_is_allocated(L, check_metatable_name) {
    let udata = luascript_internal_read_luascript_object(L, 1, check_metatable_name);

    if (!udata) return false;

    return udata.was_allocated_by_lua;
}

/**
 * @param {lua_State} lua
 * @param {string} name
 * @param {lua_CFunction} gc
 * @param {lua_CFunction} tostring
 * @param {luaL_Reg[]} fns
 */
function luascript_register(lua, name, gc, tostring, fns) {
    /**@type {luaL_Reg[]}*/
    const OBJECT_METAMETHODS = [
        ["__gc", gc],
        ["__tostring", tostring],
        //["__eq", luascript_internal_is_userdata_equals],
        [null, null]
    ];

    LUA.lua_newtable(lua);
    LUA.luaL_setfuncs(lua, fns, 0);
    LUA.lua_pushvalue(lua, -1);
    LUA.lua_setglobal(lua, name);

    LUA.luaL_newmetatable(lua, name);

    LUA.luaL_setfuncs(lua, OBJECT_METAMETHODS, 0);

    LUA.lua_pushliteral(lua, "__index");
    LUA.lua_pushvalue(lua, -3);
    LUA.lua_rawset(lua, -3);

    LUA.lua_pushliteral(lua, "__metatable");
    LUA.lua_pushvalue(lua, -3);
    LUA.lua_rawset(lua, -3);

    LUA.lua_pop(lua, 1);
}

/**
 * @param {lua_State} L
 * @param {number} idx
 * @param {boolean} throw_error
 * @returns {string|boolean|number|null}
 */
function luascript_parse_and_allocate_modding_value(L, idx, throw_error) {
    let value;

    switch (LUA.lua_type(L, idx)) {
        case LUA.LUA_TNONE:
        case LUA.LUA_TNIL:
            value = null;
            break;
        case LUA.LUA_TBOOLEAN:
            value = LUA.lua_toboolean(L, idx);
            break;
        case LUA.LUA_TSTRING:
            value = luascript_internal_get_string_copy(L, idx, null);
            break;
        case LUA.LUA_TNUMBER:
            value = LUA.lua_tonumber(L, idx);
            break;
        default:
            if (throw_error) {
                LUA.luaL_error(L, `invalid value type at ${idx}, expected: string, nil, number or boolean`,);
                return null;
            }

            console.error(
                "luascript_parse_and_allocate_modding_value() " +
                `invalid value type at ${idx}, expected: string, nil, number or boolean`
            );
            value = null;
            break;
    }

    return value;
}

/**
 * @param {lua_State} L
 * @param {boolean} deallocate
 * @param {string|boolean|number|null} value
 * @returns {number}
 */
function luascript_push_modding_value(L, deallocate, value) {
    switch (typeof value) {
        default:
        case "object":
            if (value == null) LUA.lua_pushnil(L);
            break;
        case "boolean":
            LUA.lua_pushboolean(L, value);
            break;
        case "string":
            LUA.lua_pushstring(L, value);
            break;
        case "number":
            LUA.lua_pushnumber(L, value);
            break;
    }

    if (deallocate) value = undefined;

    return 1;
}

/**
 * @param {lua_State} L
 */
function luascript_handle_error(L) {
    let msg = LUA.lua_tostring(L, -1);
    LUA.luaL_traceback(L, L, msg, 2);
    LUA.lua_remove(L, -2);

    // keep the error (message & traceback)  in the stack
    return 1;
}


/**
 * @param {lua_State} L
 */
function luascript_change_working_folder(L) {
    let working_folder = luascript_get_instance(L).working_folder;
    if (working_folder) {
        fs_folder_stack_push();
        fs_set_working_folder(working_folder, false);
    }
}

/**
 * @param {lua_State} L
 */
function luascript_restore_working_folder(L) {
    let working_folder = luascript_get_instance(L).working_folder;
    if (working_folder) {
        fs_folder_stack_pop();
    }
}


/**
 * @param {lua_State} L
 * @param {number} arguments_count
 * @param {number} results_count
 * @returns {Promise<number>}
 */
async function luascript_pcallk(L, arguments_count, results_count) {
    // push error handler
    LUA.lua_pushcfunction(L, luascript_handle_error);
    LUA.lua_insert(L, 1);

    let result = LUA.lua_pcallk(L, arguments_count, results_count, 1, 0, null);
    result = await LUA_check_async(result);

    // remove error handler
    LUA.lua_remove(L, 1);

    return result;
}


/**
 * @param {LuascriptEnums} source
 * @param {number} value
 * @returns {string}
 */
function luascript_helper_enums_stringify(source, value) {
    console.assert(source, "enums source can not be null");

    for (let ptr of source.pairs) {
        if (ptr.value == value) {
            return ptr.name;
        }
    }

    console.warn(`unknown enum value ${value} for ${source.enum_name}`);

    // default name (this never should happen)
    return source.pairs[0].name;
}

/**
 * @param {lua_State} L
 * @param {number} idx
 * @param {LuascriptEnums} source
 * @returns {number}
 */
function luascript_helper_optenum(L, idx, source) {
    let unparsed_value = LUA.luaL_optstring(L, idx, null);

    if (!unparsed_value) {
        if (source.reject_on_null_or_empty)
            return LUA.luaL_error(L, `the ${source.enum_name} enum must have a value`);
        else
            return source.pairs[0].value; // default value
    }

    for (let ptr of source.pairs) {
        if (!ptr.name) break;

        if (ptr.name == unparsed_value) {
            return ptr.value;
        }
    }

    return LUA.luaL_error(L, `invalid ${source.enum_name} enum value: ${unparsed_value}`);
}

/**
 * @param {lua_State} L
 * @param {LuascriptEnums} source
 * @param {number} value
 */
function luascript_helper_pushenum(L, source, value) {
    let string_value = luascript_helper_enums_stringify(source, value);
    LUA.lua_pushstring(L, string_value);
}



/**
 * @template T
 * @param {lua_State} L
 * @param {string} field_name
 * @param {function(lua_State, T):void} setter
 * @param {T} value
 */
function luascript_helper_add_table_field(L, field_name, setter, value) {
    LUA.lua_pushstring(L, field_name);
    setter(L, value);
    LUA.lua_settable(L, -3);
}

function luascript_helper_add_field_integer(L, name, integer) {
    LUA.lua_pushinteger(L, integer);
    LUA.lua_setfield(L, -2, name);
}

function luascript_helper_add_field_string(L, name, string) {
    LUA.lua_pushstring(L, string);
    LUA.lua_setfield(L, -2, name);
}

function luascript_helper_add_field_number(L, name, number) {
    LUA.lua_pushnumber(L, number);
    LUA.lua_setfield(L, -2, name);
}

function luascript_helper_add_field_boolean(L, name, boolean) {
    LUA.lua_pushboolean(L, boolean);
    LUA.lua_setfield(L, -2, name);
}

function luascript_helper_add_field_null(L, name) {
    LUA.lua_pushnil(L);
    LUA.lua_setfield(L, -2, name);
}

function luascript_helper_add_field_table(L, name, narr, nrec) {
    LUA.lua_pushstring(L, name);
    LUA.lua_createtable(L, narr, nrec);
}

function luascript_helper_add_field_enum(L, name, source, value) {
    luascript_helper_pushenum(L, source, value);
    LUA.lua_setfield(L, -2, name);
}

function luascript_helper_add_field_array_item_as_table(L, narr, nrec, index_in_table) {
    LUA.lua_pushinteger(L, index_in_table);
    LUA.lua_createtable(L, narr, nrec);
}


function luascript_helper_get_field_integer(L, idx, name, def_value) {
    LUA.lua_getfield(L, idx, name);
    let ret = LUA.luaL_optinteger(L, LUA.lua_gettop(L), def_value);
    LUA.lua_pop(L, 1);
    return ret;
}

function luascript_helper_get_field_string(L, idx, name, def_value) {
    LUA.lua_getfield(L, idx, name);
    let ret = luascript_internal_get_string_copy(L, LUA.lua_gettop(L), def_value);
    LUA.lua_pop(L, 1);
    return ret;
}

function luascript_helper_get_field_align(L, idx, name, def_value) {
    LUA.lua_getfield(L, idx, name);

    let ret = luascript_helper_optenum(L, LUA.lua_gettop(L), LUASCRIPT_ENUMS_Align);

    if (ret == LUASCRIPT_ENUMS_Align.pairs[0].value) {
        ret = def_value;
    }

    LUA.lua_pop(L, 1);
    return ret;
}

function luascript_helper_get_field_number(L, idx, name, def_value) {
    LUA.lua_getfield(L, idx, name);
    let ret = LUA.luaL_optnumber(L, LUA.lua_gettop(L), def_value);
    LUA.lua_pop(L, 1);
    return ret;
}

function luascript_helper_get_field_boolean(L, idx, name, def_value) {
    let type = LUA.lua_getfield(L, idx, name);

    let ret;
    if (type == LUA.LUA_TNONE)
        ret = def_value;
    else
        ret = LUA.lua_toboolean(L, LUA.lua_gettop(L));

    LUA.lua_pop(L, 1);
    return ret;
}

function luascript_helper_get_field_table(L, idx, name) {
    let type = LUA.lua_getfield(L, idx, name);
    if (type == LUA.LUA_TTABLE) return true;

    LUA.lua_pop(L, 1);
    return false;
}

function luascript_helper_get_array_item_as_table(L, idx, index_in_table) {
    // this performs "local item = array[index_in_table]"
    LUA.lua_pushinteger(L, index_in_table);
    let type = LUA.lua_gettable(L, idx);

    if (type == LUA.LUA_TTABLE) return true;

    LUA.lua_pop(L, 1);
    return false;
}

function luascript_helper_has_table_field(L, idx, name, desired_type) {
    let type = LUA.lua_getfield(L, idx, name);
    LUA.lua_pop(L, 1);
    return type == desired_type;
}


/**
 * 
 * @param {lua_State} L 
 * @param {number} idx 
 * @returns {boolean|null}
 */
function luascript_helper_optnbool(L, idx) {
    switch (LUA.lua_type(L, idx)) {
        case LUA.LUA_TNONE:
        case LUA.LUA_TNIL:
            return null;
        default:
            return LUA.lua_toboolean(L, idx);
    }
}

/**
 * 
 * @param {lua_State} L 
 * @param {boolean|null} value 
 */
function luascript_helper_pushnbool(L, value) {
    if (value == null)
        LUA.lua_pushnil(L);
    else if (value)
        LUA.lua_pushboolean(L, true);
    else
        LUA.lua_pushboolean(L, false);
}
