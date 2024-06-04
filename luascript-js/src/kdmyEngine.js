
const kdmyEngine_MAX_INDEXES = 4096;
const kdmyEngine_textDecoder = new TextDecoder("UTF-8", { ignoreBOM: true, fatal: true });
const kdmyEngine_textEncoder = new TextEncoder();
var sizet_buffer_ptr = 0x00;
var strbuf1 = null;
var strbuf2 = null;
var strbuf_large = null;


function kdmyEngine_obtain(/**@type {lua_State}*/L, /**@type {number|object}*/key) {
    //if (key === undefined) console.warn("undefined value passed");
    if (key == null) return 0;

    if (typeof (key) == "number") {
        if (key == 0) return null;

        for (let [obj, idx] of L.objectMap) {
            if (key == idx) {
                if (HEAPU8[idx] == 0) {
                    throw new Error("forgotten object index " + key);
                }
                return obj;
            }
        }

        throw new Error("unknown object index " + key);
    }

    let index = L.objectMap.get(key);
    if (index !== undefined) return index;

    let new_index = -1;

    // find an available index
    for (let i = 0; i < kdmyEngine_MAX_INDEXES; i++) {
        let index = i + L.objectIndexes;

        if (HEAPU8[index] == 0) {
            new_index = index;
            break;
        }
    }

    if (new_index < 0) {
        throw new Error("the objectMap is full");
    }

    HEAPU8[index] = 1;
    L.objectMap.set(key, new_index);

    return new_index;
}
function kdmyEngine_forget(/**@type {lua_State}*/L, /**@type {number}*/target_id) {
    for (let [obj, idx] of L.objectMap) {
        if (idx == target_id) {
            if (HEAPU8[idx] == 0) {
                throw new Error("already forgotten object index " + target_id);
            }
            HEAPU8[idx] = 0;
            L.objectMap.delete(obj);
            return;
        }
    }
}
function kdmyEngine_ptrToString(/**@type {number}*/ptr) {
    if (ptr == 0) return null;

    let end = HEAPU8.indexOf(0x00, ptr);
    if (end < 0) end = HEAPU8.byteLength;// this never should happen

    let char_array = HEAPU8.subarray(ptr, end);
    return kdmyEngine_textDecoder.decode(char_array, { stream: false });
}
function kdmyEngine_yieldAsync() {
    let id = Asyncify.callStackId - 1;
    if (Asyncify.callStackIdToName[id]) {
        if (Asyncify.asyncPromiseHandlers && Asyncify.asyncPromiseHandlers.kdmyEngine_allocated) {
            // already handled, do not return a Promise to avoid deadlock
            return void 0;
        }

        return new Promise(function (resolve, reject) {
            Asyncify.asyncPromiseHandlers = {
                resolve: function (res) {
                    Asyncify.callStackIdToName[id] = undefined;
                    resolve(res);
                },
                reject: function (err) {
                    Asyncify.callStackIdToName[id] = undefined;
                    reject(err);
                },
                kdmyEngine_allocated: true
            };
        });
    }
    return void 0;
}
function kdmyEngine_set_uint32(/**@type {number}*/address, /**@type {number}*/value) {
    let buffer = new Uint32Array(wasmMemory.buffer, address, 1);
    buffer[0] = value;
}
function kdmyEngine_get_uint32(/**@type {number}*/address) {
    let buffer = new Uint32Array(wasmMemory.buffer, address, 1);
    return buffer[0];
}
function kdmyEngine_stringToBuffer(/**@type {string}*/str, /**@type {LuaStringBuffer}*/strbuf, /**@type {number}*/count = 0) {
    if (str == null) {
        strbuf.written = 0;
        return 0;
    } else if (str.length < 1) {
        HEAPU8[strbuf.pointer] = 0x00;
        strbuf.written = 0;
        return strbuf.pointer;
    }

    let buffer = HEAPU8.subarray(strbuf.pointer, strbuf.pointer + strbuf.available);
    let info = kdmyEngine_textEncoder.encodeInto(str, buffer);

    if (count < 1)
        count = str.length;
    else if (count > str.length)
        throw new Error("Invalid 'count' parameter, count > str.length");

    if (info.read < count) {
        // increase buffer size
        let new_size = (count * 3) + 1;
        let new_pointer = _realloc(strbuf.pointer, new_size);

        if (new_pointer == 0) {
            throw new Error(`Not enough WASM memory to allocate ${new_size} bytes`);
        }

        strbuf.pointer = new_pointer;
        strbuf.available = new_size - 1;

        // encode again
        buffer = HEAPU8.subarray(new_pointer, new_pointer + strbuf.available);
        info = kdmyEngine_textEncoder.encodeInto(str, buffer);
    }

    // write null terminator
    HEAPU8[strbuf.pointer + info.written] = 0x00;

    // remember size
    strbuf.written = info.written;

    return strbuf.pointer;
}
function kdmyEngine_stringBuffer_alloc(/**@type {number}*/length) {
    let ptr = _malloc(length + 1);

    if (ptr == 0) {
        throw new Error(`Failed to allocate ${length + 1} bytes for the string buffer`);
    }

    HEAPU8[ptr] = 0x00;

    /**@type {LuaStringBuffer}*/
    let strbuf = { pointer: ptr, available: length, written: 0 };

    return strbuf;
}
function kdmyEngine_doLuaExport(/**@type {number}*/LUA_REGISTRYINDEX_value) {
    LUA_EXPORTS.LUA_REGISTRYINDEX = LUA_REGISTRYINDEX_value;

    Object.assign(Module, LUA_EXPORTS);
    Object.assign(Module, PRINT_EXPORTS);

    sizet_buffer_ptr = _malloc(4) | 0;
    if (sizet_buffer_ptr == 0) throw new Error("failed to allocate 4 bytes for 'sizet_buffer_ptr'");

    // allocate string buffers
    strbuf1 = kdmyEngine_stringBuffer_alloc(256);
    strbuf2 = kdmyEngine_stringBuffer_alloc(256);
    strbuf_large = kdmyEngine_stringBuffer_alloc(8192);
}


/**@type {LUA} */
const LUA_EXPORTS = {
    kdmyEngine_obtain: kdmyEngine_obtain,
    kdmyEngine_forget: kdmyEngine_forget,
    kdmyEngine_yieldAsync: kdmyEngine_yieldAsync,
    lua_pushstring: function (L, s) {
        let s_ptr = kdmyEngine_stringToBuffer(s, strbuf1);
        _lua_pushstring(L.ptr, s_ptr);
    },
    lua_pushcfunction: function (L, f) {
        let f_ptr = addFunction(f, "ip");
        _lua_pushcclosure(L.ptr, f_ptr, 0);
    },
    lua_setfield: function (L, idx, k) {
        let k_ptr = kdmyEngine_stringToBuffer(k, strbuf1);
        _lua_setfield(L.ptr, idx | 0, k_ptr);
    },
    lua_pushboolean: function (L, b) {
        return _lua_pushboolean(L.ptr, b ? 1 : 0) != 0;
    },
    luaL_checkstring: function (L, n) {
        let ret = _luaL_checklstring(L.ptr, n | 0, 0x00);
        return kdmyEngine_ptrToString(ret);
    },
    luaL_optstring: function (L, n, d) {
        // use "0x01" as string placeholder
        let ret = _luaL_optlstring(L.ptr, n | 0, 0x01, 0x00);

        // check if the placeholder was returned
        if (ret == 0x01) return d;

        return kdmyEngine_ptrToString(ret);
    },
    luaL_checknumber: function (L, arg) {
        return _luaL_checknumber(L.ptr, arg | 0);
    },
    luaL_optnumber: function (L, arg, def) {
        return _luaL_optnumber(L.ptr, arg | 0, def);
    },
    luaL_checkinteger: function (L, arg) {
        return _luaL_checkinteger(L.ptr, arg | 0);
    },
    luaL_optinteger: function (L, arg, def) {
        return _luaL_optinteger(L.ptr, arg | 0, def | 0);
    },
    luaL_newmetatable: function (L, tname) {
        let tmpname_ptr = kdmyEngine_stringToBuffer(tname, strbuf1);
        let ret = _luaL_newmetatable(L.ptr, tmpname_ptr);
        return ret;
    },
    luaL_error: function (L, fmt) {
        let fmt_ptr = kdmyEngine_stringToBuffer(fmt, strbuf2);
        return _luaL_error(L.ptr, fmt_ptr);
    },
    luaL_checklstring: function (L, arg, l) {
        let ret = _luaL_checklstring(L.ptr, arg | 0, sizet_buffer_ptr);

        let ret_length = kdmyEngine_get_uint32(sizet_buffer_ptr);

        l[0] = ret_length;
        return wasmMemory.buffer.slice(ret, ret + ret_length);
    },
    luaL_checkudata: function (L, ud, tname) {
        let tname_ptr = kdmyEngine_stringToBuffer(tname, strbuf1);

        let ret = _luaL_checkudata(L.ptr, ud | 0, tname_ptr);
        return kdmyEngine_obtain(L, ret);
    },
    luaL_testudata: function (L, ud, tname) {
        let tname_ptr = kdmyEngine_stringToBuffer(tname, strbuf1);

        let ret = _luaL_testudata(L.ptr, ud | 0, tname_ptr);
        return kdmyEngine_obtain(L, ret);
    },
    luaL_getmetatable: function (L, n) {
        let n_ptr = kdmyEngine_stringToBuffer(n, strbuf1);
        return _lua_getfield(L.ptr, this.LUA_REGISTRYINDEX, n_ptr);
    },
    luaL_ref: function (L, t) {
        return _luaL_ref(L.ptr, t | 0);
    },
    luaL_unref: function (L, t, ref) {
        return _luaL_unref(L.ptr, t | 0, ref | 0);
    },
    luaL_traceback: function (L, L1, msg, level) {
        let msg_ptr = kdmyEngine_stringToBuffer(msg, strbuf1);
        _luaL_traceback(L.ptr, L1.ptr, msg_ptr, level | 0);
    },
    luaL_newstate: function () {
        let ret = _luaL_newstate();
        if (ret == 0) {
            return null;
        }

        /**@type {lua_State} */
        let L = {
            ptr: ret,
            objectMap: new Map(),
            objectIndexes: _malloc(kdmyEngine_MAX_INDEXES * Int8Array.BYTES_PER_ELEMENT)
        };

        if (L.objectIndexes == 0) throw new Error("failed to allocate kdmyEngine_objectIndexes");
        HEAPU8.fill(0, L.objectIndexes, L.objectIndexes + kdmyEngine_MAX_INDEXES);

        return L;
    },
    luaL_setfuncs: function (L, l, nup) {
        if (nup != 0) {
            throw new Error("(not supported) the number upvalues must be 0.");
        }

        for (let item of l) {
            if (item[0] == null) {
                break;
            }
            if (item[1] == null) {
                throw new Error("(not supported) the function can not be null");
            }

            let func_ptr = addFunction(item[1], "ip");
            let name_ptr = kdmyEngine_stringToBuffer(item[0], strbuf1);

            _lua_pushcclosure(L.ptr, func_ptr, 0);
            _lua_setfield(L.ptr, -2, name_ptr);
        }
    },
    luaL_openlibs: function (L) {
        _luaL_openlibs(L.ptr);
    },
    luaL_dostring: function (L, s) {
        let s_ptr = kdmyEngine_stringToBuffer(s, strbuf_large);

        let ret = _luaL_loadstring(L.ptr, s_ptr);

        if (ret == this.LUA_OK) {
            ret = _lua_pcallk(L.ptr, 0, this.LUA_MULTRET, 0, 0x00, 0x00);
        }

        return ret;
    },
    luaL_loadbufferx: function (L, buff, sz, name, mode) {
        let buff_ptr = kdmyEngine_stringToBuffer(buff, strbuf_large, sz | 0);
        let name_ptr = kdmyEngine_stringToBuffer(name, strbuf1);
        let mode_ptr = kdmyEngine_stringToBuffer(mode, strbuf2);

        sz = strbuf_large.written;

        return _luaL_loadbufferx(L.ptr, buff_ptr, sz, name_ptr, mode_ptr);
    },
    luaL_loadstring: function (L, s) {
        let s_ptr = kdmyEngine_stringToBuffer(s, strbuf_large);
        return _luaL_loadstring(L.ptr, s_ptr);
    },
    lua_tonumber: function (L, i) {
        return _lua_tonumberx(L.ptr, i | 0, 0x00);
    },
    lua_toboolean: function (L, idx) {
        return _lua_toboolean(L.ptr, idx | 0) != 0;
    },
    lua_tostring: function (L, idx) {
        let ret = _lua_tolstring(L.ptr, idx | 0, 0x00);
        return kdmyEngine_ptrToString(ret);
    },
    lua_touserdata: function (L, idx) {
        let ret = _lua_touserdata(L.ptr, idx | 0);
        return kdmyEngine_obtain(L, ret);
    },
    lua_createtable: function (L, narr, nrec) {
        _lua_createtable(L.ptr, narr | 0, nrec | 0);
    },
    lua_pushnumber: function (L, n) {
        _lua_pushnumber(L.ptr, n);
    },
    lua_pushnil: function (L) {
        _lua_pushnil(L.ptr);
    },
    lua_pushinteger: function (L, n) {
        _lua_pushinteger(L.ptr, n | 0);
    },
    lua_pushfstring: function (L, fmt) {
        let fmt_ptr = kdmyEngine_stringToBuffer(fmt, strbuf_large);
        _lua_pushfstring(L.ptr, fmt_ptr);
    },
    lua_pushlstring: function (L, s, len) {
        if (s == null) throw new Error("The parameter 's' can not be null");

        len = len | 0;
        if (len < 0 || len > s.byteLength) throw new Error("Invalid 'len' parameter");

        if (len <= strbuf_large.available) {
            HEAPU8.set(new Uint8Array(s, 0, len), strbuf_large.pointer);
            _lua_pushlstring(L.ptr, strbuf_large.pointer, len);
            return;
        }

        // allocate a temporal buffer
        let s_ptr = _malloc(len);
        if (s_ptr == 0) throw new Error(`Failed to allocate ${len} bytes`);

        HEAPU8.set(new Uint8Array(s, 0, len), s_ptr);
        _lua_pushlstring(L.ptr, s_ptr, len);

        _free(s_ptr);
    },
    lua_pushvalue: function (L, idx) {
        _lua_pushvalue(L.ptr, idx | 0);
    },
    lua_pushlightuserdata: function (L, p) {
        let p_ptr = kdmyEngine_obtain(L, p);
        _lua_pushlightuserdata(L.ptr, p_ptr);
    },
    lua_pushliteral: function (L, s) {
        let s_ptr = kdmyEngine_stringToBuffer(s);
        _lua_pushstring(L.ptr, s_ptr);
    },
    lua_isnil: function (L, n) {
        return _lua_type(L.ptr, n | 0) == this.LUA_TNIL;
    },
    lua_pop: function (L, n) {
        _lua_settop(L.ptr, -(n | 0) - 1);
    },
    lua_rawseti: function (L, idx, n) {
        _lua_rawseti(L.ptr, idx | 0, n | 0);
    },
    lua_setglobal: function (L, name) {
        let name_ptr = kdmyEngine_stringToBuffer(name);
        _lua_setglobal(L.ptr, name_ptr);
    },
    lua_settable: function (L, idx) {
        _lua_settable(L.ptr, idx | 0);
    },
    lua_gettop: function (L) {
        return _lua_gettop(L.ptr);
    },
    lua_getfield: function (L, idx, k) {
        let k_ptr = kdmyEngine_stringToBuffer(k);
        return _lua_getfield(L.ptr, idx | 0, k_ptr);
    },
    lua_gettable: function (L, idx) {
        return _lua_gettable(L.ptr, idx | 0);
    },
    lua_rawlen: function (L, idx) {
        return _lua_rawlen(L.ptr, idx | 0);
    },
    lua_newuserdata: function (L, s) {
        if (s < 4) throw new Error("(not supported) the parameter 's' must be at least 4");

        let ptr = _lua_newuserdatauv(L.ptr, s | 0, 1);
        if (!ptr) {
            return null;
        }

        // use the returned pointer to store an javascript object
        let obj = {};
        let obj_ptr = kdmyEngine_obtain(L, obj);

        kdmyEngine_set_uint32(ptr, obj_ptr);
        return obj;
    },
    lua_setmetatable: function (L, objindex) {
        _lua_setmetatable(L.ptr, objindex);
    },
    lua_newtable: function (L) {
        _lua_createtable(L.ptr, 0, 0);
    },
    lua_rawset: function (L, idx) {
        _lua_rawset(L.ptr, idx | 0);
    },
    lua_type: function (L, idx) {
        return _lua_type(L.ptr, idx | 0);
    },
    lua_remove: function (L, idx) {
        _lua_rotate(L.ptr, idx | 0, -1);
        _lua_settop(L.ptr, -2);
    },
    lua_insert: function (L, idx) {
        _lua_rotate(L.ptr, idx | 0, 1);
    },
    lua_pcallk: function (L, nargs, nresults, errfunc, ctx, k) {
        if (ctx != 0) throw new Error("(not supported) the 'ctx' parameter must be zero.");
        if (k != null) throw new Error("(not supported) the 'k' parameter must be null.");

        return _lua_pcallk(L.ptr, nargs | 0, nresults | 0, errfunc, 0x00, 0x00);
    },
    lua_getglobal: function (L, name) {
        let name_ptr = kdmyEngine_stringToBuffer(name);
        return _lua_getglobal(L.ptr, name_ptr);
    },
    lua_setwarnf: function (L, f, ud) {
        if (ud != null) throw new Error("(not supported) the 'ud' parameter must be null.");

        let f_ptr = addFunction(f, "vppi");

        _lua_setwarnf(L.ptr, f_ptr, 0x00);
    },
    lua_close: function (L) {
        _lua_close(L.ptr);

        _free(L.objectIndexes);
        L.objectMap.clear();

        L.ptr = NaN;
        L.objectIndexes = NaN;
        L.objectMap = null;
    },
    LUA_OK: 0,
    LUA_TNONE: -1,
    LUA_TNIL: 0,
    LUA_TBOOLEAN: 1,
    LUA_TNUMBER: 3,
    LUA_TSTRING: 4,
    LUA_TTABLE: 5,
    LUA_TFUNCTION: 6,
    LUA_REFNIL: -1,
    LUA_NOREF: -2,
    LUA_MULTRET: -1,
    LUA_REGISTRYINDEX: (-(/*LUAI_MAXSTACK*/1000000) - 1000)
};
const PRINT_EXPORTS = {
    print: function (text) {
        console.log("%c[ModuleLua-stdout]%c", "font-weight: bold", "font-weight: normal", text);
    },
    printErr: function (text) {
        console.error("%c[ModuleLua-stderr]%c", "font-weight: bold", "font-weight: normal", text);
    }
};

