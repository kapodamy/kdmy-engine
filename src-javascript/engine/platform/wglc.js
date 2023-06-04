"use strict";


const WEBGL_MAT4 = new Float32Array(4 * 4);
const WEBGL_RGBA = new Float32Array(2 * 2);
const WEBGL_QUAD = new Float32Array([0, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 1]);
const WEBGL_QUAD_SCREEN = new Float32Array([
    -1, 1, 0, 1, -1, -1, 0, 0, 1, -1, 1, 0, -1, 1, 0, 1, 1, -1, 1, 0, 1, 1, 1, 1
]);



class WebGL2Context {

    /**@type {WebGLContextProgram}*/program_solid;
    /**@type {WebGLContextProgram}*/program_textured;
    /**@type {WebGLContextProgram}*/program_framebuffer;
    /**@type {WebGLContextProgramGlyphs}*/program_glyphs;

    /**@type {WebGL2RenderingContext}*/ gl;

    /**@type {WebGLBuffer}*/position_buffer;

    /**@type {number}*/ draw_dotted;

    /**@type {WebGLShader}*/stock_shadervertex;
    /**@type {WebGLShader}*/stock_shaderfragment;

    /**@type {bool}*/has_texture_uploads;

    /**@type {WEBGL_compressed_texture_s3tc}*/s3tc_ext;

    constructor(/**@type {HTMLCanvasElement}*/canvas) {
        this.gl = canvas.getContext("webgl2", {
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

    /**@type {number}*/a_position;
    /**@type {WebGLVertexArrayObject}*/vao;

    /**@type {WebGLUniformLocation}*/u_matrix_model;
    /**@type {WebGLUniformLocation}*/u_matrix_transform;
    /**@type {WebGLUniformLocation}*/u_matrix_texture;
    /**@type {WebGLUniformLocation}*/u_texture;
    /**@type {WebGLUniformLocation}*/u_alpha;
    /**@type {WebGLUniformLocation}*/u_vertex_color;
    /**@type {WebGLUniformLocation}*/u_offsetcolor_mul_or_diff;
    /**@type {WebGLUniformLocation}*/u_offsetcolor_enabled;
    /**@type {WebGLUniformLocation}*/u_offsetcolor;
    /**@type {WebGLUniformLocation}*/u_dotted;

    /**@type {WebGLUniformLocation}*/u_darken;

    /**@type {bool}*/darken_enabled;


    constructor(/**@type {WebGL2RenderingContext}*/gl, /**@type {WebGLProgram}*/program) {
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
        this.u_dotted = gl.getUniformLocation(program, "u_dotted");
    }

    resizeProyection(/**@type {WebGL2RenderingContext}*/gl, /**@type {Float32Array}*/proyection_matrix) {
        let u_matrix_proyection = gl.getUniformLocation(this.program, "u_matrix_proyection");
        gl.useProgram(this.program);
        gl.uniformMatrix4fv(u_matrix_proyection, false, proyection_matrix);
    }
}

class WebGLContextProgramGlyphs {

    /**@type {WebGLProgram}*/ program;

    /**@type {number}*/a_position;
    /**@type {number}*/a_source_coords;
    /**@type {number}*/a_draw_coords;
    /**@type {number}*/a_color_alt;
    /**@type {WebGLVertexArrayObject}*/vao;

    /**@type {WebGLUniformLocation}*/u_color;
    /**@type {WebGLUniformLocation}*/u_color_by_diff;
    /**@type {WebGLUniformLocation}*/u_grayscale;
    /**@type {WebGLUniformLocation}*/u_matrix_transform;
    /**@type {WebGLUniformLocation}*/u_offsetcolor_enabled;
    /**@type {WebGLUniformLocation}*/u_offsetcolor;
    /**@type {WebGLUniformLocation}*/u_offsetcolor_mul_or_diff;
    /**@type {WebGLUniformLocation}*/u_texture0;
    /**@type {WebGLUniformLocation}*/u_texture1;
    /**@type {WebGLUniformLocation}*/u_sdf_smoothing;
    /**@type {WebGLUniformLocation}*/u_sdf_thickness;

    /**@type {WebGLBuffer}*/buffer_indices;
    /**@type {WebGLBuffer}*/buffer_vertex;
    /**@type {WebGLBuffer}*/buffer_context;

    constructor(/**@type {WebGL2RenderingContext}*/gl, /**@type {WebGLProgram}*/program) {
        this.program = program;

        // vertex array object
        this.vao = gl.createVertexArray();

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
            this.u_sdf_smoothing = gl.getUniformLocation(program, "u_sdf_smoothing");
            this.u_sdf_thickness = gl.getUniformLocation(program, "u_sdf_thickness");
        }

        // glyphs buffer
        this.buffer_indices = gl.createBuffer();
        this.buffer_vertex = gl.createBuffer();
        this.buffer_context = gl.createBuffer();
    }

    resizeProyection(/**@type {WebGL2RenderingContext}*/gl, /**@type {Float32Array}*/proyection_matrix) {
        let u_matrix_proyection = gl.getUniformLocation(this.program, "u_matrix_proyection");
        gl.useProgram(this.program);
        gl.uniformMatrix4fv(u_matrix_proyection, false, proyection_matrix);
    }
}

class PSShader {
    /**
     * @typedef {object} PostprocessingUniform
     * @property {number} id
     * @property {string} name
     * @property {WebGLUniformLocation} handle
     */

    /**@type {WebGLProgram}*/program;
    /**@type {WebGLBuffer}*/buffer;
    /**@type {WebGLVertexArrayObject}*/vao;

    /**@type {number}*/a_position;
    /**@type {number}*/a_texcoord;

    /**@type {WebGLUniformLocation}*/u_kdy_time;
    /**@type {WebGLUniformLocation}*/u_kdy_delta;
    /**@type {WebGLUniformLocation}*/u_kdy_frame;
    /**@type {WebGLUniformLocation}*/u_kdy_screen;
    /**@type {WebGLUniformLocation}*/u_kdy_texsize;

    /**@type {PostprocessingUniform[]}*/uniform_cache;

    /**@type {PVRContext}*/pvrctx;
    /**@type {number}*/resolution_changes;

    static #buffer_floats = new Float32Array(16);


    constructor(/**@type {PVRContext}*/pvrctx, /**@type {WebGLProgram}*/program) {
        const gl = pvrctx.webopengl.gl;

        this.program = program;
        this.pvrctx = pvrctx;
        this.resolution_changes = pvrctx.resolution_changes;

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
        gl.bufferData(gl.ARRAY_BUFFER, WEBGL_QUAD_SCREEN, gl.STATIC_DRAW);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);
    }

    GetUniformIndex(/**@type {string}*/name) {
        if (!name) return -1;

        const gl = this.pvrctx.webopengl.gl;

        for (let u of this.uniform_cache) {
            if (u.name == name) return u.id;
        }

        /*
        // C# only
        let uniform = gl.getUniformLocation(this.program, name);
        this.uniform_cache.push({ id: uniform.ID, name: name });

        return uniform.ID;
        */

        for (let i = 0, count = gl.getProgramParameter(this.program, gl.ACTIVE_UNIFORMS); i < count; i++) {
            let info = gl.getActiveUniform(this.program, i);
            if (info.name == name) {
                this.uniform_cache.push({
                    id: i,
                    name: name,
                    handle: gl.getUniformLocation(this.program, name)
                });
                return i;
            }
        }

        return -1;
    }

    SetUniformInteger(/**@type {number}*/index, /**@type {number}*/components,/**@type {Int32Array}*/values, /**@type {bool}*/variable, /**@type {number}*/offset) {
        const gl = this.pvrctx.webopengl.gl;
        let handle = null;

        // (JS only) find uniform handle
        for (let u of this.uniform_cache) {
            if (u.id == index) {
                handle = u.handle;
                break;
            }
        }
        if (handle === null) return -1;

        if (!values) return -4;
        if (offset < 0 || offset > values.length) return -3;

        let size = values.length - offset;
        if (offset > 0) values = values.subarray(offset/*, values.length*/);

        // clear error
        gl.getError();

        switch (components) {
            case 1:
                if (size < 1) return -2;
                if (variable)
                    gl.uniform1iv(handle, values);
                else
                    gl.uniform1i(handle, values[0]);
                break;
            case 2:
                if (size < 2) return -2;
                if (variable)
                    gl.uniform2iv(handle, values);
                else
                    gl.uniform2i(handle, values[0], values[1]);
                break;
            case 3:
                if (size < 3) return -2;
                if (variable)
                    gl.uniform3iv(handle, values);
                else
                    gl.uniform3i(handle, values[0], values[1], values[2]);
                break;
            case 4:
                if (size < 4) return -2;
                if (variable)
                    gl.uniform4iv(handle, values);
                else
                    gl.uniform4i(handle, values[0], values[1], values[2], values[3]);
                break;
            default:
                throw new Error(`Invalid components=${components}. Expected 1/2/3/4`);
        }

        let error = gl.getError();

        return error == gl.NONE ? 1 : 0;
    }

    SetUniformFloat(/**@type {number}*/index, /**@type {number}*/components,/**@type {Float32Array}*/values, /**@type {bool}*/variable, /**@type {number}*/offset) {
        const gl = this.pvrctx.webopengl.gl;
        let handle = null;

        // (JS only) find uniform handle
        for (let u of this.uniform_cache) {
            if (u.id == index) {
                handle = u.handle;
                break;
            }
        }
        if (handle === null) return -1;

        if (!values) return -4;
        if (offset < 0 || offset > values.length) return -3;

        let size = values.length - offset;
        if (offset > 0) values = values.subarray(offset/*, values.length*/);

        // clear error
        gl.getError();

        switch (components) {
            case 1:
                if (size < 1) return -2;
                if (variable)
                    gl.uniform1fv(handle, values);
                else
                    gl.uniform1f(handle, values[0]);
                break;
            case 2:
                if (size < 2) return -2;
                if (variable)
                    gl.uniform2fv(handle, values);
                else
                    gl.uniform2f(handle, values[0], values[1]);
                break;
            case 3:
                if (size < 3) return -2;
                if (variable)
                    gl.uniform3fv(handle, values);
                else
                    gl.uniform3f(handle, values[0], values[1], values[2]);
                break;
            case 4:
                if (size < 4) return -2;
                if (variable)
                    gl.uniform4fv(handle, values);
                else
                    gl.uniform4f(handle, values[0], values[1], values[2], values[3]);
                break;
            default:
                throw new Error(`Invalid components=${components}. Expected 1/2/3/4`);
        }

        let error = gl.getError();

        return error == gl.NONE ? 1 : 0;
    }

    SetUniformMatrix(/**@type {number}*/index,/**@type {number}*/components, /**@type {Float32Array}*/values, /**@type {number}*/offset) {
        const gl = this.pvrctx.webopengl.gl;
        let handle = null;

        // (JS only) find uniform handle
        for (let u of this.uniform_cache) {
            if (u.id == index) {
                handle = u.handle;
                break;
            }
        }
        if (handle === null) return -1;

        if (!values) return -4;
        if (offset < 0 || offset > values.length) return -3;

        let size = values.length - offset;
        if (offset > 0) values = values.subarray(offset/*, values.length*/);

        // clear error
        gl.getError();

        switch (components) {
            case 2:
                if (size < 4) return -2;
                gl.uniformMatrix2fv(handle, false, values);
                break;
            case 3:
                if (size < 9) return -2;
                gl.uniformMatrix3fv(handle, false, values);
                break;
            case 4:
                if (size < 16) return -2;
                gl.uniformMatrix4fv(handle, false, values);
                break;
            default:
                throw new Error(`Invalid components=${components}. Expected 2/3/4`);
        }

        let error = gl.getError();

        return error == gl.NONE ? 1 : 0;
    }

    SetUniformAny(/**@type {string}*/name,  /**@type {number[]}*/values) {
        const gl = this.pvrctx.webopengl.gl;

        let type = 0;

        let count = gl.getProgramParameter(this.program, gl.ACTIVE_UNIFORMS);
        let location = gl.getUniformLocation(this.program, name);

        if (!location) return 0;

        for (let i = 0; i < count; i++) {
            let info = gl.getActiveUniform(this.program, i);
            if (info.name == name) {
                type = info.type;
                break;
            }
        }

        // ignore any unchecked error
        gl.getError();

        const buffer = PSShader.#buffer_floats;
        switch (type) {
            case gl.FLOAT_MAT2:
            case gl.FLOAT_MAT3:
            case gl.FLOAT_MAT4:
            case gl.FLOAT_MAT2x3:
            case gl.FLOAT_MAT2x4:
            case gl.FLOAT_MAT3x2:
            case gl.FLOAT_MAT3x4:
            case gl.FLOAT_MAT4x2:
            case gl.FLOAT_MAT4x3:
                for (let i = 0; i < buffer.length; i++) {
                    if (i < values.length)
                        buffer[i] = values[i];
                    else
                        buffer[i] = 0.0;
                }
                break;
        }

        switch (type) {
            case gl.FLOAT:
                gl.uniform1f(location, values[0]);
                break;
            case gl.FLOAT_VEC2:
                gl.uniform2f(location, values[0], values[1]);
                break;
            case gl.FLOAT_VEC3:
                gl.uniform3f(location, values[0], values[1], values[2]);
                break;
            case gl.FLOAT_VEC4:
                gl.uniform4f(location, values[0], values[1], values[2], values[3]);
                break;
            case gl.INT:
                gl.uniform1i(location, values[0]);
                break;
            case gl.INT_VEC2:
                gl.uniform2i(location, values[0], values[1]);
                break;
            case gl.INT_VEC3:
                gl.uniform3i(location, values[0], values[1], values[2]);
                break;
            case gl.INT_VEC4:
                gl.uniform4i(location, values[0], values[1], values[2], values[3]);
                break;
            case gl.BOOL:
                gl.uniform1i(location, values[0]);
                break;
            case gl.BOOL_VEC2:
                gl.uniform2i(location, values[0], values[1]);
                break;
            case gl.BOOL_VEC3:
                gl.uniform3i(location, values[0], values[1], values[2]);
                break;
            case gl.BOOL_VEC4:
                gl.uniform4i(location, values[0], values[1], values[2], values[3]);
                break;
            case gl.FLOAT_MAT2:
                gl.uniformMatrix2fv(location, false, buffer);
                break;
            case gl.FLOAT_MAT3:
                gl.uniformMatrix3fv(location, false, buffer);
                break;
            case gl.FLOAT_MAT4:
                gl.uniformMatrix4fv(location, false, buffer);
                break;
            case gl.UNSIGNED_INT:
                gl.uniform1ui(location, values[0]);
                break;
            case gl.UNSIGNED_INT_VEC2:
                gl.uniform2ui(location, values[0], values[1]);
                break;
            case gl.UNSIGNED_INT_VEC3:
                gl.uniform3ui(location, values[0], values[1], values[2]);
                break;
            case gl.UNSIGNED_INT_VEC4:
                gl.uniform4ui(location, values[0], values[1], values[2], values[3]);
                break;
            case gl.FLOAT_MAT2x3:
                gl.uniformMatrix3fv(location, false, buffer);
                break;
            case gl.FLOAT_MAT2x4:
                gl.uniformMatrix3fv(location, false, buffer);
                break;
            case gl.FLOAT_MAT3x2:
                gl.uniformMatrix3fv(location, false, buffer);
                break;
            case gl.FLOAT_MAT3x4:
                gl.uniformMatrix4fv(location, false, buffer);
                break;
            case gl.FLOAT_MAT4x2:
                gl.uniformMatrix3fv(location, false, buffer);
                break;
            case gl.FLOAT_MAT4x3:
                gl.uniformMatrix4fv(location, false, buffer);
                break;
            default:
                return -1;
        }

        let error = gl.getError();

        return error != gl.NONE ? 1 : -2;
    }

    SetUniform1I(/**@type {string}*/name,  /**@type {number}*/value) {
        const gl = this.pvrctx.webopengl.gl;

        let location = gl.getUniformLocation(this.program, name);
        if (!location) return false;

        // forget last error
        gl.getError();

        gl.uniform1i(location, value);
        return gl.getError() == gl.NONE ? true : false;
    }

    SetUniform1F(/**@type {string}*/name,  /**@type {number}*/value) {
        const gl = this.pvrctx.webopengl.gl;

        let location = gl.getUniformLocation(this.program, name);
        if (!location) return false;

        // forget last error
        gl.getError();

        gl.uniform1f(location, value);
        return gl.getError() == gl.NONE ? true : false;
    }

    Draw(/**@type {PSFramebuffer}*/ from_framebuffer) {
        const gl = this.pvrctx.webopengl.gl;

        if (this.resolution_changes != this.pvrctx.resolution_changes) {
            // update quad screen buffer
            this.resolution_changes = this.pvrctx.resolution_changes;
            gl.bindBuffer(gl.ARRAY_BUFFER, this.buffer);
            gl.bufferData(gl.ARRAY_BUFFER, WEBGL_QUAD_SCREEN, gl.STATIC_DRAW);
            gl.bindBuffer(gl.ARRAY_BUFFER, null);
        }

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
        gl.uniform1f(this.u_kdy_time, timer_ms_gettime64() / 1000.0);
        gl.uniform1f(this.u_kdy_delta, this.pvrctx.last_elapsed);
        gl.uniform1i(this.u_kdy_frame, this.pvrctx.frame_rendered);
        gl.uniform3fv(this.u_kdy_screen, from_framebuffer.screen_dimmens);
        gl.uniform3fv(this.u_kdy_texsize, from_framebuffer.texture_dimmens);

        gl.drawArrays(gl.TRIANGLES, 0, 6);

        gl.bindBuffer(gl.ARRAY_BUFFER, null);
        gl.bindTexture(gl.TEXTURE_2D, null);
        gl.bindVertexArray(null);
        //gl.useProgram(null);
    }

    Destroy() {
        const gl = this.pvrctx.webopengl.gl;

        gl.deleteProgram(this.program);
        gl.deleteBuffer(this.buffer);
        gl.deleteVertexArray(this.vao);

        this.pvrctx = null;
        this.buffer = null;
        this.program = null;
        this.uniform_cache = null;
    }

    static BuildFromSource(/**@type {PVRContext}*/pvrctx, /**@type {string}*/vertex_shader_sourcecode, /**@type {string}*/fragment_shader_sourcecode) {
        const wglc = pvrctx.webopengl;
        const gl = wglc.gl;

        let vertex_shader = wglc.stock_shadervertex, fragment_shader = wglc.stock_shaderfragment;

        //if (!vertex_shader_sourcecode && !fragment_shader_sourcecode) return null;

        if (vertex_shader_sourcecode) {
            vertex_shader_sourcecode = webopengl_patch_shader(vertex_shader_sourcecode, true);
            vertex_shader = webopengl_internal_create_shader(gl, vertex_shader_sourcecode, true, false);
            if (!vertex_shader) return null;
        }

        if (fragment_shader_sourcecode) {
            fragment_shader_sourcecode = webopengl_patch_shader(fragment_shader_sourcecode, false);
            fragment_shader = webopengl_internal_create_shader(gl, fragment_shader_sourcecode, false, false);
            if (!fragment_shader) return null;
        }

        if (!vertex_shader || !fragment_shader) return null;

        let program = gl.createProgram();
        gl.attachShader(program, vertex_shader);
        gl.attachShader(program, fragment_shader);
        gl.linkProgram(program);

        let status = gl.getProgramParameter(program, gl.LINK_STATUS);
        if (status) return new PSShader(pvrctx, program);

        // something went wrong with the link
        let msg = gl.getProgramInfoLog(program);
        console.error(`WebGLContextProgramPostprocessing::BuildProgram() failed: ${msg}`);
        gl.deleteProgram(program);

        return null;
    }

    static async BuildFromFiles(/**@type {PVRContext}*/pvrctx, /**@type {string}*/vertex_shader_path, /**@type {string}*/fragment_shader_path) {
        let vertex_shader_sourcecode = null, fragment_shader_sourcecode = null;

        if (vertex_shader_path) vertex_shader_sourcecode = await fs_readtext(vertex_shader_path);
        if (fragment_shader_path) fragment_shader_sourcecode = await fs_readtext(fragment_shader_path);

        return this.BuildFromSource(pvrctx, vertex_shader_sourcecode, fragment_shader_sourcecode);
    }

}

class PSFramebuffer {

    /**@type {number}*/pow2_width;
    /**@type {number}*/pow2_height;
    /**@type {Float32Array}*/texture_dimmens;
    /**@type {Float32Array}*/screen_dimmens;

    /**@type {WebGLFramebuffer}*/framebuffer;
    /**@type {WebGLTexture}*/texture;
    ///**@type {WebGLRenderbuffer}*/renderbuffer;

    /**@type {PVRContext}*/pvrctx;


    constructor(/**@type {PVRContext}*/pvrctx) {
        const gl = pvrctx.webopengl.gl;

        this.pvrctx = pvrctx;
        this.pow2_width = math2d_poweroftwo_calc(pvrctx.screen_width);
        this.pow2_height = math2d_poweroftwo_calc(pvrctx.screen_height);

        // width, height, aspect-ratio
        this.screen_dimmens = new Float32Array([
            pvrctx.screen_width, pvrctx.screen_height, pvrctx.screen_width / pvrctx.screen_height
        ]);
        // width, height, aspect-ratio
        this.texture_dimmens = new Float32Array([
            this.pow2_width, this.pow2_height, this.pow2_width / this.pow2_height
        ]);

        // framebuffer configuration
        this.framebuffer = gl.createFramebuffer();
        gl.bindFramebuffer(gl.FRAMEBUFFER, this.framebuffer);

        this.texture = gl.createTexture();
        gl.bindTexture(gl.TEXTURE_2D, this.texture);
        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, this.pow2_width, this.pow2_height, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
        gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, this.texture, 0);

        // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
        //this.renderbuffer = gl.createRenderbuffer();
        //gl.bindRenderbuffer(gl.RENDERBUFFER, this.renderbuffer);
        //gl.renderbufferStorage(gl.RENDERBUFFER, gl.STENCIL_INDEX8, this.pow2_width, this.pow2_height);
        //gl.framebufferRenderbuffer(gl.FRAMEBUFFER, gl.STENCIL_ATTACHMENT, gl.RENDERBUFFER, this.renderbuffer);

        let status = gl.checkFramebufferStatus(gl.FRAMEBUFFER);
        if (status != gl.FRAMEBUFFER_COMPLETE) {
            console.error("framebuffer is not complete, status was 0x" + status.toString(16));
        }

        gl.bindFramebuffer(gl.FRAMEBUFFER, null);
        gl.bindTexture(gl.TEXTURE_2D, null);
        //gl.bindRenderbuffer(gl.RENDERBUFFER, null);
    }

    Resize() {
        const gl = this.pvrctx.webopengl.gl;

        // update internal dimensions
        this.pow2_width = math2d_poweroftwo_calc(this.pvrctx.screen_width);
        this.pow2_height = math2d_poweroftwo_calc(this.pvrctx.screen_height);

        // width, height, aspect-ratio
        this.screen_dimmens[0] = this.pvrctx.screen_width;
        this.screen_dimmens[1] = this.pvrctx.screen_height;
        this.screen_dimmens[2] = this.pvrctx.screen_width / this.pvrctx.screen_height;

        // width, height, aspect-ratio
        this.texture_dimmens[0] = this.pow2_width;
        this.texture_dimmens[1] = this.pow2_height;
        this.texture_dimmens[2] = this.pow2_width / this.pow2_height;

        // resize color attachment
        gl.bindTexture(gl.TEXTURE_2D, this.texture);
        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, this.pow2_width, this.pow2_height, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);

        // resize stencil attachment
        //gl.bindRenderbuffer(gl.RENDERBUFFER, this.renderbuffer);
        //gl.renderbufferStorage(gl.RENDERBUFFER, gl.STENCIL_INDEX8, this.pow2_width, this.pow2_height);

        // invalidate framebuffer
        gl.bindFramebuffer(gl.FRAMEBUFFER, this.framebuffer);
        gl.invalidateFramebuffer(gl.FRAMEBUFFER, [gl.COLOR_ATTACHMENT0]);

        gl.bindFramebuffer(gl.FRAMEBUFFER, null);
        gl.bindTexture(gl.TEXTURE_2D, null);
        //gl.bindRenderbuffer(gl.RENDERBUFFER, null);
    }

    Use( /**@type {bool}*/clear_first) {
        const gl = this.pvrctx.webopengl.gl;

        gl.bindFramebuffer(gl.FRAMEBUFFER, this.framebuffer);

        if (clear_first) {
            // clear contents
            gl.clearColor(0.0, 0.0, 0.0, 0.0);
            gl.clear(gl.COLOR_BUFFER_BIT);
        }
    }

    Destroy() {
        const gl = this.pvrctx.webopengl.gl;

        gl.deleteFramebuffer(this.framebuffer);
        gl.deleteTexture(this.texture);
        //gl.deleteRenderbuffer(this.renderbuffer);

        this.framebuffer = null;
        this.texture = null;
        //this.renderbuffer = null;

        //if (this.pvrctx.target_framebuffer == this) this.pvrctx.target_framebuffer = null;
    }

    Invalidate() {
        const gl = this.pvrctx.webopengl.gl;

        let temp = gl.getParameter(gl.FRAMEBUFFER_BINDING);

        gl.bindFramebuffer(gl.FRAMEBUFFER, this.framebuffer);
        gl.invalidateFramebuffer(gl.FRAMEBUFFER, [gl.COLOR_ATTACHMENT0]);

        gl.bindFramebuffer(gl.FRAMEBUFFER, temp);
    }

    static UseScreenFramebuffer(/**@type {PVRContext}*/pvrctx) {
        const gl = pvrctx.webopengl.gl;

        gl.bindFramebuffer(gl.FRAMEBUFFER, null);
    }

    static ResizeQuadScreen(/**@type {PVRContext}*/pvrctx) {
        let width = pvrctx.screen_stride;
        let height = pvrctx.screen_height;
        let pow2_width = math2d_poweroftwo_calc(width);
        let pow2_height = math2d_poweroftwo_calc(height);
        let u = width / pow2_width;
        let v = height / pow2_height;

        WEBGL_QUAD_SCREEN[20 + 2] = WEBGL_QUAD_SCREEN[16 + 2] = WEBGL_QUAD_SCREEN[8 + 2] = u;
        WEBGL_QUAD_SCREEN[20 + 3] = WEBGL_QUAD_SCREEN[12 + 3] = WEBGL_QUAD_SCREEN[0 + 3] = v;
    }
}

async function webopengl_init(canvas) {
    let wglc = new WebGL2Context(canvas);
    const gl = wglc.gl;

    // setup GLSL programs
    let program_solid = await webopengl_internal_create_program(gl, "solid");
    let program_textured = await webopengl_internal_create_program(gl, "textured");
    let program_framebuffer = await webopengl_internal_create_program(gl, "framebuffer");
    let program_glyphs = await webopengl_internal_create_program(gl, "glyphs");

    wglc.program_solid = new WebGLContextProgram(gl, program_solid);
    wglc.program_textured = new WebGLContextProgram(gl, program_textured);
    wglc.program_glyphs = new WebGLContextProgramGlyphs(gl, program_glyphs);
    wglc.program_framebuffer = new WebGLContextProgram(gl, program_framebuffer);

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

    // convert from pixels to clip space
    webopengl_resize_projection(wglc, 0, 0, canvas.width, canvas.height);

    canvas.addEventListener('webglcontextlost', (event) => {
        console.error("WebGL context lost, attemp to restore", event);
        event.preventDefault();
        alert("¡FATAL ERROR!\nWebGL context is lost, attempting to restore.");
    });

    // build stock post-processing shaders
    wglc.stock_shadervertex = webopengl_internal_create_shader(
        gl, webopengl_patch_shader(await webopengl_internal_load_shader("stock", true), true), true, true
    );
    wglc.stock_shaderfragment = webopengl_internal_create_shader(
        gl, webopengl_patch_shader(await webopengl_internal_load_shader("stock", false), false), false, true
    );

    wglc.has_texture_uploads = 0;

    // required to support compressed textures
    wglc.s3tc_ext = gl.getExtension("WEBGL_compressed_texture_s3tc");

    return wglc;
}

function webopengl_create_texture(/**@type {WebGL2Context}*/wglc, texture_width, texture_height, bitmap_data) {
    const gl = wglc.gl;

    // clear any previous error
    gl.getError();

    let tex = gl.createTexture();

    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, tex);

    if (bitmap_data instanceof DDS) {
        let dds = bitmap_data;

        if (wglc.s3tc_ext == null) {
            console.error("WebGLContext::CreateTexture() EXT_texture_compression_s3tc is not supported");
            return null;
        }

        let format;
        switch (dds.Compression) {
            case DDSCompression.DXT1:
                format = wglc.s3tc_ext.COMPRESSED_RGB_S3TC_DXT1_EXT;
                break;
            case DDSCompression.DXT3:
                format = wglc.s3tc_ext.COMPRESSED_RGBA_S3TC_DXT3_EXT;
                break;
            case DDSCompression.DXT5:
                format = wglc.s3tc_ext.COMPRESSED_RGBA_S3TC_DXT5_EXT;
                break;
            default:
                // this never should happen
                gl.deleteTexture(tex);
                return null;
        }

        // upload main bitmap
        gl.compressedTexImage2D(
            gl.TEXTURE_2D,
            0,
            format,
            dds.width, dds.height,
            0,
            dds.pixels
        );

        // upload mipmaps (if present)
        for (let i = 0; i < dds.Mipmaps.length; i++) {
            gl.compressedTexImage2D(
                gl.TEXTURE_2D,
                i + 1,
                format,
                dds.Mipmaps[i].width, dds.Mipmaps[i].height,
                0,
                dds.Mipmaps[i].pixels
            );
        }
    } else {
        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, bitmap_data);
    }

    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);

    //
    // Note:
    //       most of uncompresed textures are 8192x8192 and requires
    //       256MiB of vram each one.
    //       Mipmaping such texture will require ~476MiB, when the game
    //       normally requires 512MiB~1.2GiB of vram. This increases the
    //       requeriments to 1.4Gib~2.25GiB of vram.
    //gl.generateMipmap(gl.TEXTURE_2D);

    wglc.has_texture_uploads = 1;

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

function webopengl_destroy_texture(/**@type {WebGL2Context}*/wglc, wgl_tex) {
    wglc.gl.deleteTexture(wgl_tex.vram);
}

function webopengl_clear_screen(/**@type {WebGL2Context}*/wglc, rgba_color) {
    if (rgba_color)
        wglc.gl.clearColor(rgba_color[0], rgba_color[1], rgba_color[2], rgba_color[3]);
    else
        wglc.gl.clearColor(0, 0, 0, 0);

    wglc.gl.clear(wglc.gl.COLOR_BUFFER_BIT);
}

function webopengl_resize_projection(/**@type {WebGL2Context}*/wglc, offset_x, offset_y, width, height) {
    webopengl_internal_orthographic_2d(WEBGL_MAT4, width, height);
    wglc.gl.viewport(offset_x, offset_y, width, height);
    wglc.program_solid.resizeProyection(wglc.gl, WEBGL_MAT4);
    wglc.program_textured.resizeProyection(wglc.gl, WEBGL_MAT4);
    wglc.program_glyphs.resizeProyection(wglc.gl, WEBGL_MAT4);

    WEBGL_MAT4[5] = -WEBGL_MAT4[5];
    wglc.program_framebuffer.resizeProyection(wglc.gl, WEBGL_MAT4);
}

/**
 * @param {PVRContext} pvrctx
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
function webopengl_draw_texture(/**@type {PVRContext}*/pvrctx, tex, sx, sy, sw, sh, dx, dy, dw, dh) {
    const wglc = pvrctx.webopengl;
    const gl = pvrctx.webopengl.gl;
    const tex_width = tex.width;
    const tex_height = tex.height;

    // Tell WebGL to use our shader program pair
    gl.useProgram(wglc.program_textured.program);
    gl.bindVertexArray(wglc.program_textured.vao);

    // bind texture and update antialiasing
    gl.bindTexture(gl.TEXTURE_2D, tex.data_vram);
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

    // enable/disable rgb color components be multiplied by the render alpha
    gl.uniform1i(wglc.program_textured.u_darken, wglc.program_textured.darken_enabled ? 1 : 0);

    // @ts-ignore
    if (window.ENABLE_DOTTED) wglc.gl.uniform1i(wglc.program_textured.u_dotted, wglc.draw_dotted);

    // draw the quad (2 triangles, 6 vertices)
    gl.drawArrays(gl.TRIANGLES, 0, 6);

    // unbind buffers and texture
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.bindTexture(gl.TEXTURE_2D, null);
    gl.bindVertexArray(null);
}

/**
 * @param {PVRContext} pvrctx
 * @param {[number, number, number]} rgb_color
 * @param {number} dx
 * @param {number} dy
 * @param {number} dw
 * @param {number} dh
 */
function webopengl_draw_solid(/**@type {PVRContext}*/pvrctx, rgb_color, dx, dy, dw, dh) {
    const wglc = pvrctx.webopengl;
    const gl = pvrctx.webopengl.gl;

    gl.useProgram(wglc.program_solid.program);
    gl.bindVertexArray(wglc.program_solid.vao);

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
    gl.uniform4fv(wglc.program_solid.u_vertex_color, WEBGL_RGBA);

    // enable/disable rgb color components be multiplied by the render alpha
    gl.uniform1i(wglc.program_solid.u_darken, wglc.program_solid.darken_enabled ? 1 : 0);

    // @ts-ignore
    if (window.ENABLE_DOTTED) wglc.gl.uniform1i(wglc.program_solid.u_dotted, wglc.draw_dotted);

    // draw the quad (2 triangles, 6 vertices)
    gl.drawArrays(gl.TRIANGLES, 0, 6);

    // unbind buffers
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.bindVertexArray(null);
}

/**
 * @param {PVRContext} pvrctx
 * @param {PSFramebuffer} frmbffr
 * @param {number} sx
 * @param {number} sy
 * @param {number} sw
 * @param {number} sh
 * @param {number} dx
 * @param {number} dy
 * @param {number} dw
 * @param {number} dh
 */
function webopengl_draw_framebuffer(/**@type {PVRContext}*/pvrctx, frmbffr, sx, sy, sw, sh, dx, dy, dw, dh) {
    const wglc = pvrctx.webopengl;
    const gl = pvrctx.webopengl.gl;
    const tex_width = frmbffr.pow2_width;
    const tex_height = frmbffr.pow2_height;

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
    webopengl_internal_identity(WEBGL_MAT4, 4);
    WEBGL_MAT4[0] = dw;
    WEBGL_MAT4[5] = dh;
    WEBGL_MAT4[12] = dx;
    WEBGL_MAT4[13] = dy;
    gl.uniformMatrix4fv(wglc.program_framebuffer.u_matrix_model, false, WEBGL_MAT4);

    // copy transformation matrix (with all modifiers applied)
    gl.uniformMatrix4fv(wglc.program_framebuffer.u_matrix_transform, false, pvr_context.current_matrix);

    // optimized sub-texture matrix, contains the source size and location
    webopengl_internal_identity(WEBGL_MAT4, 4);
    WEBGL_MAT4[12] = sx / tex_width;
    WEBGL_MAT4[13] = sy / tex_height;
    if (sw != tex_width) WEBGL_MAT4[0] = sw / tex_width;
    if (sh != tex_height) WEBGL_MAT4[5] = sh / tex_height;
    gl.uniformMatrix4fv(wglc.program_framebuffer.u_matrix_texture, false, WEBGL_MAT4);

    // Tell the shader to get the texture from texture unit 0
    gl.uniform1i(wglc.program_framebuffer.u_texture, 0);
    gl.uniform1f(wglc.program_framebuffer.u_alpha, pvrctx.render_alpha);

    // draw the quad (2 triangles, 6 vertices)
    gl.drawArrays(gl.TRIANGLES, 0, 6);

    // unbind buffers and texture
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.bindTexture(gl.TEXTURE_2D, null);
    gl.bindVertexArray(null);
}

function webopengl_set_blend(/**@type {PVRContext}*/pvrctx, enabled, src_rgb, dst_rgb, src_alpha, dst_alpha) {
    const gl = pvrctx.webopengl.gl;

    if (enabled) {
        gl.enable(gl.BLEND);
    } else {
        gl.disable(gl.BLEND);
        return;
    }

    let src = webopengl_internal_get_blend(gl, src_rgb, gl.SRC_ALPHA);
    let dst = webopengl_internal_get_blend(gl, dst_rgb, gl.ONE_MINUS_SRC_ALPHA);
    let src_a = webopengl_internal_get_blend(gl, src_alpha, gl.ONE);
    let dst_a = webopengl_internal_get_blend(gl, dst_alpha, gl.ONE_MINUS_SRC_ALPHA);

    gl.blendFuncSeparate(src, dst, src_a, dst_a);
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

function webopengl_internal_create_shader(gl, source, is_vertex_shader, throw_error) {
    const shader = gl.createShader(is_vertex_shader ? gl.VERTEX_SHADER : gl.FRAGMENT_SHADER);
    gl.shaderSource(shader, source);
    gl.compileShader(shader);

    // Check the compile status
    let status = gl.getShaderParameter(shader, gl.COMPILE_STATUS);
    if (status) return shader;

    let msg = gl.getShaderInfoLog(shader);
    let type = is_vertex_shader ? "vertex" : "fragment";
    console.error(`webopengl_internal_create_shader() failed to create a ${type} shader:\n${msg}`, source);
    gl.deleteShader(shader);

    let error_msg = type + " shader compilation failed\n" + msg;

    if (throw_error) throw new Error(error_msg);

    return null;
}

async function webopengl_internal_create_program(gl, shader_name) {
    let vertex_shader_sourcecode = await webopengl_internal_load_shader(shader_name, true);
    let fragment_shader_sourcecode = await webopengl_internal_load_shader(shader_name, false);

    let vertex_shader = webopengl_internal_create_shader(gl, vertex_shader_sourcecode, true, true);
    let fragment_shader = webopengl_internal_create_shader(gl, fragment_shader_sourcecode, false, true);

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

function webopengl_internal_enable_dotted(/**@type {WebGL2Context}*/wglc, enable) {
    wglc.draw_dotted = enable ? 1 : 0;
}

function webopengl_set_shader_version(sourcecode) {
    if (window["ENABLE_DOTTED"]) {
        const DOTTED = "#define DOTTED\n";
        sourcecode = DOTTED + sourcecode;
    }
    return "#version 300 es\nprecision highp float;\n\n" + sourcecode;

}

async function webopengl_internal_load_shader(shader_prefix, vertexshader_or_fragmentshader) {
    const shader_suffix = vertexshader_or_fragmentshader ? "vertexshader" : "fragmentshader";
    let src = `assets/shaders/${shader_prefix}_${shader_suffix}.glsl`;
    let text = await io_foreground_request(src, IO_REQUEST_STRING);

    if (shader_prefix != "stock" && text) text = webopengl_set_shader_version(text);

    return text;
}

function webopengl_patch_shader(sourcecode, is_vertex_shader) {
    const header_vertex = "in vec4 a_position;\nin vec2 a_texcoord;\nout vec2 TexCoord;";
    const header_fragment = "out vec4 FragColor;\nin vec2 TexCoord;\nuniform sampler2D Texture;";

    const common = "uniform float u_kdy_time, u_kdy_delta;\nuniform int u_kdy_frame;\nuniform vec3 u_kdy_screen, u_kdy_texsize;\n";

    const rx_header = /#pragma header[\s\t\xA0]*\r?\n/g;
    const rx_shadertoy = /#pragma shadertoy_mainImage[\s\t\xA0]*\r?\n/g;

    const shadertoy_stubs = `
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

`;

    // remove all empty lines or spaces at the start
    sourcecode = sourcecode.trimStart();

    sourcecode = webopengl_set_shader_version(sourcecode);

    if (is_vertex_shader) {
        sourcecode = sourcecode.replace(rx_header, `${header_vertex}\n\n${common}`);
    } else {
        sourcecode = sourcecode.replace(rx_header, `${header_fragment}\n\n${common}`);
        sourcecode = sourcecode.replace(rx_shadertoy, shadertoy_stubs);
    }

    return sourcecode;
}

function webopengl_internal_get_blend(/**@type {WebGL2RenderingContext} */ gl, enum_blend, def) {
    switch (enum_blend) {
        default:
            return def;
        case BLEND_ZERO:
            return gl.ZERO;
        case BLEND_ONE:
            return gl.ONE;
        case BLEND_SRC_COLOR:
            return gl.SRC_COLOR;
        case BLEND_ONE_MINUS_SRC_COLOR:
            return gl.ONE_MINUS_SRC_COLOR;
        case BLEND_DST_COLOR:
            return gl.DST_COLOR;
        case BLEND_ONE_MINUS_DST_COLOR:
            return gl.ONE_MINUS_DST_COLOR;
        case BLEND_SRC_ALPHA:
            return gl.SRC_ALPHA;
        case BLEND_ONE_MINUS_SRC_ALPHA:
            return gl.ONE_MINUS_SRC_ALPHA;
        case BLEND_DST_ALPHA:
            return gl.DST_ALPHA;
        case BLEND_ONE_MINUS_DST_ALPHA:
            return gl.ONE_MINUS_DST_ALPHA;
        case BLEND_CONSTANT_COLOR:
            return gl.CONSTANT_COLOR;
        case BLEND_ONE_MINUS_CONSTANT_COLOR:
            return gl.ONE_MINUS_CONSTANT_COLOR;
        case BLEND_CONSTANT_ALPHA:
            return gl.CONSTANT_ALPHA;
        case BLEND_ONE_MINUS_CONSTANT_ALPHA:
            return gl.ONE_MINUS_CONSTANT_ALPHA;
        case BLEND_SRC_ALPHA_SATURATE:
            return gl.SRC_ALPHA_SATURATE;
    }
}

