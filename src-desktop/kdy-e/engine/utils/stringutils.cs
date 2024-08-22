using System;
using System.Runtime.CompilerServices;
using System.Text;

namespace Engine.Utils;

public ref struct Grapheme {
    public int code;
    public int size;
}

public static partial class StringUtils {

    public static int OccurrencesOfKDY(this string str, char c) {
        if (str == null) return 0;

        int count = 0;
        foreach (char strc in str) {
            if (strc == c) count++;
        }

        return count;
    }

    public static int OccurrencesOfKDY(this string str, string substr) {
        if (str == null) return 0;

        int count = 0;
        int i = 0;
        int l = substr.Length;
        while ((i = str.IndexOf(substr, i)) >= 0) {
            count++;
            i += l;
        }

        return count;

    }

    public static string SubstringKDY(this string str, int start, int end) {
        return str.Substring(start, end - start);
    }

    public static bool LowercaseEndsWithKDY(this string str, string substring) {
        return str.EndsWith(substring, StringComparison.InvariantCultureIgnoreCase);
    }


    public static string CopyAndInsert(string str, int index, string substring) {
        if (StringUtils.IsEmpty(str) || StringUtils.IsEmpty(substring) || index > str.Length) return str;
        if (index < 0) throw new ArgumentOutOfRangeException("index", "index was negative");

        ReadOnlySpan<char> part_a = str.SubstringKDY(0, index);
        ReadOnlySpan<char> part_b = str.SubstringKDY(index, str.Length);

        return String.Concat(part_a, substring, (ReadOnlySpan<char>)part_b);
    }

    public static string Concat(params string[] strs) {
        StringBuilder stringBuilder = new StringBuilder(128);
        foreach (string str in strs) {
            if (str != null) stringBuilder.Append(str);
        }
        return stringBuilder.ToString();
    }

    public static string ConcatForStateName(params string[] strings) {
        string str = "";
        for (int i = 0 ; i < strings.Length ; i++) {
            string src = strings[i];
            if (StringUtils.IsEmpty(src)) continue;
            if (str.Length > 0) str += "\x20";
            str += src;
        }
        return str;
    }


    public static string Trim(string str, bool trim_start, bool trim_end) {
        if (StringUtils.IsEmpty(str) || (!trim_start && !trim_end)) return str;

        int start_index = 0;
        int end_index = str.Length;
        bool start = trim_start;
        bool end = !trim_start && trim_end;

        for (int index = 0 ; index < str.Length ; index++) {
            bool whitespace = false;
            switch (str[index]) {
                case (char)0x0A:
                case (char)0x0B:
                case (char)0x0D:
                case (char)0x20:
                case (char)0x09:
                    whitespace = true;
                    break;
            }

            if (start) {
                if (whitespace) {
                    start_index = index;
                } else if (trim_end) {
                    start = false;
                    end = true;
                    end_index = index;
                } else {
                    break;
                }
            } else if (end && !whitespace) {
                end_index = index;
            }
        }

        return str.SubstringKDY(start_index, end_index).ToString();
    }


    public static int ByteLength(string str) {
        if (str == null) return 0;

        int size = 0;
        int index = 0;
        Grapheme grapheme = new Grapheme();
        while (StringUtils.GetCharacterCodepoint(str, index, ref grapheme)) {
            if (grapheme.code < 0x80) size++;
            else if (grapheme.code < 0x800) size += 2;
            else if (grapheme.code < 0x10000) size += 3;
            else if (grapheme.code < 0x110000) size += 4;
            else throw new Exception("Bad codepoint: 0x" + grapheme.code.ToString("X"));
            index += grapheme.size;
        }

        return size + 1;
    }


    public static bool GetCharacterCodepoint(string str, int index, ref Grapheme grapheme) {
        if (index >= str.Length) return false;

        grapheme.code = Char.ConvertToUtf32(str, index);
        grapheme.size = Char.IsHighSurrogate(str[index]) ? 2 : 1;
        return true;
    }


    public static int CodePointAtKDY(this string str, int index) {
        return Char.ConvertToUtf32(str, index);
    }

    public static bool StartsWithKDY(this string str, string substr, int start_index) {
        int index = str.IndexOf(substr, start_index);
        return index == start_index;
    }


    public static string CreateFormattedString(string format, params object[] values) {
        return STRINGBUILDER.InternalHelperCreateFormattedString(format, values).ToString();
    }

    public static string RepeatString(string str, int amount) {
        if (amount < 0) return null;

        StringBuilder builder = new StringBuilder(str.Length * amount);
        for (int i = 0 ; i < amount ; i++) builder.Append(str);

        return builder.ToString();
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool IsEmpty(string str) {
        return str == null || str.Length == 0;
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool IsNotEmpty(string str) {
        return str != null && str.Length > 0;
    }

    public static bool EqualsIgnoreCase(string str1, string str2) {
        if (str1 == null && str2 == null)
            return true;
        else if (str1 == null || str2 == null)
            return false;
        else
            return String.Equals(str1, str2, StringComparison.OrdinalIgnoreCase);
    }

}
