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
    let type_ptr = ModuleLuaScript.kdmyEngine_stringToPtr("\0\0\0\0");

    let ret = _luascriptcript_call(
        ModuleLuaScript._luascript_notify_modding_exit,
        luascript, type_ptr
    );

    function process_return(value_ptr) {
        const ramview = ModuleLuaScript.kdmyEngine_get_ram();
        let type = ramview.getUint32(type_ptr);
        let value;

        switch (type) {
            case MODDING_VALUE_TYPE_BOOLEAN:
                value = ramview.getUint32(value_ptr) != 0;
                break;
            case MODDING_VALUE_TYPE_DOUBLE:
                value = ramview.getFloat64(value_ptr);
                break;
            case MODDING_VALUE_TYPE_STRING:
                value = ModuleLuaScript.kdmyEngine_ptrToString(value_ptr);
                break;
            case MODDING_VALUE_TYPE_NULL:
            default:
                value = null;
                break;
        }

        ModuleLuaScript.kdmyEngine_deallocate(type_ptr);
        ModuleLuaScript.kdmyEngine_deallocate(value_ptr);

        return value;
    }

    async function process_return_async() {
        try {
            let value_ptr = await ret;
            return process_return(value_ptr);
        } catch (e) {
            ModuleLuaScript.kdmyEngine_deallocate(type_ptr);
            throw e;
        }
    }

    if (ret instanceof Promise)
        return process_return_async();
    else
        return process_return();
}

function luascript_notify_modding_init(luascript, value) {
    const ramview = ModuleLuaScript.kdmyEngine_get_ram();
    let arg_value;
    let arg_type;

    switch (typeof value) {
        case "boolean":
            arg_type = MODDING_VALUE_TYPE_BOOLEAN;
            arg_value = ModuleLuaScript.kdmyEngine_stringToPtr("\0\0\0\0");
            ramview.setUint32(arg_value, value ? 1 : 0, ModuleLuaScript.kdmyEngine_endianess);
            break;
        case "string":
            arg_type = MODDING_VALUE_TYPE_STRING;
            arg_value = ModuleLuaScript.kdmyEngine_stringToPtr(value);
            break;
        case "number":
            arg_type = MODDING_VALUE_TYPE_DOUBLE;
            arg_value = ModuleLuaScript.kdmyEngine_stringToPtr("\0\0\0\0\0\0\0\0");
            ramview.setFloat64(arg_value, value, ModuleLuaScript.kdmyEngine_endianess);
            break;
        default:
            arg_type = MODDING_VALUE_TYPE_NULL;
            arg_value = 0x00;
            break;
    }

    return _luascriptcript_call(
        ModuleLuaScript._luascript_notify_modding_init,
        luascript, arg_type, arg_value
    );
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

function luascript_notify_modding_event(luascript, event_name) {
    let event_name_ptr = ModuleLuaScript.kdmyEngine_stringToPtr(event_name);

    return _luascriptcript_call(
        ModuleLuaScript._luascript_notify_modding_event,
        luascript, event_name_ptr
    );
}

function luascript_notify_modding_event(luascript, event_name) {
    let event_name_ptr = ModuleLuaScript.kdmyEngine_stringToPtr(event_name);

    return _luascriptcript_call(
        ModuleLuaScript._luascript_notify_modding_event,
        luascript, event_name_ptr
    );
}

function luascript_notify_modding_handle_custom_option(luascript, option_name) {
    let option_name_ptr = ModuleLuaScript.kdmyEngine_stringToPtr(option_name);

    return _luascriptcript_call(
        ModuleLuaScript._luascript_notify_modding_handle_custom_option,
        luascript, option_name_ptr
    );
}

function luascript_notify_modding_window_focus(luascript, focused) {
    return _luascriptcript_call(
        ModuleLuaScript._luascript_notify_modding_window_focus,
        luascript, focused ? 1 : 0
    );
}

function luascript_notify_modding_window_minimized(luascript, minimized) {
    return _luascriptcript_call(
        ModuleLuaScript._luascript_notify_modding_window_minimized,
        luascript, minimized ? 1 : 0
    );
}

function luascript_notify_input_keyboard(luascript, key, scancode, is_pressed, mods) {
    return _luascriptcript_call(
        ModuleLuaScript._luascript_notify_input_keyboard,
        luascript, key, scancode, is_pressed ? 1 : 0, mods
    );
}

function luascript_notify_input_mouse_position(luascript, x, y) {
    return _luascriptcript_call(
        ModuleLuaScript._luascript_notify_input_mouse_position,
        luascript, x, y
    );
}

function luascript_notify_input_mouse_enter(Luascript, entering) {
    return _luascriptcript_call(
        ModuleLuaScript._luascript_notify_input_mouse_enter,
        Luascript, entering ? 1 : 0
    );
}

function luascript_notify_input_mouse_button(luascript, button, is_pressed, mods) {
    return _luascriptcript_call(
        ModuleLuaScript._luascript_notify_input_mouse_button,
        luascript, button, is_pressed ? 1 : 0, mods
    );
}

function luascript_notify_input_mouse_scroll(luascript, x, y) {
    return _luascriptcript_call(
        ModuleLuaScript._luascript_notify_input_mouse_scroll,
        luascript, x, y
    );
}

function luascript_notify_window_size_changed(luascript, screen_width, screen_height) {
    return _luascriptcript_call(
        ModuleLuaScript._luascript_notify_window_size_changed,
        luascript, screen_width, screen_height
    );
}

function _luascriptcript_call(fn, ...args) {
    if (!args[0]) return;

    let ret = fn.apply(null, args);
    let promise = ModuleLuaScript.kdmyEngine_yieldAsync();
    return promise === undefined ? ret : promise;
}

function luascript_helper_parse_json(L, json) {
    if (!json) {
        ModuleLuaScript._lua_pushnil(L);
        return 1;
    }

    let type = typeof(json);

    if (Array.isArray(json)) {
        luascript_helper_parse_json_array(L, json);
    } else if (type == null) {
        ModuleLuaScript._lua_pushnil(L);
    } else if (type === "boolean") {
        ModuleLuaScript._lua_pushboolean(L, json);
    } else if (type === "number") {
        ModuleLuaScript._lua_pushnumber(L, json);
    } else if (type === "string") {
        let string_ptr = ModuleLuaScript.kdmyEngine_stringToPtr(json);
        ModuleLuaScript._lua_pushstring(L, string_ptr);
        ModuleLuaScript.kdmyEngine_deallocate(string_ptr);
    } else if (type === "object") {
        luascript_helper_parse_json_object(L, json);
    }

    return 1;
}

function luascript_helper_parse_json_object(L, obj) {
    let props = Object.getOwnPropertyNames(obj);

    ModuleLuaScript._lua_createtable(L, 0, props.length);

    for (let i = 0; i < props.length; i++) {
        let value = obj[props[i]];
        let type = typeof value;
        let name_ptr = ModuleLuaScript.kdmyEngine_stringToPtr(props[i]);

        switch (type) {
            case "number":
                ModuleLuaScript._lua_pushnumber(L, value);
                ModuleLuaScript._lua_setfield(L, -2, name_ptr);
                break;
            case "string":
                let string_ptr = ModuleLuaScript.kdmyEngine_stringToPtr(value);
                ModuleLuaScript._lua_pushstring(L, string_ptr);
                ModuleLuaScript._lua_setfield(L, -2, name_ptr);
                ModuleLuaScript.kdmyEngine_deallocate(string_ptr);
                break;
            case "boolean":
                ModuleLuaScript._lua_pushboolean(L, value ? 1 : 0);
                ModuleLuaScript._lua_setfield(L, -2, name_ptr);
                break;
            case "object":
                if (Array.isArray(value)) {
                    ModuleLuaScript._lua_pushstring(L, name_ptr);
                    luascript_helper_parse_json_array(L, value);
                    ModuleLuaScript._lua_settable(L, -3);
                } else if (value == null) {
                    ModuleLuaScript._lua_pushnil(L);
                    ModuleLuaScript._lua_setfield(L, -2, name_ptr);
                } else {
                    ModuleLuaScript._lua_pushstring(L, name_ptr);
                    luascript_helper_parse_json_object(L, value);
                    ModuleLuaScript._lua_settable(L, -3);
                }
                break;
        }

        ModuleLuaScript.kdmyEngine_deallocate(name_ptr);
    }

}

function luascript_helper_parse_json_array(L, array) {
    ModuleLuaScript._lua_createtable(L, array.length, 0);

    for (let i = 0; i < array.length; i++) {
        let value = array[i];
        let type = typeof value;

        switch (type) {
            case "number":
                ModuleLuaScript._lua_pushnumber(L, value);
                break;
            case "string":
                let string_ptr = ModuleLuaScript.kdmyEngine_stringToPtr(value);
                ModuleLuaScript._lua_pushstring(L, string_ptr);
                ModuleLuaScript.kdmyEngine_deallocate(string_ptr);
                break;
            case "boolean":
                ModuleLuaScript._lua_pushboolean(L, value ? 1 : 0);
                break;
            case "object":
                if (Array.isArray(value))
                    luascript_helper_parse_json_array(L, value);
                else if (value == null)
                    ModuleLuaScript._lua_pushnil(L);
                else
                    luascript_helper_parse_json_object(L, value);
                break;
        }

        ModuleLuaScript._lua_rawseti(L, -2, i + 1);
    }
}

