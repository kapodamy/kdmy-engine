"use strict";


const WEBGL_MAT4 = new Float32Array(4 * 4);
const WEBGL_RGBA = new Float32Array(2 * 2);
const WEBGL_QUAD = new Float32Array([0, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 1]);



class WebGLContext {

    /**@type {WebGLContextProgram}*/program_solid;
    /**@type {WebGLContextProgram}*/program_textured;
    /**@type {WebGLContextProgramGlyphs}*/program_glyphs;

    /**@type {WebGLRenderingContext}*/ gl;

    /**@type {WebGLBuffer}*/position_buffer;

    /**@type {ANGLE_instanced_arrays}*/ANGLE_instanced_arrays;

    /**@type {number}*/ draw_dotted;

    constructor(/**@type {HTMLCanvasElement}*/canvas) {
        this.gl = canvas.getContext("webgl", {
            alpha: false,
            stencil: true,
            depth: true,
            antialias: true,
            preserveDrawingBuffer: false,
            desynchronized: true,
			premultipliedAlpha: false
        });
        this.draw_dotted = 0;
    }


}

class WebGLContextProgram {

    /**@type {WebGLProgram}*/ program;

    /**@type {number}*/a_position = 0;

    /**@type {WebGLUniformLocation}*/u_matrix_model;
    /**@type {WebGLUniformLocation}*/u_matrix_transform;
    /**@type {WebGLUniformLocation}*/u_matrix_texture;
    /**@type {WebGLUniformLocation}*/u_texture;
    /**@type {WebGLUniformLocation}*/u_alpha;
    /**@type {WebGLUniformLocation}*/u_vetex_color;
    /**@type {WebGLUniformLocation}*/u_offsetcolor_mul_or_diff;
    /**@type {WebGLUniformLocation}*/u_offsetcolor_enabled;
    /**@type {WebGLUniformLocation}*/u_offsetcolor;
    /**@type {WebGLUniformLocation}*/u_dotted;


    constructor(/**@type {WebGLRenderingContext}*/gl, /**@type {WebGLProgram}*/program) {
        this.program = program;

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
        this.u_dotted = gl.getUniformLocation(program, "u_dotted");
    }

    resizeProyection(/**@type {WebGLRenderingContext}*/gl, /**@type {Float32Array}*/proyection_matrix) {
        let u_matrix_proyection = gl.getUniformLocation(this.program, "u_matrix_proyection");
        gl.useProgram(this.program);
        gl.uniformMatrix4fv(u_matrix_proyection, false, proyection_matrix);
    }
}

class WebGLContextProgramGlyphs {

    /**@type {WebGLProgram}*/ program;

    /**@type {number}*/a_position = 0;
    /**@type {number}*/a_source_coords = 0;
    /**@type {number}*/a_draw_coords = 0;
    /**@type {number}*/a_texture_id = 0;
    /**@type {number}*/a_color_alt = 0;

    /**@type {WebGLUniformLocation}*/u_color;
    /**@type {WebGLUniformLocation}*/u_color_by_diff;
    /**@type {WebGLUniformLocation}*/u_grayscale;
    /**@type {WebGLUniformLocation}*/u_matrix_transform;
    /**@type {WebGLUniformLocation}*/u_offsetcolor_enabled;
    /**@type {WebGLUniformLocation}*/u_offsetcolor;
    /**@type {WebGLUniformLocation}*/u_offsetcolor_mul_or_diff;
    /**@type {WebGLUniformLocation}*/u_texture0;
    /**@type {WebGLUniformLocation}*/u_texture1;
    /**@type {WebGLUniformLocation}*/u_sdf_width;
    /**@type {WebGLUniformLocation}*/u_sdf_edge;

    /**@type {WebGLBuffer}*/buffer_indices;
    /**@type {WebGLBuffer}*/buffer_vertex;
    /**@type {WebGLBuffer}*/buffer_context;

    constructor(/**@type {WebGLRenderingContext}*/gl, /**@type {WebGLProgram}*/program) {
        this.program = program;

        // look up where the vertex data needs to go.
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
        this.u_dotted = gl.getUniformLocation(program, "u_dotted");

        if (SDF_FONT) {
            // sdf specific uniforms
            this.u_sdf_width = gl.getUniformLocation(program, "u_sdf_width");
            this.u_sdf_edge = gl.getUniformLocation(program, "u_sdf_edge");
        }

        // glyphs buffer
        this.buffer_indices = gl.createBuffer();
        this.buffer_vertex = gl.createBuffer();
        this.buffer_context = gl.createBuffer();
    }

    resizeProyection(/**@type {WebGLRenderingContext}*/gl, /**@type {Float32Array}*/proyection_matrix) {
        let u_matrix_proyection = gl.getUniformLocation(this.program, "u_matrix_proyection");
        gl.useProgram(this.program);
        gl.uniformMatrix4fv(u_matrix_proyection, false, proyection_matrix);
    }
}

async function webopengl_init(canvas) {
    let wglc = new WebGLContext(canvas);
    const gl = wglc.gl;

    // setup GLSL programs
    let program_solid = await webopengl_internal_create_program(gl, "solid");
    let program_textured = await webopengl_internal_create_program(gl, "textured");
    let program_glyphs = await webopengl_internal_create_program(gl, "glyphs");

    wglc.program_solid = new WebGLContextProgram(gl, program_solid);
    wglc.program_textured = new WebGLContextProgram(gl, program_textured);
    wglc.program_glyphs = new WebGLContextProgramGlyphs(gl, program_glyphs);

    // quads buffer
    wglc.position_buffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, wglc.position_buffer);

    // Put a unit quad in the buffer
    gl.bufferData(gl.ARRAY_BUFFER, WEBGL_QUAD, gl.STATIC_DRAW);

    gl.disable(gl.CULL_FACE);
    gl.disable(gl.DEPTH_TEST);
    gl.depthMask(false);

    //gl.blendFuncSeparate(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA, gl.ONE, gl.ONE_MINUS_SRC_ALPHA);
	gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
    gl.enable(gl.BLEND);

    gl.pixelStorei(gl.UNPACK_PREMULTIPLY_ALPHA_WEBGL, false);

    let OES_element_index_uint = gl.getExtension("OES_element_index_uint");
    console.assert(OES_element_index_uint, "missing OES_element_index_uint which is required for text rendering");

    let extension = gl.getExtension("ANGLE_instanced_arrays");
    wglc.ANGLE_instanced_arrays = extension;
    console.assert(extension, "missing ANGLE_instanced_arrays which is required for text rendering");

    // convert from pixels to clip space
    webopengl_resize_projection(wglc, 0, 0, canvas.width, canvas.height);

    canvas.addEventListener('webglcontextlost', (event) => {
        console.error("WebGL context lost, attemp to restore", event);
        event.preventDefault();
        alert("¡FATAL ERROR!\nWebGL context is lost, attempting to restore.");
    });

    return wglc;
}

function webopengl_create_texture(/**@type {WebGLContext}*/wglc, texture_width, texture_height, bitmap_data) {
    const gl = wglc.gl;
    const pow2_width = math2d_poweroftwo_calc(texture_width);
    const pow2_height = math2d_poweroftwo_calc(texture_height);
            
    // clear any previous error
    gl.getError();

    let tex = gl.createTexture();

	gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, tex);

    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, bitmap_data);

    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);

    if (pow2_width == texture_width && pow2_height == texture_height) {
        gl.generateMipmap(gl.TEXTURE_2D);
    } else {
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    }

    return tex;
}

function webopengl_change_texture_filtering(/**@type {PVRContext}*/pvrctx) {
    const gl = pvrctx.webopengl.gl;

    let filter;
    if (pvrctx.render_antialiasing == PVR_FLAG_DISABLE)
        filter = gl.NEAREST;
    else
        filter = gl.LINEAR;

    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, filter);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, filter);
}

function webopengl_destroy_texture(/**@type {WebGLContext}*/wglc, wgl_tex) {
    wglc.gl.deleteTexture(wgl_tex.vram);
}

function webopengl_clear_screen(/**@type {WebGLContext}*/wglc, rgba_color) {
    if (rgba_color)
        wglc.gl.clearColor(rgba_color[0], rgba_color[1], rgba_color[2], rgba_color[3]);
    else
        wglc.gl.clearColor(0, 0, 0, 0);

    wglc.gl.clear(wglc.gl.COLOR_BUFFER_BIT);
}

function webopengl_resize_projection(/**@type {WebGLContext}*/wglc, offset_x, offset_y, width, height) {
	webopengl_internal_orthographic_2d(WEBGL_MAT4, width, height);
	wglc.gl.viewport(offset_x, offset_y, width, height);
    wglc.program_solid.resizeProyection(wglc.gl, WEBGL_MAT4);
    wglc.program_textured.resizeProyection(wglc.gl, WEBGL_MAT4);
    wglc.program_glyphs.resizeProyection(wglc.gl, WEBGL_MAT4);
}

/**
 * @param {PVRContext} pvrctx
 * @param {WebGLContext} wglc
 * @param {object} tex
 * @param {number} sx
 * @param {number} sy
 * @param {number} sw
 * @param {number} sh
 * @param {number} dx
 * @param {number} dy
 * @param {number} dw
 * @param {number} dh
 */
function webopengl_draw_texture(/**@type {PVRContext}*/pvrctx, wglc, tex, sx, sy, sw, sh, dx, dy, dw, dh) {
    const gl = wglc.gl;
    const tex_width = tex.width;
    const tex_height = tex.height;

    // Tell WebGL to use our shader program pair
    gl.useProgram(wglc.program_textured.program);

    gl.bindTexture(gl.TEXTURE_2D, tex.data_vram);

    // update antialiasing
    webopengl_change_texture_filtering(pvrctx);

    gl.bindBuffer(gl.ARRAY_BUFFER, wglc.position_buffer);
    gl.enableVertexAttribArray(wglc.program_textured.a_position);
    gl.vertexAttribPointer(wglc.program_textured.a_position, 2, gl.FLOAT, false, 0, 0);

    // optimized matrix model, contains the draw size and location
    webopengl_internal_identity(WEBGL_MAT4, 4);
    WEBGL_MAT4[0] = dw;
    WEBGL_MAT4[5] = dh;
    WEBGL_MAT4[12] = dx;
    WEBGL_MAT4[13] = dy;
    gl.uniformMatrix4fv(wglc.program_textured.u_matrix_model, false, WEBGL_MAT4);

    // copy transformation matrix (with all modifiers applied)
    gl.uniformMatrix4fv(wglc.program_textured.u_matrix_transform, false, pvr_context.current_matrix);

    // optimized sub-texture matrix, contains the source size and location
    webopengl_internal_identity(WEBGL_MAT4, 4);
    WEBGL_MAT4[12] = sx / tex_width;
    WEBGL_MAT4[13] = sy / tex_height;
    if (sw != tex_width) WEBGL_MAT4[0] = sw / tex_width;
    if (sh != tex_height) WEBGL_MAT4[5] = sh / tex_height;
    gl.uniformMatrix4fv(wglc.program_textured.u_matrix_texture, false, WEBGL_MAT4);

    // Tell the shader to get the texture from texture unit 0
    gl.uniform1i(wglc.program_textured.u_texture, 0);
    gl.uniform1f(wglc.program_textured.u_alpha, pvrctx.render_alpha);

    // if the offsetcolor alpha is negative, disable the offsetcolor processing
    // "u_offsetcolor_enabled" and "u_offsetcolor_mul_or_diff" are boolean values
    if (pvrctx.render_offsetcolor[3] < 0) {
        gl.uniform1i(wglc.program_textured.u_offsetcolor_enabled, 0);
    } else {
        gl.uniform1i(wglc.program_textured.u_offsetcolor_enabled, 1);
        gl.uniform4fv(wglc.program_textured.u_offsetcolor, pvrctx.render_offsetcolor);
        gl.uniform1i(wglc.program_textured.u_offsetcolor_mul_or_diff, pvrctx.render_offsetcolor_multiply ? 1 : 0);
    }

    // @ts-ignore
    if (window.ENABLE_DOTTED) wglc.gl.uniform1i(wglc.program_textured.u_dotted, wglc.draw_dotted);

    // draw the quad (2 triangles, 6 vertices)
    gl.drawArrays(gl.TRIANGLES, 0, 6);

    // unbind buffer and texture
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.bindTexture(gl.TEXTURE_2D, null);
}

/**
 * @param {PVRContext} pvrctx
 * @param {WebGLContext} wglc
 * @param {[number, number, number]} rgb_color
 * @param {number} dx
 * @param {number} dy
 * @param {number} dw
 * @param {number} dh
 */
function webopengl_draw_solid(/**@type {PVRContext}*/pvrctx, wglc, rgb_color, dx, dy, dw, dh) {
    const gl = wglc.gl;

    gl.useProgram(wglc.program_solid.program);

    gl.bindBuffer(gl.ARRAY_BUFFER, wglc.position_buffer);
    gl.enableVertexAttribArray(wglc.program_solid.a_position);
    gl.vertexAttribPointer(wglc.program_solid.a_position, 2, gl.FLOAT, false, 0, 0);

    // optimized matrix model, contains the draw size and location
    webopengl_internal_identity(WEBGL_MAT4, 4);
    WEBGL_MAT4[0] = dw;
    WEBGL_MAT4[5] = dh;
    WEBGL_MAT4[12] = dx;
    WEBGL_MAT4[13] = dy;
    gl.uniformMatrix4fv(wglc.program_solid.u_matrix_model, false, WEBGL_MAT4);

    // copy transformation matrix (with all modifiers applied)
    gl.uniformMatrix4fv(wglc.program_solid.u_matrix_transform, false, pvr_context.current_matrix);

    // build rgba color
    WEBGL_RGBA[0] = rgb_color[0];
    WEBGL_RGBA[1] = rgb_color[1];
    WEBGL_RGBA[2] = rgb_color[2];
    WEBGL_RGBA[3] = pvrctx.render_alpha;

    // if the offsetcolor alpha is negative, disable the offsetcolor processing
    // "u_offsetcolor_enabled" and "u_offsetcolor_mul_or_diff" are boolean values
    if (pvrctx.render_offsetcolor[3] < 0) {
        gl.uniform1i(wglc.program_solid.u_offsetcolor_enabled, 0);
    } else {
        gl.uniform1i(wglc.program_solid.u_offsetcolor_enabled, 1);
        gl.uniform4fv(wglc.program_solid.u_offsetcolor, pvrctx.render_offsetcolor);
        let use_multiply = pvrctx.render_offsetcolor_multiply != PVR_FLAG_DISABLE;
        gl.uniform1i(wglc.program_solid.u_offsetcolor_mul_or_diff, use_multiply ? 1 : 0);
    }

    // build rgba color & upload vertex color
    WEBGL_RGBA[0] = rgb_color[0];
    WEBGL_RGBA[1] = rgb_color[1];
    WEBGL_RGBA[2] = rgb_color[2];
    WEBGL_RGBA[3] = pvrctx.render_alpha;
    gl.uniform4fv(wglc.program_solid.u_vetex_color, WEBGL_RGBA);

    // @ts-ignore
    if (window.ENABLE_DOTTED) wglc.gl.uniform1i(wglc.program_solid.u_dotted, wglc.draw_dotted);

    // draw the quad (2 triangles, 6 vertices)
    gl.drawArrays(gl.TRIANGLES, 0, 6);

    // unbind buffer
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
}


function webopengl_internal_orthographic_2d(matrix, screen_width, screen_height) {
    // optimized orthographic matrix creation
    matrix[0] = 2 / screen_width;
    matrix[1] = matrix[2] = matrix[3] = matrix[4] = 0;
    matrix[5] = -2 / screen_height;// invert the y axis (avoids render upside down)
    matrix[6] = matrix[7] = matrix[8] = matrix[9] = 0;
    matrix[10] = matrix[12] = -1;
    matrix[11] = matrix[14] = 0;
    matrix[13] = matrix[15] = 1;
}

function webopengl_internal_create_shader(gl, source, is_vertex_shader) {
    const shader = gl.createShader(is_vertex_shader ? gl.VERTEX_SHADER : gl.FRAGMENT_SHADER);
    gl.shaderSource(shader, source);
    gl.compileShader(shader);

    // Check the compile status
    let status = gl.getShaderParameter(shader, gl.COMPILE_STATUS);
    if (status) return shader;

    let msg = gl.getShaderInfoLog(shader);
    let type = is_vertex_shader ? "vertex" : "fragment";
    console.error(`webopengl_internal_create_shader() failed to create a ${type} shader:\r\n${msg}`, source);
    gl.deleteShader(shader);

    throw new Error(type + " shader compilation failed\r\n" + msg);
}

async function webopengl_internal_create_program(gl, shader_name) {
    let vertex_shader_sourcecode = await webopengl_internal_load_shader(shader_name, true);
    let fragment_shader_sourcecode = await webopengl_internal_load_shader(shader_name, false);

    if (window["ENABLE_DOTTED"]) {
        const DOTTED = "#define DOTTED\r\n";
        vertex_shader_sourcecode = DOTTED + vertex_shader_sourcecode;
        fragment_shader_sourcecode = DOTTED + fragment_shader_sourcecode;
    }

    let vertex_shader = webopengl_internal_create_shader(gl, vertex_shader_sourcecode, true);
    let fragment_shader = webopengl_internal_create_shader(gl, fragment_shader_sourcecode, false);

    let program = gl.createProgram();
    gl.attachShader(program, vertex_shader);
    gl.attachShader(program, fragment_shader);

    gl.linkProgram(program);

    let status = gl.getProgramParameter(program, gl.LINK_STATUS);
    if (status) return program;

    // something went wrong with the link
    let msg = gl.getProgramInfoLog(program);
    console.error(`webopengl_internal_create_program() failed: ${msg}`);
    gl.deleteProgram(program);
    throw new Error("Shaders compilation failed:\n" + msg);
}

function webopengl_internal_identity(matrix, stride) {
    matrix.fill(0);

    if (stride == 3)
        matrix[0] = matrix[4] = matrix[8] = 1;
    else if (stride == 4)
        matrix[0] = matrix[5] = matrix[10] = matrix[15] = 1;

}

function webopengl_internal_enable_dotted(/**@type {WebGLContext}*/wglc, enable) {
    wglc.draw_dotted = enable ? 1 : 0;
}

async function webopengl_internal_load_shader(shader_prefix, vertexshader_or_fragmentshader) {
    const shader_suffix = vertexshader_or_fragmentshader ? "vertexshader" : "fragmentshader";
    let res = await fetch(`assets/shaders_webgl/${shader_prefix}_${shader_suffix}.glsl`);
    return await res.text();
}

