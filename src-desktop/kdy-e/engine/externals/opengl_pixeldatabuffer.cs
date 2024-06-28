using System.Threading;
using Engine.Platform;

namespace Engine.Externals;

public class WGLUnPackBufferBuilder : IPixelDataBufferBuilder {

    private readonly WebGL2RenderingContext gl;
    private readonly Thread renderer;

    public WGLUnPackBufferBuilder(WebGL2RenderingContext gl) {
        this.gl = gl;
        this.renderer = Thread.CurrentThread;
    }

    public IPixelDataBuffer CreatePixelDataBuffer(int byte_size) {
        WebGLBuffer pbo = gl.createBuffer();
        if (pbo.IsNull) {
            Logger.Error("WGLUnPackBufferBuilder::CreatePixelDataBuffer() buffer creation failed");
            return null;
        }

        // forget last error
        gl.getError();

        gl.bindBuffer(gl.PIXEL_UNPACK_BUFFER, pbo);
        gl.bufferData(gl.PIXEL_UNPACK_BUFFER, byte_size, (byte[])null, gl.STREAM_DRAW);
        nint mapped_buffer = gl.mapBuffer(gl.PIXEL_UNPACK_BUFFER, gl.WRITE_ONLY);

        WGLUnPackBuffer buffer = new WGLUnPackBuffer(gl, byte_size, pbo, mapped_buffer);

        GLenum error = gl.getError();
        if (error != GLenum.GL_NONE) {
            string e = ((int)error).ToString("X");
            Logger.Error($"WGLUnPackBufferBuilder::CreatePixelDataBuffer() gl error 0x{e}");
            buffer.Dispose();
            return null;
        }

        gl.bindBuffer(gl.PIXEL_UNPACK_BUFFER, WebGLBuffer.Null);
        return buffer;
    }

    public bool CanCreatePixelDataBuffer() {
        return Thread.CurrentThread == renderer;
    }
}


public class WGLUnPackBuffer : IPixelDataBuffer {
    private readonly WebGL2RenderingContext gl;
    private readonly int length;
    internal WebGLBuffer pbo;
    internal nint mapped_buffer;

    public WGLUnPackBuffer(WebGL2RenderingContext gl, int length, WebGLBuffer pbo, nint mapped_buffer) {
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
