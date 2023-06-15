using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Runtime.InteropServices;

#pragma warning disable CA1416

namespace Engine.Platform {
    public class ImageWritter {

        public static void WriteImageAndFree(IntPtr rgba_pixels, int width, int height, bool flip_y, string filename) {
            int stride = width * sizeof(uint);
            int length = width * height;
            string dir = Path.GetDirectoryName(filename);

            unsafe {
                if (BitConverter.IsLittleEndian)
                    TextureLoader.Parallel_For(rgba_pixels, length, ToARGB_FromLittleEndian);
                else
                    TextureLoader.Parallel_For(rgba_pixels, length, ToARGBA_FromBigEndian);
            }

            try {
                if (!Directory.Exists(dir)) Directory.CreateDirectory(dir);
            } catch (Exception e) {
                Console.Error.WriteLine("[ERROR] failed to create folder " + dir + "\n" + e.Message);
                Marshal.FreeHGlobal(rgba_pixels);
            }

            try {
                using (Bitmap bitmap = new Bitmap(width, height, stride, PixelFormat.Format32bppArgb, rgba_pixels)) {
                    if (flip_y) bitmap.RotateFlip(RotateFlipType.RotateNoneFlipY);
                    bitmap.Save(filename, ImageFormat.Png);
                }
            } catch (Exception e) {
                Console.Error.WriteLine("[ERROR] failed to write " + filename + "\n" + e.Message);
            } finally {
                Marshal.FreeHGlobal(rgba_pixels);
            }
        }


        internal static unsafe void ToARGB_FromLittleEndian(int length, uint* texture_data) {
            // abgr --> rgba
            for (int i = 0 ; i < length ; i++) {
                uint a = /*texture_data[i] & */0xFF000000;
                uint r = (texture_data[i] & 0x00FF0000) >> 16;
                uint g = texture_data[i] & 0x0000FF00;
                uint b = (texture_data[i] & 0x000000FF) << 16;
                texture_data[i] = r | g | b | a;
            }
        }

        internal static unsafe void ToARGBA_FromBigEndian(int length, uint* texture_data) {
            // argb --> rgba
            for (int i = 0 ; i < length ; i++) {
                uint a = /*(texture_data[i] & */0x000000FF/*) << 24*/;
                uint rgb = (texture_data[i] & 0xFFFFFF00) >> 8;
                texture_data[i] = rgb | a;
            }
        }

    }
}
