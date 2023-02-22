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
