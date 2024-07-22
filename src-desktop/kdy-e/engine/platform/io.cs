using System;
using System.IO;
using System.Text;
using Engine.Utils;

namespace Engine.Platform;

public static class IO {

    private static readonly string engine_directory;

    static IO() {
        engine_directory = AppDomain.CurrentDomain.BaseDirectory;
    }

    public static ImageData ReadTexture(string src) {
        string absolute_path = IO.GetAbsolutePath(src, true, false, true);
        byte[] buffer = PreloadCache.RetrieveBuffer(absolute_path);

        if (buffer != null)
            return TextureLoader.ReadTexture(buffer);
        else
            return TextureLoader.ReadTexture(absolute_path);
    }

    internal static string ReadText(string src) {
        try {
            string absolute_path = IO.GetAbsolutePath(src, true, false, true);
            byte[] buffer = PreloadCache.RetrieveBuffer(absolute_path);

            if (buffer != null)
                return Encoding.UTF8.GetString(buffer);
            else
                return File.ReadAllText(absolute_path, Encoding.UTF8);
        } catch (Exception e) {
            Logger.Error($"read_text() src={src}\n{e.Message}");
            return null;
        }
    }

    internal static byte[] ReadArrayBuffer(string src) {
        try {
            string absolute_path = IO.GetAbsolutePath(src, true, false, true);
            byte[] buffer = PreloadCache.RetrieveBuffer(absolute_path);

            if (buffer != null)
                return buffer;
            else
                return File.ReadAllBytes(absolute_path);
        } catch (Exception e) {
            Logger.Error($"read_arraybuffer() src={src}\n{e.Message}");
            return null;
        }
    }

    internal static bool ResourceExists(string src, bool expect_file, bool expect_folder) {
        try {
            src = IO.GetAbsolutePath(src, expect_file, expect_folder, true);

            if (expect_file)
                return File.Exists(src);
            else if (expect_folder)
                return Directory.Exists(src);
            else
                return false;
        } catch {
            return false;
        }
    }

    internal static long FileSize(string src) {
        try {
            return new FileInfo(GetAbsolutePath(src, true, false, true)).Length;
        } catch {
            return -1L;
        }
    }

    internal static string GetAbsolutePath(string src, bool is_file, bool is_folder, bool resolve_expansion) {
        string base_path = IO.engine_directory;
        string path = src;
        int index = 0;

        if (resolve_expansion && src.StartsWithKDY("/assets", 0)) {
            path = Expansions.ResolvePath(path, is_file, is_folder);
        }

        if (path.StartsWith("/~assets") || path.StartsWith("/~expansions")) {
            index = 2;
        } else {
            switch (path[0]) {
                case '/':
                case '\\':
                    index = 1;
                    break;
            }

            if (!path.StartsWithKDY("assets", index) && !path.StartsWithKDY("expansions", index)) {
                Logger.Warn($"io_get_absolute_path() path outside of 'assets' or 'expansions' folder: {path}");
                base_path = "assets/";
            }
        }

        string new_path = String.Concat((ReadOnlySpan<char>)base_path, path.AsSpan(index));

        return new_path.Replace(FS.CHAR_SEPARATOR, Path.DirectorySeparatorChar);
    }

}
