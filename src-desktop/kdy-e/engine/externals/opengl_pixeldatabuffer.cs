using System;
using System.Threading;
using Engine.Platform;

namespace Engine.Externals;

public class PixelUnPackBufferBuilder : IPixelDataBufferBuilder {

    private readonly WebGL2RenderingContext gl;

    public PixelUnPackBufferBuilder(WebGL2RenderingContext gl) {
        this.gl = gl;
    }

    public IPixelDataBuffer CreatePixelDataBuffer(int byte_size) {
        WebGLBuffer pbo = gl.createBuffer();
        if (pbo.IsNull) {
            Console.Error.WriteLine("[ERROR] PixelUnPackBufferBuilder::CreatePixelDataBuffer() buffer creation failed");
            return null;
        }


        // forget last error
        gl.getError();

        gl.bindBuffer(gl.PIXEL_UNPACK_BUFFER, pbo);
        gl.bufferData(gl.PIXEL_UNPACK_BUFFER, byte_size, (byte[])null, gl.STREAM_DRAW);
        IntPtr mapped_buffer = gl.mapBuffer(gl.PIXEL_UNPACK_BUFFER, gl.WRITE_ONLY);

        PixelUnPackBuffer buffer = new PixelUnPackBuffer(gl, byte_size, pbo, mapped_buffer);

        GLenum error = gl.getError();
        if (error != GLenum.GL_NONE) {
            Console.Error.WriteLine("[ERROR] PixelUnPackBufferBuilder::CreatePixelDataBuffer() gl error 0x" + ((int)error).ToString("X"));
            buffer.Dispose();
            return null;
        }


        error = gl.getError();
        if (error != GLenum.GL_NONE) {
            Console.Error.WriteLine("[ERROR] PixelUnPackBufferBuilder::CreatePixelDataBuffer() gl error 0x" + ((int)error).ToString("X"));
            buffer.Dispose();
            return null;
        }

        gl.bindBuffer(gl.PIXEL_UNPACK_BUFFER, WebGLBuffer.Null);
        return buffer;
    }

    public bool CanCreatePixelDataBuffer() {
        return Thread.CurrentThread.Name == "MainThread";
    }
}


public class PixelUnPackBuffer : IPixelDataBuffer {
    private readonly WebGL2RenderingContext gl;
    private readonly int length;
    internal WebGLBuffer pbo;
    internal IntPtr mapped_buffer;

    public PixelUnPackBuffer(WebGL2RenderingContext gl, int length, WebGLBuffer pbo, IntPtr mapped_buffer) {
        this.gl = gl;
        this.length = length;
        this.pbo = pbo;
        this.mapped_buffer = mapped_buffer;
    }

    public IntPtr DataPointer => mapped_buffer;

    public int Length => length;

    public void Dispose() {
        if (pbo.IsNull) return;

        gl.bindBuffer(gl.PIXEL_UNPACK_BUFFER, pbo);

        if (mapped_buffer != IntPtr.Zero) {
            gl.unmapBuffer(gl.PIXEL_UNPACK_BUFFER);
            mapped_buffer = IntPtr.Zero;
        }

        gl.deleteBuffer(pbo);
        gl.bindBuffer(gl.PIXEL_UNPACK_BUFFER, WebGLBuffer.Null);

        pbo = WebGLBuffer.Null;
    }
}
