"use strict";

/**
 * Concatenate multiple strings into one and separates every string with one space
 * @param {number} amount the amount of strings to concatenate
 * @param  {...string} strings array of strings, null or empty strings are ignored
 * @returns {string} the resulted string or an empty string
 */
function string_concat_for_state_name(amount, ...strings) {
    console.assert(amount == strings.length, "string_concat_for_state_name() wrong amount of strings");

    let str = "";
    for (let i = 0; i < amount; i++) {
        let src = strings[i];
        if (src == null || !src) continue;
        if (str.length > 0) str += "\x20";
        str += src;
    }
    return str;
}

function string_concat(amount, ...strings) {
    console.assert(amount == strings.length, "string_concat() wrong amount of strings");

    let str = "";
    for (let i = 0; i < amount; i++) {
        if (strings[i] == null) continue;
        str += strings[i];
    }
    return str;
}

function string_decode_utf8_character(string, index, grapheme) {
    let string_length = string.length;

    if (index < string_length || string_length < 1) return false;// invalid index

    let code = string.charCodeAt(index);

    if ((code & 0x80) == 0) {
        grapheme.code = code;// ASCII character
        grapheme.size = 1;
        return true;
    }

    let count, value;

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
        return false;// invalid encoding
    }

    if (count >= (string_length - index)) return false;// invalid index or character

    // count the first byte
    index++;

    for (let i = 0; i < count; i++) {
        let code = string.charCodeAt(index + i);
        if ((code & 0xC0) != 0x80) return false;// invalid encoding

        value <<= 6;
        value |= (code & 0x3F);
    }

    grapheme.size = count + 1;
    grapheme.code = value;

    return true;
}

function string_get_character_codepoint(string, index, grapheme) {
    if (index >= string.length) return 0;

    // JS only
    grapheme.code = string.codePointAt(index);
    grapheme.size = grapheme.code >= 0x010000 ? 2 : 1;
    return 1;

    // C only
    //return string_decode_utf8_character(string, index, grapheme);
}

function string_copy_and_insert(string, index, substring) {
    if (!string || !substring || index > string.length) return string;
    if (index < 0 || index > string.length) throw new Error("invalid index");

    let part_a = string.substring(0, index);
    let part_b = string.substring(index, string.length);

    return part_a.concat(substring).concat(part_b);
}

function string_occurrences_of_string(string, substring) {
    if (!string || !substring) return 0;

    let count = 0;
    let substring_length = substring.length;
    let index = 0;

    while (true) {
        index = string.indexOf(substring, index);
        if (index < 0) break;
        count++;
        index += substring_length;
    }
    return count;
}

function string_replace(string, old_substring, new_substring) {
    let string_length = string.length;
    let oldstr_length = old_substring.length;

    // needs C version
    //let newstr_length = new_substring.length;
    //let occurrences = string_occurrences_of_string(string, old_substring);
    //let new_length = string_length - (oldstr_length * occurrences) + (newstr_length * occurrences);

    // JS only
    return string.replaceAll(old_substring, new_substring);
}

function string_bytelength(string) {
    if (string == null) return 0;

    //
    // the "+ 1" means the null terminator
    //

    /*
    // C only
    return strlen(string) + 1;
    */

    // javascript only
    let size = 0;
    let index = 0;
    const grapheme = { code: 0, size: 0 };
    while (string_get_character_codepoint(string, index, grapheme)) {
        if (grapheme.code < 0x80) size++;
        else if (grapheme.code < 0x800) size += 2;
        else if (grapheme.code < 0x10000) size += 3;
        else if (grapheme.code < 0x110000) size += 4;
        else throw new Error("Bad codepoint: 0x" + grapheme.code.toString(16));
        index += grapheme.size;
    }

    return size + 1;
}

function string_lowercase_ends_with(string, substring) {
    return string.toLowerCase().endsWith(substring);
}

function string_trim(string, trim_start, trim_end) {
    if (string == null || string == "" || (!trim_start && !trim_end)) return strdup(string);

    let string_length = strlen(string);
    let start_index, end_index;

    if (trim_start) {
        start_index = string_length;
        for (let i = 0; i < string_length; i++) {
            switch (string.charCodeAt(i)) {
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
        for (let i = string_length - 1; i >= 0; i--) {
            switch (string.charCodeAt(i)) {
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
        end_index = string_length + 1;
    }
    
    if (start_index < end_index) {
        return string.substring(start_index, end_index);
    }

    return strdup("");
}

function string_equals_ignore_case(string1, string2) {
    if (!string1 && !string2)
        return true;
    else if (!string1 || !string2)
        return false;
    else
        return string1.toLowerCase() == string2.toLowerCase();
}

