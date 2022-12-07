#ifndef __commons_h
#define __commons_h

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

static void print_stub(const char* function_name, const char* format, ...) {
    va_list va_args;
    va_start(va_args, format);
    
    printf("[STUB] %s:  ", function_name);
    vprintf(format, va_args);
    printf("\r\n");

    va_end(va_args);
}

#endif