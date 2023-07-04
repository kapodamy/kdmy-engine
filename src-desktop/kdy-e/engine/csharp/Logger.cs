using System;
using System.IO;
using System.Runtime.CompilerServices;

namespace Engine;

internal static class Logger {

    public static void Info(string str) {
        Print('i', ConsoleColor.White, ConsoleColor.DarkCyan, str);
    }

    public static void Warn(string str) {
        Print('!', ConsoleColor.Black, ConsoleColor.DarkYellow, str);
    }

    public static void Error(string str) {
        Print('x', ConsoleColor.White, ConsoleColor.DarkRed, str);
    }

    public static void Log(string str) {
        Print('·', ConsoleColor.Black, ConsoleColor.White, str);
    }


    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    private static void Print(char icon, ConsoleColor fg, ConsoleColor bg, string str) {
        ConsoleColor fg_orig = Console.ForegroundColor;
        ConsoleColor bg_orig = Console.BackgroundColor;

        Console.ForegroundColor = fg;
        Console.BackgroundColor = bg;
        Console.Out.Write(icon);

        Console.ForegroundColor = fg_orig;
        Console.BackgroundColor = bg_orig;
        Console.Out.Write(' ');
        Console.Out.WriteLine(str);
    }

}
