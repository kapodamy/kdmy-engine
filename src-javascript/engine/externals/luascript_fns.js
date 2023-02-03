"use strict";


function luascript_notify_dialogue_line_starts(luascript, line_index, state_name, text) {
    // (JS only) allocate string in WASM memory, the caller deallocates it.
    let state_name_ptr = ModuleLuaScript.kdmyEngine_stringToPtr(state_name);
    let text_ptr = ModuleLuaScript.kdmyEngine_stringToPtr(text);

    return _luascriptcript_call(
        ModuleLuaScript._luascript_notify_dialogue_line_starts,
        luascript, line_index, state_name_ptr, text_ptr
    );
}

function luascript_notify_dialogue_line_ends(luascript, line_index, state_name, text) {
    // (JS only) allocate string in WASM memory, the caller deallocates it.
    let state_name_ptr = ModuleLuaScript.kdmyEngine_stringToPtr(state_name);
    let text_ptr = ModuleLuaScript.kdmyEngine_stringToPtr(text);

    return _luascriptcript_call(
        ModuleLuaScript._luascript_notify_dialogue_line_ends,
        luascript, line_index, state_name_ptr, text_ptr
    );
}

function luascript_notify_modding_menu_option_selected(luascript, menu, index, name) {
    // (JS only) allocate string in WASM memory, the caller deallocates it.
    let name_ptr = ModuleLuaScript.kdmyEngine_stringToPtr(name);
    let menu_ptr = ModuleLuaScript.kdmyEngine_obtain(menu);

    return _luascriptcript_call(
        ModuleLuaScript._luascript_notify_modding_menu_option_selected,
        luascript, menu_ptr, index, name_ptr
    );
}

function luascript_notify_modding_menu_option_choosen(luascript, menu, index, name) {
    // (JS only) allocate string in WASM memory, the caller deallocates it.
    let name_ptr = ModuleLuaScript.kdmyEngine_stringToPtr(name);
    let menu_ptr = ModuleLuaScript.kdmyEngine_obtain(menu);

    return _luascriptcript_call(
        ModuleLuaScript._luascript_notify_modding_menu_option_choosen,
        luascript, menu_ptr, index, name_ptr
    );
}

function luascript_notify_modding_back(luascript) {
    return _luascriptcript_call(
        ModuleLuaScript._luascript_notify_modding_back,
        luascript
    );
}

function luascript_notify_modding_exit(luascript) {
    let return_values_ptr = ModuleLuaScript.kdmyEngine_stringToPtr("\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0");
    let return_lua_type_ptr = return_values_ptr + 0;
    let return_lua_value_ptr = return_values_ptr + 4;

    let ret = _luascriptcript_call(
        ModuleLuaScript._luascript_notify_modding_exit,
        luascript, return_lua_type_ptr, return_lua_value_ptr
    );

    function process_return() {
        const LUA_TNIL = 0;
        const LUA_TBOOLEAN = 1;
        const LUA_TNUMBER = 3;
        const LUA_TSTRING = 4;

        const dataview = ModuleLuaScript.kdmyEngine_get_ram();
        let type = dataview.getUint32(return_lua_type_ptr);
        let value;

        switch (type) {
            case LUA_TBOOLEAN:
                value = dataview.getUint32(return_lua_value_ptr) != 0;
                break;
            case LUA_TNUMBER:
                value = dataview.getFloat64(return_lua_value_ptr);
                break;
            case LUA_TSTRING:
                value = ModuleLuaScript.kdmyEngine_ptrToString(dataview.getUint32(return_lua_value_ptr));
                break;
            case LUA_TNIL:
            default:
                value = null;
                break;
        }

        ModuleLuaScript.kdmyEngine_deallocate(return_values_ptr);
        return value;
    }

    async function process_return_async() {
        try {
            await ret;
            return process_return();
        } catch (e) {
            ModuleLuaScript.kdmyEngine_deallocate(return_values_ptr);
            throw e;
        }
    }

    if (ret instanceof Promise)
        return process_return_async();
    else
        return process_return();
}

function luascript_notify_modding_init(luascript, value) {
    const LUA_TNIL = 0;
    const LUA_TBOOLEAN = 1;
    const LUA_TNUMBER = 3;
    const LUA_TSTRING = 4;

    let value_ptr = ModuleLuaScript.kdmyEngine_stringToPtr("\0\0\0\0\0\0\0\0");
    let value_type;
    let val;

    const dataview = ModuleLuaScript.kdmyEngine_get_ram();
    switch (typeof (value)) {
        case "boolean":
            value_type = LUA_TBOOLEAN;
            val = value ? 1 : 0;
            dataview.setUint32(value_ptr, val, ModuleLuaScript.kdmyEngine_endianess);
            break;
        case "string":
            value_type = LUA_TSTRING;
            val = ModuleLuaScript.kdmyEngine_stringToPtr(value);
            dataview.setUint32(value_ptr, val, ModuleLuaScript.kdmyEngine_endianess);
            break;
        case "number":
            value_type = LUA_TNUMBER;
            dataview.setFloat64(value_ptr, value, ModuleLuaScript.kdmyEngine_endianess);
            break;
        default:
            value_type = LUA_TNIL;
            break;
    }


    let ret = _luascriptcript_call(
        ModuleLuaScript._luascript_notify_modding_init,
        luascript, value_type, value_ptr
    );

    async function process_return_async() {
        try {
            await ret;
        } finally {
            ModuleLuaScript.kdmyEngine_deallocate(value_ptr);
        }
    }

    if (ret instanceof Promise) {
        return process_return_async();
    }

    ModuleLuaScript.kdmyEngine_deallocate(value_ptr);
}

function luascript_call_function(luascript, function_name) {
    let function_name_ptr = ModuleLuaScript.kdmyEngine_stringToPtr(function_name);

    return _luascriptcript_call(
        ModuleLuaScript._luascript_call_function,
        luascript, function_name_ptr
    );
}

function luascript_eval(luascript, eval_string) {
    let eval_string_ptr = ModuleLuaScript.kdmyEngine_stringToPtr(eval_string);

    return _luascriptcript_call(
        ModuleLuaScript._luascript_call_function,
        luascript, eval_string_ptr
    );
}


function _luascriptcript_call(fn, ...args) {
    if (!args[0]) return;

    let ret = fn.apply(null, args);
    let promise = ModuleLuaScript.kdmyEngine_yieldAsync();
    return promise === undefined ? ret : promise;
}
