#ifndef _nbool_h
#define _nbool_h

#define unset 2

// nullable/tristate boolean: false, true, unset
typedef enum NBOOL {
    NBOOL_FALSE = false,
    NBOOL_TRUE = true,
    NBOOL_UNSET = unset
} nbool;

#define nbool nbool

#endif
