using System;
using System.Globalization;
using System.Text;

namespace Engine.Utils;

public class Grapheme {
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
        return str.ToLower(CultureInfo.InvariantCulture).EndsWith(substring);
    }


    public static string CopyAndInsert(string str, int index, string substring) {
        if (String.IsNullOrEmpty(str) || String.IsNullOrEmpty(substring) || index > str.Length) return str;
        if (index < 0) throw new ArgumentOutOfRangeException("index", "index was negative");

        string part_a = str.SubstringKDY(0, index);
        string part_b = str.SubstringKDY(index, str.Length);

        return part_a + substring + part_b;
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
            if (String.IsNullOrEmpty(src)) continue;
            if (str.Length > 0) str += "\x20";
            str += src;
        }
        return str;
    }


    public static string Trim(string str, bool trim_start, bool trim_end) {
        if (String.IsNullOrEmpty(str) || (!trim_start && !trim_end)) return str;

        Grapheme grapheme = new Grapheme();

        int start_index = 0;
        int end_index = str.Length;
        int index = 0;
        bool start = trim_start;
        bool end = !trim_start && trim_end;

        while (StringUtils.GetCharacterCodepoint(str, index, grapheme)) {
            bool whitespace = false;
            switch (grapheme.code) {
                case 0x0A:
                case 0x0B:
                case 0x0D:
                case 0x20:
                case 0x09:
                    whitespace = true;
                    break;
            }

            index += grapheme.size;

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

        return str.SubstringKDY(start_index, end_index);
    }


    public static int ByteLength(string str) {
        if (str == null) return 0;

        int size = 0;
        int index = 0;
        Grapheme grapheme = new Grapheme();
        while (StringUtils.GetCharacterCodepoint(str, index, grapheme)) {
            if (grapheme.code < 0x80) size++;
            else if (grapheme.code < 0x800) size += 2;
            else if (grapheme.code < 0x10000) size += 3;
            else if (grapheme.code < 0x110000) size += 4;
            else throw new Exception("Bad codepoint: 0x" + grapheme.code.ToString("X"));
            index += grapheme.size;
        }

        return size + 1;
    }


    public static bool GetCharacterCodepoint(string str, int index, Grapheme grapheme) {
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

}
