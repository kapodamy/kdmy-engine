"use strict";

//
// Matrix computation using Hitachi SH-4 CPU dedicated instructions
//

const SH4MATRIX_SIZE = 16;

function sh4matrix_init() {
    let sh4matrix = new Float32Array(SH4MATRIX_SIZE);
    sh4matrix[0] = sh4matrix[5] = sh4matrix[10] = sh4matrix[15] = 1;
    return sh4matrix;
}

function sh4matrix_destroy(sh4matrix) {
    sh4matrix = undefined;
}


function sh4matrix_flip_x(sh4matrix) {
    sh4matrix_scale_x(sh4matrix, -1);
}

function sh4matrix_flip_y(sh4matrix) {
    sh4matrix_scale_y(sh4matrix, -1);
}


function sh4matrix_rotate_by_degs(sh4matrix, angle) {
    sh4matrix_rotate_by_rads(sh4matrix, angle * MATH2D_DEG_TO_RAD);
}

function sh4matrix_rotate_by_rads(sh4matrix, angle) {
    if (angle == 0) return;

    let sin = Math.sin(angle);
    let cos = Math.cos(angle);

    let a = sh4matrix[0];
    let b = sh4matrix[1];
    let c = sh4matrix[2];
    let d = sh4matrix[3];
    let e = sh4matrix[4];
    let g = sh4matrix[5];
    let h = sh4matrix[6];
    let i = sh4matrix[7];

    sh4matrix[0] = a * cos + e * sin;
    sh4matrix[1] = b * cos + g * sin;
    sh4matrix[2] = c * cos + h * sin;
    sh4matrix[3] = d * cos + i * sin;
    sh4matrix[4] = e * cos - a * sin;
    sh4matrix[5] = g * cos - b * sin;
    sh4matrix[6] = h * cos - c * sin;
    sh4matrix[7] = i * cos - d * sin;

}

function sh4matrix_scale(sh4matrix, sx, sy) {
    if (sx == 1 && sy == 1) return;
    sh4matrix_multiply(sh4matrix, sx, 0, 0, 0, 0, sy, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
}

function sh4matrix_scale_x(sh4matrix, sx) {
    sh4matrix_scale(sh4matrix, sx, 1);
}

function sh4matrix_scale_y(sh4matrix, sy) {
    sh4matrix_scale(sh4matrix, 1, sy);
}

function sh4matrix_skew(sh4matrix, sx, sy) {
    if (sx == 0 && sy == 0) return;
    sh4matrix_multiply(sh4matrix, 1, sy, 0, 0, sx, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
}

function sh4matrix_skew_x(sh4matrix, sx) {
    sh4matrix_skew(sh4matrix, sx, 0);
}

function sh4matrix_skew_y(sh4matrix, sy) {
    sh4matrix_skew(sh4matrix, 0, sy);
}

function sh4matrix_translate(sh4matrix, tx, ty) {
    if (tx == 0 && ty == 0) return;
    sh4matrix_multiply(sh4matrix, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, tx, ty, 0, 1);
}

function sh4matrix_translate_x(sh4matrix, tx) {
    sh4matrix_translate(sh4matrix, tx, 0);
}

function sh4matrix_translate_y(sh4matrix, ty) {
    sh4matrix_translate(sh4matrix, 0, ty);
}


function sh4matrix_multiply_with_matrix(sh4matrix1, sh4matrix2) {
    sh4matrix_multiply(
        sh4matrix1,
        sh4matrix2[0], sh4matrix2[1], sh4matrix2[2], sh4matrix2[3],
        sh4matrix2[4], sh4matrix2[5], sh4matrix2[6], sh4matrix2[7],
        sh4matrix2[8], sh4matrix2[9], sh4matrix2[10], sh4matrix2[11],
        sh4matrix2[12], sh4matrix2[13], sh4matrix2[14], sh4matrix2[15]
    )
}

function sh4matrix_multiply(sh4matrix, a, b, c, d, e, f, g, h, i, j, k, l, m, n, ñ, o) {
    let a1 = sh4matrix[0]; let b1 = sh4matrix[1]; let c1 = sh4matrix[2]; let d1 = sh4matrix[3];
    let e1 = sh4matrix[4]; let f1 = sh4matrix[5]; let g1 = sh4matrix[6]; let h1 = sh4matrix[7];
    let i1 = sh4matrix[8]; let j1 = sh4matrix[9]; let k1 = sh4matrix[10]; let l1 = sh4matrix[11];
    let m1 = sh4matrix[12]; let n1 = sh4matrix[13]; let ñ1 = sh4matrix[14]; let o1 = sh4matrix[15];

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
}

function sh4matrix_apply_modifier(sh4matrix, modifier) {
    return sh4matrix_apply_modifier2(
        sh4matrix, modifier,
        modifier.x, modifier.y, modifier.width, modifier.height
    );
}

function sh4matrix_apply_modifier2(sh4matrix, modifier, draw_x, draw_y, draw_width, draw_height) {
    let valid_size = draw_width > 0 && draw_height > 0;

    if (modifier.scale_direction_x != 0 && modifier.scale_x != 1.0 && valid_size) {
        sh4matrix_internal_calc_scale_offset(
            sh4matrix, sh4matrix_translate_x,
            draw_width, modifier.scale_x, modifier.scale_direction_x
        );
    }
    if (modifier.scale_direction_y != 0 && modifier.scale_y != 1.0 && valid_size) {
        sh4matrix_internal_calc_scale_offset(
            sh4matrix, sh4matrix_translate_y,
            draw_height, modifier.scale_y, modifier.scale_direction_y
        );
    }

    let tx = modifier.translate_x;
    let ty = modifier.translate_y;
    if (modifier.scale_translation == true) {
        // ¿should be absolute?
        tx *= Math.abs(modifier.scale_x);
        ty *= Math.abs(modifier.scale_y);
    }

    // combo: matrix translation + matrix rotation (with or without polygon u/v)
    let translate = modifier.tx != 0 || modifier.ty != 0;
    if (modifier.rotate != 0) {
        /*if (!translate) {
            sh4matrix_internal_rotate(sh4matrix, draw_x, draw_y, draw_width, draw_height, modifier);
        } else */if (modifier.translate_rotation) {
            sh4matrix_internal_rotate(sh4matrix, draw_x, draw_y, draw_width, draw_height, modifier);
            if (translate) sh4matrix_translate(sh4matrix, tx, modifier.ty);
        } else {
            if (translate) sh4matrix_translate(sh4matrix, tx, ty);
            sh4matrix_internal_rotate(sh4matrix, draw_x, draw_y, draw_width, draw_height, modifier);
        }
    } else if (translate) {
        sh4matrix_translate(sh4matrix, tx, ty);
    }


    if (modifier.skew_x != 0 || modifier.skew_y != 0)
        sh4matrix_skew(sh4matrix, modifier.skew_x, modifier.skew_y);

    if (modifier.scale_x != 1.0 || modifier.scale_y != 1.0) {
        if (modifier.scale_size)
            sh4matrix_scale_size(sh4matrix, draw_x, draw_y, modifier.scale_x, modifier.scale_y);
        else
            sh4matrix_scale(sh4matrix, modifier.scale_x, modifier.scale_y);
    }
}


function sh4matrix_is_identity(sh4matrix) {
    return math2d_floats_are_near_equal(sh4matrix[0], 1) &&
        math2d_floats_are_near_equal(sh4matrix[1], 0) &&
        math2d_floats_are_near_equal(sh4matrix[2], 0) &&
        math2d_floats_are_near_equal(sh4matrix[3], 0) &&
        math2d_floats_are_near_equal(sh4matrix[4], 0) &&
        math2d_floats_are_near_equal(sh4matrix[5], 1) &&
        math2d_floats_are_near_equal(sh4matrix[6], 0) &&
        math2d_floats_are_near_equal(sh4matrix[7], 0) &&
        math2d_floats_are_near_equal(sh4matrix[8], 0) &&
        math2d_floats_are_near_equal(sh4matrix[9], 0) &&
        math2d_floats_are_near_equal(sh4matrix[10], 1) &&
        math2d_floats_are_near_equal(sh4matrix[11], 0) &&
        math2d_floats_are_near_equal(sh4matrix[12], 0) &&
        math2d_floats_are_near_equal(sh4matrix[13], 0) &&
        math2d_floats_are_near_equal(sh4matrix[14], 0) &&
        math2d_floats_are_near_equal(sh4matrix[15], 1);
}

function sh4matrix_is_equal(sh4matrix1, sh4matrix2) {
    for (let i = 0; i < SH4MATRIX_SIZE; i++) {
        if (!math2d_floats_are_near_equal(sh4matrix1[i], sh4matrix2[i])) return 0;
    }
    return 1;
}

function sh4matrix_clear(sh4matrix) {
    sh4matrix[0] = 1; sh4matrix[1] = 0; sh4matrix[2] = 0; sh4matrix[3] = 0;
    sh4matrix[4] = 0; sh4matrix[5] = 1; sh4matrix[6] = 0; sh4matrix[7] = 0;
    sh4matrix[8] = 0; sh4matrix[9] = 0; sh4matrix[10] = 1; sh4matrix[11] = 0;
    sh4matrix[12] = 0; sh4matrix[13] = 0; sh4matrix[14] = 0; sh4matrix[15] = 1;
}

function sh4matrix_set_transform(sh4matrix, a, b, c, d, e, f, g, h, i, j, k, l, m, n, ñ, o) {
    sh4matrix[0] = a; sh4matrix[1] = b; sh4matrix[2] = c; sh4matrix[3] = d;
    sh4matrix[4] = e; sh4matrix[5] = f; sh4matrix[6] = g; sh4matrix[7] = h;
    sh4matrix[8] = i; sh4matrix[9] = j; sh4matrix[10] = k; sh4matrix[11] = l;
    sh4matrix[12] = m; sh4matrix[13] = n; sh4matrix[14] = ñ; sh4matrix[15] = o;
}

function sh4matrix_copy_to(sh4matrix_source, sh4matrix_dest) {
    sh4matrix_dest.set(sh4matrix_source, 0);
}

function sh4matrix_corner_rotate(sh4matrix, matrix_corner, x, y, width, height) {
    sh4matrix_translate(sh4matrix, x, y);
    sh4matrix_rotate_by_rads(sh4matrix, matrix_corner.angle);
    sh4matrix_translate(sh4matrix, -x, -y);
    sh4matrix_translate(sh4matrix, matrix_corner.x * width, matrix_corner.y * height);
}

function sh4matrix_scale_size(sh4matrix, draw_x, draw_y, scale_x, scale_y) {
    sh4matrix_translate(sh4matrix, draw_x - (draw_x * scale_x), draw_y - (draw_y * scale_y));
    sh4matrix_scale(sh4matrix, scale_x, scale_y);
}



function sh4matrix_internal_rotate(sh4matrix, draw_x, draw_y, draw_width, draw_height, modifier) {
    if (modifier.rotate == 0) return;

    if (!modifier.rotate_pivot_enabled || draw_width < 0 || draw_height < 0) {
        sh4matrix_rotate_by_rads(sh4matrix, modifier.rotate);
        return;
    }

    // the pivot is calculated from inside of the polygon
    let x = draw_x + (modifier.rotate_pivot_u * draw_width);
    let y = draw_y + (modifier.rotate_pivot_v * draw_height);
    sh4matrix_translate(sh4matrix, x, y);
    sh4matrix_rotate_by_rads(sh4matrix, modifier.rotate);
    sh4matrix_translate(sh4matrix, -x, -y);
}

function sh4matrix_internal_calc_scale_offset(sh4matrix, setter, dimmen, scale_dimmen, offset_dimmen) {
    let sign = Math.sign(scale_dimmen);
    let offset = dimmen * (Math.abs(scale_dimmen) - 1.0);
    let translate = offset * sign * offset_dimmen;

    setter(sh4matrix, translate);
}

function sh4matrix_helper_calculate_corner_rotation(matrix_corner, corner) {
    let angle;
    let x;
    let y;

    switch (corner) {
        default:
        case CORNER_TOPLEFT:
            angle = 0;
            x = 0;
            y = 0;
            break;
        case CORNER_TOPRIGHT:
            angle = -90 * MATH2D_DEG_TO_RAD;
            x = -1;
            y = 0;
            break;
        case CORNER_BOTTOMLEFT:
            angle = 90 * MATH2D_DEG_TO_RAD;
            x = 0;
            y = -1;
            break;
        case CORNER_BOTTOMRIGHT:
            angle = 180 * MATH2D_DEG_TO_RAD;
            x = -1;
            y = -1;
            break;
    }

    matrix_corner.x = x;
    matrix_corner.y = y;
    matrix_corner.angle = angle;
}

