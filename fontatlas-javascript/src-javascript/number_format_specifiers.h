#ifndef _int_format_specifiers_h
#define _int_format_specifiers_h

#include <stdint.h>

#include "float64.h"


#ifdef _arch_dreamcast

#define FMTL_INT64 "%*lli"
#define FMTL_INT32 "%*li"
#define FMTL_UINT64 "%*llu"
#define FMTL_UINT32 "%*lu"

#define FMT_I8 "%lli"
#define FMT_I4 "%li"
#define FMT_U8 "%llu"
#define FMT_U4 "%lu"

#define FMT_ZU "%u"
#define FMT_ZI "%i"

#define FMT_FLT "%d"

#ifdef FLOAT64_EMULATION_ENABLED
    #define FMT_FLT64 "%Lf"
    #define FMT_FLT64_DCMLS(PARAMS) "%"#PARAMS"Lf"
#else
    #define FMT_FLT64 "%f"
    #define FMT_FLT64_DCMLS(PARAMS) "%"#PARAMS"f"
#endif

#else

#define FMTL_INT64 "%*li"
#define FMTL_INT32 "%*i"
#define FMTL_UINT64 "%*lu"
#define FMTL_UINT32 "%*u"

#define FMT_I8 "%li"
#define FMT_I4 "%i"
#define FMT_U8 "%lu"
#define FMT_U4 "%u"

#define FMT_ZU "%zu"
#define FMT_ZI "%zi"

#define FMT_FLT "%f"
#define FMT_FLT64 "%f"
#define FMT_FLT64_DCMLS(PARAMS) "%"#PARAMS"f"

#endif

#endif
