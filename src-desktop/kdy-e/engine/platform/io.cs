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

    public static ImageData ReadTexture(string absolute_path) {
        string native_path = IO.GetNativePath(absolute_path, true, false, true);
        byte[] buffer = PreloadCache.RetrieveBuffer(native_path);

        if (buffer != null)
            return TextureLoader.ReadTexture(buffer);
        else
            return TextureLoader.ReadTexture(native_path);
    }

    internal static string ReadText(string absolute_path) {
        try {
            string native_path = IO.GetNativePath(absolute_path, true, false, true);
            byte[] buffer = PreloadCache.RetrieveBuffer(native_path);

            if (buffer != null)
                return Encoding.UTF8.GetString(buffer);
            else
                return File.ReadAllText(native_path, Encoding.UTF8);
        } catch (Exception e) {
            Logger.Error($"read_text() src={absolute_path}\n{e.Message}");
            return null;
        }
    }

    internal static byte[] ReadArrayBuffer(string absolute_path) {
        try {
            string native_path = IO.GetNativePath(absolute_path, true, false, true);
            byte[] buffer = PreloadCache.RetrieveBuffer(native_path);

            if (buffer != null)
                return buffer;
            else
                return File.ReadAllBytes(native_path);
        } catch (Exception e) {
            Logger.Error($"read_arraybuffer() src={absolute_path}\n{e.Message}");
            return null;
        }
    }

    internal static bool ResourceExists(string absolute_path, bool expect_file, bool expect_folder) {
        try {
            string native_path = IO.GetNativePath(absolute_path, expect_file, expect_folder, true);

            if (expect_file)
                return File.Exists(native_path);
            else if (expect_folder)
                return Directory.Exists(native_path);
            else
                return false;
        } catch {
            return false;
        }
    }

    internal static long FileSize(string absolute_path) {
        try {
            return new FileInfo(GetNativePath(absolute_path, true, false, true)).Length;
        } catch {
            return -1L;
        }
    }

    internal static string GetNativePath(string absolute_path, bool is_file, bool is_folder, bool resolve_expansion) {
        string base_path = IO.engine_directory;
        string path = absolute_path;
        int index = 0;

        if (resolve_expansion && absolute_path.StartsWithKDY("/assets", 0)) {
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
