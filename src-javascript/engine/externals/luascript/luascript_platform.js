"use strict";

class LuascriptPlatform {

    //#region GLFW constants
    #GLFW_MOUSE_BUTTON_1 = 0;
    #GLFW_MOUSE_BUTTON_2 = 1;
    #GLFW_MOUSE_BUTTON_3 = 2;
    #GLFW_MOUSE_BUTTON_4 = 3;
    #GLFW_MOUSE_BUTTON_5 = 4;
    #GLFW_MOUSE_BUTTON_6 = 5;
    #GLFW_MOUSE_BUTTON_7 = 6;
    #GLFW_MOUSE_BUTTON_8 = 7;

    #GLFW_MOD_SHIFT = 0x0001;
    #GLFW_MOD_CONTROL = 0x0002;
    #GLFW_MOD_ALT = 0x0004;
    #GLFW_MOD_SUPER = 0x0008;
    #GLFW_MOD_CAPS_LOCK = 0x0010;
    #GLFW_MOD_NUM_LOCK = 0x0020;

    #GLFW_KEY_UNKNOWN = -1;
    #GLFW_KEY_SPACE = 32;
    #GLFW_KEY_APOSTROPHE = 39;
    #GLFW_KEY_COMMA = 44;
    #GLFW_KEY_MINUS = 45;
    #GLFW_KEY_PERIOD = 46;
    #GLFW_KEY_SLASH = 47;
    #GLFW_KEY_0 = 48;
    #GLFW_KEY_1 = 49;
    #GLFW_KEY_2 = 50;
    #GLFW_KEY_3 = 51;
    #GLFW_KEY_4 = 52;
    #GLFW_KEY_5 = 53;
    #GLFW_KEY_6 = 54;
    #GLFW_KEY_7 = 55;
    #GLFW_KEY_8 = 56;
    #GLFW_KEY_9 = 57;
    #GLFW_KEY_SEMICOLON = 59;
    #GLFW_KEY_EQUAL = 61;
    #GLFW_KEY_A = 65;
    #GLFW_KEY_B = 66;
    #GLFW_KEY_C = 67;
    #GLFW_KEY_D = 68;
    #GLFW_KEY_E = 69;
    #GLFW_KEY_F = 70;
    #GLFW_KEY_G = 71;
    #GLFW_KEY_H = 72;
    #GLFW_KEY_I = 73;
    #GLFW_KEY_J = 74;
    #GLFW_KEY_K = 75;
    #GLFW_KEY_L = 76;
    #GLFW_KEY_M = 77;
    #GLFW_KEY_N = 78;
    #GLFW_KEY_O = 79;
    #GLFW_KEY_P = 80;
    #GLFW_KEY_Q = 81;
    #GLFW_KEY_R = 82;
    #GLFW_KEY_S = 83;
    #GLFW_KEY_T = 84;
    #GLFW_KEY_U = 85;
    #GLFW_KEY_V = 86;
    #GLFW_KEY_W = 87;
    #GLFW_KEY_X = 88;
    #GLFW_KEY_Y = 89;
    #GLFW_KEY_Z = 90;
    #GLFW_KEY_LEFT_BRACKET = 91;
    #GLFW_KEY_BACKSLASH = 92;
    #GLFW_KEY_RIGHT_BRACKET = 93;
    #GLFW_KEY_GRAVE_ACCENT = 96;
    #GLFW_KEY_WORLD_1 = 161;
    #GLFW_KEY_WORLD_2 = 162;
    #GLFW_KEY_ESCAPE = 256;
    #GLFW_KEY_ENTER = 257;
    #GLFW_KEY_TAB = 258;
    #GLFW_KEY_BACKSPACE = 259;
    #GLFW_KEY_INSERT = 260;
    #GLFW_KEY_DELETE = 261;
    #GLFW_KEY_RIGHT = 262;
    #GLFW_KEY_LEFT = 263;
    #GLFW_KEY_DOWN = 264;
    #GLFW_KEY_UP = 265;
    #GLFW_KEY_PAGE_UP = 266;
    #GLFW_KEY_PAGE_DOWN = 267;
    #GLFW_KEY_HOME = 268;
    #GLFW_KEY_END = 269;
    #GLFW_KEY_CAPS_LOCK = 280;
    #GLFW_KEY_SCROLL_LOCK = 281;
    #GLFW_KEY_NUM_LOCK = 282;
    #GLFW_KEY_PRINT_SCREEN = 283;
    #GLFW_KEY_PAUSE = 284;
    #GLFW_KEY_F1 = 290;
    #GLFW_KEY_F2 = 291;
    #GLFW_KEY_F3 = 292;
    #GLFW_KEY_F4 = 293;
    #GLFW_KEY_F5 = 294;
    #GLFW_KEY_F6 = 295;
    #GLFW_KEY_F7 = 296;
    #GLFW_KEY_F8 = 297;
    #GLFW_KEY_F9 = 298;
    #GLFW_KEY_F10 = 299;
    #GLFW_KEY_F11 = 300;
    #GLFW_KEY_F12 = 301;
    #GLFW_KEY_F13 = 302;
    #GLFW_KEY_F14 = 303;
    #GLFW_KEY_F15 = 304;
    #GLFW_KEY_F16 = 305;
    #GLFW_KEY_F17 = 306;
    #GLFW_KEY_F18 = 307;
    #GLFW_KEY_F19 = 308;
    #GLFW_KEY_F20 = 309;
    #GLFW_KEY_F21 = 310;
    #GLFW_KEY_F22 = 311;
    #GLFW_KEY_F23 = 312;
    #GLFW_KEY_F24 = 313;
    #GLFW_KEY_F25 = 314;
    #GLFW_KEY_KP_0 = 320;
    #GLFW_KEY_KP_1 = 321;
    #GLFW_KEY_KP_2 = 322;
    #GLFW_KEY_KP_3 = 323;
    #GLFW_KEY_KP_4 = 324;
    #GLFW_KEY_KP_5 = 325;
    #GLFW_KEY_KP_6 = 326;
    #GLFW_KEY_KP_7 = 327;
    #GLFW_KEY_KP_8 = 328;
    #GLFW_KEY_KP_9 = 329;
    #GLFW_KEY_KP_DECIMAL = 330;
    #GLFW_KEY_KP_DIVIDE = 331;
    #GLFW_KEY_KP_MULTIPLY = 332;
    #GLFW_KEY_KP_SUBTRACT = 333;
    #GLFW_KEY_KP_ADD = 334;
    #GLFW_KEY_KP_ENTER = 335;
    #GLFW_KEY_KP_EQUAL = 336;
    #GLFW_KEY_LEFT_SHIFT = 340;
    #GLFW_KEY_LEFT_CONTROL = 341;
    #GLFW_KEY_LEFT_ALT = 342;
    #GLFW_KEY_LEFT_SUPER = 343;
    #GLFW_KEY_RIGHT_SHIFT = 344;
    #GLFW_KEY_RIGHT_CONTROL = 345;
    #GLFW_KEY_RIGHT_ALT = 346;
    #GLFW_KEY_RIGHT_SUPER = 347;
    #GLFW_KEY_MENU = 348;
    //#endregion


    #luascript_instances = arraylist_init2(2);
    #last_mouse_position_x = NaN;
    #last_mouse_position_y = NaN;
    #last_window_focused = false;
    #last_window_minimized = false;
    #title_was_changed = false;
    #last_resolution_changes = 1;


    InitializeCallbacks() {
        const THIS = this;

        window.addEventListener("keydown", function (evt) {
            if (evt.repeat) return;// GLFW_REPEAT (ignored)
            THIS.#NotifyKeyboard(evt, 1);// GLFW_PRESS

            /*
            switch (event.code) {
                case "Escape":
                case "Backspace":
                case "Tab":
                case "OSLeft":
                case "MetaLeft":
                case "OSRight":
                case "MetaRight":
                    event.preventDefault();
            }
            */
        }, true);
        window.addEventListener("keyup", function (event) {
            THIS.#NotifyKeyboard(event, 0); // GLFW_RELEASE
        }, true);
        pvr_context._html5canvas.addEventListener("mousemove", function (evt) {
            THIS.#CallbackMouseMove(evt);
        }, true);
        pvr_context._html5canvas.addEventListener("mousedown", function (evt) {
            THIS.#CallbackMouseMove(evt);
            THIS.#NotifyMouseButton(evt, 1); // GLFW_PRESS
        }, true);
        pvr_context._html5canvas.addEventListener("mouseup", function (evt) {
            THIS.#CallbackMouseMove(evt);
            THIS.#NotifyMouseButton(evt, 0); // GLFW_RELEASE
        }, true);
        pvr_context._html5canvas.addEventListener('wheel', function (evt) {
            THIS.#NotifyMouseWheel(evt);
        }, true);
        pvr_context._html5canvas.addEventListener('mouseenter', function () {
            THIS.#NotifyMouseHover(1);
        }, true);
        pvr_context._html5canvas.addEventListener('mouseleave', function () {
            THIS.#NotifyMouseHover(0);
        }, true);
    }

    async AddLuascript(luascript) {
        if (luascript == null || arraylist_has(this.#luascript_instances, luascript)) return;
        arraylist_add(this.#luascript_instances, luascript);

        // notify input states if they do not have default values
        if (!Number.isNaN(this.#last_mouse_position_x) && !Number.isNaN(this.#last_mouse_position_y)) {
            await luascript_notify_input_mouse_position(luascript, this.#last_mouse_position_x, this.#last_mouse_position_y);
        }
        if (!this.#last_window_focused) await luascript_notify_modding_window_focus(luascript, this.#last_window_focused);
        if (this.#last_window_minimized) await luascript_notify_modding_window_focus(luascript, this.#last_window_minimized);
    }

    RemoveLuascript(luascript) {
        arraylist_remove(this.#luascript_instances, luascript);
    }

    async PollWindowState() {
        let focused = document.activeElement == pvr_context._html5canvas;
        let minimized = document.hidden;
        let resolution_changes = pvr_context.resolution_changes;

        let focused_updated = focused != this.#last_window_focused;
        let minimize_updated = minimized != this.#last_window_minimized;
        let resolution_updated = resolution_changes != this.#last_resolution_changes;

        if (focused_updated || minimize_updated || resolution_updated) {
            const array = this.#luascript_instances.array;
            const size = this.#luascript_instances.size;
            for (let i = 0; i < size; i++) {
                const luascript = array[i];
                if (focused_updated) await luascript_notify_modding_window_focus(luascript, focused);
                if (minimize_updated) await luascript_notify_modding_window_minimized(luascript, minimized);
                if (resolution_updated) await luascript_notify_window_size_changed(luascript, pvr_context.screen_width, pvr_context.screen_height);
            }

            this.#last_window_focused = focused;
            this.#last_window_minimized = minimized;
        }
    }


    RequestWindowAttention() {
        alert("LuascriptPlatform::RequestWindowAttention()");
    }

    ChangeWindowTitle(title, changed_from_modding_context) {
        document.title = title ?? pvr_context.native_window_title;

        if (!changed_from_modding_context && title != null)
            this.#title_was_changed = true;
        else if (title == null)
            this.#title_was_changed = false;
    }

    RestoreWindowTitleIfChanged() {
        if (!this.#title_was_changed) return;
        document.title = pvr_context.native_window_title;
    }

    GetLanguage() {
        return window.navigator.language;
    }

    GetUsername() {
        return null;//navigator.userAgent;
    }

    GetCmdargs() {
        let query = location.search;
        if (query.length > 0 && query[0] == '?') query = query.substring(1);

        let name = location.pathname;
        let idx = name.lastIndexOf('/');
        if (idx >= 0) name = name.substring(idx + 1);

        /**@type {string[]}*/let cmdargs = new Array();

        cmdargs.push(name ? name : (location.pathname ? location.pathname : location.href));

        for (const [key, value] of new URLSearchParams(query)) {
            cmdargs.push(key);
            cmdargs.push(value);
        }

        return cmdargs;
    }

    OpenWWWLink(url) {
        if (!url) return;
        if (url.startsWith("javascript:") || url.startsWith("blob:")) {
            console.warn("LuascriptPlatform::OpenWWWLink() attempt to open a harmful url", url);
            return;
        }
        window.open(url, "_blank", "noopener,noreferrer");
    }

    GetScreenSize(screen_size) {
        screen_size[0] = pvr_context.screen_width;
        screen_size[1] = pvr_context.screen_height;
    }

    Exit(exit_code) {
        let html = "<html><body><h1>Exit Code: " + exit_code + "</h1></body></html>";
        html = btoa('base64');
        document.location = "data:text/html;base64," + html;
    }


    async #NotifyKeyboard(/**@type {KeyboardEvent}*/event, is_pressed) {
        function hashCode(s) {
            let h = 0;
            for (let i = 0; i < s.length; i++) {
                h = Math.imul(31, h) + s.charCodeAt(i) | 0;
            }
            return h;
        }

        const scancode = hashCode(event.code);
        let key = this.#GLFW_KEY_UNKNOWN;
        switch (event.code) {
            case "Space": key = this.#GLFW_KEY_SPACE;
            case "Quote": key = this.#GLFW_KEY_APOSTROPHE;
            case "Comma": key = this.#GLFW_KEY_COMMA;
            case "Minus": key = this.#GLFW_KEY_MINUS;
            case "Period": key = this.#GLFW_KEY_PERIOD;
            case "Slash": key = this.#GLFW_KEY_SLASH;
            case "Digit0": key = this.#GLFW_KEY_0;
            case "Digit1": key = this.#GLFW_KEY_1;
            case "Digit2": key = this.#GLFW_KEY_2;
            case "Digit3": key = this.#GLFW_KEY_3;
            case "Digit4": key = this.#GLFW_KEY_4;
            case "Digit5": key = this.#GLFW_KEY_5;
            case "Digit6": key = this.#GLFW_KEY_6;
            case "Digit7": key = this.#GLFW_KEY_7;
            case "Digit8": key = this.#GLFW_KEY_8;
            case "Digit9": key = this.#GLFW_KEY_9;
            case "Semicolon": key = this.#GLFW_KEY_SEMICOLON;
            case "Equal": key = this.#GLFW_KEY_EQUAL;
            case "KeyA": key = this.#GLFW_KEY_A;
            case "KeyB": key = this.#GLFW_KEY_B;
            case "KeyC": key = this.#GLFW_KEY_C;
            case "KeyD": key = this.#GLFW_KEY_D;
            case "KeyE": key = this.#GLFW_KEY_E;
            case "KeyF": key = this.#GLFW_KEY_F;
            case "KeyG": key = this.#GLFW_KEY_G;
            case "KeyH": key = this.#GLFW_KEY_H;
            case "KeyI": key = this.#GLFW_KEY_I;
            case "KeyJ": key = this.#GLFW_KEY_J;
            case "KeyK": key = this.#GLFW_KEY_K;
            case "KeyL": key = this.#GLFW_KEY_L;
            case "KeyM": key = this.#GLFW_KEY_M;
            case "KeyN": key = this.#GLFW_KEY_N;
            case "KeyO": key = this.#GLFW_KEY_O;
            case "KeyP": key = this.#GLFW_KEY_P;
            case "KeyQ": key = this.#GLFW_KEY_Q;
            case "KeyR": key = this.#GLFW_KEY_R;
            case "KeyS": key = this.#GLFW_KEY_S;
            case "KeyT": key = this.#GLFW_KEY_T;
            case "KeyU": key = this.#GLFW_KEY_U;
            case "KeyV": key = this.#GLFW_KEY_V;
            case "KeyW": key = this.#GLFW_KEY_W;
            case "KeyX": key = this.#GLFW_KEY_X;
            case "KeyY": key = this.#GLFW_KEY_Y;
            case "KeyZ": key = this.#GLFW_KEY_Z;
            case "BracketLeft": key = this.#GLFW_KEY_LEFT_BRACKET;
            case "Backslash": key = this.#GLFW_KEY_BACKSLASH;
            case "BracketRight": key = this.#GLFW_KEY_RIGHT_BRACKET;
            case "Backquote": key = this.#GLFW_KEY_GRAVE_ACCENT;
            case "Escape": key = this.#GLFW_KEY_ESCAPE;
            case "Enter": key = this.#GLFW_KEY_ENTER;
            case "Tab": key = this.#GLFW_KEY_TAB;
            case "Backspace": key = this.#GLFW_KEY_BACKSPACE;
            case "Insert": key = this.#GLFW_KEY_INSERT;
            case "Delete": key = this.#GLFW_KEY_DELETE;
            case "ArrowRight": key = this.#GLFW_KEY_RIGHT;
            case "Arrowleft": key = this.#GLFW_KEY_LEFT;
            case "Arrowdown": key = this.#GLFW_KEY_DOWN;
            case "Arrowup": key = this.#GLFW_KEY_UP;
            case "PageUp": key = this.#GLFW_KEY_PAGE_UP;
            case "PageDown": key = this.#GLFW_KEY_PAGE_DOWN;
            case "Home": key = this.#GLFW_KEY_HOME;
            case "End": key = this.#GLFW_KEY_END;
            case "CapsLock": key = this.#GLFW_KEY_CAPS_LOCK;
            case "ScrollLock": key = this.#GLFW_KEY_SCROLL_LOCK;
            case "NumLock": key = this.#GLFW_KEY_NUM_LOCK;
            case "NumLock": key = this.#GLFW_KEY_PRINT_SCREEN;
            case "Pause": key = this.#GLFW_KEY_PAUSE;
            case "F1": key = this.#GLFW_KEY_F1;
            case "F2": key = this.#GLFW_KEY_F2;
            case "F3": key = this.#GLFW_KEY_F3;
            case "F4": key = this.#GLFW_KEY_F4;
            case "F5": key = this.#GLFW_KEY_F5;
            case "F6": key = this.#GLFW_KEY_F6;
            case "F7": key = this.#GLFW_KEY_F7;
            case "F8": key = this.#GLFW_KEY_F8;
            case "F9": key = this.#GLFW_KEY_F9;
            case "F10": key = this.#GLFW_KEY_F10;
            case "F11": key = this.#GLFW_KEY_F11;
            case "F12": key = this.#GLFW_KEY_F12;
            case "F13": key = this.#GLFW_KEY_F13;
            case "F14": key = this.#GLFW_KEY_F14;
            case "F15": key = this.#GLFW_KEY_F15;
            case "F16": key = this.#GLFW_KEY_F16;
            case "F17": key = this.#GLFW_KEY_F17;
            case "F18": key = this.#GLFW_KEY_F18;
            case "F19": key = this.#GLFW_KEY_F19;
            case "F20": key = this.#GLFW_KEY_F20;
            case "F21": key = this.#GLFW_KEY_F21;
            case "F22": key = this.#GLFW_KEY_F22;
            case "F23": key = this.#GLFW_KEY_F23;
            case "F24": key = this.#GLFW_KEY_F24;
            case "Numpad0": key = this.#GLFW_KEY_KP_0;
            case "Numpad1": key = this.#GLFW_KEY_KP_1;
            case "Numpad2": key = this.#GLFW_KEY_KP_2;
            case "Numpad3": key = this.#GLFW_KEY_KP_3;
            case "Numpad4": key = this.#GLFW_KEY_KP_4;
            case "Numpad5": key = this.#GLFW_KEY_KP_5;
            case "Numpad6": key = this.#GLFW_KEY_KP_6;
            case "Numpad7": key = this.#GLFW_KEY_KP_7;
            case "Numpad8": key = this.#GLFW_KEY_KP_8;
            case "Numpad9": key = this.#GLFW_KEY_KP_9;
            case "NumpadDecimal": key = this.#GLFW_KEY_KP_DECIMAL;
            case "NumpadDivide": key = this.#GLFW_KEY_KP_DIVIDE;
            case "NumpadMultiply": key = this.#GLFW_KEY_KP_MULTIPLY;
            case "NumpadSubtract": key = this.#GLFW_KEY_KP_SUBTRACT;
            case "NumpadAdd": key = this.#GLFW_KEY_KP_ADD;
            case "ShiftLeft": key = this.#GLFW_KEY_LEFT_SHIFT;
            case "ControlLeft": key = this.#GLFW_KEY_LEFT_CONTROL;
            case "AltLeft": key = this.#GLFW_KEY_LEFT_ALT;
            case "OSLeft": case "MetaLeft": key = this.#GLFW_KEY_LEFT_SUPER;
            case "OSRight": case "MetaRight": key = this.#GLFW_KEY_RIGHT_SUPER;
            case "NumpadEnter": key = this.#GLFW_KEY_KP_ENTER;
            case "NumpadEqual": key = this.#GLFW_KEY_KP_EQUAL;
            case "ShiftRight": key = this.#GLFW_KEY_RIGHT_SHIFT;
            case "ControlRight": key = this.#GLFW_KEY_RIGHT_CONTROL;
            case "AltRight": key = this.#GLFW_KEY_RIGHT_ALT;
            case "ContextMenu": key = this.#GLFW_KEY_MENU;
            default:
                // missing GLFW_KEY_WORLD_1 GLFW_KEY_WORLD_2 GLFW_KEY_F25
                break;
        };

        let mods = 0x00;
        if (event.shiftKey) mods |= this.#GLFW_MOD_SHIFT;
        if (event.ctrlKey) mods |= this.#GLFW_MOD_CONTROL;
        if (event.altKey) mods |= this.#GLFW_MOD_ALT;
        if (event.metaKey) mods |= this.#GLFW_MOD_SUPER;
        if (event.getModifierState("CapsLock")) mods |= this.#GLFW_MOD_CAPS_LOCK;
        if (event.getModifierState("NumLock")) mods |= this.#GLFW_MOD_NUM_LOCK;

        // special keys
        switch (key) {
            case this.#GLFW_KEY_F11:
            case this.#GLFW_KEY_F12:
            case this.#GLFW_KEY_KP_SUBTRACT:
            case this.#GLFW_KEY_KP_ADD:
                return;
        }

        const array = this.#luascript_instances.array;
        const size = this.#luascript_instances.size;
        for (let i = 0; i < size; i++) await luascript_notify_input_keyboard(array[i], key, scancode, is_pressed, mods);
    }

    async #CallbackMouseMove(event) {
        let rect;
        if (pvr_context._html5canvas instanceof OffscreenCanvas)
            rect = document.body.getBoundingClientRect();
        else
            rect = pvr_context._html5canvas.getBoundingClientRect();

        let position_x = event.pageX - (rect.left + window.scrollX);
        let position_y = event.pageY - (rect.top + window.screenTop);

        if (position_x == this.#last_mouse_position_x && position_y == this.#last_mouse_position_y) {
            return;
        }

        this.#last_mouse_position_x = position_x;
        this.#last_mouse_position_y = position_y;

        const array = this.#luascript_instances.array;
        const size = this.#luascript_instances.size;
        for (let i = 0; i < size; i++) await luascript_notify_input_mouse_position(array[i], position_x, position_y);
    }

    async #NotifyMouseButton(event, is_pressed) {
        if (event.target != pvr_context._html5canvas) return;

        let button = event.button;

        if (button == 1)
            button = this.#GLFW_MOUSE_BUTTON_3;
        else if (button == 2)
            button = this.#GLFW_MOUSE_BUTTON_2;

        let mods = 0x00;
        if (event.shiftKey) mods |= this.#GLFW_MOD_SHIFT;
        if (event.ctrlKey) mods |= this.#GLFW_MOD_CONTROL;
        if (event.altKey) mods |= this.#GLFW_MOD_ALT;
        if (event.metaKey) mods |= this.#GLFW_MOD_SUPER;
        if (event.getModifierState("CapsLock")) mods |= this.#GLFW_MOD_CAPS_LOCK;
        if (event.getModifierState("NumLock")) mods |= this.#GLFW_MOD_NUM_LOCK;

        const array = this.#luascript_instances.array;
        const size = this.#luascript_instances.size;
        for (let i = 0; i < size; i++) await luascript_notify_input_mouse_button(array[i], button, is_pressed, mods);
    }

    async #NotifyMouseHover(entering) {
        const array = this.#luascript_instances.array;
        const size = this.#luascript_instances.size;
        for (let i = 0; i < size; i++) await luascript_notify_input_mouse_enter(array[i], entering);
    }

    async #NotifyMouseWheel(evt) {
        this.#CallbackMouseMove(evt);

        const deltaX = evt.deltaX;
        const deltaY = evt.deltaY;

        const array = this.#luascript_instances.array;
        const size = this.#luascript_instances.size;
        for (let i = 0; i < size; i++) await luascript_notify_input_mouse_scroll(array[i], deltaX, deltaY);

        //evt.preventDefault();
    }

}

const luascriptplatform = new LuascriptPlatform();
