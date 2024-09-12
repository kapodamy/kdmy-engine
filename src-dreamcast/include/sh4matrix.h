#ifndef _sh4matrix_h
#define _sh4matrix_h

#include <stdalign.h>

#include <dc/matrix3d.h>

#include "math2d.h"
#include "modifier.h"
#include "sh4matrix_types.h"
#include "string.h"


// #define SH4MATRIX_POINT_TRANSFORM mat_trans_single
#define SH4MATRIX_POINT_TRANSFORM mat_trans_single3
// #define SH4MATRIX_POINT_TRANSFORM mat_trans_single3_nodiv


float* sh4matrix_init();
void sh4matrix_destroy(float** sh4matrix);

__FMINLINE void sh4matrix_flip_x(SH4Matrix sh4matrix) {
    mat_load((matrix_t*)sh4matrix);
    mat_scale(-1.0f, 1.0f, 1.0f);
    mat_store((matrix_t*)sh4matrix);
}

__FMINLINE void sh4matrix_flip_y(SH4Matrix sh4matrix) {
    mat_load((matrix_t*)sh4matrix);
    mat_scale(1.0f, -1.0f, 1.0f);
    mat_store((matrix_t*)sh4matrix);
}


__FMINLINE void sh4matrix_rotate_by_degs(SH4Matrix sh4matrix, float angle) {
    //
    // Note: for some reason the angle (in radians) must be negative
    //

    mat_load((matrix_t*)sh4matrix);
    mat_rotate(0.0f, 0.0f, angle * -MATH2D_DEG_TO_RAD);
    mat_store((matrix_t*)sh4matrix);
}

__FMINLINE void sh4matrix_rotate_by_rads(SH4Matrix sh4matrix, float angle) {
    if (angle == 0.0f) return;

    //
    // Note: for some reason the angle must be negative
    //

    mat_load((matrix_t*)sh4matrix);
    mat_rotate(0.0f, 0.0f, -angle);
    mat_store((matrix_t*)sh4matrix);
}



__FMINLINE void sh4matrix_scale(SH4Matrix sh4matrix, float sx, float sy) {
    if (sx == 1.0f && sy == 1.0f) return;

    mat_load((matrix_t*)sh4matrix);
    mat_scale(sx, sy, 1.0f);
    mat_store((matrix_t*)sh4matrix);
}

__FMINLINE void sh4matrix_scale_x(SH4Matrix sh4matrix, float sx) {
    mat_load((matrix_t*)sh4matrix);
    mat_scale(sx, 1.0f, 1.0f);
    mat_store((matrix_t*)sh4matrix);
}

__FMINLINE void sh4matrix_scale_y(SH4Matrix sh4matrix, float sy) {
    mat_load((matrix_t*)sh4matrix);
    mat_scale(1.0f, sy, 1.0f);
    mat_store((matrix_t*)sh4matrix);
}

__FMINLINE void sh4matrix_skew(SH4Matrix sh4matrix, float sx, float sy) {
    if (sx == 0.0f && sy == 0.0f) return;

    SH4Matrix tmp = {
        1.0f, sy, 0.0f, 0.0f, sx, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f
    };

    mat_load((matrix_t*)sh4matrix);
    mat_apply((matrix_t*)tmp);
    mat_store((matrix_t*)sh4matrix);
}

__FMINLINE void sh4matrix_skew_x(SH4Matrix sh4matrix, float sx) {
    sh4matrix_skew(sh4matrix, sx, 0.0f);
}

__FMINLINE void sh4matrix_skew_y(SH4Matrix sh4matrix, float sy) {
    sh4matrix_skew(sh4matrix, 0.0f, sy);
}

__FMINLINE void sh4matrix_translate(SH4Matrix sh4matrix, float tx, float ty) {
    if (tx == 0.0f && ty == 0.0f) return;

    mat_load((matrix_t*)sh4matrix);
    mat_translate(tx, ty, 0.0f);
    mat_store((matrix_t*)sh4matrix);
}

__FMINLINE void sh4matrix_translate_x(SH4Matrix sh4matrix, float tx) {
    if (tx == 0.0f) return;

    mat_load((matrix_t*)sh4matrix);
    mat_translate(tx, 0.0f, 0.0f);
    mat_store((matrix_t*)sh4matrix);
}

__FMINLINE void sh4matrix_translate_y(SH4Matrix sh4matrix, float ty) {
    if (ty == 0.0f) return;

    mat_load((matrix_t*)sh4matrix);
    mat_translate(0.0f, ty, 0.0f);
    mat_store((matrix_t*)sh4matrix);
}


__FMINLINE void sh4matrix_multiply_with_matrix(SH4Matrix sh4matrix1, SH4Matrix sh4matrix2) {
    mat_load((matrix_t*)sh4matrix1);
    mat_apply((matrix_t*)sh4matrix2);
    mat_store((matrix_t*)sh4matrix1);
}

__FMINLINE void sh4matrix_multiply(SH4Matrix sh4matrix, float a, float b, float c, float d, float e, float f, float g, float h, float i, float j, float k, float l, float m, float n, float ñ, float o) {
    SH4Matrix tmp = {a, b, c, d, e, f, g, h, i, j, k, l, m, n, ñ, o};

    mat_load((matrix_t*)sh4matrix);
    mat_apply((matrix_t*)tmp);
    mat_store((matrix_t*)sh4matrix);
}

void sh4matrix_apply_modifier2(SH4Matrix sh4matrix, Modifier* modifier, float draw_x, float draw_y, float draw_width, float draw_height);

__FMINLINE void sh4matrix_apply_modifier(SH4Matrix sh4matrix, Modifier* modifier) {
    sh4matrix_apply_modifier2(
        sh4matrix, modifier,
        modifier->x, modifier->y, modifier->width, modifier->height
    );
}

__FMINLINE void sh4matrix_multiply_point(SH4Matrix sh4matrix, float* target_2d_point_x, float* target_2d_point_y) {
    mat_load((matrix_t*)sh4matrix);

    // unused for 2D rendering
    float z = 0.0f;

    SH4MATRIX_POINT_TRANSFORM(*target_2d_point_x, *target_2d_point_y, z);
}

bool sh4matrix_is_identity(SH4Matrix sh4matrix);
bool sh4matrix_is_equal(SH4Matrix sh4matrix1, SH4Matrix sh4matrix2);

__FMINLINE void sh4matrix_clear(SH4Matrix sh4matrix) {
    mat_identity();
    mat_store((matrix_t*)sh4matrix);
}

__FMINLINE void sh4matrix_set_transform(SH4Matrix sh4matrix, float a, float b, float c, float d, float e, float f, float g, float h, float i, float j, float k, float l, float m, float n, float ñ, float o) {
    sh4matrix[0] = a;
    sh4matrix[1] = b;
    sh4matrix[2] = c;
    sh4matrix[3] = d;
    sh4matrix[4] = e;
    sh4matrix[5] = f;
    sh4matrix[6] = g;
    sh4matrix[7] = h;
    sh4matrix[8] = i;
    sh4matrix[9] = j;
    sh4matrix[10] = k;
    sh4matrix[11] = l;
    sh4matrix[12] = m;
    sh4matrix[13] = n;
    sh4matrix[14] = ñ;
    sh4matrix[15] = o;
}

__FMINLINE void sh4matrix_copy_to(SH4Matrix sh4matrix_source, SH4Matrix sh4matrix_dest) {
    memcpy(sh4matrix_dest, sh4matrix_source, sizeof(SH4Matrix));
}

void sh4matrix_corner_rotate(SH4Matrix sh4matrix, CornerRotation matrix_corner, float x, float y, float width, float height);

__FMINLINE void sh4matrix_scale_size(SH4Matrix sh4matrix, float draw_x, float draw_y, float scale_x, float scale_y) {
    sh4matrix_translate(sh4matrix, draw_x - (draw_x * scale_x), draw_y - (draw_y * scale_y));
    sh4matrix_scale(sh4matrix, scale_x, scale_y);
}

void sh4matrix_helper_calculate_corner_rotation(CornerRotation* matrix_corner, Corner corner);

#endif
