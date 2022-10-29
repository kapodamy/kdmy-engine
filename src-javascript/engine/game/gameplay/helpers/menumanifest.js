"use strict";

async function menumanifest_init(src) {

    let json = await json_load_from(src);
    if (!json) throw new Error("menumanifest_init() misssing or invalid file: " + src);

    fs_folder_stack_push();
    fs_set_working_folder(src, 1);

    let align_value = json_read_string(json, "itemsAlign", null);
    let align = vertexprops_parse_align2(align_value);
    align_value = undefined;
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
            suffix_selected: json_read_string(json, "suffixSelected", "selected"),
            suffix_choosen: json_read_string(json, "suffixChoosen", "choosen"),
            suffix_discarded: json_read_string(json, "suffixDiscarded", "discarded"),
            suffix_idle: json_read_string(json, "suffixIdle", "idle"),
            suffix_rollback: json_read_string(json, "suffixRollback", "rollback"),
            suffix_in: json_read_string(json, "suffixIn", "in"),
            suffix_out: json_read_string(json, "suffixOut", "out"),

            atlas: menumanifest_internal_parse_path(json, "atlas"),
            animlist: menumanifest_internal_parse_path(json, "animlist"),

            anim_selected: json_read_string(json, "animSelected", null),
            anim_choosen: json_read_string(json, "animChoosen", null),
            anim_discarded: json_read_string(json, "animDiscarded", null),
            anim_idle: json_read_string(json, "animIdle", null),
            anim_rollback: json_read_string(json, "animRollback", null),
            anim_in: json_read_string(json, "animIn", null),
            anim_out: json_read_string(json, "animOut", null),

            anim_transition_in_delay: json_read_number(json, "transitionInDelay", 0),
            anim_transition_out_delay: json_read_number(json, "transitionOutDelay", 0),

            font: json_read_string(json, "fontPath", null),
            font_glyph_suffix: json_read_string(json, "fontSuffix", null),
            font_color_by_difference: json_read_boolean(json, "fontColorByDifference", 0),
            font_size: json_read_number(json, "fontSize", 0),
            font_color: json_read_hex(json, "fontColor", 0xFFFFFF),
            font_border_color: json_read_hex(json, "fontBorderColor", 0x0),
            font_border_size: json_read_number(json, "fontBorderSize", 0),

            is_sparse: json_read_boolean(json, "isSparse", 0),
            is_vertical: json_read_boolean(json, "isVertical", 1),
            is_per_page: json_read_boolean(json, "isPerPage", 0),

            items_align: align,
            items_gap: json_read_number(json, "itemsGap", 0),
            items_dimmen: json_read_number(json, "itemsDimmen", 0),
            static_index: json_read_number(json, "staticIndex", 0),
            texture_scale: json_read_number(json, "textureScale", NaN),
            enable_horizontal_text_correction: json_read_boolean(json, "enableHorizontalTextCorrection", 0)
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
            hidden: json_read_boolean(json_item, "hidden", 0),

            placement: {
                x: json_read_number(json_placement, "x", 0),
                y: json_read_number(json_placement, "y", 0),
                dimmen: json_read_number(json_placement, "dimmen", 0),
                gap: json_read_number(json_placement, "gap", 0)
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
    for (let i = 0; i < menumanifest.items_size; i++) {
        menumanifest.items[i].name = undefined;
        menumanifest.items[i].text = undefined;
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


function menumanifest_internal_parse_path(json, property_name) {
    let str = json_read_string(json, property_name, null);
    if (str == null) return null;
    if (str.length < 1) {
        str = undefined;
        return null;
    }

    let path = fs_get_full_path(str);
    str = undefined;

    return path;
}

