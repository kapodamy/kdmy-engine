"use strict";

//
// Public constants
//
const LAYOUT_TYPE_NOTFOUND = 0x00;
const LAYOUT_TYPE_STRING = 0x01;
const LAYOUT_TYPE_FLOAT = 0x02;
const LAYOUT_TYPE_INTEGER = 0x04;
const LAYOUT_TYPE_HEX = 0x08;// unsigned integer
const LAYOUT_TYPE_BOOLEAN = 0x10;


const LAYOUT_ACTION_LOCATION = 0;
const LAYOUT_ACTION_SIZE = 1;
const LAYOUT_ACTION_ANIMATION = 2;
const LAYOUT_ACTION_ANIMATIONREMOVE = 3;
const LAYOUT_ACTION_OFFSETCOLOR = 4;
const LAYOUT_ACTION_COLOR = 5;
const LAYOUT_ACTION_PROPERTY = 6;
const LAYOUT_ACTION_ATLASAPPLY = 7;
const LAYOUT_ACTION_VISIBILITY = 8;
const LAYOUT_ACTION_MODIFIER = 9;
const LAYOUT_ACTION_PARALLAX = 10;
const LAYOUT_ACTION_TEXTBORDER = 11;
const LAYOUT_ACTION_RESIZE = 12;
const LAYOUT_ACTION_RESETMATRIX = 13;
const LAYOUT_ACTION_STATIC = 14;
const LAYOUT_ACTION_ANIMATIONEND = 15;
const LAYOUT_ACTION_EXECUTE = 16;
const LAYOUT_ACTION_PUPPETITEM = 17;
const LAYOUT_ACTION_PUPPETGROUP = 18;
const LAYOUT_ACTION_SETSHADER = 19;
const LAYOUT_ACTION_REMOVESHADER = 20;
const LAYOUT_ACTION_SETSHADERUNIFORM = 22;
const LAYOUT_ACTION_SETBLENDING = 23;
const LAYOUT_ACTION_VIEWPORT = 24;
const LAYOUT_ACTION_TEXTBORDEROFFSET = 25;
const LAYOUT_ACTION_SPRITE_TRAILING = 26;
const LAYOUT_ACTION_SPRITE_TRAILINGOFFSETCOLOR = 27;
const LAYOUT_ACTION_TEXTBACKGROUND = 28;
const LAYOUT_ACTION_TEXTBACKGROUNDCOLOR = 29;
const LAYOUT_ACTION_SOUNDFADE = 30;
const LAYOUT_ACTION_SEEK = 31;

const LAYOUT_GROUP_ROOT = Symbol("root-group");
const LAYOUT_BPM_STEPS = 32;// 1/32 beats
var LAYOUT_DEBUG_PRINT_TRIGGER_CALLS = false;


async function layout_init(src) {
    let xml;
    try {
        xml = await fs_readxml(src);
    } catch (e) {
        console.error("layout_init() error reading: " + src, e);
        return null;
    }

    // change the current working folder but remember the old one
    fs_folder_stack_push();
    fs_set_working_folder(src, 1);

    let layout_context = {
        animlist: null,
        vertex_list: arraylist_init(),
        group_list: arraylist_init(),
        camera_list: arraylist_init(),
        trigger_list: arraylist_init(),
        sound_list: arraylist_init(),
        video_list: arraylist_init(),
        resource_pool: { textures: arraylist_init(), atlas: arraylist_init() },
        fonts: null, fonts_size: -1,
        values: null, values_size: -1,
        macro_list: null, macro_list_size: 0
    };

    let root = xml.querySelector("Layout");

    // step 1: read attributes of the root
    let viewport_width = layout_helper_parse_float(root, "width", -1);
    let viewport_height = layout_helper_parse_float(root, "height", -1);
    let animation_speed = layout_helper_parse_float(root, "animationSpeed", 1.0);
    let keep_aspect_ratio = vertexprops_parse_boolean(root, "keepAspectRatio", 0);

    if (viewport_width < 1) throw new Error("Invalid/missing layout width");
    if (viewport_height < 1) throw new Error("Invalid/missing layout height");

    let animlist_filename = root.getAttribute("animationList");
    if (animlist_filename) {
        layout_context.animlist = await animlist_init(animlist_filename);
    }

    // step 2.1: read fonts
    await layout_parse_fonts(root, layout_context);

    // step 2.2: import all external values
    await layout_parse_externalvalues(root, layout_context);

    // step 2.3: import macros
    await layout_parse_macro(root, layout_context);

    // step 3: import all groups (including sprites, texts and placeholders)
    await layout_parse_group(root, layout_context, null);


    // step 4: build the layout object
    let layout = {
        animation_speed: animation_speed,

        camera_helper: null,
        camera_secondary_helper: null,

        fonts: layout_context.fonts,
        fonts_size: layout_context.fonts_size,

        values: layout_context.values,
        values_size: layout_context.values_size,

        macro_list: layout_context.macro_list,
        macro_list_size: layout_context.macro_list_size,

        keep_aspect_ratio: keep_aspect_ratio,

        viewport_width: viewport_width,
        viewport_height: viewport_height,

        modifier_camera: {},
        modifier_camera_secondary: {},
        modifier_viewport: {},

        vertex_list: null,
        vertex_list_size: -1,

        group_list: null,
        group_list_size: -1,

        camera_list: null,
        camera_list_size: -1,

        trigger_list: null,
        trigger_list_size: -1,

        sound_list: null,
        sound_list_size: -1,

        video_list: null,
        video_list_size: -1,

        textures: null,
        textures_size: 0,

        z_buffer: null,
        z_buffer_size: 0,

        external_vertex_list: null,
        external_vertex_list_size: 0,

        single_item: null,
        suspended: 0,

        beatwatcher_synced_triggers: 0,
        beatwatcher_last_timestamp: 0,
        beatwatcher_resyncs_count: 0,
        beatwatcher: {},

        antialiasing_disabled: 0,
        resolution_changes: 0,

        psshader: null
    };

    // step 5: build modifiers
    pvrctx_helper_clear_modifier(layout.modifier_camera);
    pvrctx_helper_clear_modifier(layout.modifier_camera_secondary);
    pvrctx_helper_clear_modifier(layout.modifier_viewport);

    layout.modifier_camera.width = viewport_width;
    layout.modifier_camera.height = viewport_height;
    layout.camera_helper = camera_init(layout.modifier_camera, viewport_width, viewport_height);
    layout.camera_secondary_helper = camera_init(layout.modifier_camera_secondary, viewport_width, viewport_height);

    layout.modifier_viewport.x = 0;
    layout.modifier_viewport.y = 0;
    layout.modifier_viewport.width = layout.viewport_width;
    layout.modifier_viewport.height = layout.viewport_height;

    // step 6: build vertex and group arrays
    arraylist_destroy2(layout_context.vertex_list, layout, "vertex_list_size", "vertex_list");
    arraylist_destroy2(layout_context.group_list, layout, "group_list_size", "group_list");
    arraylist_destroy2(layout_context.camera_list, layout, "camera_list_size", "camera_list");
    arraylist_destroy2(layout_context.trigger_list, layout, "trigger_list_size", "trigger_list");
    arraylist_destroy2(layout_context.sound_list, layout, "sound_list_size", "sound_list");
    arraylist_destroy2(layout_context.video_list, layout, "video_list_size", "video_list");

    // step 7: build textures array
    layout.textures_size = arraylist_size(layout_context.resource_pool.textures);
    layout.textures = new Array(layout.textures_size);

    let list = arraylist_peek_array(layout_context.resource_pool.textures);
    for (let i = 0; i < layout.textures_size; i++) {
        layout.textures[i] = list[i].data;
        list[i].src = undefined;
    }

    // step 8: build z-buffer
    layout.z_buffer = new Array(layout.vertex_list_size);
    layout.z_buffer_size = layout.vertex_list_size;
    layout_helper_zbuffer_build(layout);

    // step 9: cleanup
    for (let definition of arraylist_iterate4(layout_context.resource_pool.atlas)) {
        if (definition.data) atlas_destroy(definition.data);
        definition.src = undefined;
    }

    arraylist_destroy(layout_context.resource_pool.textures, 0);
    arraylist_destroy(layout_context.resource_pool.atlas, 0);

    if (layout_context.animlist) animlist_destroy(layout_context.animlist);

    // configure layout viewport with the PVR's render size (screen resolution)
    layout.resolution_changes = pvr_context.resolution_changes;
    layout_update_render_size(layout, pvr_context.screen_width, pvr_context.screen_height);

    // step 10: execute all default actions (actions with no names) and initial actions
    for (let i = 0; i < layout.vertex_list_size; i++) {
        let initial_action_name = layout.vertex_list[i].initial_action_name;
        let intial_action_found = initial_action_name == null;

        for (let j = 0; j < layout.vertex_list[i].actions_size; j++) {
            let action_name = layout.vertex_list[i].actions[j].name;

            if (action_name == null || action_name == initial_action_name) {
                if (action_name == initial_action_name) intial_action_found = true;

                layout_helper_execute_action(
                    layout, layout.vertex_list[i], layout.vertex_list[i].actions[j]
                );
            }
        }

        if (!intial_action_found)
            console.warn("layout_init() initial action not found" + initial_action_name);
    }

    for (let i = 0; i < layout.group_list_size; i++) {
        let initial_action_name = layout.group_list[i].initial_action_name;
        let intial_action_found = initial_action_name == null;

        for (let j = 0; j < layout.group_list[i].actions_size; j++) {
            let action_name = layout.group_list[i].actions[j].name;

            if (action_name == null || action_name == initial_action_name) {
                if (action_name == initial_action_name) intial_action_found = true;

                layout_helper_execute_action_in_group(
                    layout.group_list[i].actions[j], layout.group_list[i]
                );
            }

        }

        if (!intial_action_found)
            console.warn("layout_init() initial action not found" + initial_action_name);
    }

    for (let i = 0; i < layout.sound_list_size; i++) {
        let initial_action_name = layout.sound_list[i].initial_action_name;
        let intial_action_found = initial_action_name == null;

        for (let j = 0; j < layout.sound_list[i].actions_size; j++) {
            let action_name = layout.sound_list[i].actions[j].name;

            if (action_name == null || action_name == initial_action_name) {
                if (action_name == initial_action_name) intial_action_found = true;

                layout_helper_execute_action_in_sound(
                    layout.sound_list[i].actions[j], layout.sound_list[i]
                );
            }

        }

        if (!intial_action_found)
            console.warn("layout_init() initial action not found" + initial_action_name);
    }

    for (let i = 0; i < layout.video_list_size; i++) {
        let initial_action_name = layout.video_list[i].initial_action_name;
        let intial_action_found = initial_action_name == null;

        for (let j = 0; j < layout.video_list[i].actions_size; j++) {
            let action_name = layout.video_list[i].actions[j].name;

            if (action_name == null || action_name == initial_action_name) {
                if (action_name == initial_action_name) intial_action_found = true;

                const item_sprite = layout.vertex_list[layout.video_list[i].in_vertex_list_index];
                layout_helper_execute_action_in_video(
                    layout.video_list[i].actions[j], layout.video_list[i],
                    item_sprite, layout.viewport_width, layout.viewport_height
                );
            }

        }

        if (!intial_action_found)
            console.warn("layout_init() initial action not found" + initial_action_name);
    }

    for (let i = 0; i < layout.trigger_list_size; i++) {
        if (layout.trigger_list[i].name != null) continue;
        layout_helper_execute_trigger(layout, layout.trigger_list[i]);
    }

    // only is possible to trigger the first null-named camera
    camera_from_layout(layout.camera_helper, layout, null);
    camera_set_parent_layout(layout.camera_helper, layout);
    camera_set_parent_layout(layout.camera_secondary_helper, layout);

    // set default beats per second
    let bpm = layout_helper_parse_float(root, "defaultBPM", 100);
    beatwatcher_reset(layout.beatwatcher, 1, bpm);
    layout_set_bpm(layout, bpm);

    // restore previous working folder
    fs_folder_stack_pop();

    return layout;
}

function layout_destroy(layout) {

    for (let i = 0; i < layout.vertex_list_size; i++) {
        layout_helper_destroy_actions(layout.vertex_list[i].actions, layout.vertex_list[i].actions_size);
        layout.vertex_list[i].name = undefined;
        layout.vertex_list[i].initial_action_name = undefined;

        if (layout.vertex_list[i].videoplayer) continue;

        switch (layout.vertex_list[i].type) {
            case VERTEX_SPRITE:
                sprite_destroy(layout.vertex_list[i].vertex);
                break;
            case VERTEX_DRAWABLE:
                layout.vertex_list[i].placeholder.vertex = null;// external drawable ¡DO NOT DISPOSE!
                layout.vertex_list[i].placeholder = undefined;
                break;
            case VERTEX_TEXTSPRITE:
                textsprite_destroy(layout.vertex_list[i].vertex);
                break;
        }
    }
    layout.vertex_list = undefined;

    for (let i = 0; i < layout.values_size; i++) {
        if (layout.values[i].type == LAYOUT_TYPE_STRING) layout.values[i].misc = undefined;
        layout.values[i].name = undefined;
    }
    layout.values = undefined;

    for (let i = 0; i < layout.fonts_size; i++) {
        layout.fonts[i].name = undefined;
        fontholder_destroy(layout.fonts[i].fontholder);
    }
    layout.fonts = undefined;

    for (let i = 0; i < layout.textures_size; i++) {
        texture_destroy(layout.textures[i]);
    }
    layout.textures = undefined;

    for (let i = 0; i < layout.group_list_size; i++) {
        layout.group_list[i].name = undefined;
        layout.group_list[i].initial_action_name = undefined;
        layout_helper_destroy_actions(layout.group_list[i].actions, layout.group_list[i].actions_size);
        if (layout.group_list[i].psframebuffer) layout.group_list[i].psframebuffer.Destroy();
    }
    layout.group_list = undefined;

    for (let i = 0; i < layout.camera_list_size; i++) {
        layout.camera_list[i].name = undefined;
        if (layout.camera_list[i].animation) animsprite_destroy(layout.camera_list[i].animation);
    }
    layout.camera_list = undefined;

    for (let i = 0; i < layout.trigger_list_size; i++) {
        layout.trigger_list[i].name = undefined;
        layout.trigger_list[i].initial_action_name = undefined;
        layout.trigger_list[i].action_name = undefined;
        layout.trigger_list[i].camera_name = undefined;
        layout.trigger_list[i].trigger_name = undefined;
        layout.trigger_list[i].stop_trigger_name = undefined;

    }
    layout.trigger_list = undefined;

    for (let i = 0; i < layout.sound_list_size; i++) {
        layout_helper_destroy_actions(layout.sound_list[i].actions, layout.sound_list[i].actions_size);
        layout.sound_list[i].name = undefined;
        layout.sound_list[i].initial_action_name = undefined;
        soundplayer_destroy(layout.sound_list[i].soundplayer);

    }
    layout.sound_list = undefined;

    for (let i = 0; i < layout.video_list_size; i++) {
        layout_helper_destroy_actions(layout.video_list[i].actions, layout.video_list[i].actions_size);
        layout.video_list[i].name = undefined;
        layout.video_list[i].initial_action_name = undefined;
        videoplayer_destroy(layout.video_list[i].videoplayer);

    }
    layout.video_list = undefined;

    for (let i = 0; i < layout.macro_list_size; i++) {
        for (let j = 0; j < layout.macro_list[i].actions_size; j++) {
            layout.macro_list[i].actions[j].target_name = undefined;
            layout.macro_list[i].actions[j].action_name = undefined;
            layout.macro_list[i].actions[j].trigger_name = undefined;
            layout.macro_list[i].actions[j].stop_trigger_name = undefined;
            layout.macro_list[i].actions[j].camera_name = undefined;
        }
        layout.macro_list[i].actions = undefined;
    }
    layout.macro_list = undefined;

    camera_destroy(layout.camera_helper);
    camera_destroy(layout.camera_secondary_helper);

    layout.external_vertex_list = undefined;
    layout.z_buffer = undefined;

    ModuleLuaScript.kdmyEngine_drop_shared_object(layout.modifier_camera);
    ModuleLuaScript.kdmyEngine_drop_shared_object(layout.modifier_camera_secondary);
    ModuleLuaScript.kdmyEngine_drop_shared_object(layout);
    layout = undefined;
}


function layout_trigger_any(layout, action_triger_camera_interval_name) {
    if (LAYOUT_DEBUG_PRINT_TRIGGER_CALLS) {
        console.log(`layout_trigger_any() target='{action_triger_camera_interval_name}'`);
    }
    let res = 0;
    res += layout_trigger_action(layout, null, action_triger_camera_interval_name);
    res += layout_trigger_camera(layout, action_triger_camera_interval_name) ? 1 : 0;
    res += layout_trigger_trigger(layout, action_triger_camera_interval_name);

    if (LAYOUT_DEBUG_PRINT_TRIGGER_CALLS) {
        console.log(`layout_trigger_any() target='{action_triger_camera_interval_name}' result={res}`);
    }

    return res;
}

function layout_trigger_action(layout, target_name, action_name) {
    let count = 0;
    let initial_action_name;

    if (LAYOUT_DEBUG_PRINT_TRIGGER_CALLS) {
        console.log(`layout_trigger_action() target='{target_name}' action='{action_name}'`);
    }

    for (let i = 0; i < layout.vertex_list_size; i++) {
        if (target_name != null && layout.vertex_list[i].name != target_name) continue;

        if (action_name == null && layout.vertex_list[i].initial_action_name != null)
            initial_action_name = layout.vertex_list[i].initial_action_name;
        else
            initial_action_name = action_name;

        for (let j = 0; j < layout.vertex_list[i].actions_size; j++) {
            let action = layout.vertex_list[i].actions[j];

            if (action.name == initial_action_name || action.name == action_name) {
                layout_helper_execute_action(layout, layout.vertex_list[i], action);
                count++;
            }
        }
    }

    for (let i = 0; i < layout.group_list_size; i++) {
        if (target_name != null && layout.group_list[i].name != target_name) continue;

        for (let j = 0; j < layout.group_list[i].actions_size; j++) {
            let action = layout.group_list[i].actions[j];

            if (action_name == null && layout.group_list[i].initial_action_name != null)
                initial_action_name = layout.group_list[i].initial_action_name;
            else
                initial_action_name = action_name;

            if (action.name == initial_action_name || action.name == action_name) {
                layout_helper_execute_action_in_group(action, layout.group_list[i]);
                count++;
            }
        }
    }

    for (let i = 0; i < layout.sound_list_size; i++) {
        if (target_name != null && layout.sound_list[i].name != target_name) continue;

        for (let j = 0; j < layout.sound_list[i].actions_size; j++) {
            let action = layout.sound_list[i].actions[j];

            if (action_name == null && layout.sound_list[i].initial_action_name != null)
                initial_action_name = layout.sound_list[i].initial_action_name;
            else
                initial_action_name = action_name;

            if (action.name == initial_action_name || action.name == action_name) {
                layout_helper_execute_action_in_sound(action, layout.sound_list[i]);
                count++;
            }
        }
    }

    for (let i = 0; i < layout.video_list_size; i++) {
        if (target_name != null && layout.video_list[i].name != target_name) continue;

        for (let j = 0; j < layout.video_list[i].actions_size; j++) {
            let action = layout.video_list[i].actions[j];

            if (action_name == null && layout.video_list[i].initial_action_name != null)
                initial_action_name = layout.video_list[i].initial_action_name;
            else
                initial_action_name = action_name;

            if (action.name == initial_action_name || action.name == action_name) {
                const item_video = layout.video_list[i];
                const item_sprite = layout.vertex_list[item_video.in_vertex_list_index];
                layout_helper_execute_action_in_video(
                    action, item_video,
                    item_sprite, layout.viewport_width, layout.viewport_height
                );
                count++;
            }
        }
    }

    if (LAYOUT_DEBUG_PRINT_TRIGGER_CALLS) {
        console.log(`layout_trigger_action() target='{target_name}' action='{action_name}' res={count}`);
    }

    return count;
}

function layout_contains_action(layout, target_name, action_name) {
    if (action_name == null) throw new Error("action_name is required");

    let count = 0;

    for (let i = 0; i < layout.vertex_list_size; i++) {
        if (layout.vertex_list[i].name != target_name) continue;
        for (let j = 0; j < layout.vertex_list[i].actions_size; j++) {
            if (layout.vertex_list[i].actions[j].name == action_name) count++;
        }
    }

    for (let i = 0; i < layout.group_list_size; i++) {
        if (layout.group_list[i].name != target_name) continue;
        for (let j = 0; j < layout.group_list[i].actions_size; j++) {
            if (layout.group_list[i].actions[j].name == action_name) count++;
        }
    }

    for (let i = 0; i < layout.sound_list_size; i++) {
        if (layout.sound_list[i].name != target_name) continue;
        for (let j = 0; j < layout.sound_list[i].actions_size; j++) {
            if (layout.sound_list[i].actions[j].name == action_name) count++;
        }
    }

    for (let i = 0; i < layout.video_list_size; i++) {
        if (layout.video_list[i].name != target_name) continue;
        for (let j = 0; j < layout.video_list[i].actions_size; j++) {
            if (layout.video_list[i].actions[j].name == action_name) count++;
        }
    }

    return count;
}

function layout_animation_is_completed(layout, item_name) {
    if (item_name == null) throw new Error("entry_name is required");

    let type = -1;
    let vertex = null;
    let animsprite = null;

    for (let i = 0; i < layout.vertex_list_size; i++) {
        if (layout.vertex_list[i].name == item_name) {
            type = layout.vertex_list[i].type;
            vertex = layout.vertex_list[i].vertex;
            animsprite = layout.vertex_list[i].animation;
            break;
        }
    }

    if (!vertex) {
        // check if a group with this name exists
        for (let i = 0; i < layout.group_list_size; i++) {
            if (layout.group_list[i].name == item_name) {
                vertex = null;
                animsprite = layout.vertex_list[i].animation;
                break;
            }
        }
    }

    if (!vertex && !animsprite) return 2;

    if (animsprite != null) {
        if (animsprite_is_completed(animsprite)) return 1;
    } else {
        // call *_animate() to check if was completed.
        let ret = 0;
        switch (type) {
            case VERTEX_DRAWABLE:
                ret = drawable_animate(vertex, 0.0);
                break;
            case VERTEX_SPRITE:
                ret = sprite_animate(vertex, 0.0);
                break;
            case VERTEX_STATESPRITE:
                ret = statesprite_animate(vertex, 0.0);
                break;
            case VERTEX_TEXTSPRITE:
                ret = textsprite_animate(vertex, 0.0);
                break;
            default:
                return -1;
        }

        if (ret > 0) return 1;
    }

    return 0;
}

function layout_update_render_size(layout, screen_width, screen_height) {
    if (layout.keep_aspect_ratio) {
        let scale_x = screen_width / layout.viewport_width;
        let scale_y = screen_height / layout.viewport_height;
        let scale = Math.min(scale_x, scale_y);

        layout.modifier_viewport.scale_x = layout.modifier_viewport.scale_y = scale;
        layout.modifier_viewport.translate_x = (screen_width - layout.viewport_width * scale) / 2;
        layout.modifier_viewport.translate_y = (screen_height - layout.viewport_height * scale) / 2;
    } else {
        layout.modifier_viewport.scale_x = screen_width / layout.viewport_width;
        layout.modifier_viewport.scale_y = screen_height / layout.viewport_height;
        layout.modifier_viewport.translate_x = layout.modifier_viewport.translate_y = 0;
    }
}

function layout_screen_to_layout_coordinates(layout, screen_x, screen_y, calc_with_camera, output_coords) {
    // screen aspect ratio correction
    screen_x -= pvr_context.screen_stride - pvr_context.screen_width;

    const temp = new Float32Array(SH4MATRIX_SIZE);
    sh4matrix_clear(temp);
    sh4matrix_apply_modifier(temp, layout.modifier_viewport);

    if (calc_with_camera) {
        sh4matrix_apply_modifier(temp, layout.modifier_camera_secondary);
        sh4matrix_apply_modifier(temp, layout.modifier_camera);
    }

    output_coords[0] = screen_x;
    output_coords[1] = screen_y;
    sh4matrix_multiply_point(temp, output_coords);
}

function layout_camera_set_view(layout, x, y, depth) {
    camera_set_absolute(layout.camera_helper, x, y, depth);
}

function layout_camera_is_completed(layout) {
    return camera_is_completed(layout.camera_helper);
}

function layout_trigger_camera(layout, camera_name) {
    if (LAYOUT_DEBUG_PRINT_TRIGGER_CALLS) {
        console.log(`layout_trigger_camera() target='{camera_name}'`);
    }
    return camera_from_layout(layout.camera_helper, layout, camera_name);
}

function layout_trigger_trigger(layout, trigger_name) {
    if (LAYOUT_DEBUG_PRINT_TRIGGER_CALLS) {
        console.log(`layout_trigger_trigger() target='{trigger_name}'`);
    }

    let count = 0;
    for (let i = 0; i < layout.trigger_list_size; i++) {
        if (layout.trigger_list[i].name == trigger_name) {
            layout_helper_execute_trigger(layout, layout.trigger_list[i]);
        }
    }

    if (LAYOUT_DEBUG_PRINT_TRIGGER_CALLS) {
        console.log(`layout_trigger_trigger() target='{trigger_name}' res={count}`);
    }

    return count;
}

function layout_stop_trigger(layout, trigger_name) {
    for (let i = 0; i < layout.trigger_list_size; i++) {
        if (layout.trigger_list[i].name == trigger_name)
            layout.trigger_list[i].context.running = 0;
    }
}

function layout_stop_all_triggers(layout) {
    for (let i = 0; i < layout.trigger_list_size; i++) {
        layout.trigger_list[i].context.running = 0;
    }
}

function layout_set_placeholder_drawable_by_id(layout, id, drawable) {
    if (id < 0 || id >= layout.vertex_list_size) return 0;
    if (layout.vertex_list[id].type != VERTEX_DRAWABLE) return 0;

    layout.vertex_list[id].placeholder.vertex = drawable;
    return 1;
}

function layout_sync_triggers_with_global_beatwatcher(layout, enable) {
    layout.beatwatcher_synced_triggers = !!enable;
    layout.beatwatcher.last_global_timestamp = -1;
    layout.beatwatcher.resyncs = 1;
}

function layout_set_bpm(layout, beats_per_minute) {
    let duration = math2d_beats_per_minute_to_beat_per_milliseconds(beats_per_minute);

    camera_set_bpm(layout.camera_helper, beats_per_minute);
    camera_set_bpm(layout.camera_secondary_helper, beats_per_minute);

    beatwatcher_change_bpm(layout.beatwatcher, beats_per_minute);

    for (let i = 0; i < layout.trigger_list_size; i++) {
        let trigger = layout.trigger_list[i];

        if (trigger.loop_delay_beats_in_beats) {
            trigger.loop_delay = trigger.loop_delay_beats * duration;
        }
        if (trigger.start_delay_beats_in_beats) {
            trigger.start_delay = trigger.start_delay_beats * duration;
        }
    }
}


function layout_get_placeholder(layout, name) {
    for (let i = 0; i < layout.vertex_list_size; i++) {
        if (layout.vertex_list[i].placeholder && layout.vertex_list[i].placeholder.name == name) {
            return layout.vertex_list[i].placeholder;
        }
    }
    return null;
}

function layout_get_placeholder_id(layout, name) {
    for (let i = 0; i < layout.vertex_list_size; i++) {
        if (layout.vertex_list[i].placeholder && layout.vertex_list[i].placeholder.name == name) {
            return i;
        }
    }
    return -1;
}


function layout_get_textsprite(layout, name) {
    return layout_helper_get_vertex(layout, VERTEX_TEXTSPRITE, name);
}

function layout_get_sprite(layout, name) {
    return layout_helper_get_vertex(layout, VERTEX_SPRITE, name);
}

function layout_get_soundplayer(layout, name) {
    for (let i = 0; i < layout.sound_list_size; i++) {
        if (layout.sound_list[i].name == name) {
            return layout.sound_list[i].soundplayer;
        }
    }
    return null;
}

function layout_get_videoplayer(layout, name) {
    for (let i = 0; i < layout.video_list_size; i++) {
        if (layout.video_list[i].name == name) {
            return layout.video_list[i].videoplayer;
        }
    }
    return null;
}

function layout_get_camera_helper(layout) {
    return layout.camera_helper;
}

function layout_get_secondary_camera_helper(layout) {
    return layout.camera_secondary_helper;
}


function layout_get_attached_font(layout, font_name) {
    if (font_name == null)
        return layout.fonts_size < 1 ? null : layout.fonts[0].fontholder;

    for (let i = 0; i < layout.fonts_size; i++) {
        if (layout.fonts[i].name == font_name)
            return layout.fonts[i].fontholder;
    }

    return null;
}

function layout_get_attached_value(layout, name, expected_type, default_value) {
    for (let i = 0; i < layout.values_size; i++) {
        if (layout.values[i].name != name) continue;

        if ((layout.values[i].type & expected_type) == 0x00) {
            console.warn("layout_get_attached_value() type missmatch of: " + name);

            if (expected_type == LAYOUT_TYPE_FLOAT && layout.values[i].type == LAYOUT_TYPE_INTEGER)
                return layout.values[i].value;

            break;// type missmatch
        }
        return layout.values[i].value;
    }
    //console.log("layout_get_attached_value() value not found: " + name);
    return default_value;
}

function layout_get_attached_value2(layout, name, result_ptr) {
    for (let i = 0; i < layout.values_size; i++) {
        if (layout.values[i].name != name) continue;

        result_ptr[0] = layout.values[i].value;
        return layout.values[i].type;
    }

    return LAYOUT_TYPE_NOTFOUND;
}

function layout_get_attached_value_as_float(layout, name, default_value) {
    const LIKE_NUMBER = LAYOUT_TYPE_INTEGER | LAYOUT_TYPE_FLOAT | LAYOUT_TYPE_HEX;

    for (let i = 0; i < layout.values_size; i++) {
        if (layout.values[i].name != name) continue;

        if ((layout.values[i].type & LIKE_NUMBER) == 0x00) break;// type missmatch

        // convert the value to float (IEEE 754)
        switch (layout.values[i].type) {
            case LAYOUT_TYPE_INTEGER:
                return parseFloat(layout.values[i].value);// JS only
            case LAYOUT_TYPE_FLOAT:
                return layout.values[i].value;
            case LAYOUT_TYPE_HEX:
                return parseFloat(layout.values[i].value);// JS only
        }
    }
    return default_value;
}

function layout_get_attached_value_type(layout, name) {
    for (let i = 0; i < layout.values_size; i++) {
        if (layout.values[i].name == name) return layout.values[i].type;
    }
    return LAYOUT_TYPE_NOTFOUND;
}

function layout_get_modifier_viewport(layout) {
    return layout.modifier_viewport;
}

function layout_get_modifier_camera(layout) {
    return layout.modifier_camera;
}

function layout_get_viewport_size(layout, size) {
    size[0] = layout.viewport_width;
    size[1] = layout.viewport_height;
    return size;
}

function layout_external_vertex_create_entries(layout, amount) {
    if (amount < 0) amount = 0;

    if (layout.external_vertex_list) {
        layout.external_vertex_list = realloc(layout.external_vertex_list, amount);
        if (!layout.external_vertex_list) throw new Error("not enough system memory");
    } else {
        layout.external_vertex_list = new Array(amount);
    }

    for (let i = layout.external_vertex_list_size; i < amount; i++) {
        layout.external_vertex_list[i] = {
            vertex: null,
            type: -1,
            group_id: 0,// layout root
            parallax: { x: 1.0, y: 1.0, z: 1.0 },
            static_camera: 0,
            placeholder: null
        };
    }
    layout.external_vertex_list_size = amount;

    layout.z_buffer_size = layout.vertex_list_size + layout.external_vertex_list_size;
    layout.z_buffer = realloc(layout.z_buffer, layout.z_buffer_size);
    if (!layout.z_buffer) throw new Error("layout_external_vertex_create_entries() not enough system memory");

    // re-build z-buffer
    layout_helper_zbuffer_build(layout);
}

function layout_external_vertex_set_entry(layout, index, vertex_type, vertex, group_id) {
    if (index < 0 || index >= layout.external_vertex_list_size) return 0;
    if (group_id < 0 || group_id >= layout.group_list_size) group_id = 0;
    layout.external_vertex_list[index].vertex = vertex;
    layout.external_vertex_list[index].type = vertex_type;
    layout.external_vertex_list[index].group_id = group_id;
    return 1;
}

function layout_external_create_group(layout, group_name, parent_group_id) {
    if (group_name != null) {
        let index = layout_helper_get_group_index(layout, group_name);
        if (index >= 0) {
            console.warn(`layout_external_create_group() the group '${group_name}' already exists`);
            return -1;
        }
    }

    if (parent_group_id < 0 || parent_group_id >= layout.group_list_size) {
        // layout root
        parent_group_id = 0;
    }

    // increase group_list size
    let group_id = layout.group_list_size++;
    layout.group_list = realloc(layout.group_list, layout.group_list_size);
    if (!layout.group_list) throw new Error("layout_external_create_group() out-of-memory");

    layout.group_list[group_id] = {
        name: strdup(group_name),
        group_id: parent_group_id,
        actions: null,
        actions_size: 0,
        initial_action_name: null,
        antialiasing: PVR_FLAG_DEFAULT,

        visible: 1,
        alpha: 1.0,
        alpha2: 1.0,
        offsetcolor: [],
        modifier: {},
        parallax: { x: 1.0, y: 1.0, z: 1.0 },
        static_camera: 0,
        static_screen: null,

        animation: null,
        psshader: null,
        psframebuffer: null,

        blend_enabled: 1,
        blend_src_rgb: BLEND_DEFAULT,
        blend_dst_rgb: BLEND_DEFAULT,
        blend_src_alpha: BLEND_DEFAULT,
        blend_dst_alpha: BLEND_DEFAULT,

        viewport_x: -1,
        viewport_y: -1,
        viewport_width: -1,
        viewport_height: -1,

        context: {
            visible: 1,
            alpha: 1.0,
            antialiasing: PVR_FLAG_DEFAULT,
            matrix: new Float32Array(SH4MATRIX_SIZE),
            offsetcolor: [],
            parallax: { x: 1.0, y: 1.0, z: 1.0 },

            next_child: null,
            next_sibling: null,
            parent_group: null,
            last_z_index: -1,
        }
    };

    // append to parent group
    layout_helper_add_group_to_parent(
        layout.group_list[parent_group_id].context, layout.group_list[group_id]
    );

    //sh4matrix_reset(layout.group_list[group_id].matrix);
    pvrctx_helper_clear_modifier(layout.group_list[group_id].modifier);
    pvrctx_helper_clear_offsetcolor(layout.group_list[group_id].offsetcolor);

    return group_id;
}

function layout_set_group_static_to_camera(layout, group_name, enable) {
    let index = layout_helper_get_group_index(layout, group_name);
    if (index < 0) return 0
    layout.group_list[index].static_camera = !!enable;
    return 1;
}

function layout_set_group_static_to_camera_by_id(layout, group_id, enable) {
    if (group_id < 0 || group_id >= layout.group_list_size) return 0
    layout.group_list[group_id].static_camera = !!enable;
    return 1;
}

function layout_set_group_static_to_screen_by_id(layout, group_id, sh4matrix) {
    if (group_id < 0 || group_id >= layout.group_list_size) return 0;
    layout.group_list[group_id].static_screen = sh4matrix;
    return 1;
}

function layout_external_vertex_set_entry_static(layout, vertex_index, enable) {
    if (vertex_index < 0 || vertex_index >= layout.external_vertex_list_size) return 0;
    layout.external_vertex_list[vertex_index].static_camera = enable;
    return 1;
}

function layout_set_group_visibility(layout, group_name, visible) {
    visible = !!visible;
    let index = layout_helper_get_group_index(layout, group_name);
    if (index >= 0) layout.group_list[index].visible = visible;
}

function layout_set_group_visibility_by_id(layout, group_id, visible) {
    visible = !!visible;
    if (group_id < 0 || group_id >= layout.group_list_size) return;
    layout.group_list[group_id].visible = visible;
}

function layout_set_group_alpha(layout, group_name, alpha) {
    if (!Number.isFinite(alpha)) alpha = 0.0;
    let index = layout_helper_get_group_index(layout, group_name);
    if (index >= 0) layout.group_list[index].alpha = math2d_clamp_float(alpha, 0.0, 1.0);
}

function layout_set_group_alpha_by_id(layout, group_id, alpha) {
    if (!Number.isFinite(alpha)) alpha = 0.0;
    if (group_id < 0 || group_id >= layout.group_list_size) return;
    layout.group_list[group_id].alpha = math2d_clamp_float(alpha, 0.0, 1.0);
}

function layout_set_group_antialiasing(layout, group_name, antialiasing) {
    let index = layout_helper_get_group_index(layout, group_name);
    if (index >= 0) layout.group_list[index].antialiasing = Math.trunc(antialiasing);
}

function layout_set_group_antialiasing_by_id(layout, group_id, antialiasing) {
    if (group_id < 0 || group_id >= layout.group_list_size) return;
    layout.group_list[group_id].antialiasing = Math.trunc(antialiasing);
}

function layout_set_group_offsetcolor(layout, group_name, r, g, b, a) {
    let index = layout_helper_get_group_index(layout, group_name);
    if (index >= 0) {
        if (Number.isFinite(r)) layout.group_list[index].offsetcolor[0] = math2d_clamp_float(r, 0.0, 1.0);
        if (Number.isFinite(g)) layout.group_list[index].offsetcolor[1] = math2d_clamp_float(g, 0.0, 1.0);
        if (Number.isFinite(b)) layout.group_list[index].offsetcolor[2] = math2d_clamp_float(b, 0.0, 1.0);
        if (Number.isFinite(a)) layout.group_list[index].offsetcolor[3] = math2d_clamp_float(a, 0.0, 1.0);
    }
}

function layout_get_group_modifier(layout, group_name) {
    let index = layout_helper_get_group_index(layout, group_name);
    return index < 0 ? null : layout.group_list[index].modifier;
}

function layout_get_group_modifier_by_id(layout, group_id) {
    if (group_id < 0 || group_id >= layout.group_list_size) return null;
    return layout.group_list[group_id].modifier;
}

function layout_get_group_id(layout, group_name) {
    return layout_helper_get_group_index(layout, group_name);
}

function layout_get_camera_placeholder(layout, camera_name) {
    for (let i = 0; i < layout.camera_list_size; i++) {
        if (layout.camera_list[i].name == camera_name) {
            return layout.camera_list[i];
        }
    }
    return null;
}

function layout_set_single_item_to_draw(layout, item_name) {
    layout.single_item = null;
    if (item_name == null) return true;
    for (let i = 0; i < layout.vertex_list_size; i++) {
        let name;
        if (layout.vertex_list[i].type == VERTEX_DRAWABLE) {
            name = layout.vertex_list[i].placeholder.name;
        } else {
            name = layout.vertex_list[i].name;
        }
        if (name == item_name) {
            layout.single_item = layout.vertex_list[i];
            return 1;
        }
    }
    return 0;
}

function layout_suspend(layout) {
    for (let i = 0; i < layout.sound_list_size; i++) {
        let soundplayer = layout.sound_list[i].soundplayer;
        layout.sound_list[i].was_playing = soundplayer_is_playing(soundplayer);
        if (layout.sound_list[i].was_playing) {
            soundplayer_pause(soundplayer);
            if (soundplayer_has_fading(soundplayer) == FADDING_OUT) soundplayer_set_volume(soundplayer, 0);
        }
    }
    for (let i = 0; i < layout.video_list_size; i++) {
        let videoplayer = layout.video_list[i].videoplayer;
        layout.video_list[i].was_playing = videoplayer_is_playing(videoplayer);
        if (layout.video_list[i].was_playing) {
            videoplayer_pause(videoplayer);
            if (videoplayer_has_fading_audio(videoplayer) == FADDING_OUT) videoplayer_set_volume(videoplayer, 0);
        }
    }
    layout.suspended = 1;
}

function layout_resume(layout) {
    for (let i = 0; i < layout.sound_list_size; i++) {
        let soundplayer = layout.sound_list[i].soundplayer;
        if (layout.sound_list[i].was_playing) soundplayer_play(soundplayer);
    }
    for (let i = 0; i < layout.video_list_size; i++) {
        let videoplayer = layout.video_list[i].videoplayer;
        if (layout.video_list[i].was_playing) videoplayer_play(videoplayer);
    }
    layout.suspended = 0;
}

function layout_disable_antialiasing(layout, disable) {
    layout.antialiasing_disabled = !!disable;
}

function layout_is_antialiasing_disabled(layout) {
    return layout.antialiasing_disabled;
}

function layout_get_layout_antialiasing(layout) {
    return layout.group_list[0].antialiasing;
}

function layout_set_layout_antialiasing(layout, flag) {
    layout.group_list[0].antialiasing = flag;
}

function layout_set_shader(layout, psshader) {
    layout.psshader = psshader;
}

function layout_get_group_shader(layout, group_name) {
    let index = layout_helper_get_group_index(layout, group_name);
    if (index < 0) return null;

    return layout.group_list[index].psshader;
}

function layout_set_group_shader(layout, group_name, psshader) {
    let index = layout_helper_get_group_index(layout, group_name);
    if (index < 0) return 0;

    layout.group_list[index].psshader = psshader;
    return 1;
}


function layout_animate(layout, elapsed) {
    if (layout.suspended) return 0;
    if (layout.animation_speed != 1.0) elapsed *= layout.animation_speed;

    let completed = 0;

    completed += layout_helper_check_trigger_queue(layout, elapsed);

    for (let i = 0; i < layout.vertex_list_size; i++) {
        switch (layout.vertex_list[i].type) {
            case VERTEX_SPRITE:
                completed += sprite_animate(layout.vertex_list[i].vertex, elapsed);
                break;
            case VERTEX_TEXTSPRITE:
                completed += textsprite_animate(layout.vertex_list[i].vertex, elapsed);
                break;
            case VERTEX_DRAWABLE:
                let placeholder = layout.vertex_list[i].placeholder;
                if (placeholder.vertex)
                    completed += drawable_animate(placeholder.vertex, elapsed);
                break;
        }
    }

    for (let i = 0; i < layout.external_vertex_list_size; i++) {
        let vertex = layout.external_vertex_list[i].vertex;
        if (!vertex) continue;

        switch (layout.external_vertex_list[i].type) {
            case VERTEX_SPRITE:
                completed += sprite_animate(vertex, elapsed);
                break;
            case VERTEX_TEXTSPRITE:
                completed += textsprite_animate(vertex, elapsed);
                break;
            case VERTEX_DRAWABLE:
                if (vertex)
                    completed += drawable_animate(vertex, elapsed);
                break;
        }
    }

    for (let i = 0; i < layout.group_list_size; i++) {
        completed += layout_helper_group_animate(layout.group_list[i], elapsed);
    }

    for (let i = 0; i < layout.fonts_size; i++) {
        let fontholder = layout.fonts[i].fontholder;
        if (fontholder.font_from_atlas) completed += fontglyph_animate(fontholder.font, elapsed);
    }

    if (!camera_animate(layout.camera_helper, elapsed)) {
        camera_apply(layout.camera_helper, null);
    }

    if (!camera_animate(layout.camera_secondary_helper, elapsed)) {
        camera_apply(layout.camera_secondary_helper, null);
    }

    return completed;
}

function layout_draw(layout,/**@type {PVRContext} */ pvrctx) {
    const MATRIX_SCREEN = new Float32Array(SH4MATRIX_SIZE);
    const MATRIX_VIEWPORT = new Float32Array(SH4MATRIX_SIZE);

    pvr_context_save(pvrctx);
    if (layout.psshader) pvr_context_add_shader(pvrctx, layout.psshader);

    if (layout.antialiasing_disabled) pvr_context_set_global_antialiasing(pvrctx, PVR_FLAG_DISABLE);

    if (layout.resolution_changes != pvrctx.resolution_changes) {
        layout_update_render_size(layout, pvrctx.screen_width, pvrctx.screen_height);
        for (let i = 0; i < layout.group_list_size; i++) {
            if (layout.group_list[i].psframebuffer)
                layout.group_list[i].psframebuffer.Resize();
        }

        layout.resolution_changes = pvrctx.resolution_changes;
    }

    // backup PVR screen matrix required for groups marked as "static_screen"
    sh4matrix_copy_to(pvrctx.current_matrix, MATRIX_SCREEN);

    // apply viewport modifier to PVR screen matrix and backup for elements marked as "static_camera"
    pvr_context_apply_modifier(pvrctx, layout.modifier_viewport);
    sh4matrix_copy_to(pvrctx.current_matrix, MATRIX_VIEWPORT);

    // transform PVR screen matrix with secondary camera offset+focus
    camera_apply_offset(layout.camera_secondary_helper, pvrctx.current_matrix);
    pvr_context_apply_modifier(pvrctx, layout.modifier_camera_secondary);

    // transform PVR screen matrix with primary camera offset. Note: the focus is used later as parallax
    camera_apply_offset(layout.camera_helper, pvrctx.current_matrix);

    // step 1: sort z_buffer
    for (let i = 0; i < layout.z_buffer_size; i++) {
        let item = layout.z_buffer[i].item;
        let vertex = layout.z_buffer[i].item.vertex;
        let vertex_type = layout.z_buffer[i].item.type;

        switch (vertex_type) {
            case VERTEX_SPRITE:
                layout.z_buffer[i].z_index = sprite_get_z_index(vertex);
                layout.z_buffer[i].visible = sprite_is_visible(vertex);
                break;
            case VERTEX_TEXTSPRITE:
                layout.z_buffer[i].z_index = textsprite_get_z_index(vertex);
                layout.z_buffer[i].visible = textsprite_is_visible(vertex);
                break;
            case VERTEX_DRAWABLE:
                if (item.placeholder) vertex = item.placeholder.vertex;
                if (vertex) {
                    layout.z_buffer[i].z_index = drawable_get_z_index(vertex);
                    layout.z_buffer[i].visible = drawable_is_visible(vertex);
                } else {
                    layout.z_buffer[i].z_index = MATH2D_MAX_INT32;
                    layout.z_buffer[i].visible = 0;
                }
                break;
        }
    }
    qsort(layout.z_buffer, layout.z_buffer_size, NaN, layout_helper_zbuffer_sort);

    // step 2: find top-most item of each group
    for (let i = 0; i < layout.z_buffer_size; i++) {
        let group = layout.group_list[layout.z_buffer[i].item.group_id];
        group.context.last_z_index = i;
    }

    // step 3: build root group context
    let layout_root = layout.group_list[0];

    //sh4matrix_copy_to(layout_root.matrix, layout_root.context.matrix);
    sh4matrix_clear(layout_root.context.matrix);
    sh4matrix_apply_modifier(layout_root.context.matrix, layout_root.modifier);

    layout_root.context.alpha = layout_root.alpha;
    layout_root.context.antialiasing = layout_root.antialiasing;
    layout_root.context.visible = layout_root.visible;
    layout_root.context.parallax.x = layout_root.parallax.x;
    layout_root.context.parallax.y = layout_root.parallax.y;
    layout_root.context.parallax.z = layout_root.parallax.z;
    for (let i = 0; i < 4; i++) layout_root.context.offsetcolor[i] = layout_root.offsetcolor[i];

    // step 4: stack all groups
    layout_helper_stack_groups(layout_root);

    // step 5: draw all layout items
    let has_single_item = layout.single_item != null;
    for (let i = 0; i < layout.z_buffer_size; i++) {
        if (!layout.z_buffer[i].visible) continue;
        if (has_single_item && layout.z_buffer[i].item != layout.single_item) continue;

        let vertex = layout.z_buffer[i].item.vertex;
        let vertex_type = layout.z_buffer[i].item.type;
        let group = layout.group_list[layout.z_buffer[i].item.group_id];
        let item_parallax = layout.z_buffer[i].item.parallax;
        let item_is_static_to_camera = layout.z_buffer[i].item.static_camera;

        if (layout.z_buffer[i].item.placeholder != null) {
            item_is_static_to_camera = layout.z_buffer[i].item.placeholder.static_camera;
            vertex = layout.z_buffer[i].item.placeholder.vertex;
            item_parallax = layout.z_buffer[i].item.placeholder.parallax;
        }
        if (layout.z_buffer[i].item.videoplayer != null) {
            videoplayer_poll_streams(layout.z_buffer[i].item.videoplayer);
        }

        if (!group.context.visible) continue;

        pvr_context_save(pvrctx);

        // check whatever the current and/or parent group has framebuffer
        if (group.psframebuffer) {
            pvr_context_set_framebuffer(pvrctx, group.psframebuffer);
        } else {
            // use group and parent group shaders
            layout_helper_stack_groups_shaders(group, pvrctx);
            pvr_context_set_framebuffer(pvrctx, null);
        }

        const draw_location = [0, 0];
        const matrix = pvrctx.current_matrix;
        let draw_fn;

        // apply group context
        pvr_context_set_global_alpha(pvrctx, group.psframebuffer ? 1.0 : group.context.alpha);
        pvr_context_set_global_antialiasing(pvrctx, group.context.antialiasing);
        pvr_context_set_global_offsetcolor(pvrctx, group.context.offsetcolor);

        if (item_is_static_to_camera || group.static_camera || group.static_screen) {
            if (group.static_screen) {
                // restore backup for the current group marked as "static_screen"
                sh4matrix_copy_to(MATRIX_SCREEN, matrix);
                sh4matrix_multiply_with_matrix(matrix, group.static_screen);
            } else {
                // restore backup with viewport transform applied for elements marked as "static_camera"
                sh4matrix_copy_to(MATRIX_VIEWPORT, matrix);
            }

            // apply group context matrix
            sh4matrix_multiply_with_matrix(matrix, group.context.matrix);
            pvr_context_flush(pvrctx);

            switch (vertex_type) {
                case VERTEX_SPRITE:
                    sprite_draw(vertex, pvrctx);
                    break;
                case VERTEX_TEXTSPRITE:
                    textsprite_draw(vertex, pvrctx);
                    break;
                case VERTEX_DRAWABLE:
                    drawable_draw(vertex, pvrctx);
                    break;
            }

            pvr_context_restore(pvrctx);
            continue;
        }

        // pick the draw location and location
        switch (vertex_type) {
            case VERTEX_SPRITE:
                sprite_get_draw_location(vertex, draw_location);
                draw_fn = sprite_draw;
                break;
            case VERTEX_TEXTSPRITE:
                textsprite_get_draw_location(vertex, draw_location);
                draw_fn = textsprite_draw;
                break;
            case VERTEX_DRAWABLE:
                //    
                // There two posible ways to obtain the draw location:
                //       1. using the placeholder values
                //       2. from the drawable modifier
                //
                drawable_get_draw_location(vertex, draw_location);
                draw_fn = drawable_draw;
                break;
        }

        // apply group context matrix
        sh4matrix_multiply_with_matrix(matrix, group.context.matrix);

        let translate_x = layout.modifier_camera.translate_x;
        let translate_y = layout.modifier_camera.translate_y;
        let px = group.context.parallax.x * item_parallax.x;
        let py = group.context.parallax.y * item_parallax.y;
        let pz = group.context.parallax.z * item_parallax.z;

        // parallax z as percent of camera scale
        let scale_x = 1.0 - ((1.0 - layout.modifier_camera.scale_x) * pz);
        let scale_y = 1.0 - ((1.0 - layout.modifier_camera.scale_y) * pz);

        // parallax translation
        let tx = translate_x * px;
        let ty = translate_y * py;

        // camera translation+scale position correction
        tx += px * ((translate_x * scale_x) - translate_x);
        ty += py * ((translate_y * scale_y) - translate_y);

        // camera with parallax scale correction
        tx += (layout.viewport_width * (Math.abs(scale_x) - 1.0) * Math.sign(scale_x)) / -2;
        ty += (layout.viewport_height * (Math.abs(scale_y) - 1.0) * Math.sign(scale_y)) / -2;

        // apply translation (with all parallax corrections)
        sh4matrix_translate(matrix, tx, ty);

        // parallax scale
        sh4matrix_scale(matrix, scale_x, scale_y);

        pvr_context_flush(pvrctx);

        draw_fn(vertex, pvrctx);

        pvr_context_restore(pvrctx);

        // if the last item of the current group was drawn, flush the group framebuffer
        if (group.psframebuffer && group.context.last_z_index == i) {

            pvr_context_save(pvrctx);
            sh4matrix_copy_to(MATRIX_VIEWPORT, pvrctx.current_matrix);

            // draw group framebuffer
            pvr_context_set_framebuffer(pvrctx, null);

            // use group and parent group shaders
            layout_helper_stack_groups_shaders(group, pvrctx);

            pvr_context_set_vertex_blend(
                pvrctx, group.blend_enabled, group.blend_src_rgb, group.blend_dst_rgb, group.blend_src_alpha, group.blend_dst_alpha
            );

            // draw group framebuffer in the screen
            let x = group.viewport_x > 0 ? group.viewport_x : 0;
            let y = group.viewport_y > 0 ? group.viewport_y : 0;
            let width = group.viewport_width > 0 ? group.viewport_width : layout.viewport_width;
            let height = group.viewport_height > 0 ? group.viewport_height : layout.viewport_height;

            //pvr_context_apply_modifier(pvrctx, layout.modifier_viewport);
            pvr_context_set_vertex_alpha(pvrctx, group.context.alpha);

            let sx = x * layout.modifier_viewport.scale_x;
            let sy = y * layout.modifier_viewport.scale_y;
            let sw = width * layout.modifier_viewport.scale_x;
            let sh = height * layout.modifier_viewport.scale_y;

            pvr_context_draw_framebuffer(pvrctx, group.psframebuffer, sx, sy, sw, sh, x, y, width, height);

            pvr_context_restore(pvrctx);
            group.psframebuffer.Invalidate();
        }
    }

    pvr_context_restore(pvrctx);
}



//////////////////////////////////
///          HELPERS           ///
//////////////////////////////////

function layout_helper_destroy_actions(actions, actions_size) {
    for (let i = 0; i < actions_size; i++) {
        let action = actions[i];

        for (let j = 0; j < action.entries_size; j++) {
            switch (action.entries[j].type) {
                case LAYOUT_ACTION_ANIMATION:
                    if (action.entries[j].misc) animsprite_destroy(action.entries[j].misc);
                    break;
                case LAYOUT_ACTION_ATLASAPPLY:
                    action.entries[j].misc = undefined;
                    break;
                case LAYOUT_ACTION_PROPERTY:
                    if (action.entries[j].property == TEXTSPRITE_PROP_STRING)
                        action.entries[j].misc = undefined;
                    break;
                case LAYOUT_ACTION_SETSHADER:
                    action.entries[j].misc.Destroy();
                    break;
                case LAYOUT_ACTION_SETSHADERUNIFORM:
                    action.entries[j].uniform_name = undefined;
                    action.entries[j].misc = undefined;
                    break;
            }
        }
        action.entries = undefined;
        action.name = undefined;

    }
    actions = undefined;
}

async function layout_helper_get_resource(resource_pool, src, is_texture) {
    let pool = is_texture ? resource_pool.textures : resource_pool.atlas;

    for (let definition of arraylist_iterate4(pool)) {
        if (definition.src == src) return definition.data;
    }

    // resource not found in the pool load it
    let data;

    if (is_texture)
        data = await texture_init_deferred(src, 1/* do not upload to the PVR VRAM */);
    else
        data = await atlas_init(src);

    if (!data) {
        let path = await fs_get_full_path_and_override(src);
        console.warn(`layout_helper_get_resource() missing resource '${src}' (${path})`);
        path = undefined;
    }

    if (data) {
        let new_definition = { data: data, src: strdup(src), is_texture: is_texture };
        arraylist_add(pool, new_definition);
    }

    return data;
}

function layout_helper_location(action_entry, width, height, v_width, v_height, location) {
    const align_vertical = action_entry.align_vertical;
    const align_horizontal = action_entry.align_horizontal;
    const location_x = action_entry.x;
    const location_y = action_entry.y;

    let offset_x = 0, offset_y = 0;
    // Note: align center means relative to the viewport

    // vertical align
    switch (align_vertical) {
        case ALIGN_START:
            offset_y = 0;
            break;
        case ALIGN_CENTER:
            offset_y = (v_height - height) / 2.0;
            break;
        case ALIGN_END:
            offset_y = v_height - height;
            break;
    }

    // horizontal align
    switch (align_horizontal) {
        case ALIGN_START:
            offset_x = 0;
            break;
        case ALIGN_CENTER:
            offset_x = (v_width - width) / 2.0;
            break;
        case ALIGN_END:
            offset_x = v_width - width;
            break;
    }

    offset_x += location_x;
    offset_y += location_y;

    location[0] = offset_x;
    location[1] = offset_y;

    return location;
}

function layout_helper_parse_hex(node, attr_name, def_value) {
    const value = [0x0000];
    if (!vertexprops_parse_hex(node.getAttribute(attr_name), value, false)) {
        console.error(`layout_helper_parse_hex() invalid value of '${attr_name}': `, node.outerHTML);
        return def_value;
    }

    return value[0];
}

function layout_helper_parse_float(node, attr_name, def_value) {
    if (!node.getAttribute(attr_name)) return def_value;

    let value = vertexprops_parse_float(node, attr_name, def_value);

    if (Number.isNaN(value)) {
        console.error("layout_parse_float(): invalid value: " + node.getAttribute(attr_name));
        return def_value;
    }

    return value;
}

function layout_helper_parse_align(node, is_vertical) {
    let attribute = is_vertical ? "alignVertical" : "alignHorizontal";
    return vertexprops_parse_align(node, attribute, 0, 1);
}

function layout_helper_parse_align2(node, is_vertical, def_value) {
    let attribute = is_vertical ? "alignVertical" : "alignHorizontal";
    if (!node.hasAttribute(attribute)) return def_value;

    let align = vertexprops_parse_align2(node.getAttribute(attribute));
    if (align == ALIGN_BOTH) {
        align = ALIGN_START;
        console.warn(`layout_helper_parse_align2() invalid align found at: ${node.outerHTML}`);
    }

    return align;
}

function layout_helper_parse_color(node, rgba) {
    if (node.hasAttribute("color")) {
        //
        // Check if the color is rgba (0xRRGGBBAA format) or rgb (0xRRGGBB format)
        //
        let value = node.getAttribute("color");
        let length = value.length;
        if (value.startsWith("0x") || value.startsWith("0X")) length -= 2;

        let raw_value = layout_helper_parse_hex(node, "color", 0xFFFFFF);
        let has_alpha = length == 8;
        math2d_color_bytes_to_floats(raw_value, has_alpha, rgba);
    } else if (node.hasAttribute("rgb")) {
        let rgb8_color = layout_helper_parse_hex(node, "rgb", 0xFFFFFF);
        math2d_color_bytes_to_floats(rgb8_color, 0, rgba);
    } else if (node.hasAttribute("rgba")) {
        let rgba8_color = layout_helper_parse_hex(node, "rgba", 0xFFFFFFFF);
        math2d_color_bytes_to_floats(rgba8_color, 1, rgba);
    } else if (node.hasAttribute("argb")) {
        let rgba8_color = layout_helper_parse_hex(node, "argb", 0xFFFFFFFF);
        math2d_color_bytes_to_floats(rgba8_color, 1, rgba);
        // move alpha location
        let a = rgba[0];
        let r = rgba[1];
        let g = rgba[2];
        let b = rgba[3];
        rgba[0] = r;
        rgba[1] = g;
        rgba[2] = b;
        rgba[3] = a;
    } else {
        rgba[0] = layout_helper_parse_float(node, "r", NaN);
        rgba[1] = layout_helper_parse_float(node, "g", NaN);
        rgba[2] = layout_helper_parse_float(node, "b", NaN);
        rgba[3] = layout_helper_parse_float(node, "a", NaN);
    }
}

function layout_helper_get_vertex(layout, type, name) {
    for (let i = 0; i < layout.vertex_list_size; i++) {
        if (layout.vertex_list[i].videoplayer) continue;
        if (layout.vertex_list[i].type == type && layout.vertex_list[i].name == name) {
            return layout.vertex_list[i].vertex;
        }
    }
    return null;
}

function layout_helper_get_group_index(layout, name) {
    if (name === LAYOUT_GROUP_ROOT) {
        return 0;
    } else {
        let size = layout.group_list_size;
        for (let i = 1; i < size; i++) {
            if (layout.group_list[i].name === name) {
                return i;
            }
        }
    }
    return -1;
}

function layout_helper_execute_action(layout, vertex, action) {
    switch (vertex.type) {
        case VERTEX_SPRITE:
            layout_helper_execute_action_in_sprite(
                action, vertex, layout.viewport_width, layout.viewport_height
            );
            break;
        case VERTEX_TEXTSPRITE:
            layout_helper_execute_action_in_textsprite(
                action, vertex, layout.viewport_width, layout.viewport_height
            );
            break;
    }
}

function layout_helper_execute_trigger(layout, trigger) {
    trigger.context.running = 1;
    trigger.context.progress_delay = 0;
    trigger.context.loop_waiting = 0;
    trigger.context.loop_count = 0;

    if (trigger.start_delay > 0) return;

    // there no start delay, commit now
    layout_helper_commit_trigger(layout, trigger);
}

function layout_helper_set_parallax_info(parallax_info, parallax_action) {
    if (!Number.isNaN(parallax_action.x)) parallax_info.x = parallax_action.x;
    if (!Number.isNaN(parallax_action.y)) parallax_info.y = parallax_action.y;
    if (!Number.isNaN(parallax_action.z)) parallax_info.z = parallax_action.z;
}

function layout_helper_stack_groups(parent_group) {
    let parent_visible = parent_group.context.visible && parent_group.context.alpha > 0;
    let group = parent_group.context.next_child;

    while (group) {
        let group_alpha = group.alpha * group.alpha2;
        group.context.visible = parent_visible && group.visible && group_alpha > 0;
        group.context.parent_group = parent_group;

        if (group.context.visible) {
            // interpolate the parent context in the current context
            math2d_color_blend_normal(
                group.offsetcolor, parent_group.context.offsetcolor, group.context.offsetcolor
            );

            sh4matrix_copy_to(parent_group.context.matrix, group.context.matrix);
            sh4matrix_apply_modifier(group.context.matrix, group.modifier);

            group.context.alpha = group_alpha * parent_group.context.alpha;

            if (group.antialiasing == PVR_FLAG_DEFAULT)
                group.context.antialiasing = parent_group.context.antialiasing;
            else
                group.context.antialiasing = group.antialiasing;

            group.context.parallax.x = group.parallax.x * parent_group.context.parallax.x;
            group.context.parallax.y = group.parallax.y * parent_group.context.parallax.y;
            group.context.parallax.z = group.parallax.z * parent_group.context.parallax.z;
        }

        if (group.context.next_child) layout_helper_stack_groups(group);

        group = group.context.next_sibling;
    }
}

function layout_helper_stack_groups_shaders(group, pvrctx) {
    // if the parent has framebuffer, stop going up
    while (group) {
        if (group.psshader) {
            if (!pvr_context_add_shader(pvrctx, group.psshader)) {
                // limit reached
                break;
            }
        }
        group = group.context.parent_group;
    }
}

function layout_helper_group_animate(group, elapsed) {
    if (!group.animation) return 1;

    let completed = animsprite_animate(group.animation, elapsed);
    animsprite_update_using_callback(group.animation, group, layout_helper_group_set_property, 1);

    return completed;
}

function layout_helper_group_set_property(group, property_id, value) {
    switch (property_id) {
        case SPRITE_PROP_X:
            group.modifier.x = value;
            break;
        case SPRITE_PROP_Y:
            group.modifier.y = value;
            break;
        case SPRITE_PROP_WIDTH:
            group.modifier.width = value;
            break;
        case SPRITE_PROP_HEIGHT:
            group.modifier.height = value;
            break;
        case SPRITE_PROP_ALPHA:
            group.alpha = math2d_clamp_float(value, 0.0, 1.0);
            break;
        case SPRITE_PROP_VERTEX_COLOR_OFFSET_R:
            group.offsetcolor[0] = value;
            break;
        case SPRITE_PROP_VERTEX_COLOR_OFFSET_G:
            group.offsetcolor[1] = value;
            break;
        case SPRITE_PROP_VERTEX_COLOR_OFFSET_B:
            group.offsetcolor[2] = value;
            break;
        case SPRITE_PROP_VERTEX_COLOR_OFFSET_A:
            group.offsetcolor[3] = value;
            break;
        case SPRITE_PROP_ANTIALIASING:
            group.antialiasing = Math.trunc(value);
            break;
        case SPRITE_PROP_ALPHA2:
            group.alpha2 = value;
            break;
        default:
            pvrctx_helper_set_modifier_property(group.modifier, property_id, value);
            break;
    }
}

function layout_helper_parse_property(unparsed_entry, property_id, value_holder, action_entries) {

    let action_entry = {
        type: LAYOUT_ACTION_PROPERTY,
        property: property_id,
        value: NaN,
        misc: null,
    };

    arraylist_add(action_entries, action_entry);

    if (vertexprops_is_property_boolean(property_id)) {
        let value = vertexprops_parse_boolean(unparsed_entry, value_holder, -1);
        if (value != -1) {
            action_entry.value = value;
            return;
        }
    }
    switch (property_id) {
        case TEXTSPRITE_PROP_ALIGN_V:
        case TEXTSPRITE_PROP_ALIGN_H:
        case TEXTSPRITE_PROP_ALIGN_PARAGRAPH:
            action_entry.value = vertexprops_parse_align(unparsed_entry, value_holder, 1, 1);
            break;
        case TEXTSPRITE_PROP_FORCE_CASE:
            action_entry.value = vertexprops_parse_textsprite_forcecase(unparsed_entry, value_holder, 1);
            break;
        case TEXTSPRITE_PROP_FONT_COLOR:
            action_entry.value = layout_helper_parse_hex(unparsed_entry, value_holder, 0xFFFFFF);
            break;
        case TEXTSPRITE_PROP_STRING:
            action_entry.misc = strdup(unparsed_entry.getAttribute(value_holder));
            break;
        case MEDIA_PROP_PLAYBACK:
            action_entry.value = vertexprops_parse_playback(unparsed_entry, value_holder, 1);
            break;
        case SPRITE_PROP_ANTIALIASING:
            action_entry.value = vertexprops_parse_flag(unparsed_entry, value_holder, 1);
            break;
        case FONT_PROP_WORDBREAK:
            action_entry.value = vertexprops_parse_wordbreak(unparsed_entry, value_holder, 1);
            break;
        default:
            action_entry.value = layout_helper_parse_float(unparsed_entry, value_holder, 0);
            break;
    }
}

function layout_herper_parse_offsetmovefromto(unparsed_offsetmovefromto, output_xyz) {
    output_xyz[0] = layout_helper_parse_float(unparsed_offsetmovefromto, "x", NaN);
    output_xyz[1] = layout_helper_parse_float(unparsed_offsetmovefromto, "y", NaN);
    output_xyz[2] = layout_helper_parse_float(unparsed_offsetmovefromto, "z", NaN);
}

function layout_helper_zbuffer_build(layout) {
    let j = 0;
    for (let i = 0; i < layout.z_buffer_size; i++) {
        layout.z_buffer[i] = { item: null, visible: 0, z_index: 0 };
    }
    for (let i = 0; i < layout.vertex_list_size; i++) {
        layout.z_buffer[j++].item = layout.vertex_list[i];
    }
    for (let i = 0; i < layout.external_vertex_list_size; i++) {
        layout.z_buffer[j++].item = layout.external_vertex_list[i];
    }
}

function layout_helper_zbuffer_sort(entry1, entry2) {
    return entry1.z_index - entry2.z_index;
}

function layout_helper_add_group_to_parent(parent_context, group) {
    if (parent_context.next_child) {
        let sibling_group = parent_context.next_child;
        while (1) {
            if (!sibling_group.context.next_sibling) break;
            sibling_group = sibling_group.context.next_sibling;
        }
        sibling_group.context.next_sibling = group;
    } else {
        parent_context.next_child = group;
    }
}

function layout_helper_check_trigger_queue(layout, elapsed) {
    let completed_count = 0;

    // complex timestamp based checker
    if (layout.beatwatcher_synced_triggers) {
        let do_sync = 0;
        let has_beat = beatwatcher_poll(layout.beatwatcher);

        // check if the beatwatcher was resynchronized
        if (layout.beatwatcher.resyncs > 0) {
            // snapshot current beatwatcher timestamp
            if (layout.beatwatcher_resyncs_count != layout.beatwatcher.resyncs) {
                layout.beatwatcher_resyncs_count = layout.beatwatcher.resyncs;
                layout.beatwatcher_last_timestamp = layout.beatwatcher.last_global_timestamp;
                return completed_count;
            }

            // wait for a beat, this keep harmonic all beat-dependent animations (in theory)
            if (!has_beat) return;

            do_sync = 1;
            layout.beatwatcher.resyncs = layout.beatwatcher_resyncs_count = 0;
        }

        // calc time elasped since last layout_helper_check_trigger_queue() call
        elapsed = layout.beatwatcher.last_global_timestamp - layout.beatwatcher_last_timestamp;
        layout.beatwatcher_last_timestamp = layout.beatwatcher.last_global_timestamp;

        if (do_sync) {
            // resync all trigger progress and check later the queue
            for (let i = 0; i < layout.trigger_list_size; i++) {
                layout.trigger_list[i].context.progress_delay = 0;
            }
        }

        for (let i = 0; i < layout.trigger_list_size; i++) {
            let trigger_is_completed = 0;
            let trigger = layout.trigger_list[i];

            if (!trigger.context.running) {
                completed_count++;
                continue;
            }

            let commit = 0;
            trigger.context.progress_delay += elapsed;

            while (1) {
                let target_delay;
                if (trigger.context.loop_waiting) {
                    target_delay = trigger.loop_delay;
                } else {
                    target_delay = trigger.start_delay;
                }

                if (trigger.context.progress_delay >= target_delay) {
                    trigger.context.progress_delay -= target_delay;// keep in sync
                    commit = 1;
                    trigger.context.loop_waiting = 1;
                    trigger.context.loop_count++;
                } else {
                    break;
                }

                if (trigger.loop < 1) {
                    trigger_is_completed = 1;
                    break;
                }
                if (trigger.context.loop_count >= trigger.loop) {
                    trigger.context.running = 0;
                    trigger_is_completed = 1;
                    break;
                }
            }

            if (trigger_is_completed) trigger.context.progress_delay = 0;

            if (commit) {
                layout_helper_commit_trigger(layout, trigger);
            }
        }

        return completed_count;
    }

    // classic way
    for (let i = 0; i < layout.trigger_list_size; i++) {
        let trigger = layout.trigger_list[i];

        if (!trigger.context.running) {
            completed_count++;
            continue;
        }

        let target_delay;

        if (trigger.context.loop_waiting) {
            target_delay = trigger.loop_delay;
        } else {
            target_delay = trigger.start_delay;
        }

        if (trigger.context.progress_delay >= target_delay) {
            trigger.context.progress_delay -= target_delay;// keep in sync
            layout_helper_commit_trigger(layout, trigger);
        }

        trigger.context.progress_delay += elapsed;
    }

    return completed_count;
}

function layout_helper_commit_trigger(layout, trigger) {
    // increase the number of loops (or queue for the next loop) and check if still should run
    trigger.context.loop_count++;
    trigger.context.loop_waiting = 1;
    trigger.context.running = trigger.loop < 1 || trigger.context.loop_count < trigger.loop;

    if (trigger.action_name != null && !layout_trigger_action(layout, null, trigger.action_name)) {
        console.warn(`layout_helper_commit_trigger() no actions with name: ${trigger.action_name}`);
    }
    if (trigger.camera_name != null && !layout_trigger_camera(layout, trigger.camera_name)) {
        console.warn(`layout_helper_commit_trigger() no camera with name: ${trigger.camera_name}`);
    }
    if (trigger.stop_trigger_name != null) {
        layout_stop_trigger(layout, trigger.stop_trigger_name);
    }
    if (trigger.trigger_name == null) {
        return;
    }

    trigger.context.reject_recursive = 1;// avoid self-trigger

    for (let i = 0; i < layout.trigger_list_size; i++) {
        let trigger2 = layout.trigger_list[i];
        if (trigger2.name != trigger.trigger_name) continue;

        if (!trigger2.context.reject_recursive) {
            layout_helper_execute_trigger(layout, trigger2);
            continue;
        }

        console.warn(`layout_helper_commit_trigger() self-trigger avoided: ${trigger.action_name}`);
    }

    trigger.context.reject_recursive = 0;
    return;
}

function layout_helper_set_shader_uniform(psshader, action_entry) {
    if (!psshader) {
        console.warn(`layout_helper_set_shader_uniform() can not set ${action_entry.uniform_name}, there no shader`);
        return;
    }

    let ret = psshader.SetUniformAny(action_entry.uniform_name, action_entry.misc);

    switch (ret) {
        case 0:
            console.warn(`layout_helper_set_shader_uniform() the shader does not have ${action_entry.uniform_name}`);
            break;
        case -1:
            console.warn(`layout_helper_set_shader_uniform() type of ${action_entry.uniform_name} is not supported`);
            break;
        case -2:
            console.error(`layout_helper_set_shader_uniform() bad setter for ${action_entry.uniform_name}`);
            break;
    }
}


//////////////////////////////////
///        VERTEX PARSER       ///
//////////////////////////////////

function layout_parse_placeholder(unparsed_plchdlr, layout_context, group_id) {
    let name = unparsed_plchdlr.getAttribute("name");
    if (!name) {
        console.warn("Missing placeholder name: " + unparsed_plchdlr.outerHTML);
        return;
    }

    let placeholder = {
        type: VERTEX_DRAWABLE,
        group_id: group_id,

        name: strdup(name),

        align_vertical: layout_helper_parse_align2(unparsed_plchdlr, 1, ALIGN_NONE),
        align_horizontal: layout_helper_parse_align2(unparsed_plchdlr, 0, ALIGN_NONE),

        x: layout_helper_parse_float(unparsed_plchdlr, "x", 0),
        y: layout_helper_parse_float(unparsed_plchdlr, "y", 0),
        z: layout_helper_parse_float(unparsed_plchdlr, "z", 0),
        height: layout_helper_parse_float(unparsed_plchdlr, "height", -1),
        width: layout_helper_parse_float(unparsed_plchdlr, "width", -1),

        parallax: { x: 1, y: 1, z: 1 },
        static_camera: vertexprops_parse_boolean(unparsed_plchdlr, "static", false),

        vertex: null
    };

    const xyz = [0, 0, 0];

    let unparsed_parallax = unparsed_plchdlr.querySelector("Parallax");
    if (unparsed_parallax) {
        layout_herper_parse_offsetmovefromto(unparsed_parallax, xyz);
        if (Number.isFinite(xyz[0])) placeholder.parallax.x = xyz[0];
        if (Number.isFinite(xyz[1])) placeholder.parallax.y = xyz[1];
        if (Number.isFinite(xyz[2])) placeholder.parallax.z = xyz[2];
    }

    let unparsed_location = unparsed_plchdlr.querySelector("Location");
    if (unparsed_location) {
        layout_herper_parse_offsetmovefromto(unparsed_location, xyz);
        if (Number.isFinite(xyz[0])) placeholder.x = xyz[0];
        if (Number.isFinite(xyz[1])) placeholder.y = xyz[1];
        if (Number.isFinite(xyz[2])) placeholder.z = xyz[2];
        placeholder.align_vertical = layout_helper_parse_align2(
            unparsed_location, 1, placeholder.align_vertical
        );
        placeholder.align_horizontal = layout_helper_parse_align2(
            unparsed_location, 0, placeholder.align_horizontal
        );
    }

    let unparsed_size = unparsed_plchdlr.querySelector("Size");
    if (unparsed_size) {
        placeholder.width = layout_helper_parse_float(unparsed_size, "width", placeholder.width);
        placeholder.height = layout_helper_parse_float(unparsed_size, "height", placeholder.height);
    }

    let unparsed_static = unparsed_plchdlr.querySelector("Static");
    if (unparsed_static) {
        placeholder.static_camera = vertexprops_parse_boolean(
            unparsed_static, "enable", 1
        );
    }

    let item = {
        name: null,// STUB, the placeholder field contains the name
        actions_size: 0,
        type: VERTEX_DRAWABLE,
        group_id: group_id,
        placeholder: placeholder,
        parallax: {},// STUB, the placeholder field contains the parallax
        initial_action_name: null
    };
    arraylist_add(layout_context.vertex_list, item);
}

async function layout_parse_sprite(unparsed_sprite, layout_context, group_id) {
    let actions = unparsed_sprite.querySelectorAll("Action");
    let texture_filename = unparsed_sprite.getAttribute("texture");
    let atlas_filename = unparsed_sprite.getAttribute("atlas");
    let sprite;

    let atlas;
    let atlas_texture_path = null;
    if (atlas_filename) {
        atlas = await layout_helper_get_resource(layout_context.resource_pool, atlas_filename, 0);
        if (atlas && !texture_filename) {
            atlas_texture_path = atlas_get_texture_path(atlas);
            if (!await fs_file_exists(atlas_texture_path)) {
                // the imagePath attribute has an invalid filename
                console.warn(`layout_parse_sprite() texture pointed by imagePath='${atlas_texture_path}' not found in atlas '${atlas_filename}'`);
                let temp_value = fs_get_filename_without_extension(atlas_filename);
                let temp_texture_filename = string_concat(2, temp_value, ".png");
                atlas_texture_path = fs_build_path2(atlas_filename, temp_texture_filename);
                temp_value = undefined;
                temp_texture_filename = undefined;
            }
        }
    } else {
        atlas = null;
    }

    if (texture_filename || atlas_texture_path) {
        let src = texture_filename ?? atlas_texture_path;
        let texture = await layout_helper_get_resource(layout_context.resource_pool, src, 1);
        sprite = sprite_init(texture);
        if (!atlas) sprite_set_draw_size_from_source_size(sprite);
    } else {
        let rgba = [0.0, 0.0, 0.0, NaN];
        layout_helper_parse_color(unparsed_sprite, rgba);

        sprite = sprite_init_from_rgb8(0xFFFFFF);

        sprite_set_vertex_color(sprite, rgba[0], rgba[1], rgba[2]);
        if (!Number.isNaN(rgba[3])) sprite_set_alpha(sprite, rgba[3]);
    }

    let vertex = {
        type: VERTEX_SPRITE,
        name: unparsed_sprite.getAttribute("name"),
        vertex: sprite,
        group_id: group_id,
        actions_size: -1,
        actions: null,
        initial_action_name: unparsed_sprite.getAttribute("initialAction"),
        parallax: { x: 1.0, y: 1.0, z: 1.0 },
        static_camera: 0,
        animation: null,
        placeholder: null
    };

    let actions_arraylist = arraylist_init2(actions.length);
    for (let action of actions) {
        await layout_parse_sprite_action(action, layout_context.animlist, atlas, actions_arraylist, 0);
    }
    arraylist_destroy2(actions_arraylist, vertex, "actions_size", "actions");

    arraylist_add(layout_context.vertex_list, vertex);
}

async function layout_parse_text(unparsed_text, layout_context, group_id) {
    let fontholder = null;
    let font_name = unparsed_text.getAttribute("fontName");
    let font_size = layout_helper_parse_float(unparsed_text, "fontSize", 12);
    let font_color = [1.0, 1.0, 1.0, NaN];
    let actions = unparsed_text.querySelectorAll("Action");

    layout_helper_parse_color(unparsed_text, font_color);

    for (let i = 0; i < layout_context.fonts_size; i++) {
        if (layout_context.fonts[i].name == font_name) {
            fontholder = layout_context.fonts[i].fontholder;
            break;
        }
    }

    if (!fontholder) {
        console.error(`layout_parse_text() the font '${font_name}' is not attached`);
        return;
    }

    let vertex = {
        name: unparsed_text.getAttribute("name"),
        type: VERTEX_TEXTSPRITE,
        vertex: textsprite_init2(fontholder, font_size, 0x000000),
        actions_size: -1,
        actions: null,
        group_id: group_id,
        initial_action_name: unparsed_text.getAttribute("initialAction"),
        parallax: { x: 1.0, y: 1.0, z: 1.0 },
        static_camera: 0,
        animation: null,
        placeholder: null
    };

    textsprite_set_color(vertex.vertex, font_color[0], font_color[1], font_color[2]);
    if (!Number.isNaN(font_color[3])) textsprite_set_alpha(vertex.vertex, font_color[3]);

    let actions_arraylist = arraylist_init2(actions.length);
    for (let action of actions) {
        await layout_parse_text_action(action, layout_context.animlist, actions_arraylist);
    }
    arraylist_destroy2(actions_arraylist, vertex, "actions_size", "actions");

    arraylist_add(layout_context.vertex_list, vertex);
}

async function layout_parse_group(unparsed_group, layout_context, parent_context) {
    let actions_arraylist = arraylist_init2(unparsed_group.children.length);

    let group = {
        name: parent_context ? unparsed_group.getAttribute("name") : LAYOUT_GROUP_ROOT,
        group_id: arraylist_size(layout_context.group_list),
        actions: null,
        actions_size: -1,
        initial_action_name: unparsed_group.getAttribute("initialAction"),

        visible: vertexprops_parse_boolean(unparsed_group, "visible", 1),
        alpha: layout_helper_parse_float(unparsed_group, "alpha", 1.0),
        alpha2: 1.0,
        antialiasing: vertexprops_parse_flag(unparsed_group, "antialiasing", PVR_FLAG_DEFAULT),
        offsetcolor: [],
        modifier: {},
        parallax: { x: 1.0, y: 1.0, z: 1.0 },
        static_camera: 0,
        static_screen: null,

        animation: null,
        psshader: null,
        psframebuffer: null,

        blend_enabled: 1,
        blend_src_rgb: BLEND_DEFAULT,
        blend_dst_rgb: BLEND_DEFAULT,
        blend_src_alpha: BLEND_DEFAULT,
        blend_dst_alpha: BLEND_DEFAULT,

        viewport_x: -1,
        viewport_y: -1,
        viewport_width: -1,
        viewport_height: -1,

        context: {
            visible: 1,
            alpha: 1.0,
            antialiasing: PVR_FLAG_DEFAULT,
            matrix: new Float32Array(SH4MATRIX_SIZE),
            offsetcolor: [],
            parallax: { x: 1.0, y: 1.0, z: 1.0 },
            last_z_index: -1,

            next_child: null,
            next_sibling: null,
            parent_group: null
        }
    };

    //sh4matrix_reset(group.matrix);
    pvrctx_helper_clear_modifier(group.modifier);
    pvrctx_helper_clear_offsetcolor(group.offsetcolor);

    if (vertexprops_parse_boolean(unparsed_group, "framebuffer", 0)) {
        // assume layout as part of the main PVRContext renderer
        group.psframebuffer = new PSFramebuffer(pvr_context);
    }

    arraylist_add(layout_context.group_list, group);

    for (let item of unparsed_group.children) {
        switch (item.tagName) {
            case "Action":
                /*if (!parent_context) {
                    console.warn(
                        "layout_parse_group() action found in the layout root, " +
                        "will be imported as root group action."
                    );
                }*/
                await layout_parse_group_action(item, layout_context.animlist, actions_arraylist);
                break;
            case "Sprite":
                await layout_parse_sprite(item, layout_context, group.group_id);
                break;
            case "Text":
                await layout_parse_text(item, layout_context, group.group_id);
                break;
            case "Camera":
                /*if (parent_context) {
                    console.warn(
                        "layout_parse_group() groups can not contains cameras"
                    );
                }*/
                layout_parse_camera(item, layout_context);
                break;
            case "Group":
                await layout_parse_group(item, layout_context, group.context);
                break;
            case "Placeholder":
                layout_parse_placeholder(item, layout_context, group.group_id);
                break;
            case "Sound":
                await layout_parse_sound(item, layout_context);
                break;
            case "Video":
                await layout_parse_video(item, layout_context, group.group_id);
                break;
            case "AttachValue":
            case "Font":
                break;
            case "Trigger":
                await layout_parse_triggers(item, layout_context);
                break;
            default:
                console.warn("layout_parse_group() unknown element: " + item.tagName);
                break;
        }
    }

    arraylist_destroy2(actions_arraylist, group, "actions_size", "actions");

    // add to group tree
    if (parent_context) {
        layout_helper_add_group_to_parent(parent_context, group);
    }

}

async function layout_parse_fonts(unparsed_root, layout_context) {
    let unparsed_fonts = unparsed_root.querySelectorAll("Font");
    let fonts_arraylist = arraylist_init2(unparsed_fonts.length);

    for (let item of unparsed_fonts) {
        let name = item.getAttribute("name");
        let path = item.getAttribute("path");
        let glyph_animate = vertexprops_parse_boolean(item, "glyphAnimate", 1);
        let glyph_suffix = item.getAttribute("glyphSuffix");
        let glyph_color_by_difference = vertexprops_parse_boolean(item, "colorByDifference", 0);

        if (!name) {
            console.error("layout_parse_fonts() missing font name: " + item.outerHTML);
            continue;
        }
        if (!path) {
            console.error("layout_parse_fonts() missing font path: " + item.outerHTML);
            continue;
        }

        try {
            let font;
            let is_atlas = atlas_utils_is_known_extension(path);

            if (is_atlas) {
                font = await fontglyph_init(path, glyph_suffix, glyph_animate);
                if (glyph_color_by_difference) fontglyph_enable_color_by_difference(font, 1);
            } else {
                font = await fonttype_init(path);
            }

            if (!font) throw new Error("missing or invalid font: " + path);
            let fontholder = fontholder_init2(font, is_atlas, -1);

            arraylist_add(fonts_arraylist, { name: strdup(name), fontholder: fontholder });
        } catch (e) {
            console.error("layout_parse_fonts() unable to read the font: " + path, e);
            continue;
        } finally {
            path = undefined;
        }
    }

    arraylist_destroy2(fonts_arraylist, layout_context, "fonts_size", "fonts");
}

function layout_parse_camera(unparsed_camera, layout_context) {
    const xyz = [0, 0, 0];

    let duration_beats = layout_helper_parse_float(unparsed_camera, "durationInBeats", NaN);
    let duration_milliseconds = layout_helper_parse_float(unparsed_camera, "duration", NaN);
    let enable_offset_zoom = vertexprops_parse_boolean(unparsed_camera, "offsetZoom", 1);

    let duration_in_beats, duration, has_duration;

    if (Number.isNaN(duration_beats) && Number.isNaN(duration_milliseconds)) {
        duration_in_beats = 1;
        duration = 1;
        has_duration = 0;
    } else {
        duration_in_beats = Number.isNaN(duration_milliseconds);
        duration = duration_in_beats ? duration_beats : duration_milliseconds;
        has_duration = 1;
    }

    let camera_placeholder = {
        name: unparsed_camera.getAttribute("name"),

        enable_offset_zoom: enable_offset_zoom,

        has_duration: has_duration,
        duration_in_beats: duration_in_beats,
        duration: duration,

        move_only: 0,
        has_from: 0,

        is_empty: 0,

        has_parallax_offset_only: 0,

        move_offset_only: 0,
        has_offset_from: 0,
        has_offset_to: 0,

        to_offset_x: NaN,
        to_offset_y: NaN,
        to_offset_z: NaN,

        from_offset_x: NaN,
        from_offset_y: NaN,
        from_offset_z: NaN,

        animation: null,

        from_x: NaN, from_y: NaN, from_z: NaN,
        to_x: NaN, to_y: NaN, to_z: NaN,
        offset_x: 0, offset_y: 0, offset_z: 1
    };

    let anim_name = unparsed_camera.getAttribute("animationName");
    let unparsed_move = unparsed_camera.querySelector("Move");
    let unparsed_from = unparsed_camera.querySelector("From");
    let unparsed_to = unparsed_camera.querySelector("To");

    let unparsed_offset = unparsed_camera.querySelector("Offset");
    let unparsed_move_offset = unparsed_camera.querySelector("OffsetMove");
    let unparsed_from_offset = unparsed_camera.querySelector("FromOffset");
    let unparsed_to_offset = unparsed_camera.querySelector("ToOffset");

    if (
        !anim_name &&
        !unparsed_move && !unparsed_from && !unparsed_to &&
        !unparsed_offset &&
        !unparsed_move_offset && !unparsed_from_offset && !unparsed_to_offset
    ) {
        // no animation or tween is defined
        camera_placeholder.is_empty = 1;

        arraylist_add(layout_context.camera_list, camera_placeholder);
        return;
    }


    if (unparsed_move) {
        camera_placeholder.move_only = 1;
        layout_herper_parse_offsetmovefromto(unparsed_move, xyz);
        camera_placeholder.from_x = camera_placeholder.to_x = xyz[0];
        camera_placeholder.from_y = camera_placeholder.to_y = xyz[1];
        camera_placeholder.from_z = camera_placeholder.to_z = xyz[2];
    } else {
        if (unparsed_from) {
            layout_herper_parse_offsetmovefromto(unparsed_from, xyz);
            camera_placeholder.from_x = xyz[0];
            camera_placeholder.from_y = xyz[1];
            camera_placeholder.from_z = xyz[2];
            camera_placeholder.has_from = 1;
        }
        if (unparsed_to) {
            layout_herper_parse_offsetmovefromto(unparsed_to, xyz);
            camera_placeholder.to_x = xyz[0];
            camera_placeholder.to_y = xyz[1];
            camera_placeholder.to_z = xyz[2];
        }
    }

    if (unparsed_offset) {
        layout_herper_parse_offsetmovefromto(unparsed_offset, xyz);
        camera_placeholder.offset_x = xyz[0];
        camera_placeholder.offset_y = xyz[1];
        camera_placeholder.offset_z = xyz[2];
        camera_placeholder.has_parallax_offset_only = 1;
    } else if (unparsed_move_offset) {
        layout_herper_parse_offsetmovefromto(unparsed_move_offset, xyz);
        camera_placeholder.to_offset_x = xyz[0];
        camera_placeholder.to_offset_y = xyz[1];
        camera_placeholder.to_offset_z = xyz[2];
        camera_placeholder.move_offset_only = 1;
    } else {
        if (unparsed_from_offset) {
            layout_herper_parse_offsetmovefromto(unparsed_from_offset, xyz);
            camera_placeholder.from_offset_x = xyz[0];
            camera_placeholder.from_offset_y = xyz[1];
            camera_placeholder.from_offset_z = xyz[2];
            camera_placeholder.has_offset_from = 1;
        }
        if (unparsed_to_offset) {
            layout_herper_parse_offsetmovefromto(unparsed_to_offset, xyz);
            camera_placeholder.to_offset_x = xyz[0];
            camera_placeholder.to_offset_y = xyz[1];
            camera_placeholder.to_offset_z = xyz[2];
            camera_placeholder.has_offset_to = 1;
        }
    }

    if (anim_name) {
        if (layout_context.animlist) {
            camera_placeholder.animation = animsprite_init_from_animlist(
                layout_context.animlist, anim_name
            );
        } else {
            console.warn(`layout_parse_camera() can not import '${anim_name}', layout does not have an animlist`);
        }
    }

    arraylist_add(layout_context.camera_list, camera_placeholder);
}

function layout_parse_externalvalues(unparsed_root, layout_context) {
    //let list = unparsed_root.querySelectorAll(":scope > AttachValue");
    let list = unparsed_root.querySelectorAll("AttachValue");

    let values_arraylist = arraylist_init2(list.length);

    for (let item of list) {
        let name = item.getAttribute("name");
        let unparsed_type = item.getAttribute("type");
        let unparsed_value = item.getAttribute("value");

        if (!name) {
            console.error("layout_parse_externalvalues() missing AttachValue name: " + item.outerHTML);
            continue;
        }
        if (!unparsed_type) {
            console.error("layout_parse_externalvalues() missing AttachValue type: " + item.outerHTML);
            continue;
        }

        let value, type, invalid;

        switch (unparsed_type.toLowerCase()) {
            case "string":
                value = unparsed_value;
                type = LAYOUT_TYPE_STRING;
                invalid = false;
                break;
            case "float":
                value = Number.parseFloat(unparsed_value);
                type = LAYOUT_TYPE_FLOAT;
                invalid = !Number.isFinite(value);
                break;
            case "integer":
                value = vertexprops_parse_integer2(unparsed_value, NaN);
                type = LAYOUT_TYPE_INTEGER;
                invalid = !Number.isFinite(value);
                break;
            case "hex":
                value = vertexprops_parse_hex2(unparsed_value, NaN, false);
                type = LAYOUT_TYPE_HEX;
                invalid = !Number.isFinite(value);
                break;
            case "boolean":
                value = vertexprops_parse_boolean2(unparsed_value, 0);
                type = LAYOUT_TYPE_BOOLEAN;
                invalid = false;
                break;
            default:
                console.error("layout_parse_externalvalues() unknown AttachValue type: " + item.outerHTML);
                continue;
        }

        if (invalid) {
            console.error("layout_parse_externalvalues() value in : " + item.outerHTML);
            continue;
        }

        let entry = { name, value, type };
        arraylist_add(values_arraylist, entry);
    }

    arraylist_destroy2(values_arraylist, layout_context, "values_size", "values");
}

function layout_parse_triggers(unparsed_trigger, layout_context) {
    let trigger = {
        name: unparsed_trigger.getAttribute("name"),

        action_name: unparsed_trigger.getAttribute("action"),
        camera_name: unparsed_trigger.getAttribute("camera"),
        trigger_name: unparsed_trigger.getAttribute("trigger"),
        stop_trigger_name: unparsed_trigger.getAttribute("stopTrigger"),

        loop: vertexprops_parse_integer(unparsed_trigger, "loop", 1),// 1 means execute once

        loop_delay: 0,
        loop_delay_beats: 0,
        loop_delay_beats_in_beats: 0,

        start_delay: 0,
        start_delay_beats: 0,
        start_delay_beats_in_beats: 0,

        context: {
            running: 0,
            reject_recursive: 0,
            progress_delay: 0,
            loop_waiting: 0,
            loop_count: 0,
        }
    };

    if (unparsed_trigger.hasAttribute("loopDelayBeats")) {
        trigger.loop_delay_beats_in_beats = 1;
        trigger.loop_delay_beats = layout_helper_parse_float(unparsed_trigger, "loopDelayBeats", 0);
    } else if (unparsed_trigger.hasAttribute("loopDelay")) {
        trigger.loop_delay_beats_in_beats = 0;
        trigger.loop_delay = layout_helper_parse_float(unparsed_trigger, "loopDelay", 0);
    }

    if (unparsed_trigger.hasAttribute("startDelayBeats")) {
        trigger.start_delay_beats_in_beats = 1;
        trigger.start_delay_beats = layout_helper_parse_float(unparsed_trigger, "startDelayBeats", 0);
    } else if (unparsed_trigger.hasAttribute("startDelay")) {
        trigger.start_delay_beats_in_beats = 0;
        trigger.start_delay = layout_helper_parse_float(unparsed_trigger, "startDelay", 0);
    }

    arraylist_add(layout_context.trigger_list, trigger);
}

async function layout_parse_sound(unparsed_sound, layout_context) {
    let src = unparsed_sound.getAttribute("src");
    if (!src) {
        console.error("layout_parse_sound() missing sound 'src'");
        return;
    }

    let soundplayer = await soundplayer_init(src);
    if (!soundplayer) {
        console.warn("layout_parse_sound() can not load:" + src);
        return;
    }

    let volume = layout_helper_parse_float(unparsed_sound, "volume", 1.0);
    let looped = vertexprops_parse_boolean(unparsed_sound, "looped", 0);
    //let pan = layout_helper_parse_float(unparsed_sound, "pan", 0.0);
    //let muted = vertexprops_parse_boolean(unparsed_sound, "muted", 0);
    let actions = unparsed_sound.querySelectorAll("Action");
    let actions_arraylist = arraylist_init2(actions.length);

    let sound = {
        name: unparsed_sound.getAttribute("name"),
        initial_action_name: unparsed_sound.getAttribute("initialAction"),
        soundplayer: soundplayer,

        actions_size: 0,
        actions: null,

        was_playing: 0
    };

    soundplayer_set_volume(sound.soundplayer, volume);
    soundplayer_loop_enable(sound.soundplayer, looped);

    for (let action of actions) {
        layout_parse_sound_action(action, layout_context.animlist, actions_arraylist);
    }
    arraylist_destroy2(actions_arraylist, sound, "actions_size", "actions");

    arraylist_add(layout_context.sound_list, sound);
}

async function layout_parse_video(unparsed_video, layout_context, group_id) {
    let src = unparsed_video.getAttribute("src");
    if (!src) {
        console.error("layout_parse_video() missing video 'src'");
        return;
    }

    let videoplayer = await videoplayer_init(src);
    if (!videoplayer) {
        console.warn("layout_parse_video() can not load:" + src);
        return;
    }

    let volume = layout_helper_parse_float(unparsed_video, "volume", 1.0);
    let looped = vertexprops_parse_boolean(unparsed_video, "looped", 0);
    //let pan = layout_helper_parse_float(unparsed_video, "pan", 0.0);
    //let muted = vertexprops_parse_boolean(unparsed_video, "muted", 0);
    let actions = unparsed_video.querySelectorAll("Action");
    let actions_arraylist = arraylist_init2(actions.length);

    let video = {
        name: unparsed_video.getAttribute("name"),
        initial_action_name: unparsed_video.getAttribute("initialAction"),
        videoplayer: videoplayer,

        actions_size: 0,
        actions: null,

        was_playing: 0,
        in_vertex_list_index: arraylist_size(layout_context.vertex_list)
    };

    videoplayer_set_volume(video.videoplayer, volume);
    videoplayer_loop_enable(video.videoplayer, looped);

    for (let action of actions) {
        await layout_parse_video_action(action, layout_context.animlist, actions_arraylist);
    }
    arraylist_destroy2(actions_arraylist, video, "actions_size", "actions");

    let sprite = {
        actions: null,
        actions_size: 0,
        animation: null,
        group_id: group_id,
        initial_action_name: null,
        name: null,
        parallax: { x: 1.0, y: 1.0, z: 1.0 },
        placeholder: null,
        soundplayer: null,
        videoplayer: videoplayer,
        static_camera: false,
        type: VERTEX_SPRITE,
        vertex: videoplayer_get_sprite(videoplayer),
        was_playing: false
    };

    arraylist_add(layout_context.vertex_list, sprite);
    arraylist_add(layout_context.video_list, video);
}

function layout_parse_macro(/** @type {Element} */ unparsed_root, layout_context) {
    //let list = unparsed_root.querySelectorAll(":scope > Macro");
    let list = unparsed_root.querySelectorAll("Macro");
    let macro_arraylist = arraylist_init2(list.length);

    for (let unparsed_macro of list) {
        if (!unparsed_macro.hasAttribute("name")) {
            console.warn("layout_parse_macro() missing name in:" + unparsed_macro.outerHTML);
            continue;
        }

        let name = unparsed_macro.getAttribute("name");
        let animation = unparsed_macro.getAttribute("animation") ?? name;
        let animsprite = animsprite_init_from_animlist(layout_context.animlist, animation);

        if (!animsprite) {
            console.warn(`layout_parse_macro() missing animation: ${animation}`);
            continue;
        }

        let macro = {
            name: name,
            loop: vertexprops_parse_integer(unparsed_macro, "loopByBeats", 0),
            loop_by_beats: vertexprops_parse_boolean(unparsed_macro, "loopByBeats", 0),
            actions: null,
            actions_size: 0
        };

        layout_parse_macro_actions(unparsed_macro.children, macro);
        arraylist_add(macro_arraylist, macro);
    }

    arraylist_destroy2(macro_arraylist, layout_context, "macro_list_size", "macro_list");
}


//////////////////////////////////
///        ACTION PARSER       ///
//////////////////////////////////

async function layout_parse_sprite_action(unparsed_action, animlist, atlas, action_entries, from_video) {
    let entries = arraylist_init2(unparsed_action.children.length);

    for (let unparsed_entry of unparsed_action.children) {
        switch (unparsed_entry.tagName) {
            case "Location":
                layout_helper_add_action_location(unparsed_entry, entries);
                break;
            case "Size":
                layout_helper_add_action_size(unparsed_entry, entries);
                break;
            case "OffsetColor":
                layout_helper_add_action_offsetcolor(unparsed_entry, entries);
                break;
            case "Color":
                layout_helper_add_action_color(unparsed_entry, entries);
                break;
            case "Property":
                layout_helper_add_action_property(unparsed_entry, 0, entries);
                break;
            case "Properties":
                layout_helper_add_action_properties(unparsed_entry, 0, entries);
                break;
            case "AtlasApply":
                layout_helper_add_action_atlasapply(unparsed_entry, atlas, entries);
                break;
            case "Resize":
                layout_helper_add_action_resize(unparsed_entry, entries);
                break;
            case "Animation":
                layout_helper_add_action_animation(unparsed_entry, animlist, entries);
                break;
            case "AnimationFromAtlas":
                layout_helper_add_action_animationfromatlas(unparsed_entry, atlas, entries);
                break;
            case "AnimationRemove":
                layout_helper_add_action_animationremove(unparsed_entry, entries);
                break;
            case "Parallax":
                layout_helper_add_action_parallax(unparsed_entry, entries);
                break;
            case "Modifier":
                layout_helper_add_action_modifier(unparsed_entry, entries);
                break;
            case "Static":
                layout_helper_add_action_static(unparsed_entry, entries);
                break;
            case "Hide":
            case "Show":
                layout_helper_add_action_visibility(unparsed_entry, entries);
                break;
            case "SetShader":
                await layout_helper_add_action_setshader(unparsed_entry, entries);
                break;
            case "RemoveShader":
                layout_helper_add_action_removeshader(unparsed_entry, entries);
                break;
            case "SetShaderUniform":
                layout_helper_add_action_setshaderuniform(unparsed_entry, entries);
                break;
            case "SetBlending":
                layout_helper_add_action_setblending(unparsed_entry, entries);
                break;
            case "SetTrailing":
                layout_helper_add_action_spritetrailing(unparsed_entry, entries);
                break;
            case "SetTrailingOffsetcolor":
                layout_helper_add_action_spritetrailingoffsetcolor(unparsed_entry, entries);
                break;
            default:
                if (!from_video) {
                    console.warn("layout_parse_sprite_action() unknown action entry: " + unparsed_entry.tagName);
                }
                break;
        }
    }

    let action = {
        name: unparsed_action.getAttribute("name"),
        entries: null,
        entries_size: -1
    };

    arraylist_destroy2(entries, action, "entries_size", "entries");
    arraylist_add(action_entries, action);
}

async function layout_parse_text_action(unparsed_action, animlist, action_entries) {
    let entries = arraylist_init2(unparsed_action.children.length);

    for (let unparsed_entry of unparsed_action.children) {
        switch (unparsed_entry.tagName) {
            case "Property":
                layout_helper_add_action_property(unparsed_entry, 1, entries);
                break;
            case "Properties":
                layout_helper_add_action_properties(unparsed_entry, 1, entries);
                break;
            case "Location":
                layout_helper_add_action_location(unparsed_entry, entries);
                break;
            case "Color":
                layout_helper_add_action_color(unparsed_entry, entries);
                break;
            case "MaxSize":
                layout_helper_add_action_size(unparsed_entry, entries);
                break;
            case "String":
                let entry = {
                    type: LAYOUT_ACTION_PROPERTY,
                    property: TEXTSPRITE_PROP_STRING,
                    misc: unparsed_entry.textContent,
                };
                arraylist_add(entries, entry);
                break;
            case "Border":
                layout_helper_add_action_textborder(unparsed_entry, entries);
                break;
            case "BorderOffset":
                layout_helper_add_action_textborderoffset(unparsed_entry, entries);
                break;
            case "Background":
                layout_helper_add_action_textbackground(unparsed_entry, entries);
                break;
            case "BackgroundColor":
                layout_helper_add_action_textbackgroundcolor(unparsed_entry, entries);
                break;
            case "Animation":
                layout_helper_add_action_animation(unparsed_entry, animlist, entries);
                break;
            case "AnimationRemove":
                layout_helper_add_action_animationremove(unparsed_entry, entries);
                break;
            case "Parallax":
                layout_helper_add_action_parallax(unparsed_entry, entries);
                break;
            case "Modifier":
                layout_helper_add_action_modifier(unparsed_entry, entries);
                break;
            case "Static":
                layout_helper_add_action_static(unparsed_entry, entries);
                break;
            case "Hide":
            case "Show":
                layout_helper_add_action_visibility(unparsed_entry, entries);
                break;
            case "SetShader":
                await layout_helper_add_action_setshader(unparsed_entry, entries);
                break;
            case "RemoveShader":
                layout_helper_add_action_removeshader(unparsed_entry, entries);
                break;
            case "SetShaderUniform":
                layout_helper_add_action_setshaderuniform(unparsed_entry, entries);
                break;
            case "SetBlending":
                layout_helper_add_action_setblending(unparsed_entry, entries);
                break;
            default:
                console.warn("layout_parse_text_action() unknown Text action entry:" + unparsed_entry.tagName);
                break;
        }
    }

    let action = {
        name: unparsed_action.getAttribute("name"),
        entries: null,
        entries_size: -1
    };

    arraylist_destroy2(entries, action, "entries_size", "entries");
    arraylist_add(action_entries, action);
}

async function layout_parse_group_action(unparsed_action, animlist, action_entries) {
    let entries = arraylist_init2(unparsed_action.children.length);

    for (let unparsed_entry of unparsed_action.children) {
        switch (unparsed_entry.tagName) {
            case "ResetMatrix":
                layout_helper_add_action_resetmatrix(unparsed_entry, entries);
                break;
            case "Modifier":
                layout_helper_add_action_modifier(unparsed_entry, entries);
                break;
            case "Property":
                layout_helper_add_action_property(unparsed_entry, 0, entries);
                break;
            case "Properties":
                layout_helper_add_action_properties(unparsed_entry, 0, entries);
                break;
            case "OffsetColor":
                layout_helper_add_action_offsetcolor(unparsed_entry, entries);
                break;
            case "Parallax":
                layout_helper_add_action_parallax(unparsed_entry, entries);
                break;
            case "Static":
                layout_helper_add_action_static(unparsed_entry, entries);
                break;
            case "Hide":
            case "Show":
                layout_helper_add_action_visibility(unparsed_entry, entries);
                break;
            case "Animation":
                layout_helper_add_action_animation(unparsed_entry, animlist, entries);
                break;
            case "AnimationRemove":
                layout_helper_add_action_animationremove(unparsed_entry, entries);
                break;
            case "SetShader":
                await layout_helper_add_action_setshader(unparsed_entry, entries);
                break;
            case "RemoveShader":
                layout_helper_add_action_removeshader(unparsed_entry, entries);
                break;
            case "SetShaderUniform":
                layout_helper_add_action_setshaderuniform(unparsed_entry, entries);
                break;
            case "SetBlending":
                layout_helper_add_action_setblending(unparsed_entry, entries);
                break;
            case "Viewport":
                layout_helper_add_action_viewport(unparsed_entry, entries);
                break;
        }
    }

    let action = {
        name: unparsed_action.getAttribute("name"),
        entries: null,
        entries_size: -1
    };

    arraylist_destroy2(entries, action, "entries_size", "entries");
    arraylist_add(action_entries, action);
}

function layout_parse_sound_action(unparsed_action, animlist, action_entries) {
    let entries = arraylist_init2(unparsed_action.children.length);

    for (let unparsed_entry of unparsed_action.children) {
        switch (unparsed_entry.tagName) {
            case "Property":
                layout_helper_add_action_mediaproperty(unparsed_entry, entries);
                break;
            case "Properties":
                layout_helper_add_action_mediaproperties(unparsed_entry, entries);
                break;
            case "FadeIn":
            case "FadeOut":
                layout_helper_add_action_soundfade(unparsed_entry, entries);
                break;
            case "Seek":
                layout_helper_add_action_seek(unparsed_entry, entries);
                break;
            /*case "Animation":
                layout_helper_add_action_animation(unparsed_entry, animlist, entries);
                break;
            case "AnimationRemove":
                layout_helper_add_action_animationremove(unparsed_entry, entries);*/
            default:
                if (layout_helper_add_action_media(unparsed_entry, entries))
                    console.warn("layout_parse_sound_action() unknown Sound action entry:" + unparsed_entry.tagName);
                break;
        }
    }

    let action = {
        name: unparsed_action.getAttribute("name"),
        entries: null,
        entries_size: -1
    };

    arraylist_destroy2(entries, action, "entries_size", "entries");
    arraylist_add(action_entries, action);
}

async function layout_parse_video_action(unparsed_action, animlist, action_entries) {
    let entries = arraylist_init2(unparsed_action.children.length);

    for (let unparsed_entry of unparsed_action.children) {
        switch (unparsed_entry.tagName) {
            case "Property":
                layout_helper_add_action_mediaproperty(unparsed_entry, entries);
                break;
            case "Properties":
                layout_helper_add_action_mediaproperties(unparsed_entry, entries);
                break;
            case "FadeIn":
            case "FadeOut":
                layout_helper_add_action_soundfade(unparsed_entry, entries);
                break;
            case "Seek":
                layout_helper_add_action_seek(unparsed_entry, entries);
                break;
            default:
                if (!layout_helper_add_action_media(unparsed_entry, entries)) break;
                await layout_parse_sprite_action(unparsed_action, animlist, null, action_entries, 1);
                break;
        }
    }

    let action = {
        name: unparsed_action.getAttribute("name"),
        entries: null,
        entries_size: -1
    };

    arraylist_destroy2(entries, action, "entries_size", "entries");
    arraylist_add(action_entries, action);
}

function layout_parse_macro_actions(unparsed_actions, macro) {
    let actions_arraylist = arraylist_init2(unparsed_actions.length);

    for (let unparsed_action of unparsed_actions) {
        let type;
        let target_name = null;
        let action_name = null;
        let trigger_name = null;
        let stop_trigger_name = null;
        let camera_name = null;

        switch (unparsed_action.tagName) {
            case "PuppetGroup":
                type = LAYOUT_ACTION_PUPPETGROUP;
                target_name = unparsed_action.getAttribute("target");
                break;
            case "Puppet":
                type = LAYOUT_ACTION_PUPPETITEM;
                target_name = unparsed_action.getAttribute("target");
                break;
            case "Execute":
                type = LAYOUT_ACTION_EXECUTE;
                target_name = unparsed_action.getAttribute("target");
                action_name = unparsed_action.getAttribute("action");
                trigger_name = unparsed_action.getAttribute("trigger");
                stop_trigger_name = unparsed_action.getAttribute("stopTrigger");
                camera_name = unparsed_action.getAttribute("camera");
                break;
            default:
                console.warn("layout_parse_macro_actions() unknown Macro action:" + unparsed_action.tagName);
                continue;
        }

        if (!unparsed_action.hasAttribute("id")) {
            console.warn("layout_parse_macro_actions() missing event id in Macro action:" + unparsed_action.outerHTML);
            target_name = undefined;
            action_name = undefined;
            trigger_name = undefined;
            stop_trigger_name = undefined;
            camera_name = undefined;
            continue;
        }

        let id = vertexprops_parse_integer(unparsed_action, "id", 0);
        let action = {
            type,
            id,
            target_name,
            action_name,
            trigger_name,
            stop_trigger_name,
            camera_name
        };

        arraylist_add(actions_arraylist, action);
    }

    arraylist_destroy2(actions_arraylist, macro, "actions_size", "actions");
}


//////////////////////////////////
///      ACTION EXECUTERS      ///
//////////////////////////////////

function layout_helper_execute_action_in_sprite(action, item, viewport_width, viewport_height) {
    const location = [0, 0];
    const draw_size = [0, 0];

    let sprite = item.vertex;

    for (let i = 0; i < action.entries_size; i++) {
        let entry = action.entries[i];

        switch (entry.type) {
            case LAYOUT_ACTION_LOCATION:

                // Note: The sprite must have a draw size
                sprite_get_draw_size(sprite, draw_size);

                if (sprite_is_textured(sprite)) {
                    layout_helper_location(
                        entry, draw_size[0], draw_size[1], viewport_width, viewport_height, location
                    );
                } else {
                    sprite_get_draw_location(sprite, location);
                    if (Number.isFinite(entry.x)) location[0] = entry.x;
                    if (Number.isFinite(entry.y)) location[1] = entry.y;
                }
                sprite_set_draw_location(sprite, location[0], location[1]);
                if (!Number.isNaN(entry.z)) sprite_set_z_index(sprite, entry.z);
                break;
            case LAYOUT_ACTION_SIZE:
                if (entry.has_resize)
                    sprite_resize_draw_size(sprite, entry.width, entry.height, null);
                else
                    sprite_set_draw_size(sprite, entry.width, entry.height);
                break;
            case LAYOUT_ACTION_OFFSETCOLOR:
                sprite_set_offsetcolor(
                    sprite, entry.rgba[0], entry.rgba[1], entry.rgba[2], entry.rgba[3]
                );
                break;
            case LAYOUT_ACTION_COLOR:
                sprite_set_vertex_color(sprite, entry.rgba[0], entry.rgba[1], entry.rgba[2]);
                if (!Number.isNaN(entry.rgba[3])) sprite_set_alpha(sprite, entry.rgba[3]);
                break;
            case LAYOUT_ACTION_PROPERTY:
                sprite_set_property(sprite, entry.property, entry.value);
                break;
            case LAYOUT_ACTION_ATLASAPPLY:
                atlas_apply_from_entry(sprite, entry.misc, entry.override_size);
                break;
            case LAYOUT_ACTION_RESIZE:
                imgutils_calc_resize_sprite(
                    sprite, entry.max_width, entry.max_height, entry.cover, entry.center
                );
                break;
            case LAYOUT_ACTION_ANIMATION:
                if (!entry.misc && !item.animation) break;
                if (entry.misc) item.animation = entry.misc;
                if (entry.restart) animsprite_restart(item.animation);
                if (entry.stop_in_loop) animsprite_disable_loop(item.animation);

                sprite_animation_play_by_animsprite(sprite, item.animation, 0);
                sprite_animate(sprite, 0);
                break;
            case LAYOUT_ACTION_ANIMATIONREMOVE:
                sprite_animation_play_by_animsprite(sprite, null, 0);
                break;
            case LAYOUT_ACTION_ANIMATIONEND:
                sprite_animation_end(sprite);
                break;
            case LAYOUT_ACTION_VISIBILITY:
                sprite_set_visible(sprite, entry.visible);
                break;
            case LAYOUT_ACTION_PARALLAX:
                layout_helper_set_parallax_info(item.parallax, entry);
                break;
            case LAYOUT_ACTION_MODIFIER:
                pvrctx_helper_copy_modifier(entry.misc, sprite_matrix_get_modifier(sprite));
                break;
            case LAYOUT_ACTION_STATIC:
                item.static_camera = entry.enable;
                break;
            case LAYOUT_ACTION_SETSHADER:
                sprite_set_shader(sprite, entry.misc);
                break;
            case LAYOUT_ACTION_REMOVESHADER:
                sprite_set_shader(sprite, null);
                break;
            case LAYOUT_ACTION_SETSHADERUNIFORM:
                let psshader = sprite_get_shader(sprite);
                layout_helper_set_shader_uniform(psshader, entry);
                break;
            case LAYOUT_ACTION_SETBLENDING:
                if (entry.has_enable) sprite_blend_enable(sprite, entry.enable);
                sprite_blend_set(sprite, entry.blend_src_rgb, entry.blend_dst_rgb, entry.blend_src_alpha, entry.blend_dst_alpha);
                break;
            case LAYOUT_ACTION_SPRITE_TRAILING:
                if (entry.has_enable) sprite_trailing_enabled(sprite, entry.enable);
                sprite_trailing_set_params(sprite, entry.length, entry.trail_delay, entry.trail_alpha, entry.has_darken ? entry.darken : null);
                break;
            case LAYOUT_ACTION_SPRITE_TRAILINGOFFSETCOLOR:
                sprite_trailing_set_offsetcolor(sprite, entry.rgba[0], entry.rgba[1], entry.rgba[2]);
                break;
        }
    }
}

function layout_helper_execute_action_in_textsprite(action, item, viewport_width, viewport_height) {
    let textsprite = item.vertex;
    for (let i = 0; i < action.entries_size; i++) {
        let entry = action.entries[i];
        switch (entry.type) {
            case LAYOUT_ACTION_TEXTBORDER:
                if (entry.has_enable) textsprite_border_enable(textsprite, entry.enable);
                if (!Number.isNaN(entry.size)) textsprite_border_set_size(textsprite, entry.size);
                textsprite_border_set_color(
                    textsprite, entry.rgba[0], entry.rgba[1], entry.rgba[2], entry.rgba[3]
                );
                break;
            case LAYOUT_ACTION_TEXTBORDEROFFSET:
                textsprite_border_set_offset(textsprite, action.x, action.y);
                break;
            case LAYOUT_ACTION_TEXTBACKGROUND:
                if (entry.has_enable) textsprite_background_enable(textsprite, entry.enable);
                textsprite_background_set_offets(textsprite, entry.x, entry.y);
                if (!Number.isNaN(entry.size)) textsprite_background_set_size(textsprite, entry.size);
                break;
            case LAYOUT_ACTION_TEXTBACKGROUNDCOLOR:
                textsprite_background_set_color(textsprite, entry.rgba[0], entry.rgba[1], entry.rgba[2], entry.rgba[3]);
                break;
            case LAYOUT_ACTION_PROPERTY:
                if (entry.property == TEXTSPRITE_PROP_STRING)
                    textsprite_set_text_intern(textsprite, 1, entry.misc);
                else
                    textsprite_set_property(textsprite, entry.property, entry.value);
                break;
            case LAYOUT_ACTION_LOCATION:
                const location = [0, 0];
                const draw_size = [0, 0];

                // NOTE: there must be text and size already set, otherwise
                // this will not work !!!
                textsprite_get_draw_size(textsprite, draw_size);
                layout_helper_location(
                    entry, draw_size[0], draw_size[1], viewport_width, viewport_height, location
                );
                textsprite_set_draw_location(textsprite, location[0], location[1]);
                if (!Number.isNaN(entry.z)) textsprite_set_z_index(textsprite, entry.z);
                break;
            case LAYOUT_ACTION_COLOR:
                textsprite_set_color(textsprite, entry.rgba[0], entry.rgba[1], entry.rgba[2]);
                if (!Number.isNaN(entry.rgba[3])) textsprite_set_alpha(textsprite, entry.rgba[3]);
                break;
            case LAYOUT_ACTION_SIZE:
                textsprite_set_max_draw_size(textsprite, entry.width, entry.height);
                break;
            case LAYOUT_ACTION_ANIMATION:
                if (!entry.misc && !item.animation) break;
                if (entry.misc) item.animation = entry.misc;
                if (entry.restart) animsprite_restart(item.animation);
                if (entry.stop_in_loop) animsprite_disable_loop(item.animation);

                textsprite_animation_set(textsprite, entry.misc);
                animsprite_animate(textsprite, 0);
                break;
            case LAYOUT_ACTION_ANIMATIONREMOVE:
                textsprite_animation_set(textsprite, null);
                break;
            case LAYOUT_ACTION_ANIMATIONEND:
                textsprite_animation_end(textsprite);
                break;
            case LAYOUT_ACTION_VISIBILITY:
                textsprite_set_visible(textsprite, entry.visible);
                break;
            case LAYOUT_ACTION_PARALLAX:
                layout_helper_set_parallax_info(item.parallax, entry);
                break;
            case LAYOUT_ACTION_MODIFIER:
                pvrctx_helper_copy_modifier(entry.misc, textsprite_matrix_get_modifier(textsprite));
                break;
            case LAYOUT_ACTION_STATIC:
                item.static_camera = entry.enable;
                break;
            case LAYOUT_ACTION_SETSHADER:
                textsprite_set_shader(textsprite, entry.misc);
                break;
            case LAYOUT_ACTION_REMOVESHADER:
                textsprite_set_shader(textsprite, null);
                break;
            case LAYOUT_ACTION_SETSHADERUNIFORM:
                let psshader = textsprite_get_shader(textsprite);
                layout_helper_set_shader_uniform(psshader, entry);
                break;
            case LAYOUT_ACTION_SETBLENDING:
                if (entry.has_enable) textsprite_blend_enable(textsprite, entry.enable);
                textsprite_blend_set(textsprite, entry.blend_src_rgb, entry.blend_dst_rgb, entry.blend_src_alpha, entry.blend_dst_alpha);
                break;
        }
    }
}

function layout_helper_execute_action_in_group(action, group) {
    for (let i = 0; i < action.entries_size; i++) {
        let entry = action.entries[i];
        switch (entry.type) {
            case LAYOUT_ACTION_RESETMATRIX:
                pvrctx_helper_clear_modifier(group.modifier);
                //sh4matrix_reset(group.matrix);
                break;
            case LAYOUT_ACTION_MODIFIER:
                pvrctx_helper_copy_modifier(entry.misc, group.modifier);
                //sh4matrix_apply_modifier(group.matrix, entry.misc);
                break;
            case LAYOUT_ACTION_PROPERTY:
                layout_helper_group_set_property(group, entry.property, entry.value);
                break;
            case LAYOUT_ACTION_OFFSETCOLOR:
                group.offsetcolor[0] = entry.rgba[0];
                group.offsetcolor[1] = entry.rgba[1];
                group.offsetcolor[2] = entry.rgba[2];
                group.offsetcolor[3] = entry.rgba[3];
                break;
            case LAYOUT_ACTION_PARALLAX:
                layout_helper_set_parallax_info(group.parallax, entry);
                break;
            case LAYOUT_ACTION_VISIBILITY:
                group.visible = entry.visible;
                break;
            case LAYOUT_ACTION_ANIMATION:
                if (!entry.misc && !group.animation) break;
                if (entry.misc) group.animation = entry.misc;
                if (entry.restart) animsprite_restart(group.animation);
                if (entry.stop_in_loop) animsprite_disable_loop(group.animation);

                layout_helper_group_animate(group, 0);
                break;
            case LAYOUT_ACTION_ANIMATIONREMOVE:
                group.animation = null;
                break;
            case LAYOUT_ACTION_ANIMATIONEND:
                if (group.animation) {
                    animsprite_force_end(group.animation);
                    animsprite_update_using_callback(
                        group.animation, group, layout_helper_group_set_property, 1
                    );
                }
                break;
            case LAYOUT_ACTION_STATIC:
                group.static_camera = entry.enable;
                break;
            case LAYOUT_ACTION_SETSHADER:
                group.psshader = entry.misc;
                break;
            case LAYOUT_ACTION_REMOVESHADER:
                group.psshader = null;
                break;
            case LAYOUT_ACTION_SETSHADERUNIFORM:
                layout_helper_set_shader_uniform(group.psshader, entry);
                break;
            case LAYOUT_ACTION_SETBLENDING:
                if (entry.has_enable) group.blend_enabled = entry.enable;
                group.blend_src_rgb = entry.src_rgb;
                group.blend_dst_rgb = entry.dst_rgb;
                group.blend_src_alpha = entry.src_alpha;
                group.blend_dst_alpha = entry.dst_alpha;
                break;
            case LAYOUT_ACTION_VIEWPORT:
                if (!Number.isNaN(entry.x)) group.viewport_x = entry.x;
                if (!Number.isNaN(entry.y)) group.viewport_y = entry.y;
                if (!Number.isNaN(entry.width)) group.viewport_width = entry.width;
                if (!Number.isNaN(entry.height)) group.viewport_height = entry.height;
                break;
        }
    }
}

function layout_helper_execute_action_in_sound(action, item) {
    let soundplayer = item.soundplayer;
    for (let i = 0; i < action.entries_size; i++) {
        let entry = action.entries[i];
        switch (entry.type) {
            case LAYOUT_ACTION_PROPERTY:
                soundplayer_set_property(soundplayer, entry.property, entry.value);
                break;
            case LAYOUT_ACTION_SOUNDFADE:
                soundplayer_fade(soundplayer, entry.enable, entry.size);
                break;
            case LAYOUT_ACTION_SEEK:
                soundplayer_seek(soundplayer, entry.position);
                break;
            /*case LAYOUT_ACTION_ANIMATION:
                if (!entry.misc && !item.animation) break;
                if (entry.misc) item.animation = entry.misc;
                if (entry.restart) animsprite_restart(item.animation);
                if (entry.stop_in_loop) animsprite_disable_loop(item.animation);

                soundplayer_animation_set(soundplayer, entry.misc);
                soundplayer_animate(soundplayer, 0);
                break;
            case LAYOUT_ACTION_ANIMATIONREMOVE:
                soundplayer_animation_set(soundplayer, null);
                break;
            case LAYOUT_ACTION_ANIMATIONEND:
                soundplayer_animation_end(soundplayer);
                break;*/
        }
    }
}

function layout_helper_execute_action_in_video(action, item_video, item_sprite, viewport_width, viewport_height) {
    let videoplayer = item_video.videoplayer;
    for (let i = 0; i < action.entries_size; i++) {
        let entry = action.entries[i];
        switch (entry.type) {
            case LAYOUT_ACTION_PROPERTY:
                videoplayer_set_property(videoplayer, entry.property, entry.value);
                break;
            case LAYOUT_ACTION_SOUNDFADE:
                videoplayer_fade_audio(videoplayer, entry.enable, entry.size);
                break;
            case LAYOUT_ACTION_SEEK:
                videoplayer_seek(videoplayer, entry.position);
                break;
            default:
                layout_helper_execute_action_in_sprite(action, item_sprite, viewport_width, viewport_height);
                break;
        }
    }
}




//////////////////////////////////
///    ACTION ENTRY PARSERS    ///
//////////////////////////////////

function layout_helper_add_action_property(unparsed_entry, is_textsprite, action_entries) {
    let property_id = vertexprops_parse_sprite_property(unparsed_entry, "name", !is_textsprite);
    if (property_id == -1 && is_textsprite) {
        property_id = vertexprops_parse_textsprite_property(unparsed_entry, "name", 1);
    } else if (property_id == -1) {
        property_id = vertexprops_parse_layout_property(unparsed_entry, "name", 1);
    }

    if (property_id < 0) return;

    layout_helper_parse_property(unparsed_entry, property_id, "value", action_entries);
}

function layout_helper_add_action_properties(unparsed_entry, is_textsprite, action_entries) {
    if (unparsed_entry.attributes.length < 1) {
        console.warn(
            "layout_helper_add_action_properties() 'Properties' was empty" + unparsed_entry.outerHTML
        );
        return;
    }

    for (let attribute of unparsed_entry.attributes) {
        let name = attribute.name;
        let property_id;

        property_id = vertexprops_parse_sprite_property2(name);
        if (property_id == -1 && is_textsprite) {
            property_id = vertexprops_parse_textsprite_property2(name);
        } else if (property_id == -1) {
            property_id = vertexprops_parse_layout_property2(name);
        }

        if (property_id < 0) {
            console.warn(
                "layout_helper_add_action_properties() unknown property '" +
                name + "' in: " + unparsed_entry.outerHTML
            );
            continue;
        }

        layout_helper_parse_property(unparsed_entry, property_id, name, action_entries);
    }

}

function layout_helper_add_action_offsetcolor(unparsed_entry, action_entries) {
    let entry = {
        type: LAYOUT_ACTION_OFFSETCOLOR,
        rgba: [NaN, NaN, NaN, NaN]
    };
    layout_helper_parse_color(unparsed_entry, entry.rgba);
    arraylist_add(action_entries, entry);
    return entry;
}

function layout_helper_add_action_color(unparsed_entry, action_entries) {
    let entry = {
        type: LAYOUT_ACTION_COLOR,
        rgba: [NaN, NaN, NaN, NaN]
    };
    layout_helper_parse_color(unparsed_entry, entry.rgba);
    arraylist_add(action_entries, entry);
}

function layout_helper_add_action_modifier(unparsed_entry, action_entries) {
    const modifier_mask = {};
    let action_entry = { type: LAYOUT_ACTION_MODIFIER, misc: modifier_mask };

    pvrctx_helper_clear_modifier(modifier_mask);
    for (let field in modifier_mask) modifier_mask[field] = NaN;

    for (let i = 0; i < unparsed_entry.attributes.length; i++) {
        let attribute = unparsed_entry.attributes[i].name;
        let name = attribute.toLowerCase();
        let value;

        switch (name) {
            case "rotatepivotenabled":
            case "scalesize":
            case "scaletranslation":
            case "translaterotation":
                value = vertexprops_parse_boolean(unparsed_entry, attribute, 0);
                break;
            default:
                value = layout_helper_parse_float(unparsed_entry, attribute, NaN);
                if (Number.isNaN(value)) continue;
                break;
        }

        switch (name) {
            case "x":
                modifier_mask.x = value;
                break;
            case "y":
                modifier_mask.y = value;
                break;
            case "translate":
                modifier_mask.translate_x = modifier_mask.translate_y = value;
                break;
            case "translatex":
                modifier_mask.translate_x = value;
                break;
            case "translatey":
                modifier_mask.translate_y = value;
                break;
            case "scale":
                modifier_mask.scale_x = modifier_mask.scale_y = value;
                break;
            case "scalex":
                modifier_mask.scale_x = value;
                break;
            case "scaley":
                modifier_mask.scale_y = value;
                break;
            case "skew":
                modifier_mask.skew_x = modifier_mask.skew_y = value;
                break;
            case "skewx":
                modifier_mask.skew_x = value;
                break;
            case "skewy":
                modifier_mask.skew_y = value;
                break;
            case "rotate":
                modifier_mask.rotate = value * MATH2D_DEG_TO_RAD;
                break;
            case "scaledirection":
                modifier_mask.scale_direction_x = modifier_mask.scale_direction_y = value;
                break;
            case "scaledirectionx":
                modifier_mask.scale_direction_x = value;
                break;
            case "scaledirectiony":
                modifier_mask.scale_direction_y = value;
                break;
            case "rotatepivot":
                modifier_mask.rotate_pivot_u = modifier_mask.rotate_pivot_v = value;
                break;
            case "rotatepivotu":
                modifier_mask.rotate_pivot_u = value;
                break;
            case "rotatepivotv":
                modifier_mask.rotate_pivot_v = value;
                break;
        }
    }

    arraylist_add(action_entries, action_entry);
}

function layout_helper_add_action_parallax(unparsed_entry, action_entries) {
    let entry = {
        type: LAYOUT_ACTION_PARALLAX,
        x: NaN,
        y: NaN,
        z: NaN
    };

    for (let i = 0; i < unparsed_entry.attributes.length; i++) {
        let name = unparsed_entry.attributes[i].name;
        let value = layout_helper_parse_float(unparsed_entry, name, NaN);

        if (Number.isNaN(value)) continue;

        switch (name) {
            case "x":
                entry.x = value;
                break;
            case "y":
                entry.y = value;
                break;
            case "z":
                entry.z = value;
                break;
        }
    }

    arraylist_add(action_entries, entry);
}

function layout_helper_add_action_location(unparsed_entry, action_entries) {
    let entry = {
        type: LAYOUT_ACTION_LOCATION,
        align_vertical: layout_helper_parse_align(unparsed_entry, 1),
        align_horizontal: layout_helper_parse_align(unparsed_entry, 0),
        x: layout_helper_parse_float(unparsed_entry, "x", 0),
        y: layout_helper_parse_float(unparsed_entry, "y", 0),
        z: layout_helper_parse_float(unparsed_entry, "z", NaN),
    };
    arraylist_add(action_entries, entry);
}

function layout_helper_add_action_size(unparsed_entry, action_entries) {
    let width = layout_helper_parse_float(unparsed_entry, "width", NaN);
    let height = layout_helper_parse_float(unparsed_entry, "height", NaN);
    if (Number.isNaN(width) && Number.isNaN(height)) {
        console.error("layout_helper_add_action_size() invalid size: " + unparsed_entry.outerHTML);
        return;
    }

    let has_resize = width < 0 || height < 0;
    if (has_resize && (Number.isNaN(width) || Number.isNaN(height))) {
        console.warn("layout_helper_add_action_size() invalid resize: " + unparsed_entry.outerHTML);
        return;
    }

    let entry = { type: LAYOUT_ACTION_SIZE, width, height, has_resize };
    arraylist_add(action_entries, entry);
}

function layout_helper_add_action_visibility(unparsed_entry, action_entries) {
    let entry = { type: LAYOUT_ACTION_VISIBILITY, visible: unparsed_entry.tagName != "Hide" };
    arraylist_add(action_entries, entry);
}

function layout_helper_add_action_atlasapply(unparsed_entry, atlas, action_entries) {
    if (!atlas) {
        console.error(
            "layout_helper_add_action_atlasapply() missing atlas, can not import: " +
            unparsed_entry.outerHTML
        );
        return;
    }

    let atlas_entry_name = unparsed_entry.getAttribute("entry");
    let atlas_entry = atlas_get_entry_copy(atlas, atlas_entry_name);

    if (!atlas_entry) {
        console.warn(`layout_helper_add_action_atlasapply() missing atlas entry name '${atlas_entry_name}': ${unparsed_entry.outerHTML}`);
        return;
    }

    let entry = {
        type: LAYOUT_ACTION_ATLASAPPLY,
        misc: atlas_entry,
        override_size: vertexprops_parse_boolean(unparsed_entry, "overrideSize", 0)
    };

    arraylist_add(action_entries, entry);
}

function layout_helper_add_action_animation(unparsed_entry, animlist, action_entries) {
    if (!animlist) {
        console.error("layout_helper_add_action_animation() failed, missing animlist");
        return;
    }

    let anim_name = unparsed_entry.getAttribute("name");
    let animsprite = animsprite_init_from_animlist(animlist, anim_name);

    if (!animsprite) {
        console.warn(`layout_helper_add_action_animation() missing animation '${anim_name}': ${unparsed_entry.outerHTML}`);
        return;
    }

    let entry = {
        type: LAYOUT_ACTION_ANIMATION,
        misc: animsprite,
        restart: vertexprops_parse_boolean(unparsed_entry, "restart", 1),
        stop_in_loop: vertexprops_parse_boolean(unparsed_entry, "stopOnLoop", 0)
    };

    arraylist_add(action_entries, entry);
}

function layout_helper_add_action_animationfromatlas(unparsed_entry, atlas, action_entries) {
    if (!atlas) {
        console.error(
            "layout_helper_add_action_animationfromatlas() failed, sprite has no atlas. " + unparsed_entry.outerHTML
        );
        return;
    }

    let anim_name = unparsed_entry.getAttribute("name");
    if (!anim_name) {
        console.error("layout_helper_add_action_animationfromatlas() missing animation name on: " + unparsed_entry.outerHTML);
        return;
    }

    let loop = vertexprops_parse_integer(unparsed_entry, "loop", 1);
    let has_number_suffix = vertexprops_parse_boolean(unparsed_entry, "hasNumberSuffix", 1);
    let fps = vertexprops_parse_float(unparsed_entry, "fps", 0);

    if (fps < 1) {
        fps = atlas_get_glyph_fps(atlas);
        if (fps < 1) fps = FUNKIN_DEFAULT_ANIMATIONS_FRAMERATE;
    }

    let animsprite = animsprite_init_from_atlas(fps, loop, atlas, anim_name, has_number_suffix);
    let override_size = vertexprops_parse_boolean(unparsed_entry, "overrideSize", 0);

    if (!animsprite) {
        console.warn(`layout_helper_add_action_animationfromatlas() missing animation '${anim_name}': ${unparsed_entry.outerHTML}`);
        return;
    }

    let entry = {
        type: LAYOUT_ACTION_ANIMATION,
        misc: animsprite,
        restart: vertexprops_parse_boolean(unparsed_entry, "restart", 1),
        stop_in_loop: vertexprops_parse_boolean(unparsed_entry, "stopOnLoop", 0)
    };

    animsprite_allow_override_sprite_size(animsprite, override_size);

    arraylist_add(action_entries, entry);
}

function layout_helper_add_action_animationremove(unparsed_entry, action_entries) {
    let entry = { type: LAYOUT_ACTION_ANIMATIONREMOVE };
    arraylist_add(action_entries, entry);
}

function layout_helper_add_action_animationend(unparsed_entry, action_entries) {
    let entry = { type: LAYOUT_ACTION_ANIMATIONEND };
    arraylist_add(action_entries, entry);
}

function layout_helper_add_action_textborder(unparsed_entry, action_entries) {
    let entry = {
        type: LAYOUT_ACTION_TEXTBORDER,
        size: layout_helper_parse_float(unparsed_entry, "size", NaN),
        has_enable: unparsed_entry.hasAttribute("enable"),
        enable: vertexprops_parse_boolean(unparsed_entry, "enable", 0),
        rgba: [NaN, NaN, NaN, NaN]
    };

    let alpha = layout_helper_parse_float(unparsed_entry, "alpha", NaN);
    layout_helper_parse_color(unparsed_entry, entry.rgba);

    if (Number.isFinite(alpha)) entry.rgba[3] = alpha;

    arraylist_add(action_entries, entry);
}

function layout_helper_add_action_resize(unparsed_entry, action_entries) {
    let max_width = layout_helper_parse_float(unparsed_entry, "maxWidth", NaN);
    let max_height = layout_helper_parse_float(unparsed_entry, "maxHeight", NaN);

    if (Number.isNaN(max_width) && Number.isNaN(max_height)) {
        console.error("layout_helper_add_action_resize() invalid resize: " + unparsed_entry.outerHTML);
        return;
    }

    if (Number.isNaN(max_width)) max_width = -1;
    if (Number.isNaN(max_height)) max_height = -1;

    let entry = {
        type: LAYOUT_ACTION_RESIZE,
        max_width,
        max_height,
        cover: vertexprops_parse_boolean(unparsed_entry, "cover", 0),
        center: vertexprops_parse_boolean(unparsed_entry, "center", 0)
    };

    arraylist_add(action_entries, entry);
}

function layout_helper_add_action_resetmatrix(unparsed_entry, action_entries) {
    let entry = { type: LAYOUT_ACTION_RESETMATRIX };
    arraylist_add(action_entries, entry);
}

function layout_helper_add_action_static(unparsed_entry, action_entries) {
    let entry = {
        type: LAYOUT_ACTION_STATIC,
        enable: vertexprops_parse_boolean(unparsed_entry, "enable", true)
    };
    arraylist_add(action_entries, entry);
}

function layout_helper_add_action_media(unparsed_entry, action_entries) {
    let value;

    switch (unparsed_entry.tagName) {
        case "Play":
            value = PLAYBACK_PLAY;
            break;
        case "Pause":
            value = PLAYBACK_PAUSE;
            break;
        case "Stop":
            value = PLAYBACK_STOP;
            break;
        case "Mute":
            value = PLAYBACK_MUTE;
            break;
        case "Unmute":
            value = PLAYBACK_UNMUTE;
            break;
        default:
            return 1;
    }

    let entry = {
        type: LAYOUT_ACTION_PROPERTY,
        property: MEDIA_PROP_PLAYBACK,
        value,
        misc: null,
    };

    arraylist_add(action_entries, entry);
    return 0;
}

function layout_helper_add_action_mediaproperty(unparsed_entry, action_entries) {
    let property_id = vertexprops_parse_media_property(unparsed_entry, "name", 0);
    if (property_id < 0) return;
    layout_helper_parse_property(unparsed_entry, property_id, "value", action_entries);
}

function layout_helper_add_action_mediaproperties(unparsed_entry, action_entries) {
    if (unparsed_entry.attributes.length < 1) {
        console.warn(
            "layout_helper_add_action_properties() 'Properties' was empty" + unparsed_entry.outerHTML
        );
        return;
    }

    for (let attribute of unparsed_entry.attributes) {
        let name = attribute.name;
        let property_id;

        property_id = vertexprops_parse_media_property2(name);

        if (property_id < 0) {
            console.warn(
                "layout_helper_add_action_mediaproperties() unknown property '" +
                name + "' in: " + unparsed_entry.outerHTML
            );
            continue;
        }

        layout_helper_parse_property(unparsed_entry, property_id, name, action_entries);
    }

}

async function layout_helper_add_action_setshader(unparsed_entry, action_entries) {
    let shader_vertex_src = unparsed_entry.getAttribute("vertexSrc");
    let shader_fragment_src = unparsed_entry.getAttribute("fragmentSrc");
    let shader_sources = unparsed_entry.children;

    if (!shader_fragment_src && !shader_vertex_src && shader_sources.length < 1) {
        layout_helper_add_action_removeshader(unparsed_entry, action_entries);
        return;
    }


    let sourcecode_vertex = stringbuilder_init();
    let sourcecode_fragment = stringbuilder_init();

    if (shader_vertex_src) {
        let tmp = await fs_readtext(shader_vertex_src);
        if (tmp) stringbuilder_add(sourcecode_vertex, tmp);
        tmp = undefined;
    }

    if (shader_fragment_src) {
        let tmp = await fs_readtext(shader_fragment_src);
        if (tmp) stringbuilder_add(sourcecode_fragment, tmp);
        tmp = undefined;
    }

    // parse source elements
    for (let source of shader_sources) {
        let target;
        switch (source.tagName) {
            case "VertexSource":
                target = sourcecode_vertex;
                break;
            case "FragmentSource":
                target = sourcecode_fragment;
                break;
            default:
                console.warn(`layout_helper_add_action_setshader() unknown element: ${source.outerHTML}`);
                continue;
        }

        stringbuilder_add_char_codepoint(target, 0x0A);// newline char
        stringbuilder_add(target, source.textContent);
    }

    let str_vertex = stringbuilder_finalize(sourcecode_vertex);
    let str_fragment = stringbuilder_finalize(sourcecode_fragment);

    if (!str_vertex && !str_fragment) {
        str_vertex = undefined;
        str_fragment = undefined;
        console.warn(`layout_helper_add_action_setshader() empty shader: ${unparsed_entry.outerHTML}`);
        return;
    }

    // assume layout as part of the main PVRContext renderer
    let psshader = PSShader.BuildFromSource(pvr_context, str_vertex, str_fragment);
    str_vertex = undefined;
    str_fragment = undefined;

    if (!psshader) {
        console.warn(`layout_helper_add_action_setshader() compilation failed: ${unparsed_entry.outerHTML}`);
        return;
    }

    let entry = { type: LAYOUT_ACTION_SETSHADER, misc: psshader };
    arraylist_add(action_entries, entry);
}

function layout_helper_add_action_removeshader(unparsed_entry, action_entries) {
    let entry = { type: LAYOUT_ACTION_REMOVESHADER };
    arraylist_add(action_entries, entry);
}

function layout_helper_add_action_setshaderuniform(unparsed_entry, action_entries) {

    let values = new Array(16);
    for (let i = 0; i < 16; i++) values[i] = 0.0;

    let name = unparsed_entry.getAttribute("name");
    let value = vertexprops_parse_double(unparsed_entry, "value", NaN);
    let unparsed_values = unparsed_entry.getAttribute("values");

    if (!name) {
        console.error("layout_helper_add_action_setshaderuniform() missing name: " + unparsed_entry.outerHTML);
        return;
    }

    if (unparsed_values != null) {
        // separator: white-space, hard-space, tabulation, carrier-return, new-line
        let tokenizer = tokenizer_init("\x20\xA0\t\r\n", 1, 0, unparsed_values);
        let index = 0;
        let str;

        while ((str = tokenizer_read_next(tokenizer)) != null) {
            let temp_value = vertexprops_parse_double2(str, NaN);
            if (Number.isNaN(temp_value)) {
                console.warn("layout_helper_add_action_setshaderuniform() invalid value: " + str);
                temp_value = 0.0;
            }

            str = undefined;
            values[index++] = temp_value;
            if (index >= 16) break;
        }
        tokenizer_destroy(tokenizer);
    } else if (!Number.isNaN(value)) {
        values[0] = value;
    }


    let entry = { type: LAYOUT_ACTION_SETSHADERUNIFORM, uniform_name: name, misc: values };
    arraylist_add(action_entries, entry);
}

function layout_helper_add_action_setblending(unparsed_entry, action_entries) {
    let has_enable = unparsed_entry.hasAttribute("enabled");
    let enabled = vertexprops_parse_boolean(unparsed_entry, "enabled", 1);

    let blend_src_rgb = vertexprops_parse_blending(unparsed_entry.getAttribute("srcRGB"));
    let blend_dst_rgb = vertexprops_parse_blending(unparsed_entry.getAttribute("dstRGB"));
    let blend_src_alpha = vertexprops_parse_blending(unparsed_entry.getAttribute("srcAlpha"));
    let blend_dst_alpha = vertexprops_parse_blending(unparsed_entry.getAttribute("dstAlpha"));

    let entry = {
        type: LAYOUT_ACTION_SETBLENDING,
        enabled,
        has_enable,
        blend_src_rgb,
        blend_dst_rgb,
        blend_src_alpha,
        blend_dst_alpha
    };

    arraylist_add(action_entries, entry);
}

function layout_helper_add_action_viewport(unparsed_entry, action_entries) {
    let x = vertexprops_parse_float(unparsed_entry, "x", NaN);
    let y = vertexprops_parse_float(unparsed_entry, "y", NaN);
    let width = vertexprops_parse_float(unparsed_entry, "width", NaN);
    let height = vertexprops_parse_float(unparsed_entry, "height", NaN);

    let entry = {
        type: LAYOUT_ACTION_VIEWPORT,
        x, y, width, height
    };

    arraylist_add(action_entries, entry);
}

function layout_helper_add_action_textborderoffset(unparsed_entry, action_entries) {
    let offset_x = layout_helper_parse_float(unparsed_entry, "offsetX", NaN);
    let offset_y = layout_helper_parse_float(unparsed_entry, "offsetY", NaN);
    if (Number.isNaN(offset_x) && Number.isNaN(offset_y)) {
        console.error("layout_helper_add_action_borderoffser() invalid offset: " + unparsed_entry.outerHTML);
        return;
    }

    let entry = { type: LAYOUT_ACTION_TEXTBORDEROFFSET, x: offset_x, y: offset_y };
    arraylist_add(action_entries, entry);
}

function layout_helper_add_action_textbackground(unparsed_entry, action_entries) {
    let enable = vertexprops_parse_boolean(unparsed_entry, "enable", 0);
    let has_enable = unparsed_entry.hasAttribute("enable");
    let size = layout_helper_parse_float(unparsed_entry, "size", NaN);
    let offset_x = layout_helper_parse_float(unparsed_entry, "offsetX", NaN);
    let offset_y = layout_helper_parse_float(unparsed_entry, "offsetY", NaN);

    let entry = {
        type: LAYOUT_ACTION_TEXTBACKGROUND,
        size: size,
        x: offset_x, y: offset_y,
        has_enable: has_enable,
        enable: enable
    };
    arraylist_add(action_entries, entry);
}

function layout_helper_add_action_textbackgroundcolor(unparsed_entry, action_entries) {
    let entry = {
        type: LAYOUT_ACTION_TEXTBACKGROUNDCOLOR,
        rgba: [NaN, NaN, NaN, NaN]
    };

    layout_helper_parse_color(unparsed_entry, entry.rgba);

    arraylist_add(action_entries, entry);
}

function layout_helper_add_action_spritetrailing(unparsed_entry, action_entries) {
    let enable = vertexprops_parse_boolean(unparsed_entry, "enable", 0);
    let has_enable = unparsed_entry.hasAttribute("enable");
    let length = vertexprops_parse_integer(unparsed_entry, "length", -1);
    let trail_delay = vertexprops_parse_float(unparsed_entry, "trailDelay", NaN);
    let trail_alpha = vertexprops_parse_float(unparsed_entry, "trailAlpha", NaN);
    let darken = vertexprops_parse_boolean(unparsed_entry, "darkenColors", false);
    let has_darken = unparsed_entry.hasAttribute("darkenColors");

    let entry = {
        type: LAYOUT_ACTION_SPRITE_TRAILING,
        enable: enable,
        has_enable: has_enable,
        length: length,
        trail_delay: trail_delay,
        trail_alpha: trail_alpha,
        darken: darken,
        has_darken: has_darken
    };

    arraylist_add(action_entries, entry);
}

function layout_helper_add_action_spritetrailingoffsetcolor(unparsed_entry, action_entries) {
    let entry = {
        type: LAYOUT_ACTION_SPRITE_TRAILINGOFFSETCOLOR,
        rgba: [NaN, NaN, NaN, NaN]
    };

    layout_helper_parse_color(unparsed_entry, entry.rgba);

    arraylist_add(action_entries, entry);
}

function layout_helper_add_action_soundfade(unparsed_entry, action_entries) {
    let entry = {
        type: LAYOUT_ACTION_SOUNDFADE,
        enable: unparsed_entry.tagName == "FadeIn",
        size: layout_helper_parse_float(unparsed_entry, "duration", 1000.0)
    };

    arraylist_add(action_entries, entry);
}

function layout_helper_add_action_seek(unparsed_entry, action_entries) {
    let entry = {
        type: LAYOUT_ACTION_SEEK,
        position: vertexprops_parse_double(unparsed_entry, "position", 0.0)
    };

    arraylist_add(action_entries, entry);
}

