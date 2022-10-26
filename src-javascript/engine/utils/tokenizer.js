"use strict";

function tokenizer_init(token_chars, ignore_empty_tokens, duplicate_string_before_init, string) {
    if (string == null) return null;

    // In C duplicate the string
    if (duplicate_string_before_init) string = strdup(string);

    return {
        tokens: token_chars,
        string: string,
        length: string.length,
        empty: !ignore_empty_tokens,
        index: 0,
        dup: !!duplicate_string_before_init
    };
}

function tokenizer_destroy(tokenizer) {
    if (tokenizer.dup) tokenizer.string = undefined;
    tokenizer = undefined;
}

function tokenizer_count_occurrences(tokenizer) {
    let count = 0;
    let readed = 0;

    for (let i = 0; i < tokenizer.length; i++) {
        let char = tokenizer.string.charAt(i);
        
        if (char == tokenizer.tokens.includes(char)) {
            if (readed > 0 || tokenizer.empty) {
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

function tokenizer_read_next(tokenizer) {
    if (tokenizer.index >= tokenizer.length) {
        if (
            tokenizer.index == tokenizer.length &&
            tokenizer.empty &&
            tokenizer.length > 0 &&
            tokenizer.tokens.includes(tokenizer.string.charAt(tokenizer.length - 1))
        ) {
            tokenizer.index++;
            return strdup("");
        }
        return null;
    }

    let index = tokenizer.index;
    let length = 0;

    for (; index < tokenizer.length; index++) {
        let char = tokenizer.string.charAt(index);
        if (tokenizer.tokens.includes(char)) {
            if (length > 0 || tokenizer.empty) break;
            continue;
        }
        length++;
    }

    let str;

    if (length > 0) {
        let start_index = index - length;
        str = tokenizer.string.substring(start_index, index);
    } else if (tokenizer.empty) {
        str = strdup("");
    } else {
        str = null;
    }

    tokenizer.index = index + 1;
    return str;
}

function tokenizer_restart(tokenizer) {
    tokenizer.index = 0;
}

