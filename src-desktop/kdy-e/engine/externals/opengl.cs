using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Engine.Externals {

    public struct WebGLBuffer {
        internal uint value;
        public static WebGLBuffer Null = new WebGLBuffer() { value = 0x0000 };
        public override string ToString() {
            return "WebGLBuffer: " + (value == 0 ? "(null)" : value.ToString());
        }
    }
    public struct WebGLUniformLocation {
        internal int value;
        public static WebGLUniformLocation Null = new WebGLUniformLocation() { value = -1 };
        public override string ToString() {
            return "WebGLUniformLocation: " + (value == 0 ? "(null)" : value.ToString());
        }
    }
    public struct WebGLProgram {
        internal uint value;
        public static WebGLProgram Null = new WebGLProgram() { value = 0x0000 };
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
        public override string ToString() {
            return "WebGLShader: " + (value == 0 ? "(null)" : value.ToString());
        }
    }

    public partial class WebGLRenderingContext {

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

        internal GLenum getProgramParameter(WebGLProgram program, GLenum pname) {
            int result = 0;
            NativeMethods.glGetProgramiv(program.value, pname, out result);
            return (GLenum)result;
        }

        internal string getProgramInfoLog(WebGLProgram program) {
            int length = 0;
            NativeMethods.glGetProgramiv(program.value, GLenum.GL_INFO_LOG_LENGTH, out length);
            if (length < 1) return String.Empty;

            IntPtr temp = Marshal.AllocHGlobal(length * 2);
            NativeMethods.glGetProgramInfoLog(program.value, length, out length, temp);

            string str = Marshal.PtrToStringAnsi(temp);
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
            int length;
            NativeMethods.glGetShaderiv(shader.value, GLenum.GL_INFO_LOG_LENGTH, out length);
            if (length < 1) return String.Empty;

            IntPtr temp = Marshal.AllocHGlobal(length * 2);
            NativeMethods.glGetShaderInfoLog(shader.value, length, out length, temp);

            string str = Marshal.PtrToStringAnsi(temp);
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

        internal void uniform1f(WebGLUniformLocation location, float value) {
            NativeMethods.glUniform1f(location.value, value);
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

        internal IWebGLExtension getExtension(string name) {
            switch (name) {
                case "OES_element_index_uint":
                    return new OES_element_index_uint();
                case "ANGLE_instanced_arrays":
                    return new ANGLE_instanced_arrays();
                default:
                    return null;
            }
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
        
        internal void deleteTexture(WebGLTexture texture) {
            uint[] values = { texture.value };
            NativeMethods.glDeleteTextures(1, values);
        }

        internal int getParameter(GLenum pname) {
            int[] value = new int[1];
            NativeMethods.glGetIntegerv(pname, value);
            return value[0];
        }


        internal uint genVertexArray() {
            uint[] values = new uint[1];
            NativeMethods.glGenVertexArrays(1, values);

            return values[0];
        }

        internal void bindVertexArray(uint array) {
            NativeMethods.glBindVertexArray(array);
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


#pragma warning restore IDE1006

    }
}
