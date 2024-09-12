#ifndef _gamepad_h
#define _gamepad_h

#include <stdbool.h>
#include <stdint.h>

#include "gamepad_types.h"

typedef struct {
    GamepadButtons previous;
    GamepadButtons current;
} GamepadButtonsChanges;

Gamepad gamepad_init(int32_t controller_device_index);
Gamepad gamepad_init2(int32_t controller_device_index, bool mix_keyboard);
void gamepad_destroy(Gamepad* gamepad);

bool gamepad_is_dettached(Gamepad gamepad);
bool gamepad_pick(Gamepad gamepad, bool pick_on_start_press);
int32_t gamepad_get_controller_index(Gamepad gamepad);
void gamepad_set_buttons_delay(Gamepad gamepad, int32_t delay_time);
void gamepad_enforce_buttons_delay(Gamepad gamepad);

void gamepad_clear_buttons(Gamepad gamepad);
void gamepad_clear_buttons2(Gamepad gamepad, GamepadButtons buttons_to_clear);
void gamepad_clear_all_gamepads();
GamepadButtons gamepad_get_pressed(Gamepad gamepad);
GamepadButtons gamepad_get_last_pressed(Gamepad gamepad);
GamepadButtons gamepad_get_managed_presses(Gamepad gamepad, bool update_state, GamepadButtons* out_buttons);
GamepadButtons gamepad_has_pressed(Gamepad gamepad, GamepadButtons button_flags);
GamepadButtons gamepad_has_pressed_delayed(Gamepad gamepad, GamepadButtons button_flags);
GamepadButtonsChanges gamepad_direct_state_update(Gamepad gamepad);

#endif