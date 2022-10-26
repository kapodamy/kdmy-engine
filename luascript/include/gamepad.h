#ifndef _gamepad_h
#define _gamepad_h

#include <stdint.h>

static const uint32_t GAMEPAD_A = 1 << 0;
static const uint32_t GAMEPAD_B = 1 << 1;
static const uint32_t GAMEPAD_X = 1 << 2;
static const uint32_t GAMEPAD_Y = 1 << 3;

static const uint32_t GAMEPAD_DPAD_UP = 1 << 4;
static const uint32_t GAMEPAD_DPAD_DOWN = 1 << 5;
static const uint32_t GAMEPAD_DPAD_RIGHT = 1 << 6;
static const uint32_t GAMEPAD_DPAD_LEFT = 1 << 7;

static const uint32_t GAMEPAD_START = 1 << 8;
static const uint32_t GAMEPAD_SELECT = 1 << 9;

static const uint32_t GAMEPAD_TRIGGER_LEFT = 1 << 10;
static const uint32_t GAMEPAD_TRIGGER_RIGHT = 1 << 11;
static const uint32_t GAMEPAD_BUMPER_LEFT = 1 << 12;
static const uint32_t GAMEPAD_BUMPER_RIGHT = 1 << 13;

static const uint32_t GAMEPAD_APAD_UP = 1 << 14;
static const uint32_t GAMEPAD_APAD_DOWN = 1 << 15;
static const uint32_t GAMEPAD_APAD_RIGHT = 1 << 16;
static const uint32_t GAMEPAD_APAD_LEFT = 1 << 17;

static const uint32_t GAMEPAD_DPAD2_UP = 1 << 18;
static const uint32_t GAMEPAD_DPAD2_DOWN = 1 << 19;
static const uint32_t GAMEPAD_DPAD2_RIGHT = 1 << 20;
static const uint32_t GAMEPAD_DPAD2_LEFT = 1 << 21;

static const uint32_t GAMEPAD_DPAD3_UP = 1 << 22;
static const uint32_t GAMEPAD_DPAD3_DOWN = 1 << 23;
static const uint32_t GAMEPAD_DPAD3_RIGHT = 1 << 24;
static const uint32_t GAMEPAD_DPAD3_LEFT = 1 << 25;

static const uint32_t GAMEPAD_DPAD4_UP = 1 << 26;
static const uint32_t GAMEPAD_DPAD4_DOWN = 1 << 27;
static const uint32_t GAMEPAD_DPAD4_RIGHT = 1 << 28;
static const uint32_t GAMEPAD_DPAD4_LEFT = 1 << 29;

static const uint32_t GAMEPAD_BACK = 1 << 30;

#endif