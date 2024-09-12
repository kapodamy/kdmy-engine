

/**
 * @typedef {object} AsyncPromiseHandlers
 * @property {function(any):void} resolve
 * @property {function(any):void} reject
 * @property {boolean?} kdmyEngine_allocated
 */
/**
 * @typedef {object} Asyncify
 * @property {number} callStackId
 * @property {Object.<number, string>} callStackIdToName
 * @property {AsyncPromiseHandlers} asyncPromiseHandlers
*/

/**
 * @typedef {object} LuaStringBuffer
 * @property {number} pointer
 * @property {number} written
 * @property {number} available
 */

/**
 * @callback lua_CFunction
 * @param {lua_State} L
 * @returns {number|Promise<number>}
 */

/**
 * @typedef {[string, lua_CFunction]} luaL_Reg
 */

/**
 * @typedef {object} lua_State
 * @property {number} ptr
 * @property {Map<!object, number>} objectMap
 * @property {Map<!Function, number>} functionMap
 * @property {number} objectIndexes
 */

/**
 * @typedef {object} LUA
 * 
 * 
 * @property {(function(lua_State,number):!object) | (function(lua_State,!object):number)} kdmyEngine_obtain
 * @property {function(!lua_State, number): void} kdmyEngine_forget
 * @property {function():undefined|Promise} kdmyEngine_yieldAsync
 * 
 * @property {function(lua_State, number):string} luaL_checkstring
 * @property {function(lua_State, number, string):string} luaL_optstring
 * @property {function(lua_State, string):void} LUA.lua_pushstring
 * @property {function(lua_State, string):number} luaL_newmetatable
 * @property {function(lua_State, lua_CFunction):void} lua_pushcfunction
 * @property {function(lua_State, number, string):void} lua_setfield
 * @property {function(lua_State, boolean):void} lua_pushboolean
 * @property {function(lua_State, string):number} luaL_error
 * @property {function(lua_State, number):number} luaL_checknumber
 * @property {function(lua_State, number):number} luaL_checkinteger
 * @property {function(lua_State, number):boolean} lua_toboolean
 * @property {function(lua_State, number, number):void} lua_createtable
 * @property {function(lua_State, number):void} lua_pushnumber
 * @property {function(lua_State):void} lua_pushnil
 * @property {function(lua_State, number):void} lua_pushinteger
 * @property {function(lua_State, number, number):number} luaL_optnumber
 * @property {function(lua_State, number, number):number} luaL_optinteger
 * @property {function(lua_State, number):boolean} lua_isnil
 * @property {function(lua_State, number, string):object} luaL_testudata
 * @property {function(lua_State, number):void} lua_pop
 * @property {function(lua_State, number, number):void} lua_rawseti
 * @property {function(lua_State, string):void} lua_pushfstring
 * @property {function(lua_State, ArrayBuffer, number): void} lua_pushlstring
 * @property {function(lua_State, string): void} LUA.lua_setglobal
 * @property {function(lua_State, number): void} lua_settable
 * @property {function(lua_State, number): void} lua_pushvalue
 * @property {function(lua_State): number} lua_gettop
 * @property {function(lua_State, string): number} luaL_dostring
 * @property {function(lua_State, number, string): number} lua_getfield
 * @property {function(lua_State, number): number} lua_gettable
 * @property {function(lua_State, number): number} lua_rawlen
 * @property {function(lua_State, number): string} lua_tostring
 * @property {function(lua_State, number, number[]): ArrayBuffer} luaL_checklstring
 * @property {function(lua_State, number, string): object} luaL_checkudata
 * @property {function(lua_State, number): object} lua_newuserdata
 * @property {function(lua_State, string): number} luaL_getmetatable
 * @property {function(lua_State, number): void} lua_setmetatable
 * @property {function(lua_State, number): number} luaL_ref
 * @property {function(lua_State, number, number): void} luaL_unref
 * @property {function(lua_State, object): void} lua_pushlightuserdata
 * @property {function(lua_State, number): object} lua_touserdata
 * @property {function(lua_State):void } lua_newtable
 * @property {function(lua_State, luaL_Reg[], 0):void } luaL_setfuncs
 * @property {function(lua_State, string):void } lua_pushliteral
 * @property {function(lua_State, number):void } lua_rawset
 * @property {function(lua_State, number):number } lua_type
 * @property {function(lua_State, number):number } lua_tonumber
 * @property {function(lua_State, number):void } lua_remove
 * @property {function(lua_State, lua_State, string, number):void } luaL_traceback
 * @property {function(lua_State, number):void} lua_insert
 * @property {function(lua_State,number,number, number, 0, null):number} lua_pcallk
 * @property {function(lua_State, string):number} lua_getglobal
 * @property {function():lua_State} luaL_newstate
 * @property {function(lua_State):void} luaL_openlibs
 * @property {function(lua_State, function(object, string, number):void, null):void} lua_setwarnf
 * @property {function(lua_State, string, number, string, string):number} luaL_loadbufferx
 * @property {function(lua_State):void} lua_close
 * @property {function(lua_State, string):number} luaL_loadstring
 *
 * @property {number} LUA_OK
 * @property {number} LUA_TNONE
 * @property {number} LUA_TTABLE
 * @property {number} LUA_TNUMBER
 * @property {number} LUA_NOREF
 * @property {number} LUA_REFNIL
 * @property {number} LUA_REGISTRYINDEX
 * @property {number} LUA_TNIL
 * @property {number} LUA_TBOOLEAN
 * @property {number} LUA_TSTRING
 * @property {number} LUA_TFUNCTION
 * @property {number} LUA_MULTRET
 */



/**@type {Asyncify} */
var Asyncify;

/**@type {Uint8Array} */
var HEAPU8;

/**@type {WebAssembly.Memory} */
var wasmMemory;

var Module = {};

/**
 * @param {function} fn 
 * @param {string} sig 
 * @returns {number}
 */
function addFunction(fn, sig) { return 0; }

/**
 * @param {number} fn_ptr
 */
function removeFunction(fn_ptr) { }

/**
 * @param {number} sz 
 * @returns {number}
 */
function _malloc(sz) { return 1; }

/**
 * @param {number} ptr 
 * @param {number} sz 
 * @returns {number}
 */
function _realloc(ptr, sz) { return 2; }

/**
 * @param {number} ptr 
 */
function _free(ptr) { }


function _luaL_checkstring(/**@type {number[]} */...stub) { return 0; }
function _luaL_optstring(/**@type {number[]} */...stub) { return 0; }
function _lua_pushstring(/**@type {number[]} */...stub) { return 0; }
function _luaL_newmetatable(/**@type {number[]} */...stub) { return 0; }
function _lua_pushcfunction(/**@type {number[]} */...stub) { return 0; }
function _lua_setfield(/**@type {number[]} */...stub) { return 0; }
function _lua_pushboolean(/**@type {number[]} */...stub) { return 0; }
function _luaL_error(/**@type {number[]} */...stub) { return 0; }
function _luaL_checknumber(/**@type {number[]} */...stub) { return 0; }
function _luaL_checkinteger(/**@type {number[]} */...stub) { return 0; }
function _lua_toboolean(/**@type {number[]} */...stub) { return 0; }
function _lua_createtable(/**@type {number[]} */...stub) { return 0; }
function _lua_pushnumber(/**@type {number[]} */...stub) { return 0; }
function _lua_pushnil(/**@type {number[]} */...stub) { return 0; }
function _lua_pushinteger(/**@type {number[]} */...stub) { return 0; }
function _luaL_optnumber(/**@type {number[]} */...stub) { return 0; }
function _luaL_optinteger(/**@type {number[]} */...stub) { return 0; }
function _lua_isnil(/**@type {number[]} */...stub) { return 0; }
function _luaL_testudata(/**@type {number[]} */...stub) { return 0; }
function _lua_pop(/**@type {number[]} */...stub) { return 0; }
function _lua_rawseti(/**@type {number[]} */...stub) { return 0; }
function _lua_pushfstring(/**@type {number[]} */...stub) { return 0; }
function _lua_pushlstring(/**@type {number[]} */...stub) { return 0; }
function _lua_setglobal(/**@type {number[]} */...stub) { return 0; }
function _lua_settable(/**@type {number[]} */...stub) { return 0; }
function _lua_pushvalue(/**@type {number[]} */...stub) { return 0; }
function _lua_gettop(/**@type {number[]} */...stub) { return 0; }
function _luaL_dostring(/**@type {number[]} */...stub) { return 0; }
function _lua_getfield(/**@type {number[]} */...stub) { return 0; }
function _lua_gettable(/**@type {number[]} */...stub) { return 0; }
function _lua_rawlen(/**@type {number[]} */...stub) { return 0; }
function _lua_tostring(/**@type {number[]} */...stub) { return 0; }
function _luaL_checklstring(/**@type {number[]} */...stub) { return 0; }
function _luaL_checkudata(/**@type {number[]} */...stub) { return 0; }
function _lua_newuserdata(/**@type {number[]} */...stub) { return 0; }
function _luaL_getmetatable(/**@type {number[]} */...stub) { return 0; }
function _lua_setmetatable(/**@type {number[]} */...stub) { return 0; }
function _luaL_ref(/**@type {number[]} */...stub) { return 0; }
function _luaL_unref(/**@type {number[]} */...stub) { return 0; }
function _lua_pushlightuserdata(/**@type {number[]} */...stub) { return 0; }
function _lua_touserdata(/**@type {number[]} */...stub) { return 0; }
function _lua_newtable(/**@type {number[]} */...stub) { return 0; }
function _luaL_setfuncs(/**@type {number[]} */...stub) { return 0; }
function _lua_pushliteral(/**@type {number[]} */...stub) { return 0; }
function _lua_rawset(/**@type {number[]} */...stub) { return 0; }
function _lua_type(/**@type {number[]} */...stub) { return 0; }
function _lua_tonumber(/**@type {number[]} */...stub) { return 0; }
function _lua_remove(/**@type {number[]} */...stub) { return 0; }
function _luaL_traceback(/**@type {number[]} */...stub) { return 0; }
function _lua_insert(/**@type {number[]} */...stub) { return 0; }
function _lua_pcallk(/**@type {number[]} */...stub) { return 0; }
function _lua_getglobal(/**@type {number[]} */...stub) { return 0; }
function _luaL_newstate(/**@type {number[]} */...stub) { return 0; }
function _luaL_openlibs(/**@type {number[]} */...stub) { return 0; }
function _lua_setwarnf(/**@type {number[]} */...stub) { return 0; }
function _luaL_loadbufferx(/**@type {number[]} */...stub) { return 0; }
function _lua_close(/**@type {number[]} */...stub) { return 0; }
function _luaL_loadstring(/**@type {number[]} */...stub) { return 0; }
function _lua_pushcclosure(/**@type {number[]} */...stub) { return 0; }
function _luaL_optlstring(/**@type {number[]} */...stub) { return 0; }
function _lua_tonumberx(/**@type {number[]} */...stub) { return 0; }
function _lua_tolstring(/**@type {number[]} */...stub) { return 0; }
function _lua_settop(/**@type {number[]} */...stub) { return 0; }
function _lua_rotate(/**@type {number[]} */...stub) { return 0; }
function _lua_newuserdatauv(/**@type {number[]} */...stub) { return null; }
