/**
 * Sega Dreamcast PowerVR graphics processor backend
 */
"use strict";

const PVR_FPS_WIDTH = 25;
/**@type {RGBA}*/ const PVR_CLEAR_COLOR = [0.5, 0.5, 0.5, 1.0];

const PVR_AUTORUN = 1;
const PVR_DREAMCAST_VRAM_SIZE = 1024 * 1024 * 8;
/**@type {RGBA}*/ const PVR_DEFAULT_OFFSET_COLOR = [0.0, 0.0, 0.0, -1.0];

const PVR_FRAME_TIME = 0;//1000.0 / 60;
const PVR_AUTOHIDE_CURSOR_MILLISECONDS = 3000;
var pvr_last_timestamp = -1;
var pvr_total_elapsed = 0;
var pvr_total_frames = 0;
var pvr_cur_frames = 0;
var pvr_frame_next = 1000;
var pvr_suspended = 0;
var pvr_init = 1;
var pvr_callback_resume = null;
var /**@type {MediaQueryList}*/pvr_last_match_media = null;
var /**@type {string[]}*/ pvr_launch_args = new Array();

var /** @type {HTMLInputElement} */PVR_STATUS = null;


const VERTEX_SPRITE = 0;
const VERTEX_TEXTSPRITE = 1;
const VERTEX_STATESPRITE = 2;
const VERTEX_DRAWABLE = 3;
const VERTEX_SHAPE = 4;


/**
 * The PVR system allocates enough space for two frames: one in
   data collection mode, and another in rendering mode. If a frame is 
   currently rendering, and another frame has already been closed, then the
   caller cannot do anything else until the rendering frame completes. Note
   also that the new frame cannot be activated except during a vertical
   blanking period, so this essentially waits until a rendered frame is
   complete __and__ a vertical blank happens.
 * @summary Block the caller until the PVR system is ready for another frame to be
   submitted
 * @returns {Promise<number>}	On success. Returns the amount of milliseconds
   since the last frame (last call to {@link pvrctx_wait_ready}), also the PVR system is ready for another
   frame's data to be collected.
   On error. Returns -1, something is probably very wrong...
 * @async
 */
function pvrctx_wait_ready() {

    if (pvr_context.webopengl.has_texture_uploads) {
        // wait until all textures are completely uploaded
        pvr_context.webopengl.has_texture_uploads = false;
        pvr_context.webopengl.gl.flush();
    }

    // flush front framebuffer if there anything drawn
    pvr_context.FlushFramebuffer();
    pvr_context.shader_framebuffer_front.Invalidate();
    pvr_context.shader_framebuffer_back.Invalidate();

    mastervolume_draw(pvr_context);

    return new Promise(function (resolve, reject) {
        const handle = requestAnimationFrame(function (now) {
            document.removeEventListener("visibilitychange", visibilitychange);
            if (pvr_suspended) {
                pvr_callback_resume = resolve;
                return;
            }

            let elapsed;
            if (pvr_last_timestamp < 0)
                elapsed = 0;
            else
                elapsed = now - pvr_last_timestamp;

            pvr_last_timestamp = now;

            //pvr_context_clear_screen(pvr_context, null);
            //pvr_context_clear_screen(pvr_context, "gray");

            pvr_show_fps(elapsed);

            //
            // Note: the engine should not run below 15fps, or the beat synchronization will be lost
            //
            elapsed = Math.min(elapsed, 66.66666666666667);

            pvr_context.last_elapsed = elapsed;
            pvr_context.frame_rendered++;

            // resize framebuffers if the screen size has changed
            pvr_context.CheckFramebufferSize();
            luascriptplatform.PollWindowState().then(function () {
                resolve(elapsed);
            });
        });

        if (document.hidden) return;

        function visibilitychange() {
            cancelAnimationFrame(handle);
            document.removeEventListener("visibilitychange", visibilitychange);
            resolve(0);
        }

        document.addEventListener("visibilitychange", visibilitychange);
    });
}


function pvr_mem_available() {
    let commited = 0;

    for (const [id, tex] of TEXTURE_POOL) {
        if (tex.data && !tex.defer)
            commited += tex.size;
    }

    return PVR_DREAMCAST_VRAM_SIZE - commited;
}


var PVR_WIDTH = 640, PVR_HEIGHT = 480;
var PVR_WIDTH_WS = 960, PVR_HEIGHT_WS = 540;
var PVR_PIXELFORMAT_BYTES = 4;// in the dreamcast is 2 bytes (RGB565)
var pvr_draw_callback = function (elapsed) { };

/**@type {PVRContext}*/ var pvr_context;


function pvrctx_helper_draw_vertex(vertex_type, vertex_holder) {
    switch (vertex_type) {
        case VERTEX_SPRITE:
            sprite_draw(vertex_holder, pvr_context);
            break;
        case VERTEX_TEXTSPRITE:
            textsprite_draw(vertex_holder, pvr_context);
            break;
        case VERTEX_STATESPRITE:
            statesprite_draw(vertex_holder, pvr_context);
            break;
    }
}

/** @deprecated */
function pvr_draw_scene() {
    //pvr_context_clear_screen(pvr_context);

    // important step
    pvr_context_reset(pvr_context);

    let buffer = new Array(SPRITE_POOL.size + TEXTSPRITE_POOL.size);
    let i = 0;

    for (const [id, sprite] of SPRITE_POOL) {
        if (sprite.visible) {
            buffer[i++] = {
                type: VERTEX_SPRITE, holder: sprite, z: sprite_get_z_index(sprite)
            };
        }
    }

    for (const [id, textsprite] of TEXTSPRITE_POOL) {
        if (textsprite.visible)
            buffer[i++] = {
                type: VERTEX_TEXTSPRITE, holder: textsprite, z: textsprite_get_z_index(textsprite)
            };
    }

    for (const [id, statesprite] of STATESPRITE_POOL) {
        if (statesprite.visible)
            buffer[i++] = {
                type: VERTEX_TEXTSPRITE, holder: statesprite, z: statesprite_get_z_index(statesprite)
            };
    }

    for (const [id, drawable] of DRAWABLE_POOL) {
        if (drawable.visible)
            buffer[i++] = {
                type: VERTEX_TEXTSPRITE, holder: drawable, z: drawable_get_z_index(drawable)
            };
    }

    buffer.sort((a, b) => a.z - b.z);

    for (let j = 0; j < i; j++) {
        pvrctx_helper_draw_vertex(buffer[j].type, buffer[j].holder);
    }
}

function pvr_show_fps(elapsed) {
    pvr_total_frames++;
    pvr_total_elapsed += elapsed;

    if (pvr_total_elapsed >= pvr_frame_next) {
        pvr_cur_frames = pvr_total_frames;
        pvr_total_frames = 0;
        pvr_frame_next += 1000;
    }
    PVR_STATUS.value = pvr_cur_frames.toString();
}

function pvr_onoff(e) {
    if (e.type == "keypress") {
        if (e.key != "p" && e.key != "P") return;
        console.info("PVR: [p] key pressed");
    } else {
        e.target.blur();
    }

    if (pvr_init) {
        pvr_init = 0;
        pvr_context_clear_screen(pvr_context, PVR_CLEAR_COLOR);

        // read the entry point
        /**@type {HTMLInputElement} */
        let entry_point_input = document.querySelector("input[name=entry_point]");
        let entry_point = entry_point_input.value;
        /** @type {function} */
        let main_fn = window[entry_point];

        // emulate main thread
        kos_thd_id++;
        main_fn(pvr_launch_args.length, pvr_launch_args);
        return;
    }

    pvr_suspended = pvr_suspended ? 0 : 1;
    if (pvr_suspended) {
        PVR_STATUS.value += " (suspended)";
    } else if (pvr_callback_resume) {
        let fn = pvr_callback_resume;
        pvr_callback_resume = null;
        fn(0);
    }
}

async function pvr_copy_framebuffer() {
    // TODO: in the dreamcast, read the framebuffer directly
    let canvas = document.createElement("canvas");
    canvas.width = math2d_poweroftwo_calc(pvr_context.screen_width);
    canvas.height = math2d_poweroftwo_calc(pvr_context.screen_height);
    let ctx = canvas.getContext("2d");
    ctx.drawImage(pvr_context._html5canvas, 0, 0);
    let raw_data = await createImageBitmap(canvas);
    let byte_length = canvas.width * canvas.height * 4;// estimated

    return texture_init_from_raw(raw_data, byte_length, 0, canvas.width, canvas.height, pvr_context.screen_width, pvr_context.screen_height);
}

async function pvr_fullscreen(e) {
    try {
        e.target.blur();

        if (document.fullscreenElement)
            await document.exitFullscreen();
        else
            await pvr_context._html5canvas.requestFullscreen({ navigationUI: "hide" });
    } catch (err) {
        console.error("pvr_fullscreen() failed", err);
    }
}

async function pvr_widescreen(e) {
    if (document.fullscreenElement) return;
    e.target.blur();

    let widescreen = SETTINGS.storage_load_boolean(SETTINGS.INI_MISC_SECTION, "widescreen", false);
    SETTINGS.storage_save_boolean(SETTINGS.INI_MISC_SECTION, "widescreen", !widescreen);

    pvr_update_devicePixelRatio();
}


function pvr_update_devicePixelRatio() {
    if (pvr_last_match_media != null) {
        pvr_last_match_media.removeEventListener("change", pvr_update_devicePixelRatio);
    }

    pvr_last_match_media = matchMedia(`(resolution: ${window.devicePixelRatio}dppx)`);
    if (pvr_last_match_media) {
        pvr_last_match_media.addEventListener("change", pvr_update_devicePixelRatio);
    }

    if (pvr_context._html5canvas instanceof OffscreenCanvas) return;

    let ratio = window.devicePixelRatio;
    let canvas = pvr_context._html5canvas;
    let target_width = PVR_WIDTH, target_height = PVR_HEIGHT;

    if (document.fullscreenElement != null) {
        target_width = window.innerWidth;
        target_height = window.innerHeight;
    } else if ("main_layout_visor" in window) {
        // @ts-ignore
        let widescreen_checked = document.getElementById("widescreen").checked;
        target_width = widescreen_checked ? PVR_WIDTH_WS : PVR_WIDTH
        target_height = widescreen_checked ? PVR_HEIGHT_WS : PVR_HEIGHT;
    } else if (SETTINGS.storage_load_boolean(SETTINGS.INI_MISC_SECTION, "widescreen", false)) {
        target_width = PVR_WIDTH_WS;
        target_height = PVR_HEIGHT_WS;
    }

    if (ratio != 1.0) {
        canvas.width = target_width * ratio;
        canvas.height = target_height * ratio;
        canvas.style.width = `${target_width}px`;
        canvas.style.height = `${target_height}px`;
    } else {
        canvas.width = target_width;
        canvas.height = target_height;
        canvas.style.width = '';
        canvas.style.height = '';
    }

    pvr_context.resolution_changes++;
}

function pvr_cursor_hidder() {
    function hide_cursor() {
        if (SETTINGS.autohide_cursor) {
            pvr_context._html5canvas.classList.add("hide-cursor");
        }
    }
    let id = setTimeout(hide_cursor, PVR_AUTOHIDE_CURSOR_MILLISECONDS);

    document.addEventListener("mousemove", function (e) {
        clearTimeout(id);
        pvr_context._html5canvas.classList.remove("hide-cursor");
        id = setTimeout(hide_cursor, PVR_AUTOHIDE_CURSOR_MILLISECONDS);
    });
}

function pvrctx_helper_clear_modifier(modifier) {
    const BASE_MODIFIER = {
        translate_x: 0.0,
        translate_y: 0.0,

        rotate: 0.0,

        skew_x: 0.0,
        skew_y: 0.0,

        scale_x: 1.0,
        scale_y: 1.0,

        scale_direction_x: 0,
        scale_direction_y: 0,

        rotate_pivot_enabled: 0,
        rotate_pivot_u: 0.5,
        rotate_pivot_v: 0.5,

        translate_rotation: 0,
        scale_size: 0,
        scale_translation: 0,

        x: 0,
        y: 0,
        width: -1,
        height: -1
    };

    for (const field in BASE_MODIFIER) modifier[field] = BASE_MODIFIER[field];
}

function pvrctx_helper_clear_offsetcolor(offsetcolor) {
    offsetcolor[0] = 0.0;
    offsetcolor[1] = 0.0;
    offsetcolor[2] = 0.0;
    offsetcolor[3] = -1.0;
}

function pvrctx_helper_set_modifier_property(modifier, property, value) {
    switch (property) {
        case SPRITE_PROP_ROTATE:
            modifier.rotate = value * MATH2D_DEG_TO_RAD;
            break;
        case SPRITE_PROP_SCALE_X:
            modifier.scale_x = value;
            break;
        case SPRITE_PROP_SCALE_Y:
            modifier.scale_y = value;
            break;
        case SPRITE_PROP_TRANSLATE_X:
            modifier.translate_x = value;
            break;
        case SPRITE_PROP_TRANSLATE_Y:
            modifier.translate_y = value;
            break;
        case SPRITE_PROP_SKEW_X:
            modifier.skew_x = value;
            break;
        case SPRITE_PROP_SKEW_Y:
            modifier.skew_y = value;
            break;
        case SPRITE_PROP_ROTATE_PIVOT_ENABLED:
            modifier.rotate_pivot_enabled = value;
            break;
        case SPRITE_PROP_ROTATE_PIVOT_U:
            modifier.rotate_pivot_u = value;
            break;
        case SPRITE_PROP_ROTATE_PIVOT_V:
            modifier.rotate_pivot_v = value;
            break;
        case SPRITE_PROP_SCALE_DIRECTION_X:
            modifier.scale_direction_x = value;
            break;
        case SPRITE_PROP_SCALE_DIRECTION_Y:
            modifier.scale_direction_y = value;
            break;
        case SPRITE_PROP_TRANSLATE_ROTATION:
            modifier.translate_rotation = value;
            break;
        case SPRITE_PROP_SCALE_SIZE:
            modifier.scale_size = value;
            break;
        case SPRITE_PROP_SCALE_TRANSLATION:
            modifier.scale_translation = value >= 1.0;
            break;
    }
}

function pvrctx_helper_copy_modifier(modifier_source, modifier_dest) {
    for (const field in modifier_source) {
        if (Number.isNaN(modifier_source[field])) continue;
        modifier_dest[field] = modifier_source[field];
    }
}

function pvrctx_is_widescreen() {
    return !math2d_floats_are_near_equal(640 / 480, pvr_context.screen_stride / pvr_context.screen_height);
}

document.addEventListener("DOMContentLoaded", async function (e) {
    let url_search_params = new URLSearchParams(window.location.search);

    let path_name = location.pathname;
    let idx = path_name.lastIndexOf("/");
    if (idx >= 0) path_name = path_name.substring(idx + 1);
    pvr_launch_args.push(path_name);

    for (const [key, value] of url_search_params) {
        if (key != null) pvr_launch_args.push(`-${key}`);
        pvr_launch_args.push(value);
    }

    let canvas = document.querySelector('canvas');

    /** @ts-ignore */
    PVR_STATUS = document.getElementById("pvr-status");
    PVR_WIDTH = canvas.width;
    PVR_HEIGHT = canvas.height;
    PVR_WIDTH_WS = parseInt(canvas.getAttribute("width2"));
    PVR_HEIGHT_WS = parseInt(canvas.getAttribute("height2"));

    // @ts-ignore
    if ("main_layout_visor" in window && document.getElementById("widescreen").checked) {
        canvas.width = PVR_WIDTH_WS;
        canvas.height = PVR_HEIGHT_WS;
    } else if (SETTINGS.storage_load_boolean(SETTINGS.INI_MISC_SECTION, "widescreen", false)) {
        canvas.width = PVR_WIDTH_WS;
        canvas.height = PVR_HEIGHT_WS;
    }

    pvr_context = new PVRContext(canvas);
    await pvr_context._initWebGL();

    PVR_STATUS.value = "";
    pvr_context_clear_screen(pvr_context, PVR_CLEAR_COLOR);

    if ("matchMedia" in window) pvr_update_devicePixelRatio();

    document.getElementById("pvr-onoff").addEventListener("click", pvr_onoff, false);
    document.getElementById("pvr-fullscreen")?.addEventListener("click", pvr_fullscreen, false);
    document.getElementById("pvr-widescreen")?.addEventListener("click", pvr_widescreen, false);
    //document.addEventListener("keypress", pvr_onoff);

    document.addEventListener("fullscreenchange", pvr_update_devicePixelRatio);
    pvr_cursor_hidder();

    if (PVR_AUTORUN)
        pvr_onoff(e);
    /*else
        pvr_draw_top_text("(PVR is not running)");*/
}, false);

