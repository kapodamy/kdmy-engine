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
            Logger.Error("PixelUnPackBufferBuilder::CreatePixelDataBuffer() buffer creation failed");
            return null;
        }


        // forget last error
        gl.getError();

        gl.bindBuffer(gl.PIXEL_UNPACK_BUFFER, pbo);
        gl.bufferData(gl.PIXEL_UNPACK_BUFFER, byte_size, (byte[])null, gl.STREAM_DRAW);
        nint mapped_buffer = gl.mapBuffer(gl.PIXEL_UNPACK_BUFFER, gl.WRITE_ONLY);

        PixelUnPackBuffer buffer = new PixelUnPackBuffer(gl, byte_size, pbo, mapped_buffer);

        GLenum error = gl.getError();
        if (error != GLenum.GL_NONE) {
            string e = ((int)error).ToString("X");
            Logger.Error($"PixelUnPackBufferBuilder::CreatePixelDataBuffer() gl error 0x{e}");
            buffer.Dispose();
            return null;
        }


        error = gl.getError();
        if (error != GLenum.GL_NONE) {
            string e = ((int)error).ToString("X");
            Logger.Error($"PixelUnPackBufferBuilder::CreatePixelDataBuffer() gl error 0x{e}");
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
    internal nint mapped_buffer;

    public PixelUnPackBuffer(WebGL2RenderingContext gl, int length, WebGLBuffer pbo, nint mapped_buffer) {
        this.gl = gl;
        this.length = length;
        this.pbo = pbo;
        this.mapped_buffer = mapped_buffer;
    }

    public nint DataPointer => mapped_buffer;

    public int Length => length;

    public void Dispose() {
        if (pbo.IsNull) return;

        gl.bindBuffer(gl.PIXEL_UNPACK_BUFFER, pbo);

        if (mapped_buffer != 0x00) {
            gl.unmapBuffer(gl.PIXEL_UNPACK_BUFFER);
            mapped_buffer = 0x00;
        }

        gl.deleteBuffer(pbo);
        gl.bindBuffer(gl.PIXEL_UNPACK_BUFFER, WebGLBuffer.Null);

        pbo = WebGLBuffer.Null;
    }
}
