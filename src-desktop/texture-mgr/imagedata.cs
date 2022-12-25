using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;

namespace Engine.Platform {

    public class ImageData : IDisposable {
        public readonly int size;
        public readonly int orig_width;
        public readonly int orig_height;
        public readonly int pow2_width;
        public readonly int pow2_height;

        private IPixelDataBuffer pixelbuffer;
        private IntPtr ptr;
        private Bitmap bitmap;
        private BitmapData bitmapdata;

        public ImageData(IPixelDataBuffer buffer, int orig_width, int orig_height, int pow2_width, int pow2_height) {
            this.orig_width = orig_width;
            this.orig_height = orig_height;
            this.pow2_width = pow2_width;
            this.pow2_height = pow2_height;
            this.size = pow2_width * pow2_height * sizeof(uint);
            this.pixelbuffer = buffer;
            this.ptr = IntPtr.Zero;
            this.bitmap = null;
            this.bitmapdata = null;
        }

        public ImageData(IntPtr ptr, int orig_width, int orig_height, int pow2_width, int pow2_height) {
            this.orig_width = orig_width;
            this.orig_height = orig_height;
            this.pow2_width = pow2_width;
            this.pow2_height = pow2_height;
            this.size = pow2_width * pow2_height * sizeof(uint);
            this.pixelbuffer = null;
            this.ptr = ptr;
            this.bitmap = null;
            this.bitmapdata = null;
        }

        public ImageData(Bitmap bitmap, int pow2_width, int pow2_height) {
            this.orig_width = bitmap.Width;
            this.orig_height = bitmap.Height;
            this.pow2_width = pow2_width;
            this.pow2_height = pow2_height;
            this.size = pow2_width * pow2_height * sizeof(uint);
            this.pixelbuffer = null;

            Rectangle rectangle = new Rectangle(0, 0, orig_width, orig_height);

            this.bitmap = bitmap;
            this.bitmapdata = bitmap.LockBits(rectangle, ImageLockMode.ReadWrite, TextureLoader.FORMAT);
            this.ptr = this.bitmapdata.Scan0;
        }

        public void Dispose() {
            if (pixelbuffer != null) {
                pixelbuffer.Dispose();
                pixelbuffer = null;
            } else if (bitmap != null) {
                bitmap.UnlockBits(bitmapdata);
                bitmap.Dispose();
                bitmap = null;
                bitmapdata = null;
            } else if (ptr != IntPtr.Zero) {
                Marshal.FreeHGlobal(ptr);
                ptr = IntPtr.Zero;
            }
        }

        public IntPtr Addr { get => this.pixelbuffer != null ? this.pixelbuffer.DataPointer : this.ptr; }

        public IPixelDataBuffer pixelDataBuffer { get => this.pixelbuffer; }

    }

}
