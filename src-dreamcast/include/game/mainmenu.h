#ifndef _mainmenu_h
#define _mainmenu_h

#include "gamepad.h"

#define MAINMENU_GAMEPAD_OK (GAMEPAD_A | GAMEPAD_X | GAMEPAD_START)
#define MAINMENU_GAMEPAD_CANCEL (GAMEPAD_B | GAMEPAD_Y | GAMEPAD_BACK)
#define MAINMENU_GAMEPAD_BUTTONS (MAINMENU_GAMEPAD_OK | MAINMENU_GAMEPAD_CANCEL | GAMEPAD_AD)


bool mainmenu_main();

#endif