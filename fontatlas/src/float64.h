#ifndef _float64_support_h
#define _float64_support_h

#ifndef NO_FORCE_FLOAT64_IMPLEMENTATION
#if __SIZEOF_FLOAT__ == __SIZEOF_DOUBLE__

// Has 64bit floating-point number emulation, the current platform does not support "double" and is implemented as "long double".
#define FLOAT64_EMULATION_ENABLED
    
#endif
#endif


#ifdef FLOAT64_EMULATION_ENABLED
    // 64bit floating-point number (without SH4 FPU acceleration)
    typedef long double float64;

    #define FLOAT64_NAN_CONSTANT __builtin_nanl("")
    #define FLOAT64_INF_CONSTANT __builtin_infl()
    #define FLOAT64_EPSILON __LDBL_EPSILON__

#else

    // 64bit floating-point number (this is a double alias)
    typedef double float64;

    #define FLOAT64_NAN_CONSTANT __builtin_nan("")
    #define FLOAT64_INF_CONSTANT __builtin_inf()
    #define FLOAT64_EPSILON __DBL_EPSILON__

#endif

#endif
