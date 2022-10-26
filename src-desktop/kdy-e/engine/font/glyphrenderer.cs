using System;
using System.Diagnostics;
using Engine.Externals;
using Engine.Image;
using Engine.Platform;

namespace Engine.Font {

    public static class GlyphRenderer {

        private const int MAX_GLYPHS = 131072;// ~5MiB in buffers
        private const int FLOATS_PER_VERTEX = 8;// GlyphRenderer.QUAD.length
        private const int FLOATS_PER_INDEX = 6;// GlyphRenderer.GLYPH_INDICES.length
        private const int FLOATS_PER_CONTEXT = /*Dxywh*/4 + /*Sxywh*/4 + /*t*/1 + /*ac*/1;
        private const int BYTES_PER_VERTEX = GlyphRenderer.FLOATS_PER_VERTEX * /*sizeof(float)*/4;
        private const int BYTES_PER_INDEX = GlyphRenderer.FLOATS_PER_INDEX * /*sizeof(float)*/4;
        private const int BYTES_PER_CONTEXT = GlyphRenderer.FLOATS_PER_CONTEXT * /*sizeof(float)*/4;
        private static readonly float[] QUAD = { 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
        private static readonly uint[] QUAD_INDICES = { 0, 1, 2, 2, 1, 3 };

        private static int glyphrenderer_glyphs_total = 128;// by default
        private static float[] glyphrenderer_vertex_array = new float[GlyphRenderer.FLOATS_PER_VERTEX * glyphrenderer_glyphs_total];
        private static uint[] glyphrenderer_indices_array = new uint[GlyphRenderer.FLOATS_PER_INDEX * glyphrenderer_glyphs_total];
        private static float[] glyphrenderer_context_array = new float[GlyphRenderer.FLOATS_PER_CONTEXT * glyphrenderer_glyphs_total];
        private static int glyphrenderer_glyphs_added = 0;
        private static int glyphrenderer_glyphs_index_normals = 0;
        private static int glyphrenderer_glyphs_index_outlines = 0;

        private static bool glyphrenderer_prefill = true;


        public static int Prepare(int glyph_count, bool has_outlines) {
            if (glyph_count > glyphrenderer_glyphs_total) {
                if (glyph_count > GlyphRenderer.MAX_GLYPHS) {
                    // thats alot of text
                    glyph_count = GlyphRenderer.MAX_GLYPHS;
                }

                glyphrenderer_vertex_array = new float[GlyphRenderer.FLOATS_PER_VERTEX * glyph_count];
                glyphrenderer_indices_array = new uint[GlyphRenderer.FLOATS_PER_INDEX * glyph_count];
                glyphrenderer_context_array = new float[GlyphRenderer.FLOATS_PER_CONTEXT * glyph_count];
                glyphrenderer_prefill = true;
            }

            glyphrenderer_glyphs_added = 0;
            glyphrenderer_glyphs_total = glyph_count;

            glyphrenderer_glyphs_index_normals = has_outlines ? (glyph_count / 2) : 0;
            glyphrenderer_glyphs_index_outlines = 0;

            if (glyphrenderer_prefill) {
                // fill indices buffer
                for (uint i = 0, j = 0, k = 0 ; i < glyph_count ; i++) {
                    for (uint l = 0 ; l < GlyphRenderer.QUAD_INDICES.Length ; l++) {
                        glyphrenderer_indices_array[j++] = GlyphRenderer.QUAD_INDICES[l] + k;
                    }
                    k += 4;

                    // FIXME: alpha works if this break is here
                   if (true) break;
                }

                // fill vertex buffer
                for (int i = 0, j = 0 ; i < glyph_count ; i++) {
                    Array.Copy(GlyphRenderer.QUAD, 0, glyphrenderer_vertex_array, j, GlyphRenderer.QUAD.Length);
                    j += GlyphRenderer.QUAD.Length;
                }

                glyphrenderer_prefill = false;
            }

            return glyph_count;
        }

        public static void AppendGlyph(Texture texture, bool is_tex1, bool is_outline, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh) {
            if (glyphrenderer_glyphs_added > glyphrenderer_glyphs_total) return;

            int index;
            if (is_outline)
                index = glyphrenderer_glyphs_index_outlines++;
            else
                index = glyphrenderer_glyphs_index_normals++;

            index *= GlyphRenderer.FLOATS_PER_CONTEXT;

            // just in case
            Debug.Assert(texture != null, "texture expected");
            Debug.Assert(index < glyphrenderer_context_array.Length, "to many glyphs");

            // sub-texture location & size
            glyphrenderer_context_array[index + 0] = sx / texture.width;
            glyphrenderer_context_array[index + 1] = sy / texture.height;
            glyphrenderer_context_array[index + 2] = sw / texture.width;
            glyphrenderer_context_array[index + 3] = sh / texture.height;
            // draw location
            glyphrenderer_context_array[index + 4] = dx;
            glyphrenderer_context_array[index + 5] = dy;
            glyphrenderer_context_array[index + 6] = dw;
            glyphrenderer_context_array[index + 7] = dh;
            // texture id
            glyphrenderer_context_array[index + 8] = is_tex1 ? 1 : 0;
            // color selection (for outlines)
            glyphrenderer_context_array[index + 9] = is_outline ? 1 : 0;

            glyphrenderer_glyphs_added++;
        }

        public static void Draw(PVRContext pvrctx, float[] color, float[] color_outline, bool by_diff, bool is_gryscl, Texture tex0, Texture tex1) {
            WebGLRenderingContext gl = pvrctx.webopengl.gl;
            WebGLContextProgramGlyphs program_glyphs = pvrctx.webopengl.program_glyphs;
            ANGLE_instanced_arrays ANGLE_instanced_arrays = pvrctx.webopengl.ANGLE_instanced_arrays;

            // prepare buffers
            int total_indices = glyphrenderer_glyphs_total * GlyphRenderer.FLOATS_PER_INDEX;
            int vertex_array_length = glyphrenderer_glyphs_total * GlyphRenderer.FLOATS_PER_VERTEX;
            int index_array_length = total_indices;
            int context_array_length = glyphrenderer_glyphs_total * GlyphRenderer.FLOATS_PER_CONTEXT;

            gl.useProgram(program_glyphs.program);
            gl.bindVertexArray(program_glyphs.vao);

            // bind textures
            bool change_filtering;
#if SDF_FONT
            change_filtering = !is_gryscl;
#else
            change_filtering = true;
#endif

            if (tex0 != null) {
                gl.activeTexture(gl.TEXTURE0);
                gl.bindTexture(gl.TEXTURE_2D, tex0.data_vram);
                gl.uniform1i(program_glyphs.u_texture0, 0);

                // update antialiasing
                if (change_filtering) pvrctx.webopengl.ChangeTextureFiltering(pvrctx);
            }
            if (tex1 != null) {
                gl.activeTexture(gl.TEXTURE1);
                gl.bindTexture(gl.TEXTURE_2D, tex1.data_vram);
                gl.uniform1i(program_glyphs.u_texture1, 1);

                // update antialiasing
                if (change_filtering) pvrctx.webopengl.ChangeTextureFiltering(pvrctx);
            }

            // send quad units
            gl.bindBuffer(gl.ARRAY_BUFFER, program_glyphs.buffer_vertex);
            gl.enableVertexAttribArray(program_glyphs.a_position);
            gl.vertexAttribPointer(program_glyphs.a_position, 2, gl.FLOAT, false, 0, 0);
            gl.bufferData(gl.ARRAY_BUFFER, vertex_array_length, glyphrenderer_vertex_array, gl.DYNAMIC_DRAW);

            // send glyph contexts
            gl.bindBuffer(gl.ARRAY_BUFFER, program_glyphs.buffer_context);
            gl.enableVertexAttribArray(program_glyphs.a_source_coords);
            gl.vertexAttribPointer(program_glyphs.a_source_coords, 4, gl.FLOAT, false, GlyphRenderer.BYTES_PER_CONTEXT, 0);
            ANGLE_instanced_arrays.vertexAttribDivisorANGLE(program_glyphs.a_source_coords, 1);
            gl.enableVertexAttribArray(program_glyphs.a_draw_coords);
            gl.vertexAttribPointer(program_glyphs.a_draw_coords, 4, gl.FLOAT, false, GlyphRenderer.BYTES_PER_CONTEXT, 16);
            ANGLE_instanced_arrays.vertexAttribDivisorANGLE(program_glyphs.a_draw_coords, 1);
            gl.enableVertexAttribArray(program_glyphs.a_texture_alt);
            gl.vertexAttribPointer(program_glyphs.a_texture_alt, 1, gl.FLOAT, false, GlyphRenderer.BYTES_PER_CONTEXT, 32);
            ANGLE_instanced_arrays.vertexAttribDivisorANGLE(program_glyphs.a_texture_alt, 1);
            gl.enableVertexAttribArray(program_glyphs.a_color_alt);
            gl.vertexAttribPointer(program_glyphs.a_color_alt, 1, gl.FLOAT, false, GlyphRenderer.BYTES_PER_CONTEXT, 36);
            ANGLE_instanced_arrays.vertexAttribDivisorANGLE(program_glyphs.a_color_alt, 1);
            gl.bufferData(gl.ARRAY_BUFFER, context_array_length, glyphrenderer_context_array, gl.DYNAMIC_DRAW);

            // copy transformation matrix (with all modifiers applied)
            gl.uniformMatrix4fv(program_glyphs.u_matrix_transform, false, pvrctx.CurrentMatrix.matrix);

            // render alpha value and the text color
            WebGLContext.WEBGL_RGBA[0] = color[0];
            WebGLContext.WEBGL_RGBA[1] = color[1];
            WebGLContext.WEBGL_RGBA[2] = color[2];
            WebGLContext.WEBGL_RGBA[3] = pvrctx.render_alpha;
            gl.uniform4fv(program_glyphs.u_color, WebGLContext.WEBGL_RGBA);

            // outline color (if used)
            WebGLContext.WEBGL_RGBA[0] = color_outline[0];
            WebGLContext.WEBGL_RGBA[1] = color_outline[1];
            WebGLContext.WEBGL_RGBA[2] = color_outline[2];
            WebGLContext.WEBGL_RGBA[3] = color_outline[3] * pvrctx.global_alpha;
            gl.uniform4fv(program_glyphs.u_color_outline, WebGLContext.WEBGL_RGBA);

            // if the offsetcolor alpha is negative, disable the offsetcolor processing
            // "u_offsetcolor_enabled" and "u_offsetcolor_mul_or_diff" are boolean values
            if (pvrctx.render_offsetcolor[3] < 0) {
                gl.uniform1i(program_glyphs.u_offsetcolor_enabled, 0);
            } else {
                gl.uniform1i(program_glyphs.u_offsetcolor_enabled, 1);
                gl.uniform4fv(program_glyphs.u_offsetcolor, pvrctx.render_offsetcolor);
                gl.uniform1i(program_glyphs.u_offsetcolor_mul_or_diff, pvrctx.render_offsetcolor_multiply != PVRContextFlag.DISABLE ? 1 : 0);
            }

            // grayscale textures used by fonttype_draw() or rgba textures used by fontglyph_draw()
            gl.uniform1i(program_glyphs.u_grayscale, is_gryscl ? 1 : 0);

            // used only by fontglyph_draw() function
            gl.uniform1i(program_glyphs.u_color_by_diff, by_diff ? 1 : 0);

            // send vertex indices
            gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, program_glyphs.buffer_indices);
            gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, total_indices, glyphrenderer_indices_array, gl.DYNAMIC_DRAW);

            // commit draw
            ANGLE_instanced_arrays.drawElementsInstancedANGLE(
                gl.TRIANGLES, total_indices, gl.UNSIGNED_INT, 0, glyphrenderer_glyphs_total
            );
            gl.flush();


            // unbind textures
            gl.activeTexture(gl.TEXTURE1);
            gl.bindTexture(gl.TEXTURE_2D, WebGLTexture.Null);
            gl.activeTexture(gl.TEXTURE0);
            gl.bindTexture(gl.TEXTURE_2D, WebGLTexture.Null);

            // unbind buffers
            gl.bindBuffer(gl.ARRAY_BUFFER, WebGLBuffer.Null);
            gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, WebGLBuffer.Null);
            gl.bindVertexArray(0);
        }

#if SDF_FONT
        public static void SetSDFParams(PVRContext pvrctx, float width, float edge) {
            WebGLRenderingContext gl = pvrctx.webopengl.gl;
            WebGLContextProgramGlyphs program_glyphs = pvrctx.webopengl.program_glyphs;

            gl.useProgram(program_glyphs.program);
            gl.uniform1f(program_glyphs.u_sdf_width, width);
            gl.uniform1f(program_glyphs.u_sdf_edge, edge);
        }
#endif

    }

}
