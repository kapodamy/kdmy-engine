
/**
 * @typedef {object} ModuleFontAtlas
 * @property {function(boolean):void} enable_sdf
 * @property {function(ArrayBuffer, number):FontAtlas} init
 * @property {function(FontAtlas):void} destroy
 * @property {function(FontAtlas, number, number, Uint32Array):FontCharMap} atlas_build
 * @property {function(FontAtlas, number, number):FontCharMap} atlas_build_complete
 * @property {function(FontCharMap):void} atlas_destroy_texture_only
 * @property {function(FontCharMap):void} atlas_destroy
 * @property {function():string} get_version
 */

/**
 * @typedef {object} FontAtlas
 * @property {number} fontatlas_ptr
 * @property {number} font_data_ptr
 */

/**
 * @typedef {object} FontCharMap
 * @property {FontCharData[]} char_array
 * @property {number} char_array_size
 * @property {Uint8Array} texture 8bpp grayscale texture
 * @property {number} texture_width
 * @property {number} texture_height
 * @property {number} texture_byte_size
 * @property {number} ascender
 * @property {number} __ptr
 */

/**
 * @typedef {object} FontCharData
 * @property {number} codepoint
 * @property {number} offset_y
 * @property {number} offset_x
 * @property {number} advancex
 * @property {number} advancey
 * @property {number} width
 * @property {number} height
 * @property {FontCharDataKerning[]} kernings
 * @property {number} kernings_size
 * @property {FontCharDataAtlasEntry} atlas_entry
 * @property {boolean} has_atlas_entry
 */

/**
 * @typedef {object} FontCharDataAtlasEntry
 * @property {number} x
 * @property {number} y
*/

/**
 * @typedef {object} FontCharDataKerning
 * @property {number} codepoint
 * @property {number} x
 */


/**
 * @typedef {object} LUA
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
 * @property {function(lua_State, number, string): LuascriptObject} luaL_checkudata
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
 * @property {number} objectIndexes
 */

/**
 * @typedef {object} LuaStringBuffer
 * @property {number} pointer
 * @property {number} written
 * @property {number} available
 */


/**
 * @template T
 * @param {function} fn 
 * @param  {...any} args 
 * @returns {T|Promise<T>}
 */
function LUA_warp_function(fn, ...args) {
    if (!args[0]) return;

    let ret = fn.apply(null, args);
    let promise = LUA.kdmyEngine_yieldAsync();
    return promise === undefined ? ret : promise;
}

/**
 * @template T
 * @param {T} previous_returned_value
 * @returns {T|Promise<T>}
 */
function LUA_check_async(previous_returned_value) {
    let promise = LUA.kdmyEngine_yieldAsync();

    if (promise === undefined)
        return previous_returned_value;
    else
        return promise;
}

/** @type {ModuleFontAtlas} */
var FontAtlas;

/** @type {LUA} */
var LUA;

async function main_initialize_wasm_modules() {
    //
    // Dumb VSCode treat "lua.js" and "module_fontatlas.js"
    // files as javascript ES Modules
    //

    // LUA is not available when running layoutvisor
    if ("ModuleLua" in window) {
        // @ts-ignore
        LUA = await ModuleLua();
    }

    // @ts-ignore
    FontAtlas = await ModuleFontAtlas();

}
