#ifndef _math2d_h
#define _math2d_h

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

float math2d_lerp(float start, float end, float step) {
    return start + (end - start) * step;
}

float math2d_inverselerp(float start, float end, float value) {
    return (value - start) / (end - start);
}

float math2d_nearestdown(float value, float step) {
    return value - fmodf(value, step);
}

void math2d_rotate_point(float radians, float* x, float* y) {
    float X = *x;
    float Y = *y;

    float s = sinf(radians);
    float c = cosf(radians);
    
    *x = X * c - Y * s;
    *y = X * s + Y * c;
}

float math2d_points_distance(float x1, float y1, float x2, float y2) {
    return sqrtf(powf(x1 - x2, 2.0f) + powf(y1 - y2, 2.0f));
}

#endif