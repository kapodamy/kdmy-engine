"use strict";

const GLYPHRENDERDER_MAX_GLYPHS = 131072;// ~5MiB in buffers
const GLYPHRENDERDER_FLOATS_PER_VERTEX = 8;// GLYPHRENDERDER_QUAD.length
const GLYPHRENDERDER_FLOATS_PER_INDEX = 6;// GLYPHRENDERDER_GLYPH_INDICES.length
const GLYPHRENDERDER_FLOATS_PER_CONTEXT = /*Dxywh*/4 + /*Sxywh*/4 + /*t*/1 + /*ac*/1;
const GLYPHRENDERDER_BYTES_PER_CONTEXT = GLYPHRENDERDER_FLOATS_PER_CONTEXT * /*sizeof(float)*/4;
const GLYPHRENDERDER_QUAD = new Float32Array([0, 0, 0, 1, 1, 0, 1, 1]);
const GLYPHRENDERDER_QUAD_INDICES = new Uint32Array([0, 1, 2, 2, 1, 3]);

var glyphrenderer_glyphs_total = 128;// by default
var glyphrenderer_vertex_array = new Float32Array(GLYPHRENDERDER_FLOATS_PER_VERTEX * glyphrenderer_glyphs_total);
var glyphrenderer_indices_array = new Uint32Array(GLYPHRENDERDER_FLOATS_PER_INDEX * glyphrenderer_glyphs_total);
var glyphrenderer_context_array = new Float32Array(GLYPHRENDERDER_FLOATS_PER_CONTEXT * glyphrenderer_glyphs_total);
var glyphrenderer_glyphs_added = 0;
var glyphrenderer_glyphs_index_normals = 0;
var glyphrenderer_glyphs_index_outlines = 0;
var glyphrenderer_prefill = true;


function glyphrenderer_prepare(glyph_count, has_outlines) {
    if (glyph_count > glyphrenderer_glyphs_total) {
        if (glyph_count > GLYPHRENDERDER_MAX_GLYPHS) {
            // thats alot of text
            glyph_count = GLYPHRENDERDER_MAX_GLYPHS;
        }

        glyphrenderer_vertex_array = new Float32Array(GLYPHRENDERDER_FLOATS_PER_VERTEX * glyph_count);
        glyphrenderer_indices_array = new Uint32Array(GLYPHRENDERDER_FLOATS_PER_INDEX * glyph_count);
        glyphrenderer_context_array = new Float32Array(GLYPHRENDERDER_FLOATS_PER_CONTEXT * glyph_count);
        glyphrenderer_prefill = true;
    }

    glyphrenderer_glyphs_added = 0;
    glyphrenderer_glyphs_total = glyph_count;

    glyphrenderer_glyphs_index_normals = has_outlines ? (glyph_count / 2) : 0;
    glyphrenderer_glyphs_index_outlines = 0;

    if (glyphrenderer_prefill) {
        // fill indices buffer
        for (let i = 0, j = 0, k = 0; i < glyph_count; i++) {
            for (let l = 0; l < GLYPHRENDERDER_QUAD_INDICES.length; l++) {
                glyphrenderer_indices_array[j++] = GLYPHRENDERDER_QUAD_INDICES[l] + k;
            }
            k += 4;
            
            // FIXME: alpha works if this break is here
            break;
        }

        // fill vertex buffer
        for (let i = 0, j = 0; i < glyph_count; i++) {
            glyphrenderer_vertex_array.set(GLYPHRENDERDER_QUAD, j);
            j += GLYPHRENDERDER_QUAD.length;
        }

        glyphrenderer_prefill = false;
    }

    return glyph_count;
}

function glyphrenderer_append_glyph(texture, is_tex1, is_outline, sx, sy, sw, sh, dx, dy, dw, dh) {
    if (glyphrenderer_glyphs_added > glyphrenderer_glyphs_total) return;

    let index;
    if (is_outline)
        index = glyphrenderer_glyphs_index_outlines++;
    else
        index = glyphrenderer_glyphs_index_normals++;

    index *= GLYPHRENDERDER_FLOATS_PER_CONTEXT;

    // just in case
    console.assert(texture, "texture expected");
    console.assert(index < glyphrenderer_context_array.length, "to many glyphs");

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

function glyphrenderer_draw(/**@type {PVRContext}*/pvrctx, color, color_outline, by_diff, is_gryscl, tex0, tex1) {
    const gl = pvrctx.webopengl.gl;
    const program_glyphs = pvrctx.webopengl.program_glyphs;
    const ANGLE_instanced_arrays = pvrctx.webopengl.ANGLE_instanced_arrays;

    // prepare buffers
    const total_indices = glyphrenderer_glyphs_total * GLYPHRENDERDER_FLOATS_PER_INDEX;
    const vertex_array = glyphrenderer_vertex_array.subarray(
        0, glyphrenderer_glyphs_total * GLYPHRENDERDER_FLOATS_PER_VERTEX
    );
    const index_array = glyphrenderer_indices_array.subarray(
        0, total_indices
    );
    const context_array = glyphrenderer_context_array.subarray(
        0, glyphrenderer_glyphs_total * GLYPHRENDERDER_FLOATS_PER_CONTEXT
    );

    gl.useProgram(program_glyphs.program);

    // bind textures
    let change_filtering;
    if (SDF_FONT) {
        change_filtering = !is_gryscl;
    } else {
        change_filtering = true;
    }

    if (tex0) {
        gl.activeTexture(gl.TEXTURE0);
        gl.bindTexture(gl.TEXTURE_2D, tex0.data_vram);
        gl.uniform1i(program_glyphs.u_texture0, 0);

        // update antialiasing
        if (change_filtering) webopengl_change_texture_filtering(pvrctx);
    }
    if (tex1) {
        gl.activeTexture(gl.TEXTURE1);
        gl.bindTexture(gl.TEXTURE_2D, tex1.data_vram);
        gl.uniform1i(program_glyphs.u_texture1, 1);

        // update antialiasing
        if (change_filtering) webopengl_change_texture_filtering(pvrctx);
    }

    // send quad units
    gl.bindBuffer(gl.ARRAY_BUFFER, program_glyphs.buffer_vertex);
    gl.enableVertexAttribArray(program_glyphs.a_position);
    gl.vertexAttribPointer(program_glyphs.a_position, 2, gl.FLOAT, false, 0, 0);
    gl.bufferData(gl.ARRAY_BUFFER, vertex_array, gl.DYNAMIC_DRAW);

    // send glyph contexts
    gl.bindBuffer(gl.ARRAY_BUFFER, program_glyphs.buffer_context);
    gl.enableVertexAttribArray(program_glyphs.a_source_coords);
    gl.vertexAttribPointer(program_glyphs.a_source_coords, 4, gl.FLOAT, false, GLYPHRENDERDER_BYTES_PER_CONTEXT, 0);
    ANGLE_instanced_arrays.vertexAttribDivisorANGLE(program_glyphs.a_source_coords, 1);
    gl.enableVertexAttribArray(program_glyphs.a_draw_coords);
    gl.vertexAttribPointer(program_glyphs.a_draw_coords, 4, gl.FLOAT, false, GLYPHRENDERDER_BYTES_PER_CONTEXT, 16);
    ANGLE_instanced_arrays.vertexAttribDivisorANGLE(program_glyphs.a_draw_coords, 1);
    gl.enableVertexAttribArray(program_glyphs.a_texture_alt);
    gl.vertexAttribPointer(program_glyphs.a_texture_alt, 1, gl.FLOAT, false, GLYPHRENDERDER_BYTES_PER_CONTEXT, 32);
    ANGLE_instanced_arrays.vertexAttribDivisorANGLE(program_glyphs.a_texture_alt, 1);
    gl.enableVertexAttribArray(program_glyphs.a_color_alt);
    gl.vertexAttribPointer(program_glyphs.a_color_alt, 1, gl.FLOAT, false, GLYPHRENDERDER_BYTES_PER_CONTEXT, 36);
    ANGLE_instanced_arrays.vertexAttribDivisorANGLE(program_glyphs.a_color_alt, 1);
    gl.bufferData(gl.ARRAY_BUFFER, context_array, gl.DYNAMIC_DRAW);

    // copy transformation matrix (with all modifiers applied)
    gl.uniformMatrix4fv(program_glyphs.u_matrix_transform, false, pvrctx.current_matrix);

    // render alpha value and the text color
    WEBGL_RGBA[0] = color[0];
    WEBGL_RGBA[1] = color[1];
    WEBGL_RGBA[2] = color[2];
    WEBGL_RGBA[3] = pvrctx.render_alpha;
    gl.uniform4fv(program_glyphs.u_color, WEBGL_RGBA);

    // outline color (if used)
    WEBGL_RGBA[0] = color_outline[0];
    WEBGL_RGBA[1] = color_outline[1];
    WEBGL_RGBA[2] = color_outline[2];
    WEBGL_RGBA[3] = color_outline[3] * pvrctx.global_alpha;
    gl.uniform4fv(program_glyphs.u_color_outline, WEBGL_RGBA);

    // if the offsetcolor alpha is negative, disable the offsetcolor processing
    // "u_offsetcolor_enabled" and "u_offsetcolor_mul_or_diff" are boolean values
    if (pvrctx.render_offsetcolor[3] < 0) {
        gl.uniform1i(program_glyphs.u_offsetcolor_enabled, 0);
    } else {
        gl.uniform1i(program_glyphs.u_offsetcolor_enabled, 1);
        gl.uniform4fv(program_glyphs.u_offsetcolor, pvrctx.render_offsetcolor);
        gl.uniform1i(program_glyphs.u_offsetcolor_mul_or_diff, pvrctx.render_offsetcolor_multiply ? 1 : 0);
    }

    // grayscale textures used by fonttype_draw() or rgba textures used by fontglyph_draw()
    gl.uniform1i(program_glyphs.u_grayscale, is_gryscl ? 1 : 0);

    // used only by fontglyph_draw() function
    gl.uniform1i(program_glyphs.u_color_by_diff, by_diff ? 1 : 0);

    // @ts-ignore
    if (window.ENABLE_DOTTED) gl.uniform1i(program_glyphs.u_dotted, pvrctx.webopengl.draw_dotted);

    // send vertex indices
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, program_glyphs.buffer_indices);
    gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, index_array, gl.DYNAMIC_DRAW);
    
    // commit draw
    ANGLE_instanced_arrays.drawElementsInstancedANGLE(
        gl.TRIANGLES, total_indices, gl.UNSIGNED_INT, 0, glyphrenderer_glyphs_total
    );

    // unbind textures
    gl.activeTexture(gl.TEXTURE1);
    gl.bindTexture(gl.TEXTURE_2D, null);
    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, null);

    // unbind buffers
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, null);
}

function glyphrenderer_set_sdf_params(pvrctx, width, edge) {
    const gl = pvrctx.webopengl.gl;
    let program_glyphs = pvrctx.webopengl.program_glyphs;

    gl.useProgram(program_glyphs.program);
    gl.uniform1f(program_glyphs.u_sdf_width, width);
    gl.uniform1f(program_glyphs.u_sdf_edge, edge);
}

