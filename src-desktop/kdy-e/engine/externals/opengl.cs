using System;
using System.Runtime.InteropServices;
using System.Text;
using Engine.Platform;

namespace Engine.Externals {

    public struct WebGLBuffer {
        internal uint value;
        public static WebGLBuffer Null = new WebGLBuffer() { value = 0x0000 };
        public bool IsNull { get => value == 0x0000; }
        public override string ToString() {
            return "WebGLBuffer: " + (value == 0 ? "(null)" : value.ToString());
        }
    }
    public struct WebGLUniformLocation {
        internal int value;
        public static WebGLUniformLocation Null = new WebGLUniformLocation() { value = -1 };
        public bool IsNull { get => value == 0x0000; }
        public override string ToString() {
            return "WebGLUniformLocation: " + (value == 0 ? "(null)" : value.ToString());
        }
    }
    public struct WebGLProgram {
        internal uint value;
        public static WebGLProgram Null = new WebGLProgram() { value = 0x0000 };
        public bool IsNull { get => value == 0x0000; }
        public override string ToString() {
            return "WebGLProgram: " + (value == 0 ? "(null)" : value.ToString());
        }
    }
    public struct WebGLTexture {
        internal uint value;
        public static WebGLTexture Null = new WebGLTexture() { value = 0x0000 };
        public bool IsNull { get => value == 0x0000; }
        public override string ToString() {
            return "WebGLTexture: " + (value == 0 ? "(null)" : value.ToString());
        }
    }
    public struct WebGLShader {
        internal uint value;
        public static WebGLShader Null = new WebGLShader() { value = 0x0000 };
        public bool IsNull { get => value == 0x0000; }
        public override string ToString() {
            return "WebGLShader: " + (value == 0 ? "(null)" : value.ToString());
        }
    }
    public struct WebGLVertexArrayObject {
        internal uint value;
        public static WebGLVertexArrayObject Null = new WebGLVertexArrayObject() { value = 0x0000 };
        public override string ToString() {
            return "WebGLVertexArrayObject: " + (value == 0 ? "(null)" : value.ToString());
        }
    }
    public struct WebGLFramebuffer {
        internal uint value;
        public static WebGLFramebuffer Null = new WebGLFramebuffer() { value = 0x0000 };
        public override string ToString() {
            return "WebGLFramebuffer: " + (value == 0 ? "(null)" : value.ToString());
        }
    }
    public struct WebGLActiveInfo {
        public string name;
        public int size;
        public GLenum type;
    }

    public partial class WebGL2RenderingContext {

        public static int KDY_draw_calls_count = 0;

#pragma warning disable IDE1006

        public void flush() {
            NativeMethods.glFlush();
        }

        public string getString(GLenum name) {
            IntPtr ptr = NativeMethods.glGetString(name);
            if (ptr == IntPtr.Zero) return null;
            return Marshal.PtrToStringAnsi(ptr);
        }

        public GLenum getError() {
            return NativeMethods.glGetError();
        }

        public void activeTexture(GLenum texture) {
            NativeMethods.glActiveTexture(texture);
        }

        public void bindTexture(GLenum type, WebGLTexture texture) {
            NativeMethods.glBindTexture(type, texture.value);
        }

        internal void useProgram(WebGLProgram program) {
            NativeMethods.glUseProgram(program.value);
        }

        internal void bindBuffer(GLenum target, WebGLBuffer buffer) {
            NativeMethods.glBindBuffer(target, buffer.value);
        }

        internal void enableVertexAttribArray(uint index) {
            NativeMethods.glEnableVertexAttribArray(index);
        }

        internal int getProgramParameter(WebGLProgram program, GLenum pname) {
            int result = 0;
            NativeMethods.glGetProgramiv(program.value, pname, out result);
            return result;
        }

        internal string getProgramInfoLog(WebGLProgram program) {
            int maxLength;
            NativeMethods.glGetProgramiv(program.value, GLenum.GL_INFO_LOG_LENGTH, out maxLength);
            if (maxLength < 1) return String.Empty;

            int length;
            IntPtr temp = Marshal.AllocHGlobal(maxLength * 2);
            NativeMethods.glGetProgramInfoLog(program.value, maxLength, out length, temp);

            string str = Marshal.PtrToStringAnsi(temp, length);
            Marshal.FreeHGlobal(temp);

            return str;
        }

        internal void deleteProgram(WebGLProgram program) {
            NativeMethods.glDeleteProgram(program.value);
        }

        internal WebGLProgram createProgram() {
            uint program = NativeMethods.glCreateProgram();
            return new WebGLProgram() { value = program };
        }

        internal void attachShader(WebGLProgram program, WebGLShader shader) {
            NativeMethods.glAttachShader(program.value, shader.value);
        }

        internal void linkProgram(WebGLProgram program) {
            NativeMethods.glLinkProgram(program.value);
        }


        internal GLenum getShaderParameter(WebGLShader shader, GLenum pname) {
            int result = 0;
            NativeMethods.glGetShaderiv(shader.value, pname, out result);
            return (GLenum)result;
        }

        internal void deleteShader(WebGLShader shader) {
            NativeMethods.glDeleteShader(shader.value);
        }

        internal string getShaderInfoLog(WebGLShader shader) {
            int maxLength;
            NativeMethods.glGetShaderiv(shader.value, GLenum.GL_INFO_LOG_LENGTH, out maxLength);
            if (maxLength < 1) return String.Empty;

            int length;
            IntPtr temp = Marshal.AllocHGlobal(maxLength * 2);
            NativeMethods.glGetShaderInfoLog(shader.value, maxLength, out length, temp);

            string str = Marshal.PtrToStringAnsi(temp, length);
            Marshal.FreeHGlobal(temp);

            return str;
        }

        internal void compileShader(WebGLShader shader) {
            NativeMethods.glCompileShader(shader.value);
        }

        internal void shaderSource(WebGLShader shader, string source) {
            int length = Encoding.ASCII.GetByteCount(source);
            NativeMethods.glShaderSource(shader.value, 1, ref source, ref length);
        }

        internal WebGLShader createShader(GLenum shaderType) {
            uint result = NativeMethods.glCreateShader(shaderType);
            return new WebGLShader() { value = result };
        }

        internal void drawArrays(GLenum mode, int first, int count) {
            NativeMethods.glDrawArrays(mode, first, count);
            KDY_draw_calls_count++;
        }

        internal void uniform4fv(WebGLUniformLocation location, float[] value) {
            NativeMethods.glUniform4fv(location.value, 1, value);
        }

        internal void uniformMatrix2fv(WebGLUniformLocation location, bool transpose, float[] matrix) {
            byte flag = transpose ? (byte)1 : (byte)0;
            NativeMethods.glUniformMatrix2fv(location.value, 1, flag, matrix);
        }

        internal void uniformMatrix3fv(WebGLUniformLocation location, bool transpose, float[] matrix) {
            byte flag = transpose ? (byte)1 : (byte)0;
            NativeMethods.glUniformMatrix3fv(location.value, 1, flag, matrix);
        }

        internal void uniformMatrix4fv(WebGLUniformLocation location, bool transpose, float[] matrix) {
            byte flag = transpose ? (byte)1 : (byte)0;
            NativeMethods.glUniformMatrix4fv(location.value, 1, flag, matrix);
        }

        internal void vertexAttribPointer(uint index, int size, GLenum type, bool normalized, int stride, int offset) {
            byte flag = normalized ? (byte)1 : (byte)0;
            NativeMethods.glVertexAttribPointer(index, size, type, flag, stride, (IntPtr)offset);
        }

        internal uint getAttribLocation(WebGLProgram program, string name) {
            int position = NativeMethods.glGetAttribLocation(program.value, name);

            if (position < 0) Console.Error.WriteLine("getAttribLocation() warning, missing '" + name + "' attribute");
            return (uint)position;
        }

        internal WebGLUniformLocation getUniformLocation(WebGLProgram program, string name) {
            int result = NativeMethods.glGetUniformLocation(program.value, name);
            return new WebGLUniformLocation() { value = result };
        }

        internal WebGLBuffer createBuffer() {
            uint[] result = new uint[1];
            NativeMethods.glGenBuffers(1, result);
            return new WebGLBuffer() { value = result[0] };
        }

        internal void uniform1i(WebGLUniformLocation location, int value) {
            NativeMethods.glUniform1i(location.value, value);
        }

        internal void uniform2i(WebGLUniformLocation location, int v0, int v1) {
            NativeMethods.glUniform2i(location.value, v0, v1);
        }

        internal void uniform3i(WebGLUniformLocation location, int v0, int v1, int v2) {
            NativeMethods.glUniform3i(location.value, v0, v1, v2);
        }

        internal void uniform4i(WebGLUniformLocation location, int v0, int v1, int v2, int v3) {
            NativeMethods.glUniform4i(location.value, v0, v1, v2, v3);
        }

        internal void uniform1f(WebGLUniformLocation location, float value) {
            NativeMethods.glUniform1f(location.value, value);
        }

        internal void uniform2f(WebGLUniformLocation location, float v0, float v1) {
            NativeMethods.glUniform2f(location.value, v0, v1);
        }

        internal void uniform3f(WebGLUniformLocation location, float v0, float v1, float v2) {
            NativeMethods.glUniform3f(location.value, v0, v1, v2);
        }

        internal void uniform4f(WebGLUniformLocation location, float v0, float v1, float v2, float v3) {
            NativeMethods.glUniform4f(location.value, v0, v1, v2, v3);
        }

        internal void uniform1ui(WebGLUniformLocation location, uint v0) {
            NativeMethods.glUniform1ui(location.value, v0);
        }

        internal void uniform2ui(WebGLUniformLocation location, uint v0, uint v1) {
            NativeMethods.glUniform2ui(location.value, v0, v1);
        }

        internal void uniform3ui(WebGLUniformLocation location, uint v0, uint v1, uint v2) {
            NativeMethods.glUniform3ui(location.value, v0, v1, v2);
        }

        internal void uniform4ui(WebGLUniformLocation location, uint v0, uint v1, uint v2, uint v3) {
            NativeMethods.glUniform4ui(location.value, v0, v1, v2, v3);
        }

        internal void clear(uint mask) {
            NativeMethods.glClear(mask);
        }

        internal void clearColor(float red, float green, float blue, float alpha) {
            NativeMethods.glClearColor(red, green, blue, alpha);
        }

        internal void texParameteri(GLenum target, GLenum pname, int param) {
            NativeMethods.glTexParameteri(target, pname, param);
        }

        internal void texParameteri(GLenum target, GLenum pname, GLenum param) {
            NativeMethods.glTexParameteri(target, pname, (int)((uint)param));
        }

        internal WebGLTexture createTexture() {
            uint[] result = new uint[1];
            NativeMethods.glGenTextures(1, result);
            return new WebGLTexture() { value = result[0] };
        }

        internal void generateMipmap(GLenum target) {
            NativeMethods.glGenerateMipmap(target);
        }

        internal void texImage2D(GLenum target, int level, GLenum internalformat, GLenum format, GLenum type, ImageData data) {
            int width = data.pow2_width;
            int height = data.pow2_height;
            IntPtr ptr = data.Addr;

            NativeMethods.glTexImage2D(target, level, internalformat, width, height, 0, format, type, ptr);
        }

        internal void texImage2D(GLenum target, int level, GLenum internalformat, int width, int height, int border, GLenum format, GLenum type, IntPtr pixels) {
            NativeMethods.glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
        }

        internal void compressedTexSubImage2D(GLenum target, int level, int xoffset, int yoffset, int width, int height, GLenum format, DataView srcData) {
            if (xoffset != 0 || yoffset != 0) throw new NotSupportedException("compressedTexSubImage2D() xoffset and yoffset are not supported");
            NativeMethods.glCompressedTexImage2D(target, level, format, width, height, 0, srcData.length, srcData.pointer);
        }

        internal IWebGLExtension getExtension(string name) {
            string gl_ext_name;
            IWebGLExtension ext;

            switch (name) {
                case "WEBGL_compressed_texture_s3tc":
                    gl_ext_name = "GL_EXT_texture_compression_s3tc";
                    ext = new WEBGL_compressed_texture_s3tc();
                    break;
                case "EXT_texture_compression_bptc":
                    // in GLES use "GL_EXT_texture_compression_bptc" instead
                    gl_ext_name = "GL_ARB_texture_compression_bptc";
                    ext = new EXT_texture_compression_bptc();
                    break;
                default:
                    return null;
            }

            int[] count = new int[1];
            NativeMethods.glGetIntegerv(GLenum.GL_NUM_EXTENSIONS, count);

            for (uint i = 0 ; i < count[0] ; i++) {
                IntPtr readed_ext_name_ptr = NativeMethods.glGetStringi(GLenum.GL_EXTENSIONS, i);
                string readed_ext_name = Marshal.PtrToStringAnsi(readed_ext_name_ptr);

                if (readed_ext_name == gl_ext_name) {
                    return ext;
                }
            }

#if DEBUG
            Console.WriteLine($"[WARN] {gl_ext_name} (${name}) not found or not supported");
#endif
            return null;
        }

        internal void pixelStorei(GLenum pname, int param) {
            if (pname == GLenum.UNPACK_PREMULTIPLY_ALPHA_WEBGL) return;
            NativeMethods.glPixelStorei(pname, param);
        }

        internal void blendFuncSeparate(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha) {
            NativeMethods.glBlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);
        }

        internal void blendFunc(GLenum sfactor, GLenum dfactor) {
            NativeMethods.glBlendFunc(sfactor, dfactor);
        }

        internal void enable(GLenum cap) {
            NativeMethods.glEnable(cap);
        }

        internal void depthMask(bool flag) {
            byte @bool = flag ? (byte)1 : (byte)0;
            NativeMethods.glDepthMask(@bool);
        }

        internal void disable(GLenum cap) {
            NativeMethods.glDisable(cap);
        }

        internal void bufferData(GLenum target, float[] srcData, GLenum usage) {
            GCHandle handle = GCHandle.Alloc(srcData, GCHandleType.Pinned);
            IntPtr data = handle.AddrOfPinnedObject();

            NativeMethods.glBufferData(target, new IntPtr(srcData.Length * sizeof(float)), data, usage);

            handle.Free();
        }

        internal void bufferData(GLenum target, uint[] srcData, GLenum usage) {
            GCHandle handle = GCHandle.Alloc(srcData, GCHandleType.Pinned);
            IntPtr data = handle.AddrOfPinnedObject();

            NativeMethods.glBufferData(target, new IntPtr(srcData.Length * sizeof(uint)), data, usage);

            handle.Free();
        }

        internal void bufferData(GLenum target, int maxelements, float[] srcData, GLenum usage) {
            GCHandle handle = GCHandle.Alloc(srcData, GCHandleType.Pinned);
            IntPtr data = handle.AddrOfPinnedObject();

            NativeMethods.glBufferData(target, new IntPtr(maxelements * sizeof(float)), data, usage);

            handle.Free();
        }

        internal void bufferData(GLenum target, int maxelements, uint[] srcData, GLenum usage) {
            GCHandle handle = GCHandle.Alloc(srcData, GCHandleType.Pinned);
            IntPtr data = handle.AddrOfPinnedObject();

            NativeMethods.glBufferData(target, new IntPtr(maxelements * sizeof(float)), data, usage);

            handle.Free();
        }

        internal void bufferData(GLenum target, int maxelements, byte[] srcData, GLenum usage) {
            GCHandle handle = GCHandle.Alloc(srcData, GCHandleType.Pinned);
            IntPtr data = handle.AddrOfPinnedObject();

            NativeMethods.glBufferData(target, new IntPtr(maxelements * sizeof(byte)), data, usage);

            handle.Free();
        }

        internal void deleteTexture(WebGLTexture texture) {
            uint[] values = { texture.value };
            NativeMethods.glDeleteTextures(1, values);
        }

        internal int getParameter(GLenum pname) {
            int[] value = new int[1];
            NativeMethods.glGetIntegerv(pname, value);
            return value[0];
        }


        internal WebGLVertexArrayObject createVertexArray() {
            uint[] values = new uint[1];
            NativeMethods.glGenVertexArrays(1, values);

            return new WebGLVertexArrayObject() { value = values[0] };
        }

        internal void bindVertexArray(WebGLVertexArrayObject array) {
            NativeMethods.glBindVertexArray(array.value);
        }

        internal void deleteVertexArray(WebGLVertexArrayObject array) {
            NativeMethods.glBindVertexArray(array.value);
        }

        internal void viewport(int x, int y, int width, int height) {
            NativeMethods.glViewport(x, y, width, height);
        }

        internal void getIntegerv(GLenum pname, int[] data) {
            NativeMethods.glGetIntegerv(pname, data);
        }

        internal void readPixels(int x, int y, int width, int height, GLenum format, GLenum type, IntPtr data) {
            NativeMethods.glReadPixels(x, y, width, height, format, type, data);
        }

        internal void uniform3fv(WebGLUniformLocation location, float[] value) {
            NativeMethods.glUniform3fv(location.value, value.Length, value);
        }

        internal void deleteBuffer(WebGLBuffer buffer) {
            uint[] value = new uint[] { buffer.value };
            NativeMethods.glDeleteBuffers(1, value);
        }

        internal WebGLFramebuffer createFramebuffer() {
            uint[] result = new uint[1];
            NativeMethods.glGenFramebuffers(1, result);
            return new WebGLFramebuffer() { value = result[0] };
        }

        internal void bindFramebuffer(GLenum target, WebGLFramebuffer framebuffer) {
            NativeMethods.glBindFramebuffer(target, framebuffer.value);
        }

        internal void bindFramebuffer(GLenum target, int framebuffer) {
            NativeMethods.glBindFramebuffer(target, (uint)framebuffer);
        }

        internal void framebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, WebGLTexture texture, int level) {
            NativeMethods.glFramebufferTexture2D(target, attachment, textarget, texture.value, level);
        }

        internal GLenum checkFramebufferStatus(GLenum target) {
            return NativeMethods.glCheckFramebufferStatus(target);
        }

        internal void invalidateFramebuffer(GLenum target, params GLenum[] attachments) {
            NativeMethods.glInvalidateFramebuffer(target, attachments.Length, attachments);
        }

        internal void deleteFramebuffer(WebGLFramebuffer framebuffer) {
            uint[] value = { framebuffer.value };
            NativeMethods.glDeleteFramebuffers(1, value);
        }

        internal WebGLActiveInfo getActiveUniform(WebGLProgram program, uint index) {
            int buffSize;
            NativeMethods.glGetProgramiv(program.value, GLenum.GL_ACTIVE_UNIFORM_MAX_LENGTH, out buffSize);

            if (buffSize < 1) return new WebGLActiveInfo() { size = -1, type = GLenum.GL_INVALID_VALUE };

            IntPtr ptr = Marshal.AllocHGlobal(buffSize);
            int length;
            int size;
            GLenum type;

            NativeMethods.glGetActiveUniform(program.value, index, buffSize, out length, out size, out type, ptr);

            string name = Marshal.PtrToStringAnsi(ptr, length);
            Marshal.FreeHGlobal(ptr);

            WebGLActiveInfo info = new WebGLActiveInfo() {
                type = type,
                size = size,
                name = name
            };

            return info;
        }

        internal void vertexAttribDivisor(uint index, uint divisor) {
            NativeMethods.glVertexAttribDivisor(index, divisor);
        }

        internal void drawElementsInstanced(GLenum mode, int count, GLenum type, int offset, int primcount) {
            KDY_draw_calls_count++;
            NativeMethods.glDrawElementsInstanced(mode, count, type, (IntPtr)offset, primcount);
        }

        internal IntPtr mapBuffer(GLenum target, GLenum access) {
            return NativeMethods.glMapBuffer(target, access);
        }

        internal bool unmapBuffer(GLenum target) {
            return NativeMethods.glUnmapBuffer(target) != 0;
        }

        internal void finish() {
            NativeMethods.glFinish();
        }

        internal void texSubImage2D(GLenum target, int level, int xoffset, int yoffset, int width, int height, GLenum format, GLenum type, IntPtr pixels) {
            NativeMethods.glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
        }

#pragma warning restore IDE1006

        public struct DataView {
            internal readonly int length;
            internal readonly IntPtr pointer;

            public DataView(IntPtr buffer, int byteOffset, int byteLength) {
                if (buffer == IntPtr.Zero) throw new ArgumentNullException("buffer");
                if (byteLength < 1) throw new ArgumentOutOfRangeException("byteLength");
                if (byteOffset < 0 || byteOffset >= byteLength) throw new ArgumentOutOfRangeException("byteOffset");

                pointer = IntPtr.Add(buffer, byteOffset);
                length = byteLength;
            }
        }

    }
}
