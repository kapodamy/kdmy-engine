#ifndef _color2d_h
#define _color2d_h

#include <stdint.h>

typedef union {
    uint32_t argb8;
    struct {
        uint8_t a;
        uint8_t r;
        uint8_t g;
        uint8_t b;
    };
} ARGB8;

typedef union {
    uint32_t rgb8;
    struct {
        uint8_t x;
        uint8_t r;
        uint8_t g;
        uint8_t b;
    };
} RGB8;

typedef float RGBA[4];

#endif
