using System;
using System.Runtime.InteropServices;

namespace Engine.Externals.FFGraphInterop;

public struct FFGraphFrame {
    public nint y, u, v;
    public int y_stride, u_stride, v_stride;
    public nint rgb24;
    public int width, height;

    public void CopyRGB24ToBuffer(nint dest_buffer, int max_buffer_size) {
        int size = (this.width * this.height) * 3;

#if DEBUG
        if (dest_buffer == 0x00)
            throw new ArgumentNullException(nameof(dest_buffer));

        if (max_buffer_size < 1)
            throw new ArgumentException(nameof(max_buffer_size), "invalid size");

        if (max_buffer_size < size)
            throw new ArgumentOutOfRangeException(nameof(max_buffer_size), $"expected at least {size}");
#endif

        unsafe {
            NativeMemory.Copy((void*)this.rgb24, (void*)dest_buffer, (nuint)size);
        }
    }

    public void CopyYUVToBuffer(nint dest_buffer, int max_buffer_size) {
        int size;
        int half_height = this.height / 2;

#if DEBUG
        if (dest_buffer == 0x00)
            throw new ArgumentNullException(nameof(dest_buffer));

        if (max_buffer_size < 1)
            throw new ArgumentException(nameof(max_buffer_size), "invalid size");

        size = (this.width + this.y_stride) * this.height;
        size += this.u_stride * half_height;
        size += this.v_stride * half_height;

        if (max_buffer_size < size)
            throw new ArgumentOutOfRangeException(nameof(max_buffer_size), $"expected at least {size}");
#endif

        unsafe {
            size = this.y_stride * this.height;
            NativeMemory.Copy((void*)this.y, (void*)dest_buffer, (nuint)size);
            dest_buffer += size;

            size = this.u_stride * half_height;
            NativeMemory.Copy((void*)this.u, (void*)dest_buffer, (nuint)size);
            dest_buffer += size;

            size = this.v_stride * half_height;
            NativeMemory.Copy((void*)this.v, (void*)dest_buffer, (nuint)size);
            //dest_buffer += size;
        }
    }

}
