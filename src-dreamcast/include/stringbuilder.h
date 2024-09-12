#ifndef _stringbuilder_h
#define _stringbuilder_h

#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>


typedef struct StringBuilder_s* StringBuilder;


StringBuilder stringbuilder_init(size_t initial_capacity);
void stringbuilder_destroy(StringBuilder* stringbuilder);
char* stringbuilder_finalize(StringBuilder* stringbuilder);
void stringbuilder_clear(StringBuilder stringbuilder);
const char* const* stringbuilder_intern(StringBuilder stringbuilder);

void stringbuilder_add(StringBuilder stringbuilder, const char* string);
void stringbuilder_add_char(StringBuilder stringbuilder, const char value);
void stringbuilder_add_substring(StringBuilder stringbuilder, const char* string, int32_t start, int32_t end);
char* stringbuilder_get_copy(StringBuilder stringbuilder);

void stringbuilder_add_substring(StringBuilder stringbuilder, const char* string, int32_t start, int32_t end);
void stringbuilder_add_char_codepoint(StringBuilder stringbuilder, uint32_t codepoint);
void stringbuilder_add_with_replace(StringBuilder stringbuilder, const char* source_string, const char* old_substring, const char* new_substring);
void stringbuilder_lowercase(StringBuilder stringbuilder);

void stringbuilder_add_format(StringBuilder stringbuilder, const char* format, ...);
void stringbuilder_add_format2(StringBuilder stringbuilder, const char* format, va_list params);
char* stringbuilder_helper_create_formatted_string(const char* format, ...);
char* stringbuilder_helper_create_formatted_string2(const char* format, va_list params);

size_t stringbuilder_get_length(StringBuilder stringbuilder);
size_t stringbuilder_get_bytelength(StringBuilder stringbuilder);

#endif
