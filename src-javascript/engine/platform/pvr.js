/**
 * Sega Dreamcast PowerVR graphics processor backend
 */
"use strict";

const PVR_FPS_WIDTH = 25;
/**@type {RGBA}*/ const PVRCTX_CLEAR_COLOR = [0.5, 0.5, 0.5, 1.0];

const PVR_AUTORUN = 1;
const PVR_DREAMCAST_VRAM_SIZE = 1024 * 1024 * 8;
/**@type {RGBA}*/ const PVRCTX_DEFAULT_OFFSET_COLOR = [0.0, 0.0, 0.0, -1.0];

const PVR_FRAME_TIME = 0;//1000.0 / 60;
const PVR_AUTOHIDE_CURSOR_MILLISECONDS = 3000;
var pvr_last_timestamp = -1;
var pvr_total_elapsed = 0;
var pvr_total_frames = 0;
var pvr_cur_frames = 0;
var pvr_frame_next = 1000;
var pvr_suspended = 0;
var pvr_callback_resume = null;
var /**@type {MediaQueryList}*/pvr_last_match_media = null;
var /**@type {string[]}*/ pvr_launch_args = new Array();

var /** @type {HTMLInputElement} */PVR_STATUS = null;


const VERTEX_NONE = -1;
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

            for (let i = 0; i < KOS_VBLANK_HANDLERS.length; i++) {
                let handler = KOS_VBLANK_HANDLERS[i];
                if (handler) {
                    try {
                        handler.hnd(ASIC_EVT_PVR_VBLANK_BEGIN, handler.data);
                    } catch (e) {
                        console.error(e);
                    }
                }
            }

            let elapsed;
            if (pvr_last_timestamp < 0)
                elapsed = 0.0;
            else
                elapsed = now - pvr_last_timestamp;

            pvr_last_timestamp = now;

            //pvr_context_clear_screen(pvr_context, null);
            //pvr_context_clear_screen(pvr_context, "gray");

            pvr_show_fps(elapsed);

            //
            // Notes:
            //          * the engine should not run below 15fps, or the beat synchronization will be lost.
            //          * set elapsed to zero is bigger than 200ms, this happen between load screens,
            //          * do not fire threshold twice, check if last elapsed value was zero.
            //
            if (elapsed >= 200.0 && pvr_context.last_elapsed != 0.0)
                elapsed = 0.0;
            else if (elapsed > 66.66666666666667)
                elapsed = 66.66666666666667;

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


function pvr_context_helper_draw_vertex(vertex_type, vertex_holder) {
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
        pvr_context_helper_draw_vertex(buffer[j].type, buffer[j].holder);
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
    } else if (e && e.target && e.target.blur) {
        e.target.blur();
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

    SETTINGS.widescreen = !SETTINGS.widescreen;
    SETTINGS.storage_save_boolean(SETTINGS.INI_MISC_SECTION, "widescreen", SETTINGS.widescreen);

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
    } else if (SETTINGS.widescreen) {
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

    pvr_context.screen_stride = pvr_context.screen_width;
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

function pvr_context_helper_clear_modifier(modifier) {
    const BASE_MODIFIER = {
        translate_x: 0.0,
        translate_y: 0.0,

        rotate: 0.0,

        skew_x: 0.0,
        skew_y: 0.0,

        scale_x: 1.0,
        scale_y: 1.0,

        scale_direction_x: 0.0,
        scale_direction_y: 0.0,

        rotate_pivot_enabled: false,
        rotate_pivot_u: 0.5,
        rotate_pivot_v: 0.5,

        translate_rotation: false,
        scale_size: false,
        scale_translation: false,

        x: 0.0,
        y: 0.0,
        width: -1.0,
        height: -1.0
    };

    for (const field in BASE_MODIFIER) modifier[field] = BASE_MODIFIER[field];
}

function pvr_context_helper_clear_offsetcolor(offsetcolor) {
    offsetcolor[0] = 0.0;
    offsetcolor[1] = 0.0;
    offsetcolor[2] = 0.0;
    offsetcolor[3] = -1.0;
}

function pvr_context_helper_set_modifier_property(modifier, property, value) {
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

function pvr_context_helper_copy_modifier(modifier_source, modifier_dest) {
    for (const field in modifier_source) {
        if (Number.isNaN(modifier_source[field])) continue;
        modifier_dest[field] = modifier_source[field];
    }
}

function pvr_context_is_widescreen() {
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
    PVR_STATUS.value = "";

    document.getElementById("pvr-onoff").addEventListener("click", pvr_onoff, false);
    document.getElementById("pvr-fullscreen")?.addEventListener("click", pvr_fullscreen, false);
    document.getElementById("pvr-widescreen")?.addEventListener("click", pvr_widescreen, false);
    //document.addEventListener("keypress", pvr_onoff);

    document.addEventListener("fullscreenchange", pvr_update_devicePixelRatio);

    if (PVR_AUTORUN) pvr_call_entry_point();
}, false);

async function pvr_call_entry_point() {
    // emulate main thread
    kos_thd_id++;

    // read the entry point
    let /**@type {HTMLInputElement} */ entry_point_input = document.querySelector("input[name=entry_point]");
    let /**@type {function(number,string[]):void}*/entry_point = window[entry_point_input.value];

    //
    // prepare startup
    //
    let expansion_directory = null;

    // adquire the executable name and arguments
    let args = pvr_launch_args;

    // load ini file containing all settings
    SETTINGS.reload();

    // parse command line arguments
    for (let i = 1; i < pvr_launch_args.length; i++) {
        let next_argument = i + i;
        let is_last_argument = next_argument >= pvr_launch_args.length;


        switch (args[i].toLowerCase()) {
            case "-expansionsloader":
                expansion_directory = await expansionsloader_main();
                if (!expansion_directory) continue;

                if (expansion_directory != null) {
                    console.info(`Selected expansion: ${expansion_directory}`);
                }
                break;
            case "-expansion":
                if (is_last_argument) continue;
                expansion_directory = args[next_argument];
                if (!expansion_directory) {
                    expansion_directory = null;
                    break;
                }
                if (expansion_directory.toLowerCase() == "funkin") {
                    console.info("'/expansions/funkin' is always applied");
                    expansion_directory = null;
                }
                break;
            case "-style":
                if (is_last_argument || !args[next_argument]) continue;
                SETTINGS.style_from_week_name = strdup(args[next_argument]);
                break;
            case "-fullscreen":
                SETTINGS.fullscreen = true;
                break;
            case "-nowidescreen":
                SETTINGS.widescreen = false;
                break;
            case "-layoutdebugtriggercalls":
                LAYOUT_DEBUG_PRINT_TRIGGER_CALLS = true;
                break;
            case "-saveslots":
                if (is_last_argument) continue;
                SETTINGS.saveslots = parseInt(args[next_argument]);
                if (!Number.isFinite(SETTINGS.saveslots)) SETTINGS.saveslots = 1;
                break;
            case "-h":
            case "-help":
            case "--help":
            case "/help":
            case "/h":
                let help =
                    `${ENGINE_NAME} ${ENGINE_VERSION}` +
                    "\r\n" +
                    `    ${args[0]} [-help] [-saveslots #] [-expansion FOLDER_NAME] [-style WEEK_NAME] [-fullscreen] [-nowidescreen] [-console] [-expansionsloader] [-layoutdebugtriggercalls]\r\n` +
                    "\r\n" +
                    "Options:\r\n" +
                    "    -help                      Show this help message\r\n" +
                    "    -saveslots                 Number of emulated VMU (Visual Memory Card) availabe. Defaults to 1\r\n" +
                    "    -expansion                 Folder name inside of '/expansions' folder, this overrides the '/assets' folder contents. Disabled by default\r\n" +
                    "    -style                     Week name (folder name inside of '/assets/weeks' folder) and picks the folder '/assets/weeks/WEEK_NAME/custom' or the defined in 'about.json' file. Defaults to the last played week\r\n" +
                    "    -fullscreen                Starts the engine in fullscreen, toggle to windowed pressing 'F11' key. Defaults to windowed\r\n" +
                    "    -nowidescreen              Forces the 4:3 aspect ratio like in the dreamcast. Defaults to 16:9, but changes if resized\r\n" +
                    "    -console                   Opens a console window for all engine and lua scripts messages\r\n" +
                    "    -expansionloader           Opens a window to choose the expansion to use\r\n" +
                    "    -layoutdebugtriggercalls   Prints in console all layout_triger_***() calls\r\n" +
                    "\r\n" +
                    "Notes:\r\n" +
                    "  -nowidescreen uses the 640x480 window size, otherwise defaults to 950x540. Anyways, the window still can be resized.\r\n" +
                    "  in -saveslots, the maximum is 8 VMUS, and the names are a1, b1, c1, d1, a3, b5, etc...\r\n" +
                    "  the folder '/expansions/funkin' is always applied, place modifications here if you do not want to messup '/assets' contents\r\n" +
                    "  the files override order is:   '<requested file in assets>' --> '/assets/weeks/WEEK_NAME/custom_commons' --> '/expansions/EXPANSION_NAME' --> '/expansions/funkin' --> '/assets/*'\r\n" +
                    "  shaders under '/assets/shaders' can not be overrided (for now).\n" +
                    "  -style only overrides '/assets/common' folder while expansions overrides everything under '/assets'\r\n" +
                    "\r\n";
                console.info(help);
                alert(help);
                return 0;
            default:
                continue;
        }
        i++;
    }

    //Initialize WASM versions of Lua and FontAtlas
    await main_initialize_wasm_modules();

    // load selected expansion or default ("funkin" folder)
    await expansions_load(expansion_directory);

    //await www_autoplay();
    await pvr_context_init();
    pvr_cursor_hidder();
    pvr_update_devicePixelRatio();

    // now run the engine
    setTimeout(entry_point, 0, pvr_launch_args.length, pvr_launch_args);
}

