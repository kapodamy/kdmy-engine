using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;

namespace Engine.Externals {

    public class ImageData : IDisposable {
        public int size;
        public int orig_width;
        public int orig_height;
        public int pow2_width;
        public int pow2_height;

        private IntPtr data;

        private Bitmap bitmap;
        private BitmapData bitmap_data;


        public ImageData(int orig_width, int orig_height, int pow2_width, int pow2_height) {
            this.orig_width = orig_width;
            this.orig_height = orig_height;
            this.pow2_width = pow2_width;
            this.pow2_height = pow2_height;
        }

        public void SetBitmap(Bitmap bitmap) {
            this.Dispose();

            this.bitmap = bitmap;
            this.bitmap_data = bitmap.LockBits(
                 new Rectangle(0, 0, pow2_width, pow2_height),
                 ImageLockMode.ReadOnly,
                 bitmap.PixelFormat//PixelFormat.DontCare
             );
        }

        public void SetPointer(IntPtr data) {
            this.Dispose();

            this.data = data;
        }

        public void Dispose() {
            if (data != IntPtr.Zero) Marshal.FreeHGlobal(data);

            if (bitmap != null) {
                bitmap.UnlockBits(bitmap_data);
                bitmap.Dispose();
            }

            //size = orig_width = orig_height = pow2_width = pow2_height = -1;
            bitmap = null;
            bitmap_data = null;
            data = IntPtr.Zero;
        }

        public IntPtr Addr {
            get
            {
                if (this.bitmap != null) return this.bitmap_data.Scan0;
                return this.data;
            }
        }

        public int Length {
            get => this.pow2_width * this.pow2_height * sizeof(uint);
        }

    }
}
