#include <stdbool.h>

#include <arch/timer.h>
#include <dc/maple.h>
#include <dc/maple/controller.h>

#include "gamepad.h"
#include "malloc_utils.h"
#include "map.h"


//
// Unavailable buttons, these buttons does not exists on
// the dreamcast controller, stub it to the highest bit
//

#define CONT_EX_BACK_OR_SELECT 2147483648u
#define CONT_EX_DPAD3_UP 2147483648u
#define CONT_EX_DPAD3_DOWN 2147483648u
#define CONT_EX_DPAD3_LEFT 2147483648u
#define CONT_EX_DPAD3_RIGHT 2147483648u
#define CONT_EX_DPAD4_UP 2147483648u
#define CONT_EX_DPAD4_DOWN 2147483648u
#define CONT_EX_DPAD4_LEFT 2147483648u
#define CONT_EX_DPAD4_RIGHT 2147483648u

struct Gamepad_s {
    GamepadButtons buttons;
    GamepadButtons last_buttons;
    GamepadButtons clear_mask;
    uint32_t delay_duration;
    uint64_t delay_timestamp;
    int8_t controller_index;
    maple_device_t* device;
    bool mix_keyboard;
};

static bool gamepad_internal_pick_maple_device(Gamepad gamepad);
static void gamepad_internal_read_device(Gamepad gamepad, const cont_state_t* controller_status);
static void gamepad_internal_update_state(Gamepad gamepad);
static void gamepad_internal_apply_clear_mask(Gamepad gamepad);


static const uint8_t GAMEPAD_ANALOG_DEAD_ZONE = 0x40; // considered "pressed" any analog input if 25% or more
static const uint32_t GAMEPAD_BUTTONS_MAP_FROM[] = {
    CONT_A,
    CONT_B,
    CONT_X,
    CONT_Y,
    CONT_DPAD_UP,
    CONT_DPAD_DOWN,
    CONT_DPAD_LEFT,
    CONT_DPAD_RIGHT,
    CONT_START,
    CONT_D,
    CONT_EX_BACK_OR_SELECT,
    CONT_C,
    CONT_Z,
    CONT_DPAD2_UP,
    CONT_DPAD2_DOWN,
    CONT_DPAD2_LEFT,
    CONT_DPAD2_RIGHT,
    CONT_EX_DPAD3_UP,
    CONT_EX_DPAD3_DOWN,
    CONT_EX_DPAD3_LEFT,
    CONT_EX_DPAD3_RIGHT,
    CONT_EX_DPAD4_UP,
    CONT_EX_DPAD4_DOWN,
    CONT_EX_DPAD4_LEFT,
    CONT_EX_DPAD4_RIGHT,
};
static const GamepadButtons GAMEPAD_BUTTONS_MAP_TO[] = {
    GAMEPAD_A,
    GAMEPAD_B,
    GAMEPAD_X,
    GAMEPAD_Y,
    GAMEPAD_DPAD_UP,
    GAMEPAD_DPAD_DOWN,
    GAMEPAD_DPAD_LEFT,
    GAMEPAD_DPAD_RIGHT,
    GAMEPAD_START,
    GAMEPAD_SELECT,
    GAMEPAD_BACK,
    GAMEPAD_BUMPER_LEFT,
    GAMEPAD_BUMPER_RIGHT,
    GAMEPAD_DPAD2_UP,
    GAMEPAD_DPAD2_DOWN,
    GAMEPAD_DPAD2_LEFT,
    GAMEPAD_DPAD2_RIGHT,
    GAMEPAD_DPAD3_UP,
    GAMEPAD_DPAD3_DOWN,
    GAMEPAD_DPAD3_LEFT,
    GAMEPAD_DPAD3_RIGHT,
    GAMEPAD_DPAD4_UP,
    GAMEPAD_DPAD4_DOWN,
    GAMEPAD_DPAD4_LEFT,
    GAMEPAD_DPAD4_RIGHT,
};
static const size_t GAMEPAD_BUTTONS_MAP_LENGTH = sizeof(GAMEPAD_BUTTONS_MAP_FROM) / sizeof(uint32_t);


static Map GAMEPAD_POOL = NULL;

void __attribute__((constructor)) __ctor_gamepad() {
    GAMEPAD_POOL = map_init();
}


Gamepad gamepad_init(int32_t controller_device_index) {
    return gamepad_init2(controller_device_index, true);
}

Gamepad gamepad_init2(int32_t controller_device_index, bool mix_keyboard) {
    Gamepad gamepad = malloc_chk(sizeof(struct Gamepad_s));
    malloc_assert(gamepad, Gamepad);

    *gamepad = (struct Gamepad_s){
        .buttons = 0x00,
        .last_buttons = 0x00,
        .clear_mask = 0x00,
        .controller_index = controller_device_index,
        .delay_duration = 0,
        .delay_timestamp = 0,
        .device = NULL,
        .mix_keyboard = controller_device_index < 1 && mix_keyboard,
    };
    map_add(GAMEPAD_POOL, 0, gamepad);

    // check if should use any controller attached
    if (controller_device_index < 0) return gamepad;

    // pick dreamcast gamepad if connected
    gamepad_internal_pick_maple_device(gamepad);

    return gamepad;
}

void gamepad_destroy(Gamepad* gamepad) {
    if (!*gamepad) return;

    map_delete2(GAMEPAD_POOL, *gamepad);

    free_chk(*gamepad);
    *gamepad = NULL;
}


bool gamepad_is_dettached(Gamepad gamepad) {
    if (gamepad->mix_keyboard) return false;
    return gamepad->device ? (!gamepad->device->valid) : false;
}

bool gamepad_pick(Gamepad gamepad, bool pick_on_start_press) {
    if (gamepad->device) {
        if (gamepad->device->valid) return true;
    } else if (gamepad_internal_pick_maple_device(gamepad))
        return true;
    if (!pick_on_start_press) return false;

    //
    // Pick another maple controller connected to the dreamcast
    // but also check if not used by any other gamepad instance
    // the user must press the START button to detect them
    //
    int maple_devices_found = maple_enum_count();
    for (int i = 0; i < maple_devices_found; i++) {
        maple_device_t* device = maple_enum_type(i, MAPLE_FUNC_CONTROLLER);
        if (!device || !device->valid) continue;

        bool in_use = false;
        foreach (Gamepad, j, MAP_ITERATOR, GAMEPAD_POOL) {
            if (j->device == device) {
                in_use = true;
                break;
            }
        }

        if (in_use) continue;

        // check if the user is pressing the button START
        cont_state_t* status = (cont_state_t*)maple_dev_status(device);
        if (status->buttons & CONT_START) {
            gamepad->device = device;
            return true;
        }
    }

    return false;
}

int32_t gamepad_get_controller_index(Gamepad gamepad) {
    return gamepad->controller_index;
}

void gamepad_set_buttons_delay(Gamepad gamepad, int32_t delay_time) {
    gamepad->delay_duration = delay_time < 0 ? 0 : (uint32_t)delay_time;
    gamepad->delay_timestamp = 0;
}

void gamepad_enforce_buttons_delay(Gamepad gamepad) {
    gamepad->delay_timestamp = timer_ms_gettime64() + gamepad->delay_duration;
}



void gamepad_clear_buttons(Gamepad gamepad) {
    gamepad->clear_mask = 0x00;
    gamepad_internal_update_state(gamepad);
    gamepad->clear_mask = ~gamepad->buttons;
    gamepad->buttons = 0x00;
}

void gamepad_clear_buttons2(Gamepad gamepad, GamepadButtons buttons_to_clear) {
    gamepad->clear_mask = 0x00;
    gamepad_internal_update_state(gamepad);
    gamepad->clear_mask = ~(gamepad->buttons & buttons_to_clear);
    gamepad->buttons &= ~buttons_to_clear;
}

void gamepad_clear_all_gamepads() {
    foreach (Gamepad, gamepad, MAP_ITERATOR, GAMEPAD_POOL) {
        gamepad->clear_mask = 0x00;
        gamepad_internal_update_state(gamepad);
        gamepad->clear_mask = ~gamepad->buttons;
        // gamepad->last_buttons = gamepad->buttons;
        gamepad->buttons = 0x00;
    }
}

GamepadButtons gamepad_get_pressed(Gamepad gamepad) {
    gamepad_internal_update_state(gamepad);
    return gamepad->buttons;
}

GamepadButtons gamepad_get_last_pressed(Gamepad gamepad) {
    return gamepad->buttons;
}

GamepadButtons gamepad_get_managed_presses(Gamepad gamepad, bool update_state, GamepadButtons* out_buttons) {
    if (update_state) gamepad_internal_update_state(gamepad);
    if (gamepad->buttons == gamepad->last_buttons) return false;

    *out_buttons = gamepad->buttons;
    gamepad->last_buttons = gamepad->buttons;
    return true;
}

GamepadButtons gamepad_has_pressed(Gamepad gamepad, GamepadButtons button_flags) {
    gamepad_internal_update_state(gamepad);

    return (gamepad->buttons & button_flags) /* != 0*/;
}

GamepadButtons gamepad_has_pressed_delayed(Gamepad gamepad, GamepadButtons button_flags) {
    gamepad_internal_update_state(gamepad);

    GamepadButtons pressed = gamepad->buttons & button_flags;
    if (pressed == 0x00 || gamepad->delay_duration < 1) return 0x00;

    GamepadButtons now = timer_ms_gettime64();
    if (now < gamepad->delay_timestamp) return 0x00;

    gamepad->delay_timestamp = now + gamepad->delay_duration;
    return pressed;
}

GamepadButtonsChanges gamepad_direct_state_update(Gamepad gamepad) {
    if (!gamepad->device || !gamepad->device->valid) {
        if (gamepad->mix_keyboard) {
            return (GamepadButtonsChanges){
                .previous = gamepad_get_last_pressed(gamepad),
                .current = gamepad_get_pressed(gamepad)
            };
        }
        return (GamepadButtonsChanges){.previous = UINT32_MAX, .current = UINT32_MAX};
    }

    GamepadButtons old = gamepad->buttons;
    gamepad->buttons = 0x00;

    const cont_state_t* controller_status = (cont_state_t*)maple_dev_status(gamepad->device);
    gamepad_internal_read_device(gamepad, controller_status);

    return (GamepadButtonsChanges){
        .previous = old,
        .current = gamepad->buttons
    };
}


static bool gamepad_internal_pick_maple_device(Gamepad gamepad) {
    int32_t index = 0;
    int maple_devices_found = maple_enum_count();

    // find the desired controller number/index/position
    for (int i = 0; i < maple_devices_found; i++) {
        maple_device_t* device = maple_enum_type(i, MAPLE_FUNC_CONTROLLER);
        if (!device) continue;

        if (index == gamepad->controller_index) {
            gamepad->device = device;
            return true;
        }

        index++;
    }

    return false;
}

static void gamepad_internal_read_device(Gamepad gamepad, const cont_state_t* controller_status) {
    // digital buttons
    for (size_t i = 0; i < GAMEPAD_BUTTONS_MAP_LENGTH; i++) {
        uint32_t source = GAMEPAD_BUTTONS_MAP_FROM[i];
        GamepadButtons target = GAMEPAD_BUTTONS_MAP_TO[i];
        if (controller_status->buttons & source) gamepad->buttons |= target;
    }

    // analog left trigger
    if (controller_status->ltrig >= GAMEPAD_ANALOG_DEAD_ZONE) gamepad->buttons |= GAMEPAD_TRIGGER_LEFT;
    // analog right trigger
    if (controller_status->rtrig >= GAMEPAD_ANALOG_DEAD_ZONE) gamepad->buttons |= GAMEPAD_TRIGGER_RIGHT;

    // analog left stick
    if (controller_status->joyx <= -GAMEPAD_ANALOG_DEAD_ZONE)
        gamepad->buttons |= GAMEPAD_APAD_LEFT;
    else if (controller_status->joyx >= GAMEPAD_ANALOG_DEAD_ZONE)
        gamepad->buttons |= GAMEPAD_APAD_RIGHT;
    // analog right stick
    if (controller_status->joyy <= -GAMEPAD_ANALOG_DEAD_ZONE)
        gamepad->buttons |= GAMEPAD_APAD_DOWN;
    else if (controller_status->joyy >= GAMEPAD_ANALOG_DEAD_ZONE)
        gamepad->buttons |= GAMEPAD_APAD_UP;
}

static void gamepad_internal_update_state(Gamepad gamepad) {
    gamepad->buttons = 0x00;

    if (gamepad->device) {
        // read controller status
        const cont_state_t* controller_status = (cont_state_t*)maple_dev_status(gamepad->device);
        // map the buttons in a form the engine can understand
        gamepad_internal_read_device(gamepad, controller_status);
    } else if (gamepad->controller_index < 0) {
        int count = maple_enum_count();
        for (int i = 0; i < count; i++) {
            maple_device_t* device = maple_enum_type(i, MAPLE_FUNC_CONTROLLER);
            if (device) {
                cont_state_t* controller_status = (cont_state_t*)maple_dev_status(device);
                gamepad_internal_read_device(gamepad, controller_status);
            }
        }
    }

    // apply clear mask if necessary
    gamepad_internal_apply_clear_mask(gamepad);
}

static void gamepad_internal_apply_clear_mask(Gamepad gamepad) {
    if (gamepad->clear_mask != 0x00) {
        // forget released buttons present in the mask
        gamepad->clear_mask = ~(~gamepad->clear_mask & gamepad->buttons);

        // clear pressed buttons
        gamepad->buttons &= gamepad->clear_mask;
    }
}
