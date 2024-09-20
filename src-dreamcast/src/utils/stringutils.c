#include <sys/types.h>

#include "malloc_utils.h"
#include "stringutils.h"


size_t string_from_codepoint(uint8_t buffer[4], uint32_t code) {
    //
    // for UTF-8 chars the highest is bit is always set (char & 0x80 == 0x80)
    // this allows to differentiate ASCII chars with partial UTF-8 chars
    //

    if (code <= 0x7F) {
        buffer[0] = code;
        return 1;
    }
    if (code <= 0x7FF) {
        buffer[0] = 0xC0 | (code >> 6);
        buffer[1] = 0x80 | (code & 0x3F);
        return 2;
    }
    if (code <= 0xFFFF) {
        buffer[0] = 0xE0 | (code >> 12);
        buffer[1] = 0x80 | ((code >> 6) & 0x3F);
        buffer[2] = 0x80 | (code & 0x3F);
        return 3;
    }
    if (code <= 0x10FFFF) {
        buffer[0] = 0xF0 | (code >> 18);
        buffer[1] = 0x80 | ((code >> 12) & 0x3F);
        buffer[2] = 0x80 | ((code >> 6) & 0x3F);
        buffer[3] = 0x80 | (code & 0x3F);
        return 4;
    }

    // invalid codepoint
    return 0;
}

size_t string_length_from_codepoint(uint32_t code) {
    if (code <= 0x7F) {
        return 1;
    }
    if (code <= 0x7FF) {
        return 2;
    }
    if (code <= 0xFFFF) {
        return 3;
    }
    if (code <= 0x10FFFF) {
        return 4;
    }

    // invalid codepoint
    return 0;
}

bool string_get_character_codepoint(const char* utf8_string, int32_t index, size_t string_length, Grapheme* grapheme) {
    if (index >= string_length || string_length < 1) return false; // invalid index

    uint8_t code = (uint8_t)utf8_string[index];

    if ((code & 0x80) == 0) {
        grapheme->code = code; // ASCII character
        grapheme->size = 1;
        return true;
    }

    uint8_t count;
    uint32_t value;

    if ((code & 0xF8) == 0xF0) {
        count = 3;
        value = code & 0x07;
    } else if ((code & 0xF0) == 0xE0) {
        count = 2;
        value = code & 0x0F;
    } else if ((code & 0xE0) == 0xC0) {
        count = 1;
        value = code & 0x1F;
    } else {
        return false; // invalid encoding
    }

    if (count >= (string_length - (size_t)index)) return 0; // invalid index or character

    // count the first byte
    index++;
    utf8_string += index;

    for (uint8_t i = 0; i < count; i++) {
        uint8_t code = (uint8_t)*utf8_string;
        if ((code & 0xC0) != 0x80) return 0; // invalid encoding

        value <<= 6;
        value |= (code & 0x3F);
        utf8_string++;
    }

    grapheme->size = count + 1;
    grapheme->code = value;

    return 1;
}


int32_t string_occurrences_of_char(const char* string, char value) {
    if (!string) return 0;

    int32_t count = 0;

    while (true) {
        string = strchr(string, value);
        if (!string) break;

        string++;
        count++;
    }

    return count;
}

int32_t string_occurrences_of_string(const char* string, const char* substring) {
    if (!string || !substring) return 0;

    int32_t count = 0;
    size_t substring_length = strlen(substring);

    while (true) {
        char* ptr = strstr(string, substring);
        if (!ptr) break;

        string = ptr + substring_length;
        count++;
    }

    return count;
}


bool string_starts_with(const char* string, const char* substring) {
    size_t string_length = strlen(string);
    size_t substring_length = strlen(substring);

    if (string_length < substring_length) return false;

    return memcmp(string, substring, substring_length) == 0;
}

bool string_equals(const char* str1, const char* str2) {
    if (str1 == str2) return true; // same pointer
    if (!str1 && !str2) return true;
    if (!str1 || !str2) return false;

    return strcmp(str1, str2) == 0;
}

/*bool string_equals_ignore_case(const char* str1, const char* str2) {
    if (str1 == str2) return true; // same pointer
    if (!str1 && !str2) return true;
    if (!str1 || !str2) return false;

    size_t str1_length = strlen(str1);
    size_t str2_length = strlen(str2);

    if (str1_length != str2_length) return false;

    // TODO: implement string case conversion
    char* str1_lw = string_to_lowercase(str1);
    char* str2_lw = string_to_lowercase(str2);

    int ret = strcmp(str1_lw, str2_lw);
    free_chk(str1_lw);
    free_chk(str2_lw);

    return ret == 0;
}*/

char* string_duplicate(const char* string) {
    if (!string) return NULL;

    size_t length = strlen(string) + 1;

    char* string_copy = malloc_chk(length);
    malloc_assert(string_copy, char*);

    memcpy(string_copy, string, length);
    return string_copy;
}

int32_t string_index_of_char(const char* string, int32_t start_index, char value) {
    assert(string);

#ifdef DEBUG
    assert(value != '\0');
    assert(start_index >= 0);
    assert(start_index <= strlen(string));
#endif

    char* ptr = strchr(string + start_index, value);

    if (ptr)
        return (int32_t)(ptr - string);
    else
        return -1;
}

int32_t string_index_of(const char* string, int32_t start_index, const char* substring) {
    assert(string);

#ifdef DEBUG
    assert(substring);
    assert(start_index >= 0);
    assert(start_index <= strlen(string));
#endif

    char* ptr = strstr(string + start_index, substring);

    if (ptr)
        return (int32_t)(ptr - string);
    else
        return -1;
}

int32_t string_index_of_any_char(const char* string, const char* utf8_chars_list) {
    int32_t found_index = -1;

    Grapheme grapheme1;
    size_t length1 = strlen(string);

    Grapheme grapheme2;
    size_t length2 = strlen(utf8_chars_list);

    int32_t index1 = 0;
    while (string_get_character_codepoint(string, index1, length1, &grapheme1)) {

        int32_t index2 = 0;
        while (string_get_character_codepoint(utf8_chars_list, index2, length2, &grapheme2)) {
            if (grapheme1.code == grapheme2.code) {
                if (found_index < 0 || index1 < found_index) {
                    found_index = index1;
                }
                break;
            }
            index2 += grapheme2.size;
        }
        index1 += grapheme1.size;
    }

    return found_index;
}


int32_t string_last_index_of_char(const char* string, char value) {
    assert(string);

#ifdef DEBUG
    assert(value != '\0');
#endif

    size_t length = strlen(string);
    const char* ptr = string + length;

    for (int32_t i = (int32_t)length; i >= 0; i--) {
        if (*ptr == value) {
            return i;
        }
        ptr--;
    }

    return -1;
}

int32_t string_last_index_of_string(const char* string, int32_t start_index, const char* substring) {
    assert(string);

#ifdef DEBUG
    assert(substring);
    assert(start_index >= 0);
    assert(start_index <= strlen(string));
#endif

    size_t substring_length = strlen(substring);
    ssize_t index = -1;
    const char* ptr = string + start_index;

    while (true) {
        ptr = strstr(ptr, substring);
        if (!ptr) break;

        index = (ssize_t)(ptr - string);
        ptr += substring_length;
    }

    return (int32_t)index;
}

char* string_substring(const char* string, int32_t start, int32_t end) {
    assert(string);

    size_t string_length = strlen(string);
    assert(start >= 0 && start <= end && end <= string_length);

    size_t substring_length = (size_t)(end - start);
    char* substring = malloc_chk(substring_length + 1);
    malloc_assert(substring, char*);

    memcpy(substring, string + start, substring_length);
    substring[substring_length] = '\0';

    return substring;
}


char* string_concat_for_state_name(size_t amount, ...) {
    va_list params;
    size_t length = 0;

    va_start(params, amount);
    for (size_t i = 0; i < amount; i++) {
        const char* string = va_arg(params, const char*);
        if (!string) continue;

        // count space
        if (length > 0) length++;

        size_t string_length = strlen(string);
        if (string_length < 1) continue;

        length += string_length;
    }

    char* state_name = malloc_chk(length + 1);
    malloc_assert(state_name, char*);

    char* ptr = state_name;

    va_start(params, amount);
    for (size_t i = 0; i < amount; i++) {
        const char* string = va_arg(params, const char*);
        if (!string) continue;

        if (ptr > state_name) *ptr++ = '\x20';

        size_t string_length = strlen(string);
        if (string_length < 1) continue;

        memcpy(ptr, string, string_length);
        ptr += string_length;
    }

    *ptr = '\0';
    va_end(params);
    return state_name;
}

char* string_concat(size_t count, ...) {
    size_t length = 0;
    va_list params;

    va_start(params, count);
    for (size_t i = 0; i < count; i++) {
        const char* string = va_arg(params, const char*);
        if (string) length += strlen(string);
    }

    char* str = malloc_chk(length + 1);
    char* ptr = str;

    va_start(params, count);
    for (size_t i = 0; i < count; i++) {
        const char* string = va_arg(params, const char*);
        if (string) {
            length = strlen(string);
            memcpy(ptr, string, length);
            ptr += length;
        }
    }

    *ptr = '\0';
    va_end(params);
    return str;
}

char* string_concat_with_substring(const char* substring, int32_t start, int32_t end, int32_t insert_index, size_t count, ...) {
    assert(substring);

    size_t substring_length = strlen(substring);
    assert(start >= 0 && start < end && end <= substring_length);
    substring_length = (size_t)(end - start);

    size_t length = 0;
    va_list params;

    va_start(params, count);
    for (size_t i = 0; i < count; i++) {
        const char* string = va_arg(params, const char*);
        if (string) length += strlen(string);
    }

    char* str = malloc_chk(length + substring_length + 1);
    char* ptr = str;

    va_start(params, count);
    for (size_t i = 0; i < count; i++) {
        const char* string = va_arg(params, const char*);
        if (!string) continue;

        if (i == insert_index) {
            memcpy(ptr, substring + start, substring_length);
            ptr += substring_length;
        }

        length = strlen(string);
        memcpy(ptr, string, length);
        ptr += length;
    }

    if (count == insert_index) {
        memcpy(ptr, substring + start, substring_length);
        ptr += substring_length;
    }

    *ptr = '\0';
    va_end(params);
    return str;
}


size_t string_get_bytelength(const char* string) {
    if (string == NULL)
        return 0;
    else
        return strlen(string) + 1;
}

int32_t string_get_length(const char* string) {
    if (string == NULL)
        return 0;
    else
        return (int32_t)strlen(string);
}


bool string_ends_with(const char* string, const char* substring) {
    size_t string_length = strlen(string);
    size_t substring_length = strlen(substring);

    if (string_length < substring_length) return false;

    size_t offset = string_length - substring_length;
    return memcmp(string + offset, substring, substring_length) == 0;
}

bool string_lowercase_ends_with(const char* string, const char* substring) {
    /*size_t string_length = strlen(string);
    size_t substring_length = strlen(substring);

    if (substring_length > string_length)
        return false;
    else
        return string_equals_ignore_case(string + (string_length - substring_length), substring);*/

    char* string_lw = string_to_lowercase(string);
    char* substring_lw = string_to_lowercase(substring);
    size_t string_lw_length = strlen(string_lw);
    size_t substring_lw_length = strlen(substring_lw);

    bool ret;

    if (string_lw_length < substring_lw_length) {
        ret = false;
    } else {
        size_t offset = string_lw_length - substring_lw_length;
        ret = memcmp(string_lw + offset, substring, substring_lw_length) == 0;
    }

    free_chk(string_lw);
    free_chk(substring_lw);

    return ret;
}

char* string_copy_and_insert(const char* string, int32_t index, const char* substring) {
    if (!string || !substring || substring[0] == '\0') return string_duplicate(string);

    size_t string_length = strlen(string);
    assert(index >= 0 && index <= string_length);

    size_t substring_length = strlen(substring);
    size_t final_length = string_length + substring_length;

    char* str = malloc_chk(final_length + 1);
    malloc_assert(str, char*);

    char* ptr = str;

    memcpy(ptr, string, (size_t)index);
    ptr += (size_t)index;

    memcpy(ptr, substring, substring_length);
    ptr += substring_length;

    memcpy(ptr, string + index, string_length - (size_t)index);

    str[final_length] = '\0';

    return str;
}

char* string_replace(const char* string, const char* old_substring, const char* new_substring) {
    size_t occurences = (size_t)string_occurrences_of_string(string, old_substring);
    if (occurences < 1) {
        return string_duplicate(string);
    }

    size_t string_length = strlen(string);
    size_t old_substring_length = strlen(old_substring);
    size_t new_substring_length = strlen(new_substring);

    char* str = malloc_chk((string_length - (old_substring_length * occurences)) + (new_substring_length * occurences) + 1);
    malloc_assert(str, char*);
    char* str_ptr = str;

    const char* string_end = string + string_length;
    size_t length;

    while (true) {
        char* ptr = strstr(string, old_substring);
        if (!ptr) {
            length = (size_t)(string_end - string);
            memcpy(str_ptr, string, length);
            ptr[length] = '\0';
            break;
        }

        length = (size_t)(ptr - string);
        memcpy(str_ptr, string, length);
        str_ptr += length;

        memcpy(str_ptr, new_substring, new_substring_length);
        str_ptr += new_substring_length;

        string = ptr + old_substring_length;
    }

    return str;
}

char* string_replace_char(const char* string, const char old_char, const char new_new_char) {
    char* str = string_duplicate(string);

    if (str) {
        for (char* str_ptr = str; *str_ptr; str_ptr++) {
            if (*str_ptr == old_char) {
                *str_ptr = new_new_char;
            }
        }
    }

    return str;
}


char* string_trim(const char* string, bool trim_start, bool trim_end) {
    if (string_is_empty(string) || (!trim_start && !trim_end)) return string_duplicate(string);

    int32_t string_length = (int32_t)strlen(string);
    int32_t start_index, end_index;

    if (trim_start) {
        start_index = string_length;
        for (int32_t i = 0; i < string_length; i++) {
            switch (string[i]) {
                case 0x0A:
                case 0x0B:
                case 0x0D:
                case 0x20:
                case 0x09:
                    continue;
            }
            start_index = i;
            break;
        }
    } else {
        start_index = 0;
    }

    if (trim_end) {
        end_index = 0;
        for (int32_t i = string_length - 1; i >= 0; i--) {
            switch (string[i]) {
                case 0x0A:
                case 0x0B:
                case 0x0D:
                case 0x20:
                case 0x09:
                    continue;
            }
            end_index = i + 1;
            break;
        }
    } else {
        end_index = string_length;
    }

    if (start_index < end_index) {
        return string_substring(string, start_index, end_index);
    }

    return string_duplicate("");
}
