"use strict";

const MATH2D_DEG_TO_RAD = Math.PI / 180.0;
const MATH2D_MAX_INT32 = 0x7FFFFFFF;
const MATH2D_LOG100 = Math.log(100);
const MATH2D_HALF_PI = Math.PI / 2.0;
const MATH2D_PRNG = new Uint32Array(1);

function math2d_random(min, max) {
    //return Math.random() * (max - min + 1) + min;
    window.crypto.getRandomValues(MATH2D_PRNG);
    let pecent = MATH2D_PRNG[0] / 0xffffffff;
    return pecent * (max - min + 1) + min;
}

function math2d_random_int(min, max) {
    let value = math2d_random(min, max);
    value = Math.trunc(value);

    if (value < min) return min;
    if (value > max) return max;
    return value;
}


function math2d_lerp(start, end, step) {
    return start + (end - start) * step;
}

function math2d_inverselerp(start, end, value) {
    return (value - start) / (end - start);
}

function math2d_nearestdown(value, step) {
    return value - (value % step);
}

function math2d_cubicbezier(offset, point0, point1, point2, point3) {
    let neg = 1 - offset;
    return (neg * neg * neg) * point0 +
        3 * offset * (neg * neg) * point1 +
        3 * (offset * offset) * neg * point2 +
        (offset * offset * offset) * point3;
}

function math2d_clamp(value, min, max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

function math2d_rotate_point(point, radians) {
    let s = Math.sin(radians);
    let c = Math.cos(radians);
    let x = point[0] * c - point[1] * s;
    let y = point[0] * s + point[1] * c;

    point[0] = x;
    point[1] = y;

    return point;
}

function math2d_color_blend_normal(back_color, front_color, output_color) {
    //
    // Avoid useless calculations if both colors are equal, also both alphas must be 1.0
    //
    L_check_equal: {
        for (let i = 0; i < 4; i++) {
            if (back_color[i] != front_color[i]) break L_check_equal;
        }

        if (back_color[3] == 1.0/* && front_color[3] == 1.0*/) {
            // rgb components are equal an their alphas are 1.0
            if (front_color != output_color && back_color != output_color) {
                for (let i = 0; i < 4; i++) output_color[i] = front_color[i];
            }
            return;
        }
    }

    if (back_color[3] <= 0) {
        // back color is transparent, return the front color
        if (front_color != output_color) {
            for (let i = 0; i < 4; i++) output_color[i] = front_color[i];
        }
        return;
    } else if (front_color[3] <= 0) {
        // front color is transparent, return the back color
        if (back_color != output_color) {
            for (let i = 0; i < 4; i++) output_color[i] = back_color[i];
        }
        return;
    }

    //
    // Color blending algorithm starts here
    //

    function normal_alpha_compose(bg_alpha, fg_alpha, cmst_a, bg_cmpt, fg_cmpt) {
        let s = fg_alpha / cmst_a;
        let t = (1 - s) * bg_cmpt;
        let u = (1 - bg_alpha) * fg_cmpt + bg_alpha * fg_cmpt/*composite color component*/;

        /* u = math2d_roundf(u); */
        return t + (s * u);
    }

    let ba = back_color[3], fa = front_color[3];

    if (ba <= 0.0) {
        output_color[0] = front_color[0];
        output_color[1] = front_color[1];
        output_color[2] = front_color[2];
        output_color[3] = fa;
        return;
    }

    let ca = fa + ba - fa * ba;
    let r = normal_alpha_compose(ba, fa, ca, back_color[0], front_color[0]);
    let g = normal_alpha_compose(ba, fa, ca, back_color[1], front_color[1]);
    let b = normal_alpha_compose(ba, fa, ca, back_color[2], front_color[2]);

    /*
    // round final color components
    r = math2d_roundf(r);
    g = math2d_roundf(g);
    b = math2d_roundf(b);
    */

    output_color[0] = r;
    output_color[1] = g;
    output_color[2] = b;
    output_color[3] = ca;
}

function math2d_color_floats_to_bytes(float_array, has_alpha) {
    let length = has_alpha ? 4 : 3;
    let shift = has_alpha ? 24 : 16;
    let argb = 0x00;

    //
    // Array disposition:
    //      rgb8    [r, g, b]
    //      rgba8   [r, g, b, a]
    //

    for (let i = 0; i < length; i++) {
        let c = (float_array[i] * 255) & 0xFF;
        argb |= c << shift;
        shift -= 8;
    }

    return argb;
}

function math2d_color_bytes_to_floats(rgba8_color, has_alpha, output_float_array) {
    let length = has_alpha ? 4 : 3;
    let shift = has_alpha ? 24 : 16;

    for (let i = 0; i < length; i++) {
        let c = (rgba8_color >>> shift) & 0xFF;
        output_float_array[i] = c / 255.0;
        shift -= 8;
    }

}

/** @deprecated */
function math2d_color_float_to_css_color(float_array, has_alpha) {
    let css_color = "#";

    function hex(array, index) {
        let c = array[index];
        c = Math.trunc(c * 255);
        c = c.toString(16);
        if (c.length < 2) c = "0".concat(c);
        return c;
    }

    let length = has_alpha ? 4 : 3;

    for (let i = 0; i < length; i++)
        css_color += hex(float_array, i);

    return css_color;
}

function math2d_bitcount(value) {
    value = value - ((value >>> 1) & 0x55555555)
    value = (value & 0x33333333) + ((value >>> 2) & 0x33333333)
    return ((value + (value >>> 4) & 0xF0F0F0F) * 0x1010101) >>> 24
}

function math2d_beats_per_minute_to_beat_per_milliseconds(bmp) {
    return 60000 / bmp;
}

function math2d_points_distance(x1, y1, x2, y2) {
    return Math.sqrt(Math.pow(x1 - x2, 2) + Math.pow(y1 - y2, 2));
}

function math2d_poweroftwo_calc(dimmen) {
    let size = 2;
    while (size < 0x80000) {
        if (size >= dimmen) break;
        size *= 2;
    }

    if (size >= 0x80000) throw new Error("math2d_poweroftwo_calc() failed for: " + dimmen);
    return size;
}

function math2d_floats_are_near_equal(float1, float2) {
    return Math.abs(float1 - float2) < Number.EPSILON;
}

function math2d_float_are_near_zero(number) {
    return Math.abs(number) < Number.EPSILON;
}

function math2d_timestamp_to_string(timestamp) {
    const TIME_SECONDS = "$1ds";// 1.2s
    const TIME_MINUTES = "$im$2is";// 1m23s
    const TIME_HOURS = "$ih$2i$2i";// 1h23m45s

    if (Number.isNaN(timestamp)) return "--:--.---";

    timestamp /= 1000.0;
    let h = Math.floor(timestamp / 3600.0);
    let m = Math.floor((timestamp - (h * 3600.0)) / 60.0);
    let s = timestamp - (h * 3600.0) - (m * 60.0);

    let stringbuilder = stringbuilder_init(9);

    if (h > 0.0)
        stringbuilder_add_format(stringbuilder, TIME_HOURS, Math.trunc(h), Math.trunc(m), Math.trunc(s));
    else if (m > 0.0)
        stringbuilder_add_format(stringbuilder, TIME_MINUTES, Math.trunc(m), Math.trunc(s));
    else
        stringbuilder_add_format(stringbuilder, TIME_SECONDS, s);

    let str = stringbuilder_finalize(stringbuilder);

    return str;
}

function math2d_lerp_cubic(percent) {
    return Math.pow(percent, 3);
}

function math2d_lerp_quad(percent) {
    return Math.pow(percent, 2);
}

function math2d_lerp_expo(percent) {
    return Math.pow(2.0, 10.0 * (percent - 1));
}

function math2d_lerp_sin(percent) {
    return Math.sin(percent * MATH2D_HALF_PI);
}
