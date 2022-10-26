#define PARALLEL
using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;
using Engine.Externals;
using System.Threading;

namespace Engine.Platform {

    public static class TextureLoader {

        private const int CHUNK_SIZE = UInt16.MaxValue + 1;


        public static ImageData ReadTexture(string src) {
            const PixelFormat FORMAT = PixelFormat.Format32bppArgb;

            Bitmap bitmap;
            try {
                bitmap = new Bitmap(src, false);
            } catch (Exception e) {
                Console.Error.WriteLine(
                    $"[ERROR] read_texture() failed to read/parse: {src}\n" + e.Message
                );
                return null;
            }

            int tex_width = PowerOfTwoCalc(bitmap.Width);
            int tex_height = PowerOfTwoCalc(bitmap.Height);
            int width = bitmap.Width;
            int height = bitmap.Height;

            // ¿what is going on here?
            //      - read the image file
            //      - if required, allocate memory for the texture with power-of-two size
            //      - and create a fake bitmap and draw the image pixels
            //      - change format from ARGB --> RGBA
            //      - return texture pointer and his dimmensions

            ImageData image_data = new ImageData(width, height, tex_width, tex_height);
            int total_pixels = tex_width * tex_height;

            if (tex_width == width && tex_height == height && bitmap.PixelFormat == FORMAT) {
                image_data.SetBitmap(bitmap);
            } else {
                IntPtr ptr;
                try {
                    int length = image_data.Length;
                    ptr = Marshal.AllocHGlobal(length);
#if PARALLEL
                    unsafe { Parallel_For(ptr, total_pixels, Zeros4); }
#else
                    Zeros(length, ptr);
#endif
                } catch (OutOfMemoryException) {
                    Console.Error.WriteLine($"[ERROR] read_texture() not enough RAM for {src}\n");
                    bitmap.Dispose();
                    return null;
                }

                int stride = tex_width * sizeof(uint);
                image_data.SetPointer(ptr);

                try {
                    using (Bitmap dest = new Bitmap(tex_width, tex_height, stride, FORMAT, ptr)) {
                        using (Graphics graphics_dest = Graphics.FromImage(dest)) {
                            graphics_dest.DrawImage(bitmap, 0, 0, bitmap.Width, bitmap.Height);
                        }
                    }
                } catch (Exception e) {
                    Console.Error.WriteLine(
                        $"[ERROR] read_texture() failed to reallocate pixels {src}\n" + e.Message
                    );
                    image_data.Dispose();
                    return null;
                } finally {
                    bitmap.Dispose();
                }
            }

            // OpenGL expects RGBA but must be in platform "endian" stored
            unsafe {
#if PARALLEL
                if (BitConverter.IsLittleEndian)
                    Parallel_For(image_data.Addr, total_pixels, ToRGBA_FromLittleEndian);
                else
                    Parallel_For(image_data.Addr, total_pixels, ToRGBA_FromBigEndian);
#else
                if (BitConverter.IsLittleEndian)
                    ToRGBA_FromLittleEndian(length, (uint*)image_data.Addr);
                else
                    ToRGBA_FromBigEndian(length, (uint*)image_data.Addr);
#endif
            }

            return image_data;
        }

        private static int PowerOfTwoCalc(int value) {
            int current = 1;

            while (current < value) {
                current <<= 1;
                if (current == 0) throw new ArgumentOutOfRangeException("value");
            }

            return current;
        }

        private static unsafe void Zeros(int length, IntPtr ptr) {
            if (length % sizeof(ulong) == 0) {
                length /= sizeof(ulong);
                ulong* ptr_ulong = (ulong*)ptr;
                for (int i = 0 ; i < length ; i++) ptr_ulong[i] = 0UL;
            } else {
                length /= sizeof(uint);
                uint* ptr_uint = (uint*)ptr;
                for (int i = 0 ; i < length ; i++) ptr_uint[i] = 0U;
            }
        }

        internal static unsafe void Zeros4(int length, uint* ptr) {
            for (int i = 0 ; i < length ; i++) ptr[i] = 0U;
        }


        internal static unsafe void ToRGBA_FromLittleEndian(int length, uint* texture_data) {
            // argb --> abgr
            for (int i = 0 ; i < length ; i++) {
                uint a = texture_data[i] & 0xFF000000;
                uint r = (texture_data[i] & 0x00FF0000) >> 16;
                uint g = texture_data[i] & 0x0000FF00;
                uint b = (texture_data[i] & 0x000000FF) << 16;
                texture_data[i] = r | g | b | a;
            }
        }

        internal static unsafe void ToRGBA_FromBigEndian(int length, uint* texture_data) {
            // argb --> rgba
            for (int i = 0 ; i < length ; i++) {
                uint a = (texture_data[i] & 0xFF000000) >> 24;
                uint rgb = (texture_data[i] & 0x00FFFFFF) << 8;
                texture_data[i] = rgb | a;
            }

        }


        internal static unsafe void Parallel_For(IntPtr ptr, int length, Worker worker) {
            uint* data = (uint*)ptr;
            const int chunk_size = CHUNK_SIZE / sizeof(uint);
            int chunks = length / chunk_size;
            int remaing = chunks;
            int last_chunk = chunks - 1;
            int last_chunk_size = length - (last_chunk * chunk_size);
            //if (last_chunk_size < 1) last_chunk_size = chunk_size;

            if (chunks < 2) {
                worker(length, data);
                return;
            }
            ManualResetEvent mre_start = new ManualResetEvent(false);
            ManualResetEvent mre_end = new ManualResetEvent(false);

            WaitCallback wrapper = delegate (object state) {
                mre_start.WaitOne();
                int current_chunk = (int)state;
                int size = current_chunk == last_chunk ? last_chunk_size : chunk_size;

                uint* offset = data + (current_chunk * chunk_size);
                worker(size, offset);

                if (Interlocked.Decrement(ref remaing) == 0) mre_end.Set();
            };

            //ThreadPool.SetMaxThreads(Environment.ProcessorCount, Environment.ProcessorCount);

            for (int i = 0 ; i < chunks ; i++) {
                ThreadPool.QueueUserWorkItem(wrapper, i);
            }

            mre_start.Set();
            mre_end.WaitOne();

            mre_start.Close();
            mre_end.Close();
        }

        internal unsafe delegate void Worker(int length, uint* data);

    }
}