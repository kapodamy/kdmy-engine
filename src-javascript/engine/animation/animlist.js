"use strict";


const ANIM_MACRO_INTERPOLATOR = 0;
const ANIM_MACRO_SET = 1;
const ANIM_MACRO_YIELD = 2;
const ANIM_MACRO_PAUSE = 3;
const ANIM_MACRO_RESET = 4;
const ANIM_MACRO_RANDOM_SETUP = 5;
const ANIM_MACRO_RANDOM_CHOOSE = 6;
const ANIM_MACRO_RANDOM_EXACT = 7;
const ANIM_MACRO_REGISTER_PROP = 8;
const ANIM_MACRO_REGISTER_SET = 9;

const ANIM_MACRO_INTERPOLATOR_EASE = 0;
const ANIM_MACRO_INTERPOLATOR_EASE_IN = 1;
const ANIM_MACRO_INTERPOLATOR_EASE_OUT = 2;
const ANIM_MACRO_INTERPOLATOR_EASE_IN_OUT = 3;
const ANIM_MACRO_INTERPOLATOR_LINEAR = 4;
const ANIM_MACRO_INTERPOLATOR_STEPS = 5;
const ANIM_MACRO_INTERPOLATOR_CUBIC = 6;
const ANIM_MACRO_INTERPOLATOR_QUAD = 7;
const ANIM_MACRO_INTERPOLATOR_EXPO = 8;
const ANIM_MACRO_INTERPOLATOR_SIN = 9;

const ANIM_MACRO_INTERPOLATOR_MODIFIER_SINE = 0;
const ANIM_MACRO_INTERPOLATOR_MODIFIER_COSINE = 1;
const ANIM_MACRO_INTERPOLATOR_MODIFIER_LOG = 2;
const ANIM_MACRO_INTERPOLATOR_MODIFIER_EXP = 3;

const ANIMLIST_POOL = new Map();
var ANIMLIST_IDS = 0;

async function animlist_init(src) {
    let full_path = await fs_get_full_path_and_override(src);

    // find a previous loaded animlist
    for (const [obj] of ANIMLIST_POOL) {
        if (obj.src == full_path) {
            obj.references++;
            full_path = undefined;
            return obj;
        }
    }

    let xml;

    try {
        xml = await fs_readxml(src);
    } catch (e) {
        if (e instanceof KDMYEngineIOError) {
            console.error(`animlist_init() error loading "${src}"`, e);
            return null;
        }

        // malformed/corrupt xml file
        throw e;
    }

    // C and C# only
    //fs_folder_stack_push();
    //fs_set_working_folder(full_path, 1);



    let anims_list = xml.querySelector("AnimationList");
    console.assert(anims_list != null, xml);

    let anims = anims_list.children;
    let atlas_cache = linkedlist_init();
    let parsed_animations = linkedlist_init();
    let default_fps = vertexprops_parse_float(anims_list, "frameRate", FUNKIN_DEFAULT_ANIMATIONS_FRAMERATE);
    let default_atlas;
    let is_macro;

    if (anims_list.hasAttribute("atlasPath")) {
        let default_atlas_path = fs_build_path2(full_path, anims_list.getAttribute("atlasPath"));
        default_atlas = await animlist_load_atlas(default_atlas_path);
        default_atlas_path = undefined;
    } else {
        default_atlas = null;
    }

    for (let i = 0; i < anims.length; i++) {

        switch (anims[i].tagName) {
            case "Animation":
                is_macro = 0;
                break;
            case "AnimationMacro":
                is_macro = 1;
                break;
            case "TweenKeyframe":
                linkedlist_add_item(parsed_animations, animlist_read_tweenkeyframe_animation(anims[i]));
                continue;
            default:
                console.warn("animlist_init() unknown animation: " + anims[i].tagName);
                continue;
        }

        let atlas = await animlist_load_required_atlas(
            anims[i], atlas_cache, default_atlas, is_macro, full_path
        );
        let animlist_item;

        if (is_macro)
            animlist_item = animlist_read_macro_animation(anims[i], atlas);
        else
            animlist_item = animlist_read_frame_animation(anims[i], atlas, default_fps);

        if (animlist_item) linkedlist_add_item(parsed_animations, animlist_item);
    }

    let animlist = {};
    animlist.entries_count = linkedlist_count(parsed_animations);
    animlist.entries = linkedlist_to_array(parsed_animations);

    // destroy the list and release all allocated items
    linkedlist_destroy2(parsed_animations, free);

    if (default_atlas) atlas_destroy(default_atlas);

    // dispose atlas cache
    for (let entry of linkedlist_iterate4(atlas_cache)) {
        if (entry.atlas) atlas_destroy(entry.atlas);
        entry.path = undefined;
        entry = undefined;
    }

    linkedlist_destroy(atlas_cache);

    animlist.src = full_path;
    animlist.id = ANIMLIST_IDS++;
    animlist.references = 1;
    ANIMLIST_POOL.set(animlist.id, animlist);

    // C and C# only
    //fs_folder_stack_pop();

    return animlist;
}

function animlist_destroy(animlist) {
    animlist.references--;
    if (animlist.references > 0) return;

    for (let i = 0; i < animlist.entries_count; i++) {
        animlist.entries[i].frames = undefined;
        animlist.entries[i].alternate_set = undefined;

        for (let j = 0; j < animlist.entries[i].instructions_count; j++) {
            animlist.entries[i].instructions[j].values = undefined;
            animlist.entries[i].instructions[j] = undefined;
        }

        if (animlist.entries[i].is_tweenkeyframe) animlist.entries[i].tweenkeyframe_entries = undefined;

        animlist.entries[i].frames = undefined;
        animlist.entries[i].instructions = undefined;


        ModuleLuaScript.kdmyEngine_drop_shared_object(animlist.entries[i]);
        animlist.entries[i] = undefined;
    }

    animlist.entries = undefined;
    ModuleLuaScript.kdmyEngine_drop_shared_object(animlist);
    ANIMLIST_POOL.delete(animlist.id);
    animlist = undefined;
}

function animlist_get_animation(animlist, animation_name) {
    for (let i = 0; i < animlist.entries_count; i++)
        if (animlist.entries[i].name == animation_name)
            return animlist.entries[i];

    return null;
}

function animlist_is_item_macro_animation(animlist_item) {
    return animlist_item.instructions_count > 0;
}

function animlist_is_item_frame_animation(animlist_item) {
    return !animlist_item.is_tweenkeyframe && animlist_item.instructions_count < 1;
}

function animlist_is_item_tweenkeyframe_animation(animlist_item) {
    return animlist_item.is_tweenkeyframe;
}



async function animlist_load_required_atlas(animlist_item, atlas_list, def_atlas, is_macro, ref_path) {
    let filename = animlist_item.getAttribute("atlasPath");

    if (!filename) {
        if (is_macro) return null;

        if (!def_atlas) {
            console.warn(
                "animlist_load_required_atlas() animation without atlas", animlist_item.outerHTML
            );
        }
        return def_atlas;
    }

    let obj = null;

    for (let entry of linkedlist_iterate4(atlas_list)) {
        if (entry.path == filename) {
            obj = entry;
            break;
        }
    }

    if (!obj) {
        // JS only (build path)
        let atlas_path = fs_build_path2(ref_path, filename);
        obj = { path: filename, atlas: await animlist_load_atlas(atlas_path) };
        atlas_path = undefined;
        linkedlist_add_item(atlas_list, obj);
    }

    if (!obj.atlas) {
        console.warn(
            "animlist_load_required_atlas() missing atlas " + filename, animlist_item.outerHTML
        );
    }

    return obj.atlas;
}

async function animlist_load_atlas(src) {
    return src ? await atlas_init(src) : null;
}

function animlist_read_frame_animation(entry, atlas, default_fps) {
    let name = entry.getAttribute("name");
    if (!name) {
        console.error("animlist_read_frame_animation() missing animation name", entry.outerHTML);
        return null;
    }
    if (!atlas) {
        console.error("animlist_read_frame_animation() missing atlas", entry.outerHTML);
        return null;
    }

    let anim = {};
    anim.is_tweenkeyframe = 0;
    anim.name = name;
    anim.loop = vertexprops_parse_integer(entry, "loop", 1);
    anim.frame_rate = vertexprops_parse_float(entry, "frameRate", default_fps);
    anim.alternate_per_loop = vertexprops_parse_boolean(entry, "alternateInLoops", 0);
    anim.alternate_no_random = !vertexprops_parse_boolean(entry, "alternateRandomize", 0);
    anim.loop_from_index = 0;

    let frames = entry.children;
    let parsed_frames = linkedlist_init();
    let parsed_alternates = linkedlist_init();
    let last_alternate_index = 0;

    for (let i = 0; i < frames.length; i++) {
        switch (frames[i].tagName) {
            case "FrameArray":
                let name_prefix = frames[i].getAttribute("entryPrefixName");
                let name_suffix = frames[i].getAttribute("entrySuffixName");
                let has_number_suffix = vertexprops_parse_boolean(frames[i], "hasNumberSuffix", 1);
                let index_start = vertexprops_parse_integer(frames[i], "indexStart", 0);
                let index_end = vertexprops_parse_integer(frames[i], "indexEnd", -1);

                let frame_name = name_prefix ? name_prefix : name;
                if (name_suffix) frame_name += ` ${name_suffix}`;// add space before suffix         

                animlist_read_entries_to_frames_array(
                    parsed_frames, frame_name, has_number_suffix, atlas, index_start, index_end
                );

                //if (name_prefix) name_prefix = undefined;
                //if (name_suffix) name_suffix = undefined;
                frame_name = undefined;
                break;
            case "Frame":
                let name_frame = frames[i].getAttribute("entryName");
                if (name_frame) name = name_frame;

                animlist_add_entry_from_atlas(parsed_frames, name, atlas);
                if (name_frame != name) name_frame = undefined;
                break;
            case "Pause":
                let duration = vertexprops_parse_float(frames[i], "duration", 1);
                let last_frame = linkedlist_get_last_item(parsed_frames);
                if (last_frame) {
                    while (duration-- > 0) linkedlist_add_item(parsed_frames, last_frame);
                }
                break;
            case "AlternateSet":
                let frame_count = linkedlist_count(parsed_frames);
                if (animlist_add_alternate_entry(parsed_alternates, frame_count, last_alternate_index))
                    console.error(`animlist_read_frame_animation() consecutive AlternateSet found (no enough frames in '${name}')`);
                else
                    last_alternate_index = frame_count;
                break;
            case "LoopMark":
                let offset = vertexprops_parse_integer(frames[i], "lastFrames", 0);
                anim.loop_from_index = linkedlist_count(parsed_frames) - offset;
                break;
            default:
                console.error("animlist_read_frame_animation() unknown frame type: " + frames[i].tagName, frames[i]);
                break;
        }
    }

    anim.frame_count = linkedlist_count(parsed_frames);
    anim.frames = linkedlist_to_solid_array(parsed_frames);

    anim.instructions = null;
    anim.instructions_count = 0;

    if (linkedlist_count(parsed_alternates) > 0) {
        // add the last frames set
        let frames_count = linkedlist_count(parsed_frames);
        animlist_add_alternate_entry(parsed_alternates, frames_count, last_alternate_index);
    }
    anim.alternate_set = linkedlist_to_solid_array(parsed_alternates);
    anim.alternate_set_size = linkedlist_count(parsed_alternates);

    if (anim.loop_from_index >= anim.frame_count || anim.loop_from_index < 0) {
        anim.loop_from_index = 0;
    }

    linkedlist_destroy(parsed_frames);

    return anim;
}

function animlist_copy_entries_to_frames_array(frame_list, name, has_number_suffix, atlas) {
    let start = name.length;

    for (let i = 0; i < atlas.size; i++) {
        let atlas_entry_name = atlas.entries[i].name;
        if (!atlas_entry_name.startsWith(name)) continue;

        if (has_number_suffix && !atlas_name_has_number_suffix(atlas_entry_name, start))
            continue;

        linkedlist_add_item(frame_list, atlas.entries[i]);
    }
}

function animlist_read_entries_to_frames_array(frame_list, name, has_number_suffix, atlas, start, end) {
    let index_name_start = name.length;

    if (end < 0) end = atlas.size;

    let found_index = 0;

    for (let i = 0; i < atlas.size; i++) {
        let atlas_entry_name = atlas.entries[i].name;
        if (!atlas_entry_name.startsWith(name)) continue;

        if (has_number_suffix && !atlas_name_has_number_suffix(atlas_entry_name, index_name_start))
            continue;

        if (found_index >= start) {
            linkedlist_add_item(frame_list, atlas.entries[i]);
            if (found_index >= end) break;
        }

        found_index++;
    }
}

function animlist_add_entry_from_atlas(frame_list, name, atlas) {
    for (let i = 0; i < atlas.size; i++) {
        let entry = atlas.entries[i];

        if (entry.name == name) {
            linkedlist_add_item(frame_list, entry);
            return;
        }
    }

    console.error("animlist_add_entry_from_atlas() Missing atlas entry: " + name, atlas);
}


function animlist_parse_step_method(node) {
    return vertexprops_parse_align(node, "stepsMethod", 0, 0);
}

function animlist_parse_interpolator(node, name) {
    let type = node.getAttribute(name);

    if (!type)
        return ANIM_MACRO_INTERPOLATOR_LINEAR;

    type = type.toLowerCase();

    switch (type) {
        case "ease":
            return ANIM_MACRO_INTERPOLATOR_EASE;
        case "ease-in":
            return ANIM_MACRO_INTERPOLATOR_EASE_IN;
        case "ease-out":
            return ANIM_MACRO_INTERPOLATOR_EASE_OUT;
        case "ease-in-out":
            return ANIM_MACRO_INTERPOLATOR_EASE_IN_OUT;
        case "linear":
            return ANIM_MACRO_INTERPOLATOR_LINEAR;
        case "steps":
            return ANIM_MACRO_INTERPOLATOR_STEPS;
        case "cubic":
            return ANIM_MACRO_INTERPOLATOR_CUBIC;
        case "quad":
            return ANIM_MACRO_INTERPOLATOR_QUAD;
        case "expo":
            return ANIM_MACRO_INTERPOLATOR_EXPO;
        case "sin":
            return ANIM_MACRO_INTERPOLATOR_SIN;
    }

    console.warn("animlist_parse_interpolator() unknown interpolator type " + type);
    return ANIM_MACRO_INTERPOLATOR_LINEAR;
}

function animlist_parse_register(node) {
    let value = node.getAttribute("register");
    if (value != null) {
        value = value.toLowerCase();
        for (let i = 0; i < MACROEXECUTOR_REGISTER_COUNT; i++) {
            if (value == `reg${i}`) return i;
        }
        console.warn("animlist_parse_register() invalid register", value);
    }
    return -1;
}

function animlist_parse_complex_value(unparsed_value, def_value, value) {
    if (unparsed_value == null || unparsed_value == "") {
        // returnd default value (as literal)
        value.kind = ANIM_MACRO_VALUE_KIND_LITERAL;
        value.literal = def_value;
        return;
    }

    // special values used in macroexecutor
    let as_special = unparsed_value.toLowerCase();
    value.kind = ANIM_MACRO_VALUE_KIND_SPECIAL;
    switch (as_special) {
        case "rnd":
            value.reference = ANIM_MACRO_SPECIAL_RANDOM;
            return;
        case "reg0":
            value.reference = ANIM_MACRO_SPECIAL_REGISTER0;
            return;
        case "reg1":
            value.reference = ANIM_MACRO_SPECIAL_REGISTER1;
            return;
        case "reg2":
            value.reference = ANIM_MACRO_SPECIAL_REGISTER2;
            return;
        case "reg3":
            value.reference = ANIM_MACRO_SPECIAL_REGISTER3;
            return;
    }

    // boolean and numbers are literals
    value.kind = ANIM_MACRO_VALUE_KIND_LITERAL;

    // check if is a boolean value
    switch (unparsed_value.toLowerCase()) {
        case "true":
            value.literal = 1;
            return;
        case "false":
            value.literal = 0;
            return;
    }

    const as_unsigned = [0];
    if (vertexprops_parse_hex(unparsed_value, as_unsigned, true)) {
        value.literal = as_unsigned;
        return;
    }

    // check if the value is just a number
    let as_number = Number.parseFloat(unparsed_value);
    if (!Number.isNaN(as_number)) {
        value.literal = as_number;
        return;
    }

    // check if the value is a property name
    let as_property = vertexprops_parse_textsprite_property2(unparsed_value);
    if (as_property < 0) as_property = vertexprops_parse_sprite_property2(unparsed_value);
    if (as_property < 0) as_property = vertexprops_parse_media_property2(unparsed_value);
    if (as_property < 0) as_property = vertexprops_parse_layout_property2(unparsed_value);

    if (as_property >= 0) {
        value.kind = ANIM_MACRO_VALUE_KIND_PROPERTY;
        value.reference = as_property;
        return;
    }

    // check if the value is a property value enum
    let as_enum = vertexprops_parse_flag2(unparsed_value, -1);
    if (as_enum < 0) as_enum = vertexprops_parse_textsprite_forcecase2(unparsed_value);
    if (as_enum < 0) as_enum = vertexprops_parse_wordbreak2(unparsed_value);
    if (as_enum < 0) as_enum = vertexprops_parse_playback2(unparsed_value);
    if (as_enum < 0) as_enum = vertexprops_parse_align2(unparsed_value);
    if (as_enum >= 0) {
        value.kind = ANIM_MACRO_VALUE_KIND_LITERAL;
        value.literal = as_enum;
        return;
    }

    // no matches, return the default value
    value.kind = ANIM_MACRO_VALUE_KIND_LITERAL;
    value.literal = def_value;
}

function animlist_parse_complex_value2(node, name, def_value, value) {
    animlist_parse_complex_value(node.getAttribute(name), def_value, value);
}





function animlist_add_alternate_entry(list, frame_count, index) {
    let length = frame_count - index;
    if (length < 1) return 1;
    linkedlist_add_item(list, { index, length });
    return 0;
}


function animlist_parse_float_with_rate(node, attr_name, frame_time, def_value) {
    let value = vertexprops_parse_float(node, attr_name, def_value);
    if (isNaN(value)) return NaN;

    return value * frame_time;
}

function animlist_read_macro_animation(entry, atlas) {

    let anim = {};
    anim.is_tweenkeyframe = 0;
    anim.name = entry.getAttribute("name");
    anim.loop = vertexprops_parse_integer(entry, "loop", 1);
    anim.frames = null;
    anim.frames_count = 0;
    anim.frame_restart_index = vertexprops_parse_integer(entry, "frameRestartIndex", -1);
    anim.frame_allow_size_change = vertexprops_parse_boolean(entry, "frameAllowChangeSize", 0);

    anim.alternate_set = null;
    anim.alternate_set_size = 0;

    let atlasPrefixEntryName = entry.getAttribute("atlasPrefixEntryName");
    let atlasHasNumberSuffix = vertexprops_parse_boolean(entry, "atlasHasNumberSuffix", 1);

    let unparsed_list = entry.children;
    let parsed_instructions = linkedlist_init();

    let instruction, property_id;

    for (let i = 0; i < unparsed_list.length; i++) {
        switch (unparsed_list[i].tagName) {
            case "Interpolator":
                property_id = animlist_parse_property(unparsed_list[i], "property", 1);
                if (property_id < 0) continue;

                instruction = {
                    type: ANIM_MACRO_INTERPOLATOR,
                    interpolator: animlist_parse_interpolator(unparsed_list[i], "type"),
                    property: property_id,
                    start: { reference: -1, literal: NaN, kind: ANIM_MACRO_VALUE_KIND_LITERAL },
                    end: { reference: -1, literal: NaN, kind: ANIM_MACRO_VALUE_KIND_LITERAL },
                    duration: { reference: -1, literal: NaN, kind: ANIM_MACRO_VALUE_KIND_LITERAL },
                    steps_count: { reference: -1, literal: NaN, kind: ANIM_MACRO_VALUE_KIND_LITERAL },
                    steps_method: { reference: -1, literal: NaN, kind: ANIM_MACRO_VALUE_KIND_LITERAL }
                };

                animlist_parse_complex_value2(unparsed_list[i], "start", NaN, instruction.start);
                animlist_parse_complex_value2(unparsed_list[i], "end", NaN, instruction.end);
                animlist_parse_complex_value2(unparsed_list[i], "duration", 0, instruction.duration);
                animlist_parse_complex_value2(unparsed_list[i], "steps_count", 1, instruction.steps_count);
                animlist_parse_complex_value2(unparsed_list[i], "steps_method", 0, instruction.steps_method);

                linkedlist_add_item(parsed_instructions, instruction);
                break;
            case "Set":
                property_id = animlist_parse_property(unparsed_list[i], "property", 1);
                if (property_id < 0) continue;

                instruction = {
                    type: ANIM_MACRO_SET,
                    property: property_id,
                    value: { reference: -1, literal: NaN, kind: ANIM_MACRO_VALUE_KIND_LITERAL }
                };

                animlist_parse_complex_value2(unparsed_list[i], "value", 0, instruction.value);

                linkedlist_add_item(parsed_instructions, instruction);
                break;
            case "Yield":
                instruction = {
                    type: ANIM_MACRO_YIELD,
                    value: { reference: -1, literal: NaN, kind: ANIM_MACRO_VALUE_KIND_LITERAL }
                };

                animlist_parse_complex_value2(unparsed_list[i], "duration", 0, instruction.value);

                linkedlist_add_item(parsed_instructions, instruction);
                break;
            case "Pause":
                instruction = {
                    type: ANIM_MACRO_PAUSE,
                    value: { reference: -1, literal: NaN, kind: ANIM_MACRO_VALUE_KIND_LITERAL }
                };

                animlist_parse_complex_value2(unparsed_list[i], "duration", 1, instruction.value);

                linkedlist_add_item(parsed_instructions, instruction);
                break;
            case "Reset":
                instruction = {
                    type: ANIM_MACRO_RESET
                };
                linkedlist_add_item(parsed_instructions, instruction);
                break;
            case "RandomSetup":
                instruction = {
                    type: ANIM_MACRO_RANDOM_SETUP,
                    start: { reference: -1, literal: NaN, kind: ANIM_MACRO_VALUE_KIND_LITERAL },
                    end: { reference: -1, literal: NaN, kind: ANIM_MACRO_VALUE_KIND_LITERAL }
                };

                animlist_parse_complex_value2(unparsed_list[i], "start", 0.0, instruction.start);
                animlist_parse_complex_value2(unparsed_list[i], "end", 1.0, instruction.end);

                linkedlist_add_item(parsed_instructions, instruction);
                break;
            case "RandomChoose":
                instruction = {
                    type: ANIM_MACRO_RANDOM_CHOOSE
                };
                linkedlist_add_item(parsed_instructions, instruction);
                break;
            case "RandomExact":
                const values_size = [0];
                let values_array = animlist_parse_randomexact(
                    unparsed_list[i], values_size
                );
                if (values_array == null) continue;

                instruction = {
                    type: ANIM_MACRO_RANDOM_EXACT,
                    values: values_array,
                    values_size: values_size[0]
                };
                linkedlist_add_item(parsed_instructions, instruction);
                break;
            case "RegisterProp":
                property_id = animlist_parse_property(unparsed_list[i], "property", 1);
                if (property_id < 0) continue;

                instruction = {
                    type: ANIM_MACRO_REGISTER_PROP,
                    register_index: animlist_parse_register(unparsed_list[i]),
                    property: property_id
                };
                linkedlist_add_item(parsed_instructions, instruction);
                break;
            case "RegisterSet":
                instruction = {
                    type: ANIM_MACRO_REGISTER_SET,
                    register_index: animlist_parse_register(unparsed_list[i]),
                    value: { reference: -1, literal: NaN, kind: ANIM_MACRO_VALUE_KIND_LITERAL }
                };

                animlist_parse_complex_value2(unparsed_list[i], "value", 0.0, instruction.value);

                linkedlist_add_item(parsed_instructions, instruction);
                break;
            default:
                console.warn(
                    "animlist_read_macro_animation() unknown instruction: " + unparsed_list[i].tagName,
                    unparsed_list[i].outerHTML
                );
                break;
        }
    }

    anim.instructions_count = linkedlist_count(parsed_instructions);
    anim.instructions = linkedlist_to_solid_array(parsed_instructions);
    linkedlist_destroy2(parsed_instructions, free);// note: keep "instruction[].values" allocated

    if (atlasPrefixEntryName) {
        let parsed_frames = linkedlist_init();

        animlist_copy_entries_to_frames_array(
            parsed_frames, atlasPrefixEntryName, atlasHasNumberSuffix, atlas
        );
        anim.frame_count = linkedlist_count(parsed_frames);
        anim.frames = linkedlist_to_solid_array(parsed_frames);

        linkedlist_destroy(parsed_frames);
    } else {
        anim.frame_count = 0;
        anim.frames = null;
    }

    return anim;
}

function animlist_parse_randomexact(unparsed_randomexact, out_size) {
    let unparsed_values = unparsed_randomexact.getAttribute("values");

    let tokenizer = tokenizer_init("\x20", 1, 0, unparsed_values);
    if (!tokenizer) {
        console.warn("missing attribute values in RandomExact", unparsed_randomexact);

        out_size[0] = -1;
        return null;
    }

    let parsed_values = arraylist_init();
    let str;

    while ((str = tokenizer_read_next(tokenizer)) != null) {
        const parsed_value = { reference: -1, literal: NaN, kind: ANIM_MACRO_VALUE_KIND_LITERAL };
        animlist_parse_complex_value(str, NaN, parsed_value);

        if (parsed_value.reference == TEXTSPRITE_PROP_STRING && parsed_value.kind == ANIM_MACRO_VALUE_KIND_PROPERTY) {
            console.error(
                "animlist_read_macro_animation() illegal property used: string",
                unparsed_randomexact.outerHTML
            );

            str = undefined;
            continue;
        }

        if (Number.isNaN(parsed_value.literal) && parsed_value.kind == ANIM_MACRO_VALUE_KIND_LITERAL) {
            console.error(
                "animlist_read_macro_animation() invalid or unreconized value found",
                unparsed_randomexact.outerHTML
            );

            //free(string);
            continue;
        }

        arraylist_add(parsed_values, parsed_value);
        str = undefined;// In C free() the returned string
    }

    tokenizer_destroy(tokenizer);

    let values_array = [null];
    out_size[0] = arraylist_size(parsed_values);

    arraylist_destroy2(parsed_values, values_array, null, 0);
    return values_array[0];
}

function animlist_parse_property(node, name, warn) {
    let value = vertexprops_parse_textsprite_property(node, name, 0);
    if (value < 0) value = vertexprops_parse_sprite_property(node, name, 0);
    if (value < 0) value = vertexprops_parse_media_property(node, name, 0);
    if (value < 0) value = vertexprops_parse_layout_property(node, name, 0);
    if (value < 0) value = vertexprops_parse_camera_property(node, name, 0);

    if (value == TEXTSPRITE_PROP_STRING) {
        console.error("animlist_parse_property() illegal property: string", node.outerHTML);
        return -1;
    }

    if (value < 0 && warn) {
        console.warn("animlist_parse_property() unknown property: ", node.outerHTML);
    }

    return value;
}

function animlist_read_tweenkeyframe_animation(entry) {
    let nodes = entry.querySelectorAll("Keyframe");
    let arraylist = arraylist_init2(nodes.length);

    let reference_duration = 1;
    if (entry.hasAttribute("referenceDuration")) {
        reference_duration = vertexprops_parse_float(entry, "referenceDuration", NaN);
        if (Number.isNaN(reference_duration)) {
            console.warn("animlist_read_tweenkeyframe_animation() invalid tweenkeyframe 'referenceDuration' value: " + entry.outerHTML);
            reference_duration = 1;
        }
    }

    for (let node of nodes) {
        // <Keyframe at="80%" id="alpha" interpolator="steps" stepsMethod="both" stepsCount="34" value="1.0" />
        // <Keyframe at="1000" id="translateX" interpolator="ease" value="123" />

        let unparsed_at = node.getAttribute("at");
        if (!unparsed_at) {
            console.warn("animlist_read_tweenkeyframe_animation() missing Keyframe 'at' attribute: " + node.outerHTML);
            continue;
        }

        let at = NaN;

        if (unparsed_at.indexOf('%') >= 0) {
            if (reference_duration > 1) {
                console.warn("animlist_read_tweenkeyframe_animation() invalid Keyframe , 'at' is a percent value and TweenKeyframe have 'referenceDuration' attribute: " + node.outerHTML);
                continue;
            }

            if (unparsed_at.length > 1) {
                let str = unparsed_at.substring(0, unparsed_at.length - 1);
                at = vertexprops_parse_float2(str, NaN);
                str = undefined;
            }
        } else {
            if (reference_duration < 1) {
                console.warn("animlist_read_tweenkeyframe_animation() invalid Keyframe , 'at' is a timestamp value and TweenKeyframe does not have 'referenceDuration' attribute: " + node.outerHTML);
                continue;
            }
            at = vertexprops_parse_float2(unparsed_at, NaN);
        }

        if (Number.isNaN(at)) {
            console.warn("animlist_read_tweenkeyframe_animation() invalid 'at' value: " + node.outerHTML);
            continue;
        }

        if (reference_duration > 1)
            at /= reference_duration;
        else
            at /= 100.0;


        let id = animlist_parse_property(node, "id", 0);

        let keyframe_interpolator = animlist_parse_interpolator(node, "type");

        let steps_count = vertexprops_parse_integer(node, "stepsCount", -1);
        if (keyframe_interpolator == ANIM_MACRO_INTERPOLATOR_STEPS && steps_count < 0) {
            console.warn("animlist_read_tweenkeyframe_animation() invalid o missing 'stepsCount' value: " + node.outerHTML);
            continue;
        }

        let steps_dir = vertexprops_parse_align2(node.getAttribute("stepsMethod"));
        if (keyframe_interpolator == ANIM_MACRO_INTERPOLATOR_STEPS && (steps_dir == ALIGN_CENTER || steps_dir < 0)) {
            console.warn("animlist_read_tweenkeyframe_animation() invalid o missing 'stepsMethod' value: " + node.outerHTML);
            continue;
        }

        let value = vertexprops_parse_float(node, "value", NaN);
        if (Number.isNaN(value)) {
            console.warn("animlist_read_tweenkeyframe_animation() invalid 'value' value: " + node.outerHTML);
            continue;
        }

        let keyframe = {
            at, id, value, interpolator: keyframe_interpolator, steps_dir, steps_count
        };

        arraylist_add(arraylist, keyframe);
    }

    let interpolator = ANIM_MACRO_INTERPOLATOR_LINEAR;
    if (entry.hasAttribute("defaultInterpolator"))
        interpolator = animlist_parse_interpolator(entry, "defaultInterpolator")


    let item = {
        name: entry.getAttribute("name"),
        tweenkeyframe_default_interpolator: interpolator,
        is_tweenkeyframe: 1,
        tweenkeyframe_entries: null,
        tweenkeyframe_entries_count: 0
    };

    arraylist_destroy2(arraylist, item, "tweenkeyframe_entries_count", "tweenkeyframe_entries");

    return item;
}

