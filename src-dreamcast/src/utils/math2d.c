#include <assert.h>
#include <stdlib.h>
#include <time.h>

#include <arch/rtc.h>
#include <arch/timer.h>

#include "math2d.h"
#include "stringbuilder.h"
#include "stringutils.h"

#include "../externals/xoshiro128.c"

void __attribute__((constructor)) __ctor_math2d() {
    //
    // fake seed xoshiro rng
    //

    time_t t = rtc_unix_secs();
    struct tm* tm = localtime(&t);

    uint32_t tm_hour, tm_min, tm_sec, tm_day;
    if (tm == NULL) {
        tm_sec = (uint32_t)(t & 0x3B);
        tm_min = (uint32_t)((t & 0x3B00) >> 8);
        tm_hour = (uint32_t)(((t & 0x0F0000) >> 16) & 15);
        tm_day = (uint32_t)(((t & 0x700000) >> 20) & 7);
    } else {
        tm_sec = (uint32_t)tm->tm_sec;
        tm_min = (uint32_t)tm->tm_min;
        tm_hour = (uint32_t)tm->tm_hour;
        tm_day = (uint32_t)tm->tm_mday;
    }

    xoshiro128_state[0] += tm_hour;
    xoshiro128_state[1] += tm_min << 8;
    xoshiro128_state[2] += tm_sec;

    uint64_t ms = timer_ms_gettime64();
    uint64_t rnd = ((ms & 0xFF00) >> 8) + ((ms & 0xFF) >> 0);
    int seed = (int)(rnd & 0xFF);

    for (int i = 0; i < seed; i++) {
        xoshiro128_next_starstar();
    }

    for (uint32_t i = 0; i < tm_day; i += 10) {
        xoshiro128_next_starstar();
    }
}

static float math2d_internal_normal_alpha_compose(float bg_alpha, float fg_alpha, float cmst_a, float bg_cmpt, float fg_cmpt);


float math2d_random_float() {
    uint32_t value = xoshiro128_next_plus();

    union {
        uint32_t integer;
        float fp;
    } casted_value = {
        .integer = value >> 9 | 0x3f800000
    };

    return casted_value.fp - 1.0f;
}

float math2d_random_float_range(float min, float max) {
    float pecent = math2d_random_float();
    return pecent * (max - min) + min;
}

int32_t math2d_random_int(int32_t min, int32_t max) {
    uint32_t value = xoshiro128_next_starstar();

    union {
        uint32_t integer_unsigned;
        int32_t integer_signed;
    } casted_value = {
        .integer_signed = max - min
    };

    return casted_value.integer_signed ? min + (int32_t)(value % casted_value.integer_unsigned) : min;
}

uint32_t math2d_random_uint(uint32_t min, uint32_t max) {
    uint32_t value = xoshiro128_next_starstar();

#ifdef DEBUG
    assert(min < max);
#endif

    uint32_t diff = max - min;

    return diff ? min + (value % diff) : min;
}


void math2d_color_blend_normal(const RGBA back_color, const RGBA front_color, RGBA output_color) {
    //
    // Avoid useless calculations if both colors are equal, also both alphas must be 1.0
    //
    for (size_t i = 0; i < 4; i++) {
        if (back_color[i] != front_color[i]) goto L_check_back_color;
    }

    if (back_color[3] == 1.0f /*&& front_color[3] == 1.0f*/) {
        // rgb components are equal an their alphas are 1.0f
        if (front_color != output_color && back_color != output_color) {
            for (size_t i = 0; i < 4; i++) output_color[i] = front_color[i];
        }
        return;
    }

L_check_back_color:
    if (back_color[3] <= 0) {
        // back color is transparent, return the front color
        if (front_color != output_color) {
            for (size_t i = 0; i < 4; i++) output_color[i] = front_color[i];
        }
        return;
    } else if (front_color[3] <= 0) {
        // front color is transparent, return the back color
        if (back_color != output_color) {
            for (size_t i = 0; i < 4; i++) output_color[i] = back_color[i];
        }
        return;
    }

    //
    // Color blending algorithm starts here
    //

    float ba = back_color[3], fa = front_color[3];

    if (ba <= 0.0) {
        output_color[0] = front_color[0];
        output_color[1] = front_color[1];
        output_color[2] = front_color[2];
        output_color[3] = fa;
        return;
    }

    float ca = fa + ba - fa * ba;
    float r = math2d_internal_normal_alpha_compose(ba, fa, ca, back_color[0], front_color[0]);
    float g = math2d_internal_normal_alpha_compose(ba, fa, ca, back_color[1], front_color[1]);
    float b = math2d_internal_normal_alpha_compose(ba, fa, ca, back_color[2], front_color[2]);

    /*
    // round final color components
    r = roundf(r);
    g = roundf(g);
    b = roundf(b);
    */

    output_color[0] = r;
    output_color[1] = g;
    output_color[2] = b;
    output_color[3] = ca;
}


uint32_t math2d_color_floats_to_bytes(float* float_array, bool has_alpha) {
    size_t length = has_alpha ? 4 : 3;
    size_t shift = has_alpha ? 24 : 16;
    uint32_t argb = 0x00;

    //
    // Array disposition:
    //      rgb8    [r, g, b]
    //      rgba8   [r, g, b, a]
    //

    for (size_t i = 0; i < length; i++) {
        float percent = float_array[i];

        // Note: values must be already clamped to [0.0, 1.0]
        // percent = math2d_clamp(percent, 0.0f, 1.0f);

        uint32_t c = (uint32_t)(percent * 255.0f) & 0xFF;
        argb |= c << shift;
        shift -= 8;
    }

    return argb;
}

void math2d_color_bytes_to_floats(uint32_t rgba8_color, bool has_alpha, float* output_float_array) {
    if (has_alpha) {
        output_float_array[0] = ((rgba8_color >> 24) & 0xFF) / 255.0f;
        output_float_array[1] = ((rgba8_color >> 16) & 0xFF) / 255.0f;
        output_float_array[2] = ((rgba8_color >> 8) & 0xFF) / 255.0f;
        output_float_array[3] = ((rgba8_color >> 0) & 0xFF) / 255.0f;
    } else {
        output_float_array[0] = ((rgba8_color >> 16) & 0xFF) / 255.0f;
        output_float_array[1] = ((rgba8_color >> 8) & 0xFF) / 255.0f;
        output_float_array[2] = ((rgba8_color >> 0) & 0xFF) / 255.0f;
    }
}


int32_t math2d_poweroftwo_calc(int32_t dimmen) {
    int32_t size = 2;
    while (size < 0x80000) {
        if (size >= dimmen) break;
        size *= 2;
    }

    assert(size < 0x80000);

    return size;
}


char* math2d_timestamp_to_string(float64 timestamp) {
    static const char* TIME_SECONDS = "$1ds";    // 1.2s
    static const char* TIME_MINUTES = "$im$2is"; // 1m23s
    static const char* TIME_HOURS = "$ih$2i$2i"; // 1h23m45s

    if (math2d_is_double_NaN(timestamp)) return string_duplicate("--:--.---");

    timestamp /= 1000.0;
    float64 h = float64_floor(timestamp / (float64)3600.0);
    float64 m = float64_floor((timestamp - (h * (float64)3600.0)) / (float64)60.0);
    float64 s = timestamp - (h * (float64)3600.0) - (m * (float64)60.0);

    StringBuilder stringbuilder = stringbuilder_init(9);

    if (h > 0.0)
        stringbuilder_add_format(stringbuilder, TIME_HOURS, float64_trunc(h), float64_trunc(m), float64_trunc(s));
    else if (m > 0.0)
        stringbuilder_add_format(stringbuilder, TIME_MINUTES, float64_trunc(m), float64_trunc(s));
    else
        stringbuilder_add_format(stringbuilder, TIME_SECONDS, s);

    char* str = stringbuilder_finalize(&stringbuilder);

    return str;
}

int math2d_float_comparer(float a, float b) {
    if (a < b) return -1;
    if (a > b) return 1;
    if (a == b) return 0;

    if (math2d_is_float_NaN(a)) {
        if (math2d_is_float_NaN(b))
            return 0;
        else
            return -1;
    } else {
        return 1;
    }
}

int math2d_double_comparer(float64 a, float64 b) {
    if (a < b) return -1;
    if (a > b) return 1;
    if (a == b) return 0;

    if (math2d_is_double_NaN(a)) {
        if (math2d_is_double_NaN(b))
            return 0;
        else
            return -1;
    } else {
        return 1;
    }
}


static float math2d_internal_normal_alpha_compose(float bg_alpha, float fg_alpha, float cmst_a, float bg_cmpt, float fg_cmpt) {
    float s = fg_alpha / cmst_a;
    float t = (1.0f - s) * bg_cmpt;
    float u = (1.0f - bg_alpha) * fg_cmpt + bg_alpha * fg_cmpt /*composite color component*/;

    /* u = roundf(u); */
    return t + (s * u);
}
