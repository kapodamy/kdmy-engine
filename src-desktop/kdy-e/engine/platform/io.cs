using System;
using System.IO;
using System.Text;
using Engine.Utils;

namespace Engine.Platform {

    public static class IO {

        private static readonly string engine_directory;

        static IO() {
            engine_directory = AppDomain.CurrentDomain.BaseDirectory;
        }

        public static ImageData ReadTexture(string src) {
            string absolute_path = IO.GetAbsolutePath(src, true, true);
            Stream stream = PreloadCache.RetrieveStream(absolute_path);

            if (stream != null)
                return TextureLoader.ReadTexture(stream);
            else
                return TextureLoader.ReadTexture(absolute_path);
        }

        internal static string ReadText(string src) {
            try {
                string absolute_path = IO.GetAbsolutePath(src, true, true);
                byte[] buffer = PreloadCache.RetrieveBuffer(absolute_path);

                if (buffer != null)
                    return Encoding.UTF8.GetString(buffer);
                else
                    return File.ReadAllText(absolute_path, Encoding.UTF8);
            } catch (Exception e) {
                Console.Error.WriteLine("read_text() src=" + src + ":\r\n" + e.Message);
                return null;
            }
        }

        internal static byte[] ReadArrayBuffer(string src) {
            try {
                string absolute_path = IO.GetAbsolutePath(src, true, true);
                byte[] buffer = PreloadCache.RetrieveBuffer(absolute_path);

                if (buffer != null)
                    return buffer;
                else
                    return File.ReadAllBytes(absolute_path);
            } catch (Exception e) {
                Console.Error.WriteLine("read_text() src=" + src + ":\r\n" + e.Message);
                return null;
            }
        }

        internal static bool ResourceExists(string src, bool expect_file, bool expect_folder) {
            try {
                bool is_file0 = File.Exists(IO.GetAbsolutePath(src, true, false));
                bool is_folder0 = Directory.Exists(IO.GetAbsolutePath(src, false, false));
                bool is_file1 = File.Exists(IO.GetAbsolutePath(src, true, true));
                bool is_folder1 = Directory.Exists(IO.GetAbsolutePath(src, false, true));

                bool is_file = is_file0 || is_file1;
                bool is_folder = is_folder0 || is_folder1;
                return (expect_file && is_file) || (expect_folder && is_folder);
            } catch {
                return false;
            }
        }

        internal static long FileSize(string src) {
            try {
                return new FileInfo(GetAbsolutePath(src, true, true)).Length;
            } catch {
                return -1L;
            }
        }

        internal static string GetAbsolutePath(string src, bool is_file, bool resolve_expansion) {
            string path = src;

            if (resolve_expansion && src.StartsWithKDY("/assets", 0)) {
                path = Expansions.ResolvePath(path, is_file);
            }

            if (path.StartsWith("/~assets") || path.StartsWith("/~expansions")) {
                path = path.Substring(2);
            } else {
                switch (path[0]) {
                    case '/':
                    case '\\':
                        path = path.Substring(1);
                        break;
                }

                if (!path.StartsWith("assets") && !path.StartsWith("expansions")) {
                    Console.Error.WriteLine("io_get_absolute_path() path outside of 'assets' or 'expansions' folder: " + path);
                    path = "assets/" + path;
                }
            }

            string new_path = engine_directory + path;
            return new_path.Replace(FS.CHAR_SEPARATOR, Path.DirectorySeparatorChar);
        }

    }
}