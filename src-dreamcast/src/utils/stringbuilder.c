#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "float64.h"
#include "float64_math.h"
#include "malloc_utils.h"
#include "number_format_specifiers.h"
#include "stringbuilder.h"
#include "stringutils.h"


#define STRINGBUILDER_CHUNK_SIZE 64

#define WRITE_CHAR(BUFFER, CHAR, REPEATS, COUNTER) \
    if (REPEATS < 1) REPEATS = 1;                  \
    if (BUFFER) {                                  \
        for (int r = 0; r < REPEATS; r++) {        \
            *BUFFER++ = CHAR;                      \
        }                                          \
    }                                              \
    COUNTER += (size_t)REPEATS;

#define WRITE_STRING(BUFFER, STRING, STRING_LENGTH, REPEATS, COUNTER) \
    if (REPEATS < 1) REPEATS = 1;                                     \
    if (BUFFER) {                                                     \
        for (int r = 0; r < REPEATS; r++) {                           \
            memcpy(BUFFER, STRING, STRING_LENGTH);                    \
            BUFFER += STRING_LENGTH;                                  \
        }                                                             \
    }                                                                 \
    COUNTER += STRING_LENGTH * (size_t)REPEATS;

#define WRITE_CASE_STRING(BUFFER, STRING, PARAMS, CASE, REPEATS, COUNTER) \
    {                                                                     \
        STRING = va_arg(params, const char*);                             \
        if (BUFFER) {                                                     \
            char* tmp = string_to_##CASE(STRING);                         \
            if (tmp) {                                                    \
                size_t tmp_length = strlen(tmp);                          \
                WRITE_STRING(BUFFER, tmp, tmp_length, REPEATS, COUNTER);  \
                free_chk(tmp);                                            \
            }                                                             \
        } else {                                                          \
            size_t tmp_len = string_to_##CASE##_length(STRING);           \
            if (REPEATS > 1) tmp_len *= (size_t)REPEATS;                  \
            COUNTER += tmp_len;                                           \
        }                                                                 \
    }

#define WRITE_INTEGER(BUFFER, TYPE, FMT, PARAMS, PADDING, COUNTER)                       \
    {                                                                                    \
        const TYPE##_t arg_##TYPE = va_arg(PARAMS, const TYPE##_t);                      \
        int integer_length = snprintf(BUFFER, UINT8_MAX, FMT, (int)PADDING, arg_##TYPE); \
        assert(integer_length >= 0);                                                     \
        if (BUFFER) BUFFER += (size_t)integer_length;                                    \
        COUNTER += (size_t)integer_length;                                               \
    }



struct StringBuilder_s {
    size_t capacity;
    size_t length;
    char* buffer;
};


static void stringbuilder_internal_ensure_capacity(StringBuilder stringbuilder, size_t new_length);
static size_t stringbuilder_internal_build_format(char* output_buffer, bool add_terminator, const char* format, va_list params);


StringBuilder stringbuilder_init(size_t initial_capacity) {
    if (initial_capacity < 1) initial_capacity = STRINGBUILDER_CHUNK_SIZE;

    StringBuilder stringbuilder = malloc_chk(sizeof(struct StringBuilder_s));
    malloc_assert(stringbuilder, StringBuilder);

    *stringbuilder = (struct StringBuilder_s){
        .capacity = initial_capacity,
        .length = 0,
        .buffer = malloc_chk(initial_capacity),
    };
    malloc_assert(stringbuilder->buffer, char*);

    stringbuilder->buffer[0] = '\0';

    return stringbuilder;
}

void stringbuilder_destroy(StringBuilder* stringbuilder) {
    if (!*stringbuilder) return;

    free_chk((*stringbuilder)->buffer);
    free_chk(*stringbuilder);

    *stringbuilder = NULL;
}

char* stringbuilder_finalize(StringBuilder* stringbuilder_ptr) {
    StringBuilder stringbuilder = *stringbuilder_ptr;
    if (!stringbuilder) return NULL;

    char* str = stringbuilder->buffer;
    size_t length = stringbuilder->length + 1;

    if (stringbuilder->capacity > length) {
        str = realloc_chk(str, length);
        assert(str);
    }

    free_chk(stringbuilder);
    *stringbuilder_ptr = NULL;

    return str;
}

void stringbuilder_clear(StringBuilder stringbuilder) {
    *stringbuilder->buffer = '\0';
    stringbuilder->length = 0;
}

const char* const* stringbuilder_intern(StringBuilder stringbuilder) {
    return (const char* const*)&stringbuilder->buffer;
}


void stringbuilder_add(StringBuilder stringbuilder, const char* string) {
    if (!string || string[0] == '\0') return;

    size_t length = stringbuilder->length;
    size_t string_length = strlen(string);
    stringbuilder_internal_ensure_capacity(stringbuilder, length + string_length);

    char* ptr = stringbuilder->buffer + length;
    memcpy(ptr, string, string_length);

    ptr[string_length] = '\0';
}

void stringbuilder_add_char(StringBuilder stringbuilder, const char value) {
    size_t length = stringbuilder->length;
    stringbuilder_internal_ensure_capacity(stringbuilder, length + 1);

    char* ptr = stringbuilder->buffer + length;
    *ptr++ = value;
    *ptr++ = '\0';
}

char* stringbuilder_get_copy(StringBuilder stringbuilder) {
    size_t length = stringbuilder->length + 1;

    char* string = malloc_chk(length);
    malloc_assert(string, char*);

    memcpy(string, stringbuilder->buffer, length);

    return string;
}


void stringbuilder_add_substring(StringBuilder stringbuilder, const char* string, int32_t start, int32_t end) {
    if (!string) return;

    size_t substring_length = strlen(string);
    assert(start >= 0 && start < end && end <= substring_length);

    substring_length = (size_t)(end - start);

    size_t length = stringbuilder->length;
    stringbuilder_internal_ensure_capacity(stringbuilder, length + substring_length);

    char* ptr = stringbuilder->buffer + length;
    memcpy(ptr, string + start, substring_length);

    ptr[substring_length] = '\0';

    return;
}

void stringbuilder_add_char_codepoint(StringBuilder stringbuilder, uint32_t codepoint) {
    if (!codepoint) return;

    uint8_t buffer[5];

    size_t bytes = string_from_codepoint(buffer, codepoint);
    if (bytes < 1 /*|| bytes > 4*/) return;

    buffer[bytes] = '\0';
    stringbuilder_add(stringbuilder, (const char*)buffer);
}

void stringbuilder_add_with_replace(StringBuilder stringbuilder, const char* source_string, const char* old_substring, const char* new_substring) {
    if (old_substring == NULL) {
        stringbuilder_add(stringbuilder, source_string);
        return;
    }

    size_t occurrences = (size_t)string_occurrences_of_string(source_string, old_substring);
    if (occurrences < 1) {
        stringbuilder_add(stringbuilder, source_string);
        return;
    }

    size_t source_string_length = strlen(source_string);
    size_t old_substring_length = strlen(old_substring);
    size_t new_substring_length = string_is_empty(new_substring) ? 0 : strlen(new_substring);

    size_t final_source_string_length = (source_string_length - (occurrences * old_substring_length)) + (occurrences * new_substring_length);
    size_t stringbuilder_length = stringbuilder->length;

    stringbuilder_internal_ensure_capacity(stringbuilder, stringbuilder_length + final_source_string_length);

    const char* source_string_end = source_string + strlen(source_string);
    char* buffer_ptr = stringbuilder->buffer + stringbuilder_length;

    size_t orig_length;
    while (true) {
        const char* ptr = strstr(source_string, old_substring);
        if (!ptr) {
            orig_length = (size_t)(source_string_end - source_string);
            memcpy(buffer_ptr, source_string, orig_length);
            buffer_ptr[orig_length] = '\0';
            break;
        }

        orig_length = (size_t)(ptr - source_string);
        memcpy(buffer_ptr, source_string, orig_length);
        buffer_ptr += orig_length;

        memcpy(buffer_ptr, new_substring, new_substring_length);
        buffer_ptr += new_substring_length;

        source_string = ptr + old_substring_length;
    }
}

void stringbuilder_lowercase(StringBuilder stringbuilder) {
    char* buffer_lw = string_to_lowercase(stringbuilder->buffer);
    size_t buffer_lw_length = strlen(buffer_lw);

    stringbuilder_internal_ensure_capacity(stringbuilder, buffer_lw_length);

    memcpy(stringbuilder->buffer, buffer_lw, buffer_lw_length);
    free_chk(buffer_lw);
}


void stringbuilder_add_format(StringBuilder stringbuilder, const char* format, ...) {
    va_list params;
    va_start(params, format);
    stringbuilder_add_format2(stringbuilder, format, params);
    va_end(params);
}

void stringbuilder_add_format2(StringBuilder stringbuilder, const char* format, va_list params) {
    va_list params_orig;
    va_copy(params_orig, params);

    size_t calculated_length = stringbuilder_internal_build_format(NULL, false, format, params);
    size_t buffer_length = stringbuilder->length;

    stringbuilder_internal_ensure_capacity(stringbuilder, buffer_length + calculated_length);

    char* ptr = stringbuilder->buffer + buffer_length;
    size_t formatted_string_length = stringbuilder_internal_build_format(ptr, true, format, params_orig) - 1;
    va_end(params_orig);

    assert(calculated_length == formatted_string_length);
}

char* stringbuilder_helper_create_formatted_string(const char* format, ...) {
    va_list params;
    va_start(params, format);

    char* formatted_string = stringbuilder_helper_create_formatted_string2(format, params);

    va_end(params);
    return formatted_string;
}

char* stringbuilder_helper_create_formatted_string2(const char* format, va_list params) {
    va_list params_orig;
    va_copy(params_orig, params);

    size_t calculated_length = stringbuilder_internal_build_format(NULL, true, format, params_orig);
    va_end(params_orig);

    char* formatted_string = malloc_chk(calculated_length);
    malloc_assert(formatted_string, char*);

    size_t formatted_string_length = stringbuilder_internal_build_format(formatted_string, true, format, params);

    assert(calculated_length == formatted_string_length);
    return formatted_string;
}


size_t stringbuilder_get_length(StringBuilder stringbuilder) {
    size_t length = stringbuilder->length;
    if (length < 1)
        return 0;
    else
        return length - 1;
}

size_t stringbuilder_get_bytelength(StringBuilder stringbuilder) {
    return stringbuilder->length;
}


static void stringbuilder_internal_ensure_capacity(StringBuilder stringbuilder, size_t new_length) {
    if (new_length >= stringbuilder->capacity) {
        size_t new_capacity = new_length + STRINGBUILDER_CHUNK_SIZE - (new_length % STRINGBUILDER_CHUNK_SIZE);
        if (new_capacity == new_length) new_capacity += STRINGBUILDER_CHUNK_SIZE;

        char* new_buffer = realloc_chk(stringbuilder->buffer, new_capacity);

        if (!new_buffer) {
            new_buffer = malloc_chk(new_capacity);
            malloc_assert(new_buffer, char*);

            memcpy(new_buffer, stringbuilder->buffer, stringbuilder->length);
            free_chk(stringbuilder->buffer);
        }

        stringbuilder->buffer = new_buffer;
        stringbuilder->capacity = new_capacity;
    }
    stringbuilder->length = new_length;
}

static size_t stringbuilder_internal_build_format(char* output_buffer, bool add_terminator, const char* format, va_list params) {
    /*
            Formatter identifiers:
                $s       UTF-8 string
                $U       UTF-8 string (print as uppercase)
                $W       UTF-8 string (print as lowercase)
                $f       float with dot
                $d       double with dot
                $F       float with comma
                $D       double with comma
                $c       ASCII single char
                $b       boolean
                $l       int64
                $i       int32
                $L       uint64
                $I       uint32

            Modifiers:
                $[optional number][formatter identifier]
                    strings ($s $U $W):          repeat the string n times
                    float/double ($f $d $F $D):  truncate last n decimal digits
                    char ($c):                   repeat the character n times
                    int/uint ($L $I):            pad with spaces at the start if the string length < n

            Escape identifiers:
                Add another "$" prefix to escape the identifier.
                Example: to print "$fast$car price $50USD" use the string "$$fast$$car price $$50USD"

            Example:
                //
                // this prints: "hello world! 123 @@@ 45.67 //  7/zzz/"
                //
                const format = "$s $W! $i $3c $2f /$0c/$3i/$3s/";
                textsprite_set_text_formated(textsprite, format,
                    "hello", "WORLD", 123, '@', 45.6789, 'x', 7, "zzz"
                );
         */

    size_t total_length = 0;
    const char* arg_string;

    while (true) {
        char c = *format;
        if (c == '\0') {
            break;
        }

        if (c != '$') {
            if (output_buffer) *output_buffer++ = c;
            total_length++;
            format++;
            continue;
        }

        format++;
        c = *format;

        if (c == '\0') {
            if (output_buffer) *output_buffer++ = '$';
            total_length++;
            break;
        }

        if (c == '$') {
            if (output_buffer) *output_buffer++ = '$';
            total_length++;
            format++;
            continue;
        }

        char* end_ptr;
        long modifier = strtol(format, &end_ptr, 10);
        if (format == end_ptr || modifier < 0) modifier = 0;

        c = *end_ptr;
        format = end_ptr + 1;

        if (c == '\0') {
#if DEBUG
            logger_warn("stringbuilder_internal_build_format() truncated string");
#endif
            break;
        }

        switch (c) {
            case 's': //       UTF-8 string
                arg_string = va_arg(params, const char*);
                if (arg_string) {
                    size_t str_len = strlen(arg_string);
                    WRITE_STRING(output_buffer, arg_string, str_len, modifier, total_length);
                }
                break;
            case 'U': //       UTF-8 string (print as uppercase)
                WRITE_CASE_STRING(output_buffer, arg_string, params, uppercase, modifier, total_length);
                break;
            case 'W': //       UTF-8 string (print as lowercase)
                WRITE_CASE_STRING(output_buffer, arg_string, params, lowercase, modifier, total_length);
                break;
            case 'f': //       float with dot
            case 'd': //       double with dot
            case 'F': //       float with comma
            case 'D': //       double with comma
                const float64 arg_double = va_arg(params, const float64);
                float64 integer;
                int double_length;

                if (modifier < 1) {
                    if (float64_modf(arg_double, &integer) == 0.0)
                        double_length = snprintf(output_buffer, UINT8_MAX, FMT_FLT64_DCMLS(.1), arg_double);
                    else
                        double_length = snprintf(output_buffer, UINT8_MAX, FMT_FLT64, arg_double);
                } else {
                    double_length = snprintf(output_buffer, UINT8_MAX, FMT_FLT64_DCMLS(.*), (int)modifier, arg_double);
                }

                assert(double_length >= 0);
                total_length += (size_t)double_length;

                if (output_buffer && double_length > 0) {
                    if (c == 'F' || c == 'D') {
                        char* strdot_ptr = strchr(output_buffer, '.');
                        if (strdot_ptr) *strdot_ptr = ',';
                    }
                    output_buffer += double_length;
                }
                break;
            case 'c': //       UTF-8 single char
                const char arg_char = (const char)va_arg(params, const int);
                WRITE_CHAR(output_buffer, arg_char, modifier, total_length);
                break;
            case 'b': //       boolean
                const int arg_boolean = (const int)va_arg(params, const int);
                {
                    const char* boolean_string = arg_boolean == true ? "true" : "false";
                    size_t str_len = strlen(boolean_string);
                    modifier = 0;
                    WRITE_STRING(output_buffer, boolean_string, str_len, modifier, total_length);
                }
                break;
            case 'l': //       int64
                WRITE_INTEGER(output_buffer, int64, FMTL_INT64, params, modifier, total_length);
                break;
            case 'i': //       int32
                WRITE_INTEGER(output_buffer, int32, FMTL_INT32, params, modifier, total_length);
                break;
            case 'L': //       uint64
                WRITE_INTEGER(output_buffer, uint64, FMTL_UINT64, params, modifier, total_length);
                break;
            case 'I': //       uint32
                WRITE_INTEGER(output_buffer, uint32, FMTL_UINT32, params, modifier, total_length);
                break;
            default:
#if DEBUG
                logger_warn("stringbuilder_internal_build_format() unknown identifier '%c'", c);
#endif
                break;
        }
    }

    if (add_terminator) {
        if (output_buffer) *output_buffer = '\0';
        total_length++;
    }

    return total_length;
}
