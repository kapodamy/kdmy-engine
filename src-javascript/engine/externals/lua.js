
var ModuleLua = (() => {//@ts-ignore
    var _scriptName = typeof document != 'undefined' ? document.currentScript?.src : undefined;
    if (typeof __filename != 'undefined')
        _scriptName ||= __filename;
    return (
        function (moduleArg = {}) {
        var moduleRtn;

        var Module = Object.assign({}, moduleArg);
        var readyPromiseResolve,
        readyPromiseReject;
        var readyPromise = new Promise((resolve, reject) => {
            readyPromiseResolve = resolve;
            readyPromiseReject = reject
        });
        var ENVIRONMENT_IS_WEB = typeof window == "object";
        var ENVIRONMENT_IS_WORKER = typeof importScripts == "function";
        var ENVIRONMENT_IS_NODE = typeof process == "object" && typeof process.versions == "object" && typeof process.versions.node == "string";
        if (ENVIRONMENT_IS_NODE) {}
        const kdmyEngine_MAX_INDEXES = 4096;
        const kdmyEngine_textDecoder = new TextDecoder("UTF-8", {
            ignoreBOM: true,
            fatal: true
        });
        const kdmyEngine_textEncoder = new TextEncoder;
        var sizet_buffer_ptr = 0;
        var strbuf1 = null;
        var strbuf2 = null;
        var strbuf_large = null;
        function kdmyEngine_obtain(L, key) {
            if (key == null)
                return 0;
            if (typeof key == "number") {
                if (key == 0)
                    return null;
                for (let [obj, idx] of L.objectMap) {
                    if (key == idx) {
                        if (HEAPU8[idx] == 0) {
                            throw new Error("forgotten object index " + key)
                        }
                        return obj
                    }
                }
                throw new Error("unknown object index " + key)
            }
            let index = L.objectMap.get(key);
            if (index !== undefined)
                return index;
            let new_index = -1;
            for (let i = 0; i < kdmyEngine_MAX_INDEXES; i++) {
                let idx = i + L.objectIndexes;
                if (HEAPU8[idx] == 0) {
                    new_index = idx;
                    break
                }
            }
            if (new_index < 0) {
                throw new Error("the objectMap is full")
            }
            HEAPU8[new_index] = 1;
            L.objectMap.set(key, new_index);
            return new_index
        }
        function kdmyEngine_forget(L, target_id) {
            for (let [obj, idx] of L.objectMap) {
                if (idx == target_id) {
                    if (HEAPU8[idx] == 0) {
                        throw new Error("already forgotten object index " + target_id)
                    }
                    HEAPU8[idx] = 0;
                    L.objectMap.delete(obj);
                    return
                }
            }
        }
        function kdmyEngine_ptrToString(ptr) {
            if (ptr == 0)
                return null;
            let end = HEAPU8.indexOf(0, ptr);
            if (end < 0)
                end = HEAPU8.byteLength;
            let char_array = HEAPU8.subarray(ptr, end);
            return kdmyEngine_textDecoder.decode(char_array, {
                stream: false
            })
        }
        function kdmyEngine_yieldAsync() {
            let id = Asyncify.callStackId - 1;
            if (Asyncify.callStackIdToName[id]) {
                if (Asyncify.asyncPromiseHandlers && Asyncify.asyncPromiseHandlers.kdmyEngine_allocated) {
                    return void 0
                }
                return new Promise(function (resolve, reject) {
                    Asyncify.asyncPromiseHandlers = {
                        resolve: function (res) {
                            Asyncify.callStackIdToName[id] = undefined;
                            resolve(res)
                        },
                        reject: function (err) {
                            Asyncify.callStackIdToName[id] = undefined;
                            reject(err)
                        },
                        kdmyEngine_allocated: true
                    }
                })
            }
            return void 0
        }
        function kdmyEngine_set_uint32(address, value) {
            let buffer = new Uint32Array(wasmMemory.buffer, address, 1);
            buffer[0] = value
        }
        function kdmyEngine_get_uint32(address) {
            let buffer = new Uint32Array(wasmMemory.buffer, address, 1);
            return buffer[0]
        }
        function kdmyEngine_stringToBuffer(str, strbuf, count = 0) {
            if (str == null) {
                strbuf.written = 0;
                return 0
            } else if (str.length < 1) {
                HEAPU8[strbuf.pointer] = 0;
                strbuf.written = 0;
                return strbuf.pointer
            }
            let buffer = HEAPU8.subarray(strbuf.pointer, strbuf.pointer + strbuf.available);
            let info = kdmyEngine_textEncoder.encodeInto(str, buffer);
            if (count < 1)
                count = str.length;
            else if (count > str.length)
                throw new Error("Invalid 'count' parameter, count > str.length");
            if (info.read < count) {
                let new_size = count * 3 + 1;
                let new_pointer = _realloc(strbuf.pointer, new_size);
                if (new_pointer == 0) {
                    throw new Error(`Not enough WASM memory to allocate ${new_size} bytes`)
                }
                strbuf.pointer = new_pointer;
                strbuf.available = new_size - 1;
                buffer = HEAPU8.subarray(new_pointer, new_pointer + strbuf.available);
                info = kdmyEngine_textEncoder.encodeInto(str, buffer)
            }
            HEAPU8[strbuf.pointer + info.written] = 0;
            strbuf.written = info.written;
            return strbuf.pointer
        }
        function kdmyEngine_stringBuffer_alloc(length) {
            let ptr = _malloc(length + 1);
            if (ptr == 0) {
                throw new Error(`Failed to allocate ${length + 1} bytes for the string buffer`)
            }
            HEAPU8[ptr] = 0;
            let strbuf = {
                pointer: ptr,
                available: length,
                written: 0
            };
            return strbuf
        }
        function kdmyEngine_doLuaExport(LUA_REGISTRYINDEX_value) {
            LUA_EXPORTS.LUA_REGISTRYINDEX = LUA_REGISTRYINDEX_value;
            Object.assign(Module, LUA_EXPORTS);
            Object.assign(Module, PRINT_EXPORTS);
            sizet_buffer_ptr = _malloc(4) | 0;
            if (sizet_buffer_ptr == 0)
                throw new Error("failed to allocate 4 bytes for 'sizet_buffer_ptr'");
            strbuf1 = kdmyEngine_stringBuffer_alloc(256);
            strbuf2 = kdmyEngine_stringBuffer_alloc(256);
            strbuf_large = kdmyEngine_stringBuffer_alloc(8192)
        }
        function kdmyEngine_obtainFunction(L, f, sig, wrapper) {
            let f_ptr = L.functionMap.get(f);
            if (f_ptr === undefined) {
                let f_wrapper = wrapper.bind({
                    f: f,
                    L: L
                });
                f_ptr = addFunction(f_wrapper, sig);
                L.functionMap.set(f, f_ptr)
            }
            return f_ptr
        }
        function kdmyEngine_is_obtained(L, ptr) {
            let start = L.objectIndexes;
            let end = L.objectIndexes + kdmyEngine_MAX_INDEXES;
            return ptr >= start && ptr < end
        }
        function kdmyEngine_func_cclosure() {
            return this.f(this.L)
        }
        function kdmyEngine_func_warnf(ud_ptr, msg_ptr, cont) {
            let ud = kdmyEngine_obtain(this.L, ud_ptr);
            let msg = kdmyEngine_ptrToString(msg_ptr);
            this.f(ud, msg, cont)
        }
        const LUA_EXPORTS = {
            kdmyEngine_obtain: kdmyEngine_obtain,
            kdmyEngine_forget: kdmyEngine_forget,
            kdmyEngine_yieldAsync: kdmyEngine_yieldAsync,
            lua_pushstring: function (L, s) {
                let s_ptr = kdmyEngine_stringToBuffer(s, strbuf1);
                _lua_pushstring(L.ptr, s_ptr)
            },
            lua_pushcfunction: function (L, f) {
                let f_ptr = kdmyEngine_obtainFunction(L, f, "ip", kdmyEngine_func_cclosure);
                _lua_pushcclosure(L.ptr, f_ptr, 0)
            },
            lua_setfield: function (L, idx, k) {
                let k_ptr = kdmyEngine_stringToBuffer(k, strbuf1);
                _lua_setfield(L.ptr, idx | 0, k_ptr)
            },
            lua_pushboolean: function (L, b) {
                return _lua_pushboolean(L.ptr, b ? 1 : 0) != 0
            },
            luaL_checkstring: function (L, n) {
                let ret = _luaL_checklstring(L.ptr, n | 0, 0);
                return kdmyEngine_ptrToString(ret)
            },
            luaL_optstring: function (L, n, d) {
                let ret = _luaL_optlstring(L.ptr, n | 0, 1, 0);
                if (ret == 1)
                    return d;
                return kdmyEngine_ptrToString(ret)
            },
            luaL_checknumber: function (L, arg) {
                return _luaL_checknumber(L.ptr, arg | 0)
            },
            luaL_optnumber: function (L, arg, def) {
                return _luaL_optnumber(L.ptr, arg | 0, def)
            },
            luaL_checkinteger: function (L, arg) {
                return _luaL_checkinteger(L.ptr, arg | 0)
            },
            luaL_optinteger: function (L, arg, def) {
                return _luaL_optinteger(L.ptr, arg | 0, def | 0)
            },
            luaL_newmetatable: function (L, tname) {
                let tmpname_ptr = kdmyEngine_stringToBuffer(tname, strbuf1);
                let ret = _luaL_newmetatable(L.ptr, tmpname_ptr);
                return ret
            },
            luaL_error: function (L, fmt) {
                let fmt_ptr = kdmyEngine_stringToBuffer(fmt, strbuf2);
                return _luaL_error(L.ptr, fmt_ptr)
            },
            luaL_checklstring: function (L, arg, l) {
                let ret = _luaL_checklstring(L.ptr, arg | 0, sizet_buffer_ptr);
                let ret_length = kdmyEngine_get_uint32(sizet_buffer_ptr);
                l[0] = ret_length;
                return wasmMemory.buffer.slice(ret, ret + ret_length)
            },
            luaL_checkudata: function (L, ud, tname) {
                let tname_ptr = kdmyEngine_stringToBuffer(tname, strbuf1);
                let ret = _luaL_checkudata(L.ptr, ud | 0, tname_ptr);
                if (ret == 0)
                    return null;
                if (!kdmyEngine_is_obtained(L, ret))
                    ret = kdmyEngine_get_uint32(ret);
                return kdmyEngine_obtain(L, ret)
            },
            luaL_testudata: function (L, ud, tname) {
                let tname_ptr = kdmyEngine_stringToBuffer(tname, strbuf1);
                let ret = _luaL_testudata(L.ptr, ud | 0, tname_ptr);
                if (ret == 0)
                    return null;
                if (!kdmyEngine_is_obtained(L, ret))
                    ret = kdmyEngine_get_uint32(ret);
                return kdmyEngine_obtain(L, ret)
            },
            luaL_getmetatable: function (L, n) {
                let n_ptr = kdmyEngine_stringToBuffer(n, strbuf1);
                return _lua_getfield(L.ptr, this.LUA_REGISTRYINDEX, n_ptr)
            },
            luaL_ref: function (L, t) {
                return _luaL_ref(L.ptr, t | 0)
            },
            luaL_unref: function (L, t, ref) {
                return _luaL_unref(L.ptr, t | 0, ref | 0)
            },
            luaL_traceback: function (L, L1, msg, level) {
                let msg_ptr = kdmyEngine_stringToBuffer(msg, strbuf1);
                _luaL_traceback(L.ptr, L1.ptr, msg_ptr, level | 0)
            },
            luaL_newstate: function () {
                let ret = _luaL_newstate();
                if (ret == 0) {
                    return null
                }
                let L = {
                    ptr: ret,
                    objectMap: new Map,
                    functionMap: new Map,
                    objectIndexes: _malloc(kdmyEngine_MAX_INDEXES * Int8Array.BYTES_PER_ELEMENT)
                };
                if (L.objectIndexes == 0)
                    throw new Error("failed to allocate kdmyEngine_objectIndexes");
                HEAPU8.fill(0, L.objectIndexes, L.objectIndexes + kdmyEngine_MAX_INDEXES);
                return L
            },
            luaL_setfuncs: function (L, l, nup) {
                if (nup != 0) {
                    throw new Error("(not supported) the number upvalues must be 0.")
                }
                for (let item of l) {
                    if (item[0] == null) {
                        break
                    }
                    if (item[1] == null) {
                        throw new Error("(not supported) the function can not be null")
                    }
                    let func_ptr = kdmyEngine_obtainFunction(L, item[1], "ip", kdmyEngine_func_cclosure);
                    let name_ptr = kdmyEngine_stringToBuffer(item[0], strbuf1);
                    _lua_pushcclosure(L.ptr, func_ptr, 0);
                    _lua_setfield(L.ptr, -2, name_ptr)
                }
            },
            luaL_openlibs: function (L) {
                _luaL_openlibs(L.ptr)
            },
            luaL_dostring: function (L, s) {
                let s_ptr = kdmyEngine_stringToBuffer(s, strbuf_large);
                let ret = _luaL_loadstring(L.ptr, s_ptr);
                if (ret == this.LUA_OK) {
                    ret = _lua_pcallk(L.ptr, 0, this.LUA_MULTRET, 0, 0, 0)
                }
                return ret
            },
            luaL_loadbufferx: function (L, buff, sz, name, mode) {
                let buff_ptr = kdmyEngine_stringToBuffer(buff, strbuf_large, sz | 0);
                let name_ptr = kdmyEngine_stringToBuffer(name, strbuf1);
                let mode_ptr = kdmyEngine_stringToBuffer(mode, strbuf2);
                sz = strbuf_large.written;
                return _luaL_loadbufferx(L.ptr, buff_ptr, sz, name_ptr, mode_ptr)
            },
            luaL_loadstring: function (L, s) {
                let s_ptr = kdmyEngine_stringToBuffer(s, strbuf_large);
                return _luaL_loadstring(L.ptr, s_ptr)
            },
            lua_tonumber: function (L, i) {
                return _lua_tonumberx(L.ptr, i | 0, 0)
            },
            lua_toboolean: function (L, idx) {
                return _lua_toboolean(L.ptr, idx | 0) != 0
            },
            lua_tostring: function (L, idx) {
                let ret = _lua_tolstring(L.ptr, idx | 0, 0);
                return kdmyEngine_ptrToString(ret)
            },
            lua_touserdata: function (L, idx) {
                let ret = _lua_touserdata(L.ptr, idx | 0);
                if (ret == 0)
                    return null;
                if (!kdmyEngine_is_obtained(L, ret))
                    ret = kdmyEngine_get_uint32(ret);
                return kdmyEngine_obtain(L, ret)
            },
            lua_createtable: function (L, narr, nrec) {
                _lua_createtable(L.ptr, narr | 0, nrec | 0)
            },
            lua_pushnumber: function (L, n) {
                _lua_pushnumber(L.ptr, n)
            },
            lua_pushnil: function (L) {
                _lua_pushnil(L.ptr)
            },
            lua_pushinteger: function (L, n) {
                _lua_pushinteger(L.ptr, n | 0)
            },
            lua_pushfstring: function (L, fmt) {
                let fmt_ptr = kdmyEngine_stringToBuffer(fmt, strbuf_large);
                _lua_pushfstring(L.ptr, fmt_ptr)
            },
            lua_pushlstring: function (L, s, len) {
                if (s == null)
                    throw new Error("The parameter 's' can not be null");
                len = len | 0;
                if (len < 0 || len > s.byteLength)
                    throw new Error("Invalid 'len' parameter");
                if (len <= strbuf_large.available) {
                    HEAPU8.set(new Uint8Array(s, 0, len), strbuf_large.pointer);
                    _lua_pushlstring(L.ptr, strbuf_large.pointer, len);
                    return
                }
                let s_ptr = _malloc(len);
                if (s_ptr == 0)
                    throw new Error(`Failed to allocate ${len} bytes`);
                HEAPU8.set(new Uint8Array(s, 0, len), s_ptr);
                _lua_pushlstring(L.ptr, s_ptr, len);
                _free(s_ptr)
            },
            lua_pushvalue: function (L, idx) {
                _lua_pushvalue(L.ptr, idx | 0)
            },
            lua_pushlightuserdata: function (L, p) {
                let p_ptr = kdmyEngine_obtain(L, p);
                _lua_pushlightuserdata(L.ptr, p_ptr)
            },
            lua_pushliteral: function (L, s) {
                let s_ptr = kdmyEngine_stringToBuffer(s, strbuf1);
                _lua_pushstring(L.ptr, s_ptr)
            },
            lua_isnil: function (L, n) {
                return _lua_type(L.ptr, n | 0) == this.LUA_TNIL
            },
            lua_pop: function (L, n) {
                _lua_settop(L.ptr,  - (n | 0) - 1)
            },
            lua_rawseti: function (L, idx, n) {
                _lua_rawseti(L.ptr, idx | 0, n | 0)
            },
            lua_setglobal: function (L, name) {
                let name_ptr = kdmyEngine_stringToBuffer(name, strbuf1);
                _lua_setglobal(L.ptr, name_ptr)
            },
            lua_settable: function (L, idx) {
                _lua_settable(L.ptr, idx | 0)
            },
            lua_gettop: function (L) {
                return _lua_gettop(L.ptr)
            },
            lua_getfield: function (L, idx, k) {
                let k_ptr = kdmyEngine_stringToBuffer(k, strbuf1);
                return _lua_getfield(L.ptr, idx | 0, k_ptr)
            },
            lua_gettable: function (L, idx) {
                return _lua_gettable(L.ptr, idx | 0)
            },
            lua_rawlen: function (L, idx) {
                return _lua_rawlen(L.ptr, idx | 0)
            },
            lua_newuserdata: function (L, s) {
                if (s < 4)
                    throw new Error("(not supported) the parameter 's' must be at least 4");
                let ptr = _lua_newuserdatauv(L.ptr, s | 0, 1);
                if (!ptr) {
                    return null
                }
                let obj = {};
                let obj_ptr = kdmyEngine_obtain(L, obj);
                kdmyEngine_set_uint32(ptr, obj_ptr);
                return obj
            },
            lua_setmetatable: function (L, objindex) {
                _lua_setmetatable(L.ptr, objindex)
            },
            lua_newtable: function (L) {
                _lua_createtable(L.ptr, 0, 0)
            },
            lua_rawset: function (L, idx) {
                _lua_rawset(L.ptr, idx | 0)
            },
            lua_type: function (L, idx) {
                return _lua_type(L.ptr, idx | 0)
            },
            lua_remove: function (L, idx) {
                _lua_rotate(L.ptr, idx | 0, -1);
                _lua_settop(L.ptr, -2)
            },
            lua_insert: function (L, idx) {
                _lua_rotate(L.ptr, idx | 0, 1)
            },
            lua_pcallk: function (L, nargs, nresults, errfunc, ctx, k) {
                if (ctx != 0)
                    throw new Error("(not supported) the 'ctx' parameter must be zero.");
                if (k != null)
                    throw new Error("(not supported) the 'k' parameter must be null.");
                return _lua_pcallk(L.ptr, nargs | 0, nresults | 0, errfunc, 0, 0)
            },
            lua_getglobal: function (L, name) {
                let name_ptr = kdmyEngine_stringToBuffer(name, strbuf1);
                return _lua_getglobal(L.ptr, name_ptr)
            },
            lua_setwarnf: function (L, f, ud) {
                let f_ptr = kdmyEngine_obtainFunction(L, f, "vppi", kdmyEngine_func_warnf);
                _lua_setwarnf(L.ptr, f_ptr, kdmyEngine_obtain(L, ud))
            },
            lua_close: function (L) {
                _lua_close(L.ptr);
                _free(L.objectIndexes);
                L.objectMap.clear();
                for (const [f, f_ptr] of L.functionMap)
                    removeFunction(f_ptr);
                L.functionMap.clear();
                L.ptr = NaN;
                L.objectIndexes = NaN;
                L.objectMap = null
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
            LUA_REGISTRYINDEX: -1e6 - 1e3
        };
        const PRINT_EXPORTS = {
            print: function (text) {
                console.log("%c[ModuleLua-stdout]%c", "font-weight: bold", "font-weight: normal", text)
            },
            printErr: function (text) {
                console.error("%c[ModuleLua-stderr]%c", "font-weight: bold", "font-weight: normal", text)
            }
        };
        var moduleOverrides = Object.assign({}, Module);
        var arguments_ = [];
        var thisProgram = "./this.program";
        var quit_ = (status, toThrow) => {
            throw toThrow
        };
        var scriptDirectory = "";
        function locateFile(path) {
            return scriptDirectory + path
        }
        var read_,
        readAsync,
        readBinary;
        if (ENVIRONMENT_IS_NODE) {
            var fs = require("fs");
            var nodePath = require("path");
            scriptDirectory = __dirname + "/";
            read_ = (filename, binary) => {
                filename = isFileURI(filename) ? new URL(filename) : nodePath.normalize(filename);
                return fs.readFileSync(filename, binary ? undefined : "utf8")
            };
            readBinary = filename => {
                var ret = read_(filename, true);
                if (!ret.buffer) {
                    ret = new Uint8Array(ret)
                }
                return ret
            };
            readAsync = (filename, onload, onerror, binary = true) => {
                filename = isFileURI(filename) ? new URL(filename) : nodePath.normalize(filename);
                fs.readFile(filename, binary ? undefined : "utf8", (err, data) => {
                    if (err)
                        onerror(err);
                    else//@ts-ignore
                        onload(binary ? data.buffer : data)
                })
            };
            if (!Module["thisProgram"] && process.argv.length > 1) {
                thisProgram = process.argv[1].replace(/\\/g, "/")
            }
            arguments_ = process.argv.slice(2);
            quit_ = (status, toThrow) => {
                process.exitCode = status;
                throw toThrow
            }
        } else if (ENVIRONMENT_IS_WEB || ENVIRONMENT_IS_WORKER) {
            if (ENVIRONMENT_IS_WORKER) {
                scriptDirectory = self.location.href
            } else if (typeof document != "undefined" && document.currentScript) {//@ts-ignore
                scriptDirectory = document.currentScript.src
            }
            if (_scriptName) {
                scriptDirectory = _scriptName
            }
            if (scriptDirectory.startsWith("blob:")) {
                scriptDirectory = ""
            } else {
                scriptDirectory = scriptDirectory.substr(0, scriptDirectory.replace(/[?#].*/, "").lastIndexOf("/") + 1)
            } {
                read_ = url => {
                    var xhr = new XMLHttpRequest;
                    xhr.open("GET", url, false);
                    xhr.send(null);
                    return xhr.responseText
                };
                if (ENVIRONMENT_IS_WORKER) {
                    readBinary = url => {
                        var xhr = new XMLHttpRequest;
                        xhr.open("GET", url, false);
                        xhr.responseType = "arraybuffer";
                        xhr.send(null);
                        return new Uint8Array(xhr.response)
                    }
                }
                readAsync = (url, onload, onerror) => {
                    var xhr = new XMLHttpRequest;
                    xhr.open("GET", url, true);
                    xhr.responseType = "arraybuffer";
                    xhr.onload = () => {
                        if (xhr.status == 200 || xhr.status == 0 && xhr.response) {
                            onload(xhr.response);
                            return
                        }
                        onerror()
                    };
                    xhr.onerror = onerror;
                    xhr.send(null)
                }
            }
        } else {}
        var out = Module["print"] || console.log.bind(console);
        var err = Module["printErr"] || console.error.bind(console);
        Object.assign(Module, moduleOverrides);
        moduleOverrides = null;
        var wasmBinary;
        var wasmMemory;
        var ABORT = false;
        var EXITSTATUS;
        var HEAP8,
        HEAPU8,
        HEAP16,
        HEAPU16,
        HEAP32,
        HEAPU32,
        HEAPF32,
        HEAPF64;
        function updateMemoryViews() {
            var b = wasmMemory.buffer;
            HEAP8 = new Int8Array(b);
            HEAP16 = new Int16Array(b);
            HEAPU8 = new Uint8Array(b);
            HEAPU16 = new Uint16Array(b);
            HEAP32 = new Int32Array(b);
            HEAPU32 = new Uint32Array(b);
            HEAPF32 = new Float32Array(b);
            HEAPF64 = new Float64Array(b)
        }
        var __ATPRERUN__ = [];
        var __ATINIT__ = [];
        var __ATPOSTRUN__ = [];
        var runtimeInitialized = false;
        function preRun() {
            callRuntimeCallbacks(__ATPRERUN__)
        }
        function initRuntime() {
            runtimeInitialized = true;//@ts-ignore
            if (!Module["noFSInit"] && !FS.init.initialized)
                FS.init();
            FS.ignorePermissions = false;
            TTY.init();
            callRuntimeCallbacks(__ATINIT__)
        }
        function postRun() {
            callRuntimeCallbacks(__ATPOSTRUN__)
        }
        function addOnInit(cb) {
            __ATINIT__.unshift(cb)
        }
        var runDependencies = 0;
        var runDependencyWatcher = null;
        var dependenciesFulfilled = null;
        function getUniqueRunDependency(id) {
            return id
        }
        function addRunDependency(id) {
            runDependencies++
        }
        function removeRunDependency(id) {
            runDependencies--;
            if (runDependencies == 0) {
                if (runDependencyWatcher !== null) {
                    clearInterval(runDependencyWatcher);
                    runDependencyWatcher = null
                }
                if (dependenciesFulfilled) {
                    var callback = dependenciesFulfilled;
                    dependenciesFulfilled = null;
                    callback()
                }
            }
        }
        function abort(what) {
            what = "Aborted(" + what + ")";
            err(what);
            ABORT = true;
            EXITSTATUS = 1;
            what += ". Build with -sASSERTIONS for more info.";
            var e = new WebAssembly.RuntimeError(what);
            readyPromiseReject(e);
            throw e
        }
        var dataURIPrefix = "data:application/octet-stream;base64,";
        var isDataURI = filename => filename.startsWith(dataURIPrefix);
        var isFileURI = filename => filename.startsWith("file://");
        function findWasmBinary() {
            var f = "lua.wasm";
            if (!isDataURI(f)) {
                return locateFile(f)
            }
            return f
        }
        var wasmBinaryFile;
        function getBinarySync(file) {
            if (file == wasmBinaryFile && wasmBinary) {
                return new Uint8Array(wasmBinary)
            }
            if (readBinary) {
                return readBinary(file)
            }
            throw "both async and sync fetching of the wasm failed"
        }
        function getBinaryPromise(binaryFile) {
            if (!wasmBinary && (ENVIRONMENT_IS_WEB || ENVIRONMENT_IS_WORKER)) {
                if (typeof fetch == "function" && !isFileURI(binaryFile)) {
                    return fetch(binaryFile, {
                        credentials: "same-origin"
                    }).then(response => {
                        if (!response["ok"]) {
                            throw `failed to load wasm binary file at '${binaryFile}'`
                        }
                        return response["arrayBuffer"]()
                    }).catch(() => getBinarySync(binaryFile))
                } else if (readAsync) {
                    return new Promise((resolve, reject) => {
                        readAsync(binaryFile, response => resolve(new Uint8Array(response)), reject)
                    })
                }
            }
            return Promise.resolve().then(() => getBinarySync(binaryFile))
        }
        function instantiateArrayBuffer(binaryFile, imports, receiver) {
            return getBinaryPromise(binaryFile).then(binary => WebAssembly.instantiate(binary, imports)).then(receiver, reason => {
                err(`failed to asynchronously prepare wasm: ${reason}`);
                abort(reason)
            })
        }
        function instantiateAsync(binary, binaryFile, imports, callback) {
            if (!binary && typeof WebAssembly.instantiateStreaming == "function" && !isDataURI(binaryFile) && !isFileURI(binaryFile) && !ENVIRONMENT_IS_NODE && typeof fetch == "function") {
                return fetch(binaryFile, {
                    credentials: "same-origin"
                }).then(response => {
                    var result = WebAssembly.instantiateStreaming(response, imports);
                    return result.then(callback, function (reason) {
                        err(`wasm streaming compile failed: ${reason}`);
                        err("falling back to ArrayBuffer instantiation");
                        return instantiateArrayBuffer(binaryFile, imports, callback)
                    })
                })
            }
            return instantiateArrayBuffer(binaryFile, imports, callback)
        }
        function getWasmImports() {
            return {
                env: wasmImports,
                wasi_snapshot_preview1: wasmImports
            }
        }
        function createWasm() {
            var info = getWasmImports();
            function receiveInstance(instance, module) {
                wasmExports = instance.exports;
                wasmExports = Asyncify.instrumentWasmExports(wasmExports);
                wasmMemory = wasmExports["memory"];
                updateMemoryViews();
                wasmTable = wasmExports["__indirect_function_table"];
                addOnInit(wasmExports["__wasm_call_ctors"]);
                removeRunDependency("wasm-instantiate");
                return wasmExports
            }
            addRunDependency("wasm-instantiate");
            function receiveInstantiationResult(result) {
                receiveInstance(result["instance"])
            }
            if (!wasmBinaryFile)
                wasmBinaryFile = findWasmBinary();
            instantiateAsync(wasmBinary, wasmBinaryFile, info, receiveInstantiationResult).catch(readyPromiseReject);
            return {}
        }
        var tempDouble;
        var tempI64;
        function _doLuaExport(reg_idx_value) {
            kdmyEngine_doLuaExport(reg_idx_value)
        }
        function ExitStatus(status) {
            this.name = "ExitStatus";
            this.message = `Program terminated with exit(${status})`;
            this.status = status
        }
        var callRuntimeCallbacks = callbacks => {
            while (callbacks.length > 0) {
                callbacks.shift()(Module)
            }
        };
        var stackRestore = val => __emscripten_stack_restore(val);
        var stackSave = () => _emscripten_stack_get_current();
        var PATH = {
            isAbs: path => path.charAt(0) === "/",
            splitPath: filename => {
                var splitPathRe = /^(\/?|)([\s\S]*?)((?:\.{1,2}|[^\/]+?|)(\.[^.\/]*|))(?:[\/]*)$/;
                return splitPathRe.exec(filename).slice(1)
            },
            normalizeArray: (parts, allowAboveRoot) => {
                var up = 0;
                for (var i = parts.length - 1; i >= 0; i--) {
                    var last = parts[i];
                    if (last === ".") {
                        parts.splice(i, 1)
                    } else if (last === "..") {
                        parts.splice(i, 1);
                        up++
                    } else if (up) {
                        parts.splice(i, 1);
                        up--
                    }
                }
                if (allowAboveRoot) {
                    for (; up; up--) {
                        parts.unshift("..")
                    }
                }
                return parts
            },
            normalize: path => {
                var isAbsolute = PATH.isAbs(path),
                trailingSlash = path.substr(-1) === "/";
                path = PATH.normalizeArray(path.split("/").filter(p => !!p), !isAbsolute).join("/");
                if (!path && !isAbsolute) {
                    path = "."
                }
                if (path && trailingSlash) {
                    path += "/"
                }
                return (isAbsolute ? "/" : "") + path
            },
            dirname: path => {
                var result = PATH.splitPath(path),
                root = result[0],
                dir = result[1];
                if (!root && !dir) {
                    return "."
                }
                if (dir) {
                    dir = dir.substr(0, dir.length - 1)
                }
                return root + dir
            },
            basename: path => {
                if (path === "/")
                    return "/";
                path = PATH.normalize(path);
                path = path.replace(/\/$/, "");
                var lastSlash = path.lastIndexOf("/");
                if (lastSlash === -1)
                    return path;
                return path.substr(lastSlash + 1)
            },
            join: (...paths) => PATH.normalize(paths.join("/")),
            join2: (l, r) => PATH.normalize(l + "/" + r)
        };
        var initRandomFill = () => {
            if (typeof crypto == "object" && typeof crypto["getRandomValues"] == "function") {
                return view => crypto.getRandomValues(view)
            } else if (ENVIRONMENT_IS_NODE) {
                try {
                    var crypto_module = require("crypto");
                    var randomFillSync = crypto_module["randomFillSync"];
                    if (randomFillSync) {
                        return view => crypto_module["randomFillSync"](view)
                    }
                    var randomBytes = crypto_module["randomBytes"];
                    return view => (view.set(randomBytes(view.byteLength)), view)
                } catch (e) {}
            }
            abort("initRandomDevice")
        };
        var randomFill = view => (randomFill = initRandomFill())(view);
        var PATH_FS = {
            resolve: (...args) => {
                var resolvedPath = "",
                resolvedAbsolute = false;
                for (var i = args.length - 1; i >= -1 && !resolvedAbsolute; i--) {
                    var path = i >= 0 ? args[i] : FS.cwd();
                    if (typeof path != "string") {
                        throw new TypeError("Arguments to path.resolve must be strings")
                    } else if (!path) {
                        return ""
                    }
                    resolvedPath = path + "/" + resolvedPath;
                    resolvedAbsolute = PATH.isAbs(path)
                }
                resolvedPath = PATH.normalizeArray(resolvedPath.split("/").filter(p => !!p), !resolvedAbsolute).join("/");
                return (resolvedAbsolute ? "/" : "") + resolvedPath || "."
            },
            relative: (from, to) => {
                from = PATH_FS.resolve(from).substr(1);
                to = PATH_FS.resolve(to).substr(1);
                function trim(arr) {
                    var start = 0;
                    for (; start < arr.length; start++) {
                        if (arr[start] !== "")
                            break
                    }
                    var end = arr.length - 1;
                    for (; end >= 0; end--) {
                        if (arr[end] !== "")
                            break
                    }
                    if (start > end)
                        return [];
                    return arr.slice(start, end - start + 1)
                }
                var fromParts = trim(from.split("/"));
                var toParts = trim(to.split("/"));
                var length = Math.min(fromParts.length, toParts.length);
                var samePartsLength = length;
                for (var i = 0; i < length; i++) {
                    if (fromParts[i] !== toParts[i]) {
                        samePartsLength = i;
                        break
                    }
                }
                var outputParts = [];
                for (var i = samePartsLength; i < fromParts.length; i++) {
                    outputParts.push("..")
                }
                outputParts = outputParts.concat(toParts.slice(samePartsLength));
                return outputParts.join("/")
            }
        };
        var UTF8Decoder = typeof TextDecoder != "undefined" ? new TextDecoder("utf8") : undefined;
        var UTF8ArrayToString = (heapOrArray, idx, maxBytesToRead) => {
            var endIdx = idx + maxBytesToRead;
            var endPtr = idx;
            while (heapOrArray[endPtr] && !(endPtr >= endIdx))
                ++endPtr;
            if (endPtr - idx > 16 && heapOrArray.buffer && UTF8Decoder) {
                return UTF8Decoder.decode(heapOrArray.subarray(idx, endPtr))
            }
            var str = "";
            while (idx < endPtr) {
                var u0 = heapOrArray[idx++];
                if (!(u0 & 128)) {
                    str += String.fromCharCode(u0);
                    continue
                }
                var u1 = heapOrArray[idx++] & 63;
                if ((u0 & 224) == 192) {
                    str += String.fromCharCode((u0 & 31) << 6 | u1);
                    continue
                }
                var u2 = heapOrArray[idx++] & 63;
                if ((u0 & 240) == 224) {
                    u0 = (u0 & 15) << 12 | u1 << 6 | u2
                } else {
                    u0 = (u0 & 7) << 18 | u1 << 12 | u2 << 6 | heapOrArray[idx++] & 63
                }
                if (u0 < 65536) {
                    str += String.fromCharCode(u0)
                } else {
                    var ch = u0 - 65536;
                    str += String.fromCharCode(55296 | ch >> 10, 56320 | ch & 1023)
                }
            }
            return str
        };
        var FS_stdin_getChar_buffer = [];
        var lengthBytesUTF8 = str => {
            var len = 0;
            for (var i = 0; i < str.length; ++i) {
                var c = str.charCodeAt(i);
                if (c <= 127) {
                    len++
                } else if (c <= 2047) {
                    len += 2
                } else if (c >= 55296 && c <= 57343) {
                    len += 4;
                    ++i
                } else {
                    len += 3
                }
            }
            return len
        };
        var stringToUTF8Array = (str, heap, outIdx, maxBytesToWrite) => {
            if (!(maxBytesToWrite > 0))
                return 0;
            var startIdx = outIdx;
            var endIdx = outIdx + maxBytesToWrite - 1;
            for (var i = 0; i < str.length; ++i) {
                var u = str.charCodeAt(i);
                if (u >= 55296 && u <= 57343) {
                    var u1 = str.charCodeAt(++i);
                    u = 65536 + ((u & 1023) << 10) | u1 & 1023
                }
                if (u <= 127) {
                    if (outIdx >= endIdx)
                        break;
                    heap[outIdx++] = u
                } else if (u <= 2047) {
                    if (outIdx + 1 >= endIdx)
                        break;
                    heap[outIdx++] = 192 | u >> 6;
                    heap[outIdx++] = 128 | u & 63
                } else if (u <= 65535) {
                    if (outIdx + 2 >= endIdx)
                        break;
                    heap[outIdx++] = 224 | u >> 12;
                    heap[outIdx++] = 128 | u >> 6 & 63;
                    heap[outIdx++] = 128 | u & 63
                } else {
                    if (outIdx + 3 >= endIdx)
                        break;
                    heap[outIdx++] = 240 | u >> 18;
                    heap[outIdx++] = 128 | u >> 12 & 63;
                    heap[outIdx++] = 128 | u >> 6 & 63;
                    heap[outIdx++] = 128 | u & 63
                }
            }
            heap[outIdx] = 0;
            return outIdx - startIdx
        };
        function intArrayFromString(stringy, dontAddNull, length) {
            var len = length > 0 ? length : lengthBytesUTF8(stringy) + 1;
            var u8array = new Array(len);
            var numBytesWritten = stringToUTF8Array(stringy, u8array, 0, u8array.length);
            if (dontAddNull)
                u8array.length = numBytesWritten;
            return u8array
        }
        var FS_stdin_getChar = () => {
            if (!FS_stdin_getChar_buffer.length) {
                var result = null;
                if (ENVIRONMENT_IS_NODE) {
                    var BUFSIZE = 256;
                    var buf = Buffer.alloc(BUFSIZE);
                    var bytesRead = 0;
                    var fd = process.stdin.fd;
                    try {//@ts-ignore
                        bytesRead = fs.readSync(fd, buf, 0, BUFSIZE)
                    } catch (e) {
                        if (e.toString().includes("EOF"))
                            bytesRead = 0;
                        else
                            throw e
                    }
                    if (bytesRead > 0) {
                        result = buf.slice(0, bytesRead).toString("utf-8")
                    }
                } else if (typeof window != "undefined" && typeof window.prompt == "function") {
                    result = window.prompt("Input: ");
                    if (result !== null) {
                        result += "\n"
                    }
                } else {}
                if (!result) {
                    return null
                }
                FS_stdin_getChar_buffer = intArrayFromString(result, true)
            }
            return FS_stdin_getChar_buffer.shift()
        };
        var TTY = {
            ttys: [],
            init() {},
            shutdown() {},
            register(dev, ops) {
                TTY.ttys[dev] = {
                    input: [],
                    output: [],
                    ops: ops
                };
                FS.registerDevice(dev, TTY.stream_ops)
            },
            stream_ops: {
                open(stream) {
                    var tty = TTY.ttys[stream.node.rdev];
                    if (!tty) {
                        throw new FS.ErrnoError(43)
                    }
                    stream.tty = tty;
                    stream.seekable = false
                },
                close(stream) {
                    stream.tty.ops.fsync(stream.tty)
                },
                fsync(stream) {
                    stream.tty.ops.fsync(stream.tty)
                },
                read(stream, buffer, offset, length, pos) {
                    if (!stream.tty || !stream.tty.ops.get_char) {
                        throw new FS.ErrnoError(60)
                    }
                    var bytesRead = 0;
                    for (var i = 0; i < length; i++) {
                        var result;
                        try {
                            result = stream.tty.ops.get_char(stream.tty)
                        } catch (e) {
                            throw new FS.ErrnoError(29)
                        }
                        if (result === undefined && bytesRead === 0) {
                            throw new FS.ErrnoError(6)
                        }
                        if (result === null || result === undefined)
                            break;
                        bytesRead++;
                        buffer[offset + i] = result
                    }
                    if (bytesRead) {
                        stream.node.timestamp = Date.now()
                    }
                    return bytesRead
                },
                write(stream, buffer, offset, length, pos) {
                    if (!stream.tty || !stream.tty.ops.put_char) {
                        throw new FS.ErrnoError(60)
                    }
                    try {
                        for (var i = 0; i < length; i++) {
                            stream.tty.ops.put_char(stream.tty, buffer[offset + i])
                        }
                    } catch (e) {
                        throw new FS.ErrnoError(29)
                    }
                    if (length) {
                        stream.node.timestamp = Date.now()
                    }
                    return i
                }
            },
            default_tty_ops: {
                get_char(tty) {
                    return FS_stdin_getChar()
                },
                put_char(tty, val) {
                    if (val === null || val === 10) {
                        out(UTF8ArrayToString(tty.output, 0));
                        tty.output = []
                    } else {
                        if (val != 0)
                            tty.output.push(val)
                    }
                },
                fsync(tty) {
                    if (tty.output && tty.output.length > 0) {
                        out(UTF8ArrayToString(tty.output, 0));
                        tty.output = []
                    }
                },
                ioctl_tcgets(tty) {
                    return {
                        c_iflag: 25856,
                        c_oflag: 5,
                        c_cflag: 191,
                        c_lflag: 35387,
                        c_cc: [3, 28, 127, 21, 4, 0, 1, 0, 17, 19, 26, 0, 18, 15, 23, 22, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
                    }
                },
                ioctl_tcsets(tty, optional_actions, data) {
                    return 0
                },
                ioctl_tiocgwinsz(tty) {
                    return [24, 80]
                }
            },
            default_tty1_ops: {
                put_char(tty, val) {
                    if (val === null || val === 10) {
                        err(UTF8ArrayToString(tty.output, 0));
                        tty.output = []
                    } else {
                        if (val != 0)
                            tty.output.push(val)
                    }
                },
                fsync(tty) {
                    if (tty.output && tty.output.length > 0) {
                        err(UTF8ArrayToString(tty.output, 0));
                        tty.output = []
                    }
                }
            }
        };
        var mmapAlloc = size => {
            abort()
        };
        var MEMFS = {
            ops_table: null,
            mount(mount) {
                return MEMFS.createNode(null, "/", 16384 | 511, 0)
            },
            createNode(parent, name, mode, dev) {
                if (FS.isBlkdev(mode) || FS.isFIFO(mode)) {
                    throw new FS.ErrnoError(63)
                }
                MEMFS.ops_table ||= {
                    dir: {
                        node: {
                            getattr: MEMFS.node_ops.getattr,
                            setattr: MEMFS.node_ops.setattr,
                            lookup: MEMFS.node_ops.lookup,
                            mknod: MEMFS.node_ops.mknod,
                            rename: MEMFS.node_ops.rename,
                            unlink: MEMFS.node_ops.unlink,
                            rmdir: MEMFS.node_ops.rmdir,
                            readdir: MEMFS.node_ops.readdir,
                            symlink: MEMFS.node_ops.symlink
                        },
                        stream: {
                            llseek: MEMFS.stream_ops.llseek
                        }
                    },
                    file: {
                        node: {
                            getattr: MEMFS.node_ops.getattr,
                            setattr: MEMFS.node_ops.setattr
                        },
                        stream: {
                            llseek: MEMFS.stream_ops.llseek,
                            read: MEMFS.stream_ops.read,
                            write: MEMFS.stream_ops.write,
                            allocate: MEMFS.stream_ops.allocate,
                            mmap: MEMFS.stream_ops.mmap,
                            msync: MEMFS.stream_ops.msync
                        }
                    },
                    link: {
                        node: {
                            getattr: MEMFS.node_ops.getattr,
                            setattr: MEMFS.node_ops.setattr,
                            readlink: MEMFS.node_ops.readlink
                        },
                        stream: {}
                    },
                    chrdev: {
                        node: {
                            getattr: MEMFS.node_ops.getattr,
                            setattr: MEMFS.node_ops.setattr
                        },
                        stream: FS.chrdev_stream_ops
                    }
                };
                var node = FS.createNode(parent, name, mode, dev);
                if (FS.isDir(node.mode)) {
                    node.node_ops = MEMFS.ops_table.dir.node;
                    node.stream_ops = MEMFS.ops_table.dir.stream;//@ts-ignore
                    node.contents = {}
                } else if (FS.isFile(node.mode)) {
                    node.node_ops = MEMFS.ops_table.file.node;
                    node.stream_ops = MEMFS.ops_table.file.stream;//@ts-ignore
                    node.usedBytes = 0;//@ts-ignore
                    node.contents = null
                } else if (FS.isLink(node.mode)) {
                    node.node_ops = MEMFS.ops_table.link.node;
                    node.stream_ops = MEMFS.ops_table.link.stream
                } else if (FS.isChrdev(node.mode)) {
                    node.node_ops = MEMFS.ops_table.chrdev.node;
                    node.stream_ops = MEMFS.ops_table.chrdev.stream
                }//@ts-ignore
                node.timestamp = Date.now();
                if (parent) {
                    parent.contents[name] = node;//@ts-ignore
                    parent.timestamp = node.timestamp
                }
                return node
            },
            getFileDataAsTypedArray(node) {
                if (!node.contents)
                    return new Uint8Array(0);
                if (node.contents.subarray)
                    return node.contents.subarray(0, node.usedBytes);
                return new Uint8Array(node.contents)
            },
            expandFileStorage(node, newCapacity) {
                var prevCapacity = node.contents ? node.contents.length : 0;
                if (prevCapacity >= newCapacity)
                    return;
                var CAPACITY_DOUBLING_MAX = 1024 * 1024;
                newCapacity = Math.max(newCapacity, prevCapacity * (prevCapacity < CAPACITY_DOUBLING_MAX ? 2 : 1.125) >>> 0);
                if (prevCapacity != 0)
                    newCapacity = Math.max(newCapacity, 256);
                var oldContents = node.contents;
                node.contents = new Uint8Array(newCapacity);
                if (node.usedBytes > 0)
                    node.contents.set(oldContents.subarray(0, node.usedBytes), 0)
            },
            resizeFileStorage(node, newSize) {
                if (node.usedBytes == newSize)
                    return;
                if (newSize == 0) {
                    node.contents = null;
                    node.usedBytes = 0
                } else {
                    var oldContents = node.contents;
                    node.contents = new Uint8Array(newSize);
                    if (oldContents) {
                        node.contents.set(oldContents.subarray(0, Math.min(newSize, node.usedBytes)))
                    }
                    node.usedBytes = newSize
                }
            },
            node_ops: {
                getattr(node) {
                    var attr = {};
                    attr.dev = FS.isChrdev(node.mode) ? node.id : 1;
                    attr.ino = node.id;
                    attr.mode = node.mode;
                    attr.nlink = 1;
                    attr.uid = 0;
                    attr.gid = 0;
                    attr.rdev = node.rdev;
                    if (FS.isDir(node.mode)) {
                        attr.size = 4096
                    } else if (FS.isFile(node.mode)) {
                        attr.size = node.usedBytes
                    } else if (FS.isLink(node.mode)) {
                        attr.size = node.link.length
                    } else {
                        attr.size = 0
                    }
                    attr.atime = new Date(node.timestamp);
                    attr.mtime = new Date(node.timestamp);
                    attr.ctime = new Date(node.timestamp);
                    attr.blksize = 4096;
                    attr.blocks = Math.ceil(attr.size / attr.blksize);
                    return attr
                },
                setattr(node, attr) {
                    if (attr.mode !== undefined) {
                        node.mode = attr.mode
                    }
                    if (attr.timestamp !== undefined) {
                        node.timestamp = attr.timestamp
                    }
                    if (attr.size !== undefined) {
                        MEMFS.resizeFileStorage(node, attr.size)
                    }
                },
                lookup(parent, name) {
                    throw FS.genericErrors[44]
                },
                mknod(parent, name, mode, dev) {
                    return MEMFS.createNode(parent, name, mode, dev)
                },
                rename(old_node, new_dir, new_name) {
                    if (FS.isDir(old_node.mode)) {
                        var new_node;
                        try {
                            new_node = FS.lookupNode(new_dir, new_name)
                        } catch (e) {}
                        if (new_node) {
                            for (var i in new_node.contents) {
                                throw new FS.ErrnoError(55)
                            }
                        }
                    }
                    delete old_node.parent.contents[old_node.name];
                    old_node.parent.timestamp = Date.now();
                    old_node.name = new_name;
                    new_dir.contents[new_name] = old_node;
                    new_dir.timestamp = old_node.parent.timestamp;
                    old_node.parent = new_dir
                },
                unlink(parent, name) {
                    delete parent.contents[name];
                    parent.timestamp = Date.now()
                },
                rmdir(parent, name) {
                    var node = FS.lookupNode(parent, name);
                    for (var i in node.contents) {
                        throw new FS.ErrnoError(55)
                    }
                    delete parent.contents[name];
                    parent.timestamp = Date.now()
                },
                readdir(node) {
                    var entries = [".", ".."];
                    for (var key of Object.keys(node.contents)) {
                        entries.push(key)
                    }
                    return entries
                },
                symlink(parent, newname, oldpath) {
                    var node = MEMFS.createNode(parent, newname, 511 | 40960, 0);//@ts-ignore
                    node.link = oldpath;
                    return node
                },
                readlink(node) {
                    if (!FS.isLink(node.mode)) {
                        throw new FS.ErrnoError(28)
                    }
                    return node.link
                }
            },
            stream_ops: {
                read(stream, buffer, offset, length, position) {
                    var contents = stream.node.contents;
                    if (position >= stream.node.usedBytes)
                        return 0;
                    var size = Math.min(stream.node.usedBytes - position, length);
                    if (size > 8 && contents.subarray) {
                        buffer.set(contents.subarray(position, position + size), offset)
                    } else {
                        for (var i = 0; i < size; i++)
                            buffer[offset + i] = contents[position + i]
                    }
                    return size
                },
                write(stream, buffer, offset, length, position, canOwn) {
                    if (buffer.buffer === HEAP8.buffer) {
                        canOwn = false
                    }
                    if (!length)
                        return 0;
                    var node = stream.node;
                    node.timestamp = Date.now();
                    if (buffer.subarray && (!node.contents || node.contents.subarray)) {
                        if (canOwn) {
                            node.contents = buffer.subarray(offset, offset + length);
                            node.usedBytes = length;
                            return length
                        } else if (node.usedBytes === 0 && position === 0) {
                            node.contents = buffer.slice(offset, offset + length);
                            node.usedBytes = length;
                            return length
                        } else if (position + length <= node.usedBytes) {
                            node.contents.set(buffer.subarray(offset, offset + length), position);
                            return length
                        }
                    }
                    MEMFS.expandFileStorage(node, position + length);
                    if (node.contents.subarray && buffer.subarray) {
                        node.contents.set(buffer.subarray(offset, offset + length), position)
                    } else {
                        for (var i = 0; i < length; i++) {
                            node.contents[position + i] = buffer[offset + i]
                        }
                    }
                    node.usedBytes = Math.max(node.usedBytes, position + length);
                    return length
                },
                llseek(stream, offset, whence) {
                    var position = offset;
                    if (whence === 1) {
                        position += stream.position
                    } else if (whence === 2) {
                        if (FS.isFile(stream.node.mode)) {
                            position += stream.node.usedBytes
                        }
                    }
                    if (position < 0) {
                        throw new FS.ErrnoError(28)
                    }
                    return position
                },
                allocate(stream, offset, length) {
                    MEMFS.expandFileStorage(stream.node, offset + length);
                    stream.node.usedBytes = Math.max(stream.node.usedBytes, offset + length)
                },
                mmap(stream, length, position, prot, flags) {
                    if (!FS.isFile(stream.node.mode)) {
                        throw new FS.ErrnoError(43)
                    }
                    var ptr;
                    var allocated;
                    var contents = stream.node.contents;
                    if (!(flags & 2) && contents.buffer === HEAP8.buffer) {
                        allocated = false;
                        ptr = contents.byteOffset
                    } else {
                        if (position > 0 || position + length < contents.length) {
                            if (contents.subarray) {
                                contents = contents.subarray(position, position + length)
                            } else {
                                contents = Array.prototype.slice.call(contents, position, position + length)
                            }
                        }
                        allocated = true;
                        ptr = mmapAlloc(length);//@ts-ignore
                        if (!ptr) {
                            throw new FS.ErrnoError(48)
                        }
                        HEAP8.set(contents, ptr)
                    }
                    return {
                        ptr: ptr,
                        allocated: allocated
                    }
                },
                msync(stream, buffer, offset, length, mmapFlags) {
                    MEMFS.stream_ops.write(stream, buffer, 0, length, offset, false);
                    return 0
                }
            }
        };
        var asyncLoad = (url, onload, onerror, noRunDep) => {
            var dep = !noRunDep ? getUniqueRunDependency(`al ${url}`) : "";
            readAsync(url, arrayBuffer => {
                onload(new Uint8Array(arrayBuffer));
                if (dep)
                    removeRunDependency(dep)
            }, event => {
                if (onerror) {
                    onerror()
                } else {
                    throw `Loading data file "${url}" failed.`
                }
            });
            if (dep)
                addRunDependency(dep)
        };
        var FS_createDataFile = (parent, name, fileData, canRead, canWrite, canOwn) => {
            FS.createDataFile(parent, name, fileData, canRead, canWrite, canOwn)
        };
        var preloadPlugins = [];
        var FS_handledByPreloadPlugin = (byteArray, fullname, finish, onerror) => {//@ts-ignore
            if (typeof Browser != "undefined")//@ts-ignore
                Browser.init();
            var handled = false;
            preloadPlugins.forEach(plugin => {
                if (handled)
                    return;
                if (plugin["canHandle"](fullname)) {
                    plugin["handle"](byteArray, fullname, finish, onerror);
                    handled = true
                }
            });
            return handled
        };
        var FS_createPreloadedFile = (parent, name, url, canRead, canWrite, onload, onerror, dontCreateFile, canOwn, preFinish) => {
            var fullname = name ? PATH_FS.resolve(PATH.join2(parent, name)) : parent;
            var dep = getUniqueRunDependency(`cp ${fullname}`);
            function processData(byteArray) {
                function finish(byteArray) {
                    preFinish?.();
                    if (!dontCreateFile) {
                        FS_createDataFile(parent, name, byteArray, canRead, canWrite, canOwn)
                    }
                    onload?.();
                    removeRunDependency(dep)
                }
                if (FS_handledByPreloadPlugin(byteArray, fullname, finish, () => {
                        onerror?.();
                        removeRunDependency(dep)
                    })) {
                    return
                }
                finish(byteArray)
            }
            addRunDependency(dep);
            if (typeof url == "string") {
                asyncLoad(url, processData, onerror)
            } else {
                processData(url)
            }
        };
        var FS_modeStringToFlags = str => {
            var flagModes = {
                r: 0,
                "r+": 2,
                w: 512 | 64 | 1,
                "w+": 512 | 64 | 2,
                a: 1024 | 64 | 1,
                "a+": 1024 | 64 | 2
            };
            var flags = flagModes[str];
            if (typeof flags == "undefined") {
                throw new Error(`Unknown file open mode: ${str}`)
            }
            return flags
        };
        var FS_getMode = (canRead, canWrite) => {
            var mode = 0;
            if (canRead)
                mode |= 292 | 73;
            if (canWrite)
                mode |= 146;
            return mode
        };
        var FS = {
            root: null,
            mounts: [],
            devices: {},
            streams: [],
            nextInode: 1,
            nameTable: null,
            currentPath: "/",
            initialized: false,
            ignorePermissions: true,
            ErrnoError: class {
                constructor(errno) {
                    this.name = "ErrnoError";
                    this.errno = errno
                }
            },
            genericErrors: {},
            filesystems: null,
            syncFSRequests: 0,
            FSStream: class {
                constructor() {
                    this.shared = {}
                }
                get object() {
                    return this.node
                }
                set object(val) {
                    this.node = val
                }
                get isRead() {
                    return (this.flags & 2097155) !== 1
                }
                get isWrite() {
                    return (this.flags & 2097155) !== 0
                }
                get isAppend() {
                    return this.flags & 1024
                }
                get flags() {
                    return this.shared.flags
                }
                set flags(val) {
                    this.shared.flags = val
                }
                get position() {
                    return this.shared.position
                }
                set position(val) {
                    this.shared.position = val
                }
            },
            FSNode: class {
                constructor(parent, name, mode, rdev) {
                    if (!parent) {
                        parent = this
                    }
                    this.parent = parent;
                    this.mount = parent.mount;
                    this.mounted = null;
                    this.id = FS.nextInode++;
                    this.name = name;
                    this.mode = mode;
                    this.node_ops = {};
                    this.stream_ops = {};
                    this.rdev = rdev;
                    this.readMode = 292 | 73;
                    this.writeMode = 146
                }
                get read() {
                    return (this.mode & this.readMode) === this.readMode
                }
                set read(val) {
                    val ? this.mode |= this.readMode : this.mode &= ~this.readMode
                }
                get write() {
                    return (this.mode & this.writeMode) === this.writeMode
                }
                set write(val) {
                    val ? this.mode |= this.writeMode : this.mode &= ~this.writeMode
                }
                get isFolder() {
                    return FS.isDir(this.mode)
                }
                get isDevice() {
                    return FS.isChrdev(this.mode)
                }
            },
            lookupPath(path, opts = {}) {
                path = PATH_FS.resolve(path);
                if (!path)
                    return {
                        path: "",
                        node: null
                    };
                var defaults = {
                    follow_mount: true,
                    recurse_count: 0
                };
                opts = Object.assign(defaults, opts);
                if (opts.recurse_count > 8) {
                    throw new FS.ErrnoError(32)
                }
                var parts = path.split("/").filter(p => !!p);
                var current = FS.root;
                var current_path = "/";
                for (var i = 0; i < parts.length; i++) {
                    var islast = i === parts.length - 1;
                    if (islast && opts.parent) {
                        break
                    }
                    current = FS.lookupNode(current, parts[i]);
                    current_path = PATH.join2(current_path, parts[i]);
                    if (FS.isMountpoint(current)) {
                        if (!islast || islast && opts.follow_mount) {
                            current = current.mounted.root
                        }
                    }
                    if (!islast || opts.follow) {
                        var count = 0;
                        while (FS.isLink(current.mode)) {
                            var link = FS.readlink(current_path);
                            current_path = PATH_FS.resolve(PATH.dirname(current_path), link);
                            var lookup = FS.lookupPath(current_path, {
                                recurse_count: opts.recurse_count + 1
                            });
                            current = lookup.node;
                            if (count++ > 40) {
                                throw new FS.ErrnoError(32)
                            }
                        }
                    }
                }
                return {
                    path: current_path,
                    node: current
                }
            },
            getPath(node) {
                var path;
                while (true) {
                    if (FS.isRoot(node)) {
                        var mount = node.mount.mountpoint;
                        if (!path)
                            return mount;
                        return mount[mount.length - 1] !== "/" ? `${mount}/${path}` : mount + path
                    }
                    path = path ? `${node.name}/${path}` : node.name;
                    node = node.parent
                }
            },
            hashName(parentid, name) {
                var hash = 0;
                for (var i = 0; i < name.length; i++) {
                    hash = (hash << 5) - hash + name.charCodeAt(i) | 0
                }
                return (parentid + hash >>> 0) % FS.nameTable.length
            },
            hashAddNode(node) {
                var hash = FS.hashName(node.parent.id, node.name);
                node.name_next = FS.nameTable[hash];
                FS.nameTable[hash] = node
            },
            hashRemoveNode(node) {
                var hash = FS.hashName(node.parent.id, node.name);
                if (FS.nameTable[hash] === node) {
                    FS.nameTable[hash] = node.name_next
                } else {
                    var current = FS.nameTable[hash];
                    while (current) {
                        if (current.name_next === node) {
                            current.name_next = node.name_next;
                            break
                        }
                        current = current.name_next
                    }
                }
            },
            lookupNode(parent, name) {
                var errCode = FS.mayLookup(parent);
                if (errCode) {
                    throw new FS.ErrnoError(errCode)
                }
                var hash = FS.hashName(parent.id, name);
                for (var node = FS.nameTable[hash]; node; node = node.name_next) {
                    var nodeName = node.name;
                    if (node.parent.id === parent.id && nodeName === name) {
                        return node
                    }
                }
                return FS.lookup(parent, name)
            },
            createNode(parent, name, mode, rdev) {
                var node = new FS.FSNode(parent, name, mode, rdev);
                FS.hashAddNode(node);
                return node
            },
            destroyNode(node) {
                FS.hashRemoveNode(node)
            },
            isRoot(node) {
                return node === node.parent
            },
            isMountpoint(node) {
                return !!node.mounted
            },
            isFile(mode) {
                return (mode & 61440) === 32768
            },
            isDir(mode) {
                return (mode & 61440) === 16384
            },
            isLink(mode) {
                return (mode & 61440) === 40960
            },
            isChrdev(mode) {
                return (mode & 61440) === 8192
            },
            isBlkdev(mode) {
                return (mode & 61440) === 24576
            },
            isFIFO(mode) {
                return (mode & 61440) === 4096
            },
            isSocket(mode) {
                return (mode & 49152) === 49152
            },
            flagsToPermissionString(flag) {
                var perms = ["r", "w", "rw"][flag & 3];
                if (flag & 512) {
                    perms += "w"
                }
                return perms
            },
            nodePermissions(node, perms) {
                if (FS.ignorePermissions) {
                    return 0
                }
                if (perms.includes("r") && !(node.mode & 292)) {
                    return 2
                } else if (perms.includes("w") && !(node.mode & 146)) {
                    return 2
                } else if (perms.includes("x") && !(node.mode & 73)) {
                    return 2
                }
                return 0
            },
            mayLookup(dir) {
                if (!FS.isDir(dir.mode))
                    return 54;
                var errCode = FS.nodePermissions(dir, "x");
                if (errCode)
                    return errCode;
                if (!dir.node_ops.lookup)
                    return 2;
                return 0
            },
            mayCreate(dir, name) {
                try {
                    var node = FS.lookupNode(dir, name);
                    return 20
                } catch (e) {}
                return FS.nodePermissions(dir, "wx")
            },
            mayDelete(dir, name, isdir) {
                var node;
                try {
                    node = FS.lookupNode(dir, name)
                } catch (e) {
                    return e.errno
                }
                var errCode = FS.nodePermissions(dir, "wx");
                if (errCode) {
                    return errCode
                }
                if (isdir) {
                    if (!FS.isDir(node.mode)) {
                        return 54
                    }
                    if (FS.isRoot(node) || FS.getPath(node) === FS.cwd()) {
                        return 10
                    }
                } else {
                    if (FS.isDir(node.mode)) {
                        return 31
                    }
                }
                return 0
            },
            mayOpen(node, flags) {
                if (!node) {
                    return 44
                }
                if (FS.isLink(node.mode)) {
                    return 32
                } else if (FS.isDir(node.mode)) {
                    if (FS.flagsToPermissionString(flags) !== "r" || flags & 512) {
                        return 31
                    }
                }
                return FS.nodePermissions(node, FS.flagsToPermissionString(flags))
            },
            MAX_OPEN_FDS: 4096,
            nextfd() {
                for (var fd = 0; fd <= FS.MAX_OPEN_FDS; fd++) {
                    if (!FS.streams[fd]) {
                        return fd
                    }
                }
                throw new FS.ErrnoError(33)
            },
            getStreamChecked(fd) {
                var stream = FS.getStream(fd);
                if (!stream) {
                    throw new FS.ErrnoError(8)
                }
                return stream
            },
            getStream: fd => FS.streams[fd],
            createStream(stream, fd = -1) {
                stream = Object.assign(new FS.FSStream, stream);
                if (fd == -1) {
                    fd = FS.nextfd()
                }
                stream.fd = fd;
                FS.streams[fd] = stream;
                return stream
            },
            closeStream(fd) {
                FS.streams[fd] = null
            },
            dupStream(origStream, fd = -1) {
                var stream = FS.createStream(origStream, fd);
                stream.stream_ops?.dup?.(stream);
                return stream
            },
            chrdev_stream_ops: {
                open(stream) {
                    var device = FS.getDevice(stream.node.rdev);
                    stream.stream_ops = device.stream_ops;
                    stream.stream_ops.open?.(stream)
                },
                llseek() {
                    throw new FS.ErrnoError(70)
                }
            },
            major: dev => dev >> 8,
            minor: dev => dev & 255,
            makedev: (ma, mi) => ma << 8 | mi,
            registerDevice(dev, ops) {
                FS.devices[dev] = {
                    stream_ops: ops
                }
            },
            getDevice: dev => FS.devices[dev],
            getMounts(mount) {
                var mounts = [];
                var check = [mount];
                while (check.length) {
                    var m = check.pop();
                    mounts.push(m);
                    check.push(...m.mounts)
                }
                return mounts
            },
            syncfs(populate, callback) {
                if (typeof populate == "function") {
                    callback = populate;
                    populate = false
                }
                FS.syncFSRequests++;
                if (FS.syncFSRequests > 1) {
                    err(`warning: ${FS.syncFSRequests} FS.syncfs operations in flight at once, probably just doing extra work`)
                }
                var mounts = FS.getMounts(FS.root.mount);
                var completed = 0;
                function doCallback(errCode) {
                    FS.syncFSRequests--;
                    return callback(errCode)
                }
                function done(errCode) {
                    if (errCode) {//@ts-ignore
                        if (!done.errored) {//@ts-ignore
                            done.errored = true;
                            return doCallback(errCode)
                        }
                        return
                    }
                    if (++completed >= mounts.length) {
                        doCallback(null)
                    }
                }
                mounts.forEach(mount => {
                    if (!mount.type.syncfs) {
                        return done(null)
                    }
                    mount.type.syncfs(mount, populate, done)
                })
            },
            mount(type, opts, mountpoint) {
                var root = mountpoint === "/";
                var pseudo = !mountpoint;
                var node;
                if (root && FS.root) {
                    throw new FS.ErrnoError(10)
                } else if (!root && !pseudo) {
                    var lookup = FS.lookupPath(mountpoint, {
                        follow_mount: false
                    });
                    mountpoint = lookup.path;
                    node = lookup.node;
                    if (FS.isMountpoint(node)) {
                        throw new FS.ErrnoError(10)
                    }
                    if (!FS.isDir(node.mode)) {
                        throw new FS.ErrnoError(54)
                    }
                }
                var mount = {
                    type: type,
                    opts: opts,
                    mountpoint: mountpoint,
                    mounts: []
                };
                var mountRoot = type.mount(mount);
                mountRoot.mount = mount;
                mount.root = mountRoot;
                if (root) {
                    FS.root = mountRoot
                } else if (node) {
                    node.mounted = mount;
                    if (node.mount) {
                        node.mount.mounts.push(mount)
                    }
                }
                return mountRoot
            },
            unmount(mountpoint) {
                var lookup = FS.lookupPath(mountpoint, {
                    follow_mount: false
                });
                if (!FS.isMountpoint(lookup.node)) {
                    throw new FS.ErrnoError(28)
                }
                var node = lookup.node;
                var mount = node.mounted;
                var mounts = FS.getMounts(mount);
                Object.keys(FS.nameTable).forEach(hash => {
                    var current = FS.nameTable[hash];
                    while (current) {
                        var next = current.name_next;
                        if (mounts.includes(current.mount)) {
                            FS.destroyNode(current)
                        }
                        current = next
                    }
                });
                node.mounted = null;
                var idx = node.mount.mounts.indexOf(mount);
                node.mount.mounts.splice(idx, 1)
            },
            lookup(parent, name) {
                return parent.node_ops.lookup(parent, name)
            },
            mknod(path, mode, dev) {
                var lookup = FS.lookupPath(path, {
                    parent: true
                });
                var parent = lookup.node;
                var name = PATH.basename(path);
                if (!name || name === "." || name === "..") {
                    throw new FS.ErrnoError(28)
                }
                var errCode = FS.mayCreate(parent, name);
                if (errCode) {
                    throw new FS.ErrnoError(errCode)
                }
                if (!parent.node_ops.mknod) {
                    throw new FS.ErrnoError(63)
                }
                return parent.node_ops.mknod(parent, name, mode, dev)
            },
            create(path, mode) {
                mode = mode !== undefined ? mode : 438;
                mode &= 4095;
                mode |= 32768;
                return FS.mknod(path, mode, 0)
            },
            mkdir(path, mode) {
                mode = mode !== undefined ? mode : 511;
                mode &= 511 | 512;
                mode |= 16384;
                return FS.mknod(path, mode, 0)
            },
            mkdirTree(path, mode) {
                var dirs = path.split("/");
                var d = "";
                for (var i = 0; i < dirs.length; ++i) {
                    if (!dirs[i])
                        continue;
                    d += "/" + dirs[i];
                    try {
                        FS.mkdir(d, mode)
                    } catch (e) {
                        if (e.errno != 20)
                            throw e
                    }
                }
            },
            mkdev(path, mode, dev) {
                if (typeof dev == "undefined") {
                    dev = mode;
                    mode = 438
                }
                mode |= 8192;
                return FS.mknod(path, mode, dev)
            },
            symlink(oldpath, newpath) {
                if (!PATH_FS.resolve(oldpath)) {
                    throw new FS.ErrnoError(44)
                }
                var lookup = FS.lookupPath(newpath, {
                    parent: true
                });
                var parent = lookup.node;
                if (!parent) {
                    throw new FS.ErrnoError(44)
                }
                var newname = PATH.basename(newpath);
                var errCode = FS.mayCreate(parent, newname);
                if (errCode) {
                    throw new FS.ErrnoError(errCode)
                }
                if (!parent.node_ops.symlink) {
                    throw new FS.ErrnoError(63)
                }
                return parent.node_ops.symlink(parent, newname, oldpath)
            },
            rename(old_path, new_path) {
                var old_dirname = PATH.dirname(old_path);
                var new_dirname = PATH.dirname(new_path);
                var old_name = PATH.basename(old_path);
                var new_name = PATH.basename(new_path);
                var lookup,
                old_dir,
                new_dir;
                lookup = FS.lookupPath(old_path, {
                    parent: true
                });
                old_dir = lookup.node;
                lookup = FS.lookupPath(new_path, {
                    parent: true
                });
                new_dir = lookup.node;
                if (!old_dir || !new_dir)
                    throw new FS.ErrnoError(44);
                if (old_dir.mount !== new_dir.mount) {
                    throw new FS.ErrnoError(75)
                }
                var old_node = FS.lookupNode(old_dir, old_name);
                var relative = PATH_FS.relative(old_path, new_dirname);
                if (relative.charAt(0) !== ".") {
                    throw new FS.ErrnoError(28)
                }
                relative = PATH_FS.relative(new_path, old_dirname);
                if (relative.charAt(0) !== ".") {
                    throw new FS.ErrnoError(55)
                }
                var new_node;
                try {
                    new_node = FS.lookupNode(new_dir, new_name)
                } catch (e) {}
                if (old_node === new_node) {
                    return
                }
                var isdir = FS.isDir(old_node.mode);
                var errCode = FS.mayDelete(old_dir, old_name, isdir);
                if (errCode) {
                    throw new FS.ErrnoError(errCode)
                }
                errCode = new_node ? FS.mayDelete(new_dir, new_name, isdir) : FS.mayCreate(new_dir, new_name);
                if (errCode) {
                    throw new FS.ErrnoError(errCode)
                }
                if (!old_dir.node_ops.rename) {
                    throw new FS.ErrnoError(63)
                }
                if (FS.isMountpoint(old_node) || new_node && FS.isMountpoint(new_node)) {
                    throw new FS.ErrnoError(10)
                }
                if (new_dir !== old_dir) {
                    errCode = FS.nodePermissions(old_dir, "w");
                    if (errCode) {
                        throw new FS.ErrnoError(errCode)
                    }
                }
                FS.hashRemoveNode(old_node);
                try {
                    old_dir.node_ops.rename(old_node, new_dir, new_name)
                } catch (e) {
                    throw e
                } finally {
                    FS.hashAddNode(old_node)
                }
            },
            rmdir(path) {
                var lookup = FS.lookupPath(path, {
                    parent: true
                });
                var parent = lookup.node;
                var name = PATH.basename(path);
                var node = FS.lookupNode(parent, name);
                var errCode = FS.mayDelete(parent, name, true);
                if (errCode) {
                    throw new FS.ErrnoError(errCode)
                }
                if (!parent.node_ops.rmdir) {
                    throw new FS.ErrnoError(63)
                }
                if (FS.isMountpoint(node)) {
                    throw new FS.ErrnoError(10)
                }
                parent.node_ops.rmdir(parent, name);
                FS.destroyNode(node)
            },
            readdir(path) {
                var lookup = FS.lookupPath(path, {
                    follow: true
                });
                var node = lookup.node;
                if (!node.node_ops.readdir) {
                    throw new FS.ErrnoError(54)
                }
                return node.node_ops.readdir(node)
            },
            unlink(path) {
                var lookup = FS.lookupPath(path, {
                    parent: true
                });
                var parent = lookup.node;
                if (!parent) {
                    throw new FS.ErrnoError(44)
                }
                var name = PATH.basename(path);
                var node = FS.lookupNode(parent, name);
                var errCode = FS.mayDelete(parent, name, false);
                if (errCode) {
                    throw new FS.ErrnoError(errCode)
                }
                if (!parent.node_ops.unlink) {
                    throw new FS.ErrnoError(63)
                }
                if (FS.isMountpoint(node)) {
                    throw new FS.ErrnoError(10)
                }
                parent.node_ops.unlink(parent, name);
                FS.destroyNode(node)
            },
            readlink(path) {
                var lookup = FS.lookupPath(path);
                var link = lookup.node;
                if (!link) {
                    throw new FS.ErrnoError(44)
                }
                if (!link.node_ops.readlink) {
                    throw new FS.ErrnoError(28)
                }
                return PATH_FS.resolve(FS.getPath(link.parent), link.node_ops.readlink(link))
            },
            stat(path, dontFollow) {
                var lookup = FS.lookupPath(path, {
                    follow: !dontFollow
                });
                var node = lookup.node;
                if (!node) {
                    throw new FS.ErrnoError(44)
                }
                if (!node.node_ops.getattr) {
                    throw new FS.ErrnoError(63)
                }
                return node.node_ops.getattr(node)
            },
            lstat(path) {
                return FS.stat(path, true)
            },
            chmod(path, mode, dontFollow) {
                var node;
                if (typeof path == "string") {
                    var lookup = FS.lookupPath(path, {
                        follow: !dontFollow
                    });
                    node = lookup.node
                } else {
                    node = path
                }
                if (!node.node_ops.setattr) {
                    throw new FS.ErrnoError(63)
                }
                node.node_ops.setattr(node, {
                    mode: mode & 4095 | node.mode & ~4095,
                    timestamp: Date.now()
                })
            },
            lchmod(path, mode) {
                FS.chmod(path, mode, true)
            },
            fchmod(fd, mode) {
                var stream = FS.getStreamChecked(fd);
                FS.chmod(stream.node, mode)
            },
            chown(path, uid, gid, dontFollow) {
                var node;
                if (typeof path == "string") {
                    var lookup = FS.lookupPath(path, {
                        follow: !dontFollow
                    });
                    node = lookup.node
                } else {
                    node = path
                }
                if (!node.node_ops.setattr) {
                    throw new FS.ErrnoError(63)
                }
                node.node_ops.setattr(node, {
                    timestamp: Date.now()
                })
            },
            lchown(path, uid, gid) {
                FS.chown(path, uid, gid, true)
            },
            fchown(fd, uid, gid) {
                var stream = FS.getStreamChecked(fd);
                FS.chown(stream.node, uid, gid)
            },
            truncate(path, len) {
                if (len < 0) {
                    throw new FS.ErrnoError(28)
                }
                var node;
                if (typeof path == "string") {
                    var lookup = FS.lookupPath(path, {
                        follow: true
                    });
                    node = lookup.node
                } else {
                    node = path
                }
                if (!node.node_ops.setattr) {
                    throw new FS.ErrnoError(63)
                }
                if (FS.isDir(node.mode)) {
                    throw new FS.ErrnoError(31)
                }
                if (!FS.isFile(node.mode)) {
                    throw new FS.ErrnoError(28)
                }
                var errCode = FS.nodePermissions(node, "w");
                if (errCode) {
                    throw new FS.ErrnoError(errCode)
                }
                node.node_ops.setattr(node, {
                    size: len,
                    timestamp: Date.now()
                })
            },
            ftruncate(fd, len) {
                var stream = FS.getStreamChecked(fd);
                if ((stream.flags & 2097155) === 0) {
                    throw new FS.ErrnoError(28)
                }
                FS.truncate(stream.node, len)
            },
            utime(path, atime, mtime) {
                var lookup = FS.lookupPath(path, {
                    follow: true
                });
                var node = lookup.node;
                node.node_ops.setattr(node, {
                    timestamp: Math.max(atime, mtime)
                })
            },
            open(path, flags, mode) {
                if (path === "") {
                    throw new FS.ErrnoError(44)
                }
                flags = typeof flags == "string" ? FS_modeStringToFlags(flags) : flags;
                if (flags & 64) {
                    mode = typeof mode == "undefined" ? 438 : mode;
                    mode = mode & 4095 | 32768
                } else {
                    mode = 0
                }
                var node;
                if (typeof path == "object") {
                    node = path
                } else {
                    path = PATH.normalize(path);
                    try {
                        var lookup = FS.lookupPath(path, {
                            follow: !(flags & 131072)
                        });
                        node = lookup.node
                    } catch (e) {}
                }
                var created = false;
                if (flags & 64) {
                    if (node) {
                        if (flags & 128) {
                            throw new FS.ErrnoError(20)
                        }
                    } else {
                        node = FS.mknod(path, mode, 0);
                        created = true
                    }
                }
                if (!node) {
                    throw new FS.ErrnoError(44)
                }
                if (FS.isChrdev(node.mode)) {
                    flags &= ~512
                }
                if (flags & 65536 && !FS.isDir(node.mode)) {
                    throw new FS.ErrnoError(54)
                }
                if (!created) {
                    var errCode = FS.mayOpen(node, flags);
                    if (errCode) {
                        throw new FS.ErrnoError(errCode)
                    }
                }
                if (flags & 512 && !created) {
                    FS.truncate(node, 0)
                }
                flags &= ~(128 | 512 | 131072);
                var stream = FS.createStream({
                    node: node,
                    path: FS.getPath(node),
                    flags: flags,
                    seekable: true,
                    position: 0,
                    stream_ops: node.stream_ops,
                    ungotten: [],
                    error: false
                });
                if (stream.stream_ops.open) {
                    stream.stream_ops.open(stream)
                }
                if (Module["logReadFiles"] && !(flags & 1)) {
                    if (!FS.readFiles)
                        FS.readFiles = {};
                    if (!(path in FS.readFiles)) {
                        FS.readFiles[path] = 1
                    }
                }
                return stream
            },
            close(stream) {
                if (FS.isClosed(stream)) {
                    throw new FS.ErrnoError(8)
                }
                if (stream.getdents)
                    stream.getdents = null;
                try {
                    if (stream.stream_ops.close) {
                        stream.stream_ops.close(stream)
                    }
                } catch (e) {
                    throw e
                } finally {
                    FS.closeStream(stream.fd)
                }
                stream.fd = null
            },
            isClosed(stream) {
                return stream.fd === null
            },
            llseek(stream, offset, whence) {
                if (FS.isClosed(stream)) {
                    throw new FS.ErrnoError(8)
                }
                if (!stream.seekable || !stream.stream_ops.llseek) {
                    throw new FS.ErrnoError(70)
                }
                if (whence != 0 && whence != 1 && whence != 2) {
                    throw new FS.ErrnoError(28)
                }
                stream.position = stream.stream_ops.llseek(stream, offset, whence);
                stream.ungotten = [];
                return stream.position
            },
            read(stream, buffer, offset, length, position) {
                if (length < 0 || position < 0) {
                    throw new FS.ErrnoError(28)
                }
                if (FS.isClosed(stream)) {
                    throw new FS.ErrnoError(8)
                }
                if ((stream.flags & 2097155) === 1) {
                    throw new FS.ErrnoError(8)
                }
                if (FS.isDir(stream.node.mode)) {
                    throw new FS.ErrnoError(31)
                }
                if (!stream.stream_ops.read) {
                    throw new FS.ErrnoError(28)
                }
                var seeking = typeof position != "undefined";
                if (!seeking) {
                    position = stream.position
                } else if (!stream.seekable) {
                    throw new FS.ErrnoError(70)
                }
                var bytesRead = stream.stream_ops.read(stream, buffer, offset, length, position);
                if (!seeking)
                    stream.position += bytesRead;
                return bytesRead
            },
            write(stream, buffer, offset, length, position, canOwn) {
                if (length < 0 || position < 0) {
                    throw new FS.ErrnoError(28)
                }
                if (FS.isClosed(stream)) {
                    throw new FS.ErrnoError(8)
                }
                if ((stream.flags & 2097155) === 0) {
                    throw new FS.ErrnoError(8)
                }
                if (FS.isDir(stream.node.mode)) {
                    throw new FS.ErrnoError(31)
                }
                if (!stream.stream_ops.write) {
                    throw new FS.ErrnoError(28)
                }
                if (stream.seekable && stream.flags & 1024) {
                    FS.llseek(stream, 0, 2)
                }
                var seeking = typeof position != "undefined";
                if (!seeking) {
                    position = stream.position
                } else if (!stream.seekable) {
                    throw new FS.ErrnoError(70)
                }
                var bytesWritten = stream.stream_ops.write(stream, buffer, offset, length, position, canOwn);
                if (!seeking)
                    stream.position += bytesWritten;
                return bytesWritten
            },
            allocate(stream, offset, length) {
                if (FS.isClosed(stream)) {
                    throw new FS.ErrnoError(8)
                }
                if (offset < 0 || length <= 0) {
                    throw new FS.ErrnoError(28)
                }
                if ((stream.flags & 2097155) === 0) {
                    throw new FS.ErrnoError(8)
                }
                if (!FS.isFile(stream.node.mode) && !FS.isDir(stream.node.mode)) {
                    throw new FS.ErrnoError(43)
                }
                if (!stream.stream_ops.allocate) {
                    throw new FS.ErrnoError(138)
                }
                stream.stream_ops.allocate(stream, offset, length)
            },
            mmap(stream, length, position, prot, flags) {
                if ((prot & 2) !== 0 && (flags & 2) === 0 && (stream.flags & 2097155) !== 2) {
                    throw new FS.ErrnoError(2)
                }
                if ((stream.flags & 2097155) === 1) {
                    throw new FS.ErrnoError(2)
                }
                if (!stream.stream_ops.mmap) {
                    throw new FS.ErrnoError(43)
                }
                return stream.stream_ops.mmap(stream, length, position, prot, flags)
            },
            msync(stream, buffer, offset, length, mmapFlags) {
                if (!stream.stream_ops.msync) {
                    return 0
                }
                return stream.stream_ops.msync(stream, buffer, offset, length, mmapFlags)
            },
            ioctl(stream, cmd, arg) {
                if (!stream.stream_ops.ioctl) {
                    throw new FS.ErrnoError(59)
                }
                return stream.stream_ops.ioctl(stream, cmd, arg)
            },
            readFile(path, opts = {}) {
                opts.flags = opts.flags || 0;
                opts.encoding = opts.encoding || "binary";
                if (opts.encoding !== "utf8" && opts.encoding !== "binary") {
                    throw new Error(`Invalid encoding type "${opts.encoding}"`)
                }
                var ret;
                var stream = FS.open(path, opts.flags);
                var stat = FS.stat(path);
                var length = stat.size;
                var buf = new Uint8Array(length);
                FS.read(stream, buf, 0, length, 0);
                if (opts.encoding === "utf8") {
                    ret = UTF8ArrayToString(buf, 0)
                } else if (opts.encoding === "binary") {
                    ret = buf
                }
                FS.close(stream);
                return ret
            },
            writeFile(path, data, opts = {}) {
                opts.flags = opts.flags || 577;
                var stream = FS.open(path, opts.flags, opts.mode);
                if (typeof data == "string") {
                    var buf = new Uint8Array(lengthBytesUTF8(data) + 1);
                    var actualNumBytes = stringToUTF8Array(data, buf, 0, buf.length);
                    FS.write(stream, buf, 0, actualNumBytes, undefined, opts.canOwn)
                } else if (ArrayBuffer.isView(data)) {
                    FS.write(stream, data, 0, data.byteLength, undefined, opts.canOwn)
                } else {
                    throw new Error("Unsupported data type")
                }
                FS.close(stream)
            },
            cwd: () => FS.currentPath,
            chdir(path) {
                var lookup = FS.lookupPath(path, {
                    follow: true
                });
                if (lookup.node === null) {
                    throw new FS.ErrnoError(44)
                }
                if (!FS.isDir(lookup.node.mode)) {
                    throw new FS.ErrnoError(54)
                }
                var errCode = FS.nodePermissions(lookup.node, "x");
                if (errCode) {
                    throw new FS.ErrnoError(errCode)
                }
                FS.currentPath = lookup.path
            },
            createDefaultDirectories() {
                FS.mkdir("/tmp");
                FS.mkdir("/home");
                FS.mkdir("/home/web_user")
            },
            createDefaultDevices() {
                FS.mkdir("/dev");
                FS.registerDevice(FS.makedev(1, 3), {
                    read: () => 0,
                    write: (stream, buffer, offset, length, pos) => length
                });
                FS.mkdev("/dev/null", FS.makedev(1, 3));
                TTY.register(FS.makedev(5, 0), TTY.default_tty_ops);
                TTY.register(FS.makedev(6, 0), TTY.default_tty1_ops);
                FS.mkdev("/dev/tty", FS.makedev(5, 0));
                FS.mkdev("/dev/tty1", FS.makedev(6, 0));
                var randomBuffer = new Uint8Array(1024),
                randomLeft = 0;
                var randomByte = () => {
                    if (randomLeft === 0) {
                        randomLeft = randomFill(randomBuffer).byteLength
                    }
                    return randomBuffer[--randomLeft]
                };
                FS.createDevice("/dev", "random", randomByte);
                FS.createDevice("/dev", "urandom", randomByte);
                FS.mkdir("/dev/shm");
                FS.mkdir("/dev/shm/tmp")
            },
            createSpecialDirectories() {
                FS.mkdir("/proc");
                var proc_self = FS.mkdir("/proc/self");
                FS.mkdir("/proc/self/fd");
                FS.mount({
                    mount() {
                        var node = FS.createNode(proc_self, "fd", 16384 | 511, 73);
                        node.node_ops = {
                            lookup(parent, name) {
                                var fd = +name;
                                var stream = FS.getStreamChecked(fd);
                                var ret = {
                                    parent: null,
                                    mount: {
                                        mountpoint: "fake"
                                    },
                                    node_ops: {
                                        readlink: () => stream.path
                                    }
                                };
                                ret.parent = ret;
                                return ret
                            }
                        };
                        return node
                    }
                }, {}, "/proc/self/fd")
            },
            createStandardStreams() {
                if (Module["stdin"]) {
                    FS.createDevice("/dev", "stdin", Module["stdin"])
                } else {
                    FS.symlink("/dev/tty", "/dev/stdin")
                }
                if (Module["stdout"]) {
                    FS.createDevice("/dev", "stdout", null, Module["stdout"])
                } else {
                    FS.symlink("/dev/tty", "/dev/stdout")
                }
                if (Module["stderr"]) {
                    FS.createDevice("/dev", "stderr", null, Module["stderr"])
                } else {
                    FS.symlink("/dev/tty1", "/dev/stderr")
                }
                var stdin = FS.open("/dev/stdin", 0);
                var stdout = FS.open("/dev/stdout", 1);
                var stderr = FS.open("/dev/stderr", 1)
            },
            staticInit() {
                [44].forEach(code => {
                    FS.genericErrors[code] = new FS.ErrnoError(code);
                    FS.genericErrors[code].stack = "<generic error, no stack>"
                });
                FS.nameTable = new Array(4096);
                FS.mount(MEMFS, {}, "/");
                FS.createDefaultDirectories();
                FS.createDefaultDevices();
                FS.createSpecialDirectories();
                FS.filesystems = {
                    MEMFS: MEMFS
                }
            },
            init(input, output, error) {//@ts-ignore
                FS.init.initialized = true;
                Module["stdin"] = input || Module["stdin"];
                Module["stdout"] = output || Module["stdout"];
                Module["stderr"] = error || Module["stderr"];
                FS.createStandardStreams()
            },
            quit() {//@ts-ignore
                FS.init.initialized = false;
                for (var i = 0; i < FS.streams.length; i++) {
                    var stream = FS.streams[i];
                    if (!stream) {
                        continue
                    }
                    FS.close(stream)
                }
            },
            findObject(path, dontResolveLastLink) {
                var ret = FS.analyzePath(path, dontResolveLastLink);
                if (!ret.exists) {
                    return null
                }
                return ret.object
            },
            analyzePath(path, dontResolveLastLink) {
                try {
                    var lookup = FS.lookupPath(path, {
                        follow: !dontResolveLastLink
                    });
                    path = lookup.path
                } catch (e) {}
                var ret = {
                    isRoot: false,
                    exists: false,
                    error: 0,
                    name: null,
                    path: null,
                    object: null,
                    parentExists: false,
                    parentPath: null,
                    parentObject: null
                };
                try {
                    var lookup = FS.lookupPath(path, {
                        parent: true
                    });
                    ret.parentExists = true;
                    ret.parentPath = lookup.path;
                    ret.parentObject = lookup.node;
                    ret.name = PATH.basename(path);
                    lookup = FS.lookupPath(path, {
                        follow: !dontResolveLastLink
                    });
                    ret.exists = true;
                    ret.path = lookup.path;
                    ret.object = lookup.node;
                    ret.name = lookup.node.name;
                    ret.isRoot = lookup.path === "/"
                } catch (e) {
                    ret.error = e.errno
                }
                return ret
            },
            createPath(parent, path, canRead, canWrite) {
                parent = typeof parent == "string" ? parent : FS.getPath(parent);
                var parts = path.split("/").reverse();
                while (parts.length) {
                    var part = parts.pop();
                    if (!part)
                        continue;
                    var current = PATH.join2(parent, part);
                    try {
                        FS.mkdir(current)
                    } catch (e) {}
                    parent = current
                }
                return current
            },
            createFile(parent, name, properties, canRead, canWrite) {
                var path = PATH.join2(typeof parent == "string" ? parent : FS.getPath(parent), name);
                var mode = FS_getMode(canRead, canWrite);
                return FS.create(path, mode)
            },
            createDataFile(parent, name, data, canRead, canWrite, canOwn) {
                var path = name;
                if (parent) {
                    parent = typeof parent == "string" ? parent : FS.getPath(parent);
                    path = name ? PATH.join2(parent, name) : parent
                }
                var mode = FS_getMode(canRead, canWrite);
                var node = FS.create(path, mode);
                if (data) {
                    if (typeof data == "string") {
                        var arr = new Array(data.length);
                        for (var i = 0, len = data.length; i < len; ++i)
                            arr[i] = data.charCodeAt(i);
                        data = arr
                    }
                    FS.chmod(node, mode | 146);
                    var stream = FS.open(node, 577);
                    FS.write(stream, data, 0, data.length, 0, canOwn);
                    FS.close(stream);
                    FS.chmod(node, mode)
                }
            },
            createDevice(parent, name, input, output) {
                var path = PATH.join2(typeof parent == "string" ? parent : FS.getPath(parent), name);
                var mode = FS_getMode(!!input, !!output);//@ts-ignore
                if (!FS.createDevice.major)//@ts-ignore
                    FS.createDevice.major = 64;//@ts-ignore
                var dev = FS.makedev(FS.createDevice.major++, 0);
                FS.registerDevice(dev, {
                    open(stream) {
                        stream.seekable = false
                    },
                    close(stream) {
                        if (output?.buffer?.length) {
                            output(10)
                        }
                    },
                    read(stream, buffer, offset, length, pos) {
                        var bytesRead = 0;
                        for (var i = 0; i < length; i++) {
                            var result;
                            try {
                                result = input()
                            } catch (e) {
                                throw new FS.ErrnoError(29)
                            }
                            if (result === undefined && bytesRead === 0) {
                                throw new FS.ErrnoError(6)
                            }
                            if (result === null || result === undefined)
                                break;
                            bytesRead++;
                            buffer[offset + i] = result
                        }
                        if (bytesRead) {
                            stream.node.timestamp = Date.now()
                        }
                        return bytesRead
                    },
                    write(stream, buffer, offset, length, pos) {
                        for (var i = 0; i < length; i++) {
                            try {
                                output(buffer[offset + i])
                            } catch (e) {
                                throw new FS.ErrnoError(29)
                            }
                        }
                        if (length) {
                            stream.node.timestamp = Date.now()
                        }
                        return i
                    }
                });
                return FS.mkdev(path, mode, dev)
            },
            forceLoadFile(obj) {
                if (obj.isDevice || obj.isFolder || obj.link || obj.contents)
                    return true;
                if (typeof XMLHttpRequest != "undefined") {
                    throw new Error("Lazy loading should have been performed (contents set) in createLazyFile, but it was not. Lazy loading only works in web workers. Use --embed-file or --preload-file in emcc on the main thread.")
                } else if (read_) {
                    try {
                        obj.contents = intArrayFromString(read_(obj.url), true);
                        obj.usedBytes = obj.contents.length
                    } catch (e) {
                        throw new FS.ErrnoError(29)
                    }
                } else {
                    throw new Error("Cannot load without read() or XMLHttpRequest.")
                }
            },
            createLazyFile(parent, name, url, canRead, canWrite) {
                class LazyUint8Array {
                    constructor() {
                        this.lengthKnown = false;
                        this.chunks = []
                    }
                    get(idx) {
                        if (idx > this.length - 1 || idx < 0) {
                            return undefined
                        }
                        var chunkOffset = idx % this.chunkSize;
                        var chunkNum = idx / this.chunkSize | 0;
                        return this.getter(chunkNum)[chunkOffset]
                    }
                    setDataGetter(getter) {
                        this.getter = getter
                    }
                    cacheLength() {
                        var xhr = new XMLHttpRequest;
                        xhr.open("HEAD", url, false);
                        xhr.send(null);
                        if (!(xhr.status >= 200 && xhr.status < 300 || xhr.status === 304))
                            throw new Error("Couldn't load " + url + ". Status: " + xhr.status);
                        var datalength = Number(xhr.getResponseHeader("Content-length"));
                        var header;
                        var hasByteServing = (header = xhr.getResponseHeader("Accept-Ranges")) && header === "bytes";
                        var usesGzip = (header = xhr.getResponseHeader("Content-Encoding")) && header === "gzip";
                        var chunkSize = 1024 * 1024;
                        if (!hasByteServing)
                            chunkSize = datalength;
                        var doXHR = (from, to) => {
                            if (from > to)
                                throw new Error("invalid range (" + from + ", " + to + ") or no bytes requested!");
                            if (to > datalength - 1)
                                throw new Error("only " + datalength + " bytes available! programmer error!");
                            var xhr = new XMLHttpRequest;
                            xhr.open("GET", url, false);
                            if (datalength !== chunkSize)
                                xhr.setRequestHeader("Range", "bytes=" + from + "-" + to);
                            xhr.responseType = "arraybuffer";
                            if (xhr.overrideMimeType) {
                                xhr.overrideMimeType("text/plain; charset=x-user-defined")
                            }
                            xhr.send(null);
                            if (!(xhr.status >= 200 && xhr.status < 300 || xhr.status === 304))
                                throw new Error("Couldn't load " + url + ". Status: " + xhr.status);
                            if (xhr.response !== undefined) {
                                return new Uint8Array(xhr.response || [])
                            }
                            return intArrayFromString(xhr.responseText || "", true)
                        };
                        var lazyArray = this;
                        lazyArray.setDataGetter(chunkNum => {
                            var start = chunkNum * chunkSize;
                            var end = (chunkNum + 1) * chunkSize - 1;
                            end = Math.min(end, datalength - 1);
                            if (typeof lazyArray.chunks[chunkNum] == "undefined") {
                                lazyArray.chunks[chunkNum] = doXHR(start, end)
                            }
                            if (typeof lazyArray.chunks[chunkNum] == "undefined")
                                throw new Error("doXHR failed!");
                            return lazyArray.chunks[chunkNum]
                        });
                        if (usesGzip || !datalength) {
                            chunkSize = datalength = 1;
                            datalength = this.getter(0).length;
                            chunkSize = datalength;
                            out("LazyFiles on gzip forces download of the whole file when length is accessed")
                        }
                        this._length = datalength;
                        this._chunkSize = chunkSize;
                        this.lengthKnown = true
                    }
                    get length() {
                        if (!this.lengthKnown) {
                            this.cacheLength()
                        }
                        return this._length
                    }
                    get chunkSize() {
                        if (!this.lengthKnown) {
                            this.cacheLength()
                        }
                        return this._chunkSize
                    }
                }
                if (typeof XMLHttpRequest != "undefined") {
                    if (!ENVIRONMENT_IS_WORKER)
                        throw "Cannot do synchronous binary XHRs outside webworkers in modern browsers. Use --embed-file or --preload-file in emcc";
                    var lazyArray = new LazyUint8Array;
                    var properties = {
                        isDevice: false,
                        contents: lazyArray
                    }
                } else {//@ts-ignore
                    var properties = {
                        isDevice: false,
                        url: url
                    }
                }
                var node = FS.createFile(parent, name, properties, canRead, canWrite);
                if (properties.contents) {
                    node.contents = properties.contents
                } else if (properties.url) {
                    node.contents = null;
                    node.url = properties.url
                }
                Object.defineProperties(node, {
                    usedBytes: {
                        get: function () {
                            return this.contents.length
                        }
                    }
                });
                var stream_ops = {};
                var keys = Object.keys(node.stream_ops);
                keys.forEach(key => {
                    var fn = node.stream_ops[key];
                    stream_ops[key] = (...args) => {
                        FS.forceLoadFile(node);
                        return fn(...args)
                    }
                });
                function writeChunks(stream, buffer, offset, length, position) {
                    var contents = stream.node.contents;
                    if (position >= contents.length)
                        return 0;
                    var size = Math.min(contents.length - position, length);
                    if (contents.slice) {
                        for (var i = 0; i < size; i++) {
                            buffer[offset + i] = contents[position + i]
                        }
                    } else {
                        for (var i = 0; i < size; i++) {
                            buffer[offset + i] = contents.get(position + i)
                        }
                    }
                    return size
                }
                stream_ops.read = (stream, buffer, offset, length, position) => {
                    FS.forceLoadFile(node);
                    return writeChunks(stream, buffer, offset, length, position)
                };
                stream_ops.mmap = (stream, length, position, prot, flags) => {
                    FS.forceLoadFile(node);
                    var ptr = mmapAlloc(length);//@ts-ignore
                    if (!ptr) {
                        throw new FS.ErrnoError(48)
                    }
                    writeChunks(stream, HEAP8, ptr, length, position);
                    return {
                        ptr: ptr,
                        allocated: true
                    }
                };
                node.stream_ops = stream_ops;
                return node
            }
        };
        var UTF8ToString = (ptr, maxBytesToRead) => ptr ? UTF8ArrayToString(HEAPU8, ptr, maxBytesToRead) : "";
        var SYSCALLS = {
            DEFAULT_POLLMASK: 5,
            calculateAt(dirfd, path, allowEmpty) {
                if (PATH.isAbs(path)) {
                    return path
                }
                var dir;
                if (dirfd === -100) {
                    dir = FS.cwd()
                } else {
                    var dirstream = SYSCALLS.getStreamFromFD(dirfd);
                    dir = dirstream.path
                }
                if (path.length == 0) {
                    if (!allowEmpty) {
                        throw new FS.ErrnoError(44)
                    }
                    return dir
                }
                return PATH.join2(dir, path)
            },
            doStat(func, path, buf) {
                var stat = func(path);
                HEAP32[buf >> 2] = stat.dev;
                HEAP32[buf + 4 >> 2] = stat.mode;
                HEAPU32[buf + 8 >> 2] = stat.nlink;
                HEAP32[buf + 12 >> 2] = stat.uid;
                HEAP32[buf + 16 >> 2] = stat.gid;
                HEAP32[buf + 20 >> 2] = stat.rdev;
                tempI64 = [stat.size >>> 0, (tempDouble = stat.size, +Math.abs(tempDouble) >= 1 ? tempDouble > 0 ? +Math.floor(tempDouble / 4294967296) >>> 0 : ~~+Math.ceil((tempDouble -  + (~~tempDouble >>> 0)) / 4294967296) >>> 0 : 0)],
                HEAP32[buf + 24 >> 2] = tempI64[0],
                HEAP32[buf + 28 >> 2] = tempI64[1];
                HEAP32[buf + 32 >> 2] = 4096;
                HEAP32[buf + 36 >> 2] = stat.blocks;
                var atime = stat.atime.getTime();
                var mtime = stat.mtime.getTime();
                var ctime = stat.ctime.getTime();
                tempI64 = [Math.floor(atime / 1e3) >>> 0, (tempDouble = Math.floor(atime / 1e3), +Math.abs(tempDouble) >= 1 ? tempDouble > 0 ? +Math.floor(tempDouble / 4294967296) >>> 0 : ~~+Math.ceil((tempDouble -  + (~~tempDouble >>> 0)) / 4294967296) >>> 0 : 0)],
                HEAP32[buf + 40 >> 2] = tempI64[0],
                HEAP32[buf + 44 >> 2] = tempI64[1];
                HEAPU32[buf + 48 >> 2] = atime % 1e3 * 1e3;
                tempI64 = [Math.floor(mtime / 1e3) >>> 0, (tempDouble = Math.floor(mtime / 1e3), +Math.abs(tempDouble) >= 1 ? tempDouble > 0 ? +Math.floor(tempDouble / 4294967296) >>> 0 : ~~+Math.ceil((tempDouble -  + (~~tempDouble >>> 0)) / 4294967296) >>> 0 : 0)],
                HEAP32[buf + 56 >> 2] = tempI64[0],
                HEAP32[buf + 60 >> 2] = tempI64[1];
                HEAPU32[buf + 64 >> 2] = mtime % 1e3 * 1e3;
                tempI64 = [Math.floor(ctime / 1e3) >>> 0, (tempDouble = Math.floor(ctime / 1e3), +Math.abs(tempDouble) >= 1 ? tempDouble > 0 ? +Math.floor(tempDouble / 4294967296) >>> 0 : ~~+Math.ceil((tempDouble -  + (~~tempDouble >>> 0)) / 4294967296) >>> 0 : 0)],
                HEAP32[buf + 72 >> 2] = tempI64[0],
                HEAP32[buf + 76 >> 2] = tempI64[1];
                HEAPU32[buf + 80 >> 2] = ctime % 1e3 * 1e3;
                tempI64 = [stat.ino >>> 0, (tempDouble = stat.ino, +Math.abs(tempDouble) >= 1 ? tempDouble > 0 ? +Math.floor(tempDouble / 4294967296) >>> 0 : ~~+Math.ceil((tempDouble -  + (~~tempDouble >>> 0)) / 4294967296) >>> 0 : 0)],
                HEAP32[buf + 88 >> 2] = tempI64[0],
                HEAP32[buf + 92 >> 2] = tempI64[1];
                return 0
            },
            doMsync(addr, stream, len, flags, offset) {
                if (!FS.isFile(stream.node.mode)) {
                    throw new FS.ErrnoError(43)
                }
                if (flags & 2) {
                    return 0
                }
                var buffer = HEAPU8.slice(addr, addr + len);
                FS.msync(stream, buffer, offset, len, flags)
            },
            getStreamFromFD(fd) {
                var stream = FS.getStreamChecked(fd);
                return stream
            },
            varargs: undefined,
            getStr(ptr) {
                var ret = UTF8ToString(ptr);
                return ret
            }
        };
        function ___syscall_dup3(fd, newfd, flags) {
            try {
                var old = SYSCALLS.getStreamFromFD(fd);
                if (old.fd === newfd)
                    return -28;
                var existing = FS.getStream(newfd);
                if (existing)
                    FS.close(existing);
                return FS.dupStream(old, newfd).fd
            } catch (e) {
                if (typeof FS == "undefined" || !(e.name === "ErrnoError"))
                    throw e;
                return -e.errno
            }
        }
        function syscallGetVarargI() {
            var ret = HEAP32[+SYSCALLS.varargs >> 2];
            SYSCALLS.varargs += 4;
            return ret
        }
        var syscallGetVarargP = syscallGetVarargI;
        function ___syscall_fcntl64(fd, cmd, varargs) {
            SYSCALLS.varargs = varargs;
            try {
                var stream = SYSCALLS.getStreamFromFD(fd);
                switch (cmd) {
                case 0: {
                        var arg = syscallGetVarargI();
                        if (arg < 0) {
                            return -28
                        }
                        while (FS.streams[arg]) {
                            arg++
                        }
                        var newStream;
                        newStream = FS.dupStream(stream, arg);
                        return newStream.fd
                    }
                case 1:
                case 2:
                    return 0;
                case 3:
                    return stream.flags;
                case 4: {
                        var arg = syscallGetVarargI();
                        stream.flags |= arg;
                        return 0
                    }
                case 12: {
                        var arg = syscallGetVarargP();
                        var offset = 0;
                        HEAP16[arg + offset >> 1] = 2;
                        return 0
                    }
                case 13:
                case 14:
                    return 0
                }
                return -28
            } catch (e) {
                if (typeof FS == "undefined" || !(e.name === "ErrnoError"))
                    throw e;
                return -e.errno
            }
        }
        function ___syscall_ioctl(fd, op, varargs) {
            SYSCALLS.varargs = varargs;
            try {
                var stream = SYSCALLS.getStreamFromFD(fd);
                switch (op) {
                case 21509: {
                        if (!stream.tty)
                            return -59;
                        return 0
                    }
                case 21505: {
                        if (!stream.tty)
                            return -59;
                        if (stream.tty.ops.ioctl_tcgets) {
                            var termios = stream.tty.ops.ioctl_tcgets(stream);
                            var argp = syscallGetVarargP();
                            HEAP32[argp >> 2] = termios.c_iflag || 0;
                            HEAP32[argp + 4 >> 2] = termios.c_oflag || 0;
                            HEAP32[argp + 8 >> 2] = termios.c_cflag || 0;
                            HEAP32[argp + 12 >> 2] = termios.c_lflag || 0;
                            for (var i = 0; i < 32; i++) {
                                HEAP8[argp + i + 17] = termios.c_cc[i] || 0
                            }
                            return 0
                        }
                        return 0
                    }
                case 21510:
                case 21511:
                case 21512: {
                        if (!stream.tty)
                            return -59;
                        return 0
                    }
                case 21506:
                case 21507:
                case 21508: {
                        if (!stream.tty)
                            return -59;
                        if (stream.tty.ops.ioctl_tcsets) {
                            var argp = syscallGetVarargP();
                            var c_iflag = HEAP32[argp >> 2];
                            var c_oflag = HEAP32[argp + 4 >> 2];
                            var c_cflag = HEAP32[argp + 8 >> 2];
                            var c_lflag = HEAP32[argp + 12 >> 2];
                            var c_cc = [];
                            for (var i = 0; i < 32; i++) {
                                c_cc.push(HEAP8[argp + i + 17])
                            }
                            return stream.tty.ops.ioctl_tcsets(stream.tty, op, {
                                c_iflag: c_iflag,
                                c_oflag: c_oflag,
                                c_cflag: c_cflag,
                                c_lflag: c_lflag,
                                c_cc: c_cc
                            })
                        }
                        return 0
                    }
                case 21519: {
                        if (!stream.tty)
                            return -59;
                        var argp = syscallGetVarargP();
                        HEAP32[argp >> 2] = 0;
                        return 0
                    }
                case 21520: {
                        if (!stream.tty)
                            return -59;
                        return -28
                    }
                case 21531: {
                        var argp = syscallGetVarargP();
                        return FS.ioctl(stream, op, argp)
                    }
                case 21523: {
                        if (!stream.tty)
                            return -59;
                        if (stream.tty.ops.ioctl_tiocgwinsz) {
                            var winsize = stream.tty.ops.ioctl_tiocgwinsz(stream.tty);
                            var argp = syscallGetVarargP();
                            HEAP16[argp >> 1] = winsize[0];
                            HEAP16[argp + 2 >> 1] = winsize[1]
                        }
                        return 0
                    }
                case 21524: {
                        if (!stream.tty)
                            return -59;
                        return 0
                    }
                case 21515: {
                        if (!stream.tty)
                            return -59;
                        return 0
                    }
                default:
                    return -28
                }
            } catch (e) {
                if (typeof FS == "undefined" || !(e.name === "ErrnoError"))
                    throw e;
                return -e.errno
            }
        }
        function ___syscall_openat(dirfd, path, flags, varargs) {
            SYSCALLS.varargs = varargs;
            try {
                path = SYSCALLS.getStr(path);
                path = SYSCALLS.calculateAt(dirfd, path);
                var mode = varargs ? syscallGetVarargI() : 0;
                return FS.open(path, flags, mode).fd
            } catch (e) {
                if (typeof FS == "undefined" || !(e.name === "ErrnoError"))
                    throw e;
                return -e.errno
            }
        }
        var stringToUTF8 = (str, outPtr, maxBytesToWrite) => stringToUTF8Array(str, HEAPU8, outPtr, maxBytesToWrite);
        function ___syscall_readlinkat(dirfd, path, buf, bufsize) {
            try {
                path = SYSCALLS.getStr(path);
                path = SYSCALLS.calculateAt(dirfd, path);
                if (bufsize <= 0)
                    return -28;
                var ret = FS.readlink(path);
                var len = Math.min(bufsize, lengthBytesUTF8(ret));
                var endChar = HEAP8[buf + len];
                stringToUTF8(ret, buf, bufsize + 1);
                HEAP8[buf + len] = endChar;
                return len
            } catch (e) {
                if (typeof FS == "undefined" || !(e.name === "ErrnoError"))
                    throw e;
                return -e.errno
            }
        }
        function ___syscall_renameat(olddirfd, oldpath, newdirfd, newpath) {
            try {
                oldpath = SYSCALLS.getStr(oldpath);
                newpath = SYSCALLS.getStr(newpath);
                oldpath = SYSCALLS.calculateAt(olddirfd, oldpath);
                newpath = SYSCALLS.calculateAt(newdirfd, newpath);
                FS.rename(oldpath, newpath);
                return 0
            } catch (e) {
                if (typeof FS == "undefined" || !(e.name === "ErrnoError"))
                    throw e;
                return -e.errno
            }
        }
        function ___syscall_rmdir(path) {
            try {
                path = SYSCALLS.getStr(path);
                FS.rmdir(path);
                return 0
            } catch (e) {
                if (typeof FS == "undefined" || !(e.name === "ErrnoError"))
                    throw e;
                return -e.errno
            }
        }
        function ___syscall_unlinkat(dirfd, path, flags) {
            try {
                path = SYSCALLS.getStr(path);
                path = SYSCALLS.calculateAt(dirfd, path);
                if (flags === 0) {
                    FS.unlink(path)
                } else if (flags === 512) {
                    FS.rmdir(path)
                } else {
                    abort("Invalid flags passed to unlinkat")
                }
                return 0
            } catch (e) {
                if (typeof FS == "undefined" || !(e.name === "ErrnoError"))
                    throw e;
                return -e.errno
            }
        }
        var __abort_js = () => {
            abort("")
        };
        var nowIsMonotonic = 1;
        var __emscripten_get_now_is_monotonic = () => nowIsMonotonic;
        var __emscripten_memcpy_js = (dest, src, num) => HEAPU8.copyWithin(dest, src, src + num);
        var __emscripten_system = command => {
            if (ENVIRONMENT_IS_NODE) {
                if (!command)
                    return 1;
                var cmdstr = UTF8ToString(command);
                if (!cmdstr.length)
                    return 0;
                var cp = require("child_process");
                var ret = cp.spawnSync(cmdstr, [], {
                    shell: true,
                    stdio: "inherit"
                });
                var _W_EXITCODE = (ret, sig) => ret << 8 | sig;
                if (ret.status === null) {
                    var signalToNumber = sig => {
                        switch (sig) {
                        case "SIGHUP":
                            return 1;
                        case "SIGINT":
                            return 2;
                        case "SIGQUIT":
                            return 3;
                        case "SIGFPE":
                            return 8;
                        case "SIGKILL":
                            return 9;
                        case "SIGALRM":
                            return 14;
                        case "SIGTERM":
                            return 15
                        }
                        return 2
                    };
                    return _W_EXITCODE(0, signalToNumber(ret.signal))
                }
                return _W_EXITCODE(ret.status, 0)
            }
            if (!command)
                return 0;
            return -52
        };
        var __emscripten_throw_longjmp = () => {
            throw Infinity
        };//@ts-ignore
        var convertI32PairToI53Checked = (lo, hi) => hi + 2097152 >>> 0 < 4194305 - !!lo ? (lo >>> 0) + hi * 4294967296 : NaN;
        function __gmtime_js(time_low, time_high, tmPtr) {
            var time = convertI32PairToI53Checked(time_low, time_high);
            var date = new Date(time * 1e3);
            HEAP32[tmPtr >> 2] = date.getUTCSeconds();
            HEAP32[tmPtr + 4 >> 2] = date.getUTCMinutes();
            HEAP32[tmPtr + 8 >> 2] = date.getUTCHours();
            HEAP32[tmPtr + 12 >> 2] = date.getUTCDate();
            HEAP32[tmPtr + 16 >> 2] = date.getUTCMonth();
            HEAP32[tmPtr + 20 >> 2] = date.getUTCFullYear() - 1900;
            HEAP32[tmPtr + 24 >> 2] = date.getUTCDay();
            var start = Date.UTC(date.getUTCFullYear(), 0, 1, 0, 0, 0, 0);
            var yday = (date.getTime() - start) / (1e3 * 60 * 60 * 24) | 0;
            HEAP32[tmPtr + 28 >> 2] = yday
        }
        var isLeapYear = year => year % 4 === 0 && (year % 100 !== 0 || year % 400 === 0);
        var MONTH_DAYS_LEAP_CUMULATIVE = [0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335];
        var MONTH_DAYS_REGULAR_CUMULATIVE = [0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334];
        var ydayFromDate = date => {
            var leap = isLeapYear(date.getFullYear());
            var monthDaysCumulative = leap ? MONTH_DAYS_LEAP_CUMULATIVE : MONTH_DAYS_REGULAR_CUMULATIVE;
            var yday = monthDaysCumulative[date.getMonth()] + date.getDate() - 1;
            return yday
        };
        function __localtime_js(time_low, time_high, tmPtr) {
            var time = convertI32PairToI53Checked(time_low, time_high);
            var date = new Date(time * 1e3);
            HEAP32[tmPtr >> 2] = date.getSeconds();
            HEAP32[tmPtr + 4 >> 2] = date.getMinutes();
            HEAP32[tmPtr + 8 >> 2] = date.getHours();
            HEAP32[tmPtr + 12 >> 2] = date.getDate();
            HEAP32[tmPtr + 16 >> 2] = date.getMonth();
            HEAP32[tmPtr + 20 >> 2] = date.getFullYear() - 1900;
            HEAP32[tmPtr + 24 >> 2] = date.getDay();
            var yday = ydayFromDate(date) | 0;
            HEAP32[tmPtr + 28 >> 2] = yday;
            HEAP32[tmPtr + 36 >> 2] =  - (date.getTimezoneOffset() * 60);
            var start = new Date(date.getFullYear(), 0, 1);
            var summerOffset = new Date(date.getFullYear(), 6, 1).getTimezoneOffset();
            var winterOffset = start.getTimezoneOffset();//@ts-ignore
            var dst = (summerOffset != winterOffset && date.getTimezoneOffset() == Math.min(winterOffset, summerOffset)) | 0;
            HEAP32[tmPtr + 32 >> 2] = dst
        }
        var setTempRet0 = val => __emscripten_tempret_set(val);
        var __mktime_js = function (tmPtr) {
            var ret = (() => {
                var date = new Date(HEAP32[tmPtr + 20 >> 2] + 1900, HEAP32[tmPtr + 16 >> 2], HEAP32[tmPtr + 12 >> 2], HEAP32[tmPtr + 8 >> 2], HEAP32[tmPtr + 4 >> 2], HEAP32[tmPtr >> 2], 0);
                var dst = HEAP32[tmPtr + 32 >> 2];
                var guessedOffset = date.getTimezoneOffset();
                var start = new Date(date.getFullYear(), 0, 1);
                var summerOffset = new Date(date.getFullYear(), 6, 1).getTimezoneOffset();
                var winterOffset = start.getTimezoneOffset();
                var dstOffset = Math.min(winterOffset, summerOffset);
                if (dst < 0) {
                    HEAP32[tmPtr + 32 >> 2] = Number(summerOffset != winterOffset && dstOffset == guessedOffset)
                } else if (dst > 0 != (dstOffset == guessedOffset)) {
                    var nonDstOffset = Math.max(winterOffset, summerOffset);
                    var trueOffset = dst > 0 ? dstOffset : nonDstOffset;
                    date.setTime(date.getTime() + (trueOffset - guessedOffset) * 6e4)
                }
                HEAP32[tmPtr + 24 >> 2] = date.getDay();
                var yday = ydayFromDate(date) | 0;
                HEAP32[tmPtr + 28 >> 2] = yday;
                HEAP32[tmPtr >> 2] = date.getSeconds();
                HEAP32[tmPtr + 4 >> 2] = date.getMinutes();
                HEAP32[tmPtr + 8 >> 2] = date.getHours();
                HEAP32[tmPtr + 12 >> 2] = date.getDate();
                HEAP32[tmPtr + 16 >> 2] = date.getMonth();//@ts-ignore
                HEAP32[tmPtr + 20 >> 2] = date.getYear();
                var timeMs = date.getTime();
                if (isNaN(timeMs)) {
                    return -1
                }
                return timeMs / 1e3
            })();
            return setTempRet0((tempDouble = ret, +Math.abs(tempDouble) >= 1 ? tempDouble > 0 ? +Math.floor(tempDouble / 4294967296) >>> 0 : ~~+Math.ceil((tempDouble -  + (~~tempDouble >>> 0)) / 4294967296) >>> 0 : 0)),
            ret >>> 0
        };
        var __tzset_js = (timezone, daylight, std_name, dst_name) => {
            var currentYear = (new Date).getFullYear();
            var winter = new Date(currentYear, 0, 1);
            var summer = new Date(currentYear, 6, 1);
            var winterOffset = winter.getTimezoneOffset();
            var summerOffset = summer.getTimezoneOffset();
            var stdTimezoneOffset = Math.max(winterOffset, summerOffset);
            HEAPU32[timezone >> 2] = stdTimezoneOffset * 60;
            HEAP32[daylight >> 2] = Number(winterOffset != summerOffset);
            var extractZone = date => date.toLocaleTimeString(undefined, {
                hour12: false,
                timeZoneName: "short"
            }).split(" ")[1];
            var winterName = extractZone(winter);
            var summerName = extractZone(summer);
            if (summerOffset < winterOffset) {
                stringToUTF8(winterName, std_name, 17);
                stringToUTF8(summerName, dst_name, 17)
            } else {
                stringToUTF8(winterName, dst_name, 17);
                stringToUTF8(summerName, std_name, 17)
            }
        };
        var _emscripten_date_now = () => Date.now();
        var _emscripten_get_now;
        _emscripten_get_now = () => performance.now();
        var getHeapMax = () => 2147483648;
        var growMemory = size => {
            var b = wasmMemory.buffer;
            var pages = (size - b.byteLength + 65535) / 65536;
            try {
                wasmMemory.grow(pages);
                updateMemoryViews();
                return 1
            } catch (e) {}
        };
        var _emscripten_resize_heap = requestedSize => {
            var oldSize = HEAPU8.length;
            requestedSize >>>= 0;
            var maxHeapSize = getHeapMax();
            if (requestedSize > maxHeapSize) {
                return false
            }
            var alignUp = (x, multiple) => x + (multiple - x % multiple) % multiple;
            for (var cutDown = 1; cutDown <= 4; cutDown *= 2) {
                var overGrownHeapSize = oldSize * (1 + .2 / cutDown);
                overGrownHeapSize = Math.min(overGrownHeapSize, requestedSize + 100663296);
                var newSize = Math.min(maxHeapSize, alignUp(Math.max(requestedSize, overGrownHeapSize), 65536));
                var replacement = growMemory(newSize);
                if (replacement) {
                    return true
                }
            }
            return false
        };
        var ENV = {};
        var getExecutableName = () => thisProgram || "./this.program";
        var getEnvStrings = () => {//@ts-ignore
            if (!getEnvStrings.strings) {
                var lang = (typeof navigator == "object" && navigator.languages && navigator.languages[0] || "C").replace("-", "_") + ".UTF-8";
                var env = {
                    USER: "web_user",
                    LOGNAME: "web_user",
                    PATH: "/",
                    PWD: "/",
                    HOME: "/home/web_user",
                    LANG: lang,
                    _: getExecutableName()
                };
                for (var x in ENV) {
                    if (ENV[x] === undefined)
                        delete env[x];
                    else
                        env[x] = ENV[x]
                }
                var strings = [];
                for (var x in env) {
                    strings.push(`${x}=${env[x]}`)
                }//@ts-ignore
                getEnvStrings.strings = strings
            }//@ts-ignore
            return getEnvStrings.strings
        };
        var stringToAscii = (str, buffer) => {
            for (var i = 0; i < str.length; ++i) {
                HEAP8[buffer++] = str.charCodeAt(i)
            }
            HEAP8[buffer] = 0
        };
        var _environ_get = (__environ, environ_buf) => {
            var bufSize = 0;
            getEnvStrings().forEach((string, i) => {
                var ptr = environ_buf + bufSize;
                HEAPU32[__environ + i * 4 >> 2] = ptr;
                stringToAscii(string, ptr);
                bufSize += string.length + 1
            });
            return 0
        };
        var _environ_sizes_get = (penviron_count, penviron_buf_size) => {
            var strings = getEnvStrings();
            HEAPU32[penviron_count >> 2] = strings.length;
            var bufSize = 0;
            strings.forEach(string => bufSize += string.length + 1);
            HEAPU32[penviron_buf_size >> 2] = bufSize;
            return 0
        };
        var runtimeKeepaliveCounter = 0;
        var keepRuntimeAlive = () => runtimeKeepaliveCounter > 0;
        var _proc_exit = code => {
            EXITSTATUS = code;
            if (!keepRuntimeAlive()) {
                ABORT = true
            }
            quit_(code, new ExitStatus(code))
        };
        var exitJS = (status, implicit) => {
            EXITSTATUS = status;
            _proc_exit(status)
        };
        var _exit = exitJS;
        function _fd_close(fd) {
            try {
                var stream = SYSCALLS.getStreamFromFD(fd);
                FS.close(stream);
                return 0
            } catch (e) {
                if (typeof FS == "undefined" || !(e.name === "ErrnoError"))
                    throw e;
                return e.errno
            }
        }
        var doReadv = (stream, iov, iovcnt, offset) => {
            var ret = 0;
            for (var i = 0; i < iovcnt; i++) {
                var ptr = HEAPU32[iov >> 2];
                var len = HEAPU32[iov + 4 >> 2];
                iov += 8;
                var curr = FS.read(stream, HEAP8, ptr, len, offset);
                if (curr < 0)
                    return -1;
                ret += curr;
                if (curr < len)
                    break;
                if (typeof offset != "undefined") {
                    offset += curr
                }
            }
            return ret
        };
        function _fd_read(fd, iov, iovcnt, pnum) {
            try {
                var stream = SYSCALLS.getStreamFromFD(fd);
                var num = doReadv(stream, iov, iovcnt);
                HEAPU32[pnum >> 2] = num;
                return 0
            } catch (e) {
                if (typeof FS == "undefined" || !(e.name === "ErrnoError"))
                    throw e;
                return e.errno
            }
        }
        function _fd_seek(fd, offset_low, offset_high, whence, newOffset) {
            var offset = convertI32PairToI53Checked(offset_low, offset_high);
            try {
                if (isNaN(offset))
                    return 61;
                var stream = SYSCALLS.getStreamFromFD(fd);
                FS.llseek(stream, offset, whence);
                tempI64 = [stream.position >>> 0, (tempDouble = stream.position, +Math.abs(tempDouble) >= 1 ? tempDouble > 0 ? +Math.floor(tempDouble / 4294967296) >>> 0 : ~~+Math.ceil((tempDouble -  + (~~tempDouble >>> 0)) / 4294967296) >>> 0 : 0)],
                HEAP32[newOffset >> 2] = tempI64[0],
                HEAP32[newOffset + 4 >> 2] = tempI64[1];
                if (stream.getdents && offset === 0 && whence === 0)
                    stream.getdents = null;
                return 0
            } catch (e) {
                if (typeof FS == "undefined" || !(e.name === "ErrnoError"))
                    throw e;
                return e.errno
            }
        }
        var doWritev = (stream, iov, iovcnt, offset) => {
            var ret = 0;
            for (var i = 0; i < iovcnt; i++) {
                var ptr = HEAPU32[iov >> 2];
                var len = HEAPU32[iov + 4 >> 2];
                iov += 8;
                var curr = FS.write(stream, HEAP8, ptr, len, offset);
                if (curr < 0)
                    return -1;
                ret += curr;
                if (typeof offset != "undefined") {
                    offset += curr
                }
            }
            return ret
        };
        function _fd_write(fd, iov, iovcnt, pnum) {
            try {
                var stream = SYSCALLS.getStreamFromFD(fd);
                var num = doWritev(stream, iov, iovcnt);
                HEAPU32[pnum >> 2] = num;
                return 0
            } catch (e) {
                if (typeof FS == "undefined" || !(e.name === "ErrnoError"))
                    throw e;
                return e.errno
            }
        }
        var arraySum = (array, index) => {
            var sum = 0;
            for (var i = 0; i <= index; sum += array[i++]) {}
            return sum
        };
        var MONTH_DAYS_LEAP = [31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31];
        var MONTH_DAYS_REGULAR = [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31];
        var addDays = (date, days) => {
            var newDate = new Date(date.getTime());
            while (days > 0) {
                var leap = isLeapYear(newDate.getFullYear());
                var currentMonth = newDate.getMonth();
                var daysInCurrentMonth = (leap ? MONTH_DAYS_LEAP : MONTH_DAYS_REGULAR)[currentMonth];
                if (days > daysInCurrentMonth - newDate.getDate()) {
                    days -= daysInCurrentMonth - newDate.getDate() + 1;
                    newDate.setDate(1);
                    if (currentMonth < 11) {
                        newDate.setMonth(currentMonth + 1)
                    } else {
                        newDate.setMonth(0);
                        newDate.setFullYear(newDate.getFullYear() + 1)
                    }
                } else {
                    newDate.setDate(newDate.getDate() + days);
                    return newDate
                }
            }
            return newDate
        };
        var writeArrayToMemory = (array, buffer) => {
            HEAP8.set(array, buffer)
        };
        var _strftime = (s, maxsize, format, tm) => {
            var tm_zone = HEAPU32[tm + 40 >> 2];
            var date = {
                tm_sec: HEAP32[tm >> 2],
                tm_min: HEAP32[tm + 4 >> 2],
                tm_hour: HEAP32[tm + 8 >> 2],
                tm_mday: HEAP32[tm + 12 >> 2],
                tm_mon: HEAP32[tm + 16 >> 2],
                tm_year: HEAP32[tm + 20 >> 2],
                tm_wday: HEAP32[tm + 24 >> 2],
                tm_yday: HEAP32[tm + 28 >> 2],
                tm_isdst: HEAP32[tm + 32 >> 2],
                tm_gmtoff: HEAP32[tm + 36 >> 2],
                tm_zone: tm_zone ? UTF8ToString(tm_zone) : ""
            };
            var pattern = UTF8ToString(format);
            var EXPANSION_RULES_1 = {
                "%c": "%a %b %d %H:%M:%S %Y",
                "%D": "%m/%d/%y",
                "%F": "%Y-%m-%d",
                "%h": "%b",
                "%r": "%I:%M:%S %p",
                "%R": "%H:%M",
                "%T": "%H:%M:%S",
                "%x": "%m/%d/%y",
                "%X": "%H:%M:%S",
                "%Ec": "%c",
                "%EC": "%C",
                "%Ex": "%m/%d/%y",
                "%EX": "%H:%M:%S",
                "%Ey": "%y",
                "%EY": "%Y",
                "%Od": "%d",
                "%Oe": "%e",
                "%OH": "%H",
                "%OI": "%I",
                "%Om": "%m",
                "%OM": "%M",
                "%OS": "%S",
                "%Ou": "%u",
                "%OU": "%U",
                "%OV": "%V",
                "%Ow": "%w",
                "%OW": "%W",
                "%Oy": "%y"
            };
            for (var rule in EXPANSION_RULES_1) {
                pattern = pattern.replace(new RegExp(rule, "g"), EXPANSION_RULES_1[rule])
            }
            var WEEKDAYS = ["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"];
            var MONTHS = ["January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"];
            function leadingSomething(value, digits, character) {
                var str = typeof value == "number" ? value.toString() : value || "";
                while (str.length < digits) {
                    str = character[0] + str
                }
                return str
            }
            function leadingNulls(value, digits) {
                return leadingSomething(value, digits, "0")
            }
            function compareByDay(date1, date2) {
                function sgn(value) {
                    return value < 0 ? -1 : value > 0 ? 1 : 0
                }
                var compare;
                if ((compare = sgn(date1.getFullYear() - date2.getFullYear())) === 0) {
                    if ((compare = sgn(date1.getMonth() - date2.getMonth())) === 0) {
                        compare = sgn(date1.getDate() - date2.getDate())
                    }
                }
                return compare
            }
            function getFirstWeekStartDate(janFourth) {
                switch (janFourth.getDay()) {
                case 0:
                    return new Date(janFourth.getFullYear() - 1, 11, 29);
                case 1:
                    return janFourth;
                case 2:
                    return new Date(janFourth.getFullYear(), 0, 3);
                case 3:
                    return new Date(janFourth.getFullYear(), 0, 2);
                case 4:
                    return new Date(janFourth.getFullYear(), 0, 1);
                case 5:
                    return new Date(janFourth.getFullYear() - 1, 11, 31);
                case 6:
                    return new Date(janFourth.getFullYear() - 1, 11, 30)
                }
            }
            function getWeekBasedYear(date) {
                var thisDate = addDays(new Date(date.tm_year + 1900, 0, 1), date.tm_yday);
                var janFourthThisYear = new Date(thisDate.getFullYear(), 0, 4);
                var janFourthNextYear = new Date(thisDate.getFullYear() + 1, 0, 4);
                var firstWeekStartThisYear = getFirstWeekStartDate(janFourthThisYear);
                var firstWeekStartNextYear = getFirstWeekStartDate(janFourthNextYear);
                if (compareByDay(firstWeekStartThisYear, thisDate) <= 0) {
                    if (compareByDay(firstWeekStartNextYear, thisDate) <= 0) {
                        return thisDate.getFullYear() + 1
                    }
                    return thisDate.getFullYear()
                }
                return thisDate.getFullYear() - 1
            }
            var EXPANSION_RULES_2 = {
                "%a": date => WEEKDAYS[date.tm_wday].substring(0, 3),
                "%A": date => WEEKDAYS[date.tm_wday],
                "%b": date => MONTHS[date.tm_mon].substring(0, 3),
                "%B": date => MONTHS[date.tm_mon],
                "%C": date => {
                    var year = date.tm_year + 1900;
                    return leadingNulls(year / 100 | 0, 2)
                },
                "%d": date => leadingNulls(date.tm_mday, 2),
                "%e": date => leadingSomething(date.tm_mday, 2, " "),
                "%g": date => getWeekBasedYear(date).toString().substring(2),
                "%G": getWeekBasedYear,
                "%H": date => leadingNulls(date.tm_hour, 2),
                "%I": date => {
                    var twelveHour = date.tm_hour;
                    if (twelveHour == 0)
                        twelveHour = 12;
                    else if (twelveHour > 12)
                        twelveHour -= 12;
                    return leadingNulls(twelveHour, 2)
                },
                "%j": date => leadingNulls(date.tm_mday + arraySum(isLeapYear(date.tm_year + 1900) ? MONTH_DAYS_LEAP : MONTH_DAYS_REGULAR, date.tm_mon - 1), 3),
                "%m": date => leadingNulls(date.tm_mon + 1, 2),
                "%M": date => leadingNulls(date.tm_min, 2),
                "%n": () => "\n",
                "%p": date => {
                    if (date.tm_hour >= 0 && date.tm_hour < 12) {
                        return "AM"
                    }
                    return "PM"
                },
                "%S": date => leadingNulls(date.tm_sec, 2),
                "%t": () => "\t",
                "%u": date => date.tm_wday || 7,
                "%U": date => {
                    var days = date.tm_yday + 7 - date.tm_wday;
                    return leadingNulls(Math.floor(days / 7), 2)
                },
                "%V": date => {
                    var val = Math.floor((date.tm_yday + 7 - (date.tm_wday + 6) % 7) / 7);
                    if ((date.tm_wday + 371 - date.tm_yday - 2) % 7 <= 2) {
                        val++
                    }
                    if (!val) {
                        val = 52;
                        var dec31 = (date.tm_wday + 7 - date.tm_yday - 1) % 7;
                        if (dec31 == 4 || dec31 == 5 && isLeapYear(date.tm_year % 400 - 1)) {
                            val++
                        }
                    } else if (val == 53) {
                        var jan1 = (date.tm_wday + 371 - date.tm_yday) % 7;
                        if (jan1 != 4 && (jan1 != 3 || !isLeapYear(date.tm_year)))
                            val = 1
                    }
                    return leadingNulls(val, 2)
                },
                "%w": date => date.tm_wday,
                "%W": date => {
                    var days = date.tm_yday + 7 - (date.tm_wday + 6) % 7;
                    return leadingNulls(Math.floor(days / 7), 2)
                },
                "%y": date => (date.tm_year + 1900).toString().substring(2),
                "%Y": date => date.tm_year + 1900,
                "%z": date => {
                    var off = date.tm_gmtoff;
                    var ahead = off >= 0;
                    off = Math.abs(off) / 60;
                    off = off / 60 * 100 + off % 60;
                    return (ahead ? "+" : "-") + String("0000" + off).slice(-4)
                },
                "%Z": date => date.tm_zone,
                "%%": () => "%"
            };
            pattern = pattern.replace(/%%/g, "\0\0");
            for (var rule in EXPANSION_RULES_2) {
                if (pattern.includes(rule)) {
                    pattern = pattern.replace(new RegExp(rule, "g"), EXPANSION_RULES_2[rule](date))
                }
            }
            pattern = pattern.replace(/\0\0/g, "%");
            var bytes = intArrayFromString(pattern, false);
            if (bytes.length > maxsize) {
                return 0
            }
            writeArrayToMemory(bytes, s);
            return bytes.length - 1
        };
        var wasmTableMirror = [];
        var wasmTable;
        var getWasmTableEntry = funcPtr => {
            var func = wasmTableMirror[funcPtr];
            if (!func) {
                if (funcPtr >= wasmTableMirror.length)
                    wasmTableMirror.length = funcPtr + 1;
                wasmTableMirror[funcPtr] = func = wasmTable.get(funcPtr)
            }
            return func
        };
        var runAndAbortIfError = func => {
            try {
                return func()
            } catch (e) {
                abort(e)
            }
        };
        var handleException = e => {
            if (e instanceof ExitStatus || e == "unwind") {
                return EXITSTATUS
            }
            quit_(1, e)
        };
        var maybeExit = () => {
            if (!keepRuntimeAlive()) {
                try {
                    _exit(EXITSTATUS)
                } catch (e) {
                    handleException(e)
                }
            }
        };
        var callUserCallback = func => {
            if (ABORT) {
                return
            }
            try {
                func();
                maybeExit()
            } catch (e) {
                handleException(e)
            }
        };
        var sigToWasmTypes = sig => {
            var typeNames = {
                i: "i32",
                j: "i64",
                f: "f32",
                d: "f64",
                e: "externref",
                p: "i32"
            };
            var type = {
                parameters: [],
                results: sig[0] == "v" ? [] : [typeNames[sig[0]]]
            };
            for (var i = 1; i < sig.length; ++i) {
                type.parameters.push(typeNames[sig[i]])
            }
            return type
        };
        var Asyncify = {
            instrumentWasmImports(imports) {
                var importPattern = /^(invoke_.*|__asyncjs__.*)$/;
                for (let [x, original] of Object.entries(imports)) {
                    if (typeof original == "function") {
                        let isAsyncifyImport = original.isAsync || importPattern.test(x)
                    }
                }
            },
            instrumentWasmExports(exports) {
                var ret = {};
                for (let [x, original] of Object.entries(exports)) {
                    if (typeof original == "function") {
                        ret[x] = (...args) => {
                            Asyncify.exportCallStack.push(x);
                            try {
                                return original(...args)
                            } finally {
                                if (!ABORT) {
                                    var y = Asyncify.exportCallStack.pop();
                                    Asyncify.maybeStopUnwind()
                                }
                            }
                        }
                    } else {
                        ret[x] = original
                    }
                }
                return ret
            },
            State: {
                Normal: 0,
                Unwinding: 1,
                Rewinding: 2,
                Disabled: 3
            },
            state: 0,
            StackSize: 4096,
            currData: null,
            handleSleepReturnValue: 0,
            exportCallStack: [],
            callStackNameToId: {},
            callStackIdToName: {},
            callStackId: 0,
            asyncPromiseHandlers: null,
            sleepCallbacks: [],
            getCallStackId(funcName) {
                var id = Asyncify.callStackNameToId[funcName];
                if (id === undefined) {
                    id = Asyncify.callStackId++;
                    Asyncify.callStackNameToId[funcName] = id;
                    Asyncify.callStackIdToName[id] = funcName
                }
                return id
            },
            maybeStopUnwind() {
                if (Asyncify.currData && Asyncify.state === Asyncify.State.Unwinding && Asyncify.exportCallStack.length === 0) {
                    Asyncify.state = Asyncify.State.Normal;
                    runAndAbortIfError(_asyncify_stop_unwind);//@ts-ignore
                    if (typeof Fibers != "undefined") {//@ts-ignore
                        Fibers.trampoline()
                    }
                }
            },
            whenDone() {
                return new Promise((resolve, reject) => {
                    Asyncify.asyncPromiseHandlers = {
                        resolve: resolve,
                        reject: reject
                    }
                })
            },
            allocateData() {
                var ptr = _malloc(12 + Asyncify.StackSize);
                Asyncify.setDataHeader(ptr, ptr + 12, Asyncify.StackSize);
                Asyncify.setDataRewindFunc(ptr);
                return ptr
            },
            setDataHeader(ptr, stack, stackSize) {
                HEAPU32[ptr >> 2] = stack;
                HEAPU32[ptr + 4 >> 2] = stack + stackSize
            },
            setDataRewindFunc(ptr) {
                var bottomOfCallStack = Asyncify.exportCallStack[0];
                var rewindId = Asyncify.getCallStackId(bottomOfCallStack);
                HEAP32[ptr + 8 >> 2] = rewindId
            },
            getDataRewindFunc(ptr) {
                var id = HEAP32[ptr + 8 >> 2];
                var name = Asyncify.callStackIdToName[id];
                var func = wasmExports[name];
                return func
            },
            doRewind(ptr) {
                var start = Asyncify.getDataRewindFunc(ptr);
                return start()
            },
            handleSleep(startAsync) {
                if (ABORT)
                    return;
                if (Asyncify.state === Asyncify.State.Normal) {
                    var reachedCallback = false;
                    var reachedAfterCallback = false;
                    startAsync((handleSleepReturnValue = 0) => {
                        if (ABORT)
                            return;
                        Asyncify.handleSleepReturnValue = handleSleepReturnValue;
                        reachedCallback = true;
                        if (!reachedAfterCallback) {
                            return
                        }
                        Asyncify.state = Asyncify.State.Rewinding;
                        runAndAbortIfError(() => _asyncify_start_rewind(Asyncify.currData));//@ts-ignore
                        if (typeof Browser != "undefined" && Browser.mainLoop.func) {//@ts-ignore
                            Browser.mainLoop.resume()
                        }
                        var asyncWasmReturnValue,
                        isError = false;
                        try {
                            asyncWasmReturnValue = Asyncify.doRewind(Asyncify.currData)
                        } catch (err) {
                            asyncWasmReturnValue = err;
                            isError = true
                        }
                        var handled = false;
                        if (!Asyncify.currData) {
                            var asyncPromiseHandlers = Asyncify.asyncPromiseHandlers;
                            if (asyncPromiseHandlers) {
                                Asyncify.asyncPromiseHandlers = null;
                                (isError ? asyncPromiseHandlers.reject : asyncPromiseHandlers.resolve)(asyncWasmReturnValue);
                                handled = true
                            }
                        }
                        if (isError && !handled) {
                            throw asyncWasmReturnValue
                        }
                    });
                    reachedAfterCallback = true;
                    if (!reachedCallback) {
                        Asyncify.state = Asyncify.State.Unwinding;
                        Asyncify.currData = Asyncify.allocateData();//@ts-ignore
                        if (typeof Browser != "undefined" && Browser.mainLoop.func) {//@ts-ignore
                            Browser.mainLoop.pause()
                        }
                        runAndAbortIfError(() => _asyncify_start_unwind(Asyncify.currData))
                    }
                } else if (Asyncify.state === Asyncify.State.Rewinding) {
                    Asyncify.state = Asyncify.State.Normal;
                    runAndAbortIfError(_asyncify_stop_rewind);
                    _free(Asyncify.currData);
                    Asyncify.currData = null;
                    Asyncify.sleepCallbacks.forEach(callUserCallback)
                } else {
                    abort(`invalid state: ${Asyncify.state}`)
                }
                return Asyncify.handleSleepReturnValue
            },
            handleAsync(startAsync) {
                return Asyncify.handleSleep(wakeUp => {
                    startAsync().then(wakeUp)
                })
            }
        };
        var uleb128Encode = (n, target) => {
            if (n < 128) {
                target.push(n)
            } else {
                target.push(n % 128 | 128, n >> 7)
            }
        };
        var generateFuncType = (sig, target) => {
            var sigRet = sig.slice(0, 1);
            var sigParam = sig.slice(1);
            var typeCodes = {
                i: 127,
                p: 127,
                j: 126,
                f: 125,
                d: 124,
                e: 111
            };
            target.push(96);
            uleb128Encode(sigParam.length, target);
            for (var i = 0; i < sigParam.length; ++i) {
                target.push(typeCodes[sigParam[i]])
            }
            if (sigRet == "v") {
                target.push(0)
            } else {
                target.push(1, typeCodes[sigRet])
            }
        };
        var convertJsFunctionToWasm = (func, sig) => {//@ts-ignore
            if (typeof WebAssembly.Function == "function") {//@ts-ignore
                return new WebAssembly.Function(sigToWasmTypes(sig), func)
            }
            var typeSectionBody = [1];
            generateFuncType(sig, typeSectionBody);
            var bytes = [0, 97, 115, 109, 1, 0, 0, 0, 1];
            uleb128Encode(typeSectionBody.length, bytes);
            bytes.push(...typeSectionBody);
            bytes.push(2, 7, 1, 1, 101, 1, 102, 0, 0, 7, 5, 1, 1, 102, 0, 0);
            var module = new WebAssembly.Module(new Uint8Array(bytes));
            var instance = new WebAssembly.Instance(module, {
                e: {
                    f: func
                }
            });
            var wrappedFunc = instance.exports["f"];
            return wrappedFunc
        };
        var updateTableMap = (offset, count) => {
            if (functionsInTableMap) {
                for (var i = offset; i < offset + count; i++) {
                    var item = getWasmTableEntry(i);
                    if (item) {
                        functionsInTableMap.set(item, i)
                    }
                }
            }
        };
        var functionsInTableMap;
        var getFunctionAddress = func => {
            if (!functionsInTableMap) {
                functionsInTableMap = new WeakMap;
                updateTableMap(0, wasmTable.length)
            }
            return functionsInTableMap.get(func) || 0
        };
        var freeTableIndexes = [];
        var getEmptyTableSlot = () => {
            if (freeTableIndexes.length) {
                return freeTableIndexes.pop()
            }
            try {
                wasmTable.grow(1)
            } catch (err) {
                if (!(err instanceof RangeError)) {
                    throw err
                }
                throw "Unable to grow wasm table. Set ALLOW_TABLE_GROWTH."
            }
            return wasmTable.length - 1
        };
        var setWasmTableEntry = (idx, func) => {
            wasmTable.set(idx, func);
            wasmTableMirror[idx] = wasmTable.get(idx)
        };
        var addFunction = (func, sig) => {
            var rtn = getFunctionAddress(func);
            if (rtn) {
                return rtn
            }
            var ret = getEmptyTableSlot();
            try {
                setWasmTableEntry(ret, func)
            } catch (err) {
                if (!(err instanceof TypeError)) {
                    throw err
                }
                var wrapped = convertJsFunctionToWasm(func, sig);
                setWasmTableEntry(ret, wrapped)
            }
            functionsInTableMap.set(func, ret);
            return ret
        };
        var removeFunction = index => {
            functionsInTableMap.delete(getWasmTableEntry(index));
            setWasmTableEntry(index, null);
            freeTableIndexes.push(index)
        };
        FS.createPreloadedFile = FS_createPreloadedFile;
        FS.staticInit();
        var wasmImports = {
            __syscall_dup3: ___syscall_dup3,
            __syscall_fcntl64: ___syscall_fcntl64,
            __syscall_ioctl: ___syscall_ioctl,
            __syscall_openat: ___syscall_openat,
            __syscall_readlinkat: ___syscall_readlinkat,
            __syscall_renameat: ___syscall_renameat,
            __syscall_rmdir: ___syscall_rmdir,
            __syscall_unlinkat: ___syscall_unlinkat,
            _abort_js: __abort_js,
            _doLuaExport: _doLuaExport,
            _emscripten_get_now_is_monotonic: __emscripten_get_now_is_monotonic,
            _emscripten_memcpy_js: __emscripten_memcpy_js,
            _emscripten_system: __emscripten_system,
            _emscripten_throw_longjmp: __emscripten_throw_longjmp,
            _gmtime_js: __gmtime_js,
            _localtime_js: __localtime_js,
            _mktime_js: __mktime_js,
            _tzset_js: __tzset_js,
            emscripten_date_now: _emscripten_date_now,
            emscripten_get_now: _emscripten_get_now,
            emscripten_resize_heap: _emscripten_resize_heap,
            environ_get: _environ_get,
            environ_sizes_get: _environ_sizes_get,
            exit: _exit,
            fd_close: _fd_close,
            fd_read: _fd_read,
            fd_seek: _fd_seek,
            fd_write: _fd_write,
            invoke_vii: invoke_vii,
            strftime: _strftime
        };
        var wasmExports = createWasm();
        var ___wasm_call_ctors = () => (___wasm_call_ctors = wasmExports["__wasm_call_ctors"])();
        var ___ctor_kdmyEngine = Module["___ctor_kdmyEngine"] = () => (___ctor_kdmyEngine = Module["___ctor_kdmyEngine"] = wasmExports["__ctor_kdmyEngine"])();
        var _lua_gettop = Module["_lua_gettop"] = a0 => (_lua_gettop = Module["_lua_gettop"] = wasmExports["lua_gettop"])(a0);
        var _lua_settop = Module["_lua_settop"] = (a0, a1) => (_lua_settop = Module["_lua_settop"] = wasmExports["lua_settop"])(a0, a1);
        var _lua_rotate = Module["_lua_rotate"] = (a0, a1, a2) => (_lua_rotate = Module["_lua_rotate"] = wasmExports["lua_rotate"])(a0, a1, a2);
        var _lua_pushvalue = Module["_lua_pushvalue"] = (a0, a1) => (_lua_pushvalue = Module["_lua_pushvalue"] = wasmExports["lua_pushvalue"])(a0, a1);
        var _lua_type = Module["_lua_type"] = (a0, a1) => (_lua_type = Module["_lua_type"] = wasmExports["lua_type"])(a0, a1);
        var _lua_tonumberx = Module["_lua_tonumberx"] = (a0, a1, a2) => (_lua_tonumberx = Module["_lua_tonumberx"] = wasmExports["lua_tonumberx"])(a0, a1, a2);
        var _lua_toboolean = Module["_lua_toboolean"] = (a0, a1) => (_lua_toboolean = Module["_lua_toboolean"] = wasmExports["lua_toboolean"])(a0, a1);
        var _lua_tolstring = Module["_lua_tolstring"] = (a0, a1, a2) => (_lua_tolstring = Module["_lua_tolstring"] = wasmExports["lua_tolstring"])(a0, a1, a2);
        var _lua_rawlen = Module["_lua_rawlen"] = (a0, a1) => (_lua_rawlen = Module["_lua_rawlen"] = wasmExports["lua_rawlen"])(a0, a1);
        var _lua_touserdata = Module["_lua_touserdata"] = (a0, a1) => (_lua_touserdata = Module["_lua_touserdata"] = wasmExports["lua_touserdata"])(a0, a1);
        var _lua_pushnil = Module["_lua_pushnil"] = a0 => (_lua_pushnil = Module["_lua_pushnil"] = wasmExports["lua_pushnil"])(a0);
        var _lua_pushnumber = Module["_lua_pushnumber"] = (a0, a1) => (_lua_pushnumber = Module["_lua_pushnumber"] = wasmExports["lua_pushnumber"])(a0, a1);
        var _lua_pushinteger = Module["_lua_pushinteger"] = (a0, a1, a2) => (_lua_pushinteger = Module["_lua_pushinteger"] = wasmExports["lua_pushinteger"])(a0, a1, a2);
        var _lua_pushlstring = Module["_lua_pushlstring"] = (a0, a1, a2) => (_lua_pushlstring = Module["_lua_pushlstring"] = wasmExports["lua_pushlstring"])(a0, a1, a2);
        var _lua_pushstring = Module["_lua_pushstring"] = (a0, a1) => (_lua_pushstring = Module["_lua_pushstring"] = wasmExports["lua_pushstring"])(a0, a1);
        var _lua_pushfstring = Module["_lua_pushfstring"] = (a0, a1, a2) => (_lua_pushfstring = Module["_lua_pushfstring"] = wasmExports["lua_pushfstring"])(a0, a1, a2);
        var _lua_pushcclosure = Module["_lua_pushcclosure"] = (a0, a1, a2) => (_lua_pushcclosure = Module["_lua_pushcclosure"] = wasmExports["lua_pushcclosure"])(a0, a1, a2);
        var _lua_pushboolean = Module["_lua_pushboolean"] = (a0, a1) => (_lua_pushboolean = Module["_lua_pushboolean"] = wasmExports["lua_pushboolean"])(a0, a1);
        var _lua_pushlightuserdata = Module["_lua_pushlightuserdata"] = (a0, a1) => (_lua_pushlightuserdata = Module["_lua_pushlightuserdata"] = wasmExports["lua_pushlightuserdata"])(a0, a1);
        var _lua_getglobal = Module["_lua_getglobal"] = (a0, a1) => (_lua_getglobal = Module["_lua_getglobal"] = wasmExports["lua_getglobal"])(a0, a1);
        var _lua_gettable = Module["_lua_gettable"] = (a0, a1) => (_lua_gettable = Module["_lua_gettable"] = wasmExports["lua_gettable"])(a0, a1);
        var _lua_getfield = Module["_lua_getfield"] = (a0, a1, a2) => (_lua_getfield = Module["_lua_getfield"] = wasmExports["lua_getfield"])(a0, a1, a2);
        var _lua_createtable = Module["_lua_createtable"] = (a0, a1, a2) => (_lua_createtable = Module["_lua_createtable"] = wasmExports["lua_createtable"])(a0, a1, a2);
        var _lua_setglobal = Module["_lua_setglobal"] = (a0, a1) => (_lua_setglobal = Module["_lua_setglobal"] = wasmExports["lua_setglobal"])(a0, a1);
        var _lua_settable = Module["_lua_settable"] = (a0, a1) => (_lua_settable = Module["_lua_settable"] = wasmExports["lua_settable"])(a0, a1);
        var _lua_setfield = Module["_lua_setfield"] = (a0, a1, a2) => (_lua_setfield = Module["_lua_setfield"] = wasmExports["lua_setfield"])(a0, a1, a2);
        var _lua_rawset = Module["_lua_rawset"] = (a0, a1) => (_lua_rawset = Module["_lua_rawset"] = wasmExports["lua_rawset"])(a0, a1);
        var _lua_rawseti = Module["_lua_rawseti"] = (a0, a1, a2, a3) => (_lua_rawseti = Module["_lua_rawseti"] = wasmExports["lua_rawseti"])(a0, a1, a2, a3);
        var _lua_setmetatable = Module["_lua_setmetatable"] = (a0, a1) => (_lua_setmetatable = Module["_lua_setmetatable"] = wasmExports["lua_setmetatable"])(a0, a1);
        var _lua_pcallk = Module["_lua_pcallk"] = (a0, a1, a2, a3, a4, a5) => (_lua_pcallk = Module["_lua_pcallk"] = wasmExports["lua_pcallk"])(a0, a1, a2, a3, a4, a5);
        var _lua_setwarnf = Module["_lua_setwarnf"] = (a0, a1, a2) => (_lua_setwarnf = Module["_lua_setwarnf"] = wasmExports["lua_setwarnf"])(a0, a1, a2);
        var _lua_newuserdatauv = Module["_lua_newuserdatauv"] = (a0, a1, a2) => (_lua_newuserdatauv = Module["_lua_newuserdatauv"] = wasmExports["lua_newuserdatauv"])(a0, a1, a2);
        var _lua_close = Module["_lua_close"] = a0 => (_lua_close = Module["_lua_close"] = wasmExports["lua_close"])(a0);
        var _luaL_traceback = Module["_luaL_traceback"] = (a0, a1, a2, a3) => (_luaL_traceback = Module["_luaL_traceback"] = wasmExports["luaL_traceback"])(a0, a1, a2, a3);
        var _luaL_error = Module["_luaL_error"] = (a0, a1, a2) => (_luaL_error = Module["_luaL_error"] = wasmExports["luaL_error"])(a0, a1, a2);
        var _luaL_newmetatable = Module["_luaL_newmetatable"] = (a0, a1) => (_luaL_newmetatable = Module["_luaL_newmetatable"] = wasmExports["luaL_newmetatable"])(a0, a1);
        var _luaL_testudata = Module["_luaL_testudata"] = (a0, a1, a2) => (_luaL_testudata = Module["_luaL_testudata"] = wasmExports["luaL_testudata"])(a0, a1, a2);
        var _luaL_checkudata = Module["_luaL_checkudata"] = (a0, a1, a2) => (_luaL_checkudata = Module["_luaL_checkudata"] = wasmExports["luaL_checkudata"])(a0, a1, a2);
        var _luaL_optlstring = Module["_luaL_optlstring"] = (a0, a1, a2, a3) => (_luaL_optlstring = Module["_luaL_optlstring"] = wasmExports["luaL_optlstring"])(a0, a1, a2, a3);
        var _luaL_checklstring = Module["_luaL_checklstring"] = (a0, a1, a2) => (_luaL_checklstring = Module["_luaL_checklstring"] = wasmExports["luaL_checklstring"])(a0, a1, a2);
        var _luaL_checknumber = Module["_luaL_checknumber"] = (a0, a1) => (_luaL_checknumber = Module["_luaL_checknumber"] = wasmExports["luaL_checknumber"])(a0, a1);
        var _luaL_optnumber = Module["_luaL_optnumber"] = (a0, a1, a2) => (_luaL_optnumber = Module["_luaL_optnumber"] = wasmExports["luaL_optnumber"])(a0, a1, a2);
        var _luaL_checkinteger = Module["_luaL_checkinteger"] = (a0, a1) => (_luaL_checkinteger = Module["_luaL_checkinteger"] = wasmExports["luaL_checkinteger"])(a0, a1);
        var _luaL_optinteger = Module["_luaL_optinteger"] = (a0, a1, a2, a3) => (_luaL_optinteger = Module["_luaL_optinteger"] = wasmExports["luaL_optinteger"])(a0, a1, a2, a3);
        var _luaL_ref = Module["_luaL_ref"] = (a0, a1) => (_luaL_ref = Module["_luaL_ref"] = wasmExports["luaL_ref"])(a0, a1);
        var _luaL_unref = Module["_luaL_unref"] = (a0, a1, a2) => (_luaL_unref = Module["_luaL_unref"] = wasmExports["luaL_unref"])(a0, a1, a2);
        var _luaL_loadbufferx = Module["_luaL_loadbufferx"] = (a0, a1, a2, a3, a4) => (_luaL_loadbufferx = Module["_luaL_loadbufferx"] = wasmExports["luaL_loadbufferx"])(a0, a1, a2, a3, a4);
        var _luaL_loadstring = Module["_luaL_loadstring"] = (a0, a1) => (_luaL_loadstring = Module["_luaL_loadstring"] = wasmExports["luaL_loadstring"])(a0, a1);
        var _luaL_newstate = Module["_luaL_newstate"] = () => (_luaL_newstate = Module["_luaL_newstate"] = wasmExports["luaL_newstate"])();
        var _luaL_openlibs = Module["_luaL_openlibs"] = a0 => (_luaL_openlibs = Module["_luaL_openlibs"] = wasmExports["luaL_openlibs"])(a0);
        var _malloc = Module["_malloc"] = a0 => (_malloc = Module["_malloc"] = wasmExports["malloc"])(a0);
        var _free = Module["_free"] = a0 => (_free = Module["_free"] = wasmExports["free"])(a0);
        var _realloc = Module["_realloc"] = (a0, a1) => (_realloc = Module["_realloc"] = wasmExports["realloc"])(a0, a1);
        var _setThrew = (a0, a1) => (_setThrew = wasmExports["setThrew"])(a0, a1);
        var __emscripten_tempret_set = a0 => (__emscripten_tempret_set = wasmExports["_emscripten_tempret_set"])(a0);
        var __emscripten_stack_restore = a0 => (__emscripten_stack_restore = wasmExports["_emscripten_stack_restore"])(a0);
        var __emscripten_stack_alloc = a0 => (__emscripten_stack_alloc = wasmExports["_emscripten_stack_alloc"])(a0);
        var _emscripten_stack_get_current = () => (_emscripten_stack_get_current = wasmExports["emscripten_stack_get_current"])();
        var dynCall_iidiiii = Module["dynCall_iidiiii"] = (a0, a1, a2, a3, a4, a5, a6) => (dynCall_iidiiii = Module["dynCall_iidiiii"] = wasmExports["dynCall_iidiiii"])(a0, a1, a2, a3, a4, a5, a6);
        var dynCall_vii = Module["dynCall_vii"] = (a0, a1, a2) => (dynCall_vii = Module["dynCall_vii"] = wasmExports["dynCall_vii"])(a0, a1, a2);
        var dynCall_iiii = Module["dynCall_iiii"] = (a0, a1, a2, a3) => (dynCall_iiii = Module["dynCall_iiii"] = wasmExports["dynCall_iiii"])(a0, a1, a2, a3);
        var dynCall_iii = Module["dynCall_iii"] = (a0, a1, a2) => (dynCall_iii = Module["dynCall_iii"] = wasmExports["dynCall_iii"])(a0, a1, a2);
        var dynCall_ii = Module["dynCall_ii"] = (a0, a1) => (dynCall_ii = Module["dynCall_ii"] = wasmExports["dynCall_ii"])(a0, a1);
        var dynCall_jiji = Module["dynCall_jiji"] = (a0, a1, a2, a3, a4) => (dynCall_jiji = Module["dynCall_jiji"] = wasmExports["dynCall_jiji"])(a0, a1, a2, a3, a4);
        var dynCall_iiiii = Module["dynCall_iiiii"] = (a0, a1, a2, a3, a4) => (dynCall_iiiii = Module["dynCall_iiiii"] = wasmExports["dynCall_iiiii"])(a0, a1, a2, a3, a4);
        var dynCall_viii = Module["dynCall_viii"] = (a0, a1, a2, a3) => (dynCall_viii = Module["dynCall_viii"] = wasmExports["dynCall_viii"])(a0, a1, a2, a3);
        var _asyncify_start_unwind = a0 => (_asyncify_start_unwind = wasmExports["asyncify_start_unwind"])(a0);
        var _asyncify_stop_unwind = () => (_asyncify_stop_unwind = wasmExports["asyncify_stop_unwind"])();
        var _asyncify_start_rewind = a0 => (_asyncify_start_rewind = wasmExports["asyncify_start_rewind"])(a0);
        var _asyncify_stop_rewind = () => (_asyncify_stop_rewind = wasmExports["asyncify_stop_rewind"])();
        function invoke_vii(index, a1, a2) {
            var sp = stackSave();
            try {
                dynCall_vii(index, a1, a2)
            } catch (e) {
                stackRestore(sp);
                if (e !== e + 0)
                    throw e;
                _setThrew(1, 0)
            }
        }
        Module["addFunction"] = addFunction;
        Module["removeFunction"] = removeFunction;
        var calledRun;
        dependenciesFulfilled = function runCaller() {
            if (!calledRun)
                run();
            if (!calledRun)
                dependenciesFulfilled = runCaller
        };
        function run() {
            if (runDependencies > 0) {
                return
            }
            preRun();
            if (runDependencies > 0) {
                return
            }
            function doRun() {
                if (calledRun)
                    return;
                calledRun = true;
                Module["calledRun"] = true;
                if (ABORT)
                    return;
                initRuntime();
                readyPromiseResolve(Module);
                postRun()
            } {
                doRun()
            }
        }
        run();
        moduleRtn = readyPromise;

        return moduleRtn;
    });
})();
if (typeof exports === 'object' && typeof module === 'object')
    module.exports = ModuleLua;//@ts-ignore
else if (typeof define === 'function' && define['amd'])//@ts-ignore
    define([], () => ModuleLua);
