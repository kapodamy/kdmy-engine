#include "game/gameplay/helpers/menumanifest.h"

#include "externals/luascript.h"
#include "fs.h"
#include "jsonparser.h"
#include "logger.h"
#include "malloc_utils.h"
#include "math2d.h"
#include "stringutils.h"
#include "vertexprops.h"


static char* menumanifest_internal_parse_path(JSONToken json, const char* property_name);


MenuManifest menumanifest_init(const char* src) {

    JSONToken json = json_load_from(src);
    if (!json) {
        logger_error("menumanifest_init() misssing or invalid file: %s", src);
        assert(json);
    }

    JSONToken json_parameters = json_read_object(json, "parameters");
    if (!json_parameters) {
        logger_error("menumanifest_init() misssing parameters in json: %s", src);
        assert(json_parameters);
    }

    fs_folder_stack_push();
    fs_set_working_folder(src, true);

    const char* align_value = json_read_string(json, "itemsAlign", NULL);
    Align align = vertexprops_parse_align2(align_value);
    switch (align) {
        case ALIGN_INVALID:
            align = ALIGN_CENTER;
            break;
        case ALIGN_START:
        case ALIGN_CENTER:
        case ALIGN_END:
            break;
        default:
            logger_warn("menumanifest_init() unknown 'itemsAlign' value in: %s", src);
            align = ALIGN_CENTER;
            break;
    }

    JSONToken array_items = json_read_array(json, "items");
    int32_t array_items_length = json_read_array_length(array_items);

    if (array_items_length < 1) {
        logger_error("menumanifest_init() misssing or invalid 'items' property in: %s", src);
        assert(array_items_length >= 0);
    }

    MenuManifest menumanifest = malloc_chk(sizeof(struct MenuManifest_s));
    malloc_assert(menumanifest, MenuManifest);

    *menumanifest = (struct MenuManifest_s){
        .parameters = (MenuManifestParameters){
            .suffix_selected = json_read_string2(json_parameters, "suffixSelected", "selected"),
            .suffix_choosen = json_read_string2(json_parameters, "suffixChoosen", "choosen"),
            .suffix_discarded = json_read_string2(json_parameters, "suffixDiscarded", "discarded"),
            .suffix_idle = json_read_string2(json_parameters, "suffixIdle", "idle"),
            .suffix_rollback = json_read_string2(json_parameters, "suffixRollback", "rollback"),
            .suffix_in = json_read_string2(json_parameters, "suffixIn", "in"),
            .suffix_out = json_read_string2(json_parameters, "suffixOut", "out"),

            .atlas = menumanifest_internal_parse_path(json_parameters, "atlas"),
            .animlist = menumanifest_internal_parse_path(json_parameters, "animlist"),

            .anim_selected = json_read_string2(json_parameters, "animSelected", NULL),
            .anim_choosen = json_read_string2(json_parameters, "animChoosen", NULL),
            .anim_discarded = json_read_string2(json_parameters, "animDiscarded", NULL),
            .anim_idle = json_read_string2(json_parameters, "animIdle", NULL),
            .anim_rollback = json_read_string2(json_parameters, "animRollback", NULL),
            .anim_in = json_read_string2(json_parameters, "animIn", NULL),
            .anim_out = json_read_string2(json_parameters, "animOut", NULL),

            .anim_transition_in_delay = (float)json_read_number_double(json_parameters, "transitionInDelay", 0.0),
            .anim_transition_out_delay = (float)json_read_number_double(json_parameters, "transitionOutDelay", 0.0),

            .font = json_read_string2(json_parameters, "fontPath", NULL),
            .font_glyph_suffix = json_read_string2(json_parameters, "fontSuffix", NULL),
            .font_color_by_addition = json_read_boolean(json_parameters, "fontColorByAddition", false),
            .font_size = (float)json_read_number_double(json_parameters, "fontSize", 0.0),
            .font_color = json_read_hex(json_parameters, "fontColor", 0xFFFFFF),
            .font_border_color = json_read_hex(json_parameters, "fontBorderColor", 0x0),
            .font_border_size = (float)json_read_number_double(json_parameters, "fontBorderSize", 0.0),

            .is_sparse = json_read_boolean(json_parameters, "isSparse", false),
            .is_vertical = json_read_boolean(json_parameters, "isVertical", true),
            .is_per_page = json_read_boolean(json_parameters, "isPerPage", false),

            .items_align = align,
            .items_gap = (float)json_read_number_double(json_parameters, "itemsGap", 0.0),
            .items_dimmen = (float)json_read_number_double(json_parameters, "itemsDimmen", 0.0),
            .static_index = (int32_t)json_read_number_long(json_parameters, "staticIndex", 0),
            .texture_scale = (float)json_read_number_double(json_parameters, "textureScale", DOUBLE_NaN),
            .enable_horizontal_text_correction = json_read_boolean(json_parameters, "enableHorizontalTextCorrection", false)
        },

        .items = malloc_for_array(MenuManifestItem, array_items_length),
        .items_size = array_items_length
    };

    for (int32_t i = 0; i < array_items_length; i++) {
        JSONToken json_item = json_read_array_item_object(array_items, i);
        JSONToken json_placement = json_read_object(json_item, "placement");

        menumanifest->items[i] = (MenuManifestItem){
            .name = json_read_string2(json_item, "name", NULL),
            .text = json_read_string2(json_item, "text", NULL),
            .model = json_read_string2(json_item, "model", NULL),
            .hidden = json_read_boolean(json_item, "hidden", false),
            .description = json_read_string2(json_item, "description", NULL),
            .texture_scale = (float)json_read_number_double(json_item, "textureScale", 0.0),

            .placement = (MenuManifestPlacement){
                .x = (float)json_read_number_double(json_placement, "x", 0.0),
                .y = (float)json_read_number_double(json_placement, "y", 0.0),
                .dimmen = (float)json_read_number_double(json_placement, "dimmen", 0.0),
                .gap = (float)json_read_number_double(json_placement, "gap", 0.0)
            },

            .anim_selected = json_read_string2(json_item, "animSelected", NULL),
            .anim_choosen = json_read_string2(json_item, "animChoosen", NULL),
            .anim_discarded = json_read_string2(json_item, "animDiscarded", NULL),
            .anim_idle = json_read_string2(json_item, "animIdle", NULL),
            .anim_rollback = json_read_string2(json_item, "animRollback", NULL),
            .anim_in = json_read_string2(json_item, "animIn", NULL),
            .anim_out = json_read_string2(json_item, "animOut", NULL),
        };
    }

    fs_folder_stack_pop();
    return menumanifest;
}

void menumanifest_destroy(MenuManifest* menumanifest_ptr) {
    if (!menumanifest_ptr || !*menumanifest_ptr) return;

    MenuManifest menumanifest = *menumanifest_ptr;

    luascript_drop_shared(menumanifest);

    for (int32_t i = 0; i < menumanifest->items_size; i++) {
        free_chk(menumanifest->items[i].name);
        free_chk(menumanifest->items[i].text);
        free_chk(menumanifest->items[i].description);
        free_chk(menumanifest->items[i].anim_selected);
        free_chk(menumanifest->items[i].anim_choosen);
        free_chk(menumanifest->items[i].anim_discarded);
        free_chk(menumanifest->items[i].anim_idle);
        free_chk(menumanifest->items[i].anim_rollback);
        free_chk(menumanifest->items[i].anim_in);
        free_chk(menumanifest->items[i].anim_out);
    }

    free_chk(menumanifest->parameters.suffix_selected);
    free_chk(menumanifest->parameters.suffix_choosen);
    free_chk(menumanifest->parameters.suffix_discarded);
    free_chk(menumanifest->parameters.suffix_idle);
    free_chk(menumanifest->parameters.suffix_rollback);
    free_chk(menumanifest->parameters.suffix_in);
    free_chk(menumanifest->parameters.suffix_out);

    free_chk(menumanifest->parameters.atlas);
    free_chk(menumanifest->parameters.animlist);

    free_chk(menumanifest->parameters.anim_selected);
    free_chk(menumanifest->parameters.anim_choosen);
    free_chk(menumanifest->parameters.anim_discarded);
    free_chk(menumanifest->parameters.anim_idle);
    free_chk(menumanifest->parameters.anim_rollback);
    free_chk(menumanifest->parameters.anim_in);
    free_chk(menumanifest->parameters.anim_out);

    free_chk(menumanifest->parameters.font);
    free_chk(menumanifest->parameters.font_glyph_suffix);

    free_chk(menumanifest->items);

    free_chk(menumanifest);
    *menumanifest_ptr = NULL;
}

int32_t menumanifest_get_option_index(MenuManifest menumanifest, const char* option_name) {
    for (int32_t i = 0; i < menumanifest->items_size; i++) {
        if (string_equals(menumanifest->items[i].name, option_name)) {
            return i;
        }
    }
    return -1;
}


static char* menumanifest_internal_parse_path(JSONToken json, const char* property_name) {
    const char* str = json_read_string(json, property_name, NULL);
    if (string_is_empty(str)) return NULL;

    char* path = fs_get_full_path(str);
    return path;
}
