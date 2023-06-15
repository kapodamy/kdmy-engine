#define PARALLEL
using System;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.IO;
using System.Runtime.InteropServices;
using System.Threading;

#pragma warning disable CA1416

namespace Engine.Platform {

    public static class TextureLoader {

        private const int CHUNK_SIZE = 4096 * 1024; // 4MiB
        public const PixelFormat FORMAT = PixelFormat.Format32bppArgb;
        private static IPixelDataBufferBuilder buffer_builder;

        public static IPixelDataBufferBuilder SetPixelBufferBuilder(IPixelDataBufferBuilder builder) {
            IPixelDataBufferBuilder old = buffer_builder;
            buffer_builder = builder;
            return old;
        }


        public static ImageData ReadTexture(string src) {
            if (DDS.IsDDS(src)) {
                return DDS.Parse(src);
            }

            Bitmap bitmap;
            try {
                bitmap = new Bitmap(src);
            } catch (Exception e) {
                Console.Error.WriteLine(
                    $"[ERROR] read_texture_from_path() failed to read/parse: {src}\n" + e.Message
                );
                return null;
            }

            return ReadTexture(bitmap);
        }

        public static ImageData ReadTexture(byte[] buffer) {
            if (DDS.IsDDS(buffer)) {
                return DDS.Parse(buffer);
            }

            Bitmap bitmap;
            try {
                using (MemoryStream stream = new MemoryStream(buffer, false)) {
                    bitmap = new Bitmap(stream);
                }
            } catch (Exception e) {
                Console.Error.WriteLine(
                    $"[ERROR] read_texture_from_stream() failed to read/parse:\n" + e.Message
                );
                return null;
            }

            return ReadTexture(bitmap);
        }

        private static ImageData ReadTexture(Bitmap bitmap) {
            int tex_width = PowerOfTwoCalc(bitmap.Width);
            int tex_height = PowerOfTwoCalc(bitmap.Height);
            int width = bitmap.Width;
            int height = bitmap.Height;

            // ¿what is going on here?
            //      - read the image file
            //      - allocate memory for the texture with power-of-two size
            //      - and create a fake bitmap and draw the image pixels
            //      - change format from ARGB --> RGBA
            //      - return texture pointer and his dimmensions

            int total_pixels = width * height;
            int total_pixels_pow2 = tex_width * tex_height;
            int total_bytes_pow2 = total_pixels_pow2 * sizeof(uint);
            ImageData image_data = null;
            IPixelDataBuffer buffer = null;

            if (buffer_builder != null && buffer_builder.CanCreatePixelDataBuffer()) {
                buffer = buffer_builder.CreatePixelDataBuffer(total_bytes_pow2);
                if (buffer == null) {
                    Console.Error.WriteLine("[ERROR] TextureLoader::ReadTexture() pixel data buffer creation failed");
                    bitmap.Dispose();
                    return null;
                }
                image_data = new ImageData(buffer, width, height, tex_width, tex_height);
            }


            if (tex_width == width && tex_height == height && bitmap.PixelFormat == FORMAT) {
                if (image_data != null) {
                    // copy bitmap pixels to pixel buffer object
                    Rectangle rectangle = new Rectangle(0, 0, width, height);
                    BitmapData data = bitmap.LockBits(rectangle, ImageLockMode.ReadOnly, FORMAT);
                    unsafe { Memory.Copy((byte*)image_data.Addr, (byte*)data.Scan0, image_data.size); }
                    bitmap.UnlockBits(data);
                } else {
                    image_data = new ImageData(bitmap, tex_width, tex_height);
                }
            } else {
                if (image_data == null) {
                    IntPtr buffer_ptr = Marshal.AllocHGlobal(total_bytes_pow2);
                    image_data = new ImageData(buffer_ptr, width, height, tex_width, tex_height);
                }

                try {
                    int stride = tex_width * sizeof(uint);
                    using (Bitmap dest = new Bitmap(tex_width, tex_height, stride, FORMAT, image_data.Addr)) {
                        using (Graphics graphics_dest = Graphics.FromImage(dest)) {
                            graphics_dest.Clear(Color.Transparent);
                            graphics_dest.CompositingMode = CompositingMode.SourceCopy;
                            graphics_dest.CompositingQuality = CompositingQuality.HighQuality;
                            graphics_dest.DrawImage(bitmap, 0, 0, bitmap.Width, bitmap.Height);
                        }
                    }
                } catch (Exception e) {
                    Console.Error.WriteLine(
                        $"[ERROR] read_texture_from_bitmap() failed to reallocate pixels\n" + e.Message
                    );
                    image_data.Dispose();
                    return null;
                } finally {
                    bitmap.Dispose();
                }
            }

            //
            // OpenGL expects RGBA but must be in platform "endian" stored
            // shift all pixels but exclude the filler part
            //
            unsafe {
#if PARALLEL
                if (BitConverter.IsLittleEndian)
                    Parallel_For(image_data.Addr, total_pixels_pow2, ToRGBA_FromLittleEndian);
                else
                    Parallel_For(image_data.Addr, total_pixels_pow2, ToRGBA_FromBigEndian);
#else
                if (BitConverter.IsLittleEndian)
                    ToRGBA_FromLittleEndian(total_pixels_pow2, (uint*)image_data.Addr);
                else
                    ToRGBA_FromBigEndian(total_pixels_pow2, (uint*)image_data.Addr);
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