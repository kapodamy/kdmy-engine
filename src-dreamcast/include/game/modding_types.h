#ifndef _modding_types_h
#define _modding_types_h

#include <stdbool.h>
#include <stdlib.h>

#include "float64.h"
#include "gamepad_types.h"


typedef bool (*ModdingCallbackOption)(void* private_data, const char* option_name);

typedef enum {
    ModdingHelperResult_CONTINUE,
    ModdingHelperResult_BACK,
    ModdingHelperResult_CHOOSEN
} ModdingHelperResult;

typedef enum {
    ModdingValueType_null = 0,
    ModdingValueType_string = 1,
    ModdingValueType_boolean = 2,
    ModdingValueType_double = 3
} ModdingValueType;


#endif
