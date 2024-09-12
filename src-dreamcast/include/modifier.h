#ifndef _modifier_h
#define _modifier_h

#include "nbool.h"

typedef struct Modifier_s {
    float translate_x;
    float translate_y;

    float rotate;

    float skew_x;
    float skew_y;

    float scale_x;
    float scale_y;

    float scale_direction_x;
    float scale_direction_y;

    nbool rotate_pivot_enabled;
    float rotate_pivot_u;
    float rotate_pivot_v;

    nbool translate_rotation;
    nbool scale_size;
    nbool scale_translation;

    float x;
    float y;
    float width;
    float height;
} Modifier;

#endif
