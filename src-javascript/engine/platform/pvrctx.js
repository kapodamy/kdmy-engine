"use strict";

const PVR_FLAG_DISABLE = 0;
const PVR_FLAG_ENABLE = 1;
const PVR_FLAG_DEFAULT = 2;

/**@typedef {[number,number,number,number]} RGBA */
/**@typedef {number} PVRFLAG */


class PVRContextState {
    matrix = sh4matrix_init();
    /**@type {number}*/  global_alpha = 1.0;
    /**@type {RGBA}*/    offsetcolor = [0.0, 0.0, 0.0, 0.0];
    /**@type {PVRFLAG}*/ global_antialiasing = PVR_FLAG_DEFAULT;
    /**@type {PVRFLAG}*/ global_offsetcolor_multiply = PVR_FLAG_DEFAULT;

    constructor() {
        pvrctx_helper_clear_offsetcolor(this.offsetcolor);
    }
}

class PVRContext {
    /**@type {HTMLCanvasElement}*/ _html5canvas = null;
    /**@type {WebGLContext}*/ webopengl = null;
    /**@type {PVRContextState[]}*/ stack = new Array(16);
    /**@type {number}*/ resolution_changes = 0;

    /**@type {number}*/ stack_index = 0;
    /**@type {number}*/ stack_length = 15;

    /**@type {number}*/ vertex_alpha = 1.0;
    /**@type {RGBA}  */ vertex_offsetcolor = [0.0, 0.0, 0.0, 0.0];

    /**@type {number}*/ global_alpha = 1.0;
    /**@type {RGBA}  */ global_offsetcolor = null;

    /**@type {PVRFLAG}*/ global_antialiasing = PVR_FLAG_DEFAULT;
    /**@type {PVRFLAG}*/ vertex_antialiasing = PVR_FLAG_DEFAULT;

    /**@type {PVRFLAG}*/ global_offsetcolor_multiply = PVR_FLAG_DEFAULT;
    /**@type {PVRFLAG}*/ vertex_offsetcolor_multiply = PVR_FLAG_DEFAULT;

    /**@type {number}*/  render_alpha = 1.0;
    /**@type {RGBA}*/    render_offsetcolor = [0.0, .0, 0.0, 0.0];
    /**@type {PVRFLAG}*/ render_offsetcolor_multiply = PVR_FLAG_DEFAULT;
    /**@type {PVRFLAG}*/ render_antialiasing = PVR_FLAG_DEFAULT;

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
    }

    /** @param {HTMLCanvasElement|[number, number]} canvas */
    constructor(canvas) {
        if (canvas instanceof HTMLCanvasElement) {
            this._html5canvas = canvas;
        } else if (window.OffscreenCanvas) {
            this._html5canvas = new OffscreenCanvas(canvas[0], canvas[1]);
        } else {
            this._html5canvas = document.createElement("canvas");
            this._html5canvas.width = canvas[0];
            this._html5canvas.height = canvas[1];
        }

        for (let i = 0; i < this.stack_length; i++) this.stack[i] = new PVRContextState();
        this.global_offsetcolor = this.stack[0].offsetcolor;
        pvrctx_helper_clear_offsetcolor(this.vertex_offsetcolor);
        pvrctx_helper_clear_offsetcolor(this.render_offsetcolor);
    }
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

    pvrctx_helper_clear_offsetcolor(pvrctx.global_offsetcolor);
    pvrctx_helper_clear_offsetcolor(pvrctx.vertex_offsetcolor);

    pvrctx.global_antialiasing = PVR_FLAG_ENABLE;
    pvrctx.vertex_antialiasing = PVR_FLAG_DEFAULT;
    pvrctx.render_antialiasing = PVR_FLAG_DEFAULT;

    pvrctx.global_offsetcolor_multiply = PVR_FLAG_ENABLE;
    pvrctx.vertex_offsetcolor_multiply = PVR_FLAG_DEFAULT;
    pvrctx.render_offsetcolor_multiply = PVR_FLAG_DEFAULT;
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

/** @deprecated @param {PVRContext} pvrctx*/
function pvr_context_flush(pvrctx) { }

/** @param {PVRContext} pvrctx */
function pvr_context_save(pvrctx) {
    let next_index = pvrctx.stack_index + 1;

    if (pvrctx.stack_index >= pvrctx.stack_length) {
        console.error("pvr_context_save() the PVRContext stack is full");
        return 0;
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

    pvrctx.vertex_antialiasing = PVR_FLAG_DEFAULT;
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

    return 1;
}

/** @param {PVRContext} pvrctx */
function pvr_context_restore(pvrctx) {
    if (pvrctx.stack_index <= 0) {
        console.error("pvr_context_restore() the PVRContext stack was empty");
        return 0;
    }

    //let old_irq = SH4_INTERRUPS_DISABLE();

    pvrctx.stack_index--;

    let previous_state = pvrctx.stack[pvrctx.stack_index];

    pvrctx.global_antialiasing = previous_state.global_antialiasing;
    pvrctx.vertex_antialiasing = PVR_FLAG_DEFAULT;
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

    return 1;
}



/** @param {PVRContext} pvrctx @param {number} alpha */
function pvr_context_set_vertex_alpha(pvrctx, alpha) {
    pvrctx.vertex_alpha = alpha;
    pvrctx.render_alpha = alpha * pvrctx.global_alpha;
}

/** @param {PVRContext} pvrctx @param {PVRFLAG} flag */
function pvr_context_set_vertex_antialiasing(pvrctx, flag) {
    pvrctx.vertex_antialiasing = flag;
    pvrctx.render_antialiasing = flag == PVR_FLAG_DEFAULT ? pvrctx.global_antialiasing : flag;
}

/** @param {PVRContext} pvrctx @param {RGBA} offsetcolor */
function pvr_context_set_vertex_offsetcolor(pvrctx, offsetcolor) {
    for (let i = 0; i < 4; i++) pvrctx.vertex_offsetcolor[i] = offsetcolor[i];
    math2d_color_blend_normal(offsetcolor, pvrctx.global_offsetcolor, pvrctx.render_offsetcolor);
}

/** @param {PVRContext} pvrctx @param {PVRFLAG} flag */
function pvr_context_vertex_offsetcolor_multiply(pvrctx, flag) {
    pvrctx.vertex_offsetcolor_multiply = flag;
    pvrctx.render_offsetcolor_multiply = flag == PVR_FLAG_DEFAULT ? pvrctx.global_offsetcolor_multiply : flag;
}


/** @param {PVRContext} pvrctx @param {number} alpha */
function pvr_context_set_global_alpha(pvrctx, alpha) {
    let last_state = pvrctx._previous_state();

    if (last_state) alpha *= last_state.global_alpha;

    pvrctx.global_alpha = alpha;
    pvr_context_set_vertex_alpha(pvrctx, pvrctx.vertex_alpha);
}

/** @param {PVRContext} pvrctx @param {PVRFLAG} flag */
function pvr_context_set_global_antialiasing(pvrctx, flag) {
    let last_state = pvrctx._previous_state();

    if (last_state && flag == PVR_FLAG_DEFAULT) flag = last_state.global_antialiasing;

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

/** @param {PVRContext} pvrctx @param {PVRFLAG} flag */
function pvr_context_global_offsetcolor_multiply(pvrctx, flag) {
    let last_state = pvrctx._previous_state();

    if (last_state && flag == PVR_FLAG_DEFAULT) flag = last_state.global_offsetcolor_multiply;

    pvrctx.global_offsetcolor_multiply = flag;
    pvr_context_vertex_offsetcolor_multiply(pvrctx, pvrctx.vertex_offsetcolor_multiply);
}


/** @param {PVRContext} pvrctx */
function pvr_context_draw_texture(pvrctx, texture, sx, sy, sw, sh, dx, dy, dw, dh) {
    if (!texture.data_vram) return;
    webopengl_draw_texture(pvrctx, pvrctx.webopengl, texture, sx, sy, sw, sh, dx, dy, dw, dh);
}

/** @param {PVRContext} pvrctx */
function pvr_context_draw_solid_color(pvrctx, rgb_color, dx, dy, dw, dh) {
    webopengl_draw_solid(pvrctx, pvrctx.webopengl, rgb_color, dx, dy, dw, dh);
}

/** @param {PVRContext} pvrctx */
function pvr_is_offscreen(pvrctx) {
    return pvrctx._html5canvas.ownerDocument.hidden;
}

