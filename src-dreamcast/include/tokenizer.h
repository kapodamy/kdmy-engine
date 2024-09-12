#ifndef _tokenizer_h
#define _tokenizer_h

#include <stdbool.h>
#include <stdint.h>

typedef struct Tokenizer_s* Tokenizer;

Tokenizer tokenizer_init(const char* token_chars, bool ignore_empty_tokens, bool duplicate_string_before_init, const char* string);
Tokenizer tokenizer_init2(const char* token_chars, bool ignore_empty_tokens, const char* string, size_t string_length);
void tokenizer_destroy(Tokenizer* tokenizer);
int32_t tokenizer_count_occurrences(Tokenizer tokenizer);
char* tokenizer_read_next(Tokenizer tokenizer);
void tokenizer_restart(Tokenizer tokenizer);

#endif
