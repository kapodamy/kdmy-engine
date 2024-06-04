"use strict";

async function menumanifest_init(src) {

    let json = await json_load_from(src);
    if (!json) throw new Error("menumanifest_init() misssing or invalid file: " + src);

    let json_parameters = json_read_object(json, "parameters");
    if (!json_parameters) throw new Error("menumanifest_init() misssing parameters in json: " + src);

    fs_folder_stack_push();
    fs_set_working_folder(src, true);

    let align_value = json_read_string(json, "itemsAlign", null);
    let align = vertexprops_parse_align2(align_value);
    switch (align) {
        case -1:
            align = ALIGN_CENTER;
            break;
        case ALIGN_START:
        case ALIGN_CENTER:
        case ALIGN_END:
            break;
        default:
            console.warn("menumanifest_init() unknown 'itemsAlign' value in: " + src);
            align = ALIGN_CENTER;
            break;
    }

    let array_items = json_read_array(json, "items");
    let array_items_length = json_read_array_length(array_items);

    if (array_items_length < 1) {
        throw new Error("menumanifest_init() misssing or invalid 'items' property in: " + src);
    }

    let menumanifest = {
        parameters: {
            suffix_selected: json_read_string(json_parameters, "suffixSelected", "selected"),
            suffix_choosen: json_read_string(json_parameters, "suffixChoosen", "choosen"),
            suffix_discarded: json_read_string(json_parameters, "suffixDiscarded", "discarded"),
            suffix_idle: json_read_string(json_parameters, "suffixIdle", "idle"),
            suffix_rollback: json_read_string(json_parameters, "suffixRollback", "rollback"),
            suffix_in: json_read_string(json_parameters, "suffixIn", "in"),
            suffix_out: json_read_string(json_parameters, "suffixOut", "out"),

            atlas: menumanifest_internal_parse_path(json_parameters, "atlas"),
            animlist: menumanifest_internal_parse_path(json_parameters, "animlist"),

            anim_selected: json_read_string(json_parameters, "animSelected", null),
            anim_choosen: json_read_string(json_parameters, "animChoosen", null),
            anim_discarded: json_read_string(json_parameters, "animDiscarded", null),
            anim_idle: json_read_string(json_parameters, "animIdle", null),
            anim_rollback: json_read_string(json_parameters, "animRollback", null),
            anim_in: json_read_string(json_parameters, "animIn", null),
            anim_out: json_read_string(json_parameters, "animOut", null),

            anim_transition_in_delay: json_read_number(json_parameters, "transitionInDelay", 0.0),
            anim_transition_out_delay: json_read_number(json_parameters, "transitionOutDelay", 0.0),

            font: json_read_string(json_parameters, "fontPath", null),
            font_glyph_suffix: json_read_string(json_parameters, "fontSuffix", null),
            font_color_by_addition: json_read_boolean(json_parameters, "fontColorByAddition", false),
            font_size: json_read_number(json_parameters, "fontSize", 0.0),
            font_color: json_read_hex(json_parameters, "fontColor", 0xFFFFFF),
            font_border_color: json_read_hex(json_parameters, "fontBorderColor", 0x0),
            font_border_size: json_read_number(json_parameters, "fontBorderSize", 0.0),

            is_sparse: json_read_boolean(json_parameters, "isSparse", false),
            is_vertical: json_read_boolean(json_parameters, "isVertical", true),
            is_per_page: json_read_boolean(json_parameters, "isPerPage", false),

            items_align: align,
            items_gap: json_read_number(json_parameters, "itemsGap", 0.0),
            items_dimmen: json_read_number(json_parameters, "itemsDimmen", 0.0),
            static_index: json_read_number(json_parameters, "staticIndex", 0.0),
            texture_scale: json_read_number(json_parameters, "textureScale", NaN),
            enable_horizontal_text_correction: json_read_boolean(json_parameters, "enableHorizontalTextCorrection", false)
        },

        items: new Array(array_items_length),
        items_size: array_items_length
    };

    for (let i = 0; i < array_items_length; i++) {
        const json_item = json_read_array_item_object(array_items, i);
        const json_placement = json_read_object(json_item, "placement");

        menumanifest.items[i] = {
            name: json_read_string(json_item, "name", null),
            text: json_read_string(json_item, "text", null),
            model: json_read_string(json_item, "model", null),
            hidden: json_read_boolean(json_item, "hidden", false),
            description: json_read_string(json_item, "description", null),
            texture_scale: json_read_number(json_item, "textureScale", 0.0),

            placement: {
                x: json_read_number(json_placement, "x", 0.0),
                y: json_read_number(json_placement, "y", 0.0),
                dimmen: json_read_number(json_placement, "dimmen", 0.0),
                gap: json_read_number(json_placement, "gap", 0.0)
            },

            anim_selected: json_read_string(json_item, "animSelected", null),
            anim_choosen: json_read_string(json_item, "animChoosen", null),
            anim_discarded: json_read_string(json_item, "animDiscarded", null),
            anim_idle: json_read_string(json_item, "animIdle", null),
            anim_rollback: json_read_string(json_item, "animRollback", null),
            anim_in: json_read_string(json_item, "animIn", null),
            anim_out: json_read_string(json_item, "animOut", null),
        };
    }

    fs_folder_stack_pop();
    return menumanifest;
}

function menumanifest_destroy(menumanifest) {
    luascript_drop_shared(menumanifest);

    for (let i = 0; i < menumanifest.items_size; i++) {
        menumanifest.items[i].name = undefined;
        menumanifest.items[i].text = undefined;
        menumanifest.items[i].description = undefined;
        menumanifest.items[i].anim_selected = undefined;
        menumanifest.items[i].anim_choosen = undefined;
        menumanifest.items[i].anim_discarded = undefined;
        menumanifest.items[i].anim_idle = undefined;
        menumanifest.items[i].anim_rollback = undefined;
        menumanifest.items[i].anim_in = undefined;
        menumanifest.items[i].anim_out = undefined;
    }

    menumanifest.parameters.suffix_selected = undefined;
    menumanifest.parameters.suffix_choosen = undefined;
    menumanifest.parameters.suffix_discarded = undefined;
    menumanifest.parameters.suffix_idle = undefined;
    menumanifest.parameters.suffix_rollback = undefined;
    menumanifest.parameters.suffix_in = undefined;
    menumanifest.parameters.suffix_out = undefined;

    menumanifest.parameters.atlas = undefined;
    menumanifest.parameters.animlist = undefined;

    menumanifest.parameters.anim_selected = undefined;
    menumanifest.parameters.anim_choosen = undefined;
    menumanifest.parameters.anim_discarded = undefined;
    menumanifest.parameters.anim_idle = undefined;
    menumanifest.parameters.anim_rollback = undefined;
    menumanifest.parameters.anim_in = undefined;
    menumanifest.parameters.anim_out = undefined;

    menumanifest.parameters.font = undefined;
    menumanifest.parameters.font_glyph_suffix = undefined;

    menumanifest.items = undefined;
    menumanifest = undefined;
}

function menumanifest_get_option_index(menumanifest, option_name) {
    for (let i = 0; i < menumanifest.items_size; i++) {
        if (menumanifest.items[i].name == option_name) {
            return i;
        }
    }
    return -1;
}


function menumanifest_internal_parse_path(json, property_name) {
    let str = json_read_string(json, property_name, null);
    if (!str) return null;

    let path = fs_get_full_path(str);

    return path;
}

