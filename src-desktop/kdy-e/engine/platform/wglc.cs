using System;
using Engine.Image;
using Engine.Externals;
using System.Diagnostics;
using static Engine.Externals.WebGLRenderingContext.NativeMethods;
using System.Runtime.InteropServices;

namespace Engine.Platform {

    public class WebGLContext {

        public static readonly float[] WEBGL_MAT4 = new float[4 * 4];
        public static readonly float[] WEBGL_RGBA = new float[2 * 2];
        public static readonly float[] WEBGL_QUAD = new float[] { 0f, 0f, 0f, 1f, 1f, 0f, 1f, 0f, 0f, 1f, 1f, 1f };

        public WebGLContextProgram program_solid;
        public WebGLContextProgram program_textured;
        public WebGLContextProgramGlyphs program_glyphs;

        public WebGLRenderingContext gl;

        public WebGLBuffer position_buffer;

        public ANGLE_instanced_arrays ANGLE_instanced_arrays;

        internal WebGLContext() {
            this.gl = new WebGLRenderingContext();
            Console.Error.WriteLine("OpenGL: " + this.gl.getString(GLenum.GL_VERSION));

            // setup GLSL programs
            WebGLProgram program_solid = InternalCreateProgram(gl, "solid");
            WebGLProgram program_textured = InternalCreateProgram(gl, "textured");
            WebGLProgram program_glyphs = InternalCreateProgram(gl, "glyphs");

            this.program_solid = new WebGLContextProgram(gl, program_solid);
            this.program_textured = new WebGLContextProgram(gl, program_textured);
            this.program_glyphs = new WebGLContextProgramGlyphs(gl, program_glyphs);

            // quads buffer
            this.position_buffer = gl.createBuffer();
            gl.bindBuffer(gl.ARRAY_BUFFER, this.position_buffer);

            // Put a unit quad in the buffer
            gl.bufferData(gl.ARRAY_BUFFER, WEBGL_QUAD, gl.STATIC_DRAW);

            gl.disable(gl.CULL_FACE);
            gl.disable(gl.DEPTH_TEST);
            gl.depthMask(false);

            //gl.blendFuncSeparate(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA, gl.ONE, gl.ONE_MINUS_SRC_ALPHA);
            gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
            gl.enable(gl.BLEND);

            gl.pixelStorei(gl.UNPACK_PREMULTIPLY_ALPHA_WEBGL, 0);

            IWebGLExtension index_uint = gl.getExtension("OES_element_index_uint");
            Debug.Assert(index_uint != null, "missing OES_element_index_uint which is required for text rendering");

            IWebGLExtension instanced_arrays = gl.getExtension("ANGLE_instanced_arrays");
            this.ANGLE_instanced_arrays = (ANGLE_instanced_arrays)instanced_arrays;
            Debug.Assert(instanced_arrays != null, "missing ANGLE_instanced_arrays which is required for text rendering");

            // convert from pixels to clip space
            //ResizeProjection(pvrctx);
        }

        public WebGLTexture CreateTexture(int pow2_tex_width, int pow2_tex_height, ImageData bitmap_data) {
            WebGLRenderingContext gl = this.gl;
            
            // clear any previous error
            gl.getError();

            WebGLTexture tex = gl.createTexture();

            gl.activeTexture(gl.TEXTURE0);
            gl.bindTexture(gl.TEXTURE_2D, tex);

            gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, bitmap_data);

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
            WebGLRenderingContext gl = pvrctx.webopengl.gl;

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
            InternalIdentity(WEBGL_MAT4, 4);
            InternalOrthographic2D(WEBGL_MAT4, width, height);
            this.gl.viewport(offset_x, offset_y, width, height);
            this.program_solid.ResizeProyection(this.gl, WEBGL_MAT4);
            this.program_textured.ResizeProyection(this.gl, WEBGL_MAT4);
            this.program_glyphs.ResizeProyection(this.gl, WEBGL_MAT4);
        }


        public void DrawTexture(PVRContext pvrctx, Texture tex, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh) {
            WebGLRenderingContext gl = this.gl;
            int tex_width = tex.width;
            int tex_height = tex.height;

            gl.useProgram(this.program_textured.program);
            gl.bindVertexArray(this.program_textured.vao);

            gl.bindTexture(gl.TEXTURE_2D, tex.data_vram);

            // update antialiasing
            ChangeTextureFiltering(pvrctx);

            gl.bindBuffer(gl.ARRAY_BUFFER, this.position_buffer);
            gl.enableVertexAttribArray(this.program_textured.a_position);
            gl.vertexAttribPointer(this.program_textured.a_position, 2, gl.FLOAT, false, 0, 0);

            // optimized matrix model, contains the draw size and location
            InternalIdentity(WEBGL_MAT4, 4);
            WEBGL_MAT4[0] = dw;
            WEBGL_MAT4[5] = dh;
            WEBGL_MAT4[12] = dx;
            WEBGL_MAT4[13] = dy;
            gl.uniformMatrix4fv(this.program_textured.u_matrix_model, false, WEBGL_MAT4);

            // copy transformation matrix (with all modifiers applied)
            gl.uniformMatrix4fv(this.program_textured.u_matrix_transform, false, pvrctx.CurrentMatrix.matrix);

            // optimized sub-texture matrix, contains the source size and location
            InternalIdentity(WEBGL_MAT4, 4);
            WEBGL_MAT4[12] = sx / tex_width;
            WEBGL_MAT4[13] = sy / tex_height;
            if (sw != tex_width) WEBGL_MAT4[0] = sw / tex_width;
            if (sh != tex_height) WEBGL_MAT4[5] = sh / tex_height;
            gl.uniformMatrix4fv(this.program_textured.u_matrix_texture, false, WEBGL_MAT4);

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

            // draw the quad (2 triangles, 6 vertices)
            gl.drawArrays(gl.TRIANGLES, 0, 6);

            // unbind buffers and texture
            gl.bindBuffer(gl.ARRAY_BUFFER, WebGLBuffer.Null);
            gl.bindTexture(gl.TEXTURE_2D, WebGLTexture.Null);
            gl.bindVertexArray(0);

            // required to avoid GL_OUT_OF_MEMORY
            gl.flush();
        }

        public void DrawSolid(PVRContext pvrctx, float[] rgb_color, float dx, float dy, float dw, float dh) {
            WebGLRenderingContext gl = this.gl;

            gl.useProgram(this.program_solid.program);
            gl.bindVertexArray(this.program_solid.vao);

            gl.bindBuffer(gl.ARRAY_BUFFER, this.position_buffer);
            gl.enableVertexAttribArray(this.program_solid.a_position);
            gl.vertexAttribPointer(this.program_solid.a_position, 2, gl.FLOAT, false, 0, 0);

            // optimized matrix model, contains the draw size and location
            InternalIdentity(WEBGL_MAT4, 4);
            WEBGL_MAT4[0] = dw;
            WEBGL_MAT4[5] = dh;
            WEBGL_MAT4[12] = dx;
            WEBGL_MAT4[13] = dy;
            gl.uniformMatrix4fv(this.program_solid.u_matrix_model, false, WEBGL_MAT4);

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
            WEBGL_RGBA[0] = rgb_color[0];
            WEBGL_RGBA[1] = rgb_color[1];
            WEBGL_RGBA[2] = rgb_color[2];
            WEBGL_RGBA[3] = pvrctx.render_alpha;
            gl.uniform4fv(this.program_solid.u_vetex_color, WEBGL_RGBA);

            // draw the quad (2 triangles, 6 vertices)
            gl.drawArrays(gl.TRIANGLES, 0, 6);

            // unbind buffers
            gl.bindBuffer(gl.ARRAY_BUFFER, WebGLBuffer.Null);
            gl.bindVertexArray(0);
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

        private WebGLShader InternalCreateShader(WebGLRenderingContext gl, string source, bool is_vertex_shader) {
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
            Console.Error.WriteLine("webopengl_internal_create_shader() failed to create a " + type + "shader");
            gl.deleteShader(shader);

            throw new Exception(type + " shader compilation failed\r\n" + msg);
        }

        private WebGLProgram InternalCreateProgram(WebGLRenderingContext gl, string shader_name) {
            string vertex_shader_sourcecode = InternalLoadShader(shader_name, true);
            string fragment_shader_sourcecode = InternalLoadShader(shader_name, false);

            if (vertex_shader_sourcecode == null) throw new Exception("can not load vertex shader for " + shader_name);
            if (fragment_shader_sourcecode == null) throw new Exception("can not load fragment for " + shader_name);

            WebGLShader vertex_shader = InternalCreateShader(gl, vertex_shader_sourcecode, true);
            WebGLShader fragment_shader = InternalCreateShader(gl, fragment_shader_sourcecode, false);

            WebGLProgram program = gl.createProgram();
            gl.attachShader(program, vertex_shader);
            gl.attachShader(program, fragment_shader);

            gl.linkProgram(program);

            GLenum status = gl.getProgramParameter(program, gl.LINK_STATUS);
            string infolog = gl.getProgramInfoLog(program);
            if (!String.IsNullOrEmpty(infolog)) Console.Error.WriteLine(infolog + "\n");
            if (status == gl.TRUE) return program;

            // something went wrong with the link
            Console.Error.WriteLine("InternalCreateProgram() failed");
            gl.deleteProgram(program);
            throw new Exception("Shaders compilation failed\n");
        }

        private void InternalIdentity(float[] matrix, int stride) {
            for (int i = 0 ; i < matrix.Length ; i++) matrix[i] = 0f;

            if (stride == 3)
                matrix[0] = matrix[4] = matrix[8] = 1f;
            else if (stride == 4)
                matrix[0] = matrix[5] = matrix[10] = matrix[15] = 1f;

        }

        private static string InternalLoadShader(string shader_prefix, bool vertexshader_or_fragmentshader) {
            string shader_suffix = vertexshader_or_fragmentshader ? "vertexshader" : "fragmentshader";
            string path = "/assets/shaders/" + shader_prefix + "_" + shader_suffix + ".glsl";
            return IO.ReadText(path);
        }

    }

    public class WebGLContextProgram {

        internal WebGLProgram program;

        internal uint a_position = 0;
        internal uint vao;

        internal WebGLUniformLocation u_matrix_model;
        internal WebGLUniformLocation u_matrix_transform;
        internal WebGLUniformLocation u_matrix_texture;
        internal WebGLUniformLocation u_texture;
        internal WebGLUniformLocation u_alpha;
        internal WebGLUniformLocation u_vetex_color;
        internal WebGLUniformLocation u_offsetcolor_mul_or_diff;
        internal WebGLUniformLocation u_offsetcolor_enabled;
        internal WebGLUniformLocation u_offsetcolor;


        internal WebGLContextProgram(WebGLRenderingContext gl, WebGLProgram program) {
            this.program = program;

            // vertex array object (not used in OpenGLES/WebGL)
            this.vao = gl.genVertexArray();

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
            this.u_vetex_color = gl.getUniformLocation(program, "u_vetex_color");
        }

        internal void ResizeProyection(WebGLRenderingContext gl, float[] proyection_matrix) {
            WebGLUniformLocation u_matrix_proyection = gl.getUniformLocation(this.program, "u_matrix_proyection");
            gl.useProgram(this.program);
            gl.uniformMatrix4fv(u_matrix_proyection, false, proyection_matrix);
        }
    }

    public class WebGLContextProgramGlyphs {

        internal WebGLProgram program;

        internal uint a_position = 0;
        internal uint a_source_coords = 0;
        internal uint a_draw_coords = 0;
        internal uint a_texture_id = 0;
        internal uint a_color_alt = 0;
        internal uint a_texture_alt = 0;
        internal uint vao;


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

        internal WebGLContextProgramGlyphs(WebGLRenderingContext gl, WebGLProgram program) {
            this.program = program;

            // vertex array object (not used in OpenGLES/WebGL)
            this.vao = gl.genVertexArray();

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

        internal void ResizeProyection(WebGLRenderingContext gl, float[] proyection_matrix) {
            WebGLUniformLocation u_matrix_proyection = gl.getUniformLocation(this.program, "u_matrix_proyection");
            gl.useProgram(this.program);
            gl.uniformMatrix4fv(u_matrix_proyection, false, proyection_matrix);
        }
    }

}