#ifndef __modifier_h
#define __modifier_h

typedef struct _Modifier_t {
    float translate_x;
    float translate_y;

    float rotate;

    float skew_x;
    float skew_y;

    float scale_x;
    float scale_y;

    float scale_direction_x;
    float scale_direction_y;

    float rotate_pivot_enabled;
    float rotate_pivot_u;
    float rotate_pivot_v;

    float translate_rotation;
    float scale_translation;
    float scale_size;

    float x;
    float y;

    float width;
    float height;
} Modifier_t;

typedef Modifier_t *Modifier;

#endif

