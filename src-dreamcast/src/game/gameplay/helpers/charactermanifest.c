#include "game/gameplay/helpers/charactermanifest.h"

#include "arraylist.h"
#include "fs.h"
#include "jsonparser.h"
#include "logger.h"
#include "malloc_utils.h"
#include "stringutils.h"
#include "vertexprops.h"


static const char* CHARACTERMANIFEST_DEFAULT_OPPOSITE_DIR_FROM[] = {"left", "right"};
static const char* CHARACTERMANIFEST_DEFAULT_OPPOSITE_DIR_TO[] = {"right", "left"};


static void charactermanifest_internal_parse_extra(JSONToken json_extra, bool ignore_name, float64 default_beat_stop, CharacterManifestExtra* entry);
static void charactermanifest_internal_parse_opposite_dir(JSONToken json_array, int32_t size, char** array);
static char* charactermanifest_internal_path_of(JSONToken json, const char* property_name, const char* optional_src);
static Align charactermanifest_internal_align(JSONToken json, const char* property_name, bool is_vertical);
static void charactermanifest_internal_read_offsets(JSONToken json, float64* offset_x, float64* offset_y);
static void character_manifest_internal_parse_actions(JSONToken json_actions, CharacterManifestActions* actions, const char* src);
static void character_manifest_internal_destroy_actions(CharacterManifestActions* actions);
static ArrayList character_manifest_internal_read_additional_states(JSONToken json_array, const char* default_model, const char* src);


CharacterManifest charactermanifest_init(const char* src, bool gameplay_required_models_only) {
    JSONToken json = json_load_from(src);
    if (!json) {
        logger_error("missing or invalid file: %s", src);
        assert(json);
    }

    fs_folder_stack_push();
    fs_set_working_folder(src, true);

    char* model_character = NULL;
    char* model_health_icons = NULL;
    char* week_selector_model = NULL;
    bool has_healthbar_color = json_has_property_hex(json, "healthbarColor");
    uint32_t healthbar_color = json_read_hex(json, "healthbarColor", 0x00FFFF);

    if (gameplay_required_models_only) {
        model_character = charactermanifest_internal_path_of(json, "model", src);
        if (json_has_property_string(json, "healthIconsModel") && !json_is_property_null(json, "healthIconsModel")) {
            model_health_icons = charactermanifest_internal_path_of(json, "healthIconsModel", src);
        }
    } else {
        week_selector_model = charactermanifest_internal_path_of(json, "weekSelectorModel", NULL);
    }

    CharacterManifest character_manifest = malloc_chk(sizeof(struct CharacterManifest_s));
    malloc_assert(character_manifest, CharacterManifest);

    *character_manifest = (struct CharacterManifest_s){
        .model_character = model_character,
        .model_health_icons = model_health_icons,
        .has_healthbar_color = has_healthbar_color,
        .healthbar_color = healthbar_color,

        .sing_suffix = json_read_string2(json, "singSuffix", NULL),
        .sing_alternate_suffix = json_read_string2(json, "singAlternateSuffix", NULL),
        .sing_prefix = json_read_string2(json, "singPrefix", NULL),
        .sing_alternate_prefix = json_read_string2(json, "singAlternatePrefix", NULL),
        .allow_alternate_idle = json_read_boolean(json, "allowAlternateIdle", false),
        .continuous_idle = json_read_boolean(json, "continuousIdle", false),
        .actions_apply_chart_speed = json_read_boolean(json, "actionsApplyChartSpeed", false),

        .offset_x = (float)json_read_number_double(json, "offsetX", 0.0),
        .offset_y = (float)json_read_number_double(json, "offsetY", 0.0),
        .left_facing = json_read_boolean(json, "leftFacing", false),

        .week_selector_model = week_selector_model,
        .week_selector_idle_anim_name = json_read_string2(json, "weekSelectorIdleAnimName", NULL),
        .week_selector_choosen_anim_name = json_read_string2(json, "weekSelectorChoosenAnimName", NULL),
        .week_selector_enable_beat = json_read_boolean(json, "weekSelectorEnableBeat", true),
        .week_selector_left_facing = false,

        .actions = (CharacterManifestActions){
            .sing = NULL, .sing_size = 0, .miss = NULL, .miss_size = 0, .extras = NULL, .extras_size = 0, .idle = (CharacterManifestExtra){}, .hey = (CharacterManifestExtra){}, .has_idle = false, .has_hey = false
        },

        .align_vertical = ALIGN_CENTER,
        .align_horizontal = ALIGN_CENTER,

        .has_reference_size = false,
        .reference_width = -1.0f,
        .reference_height = -1.0f,

        .opposite_directions = (CharacterManifestOpposite){
            .sizes = 2,
            .from = (char**)CHARACTERMANIFEST_DEFAULT_OPPOSITE_DIR_FROM,
            .to = (char**)CHARACTERMANIFEST_DEFAULT_OPPOSITE_DIR_TO,
        },

        .additional_states_size = 0,
        .additional_states = NULL
    };

    character_manifest->week_selector_left_facing = json_read_boolean(
        json, "weekSelectorLeftFacing", character_manifest->left_facing
    );

    JSONToken json_actions = json_read_object(json, "actions");
    character_manifest_internal_parse_actions(json_actions, &character_manifest->actions, src);

    character_manifest->align_vertical = charactermanifest_internal_align(json, "alignVertical", true);
    character_manifest->align_horizontal = charactermanifest_internal_align(json, "alignHorizontal", false);

    character_manifest->has_reference_size = json_has_property(json, "referenceSize");
    if (character_manifest->has_reference_size) {
        if (!json_has_property_object(json, "referenceSize")) {
            logger_error("invalid 'referenceSize' in %s", src);
            assert(json_has_property_object(json, "referenceSize"));
        }
        character_manifest->reference_width = (float)json_read_number_double(json, "width", -1.0);
        character_manifest->reference_height = (float)json_read_number_double(json, "height", -1.0);
    }

    JSONToken opposite_directions = json_read_object(json, "oppositeDirections");
    if (opposite_directions) {

        JSONToken from_array = json_read_array(opposite_directions, "from");
        int32_t from_array_size = json_read_array_length(from_array);
        JSONToken to_array = json_read_array(opposite_directions, "to");
        int32_t to_array_size = json_read_array_length(to_array);

        if (from_array_size != to_array_size) {
            logger_error(
                "invalid 'oppositeDirections.from', and/or 'oppositeDirectionsto' in %s",
                src
            );
            assert(from_array_size == to_array_size);
        }

        if (to_array_size == 0) {
            character_manifest->opposite_directions.sizes = 0;
            character_manifest->opposite_directions.from = NULL;
            character_manifest->opposite_directions.to = NULL;
        } else if (to_array_size > 0) {
            character_manifest->opposite_directions.sizes = from_array_size;
            character_manifest->opposite_directions.from = malloc_for_array(char*, from_array_size);
            character_manifest->opposite_directions.to = malloc_for_array(char*, to_array_size);

            charactermanifest_internal_parse_opposite_dir(
                from_array, from_array_size, character_manifest->opposite_directions.from
            );
            charactermanifest_internal_parse_opposite_dir(
                to_array, to_array_size, character_manifest->opposite_directions.to
            );
        }
    } else {
    }

    JSONToken json_additional_states = json_read_array(json, "additionalStates");
    ArrayList additional_states = character_manifest_internal_read_additional_states(
        json_additional_states, character_manifest->model_character, src
    );
    arraylist_destroy2(&additional_states, &character_manifest->additional_states_size, (void**)&character_manifest->additional_states);

    json_destroy(&json);
    fs_folder_stack_pop();

    return character_manifest;
}

void charactermanifest_destroy(CharacterManifest* character_manifest_ptr) {
    if (!character_manifest_ptr || !*character_manifest_ptr) return;

    CharacterManifest character_manifest = *character_manifest_ptr;

    free_chk(character_manifest->model_character);
    free_chk(character_manifest->model_health_icons);
    free_chk(character_manifest->sing_suffix);
    free_chk(character_manifest->sing_alternate_suffix);
    free_chk(character_manifest->sing_alternate_prefix);
    free_chk(character_manifest->week_selector_model);
    free_chk(character_manifest->week_selector_idle_anim_name);
    free_chk(character_manifest->week_selector_choosen_anim_name);


    character_manifest_internal_destroy_actions(&character_manifest->actions);

    if (character_manifest->opposite_directions.from != (char**)CHARACTERMANIFEST_DEFAULT_OPPOSITE_DIR_FROM) {
        for (int32_t i = 0; i < character_manifest->opposite_directions.sizes; i++) {
            free_chk(character_manifest->opposite_directions.from[i]);
        }
        free_chk(character_manifest->opposite_directions.from);
    }

    if (character_manifest->opposite_directions.to != (char**)CHARACTERMANIFEST_DEFAULT_OPPOSITE_DIR_TO) {
        for (int32_t i = 0; i < character_manifest->opposite_directions.sizes; i++) {
            free_chk(character_manifest->opposite_directions.to[i]);
        }
        free_chk(character_manifest->opposite_directions.to);
    }

    for (int32_t i = 0; i < character_manifest->additional_states_size; i++) {
        free_chk(character_manifest->additional_states[i].name);
        character_manifest_internal_destroy_actions(&character_manifest->additional_states[i].actions);
    }
    free_chk(character_manifest->additional_states);

    free_chk(character_manifest);
    *character_manifest_ptr = NULL;
}


static void charactermanifest_internal_parse_extra(JSONToken json_extra, bool ignore_name, float64 default_beat_stop, CharacterManifestExtra* entry) {
    float64 offset_x = 0.0, offset_y = 0.0;
    if (ignore_name) {
        entry->name = NULL;
    } else {
        entry->name = json_read_string2(json_extra, "name", NULL);
        if (entry->name == NULL) {
            logger_error("missing extra animation name");
            assert(entry->name != NULL);
        }
    }
    entry->anim = json_read_string2(json_extra, "anim", NULL);
    entry->anim_hold = json_read_string2(json_extra, "animHold", NULL);
    entry->stop_after_beats = (float)json_read_number_double(json_extra, "stopAfterBeats", default_beat_stop);
    entry->anim_rollback = json_read_string2(json_extra, "animRollback", NULL);
    entry->model_src = charactermanifest_internal_path_of(json_extra, "model", NULL);

    charactermanifest_internal_read_offsets(json_extra, &offset_x, &offset_y);
    entry->offset_x = (float)offset_x;
    entry->offset_y = (float)offset_y;
}

static void charactermanifest_internal_parse_opposite_dir(JSONToken json_array, int32_t size, char** array) {
    for (int32_t i = 0; i < size; i++)
        array[i] = json_read_array_item_string2(json_array, i, NULL);
}

static char* charactermanifest_internal_path_of(JSONToken json, const char* property_name, const char* optional_src) {
    const char* str = json_read_string(json, property_name, NULL);
    if (string_is_empty(str)) {
        if (!optional_src) return NULL;
        logger_error("missing '%s' in the '%s' file", property_name, optional_src);
        assert(string_is_not_empty(str));
    }
    char* path = fs_get_full_path(str);
    return path;
}

static Align charactermanifest_internal_align(JSONToken json, const char* property_name, bool is_vertical) {
    const char* str = json_read_string(json, property_name, NULL);
    if (str == NULL) return is_vertical ? ALIGN_END : ALIGN_START;
    Align align = vertexprops_parse_align2(str);

    switch (align) {
        case ALIGN_START:
        case ALIGN_CENTER:
        case ALIGN_END:
            break;
        default:
            logger_warn("charactermanifest_internal_align() invalid align: %s", str);
            align = ALIGN_CENTER;
            break;
    }

    return align;
}

static void charactermanifest_internal_read_offsets(JSONToken json, float64* offset_x, float64* offset_y) {
    *offset_x = json_read_number_double(json, "offsetX", 0.0);
    *offset_y = json_read_number_double(json, "offsetY", 0.0);
}

static void character_manifest_internal_parse_actions(JSONToken json_actions, CharacterManifestActions* actions, const char* src) {
    float64 offset_x = 0.0, offset_y = 0.0;

    JSONToken sing_array = json_read_array(json_actions, "sing");
    int32_t sing_array_size = json_read_array_length(sing_array);
    JSONToken miss_array = json_read_array(json_actions, "miss");
    int32_t miss_array_size = json_read_array_length(miss_array);
    JSONToken extras_array = json_read_array(json_actions, "extras");
    int32_t extras_array_size = json_read_array_length(extras_array);

    if (sing_array_size > 0) {
        actions->sing = malloc_for_array(CharacterManifestSing, sing_array_size);
        actions->sing_size = sing_array_size;
    }
    if (miss_array_size > 0) {
        actions->miss = malloc_for_array(CharacterManifestMiss, miss_array_size);
        actions->miss_size = miss_array_size;
    }
    if (extras_array_size > 0) {
        actions->extras = malloc_for_array(CharacterManifestExtra, extras_array_size);
        actions->extras_size = extras_array_size;
    }

    for (int32_t i = 0; i < actions->sing_size; i++) {
        JSONToken item_json = json_read_array_item_object(sing_array, i);
        charactermanifest_internal_read_offsets(item_json, &offset_x, &offset_y);

        actions->sing[i] = (CharacterManifestSing){
            .direction = json_read_string2(item_json, "direction", NULL),
            .anim = json_read_string2(item_json, "anim", NULL),
            .anim_hold = json_read_string2(item_json, "animHold", NULL),
            .anim_rollback = json_read_string2(item_json, "animRollback", NULL),
            .rollback = json_read_boolean(item_json, "rollback", false),
            .follow_hold = json_read_boolean(item_json, "followHold", false),
            .full_sustain = json_read_boolean(item_json, "fullSustain", false),
            .stop_after_beats = (float)json_read_number_double(item_json, "stopAfterBeats", 1.0),
            .model_src = charactermanifest_internal_path_of(item_json, "model", NULL),
            .offset_x = (float)offset_x,
            .offset_y = (float)offset_y
        };

        if (!actions->sing[i].direction) {
            logger_error("missing 'actions->sing[" FMT_I4 "].direction' in '%s'", i, src);
            assert(actions->sing[i].direction);
        }
    }
    for (int32_t i = 0; i < actions->miss_size; i++) {
        JSONToken item_json = json_read_array_item_object(miss_array, i);
        charactermanifest_internal_read_offsets(item_json, &offset_x, &offset_y);

        actions->miss[i] = (CharacterManifestMiss){
            .direction = json_read_string2(item_json, "direction", NULL),
            .anim = json_read_string2(item_json, "anim", NULL),
            .stop_after_beats = (float)json_read_number_double(item_json, "stopAfterBeats", 1.0),
            .model_src = charactermanifest_internal_path_of(item_json, "model", NULL),
            .offset_x = (float)offset_x,
            .offset_y = (float)offset_y
        };

        if (!actions->miss[i].direction) {
            logger_error("missing 'actions->miss[" FMT_I4 "].direction' in '%s'", i, src);
        }
    }
    for (int32_t i = 0; i < actions->extras_size; i++) {
        JSONToken item_json = json_read_array_item_object(extras_array, i);
        charactermanifest_internal_parse_extra(item_json, false, -1.0, &actions->extras[i]);
    }

    actions->has_idle = json_has_property_object(json_actions, "idle");
    if (actions->has_idle) {
        JSONToken json_extra = json_read_object(json_actions, "idle");
        charactermanifest_internal_parse_extra(json_extra, true, -1.0, &actions->idle);
    }

    actions->has_hey = json_has_property_object(json_actions, "hey");
    if (actions->has_hey) {
        JSONToken json_extra = json_read_object(json_actions, "hey");
        charactermanifest_internal_parse_extra(json_extra, true, -1.0, &actions->hey);
    }
}

static void character_manifest_internal_destroy_actions(CharacterManifestActions* actions) {
    for (int32_t i = 0; i < actions->sing_size; i++) {
        free_chk(actions->sing[i].direction);
        free_chk(actions->sing[i].anim);
        free_chk(actions->sing[i].anim_hold);
        free_chk(actions->sing[i].anim_rollback);
        free_chk(actions->sing[i].model_src);
    }
    free_chk(actions->sing);

    for (int32_t i = 0; i < actions->miss_size; i++) {
        free_chk(actions->miss[i].direction);
        free_chk(actions->miss[i].anim);
        free_chk(actions->miss[i].model_src);
    }
    free_chk(actions->miss);

    for (int32_t i = 0; i < actions->extras_size; i++) {
        free_chk(actions->extras[i].name);
        free_chk(actions->extras[i].anim);
        free_chk(actions->extras[i].anim_hold);
        free_chk(actions->extras[i].anim_rollback);
        free_chk(actions->extras[i].model_src);
    }
    free_chk(actions->extras);

    if (actions->has_hey) {
        free_chk(actions->hey.name);
        free_chk(actions->hey.anim);
        free_chk(actions->hey.anim_hold);
        free_chk(actions->hey.anim_rollback);
        free_chk(actions->hey.model_src);
    }

    if (actions->has_idle) {
        free_chk(actions->idle.name);
        free_chk(actions->idle.anim);
        free_chk(actions->idle.anim_hold);
        free_chk(actions->idle.anim_rollback);
        free_chk(actions->idle.model_src);
    }
}

static ArrayList character_manifest_internal_read_additional_states(JSONToken json_array, const char* default_model, const char* src) {
    ArrayList additional_states = arraylist_init(sizeof(CharacterManifestAdditionalState));

    int32_t size = json_read_array_length(json_array);
    for (int32_t i = 0; i < size; i++) {
        JSONToken item = json_read_array_item_object(json_array, i);

        CharacterManifestAdditionalState state = {
            .name = json_read_string2(item, "name", NULL),
            .model = charactermanifest_internal_path_of(item, "model", NULL),
            .actions = {
                .extras = NULL,
                .extras_size = 0,
                .has_hey = false,
                .has_idle = false,
                .hey = {},
                .idle = {},
                .miss = NULL,
                .miss_size = 0,
                .sing = NULL,
                .sing_alt = NULL,
                .sing_size = 0
            }
        };

        if (!state.model) state.model = string_duplicate(default_model);

        JSONToken json_actions = json_read_object(item, "actions");
        if (!json_is_property_null(item, "actions")) {
            character_manifest_internal_parse_actions(json_actions, &state.actions, src);
        }

        arraylist_add(additional_states, &state);
    }

    return additional_states;
}
