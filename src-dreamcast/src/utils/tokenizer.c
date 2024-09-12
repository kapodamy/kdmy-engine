#include <stdlib.h>

#include "malloc_utils.h"
#include "stringutils.h"
#include "tokenizer.h"


struct Tokenizer_s {
    const char* tokens;
    char* string;
    size_t length;
    bool empty;
    size_t index;
    bool dup;
};


Tokenizer tokenizer_init(const char* token_chars, bool ignore_empty_tokens, bool duplicate_string_before_init, const char* string) {
    if (string == NULL) return NULL;

    if (duplicate_string_before_init) string = string_duplicate(string);

    Tokenizer tokenizer = malloc_chk(sizeof(struct Tokenizer_s));
    malloc_assert(tokenizer, Tokenizer);

    *tokenizer = (struct Tokenizer_s){
        .tokens = token_chars,
        .string = (char*)string,
        .length = strlen(string),
        .empty = !ignore_empty_tokens,
        .index = 0,
        .dup = !!duplicate_string_before_init
    };

    return tokenizer;
}

Tokenizer tokenizer_init2(const char* token_chars, bool ignore_empty_tokens, const char* string, size_t string_length) {
    if (string == NULL || string_length < 1) return NULL;

    Tokenizer tokenizer = malloc_chk(sizeof(struct Tokenizer_s));
    malloc_assert(tokenizer, Tokenizer);

    *tokenizer = (struct Tokenizer_s){
        .tokens = token_chars,
        .string = (char*)string,
        .length = string_length,
        .empty = !ignore_empty_tokens,
        .index = 0,
        .dup = false
    };

    return tokenizer;
}

void tokenizer_destroy(Tokenizer* tokenizer_ptr) {
    if (!tokenizer_ptr || !*tokenizer_ptr) return;

    Tokenizer tokenizer = *tokenizer_ptr;

    if (tokenizer->dup) free_chk(tokenizer->string);

    free_chk(tokenizer);
    *tokenizer_ptr = NULL;
}

int32_t tokenizer_count_occurrences(Tokenizer tokenizer) {
    int32_t count = 0;
    size_t readed = 0;

    for (size_t i = 0; i < tokenizer->length; i++) {
        char c = tokenizer->string[i];

        if (strchr(tokenizer->tokens, c)) {
            if (readed > 0 || tokenizer->empty) {
                readed = 0;
                count++;
            }
            continue;
        }
        readed++;
    }

    if (readed > 0) count++;

    return count;
}

char* tokenizer_read_next(Tokenizer tokenizer) {
    if (tokenizer->index >= tokenizer->length) {
        if (
            tokenizer->index == tokenizer->length &&
            tokenizer->empty &&
            tokenizer->length > 0 &&
            strchr(tokenizer->tokens, tokenizer->string[tokenizer->length - 1])
        ) {
            tokenizer->index++;
            return string_duplicate("");
        }
        return NULL;
    }

    size_t index = tokenizer->index;
    size_t length = 0;

    for (; index < tokenizer->length; index++) {
        char c = tokenizer->string[index];
        if (strchr(tokenizer->tokens, c)) {
            if (length > 0 || tokenizer->empty) break;
            continue;
        }
        length++;
    }

    char* str;

    if (length > 0) {
        size_t start_index = index - length;
        str = string_substring(tokenizer->string, (int32_t)start_index, (int32_t)index);
    } else if (tokenizer->empty) {
        str = string_duplicate("");
    } else {
        str = NULL;
    }

    tokenizer->index = index + 1;
    return str;
}

void tokenizer_restart(Tokenizer tokenizer) {
    tokenizer->index = 0;
}
