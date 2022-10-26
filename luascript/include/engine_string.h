#ifndef __engine_string_h
#define __engine_string_h

#include <string.h>
#include <stdbool.h>

static inline bool string_equals(const char* string1, const char* string2) {
    if (string1 == string2) return 1;
    return strcmp(string1, string2) == 0 ? true : false;
}

static inline int32_t string_lengthbytes(const char* string) {
    if (!string) return -1;
    return strlen(string);
}

#endif

