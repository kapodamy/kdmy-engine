#ifndef _float64_math_support_h
#define _float64_math_support_h

#include "float64.h"


#ifdef FLOAT64_EMULATION_ENABLED

#if __SIZEOF_LONG__ == 8
    #define __FLOAT64_LONG_TYPE long int
#else
    #define __FLOAT64_LONG_TYPE long long int
#endif

    static inline float64 float64_modf(float64 x, float64* intptr) {
        if (x ==  __builtin_infl() || x == -__builtin_infl() || __builtin_isnanl(x)) {
            *intptr = x;
            return x;
        }

        float64 i = (__FLOAT64_LONG_TYPE)x;

        *intptr = i;
        return x - i;
    }

    static inline float64 float64_fabs(float64 x) {
        if (__builtin_isnanl(x)) {
            return x;
        }

        if (x < 0.0L) {
            x = -x;
        }

        return x;
    }

    static inline float64 float64_trunc(float64 x) {
        if (x ==  __builtin_infl() || x == -__builtin_infl() || __builtin_isnanl(x)) {
            return x;
        }

        return (__FLOAT64_LONG_TYPE)x;
    }

    static inline float64 float64_floor(float64 x) {
        if (x ==  __builtin_infl() || x == -__builtin_infl() || __builtin_isnanl(x)) {
            return x;
        }

        __FLOAT64_LONG_TYPE tmp = (__FLOAT64_LONG_TYPE)x;

        if (tmp == x) {
            return x;
        } else if (x < 0.0L){
            return tmp - 1.0L;
        } else {
            return tmp;
        }
    }

    #undef __FLOAT64_LONG_TYPE

#else

    #include <math.h>

    // Alias to modf() function
    static inline float64 float64_modf(float64 x, float64* intptr) { return modf(x, intptr); }

    // Alias to fabs() function
    static inline float64 float64_fabs(float64 x) { return fabs(x); }

    // Alias to trunc() function
    static inline float64 float64_trunc(float64 x) { return trunc(x); }

    // Alias to floor() function
    static inline float64 float64_floor(float64 x) { return floor(x); }

#endif

#endif
