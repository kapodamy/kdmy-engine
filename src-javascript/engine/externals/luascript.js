"use strict";

//
// enclose all declare variables related to emscripten
//
(function() {
const kdmyEngine_textDecoder = new TextDecoder("UTF-8", { ignoreBOM: true, fatal: true });
const kdmyEngine_textEncoder = new TextEncoder();
const kdmyEngine_textBuffer = new Uint8Array(128);
const kdmyEngine_objectMap = new Map();
/**@type {DataView}*/let kdmyEngine_dataView;
let kdmyEngine_objectIndex = 1;
const kdmyEngine_endianess = new Uint16Array(new Uint8Array([0x00, 0xFF]).buffer)[0] == 0xFF00;
const kdmyEngine_obtain = function (key) {
    //if (key === undefined) console.warn("undefined value passed");
    if (key == null) return 0;

    if (typeof (key) == "number") {
        for (let [obj, idx] of kdmyEngine_objectMap) {
            if (key == idx) return obj;
        }

        console.assert(undefined, "unknown object index " + key);
        return undefined;
    }


    let index = kdmyEngine_objectMap.get(key);
    if (index !== undefined) return index;

    index = kdmyEngine_objectIndex++;
    kdmyEngine_objectMap.set(key, index);
    return index;
};
const kdmyEngine_forget = function (target_id) {
    for (let [obj, idx] of kdmyEngine_objectMap) {
        if (idx == target_id) {
            kdmyEngine_objectMap.delete(obj);
            return true;
        }
    }
    return false;
};
const kdmyEngine_drop_shared_object = function (target_obj) {
    for (let [obj, idx] of kdmyEngine_objectMap) {
        if (obj == target_obj) {
            _luascript_drop_shared(idx);
            kdmyEngine_objectMap.delete(obj);
        }
    }
};
const kdmyEngine_deallocate = function (ptr) {
    if (ptr == 0) return;
    _free(ptr);
};
const kdmyEngine_ptrToString = function (ptr) {
  if (ptr == 0) return null;

  let end = HEAP8.indexOf(0x00, ptr);
  if (end < 0) end = HEAP8.byteLength;// this never should happen

  let char_array = HEAP8.subarray(ptr, end);
  return kdmyEngine_textDecoder.decode(char_array, { stream: false });
};
const kdmyEngine_yieldAsync = function () {
  let id = Asyncify.callStackId - 1;
  if (Asyncify.callStackIdToName[id]){
    return new Promise(function(resolve, reject) {
	    Asyncify.asyncPromiseHandlers = {
		  resolve: function(res){
			Asyncify.callStackIdToName[id] = undefined;
			  resolve(res);
		  },
		  reject: function(err){
			Asyncify.callStackIdToName[id] = undefined;
			  reject(err);
		  }
		};
    });
  }
  return undefined;
};
const kdmyEngine_hasAsyncPending = function() {
  return !!Asyncify.callStackIdToName[Asyncify.callStackId - 1];
}
function kdmyEngine_stringToPtr(str) {
    if (str == null) return 0;

    let ptr, bytes;

    if (str.length < 1) {
        ptr = _malloc(1);
        HEAP8[ptr] = 0x00;
        return ptr;
    }

    const info = kdmyEngine_textEncoder.encodeInto(str, kdmyEngine_textBuffer);
    let buffer;

    if (info.written < kdmyEngine_textBuffer.byteLength) {
        bytes = info.written;
        buffer = kdmyEngine_textBuffer.subarray(0, bytes);
    } else {
        buffer = kdmyEngine_textEncoder.encode(str);
        bytes = buffer.byteLength;
    }

    ptr = _malloc(bytes + 1);
    HEAP8.set(buffer, ptr);
    HEAP8[ptr + bytes] = 0x00;

    return ptr;
}
function kdmyEngine_set_int32(address, value) {
    kdmyEngine_dataView.setInt32(address, value, kdmyEngine_endianess);
}
function kdmyEngine_set_uint32(address, value) {
    kdmyEngine_dataView.setUint32(address, value, kdmyEngine_endianess);
}
function kdmyEngine_set_float32(address, value) {
    kdmyEngine_dataView.setFloat32(address, value, kdmyEngine_endianess);
}
function kdmyEngine_set_float64(address, value) {
    kdmyEngine_dataView.setFloat64(address, value, kdmyEngine_endianess);
}
function kdmyEngine_get_uint32(address) {
    return kdmyEngine_dataView.getUint32(address, kdmyEngine_endianess);
}
function kdmyEngine_get_float64(address) {
    return kdmyEngine_dataView.getFloat64(address, kdmyEngine_endianess);
}
function kdmyEngine_get_ram() {
    return new DataView(buffer);
}

ModuleLuaScript.kdmyEngine_stringToPtr = kdmyEngine_stringToPtr;
ModuleLuaScript.kdmyEngine_ptrToString = kdmyEngine_ptrToString;
ModuleLuaScript.kdmyEngine_deallocate = kdmyEngine_deallocate;
ModuleLuaScript.kdmyEngine_obtain = kdmyEngine_obtain;
ModuleLuaScript.kdmyEngine_yieldAsync = kdmyEngine_yieldAsync;
ModuleLuaScript.kdmyEngine_hasAsyncPending = kdmyEngine_hasAsyncPending;
ModuleLuaScript.kdmyEngine_drop_shared_object = kdmyEngine_drop_shared_object;
ModuleLuaScript.kdmyEngine_get_ram = kdmyEngine_get_ram;
ModuleLuaScript.kdmyEngine_endianess = kdmyEngine_endianess;

var moduleOverrides = Object.assign({}, ModuleLuaScript);
var arguments_ = [];
var thisProgram = "./this.program";
var quit_ = (status, toThrow) => {
    throw toThrow
};
var ENVIRONMENT_IS_WEB = typeof window == "object";
var ENVIRONMENT_IS_WORKER = typeof importScripts == "function";
var ENVIRONMENT_IS_NODE = typeof process == "object" && typeof process.versions == "object" && typeof process.versions.node == "string";
var scriptDirectory = "";
function locateFile(path) {
    if (ModuleLuaScript["locateFile"]) {
        return ModuleLuaScript["locateFile"](path, scriptDirectory)
    }
    return scriptDirectory + path
}
var read_, readAsync, readBinary, setWindowTitle;
function logExceptionOnExit(e) {
    if (e instanceof ExitStatus)
        return;
    let toLog = e;
    err("exiting due to exception: " + toLog)
}
var fs;
var nodePath;
var requireNodeFS;
if (ENVIRONMENT_IS_NODE) {
    if (ENVIRONMENT_IS_WORKER) {
        scriptDirectory = require("path").dirname(scriptDirectory) + "/"
    } else {
        scriptDirectory = __dirname + "/"
    }
    requireNodeFS = () => {
        if (!nodePath) {
            fs = require("fs");
            nodePath = require("path")
        }
    };
    read_ = function shell_read(filename, binary) {
        requireNodeFS();
        filename = nodePath["normalize"](filename);
        return fs.readFileSync(filename, binary ? undefined : "utf8")
    };
    readBinary = filename => {
        var ret = read_(filename, true);
        if (!ret.buffer) {
            ret = new Uint8Array(ret)
        }
        return ret
    };
    readAsync = (filename, onload, onerror) => {
        requireNodeFS();
        filename = nodePath["normalize"](filename);
        fs.readFile(filename, function (err, data) {
            if (err)
                onerror(err);
            else
                onload(data.buffer)
        })
    };
    if (process["argv"].length > 1) {
        thisProgram = process["argv"][1].replace(/\\/g, "/")
    }
    arguments_ = process["argv"].slice(2);
    if (typeof module != "undefined") {
        module["exports"] = ModuleLuaScript
    }
    process["on"]("uncaughtException", function (ex) {
        if (!(ex instanceof ExitStatus)) {
            throw ex
        }
    });
    process["on"]("unhandledRejection", function (reason) {
        throw reason
    });// @ts-ignore
    quit_ = (status, toThrow) => {
        if (keepRuntimeAlive()) {
            process["exitCode"] = status;
            throw toThrow
        }
        logExceptionOnExit(toThrow);
        process["exit"](status)
    };
    ModuleLuaScript["inspect"] = function () {
        return "[Emscripten Module object]"
    }
} else if (ENVIRONMENT_IS_WEB || ENVIRONMENT_IS_WORKER) {
    if (ENVIRONMENT_IS_WORKER) {
        scriptDirectory = self.location.href
    } else if (typeof document != "undefined" && document.currentScript) {// @ts-ignore
        scriptDirectory = document.currentScript.src
    }
    if (scriptDirectory.indexOf("blob:") !== 0) {
        scriptDirectory = scriptDirectory.substr(0, scriptDirectory.replace(/[?#].*/, "").lastIndexOf("/") + 1)
    } else {
        scriptDirectory = ""
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
    setWindowTitle = title => document.title = title
} else {}
var out = ModuleLuaScript["print"] || console.log.bind(console);
var err = ModuleLuaScript["printErr"] || console.warn.bind(console);
Object.assign(ModuleLuaScript, moduleOverrides);
moduleOverrides = null;
if (ModuleLuaScript["arguments"])
    arguments_ = ModuleLuaScript["arguments"];
if (ModuleLuaScript["thisProgram"])
    thisProgram = ModuleLuaScript["thisProgram"];
if (ModuleLuaScript["quit"])
    quit_ = ModuleLuaScript["quit"];
var POINTER_SIZE = 4;
function warnOnce(text) {// @ts-ignore
    if (!warnOnce.shown)// @ts-ignore
        warnOnce.shown = {};// @ts-ignore
    if (!warnOnce.shown[text]) {// @ts-ignore
        warnOnce.shown[text] = 1;
        err(text)
    }
}
function uleb128Encode(n) {
    if (n < 128) {
        return [n]
    }
    return [n % 128 | 128, n >> 7]
}
function convertJsFunctionToWasm(func, sig) {// @ts-ignore
    if (typeof WebAssembly.Function == "function") {
        var typeNames = {
            "i": "i32",
            "j": "i64",
            "f": "f32",
            "d": "f64",
            "p": "i32"
        };
        var type = {
            parameters: [],
            results: sig[0] == "v" ? [] : [typeNames[sig[0]]]
        };
        for (var i = 1; i < sig.length; ++i) {
            type.parameters.push(typeNames[sig[i]])
        }// @ts-ignore
        return new WebAssembly.Function(type, func)
    }
    var typeSection = [1, 96];
    var sigRet = sig.slice(0, 1);
    var sigParam = sig.slice(1);
    var typeCodes = {
        "i": 127,
        "p": 127,
        "j": 126,
        "f": 125,
        "d": 124
    };
    typeSection = typeSection.concat(uleb128Encode(sigParam.length));
    for (var i = 0; i < sigParam.length; ++i) {
        typeSection.push(typeCodes[sigParam[i]])
    }
    if (sigRet == "v") {
        typeSection.push(0)
    } else {
        typeSection = typeSection.concat([1, typeCodes[sigRet]])
    }
    typeSection = [1].concat(uleb128Encode(typeSection.length), typeSection);
    var bytes = new Uint8Array([0, 97, 115, 109, 1, 0, 0, 0].concat(typeSection, [2, 7, 1, 1, 101, 1, 102, 0, 0, 7, 5, 1, 1, 102, 0, 0]));
    var module = new WebAssembly.Module(bytes);
    var instance = new WebAssembly.Instance(module, {
        "e": {
            "f": func
        }
    });
    var wrappedFunc = instance.exports["f"];
    return wrappedFunc
}
var freeTableIndexes = [];
var functionsInTableMap;
function getEmptyTableSlot() {
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
}
function updateTableMap(offset, count) {
    for (var i = offset; i < offset + count; i++) {
        var item = getWasmTableEntry(i);
        if (item) {
            functionsInTableMap.set(item, i)
        }
    }
}
var tempRet0 = 0;
var setTempRet0 = value => {
    tempRet0 = value
};
var getTempRet0 = () => tempRet0;
var wasmBinary;
if (ModuleLuaScript["wasmBinary"])
    wasmBinary = ModuleLuaScript["wasmBinary"];
var noExitRuntime = ModuleLuaScript["noExitRuntime"] || true;
if (typeof WebAssembly != "object") {
    abort("no native wasm support detected")
}
var wasmMemory;
var ABORT = false;
var EXITSTATUS;
function assert(condition, text) {
    if (!condition) {
        abort(text)
    }
}
function getCFunc(ident) {
    var func = ModuleLuaScript["_" + ident];
    return func
}
function ccall(ident, returnType, argTypes, args, opts) {
    var toC = {
        "string": function (str) {
            var ret = 0;
            if (str !== null && str !== undefined && str !== 0) {
                var len = (str.length << 2) + 1;
                ret = stackAlloc(len);
                stringToUTF8(str, ret, len)
            }
            return ret
        },
        "array": function (arr) {
            var ret = stackAlloc(arr.length);
            writeArrayToMemory(arr, ret);
            return ret
        }
    };
    function convertReturnValue(ret) {
        if (returnType === "string") {
            return UTF8ToString(ret)
        }
        if (returnType === "boolean")
            return Boolean(ret);
        return ret
    }
    var func = getCFunc(ident);
    var cArgs = [];
    var stack = 0;
    if (args) {
        for (var i = 0; i < args.length; i++) {
            var converter = toC[argTypes[i]];
            if (converter) {
                if (stack === 0)
                    stack = stackSave();
                cArgs[i] = converter(args[i])
            } else {
                cArgs[i] = args[i]
            }
        }
    }
    var previousAsync = Asyncify.currData;
    var ret = func.apply(null, cArgs);
    function onDone(ret) {
        runtimeKeepalivePop();
        if (stack !== 0)
            stackRestore(stack);
        return convertReturnValue(ret)
    }
    runtimeKeepalivePush();
    var asyncMode = opts && opts.async;
    if (Asyncify.currData != previousAsync) {
        return Asyncify.whenDone().then(onDone)
    }
    ret = onDone(ret);
    if (asyncMode)
        return Promise.resolve(ret);
    return ret
}
var UTF8Decoder = typeof TextDecoder != "undefined" ? new TextDecoder("utf8") : undefined;
function UTF8ArrayToString(heapOrArray, idx, maxBytesToRead) {
    var endIdx = idx + maxBytesToRead;
    var endPtr = idx;
    while (heapOrArray[endPtr] && !(endPtr >= endIdx))
        ++endPtr;
    if (endPtr - idx > 16 && heapOrArray.buffer && UTF8Decoder) {
        return UTF8Decoder.decode(heapOrArray.subarray(idx, endPtr))
    } else {
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
    }
    return str
}
function UTF8ToString(ptr, maxBytesToRead) {
    return ptr ? UTF8ArrayToString(HEAPU8, ptr, maxBytesToRead) : ""
}
function stringToUTF8Array(str, heap, outIdx, maxBytesToWrite) {
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
}
function stringToUTF8(str, outPtr, maxBytesToWrite) {
    return stringToUTF8Array(str, HEAPU8, outPtr, maxBytesToWrite)
}
function lengthBytesUTF8(str) {
    var len = 0;
    for (var i = 0; i < str.length; ++i) {
        var u = str.charCodeAt(i);
        if (u >= 55296 && u <= 57343)
            u = 65536 + ((u & 1023) << 10) | str.charCodeAt(++i) & 1023;
        if (u <= 127)
            ++len;
        else if (u <= 2047)
            len += 2;
        else if (u <= 65535)
            len += 3;
        else
            len += 4
    }
    return len
}
var UTF16Decoder = typeof TextDecoder != "undefined" ? new TextDecoder("utf-16le") : undefined;
function allocateUTF8(str) {
    var size = lengthBytesUTF8(str) + 1;
    var ret = _malloc(size);
    if (ret)
        stringToUTF8Array(str, HEAP8, ret, size);
    return ret
}
function writeArrayToMemory(array, buffer) {
    HEAP8.set(array, buffer)
}
function writeAsciiToMemory(str, buffer, dontAddNull) {
    for (var i = 0; i < str.length; ++i) {
        HEAP8[buffer++ >> 0] = str.charCodeAt(i)
    }
    if (!dontAddNull)
        HEAP8[buffer >> 0] = 0
}
var buffer, HEAP8, HEAPU8, HEAP16, HEAPU16, HEAP32, HEAPU32, HEAPF32, HEAPF64;
function updateGlobalBufferAndViews(buf) {
    buffer = buf;
    ModuleLuaScript["HEAP8"] = HEAP8 = new Int8Array(buf);
    ModuleLuaScript["HEAP16"] = HEAP16 = new Int16Array(buf);
    ModuleLuaScript["HEAP32"] = HEAP32 = new Int32Array(buf);
    ModuleLuaScript["HEAPU8"] = HEAPU8 = new Uint8Array(buf);
    ModuleLuaScript["HEAPU16"] = HEAPU16 = new Uint16Array(buf);
    ModuleLuaScript["HEAPU32"] = HEAPU32 = new Uint32Array(buf);
    ModuleLuaScript["HEAPF32"] = HEAPF32 = new Float32Array(buf);
    ModuleLuaScript["HEAPF64"] = HEAPF64 = new Float64Array(buf)
    kdmyEngine_dataView = new DataView(buf);
}
var INITIAL_MEMORY = ModuleLuaScript["INITIAL_MEMORY"] || 16777216;
var wasmTable;
var __ATPRERUN__ = [];
var __ATINIT__ = [];
var __ATPOSTRUN__ = [];
var runtimeInitialized = false;
function keepRuntimeAlive() {
    return noExitRuntime
}
function preRun() {
    if (ModuleLuaScript["preRun"]) {
        if (typeof ModuleLuaScript["preRun"] == "function")
            ModuleLuaScript["preRun"] = [ModuleLuaScript["preRun"]];
        while (ModuleLuaScript["preRun"].length) {
            addOnPreRun(ModuleLuaScript["preRun"].shift())
        }
    }
    callRuntimeCallbacks(__ATPRERUN__)
}
function initRuntime() {
    runtimeInitialized = true;
    if (!ModuleLuaScript["noFSInit"] && !FS.init.initialized)
        FS.init();
    FS.ignorePermissions = false;
    TTY.init();
    callRuntimeCallbacks(__ATINIT__)
}
function postRun() {
    if (ModuleLuaScript["postRun"]) {
        if (typeof ModuleLuaScript["postRun"] == "function")
            ModuleLuaScript["postRun"] = [ModuleLuaScript["postRun"]];
        while (ModuleLuaScript["postRun"].length) {
            addOnPostRun(ModuleLuaScript["postRun"].shift())
        }
    }
    callRuntimeCallbacks(__ATPOSTRUN__)
}
function addOnPreRun(cb) {
    __ATPRERUN__.unshift(cb)
}
function addOnInit(cb) {
    __ATINIT__.unshift(cb)
}
function addOnPostRun(cb) {
    __ATPOSTRUN__.unshift(cb)
}
var runDependencies = 0;
var runDependencyWatcher = null;
var dependenciesFulfilled = null;
function getUniqueRunDependency(id) {
    return id
}
function addRunDependency(id) {
    runDependencies++;
    if (ModuleLuaScript["monitorRunDependencies"]) {
        ModuleLuaScript["monitorRunDependencies"](runDependencies)
    }
}
function removeRunDependency(id) {
    runDependencies--;
    if (ModuleLuaScript["monitorRunDependencies"]) {
        ModuleLuaScript["monitorRunDependencies"](runDependencies)
    }
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
function abort(what) { {
        if (ModuleLuaScript["onAbort"]) {
            ModuleLuaScript["onAbort"](what)
        }
    }
    what = "Aborted(" + what + ")";
    err(what);
    ABORT = true;
    EXITSTATUS = 1;
    what += ". Build with -sASSERTIONS for more info.";
    var e = new WebAssembly.RuntimeError(what);
    throw e
}
var dataURIPrefix = "data:application/octet-stream;base64,";
function isDataURI(filename) {
    return filename.startsWith(dataURIPrefix)
}
function isFileURI(filename) {
    return filename.startsWith("file://")
}
var wasmBinaryFile;
wasmBinaryFile = "luascript.wasm";
if (!isDataURI(wasmBinaryFile)) {
    wasmBinaryFile = locateFile(wasmBinaryFile)
}
function getBinary(file) {
    try {
        if (file == wasmBinaryFile && wasmBinary) {
            return new Uint8Array(wasmBinary)
        }
        if (readBinary) {
            return readBinary(file)
        } else {
            throw "both async and sync fetching of the wasm failed"
        }
    } catch (err) {
        abort(err)
    }
}
function getBinaryPromise() {
    if (!wasmBinary && (ENVIRONMENT_IS_WEB || ENVIRONMENT_IS_WORKER)) {
        if (typeof fetch == "function" && !isFileURI(wasmBinaryFile)) {
            return fetch(wasmBinaryFile, {
                credentials: "same-origin"
            }).then(function (response) {
                if (!response["ok"]) {
                    throw "failed to load wasm binary file at '" + wasmBinaryFile + "'"
                }
                return response["arrayBuffer"]()
            }).catch(function () {
                return getBinary(wasmBinaryFile)
            })
        } else {
            if (readAsync) {
                return new Promise(function (resolve, reject) {
                    readAsync(wasmBinaryFile, function (response) {
                        resolve(new Uint8Array(response))
                    }, reject)
                })
            }
        }
    }
    return Promise.resolve().then(function () {
        return getBinary(wasmBinaryFile)
    })
}
function createWasm() {
    var info = {
        "env": asmLibraryArg,
        "wasi_snapshot_preview1": asmLibraryArg
    };
    function receiveInstance(instance, module) {
        var exports = instance.exports;
        exports = Asyncify.instrumentWasmExports(exports);
        ModuleLuaScript["asm"] = exports;
        wasmMemory = ModuleLuaScript["asm"]["memory"];
        updateGlobalBufferAndViews(wasmMemory.buffer);
        wasmTable = ModuleLuaScript["asm"]["__indirect_function_table"];
        addOnInit(ModuleLuaScript["asm"]["__wasm_call_ctors"]);
        removeRunDependency("wasm-instantiate")
    }
    addRunDependency("wasm-instantiate");
    function receiveInstantiationResult(result) {
        receiveInstance(result["instance"])
    }
    function instantiateArrayBuffer(receiver) {
        return getBinaryPromise().then(function (binary) {
            return WebAssembly.instantiate(binary, info)
        }).then(function (instance) {
            return instance
        }).then(receiver, function (reason) {
            err("failed to asynchronously prepare wasm: " + reason);
            abort(reason)
        })
    }
    function instantiateAsync() {
        if (!wasmBinary && typeof WebAssembly.instantiateStreaming == "function" && !isDataURI(wasmBinaryFile) && !isFileURI(wasmBinaryFile) && typeof fetch == "function") {
            return fetch(wasmBinaryFile, {
                credentials: "same-origin"
            }).then(function (response) {
                var result = WebAssembly.instantiateStreaming(response, info);
                return result.then(receiveInstantiationResult, function (reason) {
                    err("wasm streaming compile failed: " + reason);
                    err("falling back to ArrayBuffer instantiation");
                    return instantiateArrayBuffer(receiveInstantiationResult)
                })
            })
        } else {
            return instantiateArrayBuffer(receiveInstantiationResult)
        }
    }
    if (ModuleLuaScript["instantiateWasm"]) {
        try {
            var exports = ModuleLuaScript["instantiateWasm"](info, receiveInstance);
            exports = Asyncify.instrumentWasmExports(exports);
            return exports
        } catch (e) {
            err("Module.instantiateWasm callback failed with error: " + e);
            return false
        }
    }
    instantiateAsync();
    return {}
}
var tempDouble;
var tempI64;


function __asyncjs__animlist_init(src) {
    return Asyncify.handleAsync(async() => {
        let ret = await animlist_init(kdmyEngine_ptrToString(src));
        return kdmyEngine_obtain(ret)
    })
}
function __asyncjs__atlas_init(src) {
    return Asyncify.handleAsync(async() => {
        let ret = await atlas_init(kdmyEngine_ptrToString(src));
        return kdmyEngine_obtain(ret)
    })
}
function __asyncjs__dialogue_show_dialog(dialogue, dialog_src) {
    return Asyncify.handleAsync(async() => {
        return await dialogue_show_dialog(kdmyEngine_obtain(dialogue), kdmyEngine_ptrToString(dialog_src))
    })
}
function __asyncjs__dialogue_show_dialog2(dialogue, text_dialog_content) {
    return Asyncify.handleAsync(async() => {
        return await dialogue_show_dialog2(kdmyEngine_obtain(dialogue), kdmyEngine_ptrToString(text_dialog_content))
    })
}
function __asyncjs__fs_readfile(path, buffer_ptr, size_ptr) {
    return Asyncify.handleAsync(async() => {
        try {
            if (buffer_ptr == 0)
                return 0;
            let arraybuffer = await fs_readarraybuffer(kdmyEngine_ptrToString(path));
            if (!arraybuffer)
                return 0;
            let ptr = _malloc(arraybuffer.byteLength);
            if (ptr == 0) {
                console.error("__asyncjs__fs_readfile() out-of-memory, size required was " + arraybuffer.byteLength);
                return 0
            }
            new Uint8Array(buffer).set(new Uint8Array(arraybuffer), ptr);
            kdmyEngine_set_uint32(buffer_ptr, ptr);
            kdmyEngine_set_uint32(size_ptr, arraybuffer.byteLength);
            return 1
        } catch (e) {
            console.error(e);
            return 0
        }
    })
}
function __asyncjs__json_load_from(src) {
    return Asyncify.handleAsync(async() => {
        let ret = await json_load_from(kdmyEngine_ptrToString(src));
        return ModuleLuaScript.kdmyEngine_obtain(ret)
    })
}
function __asyncjs__menu_init(menumanifest, x, y, z, width, height) {
    return Asyncify.handleAsync(async() => {
        let ret = await menu_init(kdmyEngine_obtain(menumanifest), x, y, z, width, height);
        kdmyEngine_forget(menumanifest);
        return kdmyEngine_obtain(ret)
    })
}
function __asyncjs__menumanifest_init(src) {
    return Asyncify.handleAsync(async() => {
        let ret = await menumanifest_init(kdmyEngine_ptrToString(src));
        return kdmyEngine_obtain(ret)
    })
}
function __asyncjs__messagebox_set_image_from_atlas(messagebox, filename, entry_name, is_animation) {
    return Asyncify.handleAsync(async() => {
        await messagebox_set_image_from_atlas(kdmyEngine_obtain(messagebox), kdmyEngine_ptrToString(filename), kdmyEngine_ptrToString(entry_name), is_animation)
    })
}
function __asyncjs__messagebox_set_image_from_texture(messagebox, filename) {
    return Asyncify.handleAsync(async() => {
        await messagebox_set_image_from_texture(kdmyEngine_obtain(messagebox), kdmyEngine_ptrToString(filename))
    })
}
function __asyncjs__modding_get_messagebox(modding) {
    return Asyncify.handleAsync(async() => {
        let ret = await modding_get_messagebox(kdmyEngine_obtain(modding));
        return kdmyEngine_obtain(ret)
    })
}
function __asyncjs__modding_launch_credits(modding) {
    return Asyncify.handleAsync(async() => {
        modding_launch_credits(kdmyEngine_obtain(modding))
    })
}
function __asyncjs__modding_launch_freeplay(modding) {
    return Asyncify.handleAsync(async() => {
        modding_launch_freeplay(kdmyEngine_obtain(modding))
    })
}
function __asyncjs__modding_launch_mainmenu(modding) {
    return Asyncify.handleAsync(async() => {
        let ret = modding_launch_mainmenu(kdmyEngine_obtain(modding));
        return ret ? 1 : 0
    })
}
function __asyncjs__modding_launch_settings(modding) {
    return Asyncify.handleAsync(async() => {
        modding_launch_settings(kdmyEngine_obtain(modding))
    })
}
function __asyncjs__modding_launch_startscreen(modding) {
    return Asyncify.handleAsync(async() => {
        let ret = modding_launch_startscreen(kdmyEngine_obtain(modding));
        return ret ? 1 : 0
    })
}
function __asyncjs__modding_launch_week(modding, week_name, difficult, alt_tracks, bf, gf, gameplay_manifest, song_idx) {
    return Asyncify.handleAsync(async() => {
        let ret = modding_launch_week(kdmyEngine_obtain(modding), kdmyEngine_ptrToString(week_name), kdmyEngine_ptrToString(difficult), alt_tracks, kdmyEngine_ptrToString(bf), kdmyEngine_ptrToString(gf), kdmyEngine_ptrToString(gameplay_manifest), song_idx);
        _free(difficult);
        _free(bf);
        _free(gf);
        _free(gameplay_manifest);
        return ret
    })
}
function __asyncjs__modding_launch_weekselector(modding) {
    return Asyncify.handleAsync(async() => {
        let ret = modding_launch_weekselector(kdmyEngine_obtain(modding));
        return ret
    })
}
function __asyncjs__modding_replace_native_background_music(modding, music_src) {
    return Asyncify.handleAsync(async() => {
        let ret = await modding_replace_native_background_music(kdmyEngine_obtain(modding), kdmyEngine_ptrToString(music_src));
        return kdmyEngine_obtain(ret)
    })
}
function __asyncjs__modding_spawn_screen(modding, layout_src, script_src, arg_type, arg_value, ret_type) {
    return Asyncify.handleAsync(async() => {
        let arg = null;
        switch (arg_type) {
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
            break
        }
        if (arg_value != 0)
            _free(arg_value);
        let ret = await modding_spawn_screen(kdmyEngine_obtain(modding), kdmyEngine_ptrToString(layout_src), kdmyEngine_ptrToString(script_src), arg);
        let ret_ptr = 0;
        switch (typeof ret) {
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
            ret_ptr = _malloc(4);
            kdmyEngine_set_uint32(ret_ptr, ret ? 1 : 0);
            break;
        case "number":
            kdmyEngine_set_uint32(ret_type, MODDING_VALUE_TYPE_DOUBLE);
            ret_ptr = _malloc(8);
            kdmyEngine_set_float64(ret_ptr, ret);
            break
        }
        return ret_ptr
    })
}
function __asyncjs__modelholder_init(src) {
    return Asyncify.handleAsync(async() => {
        let ret = await modelholder_init(kdmyEngine_ptrToString(src));
        return kdmyEngine_obtain(ret)
    })
}
function __asyncjs__modelholder_init2(vertex_color_rgb8, atlas_src, animlist_src) {
    return Asyncify.handleAsync(async() => {
        let ret = await modelholder_init2(vertex_color_rgb8, kdmyEngine_ptrToString(atlas_src), kdmyEngine_ptrToString(animlist_src));
        return kdmyEngine_obtain(ret)
    })
}
function __asyncjs__songplayer_play(songplayer, songinfo) {
    return Asyncify.handleAsync(async() => {
        const _songinfo = {
            completed: 0,
            timestamp: 0
        };
        await songplayer_play(kdmyEngine_obtain(songplayer), _songinfo)
    })
}
function __asyncjs__week_rebuild_ui(roundcontext) {
    return Asyncify.handleAsync(async() => {
        await week_rebuild_ui(kdmyEngine_obtain(roundcontext))
    })
}
function __js__animlist_destroy(animlist) {
    animlist_destroy(kdmyEngine_obtain(kdmyEngine_get_uint32(animlist)))
}
function __js__animlist_get_animation(animlist, animation_name) {
    let ret = animlist_get_animation(kdmyEngine_obtain(animlist), kdmyEngine_ptrToString(animation_name));
    return kdmyEngine_obtain(ret)
}
function __js__animlist_is_item_frame_animation(animlist_item) {
    let ret = animlist_is_item_frame_animation(kdmyEngine_obtain(animlist_item));
    return ret ? 1 : 0
}
function __js__animlist_is_item_macro_animation(animlist_item) {
    let ret = animlist_is_item_macro_animation(kdmyEngine_obtain(animlist_item));
    return ret ? 1 : 0
}
function __js__animlist_is_item_tweenkeyframe_animation(animlist_item) {
    let ret = animlist_is_item_tweenkeyframe_animation(kdmyEngine_obtain(animlist_item));
    return ret ? 1 : 0
}
function __js__animsprite_destroy(animsprite) {
    animsprite_destroy(kdmyEngine_obtain(kdmyEngine_get_uint32(animsprite)))
}
function __js__animsprite_get_name(animsprite) {
    let ret = animsprite_get_name(kdmyEngine_obtain(animsprite));
    return kdmyEngine_obtain(ret)
}
function __js__animsprite_init(animlist_item) {
    let ret = animsprite_init(kdmyEngine_obtain(animlist_item));
    return kdmyEngine_obtain(ret)
}
function __js__animsprite_init_as_empty(name) {
    let ret = animsprite_init_as_empty(kdmyEngine_ptrToString(name));
    return kdmyEngine_obtain(ret)
}
function __js__animsprite_init_from_animlist(animlist, animation_name) {
    let ret = animsprite_init_from_animlist(kdmyEngine_obtain(animlist), kdmyEngine_ptrToString(animation_name));
    return kdmyEngine_obtain(ret)
}
function __js__animsprite_init_from_atlas(frame_rate, loop, atlas, prefix, has_number_suffix) {
    let ret = animsprite_init_from_atlas(frame_rate, loop, kdmyEngine_obtain(atlas), kdmyEngine_ptrToString(prefix), has_number_suffix);
    return kdmyEngine_obtain(ret)
}
function __js__animsprite_init_from_tweenlerp(name, loop, tweenlerp) {
    let ret = animsprite_init_from_tweenlerp(kdmyEngine_ptrToString(name), loop, kdmyEngine_obtain(tweenlerp));
    return kdmyEngine_obtain(ret)
}
function __js__animsprite_is_frame_animation(animsprite) {
    let ret = animsprite_is_frame_animation(kdmyEngine_obtain(animsprite));
    return ret ? 1 : 0
}
function __js__animsprite_restart(animsprite) {
    animsprite_restart(kdmyEngine_obtain(animsprite))
}
function __js__animsprite_set_delay(animsprite, delay_milliseconds) {
    animsprite_set_delay(kdmyEngine_obtain(animsprite), delay_milliseconds)
}
function __js__animsprite_set_loop(animsprite, loop) {
    animsprite_set_loop(kdmyEngine_obtain(animsprite), loop)
}
function __js__atlas_destroy(atlas) {
    atlas_destroy(kdmyEngine_obtain(kdmyEngine_get_uint32(atlas)))
}
function __js__atlas_get_entry(atlas, name) {
    let ret = atlas_get_entry(kdmyEngine_obtain(atlas), kdmyEngine_ptrToString(name));
    return kdmyEngine_obtain(ret)
}
function __js__atlas_get_entry_with_number_suffix(atlas, name_prefix) {
    let ret = atlas_get_entry_with_number_suffix(kdmyEngine_obtain(atlas), kdmyEngine_ptrToString(name_prefix));
    return kdmyEngine_obtain(ret)
}
function __js__atlas_get_glyph_fps(atlas) {
    let ret = atlas_get_glyph_fps(kdmyEngine_obtain(atlas));
    return ret
}
function __js__atlas_get_index_of(atlas, name) {
    let ret = atlas_get_index_of(kdmyEngine_obtain(atlas), kdmyEngine_ptrToString(name));
    return ret
}
function __js__atlas_get_texture_resolution(atlas, resolution_width, resolution_height) {
    const values = [0, 0];
    atlas_get_texture_resolution(kdmyEngine_obtain(atlas), values);
    kdmyEngine_set_int32(resolution_width, values[0]);
    kdmyEngine_set_int32(resolution_height, values[1])
}
function __js__atlas_utils_is_known_extension(src) {
    let ret = atlas_utils_is_known_extension(kdmyEngine_ptrToString(src));
    return ret ? 1 : 0
}
function __js__camera_apply(camera, pvrctx) {
    camera_apply(kdmyEngine_obtain(camera), null)
}
function __js__camera_debug_log_info(camera) {
    camera_debug_log_info(kdmyEngine_obtain(camera))
}
function __js__camera_disable_offset_zoom(camera, disable) {
    return camera_disable_offset_zoom(kdmyEngine_obtain(camera), disable)
}
function __js__camera_end(camera) {
    camera_end(kdmyEngine_obtain(camera))
}
function __js__camera_from_layout(camera, layout, camera_name) {
    return camera_from_layout(kdmyEngine_obtain(camera), kdmyEngine_obtain(layout), kdmyEngine_ptrToString(camera_name))
}
function __js__camera_get_modifier(camera) {
    const modifier = camera_get_modifier(kdmyEngine_obtain(camera));
    return kdmyEngine_obtain(modifier)
}
function __js__camera_get_offset(camera, x, y, z) {
    const values = [0, 0, 0];
    camera_get_offset(kdmyEngine_obtain(camera), values);
    kdmyEngine_set_float32(x, values[0]);
    kdmyEngine_set_float32(y, values[1]);
    kdmyEngine_set_float32(z, values[2])
}
function __js__camera_get_parent_layout(camera) {
    let ret = camera_get_parent_layout(kdmyEngine_obtain(camera));
    return kdmyEngine_obtain(ret)
}
function __js__camera_is_completed(camera) {
    return camera_is_completed(kdmyEngine_obtain(camera))
}
function __js__camera_move(camera, end_x, end_y, end_z) {
    camera_move(kdmyEngine_obtain(camera), end_x, end_y, end_z)
}
function __js__camera_move_offset(camera, end_x, end_y, end_z) {
    camera_move_offset(kdmyEngine_obtain(camera), end_x, end_y, end_z)
}
function __js__camera_repeat(camera) {
    camera_repeat(kdmyEngine_obtain(camera))
}
function __js__camera_set_absolute_position(camera, x, y) {
    camera_set_absolute_position(kdmyEngine_obtain(camera), x, y)
}
function __js__camera_set_absolute_zoom(camera, z) {
    camera_set_absolute_zoom(kdmyEngine_obtain(camera), z)
}
function __js__camera_set_animation(camera, animsprite) {
    camera_set_animation(kdmyEngine_obtain(camera), kdmyEngine_obtain(animsprite))
}
function __js__camera_set_interpolator_type(camera, type) {
    camera_set_interpolator_type(kdmyEngine_obtain(camera), type)
}
function __js__camera_set_offset(camera, x, y, z) {
    camera_set_offset(kdmyEngine_obtain(camera), x, y, z)
}
function __js__camera_set_transition_duration(camera, expresed_in_beats, value) {
    camera_set_transition_duration(kdmyEngine_obtain(camera), expresed_in_beats, value)
}
function __js__camera_slide(camera, start_x, start_y, start_z, end_x, end_y, end_z) {
    camera_slide(kdmyEngine_obtain(camera), start_x, start_y, start_z, end_x, end_y, end_z)
}
function __js__camera_slide_offset(camera, start_x, start_y, start_z, end_x, end_y, end_z) {
    camera_slide_offset(kdmyEngine_obtain(camera), start_x, start_y, start_z, end_x, end_y, end_z)
}
function __js__camera_slide_to(camera, x, y, z) {
    camera_slide_to(kdmyEngine_obtain(camera), x, y, z)
}
function __js__camera_slide_to_offset(camera, x, y, z) {
    camera_slide_to_offset(kdmyEngine_obtain(camera), x, y, z)
}
function __js__camera_slide_x(camera, start, end) {
    camera_slide_x(kdmyEngine_obtain(camera), start, end)
}
function __js__camera_slide_x_offset(camera, start, end) {
    camera_slide_x_offset(kdmyEngine_obtain(camera), start, end)
}
function __js__camera_slide_y(camera, start, end) {
    camera_slide_y(kdmyEngine_obtain(camera), start, end)
}
function __js__camera_slide_y_offset(camera, start, end) {
    camera_slide_y_offset(kdmyEngine_obtain(camera), start, end)
}
function __js__camera_slide_z(camera, start, end) {
    camera_slide_z(kdmyEngine_obtain(camera), start, end)
}
function __js__camera_slide_z_offset(camera, start, end) {
    camera_slide_z_offset(kdmyEngine_obtain(camera), start, end)
}
function __js__camera_stop(camera) {
    camera_stop(kdmyEngine_obtain(camera))
}
function __js__camera_to_origin(camera, should_slide) {
    camera_to_origin(kdmyEngine_obtain(camera), should_slide)
}
function __js__camera_to_origin_offset(camera, should_slide) {
    camera_to_origin_offset(kdmyEngine_obtain(camera), should_slide)
}
function __js__character_animation_end(character) {
    character_animation_end(kdmyEngine_obtain(character))
}
function __js__character_animation_restart(character) {
    character_animation_restart(kdmyEngine_obtain(character))
}
function __js__character_animation_set(character, animsprite) {
    character_animation_set(kdmyEngine_obtain(character), kdmyEngine_obtain(animsprite))
}
function __js__character_enable_continuous_idle(character, enable) {
    character_enable_continuous_idle(kdmyEngine_obtain(character), enable)
}
function __js__character_enable_flip_correction(character, enable) {
    return character_enable_flip_correction(kdmyEngine_obtain(character), enable)
}
function __js__character_enable_reference_size(character, enable) {
    character_enable_reference_size(kdmyEngine_obtain(character), enable)
}
function __js__character_face_as_opponent(character, face_as_opponent) {
    character_face_as_opponent(kdmyEngine_obtain(character), face_as_opponent)
}
function __js__character_flip_orientation(character, enable) {
    return character_flip_orientation(kdmyEngine_obtain(character), enable)
}
function __js__character_freeze_animation(character, enabled) {
    character_freeze_animation(kdmyEngine_obtain(character), enabled)
}
function __js__character_get_current_action(character) {
    return character_get_current_action(kdmyEngine_obtain(character))
}
function __js__character_get_modifier(character) {
    const modifier = character_get_modifier(kdmyEngine_obtain(character));
    return kdmyEngine_obtain(modifier)
}
function __js__character_get_play_calls(character) {
    return character_get_play_calls(kdmyEngine_obtain(character))
}
function __js__character_has_direction(character, name, is_extra) {
    return character_has_direction(kdmyEngine_obtain(character), kdmyEngine_ptrToString(name), is_extra)
}
function __js__character_is_idle_active(character) {
    return character_is_idle_active(kdmyEngine_obtain(character))
}
function __js__character_play_extra(character, extra_animation_name, prefer_sustain) {
    return character_play_extra(kdmyEngine_obtain(character), kdmyEngine_ptrToString(extra_animation_name), prefer_sustain)
}
function __js__character_play_hey(character) {
    return character_play_hey(kdmyEngine_obtain(character))
}
function __js__character_play_idle(character) {
    return character_play_idle(kdmyEngine_obtain(character))
}
function __js__character_play_miss(character, direction, keep_in_hold) {
    return character_play_miss(kdmyEngine_obtain(character), kdmyEngine_ptrToString(direction), keep_in_hold)
}
function __js__character_play_sing(character, direction, prefer_sustain) {
    return character_play_sing(kdmyEngine_obtain(character), kdmyEngine_ptrToString(direction), prefer_sustain)
}
function __js__character_reset(character) {
    character_reset(kdmyEngine_obtain(character))
}
function __js__character_set_alpha(character, alpha) {
    character_set_alpha(kdmyEngine_obtain(character), alpha)
}
function __js__character_set_color_offset(character, r, g, b, a) {
    character_set_color_offset(kdmyEngine_obtain(character), r, g, b, a)
}
function __js__character_set_color_offset_to_default(character) {
    character_set_color_offset_to_default(kdmyEngine_obtain(character))
}
function __js__character_set_draw_align(character, align_vertical, align_horizontal) {
    character_set_draw_align(kdmyEngine_obtain(character), align_vertical, align_horizontal)
}
function __js__character_set_draw_location(character, x, y) {
    character_set_draw_location(kdmyEngine_obtain(character), x, y)
}
function __js__character_set_idle_speed(character, speed) {
    character_set_idle_speed(kdmyEngine_obtain(character), speed)
}
function __js__character_set_offset(character, offset_x, offset_y) {
    character_set_offset(kdmyEngine_obtain(character), offset_x, offset_y)
}
function __js__character_set_scale(character, scale_factor) {
    character_set_scale(kdmyEngine_obtain(character), scale_factor)
}
function __js__character_set_visible(character, visible) {
    character_set_visible(kdmyEngine_obtain(character), visible)
}
function __js__character_set_z_index(character, z) {
    character_set_z_index(kdmyEngine_obtain(character), z)
}
function __js__character_set_z_offset(character, z_offset) {
    character_set_z_offset(kdmyEngine_obtain(character), z_offset)
}
function __js__character_state_add(character, modelholder, state_name) {
    return character_state_add(kdmyEngine_obtain(character), kdmyEngine_obtain(modelholder), kdmyEngine_ptrToString(state_name))
}
function __js__character_state_toggle(character, state_name) {
    return character_state_toggle(kdmyEngine_obtain(character), kdmyEngine_ptrToString(state_name))
}
function __js__character_trailing_enabled(character, enabled) {
    character_trailing_enabled(kdmyEngine_obtain(character), enabled)
}
function __js__character_trailing_set_offsetcolor(character, r, g, b) {
    character_trailing_set_offsetcolor(kdmyEngine_obtain(character), r, g, b)
}
function __js__character_trailing_set_params(character, length, trail_delay, trail_alpha, darken_colors) {
    character_trailing_set_params(kdmyEngine_obtain(character), length, trail_delay, trail_alpha, darken_colors == 0 ? null : kdmyEngine_get_uint32(darken_colors))
}
function __js__character_update_reference_size(character, width, height) {
    character_update_reference_size(kdmyEngine_obtain(character), width, height)
}
function __js__character_use_alternate_sing_animations(character, enable) {
    character_use_alternate_sing_animations(kdmyEngine_obtain(character), enable)
}
function __js__conductor_clear_mapping(conductor) {
    conductor_clear_mapping(kdmyEngine_obtain(conductor))
}
function __js__conductor_destroy(conductor) {
    conductor_destroy(kdmyEngine_obtain(kdmyEngine_get_uint32(conductor)))
}
function __js__conductor_disable(conductor, disable) {
    conductor_disable(kdmyEngine_obtain(conductor), disable)
}
function __js__conductor_disable_strum_line(conductor, strum, should_disable) {
    let ret = conductor_disable_strum_line(kdmyEngine_obtain(conductor), kdmyEngine_obtain(strum), should_disable);
    return ret ? 1 : 0
}
function __js__conductor_get_character(conductor) {
    let ret = conductor_get_character(kdmyEngine_obtain(conductor));
    return kdmyEngine_obtain(ret)
}
function __js__conductor_init() {
    let ret = conductor_init();
    return kdmyEngine_obtain(ret)
}
function __js__conductor_map_automatically(conductor, should_map_extras) {
    let ret = conductor_map_automatically(kdmyEngine_obtain(conductor), should_map_extras);
    return ret
}
function __js__conductor_map_strum_to_player_extra_add(conductor, strum, extra_animation_name) {
    conductor_map_strum_to_player_extra_add(kdmyEngine_obtain(conductor), kdmyEngine_obtain(strum), kdmyEngine_ptrToString(extra_animation_name))
}
function __js__conductor_map_strum_to_player_extra_remove(conductor, strum, extra_animation_name) {
    conductor_map_strum_to_player_extra_remove(kdmyEngine_obtain(conductor), kdmyEngine_obtain(strum), kdmyEngine_ptrToString(extra_animation_name))
}
function __js__conductor_map_strum_to_player_sing_add(conductor, strum, sing_direction_name) {
    conductor_map_strum_to_player_sing_add(kdmyEngine_obtain(conductor), kdmyEngine_obtain(strum), kdmyEngine_ptrToString(sing_direction_name))
}
function __js__conductor_map_strum_to_player_sing_remove(conductor, strum, sing_direction_name) {
    conductor_map_strum_to_player_sing_remove(kdmyEngine_obtain(conductor), kdmyEngine_obtain(strum), kdmyEngine_ptrToString(sing_direction_name))
}
function __js__conductor_play_hey(conductor) {
    conductor_play_hey(kdmyEngine_obtain(conductor))
}
function __js__conductor_play_idle(conductor) {
    conductor_play_idle(kdmyEngine_obtain(conductor))
}
function __js__conductor_poll(conductor) {
    conductor_poll(kdmyEngine_obtain(conductor))
}
function __js__conductor_poll_reset(conductor) {
    conductor_poll_reset(kdmyEngine_obtain(conductor))
}
function __js__conductor_remove_strum(conductor, strum) {
    let ret = conductor_remove_strum(kdmyEngine_obtain(conductor), kdmyEngine_obtain(strum));
    return ret ? 1 : 0
}
function __js__conductor_set_character(conductor, character) {
    conductor_set_character(kdmyEngine_obtain(conductor), kdmyEngine_obtain(character))
}
function __js__conductor_use_strum_line(conductor, strum) {
    conductor_use_strum_line(kdmyEngine_obtain(conductor), kdmyEngine_obtain(strum))
}
function __js__conductor_use_strums(conductor, strums) {
    conductor_use_strums(kdmyEngine_obtain(conductor), kdmyEngine_obtain(strums))
}
function __js__countdown_get_drawable(countdown) {
    let ret = countdown_get_drawable(kdmyEngine_obtain(countdown));
    return kdmyEngine_obtain(ret)
}
function __js__countdown_has_ended(countdown) {
    let ret = countdown_has_ended(kdmyEngine_obtain(countdown));
    return ret ? 1 : 0
}
function __js__countdown_ready(countdown) {
    let ret = countdown_ready(kdmyEngine_obtain(countdown));
    return ret ? 1 : 0
}
function __js__countdown_set_bpm(countdown, bpm) {
    countdown_set_bpm(kdmyEngine_obtain(countdown), bpm)
}
function __js__countdown_set_default_animation2(countdown, tweenkeyframe) {
    countdown_set_default_animation2(kdmyEngine_obtain(countdown), kdmyEngine_obtain(tweenkeyframe))
}
function __js__countdown_start(countdown) {
    let ret = countdown_start(kdmyEngine_obtain(countdown));
    return ret ? 1 : 0
}
function __js__dialogue_apply_state(dialogue, state_name) {
    return dialogue_apply_state(kdmyEngine_obtain(dialogue), kdmyEngine_ptrToString(state_name))
}
function __js__dialogue_apply_state2(dialogue, state_name, if_line_label) {
    return dialogue_apply_state2(kdmyEngine_obtain(dialogue), kdmyEngine_ptrToString(state_name), kdmyEngine_ptrToString(if_line_label))
}
function __js__dialogue_close(dialogue) {
    dialogue_close(kdmyEngine_obtain(dialogue))
}
function __js__dialogue_get_modifier(dialogue) {
    const modifier = dialogue_get_modifier(kdmyEngine_obtain(dialogue));
    return kdmyEngine_obtain(modifier)
}
function __js__dialogue_hide(dialogue, hidden) {
    dialogue_hide(kdmyEngine_obtain(dialogue), hidden)
}
function __js__dialogue_is_completed(dialogue) {
    return dialogue_is_completed(kdmyEngine_obtain(dialogue))
}
function __js__dialogue_is_hidden(dialogue) {
    return dialogue_is_hidden(kdmyEngine_obtain(dialogue))
}
function __js__dialogue_set_alpha(dialogue, alpha) {
    dialogue_set_alpha(kdmyEngine_obtain(dialogue), alpha)
}
function __js__dialogue_set_antialiasing(dialogue, antialiasing) {
    dialogue_set_antialiasing(kdmyEngine_obtain(dialogue), antialiasing)
}
function __js__dialogue_set_offsetcolor(dialogue, r, g, b, a) {
    dialogue_set_offsetcolor(kdmyEngine_obtain(dialogue), r, g, b, a)
}
function __js__drawable_blend_enable(drawable, enabled) {
    drawable_blend_enable(kdmyEngine_obtain(drawable), enabled)
}
function __js__drawable_blend_set(drawable, src_rgb, dst_rgb, src_alpha, dst_alpha) {
    drawable_blend_set(kdmyEngine_obtain(drawable), src_rgb, dst_rgb, src_alpha, dst_alpha)
}
function __js__drawable_get_alpha(drawable) {
    let ret = drawable_get_alpha(kdmyEngine_obtain(drawable));
    return ret
}
function __js__drawable_get_modifier(drawable) {
    let ret = drawable_get_modifier(kdmyEngine_obtain(drawable));
    return kdmyEngine_obtain(ret)
}
function __js__drawable_get_shader(drawable) {
    let ret = drawable_get_shader(kdmyEngine_obtain(drawable));
    return kdmyEngine_obtain(ret)
}
function __js__drawable_get_z_index(drawable) {
    let ret = drawable_get_z_index(kdmyEngine_obtain(drawable));
    return ret
}
function __js__drawable_set_alpha(drawable, alpha) {
    drawable_set_alpha(kdmyEngine_obtain(drawable), alpha)
}
function __js__drawable_set_antialiasing(drawable, antialiasing) {
    drawable_set_antialiasing(kdmyEngine_obtain(drawable), antialiasing)
}
function __js__drawable_set_offsetcolor(drawable, r, g, b, a) {
    drawable_set_offsetcolor(kdmyEngine_obtain(drawable), r, g, b, a)
}
function __js__drawable_set_offsetcolor_to_default(drawable) {
    drawable_set_offsetcolor_to_default(kdmyEngine_obtain(drawable))
}
function __js__drawable_set_shader(drawable, psshader) {
    drawable_set_shader(kdmyEngine_obtain(drawable), kdmyEngine_obtain(psshader))
}
function __js__drawable_set_z_index(drawable, z_index) {
    drawable_set_z_index(kdmyEngine_obtain(drawable), z_index)
}
function __js__drawable_set_z_offset(drawable, offset) {
    drawable_set_z_offset(kdmyEngine_obtain(drawable), offset)
}
function __js__healthbar_animation_end(healthbar) {
    healthbar_animation_end(kdmyEngine_obtain(healthbar))
}
function __js__healthbar_animation_restart(healthbar) {
    healthbar_animation_restart(kdmyEngine_obtain(healthbar))
}
function __js__healthbar_animation_set(healthbar, animsprite) {
    healthbar_animation_set(kdmyEngine_obtain(healthbar), kdmyEngine_obtain(animsprite))
}
function __js__healthbar_bump_enable(healthbar, enable_bump) {
    healthbar_bump_enable(kdmyEngine_obtain(healthbar), enable_bump)
}
function __js__healthbar_disable_icon_overlap(healthbar, disable) {
    healthbar_disable_icon_overlap(kdmyEngine_obtain(healthbar), disable)
}
function __js__healthbar_disable_progress_animation(healthbar, disable) {
    healthbar_disable_progress_animation(kdmyEngine_obtain(healthbar), disable)
}
function __js__healthbar_disable_warnings(healthbar, disable) {
    healthbar_disable_warnings(kdmyEngine_obtain(healthbar), disable)
}
function __js__healthbar_enable_extra_length(healthbar, extra_enabled) {
    healthbar_enable_extra_length(kdmyEngine_obtain(healthbar), extra_enabled)
}
function __js__healthbar_enable_low_health_flash_warning(healthbar, enable) {
    healthbar_enable_low_health_flash_warning(kdmyEngine_obtain(healthbar), enable)
}
function __js__healthbar_enable_vertical(healthbar, enable_vertical) {
    healthbar_enable_vertical(kdmyEngine_obtain(healthbar), enable_vertical)
}
function __js__healthbar_get_bar_midpoint(healthbar, x, y) {
    const values = [0, 0];
    healthbar_get_bar_midpoint(kdmyEngine_obtain(healthbar), values);
    kdmyEngine_set_float32(x, values[0]);
    kdmyEngine_set_float32(y, values[1])
}
function __js__healthbar_get_drawable(healthbar) {
    let ret = healthbar_get_drawable(kdmyEngine_obtain(healthbar));
    return kdmyEngine_obtain(ret)
}
function __js__healthbar_get_percent(healthbar) {
    let ret = healthbar_get_percent(kdmyEngine_obtain(healthbar));
    return ret
}
function __js__healthbar_hide_warnings(healthbar) {
    healthbar_hide_warnings(kdmyEngine_obtain(healthbar))
}
function __js__healthbar_load_warnings(healthbar, modelholder, use_alt_icons) {
    let ret = healthbar_load_warnings(kdmyEngine_obtain(healthbar), kdmyEngine_obtain(modelholder), use_alt_icons);
    return ret ? 1 : 0
}
function __js__healthbar_set_alpha(healthbar, alpha) {
    healthbar_set_alpha(kdmyEngine_obtain(healthbar), alpha)
}
function __js__healthbar_set_bpm(healthbar, beats_per_minute) {
    healthbar_set_bpm(kdmyEngine_obtain(healthbar), beats_per_minute)
}
function __js__healthbar_set_bump_animation_opponent(healthbar, animsprite) {
    healthbar_set_bump_animation_opponent(kdmyEngine_obtain(healthbar), kdmyEngine_obtain(animsprite))
}
function __js__healthbar_set_bump_animation_player(healthbar, animsprite) {
    healthbar_set_bump_animation_player(kdmyEngine_obtain(healthbar), kdmyEngine_obtain(animsprite))
}
function __js__healthbar_set_health_position(healthbar, max_health, health, opponent_recover) {
    let ret = healthbar_set_health_position(kdmyEngine_obtain(healthbar), max_health, health, opponent_recover);
    return ret
}
function __js__healthbar_set_health_position2(healthbar, percent) {
    healthbar_set_health_position2(kdmyEngine_obtain(healthbar), percent)
}
function __js__healthbar_set_opponent_bar_color(healthbar, r, g, b) {
    healthbar_set_opponent_bar_color(kdmyEngine_obtain(healthbar), r, g, b)
}
function __js__healthbar_set_opponent_bar_color_rgb8(healthbar, color_rgb8) {
    healthbar_set_opponent_bar_color_rgb8(kdmyEngine_obtain(healthbar), color_rgb8)
}
function __js__healthbar_set_player_bar_color(healthbar, r, g, b) {
    healthbar_set_player_bar_color(kdmyEngine_obtain(healthbar), r, g, b)
}
function __js__healthbar_set_player_bar_color_rgb8(healthbar, color_rgb8) {
    healthbar_set_player_bar_color_rgb8(kdmyEngine_obtain(healthbar), color_rgb8)
}
function __js__healthbar_set_visible(healthbar, visible) {
    healthbar_set_visible(kdmyEngine_obtain(healthbar), visible)
}
function __js__healthbar_show_drain_warning(healthbar, use_fast_drain) {
    healthbar_show_drain_warning(kdmyEngine_obtain(healthbar), use_fast_drain)
}
function __js__healthbar_show_locked_warning(healthbar) {
    healthbar_show_locked_warning(kdmyEngine_obtain(healthbar))
}
function __js__healthbar_state_background_add(healthbar, modelholder, state_name) {
    let ret = healthbar_state_background_add(kdmyEngine_obtain(healthbar), kdmyEngine_obtain(modelholder), kdmyEngine_ptrToString(state_name));
    return ret ? 1 : 0
}
function __js__healthbar_state_background_add2(healthbar, color_rgb8, animsprite, state_name) {
    let ret = healthbar_state_background_add2(kdmyEngine_obtain(healthbar), color_rgb8, kdmyEngine_obtain(animsprite), kdmyEngine_ptrToString(state_name));
    return ret ? 1 : 0
}
function __js__healthbar_state_opponent_add(healthbar, icon_mdlhldr, bar_mdlhldr, state_name) {
    let ret = healthbar_state_opponent_add(kdmyEngine_obtain(healthbar), kdmyEngine_obtain(icon_mdlhldr), kdmyEngine_obtain(bar_mdlhldr), kdmyEngine_ptrToString(state_name));
    return ret
}
function __js__healthbar_state_opponent_add2(healthbar, icon_mdlhldr, bar_color_rgb8, state_name) {
    let ret = healthbar_state_opponent_add2(kdmyEngine_obtain(healthbar), kdmyEngine_obtain(icon_mdlhldr), bar_color_rgb8, kdmyEngine_ptrToString(state_name));
    return ret
}
function __js__healthbar_state_player_add(healthbar, icon_mdlhldr, bar_mdlhldr, state_name) {
    let ret = healthbar_state_player_add(kdmyEngine_obtain(healthbar), kdmyEngine_obtain(icon_mdlhldr), kdmyEngine_obtain(bar_mdlhldr), kdmyEngine_ptrToString(state_name));
    return ret
}
function __js__healthbar_state_player_add2(healthbar, icon_modelholder, bar_color_rgb8, state_name) {
    let ret = healthbar_state_player_add2(kdmyEngine_obtain(healthbar), kdmyEngine_obtain(icon_modelholder), bar_color_rgb8, kdmyEngine_ptrToString(state_name));
    return ret
}
function __js__healthbar_state_toggle(healthbar, state_name) {
    let ret = healthbar_state_toggle(kdmyEngine_obtain(healthbar), kdmyEngine_ptrToString(state_name));
    return ret
}
function __js__healthbar_state_toggle_background(healthbar, state_name) {
    let ret = healthbar_state_toggle_background(kdmyEngine_obtain(healthbar), kdmyEngine_ptrToString(state_name));
    return ret ? 1 : 0
}
function __js__healthbar_state_toggle_opponent(healthbar, state_name) {
    let ret = healthbar_state_toggle_opponent(kdmyEngine_obtain(healthbar), kdmyEngine_ptrToString(state_name));
    return ret ? 1 : 0
}
function __js__healthbar_state_toggle_player(healthbar, state_name) {
    let ret = healthbar_state_toggle_player(kdmyEngine_obtain(healthbar), kdmyEngine_ptrToString(state_name));
    return ret ? 1 : 0
}
function __js__healthwatcher_add_opponent(healthwatcher, playerstats, can_recover, can_die) {
    let ret = healthwatcher_add_opponent(kdmyEngine_obtain(healthwatcher), kdmyEngine_obtain(playerstats), can_recover, can_die);
    return ret ? 1 : 0
}
function __js__healthwatcher_add_player(healthwatcher, playerstats, can_recover, can_die) {
    let ret = healthwatcher_add_player(kdmyEngine_obtain(healthwatcher), kdmyEngine_obtain(playerstats), can_recover, can_die);
    return ret ? 1 : 0
}
function __js__healthwatcher_balance(healthwatcher, healthbar) {
    healthwatcher_balance(kdmyEngine_obtain(healthwatcher), kdmyEngine_obtain(healthbar))
}
function __js__healthwatcher_clear(healthwatcher) {
    healthwatcher_clear(kdmyEngine_obtain(healthwatcher))
}
function __js__healthwatcher_enable_dead(healthwatcher, playerstats, can_die) {
    let ret = healthwatcher_enable_dead(kdmyEngine_obtain(healthwatcher), kdmyEngine_obtain(playerstats), can_die);
    return ret ? 1 : 0
}
function __js__healthwatcher_enable_recover(healthwatcher, playerstats, can_recover) {
    let ret = healthwatcher_enable_recover(kdmyEngine_obtain(healthwatcher), kdmyEngine_obtain(playerstats), can_recover);
    return ret ? 1 : 0
}
function __js__healthwatcher_has_deads(healthwatcher, in_players_or_opponents) {
    let ret = healthwatcher_has_deads(kdmyEngine_obtain(healthwatcher), in_players_or_opponents);
    return ret
}
function __js__healthwatcher_reset_opponents(healthwatcher) {
    healthwatcher_reset_opponents(kdmyEngine_obtain(healthwatcher))
}
function __js__kdmyEngine_change_window_title(title, from_modding_context) {
    luascriptplatform.ChangeWindowTitle(kdmyEngine_ptrToString(title), from_modding_context)
}
function __js__kdmyEngine_create_array(size) {
    return kdmyEngine_obtain(new Array(size))
}
function __js__kdmyEngine_create_object() {
    return kdmyEngine_obtain(new Object)
}
function __js__kdmyEngine_forget_obtained(obj_id) {
    let ret = kdmyEngine_forget(obj_id);
    if (!ret)
        throw new Error("Uknown object id:" + obj_id)
}
function __js__kdmyEngine_get_language() {
    const code = window.navigator.language;
    const lang = new Intl.DisplayNames([code], {
        type: "language"
    });
    const name = lang.of(code);
    return kdmyEngine_stringToPtr(name)
}
function __js__kdmyEngine_get_locationquery() {
    let query = location.search;
    if (query.length > 0 && query[0] == "?")
        query = query.substring(1);
    let name = location.pathname;
    let idx = name.lastIndexOf("/");
    if (idx >= 0)
        name = name.substring(idx + 1);
    let str = name + " ";
    for (let part of query.split("&")) {
        let idx = part.indexOf("=");
        if (idx < 0) {
            str += decodeURIComponent(part) + " ";
            continue
        }
        let key = part.substring(0, idx);
        let value = part.substring(idx + 1);
        if (value.includes(" ") && value[0] != '"' && value[value.length - 1] != '"') {
            value = '"' + value + '"'
        }
        str += "-" + decodeURIComponent(key) + " " + value + " "
    }
    return kdmyEngine_stringToPtr(str)
}
function __js__kdmyEngine_get_screen_size(screen_width, screen_height) {
    kdmyEngine_set_int32(screen_width, pvr_context.screen_width);
    kdmyEngine_set_int32(screen_height, pvr_context.screen_height)
}
function __js__kdmyEngine_get_useragent() {
    return kdmyEngine_stringToPtr(navigator.userAgent)
}
function __js__kdmyEngine_open_link(url) {
    let target_url = kdmyEngine_ptrToString(url);
    if (!target_url || target_url.startsWith("javascript:") || target_url.startsWith("blob:"))
        return;
    window.open(target_url, "_blank", "noopener,noreferrer")
}
function __js__kdmyEngine_parse_json(L, json) {
    return luascript_helper_parse_json(L, ModuleLuaScript.kdmyEngine_obtain(json))
}
function __js__kdmyEngine_read_array_item_object(array_id, index) {
    let array = kdmyEngine_obtain(array_id);
    if (!array)
        throw new Error("Uknown array id:" + array_id);
    let ret = array[index];
    return kdmyEngine_obtain(ret)
}
function __js__kdmyEngine_read_prop_boolean(obj_id, field_name) {
    let obj = kdmyEngine_obtain(obj_id);
    if (!obj)
        throw new Error("Uknown object id:" + obj_id);
    let field = kdmyEngine_ptrToString(field_name);
    let ret = obj[field];
    return ret ? 1 : 0
}
function __js__kdmyEngine_read_prop_double(obj_id, field_name) {
    let obj = kdmyEngine_obtain(obj_id);
    if (!obj)
        throw new Error("Uknown object id:" + obj_id);
    let field = kdmyEngine_ptrToString(field_name);
    let ret = obj[field];
    return ret
}
function __js__kdmyEngine_read_prop_float(obj_id, field_name) {
    let obj = kdmyEngine_obtain(obj_id);
    if (!obj)
        throw new Error("Uknown object id:" + obj_id);
    let field = kdmyEngine_ptrToString(field_name);
    let ret = obj[field];
    return ret
}
function __js__kdmyEngine_read_prop_floatboolean(obj_id, field_name) {
    let obj = kdmyEngine_obtain(obj_id);
    if (!obj)
        throw new Error("Uknown object id:" + obj_id);
    let field = kdmyEngine_ptrToString(field_name);
    let ret = obj[field];
    return ret >= 1 || ret === true
}
function __js__kdmyEngine_read_prop_integer(obj_id, field_name) {
    let obj = kdmyEngine_obtain(obj_id);
    if (!obj)
        throw new Error("Uknown object id:" + obj_id);
    let field = kdmyEngine_ptrToString(field_name);
    let ret = obj[field];
    return ret
}
function __js__kdmyEngine_read_prop_object(obj_id, field_name) {
    let obj = kdmyEngine_obtain(obj_id);
    if (!obj)
        throw new Error("Uknown object id:" + obj_id);
    let field = kdmyEngine_ptrToString(field_name);
    let ret = obj[field];
    return kdmyEngine_obtain(typeof ret === "object" ? ret : null)
}
function __js__kdmyEngine_read_prop_string(obj_id, field_name) {
    let obj = kdmyEngine_obtain(obj_id);
    if (!obj)
        throw new Error("Uknown object id:" + obj_id);
    let field = kdmyEngine_ptrToString(field_name);
    let ret = obj[field];
    return kdmyEngine_stringToPtr(ret)
}
function __js__kdmyEngine_read_window_object(variable_name) {
    let obj = window[kdmyEngine_ptrToString(variable_name)];
    return obj === undefined ? 0 : kdmyEngine_obtain(obj)
}
function __js__kdmyEngine_require_window_attention() {
    alert(document.title + "\n Environment:require_window_attention()")
}
function __js__kdmyEngine_write_in_array_boolean(array_id, index, value) {
    let array = kdmyEngine_obtain(array_id);
    if (!array)
        throw new Error("Uknown array id:" + array_id);
    array[index] = value
}
function __js__kdmyEngine_write_in_array_double(array_id, index, value) {
    let array = kdmyEngine_obtain(array_id);
    if (!array)
        throw new Error("Uknown array id:" + array_id);
    array[index] = value
}
function __js__kdmyEngine_write_in_array_float(array_id, index, value) {
    let array = kdmyEngine_obtain(array_id);
    if (!array)
        throw new Error("Uknown array id:" + array_id);
    array[index] = value
}
function __js__kdmyEngine_write_in_array_integer(array_id, index, value) {
    let array = kdmyEngine_obtain(array_id);
    if (!array)
        throw new Error("Uknown array id:" + array_id);
    array[index] = value
}
function __js__kdmyEngine_write_in_array_object(array_id, index, value) {
    let array = kdmyEngine_obtain(array_id);
    if (!array)
        throw new Error("Uknown array id:" + array_id);
    array[index] = kdmyEngine_obtain(value)
}
function __js__kdmyEngine_write_in_array_string(array_id, index, value) {
    let array = kdmyEngine_obtain(array_id);
    if (!array)
        throw new Error("Uknown array id:" + array_id);
    array[index] = kdmyEngine_ptrToString(value)
}
function __js__kdmyEngine_write_prop_boolean(obj_id, field_name, value) {
    let obj = kdmyEngine_obtain(obj_id);
    if (!obj)
        throw new Error("Uknown object id:" + obj_id);
    let field = kdmyEngine_ptrToString(field_name);
    obj[field] = value
}
function __js__kdmyEngine_write_prop_double(obj_id, field_name, value) {
    let obj = kdmyEngine_obtain(obj_id);
    if (!obj)
        throw new Error("Uknown object id:" + obj_id);
    let field = kdmyEngine_ptrToString(field_name);
    obj[field] = value
}
function __js__kdmyEngine_write_prop_float(obj_id, field_name, value) {
    let obj = kdmyEngine_obtain(obj_id);
    if (!obj)
        throw new Error("Uknown object id:" + obj_id);
    let field = kdmyEngine_ptrToString(field_name);
    obj[field] = value
}
function __js__kdmyEngine_write_prop_integer(obj_id, field_name, value) {
    let obj = kdmyEngine_obtain(obj_id);
    if (!obj)
        throw new Error("Uknown object id:" + obj_id);
    let field = kdmyEngine_ptrToString(field_name);
    obj[field] = value
}
function __js__kdmyEngine_write_prop_object(obj_id, field_name, value) {
    let obj = kdmyEngine_obtain(obj_id);
    if (!obj)
        throw new Error("Uknown object id:" + obj_id);
    let field = kdmyEngine_ptrToString(field_name);
    obj[field] = kdmyEngine_obtain(value)
}
function __js__kdmyEngine_write_prop_string(obj_id, field_name, value) {
    let obj = kdmyEngine_obtain(obj_id);
    if (!obj)
        throw new Error("Uknown object id:" + obj_id);
    let field = kdmyEngine_ptrToString(field_name);
    obj[field] = kdmyEngine_ptrToString(value)
}
function __js__layout_animation_is_completed(layout, item_name) {
    return layout_animation_is_completed(kdmyEngine_obtain(layout), kdmyEngine_ptrToString(item_name))
}
function __js__layout_camera_is_completed(layout) {
    return layout_camera_is_completed(kdmyEngine_obtain(layout))
}
function __js__layout_camera_set_view(layout, x, y, z) {
    layout_camera_set_view(kdmyEngine_obtain(layout), x, y, z)
}
function __js__layout_contains_action(layout, target_name, action_name) {
    return layout_contains_action(kdmyEngine_obtain(layout), kdmyEngine_ptrToString(target_name), kdmyEngine_ptrToString(action_name))
}
function __js__layout_disable_antialiasing(layout, antialiasing) {
    layout_disable_antialiasing(kdmyEngine_obtain(layout), antialiasing)
}
function __js__layout_get_attached_value2(layout, name, result) {
    const value = [null];
    let type = layout_get_attached_value2(kdmyEngine_obtain(layout), kdmyEngine_ptrToString(name), value);
    switch (type) {
    case LAYOUT_TYPE_NOTFOUND:
        break;
    case LAYOUT_TYPE_STRING:
        kdmyEngine_set_uint32(result, kdmyEngine_stringToPtr(value[0]));
        break;
    case LAYOUT_TYPE_FLOAT:
        kdmyEngine_set_float32(result, value[0]);
        break;
    case LAYOUT_TYPE_INTEGER:
        kdmyEngine_set_int32(result, value[0]);
        break;
    case LAYOUT_TYPE_HEX:
        kdmyEngine_set_uint32(result, value[0]);
        break;
    case LAYOUT_TYPE_BOOLEAN:
        kdmyEngine_set_int32(result, value[0] ? 1 : 0);
        break;
    default:
        console.warn("Unknown layout type-value ", type, value[0]);
        break
    }
    return type
}
function __js__layout_get_attached_value_type(layout, name) {
    return layout_get_attached_value_type(kdmyEngine_obtain(layout), kdmyEngine_ptrToString(name))
}
function __js__layout_get_camera_helper(layout) {
    const camera = layout_get_camera_helper(kdmyEngine_obtain(layout));
    return kdmyEngine_obtain(camera)
}
function __js__layout_get_group_modifier(layout, group_name) {
    let modifier = layout_get_group_modifier(kdmyEngine_obtain(layout), kdmyEngine_ptrToString(group_name));
    return kdmyEngine_obtain(modifier)
}
function __js__layout_get_group_shader(layout, group_name) {
    let psshader = layout_get_group_shader(kdmyEngine_obtain(layout), kdmyEngine_ptrToString(group_name));
    return kdmyEngine_obtain(psshader)
}
function __js__layout_get_placeholder(layout, group_name) {
    let placeholder = layout_get_placeholder(kdmyEngine_obtain(layout), kdmyEngine_ptrToString(group_name));
    return kdmyEngine_obtain(placeholder)
}
function __js__layout_get_secondary_camera_helper(layout) {
    const camera = layout_get_secondary_camera_helper(kdmyEngine_obtain(layout));
    return kdmyEngine_obtain(camera)
}
function __js__layout_get_soundplayer(layout, name) {
    const soundplayer = layout_get_soundplayer(kdmyEngine_obtain(layout), kdmyEngine_ptrToString(name));
    return kdmyEngine_obtain(soundplayer)
}
function __js__layout_get_sprite(layout, name) {
    const sprite = layout_get_sprite(kdmyEngine_obtain(layout), kdmyEngine_ptrToString(name));
    return kdmyEngine_obtain(sprite)
}
function __js__layout_get_textsprite(layout, name) {
    const textsprite = layout_get_textsprite(kdmyEngine_obtain(layout), kdmyEngine_ptrToString(name));
    return kdmyEngine_obtain(textsprite)
}
function __js__layout_get_videoplayer(layout, name) {
    let ret = layout_get_videoplayer(kdmyEngine_obtain(layout), kdmyEngine_ptrToString(name));
    return kdmyEngine_obtain(ret)
}
function __js__layout_get_viewport_size(layout, viewport_width, viewport_height) {
    const values = [0, 0];
    layout_get_viewport_size(kdmyEngine_obtain(layout), values);
    kdmyEngine_set_float32(viewport_width, values[0]);
    kdmyEngine_set_float32(viewport_height, values[1])
}
function __js__layout_resume(layout) {
    layout_resume(kdmyEngine_obtain(layout))
}
function __js__layout_screen_to_layout_coordinates(layout, screen_x, screen_y, calc_with_camera, layout_x, layout_y) {
    const output_coords = [0, 0];
    layout_screen_to_layout_coordinates(kdmyEngine_obtain(layout), screen_x, screen_y, calc_with_camera, output_coords);
    kdmyEngine_set_float32(layout_x, output_coords[0]);
    kdmyEngine_set_float32(layout_y, output_coords[1])
}
function __js__layout_set_group_alpha(layout, group_name, alpha) {
    layout_set_group_alpha(kdmyEngine_obtain(layout), kdmyEngine_ptrToString(group_name), alpha)
}
function __js__layout_set_group_antialiasing(layout, group_name, antialiasing) {
    layout_set_group_antialiasing(kdmyEngine_obtain(layout), kdmyEngine_ptrToString(group_name), antialiasing)
}
function __js__layout_set_group_offsetcolor(layout, group_name, r, g, b, a) {
    layout_set_group_offsetcolor(kdmyEngine_obtain(layout), kdmyEngine_ptrToString(group_name), r, g, b, a)
}
function __js__layout_set_group_shader(layout, group_name, psshader) {
    return layout_set_group_shader(kdmyEngine_obtain(layout), kdmyEngine_ptrToString(group_name), kdmyEngine_obtain(psshader))
}
function __js__layout_set_group_visibility(layout, group_name, visible) {
    layout_set_group_visibility(kdmyEngine_obtain(layout), kdmyEngine_ptrToString(group_name), visible)
}
function __js__layout_stop_all_triggers(layout) {
    return layout_stop_all_triggers(kdmyEngine_obtain(layout))
}
function __js__layout_stop_trigger(layout, trigger_name) {
    return layout_stop_trigger(kdmyEngine_obtain(layout), kdmyEngine_ptrToString(trigger_name))
}
function __js__layout_suspend(layout) {
    layout_suspend(kdmyEngine_obtain(layout))
}
function __js__layout_trigger_action(layout, target_name, action_name) {
    return layout_trigger_action(kdmyEngine_obtain(layout), kdmyEngine_ptrToString(target_name), kdmyEngine_ptrToString(action_name))
}
function __js__layout_trigger_any(layout, action_triger_camera_interval_name) {
    return layout_trigger_any(kdmyEngine_obtain(layout), kdmyEngine_ptrToString(action_triger_camera_interval_name))
}
function __js__layout_trigger_camera(layout, camera_name) {
    return layout_trigger_camera(kdmyEngine_obtain(layout), kdmyEngine_ptrToString(camera_name))
}
function __js__layout_trigger_trigger(layout, trigger_name) {
    return layout_trigger_trigger(kdmyEngine_obtain(layout), kdmyEngine_ptrToString(trigger_name))
}
function __js__menu_destroy(menu) {
    let ret = menu_destroy(kdmyEngine_obtain(menu));
    return kdmyEngine_obtain(ret)
}
function __js__menu_get_drawable(menu) {
    let ret = menu_get_drawable(kdmyEngine_obtain(menu));
    return kdmyEngine_obtain(ret)
}
function __js__menu_get_item_rect(menu, index, x, y, width, height) {
    const output_location = [0, 0];
    const output_size = [0, 0];
    let ret = menu_get_item_rect(kdmyEngine_obtain(menu), index, output_location, output_size);
    kdmyEngine_set_float32(x, output_location[0]);
    kdmyEngine_set_float32(y, output_location[1]);
    kdmyEngine_set_float32(width, output_size[2]);
    kdmyEngine_set_float32(height, output_size[3]);
    return ret ? 1 : 0
}
function __js__menu_get_items_count(menu) {
    let ret = menu_get_items_count(kdmyEngine_obtain(menu));
    return ret
}
function __js__menu_get_selected_index(menu) {
    let ret = menu_get_selected_index(kdmyEngine_obtain(menu));
    return ret
}
function __js__menu_get_selected_item_name(menu) {
    let ret = menu_get_selected_item_name(kdmyEngine_obtain(menu));
    return kdmyEngine_stringToPtr(ret)
}
function __js__menu_has_item(menu, name) {
    let ret = menu_has_item(kdmyEngine_obtain(menu), kdmyEngine_ptrToString(name));
    return ret ? 1 : 0
}
function __js__menu_select_horizontal(menu, offset) {
    let ret = menu_select_horizontal(kdmyEngine_obtain(menu), offset);
    return ret ? 1 : 0
}
function __js__menu_select_index(menu, index) {
    menu_select_index(kdmyEngine_obtain(menu), index)
}
function __js__menu_select_item(menu, name) {
    let ret = menu_select_item(kdmyEngine_obtain(menu), kdmyEngine_ptrToString(name));
    return ret ? 1 : 0
}
function __js__menu_select_vertical(menu, offset) {
    let ret = menu_select_vertical(kdmyEngine_obtain(menu), offset);
    return ret ? 1 : 0
}
function __js__menu_set_item_text(menu, index, text) {
    let ret = menu_set_item_text(kdmyEngine_obtain(menu), index, kdmyEngine_ptrToString(text));
    return ret ? 1 : 0
}
function __js__menu_set_item_visibility(menu, index, visible) {
    let ret = menu_set_item_visibility(kdmyEngine_obtain(menu), index, visible);
    return ret ? 1 : 0
}
function __js__menu_set_text_force_case(menu, none_or_lowercase_or_uppercase) {
    menu_set_text_force_case(kdmyEngine_obtain(menu), none_or_lowercase_or_uppercase)
}
function __js__menu_toggle_choosen(menu, enable) {
    menu_toggle_choosen(kdmyEngine_obtain(menu), enable)
}
function __js__menu_trasition_in(menu) {
    menu_trasition_in(kdmyEngine_obtain(menu))
}
function __js__menu_trasition_out(menu) {
    menu_trasition_out(kdmyEngine_obtain(menu))
}
function __js__menumanifest_destroy(menumanifest) {
    menumanifest_destroy(kdmyEngine_obtain(kdmyEngine_get_uint32(menumanifest)))
}
function __js__messagebox_get_modifier(messagebox) {
    const modifier = messagebox_get_modifier(kdmyEngine_obtain(messagebox));
    return kdmyEngine_obtain(modifier)
}
function __js__messagebox_hide(messagebox, animated) {
    messagebox_hide(kdmyEngine_obtain(messagebox), animated)
}
function __js__messagebox_hide_image(messagebox, hide) {
    messagebox_hide_image(kdmyEngine_obtain(messagebox), hide)
}
function __js__messagebox_hide_image_background(messagebox, hide) {
    messagebox_hide_image_background(kdmyEngine_obtain(messagebox), hide)
}
function __js__messagebox_set_button_single(messagebox, center_text) {
    messagebox_set_button_single(kdmyEngine_obtain(messagebox), kdmyEngine_ptrToString(center_text))
}
function __js__messagebox_set_button_single_icon(messagebox, center_icon_name) {
    messagebox_set_button_single_icon(kdmyEngine_obtain(messagebox), kdmyEngine_ptrToString(center_icon_name))
}
function __js__messagebox_set_buttons_icons(messagebox, left_icon_name, right_icon_name) {
    messagebox_set_buttons_icons(kdmyEngine_obtain(messagebox), kdmyEngine_ptrToString(left_icon_name), kdmyEngine_ptrToString(right_icon_name))
}
function __js__messagebox_set_buttons_text(messagebox, left_text, right_text) {
    messagebox_set_buttons_text(kdmyEngine_obtain(messagebox), kdmyEngine_ptrToString(left_text), kdmyEngine_ptrToString(right_text))
}
function __js__messagebox_set_image_background_color(messagebox, color_rgb8) {
    messagebox_set_image_background_color(kdmyEngine_obtain(messagebox), color_rgb8)
}
function __js__messagebox_set_image_background_color_default(messagebox) {
    messagebox_set_image_background_color_default(kdmyEngine_obtain(messagebox))
}
function __js__messagebox_set_image_sprite(messagebox, sprite) {
    messagebox_set_image_sprite(kdmyEngine_obtain(messagebox), sprite)
}
function __js__messagebox_set_message(messagebox, text) {
    messagebox_set_message(kdmyEngine_obtain(messagebox), kdmyEngine_ptrToString(text))
}
function __js__messagebox_set_title(messagebox, text) {
    messagebox_set_title(kdmyEngine_obtain(messagebox), kdmyEngine_ptrToString(text))
}
function __js__messagebox_set_z_index(messagebox, z_index) {
    messagebox_set_z_index(kdmyEngine_obtain(messagebox), z_index)
}
function __js__messagebox_show(messagebox, animated) {
    messagebox_show(kdmyEngine_obtain(messagebox), animated)
}
function __js__messagebox_show_buttons_icons(messagebox, show) {
    messagebox_show_buttons_icons(kdmyEngine_obtain(messagebox), show)
}
function __js__messagebox_use_small_size(messagebox, small_or_normal) {
    messagebox_use_small_size(kdmyEngine_obtain(messagebox), small_or_normal)
}
function __js__missnotefx_play_effect(missnotefx) {
    missnotefx_play_effect(kdmyEngine_obtain(missnotefx))
}
function __js__modding_choose_native_menu_option(modding, name) {
    let ret = modding_choose_native_menu_option(kdmyEngine_obtain(modding), kdmyEngine_ptrToString(name));
    return ret ? 1 : 0
}
function __js__modding_exit(modding) {
    modding_exit(kdmyEngine_obtain(modding))
}
function __js__modding_get_active_menu(modding) {
    let ret = modding_get_active_menu(kdmyEngine_obtain(modding));
    return kdmyEngine_obtain(ret)
}
function __js__modding_get_layout(modding) {
    let ret = modding_get_layout(kdmyEngine_obtain(modding));
    return kdmyEngine_obtain(ret)
}
function __js__modding_get_loaded_weeks(modding, out_size) {
    const size = [0];
    let ret = modding_get_loaded_weeks(kdmyEngine_obtain(modding), size);
    kdmyEngine_set_uint32(out_size, size[0]);
    return kdmyEngine_obtain(ret)
}
function __js__modding_get_native_background_music(modding) {
    let ret = modding_get_native_background_music(kdmyEngine_obtain(modding));
    return kdmyEngine_obtain(ret)
}
function __js__modding_get_native_menu(modding) {
    let ret = modding_get_native_menu(kdmyEngine_obtain(modding));
    return kdmyEngine_obtain(ret)
}
function __js__modding_set_active_menu(modding, menu) {
    modding_set_active_menu(kdmyEngine_obtain(modding), kdmyEngine_obtain(menu))
}
function __js__modding_set_exit_delay(modding, delay_ms) {
    modding_set_exit_delay(kdmyEngine_obtain(modding), delay_ms)
}
function __js__modding_set_halt(modding, halt) {
    modding_set_halt(kdmyEngine_obtain(modding), halt)
}
function __js__modding_set_menu_in_layout_placeholder(modding, placeholder_name, menu) {
    modding_set_menu_in_layout_placeholder(kdmyEngine_obtain(modding), kdmyEngine_ptrToString(placeholder_name), kdmyEngine_obtain(menu))
}
function __js__modding_storage_get(modding, week_name, name, data) {
    let arraybuffer = [null];
    let ret = modding_storage_get(kdmyEngine_obtain(modding), kdmyEngine_ptrToString(week_name), kdmyEngine_ptrToString(name), arraybuffer);
    let ptr;
    if (arraybuffer[0]) {
        ptr = _malloc(ret);
        if (ptr != 0)
            HEAPU8.set(new Uint8Array(arraybuffer[0]), ptr)
    } else {
        ptr = 0
    }
    kdmyEngine_set_uint32(data, ptr);
    return ret
}
function __js__modding_storage_set(modding, week_name, name, data, data_size) {
    let arraybuffer = data == 0 ? null : new ArrayBuffer(data_size);
    if (arraybuffer) {
        new Uint8Array(arraybuffer).set(HEAPU8.subarray(data, data + data_size), 0)
    }
    let ret = modding_storage_set(kdmyEngine_obtain(modding), kdmyEngine_ptrToString(week_name), kdmyEngine_ptrToString(name), arraybuffer, data_size);
    return ret ? 1 : 0
}
function __js__modding_unlockdirective_create(modding, name, value) {
    modding_unlockdirective_create(kdmyEngine_obtain(modding), kdmyEngine_ptrToString(name), value)
}
function __js__modding_unlockdirective_get(modding, name) {
    let ret = modding_unlockdirective_get(kdmyEngine_obtain(modding), kdmyEngine_ptrToString(name));
    return ret
}
function __js__modding_unlockdirective_has(modding, name) {
    let ret = modding_unlockdirective_has(kdmyEngine_obtain(modding), kdmyEngine_ptrToString(name));
    return ret ? 1 : 0
}
function __js__modding_unlockdirective_remove(modding, name) {
    modding_unlockdirective_remove(kdmyEngine_obtain(modding), kdmyEngine_ptrToString(name))
}
function __js__modelholder_create_animsprite(modelholder, animation_name, fallback_static, no_return_null) {
    let ret = modelholder_create_animsprite(kdmyEngine_obtain(modelholder), kdmyEngine_ptrToString(animation_name), fallback_static, no_return_null);
    return kdmyEngine_obtain(ret)
}
function __js__modelholder_destroy(modelholder) {
    modelholder_destroy(kdmyEngine_obtain(modelholder))
}
function __js__modelholder_get_animlist(modelholder) {
    let ret = modelholder_get_animlist(kdmyEngine_obtain(modelholder));
    return kdmyEngine_obtain(ret)
}
function __js__modelholder_get_atlas(modelholder) {
    let ret = modelholder_get_atlas(kdmyEngine_obtain(modelholder));
    return kdmyEngine_obtain(ret)
}
function __js__modelholder_get_atlas_entry(modelholder, atlas_entry_name, return_copy) {
    let ret = modelholder_get_atlas_entry(kdmyEngine_obtain(modelholder), kdmyEngine_ptrToString(atlas_entry_name), return_copy);
    return kdmyEngine_obtain(ret)
}
function __js__modelholder_get_atlas_entry2(modelholder, atlas_entry_name, return_copy) {
    let ret = modelholder_get_atlas_entry2(kdmyEngine_obtain(modelholder), kdmyEngine_ptrToString(atlas_entry_name), return_copy);
    return kdmyEngine_obtain(ret)
}
function __js__modelholder_get_texture_resolution(modelholder, resolution_width, resolution_height) {
    const values = [0, 0];
    modelholder_get_texture_resolution(kdmyEngine_obtain(modelholder), values);
    kdmyEngine_set_int32(resolution_width, values[0]);
    kdmyEngine_set_int32(resolution_height, values[1])
}
function __js__modelholder_get_vertex_color(modelholder) {
    let ret = modelholder_get_vertex_color(kdmyEngine_obtain(modelholder));
    return ret
}
function __js__modelholder_has_animlist(modelholder) {
    let ret = modelholder_has_animlist(kdmyEngine_obtain(modelholder));
    return ret ? 1 : 0
}
function __js__modelholder_is_invalid(modelholder) {
    let ret = modelholder_is_invalid(kdmyEngine_obtain(modelholder));
    return ret ? 1 : 0
}
function __js__modelholder_utils_is_known_extension(filename) {
    let ret = modelholder_utils_is_known_extension(kdmyEngine_ptrToString(filename));
    return ret ? 1 : 0
}
function __js__playerstats_add_extra_health(playerstats, multiplier) {
    playerstats_add_extra_health(kdmyEngine_obtain(playerstats), multiplier)
}
function __js__playerstats_add_health(playerstats, health, die_if_negative) {
    let ret = playerstats_add_health(kdmyEngine_obtain(playerstats), health, die_if_negative);
    return ret
}
function __js__playerstats_add_hit(playerstats, multiplier, base_note_duration, hit_time_difference) {
    let ret = playerstats_add_hit(kdmyEngine_obtain(playerstats), multiplier, base_note_duration, hit_time_difference);
    return ret
}
function __js__playerstats_add_miss(playerstats, multiplier) {
    playerstats_add_miss(kdmyEngine_obtain(playerstats), multiplier)
}
function __js__playerstats_add_penality(playerstats, on_empty_strum) {
    playerstats_add_penality(kdmyEngine_obtain(playerstats), on_empty_strum)
}
function __js__playerstats_add_sustain(playerstats, quarters, is_released) {
    playerstats_add_sustain(kdmyEngine_obtain(playerstats), quarters, is_released)
}
function __js__playerstats_add_sustain_delayed_hit(playerstats, multiplier, hit_time_difference) {
    let ret = playerstats_add_sustain_delayed_hit(kdmyEngine_obtain(playerstats), multiplier, hit_time_difference);
    return ret
}
function __js__playerstats_enable_penality_on_empty_strum(playerstats, enable) {
    playerstats_enable_penality_on_empty_strum(kdmyEngine_obtain(playerstats), enable)
}
function __js__playerstats_get_accuracy(playerstats) {
    let ret = playerstats_get_accuracy(kdmyEngine_obtain(playerstats));
    return ret
}
function __js__playerstats_get_bads(playerstats) {
    let ret = playerstats_get_bads(kdmyEngine_obtain(playerstats));
    return ret
}
function __js__playerstats_get_combo_breaks(playerstats) {
    let ret = playerstats_get_combo_breaks(kdmyEngine_obtain(playerstats));
    return ret
}
function __js__playerstats_get_combo_streak(playerstats) {
    let ret = playerstats_get_combo_streak(kdmyEngine_obtain(playerstats));
    return ret
}
function __js__playerstats_get_goods(playerstats) {
    let ret = playerstats_get_goods(kdmyEngine_obtain(playerstats));
    return ret
}
function __js__playerstats_get_health(playerstats) {
    let ret = playerstats_get_health(kdmyEngine_obtain(playerstats));
    return ret
}
function __js__playerstats_get_highest_combo_streak(playerstats) {
    let ret = playerstats_get_highest_combo_streak(kdmyEngine_obtain(playerstats));
    return ret
}
function __js__playerstats_get_hits(playerstats) {
    let ret = playerstats_get_hits(kdmyEngine_obtain(playerstats));
    return ret
}
function __js__playerstats_get_iterations(playerstats) {
    let ret = playerstats_get_iterations(kdmyEngine_obtain(playerstats));
    return ret
}
function __js__playerstats_get_last_accuracy(playerstats) {
    let ret = playerstats_get_last_accuracy(kdmyEngine_obtain(playerstats));
    return ret
}
function __js__playerstats_get_last_difference(playerstats) {
    let ret = playerstats_get_last_difference(kdmyEngine_obtain(playerstats));
    return ret
}
function __js__playerstats_get_last_ranking(playerstats) {
    let ret = playerstats_get_last_ranking(kdmyEngine_obtain(playerstats));
    return ret
}
function __js__playerstats_get_maximum_health(playerstats) {
    let ret = playerstats_get_maximum_health(kdmyEngine_obtain(playerstats));
    return ret
}
function __js__playerstats_get_misses(playerstats) {
    let ret = playerstats_get_misses(kdmyEngine_obtain(playerstats));
    return ret
}
function __js__playerstats_get_notes_per_seconds(playerstats) {
    let ret = playerstats_get_notes_per_seconds(kdmyEngine_obtain(playerstats));
    return ret
}
function __js__playerstats_get_notes_per_seconds_highest(playerstats) {
    let ret = playerstats_get_notes_per_seconds_highest(kdmyEngine_obtain(playerstats));
    return ret
}
function __js__playerstats_get_penalties(playerstats) {
    let ret = playerstats_get_penalties(kdmyEngine_obtain(playerstats));
    return ret
}
function __js__playerstats_get_score(playerstats) {
    let ret = playerstats_get_score(kdmyEngine_obtain(playerstats));
    return ret
}
function __js__playerstats_get_shits(playerstats) {
    let ret = playerstats_get_shits(kdmyEngine_obtain(playerstats));
    return ret
}
function __js__playerstats_get_sicks(playerstats) {
    let ret = playerstats_get_sicks(kdmyEngine_obtain(playerstats));
    return ret
}
function __js__playerstats_is_dead(playerstats) {
    let ret = playerstats_is_dead(kdmyEngine_obtain(playerstats));
    return ret ? 1 : 0
}
function __js__playerstats_kill(playerstats) {
    playerstats_kill(kdmyEngine_obtain(playerstats))
}
function __js__playerstats_kill_if_negative_health(playerstats) {
    playerstats_kill_if_negative_health(kdmyEngine_obtain(playerstats))
}
function __js__playerstats_raise(playerstats, with_full_health) {
    playerstats_raise(kdmyEngine_obtain(playerstats), with_full_health)
}
function __js__playerstats_reset(playerstats) {
    playerstats_reset(kdmyEngine_obtain(playerstats))
}
function __js__playerstats_reset_notes_per_seconds(playerstats) {
    playerstats_reset_notes_per_seconds(kdmyEngine_obtain(playerstats))
}
function __js__playerstats_set_health(playerstats, health) {
    playerstats_set_health(kdmyEngine_obtain(playerstats), health)
}
function __js__psshader_destroy(psshader) {
    kdmyEngine_obtain(kdmyEngine_get_uint32(psshader)).Destroy()
}
function __js__psshader_init(vertex_sourcecode, fragment_sourcecode) {
    let psshader = PSShader.BuildFromSource(pvr_context, kdmyEngine_ptrToString(vertex_sourcecode), kdmyEngine_ptrToString(fragment_sourcecode));
    return kdmyEngine_obtain(psshader)
}
function __js__psshader_set_uniform1f(psshader, name, value) {
    kdmyEngine_obtain(psshader).SetUniform1F(kdmyEngine_ptrToString(name), value)
}
function __js__psshader_set_uniform1i(psshader, name, value) {
    kdmyEngine_obtain(psshader).SetUniform1I(kdmyEngine_ptrToString(name), value)
}
function __js__psshader_set_uniform_any(psshader, name, values) {
    const val = new Float32Array(buffer, values, 128);
    kdmyEngine_obtain(psshader).SetUniformAny(kdmyEngine_ptrToString(name), val)
}
function __js__rankingcounter_add_state(rankingcounter, modelholder, state_name) {
    let ret = rankingcounter_add_state(kdmyEngine_obtain(rankingcounter), kdmyEngine_obtain(modelholder), kdmyEngine_ptrToString(state_name));
    return ret
}
function __js__rankingcounter_animation_end(rankingcounter) {
    rankingcounter_animation_end(kdmyEngine_obtain(rankingcounter))
}
function __js__rankingcounter_animation_restart(rankingcounter) {
    rankingcounter_animation_restart(kdmyEngine_obtain(rankingcounter))
}
function __js__rankingcounter_animation_set(rankingcounter, animsprite) {
    rankingcounter_animation_set(kdmyEngine_obtain(rankingcounter), kdmyEngine_obtain(animsprite))
}
function __js__rankingcounter_hide_accuracy(rankingcounter, hide) {
    rankingcounter_hide_accuracy(kdmyEngine_obtain(rankingcounter), hide)
}
function __js__rankingcounter_reset(rankingcounter) {
    rankingcounter_reset(kdmyEngine_obtain(rankingcounter))
}
function __js__rankingcounter_set_alpha(rankingcounter, alpha) {
    rankingcounter_set_alpha(kdmyEngine_obtain(rankingcounter), alpha)
}
function __js__rankingcounter_set_default_ranking_animation2(rankingcounter, animsprite) {
    rankingcounter_set_default_ranking_animation2(kdmyEngine_obtain(rankingcounter), kdmyEngine_obtain(animsprite))
}
function __js__rankingcounter_set_default_ranking_text_animation2(rankingcounter, animsprite) {
    rankingcounter_set_default_ranking_text_animation2(kdmyEngine_obtain(rankingcounter), kdmyEngine_obtain(animsprite))
}
function __js__rankingcounter_toggle_state(rankingcounter, state_name) {
    rankingcounter_toggle_state(kdmyEngine_obtain(rankingcounter), kdmyEngine_ptrToString(state_name))
}
function __js__rankingcounter_use_percent_instead(rankingcounter, use_accuracy_percenter) {
    rankingcounter_use_percent_instead(kdmyEngine_obtain(rankingcounter), use_accuracy_percenter)
}
function __js__roundstats_get_drawable(roundstats) {
    let ret = roundstats_get_drawable(kdmyEngine_obtain(roundstats));
    return kdmyEngine_obtain(ret)
}
function __js__roundstats_hide(roundstats, hide) {
    roundstats_hide(kdmyEngine_obtain(roundstats), hide)
}
function __js__roundstats_hide_nps(roundstats, hide) {
    roundstats_hide_nps(kdmyEngine_obtain(roundstats), hide)
}
function __js__roundstats_reset(roundstats) {
    roundstats_reset(kdmyEngine_obtain(roundstats))
}
function __js__roundstats_set_draw_y(roundstats, y) {
    roundstats_set_draw_y(kdmyEngine_obtain(roundstats), y)
}
function __js__roundstats_tweenkeyframe_set_bpm(roundstats, beats_per_minute) {
    roundstats_tweenkeyframe_set_bpm(kdmyEngine_obtain(roundstats), beats_per_minute)
}
function __js__roundstats_tweenkeyframe_set_on_beat(roundstats, tweenkeyframe, rollback_beats, beat_duration) {
    roundstats_tweenkeyframe_set_on_beat(kdmyEngine_obtain(roundstats), kdmyEngine_obtain(tweenkeyframe), rollback_beats, beat_duration)
}
function __js__roundstats_tweenkeyframe_set_on_hit(roundstats, tweenkeyframe, rollback_beats, beat_duration) {
    roundstats_tweenkeyframe_set_on_hit(kdmyEngine_obtain(roundstats), kdmyEngine_obtain(tweenkeyframe), rollback_beats, beat_duration)
}
function __js__roundstats_tweenkeyframe_set_on_miss(roundstats, tweenkeyframe, rollback_beats, beat_duration) {
    roundstats_tweenkeyframe_set_on_miss(kdmyEngine_obtain(roundstats), kdmyEngine_obtain(tweenkeyframe), rollback_beats, beat_duration)
}
function __js__songplayer_changesong(songplayer, src, prefer_no_copyright) {
    return songplayer_changesong(kdmyEngine_obtain(songplayer), kdmyEngine_ptrToString(src), prefer_no_copyright)
}
function __js__songplayer_get_duration(songplayer) {
    return songplayer_get_duration(kdmyEngine_obtain(songplayer))
}
function __js__songplayer_get_timestamp(songplayer) {
    return songplayer_get_timestamp(kdmyEngine_obtain(songplayer))
}
function __js__songplayer_is_completed(songplayer) {
    return songplayer_is_completed(kdmyEngine_obtain(songplayer))
}
function __js__songplayer_mute(songplayer, muted) {
    songplayer_mute(kdmyEngine_obtain(songplayer), muted)
}
function __js__songplayer_mute_track(songplayer, vocals_or_instrumental, muted) {
    songplayer_mute_track(kdmyEngine_obtain(songplayer), vocals_or_instrumental, muted)
}
function __js__songplayer_pause(songplayer) {
    songplayer_pause(kdmyEngine_obtain(songplayer))
}
function __js__songplayer_seek(songplayer, timestamp) {
    songplayer_seek(kdmyEngine_obtain(songplayer), timestamp)
}
function __js__songplayer_set_volume(songplayer, volume) {
    songplayer_set_volume(kdmyEngine_obtain(songplayer), volume)
}
function __js__songplayer_set_volume_track(songplayer, vocals_or_instrumental, volume) {
    songplayer_set_volume_track(kdmyEngine_obtain(songplayer), vocals_or_instrumental, volume)
}
function __js__songprogressbar_animation_end(songprogressbar) {
    songprogressbar_animation_end(kdmyEngine_obtain(songprogressbar))
}
function __js__songprogressbar_animation_restart(songprogressbar) {
    songprogressbar_animation_restart(kdmyEngine_obtain(songprogressbar))
}
function __js__songprogressbar_animation_set(songprogressbar, animsprite) {
    songprogressbar_animation_set(kdmyEngine_obtain(songprogressbar), kdmyEngine_obtain(animsprite))
}
function __js__songprogressbar_get_drawable(songprogressbar) {
    let ret = songprogressbar_get_drawable(kdmyEngine_obtain(songprogressbar));
    return kdmyEngine_obtain(ret)
}
function __js__songprogressbar_hide_time(songprogressbar, hidden) {
    songprogressbar_hide_time(kdmyEngine_obtain(songprogressbar), hidden)
}
function __js__songprogressbar_manual_set_position(songprogressbar, elapsed, duration, should_update_time_text) {
    let ret = songprogressbar_manual_set_position(kdmyEngine_obtain(songprogressbar), elapsed, duration, should_update_time_text);
    return ret
}
function __js__songprogressbar_manual_set_text(songprogressbar, text) {
    songprogressbar_manual_set_text(kdmyEngine_obtain(songprogressbar), kdmyEngine_ptrToString(text))
}
function __js__songprogressbar_manual_update_enable(songprogressbar, enabled) {
    songprogressbar_manual_update_enable(kdmyEngine_obtain(songprogressbar), enabled)
}
function __js__songprogressbar_set_background_color(songprogressbar, r, g, b, a) {
    songprogressbar_set_background_color(kdmyEngine_obtain(songprogressbar), r, g, b, a)
}
function __js__songprogressbar_set_bar_back_color(songprogressbar, r, g, b, a) {
    songprogressbar_set_bar_back_color(kdmyEngine_obtain(songprogressbar), r, g, b, a)
}
function __js__songprogressbar_set_bar_progress_color(songprogressbar, r, g, b, a) {
    songprogressbar_set_bar_progress_color(kdmyEngine_obtain(songprogressbar), r, g, b, a)
}
function __js__songprogressbar_set_duration(songprogressbar, duration) {
    songprogressbar_set_duration(kdmyEngine_obtain(songprogressbar), duration)
}
function __js__songprogressbar_set_songplayer(songprogressbar, songplayer) {
    songprogressbar_set_songplayer(kdmyEngine_obtain(songprogressbar), kdmyEngine_obtain(songplayer))
}
function __js__songprogressbar_set_text_color(songprogressbar, r, g, b, a) {
    songprogressbar_set_text_color(kdmyEngine_obtain(songprogressbar), r, g, b, a)
}
function __js__songprogressbar_set_visible(songprogressbar, visible) {
    songprogressbar_set_visible(kdmyEngine_obtain(songprogressbar), visible)
}
function __js__songprogressbar_show_elapsed(songprogressbar, elapsed_or_remain_time) {
    songprogressbar_show_elapsed(kdmyEngine_obtain(songprogressbar), elapsed_or_remain_time)
}
function __js__soundplayer_fade(soundplayer, in_or_out, duration) {
    soundplayer_fade(kdmyEngine_obtain(soundplayer), in_or_out, duration)
}
function __js__soundplayer_get_duration(soundplayer) {
    return soundplayer_get_duration(kdmyEngine_obtain(soundplayer))
}
function __js__soundplayer_get_position(soundplayer) {
    return soundplayer_get_position(kdmyEngine_obtain(soundplayer))
}
function __js__soundplayer_has_ended(soundplayer) {
    return soundplayer_has_ended(kdmyEngine_obtain(soundplayer))
}
function __js__soundplayer_is_muted(soundplayer) {
    return soundplayer_is_muted(kdmyEngine_obtain(soundplayer))
}
function __js__soundplayer_is_playing(soundplayer) {
    return soundplayer_is_playing(kdmyEngine_obtain(soundplayer))
}
function __js__soundplayer_loop_enable(soundplayer, enable) {
    soundplayer_loop_enable(kdmyEngine_obtain(soundplayer), enable)
}
function __js__soundplayer_pause(soundplayer) {
    soundplayer_pause(kdmyEngine_obtain(soundplayer))
}
function __js__soundplayer_play(soundplayer) {
    soundplayer_play(kdmyEngine_obtain(soundplayer))
}
function __js__soundplayer_seek(soundplayer, timestamp) {
    return soundplayer_seek(kdmyEngine_obtain(soundplayer), timestamp)
}
function __js__soundplayer_set_mute(soundplayer, muted) {
    soundplayer_set_mute(kdmyEngine_obtain(soundplayer), muted)
}
function __js__soundplayer_set_volume(soundplayer, volume) {
    soundplayer_set_volume(kdmyEngine_obtain(soundplayer), volume)
}
function __js__soundplayer_stop(soundplayer) {
    soundplayer_stop(kdmyEngine_obtain(soundplayer))
}
function __js__sprite_blend_enable(sprite, enabled) {
    sprite_blend_enable(kdmyEngine_obtain(sprite), enabled)
}
function __js__sprite_blend_set(sprite, src_rgb, dst_rgb, src_alpha, dst_alpha) {
    sprite_blend_set(kdmyEngine_obtain(sprite), src_rgb, dst_rgb, src_alpha, dst_alpha)
}
function __js__sprite_center_draw_location(sprite, x, y, ref_width, ref_height, applied_draw_x, applied_draw_y) {
    const values = [0, 0];
    sprite_center_draw_location(kdmyEngine_obtain(sprite), x, y, ref_width, ref_height, values);
    kdmyEngine_set_float32(applied_draw_x, values[0]);
    kdmyEngine_set_float32(applied_draw_y, values[1])
}
function __js__sprite_crop(sprite, dx, dy, dwidth, dheight) {
    return sprite_crop(kdmyEngine_obtain(sprite), dx, dy, dwidth, dheight)
}
function __js__sprite_crop_enable(sprite, enable) {
    sprite_crop_enable(kdmyEngine_obtain(sprite), enable)
}
function __js__sprite_flip_rendered_texture(sprite, flip_x, flip_y) {
    sprite_flip_rendered_texture(kdmyEngine_obtain(sprite), flip_x < 0 ? null : flip_x, flip_y < 0 ? null : flip_y)
}
function __js__sprite_flip_rendered_texture_enable_correction(sprite, enabled) {
    sprite_flip_rendered_texture_enable_correction(kdmyEngine_obtain(sprite), enabled)
}
function __js__sprite_get_shader(sprite) {
    let psshader = sprite_get_shader(kdmyEngine_obtain(sprite));
    return kdmyEngine_obtain(psshader)
}
function __js__sprite_get_source_size(sprite, source_width, source_height) {
    const values = [0, 0];
    sprite_get_source_size(kdmyEngine_obtain(sprite), values);
    kdmyEngine_set_float32(source_width, values[0]);
    kdmyEngine_set_float32(source_height, values[1])
}
function __js__sprite_is_crop_enabled(sprite) {
    return sprite_is_crop_enabled(kdmyEngine_obtain(sprite))
}
function __js__sprite_is_textured(sprite) {
    return sprite_is_textured(kdmyEngine_obtain(sprite))
}
function __js__sprite_matrix_get_modifier(sprite) {
    const modifier = sprite_matrix_get_modifier(kdmyEngine_obtain(sprite));
    return kdmyEngine_obtain(modifier)
}
function __js__sprite_matrix_reset(sprite) {
    sprite_matrix_reset(kdmyEngine_obtain(sprite))
}
function __js__sprite_resize_draw_size(sprite, max_width, max_height, applied_draw_width, applied_draw_height) {
    const values = [0, 0];
    sprite_resize_draw_size(kdmyEngine_obtain(sprite), max_width, max_height, values);
    kdmyEngine_set_float32(applied_draw_width, values[0]);
    kdmyEngine_set_float32(applied_draw_height, values[1])
}
function __js__sprite_set_alpha(sprite, alpha) {
    sprite_set_alpha(kdmyEngine_obtain(sprite), alpha)
}
function __js__sprite_set_antialiasing(sprite, antialiasing) {
    sprite_set_antialiasing(kdmyEngine_obtain(sprite), antialiasing)
}
function __js__sprite_set_draw_location(sprite, x, y) {
    sprite_set_draw_location(kdmyEngine_obtain(sprite), x, y)
}
function __js__sprite_set_draw_size(sprite, width, height) {
    sprite_set_draw_size(kdmyEngine_obtain(sprite), width, height)
}
function __js__sprite_set_draw_size_from_source_size(sprite) {
    sprite_set_draw_size_from_source_size(kdmyEngine_obtain(sprite))
}
function __js__sprite_set_offset_frame(sprite, x, y, width, height) {
    sprite_set_offset_frame(kdmyEngine_obtain(sprite), x, y, width, height)
}
function __js__sprite_set_offset_pivot(sprite, x, y) {
    sprite_set_offset_pivot(kdmyEngine_obtain(sprite), x, y)
}
function __js__sprite_set_offset_source(sprite, x, y, width, height) {
    sprite_set_offset_source(kdmyEngine_obtain(sprite), x, y, width, height)
}
function __js__sprite_set_offsetcolor(sprite, r, g, b, a) {
    sprite_set_offsetcolor(kdmyEngine_obtain(sprite), r, g, b, a)
}
function __js__sprite_set_shader(sprite, psshader) {
    sprite_set_shader(kdmyEngine_obtain(sprite), kdmyEngine_obtain(psshader))
}
function __js__sprite_set_vertex_color(sprite, r, g, b) {
    sprite_set_vertex_color(kdmyEngine_obtain(sprite), r, g, b)
}
function __js__sprite_set_visible(sprite, visible) {
    sprite_set_visible(kdmyEngine_obtain(sprite), visible)
}
function __js__sprite_set_z_index(sprite, index) {
    sprite_set_z_index(kdmyEngine_obtain(sprite), index)
}
function __js__sprite_set_z_offset(sprite, offset) {
    sprite_set_z_offset(kdmyEngine_obtain(sprite), offset)
}
function __js__sprite_trailing_enabled(sprite, enabled) {
    sprite_trailing_enabled(kdmyEngine_obtain(sprite), enabled)
}
function __js__sprite_trailing_set_offsetcolor(sprite, r, g, b) {
    sprite_trailing_set_offsetcolor(kdmyEngine_obtain(sprite), r, g, b)
}
function __js__sprite_trailing_set_params(sprite, length, trail_delay, trail_alpha, darken_colors) {
    sprite_trailing_set_params(kdmyEngine_obtain(sprite), length, trail_delay, trail_alpha, darken_colors == 0 ? null : kdmyEngine_get_uint32(darken_colors))
}
function __js__streakcounter_animation_end(streakcounter) {
    streakcounter_animation_end(kdmyEngine_obtain(streakcounter))
}
function __js__streakcounter_animation_restart(streakcounter) {
    streakcounter_animation_restart(kdmyEngine_obtain(streakcounter))
}
function __js__streakcounter_animation_set(streakcounter, animsprite) {
    streakcounter_animation_set(kdmyEngine_obtain(streakcounter), kdmyEngine_obtain(animsprite))
}
function __js__streakcounter_get_drawable(streakcounter) {
    let ret = streakcounter_get_drawable(kdmyEngine_obtain(streakcounter));
    return kdmyEngine_obtain(ret)
}
function __js__streakcounter_hide_combo_sprite(streakcounter, hide) {
    streakcounter_hide_combo_sprite(kdmyEngine_obtain(streakcounter), hide)
}
function __js__streakcounter_reset(streakcounter) {
    streakcounter_reset(kdmyEngine_obtain(streakcounter))
}
function __js__streakcounter_set_alpha(streakcounter, alpha) {
    streakcounter_set_alpha(kdmyEngine_obtain(streakcounter), alpha)
}
function __js__streakcounter_set_combo_draw_location(streakcounter, x, y) {
    streakcounter_set_combo_draw_location(kdmyEngine_obtain(streakcounter), x, y)
}
function __js__streakcounter_state_add(streakcounter, combo_modelholder, number_modelholder, state_name) {
    let ret = streakcounter_state_add(kdmyEngine_obtain(streakcounter), kdmyEngine_obtain(combo_modelholder), kdmyEngine_obtain(number_modelholder), kdmyEngine_ptrToString(state_name));
    return ret
}
function __js__streakcounter_state_toggle(streakcounter, state_name) {
    let ret = streakcounter_state_toggle(kdmyEngine_obtain(streakcounter), kdmyEngine_ptrToString(state_name));
    return ret ? 1 : 0
}
function __js__strum_animation_end(strum) {
    strum_animation_end(kdmyEngine_obtain(strum))
}
function __js__strum_animation_restart(strum) {
    strum_animation_restart(kdmyEngine_obtain(strum))
}
function __js__strum_disable_beat_synced_idle_and_continous(strum, disabled) {
    strum_disable_beat_synced_idle_and_continous(kdmyEngine_obtain(strum), disabled)
}
function __js__strum_draw_sick_effect_apart(strum, enable) {
    strum_draw_sick_effect_apart(kdmyEngine_obtain(strum), enable)
}
function __js__strum_enable_background(strum, enable) {
    strum_enable_background(kdmyEngine_obtain(strum), enable)
}
function __js__strum_enable_sick_effect(strum, enable) {
    strum_enable_sick_effect(kdmyEngine_obtain(strum), enable)
}
function __js__strum_force_key_release(strum) {
    strum_force_key_release(kdmyEngine_obtain(strum))
}
function __js__strum_get_drawable(strum) {
    let ret = strum_get_drawable(kdmyEngine_obtain(strum));
    return kdmyEngine_obtain(ret)
}
function __js__strum_get_duration(strum) {
    let ret = strum_get_duration(kdmyEngine_obtain(strum));
    return ret
}
function __js__strum_get_marker_duration(strum) {
    let ret = strum_get_marker_duration(kdmyEngine_obtain(strum));
    return ret
}
function __js__strum_get_modifier(strum) {
    let ret = strum_get_modifier(kdmyEngine_obtain(strum));
    return kdmyEngine_obtain(ret)
}
function __js__strum_get_name(strum) {
    let ret = strum_get_name(kdmyEngine_obtain(strum));
    return kdmyEngine_stringToPtr(ret)
}
function __js__strum_get_press_state(strum) {
    let ret = strum_get_press_state(kdmyEngine_obtain(strum));
    return ret
}
function __js__strum_get_press_state_changes(strum) {
    let ret = strum_get_press_state_changes(kdmyEngine_obtain(strum));
    return ret
}
function __js__strum_reset(strum, scroll_speed, state_name) {
    strum_reset(kdmyEngine_obtain(strum), scroll_speed, kdmyEngine_ptrToString(state_name))
}
function __js__strum_set_alpha(strum, alpha) {
    strum_set_alpha(kdmyEngine_obtain(strum), alpha)
}
function __js__strum_set_alpha_background(strum, alpha) {
    let ret = strum_set_alpha_background(kdmyEngine_obtain(strum), alpha);
    return ret
}
function __js__strum_set_alpha_sick_effect(strum, alpha) {
    let ret = strum_set_alpha_sick_effect(kdmyEngine_obtain(strum), alpha);
    return ret
}
function __js__strum_set_bpm(strum, bpm) {
    strum_set_bpm(kdmyEngine_obtain(strum), bpm)
}
function __js__strum_set_draw_offset(strum, offset_milliseconds) {
    strum_set_draw_offset(kdmyEngine_obtain(strum), offset_milliseconds)
}
function __js__strum_set_extra_animation(strum, strum_script_target, strum_script_on, undo, animsprite) {
    strum_set_extra_animation(kdmyEngine_obtain(strum), strum_script_target, strum_script_on, undo, kdmyEngine_obtain(animsprite))
}
function __js__strum_set_extra_animation_continuous(strum, strum_script_target, animsprite) {
    strum_set_extra_animation_continuous(kdmyEngine_obtain(strum), strum_script_target, kdmyEngine_obtain(animsprite))
}
function __js__strum_set_keep_aspect_ratio_background(strum, enable) {
    strum_set_keep_aspect_ratio_background(kdmyEngine_obtain(strum), enable)
}
function __js__strum_set_marker_duration_multiplier(strum, multipler) {
    strum_set_marker_duration_multiplier(kdmyEngine_obtain(strum), multipler)
}
function __js__strum_set_note_tweenkeyframe(strum, tweenkeyframe) {
    strum_set_note_tweenkeyframe(kdmyEngine_obtain(strum), kdmyEngine_obtain(tweenkeyframe))
}
function __js__strum_set_player_id(strum, player_id) {
    strum_set_player_id(kdmyEngine_obtain(strum), player_id)
}
function __js__strum_set_scroll_direction(strum, direction) {
    strum_set_scroll_direction(kdmyEngine_obtain(strum), direction)
}
function __js__strum_set_scroll_speed(strum, speed) {
    strum_set_scroll_speed(kdmyEngine_obtain(strum), speed)
}
function __js__strum_set_sickeffect_size_ratio(strum, size_ratio) {
    strum_set_sickeffect_size_ratio(kdmyEngine_obtain(strum), size_ratio)
}
function __js__strum_set_visible(strum, visible) {
    strum_set_visible(kdmyEngine_obtain(strum), visible)
}
function __js__strum_state_add(strum, mdlhldr_mrkr, mdlhldr_sck_ffct, mdlhldr_bckgrnd, state_name) {
    strum_state_add(kdmyEngine_obtain(strum), kdmyEngine_obtain(mdlhldr_mrkr), kdmyEngine_obtain(mdlhldr_sck_ffct), kdmyEngine_obtain(mdlhldr_bckgrnd), kdmyEngine_ptrToString(state_name))
}
function __js__strum_state_toggle(strum, state_name) {
    let ret = strum_state_toggle(kdmyEngine_obtain(strum), kdmyEngine_ptrToString(state_name));
    return ret
}
function __js__strum_state_toggle_background(strum, state_name) {
    let ret = strum_state_toggle_background(kdmyEngine_obtain(strum), kdmyEngine_ptrToString(state_name));
    return ret ? 1 : 0
}
function __js__strum_state_toggle_marker(strum, state_name) {
    let ret = strum_state_toggle_marker(kdmyEngine_obtain(strum), kdmyEngine_ptrToString(state_name));
    return ret
}
function __js__strum_state_toggle_notes(strum, state_name) {
    let ret = strum_state_toggle_notes(kdmyEngine_obtain(strum), kdmyEngine_ptrToString(state_name));
    return ret
}
function __js__strum_state_toggle_sick_effect(strum, state_name) {
    let ret = strum_state_toggle_sick_effect(kdmyEngine_obtain(strum), kdmyEngine_ptrToString(state_name));
    return ret ? 1 : 0
}
function __js__strum_update_draw_location(strum, x, y) {
    strum_update_draw_location(kdmyEngine_obtain(strum), x, y)
}
function __js__strums_animation_end(strums) {
    strums_animation_end(kdmyEngine_obtain(strums))
}
function __js__strums_animation_restart(strums) {
    strums_animation_restart(kdmyEngine_obtain(strums))
}
function __js__strums_animation_set(strums, animsprite) {
    strums_animation_set(kdmyEngine_obtain(strums), kdmyEngine_obtain(animsprite))
}
function __js__strums_decorators_add(strums, modelholder, animation_name, timestamp) {
    let ret = strums_decorators_add(kdmyEngine_obtain(strums), kdmyEngine_obtain(modelholder), kdmyEngine_ptrToString(animation_name), timestamp);
    return ret ? 1 : 0
}
function __js__strums_decorators_add2(strums, modelholder, animation_name, timestamp, from_strum_index, to_strum_index) {
    let ret = strums_decorators_add2(kdmyEngine_obtain(strums), kdmyEngine_obtain(modelholder), kdmyEngine_ptrToString(animation_name), timestamp, from_strum_index, to_strum_index);
    return ret ? 1 : 0
}
function __js__strums_decorators_get_count(strums) {
    let ret = strums_decorators_get_count(kdmyEngine_obtain(strums));
    return ret
}
function __js__strums_decorators_set_alpha(strums, alpha) {
    strums_decorators_set_alpha(kdmyEngine_obtain(strums), alpha)
}
function __js__strums_decorators_set_scroll_speed(strums, speed) {
    strums_decorators_set_scroll_speed(kdmyEngine_obtain(strums), speed)
}
function __js__strums_decorators_set_visible(strums, decorator_timestamp, visible) {
    strums_decorators_set_visible(kdmyEngine_obtain(strums), decorator_timestamp, visible)
}
function __js__strums_disable_beat_synced_idle_and_continous(strums, disabled) {
    strums_disable_beat_synced_idle_and_continous(kdmyEngine_obtain(strums), disabled)
}
function __js__strums_enable_background(strums, enable) {
    strums_enable_background(kdmyEngine_obtain(strums), enable)
}
function __js__strums_enable_post_sick_effect_draw(strums, enable) {
    strums_enable_post_sick_effect_draw(kdmyEngine_obtain(strums), enable)
}
function __js__strums_force_key_release(strums) {
    strums_force_key_release(kdmyEngine_obtain(strums))
}
function __js__strums_get_drawable(strums) {
    let ret = strums_get_drawable(kdmyEngine_obtain(strums));
    return kdmyEngine_obtain(ret)
}
function __js__strums_get_lines_count(strums) {
    let ret = strums_get_lines_count(kdmyEngine_obtain(strums));
    return ret
}
function __js__strums_get_strum_line(strums, index) {
    let ret = strums_get_strum_line(kdmyEngine_obtain(strums), index);
    return kdmyEngine_obtain(ret)
}
function __js__strums_reset(strums, scroll_speed, state_name) {
    strums_reset(kdmyEngine_obtain(strums), scroll_speed, kdmyEngine_ptrToString(state_name))
}
function __js__strums_set_alpha(strums, alpha) {
    let ret = strums_set_alpha(kdmyEngine_obtain(strums), alpha);
    return ret
}
function __js__strums_set_alpha_background(strums, alpha) {
    strums_set_alpha_background(kdmyEngine_obtain(strums), alpha)
}
function __js__strums_set_alpha_sick_effect(strums, alpha) {
    strums_set_alpha_sick_effect(kdmyEngine_obtain(strums), alpha)
}
function __js__strums_set_bpm(strums, bpm) {
    strums_set_bpm(kdmyEngine_obtain(strums), bpm)
}
function __js__strums_set_draw_offset(strums, offset_milliseconds) {
    strums_set_draw_offset(kdmyEngine_obtain(strums), offset_milliseconds)
}
function __js__strums_set_keep_aspect_ratio_background(strums, enable) {
    strums_set_keep_aspect_ratio_background(kdmyEngine_obtain(strums), enable)
}
function __js__strums_set_marker_duration_multiplier(strums, multipler) {
    strums_set_marker_duration_multiplier(kdmyEngine_obtain(strums), multipler)
}
function __js__strums_set_scroll_direction(strums, direction) {
    strums_set_scroll_direction(kdmyEngine_obtain(strums), direction)
}
function __js__strums_set_scroll_speed(strums, speed) {
    strums_set_scroll_speed(kdmyEngine_obtain(strums), speed)
}
function __js__strums_state_add(strums, mdlhldr_mrkr, mdlhldr_sck_ffct, mdlhldr_bckgrnd, state_name) {
    strums_state_add(kdmyEngine_obtain(strums), kdmyEngine_obtain(mdlhldr_mrkr), kdmyEngine_obtain(mdlhldr_sck_ffct), kdmyEngine_obtain(mdlhldr_bckgrnd), kdmyEngine_ptrToString(state_name))
}
function __js__strums_state_toggle(strums, state_name) {
    let ret = strums_state_toggle(kdmyEngine_obtain(strums), kdmyEngine_ptrToString(state_name));
    return ret
}
function __js__strums_state_toggle_marker_and_sick_effect(strums, state_name) {
    strums_state_toggle_marker_and_sick_effect(kdmyEngine_obtain(strums), kdmyEngine_ptrToString(state_name))
}
function __js__strums_state_toggle_notes(strums, state_name) {
    let ret = strums_state_toggle_notes(kdmyEngine_obtain(strums), kdmyEngine_ptrToString(state_name));
    return ret
}
function __js__textsprite_background_enable(textsprite, enabled) {
    textsprite_background_enable(kdmyEngine_obtain(textsprite), enabled)
}
function __js__textsprite_background_set_color(textsprite, r, g, b, a) {
    textsprite_background_set_color(kdmyEngine_obtain(textsprite), r, g, b, a)
}
function __js__textsprite_background_set_offets(textsprite, offset_x, offset_y) {
    textsprite_background_set_offets(kdmyEngine_obtain(textsprite), offset_x, offset_y)
}
function __js__textsprite_background_set_size(textsprite, size) {
    textsprite_background_set_size(kdmyEngine_obtain(textsprite), size)
}
function __js__textsprite_blend_enable(textsprite, enabled) {
    textsprite_blend_enable(kdmyEngine_obtain(textsprite), enabled)
}
function __js__textsprite_blend_set(textsprite, src_rgb, dst_rgb, src_alpha, dst_alpha) {
    textsprite_blend_set(kdmyEngine_obtain(textsprite), src_rgb, dst_rgb, src_alpha, dst_alpha)
}
function __js__textsprite_border_enable(textsprite, enable) {
    textsprite_border_enable(kdmyEngine_obtain(textsprite), enable)
}
function __js__textsprite_border_set_color(textsprite, r, g, b, a) {
    textsprite_border_set_color(kdmyEngine_obtain(textsprite), r, g, b, a)
}
function __js__textsprite_border_set_offset(textsprite, x, y) {
    textsprite_border_set_offset(kdmyEngine_obtain(textsprite), x, y)
}
function __js__textsprite_border_set_size(textsprite, border_size) {
    textsprite_border_set_size(kdmyEngine_obtain(textsprite), border_size)
}
function __js__textsprite_force_case(textsprite, none_or_lowercase_or_uppercase) {
    textsprite_force_case(kdmyEngine_obtain(textsprite), none_or_lowercase_or_uppercase)
}
function __js__textsprite_get_draw_size(textsprite, draw_width, draw_height) {
    const values = [0, 0];
    textsprite_get_draw_size(kdmyEngine_obtain(textsprite), values);
    kdmyEngine_set_float32(draw_width, values[0]);
    kdmyEngine_set_float32(draw_height, values[1])
}
function __js__textsprite_get_font_size(textsprite) {
    return textsprite_get_font_size(kdmyEngine_obtain(textsprite))
}
function __js__textsprite_get_shader(textsprite) {
    let psshader = textsprite_get_shader(kdmyEngine_obtain(textsprite));
    return kdmyEngine_obtain(psshader)
}
function __js__textsprite_matrix_flip(textsprite, flip_x, flip_y) {
    textsprite_matrix_flip(kdmyEngine_obtain(textsprite), flip_x, flip_y)
}
function __js__textsprite_matrix_get_modifier(textsprite) {
    const modifier = textsprite_matrix_get_modifier(kdmyEngine_obtain(textsprite));
    return kdmyEngine_obtain(modifier)
}
function __js__textsprite_matrix_reset(textsprite) {
    textsprite_matrix_reset(kdmyEngine_obtain(textsprite))
}
function __js__textsprite_set_align(textsprite, align_vertical, align_horizontal) {
    textsprite_set_align(kdmyEngine_obtain(textsprite), align_vertical, align_horizontal)
}
function __js__textsprite_set_alpha(textsprite, alpha) {
    textsprite_set_alpha(kdmyEngine_obtain(textsprite), alpha)
}
function __js__textsprite_set_antialiasing(textsprite, antialiasing) {
    textsprite_set_antialiasing(kdmyEngine_obtain(textsprite), antialiasing)
}
function __js__textsprite_set_color(textsprite, r, g, b) {
    textsprite_set_color(kdmyEngine_obtain(textsprite), r, g, b)
}
function __js__textsprite_set_color_rgba8(textsprite, rbga8_color) {
    textsprite_set_color_rgba8(kdmyEngine_obtain(textsprite), rbga8_color)
}
function __js__textsprite_set_draw_location(textsprite, x, y) {
    textsprite_set_draw_location(kdmyEngine_obtain(textsprite), x, y)
}
function __js__textsprite_set_font_size(textsprite, font_size) {
    textsprite_set_font_size(kdmyEngine_obtain(textsprite), font_size)
}
function __js__textsprite_set_max_draw_size(textsprite, max_width, max_height) {
    textsprite_set_max_draw_size(kdmyEngine_obtain(textsprite), max_width, max_height)
}
function __js__textsprite_set_maxlines(textsprite, max_lines) {
    textsprite_set_maxlines(kdmyEngine_obtain(textsprite), max_lines)
}
function __js__textsprite_set_paragraph_align(textsprite, align) {
    textsprite_set_paragraph_align(kdmyEngine_obtain(textsprite), align)
}
function __js__textsprite_set_paragraph_space(textsprite, space) {
    textsprite_set_paragraph_space(kdmyEngine_obtain(textsprite), space)
}
function __js__textsprite_set_shader(textsprite, psshader) {
    textsprite_set_shader(kdmyEngine_obtain(textsprite), kdmyEngine_obtain(psshader))
}
function __js__textsprite_set_text_intern(textsprite, intern, text) {
    textsprite_set_text_intern(kdmyEngine_obtain(textsprite), intern, kdmyEngine_ptrToString(text))
}
function __js__textsprite_set_visible(textsprite, visible) {
    textsprite_set_visible(kdmyEngine_obtain(textsprite), visible)
}
function __js__textsprite_set_wordbreak(textsprite, wordbreak) {
    textsprite_set_wordbreak(kdmyEngine_obtain(textsprite), wordbreak)
}
function __js__textsprite_set_z_index(textsprite, z_index) {
    textsprite_set_z_index(kdmyEngine_obtain(textsprite), z_index)
}
function __js__textsprite_set_z_offset(textsprite, offset) {
    textsprite_set_z_offset(kdmyEngine_obtain(textsprite), offset)
}
function __js__timer_ms_gettime32_JS() {
    return Math.trunc(performance.now())
}
function __js__tweenkeyframe_add_cubic(tweenkeyframe, at, id, value) {
    let ret = tweenkeyframe_add_cubic(kdmyEngine_obtain(tweenkeyframe), at, id, value);
    return ret
}
function __js__tweenkeyframe_add_ease(tweenkeyframe, at, id, value) {
    let ret = tweenkeyframe_add_ease(kdmyEngine_obtain(tweenkeyframe), at, id, value);
    return ret
}
function __js__tweenkeyframe_add_easein(tweenkeyframe, at, id, value) {
    let ret = tweenkeyframe_add_easein(kdmyEngine_obtain(tweenkeyframe), at, id, value);
    return ret
}
function __js__tweenkeyframe_add_easeinout(tweenkeyframe, at, id, value) {
    let ret = tweenkeyframe_add_easeinout(kdmyEngine_obtain(tweenkeyframe), at, id, value);
    return ret
}
function __js__tweenkeyframe_add_easeout(tweenkeyframe, at, id, value) {
    let ret = tweenkeyframe_add_easeout(kdmyEngine_obtain(tweenkeyframe), at, id, value);
    return ret
}
function __js__tweenkeyframe_add_expo(tweenkeyframe, at, id, value) {
    let ret = tweenkeyframe_add_expo(kdmyEngine_obtain(tweenkeyframe), at, id, value);
    return ret
}
function __js__tweenkeyframe_add_interpolator(tweenkeyframde, at, id, value, type) {
    let ret = tweenkeyframe_add_interpolator(kdmyEngine_obtain(tweenkeyframde), at, id, value, type);
    return ret
}
function __js__tweenkeyframe_add_linear(tweenkeyframe, at, id, value) {
    let ret = tweenkeyframe_add_linear(kdmyEngine_obtain(tweenkeyframe), at, id, value);
    return ret
}
function __js__tweenkeyframe_add_quad(tweenkeyframe, at, id, value) {
    let ret = tweenkeyframe_add_quad(kdmyEngine_obtain(tweenkeyframe), at, id, value);
    return ret
}
function __js__tweenkeyframe_add_sin(tweenkeyframe, at, id, value) {
    let ret = tweenkeyframe_add_sin(kdmyEngine_obtain(tweenkeyframe), at, id, value);
    return ret
}
function __js__tweenkeyframe_add_steps(tweenkeyframe, at, id, value, steps_count, steps_method) {
    let ret = tweenkeyframe_add_steps(kdmyEngine_obtain(tweenkeyframe), at, id, value, steps_count, steps_method);
    return ret
}
function __js__tweenkeyframe_animate_percent(tweenkeyframe, percent) {
    tweenkeyframe_animate_percent(kdmyEngine_obtain(tweenkeyframe), percent)
}
function __js__tweenkeyframe_destroy(tweenkeyframe) {
    tweenkeyframe_destroy(kdmyEngine_obtain(kdmyEngine_get_uint32(tweenkeyframe)))
}
function __js__tweenkeyframe_get_ids_count(tweenkeyframe) {
    let ret = tweenkeyframe_get_ids_count(kdmyEngine_obtain(tweenkeyframe));
    return ret
}
function __js__tweenkeyframe_init() {
    let ret = tweenkeyframe_init();
    return kdmyEngine_obtain(ret)
}
function __js__tweenkeyframe_init2(animlist_item) {
    let ret = tweenkeyframe_init2(kdmyEngine_obtain(animlist_item));
    return kdmyEngine_obtain(ret)
}
function __js__tweenkeyframe_peek_entry_by_index(tweenkeyframe, index, out_id, out_value) {
    const values = [0, 0];
    let ret = tweenkeyframe_peek_entry_by_index(kdmyEngine_obtain(tweenkeyframe), index, values);
    kdmyEngine_set_int32(out_id, values[0]);
    kdmyEngine_set_float32(out_value, values[1]);
    return ret ? 1 : 0
}
function __js__tweenkeyframe_peek_value(tweenkeyframe) {
    let ret = tweenkeyframe_peek_value(kdmyEngine_obtain(tweenkeyframe));
    return ret
}
function __js__tweenkeyframe_peek_value_by_id(tweenkeyframe, id) {
    let ret = tweenkeyframe_peek_value_by_id(kdmyEngine_obtain(tweenkeyframe), id);
    return ret
}
function __js__tweenkeyframe_peek_value_by_index(tweenkeyframe, index) {
    let ret = tweenkeyframe_peek_value_by_index(kdmyEngine_obtain(tweenkeyframe), index);
    return ret
}
function __js__tweenlerp_add_cubic(tweenlerp, id, start, end, duration) {
    let ret = tweenlerp_add_cubic(kdmyEngine_obtain(tweenlerp), id, start, end, duration);
    return ret
}
function __js__tweenlerp_add_ease(tweenlerp, id, start, end, duration) {
    let ret = tweenlerp_add_ease(kdmyEngine_obtain(tweenlerp), id, start, end, duration);
    return ret
}
function __js__tweenlerp_add_easein(tweenlerp, id, start, end, duration) {
    let ret = tweenlerp_add_easein(kdmyEngine_obtain(tweenlerp), id, start, end, duration);
    return ret
}
function __js__tweenlerp_add_easeinout(tweenlerp, id, start, end, duration) {
    let ret = tweenlerp_add_easeinout(kdmyEngine_obtain(tweenlerp), id, start, end, duration);
    return ret
}
function __js__tweenlerp_add_easeout(tweenlerp, id, start, end, duration) {
    let ret = tweenlerp_add_easeout(kdmyEngine_obtain(tweenlerp), id, start, end, duration);
    return ret
}
function __js__tweenlerp_add_expo(tweenlerp, id, start, end, duration) {
    let ret = tweenlerp_add_expo(kdmyEngine_obtain(tweenlerp), id, start, end, duration);
    return ret
}
function __js__tweenlerp_add_interpolator(tweenlerp, id, start, end, duration, type) {
    let ret = tweenlerp_add_interpolator(kdmyEngine_obtain(tweenlerp), id, start, end, duration, type);
    return ret
}
function __js__tweenlerp_add_linear(tweenlerp, id, start, end, duration) {
    let ret = tweenlerp_add_linear(kdmyEngine_obtain(tweenlerp), id, start, end, duration);
    return ret
}
function __js__tweenlerp_add_quad(tweenlerp, id, start, end, duration) {
    let ret = tweenlerp_add_quad(kdmyEngine_obtain(tweenlerp), id, start, end, duration);
    return ret
}
function __js__tweenlerp_add_sin(tweenlerp, id, start, end, duration) {
    let ret = tweenlerp_add_sin(kdmyEngine_obtain(tweenlerp), id, start, end, duration);
    return ret
}
function __js__tweenlerp_add_steps(tweenlerp, id, start, end, duration, steps_count, steps_method) {
    let ret = tweenlerp_add_steps(kdmyEngine_obtain(tweenlerp), id, start, end, duration, steps_count, steps_method);
    return ret
}
function __js__tweenlerp_animate(tweenlerp, elapsed) {
    let ret = tweenlerp_animate(kdmyEngine_obtain(tweenlerp), elapsed);
    return ret
}
function __js__tweenlerp_animate_percent(tweenlerp, percent) {
    let ret = tweenlerp_animate_percent(kdmyEngine_obtain(tweenlerp), percent);
    return ret
}
function __js__tweenlerp_change_bounds_by_id(tweenlerp, id, new_start, new_end) {
    let ret = tweenlerp_change_bounds_by_id(kdmyEngine_obtain(tweenlerp), id, new_start, new_end);
    return ret ? 1 : 0
}
function __js__tweenlerp_change_bounds_by_index(tweenlerp, index, new_start, new_end) {
    let ret = tweenlerp_change_bounds_by_index(kdmyEngine_obtain(tweenlerp), index, new_start, new_end);
    return ret ? 1 : 0
}
function __js__tweenlerp_change_duration_by_index(tweenlerp, index, new_duration) {
    let ret = tweenlerp_change_duration_by_index(kdmyEngine_obtain(tweenlerp), index, new_duration);
    return ret ? 1 : 0
}
function __js__tweenlerp_destroy(tweenlerp) {
    tweenlerp_destroy(kdmyEngine_obtain(kdmyEngine_get_uint32(tweenlerp)))
}
function __js__tweenlerp_end(tweenlerp) {
    tweenlerp_end(kdmyEngine_obtain(tweenlerp))
}
function __js__tweenlerp_get_elapsed(tweenlerp) {
    let ret = tweenlerp_get_elapsed(kdmyEngine_obtain(tweenlerp));
    return ret
}
function __js__tweenlerp_get_entry_count(tweenlerp) {
    let ret = tweenlerp_get_entry_count(kdmyEngine_obtain(tweenlerp));
    return ret
}
function __js__tweenlerp_init() {
    let ret = tweenlerp_init();
    return kdmyEngine_obtain(ret)
}
function __js__tweenlerp_is_completed(tweenlerp) {
    let ret = tweenlerp_is_completed(kdmyEngine_obtain(tweenlerp));
    return ret ? 1 : 0
}
function __js__tweenlerp_mark_as_completed(tweenlerp) {
    tweenlerp_mark_as_completed(kdmyEngine_obtain(tweenlerp))
}
function __js__tweenlerp_override_start_with_end_by_index(tweenlerp, index) {
    let ret = tweenlerp_override_start_with_end_by_index(kdmyEngine_obtain(tweenlerp), index);
    return ret ? 1 : 0
}
function __js__tweenlerp_peek_entry_by_index(tweenlerp, index, out_id, out_value, out_duration) {
    const values = [0, 0, 0];
    let ret = tweenlerp_peek_entry_by_index(kdmyEngine_obtain(tweenlerp), index, values);
    kdmyEngine_set_int32(out_id, values[0]);
    kdmyEngine_set_float32(out_duration, values[1]);
    kdmyEngine_set_float32(out_duration, values[2]);
    return ret ? 1 : 0
}
function __js__tweenlerp_peek_value(tweenlerp) {
    let ret = tweenlerp_peek_value(kdmyEngine_obtain(tweenlerp));
    return ret
}
function __js__tweenlerp_peek_value_by_id(tweenlerp, id) {
    let ret = tweenlerp_peek_value_by_id(kdmyEngine_obtain(tweenlerp), id);
    return ret
}
function __js__tweenlerp_peek_value_by_index(tweenlerp, index) {
    let ret = tweenlerp_peek_value_by_index(kdmyEngine_obtain(tweenlerp), index);
    return ret
}
function __js__tweenlerp_restart(tweenlerp) {
    tweenlerp_restart(kdmyEngine_obtain(tweenlerp))
}
function __js__tweenlerp_swap_bounds_by_index(tweenlerp, index) {
    let ret = tweenlerp_swap_bounds_by_index(kdmyEngine_obtain(tweenlerp), index);
    return ret ? 1 : 0
}
function __js__videoplayer_fade_audio(videoplayer, in_or_out, duration) {
    videoplayer_fade_audio(kdmyEngine_obtain(videoplayer), in_or_out, duration)
}
function __js__videoplayer_get_duration(videoplayer) {
    let ret = videoplayer_get_duration(kdmyEngine_obtain(videoplayer));
    return ret
}
function __js__videoplayer_get_position(videoplayer) {
    let ret = videoplayer_get_position(kdmyEngine_obtain(videoplayer));
    return ret
}
function __js__videoplayer_get_sprite(videoplayer) {
    let ret = videoplayer_get_sprite(kdmyEngine_obtain(videoplayer));
    return kdmyEngine_obtain(ret)
}
function __js__videoplayer_has_audio_track(videoplayer) {
    let ret = videoplayer_has_audio_track(kdmyEngine_obtain(videoplayer));
    return ret ? 1 : 0
}
function __js__videoplayer_has_ended(videoplayer) {
    let ret = videoplayer_has_ended(kdmyEngine_obtain(videoplayer));
    return ret ? 1 : 0
}
function __js__videoplayer_has_video_track(videoplayer) {
    let ret = videoplayer_has_video_track(kdmyEngine_obtain(videoplayer));
    return ret ? 1 : 0
}
function __js__videoplayer_is_muted(videoplayer) {
    let ret = videoplayer_is_muted(kdmyEngine_obtain(videoplayer));
    return ret ? 1 : 0
}
function __js__videoplayer_is_playing(videoplayer) {
    let ret = videoplayer_is_playing(kdmyEngine_obtain(videoplayer));
    return ret ? 1 : 0
}
function __js__videoplayer_loop_enable(videoplayer, enable) {
    videoplayer_loop_enable(kdmyEngine_obtain(videoplayer), enable)
}
function __js__videoplayer_pause(videoplayer) {
    videoplayer_pause(kdmyEngine_obtain(videoplayer))
}
function __js__videoplayer_play(videoplayer) {
    videoplayer_play(kdmyEngine_obtain(videoplayer))
}
function __js__videoplayer_replay(videoplayer) {
    videoplayer_replay(kdmyEngine_obtain(videoplayer))
}
function __js__videoplayer_seek(videoplayer, timestamp) {
    videoplayer_seek(kdmyEngine_obtain(videoplayer), timestamp)
}
function __js__videoplayer_set_mute(videoplayer, muted) {
    videoplayer_set_mute(kdmyEngine_obtain(videoplayer), muted)
}
function __js__videoplayer_set_volume(videoplayer, volume) {
    videoplayer_set_volume(kdmyEngine_obtain(videoplayer), volume)
}
function __js__videoplayer_stop(videoplayer) {
    videoplayer_stop(kdmyEngine_obtain(videoplayer))
}
function __js__week_change_character_camera_name(roundcontext, opponent_or_player, new_name) {
    week_change_character_camera_name(kdmyEngine_obtain(roundcontext), opponent_or_player, kdmyEngine_ptrToString(new_name))
}
function __js__week_disable_ask_ready(roundcontext, disable) {
    week_disable_ask_ready(kdmyEngine_obtain(roundcontext), disable)
}
function __js__week_disable_camera_bumping(roundcontext, disable) {
    week_disable_camera_bumping(kdmyEngine_obtain(roundcontext), disable)
}
function __js__week_disable_countdown(roundcontext, disable) {
    week_disable_countdown(kdmyEngine_obtain(roundcontext), disable)
}
function __js__week_disable_girlfriend_cry(roundcontext, disable) {
    week_disable_girlfriend_cry(kdmyEngine_obtain(roundcontext), disable)
}
function __js__week_disable_layout_rollback(roundcontext, disable) {
    week_disable_layout_rollback(kdmyEngine_obtain(roundcontext), disable)
}
function __js__week_disable_week_end_results(roundcontext, disable) {
    week_disable_week_end_results(kdmyEngine_obtain(roundcontext), disable)
}
function __js__week_enable_credits_on_completed(roundcontext) {
    week_enable_credits_on_completed(kdmyEngine_obtain(roundcontext))
}
function __js__week_end(roundcontext, round_or_week, loose_or_win) {
    week_end(kdmyEngine_obtain(roundcontext), round_or_week, loose_or_win)
}
function __js__week_get_character(roundcontext, character_index) {
    let ret = week_get_character(kdmyEngine_obtain(roundcontext), character_index);
    return kdmyEngine_obtain(ret)
}
function __js__week_get_character_count(roundcontext) {
    let ret = week_get_character_count(kdmyEngine_obtain(roundcontext));
    return ret
}
function __js__week_get_conductor(roundcontext, character_index) {
    let ret = week_get_conductor(kdmyEngine_obtain(roundcontext), character_index);
    return kdmyEngine_obtain(ret)
}
function __js__week_get_current_chart_info(roundcontext, bpm, speed) {
    const values = {
        bpm: null,
        speed: null
    };
    week_get_current_chart_info(kdmyEngine_obtain(roundcontext), values);
    kdmyEngine_set_float32(bpm, values.bpm);
    kdmyEngine_set_float64(speed, values.speed)
}
function __js__week_get_current_song_info(roundcontext, name, difficult, index) {
    const values = {
        name: null,
        difficult: null,
        index: -1
    };
    week_get_current_song_info(kdmyEngine_obtain(roundcontext), values);
    kdmyEngine_set_uint32(name, kdmyEngine_stringToPtr(values.name));
    kdmyEngine_set_uint32(difficult, kdmyEngine_stringToPtr(values.difficult));
    kdmyEngine_set_int32(index, values.index)
}
function __js__week_get_dialogue(roundcontext) {
    let ret = week_get_dialogue(kdmyEngine_obtain(roundcontext));
    return kdmyEngine_obtain(ret)
}
function __js__week_get_girlfriend(roundcontext) {
    let ret = week_get_girlfriend(kdmyEngine_obtain(roundcontext));
    return kdmyEngine_obtain(ret)
}
function __js__week_get_healthwatcher(roundcontext) {
    let ret = week_get_healthwatcher(kdmyEngine_obtain(roundcontext));
    return kdmyEngine_obtain(ret)
}
function __js__week_get_messagebox(roundcontext) {
    let ret = week_get_messagebox(kdmyEngine_obtain(roundcontext));
    return kdmyEngine_obtain(ret)
}
function __js__week_get_missnotefx(roundcontext) {
    let ret = week_get_missnotefx(kdmyEngine_obtain(roundcontext));
    return kdmyEngine_obtain(ret)
}
function __js__week_get_playerstats(roundcontext, character_index) {
    let ret = week_get_playerstats(kdmyEngine_obtain(roundcontext), character_index);
    return kdmyEngine_obtain(ret)
}
function __js__week_get_songplayer(roundcontext) {
    let ret = week_get_songplayer(kdmyEngine_obtain(roundcontext));
    return kdmyEngine_obtain(ret)
}
function __js__week_get_stage_layout(roundcontext) {
    let ret = week_get_stage_layout(kdmyEngine_obtain(roundcontext));
    return kdmyEngine_obtain(ret)
}
function __js__week_override_common_folder(roundcontext, custom_common_path) {
    week_override_common_folder(kdmyEngine_obtain(roundcontext), kdmyEngine_ptrToString(custom_common_path))
}
function __js__week_set_halt(roundcontext, halt) {
    week_set_halt(kdmyEngine_obtain(roundcontext), halt)
}
function __js__week_set_ui_shader(roundcontext, psshader) {
    week_set_ui_shader(kdmyEngine_obtain(roundcontext), kdmyEngine_obtain(psshader))
}
function __js__week_storage_get(roundcontext, name, data) {
    let arraybuffer = [null];
    let ret = week_storage_get(kdmyEngine_obtain(roundcontext), kdmyEngine_ptrToString(name), arraybuffer);
    let ptr;
    if (arraybuffer[0]) {
        ptr = _malloc(ret);
        if (ptr != 0)
            HEAPU8.set(new Uint8Array(arraybuffer[0]), ptr)
    } else {
        ptr = 0
    }
    kdmyEngine_set_uint32(data, ptr);
    return ret
}
function __js__week_storage_set(roundcontext, name, data, data_size) {
    let arraybuffer = data == 0 ? null : new ArrayBuffer(data_size);
    if (arraybuffer) {
        new Uint8Array(arraybuffer).set(HEAPU8.subarray(data, data + data_size), 0)
    }
    let ret = week_storage_set(kdmyEngine_obtain(roundcontext), kdmyEngine_ptrToString(name), arraybuffer, data_size);
    return ret ? 1 : 0
}
function __js__week_ui_get_camera(roundcontext) {
    let ret = week_ui_get_camera(kdmyEngine_obtain(roundcontext));
    return kdmyEngine_obtain(ret)
}
function __js__week_ui_get_countdown(roundcontext) {
    let ret = week_ui_get_countdown(kdmyEngine_obtain(roundcontext));
    return kdmyEngine_obtain(ret)
}
function __js__week_ui_get_healthbar(roundcontext) {
    let ret = week_ui_get_healthbar(kdmyEngine_obtain(roundcontext));
    return kdmyEngine_obtain(ret)
}
function __js__week_ui_get_layout(roundcontext) {
    let ret = week_ui_get_layout(kdmyEngine_obtain(roundcontext));
    return kdmyEngine_obtain(ret)
}
function __js__week_ui_get_rankingcounter(roundcontext) {
    let ret = week_ui_get_rankingcounter(kdmyEngine_obtain(roundcontext));
    return kdmyEngine_obtain(ret)
}
function __js__week_ui_get_round_textsprite(roundcontext) {
    let ret = week_ui_get_round_textsprite(kdmyEngine_obtain(roundcontext));
    return kdmyEngine_obtain(ret)
}
function __js__week_ui_get_roundstats(roundcontext) {
    let ret = week_ui_get_roundstats(kdmyEngine_obtain(roundcontext));
    return kdmyEngine_obtain(ret)
}
function __js__week_ui_get_songprogressbar(roundcontext) {
    let ret = week_ui_get_songprogressbar(kdmyEngine_obtain(roundcontext));
    return kdmyEngine_obtain(ret)
}
function __js__week_ui_get_streakcounter(roundcontext) {
    let ret = week_ui_get_streakcounter(kdmyEngine_obtain(roundcontext));
    return kdmyEngine_obtain(ret)
}
function __js__week_ui_get_strums(roundcontext, strums_id) {
    let ret = week_ui_get_strums(kdmyEngine_obtain(roundcontext), strums_id);
    return kdmyEngine_obtain(ret)
}
function __js__week_ui_get_strums_count(roundcontext) {
    let ret = week_ui_get_strums_count(kdmyEngine_obtain(roundcontext));
    return ret
}
function __js__week_ui_set_visibility(roundcontext, visible) {
    week_ui_set_visibility(kdmyEngine_obtain(roundcontext), visible)
}
function __js__week_unlockdirective_create(roundcontext, name, completed_round, completed_week, value) {
    week_unlockdirective_create(kdmyEngine_obtain(roundcontext), kdmyEngine_ptrToString(name), completed_round, completed_week, value)
}
function __js__week_unlockdirective_get(roundcontext, name) {
    let ret = week_unlockdirective_get(kdmyEngine_obtain(roundcontext), kdmyEngine_ptrToString(name));
    return ret
}
function __js__week_unlockdirective_has(roundcontext, name) {
    let ret = week_unlockdirective_has(kdmyEngine_obtain(roundcontext), kdmyEngine_ptrToString(name));
    return ret ? 1 : 0
}
function __js__week_unlockdirective_remove(roundcontext, name, completed_round, completed_week) {
    week_unlockdirective_remove(kdmyEngine_obtain(roundcontext), kdmyEngine_ptrToString(name), completed_round, completed_week)
}
function __js__week_update_bpm(roundcontext, bpm) {
    week_update_bpm(kdmyEngine_obtain(roundcontext), bpm)
}
function __js__week_update_speed(roundcontext, speed) {
    week_update_speed(kdmyEngine_obtain(roundcontext), speed)
}



function callRuntimeCallbacks(callbacks) {
    while (callbacks.length > 0) {
        var callback = callbacks.shift();
        if (typeof callback == "function") {
            callback(ModuleLuaScript);
            continue
        }
        var func = callback.func;
        if (typeof func == "number") {
            if (callback.arg === undefined) {
                (function () {})()
            } else {
                (function (a1) {
                    dynCall_vi.apply(null, [func, a1])
                })(callback.arg)
            }
        } else {
            func(callback.arg === undefined ? null : callback.arg)
        }
    }
}
function demangle(func) {
    return func
}
function demangleAll(text) {
    var regex = /\b_Z[\w\d_]+/g;
    return text.replace(regex, function (x) {
        var y = demangle(x);
        return x === y ? x : y + " [" + x + "]"
    })
}
var wasmTableMirror = [];
function getWasmTableEntry(funcPtr) {
    var func = wasmTableMirror[funcPtr];
    if (!func) {
        if (funcPtr >= wasmTableMirror.length)
            wasmTableMirror.length = funcPtr + 1;
        wasmTableMirror[funcPtr] = func = wasmTable.get(funcPtr)
    }
    return func
}
function handleException(e) {
    if (e instanceof ExitStatus || e == "unwind") {
        return EXITSTATUS
    }
    quit_(1, e)
}
function jsStackTrace() {
    var error = new Error;
    if (!error.stack) {
        try {
            throw new Error
        } catch (e) {
            error = e
        }
        if (!error.stack) {
            return "(no stack trace available)"
        }
    }
    return error.stack.toString()
}
function setWasmTableEntry(idx, func) {
    wasmTable.set(idx, func);
    wasmTableMirror[idx] = wasmTable.get(idx)
}
function ___assert_fail(condition, filename, line, func) {
    abort("Assertion failed: " + UTF8ToString(condition) + ", at: " + [filename ? UTF8ToString(filename) : "unknown filename", line, func ? UTF8ToString(func) : "unknown function"])
}
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
    join: function () {
        var paths = Array.prototype.slice.call(arguments, 0);
        return PATH.normalize(paths.join("/"))
    },
    join2: (l, r) => {
        return PATH.normalize(l + "/" + r)
    }
};
function getRandomDevice() {
    if (typeof crypto == "object" && typeof crypto["getRandomValues"] == "function") {
        var randomBuffer = new Uint8Array(1);
        return function () {
            crypto.getRandomValues(randomBuffer);
            return randomBuffer[0]
        }
    } else if (ENVIRONMENT_IS_NODE) {
        try {
            var crypto_module = require("crypto");
            return function () {
                return crypto_module["randomBytes"](1)[0]
            }
        } catch (e) {}
    }
    return function () {
        abort("randomDevice")
    }
}
var PATH_FS = {
    resolve: function () {
        var resolvedPath = "",
        resolvedAbsolute = false;
        for (var i = arguments.length - 1; i >= -1 && !resolvedAbsolute; i--) {
            var path = i >= 0 ? arguments[i] : FS.cwd();
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
var TTY = {
    ttys: [],
    init: function () {},
    shutdown: function () {},
    register: function (dev, ops) {
        TTY.ttys[dev] = {
            input: [],
            output: [],
            ops: ops
        };
        FS.registerDevice(dev, TTY.stream_ops)
    },
    stream_ops: {
        open: function (stream) {
            var tty = TTY.ttys[stream.node.rdev];
            if (!tty) {
                throw new FS.ErrnoError(43)
            }
            stream.tty = tty;
            stream.seekable = false
        },
        close: function (stream) {
            stream.tty.ops.flush(stream.tty)
        },
        flush: function (stream) {
            stream.tty.ops.flush(stream.tty)
        },
        read: function (stream, buffer, offset, length, pos) {
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
        write: function (stream, buffer, offset, length, pos) {
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
        get_char: function (tty) {
            if (!tty.input.length) {
                var result = null;
                if (ENVIRONMENT_IS_NODE) {
                    var BUFSIZE = 256;
                    var buf = Buffer.alloc(BUFSIZE);
                    var bytesRead = 0;
                    try {
                        bytesRead = fs.readSync(process.stdin.fd, buf, 0, BUFSIZE, -1)
                    } catch (e) {
                        if (e.toString().includes("EOF"))
                            bytesRead = 0;
                        else
                            throw e
                    }
                    if (bytesRead > 0) {
                        result = buf.slice(0, bytesRead).toString("utf-8")
                    } else {
                        result = null
                    }
                } else if (typeof window != "undefined" && typeof window.prompt == "function") {
                    result = window.prompt("Input: ");
                    if (result !== null) {
                        result += "\n"
                    }// @ts-ignore
                } else if (typeof readline == "function") {// @ts-ignore
                    result = readline();
                    if (result !== null) {
                        result += "\n"
                    }
                }
                if (!result) {
                    return null
                }
                tty.input = intArrayFromString(result, true)
            }
            return tty.input.shift()
        },
        put_char: function (tty, val) {
            if (val === null || val === 10) {
                out(UTF8ArrayToString(tty.output, 0));
                tty.output = []
            } else {
                if (val != 0)
                    tty.output.push(val)
            }
        },
        flush: function (tty) {
            if (tty.output && tty.output.length > 0) {
                out(UTF8ArrayToString(tty.output, 0));
                tty.output = []
            }
        }
    },
    default_tty1_ops: {
        put_char: function (tty, val) {
            if (val === null || val === 10) {
                err(UTF8ArrayToString(tty.output, 0));
                tty.output = []
            } else {
                if (val != 0)
                    tty.output.push(val)
            }
        },
        flush: function (tty) {
            if (tty.output && tty.output.length > 0) {
                err(UTF8ArrayToString(tty.output, 0));
                tty.output = []
            }
        }
    }
};
function mmapAlloc(size) {
    abort()
}
var MEMFS = {
    ops_table: null,
    mount: function (mount) {
        return MEMFS.createNode(null, "/", 16384 | 511, 0)
    },
    createNode: function (parent, name, mode, dev) {
        if (FS.isBlkdev(mode) || FS.isFIFO(mode)) {
            throw new FS.ErrnoError(63)
        }
        if (!MEMFS.ops_table) {
            MEMFS.ops_table = {
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
            }
        }
        var node = FS.createNode(parent, name, mode, dev);
        if (FS.isDir(node.mode)) {
            node.node_ops = MEMFS.ops_table.dir.node;
            node.stream_ops = MEMFS.ops_table.dir.stream;
            node.contents = {}
        } else if (FS.isFile(node.mode)) {
            node.node_ops = MEMFS.ops_table.file.node;
            node.stream_ops = MEMFS.ops_table.file.stream;
            node.usedBytes = 0;
            node.contents = null
        } else if (FS.isLink(node.mode)) {
            node.node_ops = MEMFS.ops_table.link.node;
            node.stream_ops = MEMFS.ops_table.link.stream
        } else if (FS.isChrdev(node.mode)) {
            node.node_ops = MEMFS.ops_table.chrdev.node;
            node.stream_ops = MEMFS.ops_table.chrdev.stream
        }
        node.timestamp = Date.now();
        if (parent) {
            parent.contents[name] = node;
            parent.timestamp = node.timestamp
        }
        return node
    },
    getFileDataAsTypedArray: function (node) {
        if (!node.contents)
            return new Uint8Array(0);
        if (node.contents.subarray)
            return node.contents.subarray(0, node.usedBytes);
        return new Uint8Array(node.contents)
    },
    expandFileStorage: function (node, newCapacity) {
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
    resizeFileStorage: function (node, newSize) {
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
        getattr: function (node) {
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
        setattr: function (node, attr) {
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
        lookup: function (parent, name) {
            throw FS.genericErrors[44]
        },
        mknod: function (parent, name, mode, dev) {
            return MEMFS.createNode(parent, name, mode, dev)
        },
        rename: function (old_node, new_dir, new_name) {
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
        unlink: function (parent, name) {
            delete parent.contents[name];
            parent.timestamp = Date.now()
        },
        rmdir: function (parent, name) {
            var node = FS.lookupNode(parent, name);
            for (var i in node.contents) {
                throw new FS.ErrnoError(55)
            }
            delete parent.contents[name];
            parent.timestamp = Date.now()
        },
        readdir: function (node) {
            var entries = [".", ".."];
            for (var key in node.contents) {
                if (!node.contents.hasOwnProperty(key)) {
                    continue
                }
                entries.push(key)
            }
            return entries
        },
        symlink: function (parent, newname, oldpath) {
            var node = MEMFS.createNode(parent, newname, 511 | 40960, 0);
            node.link = oldpath;
            return node
        },
        readlink: function (node) {
            if (!FS.isLink(node.mode)) {
                throw new FS.ErrnoError(28)
            }
            return node.link
        }
    },
    stream_ops: {
        read: function (stream, buffer, offset, length, position) {
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
        write: function (stream, buffer, offset, length, position, canOwn) {
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
        llseek: function (stream, offset, whence) {
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
        allocate: function (stream, offset, length) {
            MEMFS.expandFileStorage(stream.node, offset + length);
            stream.node.usedBytes = Math.max(stream.node.usedBytes, offset + length)
        },
        mmap: function (stream, length, position, prot, flags) {
            if (!FS.isFile(stream.node.mode)) {
                throw new FS.ErrnoError(43)
            }
            var ptr;
            var allocated;
            var contents = stream.node.contents;
            if (!(flags & 2) && contents.buffer === buffer) {
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
                ptr = mmapAlloc(length);// @ts-ignore
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
        msync: function (stream, buffer, offset, length, mmapFlags) {
            if (!FS.isFile(stream.node.mode)) {
                throw new FS.ErrnoError(43)
            }
            if (mmapFlags & 2) {
                return 0
            }
            var bytesWritten = MEMFS.stream_ops.write(stream, buffer, 0, length, offset, false);
            return 0
        }
    }
};
function asyncLoad(url, onload, onerror, noRunDep) {
    var dep = !noRunDep ? getUniqueRunDependency("al " + url) : "";
    readAsync(url, function (arrayBuffer) {
        assert(arrayBuffer, 'Loading data file "' + url + '" failed (no arrayBuffer).');
        onload(new Uint8Array(arrayBuffer));
        if (dep)
            removeRunDependency(dep)
    }, function (event) {
        if (onerror) {
            onerror()
        } else {
            throw 'Loading data file "' + url + '" failed.'
        }
    });
    if (dep)
        addRunDependency(dep)
}
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
    ErrnoError: null,
    genericErrors: {},
    filesystems: null,
    syncFSRequests: 0,
    lookupPath: (path, opts = {}) => {
        path = PATH_FS.resolve(FS.cwd(), path);
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
        var parts = PATH.normalizeArray(path.split("/").filter(p => !!p), false);
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
    getPath: node => {
        var path;
        while (true) {
            if (FS.isRoot(node)) {
                var mount = node.mount.mountpoint;
                if (!path)
                    return mount;
                return mount[mount.length - 1] !== "/" ? mount + "/" + path : mount + path
            }
            path = path ? node.name + "/" + path : node.name;
            node = node.parent
        }
    },
    hashName: (parentid, name) => {
        var hash = 0;
        for (var i = 0; i < name.length; i++) {
            hash = (hash << 5) - hash + name.charCodeAt(i) | 0
        }
        return (parentid + hash >>> 0) % FS.nameTable.length
    },
    hashAddNode: node => {
        var hash = FS.hashName(node.parent.id, node.name);
        node.name_next = FS.nameTable[hash];
        FS.nameTable[hash] = node
    },
    hashRemoveNode: node => {
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
    lookupNode: (parent, name) => {
        var errCode = FS.mayLookup(parent);
        if (errCode) {
            throw new FS.ErrnoError(errCode, parent)
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
    createNode: (parent, name, mode, rdev) => {
        var node = new FS.FSNode(parent, name, mode, rdev);
        FS.hashAddNode(node);
        return node
    },
    destroyNode: node => {
        FS.hashRemoveNode(node)
    },
    isRoot: node => {
        return node === node.parent
    },
    isMountpoint: node => {
        return !!node.mounted
    },
    isFile: mode => {
        return (mode & 61440) === 32768
    },
    isDir: mode => {
        return (mode & 61440) === 16384
    },
    isLink: mode => {
        return (mode & 61440) === 40960
    },
    isChrdev: mode => {
        return (mode & 61440) === 8192
    },
    isBlkdev: mode => {
        return (mode & 61440) === 24576
    },
    isFIFO: mode => {
        return (mode & 61440) === 4096
    },
    isSocket: mode => {
        return (mode & 49152) === 49152
    },
    flagModes: {
        "r": 0,
        "r+": 2,
        "w": 577,
        "w+": 578,
        "a": 1089,
        "a+": 1090
    },
    modeStringToFlags: str => {
        var flags = FS.flagModes[str];
        if (typeof flags == "undefined") {
            throw new Error("Unknown file open mode: " + str)
        }
        return flags
    },
    flagsToPermissionString: flag => {
        var perms = ["r", "w", "rw"][flag & 3];
        if (flag & 512) {
            perms += "w"
        }
        return perms
    },
    nodePermissions: (node, perms) => {
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
    mayLookup: dir => {
        var errCode = FS.nodePermissions(dir, "x");
        if (errCode)
            return errCode;
        if (!dir.node_ops.lookup)
            return 2;
        return 0
    },
    mayCreate: (dir, name) => {
        try {
            var node = FS.lookupNode(dir, name);
            return 20
        } catch (e) {}
        return FS.nodePermissions(dir, "wx")
    },
    mayDelete: (dir, name, isdir) => {
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
    mayOpen: (node, flags) => {
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
    nextfd: (fd_start = 0, fd_end = FS.MAX_OPEN_FDS) => {
        for (var fd = fd_start; fd <= fd_end; fd++) {
            if (!FS.streams[fd]) {
                return fd
            }
        }
        throw new FS.ErrnoError(33)
    },
    getStream: fd => FS.streams[fd],
    createStream: (stream, fd_start, fd_end) => {
        if (!FS.FSStream) {
            FS.FSStream = function () {
                this.shared = {}
            };
            FS.FSStream.prototype = {
                object: {
                    get: function () {
                        return this.node
                    },
                    set: function (val) {
                        this.node = val
                    }
                },
                isRead: {
                    get: function () {
                        return (this.flags & 2097155) !== 1
                    }
                },
                isWrite: {
                    get: function () {
                        return (this.flags & 2097155) !== 0
                    }
                },
                isAppend: {
                    get: function () {
                        return this.flags & 1024
                    }
                },
                flags: {
                    get: function () {
                        return this.shared.flags
                    },
                    set: function (val) {
                        this.shared.flags = val
                    }
                },
                position: {
                    get function () {
                        return this.shared.position
                    },
                    set: function (val) {
                        this.shared.position = val
                    }
                }
            }
        }
        stream = Object.assign(new FS.FSStream, stream);
        var fd = FS.nextfd(fd_start, fd_end);
        stream.fd = fd;
        FS.streams[fd] = stream;
        return stream
    },
    closeStream: fd => {
        FS.streams[fd] = null
    },
    chrdev_stream_ops: {
        open: stream => {
            var device = FS.getDevice(stream.node.rdev);
            stream.stream_ops = device.stream_ops;
            if (stream.stream_ops.open) {
                stream.stream_ops.open(stream)
            }
        },
        llseek: () => {
            throw new FS.ErrnoError(70)
        }
    },
    major: dev => dev >> 8,
    minor: dev => dev & 255,
    makedev: (ma, mi) => ma << 8 | mi,
    registerDevice: (dev, ops) => {
        FS.devices[dev] = {
            stream_ops: ops
        }
    },
    getDevice: dev => FS.devices[dev],
    getMounts: mount => {
        var mounts = [];
        var check = [mount];
        while (check.length) {
            var m = check.pop();
            mounts.push(m);
            check.push.apply(check, m.mounts)
        }
        return mounts
    },
    syncfs: (populate, callback) => {
        if (typeof populate == "function") {
            callback = populate;
            populate = false
        }
        FS.syncFSRequests++;
        if (FS.syncFSRequests > 1) {
            err("warning: " + FS.syncFSRequests + " FS.syncfs operations in flight at once, probably just doing extra work")
        }
        var mounts = FS.getMounts(FS.root.mount);
        var completed = 0;
        function doCallback(errCode) {
            FS.syncFSRequests--;
            return callback(errCode)
        }
        function done(errCode) {
            if (errCode) {// @ts-ignore
                if (!done.errored) {// @ts-ignore
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
    mount: (type, opts, mountpoint) => {
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
    unmount: mountpoint => {
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
    lookup: (parent, name) => {
        return parent.node_ops.lookup(parent, name)
    },
    mknod: (path, mode, dev) => {
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
    create: (path, mode) => {
        mode = mode !== undefined ? mode : 438;
        mode &= 4095;
        mode |= 32768;
        return FS.mknod(path, mode, 0)
    },
    mkdir: (path, mode) => {
        mode = mode !== undefined ? mode : 511;
        mode &= 511 | 512;
        mode |= 16384;
        return FS.mknod(path, mode, 0)
    },
    mkdirTree: (path, mode) => {
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
    mkdev: (path, mode, dev) => {
        if (typeof dev == "undefined") {
            dev = mode;
            mode = 438
        }
        mode |= 8192;
        return FS.mknod(path, mode, dev)
    },
    symlink: (oldpath, newpath) => {
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
    rename: (old_path, new_path) => {
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
    rmdir: path => {
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
    readdir: path => {
        var lookup = FS.lookupPath(path, {
            follow: true
        });
        var node = lookup.node;
        if (!node.node_ops.readdir) {
            throw new FS.ErrnoError(54)
        }
        return node.node_ops.readdir(node)
    },
    unlink: path => {
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
    readlink: path => {
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
    stat: (path, dontFollow) => {
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
    lstat: path => {
        return FS.stat(path, true)
    },
    chmod: (path, mode, dontFollow) => {
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
    lchmod: (path, mode) => {
        FS.chmod(path, mode, true)
    },
    fchmod: (fd, mode) => {
        var stream = FS.getStream(fd);
        if (!stream) {
            throw new FS.ErrnoError(8)
        }
        FS.chmod(stream.node, mode)
    },
    chown: (path, uid, gid, dontFollow) => {
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
    lchown: (path, uid, gid) => {
        FS.chown(path, uid, gid, true)
    },
    fchown: (fd, uid, gid) => {
        var stream = FS.getStream(fd);
        if (!stream) {
            throw new FS.ErrnoError(8)
        }
        FS.chown(stream.node, uid, gid)
    },
    truncate: (path, len) => {
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
    ftruncate: (fd, len) => {
        var stream = FS.getStream(fd);
        if (!stream) {
            throw new FS.ErrnoError(8)
        }
        if ((stream.flags & 2097155) === 0) {
            throw new FS.ErrnoError(28)
        }
        FS.truncate(stream.node, len)
    },
    utime: (path, atime, mtime) => {
        var lookup = FS.lookupPath(path, {
            follow: true
        });
        var node = lookup.node;
        node.node_ops.setattr(node, {
            timestamp: Math.max(atime, mtime)
        })
    },
    open: (path, flags, mode) => {
        if (path === "") {
            throw new FS.ErrnoError(44)
        }
        flags = typeof flags == "string" ? FS.modeStringToFlags(flags) : flags;
        mode = typeof mode == "undefined" ? 438 : mode;
        if (flags & 64) {
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
        if (ModuleLuaScript["logReadFiles"] && !(flags & 1)) {
            if (!FS.readFiles)
                FS.readFiles = {};
            if (!(path in FS.readFiles)) {
                FS.readFiles[path] = 1
            }
        }
        return stream
    },
    close: stream => {
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
    isClosed: stream => {
        return stream.fd === null
    },
    llseek: (stream, offset, whence) => {
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
    read: (stream, buffer, offset, length, position) => {
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
    write: (stream, buffer, offset, length, position, canOwn) => {
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
    allocate: (stream, offset, length) => {
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
    mmap: (stream, length, position, prot, flags) => {
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
    msync: (stream, buffer, offset, length, mmapFlags) => {
        if (!stream || !stream.stream_ops.msync) {
            return 0
        }
        return stream.stream_ops.msync(stream, buffer, offset, length, mmapFlags)
    },
    munmap: stream => 0,
    ioctl: (stream, cmd, arg) => {
        if (!stream.stream_ops.ioctl) {
            throw new FS.ErrnoError(59)
        }
        return stream.stream_ops.ioctl(stream, cmd, arg)
    },
    readFile: (path, opts = {}) => {
        opts.flags = opts.flags || 0;
        opts.encoding = opts.encoding || "binary";
        if (opts.encoding !== "utf8" && opts.encoding !== "binary") {
            throw new Error('Invalid encoding type "' + opts.encoding + '"')
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
    writeFile: (path, data, opts = {}) => {
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
    chdir: path => {
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
    createDefaultDirectories: () => {
        FS.mkdir("/tmp");
        FS.mkdir("/home");
        FS.mkdir("/home/web_user")
    },
    createDefaultDevices: () => {
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
        var random_device = getRandomDevice();
        FS.createDevice("/dev", "random", random_device);
        FS.createDevice("/dev", "urandom", random_device);
        FS.mkdir("/dev/shm");
        FS.mkdir("/dev/shm/tmp")
    },
    createSpecialDirectories: () => {
        FS.mkdir("/proc");
        var proc_self = FS.mkdir("/proc/self");
        FS.mkdir("/proc/self/fd");
        FS.mount({
            mount: () => {
                var node = FS.createNode(proc_self, "fd", 16384 | 511, 73);
                node.node_ops = {
                    lookup: (parent, name) => {
                        var fd = +name;
                        var stream = FS.getStream(fd);
                        if (!stream)
                            throw new FS.ErrnoError(8);
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
    createStandardStreams: () => {
        if (ModuleLuaScript["stdin"]) {
            FS.createDevice("/dev", "stdin", ModuleLuaScript["stdin"])
        } else {
            FS.symlink("/dev/tty", "/dev/stdin")
        }
        if (ModuleLuaScript["stdout"]) {
            FS.createDevice("/dev", "stdout", null, ModuleLuaScript["stdout"])
        } else {
            FS.symlink("/dev/tty", "/dev/stdout")
        }
        if (ModuleLuaScript["stderr"]) {
            FS.createDevice("/dev", "stderr", null, ModuleLuaScript["stderr"])
        } else {
            FS.symlink("/dev/tty1", "/dev/stderr")
        }
        var stdin = FS.open("/dev/stdin", 0);
        var stdout = FS.open("/dev/stdout", 1);
        var stderr = FS.open("/dev/stderr", 1)
    },
    ensureErrnoError: () => {
        if (FS.ErrnoError)
            return;
        FS.ErrnoError = function ErrnoError(errno, node) {
            this.node = node;
            this.setErrno = function (errno) {
                this.errno = errno
            };
            this.setErrno(errno);
            this.message = "FS error"
        };
        FS.ErrnoError.prototype = new Error;
        FS.ErrnoError.prototype.constructor = FS.ErrnoError;
        [44].forEach(code => {
            FS.genericErrors[code] = new FS.ErrnoError(code);
            FS.genericErrors[code].stack = "<generic error, no stack>"
        })
    },
    staticInit: () => {
        FS.ensureErrnoError();
        FS.nameTable = new Array(4096);
        FS.mount(MEMFS, {}, "/");
        FS.createDefaultDirectories();
        FS.createDefaultDevices();
        FS.createSpecialDirectories();
        FS.filesystems = {
            "MEMFS": MEMFS
        }
    },
    init: (input, output, error) => {
        FS.init.initialized = true;
        FS.ensureErrnoError();
        ModuleLuaScript["stdin"] = input || ModuleLuaScript["stdin"];
        ModuleLuaScript["stdout"] = output || ModuleLuaScript["stdout"];
        ModuleLuaScript["stderr"] = error || ModuleLuaScript["stderr"];
        FS.createStandardStreams()
    },
    quit: () => {
        FS.init.initialized = false;
        for (var i = 0; i < FS.streams.length; i++) {
            var stream = FS.streams[i];
            if (!stream) {
                continue
            }
            FS.close(stream)
        }
    },
    getMode: (canRead, canWrite) => {
        var mode = 0;
        if (canRead)
            mode |= 292 | 73;
        if (canWrite)
            mode |= 146;
        return mode
    },
    findObject: (path, dontResolveLastLink) => {
        var ret = FS.analyzePath(path, dontResolveLastLink);
        if (ret.exists) {
            return ret.object
        } else {
            return null
        }
    },
    analyzePath: (path, dontResolveLastLink) => {
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
    createPath: (parent, path, canRead, canWrite) => {
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
    createFile: (parent, name, properties, canRead, canWrite) => {
        var path = PATH.join2(typeof parent == "string" ? parent : FS.getPath(parent), name);
        var mode = FS.getMode(canRead, canWrite);
        return FS.create(path, mode)
    },
    createDataFile: (parent, name, data, canRead, canWrite, canOwn) => {
        var path = name;
        if (parent) {
            parent = typeof parent == "string" ? parent : FS.getPath(parent);
            path = name ? PATH.join2(parent, name) : parent
        }
        var mode = FS.getMode(canRead, canWrite);
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
        return node
    },
    createDevice: (parent, name, input, output) => {
        var path = PATH.join2(typeof parent == "string" ? parent : FS.getPath(parent), name);
        var mode = FS.getMode(!!input, !!output);
        if (!FS.createDevice.major)
            FS.createDevice.major = 64;
        var dev = FS.makedev(FS.createDevice.major++, 0);
        FS.registerDevice(dev, {
            open: stream => {
                stream.seekable = false
            },
            close: stream => {
                if (output && output.buffer && output.buffer.length) {
                    output(10)
                }
            },
            read: (stream, buffer, offset, length, pos) => {
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
            write: (stream, buffer, offset, length, pos) => {
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
    forceLoadFile: obj => {
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
    createLazyFile: (parent, name, url, canRead, canWrite) => {
        function LazyUint8Array() {
            this.lengthKnown = false;
            this.chunks = []
        }
        LazyUint8Array.prototype.get = function LazyUint8Array_get(idx) {// @ts-ignore
            if (idx > this.length - 1 || idx < 0) {
                return undefined
            }// @ts-ignore
            var chunkOffset = idx % this.chunkSize;// @ts-ignore
            var chunkNum = idx / this.chunkSize | 0;
            return this.getter(chunkNum)[chunkOffset]
        };
        LazyUint8Array.prototype.setDataGetter = function LazyUint8Array_setDataGetter(getter) {
            this.getter = getter
        };
        LazyUint8Array.prototype.cacheLength = function LazyUint8Array_cacheLength() {
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
                } else {
                    return intArrayFromString(xhr.responseText || "", true)
                }
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
        };
        if (typeof XMLHttpRequest != "undefined") {
            if (!ENVIRONMENT_IS_WORKER)
                throw "Cannot do synchronous binary XHRs outside webworkers in modern browsers. Use --embed-file or --preload-file in emcc";
            var lazyArray = new LazyUint8Array;
            Object.defineProperties(lazyArray, {
                length: {
                    get: function () {
                        if (!this.lengthKnown) {
                            this.cacheLength()
                        }
                        return this._length
                    }
                },
                chunkSize: {
                    get: function () {
                        if (!this.lengthKnown) {
                            this.cacheLength()
                        }
                        return this._chunkSize
                    }
                }
            });
            var properties = {
                isDevice: false,
                contents: lazyArray
            }
        } else {// @ts-ignore
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
            stream_ops[key] = function forceLoadLazyFile() {
                FS.forceLoadFile(node);
                return fn.apply(null, arguments)
            }
        });
        stream_ops.read = (stream, buffer, offset, length, position) => {
            FS.forceLoadFile(node);
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
        };
        node.stream_ops = stream_ops;
        return node
    },
    createPreloadedFile: (parent, name, url, canRead, canWrite, onload, onerror, dontCreateFile, canOwn, preFinish) => {
        var fullname = name ? PATH_FS.resolve(PATH.join2(parent, name)) : parent;
        var dep = getUniqueRunDependency("cp " + fullname);
        function processData(byteArray) {
            function finish(byteArray) {
                if (preFinish)
                    preFinish();
                if (!dontCreateFile) {
                    FS.createDataFile(parent, name, byteArray, canRead, canWrite, canOwn)
                }
                if (onload)
                    onload();
                removeRunDependency(dep)
            }// @ts-ignore
            if (Browser.handledByPreloadPlugin(byteArray, fullname, finish, () => {
                    if (onerror)
                        onerror();
                        removeRunDependency(dep)
                    })) {
                    return
                }
            finish(byteArray)
        }
        addRunDependency(dep);
        if (typeof url == "string") {
            asyncLoad(url, byteArray => processData(byteArray), onerror)
        } else {
            processData(url)
        }
    },
    indexedDB: () => {// @ts-ignore
        return window.indexedDB || window.mozIndexedDB || window.webkitIndexedDB || window.msIndexedDB
    },
    DB_NAME: () => {
        return "EM_FS_" + window.location.pathname
    },
    DB_VERSION: 20,
    DB_STORE_NAME: "FILE_DATA",
    saveFilesToDB: (paths, onload, onerror) => {
        onload = onload || (() => {});
        onerror = onerror || (() => {});
        var indexedDB = FS.indexedDB();
        try {
            var openRequest = indexedDB.open(FS.DB_NAME(), FS.DB_VERSION)
        } catch (e) {
            return onerror(e)
        }
        openRequest.onupgradeneeded = () => {
            out("creating db");
            var db = openRequest.result;
            db.createObjectStore(FS.DB_STORE_NAME)
        };
        openRequest.onsuccess = () => {
            var db = openRequest.result;
            var transaction = db.transaction([FS.DB_STORE_NAME], "readwrite");
            var files = transaction.objectStore(FS.DB_STORE_NAME);
            var ok = 0,
            fail = 0,
            total = paths.length;
            function finish() {
                if (fail == 0)
                    onload();
                else
                    onerror()
            }
            paths.forEach(path => {
                var putRequest = files.put(FS.analyzePath(path).object.contents, path);
                putRequest.onsuccess = () => {
                    ok++;
                    if (ok + fail == total)
                        finish()
                };
                putRequest.onerror = () => {
                    fail++;
                    if (ok + fail == total)
                        finish()
                }
            });
            transaction.onerror = onerror
        };
        openRequest.onerror = onerror
    },
    loadFilesFromDB: (paths, onload, onerror) => {
        onload = onload || (() => {});
        onerror = onerror || (() => {});
        var indexedDB = FS.indexedDB();
        try {
            var openRequest = indexedDB.open(FS.DB_NAME(), FS.DB_VERSION)
        } catch (e) {
            return onerror(e)
        }
        openRequest.onupgradeneeded = onerror;
        openRequest.onsuccess = () => {
            var db = openRequest.result;
            try {
                var transaction = db.transaction([FS.DB_STORE_NAME], "readonly")
            } catch (e) {
                onerror(e);
                return
            }
            var files = transaction.objectStore(FS.DB_STORE_NAME);
            var ok = 0,
            fail = 0,
            total = paths.length;
            function finish() {
                if (fail == 0)
                    onload();
                else
                    onerror()
            }
            paths.forEach(path => {
                var getRequest = files.get(path);
                getRequest.onsuccess = () => {
                    if (FS.analyzePath(path).exists) {
                        FS.unlink(path)
                    }
                    FS.createDataFile(PATH.dirname(path), PATH.basename(path), getRequest.result, true, true, true);
                    ok++;
                    if (ok + fail == total)
                        finish()
                };
                getRequest.onerror = () => {
                    fail++;
                    if (ok + fail == total)
                        finish()
                }
            });
            transaction.onerror = onerror
        };
        openRequest.onerror = onerror
    }
};
var SYSCALLS = {
    DEFAULT_POLLMASK: 5,
    calculateAt: function (dirfd, path, allowEmpty) {
        if (PATH.isAbs(path)) {
            return path
        }
        var dir;
        if (dirfd === -100) {
            dir = FS.cwd()
        } else {
            var dirstream = FS.getStream(dirfd);
            if (!dirstream)
                throw new FS.ErrnoError(8);
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
    doStat: function (func, path, buf) {
        try {
            var stat = func(path)
        } catch (e) {
            if (e && e.node && PATH.normalize(path) !== PATH.normalize(FS.getPath(e.node))) {
                return -54
            }
            throw e
        }
        HEAP32[buf >> 2] = stat.dev;
        HEAP32[buf + 4 >> 2] = 0;
        HEAP32[buf + 8 >> 2] = stat.ino;
        HEAP32[buf + 12 >> 2] = stat.mode;
        HEAP32[buf + 16 >> 2] = stat.nlink;
        HEAP32[buf + 20 >> 2] = stat.uid;
        HEAP32[buf + 24 >> 2] = stat.gid;
        HEAP32[buf + 28 >> 2] = stat.rdev;
        HEAP32[buf + 32 >> 2] = 0;
        tempI64 = [stat.size >>> 0, (tempDouble = stat.size, +Math.abs(tempDouble) >= 1 ? tempDouble > 0 ? (Math.min(+Math.floor(tempDouble / 4294967296), 4294967295) | 0) >>> 0 : ~~+Math.ceil((tempDouble -  + (~~tempDouble >>> 0)) / 4294967296) >>> 0 : 0)],
        HEAP32[buf + 40 >> 2] = tempI64[0],
        HEAP32[buf + 44 >> 2] = tempI64[1];
        HEAP32[buf + 48 >> 2] = 4096;
        HEAP32[buf + 52 >> 2] = stat.blocks;
        HEAP32[buf + 56 >> 2] = stat.atime.getTime() / 1e3 | 0;
        HEAP32[buf + 60 >> 2] = 0;
        HEAP32[buf + 64 >> 2] = stat.mtime.getTime() / 1e3 | 0;
        HEAP32[buf + 68 >> 2] = 0;
        HEAP32[buf + 72 >> 2] = stat.ctime.getTime() / 1e3 | 0;
        HEAP32[buf + 76 >> 2] = 0;
        tempI64 = [stat.ino >>> 0, (tempDouble = stat.ino, +Math.abs(tempDouble) >= 1 ? tempDouble > 0 ? (Math.min(+Math.floor(tempDouble / 4294967296), 4294967295) | 0) >>> 0 : ~~+Math.ceil((tempDouble -  + (~~tempDouble >>> 0)) / 4294967296) >>> 0 : 0)],
        HEAP32[buf + 80 >> 2] = tempI64[0],
        HEAP32[buf + 84 >> 2] = tempI64[1];
        return 0
    },
    doMsync: function (addr, stream, len, flags, offset) {
        var buffer = HEAPU8.slice(addr, addr + len);
        FS.msync(stream, buffer, offset, len, flags)
    },
    varargs: undefined,
    get: function () {
        SYSCALLS.varargs += 4;
        var ret = HEAP32[SYSCALLS.varargs - 4 >> 2];
        return ret
    },
    getStr: function (ptr) {
        var ret = UTF8ToString(ptr);
        return ret
    },
    getStreamFromFD: function (fd) {
        var stream = FS.getStream(fd);
        if (!stream)
            throw new FS.ErrnoError(8);
        return stream
    }
};
function ___syscall_dup3(fd, suggestFD, flags) {
    try {
        var old = SYSCALLS.getStreamFromFD(fd);
        if (old.fd === suggestFD)
            return -28;
        var suggest = FS.getStream(suggestFD);
        if (suggest)
            FS.close(suggest);
        return FS.createStream(old, suggestFD, suggestFD + 1).fd
    } catch (e) {
        if (typeof FS == "undefined" || !(e instanceof FS.ErrnoError))
            throw e;
        return -e.errno
    }
}
function setErrNo(value) {
    HEAP32[___errno_location() >> 2] = value;
    return value
}
function ___syscall_fcntl64(fd, cmd, varargs) {
    SYSCALLS.varargs = varargs;
    try {
        var stream = SYSCALLS.getStreamFromFD(fd);
        switch (cmd) {
        case 0: {
                var arg = SYSCALLS.get();
                if (arg < 0) {
                    return -28
                }
                var newStream;
                newStream = FS.createStream(stream, arg);
                return newStream.fd
            }
        case 1:
        case 2:
            return 0;
        case 3:
            return stream.flags;
        case 4: {
                var arg = SYSCALLS.get();
                stream.flags |= arg;
                return 0
            }
        case 5: {
                var arg = SYSCALLS.get();
                var offset = 0;
                HEAP16[arg + offset >> 1] = 2;
                return 0
            }
        case 6:
        case 7:
            return 0;
        case 16:
        case 8:
            return -28;
        case 9:
            setErrNo(28);
            return -1;
        default: {
                return -28
            }
        }
    } catch (e) {
        if (typeof FS == "undefined" || !(e instanceof FS.ErrnoError))
            throw e;
        return -e.errno
    }
}
function ___syscall_ioctl(fd, op, varargs) {
    SYSCALLS.varargs = varargs;
    try {
        var stream = SYSCALLS.getStreamFromFD(fd);
        switch (op) {
        case 21509:
        case 21505: {
                if (!stream.tty)
                    return -59;
                return 0
            }
        case 21510:
        case 21511:
        case 21512:
        case 21506:
        case 21507:
        case 21508: {
                if (!stream.tty)
                    return -59;
                return 0
            }
        case 21519: {
                if (!stream.tty)
                    return -59;
                var argp = SYSCALLS.get();
                HEAP32[argp >> 2] = 0;
                return 0
            }
        case 21520: {
                if (!stream.tty)
                    return -59;
                return -28
            }
        case 21531: {
                var argp = SYSCALLS.get();
                return FS.ioctl(stream, op, argp)
            }
        case 21523: {
                if (!stream.tty)
                    return -59;
                return 0
            }
        case 21524: {
                if (!stream.tty)
                    return -59;
                return 0
            }
        default:
            abort("bad ioctl syscall " + op)
        }
    } catch (e) {
        if (typeof FS == "undefined" || !(e instanceof FS.ErrnoError))
            throw e;
        return -e.errno
    }
}
function ___syscall_lstat64(path, buf) {
    try {
        path = SYSCALLS.getStr(path);
        return SYSCALLS.doStat(FS.lstat, path, buf)
    } catch (e) {
        if (typeof FS == "undefined" || !(e instanceof FS.ErrnoError))
            throw e;
        return -e.errno
    }
}
function ___syscall_openat(dirfd, path, flags, varargs) {
    SYSCALLS.varargs = varargs;
    try {
        path = SYSCALLS.getStr(path);
        path = SYSCALLS.calculateAt(dirfd, path);
        var mode = varargs ? SYSCALLS.get() : 0;
        return FS.open(path, flags, mode).fd
    } catch (e) {
        if (typeof FS == "undefined" || !(e instanceof FS.ErrnoError))
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
        if (typeof FS == "undefined" || !(e instanceof FS.ErrnoError))
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
        if (typeof FS == "undefined" || !(e instanceof FS.ErrnoError))
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
        if (typeof FS == "undefined" || !(e instanceof FS.ErrnoError))
            throw e;
        return -e.errno
    }
}
function __emscripten_date_now() {
    return Date.now()
}
var nowIsMonotonic = true;
function __emscripten_get_now_is_monotonic() {
    return nowIsMonotonic
}
function __emscripten_throw_longjmp() {
    throw Infinity
}
function __gmtime_js(time, tmPtr) {
    var date = new Date(HEAP32[time >> 2] * 1e3);
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
function __localtime_js(time, tmPtr) {
    var date = new Date(HEAP32[time >> 2] * 1e3);
    HEAP32[tmPtr >> 2] = date.getSeconds();
    HEAP32[tmPtr + 4 >> 2] = date.getMinutes();
    HEAP32[tmPtr + 8 >> 2] = date.getHours();
    HEAP32[tmPtr + 12 >> 2] = date.getDate();
    HEAP32[tmPtr + 16 >> 2] = date.getMonth();
    HEAP32[tmPtr + 20 >> 2] = date.getFullYear() - 1900;
    HEAP32[tmPtr + 24 >> 2] = date.getDay();
    var start = new Date(date.getFullYear(), 0, 1);
    var yday = (date.getTime() - start.getTime()) / (1e3 * 60 * 60 * 24) | 0;
    HEAP32[tmPtr + 28 >> 2] = yday;
    HEAP32[tmPtr + 36 >> 2] =  - (date.getTimezoneOffset() * 60);
    var summerOffset = new Date(date.getFullYear(), 6, 1).getTimezoneOffset();
    var winterOffset = start.getTimezoneOffset();// @ts-ignore
    var dst = (summerOffset != winterOffset && date.getTimezoneOffset() == Math.min(winterOffset, summerOffset)) | 0;
    HEAP32[tmPtr + 32 >> 2] = dst
}
function __mktime_js(tmPtr) {
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
    var yday = (date.getTime() - start.getTime()) / (1e3 * 60 * 60 * 24) | 0;
    HEAP32[tmPtr + 28 >> 2] = yday;
    HEAP32[tmPtr >> 2] = date.getSeconds();
    HEAP32[tmPtr + 4 >> 2] = date.getMinutes();
    HEAP32[tmPtr + 8 >> 2] = date.getHours();
    HEAP32[tmPtr + 12 >> 2] = date.getDate();
    HEAP32[tmPtr + 16 >> 2] = date.getMonth();
    return date.getTime() / 1e3 | 0
}
function _tzset_impl(timezone, daylight, tzname) {
    var currentYear = (new Date).getFullYear();
    var winter = new Date(currentYear, 0, 1);
    var summer = new Date(currentYear, 6, 1);
    var winterOffset = winter.getTimezoneOffset();
    var summerOffset = summer.getTimezoneOffset();
    var stdTimezoneOffset = Math.max(winterOffset, summerOffset);
    HEAP32[timezone >> 2] = stdTimezoneOffset * 60;
    HEAP32[daylight >> 2] = Number(winterOffset != summerOffset);
    function extractZone(date) {
        var match = date.toTimeString().match(/\(([A-Za-z ]+)\)$/);
        return match ? match[1] : "GMT"
    }
    var winterName = extractZone(winter);
    var summerName = extractZone(summer);
    var winterNamePtr = allocateUTF8(winterName);
    var summerNamePtr = allocateUTF8(summerName);
    if (summerOffset < winterOffset) {
        HEAPU32[tzname >> 2] = winterNamePtr;
        HEAPU32[tzname + 4 >> 2] = summerNamePtr
    } else {
        HEAPU32[tzname >> 2] = summerNamePtr;
        HEAPU32[tzname + 4 >> 2] = winterNamePtr
    }
}
function __tzset_js(timezone, daylight, tzname) {// @ts-ignore
    if (__tzset_js.called)
        return;// @ts-ignore
    __tzset_js.called = true;
    _tzset_impl(timezone, daylight, tzname)
}
function _abort() {
    abort("")
}
var _emscripten_get_now;
if (ENVIRONMENT_IS_NODE) {
    _emscripten_get_now = () => {
        var t = process["hrtime"]();
        return t[0] * 1e3 + t[1] / 1e6
    }
} else
    _emscripten_get_now = () => performance.now();
function _emscripten_memcpy_big(dest, src, num) {
    HEAPU8.copyWithin(dest, src, src + num)
}
function getHeapMax() {
    return 2147483648
}
function emscripten_realloc_buffer(size) {
    try {
        wasmMemory.grow(size - buffer.byteLength + 65535 >>> 16);
        updateGlobalBufferAndViews(wasmMemory.buffer);
        return 1
    } catch (e) {}
}
function _emscripten_resize_heap(requestedSize) {
    var oldSize = HEAPU8.length;
    requestedSize = requestedSize >>> 0;
    var maxHeapSize = getHeapMax();
    if (requestedSize > maxHeapSize) {
        return false
    }
    let alignUp = (x, multiple) => x + (multiple - x % multiple) % multiple;
    for (var cutDown = 1; cutDown <= 4; cutDown *= 2) {
        var overGrownHeapSize = oldSize * (1 + .2 / cutDown);
        overGrownHeapSize = Math.min(overGrownHeapSize, requestedSize + 100663296);
        var newSize = Math.min(maxHeapSize, alignUp(Math.max(requestedSize, overGrownHeapSize), 65536));
        var replacement = emscripten_realloc_buffer(newSize);
        if (replacement) {
            return true
        }
    }
    return false
}
var ENV = {};
function getExecutableName() {
    return thisProgram || "./this.program"
}
function getEnvStrings() {// @ts-ignore
    if (!getEnvStrings.strings) {
        var lang = (typeof navigator == "object" && navigator.languages && navigator.languages[0] || "C").replace("-", "_") + ".UTF-8";
        var env = {
            "USER": "web_user",
            "LOGNAME": "web_user",
            "PATH": "/",
            "PWD": "/",
            "HOME": "/home/web_user",
            "LANG": lang,
            "_": getExecutableName()
        };
        for (var x in ENV) {
            if (ENV[x] === undefined)
                delete env[x];
            else
                env[x] = ENV[x]
        }
        var strings = [];
        for (var x in env) {
            strings.push(x + "=" + env[x])
        }// @ts-ignore
        getEnvStrings.strings = strings
    }// @ts-ignore
    return getEnvStrings.strings
}
function _environ_get(__environ, environ_buf) {
    var bufSize = 0;
    getEnvStrings().forEach(function (string, i) {
        var ptr = environ_buf + bufSize;
        HEAPU32[__environ + i * 4 >> 2] = ptr;
        writeAsciiToMemory(string, ptr);
        bufSize += string.length + 1
    });
    return 0
}
function _environ_sizes_get(penviron_count, penviron_buf_size) {
    var strings = getEnvStrings();
    HEAPU32[penviron_count >> 2] = strings.length;
    var bufSize = 0;
    strings.forEach(function (string) {
        bufSize += string.length + 1
    });
    HEAPU32[penviron_buf_size >> 2] = bufSize;
    return 0
}
function _exit(status) {
    exit(status)
}
function _fd_close(fd) {
    try {
        var stream = SYSCALLS.getStreamFromFD(fd);
        FS.close(stream);
        return 0
    } catch (e) {
        if (typeof FS == "undefined" || !(e instanceof FS.ErrnoError))
            throw e;
        return e.errno
    }
}
function doReadv(stream, iov, iovcnt, offset) {
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
            break
    }
    return ret
}
function _fd_read(fd, iov, iovcnt, pnum) {
    try {
        var stream = SYSCALLS.getStreamFromFD(fd);
        var num = doReadv(stream, iov, iovcnt);
        HEAP32[pnum >> 2] = num;
        return 0
    } catch (e) {
        if (typeof FS == "undefined" || !(e instanceof FS.ErrnoError))
            throw e;
        return e.errno
    }
}
function convertI32PairToI53Checked(lo, hi) {// @ts-ignore
    return hi + 2097152 >>> 0 < 4194305 - !!lo ? (lo >>> 0) + hi * 4294967296 : NaN
}
function _fd_seek(fd, offset_low, offset_high, whence, newOffset) {
    try {
        var offset = convertI32PairToI53Checked(offset_low, offset_high);
        if (isNaN(offset))
            return 61;
        var stream = SYSCALLS.getStreamFromFD(fd);
        FS.llseek(stream, offset, whence);
        tempI64 = [stream.position >>> 0, (tempDouble = stream.position, +Math.abs(tempDouble) >= 1 ? tempDouble > 0 ? (Math.min(+Math.floor(tempDouble / 4294967296), 4294967295) | 0) >>> 0 : ~~+Math.ceil((tempDouble -  + (~~tempDouble >>> 0)) / 4294967296) >>> 0 : 0)],
        HEAP32[newOffset >> 2] = tempI64[0],
        HEAP32[newOffset + 4 >> 2] = tempI64[1];
        if (stream.getdents && offset === 0 && whence === 0)
            stream.getdents = null;
        return 0
    } catch (e) {
        if (typeof FS == "undefined" || !(e instanceof FS.ErrnoError))
            throw e;
        return e.errno
    }
}
function doWritev(stream, iov, iovcnt, offset) {
    var ret = 0;
    for (var i = 0; i < iovcnt; i++) {
        var ptr = HEAPU32[iov >> 2];
        var len = HEAPU32[iov + 4 >> 2];
        iov += 8;
        var curr = FS.write(stream, HEAP8, ptr, len, offset);
        if (curr < 0)
            return -1;
        ret += curr
    }
    return ret
}
function _fd_write(fd, iov, iovcnt, pnum) {
    try {
        var stream = SYSCALLS.getStreamFromFD(fd);
        var num = doWritev(stream, iov, iovcnt);
        HEAPU32[pnum >> 2] = num;
        return 0
    } catch (e) {
        if (typeof FS == "undefined" || !(e instanceof FS.ErrnoError))
            throw e;
        return e.errno
    }
}
function _getTempRet0() {
    return getTempRet0()
}
function _setTempRet0(val) {
    setTempRet0(val)
}
function __isLeapYear(year) {
    return year % 4 === 0 && (year % 100 !== 0 || year % 400 === 0)
}
function __arraySum(array, index) {
    var sum = 0;
    for (var i = 0; i <= index; sum += array[i++]) {}
    return sum
}
var __MONTH_DAYS_LEAP = [31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31];
var __MONTH_DAYS_REGULAR = [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31];
function __addDays(date, days) {
    var newDate = new Date(date.getTime());
    while (days > 0) {
        var leap = __isLeapYear(newDate.getFullYear());
        var currentMonth = newDate.getMonth();
        var daysInCurrentMonth = (leap ? __MONTH_DAYS_LEAP : __MONTH_DAYS_REGULAR)[currentMonth];
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
}
function _strftime(s, maxsize, format, tm) {
    var tm_zone = HEAP32[tm + 40 >> 2];
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
        var thisDate = __addDays(new Date(date.tm_year + 1900, 0, 1), date.tm_yday);
        var janFourthThisYear = new Date(thisDate.getFullYear(), 0, 4);
        var janFourthNextYear = new Date(thisDate.getFullYear() + 1, 0, 4);
        var firstWeekStartThisYear = getFirstWeekStartDate(janFourthThisYear);
        var firstWeekStartNextYear = getFirstWeekStartDate(janFourthNextYear);
        if (compareByDay(firstWeekStartThisYear, thisDate) <= 0) {
            if (compareByDay(firstWeekStartNextYear, thisDate) <= 0) {
                return thisDate.getFullYear() + 1
            } else {
                return thisDate.getFullYear()
            }
        } else {
            return thisDate.getFullYear() - 1
        }
    }
    var EXPANSION_RULES_2 = {
        "%a": function (date) {
            return WEEKDAYS[date.tm_wday].substring(0, 3)
        },
        "%A": function (date) {
            return WEEKDAYS[date.tm_wday]
        },
        "%b": function (date) {
            return MONTHS[date.tm_mon].substring(0, 3)
        },
        "%B": function (date) {
            return MONTHS[date.tm_mon]
        },
        "%C": function (date) {
            var year = date.tm_year + 1900;
            return leadingNulls(year / 100 | 0, 2)
        },
        "%d": function (date) {
            return leadingNulls(date.tm_mday, 2)
        },
        "%e": function (date) {
            return leadingSomething(date.tm_mday, 2, " ")
        },
        "%g": function (date) {
            return getWeekBasedYear(date).toString().substring(2)
        },
        "%G": function (date) {
            return getWeekBasedYear(date)
        },
        "%H": function (date) {
            return leadingNulls(date.tm_hour, 2)
        },
        "%I": function (date) {
            var twelveHour = date.tm_hour;
            if (twelveHour == 0)
                twelveHour = 12;
            else if (twelveHour > 12)
                twelveHour -= 12;
            return leadingNulls(twelveHour, 2)
        },
        "%j": function (date) {
            return leadingNulls(date.tm_mday + __arraySum(__isLeapYear(date.tm_year + 1900) ? __MONTH_DAYS_LEAP : __MONTH_DAYS_REGULAR, date.tm_mon - 1), 3)
        },
        "%m": function (date) {
            return leadingNulls(date.tm_mon + 1, 2)
        },
        "%M": function (date) {
            return leadingNulls(date.tm_min, 2)
        },
        "%n": function () {
            return "\n"
        },
        "%p": function (date) {
            if (date.tm_hour >= 0 && date.tm_hour < 12) {
                return "AM"
            } else {
                return "PM"
            }
        },
        "%S": function (date) {
            return leadingNulls(date.tm_sec, 2)
        },
        "%t": function () {
            return "\t"
        },
        "%u": function (date) {
            return date.tm_wday || 7
        },
        "%U": function (date) {
            var days = date.tm_yday + 7 - date.tm_wday;
            return leadingNulls(Math.floor(days / 7), 2)
        },
        "%V": function (date) {
            var val = Math.floor((date.tm_yday + 7 - (date.tm_wday + 6) % 7) / 7);
            if ((date.tm_wday + 371 - date.tm_yday - 2) % 7 <= 2) {
                val++
            }
            if (!val) {
                val = 52;
                var dec31 = (date.tm_wday + 7 - date.tm_yday - 1) % 7;
                if (dec31 == 4 || dec31 == 5 && __isLeapYear(date.tm_year % 400 - 1)) {
                    val++
                }
            } else if (val == 53) {
                var jan1 = (date.tm_wday + 371 - date.tm_yday) % 7;
                if (jan1 != 4 && (jan1 != 3 || !__isLeapYear(date.tm_year)))
                    val = 1
            }
            return leadingNulls(val, 2)
        },
        "%w": function (date) {
            return date.tm_wday
        },
        "%W": function (date) {
            var days = date.tm_yday + 7 - (date.tm_wday + 6) % 7;
            return leadingNulls(Math.floor(days / 7), 2)
        },
        "%y": function (date) {
            return (date.tm_year + 1900).toString().substring(2)
        },
        "%Y": function (date) {
            return date.tm_year + 1900
        },
        "%z": function (date) {
            var off = date.tm_gmtoff;
            var ahead = off >= 0;
            off = Math.abs(off) / 60;
            off = off / 60 * 100 + off % 60;
            return (ahead ? "+" : "-") + String("0000" + off).slice(-4)
        },
        "%Z": function (date) {
            return date.tm_zone
        },
        "%%": function () {
            return "%"
        }
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
}
function _system(command) {
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
    setErrNo(52);
    return -1
}
function runAndAbortIfError(func) {
    try {
        return func()
    } catch (e) {
        abort(e)
    }
}
function callUserCallback(func, synchronous) {
    if (ABORT) {
        return
    }
    if (synchronous) {
        func();
        return
    }
    try {
        func()
    } catch (e) {
        handleException(e)
    }
}
function runtimeKeepalivePush() {}
function runtimeKeepalivePop() {}
var Asyncify = {
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
    getCallStackId: function (funcName) {
        var id = Asyncify.callStackNameToId[funcName];
        if (id === undefined) {
            id = Asyncify.callStackId++;
            Asyncify.callStackNameToId[funcName] = id;
            Asyncify.callStackIdToName[id] = funcName
        }
        return id
    },
    instrumentWasmExports: function (exports) {
        var ret = {};
        for (var x in exports) {
            (function (x) {
                var original = exports[x];
                if (typeof original == "function") {
                    ret[x] = function () {
                        Asyncify.exportCallStack.push(x);
                        try {
                            return original.apply(null, arguments)
                        } finally {
                            if (!ABORT) {
                                var y = Asyncify.exportCallStack.pop();
                                assert(y === x);
                                Asyncify.maybeStopUnwind()
                            }
                        }
                    }
                } else {
                    ret[x] = original
                }
            })(x)
        }
        return ret
    },
    maybeStopUnwind: function () {
        if (Asyncify.currData && Asyncify.state === Asyncify.State.Unwinding && Asyncify.exportCallStack.length === 0) {
            Asyncify.state = Asyncify.State.Normal;
            runAndAbortIfError(ModuleLuaScript["_asyncify_stop_unwind"]);// @ts-ignore
            if (typeof Fibers != "undefined") {// @ts-ignore
                Fibers.trampoline()
            }
        }
    },
    whenDone: function () {
        return new Promise((resolve, reject) => {
            Asyncify.asyncPromiseHandlers = {
                resolve: resolve,
                reject: reject
            }
        })
    },
    allocateData: function () {
        var ptr = _malloc(12 + Asyncify.StackSize);
        Asyncify.setDataHeader(ptr, ptr + 12, Asyncify.StackSize);
        Asyncify.setDataRewindFunc(ptr);
        return ptr
    },
    setDataHeader: function (ptr, stack, stackSize) {
        HEAP32[ptr >> 2] = stack;
        HEAP32[ptr + 4 >> 2] = stack + stackSize
    },
    setDataRewindFunc: function (ptr) {
        var bottomOfCallStack = Asyncify.exportCallStack[0];
        var rewindId = Asyncify.getCallStackId(bottomOfCallStack);
        HEAP32[ptr + 8 >> 2] = rewindId
    },
    getDataRewindFunc: function (ptr) {
        var id = HEAP32[ptr + 8 >> 2];
        var name = Asyncify.callStackIdToName[id];
        var func = ModuleLuaScript["asm"][name];
        return func
    },
    doRewind: function (ptr) {
        var start = Asyncify.getDataRewindFunc(ptr);
        return start()
    },
    handleSleep: function (startAsync) {
        if (ABORT)
            return;
        if (Asyncify.state === Asyncify.State.Normal) {
            var reachedCallback = false;
            var reachedAfterCallback = false;
            startAsync(handleSleepReturnValue => {
                if (ABORT)
                    return;
                Asyncify.handleSleepReturnValue = handleSleepReturnValue || 0;
                reachedCallback = true;
                if (!reachedAfterCallback) {
                    return
                }
                Asyncify.state = Asyncify.State.Rewinding;
                runAndAbortIfError(() => ModuleLuaScript["_asyncify_start_rewind"](Asyncify.currData));// @ts-ignore
                if (typeof Browser != "undefined" && Browser.mainLoop.func) {// @ts-ignore
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
                Asyncify.currData = Asyncify.allocateData();
                runAndAbortIfError(() => ModuleLuaScript["_asyncify_start_unwind"](Asyncify.currData));// @ts-ignore
                if (typeof Browser != "undefined" && Browser.mainLoop.func) {// @ts-ignore
                    Browser.mainLoop.pause()
                }
            }
        } else if (Asyncify.state === Asyncify.State.Rewinding) {
            Asyncify.state = Asyncify.State.Normal;
            runAndAbortIfError(ModuleLuaScript["_asyncify_stop_rewind"]);
            _free(Asyncify.currData);
            Asyncify.currData = null;
            Asyncify.sleepCallbacks.forEach(func => callUserCallback(func))
        } else {
            abort("invalid state: " + Asyncify.state)
        }
        return Asyncify.handleSleepReturnValue
    },
    handleAsync: function (startAsync) {
        return Asyncify.handleSleep(wakeUp => {
            startAsync().then(wakeUp)
        })
    }
};
var FSNode = function (parent, name, mode, rdev) {
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
    this.rdev = rdev
};
var readMode = 292 | 73;
var writeMode = 146;
Object.defineProperties(FSNode.prototype, {
    read: {
        get: function () {
            return (this.mode & readMode) === readMode
        },
        set: function (val) {
            val ? this.mode |= readMode : this.mode &= ~readMode
        }
    },
    write: {
        get: function () {
            return (this.mode & writeMode) === writeMode
        },
        set: function (val) {
            val ? this.mode |= writeMode : this.mode &= ~writeMode
        }
    },
    isFolder: {
        get: function () {
            return FS.isDir(this.mode)
        }
    },
    isDevice: {
        get: function () {
            return FS.isChrdev(this.mode)
        }
    }
});
FS.FSNode = FSNode;
FS.staticInit();
var ASSERTIONS = false;
function intArrayFromString(stringy, dontAddNull, length) {
    var len = length > 0 ? length : lengthBytesUTF8(stringy) + 1;
    var u8array = new Array(len);
    var numBytesWritten = stringToUTF8Array(stringy, u8array, 0, u8array.length);
    if (dontAddNull)
        u8array.length = numBytesWritten;
    return u8array
}
var asmLibraryArg = {
    "__assert_fail": ___assert_fail,
    "__asyncjs__animlist_init": __asyncjs__animlist_init,
    "__asyncjs__atlas_init": __asyncjs__atlas_init,
    "__asyncjs__dialogue_show_dialog": __asyncjs__dialogue_show_dialog,
    "__asyncjs__dialogue_show_dialog2": __asyncjs__dialogue_show_dialog2,
    "__asyncjs__fs_readfile": __asyncjs__fs_readfile,
    "__asyncjs__json_load_from": __asyncjs__json_load_from,
    "__asyncjs__menu_init": __asyncjs__menu_init,
    "__asyncjs__menumanifest_init": __asyncjs__menumanifest_init,
    "__asyncjs__messagebox_set_image_from_atlas": __asyncjs__messagebox_set_image_from_atlas,
    "__asyncjs__messagebox_set_image_from_texture": __asyncjs__messagebox_set_image_from_texture,
    "__asyncjs__modding_get_messagebox": __asyncjs__modding_get_messagebox,
    "__asyncjs__modding_launch_credits": __asyncjs__modding_launch_credits,
    "__asyncjs__modding_launch_freeplay": __asyncjs__modding_launch_freeplay,
    "__asyncjs__modding_launch_mainmenu": __asyncjs__modding_launch_mainmenu,
    "__asyncjs__modding_launch_settings": __asyncjs__modding_launch_settings,
    "__asyncjs__modding_launch_startscreen": __asyncjs__modding_launch_startscreen,
    "__asyncjs__modding_launch_week": __asyncjs__modding_launch_week,
    "__asyncjs__modding_launch_weekselector": __asyncjs__modding_launch_weekselector,
    "__asyncjs__modding_replace_native_background_music": __asyncjs__modding_replace_native_background_music,
    "__asyncjs__modding_spawn_screen": __asyncjs__modding_spawn_screen,
    "__asyncjs__modelholder_init": __asyncjs__modelholder_init,
    "__asyncjs__modelholder_init2": __asyncjs__modelholder_init2,
    "__asyncjs__songplayer_play": __asyncjs__songplayer_play,
    "__asyncjs__week_rebuild_ui": __asyncjs__week_rebuild_ui,
    "__js__animlist_destroy": __js__animlist_destroy,
    "__js__animlist_get_animation": __js__animlist_get_animation,
    "__js__animlist_is_item_frame_animation": __js__animlist_is_item_frame_animation,
    "__js__animlist_is_item_macro_animation": __js__animlist_is_item_macro_animation,
    "__js__animlist_is_item_tweenkeyframe_animation": __js__animlist_is_item_tweenkeyframe_animation,
    "__js__animsprite_destroy": __js__animsprite_destroy,
    "__js__animsprite_get_name": __js__animsprite_get_name,
    "__js__animsprite_init": __js__animsprite_init,
    "__js__animsprite_init_as_empty": __js__animsprite_init_as_empty,
    "__js__animsprite_init_from_animlist": __js__animsprite_init_from_animlist,
    "__js__animsprite_init_from_atlas": __js__animsprite_init_from_atlas,
    "__js__animsprite_init_from_tweenlerp": __js__animsprite_init_from_tweenlerp,
    "__js__animsprite_is_frame_animation": __js__animsprite_is_frame_animation,
    "__js__animsprite_restart": __js__animsprite_restart,
    "__js__animsprite_set_delay": __js__animsprite_set_delay,
    "__js__animsprite_set_loop": __js__animsprite_set_loop,
    "__js__atlas_destroy": __js__atlas_destroy,
    "__js__atlas_get_entry": __js__atlas_get_entry,
    "__js__atlas_get_entry_with_number_suffix": __js__atlas_get_entry_with_number_suffix,
    "__js__atlas_get_glyph_fps": __js__atlas_get_glyph_fps,
    "__js__atlas_get_index_of": __js__atlas_get_index_of,
    "__js__atlas_get_texture_resolution": __js__atlas_get_texture_resolution,
    "__js__atlas_utils_is_known_extension": __js__atlas_utils_is_known_extension,
    "__js__camera_apply": __js__camera_apply,
    "__js__camera_debug_log_info": __js__camera_debug_log_info,
    "__js__camera_disable_offset_zoom": __js__camera_disable_offset_zoom,
    "__js__camera_end": __js__camera_end,
    "__js__camera_from_layout": __js__camera_from_layout,
    "__js__camera_get_modifier": __js__camera_get_modifier,
    "__js__camera_get_offset": __js__camera_get_offset,
    "__js__camera_get_parent_layout": __js__camera_get_parent_layout,
    "__js__camera_is_completed": __js__camera_is_completed,
    "__js__camera_move": __js__camera_move,
    "__js__camera_move_offset": __js__camera_move_offset,
    "__js__camera_repeat": __js__camera_repeat,
    "__js__camera_set_absolute_position": __js__camera_set_absolute_position,
    "__js__camera_set_absolute_zoom": __js__camera_set_absolute_zoom,
    "__js__camera_set_animation": __js__camera_set_animation,
    "__js__camera_set_interpolator_type": __js__camera_set_interpolator_type,
    "__js__camera_set_offset": __js__camera_set_offset,
    "__js__camera_set_transition_duration": __js__camera_set_transition_duration,
    "__js__camera_slide": __js__camera_slide,
    "__js__camera_slide_offset": __js__camera_slide_offset,
    "__js__camera_slide_to": __js__camera_slide_to,
    "__js__camera_slide_to_offset": __js__camera_slide_to_offset,
    "__js__camera_slide_x": __js__camera_slide_x,
    "__js__camera_slide_x_offset": __js__camera_slide_x_offset,
    "__js__camera_slide_y": __js__camera_slide_y,
    "__js__camera_slide_y_offset": __js__camera_slide_y_offset,
    "__js__camera_slide_z": __js__camera_slide_z,
    "__js__camera_slide_z_offset": __js__camera_slide_z_offset,
    "__js__camera_stop": __js__camera_stop,
    "__js__camera_to_origin": __js__camera_to_origin,
    "__js__camera_to_origin_offset": __js__camera_to_origin_offset,
    "__js__character_animation_end": __js__character_animation_end,
    "__js__character_animation_restart": __js__character_animation_restart,
    "__js__character_animation_set": __js__character_animation_set,
    "__js__character_enable_continuous_idle": __js__character_enable_continuous_idle,
    "__js__character_enable_flip_correction": __js__character_enable_flip_correction,
    "__js__character_enable_reference_size": __js__character_enable_reference_size,
    "__js__character_face_as_opponent": __js__character_face_as_opponent,
    "__js__character_flip_orientation": __js__character_flip_orientation,
    "__js__character_freeze_animation": __js__character_freeze_animation,
    "__js__character_get_current_action": __js__character_get_current_action,
    "__js__character_get_modifier": __js__character_get_modifier,
    "__js__character_get_play_calls": __js__character_get_play_calls,
    "__js__character_has_direction": __js__character_has_direction,
    "__js__character_is_idle_active": __js__character_is_idle_active,
    "__js__character_play_extra": __js__character_play_extra,
    "__js__character_play_hey": __js__character_play_hey,
    "__js__character_play_idle": __js__character_play_idle,
    "__js__character_play_miss": __js__character_play_miss,
    "__js__character_play_sing": __js__character_play_sing,
    "__js__character_reset": __js__character_reset,
    "__js__character_set_alpha": __js__character_set_alpha,
    "__js__character_set_color_offset": __js__character_set_color_offset,
    "__js__character_set_color_offset_to_default": __js__character_set_color_offset_to_default,
    "__js__character_set_draw_align": __js__character_set_draw_align,
    "__js__character_set_draw_location": __js__character_set_draw_location,
    "__js__character_set_idle_speed": __js__character_set_idle_speed,
    "__js__character_set_offset": __js__character_set_offset,
    "__js__character_set_scale": __js__character_set_scale,
    "__js__character_set_visible": __js__character_set_visible,
    "__js__character_set_z_index": __js__character_set_z_index,
    "__js__character_set_z_offset": __js__character_set_z_offset,
    "__js__character_state_add": __js__character_state_add,
    "__js__character_state_toggle": __js__character_state_toggle,
    "__js__character_trailing_enabled": __js__character_trailing_enabled,
    "__js__character_trailing_set_offsetcolor": __js__character_trailing_set_offsetcolor,
    "__js__character_trailing_set_params": __js__character_trailing_set_params,
    "__js__character_update_reference_size": __js__character_update_reference_size,
    "__js__character_use_alternate_sing_animations": __js__character_use_alternate_sing_animations,
    "__js__conductor_clear_mapping": __js__conductor_clear_mapping,
    "__js__conductor_destroy": __js__conductor_destroy,
    "__js__conductor_disable": __js__conductor_disable,
    "__js__conductor_disable_strum_line": __js__conductor_disable_strum_line,
    "__js__conductor_get_character": __js__conductor_get_character,
    "__js__conductor_init": __js__conductor_init,
    "__js__conductor_map_automatically": __js__conductor_map_automatically,
    "__js__conductor_map_strum_to_player_extra_add": __js__conductor_map_strum_to_player_extra_add,
    "__js__conductor_map_strum_to_player_extra_remove": __js__conductor_map_strum_to_player_extra_remove,
    "__js__conductor_map_strum_to_player_sing_add": __js__conductor_map_strum_to_player_sing_add,
    "__js__conductor_map_strum_to_player_sing_remove": __js__conductor_map_strum_to_player_sing_remove,
    "__js__conductor_play_hey": __js__conductor_play_hey,
    "__js__conductor_play_idle": __js__conductor_play_idle,
    "__js__conductor_poll": __js__conductor_poll,
    "__js__conductor_poll_reset": __js__conductor_poll_reset,
    "__js__conductor_remove_strum": __js__conductor_remove_strum,
    "__js__conductor_set_character": __js__conductor_set_character,
    "__js__conductor_use_strum_line": __js__conductor_use_strum_line,
    "__js__conductor_use_strums": __js__conductor_use_strums,
    "__js__countdown_get_drawable": __js__countdown_get_drawable,
    "__js__countdown_has_ended": __js__countdown_has_ended,
    "__js__countdown_ready": __js__countdown_ready,
    "__js__countdown_set_bpm": __js__countdown_set_bpm,
    "__js__countdown_set_default_animation2": __js__countdown_set_default_animation2,
    "__js__countdown_start": __js__countdown_start,
    "__js__dialogue_apply_state": __js__dialogue_apply_state,
    "__js__dialogue_apply_state2": __js__dialogue_apply_state2,
    "__js__dialogue_close": __js__dialogue_close,
    "__js__dialogue_get_modifier": __js__dialogue_get_modifier,
    "__js__dialogue_hide": __js__dialogue_hide,
    "__js__dialogue_is_completed": __js__dialogue_is_completed,
    "__js__dialogue_is_hidden": __js__dialogue_is_hidden,
    "__js__dialogue_set_alpha": __js__dialogue_set_alpha,
    "__js__dialogue_set_antialiasing": __js__dialogue_set_antialiasing,
    "__js__dialogue_set_offsetcolor": __js__dialogue_set_offsetcolor,
    "__js__drawable_blend_enable": __js__drawable_blend_enable,
    "__js__drawable_blend_set": __js__drawable_blend_set,
    "__js__drawable_get_alpha": __js__drawable_get_alpha,
    "__js__drawable_get_modifier": __js__drawable_get_modifier,
    "__js__drawable_get_shader": __js__drawable_get_shader,
    "__js__drawable_get_z_index": __js__drawable_get_z_index,
    "__js__drawable_set_alpha": __js__drawable_set_alpha,
    "__js__drawable_set_antialiasing": __js__drawable_set_antialiasing,
    "__js__drawable_set_offsetcolor": __js__drawable_set_offsetcolor,
    "__js__drawable_set_offsetcolor_to_default": __js__drawable_set_offsetcolor_to_default,
    "__js__drawable_set_shader": __js__drawable_set_shader,
    "__js__drawable_set_z_index": __js__drawable_set_z_index,
    "__js__drawable_set_z_offset": __js__drawable_set_z_offset,
    "__js__healthbar_animation_end": __js__healthbar_animation_end,
    "__js__healthbar_animation_restart": __js__healthbar_animation_restart,
    "__js__healthbar_animation_set": __js__healthbar_animation_set,
    "__js__healthbar_bump_enable": __js__healthbar_bump_enable,
    "__js__healthbar_disable_icon_overlap": __js__healthbar_disable_icon_overlap,
    "__js__healthbar_disable_progress_animation": __js__healthbar_disable_progress_animation,
    "__js__healthbar_disable_warnings": __js__healthbar_disable_warnings,
    "__js__healthbar_enable_extra_length": __js__healthbar_enable_extra_length,
    "__js__healthbar_enable_low_health_flash_warning": __js__healthbar_enable_low_health_flash_warning,
    "__js__healthbar_enable_vertical": __js__healthbar_enable_vertical,
    "__js__healthbar_get_bar_midpoint": __js__healthbar_get_bar_midpoint,
    "__js__healthbar_get_drawable": __js__healthbar_get_drawable,
    "__js__healthbar_get_percent": __js__healthbar_get_percent,
    "__js__healthbar_hide_warnings": __js__healthbar_hide_warnings,
    "__js__healthbar_load_warnings": __js__healthbar_load_warnings,
    "__js__healthbar_set_alpha": __js__healthbar_set_alpha,
    "__js__healthbar_set_bpm": __js__healthbar_set_bpm,
    "__js__healthbar_set_bump_animation_opponent": __js__healthbar_set_bump_animation_opponent,
    "__js__healthbar_set_bump_animation_player": __js__healthbar_set_bump_animation_player,
    "__js__healthbar_set_health_position": __js__healthbar_set_health_position,
    "__js__healthbar_set_health_position2": __js__healthbar_set_health_position2,
    "__js__healthbar_set_opponent_bar_color": __js__healthbar_set_opponent_bar_color,
    "__js__healthbar_set_opponent_bar_color_rgb8": __js__healthbar_set_opponent_bar_color_rgb8,
    "__js__healthbar_set_player_bar_color": __js__healthbar_set_player_bar_color,
    "__js__healthbar_set_player_bar_color_rgb8": __js__healthbar_set_player_bar_color_rgb8,
    "__js__healthbar_set_visible": __js__healthbar_set_visible,
    "__js__healthbar_show_drain_warning": __js__healthbar_show_drain_warning,
    "__js__healthbar_show_locked_warning": __js__healthbar_show_locked_warning,
    "__js__healthbar_state_background_add": __js__healthbar_state_background_add,
    "__js__healthbar_state_background_add2": __js__healthbar_state_background_add2,
    "__js__healthbar_state_opponent_add": __js__healthbar_state_opponent_add,
    "__js__healthbar_state_opponent_add2": __js__healthbar_state_opponent_add2,
    "__js__healthbar_state_player_add": __js__healthbar_state_player_add,
    "__js__healthbar_state_player_add2": __js__healthbar_state_player_add2,
    "__js__healthbar_state_toggle": __js__healthbar_state_toggle,
    "__js__healthbar_state_toggle_background": __js__healthbar_state_toggle_background,
    "__js__healthbar_state_toggle_opponent": __js__healthbar_state_toggle_opponent,
    "__js__healthbar_state_toggle_player": __js__healthbar_state_toggle_player,
    "__js__healthwatcher_add_opponent": __js__healthwatcher_add_opponent,
    "__js__healthwatcher_add_player": __js__healthwatcher_add_player,
    "__js__healthwatcher_balance": __js__healthwatcher_balance,
    "__js__healthwatcher_clear": __js__healthwatcher_clear,
    "__js__healthwatcher_enable_dead": __js__healthwatcher_enable_dead,
    "__js__healthwatcher_enable_recover": __js__healthwatcher_enable_recover,
    "__js__healthwatcher_has_deads": __js__healthwatcher_has_deads,
    "__js__healthwatcher_reset_opponents": __js__healthwatcher_reset_opponents,
    "__js__kdmyEngine_change_window_title": __js__kdmyEngine_change_window_title,
    "__js__kdmyEngine_create_array": __js__kdmyEngine_create_array,
    "__js__kdmyEngine_create_object": __js__kdmyEngine_create_object,
    "__js__kdmyEngine_forget_obtained": __js__kdmyEngine_forget_obtained,
    "__js__kdmyEngine_get_language": __js__kdmyEngine_get_language,
    "__js__kdmyEngine_get_locationquery": __js__kdmyEngine_get_locationquery,
    "__js__kdmyEngine_get_screen_size": __js__kdmyEngine_get_screen_size,
    "__js__kdmyEngine_get_useragent": __js__kdmyEngine_get_useragent,
    "__js__kdmyEngine_open_link": __js__kdmyEngine_open_link,
    "__js__kdmyEngine_parse_json": __js__kdmyEngine_parse_json,
    "__js__kdmyEngine_read_array_item_object": __js__kdmyEngine_read_array_item_object,
    "__js__kdmyEngine_read_prop_boolean": __js__kdmyEngine_read_prop_boolean,
    "__js__kdmyEngine_read_prop_double": __js__kdmyEngine_read_prop_double,
    "__js__kdmyEngine_read_prop_float": __js__kdmyEngine_read_prop_float,
    "__js__kdmyEngine_read_prop_floatboolean": __js__kdmyEngine_read_prop_floatboolean,
    "__js__kdmyEngine_read_prop_integer": __js__kdmyEngine_read_prop_integer,
    "__js__kdmyEngine_read_prop_object": __js__kdmyEngine_read_prop_object,
    "__js__kdmyEngine_read_prop_string": __js__kdmyEngine_read_prop_string,
    "__js__kdmyEngine_read_window_object": __js__kdmyEngine_read_window_object,
    "__js__kdmyEngine_require_window_attention": __js__kdmyEngine_require_window_attention,
    "__js__kdmyEngine_write_in_array_boolean": __js__kdmyEngine_write_in_array_boolean,
    "__js__kdmyEngine_write_in_array_double": __js__kdmyEngine_write_in_array_double,
    "__js__kdmyEngine_write_in_array_float": __js__kdmyEngine_write_in_array_float,
    "__js__kdmyEngine_write_in_array_integer": __js__kdmyEngine_write_in_array_integer,
    "__js__kdmyEngine_write_in_array_object": __js__kdmyEngine_write_in_array_object,
    "__js__kdmyEngine_write_in_array_string": __js__kdmyEngine_write_in_array_string,
    "__js__kdmyEngine_write_prop_boolean": __js__kdmyEngine_write_prop_boolean,
    "__js__kdmyEngine_write_prop_double": __js__kdmyEngine_write_prop_double,
    "__js__kdmyEngine_write_prop_float": __js__kdmyEngine_write_prop_float,
    "__js__kdmyEngine_write_prop_integer": __js__kdmyEngine_write_prop_integer,
    "__js__kdmyEngine_write_prop_object": __js__kdmyEngine_write_prop_object,
    "__js__kdmyEngine_write_prop_string": __js__kdmyEngine_write_prop_string,
    "__js__layout_animation_is_completed": __js__layout_animation_is_completed,
    "__js__layout_camera_is_completed": __js__layout_camera_is_completed,
    "__js__layout_camera_set_view": __js__layout_camera_set_view,
    "__js__layout_contains_action": __js__layout_contains_action,
    "__js__layout_disable_antialiasing": __js__layout_disable_antialiasing,
    "__js__layout_get_attached_value2": __js__layout_get_attached_value2,
    "__js__layout_get_attached_value_type": __js__layout_get_attached_value_type,
    "__js__layout_get_camera_helper": __js__layout_get_camera_helper,
    "__js__layout_get_group_modifier": __js__layout_get_group_modifier,
    "__js__layout_get_group_shader": __js__layout_get_group_shader,
    "__js__layout_get_placeholder": __js__layout_get_placeholder,
    "__js__layout_get_secondary_camera_helper": __js__layout_get_secondary_camera_helper,
    "__js__layout_get_soundplayer": __js__layout_get_soundplayer,
    "__js__layout_get_sprite": __js__layout_get_sprite,
    "__js__layout_get_textsprite": __js__layout_get_textsprite,
    "__js__layout_get_videoplayer": __js__layout_get_videoplayer,
    "__js__layout_get_viewport_size": __js__layout_get_viewport_size,
    "__js__layout_resume": __js__layout_resume,
    "__js__layout_screen_to_layout_coordinates": __js__layout_screen_to_layout_coordinates,
    "__js__layout_set_group_alpha": __js__layout_set_group_alpha,
    "__js__layout_set_group_antialiasing": __js__layout_set_group_antialiasing,
    "__js__layout_set_group_offsetcolor": __js__layout_set_group_offsetcolor,
    "__js__layout_set_group_shader": __js__layout_set_group_shader,
    "__js__layout_set_group_visibility": __js__layout_set_group_visibility,
    "__js__layout_stop_all_triggers": __js__layout_stop_all_triggers,
    "__js__layout_stop_trigger": __js__layout_stop_trigger,
    "__js__layout_suspend": __js__layout_suspend,
    "__js__layout_trigger_action": __js__layout_trigger_action,
    "__js__layout_trigger_any": __js__layout_trigger_any,
    "__js__layout_trigger_camera": __js__layout_trigger_camera,
    "__js__layout_trigger_trigger": __js__layout_trigger_trigger,
    "__js__menu_destroy": __js__menu_destroy,
    "__js__menu_get_drawable": __js__menu_get_drawable,
    "__js__menu_get_item_rect": __js__menu_get_item_rect,
    "__js__menu_get_items_count": __js__menu_get_items_count,
    "__js__menu_get_selected_index": __js__menu_get_selected_index,
    "__js__menu_get_selected_item_name": __js__menu_get_selected_item_name,
    "__js__menu_has_item": __js__menu_has_item,
    "__js__menu_select_horizontal": __js__menu_select_horizontal,
    "__js__menu_select_index": __js__menu_select_index,
    "__js__menu_select_item": __js__menu_select_item,
    "__js__menu_select_vertical": __js__menu_select_vertical,
    "__js__menu_set_item_text": __js__menu_set_item_text,
    "__js__menu_set_item_visibility": __js__menu_set_item_visibility,
    "__js__menu_set_text_force_case": __js__menu_set_text_force_case,
    "__js__menu_toggle_choosen": __js__menu_toggle_choosen,
    "__js__menu_trasition_in": __js__menu_trasition_in,
    "__js__menu_trasition_out": __js__menu_trasition_out,
    "__js__menumanifest_destroy": __js__menumanifest_destroy,
    "__js__messagebox_get_modifier": __js__messagebox_get_modifier,
    "__js__messagebox_hide": __js__messagebox_hide,
    "__js__messagebox_hide_image": __js__messagebox_hide_image,
    "__js__messagebox_hide_image_background": __js__messagebox_hide_image_background,
    "__js__messagebox_set_button_single": __js__messagebox_set_button_single,
    "__js__messagebox_set_button_single_icon": __js__messagebox_set_button_single_icon,
    "__js__messagebox_set_buttons_icons": __js__messagebox_set_buttons_icons,
    "__js__messagebox_set_buttons_text": __js__messagebox_set_buttons_text,
    "__js__messagebox_set_image_background_color": __js__messagebox_set_image_background_color,
    "__js__messagebox_set_image_background_color_default": __js__messagebox_set_image_background_color_default,
    "__js__messagebox_set_image_sprite": __js__messagebox_set_image_sprite,
    "__js__messagebox_set_message": __js__messagebox_set_message,
    "__js__messagebox_set_title": __js__messagebox_set_title,
    "__js__messagebox_set_z_index": __js__messagebox_set_z_index,
    "__js__messagebox_show": __js__messagebox_show,
    "__js__messagebox_show_buttons_icons": __js__messagebox_show_buttons_icons,
    "__js__messagebox_use_small_size": __js__messagebox_use_small_size,
    "__js__missnotefx_play_effect": __js__missnotefx_play_effect,
    "__js__modding_choose_native_menu_option": __js__modding_choose_native_menu_option,
    "__js__modding_exit": __js__modding_exit,
    "__js__modding_get_active_menu": __js__modding_get_active_menu,
    "__js__modding_get_layout": __js__modding_get_layout,
    "__js__modding_get_loaded_weeks": __js__modding_get_loaded_weeks,
    "__js__modding_get_native_background_music": __js__modding_get_native_background_music,
    "__js__modding_get_native_menu": __js__modding_get_native_menu,
    "__js__modding_set_active_menu": __js__modding_set_active_menu,
    "__js__modding_set_exit_delay": __js__modding_set_exit_delay,
    "__js__modding_set_halt": __js__modding_set_halt,
    "__js__modding_set_menu_in_layout_placeholder": __js__modding_set_menu_in_layout_placeholder,
    "__js__modding_storage_get": __js__modding_storage_get,
    "__js__modding_storage_set": __js__modding_storage_set,
    "__js__modding_unlockdirective_create": __js__modding_unlockdirective_create,
    "__js__modding_unlockdirective_get": __js__modding_unlockdirective_get,
    "__js__modding_unlockdirective_has": __js__modding_unlockdirective_has,
    "__js__modding_unlockdirective_remove": __js__modding_unlockdirective_remove,
    "__js__modelholder_create_animsprite": __js__modelholder_create_animsprite,
    "__js__modelholder_destroy": __js__modelholder_destroy,
    "__js__modelholder_get_animlist": __js__modelholder_get_animlist,
    "__js__modelholder_get_atlas": __js__modelholder_get_atlas,
    "__js__modelholder_get_atlas_entry": __js__modelholder_get_atlas_entry,
    "__js__modelholder_get_atlas_entry2": __js__modelholder_get_atlas_entry2,
    "__js__modelholder_get_texture_resolution": __js__modelholder_get_texture_resolution,
    "__js__modelholder_get_vertex_color": __js__modelholder_get_vertex_color,
    "__js__modelholder_has_animlist": __js__modelholder_has_animlist,
    "__js__modelholder_is_invalid": __js__modelholder_is_invalid,
    "__js__modelholder_utils_is_known_extension": __js__modelholder_utils_is_known_extension,
    "__js__playerstats_add_extra_health": __js__playerstats_add_extra_health,
    "__js__playerstats_add_health": __js__playerstats_add_health,
    "__js__playerstats_add_hit": __js__playerstats_add_hit,
    "__js__playerstats_add_miss": __js__playerstats_add_miss,
    "__js__playerstats_add_penality": __js__playerstats_add_penality,
    "__js__playerstats_add_sustain": __js__playerstats_add_sustain,
    "__js__playerstats_add_sustain_delayed_hit": __js__playerstats_add_sustain_delayed_hit,
    "__js__playerstats_enable_penality_on_empty_strum": __js__playerstats_enable_penality_on_empty_strum,
    "__js__playerstats_get_accuracy": __js__playerstats_get_accuracy,
    "__js__playerstats_get_bads": __js__playerstats_get_bads,
    "__js__playerstats_get_combo_breaks": __js__playerstats_get_combo_breaks,
    "__js__playerstats_get_combo_streak": __js__playerstats_get_combo_streak,
    "__js__playerstats_get_goods": __js__playerstats_get_goods,
    "__js__playerstats_get_health": __js__playerstats_get_health,
    "__js__playerstats_get_highest_combo_streak": __js__playerstats_get_highest_combo_streak,
    "__js__playerstats_get_hits": __js__playerstats_get_hits,
    "__js__playerstats_get_iterations": __js__playerstats_get_iterations,
    "__js__playerstats_get_last_accuracy": __js__playerstats_get_last_accuracy,
    "__js__playerstats_get_last_difference": __js__playerstats_get_last_difference,
    "__js__playerstats_get_last_ranking": __js__playerstats_get_last_ranking,
    "__js__playerstats_get_maximum_health": __js__playerstats_get_maximum_health,
    "__js__playerstats_get_misses": __js__playerstats_get_misses,
    "__js__playerstats_get_notes_per_seconds": __js__playerstats_get_notes_per_seconds,
    "__js__playerstats_get_notes_per_seconds_highest": __js__playerstats_get_notes_per_seconds_highest,
    "__js__playerstats_get_penalties": __js__playerstats_get_penalties,
    "__js__playerstats_get_score": __js__playerstats_get_score,
    "__js__playerstats_get_shits": __js__playerstats_get_shits,
    "__js__playerstats_get_sicks": __js__playerstats_get_sicks,
    "__js__playerstats_is_dead": __js__playerstats_is_dead,
    "__js__playerstats_kill": __js__playerstats_kill,
    "__js__playerstats_kill_if_negative_health": __js__playerstats_kill_if_negative_health,
    "__js__playerstats_raise": __js__playerstats_raise,
    "__js__playerstats_reset": __js__playerstats_reset,
    "__js__playerstats_reset_notes_per_seconds": __js__playerstats_reset_notes_per_seconds,
    "__js__playerstats_set_health": __js__playerstats_set_health,
    "__js__psshader_destroy": __js__psshader_destroy,
    "__js__psshader_init": __js__psshader_init,
    "__js__psshader_set_uniform1f": __js__psshader_set_uniform1f,
    "__js__psshader_set_uniform1i": __js__psshader_set_uniform1i,
    "__js__psshader_set_uniform_any": __js__psshader_set_uniform_any,
    "__js__rankingcounter_add_state": __js__rankingcounter_add_state,
    "__js__rankingcounter_animation_end": __js__rankingcounter_animation_end,
    "__js__rankingcounter_animation_restart": __js__rankingcounter_animation_restart,
    "__js__rankingcounter_animation_set": __js__rankingcounter_animation_set,
    "__js__rankingcounter_hide_accuracy": __js__rankingcounter_hide_accuracy,
    "__js__rankingcounter_reset": __js__rankingcounter_reset,
    "__js__rankingcounter_set_alpha": __js__rankingcounter_set_alpha,
    "__js__rankingcounter_set_default_ranking_animation2": __js__rankingcounter_set_default_ranking_animation2,
    "__js__rankingcounter_set_default_ranking_text_animation2": __js__rankingcounter_set_default_ranking_text_animation2,
    "__js__rankingcounter_toggle_state": __js__rankingcounter_toggle_state,
    "__js__rankingcounter_use_percent_instead": __js__rankingcounter_use_percent_instead,
    "__js__roundstats_get_drawable": __js__roundstats_get_drawable,
    "__js__roundstats_hide": __js__roundstats_hide,
    "__js__roundstats_hide_nps": __js__roundstats_hide_nps,
    "__js__roundstats_reset": __js__roundstats_reset,
    "__js__roundstats_set_draw_y": __js__roundstats_set_draw_y,
    "__js__roundstats_tweenkeyframe_set_bpm": __js__roundstats_tweenkeyframe_set_bpm,
    "__js__roundstats_tweenkeyframe_set_on_beat": __js__roundstats_tweenkeyframe_set_on_beat,
    "__js__roundstats_tweenkeyframe_set_on_hit": __js__roundstats_tweenkeyframe_set_on_hit,
    "__js__roundstats_tweenkeyframe_set_on_miss": __js__roundstats_tweenkeyframe_set_on_miss,
    "__js__songplayer_changesong": __js__songplayer_changesong,
    "__js__songplayer_get_duration": __js__songplayer_get_duration,
    "__js__songplayer_get_timestamp": __js__songplayer_get_timestamp,
    "__js__songplayer_is_completed": __js__songplayer_is_completed,
    "__js__songplayer_mute": __js__songplayer_mute,
    "__js__songplayer_mute_track": __js__songplayer_mute_track,
    "__js__songplayer_pause": __js__songplayer_pause,
    "__js__songplayer_seek": __js__songplayer_seek,
    "__js__songplayer_set_volume": __js__songplayer_set_volume,
    "__js__songplayer_set_volume_track": __js__songplayer_set_volume_track,
    "__js__songprogressbar_animation_end": __js__songprogressbar_animation_end,
    "__js__songprogressbar_animation_restart": __js__songprogressbar_animation_restart,
    "__js__songprogressbar_animation_set": __js__songprogressbar_animation_set,
    "__js__songprogressbar_get_drawable": __js__songprogressbar_get_drawable,
    "__js__songprogressbar_hide_time": __js__songprogressbar_hide_time,
    "__js__songprogressbar_manual_set_position": __js__songprogressbar_manual_set_position,
    "__js__songprogressbar_manual_set_text": __js__songprogressbar_manual_set_text,
    "__js__songprogressbar_manual_update_enable": __js__songprogressbar_manual_update_enable,
    "__js__songprogressbar_set_background_color": __js__songprogressbar_set_background_color,
    "__js__songprogressbar_set_bar_back_color": __js__songprogressbar_set_bar_back_color,
    "__js__songprogressbar_set_bar_progress_color": __js__songprogressbar_set_bar_progress_color,
    "__js__songprogressbar_set_duration": __js__songprogressbar_set_duration,
    "__js__songprogressbar_set_songplayer": __js__songprogressbar_set_songplayer,
    "__js__songprogressbar_set_text_color": __js__songprogressbar_set_text_color,
    "__js__songprogressbar_set_visible": __js__songprogressbar_set_visible,
    "__js__songprogressbar_show_elapsed": __js__songprogressbar_show_elapsed,
    "__js__soundplayer_fade": __js__soundplayer_fade,
    "__js__soundplayer_get_duration": __js__soundplayer_get_duration,
    "__js__soundplayer_get_position": __js__soundplayer_get_position,
    "__js__soundplayer_has_ended": __js__soundplayer_has_ended,
    "__js__soundplayer_is_muted": __js__soundplayer_is_muted,
    "__js__soundplayer_is_playing": __js__soundplayer_is_playing,
    "__js__soundplayer_loop_enable": __js__soundplayer_loop_enable,
    "__js__soundplayer_pause": __js__soundplayer_pause,
    "__js__soundplayer_play": __js__soundplayer_play,
    "__js__soundplayer_seek": __js__soundplayer_seek,
    "__js__soundplayer_set_mute": __js__soundplayer_set_mute,
    "__js__soundplayer_set_volume": __js__soundplayer_set_volume,
    "__js__soundplayer_stop": __js__soundplayer_stop,
    "__js__sprite_blend_enable": __js__sprite_blend_enable,
    "__js__sprite_blend_set": __js__sprite_blend_set,
    "__js__sprite_center_draw_location": __js__sprite_center_draw_location,
    "__js__sprite_crop": __js__sprite_crop,
    "__js__sprite_crop_enable": __js__sprite_crop_enable,
    "__js__sprite_flip_rendered_texture": __js__sprite_flip_rendered_texture,
    "__js__sprite_flip_rendered_texture_enable_correction": __js__sprite_flip_rendered_texture_enable_correction,
    "__js__sprite_get_shader": __js__sprite_get_shader,
    "__js__sprite_get_source_size": __js__sprite_get_source_size,
    "__js__sprite_is_crop_enabled": __js__sprite_is_crop_enabled,
    "__js__sprite_is_textured": __js__sprite_is_textured,
    "__js__sprite_matrix_get_modifier": __js__sprite_matrix_get_modifier,
    "__js__sprite_matrix_reset": __js__sprite_matrix_reset,
    "__js__sprite_resize_draw_size": __js__sprite_resize_draw_size,
    "__js__sprite_set_alpha": __js__sprite_set_alpha,
    "__js__sprite_set_antialiasing": __js__sprite_set_antialiasing,
    "__js__sprite_set_draw_location": __js__sprite_set_draw_location,
    "__js__sprite_set_draw_size": __js__sprite_set_draw_size,
    "__js__sprite_set_draw_size_from_source_size": __js__sprite_set_draw_size_from_source_size,
    "__js__sprite_set_offset_frame": __js__sprite_set_offset_frame,
    "__js__sprite_set_offset_pivot": __js__sprite_set_offset_pivot,
    "__js__sprite_set_offset_source": __js__sprite_set_offset_source,
    "__js__sprite_set_offsetcolor": __js__sprite_set_offsetcolor,
    "__js__sprite_set_shader": __js__sprite_set_shader,
    "__js__sprite_set_vertex_color": __js__sprite_set_vertex_color,
    "__js__sprite_set_visible": __js__sprite_set_visible,
    "__js__sprite_set_z_index": __js__sprite_set_z_index,
    "__js__sprite_set_z_offset": __js__sprite_set_z_offset,
    "__js__sprite_trailing_enabled": __js__sprite_trailing_enabled,
    "__js__sprite_trailing_set_offsetcolor": __js__sprite_trailing_set_offsetcolor,
    "__js__sprite_trailing_set_params": __js__sprite_trailing_set_params,
    "__js__streakcounter_animation_end": __js__streakcounter_animation_end,
    "__js__streakcounter_animation_restart": __js__streakcounter_animation_restart,
    "__js__streakcounter_animation_set": __js__streakcounter_animation_set,
    "__js__streakcounter_get_drawable": __js__streakcounter_get_drawable,
    "__js__streakcounter_hide_combo_sprite": __js__streakcounter_hide_combo_sprite,
    "__js__streakcounter_reset": __js__streakcounter_reset,
    "__js__streakcounter_set_alpha": __js__streakcounter_set_alpha,
    "__js__streakcounter_set_combo_draw_location": __js__streakcounter_set_combo_draw_location,
    "__js__streakcounter_state_add": __js__streakcounter_state_add,
    "__js__streakcounter_state_toggle": __js__streakcounter_state_toggle,
    "__js__strum_animation_end": __js__strum_animation_end,
    "__js__strum_animation_restart": __js__strum_animation_restart,
    "__js__strum_disable_beat_synced_idle_and_continous": __js__strum_disable_beat_synced_idle_and_continous,
    "__js__strum_draw_sick_effect_apart": __js__strum_draw_sick_effect_apart,
    "__js__strum_enable_background": __js__strum_enable_background,
    "__js__strum_enable_sick_effect": __js__strum_enable_sick_effect,
    "__js__strum_force_key_release": __js__strum_force_key_release,
    "__js__strum_get_drawable": __js__strum_get_drawable,
    "__js__strum_get_duration": __js__strum_get_duration,
    "__js__strum_get_marker_duration": __js__strum_get_marker_duration,
    "__js__strum_get_modifier": __js__strum_get_modifier,
    "__js__strum_get_name": __js__strum_get_name,
    "__js__strum_get_press_state": __js__strum_get_press_state,
    "__js__strum_get_press_state_changes": __js__strum_get_press_state_changes,
    "__js__strum_reset": __js__strum_reset,
    "__js__strum_set_alpha": __js__strum_set_alpha,
    "__js__strum_set_alpha_background": __js__strum_set_alpha_background,
    "__js__strum_set_alpha_sick_effect": __js__strum_set_alpha_sick_effect,
    "__js__strum_set_bpm": __js__strum_set_bpm,
    "__js__strum_set_draw_offset": __js__strum_set_draw_offset,
    "__js__strum_set_extra_animation": __js__strum_set_extra_animation,
    "__js__strum_set_extra_animation_continuous": __js__strum_set_extra_animation_continuous,
    "__js__strum_set_keep_aspect_ratio_background": __js__strum_set_keep_aspect_ratio_background,
    "__js__strum_set_marker_duration_multiplier": __js__strum_set_marker_duration_multiplier,
    "__js__strum_set_note_tweenkeyframe": __js__strum_set_note_tweenkeyframe,
    "__js__strum_set_player_id": __js__strum_set_player_id,
    "__js__strum_set_scroll_direction": __js__strum_set_scroll_direction,
    "__js__strum_set_scroll_speed": __js__strum_set_scroll_speed,
    "__js__strum_set_sickeffect_size_ratio": __js__strum_set_sickeffect_size_ratio,
    "__js__strum_set_visible": __js__strum_set_visible,
    "__js__strum_state_add": __js__strum_state_add,
    "__js__strum_state_toggle": __js__strum_state_toggle,
    "__js__strum_state_toggle_background": __js__strum_state_toggle_background,
    "__js__strum_state_toggle_marker": __js__strum_state_toggle_marker,
    "__js__strum_state_toggle_notes": __js__strum_state_toggle_notes,
    "__js__strum_state_toggle_sick_effect": __js__strum_state_toggle_sick_effect,
    "__js__strum_update_draw_location": __js__strum_update_draw_location,
    "__js__strums_animation_end": __js__strums_animation_end,
    "__js__strums_animation_restart": __js__strums_animation_restart,
    "__js__strums_animation_set": __js__strums_animation_set,
    "__js__strums_decorators_add": __js__strums_decorators_add,
    "__js__strums_decorators_add2": __js__strums_decorators_add2,
    "__js__strums_decorators_get_count": __js__strums_decorators_get_count,
    "__js__strums_decorators_set_alpha": __js__strums_decorators_set_alpha,
    "__js__strums_decorators_set_scroll_speed": __js__strums_decorators_set_scroll_speed,
    "__js__strums_decorators_set_visible": __js__strums_decorators_set_visible,
    "__js__strums_disable_beat_synced_idle_and_continous": __js__strums_disable_beat_synced_idle_and_continous,
    "__js__strums_enable_background": __js__strums_enable_background,
    "__js__strums_enable_post_sick_effect_draw": __js__strums_enable_post_sick_effect_draw,
    "__js__strums_force_key_release": __js__strums_force_key_release,
    "__js__strums_get_drawable": __js__strums_get_drawable,
    "__js__strums_get_lines_count": __js__strums_get_lines_count,
    "__js__strums_get_strum_line": __js__strums_get_strum_line,
    "__js__strums_reset": __js__strums_reset,
    "__js__strums_set_alpha": __js__strums_set_alpha,
    "__js__strums_set_alpha_background": __js__strums_set_alpha_background,
    "__js__strums_set_alpha_sick_effect": __js__strums_set_alpha_sick_effect,
    "__js__strums_set_bpm": __js__strums_set_bpm,
    "__js__strums_set_draw_offset": __js__strums_set_draw_offset,
    "__js__strums_set_keep_aspect_ratio_background": __js__strums_set_keep_aspect_ratio_background,
    "__js__strums_set_marker_duration_multiplier": __js__strums_set_marker_duration_multiplier,
    "__js__strums_set_scroll_direction": __js__strums_set_scroll_direction,
    "__js__strums_set_scroll_speed": __js__strums_set_scroll_speed,
    "__js__strums_state_add": __js__strums_state_add,
    "__js__strums_state_toggle": __js__strums_state_toggle,
    "__js__strums_state_toggle_marker_and_sick_effect": __js__strums_state_toggle_marker_and_sick_effect,
    "__js__strums_state_toggle_notes": __js__strums_state_toggle_notes,
    "__js__textsprite_background_enable": __js__textsprite_background_enable,
    "__js__textsprite_background_set_color": __js__textsprite_background_set_color,
    "__js__textsprite_background_set_offets": __js__textsprite_background_set_offets,
    "__js__textsprite_background_set_size": __js__textsprite_background_set_size,
    "__js__textsprite_blend_enable": __js__textsprite_blend_enable,
    "__js__textsprite_blend_set": __js__textsprite_blend_set,
    "__js__textsprite_border_enable": __js__textsprite_border_enable,
    "__js__textsprite_border_set_color": __js__textsprite_border_set_color,
    "__js__textsprite_border_set_offset": __js__textsprite_border_set_offset,
    "__js__textsprite_border_set_size": __js__textsprite_border_set_size,
    "__js__textsprite_force_case": __js__textsprite_force_case,
    "__js__textsprite_get_draw_size": __js__textsprite_get_draw_size,
    "__js__textsprite_get_font_size": __js__textsprite_get_font_size,
    "__js__textsprite_get_shader": __js__textsprite_get_shader,
    "__js__textsprite_matrix_flip": __js__textsprite_matrix_flip,
    "__js__textsprite_matrix_get_modifier": __js__textsprite_matrix_get_modifier,
    "__js__textsprite_matrix_reset": __js__textsprite_matrix_reset,
    "__js__textsprite_set_align": __js__textsprite_set_align,
    "__js__textsprite_set_alpha": __js__textsprite_set_alpha,
    "__js__textsprite_set_antialiasing": __js__textsprite_set_antialiasing,
    "__js__textsprite_set_color": __js__textsprite_set_color,
    "__js__textsprite_set_color_rgba8": __js__textsprite_set_color_rgba8,
    "__js__textsprite_set_draw_location": __js__textsprite_set_draw_location,
    "__js__textsprite_set_font_size": __js__textsprite_set_font_size,
    "__js__textsprite_set_max_draw_size": __js__textsprite_set_max_draw_size,
    "__js__textsprite_set_maxlines": __js__textsprite_set_maxlines,
    "__js__textsprite_set_paragraph_align": __js__textsprite_set_paragraph_align,
    "__js__textsprite_set_paragraph_space": __js__textsprite_set_paragraph_space,
    "__js__textsprite_set_shader": __js__textsprite_set_shader,
    "__js__textsprite_set_text_intern": __js__textsprite_set_text_intern,
    "__js__textsprite_set_visible": __js__textsprite_set_visible,
    "__js__textsprite_set_wordbreak": __js__textsprite_set_wordbreak,
    "__js__textsprite_set_z_index": __js__textsprite_set_z_index,
    "__js__textsprite_set_z_offset": __js__textsprite_set_z_offset,
    "__js__timer_ms_gettime32_JS": __js__timer_ms_gettime32_JS,
    "__js__tweenkeyframe_add_cubic": __js__tweenkeyframe_add_cubic,
    "__js__tweenkeyframe_add_ease": __js__tweenkeyframe_add_ease,
    "__js__tweenkeyframe_add_easein": __js__tweenkeyframe_add_easein,
    "__js__tweenkeyframe_add_easeinout": __js__tweenkeyframe_add_easeinout,
    "__js__tweenkeyframe_add_easeout": __js__tweenkeyframe_add_easeout,
    "__js__tweenkeyframe_add_expo": __js__tweenkeyframe_add_expo,
    "__js__tweenkeyframe_add_interpolator": __js__tweenkeyframe_add_interpolator,
    "__js__tweenkeyframe_add_linear": __js__tweenkeyframe_add_linear,
    "__js__tweenkeyframe_add_quad": __js__tweenkeyframe_add_quad,
    "__js__tweenkeyframe_add_sin": __js__tweenkeyframe_add_sin,
    "__js__tweenkeyframe_add_steps": __js__tweenkeyframe_add_steps,
    "__js__tweenkeyframe_animate_percent": __js__tweenkeyframe_animate_percent,
    "__js__tweenkeyframe_destroy": __js__tweenkeyframe_destroy,
    "__js__tweenkeyframe_get_ids_count": __js__tweenkeyframe_get_ids_count,
    "__js__tweenkeyframe_init": __js__tweenkeyframe_init,
    "__js__tweenkeyframe_init2": __js__tweenkeyframe_init2,
    "__js__tweenkeyframe_peek_entry_by_index": __js__tweenkeyframe_peek_entry_by_index,
    "__js__tweenkeyframe_peek_value": __js__tweenkeyframe_peek_value,
    "__js__tweenkeyframe_peek_value_by_id": __js__tweenkeyframe_peek_value_by_id,
    "__js__tweenkeyframe_peek_value_by_index": __js__tweenkeyframe_peek_value_by_index,
    "__js__tweenlerp_add_cubic": __js__tweenlerp_add_cubic,
    "__js__tweenlerp_add_ease": __js__tweenlerp_add_ease,
    "__js__tweenlerp_add_easein": __js__tweenlerp_add_easein,
    "__js__tweenlerp_add_easeinout": __js__tweenlerp_add_easeinout,
    "__js__tweenlerp_add_easeout": __js__tweenlerp_add_easeout,
    "__js__tweenlerp_add_expo": __js__tweenlerp_add_expo,
    "__js__tweenlerp_add_interpolator": __js__tweenlerp_add_interpolator,
    "__js__tweenlerp_add_linear": __js__tweenlerp_add_linear,
    "__js__tweenlerp_add_quad": __js__tweenlerp_add_quad,
    "__js__tweenlerp_add_sin": __js__tweenlerp_add_sin,
    "__js__tweenlerp_add_steps": __js__tweenlerp_add_steps,
    "__js__tweenlerp_animate": __js__tweenlerp_animate,
    "__js__tweenlerp_animate_percent": __js__tweenlerp_animate_percent,
    "__js__tweenlerp_change_bounds_by_id": __js__tweenlerp_change_bounds_by_id,
    "__js__tweenlerp_change_bounds_by_index": __js__tweenlerp_change_bounds_by_index,
    "__js__tweenlerp_change_duration_by_index": __js__tweenlerp_change_duration_by_index,
    "__js__tweenlerp_destroy": __js__tweenlerp_destroy,
    "__js__tweenlerp_end": __js__tweenlerp_end,
    "__js__tweenlerp_get_elapsed": __js__tweenlerp_get_elapsed,
    "__js__tweenlerp_get_entry_count": __js__tweenlerp_get_entry_count,
    "__js__tweenlerp_init": __js__tweenlerp_init,
    "__js__tweenlerp_is_completed": __js__tweenlerp_is_completed,
    "__js__tweenlerp_mark_as_completed": __js__tweenlerp_mark_as_completed,
    "__js__tweenlerp_override_start_with_end_by_index": __js__tweenlerp_override_start_with_end_by_index,
    "__js__tweenlerp_peek_entry_by_index": __js__tweenlerp_peek_entry_by_index,
    "__js__tweenlerp_peek_value": __js__tweenlerp_peek_value,
    "__js__tweenlerp_peek_value_by_id": __js__tweenlerp_peek_value_by_id,
    "__js__tweenlerp_peek_value_by_index": __js__tweenlerp_peek_value_by_index,
    "__js__tweenlerp_restart": __js__tweenlerp_restart,
    "__js__tweenlerp_swap_bounds_by_index": __js__tweenlerp_swap_bounds_by_index,
    "__js__videoplayer_fade_audio": __js__videoplayer_fade_audio,
    "__js__videoplayer_get_duration": __js__videoplayer_get_duration,
    "__js__videoplayer_get_position": __js__videoplayer_get_position,
    "__js__videoplayer_get_sprite": __js__videoplayer_get_sprite,
    "__js__videoplayer_has_audio_track": __js__videoplayer_has_audio_track,
    "__js__videoplayer_has_ended": __js__videoplayer_has_ended,
    "__js__videoplayer_has_video_track": __js__videoplayer_has_video_track,
    "__js__videoplayer_is_muted": __js__videoplayer_is_muted,
    "__js__videoplayer_is_playing": __js__videoplayer_is_playing,
    "__js__videoplayer_loop_enable": __js__videoplayer_loop_enable,
    "__js__videoplayer_pause": __js__videoplayer_pause,
    "__js__videoplayer_play": __js__videoplayer_play,
    "__js__videoplayer_replay": __js__videoplayer_replay,
    "__js__videoplayer_seek": __js__videoplayer_seek,
    "__js__videoplayer_set_mute": __js__videoplayer_set_mute,
    "__js__videoplayer_set_volume": __js__videoplayer_set_volume,
    "__js__videoplayer_stop": __js__videoplayer_stop,
    "__js__week_change_character_camera_name": __js__week_change_character_camera_name,
    "__js__week_disable_ask_ready": __js__week_disable_ask_ready,
    "__js__week_disable_camera_bumping": __js__week_disable_camera_bumping,
    "__js__week_disable_countdown": __js__week_disable_countdown,
    "__js__week_disable_girlfriend_cry": __js__week_disable_girlfriend_cry,
    "__js__week_disable_layout_rollback": __js__week_disable_layout_rollback,
    "__js__week_disable_week_end_results": __js__week_disable_week_end_results,
    "__js__week_enable_credits_on_completed": __js__week_enable_credits_on_completed,
    "__js__week_end": __js__week_end,
    "__js__week_get_character": __js__week_get_character,
    "__js__week_get_character_count": __js__week_get_character_count,
    "__js__week_get_conductor": __js__week_get_conductor,
    "__js__week_get_current_chart_info": __js__week_get_current_chart_info,
    "__js__week_get_current_song_info": __js__week_get_current_song_info,
    "__js__week_get_dialogue": __js__week_get_dialogue,
    "__js__week_get_girlfriend": __js__week_get_girlfriend,
    "__js__week_get_healthwatcher": __js__week_get_healthwatcher,
    "__js__week_get_messagebox": __js__week_get_messagebox,
    "__js__week_get_missnotefx": __js__week_get_missnotefx,
    "__js__week_get_playerstats": __js__week_get_playerstats,
    "__js__week_get_songplayer": __js__week_get_songplayer,
    "__js__week_get_stage_layout": __js__week_get_stage_layout,
    "__js__week_override_common_folder": __js__week_override_common_folder,
    "__js__week_set_halt": __js__week_set_halt,
    "__js__week_set_ui_shader": __js__week_set_ui_shader,
    "__js__week_storage_get": __js__week_storage_get,
    "__js__week_storage_set": __js__week_storage_set,
    "__js__week_ui_get_camera": __js__week_ui_get_camera,
    "__js__week_ui_get_countdown": __js__week_ui_get_countdown,
    "__js__week_ui_get_healthbar": __js__week_ui_get_healthbar,
    "__js__week_ui_get_layout": __js__week_ui_get_layout,
    "__js__week_ui_get_rankingcounter": __js__week_ui_get_rankingcounter,
    "__js__week_ui_get_round_textsprite": __js__week_ui_get_round_textsprite,
    "__js__week_ui_get_roundstats": __js__week_ui_get_roundstats,
    "__js__week_ui_get_songprogressbar": __js__week_ui_get_songprogressbar,
    "__js__week_ui_get_streakcounter": __js__week_ui_get_streakcounter,
    "__js__week_ui_get_strums": __js__week_ui_get_strums,
    "__js__week_ui_get_strums_count": __js__week_ui_get_strums_count,
    "__js__week_ui_set_visibility": __js__week_ui_set_visibility,
    "__js__week_unlockdirective_create": __js__week_unlockdirective_create,
    "__js__week_unlockdirective_get": __js__week_unlockdirective_get,
    "__js__week_unlockdirective_has": __js__week_unlockdirective_has,
    "__js__week_unlockdirective_remove": __js__week_unlockdirective_remove,
    "__js__week_update_bpm": __js__week_update_bpm,
    "__js__week_update_speed": __js__week_update_speed,
    "__syscall_dup3": ___syscall_dup3,
    "__syscall_fcntl64": ___syscall_fcntl64,
    "__syscall_ioctl": ___syscall_ioctl,
    "__syscall_lstat64": ___syscall_lstat64,
    "__syscall_openat": ___syscall_openat,
    "__syscall_renameat": ___syscall_renameat,
    "__syscall_rmdir": ___syscall_rmdir,
    "__syscall_unlinkat": ___syscall_unlinkat,
    "_emscripten_date_now": __emscripten_date_now,
    "_emscripten_get_now_is_monotonic": __emscripten_get_now_is_monotonic,
    "_emscripten_throw_longjmp": __emscripten_throw_longjmp,
    "_gmtime_js": __gmtime_js,
    "_localtime_js": __localtime_js,
    "_mktime_js": __mktime_js,
    "_tzset_js": __tzset_js,
    "abort": _abort,
    "emscripten_get_now": _emscripten_get_now,
    "emscripten_memcpy_big": _emscripten_memcpy_big,
    "emscripten_resize_heap": _emscripten_resize_heap,
    "environ_get": _environ_get,
    "environ_sizes_get": _environ_sizes_get,
    "exit": _exit,
    "fd_close": _fd_close,
    "fd_read": _fd_read,
    "fd_seek": _fd_seek,
    "fd_write": _fd_write,
    "getTempRet0": _getTempRet0,
    "invoke_vii": invoke_vii,
    "setTempRet0": _setTempRet0,
    "strftime": _strftime,
    "system": _system
};
var asm = createWasm();
var ___wasm_call_ctors = ModuleLuaScript["___wasm_call_ctors"] = function () {
    return (___wasm_call_ctors = ModuleLuaScript["___wasm_call_ctors"] = ModuleLuaScript["asm"]["__wasm_call_ctors"]).apply(null, arguments)
};
var _lua_pushstring = ModuleLuaScript["_lua_pushstring"] = function () {
    return (_lua_pushstring = ModuleLuaScript["_lua_pushstring"] = ModuleLuaScript["asm"]["lua_pushstring"]).apply(null, arguments)
};
var _free = ModuleLuaScript["_free"] = function () {
    return (_free = ModuleLuaScript["_free"] = ModuleLuaScript["asm"]["free"]).apply(null, arguments)
};
var _lua_pushboolean = ModuleLuaScript["_lua_pushboolean"] = function () {
    return (_lua_pushboolean = ModuleLuaScript["_lua_pushboolean"] = ModuleLuaScript["asm"]["lua_pushboolean"]).apply(null, arguments)
};
var _lua_setfield = ModuleLuaScript["_lua_setfield"] = function () {
    return (_lua_setfield = ModuleLuaScript["_lua_setfield"] = ModuleLuaScript["asm"]["lua_setfield"]).apply(null, arguments)
};
var _lua_pushnil = ModuleLuaScript["_lua_pushnil"] = function () {
    return (_lua_pushnil = ModuleLuaScript["_lua_pushnil"] = ModuleLuaScript["asm"]["lua_pushnil"]).apply(null, arguments)
};
var _lua_createtable = ModuleLuaScript["_lua_createtable"] = function () {
    return (_lua_createtable = ModuleLuaScript["_lua_createtable"] = ModuleLuaScript["asm"]["lua_createtable"]).apply(null, arguments)
};
var _lua_settable = ModuleLuaScript["_lua_settable"] = function () {
    return (_lua_settable = ModuleLuaScript["_lua_settable"] = ModuleLuaScript["asm"]["lua_settable"]).apply(null, arguments)
};
var _lua_pushnumber = ModuleLuaScript["_lua_pushnumber"] = function () {
    return (_lua_pushnumber = ModuleLuaScript["_lua_pushnumber"] = ModuleLuaScript["asm"]["lua_pushnumber"]).apply(null, arguments)
};
var _lua_pushinteger = ModuleLuaScript["_lua_pushinteger"] = function () {
    return (_lua_pushinteger = ModuleLuaScript["_lua_pushinteger"] = ModuleLuaScript["asm"]["lua_pushinteger"]).apply(null, arguments)
};
var _luascript_notify_weekinit = ModuleLuaScript["_luascript_notify_weekinit"] = function () {
    return (_luascript_notify_weekinit = ModuleLuaScript["_luascript_notify_weekinit"] = ModuleLuaScript["asm"]["luascript_notify_weekinit"]).apply(null, arguments)
};
var _luascript_notify_beforeready = ModuleLuaScript["_luascript_notify_beforeready"] = function () {
    return (_luascript_notify_beforeready = ModuleLuaScript["_luascript_notify_beforeready"] = ModuleLuaScript["asm"]["luascript_notify_beforeready"]).apply(null, arguments)
};
var _luascript_notify_ready = ModuleLuaScript["_luascript_notify_ready"] = function () {
    return (_luascript_notify_ready = ModuleLuaScript["_luascript_notify_ready"] = ModuleLuaScript["asm"]["luascript_notify_ready"]).apply(null, arguments)
};
var _luascript_notify_aftercountdown = ModuleLuaScript["_luascript_notify_aftercountdown"] = function () {
    return (_luascript_notify_aftercountdown = ModuleLuaScript["_luascript_notify_aftercountdown"] = ModuleLuaScript["asm"]["luascript_notify_aftercountdown"]).apply(null, arguments)
};
var _luascript_notify_frame = ModuleLuaScript["_luascript_notify_frame"] = function () {
    return (_luascript_notify_frame = ModuleLuaScript["_luascript_notify_frame"] = ModuleLuaScript["asm"]["luascript_notify_frame"]).apply(null, arguments)
};
var _luascript_notify_roundend = ModuleLuaScript["_luascript_notify_roundend"] = function () {
    return (_luascript_notify_roundend = ModuleLuaScript["_luascript_notify_roundend"] = ModuleLuaScript["asm"]["luascript_notify_roundend"]).apply(null, arguments)
};
var _luascript_notify_weekend = ModuleLuaScript["_luascript_notify_weekend"] = function () {
    return (_luascript_notify_weekend = ModuleLuaScript["_luascript_notify_weekend"] = ModuleLuaScript["asm"]["luascript_notify_weekend"]).apply(null, arguments)
};
var _luascript_notify_diedecision = ModuleLuaScript["_luascript_notify_diedecision"] = function () {
    return (_luascript_notify_diedecision = ModuleLuaScript["_luascript_notify_diedecision"] = ModuleLuaScript["asm"]["luascript_notify_diedecision"]).apply(null, arguments)
};
var _luascript_notify_pause = ModuleLuaScript["_luascript_notify_pause"] = function () {
    return (_luascript_notify_pause = ModuleLuaScript["_luascript_notify_pause"] = ModuleLuaScript["asm"]["luascript_notify_pause"]).apply(null, arguments)
};
var _luascript_notify_weekleave = ModuleLuaScript["_luascript_notify_weekleave"] = function () {
    return (_luascript_notify_weekleave = ModuleLuaScript["_luascript_notify_weekleave"] = ModuleLuaScript["asm"]["luascript_notify_weekleave"]).apply(null, arguments)
};
var _luascript_notify_afterresults = ModuleLuaScript["_luascript_notify_afterresults"] = function () {
    return (_luascript_notify_afterresults = ModuleLuaScript["_luascript_notify_afterresults"] = ModuleLuaScript["asm"]["luascript_notify_afterresults"]).apply(null, arguments)
};
var _luascript_notify_scriptchange = ModuleLuaScript["_luascript_notify_scriptchange"] = function () {
    return (_luascript_notify_scriptchange = ModuleLuaScript["_luascript_notify_scriptchange"] = ModuleLuaScript["asm"]["luascript_notify_scriptchange"]).apply(null, arguments)
};
var _luascript_notify_pause_optionchoosen = ModuleLuaScript["_luascript_notify_pause_optionchoosen"] = function () {
    return (_luascript_notify_pause_optionchoosen = ModuleLuaScript["_luascript_notify_pause_optionchoosen"] = ModuleLuaScript["asm"]["luascript_notify_pause_optionchoosen"]).apply(null, arguments)
};
var _luascript_notify_pause_menuvisible = ModuleLuaScript["_luascript_notify_pause_menuvisible"] = function () {
    return (_luascript_notify_pause_menuvisible = ModuleLuaScript["_luascript_notify_pause_menuvisible"] = ModuleLuaScript["asm"]["luascript_notify_pause_menuvisible"]).apply(null, arguments)
};
var _luascript_notify_note = ModuleLuaScript["_luascript_notify_note"] = function () {
    return (_luascript_notify_note = ModuleLuaScript["_luascript_notify_note"] = ModuleLuaScript["asm"]["luascript_notify_note"]).apply(null, arguments)
};
var _luascript_notify_buttons = ModuleLuaScript["_luascript_notify_buttons"] = function () {
    return (_luascript_notify_buttons = ModuleLuaScript["_luascript_notify_buttons"] = ModuleLuaScript["asm"]["luascript_notify_buttons"]).apply(null, arguments)
};
var _luascript_notify_unknownnote = ModuleLuaScript["_luascript_notify_unknownnote"] = function () {
    return (_luascript_notify_unknownnote = ModuleLuaScript["_luascript_notify_unknownnote"] = ModuleLuaScript["asm"]["luascript_notify_unknownnote"]).apply(null, arguments)
};
var _luascript_notify_beat = ModuleLuaScript["_luascript_notify_beat"] = function () {
    return (_luascript_notify_beat = ModuleLuaScript["_luascript_notify_beat"] = ModuleLuaScript["asm"]["luascript_notify_beat"]).apply(null, arguments)
};
var _luascript_notify_quarter = ModuleLuaScript["_luascript_notify_quarter"] = function () {
    return (_luascript_notify_quarter = ModuleLuaScript["_luascript_notify_quarter"] = ModuleLuaScript["asm"]["luascript_notify_quarter"]).apply(null, arguments)
};
var _luascript_notify_timer_run = ModuleLuaScript["_luascript_notify_timer_run"] = function () {
    return (_luascript_notify_timer_run = ModuleLuaScript["_luascript_notify_timer_run"] = ModuleLuaScript["asm"]["luascript_notify_timer_run"]).apply(null, arguments)
};
var _luascript_notify_timersong_run = ModuleLuaScript["_luascript_notify_timersong_run"] = function () {
    return (_luascript_notify_timersong_run = ModuleLuaScript["_luascript_notify_timersong_run"] = ModuleLuaScript["asm"]["luascript_notify_timersong_run"]).apply(null, arguments)
};
var _luascript_notify_dialogue_builtin_open = ModuleLuaScript["_luascript_notify_dialogue_builtin_open"] = function () {
    return (_luascript_notify_dialogue_builtin_open = ModuleLuaScript["_luascript_notify_dialogue_builtin_open"] = ModuleLuaScript["asm"]["luascript_notify_dialogue_builtin_open"]).apply(null, arguments)
};
var _luascript_notify_dialogue_line_starts = ModuleLuaScript["_luascript_notify_dialogue_line_starts"] = function () {
    return (_luascript_notify_dialogue_line_starts = ModuleLuaScript["_luascript_notify_dialogue_line_starts"] = ModuleLuaScript["asm"]["luascript_notify_dialogue_line_starts"]).apply(null, arguments)
};
var _luascript_notify_dialogue_line_ends = ModuleLuaScript["_luascript_notify_dialogue_line_ends"] = function () {
    return (_luascript_notify_dialogue_line_ends = ModuleLuaScript["_luascript_notify_dialogue_line_ends"] = ModuleLuaScript["asm"]["luascript_notify_dialogue_line_ends"]).apply(null, arguments)
};
var _luascript_notify_after_strum_scroll = ModuleLuaScript["_luascript_notify_after_strum_scroll"] = function () {
    return (_luascript_notify_after_strum_scroll = ModuleLuaScript["_luascript_notify_after_strum_scroll"] = ModuleLuaScript["asm"]["luascript_notify_after_strum_scroll"]).apply(null, arguments)
};
var _luascript_notify_modding_menu_option_selected = ModuleLuaScript["_luascript_notify_modding_menu_option_selected"] = function () {
    return (_luascript_notify_modding_menu_option_selected = ModuleLuaScript["_luascript_notify_modding_menu_option_selected"] = ModuleLuaScript["asm"]["luascript_notify_modding_menu_option_selected"]).apply(null, arguments)
};
var _luascript_notify_modding_menu_option_choosen = ModuleLuaScript["_luascript_notify_modding_menu_option_choosen"] = function () {
    return (_luascript_notify_modding_menu_option_choosen = ModuleLuaScript["_luascript_notify_modding_menu_option_choosen"] = ModuleLuaScript["asm"]["luascript_notify_modding_menu_option_choosen"]).apply(null, arguments)
};
var _luascript_notify_modding_back = ModuleLuaScript["_luascript_notify_modding_back"] = function () {
    return (_luascript_notify_modding_back = ModuleLuaScript["_luascript_notify_modding_back"] = ModuleLuaScript["asm"]["luascript_notify_modding_back"]).apply(null, arguments)
};
var _luascript_notify_modding_exit = ModuleLuaScript["_luascript_notify_modding_exit"] = function () {
    return (_luascript_notify_modding_exit = ModuleLuaScript["_luascript_notify_modding_exit"] = ModuleLuaScript["asm"]["luascript_notify_modding_exit"]).apply(null, arguments)
};
var _luascript_notify_modding_init = ModuleLuaScript["_luascript_notify_modding_init"] = function () {
    return (_luascript_notify_modding_init = ModuleLuaScript["_luascript_notify_modding_init"] = ModuleLuaScript["asm"]["luascript_notify_modding_init"]).apply(null, arguments)
};
var _luascript_call_function = ModuleLuaScript["_luascript_call_function"] = function () {
    return (_luascript_call_function = ModuleLuaScript["_luascript_call_function"] = ModuleLuaScript["asm"]["luascript_call_function"]).apply(null, arguments)
};
var _luascript_notify_modding_event = ModuleLuaScript["_luascript_notify_modding_event"] = function () {
    return (_luascript_notify_modding_event = ModuleLuaScript["_luascript_notify_modding_event"] = ModuleLuaScript["asm"]["luascript_notify_modding_event"]).apply(null, arguments)
};
var _luascript_notify_modding_handle_custom_option = ModuleLuaScript["_luascript_notify_modding_handle_custom_option"] = function () {
    return (_luascript_notify_modding_handle_custom_option = ModuleLuaScript["_luascript_notify_modding_handle_custom_option"] = ModuleLuaScript["asm"]["luascript_notify_modding_handle_custom_option"]).apply(null, arguments)
};
var _luascript_notify_modding_window_focus = ModuleLuaScript["_luascript_notify_modding_window_focus"] = function () {
    return (_luascript_notify_modding_window_focus = ModuleLuaScript["_luascript_notify_modding_window_focus"] = ModuleLuaScript["asm"]["luascript_notify_modding_window_focus"]).apply(null, arguments)
};
var _luascript_notify_modding_window_minimized = ModuleLuaScript["_luascript_notify_modding_window_minimized"] = function () {
    return (_luascript_notify_modding_window_minimized = ModuleLuaScript["_luascript_notify_modding_window_minimized"] = ModuleLuaScript["asm"]["luascript_notify_modding_window_minimized"]).apply(null, arguments)
};
var _luascript_notify_window_size_changed = ModuleLuaScript["_luascript_notify_window_size_changed"] = function () {
    return (_luascript_notify_window_size_changed = ModuleLuaScript["_luascript_notify_window_size_changed"] = ModuleLuaScript["asm"]["luascript_notify_window_size_changed"]).apply(null, arguments)
};
var _luascript_notify_input_keyboard = ModuleLuaScript["_luascript_notify_input_keyboard"] = function () {
    return (_luascript_notify_input_keyboard = ModuleLuaScript["_luascript_notify_input_keyboard"] = ModuleLuaScript["asm"]["luascript_notify_input_keyboard"]).apply(null, arguments)
};
var _luascript_notify_input_mouse_position = ModuleLuaScript["_luascript_notify_input_mouse_position"] = function () {
    return (_luascript_notify_input_mouse_position = ModuleLuaScript["_luascript_notify_input_mouse_position"] = ModuleLuaScript["asm"]["luascript_notify_input_mouse_position"]).apply(null, arguments)
};
var _luascript_notify_input_mouse_enter = ModuleLuaScript["_luascript_notify_input_mouse_enter"] = function () {
    return (_luascript_notify_input_mouse_enter = ModuleLuaScript["_luascript_notify_input_mouse_enter"] = ModuleLuaScript["asm"]["luascript_notify_input_mouse_enter"]).apply(null, arguments)
};
var _luascript_notify_input_mouse_button = ModuleLuaScript["_luascript_notify_input_mouse_button"] = function () {
    return (_luascript_notify_input_mouse_button = ModuleLuaScript["_luascript_notify_input_mouse_button"] = ModuleLuaScript["asm"]["luascript_notify_input_mouse_button"]).apply(null, arguments)
};
var _luascript_notify_input_mouse_scroll = ModuleLuaScript["_luascript_notify_input_mouse_scroll"] = function () {
    return (_luascript_notify_input_mouse_scroll = ModuleLuaScript["_luascript_notify_input_mouse_scroll"] = ModuleLuaScript["asm"]["luascript_notify_input_mouse_scroll"]).apply(null, arguments)
};
var _malloc = ModuleLuaScript["_malloc"] = function () {
    return (_malloc = ModuleLuaScript["_malloc"] = ModuleLuaScript["asm"]["malloc"]).apply(null, arguments)
};
var _luascript_init = ModuleLuaScript["_luascript_init"] = function () {
    return (_luascript_init = ModuleLuaScript["_luascript_init"] = ModuleLuaScript["asm"]["luascript_init"]).apply(null, arguments)
};
var _luascript_eval = ModuleLuaScript["_luascript_eval"] = function () {
    return (_luascript_eval = ModuleLuaScript["_luascript_eval"] = ModuleLuaScript["asm"]["luascript_eval"]).apply(null, arguments)
};
var _luascript_drop_shared = ModuleLuaScript["_luascript_drop_shared"] = function () {
    return (_luascript_drop_shared = ModuleLuaScript["_luascript_drop_shared"] = ModuleLuaScript["asm"]["luascript_drop_shared"]).apply(null, arguments)
};
var _luascript_destroy_JS = ModuleLuaScript["_luascript_destroy_JS"] = function () {
    return (_luascript_destroy_JS = ModuleLuaScript["_luascript_destroy_JS"] = ModuleLuaScript["asm"]["luascript_destroy_JS"]).apply(null, arguments)
};
var _luascript_set_engine_globals_JS = ModuleLuaScript["_luascript_set_engine_globals_JS"] = function () {
    return (_luascript_set_engine_globals_JS = ModuleLuaScript["_luascript_set_engine_globals_JS"] = ModuleLuaScript["asm"]["luascript_set_engine_globals_JS"]).apply(null, arguments)
};
var _lua_rawseti = ModuleLuaScript["_lua_rawseti"] = function () {
    return (_lua_rawseti = ModuleLuaScript["_lua_rawseti"] = ModuleLuaScript["asm"]["lua_rawseti"]).apply(null, arguments)
};
var _saveSetjmp = ModuleLuaScript["_saveSetjmp"] = function () {
    return (_saveSetjmp = ModuleLuaScript["_saveSetjmp"] = ModuleLuaScript["asm"]["saveSetjmp"]).apply(null, arguments)
};
var ___errno_location = ModuleLuaScript["___errno_location"] = function () {
    return (___errno_location = ModuleLuaScript["___errno_location"] = ModuleLuaScript["asm"]["__errno_location"]).apply(null, arguments)
};
var _setThrew = ModuleLuaScript["_setThrew"] = function () {
    return (_setThrew = ModuleLuaScript["_setThrew"] = ModuleLuaScript["asm"]["setThrew"]).apply(null, arguments)
};
var _emscripten_stack_set_limits = ModuleLuaScript["_emscripten_stack_set_limits"] = function () {
    return (_emscripten_stack_set_limits = ModuleLuaScript["_emscripten_stack_set_limits"] = ModuleLuaScript["asm"]["emscripten_stack_set_limits"]).apply(null, arguments)
};
var _emscripten_stack_get_base = ModuleLuaScript["_emscripten_stack_get_base"] = function () {
    return (_emscripten_stack_get_base = ModuleLuaScript["_emscripten_stack_get_base"] = ModuleLuaScript["asm"]["emscripten_stack_get_base"]).apply(null, arguments)
};
var _emscripten_stack_get_end = ModuleLuaScript["_emscripten_stack_get_end"] = function () {
    return (_emscripten_stack_get_end = ModuleLuaScript["_emscripten_stack_get_end"] = ModuleLuaScript["asm"]["emscripten_stack_get_end"]).apply(null, arguments)
};
var stackSave = ModuleLuaScript["stackSave"] = function () {
    return (stackSave = ModuleLuaScript["stackSave"] = ModuleLuaScript["asm"]["stackSave"]).apply(null, arguments)
};
var stackRestore = ModuleLuaScript["stackRestore"] = function () {
    return (stackRestore = ModuleLuaScript["stackRestore"] = ModuleLuaScript["asm"]["stackRestore"]).apply(null, arguments)
};
var stackAlloc = ModuleLuaScript["stackAlloc"] = function () {
    return (stackAlloc = ModuleLuaScript["stackAlloc"] = ModuleLuaScript["asm"]["stackAlloc"]).apply(null, arguments)
};
var dynCall_ii = ModuleLuaScript["dynCall_ii"] = function () {
    return (dynCall_ii = ModuleLuaScript["dynCall_ii"] = ModuleLuaScript["asm"]["dynCall_ii"]).apply(null, arguments)
};
var dynCall_vi = ModuleLuaScript["dynCall_vi"] = function () {
    return (dynCall_vi = ModuleLuaScript["dynCall_vi"] = ModuleLuaScript["asm"]["dynCall_vi"]).apply(null, arguments)
};
var dynCall_viii = ModuleLuaScript["dynCall_viii"] = function () {
    return (dynCall_viii = ModuleLuaScript["dynCall_viii"] = ModuleLuaScript["asm"]["dynCall_viii"]).apply(null, arguments)
};
var dynCall_vii = ModuleLuaScript["dynCall_vii"] = function () {
    return (dynCall_vii = ModuleLuaScript["dynCall_vii"] = ModuleLuaScript["asm"]["dynCall_vii"]).apply(null, arguments)
};
var dynCall_iii = ModuleLuaScript["dynCall_iii"] = function () {
    return (dynCall_iii = ModuleLuaScript["dynCall_iii"] = ModuleLuaScript["asm"]["dynCall_iii"]).apply(null, arguments)
};
var dynCall_iiii = ModuleLuaScript["dynCall_iiii"] = function () {
    return (dynCall_iiii = ModuleLuaScript["dynCall_iiii"] = ModuleLuaScript["asm"]["dynCall_iiii"]).apply(null, arguments)
};
var dynCall_iiiii = ModuleLuaScript["dynCall_iiiii"] = function () {
    return (dynCall_iiiii = ModuleLuaScript["dynCall_iiiii"] = ModuleLuaScript["asm"]["dynCall_iiiii"]).apply(null, arguments)
};
var dynCall_jiji = ModuleLuaScript["dynCall_jiji"] = function () {
    return (dynCall_jiji = ModuleLuaScript["dynCall_jiji"] = ModuleLuaScript["asm"]["dynCall_jiji"]).apply(null, arguments)
};
var dynCall_iidiiii = ModuleLuaScript["dynCall_iidiiii"] = function () {
    return (dynCall_iidiiii = ModuleLuaScript["dynCall_iidiiii"] = ModuleLuaScript["asm"]["dynCall_iidiiii"]).apply(null, arguments)
};
var _asyncify_start_unwind = ModuleLuaScript["_asyncify_start_unwind"] = function () {
    return (_asyncify_start_unwind = ModuleLuaScript["_asyncify_start_unwind"] = ModuleLuaScript["asm"]["asyncify_start_unwind"]).apply(null, arguments)
};
var _asyncify_stop_unwind = ModuleLuaScript["_asyncify_stop_unwind"] = function () {
    return (_asyncify_stop_unwind = ModuleLuaScript["_asyncify_stop_unwind"] = ModuleLuaScript["asm"]["asyncify_stop_unwind"]).apply(null, arguments)
};
var _asyncify_start_rewind = ModuleLuaScript["_asyncify_start_rewind"] = function () {
    return (_asyncify_start_rewind = ModuleLuaScript["_asyncify_start_rewind"] = ModuleLuaScript["asm"]["asyncify_start_rewind"]).apply(null, arguments)
};
var _asyncify_stop_rewind = ModuleLuaScript["_asyncify_stop_rewind"] = function () {
    return (_asyncify_stop_rewind = ModuleLuaScript["_asyncify_stop_rewind"] = ModuleLuaScript["asm"]["asyncify_stop_rewind"]).apply(null, arguments)
};
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
var calledRun;
function ExitStatus(status) {
    this.name = "ExitStatus";
    this.message = "Program terminated with exit(" + status + ")";
    this.status = status
}
dependenciesFulfilled = function runCaller() {
    if (!calledRun)
        run();
    if (!calledRun)
        dependenciesFulfilled = runCaller
};
function run(args) {
    args = args || arguments_;
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
        ModuleLuaScript["calledRun"] = true;
        if (ABORT)
            return;
        initRuntime();
        if (ModuleLuaScript["onRuntimeInitialized"])
            ModuleLuaScript["onRuntimeInitialized"]();
        postRun()
    }
    if (ModuleLuaScript["setStatus"]) {
        ModuleLuaScript["setStatus"]("Running...");
        setTimeout(function () {
            setTimeout(function () {
                ModuleLuaScript["setStatus"]("")
            }, 1);
            doRun()
        }, 1)
    } else {
        doRun()
    }
}
ModuleLuaScript["run"] = run;
function exit(status, implicit) {
    EXITSTATUS = status;
    procExit(status)
}
function procExit(code) {
    EXITSTATUS = code;
    if (!keepRuntimeAlive()) {
        if (ModuleLuaScript["onExit"])
            ModuleLuaScript["onExit"](code);
        ABORT = true
    }
    quit_(code, new ExitStatus(code))
}
if (ModuleLuaScript["preInit"]) {
    if (typeof ModuleLuaScript["preInit"] == "function")
        ModuleLuaScript["preInit"] = [ModuleLuaScript["preInit"]];
    while (ModuleLuaScript["preInit"].length > 0) {
        ModuleLuaScript["preInit"].pop()()
    }
}
run();

})();
