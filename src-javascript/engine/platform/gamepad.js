"use strict";

const GAMEPAD_A = 1 << 0;
const GAMEPAD_B = 1 << 1;
const GAMEPAD_X = 1 << 2;
const GAMEPAD_Y = 1 << 3;

const GAMEPAD_DPAD_UP = 1 << 4;
const GAMEPAD_DPAD_DOWN = 1 << 5;
const GAMEPAD_DPAD_RIGHT = 1 << 6;
const GAMEPAD_DPAD_LEFT = 1 << 7;

const GAMEPAD_START = 1 << 8;
const GAMEPAD_SELECT = 1 << 9;

const GAMEPAD_TRIGGER_LEFT = 1 << 10;
const GAMEPAD_TRIGGER_RIGHT = 1 << 11;
const GAMEPAD_BUMPER_LEFT = 1 << 12;
const GAMEPAD_BUMPER_RIGHT = 1 << 13;

const GAMEPAD_APAD_UP = 1 << 14;
const GAMEPAD_APAD_DOWN = 1 << 15;
const GAMEPAD_APAD_RIGHT = 1 << 16;
const GAMEPAD_APAD_LEFT = 1 << 17;

const GAMEPAD_DPAD2_UP = 1 << 18;
const GAMEPAD_DPAD2_DOWN = 1 << 19;
const GAMEPAD_DPAD2_RIGHT = 1 << 20;
const GAMEPAD_DPAD2_LEFT = 1 << 21;

const GAMEPAD_DPAD3_UP = 1 << 22;
const GAMEPAD_DPAD3_DOWN = 1 << 23;
const GAMEPAD_DPAD3_RIGHT = 1 << 24;
const GAMEPAD_DPAD3_LEFT = 1 << 25;

const GAMEPAD_DPAD4_UP = 1 << 26;
const GAMEPAD_DPAD4_DOWN = 1 << 27;
const GAMEPAD_DPAD4_RIGHT = 1 << 28;
const GAMEPAD_DPAD4_LEFT = 1 << 29;

const GAMEPAD_BACK = 1 << 30;


// combinations
const GAMEPAD_AD_UP = GAMEPAD_DPAD_UP | GAMEPAD_APAD_UP;
const GAMEPAD_AD_DOWN = GAMEPAD_DPAD_DOWN | GAMEPAD_APAD_DOWN;
const GAMEPAD_AD_RIGHT = GAMEPAD_DPAD_RIGHT | GAMEPAD_APAD_RIGHT;
const GAMEPAD_AD_LEFT = GAMEPAD_DPAD_LEFT | GAMEPAD_APAD_LEFT;
const GAMEPAD_AD = GAMEPAD_AD_DOWN | GAMEPAD_AD_UP | GAMEPAD_AD_LEFT | GAMEPAD_AD_RIGHT;
const GAMEPAD_T_LR = GAMEPAD_TRIGGER_LEFT | GAMEPAD_TRIGGER_RIGHT;
const GAMEPAD_B_LR = GAMEPAD_BUMPER_LEFT | GAMEPAD_BUMPER_RIGHT;
const GAMEPAD_DALL_UP = GAMEPAD_DPAD_UP | GAMEPAD_DPAD2_UP | GAMEPAD_DPAD3_UP | GAMEPAD_DPAD4_UP;
const GAMEPAD_DALL_DOWN = GAMEPAD_DPAD_DOWN | GAMEPAD_DPAD2_DOWN | GAMEPAD_DPAD3_DOWN | GAMEPAD_DPAD4_DOWN;
const GAMEPAD_DALL_RIGHT = GAMEPAD_DPAD_RIGHT | GAMEPAD_DPAD2_RIGHT | GAMEPAD_DPAD3_RIGHT | GAMEPAD_DPAD4_RIGHT;
const GAMEPAD_DALL_LEFT = GAMEPAD_DPAD_LEFT | GAMEPAD_DPAD2_LEFT | GAMEPAD_DPAD3_LEFT | GAMEPAD_DPAD4_LEFT;

var GAMEPAD_ANALOG_DEAD_ZONE = 0x40;// considered "pressed" any analog input if 25% or more

/**
 * @typedef {object} GamepadKDY
 * @property {number} buttons
 * @property {number} last_buttons
 * @property {number} clear_mask
 * @property {number} delay_duration
 * @property {number} delay_timestamp
 * @property {number} controller_index
 * @property {maple_device_t} device
 * @property {bool} mix_keyboard
 */


/** @type {Map<GamepadKDY, object>} */
const GAMEPAD_POOL = new Map();
const GAMEPAD_BUTTONS_MAP_FROM = [
    CONT_A, CONT_B, CONT_X, CONT_Y,
    CONT_DPAD_UP, CONT_DPAD_DOWN, CONT_DPAD_LEFT, CONT_DPAD_RIGHT,
    CONT_START, CONT_D,
    CONT_BACK_OR_SELECT,
    CONT_C, CONT_Z,
    CONT_DPAD2_UP, CONT_DPAD2_DOWN, CONT_DPAD2_LEFT, CONT_DPAD2_RIGHT,
    CONT_DPAD3_UP, CONT_DPAD3_DOWN, CONT_DPAD3_LEFT, CONT_DPAD3_RIGHT,
    CONT_DPAD4_UP, CONT_DPAD4_DOWN, CONT_DPAD4_LEFT, CONT_DPAD4_RIGHT,
];
const GAMEPAD_BUTTONS_MAP_TO = [
    GAMEPAD_A, GAMEPAD_B, GAMEPAD_X, GAMEPAD_Y,
    GAMEPAD_DPAD_UP, GAMEPAD_DPAD_DOWN, GAMEPAD_DPAD_LEFT, GAMEPAD_DPAD_RIGHT,
    GAMEPAD_START, GAMEPAD_SELECT,
    GAMEPAD_BACK,
    GAMEPAD_BUMPER_LEFT, GAMEPAD_BUMPER_RIGHT,
    GAMEPAD_DPAD2_UP, GAMEPAD_DPAD2_DOWN, GAMEPAD_DPAD2_LEFT, GAMEPAD_DPAD2_RIGHT,
    GAMEPAD_DPAD3_UP, GAMEPAD_DPAD3_DOWN, GAMEPAD_DPAD3_LEFT, GAMEPAD_DPAD3_RIGHT,
    GAMEPAD_DPAD4_UP, GAMEPAD_DPAD4_DOWN, GAMEPAD_DPAD4_LEFT, GAMEPAD_DPAD4_RIGHT,
];


function gamepad_init(controller_device_index) {
    return gamepad_init2(controller_device_index, 1);
}

function gamepad_init2(controller_device_index, mix_keyboard) {
    /** @type {GamepadKDY} */
    let gamepad = {
        buttons: 0x00,
        last_buttons: 0x00,
        clear_mask: 0x00,
        controller_index: controller_device_index,
        delay_duration: 0,
        delay_timestamp: 0,
        device: null,
        mix_keyboard: controller_device_index < 1 && mix_keyboard
    };
    GAMEPAD_POOL.set(gamepad, null);

    // check if should use any controller attached
    if (controller_device_index < 0) return gamepad;

    // pick dreamcast gamepad if connected
    gamepad_internal_pick_maple_device(gamepad);

    return gamepad;
}

function gamepad_destroy(/**@type {GamepadKDY}*/gamepad) {
    GAMEPAD_POOL.delete(gamepad);
    gamepad = undefined;
}

function gamepad_is_dettached(/**@type {GamepadKDY}*/gamepad) {
    if (gamepad.mix_keyboard) return 0;
    return gamepad.device ? (!gamepad.device.valid) : 0;
}

function gamepad_pick(/**@type {GamepadKDY}*/gamepad, pick_on_start_press) {
    if (gamepad.device) {
        if (gamepad.device.valid) return 1;
    } else if (gamepad_internal_pick_maple_device(gamepad)) return 1;
    if (!pick_on_start_press) return 0;

    //
    // Pick another maple controller connected to the dreamcast
    // but also check if not used by any other gamepad instance
    // the user must press the START button to detect them
    //
    let maple_devices_found = maple_enum_count();
    for (let i = 0; i < maple_devices_found; i++) {
        let device = maple_enum_type(i, MAPLE_FUNC_CONTROLLER);
        if (!device || !device.valid) continue;

        let in_use = 0;
        for (let j of GAMEPAD_POOL.keys()) {
            if (j.device == device) {
                in_use = 1;
                break;
            }
        }

        if (in_use) continue;

        // check if the user is pressing the button START
        let status = maple_dev_status(device);
        if (status.buttons & CONT_START) {
            gamepad.device = device;
            return 1;
        }
    }

    return 0;
}

function gamepad_get_controller_index(/**@type {GamepadKDY}*/gamepad) {
    return gamepad.controller_index;
}

function gamepad_set_buttons_delay(/**@type {GamepadKDY}*/gamepad, delay_time) {
    gamepad.delay_duration = delay_time;
    gamepad.delay_timestamp = 0;
}

function gamepad_enforce_buttons_delay(/**@type {GamepadKDY}*/gamepad) {
    gamepad.delay_timestamp = timer_ms_gettime64() + gamepad.delay_duration;
}



function gamepad_clear_buttons(/**@type {GamepadKDY}*/gamepad) {
    gamepad.clear_mask = 0x00;
    gamepad_internal_update_state(gamepad);
    gamepad.clear_mask = ~gamepad.buttons;
    gamepad.buttons = 0x00;
}

function gamepad_clear_all_gamepads() {
    for (let [gamepad, id] of GAMEPAD_POOL) {
        gamepad.clear_mask = 0x00;
        gamepad_internal_update_state(gamepad);
        gamepad.clear_mask = ~gamepad.buttons;
        //gamepad.last_buttons = gamepad.buttons;
        gamepad.buttons = 0x00;
    }
}

function gamepad_get_pressed(/**@type {GamepadKDY}*/gamepad) {
    gamepad_internal_update_state(gamepad);
    return gamepad.buttons;
}

function gamepad_get_last_pressed(/**@type {GamepadKDY}*/gamepad) {
    return gamepad.buttons;
}

function gamepad_get_managed_presses(/**@type {GamepadKDY}*/gamepad, update_state, buttons) {
    if (update_state) gamepad_internal_update_state(gamepad);
    if (gamepad.buttons == gamepad.last_buttons) return 0;

    buttons[0] = gamepad.buttons;
    gamepad.last_buttons = gamepad.buttons;
    return 1;
}

function gamepad_has_pressed(/**@type {GamepadKDY}*/gamepad, button_flags) {
    gamepad_internal_update_state(gamepad);

    return (gamepad.buttons & button_flags)/* != 0*/;
}

function gamepad_has_pressed_delayed(/**@type {GamepadKDY}*/gamepad, button_flags) {
    gamepad_internal_update_state(gamepad);

    let pressed = gamepad.buttons & button_flags;
    if (pressed == 0x00 || gamepad.delay_duration < 1) return 0x00;

    let now = timer_ms_gettime64();
    if (now < gamepad.delay_timestamp) return 0x00;

    gamepad.delay_timestamp = now + gamepad.delay_duration;
    return pressed;
}


function gamepad_internal_pick_maple_device(/**@type {GamepadKDY}*/gamepad) {
    let index = 0;
    let maple_devices_found = maple_enum_count();

    // find the desired controller number/index/position
    for (let i = 0; i < maple_devices_found; i++) {
        let device = maple_enum_type(i, MAPLE_FUNC_CONTROLLER);
        if (!device) continue;

        if (index == gamepad.controller_index) {
            gamepad.device = device;
            return 1;
        }

        index++;
    }

    return 0;
}

function gamepad_internal_read_device(/**@type {GamepadKDY}*/gamepad, /**@type {cont_state_t}*/controller_status) {
    // digital buttons
    for (let i = 0; i < GAMEPAD_BUTTONS_MAP_FROM.length; i++) {
        let source = GAMEPAD_BUTTONS_MAP_FROM[i];
        let target = GAMEPAD_BUTTONS_MAP_TO[i];
        if (controller_status.buttons & source) gamepad.buttons |= target;
    }

    // analog left trigger
    if (controller_status.ltrig >= GAMEPAD_ANALOG_DEAD_ZONE) gamepad.buttons |= GAMEPAD_TRIGGER_LEFT;
    // analog right trigger
    if (controller_status.rtrig >= GAMEPAD_ANALOG_DEAD_ZONE) gamepad.buttons |= GAMEPAD_TRIGGER_RIGHT;

    // analog left stick
    if (controller_status.joyx <= -GAMEPAD_ANALOG_DEAD_ZONE)
        gamepad.buttons |= GAMEPAD_APAD_LEFT;
    else if (controller_status.joyx >= GAMEPAD_ANALOG_DEAD_ZONE)
        gamepad.buttons |= GAMEPAD_APAD_RIGHT;
    // analog right stick
    if (controller_status.joyy <= -GAMEPAD_ANALOG_DEAD_ZONE)
        gamepad.buttons |= GAMEPAD_APAD_DOWN;
    else if (controller_status.joyy >= GAMEPAD_ANALOG_DEAD_ZONE)
        gamepad.buttons |= GAMEPAD_APAD_UP;
}

function gamepad_internal_update_state(/**@type {GamepadKDY}*/gamepad) {
    gamepad.buttons = 0x00;

    if (gamepad.device) {
        // read controller status
        const controller_status = maple_dev_status(gamepad.device);
        // map the buttons in a form the engine can understand
        gamepad_internal_read_device(gamepad, controller_status);
    } else if (gamepad.controller_index < 0) {
        let count = maple_enum_count();
        for (let i = 0; i < count; i++) {
            let device = maple_enum_type(i, MAPLE_FUNC_CONTROLLER);
            if (device) {
                let controller_status = maple_dev_status(device);
                gamepad_internal_read_device(gamepad, controller_status);
            }
        }
    }

    // JS C# only
    if (gamepad.mix_keyboard) {
        const keyboard_status = KOS_MAPLE_KEYBOARD.dequeque_all();
        gamepad_internal_read_device(gamepad, keyboard_status);
    }

    // apply clear mask if necessary
    gamepad_internal_apply_clear_mask(gamepad);
}

function* gamepad_internal_update_state_JSCSHARP(/**@type {GamepadKDY}*/gamepad) {
    gamepad.buttons = 0x00;

    if (gamepad.device) {
        // read controller status
        const controller_status = maple_dev_status(gamepad.device);
        // map the buttons in a form the engine can understand
        gamepad_internal_read_device(gamepad, controller_status);
    }/* else if (gamepad.controller_index < 0) {
        throw new Error("gamepad_internal_update_state_JSCSHARP() controller_index < 0");
    }*/

    if (gamepad.mix_keyboard && KOS_MAPLE_KEYBOARD.has_queued > 0) {
        for (let _ of KOS_MAPLE_KEYBOARD.poll_queue()) {
            gamepad_internal_read_device(gamepad, KOS_MAPLE_KEYBOARD.status);
            yield gamepad.buttons & gamepad.clear_mask;
        }
    } else {
        gamepad_internal_read_device(gamepad, KOS_MAPLE_KEYBOARD.status);
        yield gamepad.buttons & gamepad.clear_mask;
    }

    // apply clear mask if necessary
    gamepad_internal_apply_clear_mask(gamepad);
}

function gamepad_internal_apply_clear_mask(/**@type {GamepadKDY}*/gamepad) {
    if (gamepad.clear_mask != 0x00) {
        // forget released buttons present in the mask
        gamepad.clear_mask = ~(~gamepad.clear_mask & gamepad.buttons);

        // clear pressed buttons
        gamepad.buttons &= gamepad.clear_mask;
    }
}

function gamepad_internal_get_update_timestamp_JSCSHARP(/**@type {GamepadKDY}*/gamepad) {
    if (gamepad.mix_keyboard) {
        if (gamepad.device) {
            return Math.max(KOS_MAPLE_KEYBOARD.timestamp, gamepad.device.timestamp);
        } else {
            return KOS_MAPLE_KEYBOARD.timestamp;
        }
    } else if (gamepad.device) {
        return gamepad.device.timestamp;
    } else {
        // this never should happen
        return timer_ms_gettime64();
    }
}

function gamepad_internal_clear_controller_status(/**@type {cont_state_t}*/status) {
    status.buttons = 0x00;
    status.ltrig = 0;
    status.rtrig = 0;
    status.joyx = 0;
    status.joyy = 0;
    status.joy2x = 0;
    status.joy2y = 0;
}

