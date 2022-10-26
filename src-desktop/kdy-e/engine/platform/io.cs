using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;
using Engine.Externals;
using Engine.Utils;

namespace Engine.Platform {

    public static class IO {

        private static readonly string engine_directory;

        static IO() {
            engine_directory = AppDomain.CurrentDomain.BaseDirectory;
        }

        public static ImageData ReadTexture(string src) {
            src = IO.GetAbsolutePath(src, true, true);
            return TextureLoader.ReadTexture(src);
        }

        internal static string ReadText(string src) {
            try {
                src = IO.GetAbsolutePath(src, true, true);
                return File.ReadAllText(src, Encoding.UTF8);
            } catch (Exception e) {
                Console.Error.WriteLine("read_text() src=" + src + ":\r\n" + e.Message);
                return null;
            }
        }

        internal static byte[] ReadArrayBuffer(string src) {
            try {
                src = IO.GetAbsolutePath(src, true, true);
                return File.ReadAllBytes(src);
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

                if (!path.StartsWith("assets")) {
                    Console.Error.WriteLine("io_get_absolute_path() path outside of 'assets' folder: " + path);
                    path = "assets/" + path;
                }
            }

            string new_path = engine_directory + path;
            return new_path.Replace(FS.CHAR_SEPARATOR, Path.DirectorySeparatorChar);
        }

    }
}