using System;
using System.Globalization;
using System.Text;

namespace Engine.Utils;

public static class STRINGBUILDER {

    public const char SPECIAL = '$';

    public static /*ref*/ string InternKDY(this StringBuilder builder) {
        string internal_string = builder.ToString();
        return /*ref*/ internal_string;
    }
    public static string GetCopyKDY(this StringBuilder builder) {
        return builder.ToString();
    }

    public static StringBuilder AddKDY(this StringBuilder builder, string str) {
        return builder.Append(str);
    }

    public static StringBuilder AddSubstringKDY(this StringBuilder builder, string src, int start, int end) {
        builder.Append(src.SubstringKDY(start, end));
        return builder;
    }

    public static StringBuilder AddCharCodepointKDY(this StringBuilder builder, uint codepoint) {
#if DEBUG
        builder.Append(Char.ConvertFromUtf32(checked((int)codepoint)));
#else
        builder.Append(Char.ConvertFromUtf32(unchecked((int)codepoint)));
#endif
        return builder;
    }

    public static StringBuilder AddWithReplaceKDY(this StringBuilder builder, string str, string old_substr, string new_substr) {
        builder.Append(str.Replace(old_substr, new_substr));
        return builder;
    }

    public static StringBuilder LowerCaseKDY(this StringBuilder builder) {
        string str = StringUtils.ToLowerCase(builder.ToString());
        builder.Clear();
        builder.Append(str);
        return builder;
    }

    public static StringBuilder AddFormatKDY(this StringBuilder builder, string format, params object[] values) {
        if (StringUtils.IsEmpty(format)) return builder;
        builder.Append(InternalHelperCreateFormattedString(format, values));
        return builder;
    }


    internal static StringBuilder InternalHelperCreateFormattedString(string format, params object[] values) {
        #region Format Specs
        /*
        Formatter identifiers:
                $s       UTF-8 string
                $U       UTF-8 string (print as uppercase)
                $W       UTF-8 string (print as lowercase)
                $f       float with dot
                $d       double with dot
                $F       float with comma
                $D       double with comma
                $c       ASCII/UTF-16 single char
                $l       int64
                $i       int32
                $L       uint64
                $I       uint32
                $b       bool

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
                // this prints: "hello world! 123 @@@ 45.67 //  7/zzzzzzzzz/"
                //
                const format = "$s $W! $i $3c $2f /$0c/$3i/$3s/";
                textsprite_set_text_formated(textsprite, format,
                    "hello", "WORLD", 123, '@', 45.6789, 'x', 7, "zzz"
                );
         */
        #endregion

        int format_length = format.Length;
        StringBuilder text = new StringBuilder(format_length);
        int args_index = 0;

        for (int i = 0 ; i < format_length ; i++) {
            char chr = format[i];
            if (chr != SPECIAL) {
                text.Append(chr);
                continue;
            }

            i++;
            if (i >= format_length) break;// malformed
            if (format[i] == SPECIAL) {
                text.Append(SPECIAL);// escape
                continue;
            }

            int modifier_start = i;
            while (i < format_length) {
                char code = format[i];
                if (code < 0x30 || code > 0x39) break;// check if a number
                i++;
            }
            int modifier = Int32.MinValue;
            if (i > modifier_start) {
                modifier = VertexProps.ParseInteger2(format.SubstringKDY(modifier_start, i), Int32.MinValue);
                if (modifier == Int32.MinValue) continue;
            }

            if (i >= format_length) {
                text.Append(format.SubstringKDY(modifier_start - 1, format_length));// malformed
                break;
            }

            if (values[args_index] == null) {
                args_index++;
                continue;
            }

            switch (format[i]) {
                case 's':// utf-8 string
                    InternalHelperCreateFormattedStringCheckArg(values, args_index, typeof(string));
                    if (modifier < 0) modifier = 1;
                    text.Append(StringUtils.RepeatString(
                        (string)values[args_index], modifier
                    ));
                    break;
                case 'U':// utf-8 string (print as uppercase)
                    InternalHelperCreateFormattedStringCheckArg(values, args_index, typeof(string));
                    if (modifier < 0) modifier = 1;
                    text.Append(StringUtils.RepeatString(
                       StringUtils.ToUpperCase((string)values[args_index]), modifier
                    ));
                    break;
                case 'W':// utf-8 string (print as lowercase)
                    InternalHelperCreateFormattedStringCheckArg(values, args_index, typeof(string));
                    if (modifier < 0) modifier = 1;
                    text.Append(StringUtils.RepeatString(
                       StringUtils.ToLowerCase((string)values[args_index]), modifier
                    ));
                    break;
                case 'f':// float with dot
                    InternalHelperCreateFormattedStringCheckArg(values, args_index, typeof(float));
                    text.Append(InternalHelperCreateFormattedStringNumber(
                        (float)values[args_index], modifier, false
                    ));
                    break;
                case 'd':// double with dot
                    InternalHelperCreateFormattedStringCheckArg(values, args_index, typeof(double));
                    text.Append(InternalHelperCreateFormattedStringNumber(
                        (double)values[args_index], modifier, false
                    ));
                    break;
                case 'F':// float with comma
                    InternalHelperCreateFormattedStringCheckArg(values, args_index, typeof(float));
                    text.Append(InternalHelperCreateFormattedStringNumber(
                        (float)values[args_index], modifier, true
                    ));
                    break;
                case 'D':// double with comma
                    InternalHelperCreateFormattedStringCheckArg(values, args_index, typeof(double));
                    text.Append(InternalHelperCreateFormattedStringNumber(
                        (double)values[args_index], modifier, true
                    ));
                    break;
                case 'c':// acsii/utf-16 char
                    InternalHelperCreateFormattedStringCheckArg(values, args_index, typeof(char));
                    if (modifier < 0) modifier = 1;
                    text.Append(
                        new String((char)values[args_index], modifier)
                    );
                    break;
                case 'l':// int64
                    InternalHelperCreateFormattedStringCheckArg(values, args_index, typeof(long));
                    text.Append(InternalHelperCreateFormattedStringPad(
                        ((long)values[args_index]).ToString(), modifier
                    ));
                    break;
                case 'i':// int32
                    InternalHelperCreateFormattedStringCheckArg(values, args_index, typeof(int));
                    text.Append(InternalHelperCreateFormattedStringPad(
                        ((int)values[args_index]).ToString(), modifier
                    ));
                    break;
                case 'L':// uint64
                    InternalHelperCreateFormattedStringCheckArg(values, args_index, typeof(ulong));
                    text.Append(InternalHelperCreateFormattedStringPad(
                        ((ulong)values[args_index]).ToString(), modifier
                    ));
                    break;
                case 'I':// uint32
                    InternalHelperCreateFormattedStringCheckArg(values, args_index, typeof(uint));
                    text.Append(InternalHelperCreateFormattedStringPad(
                        ((uint)values[args_index]).ToString(), modifier
                    ));
                    break;
                case 'b':// boolean
                    if (values[args_index] is bool)
                        text.Append((bool)values[args_index] == true ? "true" : "false");
                    else if (values[args_index] is bool?)
                        text.Append((bool?)values[args_index] == true ? "true" : "false");
                    else
                        InternalHelperCreateFormattedStringCheckArg(values, args_index, typeof(bool));
                    break;
                default:
                    Logger.Warn($"StringBuilder::InternalHelperCreateFormattedString() unknown identifier {SPECIAL}{format[i - 1]}");
                    break;
            }

            args_index++;
        }

        return text;
    }

    private static void InternalHelperCreateFormattedStringCheckArg(object[] values, int index, Type type) {
        Type t = values[index].GetType();
        if (t != type) {
            throw new Exception("Expected type " + type.Name + " found " + t.Name + " at " + index);
        }
    }

    private static ReadOnlySpan<char> InternalHelperCreateFormattedStringNumber(double value, int modifier, bool comma) {
        string str = value.ToString("G17", CultureInfo.InvariantCulture);
        int index = str.IndexOf('.');

        if (comma && index >= 0) str = str.Replace('.', ',');
        if (index < 0 || modifier < 0) return str;

        if (modifier < 1) return str.SubstringKDY(0, index);

        index += modifier;
        if (index >= str.Length) return str;

        return str.AsSpan(0, index + 1);
    }

    private static string InternalHelperCreateFormattedStringPad(string number, int modifier) {
        int length = number.Length;
        if (modifier < 1 || length > modifier) return number;

        int filler = modifier - length;
        return new String('\x20', filler) + number;
    }




}
