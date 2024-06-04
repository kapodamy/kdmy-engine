"use strict";

const PVRCTX_FLAG_DISABLE = 0;
const PVRCTX_FLAG_ENABLE = 1;
const PVRCTX_FLAG_DEFAULT = 2;

const PVRCTX_SHADER_STACK_LENGTH = 64;

/**@typedef {[number,number,number,number]} RGBA */
/**@typedef {number} PVRFlag */


class PVRContextState {
    matrix = sh4matrix_init();
    /**@type {number}*/  global_alpha = 1.0;
    /**@type {RGBA}*/    offsetcolor = [0.0, 0.0, 0.0, 0.0];
    /**@type {PVRFlag}*/ global_antialiasing = PVRCTX_FLAG_DEFAULT;
    /**@type {PVRFlag}*/ global_offsetcolor_multiply = PVRCTX_FLAG_DEFAULT;
    /**@type {number}*/  added_shaders = 0;
}

class PVRContext {
    /**@type {HTMLCanvasElement}*/ _html5canvas = null;
    /**@type {WebGL2Context}*/ webopengl = null;
    /**@type {PVRContextState[]}*/ stack = new Array(16);
    /**@type {number}*/ resolution_changes = 0;
    /**@type {number}*/ last_elapsed = 0;
    /**@type {number}*/ frame_rendered = 0;
    /**@type {string}*/ native_window_title;

    /**@type {PSFramebuffer}*/ shader_framebuffer_front;
    /**@type {PSFramebuffer}*/ shader_framebuffer_back;
    /**@type {PSFramebuffer}*/ target_framebuffer;
    /**@type {boolean}         */ shader_needs_flush = false;
    /**@type {PSShader[]}   */ shader_stack = new Array();
    /**@type {number}       */ shader_last_resolution_changes = 0;
    /**@type {number}       */ screen_stride;


    /**@type {number}*/ stack_index = 0;
    /**@type {number}*/ stack_length = 15;

    /**@type {number}*/ vertex_alpha = 1.0;
    /**@type {RGBA}  */ vertex_offsetcolor = [0.0, 0.0, 0.0, 0.0];

    /**@type {number}*/ global_alpha = 1.0;
    /**@type {RGBA}  */ global_offsetcolor = null;

    /**@type {PVRFlag}*/ global_antialiasing = PVRCTX_FLAG_DEFAULT;
    /**@type {PVRFlag}*/ vertex_antialiasing = PVRCTX_FLAG_DEFAULT;

    /**@type {PVRFlag}*/ global_offsetcolor_multiply = PVRCTX_FLAG_DEFAULT;
    /**@type {PVRFlag}*/ vertex_offsetcolor_multiply = PVRCTX_FLAG_DEFAULT;

    /**@type {number}*/  render_alpha = 1.0;
    /**@type {RGBA}*/    render_offsetcolor = [0.0, 0.0, 0.0, 0.0];
    /**@type {PVRFlag}*/ render_offsetcolor_multiply = PVRCTX_FLAG_DEFAULT;
    /**@type {PVRFlag}*/ render_antialiasing = PVRCTX_FLAG_DEFAULT;

    get current_matrix() { return this.stack[this.stack_index].matrix; }
    get current_state() { return this.stack[this.stack_index]; }

    get screen_width() { return this._html5canvas.width; }
    get screen_height() { return this._html5canvas.height; }

    _previous_state() {
        if (this.stack_index > 0)
            return this.stack[this.stack_index - 1];
        else
            return null;
    }

    async _initWebGL() {
        this.webopengl = await webopengl_init(this._html5canvas);
        this.shader_framebuffer_front = new PSFramebuffer(this);
        this.shader_framebuffer_back = new PSFramebuffer(this);
        this.shader_last_resolution_changes = this.resolution_changes;
        PSFramebuffer.ResizeQuadScreen(this);
    }

    /** @param {HTMLCanvasElement|[number, number]} canvas */
    constructor(canvas) {
        if (canvas instanceof HTMLCanvasElement) {
            this._html5canvas = canvas;
        } else if (window.OffscreenCanvas) {
            throw new Error("OffscreenCanvas is not supported");
        } else {
            this._html5canvas = document.createElement("canvas");
            this._html5canvas.width = canvas[0];
            this._html5canvas.height = canvas[1];
        }

        for (let i = 0; i < this.stack_length; i++) {
            this.stack[i] = new PVRContextState();
            pvr_context_helper_clear_offsetcolor(this.stack[i].offsetcolor);
        }
        this.global_offsetcolor = this.stack[0].offsetcolor;
        pvr_context_helper_clear_offsetcolor(this.vertex_offsetcolor);
        pvr_context_helper_clear_offsetcolor(this.render_offsetcolor);

        this.screen_stride = this.screen_width;
        this.native_window_title = document.title;
    }


    FlushFramebuffer() {
        if (!this.shader_needs_flush) return;

        let front = this.shader_framebuffer_front;
        let back = this.shader_framebuffer_back;
        let last_index = this.shader_stack.length - 1;

        for (let i = 0; i < this.shader_stack.length; i++) {
            if (i == last_index)
                this.UseDefaultFramebuffer();
            else
                back.Use(true);

            this.shader_stack[i].Draw(front);

            let tmp = front;
            front = back;
            back = tmp;
        }

        this.shader_needs_flush = false;
    }

    ShaderStackPush(/**@type {PSShader}*/psshader) {
        if (!psshader) throw new Error("psshader can not be null");

        this.FlushFramebuffer();

        if (this.shader_stack.length >= PVRCTX_SHADER_STACK_LENGTH) {
            console.warn("PVRContext::ShaderStackPush() failed, the stack is full");
            return false;
        }

        this.shader_stack.push(psshader);
        this.shader_framebuffer_front.Use(true);

        return true;
    }

    ShaderStackPop(/**@type {number}*/count) {
        this.FlushFramebuffer();

        if (this.shader_stack.length < 1) {
            console.warn("PVRContext::ShaderStackPop() failed, the stack is empty");
            return false;
        }

        while (count-- > 0) this.shader_stack.pop();

        if (this.shader_stack.length < 1)
            this.UseDefaultFramebuffer();
        else
            this.shader_framebuffer_front.Use(true);

        return true;
    }

    CheckFramebufferSize() {
        if (this.resolution_changes == this.shader_last_resolution_changes) return;
        webopengl_resize_projection(this.webopengl, 0, 0, this.screen_width, this.screen_height);

        this.shader_last_resolution_changes = this.resolution_changes;
        this.shader_framebuffer_front.Resize();
        this.shader_framebuffer_back.Resize();
        PSFramebuffer.ResizeQuadScreen(this);
    }

    UseDefaultFramebuffer() {
        if (this.target_framebuffer)
            this.target_framebuffer.Use(false);
        else
            PSFramebuffer.UseScreenFramebuffer(this);
    }

}

async function pvr_context_init() {
    if (pvr_context) return;

    let canvas = document.querySelector('canvas');

    // @ts-ignore
    if ("main_layout_visor" in window && document.getElementById("widescreen").checked) {
        canvas.width = PVR_WIDTH_WS;
        canvas.height = PVR_HEIGHT_WS;
    } else if (SETTINGS.widescreen) {
        canvas.width = PVR_WIDTH_WS;
        canvas.height = PVR_HEIGHT_WS;
    }

    pvr_context = new PVRContext(canvas);
    await pvr_context._initWebGL();

    pvr_context_clear_screen(pvr_context, PVRCTX_CLEAR_COLOR);
}

/** @param {PVRContext} pvrctx */
function pvr_context_reset(pvrctx) {
    pvrctx.stack_index = 0;
    sh4matrix_clear(pvrctx.stack[0].matrix);

    pvrctx.stack[0].global_alpha = 1.0;
    pvrctx.global_offsetcolor = pvrctx.stack[0].offsetcolor;

    pvrctx.global_alpha = 1.0;
    pvrctx.vertex_alpha = 1.0;
    pvrctx.render_alpha = 1.0;

    pvr_context_helper_clear_offsetcolor(pvrctx.global_offsetcolor);
    pvr_context_helper_clear_offsetcolor(pvrctx.vertex_offsetcolor);

    pvrctx.global_antialiasing = PVRCTX_FLAG_ENABLE;
    pvrctx.vertex_antialiasing = PVRCTX_FLAG_DEFAULT;
    pvrctx.render_antialiasing = PVRCTX_FLAG_DEFAULT;

    pvrctx.global_offsetcolor_multiply = PVRCTX_FLAG_ENABLE;
    pvrctx.vertex_offsetcolor_multiply = PVRCTX_FLAG_DEFAULT;
    pvrctx.render_offsetcolor_multiply = PVRCTX_FLAG_DEFAULT;

    pvrctx.FlushFramebuffer();
    pvrctx.shader_stack = new Array();
    PSFramebuffer.UseScreenFramebuffer(pvrctx);
    webopengl_set_blend(pvrctx, 1, BLEND_DEFAULT, BLEND_DEFAULT, BLEND_DEFAULT, BLEND_DEFAULT);
    pvrctx.webopengl.program_textured.darken_enabled = false;
}

/** @param {PVRContext} pvrctx */
function pvr_context_apply_modifier(pvrctx, modifier) {
    let matrix = pvrctx.stack[pvrctx.stack_index].matrix;
    sh4matrix_apply_modifier(matrix, modifier);
}

/** @param {PVRContext} pvrctx */
function pvr_context_apply_modifier2(pvrctx, modifier, draw_x, draw_y, draw_width, draw_height) {
    let matrix = pvrctx.stack[pvrctx.stack_index].matrix;
    sh4matrix_apply_modifier2(matrix, modifier, draw_x, draw_y, draw_width, draw_height);
}

/** @param {PVRContext} pvrctx @param {RGBA} rgba_color */
function pvr_context_clear_screen(pvrctx, rgba_color) {
    webopengl_clear_screen(pvrctx.webopengl, rgba_color);
}

/** @param {PVRContext} pvrctx*/
function pvr_context_flush(pvrctx) { }

/** @param {PVRContext} pvrctx */
function pvr_context_save(pvrctx) {
    let next_index = pvrctx.stack_index + 1;

    if (pvrctx.stack_index >= pvrctx.stack_length) {
        console.error("pvr_context_save() the PVRContext stack is full");
        return false;
    }

    //let old_irq = SH4_INTERRUPS_DISABLE();

    let previous_state = pvrctx.stack[pvrctx.stack_index];
    let next_state = pvrctx.stack[next_index];

    // copy previous matrix
    sh4matrix_copy_to(previous_state.matrix, next_state.matrix);

    // backup
    previous_state.global_alpha = pvrctx.global_alpha;
    previous_state.global_antialiasing = pvrctx.global_antialiasing;
    previous_state.global_offsetcolor_multiply = pvrctx.global_offsetcolor_multiply;
    for (let i = 0; i < 4; i++) next_state.offsetcolor[i] = previous_state.offsetcolor[i];

    // set previous values as default
    pvrctx.vertex_alpha = pvrctx.global_alpha;
    pvrctx.render_alpha = pvrctx.global_alpha;

    pvrctx.vertex_antialiasing = PVRCTX_FLAG_DEFAULT;
    pvrctx.render_antialiasing = previous_state.global_antialiasing;

    pvrctx.vertex_offsetcolor_multiply = pvrctx.global_offsetcolor_multiply;
    pvrctx.render_offsetcolor_multiply = pvrctx.global_offsetcolor_multiply;

    pvrctx.global_offsetcolor = next_state.offsetcolor;// change reference
    for (let i = 0; i < 4; i++) {
        pvrctx.vertex_offsetcolor[i] = pvrctx.global_offsetcolor[i];
        pvrctx.render_offsetcolor[i] = pvrctx.global_offsetcolor[i];
    }

    pvrctx.stack_index = next_index;

    //SH4_INTERRUPS_ENABLE(old_irq);

    // remember the last count of added shaders
    previous_state.added_shaders = pvrctx.shader_stack.length;

    return true;
}

/** @param {PVRContext} pvrctx */
function pvr_context_restore(pvrctx) {
    if (pvrctx.stack_index < 1) {
        if (pvrctx.shader_stack.length > 0) {
            console.warn("pvr_context_restore() the current PVRContext has stacked shaders on empty stack");
        }
        console.error("pvr_context_restore() the PVRContext stack was empty");
        return false;
    }

    //let old_irq = SH4_INTERRUPS_DISABLE();

    pvrctx.stack_index--;

    let previous_state = pvrctx.stack[pvrctx.stack_index];

    pvrctx.global_antialiasing = previous_state.global_antialiasing;
    pvrctx.vertex_antialiasing = PVRCTX_FLAG_DEFAULT;
    pvrctx.render_antialiasing = previous_state.global_antialiasing;

    pvrctx.global_offsetcolor_multiply = previous_state.global_offsetcolor_multiply;
    pvrctx.vertex_offsetcolor_multiply = previous_state.global_offsetcolor_multiply;
    pvrctx.render_offsetcolor_multiply = previous_state.global_offsetcolor_multiply;

    pvrctx.global_alpha = previous_state.global_alpha;
    pvrctx.vertex_alpha = previous_state.global_alpha;
    pvrctx.render_alpha = previous_state.global_alpha;

    pvrctx.global_offsetcolor = previous_state.offsetcolor;// change reference
    for (let i = 0; i < 4; i++) {
        pvrctx.vertex_offsetcolor[i] = previous_state.offsetcolor[i];// restore
        pvrctx.render_offsetcolor[i] = previous_state.offsetcolor[i];// restore
    }

    //SH4_INTERRUPS_ENABLE(old_irq);

    // remove all shaders added in the current state
    let added_shaders = pvrctx.shader_stack.length - previous_state.added_shaders;
    console.assert(added_shaders >= 0);
    if (added_shaders > 0) pvrctx.ShaderStackPop(added_shaders);

    webopengl_set_blend(pvrctx, 1, BLEND_DEFAULT, BLEND_DEFAULT, BLEND_DEFAULT, BLEND_DEFAULT);

    return true;
}



/** @param {PVRContext} pvrctx @param {number} alpha */
function pvr_context_set_vertex_alpha(pvrctx, alpha) {
    pvrctx.vertex_alpha = alpha;
    pvrctx.render_alpha = alpha * pvrctx.global_alpha;
}

/** @param {PVRContext} pvrctx @param {PVRFlag} flag */
function pvr_context_set_vertex_antialiasing(pvrctx, flag) {
    pvrctx.vertex_antialiasing = flag;
    pvrctx.render_antialiasing = flag == PVRCTX_FLAG_DEFAULT ? pvrctx.global_antialiasing : flag;
}

/** @param {PVRContext} pvrctx @param {RGBA} offsetcolor */
function pvr_context_set_vertex_offsetcolor(pvrctx, offsetcolor) {
    for (let i = 0; i < 4; i++) pvrctx.vertex_offsetcolor[i] = offsetcolor[i];
    math2d_color_blend_normal(offsetcolor, pvrctx.global_offsetcolor, pvrctx.render_offsetcolor);
}

/** @param {PVRContext} pvrctx @param {PVRFlag} flag */
function pvr_context_vertex_offsetcolor_multiply(pvrctx, flag) {
    pvrctx.vertex_offsetcolor_multiply = flag;
    pvrctx.render_offsetcolor_multiply = flag == PVRCTX_FLAG_DEFAULT ? pvrctx.global_offsetcolor_multiply : flag;
}

/** @param {PVRContext} pvrctx*/
function pvr_context_set_vertex_blend(pvrctx, enabled, src_rgb, dst_rgb, src_alpha, dst_alpha) {
    webopengl_set_blend(pvrctx, enabled, src_rgb, dst_rgb, src_alpha, dst_alpha);
}

/** @param {PVRContext} pvrctx*/
function pvr_context_set_vertex_textured_darken(pvrctx, enabled) {
    pvrctx.webopengl.program_textured.darken_enabled = enabled;
    pvrctx.webopengl.program_solid.darken_enabled = enabled;
}


/** @param {PVRContext} pvrctx @param {number} alpha */
function pvr_context_set_global_alpha(pvrctx, alpha) {
    let last_state = pvrctx._previous_state();

    if (last_state) alpha *= last_state.global_alpha;

    pvrctx.global_alpha = alpha;
    pvr_context_set_vertex_alpha(pvrctx, pvrctx.vertex_alpha);
}

/** @param {PVRContext} pvrctx @param {PVRFlag} flag */
function pvr_context_set_global_antialiasing(pvrctx, flag) {
    let last_state = pvrctx._previous_state();

    if (last_state && flag == PVRCTX_FLAG_DEFAULT) flag = last_state.global_antialiasing;

    pvrctx.global_antialiasing = flag;
    pvr_context_set_vertex_antialiasing(pvrctx, pvrctx.vertex_antialiasing);
}

/** @param {PVRContext} pvrctx @param {RGBA} offsetcolor */
function pvr_context_set_global_offsetcolor(pvrctx, offsetcolor) {
    let last_state = pvrctx._previous_state();

    if (last_state) {
        math2d_color_blend_normal(offsetcolor, last_state.offsetcolor, pvrctx.global_offsetcolor);
    } else {
        for (let i = 0; i < 4; i++) pvrctx.global_offsetcolor[i] = offsetcolor[i];
    }

    pvr_context_set_vertex_offsetcolor(pvrctx, pvrctx.vertex_offsetcolor);
}

/** @param {PVRContext} pvrctx @param {PVRFlag} flag */
function pvr_context_global_offsetcolor_multiply(pvrctx, flag) {
    let last_state = pvrctx._previous_state();

    if (last_state && flag == PVRCTX_FLAG_DEFAULT) flag = last_state.global_offsetcolor_multiply;

    pvrctx.global_offsetcolor_multiply = flag;
    pvr_context_vertex_offsetcolor_multiply(pvrctx, pvrctx.vertex_offsetcolor_multiply);
}


/** @param {PVRContext} pvrctx */
function pvr_context_is_offscreen(pvrctx) {
    return document.hidden;
}

/** @param {PVRContext} pvrctx */
function pvr_context_draw_texture(pvrctx, texture, sx, sy, sw, sh, dx, dy, dw, dh) {
    if (!texture.data_vram) return;
    if (pvrctx.shader_stack.length > 0) pvrctx.shader_needs_flush = true;
    webopengl_draw_texture(pvrctx, texture, sx, sy, sw, sh, dx, dy, dw, dh);
}

/** @param {PVRContext} pvrctx */
function pvr_context_draw_solid_color(pvrctx, rgb_color, dx, dy, dw, dh) {
    if (pvrctx.shader_stack.length > 0) pvrctx.shader_needs_flush = true;
    webopengl_draw_solid(pvrctx, rgb_color, dx, dy, dw, dh);
}

/** @param {PVRContext} pvrctx @param {PSFramebuffer} psframebuffer*/
function pvr_context_draw_framebuffer(pvrctx, psframebuffer, sx, sy, sw, sh, dx, dy, dw, dh) {
    pvr_context.shader_needs_flush = true;
    webopengl_draw_framebuffer(pvrctx, psframebuffer, sx, sy, sw, sh, dx, dy, dw, dh);
}

/** @param {PVRContext} pvrctx @param {PSShader} psshader @returns {boolean}*/
function pvr_context_add_shader(pvrctx, psshader) {
    return pvrctx.ShaderStackPush(psshader);
}

/** @param {PVRContext} pvrctx @param {PSFramebuffer} psframebuffer*/
function pvr_context_set_framebuffer(pvrctx, psframebuffer) {
    pvrctx.target_framebuffer = psframebuffer;
    if (pvrctx.shader_stack.length < 1) pvrctx.UseDefaultFramebuffer();
}

