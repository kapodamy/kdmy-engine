using System;
using System.Runtime.InteropServices;
using System.Text.RegularExpressions;
using Engine.Externals;
using Engine.Externals.GLFW;
using Engine.Image;
using Engine.Utils;

namespace Engine.Platform {

    public class WebGLContext {

        public static readonly float[] MAT4 = new float[4 * 4];
        public static readonly float[] RGBA = new float[2 * 2];
        public static readonly float[] QUAD = new float[] { 0f, 0f, 0f, 1f, 1f, 0f, 1f, 0f, 0f, 1f, 1f, 1f };
        public static readonly float[] QUAD_SCREEN = new float[] {
            -1f, 1f, 0f, 1f, -1f, -1, 0f, 0f, 1f, -1f, 1f, 0f, -1f, 1f, 0f, 1f,1f, -1f, 1f, 0f, 1f, 1f, 1f, 1f
        };

        private static readonly Regex rx_header = new Regex(@"#pragma header[\s\t\xA0]*\r?\n", RegexOptions.Compiled);
        private static readonly Regex rx_shadertoy = new Regex(@"#pragma shadertoy_mainImage[\s\t\xA0]*\r?\n", RegexOptions.Compiled);

        public WebGLContextProgram program_solid;
        public WebGLContextProgram program_textured;
        public WebGLContextProgram program_framebuffer;
        public WebGLContextProgramGlyphs program_glyphs;

        public WebGL2RenderingContext gl;

        public WebGLBuffer position_buffer;

        internal WebGLShader stock_shadervertex;
        internal WebGLShader stock_shaderfragment;

        internal WebGLContext() {
            this.gl = new WebGL2RenderingContext();
            Console.Error.WriteLine("OpenGL: " + this.gl.getString(GLenum.GL_VERSION));

            // setup GLSL programs
            WebGLProgram program_solid = InternalCreateProgram(gl, "solid");
            WebGLProgram program_textured = InternalCreateProgram(gl, "textured");
            WebGLProgram program_framebuffer = InternalCreateProgram(gl, "framebuffer");
            WebGLProgram program_glyphs = InternalCreateProgram(gl, "glyphs");

            this.program_solid = new WebGLContextProgram(gl, program_solid);
            this.program_textured = new WebGLContextProgram(gl, program_textured);
            this.program_glyphs = new WebGLContextProgramGlyphs(gl, program_glyphs);
            this.program_framebuffer = new WebGLContextProgram(gl, program_framebuffer);

            // quads buffer
            this.position_buffer = gl.createBuffer();
            gl.bindBuffer(gl.ARRAY_BUFFER, this.position_buffer);

            // Put a unit quad in the buffer
            gl.bufferData(gl.ARRAY_BUFFER, WebGLContext.QUAD, gl.STATIC_DRAW);

            gl.disable(gl.CULL_FACE);
            gl.disable(gl.DEPTH_TEST);
            gl.depthMask(false);

            //gl.blendFuncSeparate(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA, gl.ONE, gl.ONE_MINUS_SRC_ALPHA);
            gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
            gl.enable(gl.BLEND);

            gl.pixelStorei(gl.UNPACK_PREMULTIPLY_ALPHA_WEBGL, 0);

            // convert from pixels to clip space
            //ResizeProjection(pvrctx);

            // build stock post-processing shaders
            this.stock_shadervertex = this.InternalCreateShader(
                gl, WebGLContext.InternalPatchShader(WebGLContext.InternalLoadShader("stock", true), true), true, true
            );
            this.stock_shaderfragment = this.InternalCreateShader(
                gl, WebGLContext.InternalPatchShader(WebGLContext.InternalLoadShader("stock", false), false), false, true
            );

        }

        public WebGLTexture CreateTexture(int pow2_tex_width, int pow2_tex_height, ImageData bitmap_data) {
            WebGL2RenderingContext gl = this.gl;

            PixelUnPackBuffer pixelDataBuffer = (PixelUnPackBuffer)bitmap_data.pixelDataBuffer;

            // clear any previous error
            gl.getError();

            WebGLTexture tex = gl.createTexture();

            gl.activeTexture(gl.TEXTURE0);
            gl.bindTexture(gl.TEXTURE_2D, tex);

            if (pixelDataBuffer != null) {
                gl.bindBuffer(gl.PIXEL_UNPACK_BUFFER, pixelDataBuffer.pbo);

                if (pixelDataBuffer.mapped_buffer != IntPtr.Zero) {
                    gl.unmapBuffer(gl.PIXEL_UNPACK_BUFFER);
                    pixelDataBuffer.mapped_buffer = IntPtr.Zero;
                }

                gl.texImage2D(
                    gl.TEXTURE_2D, 0, gl.RGBA,
                    bitmap_data.pow2_width, bitmap_data.pow2_height, 0,
                    gl.RGBA, gl.UNSIGNED_BYTE, IntPtr.Zero
                );

                gl.bindBuffer(gl.PIXEL_UNPACK_BUFFER, WebGLBuffer.Null);
            } else {
                gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, bitmap_data);
            }

            GLenum error = gl.getError();
            if (error != GLenum.GL_NONE) {
                gl.deleteTexture(tex);
                Console.Error.WriteLine("[ERROR] texImage2D() failed: " + error.ToString());
                return WebGLTexture.Null;
            }

            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);

            //gl.generateMipmap(gl.TEXTURE_2D);

            return tex;
        }

        public void ChangeTextureFiltering(PVRContext pvrctx) {
            WebGL2RenderingContext gl = pvrctx.webopengl.gl;

            int filter;
            if (pvrctx.render_antialiasing == PVRContextFlag.DISABLE)
                filter = gl.NEAREST;
            else
                filter = gl.LINEAR;

            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, filter);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, filter);
        }

        public void DestroyTexture(WebGLTexture texture) {
            this.gl.deleteTexture(texture);
        }

        public void ClearScreen(float[] rgba_color) {
            if (rgba_color != null)
                this.gl.clearColor(rgba_color[0], rgba_color[1], rgba_color[2], rgba_color[3]);
            else
                this.gl.clearColor(0f, 0f, 0f, 0f);

            this.gl.clear(this.gl.COLOR_BUFFER_BIT);
        }

        public void ResizeProjection(int offset_x, int offset_y, int width, int height) {
            WebGLContext.InternalIdentity(WebGLContext.MAT4, 4);
            InternalOrthographic2D(WebGLContext.MAT4, width, height);
            this.gl.viewport(offset_x, offset_y, width, height);
            this.program_solid.ResizeProyection(this.gl, WebGLContext.MAT4);
            this.program_textured.ResizeProyection(this.gl, WebGLContext.MAT4);
            this.program_glyphs.ResizeProyection(this.gl, WebGLContext.MAT4);

            WebGLContext.MAT4[5] = -WebGLContext.MAT4[5];
            this.program_framebuffer.ResizeProyection(this.gl, WebGLContext.MAT4);
        }


        public void DrawTexture(PVRContext pvrctx, Texture tex, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh) {
            WebGL2RenderingContext gl = this.gl;
            int tex_width = tex.width;
            int tex_height = tex.height;

            gl.useProgram(this.program_textured.program);
            gl.bindVertexArray(this.program_textured.vao);

            // bind texture and update antialiasing
            gl.bindTexture(gl.TEXTURE_2D, tex.data_vram);
            ChangeTextureFiltering(pvrctx);

            gl.bindBuffer(gl.ARRAY_BUFFER, this.position_buffer);
            gl.enableVertexAttribArray(this.program_textured.a_position);
            gl.vertexAttribPointer(this.program_textured.a_position, 2, gl.FLOAT, false, 0, 0);

            // optimized matrix model, contains the draw size and location
            WebGLContext.InternalIdentity(WebGLContext.MAT4, 4);
            WebGLContext.MAT4[0] = dw;
            WebGLContext.MAT4[5] = dh;
            WebGLContext.MAT4[12] = dx;
            WebGLContext.MAT4[13] = dy;
            gl.uniformMatrix4fv(this.program_textured.u_matrix_model, false, WebGLContext.MAT4);

            // copy transformation matrix (with all modifiers applied)
            gl.uniformMatrix4fv(this.program_textured.u_matrix_transform, false, pvrctx.CurrentMatrix.matrix);

            // optimized sub-texture matrix, contains the source size and location
            WebGLContext.InternalIdentity(WebGLContext.MAT4, 4);
            WebGLContext.MAT4[12] = sx / tex_width;
            WebGLContext.MAT4[13] = sy / tex_height;
            if (sw != tex_width) WebGLContext.MAT4[0] = sw / tex_width;
            if (sh != tex_height) WebGLContext.MAT4[5] = sh / tex_height;
            gl.uniformMatrix4fv(this.program_textured.u_matrix_texture, false, WebGLContext.MAT4);

            // Tell the shader to get the texture from texture unit 0
            gl.uniform1i(this.program_textured.u_texture, 0);
            gl.uniform1f(this.program_textured.u_alpha, pvrctx.render_alpha);

            // if the offsetcolor alpha is negative, disable the offsetcolor processing
            // "u_offsetcolor_enabled" and "u_offsetcolor_mul_or_diff" are boolean values
            if (pvrctx.render_offsetcolor[3] < 0) {
                gl.uniform1i(this.program_textured.u_offsetcolor_enabled, 0);
            } else {
                gl.uniform1i(this.program_textured.u_offsetcolor_enabled, 1);
                gl.uniform4fv(this.program_textured.u_offsetcolor, pvrctx.render_offsetcolor);
                bool use_multiply = pvrctx.render_offsetcolor_multiply != PVRContextFlag.DISABLE;
                gl.uniform1i(this.program_textured.u_offsetcolor_mul_or_diff, use_multiply ? 1 : 0);
            }

            // enable/disable rgb color components be multiplied by the render alpha
            gl.uniform1i(this.program_textured.u_darken, this.program_textured.darken_enabled ? 1 : 0);

            // draw the quad (2 triangles, 6 vertices)
            gl.drawArrays(gl.TRIANGLES, 0, 6);

            // unbind buffers and texture
            gl.bindBuffer(gl.ARRAY_BUFFER, WebGLBuffer.Null);
            gl.bindTexture(gl.TEXTURE_2D, WebGLTexture.Null);
            gl.bindVertexArray(WebGLVertexArrayObject.Null);

            // required to avoid GL_OUT_OF_MEMORY
            gl.flush();
        }

        public void DrawSolid(PVRContext pvrctx, float[] rgb_color, float dx, float dy, float dw, float dh) {
            WebGL2RenderingContext gl = this.gl;

            gl.useProgram(this.program_solid.program);
            gl.bindVertexArray(this.program_solid.vao);

            gl.bindBuffer(gl.ARRAY_BUFFER, this.position_buffer);
            gl.enableVertexAttribArray(this.program_solid.a_position);
            gl.vertexAttribPointer(this.program_solid.a_position, 2, gl.FLOAT, false, 0, 0);

            // optimized matrix model, contains the draw size and location
            WebGLContext.InternalIdentity(WebGLContext.MAT4, 4);
            WebGLContext.MAT4[0] = dw;
            WebGLContext.MAT4[5] = dh;
            WebGLContext.MAT4[12] = dx;
            WebGLContext.MAT4[13] = dy;
            gl.uniformMatrix4fv(this.program_solid.u_matrix_model, false, WebGLContext.MAT4);

            // copy transformation matrix (with all modifiers applied)
            gl.uniformMatrix4fv(this.program_solid.u_matrix_transform, false, pvrctx.CurrentMatrix.matrix);

            // if the offsetcolor alpha is negative, disable the offsetcolor processing
            // "u_offsetcolor_enabled" and "u_offsetcolor_mul_or_diff" are boolean values
            if (pvrctx.render_offsetcolor[3] < 0) {
                gl.uniform1i(this.program_solid.u_offsetcolor_enabled, 0);
            } else {
                gl.uniform1i(this.program_solid.u_offsetcolor_enabled, 1);
                gl.uniform4fv(this.program_solid.u_offsetcolor, pvrctx.render_offsetcolor);
                bool use_multiply = pvrctx.render_offsetcolor_multiply != PVRContextFlag.DISABLE;
                gl.uniform1i(this.program_solid.u_offsetcolor_mul_or_diff, use_multiply ? 1 : 0);
            }

            // build rgba color & upload vertex color 
            WebGLContext.RGBA[0] = rgb_color[0];
            WebGLContext.RGBA[1] = rgb_color[1];
            WebGLContext.RGBA[2] = rgb_color[2];
            WebGLContext.RGBA[3] = pvrctx.render_alpha;
            gl.uniform4fv(this.program_solid.u_vertex_color, WebGLContext.RGBA);

            // draw the quad (2 triangles, 6 vertices)
            gl.drawArrays(gl.TRIANGLES, 0, 6);

            // unbind buffers
            gl.bindBuffer(gl.ARRAY_BUFFER, WebGLBuffer.Null);
            gl.bindVertexArray(WebGLVertexArrayObject.Null);
        }

        public void DrawFramebuffer(PVRContext pvrctx, PSFramebuffer frmbffr, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh) {
            var wglc = pvrctx.webopengl;
            var gl = pvrctx.webopengl.gl;
            float tex_width = frmbffr.pow2_width;
            float tex_height = frmbffr.pow2_height;

            // Tell WebGL to use our shader program pair
            gl.useProgram(wglc.program_framebuffer.program);
            gl.bindVertexArray(wglc.program_framebuffer.vao);

            // bind texture and update antialiasing
            gl.bindTexture(gl.TEXTURE_2D, frmbffr.texture);

            gl.bindBuffer(gl.ARRAY_BUFFER, wglc.position_buffer);
            gl.enableVertexAttribArray(wglc.program_framebuffer.a_position);
            gl.vertexAttribPointer(wglc.program_framebuffer.a_position, 2, gl.FLOAT, false, 0, 0);

            //
            // magic, literally. I do not even know what i am calculating here
            // also in webopengl_resize_projection() the proyection matrix is not inverted
            //
            sy = frmbffr.screen_dimmens[1] - sy;
            sy -= frmbffr.screen_dimmens[1];
            sy -= sh - frmbffr.screen_dimmens[1];
            dy = frmbffr.screen_dimmens[1] - dy;
            dy -= frmbffr.screen_dimmens[1];
            dy -= dh - frmbffr.screen_dimmens[1];
            dy -= frmbffr.screen_dimmens[1];

            // optimized matrix model, contains the draw size and location
            WebGLContext.InternalIdentity(WebGLContext.MAT4, 4);
            WebGLContext.MAT4[0] = dw;
            WebGLContext.MAT4[5] = dh;
            WebGLContext.MAT4[12] = dx;
            WebGLContext.MAT4[13] = dy;
            gl.uniformMatrix4fv(wglc.program_framebuffer.u_matrix_model, false, WebGLContext.MAT4);

            // copy transformation matrix (with all modifiers applied)
            gl.uniformMatrix4fv(wglc.program_framebuffer.u_matrix_transform, false, pvrctx.CurrentMatrix.matrix);

            // optimized sub-texture matrix, contains the source size and location
            WebGLContext.InternalIdentity(WebGLContext.MAT4, 4);
            WebGLContext.MAT4[12] = sx / tex_width;
            WebGLContext.MAT4[13] = sy / tex_height;
            if (sw != tex_width) WebGLContext.MAT4[0] = sw / tex_width;
            if (sh != tex_height) WebGLContext.MAT4[5] = sh / tex_height;
            gl.uniformMatrix4fv(wglc.program_framebuffer.u_matrix_texture, false, WebGLContext.MAT4);

            // Tell the shader to get the texture from texture unit 0
            gl.uniform1i(wglc.program_framebuffer.u_texture, 0);
            gl.uniform1f(wglc.program_framebuffer.u_alpha, pvrctx.render_alpha);

            // draw the quad (2 triangles, 6 vertices)
            gl.drawArrays(gl.TRIANGLES, 0, 6);

            // unbind buffers and texture
            gl.bindBuffer(gl.ARRAY_BUFFER, WebGLBuffer.Null);
            gl.bindTexture(gl.TEXTURE_2D, WebGLTexture.Null);
            gl.bindVertexArray(WebGLVertexArrayObject.Null);
        }

        public void Flush() {
            this.gl.flush();
        }

        public IntPtr ReadFrameBuffer(out int width, out int height) {
            int[] viewport = new int[4];
            this.gl.getIntegerv(this.gl.VIEWPORT, viewport);

            int x = viewport[0];
            int y = viewport[1];
            width = viewport[2];
            height = viewport[3];

            IntPtr data = Marshal.AllocHGlobal(width * height * sizeof(uint));

            //this.gl.pixelStorei(this.gl.PACK_ALIGNMENT, 1);
            this.gl.readPixels(x, y, width, height, this.gl.RGBA, this.gl.UNSIGNED_BYTE, data);

            GLenum error = this.gl.getError();
            if (error != this.gl.NONE) {
                Console.Error.WriteLine("[ERROR] webopengl_read_framebuffer() failed: " + error.ToString());
                Marshal.FreeHGlobal(data);
                return IntPtr.Zero;
            }

            return data;
        }

        public void SetBlend(PVRContext pvrctx, bool enabled, Blend src_rgb, Blend dst_rgb, Blend src_alpha, Blend dst_alpha) {
            var gl = pvrctx.webopengl.gl;

            if (enabled) {
                gl.enable(gl.BLEND);
            } else {
                gl.disable(gl.BLEND);
                return;
            }

            GLenum src = WebGLContext.InternalGetBlend(gl, src_rgb, gl.SRC_ALPHA);
            GLenum dst = WebGLContext.InternalGetBlend(gl, dst_rgb, gl.ONE_MINUS_SRC_ALPHA);
            GLenum src_a = WebGLContext.InternalGetBlend(gl, src_alpha, gl.ONE);
            GLenum dst_a = WebGLContext.InternalGetBlend(gl, dst_alpha, gl.ONE_MINUS_SRC_ALPHA);

            gl.blendFuncSeparate(src, dst, src_a, dst_a);
        }

        private void InternalOrthographic2D(float[] matrix, int screen_width, int screen_height) {
            // optimized orthographic matrix creation
            matrix[0] = 2f / screen_width;
            matrix[1] = matrix[2] = matrix[3] = matrix[4] = 0f;
            matrix[5] = -2f / screen_height;// invert the y axis (avoids render upside down)
            matrix[6] = matrix[7] = matrix[8] = matrix[9] = 0f;
            matrix[10] = matrix[12] = -1f;
            matrix[11] = matrix[14] = 0f;
            matrix[13] = matrix[15] = 1f;
        }

        internal WebGLShader InternalCreateShader(WebGL2RenderingContext gl, string source, bool is_vertex_shader, bool throw_error) {
            WebGLShader shader = gl.createShader(is_vertex_shader ? gl.VERTEX_SHADER : gl.FRAGMENT_SHADER);
            gl.shaderSource(shader, source);
            gl.compileShader(shader);

            // Check the compile status
            GLenum status = gl.getShaderParameter(shader, gl.COMPILE_STATUS);
            string infolog = gl.getShaderInfoLog(shader);
            if (!String.IsNullOrEmpty(infolog)) Console.Error.WriteLine(infolog + "\n");
            if (status == gl.TRUE) return shader;

            string msg = gl.getShaderInfoLog(shader);
            string type = is_vertex_shader ? "vertex" : "fragment";
            Console.Error.WriteLine($"[ERROR] webopengl_internal_create_shader() failed to create a {type} shader:\n${msg}\n{source}");
            gl.deleteShader(shader);

            string error_msg = type + " shader compilation failed\n" + msg;

            if (throw_error) throw new Exception(error_msg);

            return WebGLShader.Null;
        }

        private WebGLProgram InternalCreateProgram(WebGL2RenderingContext gl, string shader_name) {
            string vertex_shader_sourcecode = InternalLoadShader(shader_name, true);
            string fragment_shader_sourcecode = InternalLoadShader(shader_name, false);

            if (vertex_shader_sourcecode == null) throw new Exception("can not load vertex shader for " + shader_name);
            if (fragment_shader_sourcecode == null) throw new Exception("can not load fragment for " + shader_name);

            WebGLShader vertex_shader = InternalCreateShader(gl, vertex_shader_sourcecode, true, true);
            WebGLShader fragment_shader = InternalCreateShader(gl, fragment_shader_sourcecode, false, true);

            WebGLProgram program = gl.createProgram();
            gl.attachShader(program, vertex_shader);
            gl.attachShader(program, fragment_shader);

            gl.linkProgram(program);

            GLenum status = (GLenum)gl.getProgramParameter(program, gl.LINK_STATUS);
            string infolog = gl.getProgramInfoLog(program);
            if (!String.IsNullOrEmpty(infolog)) Console.Error.WriteLine(infolog + "\n");
            if (status == gl.TRUE) return program;

            // something went wrong with the link
            Console.Error.WriteLine("InternalCreateProgram() failed");
            gl.deleteProgram(program);
            throw new Exception("Shaders compilation failed\n");
        }

        private static void InternalIdentity(float[] matrix, int stride) {
            for (int i = 0 ; i < matrix.Length ; i++) matrix[i] = 0f;

            if (stride == 3)
                matrix[0] = matrix[4] = matrix[8] = 1f;
            else if (stride == 4)
                matrix[0] = matrix[5] = matrix[10] = matrix[15] = 1f;

        }

        private static string SetShaderVersion(string sourcecode) {
            return "#version 330 core\nprecision highp float;\n\n" + sourcecode;
        }

        private static string InternalLoadShader(string shader_prefix, bool vertexshader_or_fragmentshader) {
            string shader_suffix = vertexshader_or_fragmentshader ? "vertexshader" : "fragmentshader";
            string path = "/assets/shaders/" + shader_prefix + "_" + shader_suffix + ".glsl";
            string text = IO.ReadText(path);

            if (shader_prefix != "stock" && !String.IsNullOrEmpty(text)) text = WebGLContext.SetShaderVersion(text);

            return text;
        }


        internal static string InternalPatchShader(string sourcecode, bool is_vertex_shader) {
            const string header_vertex = "in vec4 a_position;\nin vec2 a_texcoord;\nout vec2 TexCoord;";
            const string header_fragment = "out vec4 FragColor;\nin vec2 TexCoord;\nuniform sampler2D Texture;";

            const string common = "uniform float u_kdy_time, u_kdy_delta;\nuniform int u_kdy_frame;\nuniform vec3 u_kdy_screen, u_kdy_texsize;\n";

            const string shadertoy_stubs = @"
#define iResolution vec3(1.0)
#define iTime u_kdy_time
#define iTimeDelta u_kdy_delta
#define iFrame u_kdy_frame
#define iChannel0 Texture
#define iFrameRate (1.0/u_kdy_delta)
#define iChannelTime (float[1](u_kdy_time))
#define iChannelResolution (float[1](u_kdy_texsize))

void mainImage(out vec4, in vec2);
void main() { mainImage(FragColor, TexCoord); }

";

            // remove all empty lines or spaces at the start
            sourcecode = sourcecode.TrimStart();

            sourcecode = WebGLContext.SetShaderVersion(sourcecode);

            if (is_vertex_shader) {
                sourcecode = rx_header.Replace(sourcecode, $"{header_vertex}\n\n{common}");
            } else {
                sourcecode = rx_header.Replace(sourcecode, $"{header_fragment}\n\n{common}");
                sourcecode = rx_shadertoy.Replace(sourcecode, shadertoy_stubs);
            }
            return sourcecode;
        }

        private static GLenum InternalGetBlend(WebGL2RenderingContext gl, Blend enum_blend, GLenum def) {
            switch (enum_blend) {
                default:
                    return def;
                case Blend.ZERO:
                    return gl.ZERO;
                case Blend.ONE:
                    return gl.ONE;
                case Blend.SRC_COLOR:
                    return gl.SRC_COLOR;
                case Blend.ONE_MINUS_SRC_COLOR:
                    return gl.ONE_MINUS_SRC_COLOR;
                case Blend.DST_COLOR:
                    return gl.DST_COLOR;
                case Blend.ONE_MINUS_DST_COLOR:
                    return gl.ONE_MINUS_DST_COLOR;
                case Blend.SRC_ALPHA:
                    return gl.SRC_ALPHA;
                case Blend.ONE_MINUS_SRC_ALPHA:
                    return gl.ONE_MINUS_SRC_ALPHA;
                case Blend.DST_ALPHA:
                    return gl.DST_ALPHA;
                case Blend.ONE_MINUS_DST_ALPHA:
                    return gl.ONE_MINUS_DST_ALPHA;
                case Blend.CONSTANT_COLOR:
                    return gl.CONSTANT_COLOR;
                case Blend.ONE_MINUS_CONSTANT_COLOR:
                    return gl.ONE_MINUS_CONSTANT_COLOR;
                case Blend.CONSTANT_ALPHA:
                    return gl.CONSTANT_ALPHA;
                case Blend.ONE_MINUS_CONSTANT_ALPHA:
                    return gl.ONE_MINUS_CONSTANT_ALPHA;
                case Blend.SRC_ALPHA_SATURATE:
                    return gl.SRC_ALPHA_SATURATE;
            }
        }

    }

    public class WebGLContextProgram {

        internal WebGLProgram program;

        internal uint a_position;

        internal WebGLVertexArrayObject vao;

        internal WebGLUniformLocation u_matrix_model;
        internal WebGLUniformLocation u_matrix_transform;
        internal WebGLUniformLocation u_matrix_texture;
        internal WebGLUniformLocation u_texture;
        internal WebGLUniformLocation u_alpha;
        internal WebGLUniformLocation u_vertex_color;
        internal WebGLUniformLocation u_offsetcolor_mul_or_diff;
        internal WebGLUniformLocation u_offsetcolor_enabled;
        internal WebGLUniformLocation u_offsetcolor;
        internal WebGLUniformLocation u_darken;

        internal bool darken_enabled;


        internal WebGLContextProgram(WebGL2RenderingContext gl, WebGLProgram program) {
            this.program = program;

            // vertex array object
            this.vao = gl.createVertexArray();

            // look up where the vertex data needs to go.
            this.a_position = gl.getAttribLocation(program, "a_position");

            // lookup uniforms
            this.u_matrix_model = gl.getUniformLocation(program, "u_matrix_model");
            this.u_matrix_transform = gl.getUniformLocation(program, "u_matrix_transform");
            this.u_matrix_texture = gl.getUniformLocation(program, "u_matrix_texture");
            this.u_texture = gl.getUniformLocation(program, "u_texture");
            this.u_alpha = gl.getUniformLocation(program, "u_alpha");
            this.u_offsetcolor_mul_or_diff = gl.getUniformLocation(program, "u_offsetcolor_mul_or_diff");
            this.u_offsetcolor_enabled = gl.getUniformLocation(program, "u_offsetcolor_enabled");
            this.u_offsetcolor = gl.getUniformLocation(program, "u_offsetcolor");
            this.u_vertex_color = gl.getUniformLocation(program, "u_vertex_color");
            this.u_darken = gl.getUniformLocation(program, "u_darken");
        }

        internal void ResizeProyection(WebGL2RenderingContext gl, float[] proyection_matrix) {
            WebGLUniformLocation u_matrix_proyection = gl.getUniformLocation(this.program, "u_matrix_proyection");
            gl.useProgram(this.program);
            gl.uniformMatrix4fv(u_matrix_proyection, false, proyection_matrix);
        }
    }

    public class WebGLContextProgramGlyphs {

        internal WebGLProgram program;

        internal uint a_position;
        internal uint a_source_coords;
        internal uint a_draw_coords;
        internal uint a_color_alt;
        internal uint a_texture_alt;

        internal WebGLVertexArrayObject vao;


        internal WebGLUniformLocation u_color;
        internal WebGLUniformLocation u_color_by_diff;
        internal WebGLUniformLocation u_grayscale;
        internal WebGLUniformLocation u_matrix_transform;
        internal WebGLUniformLocation u_offsetcolor_enabled;
        internal WebGLUniformLocation u_offsetcolor;
        internal WebGLUniformLocation u_offsetcolor_mul_or_diff;
        internal WebGLUniformLocation u_texture0;
        internal WebGLUniformLocation u_texture1;
        internal WebGLUniformLocation u_color_outline;

#if SDF_FONT
        internal WebGLUniformLocation u_sdf_width;
        internal WebGLUniformLocation u_sdf_edge;
#endif

        internal WebGLBuffer buffer_indices;
        internal WebGLBuffer buffer_vertex;
        internal WebGLBuffer buffer_context;

        internal WebGLContextProgramGlyphs(WebGL2RenderingContext gl, WebGLProgram program) {
            this.program = program;

            // vertex array object
            this.vao = gl.createVertexArray();

            // look up where the vertex data needs to go. In OpenGLES/WebGL gl.getAttribLocation() is used instead
            this.a_position = gl.getAttribLocation(program, "a_position");
            this.a_source_coords = gl.getAttribLocation(program, "a_source_coords");
            this.a_draw_coords = gl.getAttribLocation(program, "a_draw_coords");
            this.a_texture_alt = gl.getAttribLocation(program, "a_texture_alt");
            this.a_color_alt = gl.getAttribLocation(program, "a_color_alt");

            // lookup uniforms
            this.u_color = gl.getUniformLocation(program, "u_color");
            this.u_color_outline = gl.getUniformLocation(program, "u_color_outline");
            this.u_color_by_diff = gl.getUniformLocation(program, "u_color_by_diff");
            this.u_grayscale = gl.getUniformLocation(program, "u_grayscale");
            this.u_matrix_transform = gl.getUniformLocation(program, "u_matrix_transform");
            this.u_offsetcolor_enabled = gl.getUniformLocation(program, "u_offsetcolor_enabled");
            this.u_offsetcolor = gl.getUniformLocation(program, "u_offsetcolor");
            this.u_offsetcolor_mul_or_diff = gl.getUniformLocation(program, "u_offsetcolor_mul_or_diff");
            this.u_texture0 = gl.getUniformLocation(program, "u_texture0");
            this.u_texture1 = gl.getUniformLocation(program, "u_texture1");

#if SDF_FONT
            // sdf specific uniforms
            this.u_sdf_width = gl.getUniformLocation(program, "u_sdf_width");
            this.u_sdf_edge = gl.getUniformLocation(program, "u_sdf_edge");
#endif

            // glyphs buffer
            this.buffer_indices = gl.createBuffer();
            this.buffer_vertex = gl.createBuffer();
            this.buffer_context = gl.createBuffer();
        }

        internal void ResizeProyection(WebGL2RenderingContext gl, float[] proyection_matrix) {
            WebGLUniformLocation u_matrix_proyection = gl.getUniformLocation(this.program, "u_matrix_proyection");
            gl.useProgram(this.program);
            gl.uniformMatrix4fv(u_matrix_proyection, false, proyection_matrix);
        }
    }

    public class PSShader {

        private WebGLProgram program;
        private WebGLBuffer buffer;
        private WebGLVertexArrayObject vao;

        private uint a_position;
        private uint a_texcoord;

        private WebGLUniformLocation u_kdy_time;
        private WebGLUniformLocation u_kdy_delta;
        private WebGLUniformLocation u_kdy_frame;
        private WebGLUniformLocation u_kdy_screen;
        private WebGLUniformLocation u_kdy_texsize;

        private PVRContext pvrctx;

        private static float[] buffer_floats = new float[16];

        public PSShader(PVRContext pvrctx, WebGLProgram program) {
            var gl = pvrctx.webopengl.gl;

            this.program = program;
            this.pvrctx = pvrctx;

            // vertex array object
            this.vao = gl.createVertexArray();

            // look up where the vertex data needs to go.
            this.a_position = gl.getAttribLocation(program, "a_position");
            this.a_texcoord = gl.getAttribLocation(program, "a_texcoord");

            // lookup uniforms
            this.u_kdy_time = gl.getUniformLocation(program, "u_kdy_time");
            this.u_kdy_delta = gl.getUniformLocation(program, "u_kdy_delta");
            this.u_kdy_frame = gl.getUniformLocation(program, "u_kdy_frame");
            this.u_kdy_screen = gl.getUniformLocation(program, "u_kdy_screen");
            this.u_kdy_texsize = gl.getUniformLocation(program, "u_kdy_texsize");

            // glyphs buffer
            this.buffer = gl.createBuffer();

            // fill the buffer
            gl.bindBuffer(gl.ARRAY_BUFFER, this.buffer);
            gl.bufferData(gl.ARRAY_BUFFER, WebGLContext.QUAD_SCREEN, gl.STATIC_DRAW);
            gl.bindBuffer(gl.ARRAY_BUFFER, WebGLBuffer.Null);
        }

        public int SetUniformAny(string name, double[] values) {
            var gl = this.pvrctx.webopengl.gl;

            GLenum type = 0;

            int count = gl.getProgramParameter(this.program, gl.ACTIVE_UNIFORMS);
            WebGLUniformLocation location = gl.getUniformLocation(this.program, name);

            if (location.IsNull) return 0;

            for (uint i = 0 ; i < count ; i++) {
                WebGLActiveInfo info = gl.getActiveUniform(this.program, i);
                if (info.name == name) {
                    type = info.type;
                    break;
                }
            }

            // ignore any unchecked error
            gl.getError();

            var buffer = PSShader.buffer_floats;
            switch (type) {
                case GLenum.GL_FLOAT_MAT2:
                case GLenum.GL_FLOAT_MAT3:
                case GLenum.GL_FLOAT_MAT4:
                case GLenum.GL_FLOAT_MAT2x3:
                case GLenum.GL_FLOAT_MAT2x4:
                case GLenum.GL_FLOAT_MAT3x2:
                case GLenum.GL_FLOAT_MAT3x4:
                case GLenum.GL_FLOAT_MAT4x2:
                case GLenum.GL_FLOAT_MAT4x3:
                    for (int i = 0 ; i < buffer.Length ; i++) {
                        if (i < values.Length)
                            buffer[i] = (float)values[i];
                        else
                            buffer[i] = 0.0f;
                    }
                    break;
            }

            switch (type) {
                case GLenum.GL_FLOAT:
                    gl.uniform1f(location, (float)values[0]);
                    break;
                case GLenum.GL_FLOAT_VEC2:
                    gl.uniform2f(location, (float)values[0], (float)values[1]);
                    break;
                case GLenum.GL_FLOAT_VEC3:
                    gl.uniform3f(location, (float)values[0], (float)values[1], (float)values[2]);
                    break;
                case GLenum.GL_FLOAT_VEC4:
                    gl.uniform4f(location, (float)values[0], (float)values[1], (float)values[2], (float)values[3]);
                    break;
                case GLenum.GL_INT:
                    gl.uniform1i(location, (int)values[0]);
                    break;
                case GLenum.GL_INT_VEC2:
                    gl.uniform2i(location, (int)values[0], (int)values[1]);
                    break;
                case GLenum.GL_INT_VEC3:
                    gl.uniform3i(location, (int)values[0], (int)values[1], (int)values[2]);
                    break;
                case GLenum.GL_INT_VEC4:
                    gl.uniform4i(location, (int)values[0], (int)values[1], (int)values[2], (int)values[3]);
                    break;
                case GLenum.GL_BOOL:
                    gl.uniform1i(location, (int)values[0]);
                    break;
                case GLenum.GL_BOOL_VEC2:
                    gl.uniform2i(location, (int)values[0], (int)values[1]);
                    break;
                case GLenum.GL_BOOL_VEC3:
                    gl.uniform3i(location, (int)values[0], (int)values[1], (int)values[2]);
                    break;
                case GLenum.GL_BOOL_VEC4:
                    gl.uniform4i(location, (int)values[0], (int)values[1], (int)values[2], (int)values[3]);
                    break;
                case GLenum.GL_FLOAT_MAT2:
                    gl.uniformMatrix2fv(location, false, buffer);
                    break;
                case GLenum.GL_FLOAT_MAT3:
                    gl.uniformMatrix3fv(location, false, buffer);
                    break;
                case GLenum.GL_FLOAT_MAT4:
                    gl.uniformMatrix4fv(location, false, buffer);
                    break;
                case GLenum.GL_UNSIGNED_INT:
                    gl.uniform1ui(location, (uint)values[0]);
                    break;
                case GLenum.GL_UNSIGNED_INT_VEC2:
                    gl.uniform2ui(location, (uint)values[0], (uint)values[1]);
                    break;
                case GLenum.GL_UNSIGNED_INT_VEC3:
                    gl.uniform3ui(location, (uint)values[0], (uint)values[1], (uint)values[2]);
                    break;
                case GLenum.GL_UNSIGNED_INT_VEC4:
                    gl.uniform4ui(location, (uint)values[0], (uint)values[1], (uint)values[2], (uint)values[3]);
                    break;
                case GLenum.GL_FLOAT_MAT2x3:
                    gl.uniformMatrix3fv(location, false, buffer);
                    break;
                case GLenum.GL_FLOAT_MAT2x4:
                    gl.uniformMatrix3fv(location, false, buffer);
                    break;
                case GLenum.GL_FLOAT_MAT3x2:
                    gl.uniformMatrix3fv(location, false, buffer);
                    break;
                case GLenum.GL_FLOAT_MAT3x4:
                    gl.uniformMatrix4fv(location, false, buffer);
                    break;
                case GLenum.GL_FLOAT_MAT4x2:
                    gl.uniformMatrix3fv(location, false, buffer);
                    break;
                case GLenum.GL_FLOAT_MAT4x3:
                    gl.uniformMatrix4fv(location, false, buffer);
                    break;
                default:
                    return -1;
            }

            GLenum error = gl.getError();

            return error != gl.NONE ? 1 : -2;
        }

        public bool SetUniform1I(string name, int value) {
            var gl = this.pvrctx.webopengl.gl;

            WebGLUniformLocation location = gl.getUniformLocation(this.program, name);
            if (location.IsNull) return false;

            // forget last error
            gl.getError();

            gl.uniform1i(location, value);
            return gl.getError() == gl.NONE ? true : false;
        }

        public bool SetUniform1F(string name, float value) {
            var gl = this.pvrctx.webopengl.gl;

            WebGLUniformLocation location = gl.getUniformLocation(this.program, name);
            if (location.IsNull) return false;

            // forget last error
            gl.getError();

            gl.uniform1f(location, value);
            return gl.getError() == gl.NONE ? true : false;
        }

        public void Draw(PSFramebuffer from_framebuffer) {
            var gl = this.pvrctx.webopengl.gl;

            gl.useProgram(this.program);
            gl.bindVertexArray(this.vao);

            // bind texture and update antialiasing
            gl.bindTexture(gl.TEXTURE_2D, from_framebuffer.texture);
            //webopengl_change_texture_filtering(this.pvrctx);

            // bind buffer
            gl.bindBuffer(gl.ARRAY_BUFFER, this.buffer);
            gl.enableVertexAttribArray(this.a_position);
            gl.vertexAttribPointer(this.a_position, 2, gl.FLOAT, false, 16, 0);
            gl.enableVertexAttribArray(this.a_texcoord);
            gl.vertexAttribPointer(this.a_texcoord, 2, gl.FLOAT, false, 16, 8);

            // update helper uniforms
            gl.uniform1f(this.u_kdy_time, (float)Glfw.GetTime());
            gl.uniform1f(this.u_kdy_delta, this.pvrctx.last_elapsed);
            gl.uniform1i(this.u_kdy_frame, this.pvrctx.frame_rendered);
            gl.uniform3fv(this.u_kdy_screen, from_framebuffer.screen_dimmens);
            gl.uniform3fv(this.u_kdy_texsize, from_framebuffer.texture_dimmens);

            gl.drawArrays(gl.TRIANGLES, 0, 6);

            gl.bindBuffer(gl.ARRAY_BUFFER, WebGLBuffer.Null);
            gl.bindTexture(gl.TEXTURE_2D, WebGLTexture.Null);
            gl.bindVertexArray(WebGLVertexArrayObject.Null);
            //gl.useProgram(WebGLProgram.NULL);
        }

        public void Destroy() {
            var gl = this.pvrctx.webopengl.gl;

            gl.deleteProgram(this.program);
            gl.deleteBuffer(this.buffer);
            gl.deleteVertexArray(this.vao);

            this.pvrctx = null;
            this.buffer = WebGLBuffer.Null;
            this.program = WebGLProgram.Null;
        }

        public static PSShader BuildFromSource(PVRContext pvrctx, string vertex_shader_sourcecode, string fragment_shader_sourcecode) {
            var wglc = pvrctx.webopengl;
            var gl = wglc.gl;

            WebGLShader vertex_shader = wglc.stock_shadervertex, fragment_shader = wglc.stock_shaderfragment;

            //if (!vertex_shader_sourcecode && !fragment_shader_sourcecode) return null;

            if (!String.IsNullOrEmpty(vertex_shader_sourcecode)) {
                vertex_shader_sourcecode = WebGLContext.InternalPatchShader(vertex_shader_sourcecode, true);
                vertex_shader = wglc.InternalCreateShader(gl, vertex_shader_sourcecode, true, false);
                if (vertex_shader.IsNull) return null;
            }

            if (!String.IsNullOrEmpty(fragment_shader_sourcecode)) {
                fragment_shader_sourcecode = WebGLContext.InternalPatchShader(fragment_shader_sourcecode, false);
                fragment_shader = wglc.InternalCreateShader(gl, fragment_shader_sourcecode, false, false);
                if (fragment_shader.IsNull) return null;
            }

            if (vertex_shader.IsNull || fragment_shader.IsNull) return null;

            WebGLProgram program = gl.createProgram();
            gl.attachShader(program, vertex_shader);
            gl.attachShader(program, fragment_shader);
            gl.linkProgram(program);

            GLenum status = (GLenum)gl.getProgramParameter(program, gl.LINK_STATUS);
            if (status == GLenum.GL_TRUE) return new PSShader(pvrctx, program);

            // something went wrong with the link
            string msg = gl.getProgramInfoLog(program);
            Console.Error.WriteLine($"[ERROR] WebGLContextProgramPostprocessing::BuildProgram() failed: {msg}");
            gl.deleteProgram(program);

            return null;
        }

        public static PSShader BuildFromFiles(PVRContext pvrctx, string vertex_shader_path, string fragment_shader_path) {
            string vertex_shader_sourcecode = null, fragment_shader_sourcecode = null;

            if (!String.IsNullOrEmpty(vertex_shader_path)) vertex_shader_sourcecode = FS.ReadText(vertex_shader_path);
            if (!String.IsNullOrEmpty(fragment_shader_path)) fragment_shader_sourcecode = FS.ReadText(fragment_shader_path);

            return PSShader.BuildFromSource(pvrctx, vertex_shader_sourcecode, fragment_shader_sourcecode);
        }

        internal static PSShader Init(string vertex_sourcecode, string fragment_sourcecode) {
            return BuildFromSource(PVRContext.global_context, vertex_sourcecode, fragment_sourcecode);
        }

    }

    public class PSFramebuffer {

        internal int pow2_width;
        internal int pow2_height;
        internal float[] texture_dimmens;
        internal float[] screen_dimmens;

        private WebGLFramebuffer framebuffer;
        internal WebGLTexture texture;
        //private WebGLRenderbuffer renderbuffer;

        private PVRContext pvrctx;


        public PSFramebuffer(PVRContext pvrctx) {
            var gl = pvrctx.webopengl.gl;

            this.pvrctx = pvrctx;
            this.pow2_width = Math2D.PowerOfTwoCalc(pvrctx.ScreenWidth);
            this.pow2_height = Math2D.PowerOfTwoCalc(pvrctx.ScreenHeight);

            // width, height, aspect-ratio
            this.screen_dimmens = new float[] {
            pvrctx.ScreenWidth, pvrctx.ScreenHeight, (float)pvrctx.ScreenWidth / (float)pvrctx.ScreenHeight
        };
            // width, height, aspect-ratio
            this.texture_dimmens = new float[] {
            this.pow2_width, this.pow2_height, (float)this.pow2_width / (float)this.pow2_height
        };

            // framebuffer configuration
            this.framebuffer = gl.createFramebuffer();
            gl.bindFramebuffer(gl.FRAMEBUFFER, this.framebuffer);

            this.texture = gl.createTexture();
            gl.bindTexture(gl.TEXTURE_2D, this.texture);
            gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, this.pow2_width, this.pow2_height, 0, gl.RGBA, gl.UNSIGNED_BYTE, IntPtr.Zero);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
            gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, this.texture, 0);

            // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
            //this.renderbuffer = gl.createRenderbuffer();
            //gl.bindRenderbuffer(gl.RENDERBUFFER, this.renderbuffer);
            //gl.renderbufferStorage(gl.RENDERBUFFER, gl.STENCIL_INDEX8, this.pow2_width, this.pow2_height);
            //gl.framebufferRenderbuffer(gl.FRAMEBUFFER, gl.STENCIL_ATTACHMENT, gl.RENDERBUFFER, this.renderbuffer);

            GLenum status = gl.checkFramebufferStatus(gl.FRAMEBUFFER);
            if (status != gl.FRAMEBUFFER_COMPLETE) {
                Console.Error.WriteLine("framebuffer is not complete, status was 0x" + ((int)status).ToString("X"));
            }

            gl.bindFramebuffer(gl.FRAMEBUFFER, WebGLFramebuffer.Null);
            gl.bindTexture(gl.TEXTURE_2D, WebGLTexture.Null);
            //gl.bindRenderbuffer(gl.RENDERBUFFER, WebGLRenderbuffer.Null);
        }

        public void Resize() {
            var gl = this.pvrctx.webopengl.gl;

            // update internal dimensions
            this.pow2_width = Math2D.PowerOfTwoCalc(this.pvrctx.ScreenWidth);
            this.pow2_height = Math2D.PowerOfTwoCalc(this.pvrctx.ScreenHeight);

            // width, height, aspect-ratio
            this.screen_dimmens[0] = this.pvrctx.ScreenWidth;
            this.screen_dimmens[1] = this.pvrctx.ScreenHeight;
            this.screen_dimmens[2] = (float)pvrctx.ScreenWidth / (float)this.pvrctx.ScreenHeight;

            // width, height, aspect-ratio
            this.texture_dimmens[0] = this.pow2_width;
            this.texture_dimmens[1] = this.pow2_height;
            this.texture_dimmens[2] = (float)this.pow2_width / (float)this.pow2_height;

            // resize color attachment
            gl.bindTexture(gl.TEXTURE_2D, this.texture);
            gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, this.pow2_width, this.pow2_height, 0, gl.RGBA, gl.UNSIGNED_BYTE, IntPtr.Zero);

            // resize stencil attachment
            //gl.bindRenderbuffer(gl.RENDERBUFFER, this.renderbuffer);
            //gl.renderbufferStorage(gl.RENDERBUFFER, gl.STENCIL_INDEX8, this.pow2_width, this.pow2_height);

            gl.bindTexture(gl.TEXTURE_2D, WebGLTexture.Null);
            //gl.bindRenderbuffer(gl.RENDERBUFFER, null);
        }

        public void Use(bool clear_first) {
            var gl = this.pvrctx.webopengl.gl;

            gl.bindFramebuffer(gl.FRAMEBUFFER, this.framebuffer);

            // clear contents
            if (clear_first) {
                gl.clearColor(0.0f, 0.0f, 0.0f, 0.0f);
                gl.clear(gl.COLOR_BUFFER_BIT);
            }
        }

        public void Invalidate() {
            var gl = this.pvrctx.webopengl.gl;

            int temp = gl.getParameter(gl.FRAMEBUFFER_BINDING);

            gl.bindFramebuffer(gl.FRAMEBUFFER, this.framebuffer);
            gl.invalidateFramebuffer(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0);

            gl.bindFramebuffer(gl.FRAMEBUFFER, temp);
        }

        public void Destroy() {
            var gl = this.pvrctx.webopengl.gl;

            gl.deleteFramebuffer(this.framebuffer);
            gl.deleteTexture(this.texture);
            //gl.deleteRenderbuffer(this.renderbuffer);

            this.framebuffer = WebGLFramebuffer.Null;
            this.texture = WebGLTexture.Null;
            //this.renderbuffer = WebGLRenderbuffer.NUll;

            //if (this.pvrctx.target_framebuffer == this) this.pvrctx.target_framebuffer = null;
        }

        public static void UseScreenFramebuffer(PVRContext pvrctx) {
            var gl = pvrctx.webopengl.gl;

            gl.bindFramebuffer(gl.FRAMEBUFFER, WebGLFramebuffer.Null);
        }

        public static void ResizeQuadScreen(PVRContext pvrctx) {
            int width = pvrctx.ScreenWidth;
            int height = pvrctx.ScreenHeight;
            int pow2_width = Math2D.PowerOfTwoCalc(width);
            int pow2_height = Math2D.PowerOfTwoCalc(height);
            float u = (float)width / pow2_width;
            float v = (float)height / pow2_height;

            WebGLContext.QUAD_SCREEN[20 + 2] = WebGLContext.QUAD_SCREEN[16 + 2] = WebGLContext.QUAD_SCREEN[8 + 2] = u;
            WebGLContext.QUAD_SCREEN[20 + 3] = WebGLContext.QUAD_SCREEN[12 + 3] = WebGLContext.QUAD_SCREEN[0 + 3] = v;
        }
    }

}

