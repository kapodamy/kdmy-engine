using System;
using System.IO;
using System.Runtime.CompilerServices;

namespace Engine;

internal static class Logger {

    public static void Info(string str) {
        Print('*', ConsoleColor.White, ConsoleColor.DarkBlue, str);
    }

    public static void Warn(string str) {
        Print('!', ConsoleColor.Black, ConsoleColor.DarkYellow, str);
    }

    public static void Error(string str) {
        Print('X', ConsoleColor.White, ConsoleColor.DarkRed, str);
    }

    public static void Log(string str) {
        Print('·', ConsoleColor.White, ConsoleColor.Black, str);
    }


    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    private static void Print(char icon, ConsoleColor fg, ConsoleColor bg, string str) {
        ConsoleColor fg_orig = Console.ForegroundColor;
        ConsoleColor bg_orig = Console.BackgroundColor;
        TextWriter std = Console.Out;

        Console.ForegroundColor = fg;
        Console.BackgroundColor = bg;
        std.Write(icon);
        std.Write(' ');

        Console.ForegroundColor = fg_orig;
        Console.BackgroundColor = bg_orig;
        std.WriteLine(str);
    }

}
