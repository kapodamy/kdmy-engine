using System;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Xml;

namespace Engine.Externals {

    using GLuint = UInt32;
    using GLint = Int32;
    using GLsizei = Int32;
    using GLfloat = Single;
    using GLboolean = Byte;
    using GLsizeiptr = IntPtr;
    using GLchar = String;// GLchar*
    using GLbitfield = UInt32;
    using GLclampd = Double;

    public partial class WebGLRenderingContext {

        internal static class NativeMethods {

            internal delegate void DglViewport(GLint x, GLint y, GLsizei width, GLsizei height);
            internal delegate void DglHint(GLenum target, GLenum mode);
            internal delegate void DglDepthFunc(GLenum func);
            internal delegate void DglClearDepth(GLclampd depth);
            internal delegate void DglShadeModel(GLenum mode);
            internal delegate IntPtr DglGetString(GLenum name);
            internal delegate void DglFlush();
            internal delegate GLenum DglGetError();
            internal delegate void DglActiveTexture(GLenum texture);
            internal delegate void DglBindTexture(GLenum target, GLuint texture);
            internal delegate void DglUseProgram(GLuint program);
            internal delegate void DglDeleteProgram(GLuint program);
            internal delegate GLuint DglCreateProgram();
            internal delegate void DglBindBuffer(GLenum target, GLuint buffer);
            internal delegate void DglEnableVertexAttribArray(GLuint index);
            internal delegate void DglGetProgramiv(GLuint program, GLenum pname, out GLint @params);
            internal delegate void DglGetProgramInfoLog(GLuint program, GLsizei maxLength, out GLsizei length, IntPtr infoLog);
            internal delegate void DglAttachShader(GLuint program, GLuint shader);
            internal delegate void DglLinkProgram(GLuint program);
            internal delegate void DglGetShaderiv(GLuint shader, GLenum pname, out GLint @params);
            internal delegate void DglDeleteShader(GLuint shader);
            internal delegate void DglGetShaderInfoLog(GLuint shader, GLsizei maxLength, out GLsizei length, IntPtr infoLog);
            internal delegate void DglCompileShader(GLuint shader);
            internal delegate void DglShaderSource(GLuint shader, GLsizei count, ref GLchar @string, ref GLint length);
            internal delegate GLuint DglCreateShader(GLenum shaderType);
            internal delegate void DglDrawArrays(GLenum mode, GLint first, GLsizei count);
            internal delegate void DglUniform4fv(GLint location, GLsizei count, GLfloat[] value);
            internal delegate void DglUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, GLfloat[] value);
            internal delegate void DglVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, IntPtr pointer);
            internal delegate GLint DglGetAttribLocation(GLuint program, GLchar name);
            internal delegate GLint DglGetUniformLocation(GLuint program, GLchar name);
            internal delegate void DglGenBuffers(GLsizei n, [In, Out] GLuint[] buffers);
            internal delegate void DglUniform1i(GLint location, GLint v0);
            internal delegate void DglUniform1f(GLint location, GLfloat v0);
            internal delegate void DglClear(GLbitfield mask);
            internal delegate void DglClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
            internal delegate void DglTexParameteri(GLenum target, GLenum pname, GLint param);
            internal delegate void DglGenTextures(GLsizei n, [In, Out] GLuint[] textures);
            internal delegate void DglGenerateMipmap(GLenum target);
            internal delegate void DglTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, IntPtr data);
            internal delegate void DglPixelStorei(GLenum pname, GLint param);
            internal delegate void DglBlendFuncSeparate(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
            internal delegate void DglBlendFunc(GLenum sfactor, GLenum dfactor);
            internal delegate void DglEnable(GLenum cap);
            internal delegate void DglDisable(GLenum cap);
            internal delegate void DglDepthMask(GLboolean flag);
            internal delegate void DglBufferData(GLenum target, GLsizeiptr size, IntPtr data, GLenum usage);
            internal delegate void DglDeleteTextures(GLsizei n, GLuint[] textures);
            internal delegate void DglGetIntegerv(GLenum pname, [In, Out] GLint[] data);
            internal delegate void DglVertexAttribDivisor(GLuint index, GLuint divisor);
            internal delegate void DglDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, IntPtr indices, GLsizei instancecount);
            internal delegate void DglGenVertexArrays(int n, [In, Out] uint[] arrays);
            internal delegate void DglBindVertexArray(uint array);
            internal delegate void DglReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, IntPtr data);
            internal delegate void DglDrawElements(GLenum mode, GLsizei count, GLenum type, IntPtr indices);

#pragma warning disable CS0649
            internal static readonly DglViewport glViewport;
            internal static readonly DglHint glHint;
            internal static readonly DglDepthFunc glDepthFunc;
            internal static readonly DglClearDepth glClearDepth;
            internal static readonly DglShadeModel glShadeModel;
            internal static readonly DglGetString glGetString;
            internal static readonly DglFlush glFlush;
            internal static readonly DglGetError glGetError;
            internal static readonly DglActiveTexture glActiveTexture;
            internal static readonly DglBindTexture glBindTexture;
            internal static readonly DglUseProgram glUseProgram;
            internal static readonly DglDeleteProgram glDeleteProgram;
            internal static readonly DglCreateProgram glCreateProgram;
            internal static readonly DglBindBuffer glBindBuffer;
            internal static readonly DglEnableVertexAttribArray glEnableVertexAttribArray;
            internal static readonly DglGetProgramiv glGetProgramiv;
            internal static readonly DglGetProgramInfoLog glGetProgramInfoLog;
            internal static readonly DglAttachShader glAttachShader;
            internal static readonly DglLinkProgram glLinkProgram;
            internal static readonly DglGetShaderiv glGetShaderiv;
            internal static readonly DglDeleteShader glDeleteShader;
            internal static readonly DglGetShaderInfoLog glGetShaderInfoLog;
            internal static readonly DglCompileShader glCompileShader;
            internal static readonly DglShaderSource glShaderSource;
            internal static readonly DglCreateShader glCreateShader;
            internal static readonly DglDrawArrays glDrawArrays;
            internal static readonly DglUniform4fv glUniform4fv;
            internal static readonly DglUniformMatrix4fv glUniformMatrix4fv;
            internal static readonly DglVertexAttribPointer glVertexAttribPointer;
            internal static readonly DglGetAttribLocation glGetAttribLocation;
            internal static readonly DglGetUniformLocation glGetUniformLocation;
            internal static readonly DglGenBuffers glGenBuffers;
            internal static readonly DglUniform1i glUniform1i;
            internal static readonly DglUniform1f glUniform1f;
            internal static readonly DglClear glClear;
            internal static readonly DglClearColor glClearColor;
            internal static readonly DglTexParameteri glTexParameteri;
            internal static readonly DglGenTextures glGenTextures;
            internal static readonly DglGenerateMipmap glGenerateMipmap;
            internal static readonly DglTexImage2D glTexImage2D;
            internal static readonly DglPixelStorei glPixelStorei;
            internal static readonly DglBlendFuncSeparate glBlendFuncSeparate;
            internal static readonly DglBlendFunc glBlendFunc;
            internal static readonly DglEnable glEnable;
            internal static readonly DglDisable glDisable;
            internal static readonly DglDepthMask glDepthMask;
            internal static readonly DglBufferData glBufferData;
            internal static readonly DglDeleteTextures glDeleteTextures;
            internal static readonly DglGetIntegerv glGetIntegerv;
            internal static readonly DglVertexAttribDivisor glVertexAttribDivisor;
            internal static readonly DglDrawElementsInstanced glDrawElementsInstanced;
            internal static readonly DglGenVertexArrays glGenVertexArrays;
            internal static readonly DglBindVertexArray glBindVertexArray;
            internal static readonly DglReadPixels glReadPixels;
            internal static readonly DglDrawElements glDrawElements;
#pragma warning restore CS0649


            [DllImport("opengl32", SetLastError = true)]
            private static extern IntPtr wglGetProcAddress(string name);
            [DllImport("kernel32", SetLastError = true)]
            private static extern IntPtr GetProcAddress(IntPtr hModule, string procName);
            [DllImport("kernel32", CharSet = CharSet.Ansi, SetLastError = true)]
            public static extern IntPtr GetModuleHandle(string lpModuleName);

            private static Delegate GetDelegateFor(Type delegateType, string functionName) {
                IntPtr proc = wglGetProcAddress(functionName);

                if (proc != IntPtr.Zero) {
                    return Marshal.GetDelegateForFunctionPointer(proc, delegateType);
                }

                // if on Windows 10 fails, try load directly from the dll
                IntPtr handle = GetModuleHandle("opengl32");
                if (handle != IntPtr.Zero) {
                    // retrieve the function
                    proc = GetProcAddress(handle, functionName);
                    if (proc != IntPtr.Zero) {
                        return Marshal.GetDelegateForFunctionPointer(proc, delegateType);
                    }
                }

                Console.Error.WriteLine("Fatal error: OpenGL function " + functionName + " not found");
                throw new MissingMethodException("OpenGL function " + functionName + " not found");
            }

            static NativeMethods() {
                Type delegateType = typeof(Delegate);
                Type self = typeof(NativeMethods);
                FieldInfo[] fields = self.GetFields(BindingFlags.Static | BindingFlags.NonPublic);

                foreach (FieldInfo field in fields) {
                    Type memberType = field.FieldType;
                    if (!delegateType.IsAssignableFrom(memberType)) continue;
                    if (!memberType.Name.StartsWith("D")) continue;

                    Delegate wrappedFunction = GetDelegateFor(memberType, field.Name);
                    field.SetValue(null, wrappedFunction);
                }
            }

        }

    }

}
