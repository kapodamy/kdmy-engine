#ifndef _stringutils_h
#define _stringutils_h

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "logger.h"


#define string_is_empty(str) (!str || str[0] == '\0')
#define string_is_not_empty(str) (str && str[0] != '\0')


typedef struct {
    uint32_t code;
    uint8_t size;
} Grapheme;


char* string_to_lowercase(const char* str);
char* string_to_uppercase(const char* str);

size_t string_to_uppercase_length(const char* str);
size_t string_to_lowercase_length(const char* str);

size_t string_from_codepoint(uint8_t buffer[4], uint32_t code);
size_t string_length_from_codepoint(uint32_t code);
bool string_get_character_codepoint(const char* utf8_string, int32_t index, size_t string_length, Grapheme* grapheme);

int32_t string_occurrences_of_char(const char* string, char value);
int32_t string_occurrences_of_string(const char* string, const char* substring);

int string_natural_comparer(const char* x, const char* y);

bool string_starts_with(const char* string, const char* substring);
bool string_equals(const char* str1, const char* str2);
bool string_equals_ignore_case(const char* str1, const char* str2);

char* string_duplicate(const char* string);

int32_t string_index_of_char(const char* string, int32_t start_index, char value);
int32_t string_index_of(const char* string, int32_t start_index, const char* substring);
int32_t string_index_of_any_char(const char* string, const char* utf8_chars_list);

int32_t string_last_index_of_char(const char* string, char value);
int32_t string_last_index_of_string(const char* string, int32_t start_index, const char* value);

char* string_substring(const char* string, int32_t start, int32_t end);

char* string_concat_for_state_name(size_t amount, ...);
char* string_concat(size_t count, ...);
char* string_concat_with_substring(const char* substring, int32_t start, int32_t end, int32_t insert_index, size_t count, ...);

// Returns the length of the string including the NULL-character terminator
size_t string_get_bytelength(const char* string);
// int32_t version of _strlen()_ with NULL string support
int32_t string_get_length(const char* string);

bool string_ends_with(const char* string, const char* substring);
bool string_lowercase_ends_with(const char* string, const char* substring);

char* string_copy_and_insert(const char* string, int32_t index, const char* substring);
char* string_replace(const char* string, const char* old_substring, const char* new_substring);
char* string_replace_char(const char* string, const char old_char, const char new_new_char);
char* string_trim(const char* string, bool trim_start, bool trim_end);

#endif
