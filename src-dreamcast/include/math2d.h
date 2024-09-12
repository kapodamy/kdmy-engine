#ifndef _math2d_h
#define _math2d_h

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if defined(_arch_dreamcast)
// STUB "__FMINLINE" to avoid "unknown register name" warnings
#ifdef __INTELLISENSE__
#define __FMINLINE static inline
#else
#include <dc/fmath.h>
#endif
#else

#define __FMINLINE static inline

#define fsqrt sqrtf
#define fsin sinf
#define fcos cosf

#endif

// imports: powf
#include <math.h>

// imports: fabsf FLT_EPSILON
#include <float.h>

#include "color2d.h"
#include "float64.h"
#include "float64_math.h"


#define MATH2D_PI 3.1415927F
#define MATH2D_DEG_TO_RAD 0.017453292F
#define MATH2D_RAD_TO_DEG 57.295778F
#define MATH2D_MAX_INT32 INT32_MAX
#define MATH2D_LOG100 4.6051702F
#define MATH2D_HALF_PI 1.5707964F

#define FLOAT_NaN __builtin_nanf("")
#define FLOAT_Inf __builtin_inff()

#define DOUBLE_NaN FLOAT64_NAN_CONSTANT
#define DOUBLE_Inf FLOAT64_INF_CONSTANT

#define math2d_float_assert_NaN(value) assert(!math2d_is_float_NaN(value));
#define math2d_double_assert_NaN(value) assert(!math2d_is_double_NaN(value));


#ifndef __STDC_VERSION__
#error "math2d is compatible with C11 or newer"
#elif __STDC_VERSION__ < 201112L
#error "math2d is compatible with C11 or newer"
#endif


float math2d_random_float();
float math2d_random_float_range(float min, float max);
int32_t math2d_random_int(int32_t min, int32_t max);
uint32_t math2d_random_uint(uint32_t min, uint32_t max);
void math2d_color_blend_normal(const RGBA back_color, const RGBA front_color, RGBA output_color);


__FMINLINE float math2d_lerp(float start, float end, float step) {
    return start + (end - start) * step;
}

__FMINLINE float math2d_inverselerp(float start, float end, float value) {
    return (value - start) / (end - start);
}

__FMINLINE float math2d_nearestdown(float value, float step) {
    return value - fmodf(value, step);
}

__FMINLINE float math2d_cubicbezier(float offset, float point0, float point1, float point2, float point3) {
    float neg = 1.0f - offset;
    return (neg * neg * neg) * point0 +
           3 * offset * (neg * neg) * point1 +
           3 * (offset * offset) * neg * point2 +
           (offset * offset * offset) * point3;
}


__FMINLINE float64 math2d_min_double(float64 number1, float64 number2) {
    return number1 < number2 ? number1 : number2;
}

__FMINLINE float64 math2d_max_double(float64 number1, float64 number2) {
    return number1 > number2 ? number1 : number2;
}


__FMINLINE float math2d_min_float(float number1, float number2) {
    return number1 < number2 ? number1 : number2;
}

__FMINLINE float math2d_max_float(float number1, float number2) {
    return number1 > number2 ? number1 : number2;
}


__FMINLINE int32_t math2d_min_int(int32_t number1, int32_t number2) {
    return number1 < number2 ? number1 : number2;
}


__FMINLINE size_t math2d_min_nint(size_t number1, size_t number2) {
    return number1 < number2 ? number1 : number2;
}

__FMINLINE int32_t math2d_max_int(int32_t number1, int32_t number2) {
    return number1 > number2 ? number1 : number2;
}


__FMINLINE float math2d_clamp_float(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

__FMINLINE int32_t math2d_clamp_int(int32_t value, int32_t min, int32_t max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

__FMINLINE float64 math2d_clamp_double(float64 value, float64 min, float64 max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}


__FMINLINE void math2d_rotate_point(float radians, float* point_x, float* point_y) {
    float x = *point_x, y = *point_y;

    float s = fsin(radians);
    float c = fcos(radians);
    x = x * c - y * s;
    y = x * s + y * c;

    *point_x = x;
    *point_y = y;
}


__FMINLINE int math2d_bitcount(uint32_t value) {
    /*value = value - ((value >> 1) & 0x55555555);
    value = (value & 0x33333333) + ((value >> 2) & 0x33333333);
    return ((value + (value >> 4) & 0xF0F0F0F) * 0x1010101) >> 24;*/

#if DEBUG
#include <assert.h>
    assert(sizeof(unsigned long) == sizeof(uint32_t));
#endif

    return __builtin_popcountl(value);
}

__FMINLINE float math2d_beats_per_minute_to_beat_per_milliseconds(float bmp) {
    return 60000.0f / bmp;
}

__FMINLINE float math2d_points_distance(float x1, float y1, float x2, float y2) {
    return fsqrt(powf(x1 - x2, 2) + powf(y1 - y2, 2));
}


uint32_t math2d_color_floats_to_bytes(float* float_array, bool has_alpha);
void math2d_color_bytes_to_floats(uint32_t rgba8_color, bool has_alpha, float* output_float_array);
int32_t math2d_poweroftwo_calc(int32_t dimmen);


__FMINLINE bool math2d_floats_are_near_equal(float number1, float number2) {
    return fabsf(number1 - number2) < FLT_EPSILON;
}

__FMINLINE bool math2d_doubles_are_near_equal(float64 number1, float64 number2) {
    return float64_fabs(number1 - number2) < FLOAT64_EPSILON;
}

__FMINLINE bool math2d_float_is_near_zero(float number) {
    return fabsf(number) < FLT_EPSILON;
}


__FMINLINE bool math2d_floats_are_greater_or_near_equal(float number1, float number2) {
    return (number1 > number2) || fabsf(number1 - number2) < FLT_EPSILON;
}

__FMINLINE bool math2d_floats_are_less_or_near_equal(float number1, float number2) {
    return (number1 < number2) || fabsf(number1 - number2) < FLT_EPSILON;
}


char* math2d_timestamp_to_string(float64 timestamp);

int math2d_float_comparer(float a, float b);
int math2d_double_comparer(float64 a, float64 b);

__FMINLINE float math2d_lerp_cubic(float percent) {
    return powf(percent, 3.0f);
}

__FMINLINE float math2d_lerp_quad(float percent) {
    return powf(percent, 2.0f);
}

__FMINLINE float math2d_lerp_expo(float percent) {
    return powf(2.0f, 10.0f * (percent - 1.0f));
}

__FMINLINE float math2d_lerp_sin(float percent) {
    return fsin(percent * MATH2D_HALF_PI);
}


__FMINLINE bool math2d_is_float_NaN(float value) {
    return isnan(value);
}

__FMINLINE bool math2d_is_float_Inf(float value) {
    return isinf(value);
}

__FMINLINE bool math2d_is_float_finite(float value) {
    return isfinite(value);
}


__FMINLINE bool math2d_is_double_NaN(float64 value) {
    return isnan(value);
}

__FMINLINE bool math2d_is_double_Inf(float64 value) {
    return isinf(value);
}

__FMINLINE bool math2d_is_double_finite(float64 value) {
    return isfinite(value);
}


__FMINLINE float math2d_sin(float r) {
    return fsin(r);
}

__FMINLINE float math2d_cos(float r) {
    return fcos(r);
}


__FMINLINE float math2d_sign(float v) {
    return __builtin_signbitf(v) ? -1.0f : 1.0f;
}

__FMINLINE bool math2d_is_float_sign_negative(float v) {
    return !!__builtin_signbitf(v);
}

#endif
