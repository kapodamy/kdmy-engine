"use strict";

const CHARACTERMANIFEST_DEFAULT_OPPOSITE_DIR_FROM = ["left", "right"];
const CHARACTERMANIFEST_DEFAULT_OPPOSITE_DIR_TO = ["right", "left"];

async function charactermanifest_init(src, gameplay_required_models_only) {
    let json = await json_load_from(src);
    if (!json) throw new Error("missing or invalid file: " + src);

    fs_folder_stack_push();
    fs_set_working_folder(src, 1);

    let model_character = null;
    let model_health_icons = null;
    let model_week_selector = null;
    let has_healthbar_color = json_has_property_hex(json, "healthbarColor");
    let healthbar_color = json_read_hex(json, "healthbarColor", 0x00FFFF);

    if (gameplay_required_models_only) {
        model_character = charactermanifest_internal_path_of(json, "model", src);
        if (json_has_property_string(json, "healthIconsModel") && !json_is_property_null(json, "healthIconsModel")) {
            model_health_icons = charactermanifest_internal_path_of(json, "healthIconsModel", src);
        }
    } else {
        model_week_selector = charactermanifest_internal_path_of(json, "modelWeekSelector", src);
    }

    let character_manifest = {
        model_character,
        model_health_icons,
        model_week_selector,
        has_healthbar_color,
        healthbar_color,

        sing_suffix: json_read_string(json, "singSuffix", null),
        sing_alternate_suffix: json_read_string(json, "singAlternateSuffix", null),
        sing_prefix: json_read_string(json, "singPrefix", null),
        sing_alternate_prefix: json_read_string(json, "singAlternatePrefix", null),
        allow_alternate_idle: json_read_string(json, "allowAlternateIdle", false),
        continuous_idle: json_read_boolean(json, "continuousIdle", false),
        actions_apply_chart_speed: json_read_boolean(json, "actionsApplyChartSpeed", false),

        offset_x: json_read_number(json, "offsetX", 0),
        offset_y: json_read_number(json, "offsetY", 0),
        left_facing: json_read_boolean(json, "leftFacing", false),
        week_selector_enable_beat: json_read_boolean(json, "weekSelectorEnableBeat", true),

        actions: {
            sing: null, sing_size: 0,
            miss: null, miss_size: 0,
            extras: null, extras_size: 0,
            idle: {},
            hey: {},
            has_idle: false,
            has_hey: false
        },

        align_vertical: ALIGN_CENTER,
        align_horizontal: ALIGN_CENTER,

        has_reference_size: false,
        reference_width: -1,
        reference_height: -1,

        opposite_directions: {
            sizes: 2,
            from: CHARACTERMANIFEST_DEFAULT_OPPOSITE_DIR_FROM,
            to: CHARACTERMANIFEST_DEFAULT_OPPOSITE_DIR_TO
        },

        additional_states_size: 0,
        additional_states: null
    };

    let json_actions = json_read_object(json, "actions");
    character_manifest_internal_parse_actions(json_actions, character_manifest.actions, src);

    character_manifest.align_vertical = charactermanifest_internal_align(json, "alignVertical", 1);
    character_manifest.align_horizontal = charactermanifest_internal_align(json, "alignHorizontal", 0);

    character_manifest.has_reference_size = json_has_property(json, "referenceSize");
    if (character_manifest.has_reference_size) {
        if (!json_has_property_object(json, "referenceSize")) {
            throw new Error(`invalid 'referenceSize' in ${src}`);
        }
        character_manifest.reference_width = json_read_number(json, "width", -1);
        character_manifest.reference_height = json_read_number(json, "height", -1);
    }

    let opposite_directions = json_read_object(json, "oppositeDirections");
    if (opposite_directions) {

        let from_array = json_read_array(opposite_directions, "from");
        let from_array_size = json_read_array_length(from_array);
        let to_array = json_read_array(opposite_directions, "to");
        let to_array_size = json_read_array_length(to_array);

        if (from_array_size != to_array_size) {
            throw new Error(
                `invalid 'oppositeDirections.from', and/or 'oppositeDirectionsto' in ${src}`
            );
        }

        if (to_array_size == 0) {
            character_manifest.opposite_directions.sizes = 0;
            character_manifest.opposite_directions.from = null;
            character_manifest.opposite_directions.to = null;
        } else if (to_array_size > 0) {
            character_manifest.opposite_directions.sizes = from_array_size;
            character_manifest.opposite_directions.from = new Array(from_array_size);
            character_manifest.opposite_directions.to = new Array(to_array_size);

            charactermanifest_internal_parse_opposite_dir(
                from_array, from_array_size, character_manifest.opposite_directions.from
            );
            charactermanifest_internal_parse_opposite_dir(
                to_array, to_array_size, character_manifest.opposite_directions.to
            );
        }
    } else {

    }

    let json_additional_states = json_read_array(json, "additionalStates");
    let additional_states = character_manifest_internal_read_additional_states(
        json_additional_states, src
    );
    arraylist_destroy2(additional_states, character_manifest, "additional_states_size", "additional_states");

    json_destroy(json);
    fs_folder_stack_pop();

    return character_manifest;
}

function charactermanifest_destroy(character_manifest) {
    character_manifest.model_character = undefined;
    character_manifest.model_week_selector = undefined;
    character_manifest.model_health_icons = undefined;
    character_manifest.sing_suffix = undefined;
    character_manifest.sing_alternate_suffix = undefined;
    character_manifest.sing_alternate_prefix = undefined;
    character_manifest.allow_alternate_idle = undefined;

    character_manifest_internal_destroy_actions(character_manifest.actions);

    if (character_manifest.opposite_directions.from != CHARACTERMANIFEST_DEFAULT_OPPOSITE_DIR_FROM) {
        for (let i = 0; i < character_manifest.opposite_directions.sizes; i++) {
            character_manifest.opposite_directions.from[i] = undefined;
        }
        character_manifest.opposite_directions.from = undefined;
    }

    if (character_manifest.opposite_directions.to != CHARACTERMANIFEST_DEFAULT_OPPOSITE_DIR_TO) {
        for (let i = 0; i < character_manifest.opposite_directions.sizes; i++) {
            character_manifest.opposite_directions.to[i] = undefined;
        }
        character_manifest.opposite_directions.to = undefined;
    }

    for (let i = 0; i < character_manifest.additional_states_size; i++) {
        character_manifest.additional_states[i].name = undefined;
        character_manifest_internal_destroy_actions(character_manifest.additional_states[i].actions);
    }
    character_manifest.additional_states = undefined;

    character_manifest = undefined;
}


function charactermanifest_internal_parse_extra(json_extra, ignore_name, entry) {
    const output_offsets = [0, 0];
    if (ignore_name) {
        entry.name = null;
    } else {
        entry.name = json_read_string(json_extra, "name", null);
        if (entry.name == null) throw new Error("missing extra animation name");
    }
    entry.anim = json_read_string(json_extra, "anim", null);
    entry.anim_hold = json_read_string(json_extra, "animHold", null);
    entry.stop_after_beats = json_read_number(json_extra, "stopAfterBeats", 0);
    entry.static_until_beat = json_read_boolean(json_extra, "staticUntilBeat", true);
    entry.anim_rollback = json_read_string(json_extra, "animRollback", null);
    entry.model_src = charactermanifest_internal_path_of(json_extra, "model", null);

    charactermanifest_internal_read_offsets(json_extra, output_offsets);
    entry.offset_x = output_offsets[0];
    entry.offset_y = output_offsets[1];
}

function charactermanifest_internal_parse_opposite_dir(json_array, size, array) {
    for (let i = 0; i < size; i++)
        array[i] = json_read_array_item_string(json_array, i, null);
}

function charactermanifest_internal_path_of(json, property_name, optional_src) {
    let str = json_read_string(json, property_name, null);
    if (!str || str.length < 1) {
        if (!optional_src) return null;
        throw new Error(`missing '${property_name}' in the '${optional_src}' file`);
    }
    let path = fs_get_full_path(str);
    str = undefined;
    return path;
}

function charactermanifest_internal_align(json, property_name, is_vertical) {
    let str = json_read_string(json, property_name, null);
    if (str == null) return is_vertical ? ALIGN_END : ALIGN_START;
    let align = vertexprops_parse_align2(str);

    switch (align) {
        case ALIGN_START:
        case ALIGN_CENTER:
        case ALIGN_END:
            break;
        default:
            console.warn("charactermanifest_internal_align() invalid align: " + str);
            align = ALIGN_CENTER;
            break;
    }

    str = undefined;
    return align;
}

function charactermanifest_internal_read_offsets(json, output_offsets) {
    output_offsets[0] = json_read_number(json, "offsetX", 0);
    output_offsets[1] = json_read_number(json, "offsetY", 0);
}

function character_manifest_internal_parse_actions(json_actions, actions, src) {
    const output_offsets = [0, 0];

    let sing_array = json_read_array(json_actions, "sing");
    let sing_array_size = json_read_array_length(sing_array);
    let miss_array = json_read_array(json_actions, "miss");
    let miss_array_size = json_read_array_length(miss_array);
    let extras_array = json_read_array(json_actions, "extras");
    let extras_array_size = json_read_array_length(extras_array);

    if (sing_array_size > 0) {
        actions.sing = new Array(sing_array_size);
        actions.sing_size = sing_array_size;
    }
    if (miss_array_size > 0) {
        actions.miss = new Array(miss_array_size);
        actions.miss_size = miss_array_size;
    }
    if (extras_array_size > 0) {
        actions.extras = new Array(extras_array_size);
        actions.extras_size = extras_array_size;
    }

    for (let i = 0; i < actions.sing_size; i++) {
        let item_json = json_read_array_item_object(sing_array, i);
        charactermanifest_internal_read_offsets(item_json, output_offsets);

        actions.sing[i] = {
            direction: json_read_string(item_json, "direction", null),
            anim: json_read_string(item_json, "anim", null),
            anim_hold: json_read_string(item_json, "animHold", null),
            anim_rollback: json_read_string(item_json, "animRollback", null),
            rollback: json_read_boolean(item_json, "rollback", false),
            follow_hold: json_read_boolean(item_json, "followHold", false),
            full_sustain: json_read_boolean(item_json, "fullSustain", false),
            model_src: charactermanifest_internal_path_of(item_json, "model", null),
            offset_x: output_offsets[0],
            offset_y: output_offsets[1]
        };

        if (!actions.sing[i].direction) {
            throw new Error(`'missing actions.sing[${i}].direction' in '${src}'`)
        }
    }
    for (let i = 0; i < actions.miss_size; i++) {
        let item_json = json_read_array_item_object(miss_array, i);
        charactermanifest_internal_read_offsets(item_json, output_offsets);

        actions.miss[i] = {
            direction: json_read_string(item_json, "direction", null),
            anim: json_read_string(item_json, "anim", null),
            stop_after_beats: json_read_number(item_json, "stopAfterBeats", 1),
            model_src: charactermanifest_internal_path_of(item_json, "model", null),
            offset_x: output_offsets[0],
            offset_y: output_offsets[1]
        };

        if (!actions.miss[i].direction) {
            throw new Error(`missing actions.miss[${i}].direction in '${src}'`);
        }
    }
    for (let i = 0; i < actions.extras_size; i++) {
        let item_json = json_read_array_item_object(extras_array, i);

        actions.extras[i] = {};
        charactermanifest_internal_parse_extra(item_json, 0, actions.extras[i]);
    }

    actions.has_idle = json_has_property_object(json_actions, "idle");
    if (actions.has_idle) {
        let json_extra = json_read_object(json_actions, "idle");
        charactermanifest_internal_parse_extra(json_extra, 1, actions.idle);
    }

    actions.has_hey = json_has_property_object(json_actions, "hey");
    if (actions.has_hey) {
        let json_extra = json_read_object(json_actions, "hey");
        charactermanifest_internal_parse_extra(json_extra, 1, actions.hey);
    }

}

function character_manifest_internal_destroy_actions(actions) {
    for (let i = 0; i < actions.sing_size; i++) {
        actions.sing[i].direction = undefined;
        actions.sing[i].anim = undefined;
        actions.sing[i].anim_hold = undefined;
        actions.sing[i].anim_rollback = undefined;
        actions.sing[i].model_src = undefined;
    }

    for (let i = 0; i < actions.miss_size; i++) {
        actions.miss[i].direction = undefined;
        actions.miss[i].anim = undefined;
        actions.miss[i].model_src = undefined;
    }

    for (let i = 0; i < actions.extra_size; i++) {
        actions.extras[i].name = undefined;
        actions.extras[i].anim = undefined;
        actions.extras[i].anim_hold = undefined;
        actions.extras[i].anim_rollback = undefined;
        actions.extras[i].model_src = undefined;
    }

    if (actions.has_hey) {
        actions.hey.name = undefined;
        actions.hey.anim = undefined;
        actions.hey.anim_hold = undefined;
        actions.hey.anim_rollback = undefined;
        actions.hey.model_src = undefined;
    }

    if (actions.has_idle) {
        actions.idle.name = undefined;
        actions.idle.anim = undefined;
        actions.idle.anim_hold = undefined;
        actions.idle.anim_rollback = undefined;
        actions.idle.model_src = undefined;
    }
}

function character_manifest_internal_read_additional_states(json_array, src) {
    let additional_states = arraylist_init();

    let size = json_read_array_length(json_array);
    for (let i = 0; i < size; i++) {
        let item = json_read_array_item_object(json_array, i);

        let state = {
            name: json_read_string(item, "name", null),
            model: charactermanifest_internal_path_of(item, "model", src),
            actions: {
                extras: null,
                extras_size: 0,
                has_hey: false,
                has_idle: false,
                hey: {},
                idle: {},
                miss: null,
                miss_size: 0,
                sing: null,
                sing_alt: null,
                sing_size: 0
            }
        };

        let json_actions = json_read_object(item, "actions");
        if (!json_is_property_null(item, "actions")) {
            character_manifest_internal_parse_actions(json_actions, state.actions, src);
        }

        arraylist_add(additional_states, state);
    }

    return additional_states;
}

