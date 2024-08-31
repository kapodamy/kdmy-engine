"use strict";

function stringbuilder_init(initial_size) {
    if (initial_size < 1) initial_size = 16;
    return {
        initial_size,
        buffer: ""
    };
}

function stringbuilder_destroy(stringbuilder) {
    stringbuilder.buffer = undefined;
    stringbuilder = undefined;
}

function stringbuilder_finalize(stringbuilder) {
    let str = stringbuilder.buffer;
    stringbuilder = undefined;

    return str;
}

function stringbuilder_clear(stringbuilder) {
    stringbuilder.buffer = "";
}

function stringbuilder_intern(stringbuilder) {
    return stringbuilder.buffer;
}


function stringbuilder_add(stringbuilder, str) {
    if (str != null && str.length > 0) stringbuilder.buffer += str;
}

function stringbuilder_add_char(stringbuilder, str) {
    if (str != null) stringbuilder.buffer += str;
}

function stringbuilder_add_format(stringbuilder, format, ...values) {
    if (format == null || format.length < 1) return;
    stringbuilder.buffer += stringbuilder_helper_create_formatted_string2(
        format,
        values
    );
}

function stringbuilder_get_copy(stringbuilder) {
    return stringbuilder.buffer;
}

function stringbuilder_add_substring(stringbuilder, source_string, start_index, end_index) {
    if (!source_string) return;
    stringbuilder.buffer += source_string.substring(start_index, end_index);
}

function stringbuilder_add_char_codepoint(stringbuilder, codepoint) {
    if (!codepoint) return;
    stringbuilder.buffer += String.fromCodePoint(codepoint);
}

function stringbuilder_add_with_replace(stringbuilder, source_string, old_substring, new_substring) {
    if (old_substring == null) {
        stringbuilder_add(stringbuilder, source_string);
        return;
    }

    let string_length = source_string.length;
    let oldstr_length = old_substring.length;

    // JS only
    let index = 0;
    let last_index = 0;
    while ((index = source_string.indexOf(old_substring, index)) >= 0) {
        stringbuilder.buffer += source_string.substring(last_index, index);
        if (new_substring) stringbuilder.buffer += new_substring;
        index += oldstr_length;
        last_index = index;
    }

    if (last_index < string_length) {
        stringbuilder.buffer += source_string.substring(last_index, string_length);
    }

}

function stringbuilder_lowercase(stringbuilder) {
    stringbuilder.buffer = stringbuilder.buffer.toLowerCase();
}


function stringbuilder_helper_create_formatted_string(format, ...values) {
    return stringbuilder_helper_create_formatted_string2(format, values)
}

function stringbuilder_helper_create_formatted_string2(format, /*va_list*/values) {
    /*
        Formatter identifiers:
            $s       UTF-8 string
            $U       UTF-8 string (print as uppercase)
            $W       UTF-8 string (print as lowercase)
            $f       float with dot
            $d       double with dot
            $F       float with comma
            $D       double with comma
            $c       acsii single char
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

    function check_arg(index, type) {
        if (values[index] != null) {
            let t = typeof (values[index]);
            if (t != type) throw new Error(`Expected type ${type}, found ${t} at ${index}`);
        }
    }

    function number_str(number, round, comma, integer) {
        let str = number.toString();

        if (integer) {
            if (round > 0 && str.length < round)
                return " ".repeat(round - str.length) + str;
            else
                return str;
        }

        let index = str.indexOf('.');

        if (index < 0) return str;
        if (round == 0) return str.substring(0, index);

        if (comma) str = str.substring(0, index) + ',' + str.substring(index + 1);

        if (round < 0) return str;

        index++;
        let dec = str.length - index;
        if (dec > round) return str.substr(0, index + round);

        return str;
    }

    const SPECIAL = '$';

    let format_length = format.length;
    let text = "";
    let args_index = 0;

    for (let i = 0; i < format_length; i++) {
        let char = format.charAt(i);
        if (char != SPECIAL) {
            text += char;
            continue;
        }

        i++;
        if (i >= format_length) break;// malformed
        if (format.charAt(i) == SPECIAL) {
            text += SPECIAL;// escape
            continue;
        }

        let modifier_start = i;
        while (i < format_length) {
            let code = format.charCodeAt(i);
            if (code < 0x30 || code > 0x39) break;// check if a number
            i++;
        }
        let modifier = -1;
        if (i > modifier_start) {
            modifier = vertexprops_parse_unsigned_integer(format.substring(modifier_start, i), NaN);
            if (Number.isNaN(modifier)) continue;
        }

        if (i >= format_length) {
            text += format.substring(modifier_start - 1);// malformed
            break;
        }

        switch (format.charAt(i)) {
            case 's':// utf-8 string
                check_arg(args_index, "string");
                if (modifier < 0) modifier = 1;
                if (values[args_index] != null)
                    text += values[args_index].repeat(modifier);
                break;
            case 'U':// utf-8 string (print as uppercase)
                check_arg(args_index, "string");
                if (modifier < 0) modifier = 1;
                if (values[args_index] != null)
                    text += values[args_index].toUpperCase().repeat(modifier);;
                break;
            case 'W':// utf-8 string (print as lowercase)
                check_arg(args_index, "string");
                if (modifier < 0) modifier = 1;
                if (values[args_index] != null)
                    text += values[args_index].toLowerCase().repeat(modifier);;
                break;
            case 'f':// float with dot
            case 'd':// double with dot
                check_arg(args_index, "number");
                if (values[args_index] != null)
                    text += number_str(values[args_index], modifier, 0);
                break;
            case 'F':// float with comma
            case 'D':// double with comma
                check_arg(args_index, "number");
                if (values[args_index] != null)
                    text += number_str(values[args_index], modifier, 1, 0);
                break;
            case 'c':// ascii char
                check_arg(args_index, "string");
                if (modifier < 0) modifier = 1;
                if (values[args_index] != null)
                    text += values[args_index].repeat(modifier);
                break;
            case 'b':// boolean
                check_arg(args_index, "boolean");
                if (values[args_index] != null)
                    text += values[args_index] ? "true" : "false";
                break;
            case 'l':// int64
            case 'i':// int32
            case 'L':// uint64
            case 'I':// uint32
                check_arg(args_index, "number");
                if (values[args_index] != null)
                    text += number_str(Math.trunc(values[args_index]), modifier, 0, 1);
                break;
            default:
                console.warn(`stringbuilder_helper_create_formatted_string2() unknown identifier ${SPECIAL}` + format.charAt(i));
                break;
        }

        args_index++;
    }

    return text;
}

