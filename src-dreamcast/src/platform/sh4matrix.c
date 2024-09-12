#include <malloc.h>
#include <stdlib.h>
#include <string.h>

#include "malloc_utils.h"
#include "sh4matrix.h"
#include "vertexprops_types.h"

//
// Matrix computation using Hitachi SH-4 CPU dedicated instructions
//


typedef void (*Setter)(SH4Matrix matrix, float value);


void sh4matrix_internal_rotate(SH4Matrix sh4matrix, float draw_x, float draw_y, float draw_width, float draw_height, Modifier* modifier);
void sh4matrix_internal_calc_scale_offset(SH4Matrix sh4matrix, Setter setter, float dimmen, float scale_dimmen, float offset_dimmen);


float* sh4matrix_init() {
    float* sh4matrix = memalign_chk(32, sizeof(SH4Matrix));
    sh4matrix_clear(sh4matrix);
    return sh4matrix;
}

void sh4matrix_destroy(float** sh4matrix) {
    if (!*sh4matrix) return;

    free_chk(*sh4matrix);
    *sh4matrix = NULL;
}


/*void sh4matrix_flip_x(SH4Matrix sh4matrix) {
    sh4matrix_scale_x(sh4matrix, -1.0f);
}*/

/*void sh4matrix_flip_y(SH4Matrix sh4matrix) {
    sh4matrix_scale_y(sh4matrix, -1.0f);
}*/


/*void sh4matrix_rotate_by_degs(SH4Matrix sh4matrix, float angle) {
    sh4matrix_rotate_by_rads(sh4matrix, angle * MATH2D_DEG_TO_RAD);
}*/

/*void sh4matrix_rotate_by_rads(SH4Matrix sh4matrix, float angle) {
    if (angle == 0.0f) return;

    float sin = math2d_sin(angle);
    float cos = math2d_cos(angle);

    float a = sh4matrix[0];
    float b = sh4matrix[1];
    float c = sh4matrix[2];
    float d = sh4matrix[3];
    float e = sh4matrix[4];
    float g = sh4matrix[5];
    float h = sh4matrix[6];
    float i = sh4matrix[7];

    sh4matrix[0] = a * cos + e * sin;
    sh4matrix[1] = b * cos + g * sin;
    sh4matrix[2] = c * cos + h * sin;
    sh4matrix[3] = d * cos + i * sin;
    sh4matrix[4] = e * cos - a * sin;
    sh4matrix[5] = g * cos - b * sin;
    sh4matrix[6] = h * cos - c * sin;
    sh4matrix[7] = i * cos - d * sin;
}*/

/*void sh4matrix_scale(SH4Matrix sh4matrix, float sx, float sy) {
    if (sx == 1.0f && sy == 1.0f) return;

    sh4matrix_multiply(sh4matrix, sx, 0.0f, 0.0f, 0.0f, 0.0f, sy, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
}*/

/*void sh4matrix_scale_x(SH4Matrix sh4matrix, float sx) {
    sh4matrix_scale(sh4matrix, sx, 1.0f);
}*/

/*void sh4matrix_scale_y(SH4Matrix sh4matrix, float sy) {
    sh4matrix_scale(sh4matrix, 1.0f, sy);
}*/

/*void sh4matrix_skew(SH4Matrix sh4matrix, float sx, float sy) {
    if (sx == 0.0f && sy == 0.0f) return;
    sh4matrix_multiply(sh4matrix, 1.0f, sy, 0.0f, 0.0f, sx, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
}*/

/*void sh4matrix_skew_x(SH4Matrix sh4matrix, float sx) {
    sh4matrix_skew(sh4matrix, sx, 0.0f);
}*/

/*void sh4matrix_skew_y(SH4Matrix sh4matrix, float sy) {
    sh4matrix_skew(sh4matrix, 0.0f, sy);
}*/

/*void sh4matrix_translate(SH4Matrix sh4matrix, float tx, float ty) {
    if (tx == 0.0f && ty == 0.0f) return;

    sh4matrix_multiply(sh4matrix, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, tx, ty, 0.0f, 1.0f);
}*/

/*void sh4matrix_translate_x(SH4Matrix sh4matrix, float tx) {
    sh4matrix_translate(sh4matrix, tx, 0.0f);
}*/

/*void sh4matrix_translate_y(SH4Matrix sh4matrix, float ty) {
    sh4matrix_translate(sh4matrix, 0.0f, ty);
}*/


/*void sh4matrix_multiply_with_matrix(SH4Matrix sh4matrix1, SH4Matrix sh4matrix2) {
    sh4matrix_multiply(
        sh4matrix1,
        sh4matrix2[0], sh4matrix2[1], sh4matrix2[2], sh4matrix2[3],
        sh4matrix2[4], sh4matrix2[5], sh4matrix2[6], sh4matrix2[7],
        sh4matrix2[8], sh4matrix2[9], sh4matrix2[10], sh4matrix2[11],
        sh4matrix2[12], sh4matrix2[13], sh4matrix2[14], sh4matrix2[15]
    );
}*/

/*void sh4matrix_multiply(SH4Matrix sh4matrix, float a, float b, float c, float d, float e, float f, float g, float h, float i, float j, float k, float l, float m, float n, float ñ, float o) {
    float a1 = sh4matrix[0];
    float b1 = sh4matrix[1];
    float c1 = sh4matrix[2];
    float d1 = sh4matrix[3];
    float e1 = sh4matrix[4];
    float f1 = sh4matrix[5];
    float g1 = sh4matrix[6];
    float h1 = sh4matrix[7];
    float i1 = sh4matrix[8];
    float j1 = sh4matrix[9];
    float k1 = sh4matrix[10];
    float l1 = sh4matrix[11];
    float m1 = sh4matrix[12];
    float n1 = sh4matrix[13];
    float ñ1 = sh4matrix[14];
    float o1 = sh4matrix[15];

    sh4matrix[0] = a * a1 + b * e1 + c * i1 + d * m1;
    sh4matrix[1] = a * b1 + b * f1 + c * j1 + d * n1;
    sh4matrix[2] = a * c1 + b * g1 + c * k1 + d * ñ1;
    sh4matrix[3] = a * d1 + b * h1 + c * l1 + d * o1;

    sh4matrix[4] = e * a1 + f * e1 + g * i1 + h * m1;
    sh4matrix[5] = e * b1 + f * f1 + g * j1 + h * n1;
    sh4matrix[6] = e * c1 + f * g1 + g * k1 + h * ñ1;
    sh4matrix[7] = e * d1 + f * h1 + g * l1 + h * o1;

    sh4matrix[8] = i * a1 + j * e1 + k * i1 + l * m1;
    sh4matrix[9] = i * b1 + j * f1 + k * j1 + l * n1;
    sh4matrix[10] = i * c1 + j * g1 + k * k1 + l * ñ1;
    sh4matrix[11] = i * d1 + j * h1 + k * l1 + l * o1;

    sh4matrix[12] = m * a1 + n * e1 + ñ * i1 + o * m1;
    sh4matrix[13] = m * b1 + n * f1 + ñ * j1 + o * n1;
    sh4matrix[14] = m * c1 + n * g1 + ñ * k1 + o * ñ1;
    sh4matrix[15] = m * d1 + n * h1 + ñ * l1 + o * o1;
}*/

/*void sh4matrix_apply_modifier(SH4Matrix sh4matrix, Modifier* modifier) {
    return sh4matrix_apply_modifier2(
        sh4matrix, modifier,
        modifier->x, modifier->y, modifier->width, modifier->height
    );
}*/

void sh4matrix_apply_modifier2(SH4Matrix sh4matrix, Modifier* modifier, float draw_x, float draw_y, float draw_width, float draw_height) {
    bool valid_size = draw_width > 0.0f && draw_height > 0.0f;

    if (modifier->scale_direction_x != 0.0f && modifier->scale_x != 1.0f && valid_size) {
        sh4matrix_internal_calc_scale_offset(
            sh4matrix, sh4matrix_translate_x,
            draw_width, modifier->scale_x, modifier->scale_direction_x
        );
    }
    if (modifier->scale_direction_y != 0.0f && modifier->scale_y != 1.0f && valid_size) {
        sh4matrix_internal_calc_scale_offset(
            sh4matrix, sh4matrix_translate_y,
            draw_height, modifier->scale_y, modifier->scale_direction_y
        );
    }

    float tx = modifier->translate_x;
    float ty = modifier->translate_y;
    if (modifier->scale_translation == true) {
        // ¿should be absolute?
        tx *= fabsf(modifier->scale_x);
        ty *= fabsf(modifier->scale_y);
    }

    // combo: matrix translation + matrix rotation (with or without polygon u/v)
    bool translate = tx != 0.0f || ty != 0.0f;
    if (modifier->rotate != 0.0f) {
        /*if (!translate) {
            sh4matrix_internal_rotate(sh4matrix, draw_x, draw_y, draw_width, draw_height, modifier);
        } else */
        if (modifier->translate_rotation) {
            sh4matrix_internal_rotate(sh4matrix, draw_x, draw_y, draw_width, draw_height, modifier);
            if (translate) sh4matrix_translate(sh4matrix, tx, ty);
        } else {
            if (translate) sh4matrix_translate(sh4matrix, tx, ty);
            sh4matrix_internal_rotate(sh4matrix, draw_x, draw_y, draw_width, draw_height, modifier);
        }
    } else if (translate) {
        sh4matrix_translate(sh4matrix, tx, ty);
    }


    if (modifier->skew_x != 0.0f || modifier->skew_y != 0.0f)
        sh4matrix_skew(sh4matrix, modifier->skew_x, modifier->skew_y);

    if (modifier->scale_x != 1.0f || modifier->scale_y != 1.0f) {
        if (modifier->scale_size)
            sh4matrix_scale_size(sh4matrix, draw_x, draw_y, modifier->scale_x, modifier->scale_y);
        else
            sh4matrix_scale(sh4matrix, modifier->scale_x, modifier->scale_y);
    }
}

/*void sh4matrix_multiply_point(SH4Matrix sh4matrix, float* target_2d_point_x, float* target_2d_point_y) {
    float x1 = *target_2d_point_x;
    float y1 = *target_2d_point_y;
    float z1 = 0.0f;
    float w1 = 1.0f;

    float a = sh4matrix[0];
    float b = sh4matrix[1];
    float e = sh4matrix[4];
    float f = sh4matrix[5];
    float i = sh4matrix[8];
    float j = sh4matrix[9];
    float m = sh4matrix[12];
    float n = sh4matrix[13];

    float x2 = (x1 * a) + (y1 * e) + (z1 * i) + (w1 * m);
    float y2 = (x1 * b) + (y1 * f) + (z1 * j) + (w1 * n);

    // float c = sh4matrix[2]; float g = sh4matrix[6]; float k = sh4matrix[10]; float ñ = sh4matrix[14];
    // float z2 = (x1 * c) + (y1 * g) + (z1 * k) + (w1 * ñ);

    // float d = sh4matrix[3]; float h = sh4matrix[7]; float l = sh4matrix[11]; float o = sh4matrix[15];
    // float w2 = (x1 * d) + (y1 * h) + (z1 * l) + (w1 * o);

    *target_2d_point_x = x2;
    *target_2d_point_y = y2;
}*/


bool sh4matrix_is_identity(SH4Matrix sh4matrix) {
    return math2d_floats_are_near_equal(sh4matrix[0], 1.0f) &&
           math2d_floats_are_near_equal(sh4matrix[1], 0.0f) &&
           math2d_floats_are_near_equal(sh4matrix[2], 0.0f) &&
           math2d_floats_are_near_equal(sh4matrix[3], 0.0f) &&
           math2d_floats_are_near_equal(sh4matrix[4], 0.0f) &&
           math2d_floats_are_near_equal(sh4matrix[5], 1.0f) &&
           math2d_floats_are_near_equal(sh4matrix[6], 0.0f) &&
           math2d_floats_are_near_equal(sh4matrix[7], 0.0f) &&
           math2d_floats_are_near_equal(sh4matrix[8], 0.0f) &&
           math2d_floats_are_near_equal(sh4matrix[9], 0.0f) &&
           math2d_floats_are_near_equal(sh4matrix[10], 1.0f) &&
           math2d_floats_are_near_equal(sh4matrix[11], 0.0f) &&
           math2d_floats_are_near_equal(sh4matrix[12], 0.0f) &&
           math2d_floats_are_near_equal(sh4matrix[13], 0.0f) &&
           math2d_floats_are_near_equal(sh4matrix[14], 0.0f) &&
           math2d_floats_are_near_equal(sh4matrix[15], 1.0f);
}

bool sh4matrix_is_equal(SH4Matrix sh4matrix1, SH4Matrix sh4matrix2) {
    return math2d_floats_are_near_equal(sh4matrix1[0], sh4matrix2[0]) &&
           math2d_floats_are_near_equal(sh4matrix1[1], sh4matrix2[1]) &&
           math2d_floats_are_near_equal(sh4matrix1[2], sh4matrix2[2]) &&
           math2d_floats_are_near_equal(sh4matrix1[3], sh4matrix2[3]) &&
           math2d_floats_are_near_equal(sh4matrix1[4], sh4matrix2[4]) &&
           math2d_floats_are_near_equal(sh4matrix1[5], sh4matrix2[5]) &&
           math2d_floats_are_near_equal(sh4matrix1[6], sh4matrix2[6]) &&
           math2d_floats_are_near_equal(sh4matrix1[7], sh4matrix2[7]) &&
           math2d_floats_are_near_equal(sh4matrix1[8], sh4matrix2[8]) &&
           math2d_floats_are_near_equal(sh4matrix1[9], sh4matrix2[9]) &&
           math2d_floats_are_near_equal(sh4matrix1[10], sh4matrix2[10]) &&
           math2d_floats_are_near_equal(sh4matrix1[11], sh4matrix2[11]) &&
           math2d_floats_are_near_equal(sh4matrix1[12], sh4matrix2[12]) &&
           math2d_floats_are_near_equal(sh4matrix1[13], sh4matrix2[13]) &&
           math2d_floats_are_near_equal(sh4matrix1[14], sh4matrix2[14]) &&
           math2d_floats_are_near_equal(sh4matrix1[15], sh4matrix2[15]);
}

/*void sh4matrix_clear(SH4Matrix sh4matrix) {
    mat_identity();
    mat_store((matrix_t*)sh4matrix);
}*/

/*void sh4matrix_set_transform(SH4Matrix sh4matrix, float a, float b, float c, float d, float e, float f, float g, float h, float i, float j, float k, float l, float m, float n, float ñ, float o) {
    sh4matrix[0] = a; sh4matrix[1] = b; sh4matrix[2] = c; sh4matrix[3] = d;
    sh4matrix[4] = e; sh4matrix[5] = f; sh4matrix[6] = g; sh4matrix[7] = h;
    sh4matrix[8] = i; sh4matrix[9] = j; sh4matrix[10] = k; sh4matrix[11] = l;
    sh4matrix[12] = m; sh4matrix[13] = n; sh4matrix[14] = ñ; sh4matrix[15] = o;
}*/

/*void sh4matrix_copy_to(SH4Matrix sh4matrix_source, SH4Matrix sh4matrix_dest) {
    memcpy(sh4matrix_dest, sh4matrix_source, sizeof(SH4Matrix));
}*/

void sh4matrix_corner_rotate(SH4Matrix sh4matrix, CornerRotation matrix_corner, float x, float y, float width, float height) {
    sh4matrix_translate(sh4matrix, x, y);
    sh4matrix_rotate_by_rads(sh4matrix, matrix_corner.angle);
    sh4matrix_translate(sh4matrix, -x, -y);
    sh4matrix_translate(sh4matrix, matrix_corner.x * width, matrix_corner.y * height);
}

/*void sh4matrix_scale_size(SH4Matrix sh4matrix, float draw_x, float draw_y, float scale_x, float scale_y) {
    sh4matrix_translate(sh4matrix, draw_x - (draw_x * scale_x), draw_y - (draw_y * scale_y));
    sh4matrix_scale(sh4matrix, scale_x, scale_y);
}*/



void sh4matrix_internal_rotate(SH4Matrix sh4matrix, float draw_x, float draw_y, float draw_width, float draw_height, Modifier* modifier) {
    if (modifier->rotate == 0.0f) return;

    if (!modifier->rotate_pivot_enabled || draw_width < 0.0f || draw_height < 0.0f) {
        sh4matrix_rotate_by_rads(sh4matrix, modifier->rotate);
        return;
    }

    // the pivot is calculated from inside of the polygon
    float x = draw_x + (modifier->rotate_pivot_u * draw_width);
    float y = draw_y + (modifier->rotate_pivot_v * draw_height);
    sh4matrix_translate(sh4matrix, x, y);
    sh4matrix_rotate_by_rads(sh4matrix, modifier->rotate);
    sh4matrix_translate(sh4matrix, -x, -y);
}

void sh4matrix_internal_calc_scale_offset(SH4Matrix sh4matrix, Setter setter, float dimmen, float scale_dimmen, float offset_dimmen) {
    float sign = math2d_cos(scale_dimmen);
    float offset = dimmen * (fabsf(scale_dimmen) - 1.0f);
    float translate = offset * sign * offset_dimmen;

    setter(sh4matrix, translate);
}


void sh4matrix_helper_calculate_corner_rotation(CornerRotation* matrix_corner, Corner corner) {
    float angle;
    float x;
    float y;

    switch (corner) {
        default:
        case Corner_TopLeft:
            angle = 0.0f;
            x = 0.0f;
            y = 0.0f;
            break;
        case Corner_TopRight:
            angle = -90.0f * MATH2D_DEG_TO_RAD;
            x = -1.0f;
            y = 0.0f;
            break;
        case Corner_BottomLeft:
            angle = 90.0f * MATH2D_DEG_TO_RAD;
            x = 0.0f;
            y = -1.0f;
            break;
        case Corner_BottomRight:
            angle = 180.0f * MATH2D_DEG_TO_RAD;
            x = -1.0f;
            y = -1.0f;
            break;
    }

    matrix_corner->x = x;
    matrix_corner->y = y;
    matrix_corner->angle = angle;
}
