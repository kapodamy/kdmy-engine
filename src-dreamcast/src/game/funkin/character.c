#include "game/funkin/character.h"

#include "arraylist.h"
#include "arraypointerlist.h"
#include "beatwatcher.h"
#include "drawable.h"
#include "externals/luascript.h"
#include "game/common/funkin.h"
#include "game/gameplay/helpers/charactermanifest.h"
#include "malloc_utils.h"
#include "math2d.h"
#include "statesprite.h"
#include "stringutils.h"
#include "unused_switch_case.h"
#include "vertexprops_types.h"


#define CHARACTER_ROLLBACK_SPEED 8.0f


typedef enum {
    CharacterAnimType_BASE,
    CharacterAnimType_HOLD,
    CharacterAnimType_ROLLBACK
} CharacterAnimType;

typedef struct {
    const char* model_src;
    ModelHolder modelholder;
} CharacterModelInfo;
typedef struct {
    Texture texture;
    float resolution_width;
    float resolution_height;
} CharacterTextureInfo;
typedef struct {
    int32_t id_extra;
    float stop_after_beats;
    bool is_valid;
    AnimSprite base;
    AnimSprite hold;
    AnimSprite rollback;
    int32_t id_texture;
    float offset_x;
    float offset_y;
} CharacterActionExtra;
typedef struct {
    int32_t id_direction;
    AnimSprite base;
    AnimSprite hold;
    AnimSprite rollback;
    bool base_can_rollback;
    bool hold_can_rollback;
    bool follow_hold;
    bool full_sustain;
    float stop_after_beats;
    int32_t id_texture;
    float offset_x;
    float offset_y;
} CharacterActionSing;
typedef struct {
    int32_t id_direction;
    AnimSprite animation;
    float stop_after_beats;
    int32_t id_texture;
    float offset_x;
    float offset_y;
} CharacterActionMiss;
typedef struct {
    char* name;
    int32_t sing_size;
    int32_t miss_size;
    int32_t extras_size;
    CharacterActionSing* sing;
    CharacterActionSing* sing_alt;
    CharacterActionMiss* miss;
    CharacterActionExtra* extras;
    CharacterActionExtra hey;
    CharacterActionExtra idle;
    CharacterActionExtra idle_alt;
} CharacterState;
typedef struct {
    ArrayPointerList /*<char*>*/ all_directions_names;
    ArrayPointerList /*<char*>*/ all_extras_names;
    ArrayList /*<CharacterTextureInfo>*/ textures;
    ArrayList /*<CharacterModelInfo>*/ modelholder_arraylist;
    ArrayList /*<CharacterState>*/ states;
    CharacterManifest charactermanifest;
} CharacterImportContext;


struct Character_s {
    StateSprite statesprite;
    int32_t current_texture_id;
    ArrayList /*<CharacterTextureInfo>*/ textures;
    int32_t all_extras_names_size;
    int32_t all_directions_names_size;
    char** all_directions_names;
    char** all_extras_names;
    CharacterState* default_state;
    ArrayList /*<CharacterState>*/ states;
    CharacterState* current_state;
    AnimSprite current_anim;
    bool current_use_frame_rollback;
    CharacterAnimType current_anim_type;
    CharacterActionExtra* current_action_extra;
    CharacterActionMiss* current_action_miss;
    CharacterActionSing* current_action_sing;
    CharacterActionType current_action_type;
    float current_expected_duration;
    bool current_follow_hold;
    bool current_idle_in_next_beat;
    bool current_waiting_animation_end_and_idle;
    bool current_waiting_animation_end;
    bool alt_enabled;
    bool continuous_idle;
    BeatWatcher beatwatcher;
    float idle_speed;
    bool allow_speed_change;
    Drawable drawable;
    AnimSprite drawable_animation;
    float draw_x;
    float draw_y;
    Align align_vertical;
    Align align_horizontal;
    Align manifest_align_vertical;
    Align manifest_align_horizontal;
    float reference_width;
    float reference_height;
    bool enable_reference_size;
    float offset_x;
    float offset_y;
    bool is_left_facing;
    bool inverted_enabled;
    int32_t inverted_size;
    int32_t* inverted_from;
    int32_t* inverted_to;
    float character_scale;
    int32_t played_actions_count;
    int32_t commited_animations_count;
    bool animation_freezed;
    bool flip_x;
};


static void character_internal_import_sing(CharacterActionSing* sing_info, ModelHolder modelholder, CharacterManifestSing* sing_entry, int32_t id_direction, const char* prefix, const char* suffix);
static void character_internal_import_miss(CharacterActionMiss* miss_info, ModelHolder modelholder, CharacterManifestMiss* miss_entry, int32_t id_direction);
static void character_internal_import_extra(CharacterActionExtra* extra_info, ArrayList mdlhldr_rrlst, ArrayList txtr_rrlst, CharacterManifestExtra* extra_entry, int32_t id_extra, const char* prefix, const char* suffix, const char* model_src);
static AnimSprite character_internal_import_animation(ModelHolder mdlhldr, const char* anim_name, const char* prefix, const char* suffix, bool is_sustain);
static AnimSprite character_internal_import_animation2(ModelHolder modelholder, const char* name, bool is_sustain);
static AnimSprite character_internal_import_animation3(ModelHolder modelholder, const char* state_name, AnimSprite animation, bool is_sustain);
static CharacterState character_internal_state_create(const char* name, int32_t sing_size, int32_t miss_size, int32_t extras_size);
static void character_internal_state_of_sing(CharacterActionSing* new_singinfo, ModelHolder modelholder, const char* state_name, CharacterActionSing* sing_info);
static void character_internal_state_of_extra(CharacterActionExtra* new_extrainfo, ModelHolder modelholder, const char* state_name, CharacterActionExtra* extra_info);
static int32_t character_internal_get_direction_id(Character character, const char* direction);
static int32_t character_internal_get_extra_id(Character character, const char* direction);
static int32_t character_internal_index_name(ArrayPointerList arraylist, const char* name, bool add_if_not_found);
static void character_internal_update_texture(Character character);
static void character_internal_import_opposite_dir(Character character, int32_t** array_ptr, char** ltr_array);
static void character_internal_calculate_location(Character character);
static void character_internal_end_current_action(Character character);
static void character_internal_fallback_idle(Character character);
static int32_t character_internal_add_texture(ArrayList texture_arraylist, ModelHolder modelholder);
static ModelHolder character_internal_get_modelholder(ArrayList modelholder_arraylist, const char* model_src, bool is_optional);
static void character_internal_destroy_modelholder(CharacterModelInfo* modelholder_arraylist_item_ptr);
static CharacterState* character_internal_import_actions(CharacterImportContext* context, CharacterManifestActions* actions, const char* model_src, const char* state_name);
static int32_t character_internal_animate_sing(Character character, bool completed, bool should_stop);
static int32_t character_internal_animate_miss(Character character, bool completed, bool should_stop);
static int32_t character_internal_animate_idle(Character character, bool completed, bool has_beat);
static int32_t character_internal_animate_extra(Character character, bool completed, bool should_stop);


Character character_init(CharacterManifest charactermanifest) {
    CharacterImportContext import_context = {
        .all_directions_names = arraypointerlist_init(),
        .all_extras_names = arraypointerlist_init(),
        .charactermanifest = charactermanifest,
        .modelholder_arraylist = arraylist_init2(sizeof(CharacterModelInfo), 4),
        .textures = arraylist_init2(sizeof(CharacterTextureInfo), 4),
        .states = arraylist_init2(sizeof(CharacterState), 4)
    };

    character_internal_get_modelholder(
        import_context.modelholder_arraylist, charactermanifest->model_character, false
    );

    Character character = malloc_chk(sizeof(struct Character_s));
    malloc_assert(character, Character);

    *character = (struct Character_s){
        .statesprite = statesprite_init_from_texture(NULL),

        .current_texture_id = -1,
        .textures = import_context.textures,

        .all_extras_names_size = -1,
        .all_extras_names = NULL,
        .all_directions_names_size = -1,
        .all_directions_names = NULL,

        .states = import_context.states,
        .default_state = NULL,
        .current_state = NULL,

        .current_action_extra = NULL,
        .current_action_miss = NULL,
        .current_action_sing = NULL,

        .current_anim_type = CharacterAnimType_BASE,
        .current_action_type = CharacterActionType_NONE,
        .current_anim = NULL,
        .current_use_frame_rollback = false,
        .current_follow_hold = false,
        .current_expected_duration = 0.0f,
        .current_idle_in_next_beat = false,
        .current_waiting_animation_end_and_idle = false,
        .current_waiting_animation_end = false,

        .alt_enabled = false,
        .continuous_idle = charactermanifest->continuous_idle,

        .beatwatcher = (BeatWatcher){
            .count = 0,
            .count_beats_or_quarters = false,
            .drift_count = 0,
            .drift_timestamp = 0,
            .since = 0.0f,
            .tick = 0.0f,
        },

        .idle_speed = 1.0f,
        .allow_speed_change = charactermanifest->actions_apply_chart_speed,

        .drawable = NULL,
        .drawable_animation = NULL,

        .draw_x = 0.0f,
        .draw_y = 1.0f,
        .align_vertical = charactermanifest->align_vertical,
        .align_horizontal = charactermanifest->align_horizontal,

        .manifest_align_vertical = charactermanifest->align_vertical,
        .manifest_align_horizontal = charactermanifest->align_horizontal,

        .reference_width = -1.0f,
        .reference_height = -1.0f,
        .enable_reference_size = false,

        .offset_x = charactermanifest->offset_x,
        .offset_y = charactermanifest->offset_y,

        .is_left_facing = charactermanifest->left_facing,

        .inverted_enabled = false,

        .inverted_size = -1,
        .inverted_from = NULL,
        .inverted_to = NULL,

        .character_scale = 1.0f,
        .played_actions_count = 0,
        .commited_animations_count = 0,
        .animation_freezed = false,
        .flip_x = false
    };

    beatwatcher_reset(&character->beatwatcher, true, 100.0f);

    if (charactermanifest->has_reference_size) {
        character->reference_width = charactermanifest->reference_width;
        character->reference_height = charactermanifest->reference_height;
    }

    statesprite_set_visible(character->statesprite, false);
    statesprite_flip_texture_enable_correction(character->statesprite, false);

    // import default actions
    character->default_state = character_internal_import_actions(&import_context, &charactermanifest->actions, NULL, NULL);
    character->current_state = character->default_state;

    // import additional actions as states
    for (int32_t i = 0; i < charactermanifest->additional_states_size; i++) {
        CharacterManifestAdditionalState* additionalstate = &charactermanifest->additional_states[i];
        character_internal_import_actions(&import_context, &additionalstate->actions, additionalstate->model, additionalstate->name);
    }

    character->drawable = drawable_init(0.0f, character, (DelegateDraw)character_draw, (DelegateAnimate)character_animate);

    arraylist_destroy3(&import_context.modelholder_arraylist, (FreeDelegate)character_internal_destroy_modelholder);
    arraypointerlist_destroy2(&import_context.all_directions_names, &character->all_directions_names_size, (void**)&character->all_directions_names);
    arraypointerlist_destroy2(&import_context.all_extras_names, &character->all_extras_names_size, (void**)&character->all_extras_names);

    character->inverted_size = charactermanifest->opposite_directions.sizes;
    if (character->inverted_size > 0 && charactermanifest->actions.sing_size > 0) {
        character_internal_import_opposite_dir(
            character, &character->inverted_from, charactermanifest->opposite_directions.from
        );
        character_internal_import_opposite_dir(
            character, &character->inverted_to, charactermanifest->opposite_directions.to
        );
    }

    character_state_toggle(character, NULL);
    character_play_idle(character);
    // character_face_as_opponent(character, character->is_left_facing);

    return character;
}

void character_destroy(Character* character_ptr) {
    if (!character_ptr || !*character_ptr) return;

    Character character = *character_ptr;

    luascript_drop_shared(character);

    statesprite_destroy(&character->statesprite);

    int32_t states_size = arraylist_size(character->states);

    for (int32_t i = 0; i < states_size; i++) {
        CharacterState* state = arraylist_get(character->states, i);

        for (int32_t j = 0; j < state->sing_size; j++) {
            if (state->sing[j].base) animsprite_destroy(&state->sing[j].base);
            if (state->sing[j].hold) animsprite_destroy(&state->sing[j].hold);
            if (state->sing[j].rollback) animsprite_destroy(&state->sing[j].rollback);

            if (state->sing_alt[j].base) animsprite_destroy(&state->sing_alt[j].base);
            if (state->sing_alt[j].hold) animsprite_destroy(&state->sing_alt[j].hold);
            if (state->sing_alt[j].rollback) animsprite_destroy(&state->sing_alt[j].rollback);
        }

        for (int32_t j = 0; j < state->miss_size; j++) {
            if (state->miss[j].animation) animsprite_destroy(&state->miss[j].animation);
        }

        for (int32_t j = 0; j < state->extras_size; j++) {
            if (state->extras[j].base) animsprite_destroy(&state->extras[j].base);
            if (state->extras[j].hold) animsprite_destroy(&state->extras[j].hold);
            if (state->extras[j].rollback) animsprite_destroy(&state->extras[j].rollback);
        }

        if (state->hey.base) animsprite_destroy(&state->hey.base);
        if (state->hey.hold) animsprite_destroy(&state->hey.hold);
        if (state->hey.rollback) animsprite_destroy(&state->hey.rollback);

        if (state->idle.base) animsprite_destroy(&state->idle.base);
        if (state->idle.hold) animsprite_destroy(&state->idle.hold);
        if (state->idle.rollback) animsprite_destroy(&state->idle.rollback);

        if (state->idle_alt.base) animsprite_destroy(&state->idle_alt.base);
        if (state->idle_alt.hold) animsprite_destroy(&state->idle_alt.hold);
        if (state->idle_alt.rollback) animsprite_destroy(&state->idle_alt.rollback);

        free_chk(state->name);
        free_chk(state->sing);
        free_chk(state->sing_alt);
        free_chk(state->miss);
        free_chk(state->extras);
    }

    arraylist_destroy(&character->states);

    for (int32_t i = 0; i < character->all_directions_names_size; i++) {
        free_chk(character->all_directions_names[i]);
    }
    for (int32_t i = 0; i < character->all_extras_names_size; i++) {
        free_chk(character->all_extras_names[i]);
    }

    drawable_destroy(&character->drawable);
    if (character->drawable_animation) animsprite_destroy(&character->drawable_animation);

    if (character->inverted_size > 0) {
        free_chk(character->inverted_from);
        free_chk(character->inverted_to);
    }

    free_chk(character->all_directions_names);
    free_chk(character->all_extras_names);

    foreach (CharacterTextureInfo*, texture_info, ARRAYLIST_ITERATOR, character->textures)
        texture_destroy(&texture_info->texture);
    arraylist_destroy(&character->textures);

    free_chk(character);

    *character_ptr = NULL;
}


void character_use_alternate_sing_animations(Character character, bool enable) {
    character->alt_enabled = enable;
}

void character_set_draw_location(Character character, float x, float y) {
    Modifier* modifier = drawable_get_modifier(character->drawable);
    if (!math2d_is_float_NaN(x)) character->draw_x = modifier->x = x;
    if (!math2d_is_float_NaN(y)) character->draw_y = modifier->y = y;
    character_internal_calculate_location(character);
}

void character_set_draw_align(Character character, Align align_vertical, Align align_horizontal) {
    if (align_vertical == ALIGN_BOTH || align_vertical == ALIGN_NONE)
        character->align_vertical = character->manifest_align_vertical;
    else
        character->align_vertical = align_vertical;

    if (align_horizontal == ALIGN_BOTH || align_horizontal == ALIGN_NONE)
        character->align_horizontal = character->manifest_align_horizontal;
    else
        character->align_horizontal = align_horizontal;

    character_internal_calculate_location(character);
}

void character_update_reference_size(Character character, float width, float height) {
    if (width >= 0.0f) character->reference_width = width;
    if (height >= 0.0f) character->reference_height = height;
    character_internal_calculate_location(character);
}

void character_enable_reference_size(Character character, bool enable) {
    character->enable_reference_size = enable;
    character_internal_calculate_location(character);
}

void character_set_offset(Character character, float offset_x, float offset_y) {
    character->offset_x = offset_x;
    character->offset_y = offset_y;
}


bool character_state_add(Character character, ModelHolder modelholder, const char* state_name) {
    //
    // expensive operation, call it before any gameplay
    //
    if (!modelholder) return false;

    int32_t states_size = arraylist_size(character->states);
    for (int32_t i = 0; i < states_size; i++) {
        CharacterState* existing_state = arraylist_get(character->states, i);
        if (string_equals(existing_state->name, state_name)) return false;
    }

    int32_t id_texture = character_internal_add_texture(character->textures, modelholder);
    CharacterState* default_state = character->default_state;

    CharacterState state = character_internal_state_create(
        state_name, default_state->sing_size, default_state->miss_size, default_state->extras_size
    );

    for (int32_t i = 0; i < default_state->sing_size; i++) {
        state.sing[i].id_texture = id_texture;
        character_internal_state_of_sing(&state.sing[i], modelholder, state_name, &default_state->sing[i]);

        state.sing_alt[i].id_texture = id_texture;
        character_internal_state_of_sing(&state.sing_alt[i], modelholder, state_name, &default_state->sing_alt[i]);
    }

    for (int32_t i = 0; i < default_state->miss_size; i++) {
        AnimSprite animation = character_internal_import_animation3(modelholder, state_name, default_state->miss[i].animation, false);

        state.miss[i].id_direction = default_state->miss[i].id_direction;
        state.miss[i].stop_after_beats = default_state->miss[i].stop_after_beats;
        state.miss[i].animation = animation;
        state.miss[i].id_texture = id_texture;
        state.miss[i].offset_x = default_state->miss[i].offset_x;
        state.miss[i].offset_y = default_state->miss[i].offset_y;
    }

    for (int32_t i = 0; i < default_state->extras_size; i++) {
        state.extras[i].id_texture = id_texture;
        character_internal_state_of_extra(&state.extras[i], modelholder, state_name, &default_state->extras[i]);
    }

    state.hey.id_texture = id_texture;
    character_internal_state_of_extra(&state.hey, modelholder, state_name, &default_state->hey);

    state.idle.id_texture = id_texture;
    character_internal_state_of_extra(&state.idle, modelholder, state_name, &default_state->idle);

    arraylist_add(character->states, &state);
    return true;
}

bool character_state_toggle(Character character, const char* state_name) {
    int32_t states_size = arraylist_size(character->states);

    for (int32_t i = 0; i < states_size; i++) {
        CharacterState* state = arraylist_get(character->states, i);
        if (string_equals(state->name, state_name)) {
            character->current_state = state;
            character_internal_update_texture(character);
            return true;
        }
    }

    return false;
}


bool character_play_hey(Character character) {
#ifdef DEBUG
    if (!character->current_state) {
        logger_error("character->current_state was NULL");
        assert(character->current_state);
    }
#endif

    CharacterActionExtra* extra_info = &character->current_state->hey;

    if (!extra_info->is_valid && character->current_state != character->default_state) {
        // attempt use default state
        extra_info = &character->default_state->hey;
    }

    if (!extra_info->is_valid) {
        character_internal_fallback_idle(character);
        return false;
    }

    // end current action
    character_internal_end_current_action(character);

    if (extra_info->stop_after_beats <= 0.0f) {
        character->current_expected_duration = FLOAT_Inf;
        character->current_waiting_animation_end = true;
        character->current_waiting_animation_end_and_idle = extra_info->stop_after_beats == 0.0f;
    } else {
        character->current_expected_duration = extra_info->stop_after_beats * character->beatwatcher.tick;
        character->current_waiting_animation_end = false;
        character->current_waiting_animation_end_and_idle = false;
    }

    if (extra_info->base) {
        character->current_anim = extra_info->base;
        character->current_anim_type = CharacterAnimType_BASE;
        character->current_follow_hold = extra_info->hold != NULL;
    } else {
        character->current_anim = extra_info->hold;
        character->current_anim_type = CharacterAnimType_HOLD;
        character->current_follow_hold = false;
    }

    animsprite_restart(character->current_anim);

    character->current_action_extra = extra_info;
    character->current_action_type = CharacterActionType_EXTRA;

    character->current_use_frame_rollback = false;
    character->current_idle_in_next_beat = false;

    character_internal_update_texture(character);
    character_internal_calculate_location(character);

    character->played_actions_count++;
    character->commited_animations_count++;

    return true;
}

int32_t character_play_idle(Character character) {
#ifdef DEBUG
    if (!character->current_state) {
        logger_error("character->current_state was NULL");
        assert(character->current_state);
    }
#endif

    character->played_actions_count++;

    CharacterActionType action_type = character->current_action_type;
    bool rollback_active = character->current_anim_type == CharacterAnimType_ROLLBACK;
    if (action_type != CharacterActionType_NONE && action_type != CharacterActionType_IDLE && !rollback_active) {

        // rollback the current action (if possible)
        int32_t ret = 0;
        switch (character->current_action_type) {
            case CharacterActionType_SING:
                ret = character_internal_animate_sing(character, true, true);
                break;
            case CharacterActionType_MISS:
                ret = character_internal_animate_miss(character, true, true);
                break;
            case CharacterActionType_EXTRA:
                ret = character_internal_animate_extra(character, true, true);
                break;
            default:
                character->current_idle_in_next_beat = true;
                character->current_action_type = CharacterActionType_NONE;
                break;
        }

        // do nothing if rollback is active or idle was previously used
        if (!character->current_idle_in_next_beat || character->current_action_type == CharacterActionType_IDLE) {
            return ret;
        }
    }

    CharacterState* state = character->current_state;
    CharacterActionExtra* extra_info = NULL;

L_read_state:
    extra_info = &state->idle;
    if (character->alt_enabled && state->idle_alt.is_valid) extra_info = &state->idle_alt;

    if (!extra_info->is_valid) {
        if (state != character->default_state) {
            // attempt use default state
            state = character->default_state;
            goto L_read_state;
        }
        return 0;
    }

    // end current action
    character_internal_end_current_action(character);

    if (extra_info->base) {
        character->current_anim = extra_info->base;
        character->current_anim_type = CharacterAnimType_BASE;
        character->current_follow_hold = extra_info->hold != NULL;
    } else {
        character->current_anim = extra_info->hold;
        character->current_anim_type = CharacterAnimType_HOLD;
        character->current_follow_hold = false;
    }

    animsprite_restart(character->current_anim);

    character->current_action_extra = extra_info;
    character->current_action_type = CharacterActionType_IDLE;

    character->current_expected_duration = 0.0f;
    character->current_use_frame_rollback = false;
    character->current_idle_in_next_beat = false;
    character->current_waiting_animation_end = false;

    character_internal_update_texture(character);
    character_internal_calculate_location(character);

    character->commited_animations_count++;

    return true;
}

bool character_play_sing(Character character, const char* direction, bool prefer_sustain) {
#ifdef DEBUG
    if (!character->current_state) {
        logger_error("character->current_state was NULL");
        assert(character->current_state);
    }
#endif

    int32_t id_direction = character_internal_get_direction_id(character, direction);
    if (id_direction < 0) {
        // unknown direction
        character_internal_fallback_idle(character);
        return false;
    };

    CharacterActionSing* sing_info = NULL;
    CharacterState* state = character->current_state;

L_read_state:
    CharacterActionSing* array = character->alt_enabled ? state->sing_alt : state->sing;
    for (int32_t i = 0; i < state->sing_size; i++) {
        if (array[i].id_direction == id_direction) {
            sing_info = &array[i];
            break;
        }
    }

    if (!sing_info || (!sing_info->base && !sing_info->hold)) {
        // attempt to use the non-alt sing direction
        if (character->alt_enabled) {
            for (int32_t i = 0; i < state->sing_size; i++) {
                if (state->sing[i].id_direction == id_direction) {
                    sing_info = &state->sing[i];
                    break;
                }
            }
        }

        if (!sing_info || (!sing_info->base && !sing_info->hold)) {
            if (state != character->default_state) {
                // attempt use default state
                state = character->default_state;
                goto L_read_state;
            }
            // logger_error("unknown sing direction: %s", direction);
            // assert((sing_info && (sing_info->base || sing_info->hold)) && (state == character->default_state));
            character_internal_fallback_idle(character);
            return false;
        }
    }

    // end current action
    character_internal_end_current_action(character);

    if (prefer_sustain) {
        // ignore "stopAfterBeats"
        character->current_expected_duration = FLOAT_Inf;
        character->current_waiting_animation_end = false;
        character->current_waiting_animation_end_and_idle = false;

        if (sing_info->full_sustain) {
            if (sing_info->base) {
                character->current_anim = sing_info->base;
                character->current_anim_type = CharacterAnimType_BASE;
                character->current_follow_hold = sing_info->hold != NULL;
            } else {
                character->current_anim = sing_info->hold;
                character->current_anim_type = CharacterAnimType_HOLD;
                character->current_follow_hold = false;
            }
        } else {
            character->current_anim = sing_info->hold ? sing_info->hold : sing_info->base;
            character->current_anim_type = sing_info->hold ? CharacterAnimType_HOLD : CharacterAnimType_BASE;
            character->current_follow_hold = false;
        }
    } else {
        character->current_anim = sing_info->base ? sing_info->base : sing_info->hold;
        character->current_anim_type = sing_info->base ? CharacterAnimType_BASE : CharacterAnimType_HOLD;
        character->current_follow_hold = sing_info->follow_hold && sing_info->base && sing_info->hold;

        // ignore "stopAfterBeats" if negative (waits for animation completion)
        if (sing_info->stop_after_beats <= 0.0) {
            character->current_waiting_animation_end = true;
            character->current_expected_duration = FLOAT_Inf;
            character->current_waiting_animation_end_and_idle = sing_info->stop_after_beats == 0.0f;
        } else {
            character->current_waiting_animation_end = false;
            character->current_expected_duration = sing_info->stop_after_beats * character->beatwatcher.tick;
            character->current_waiting_animation_end_and_idle = false;
        }
    }

    animsprite_restart(character->current_anim);

    character->current_action_sing = sing_info;
    character->current_action_type = CharacterActionType_SING;

    character->current_use_frame_rollback = false;
    character->current_idle_in_next_beat = false;

    character_internal_update_texture(character);
    character_internal_calculate_location(character);

    character->played_actions_count++;
    character->commited_animations_count++;

    return true;
}

bool character_play_miss(Character character, const char* direction, bool keep_in_hold) {
#ifdef DEBUG
    if (!character->current_state) {
        logger_error("character->current_state was NULL");
        assert(character->current_state);
    }
#endif

    int32_t id_direction = character_internal_get_direction_id(character, direction);
    if (id_direction < 0) {
        // unknown direction
        character_internal_fallback_idle(character);
        return false;
    };

    CharacterActionMiss* miss_info = NULL;
    CharacterState* state = character->current_state;

L_read_state:
    for (int32_t i = 0; i < state->miss_size; i++) {
        if (state->miss[i].id_direction == id_direction) {
            miss_info = &state->miss[i];
            break;
        }
    }

    if (!miss_info || !miss_info->animation) {
        if (state != character->default_state) {
            // attempt use default state
            state = character->default_state;
            goto L_read_state;
        }
        character_internal_fallback_idle(character);
        return false;
    }

    // end current action
    character_internal_end_current_action(character);

    if (keep_in_hold) {
        // ignore "stopAfterBeats"
        character->current_expected_duration = FLOAT_Inf;
        character->current_waiting_animation_end = false;
        character->current_waiting_animation_end_and_idle = false;
    } else if (miss_info->stop_after_beats <= 0.0f) {
        // wait for animation completion
        character->current_expected_duration = FLOAT_Inf;
        character->current_waiting_animation_end = true;
        character->current_waiting_animation_end_and_idle = miss_info->stop_after_beats == 0.0f;
    } else {
        character->current_expected_duration = miss_info->stop_after_beats * character->beatwatcher.tick;
        character->current_waiting_animation_end = false;
        character->current_waiting_animation_end_and_idle = false;
    }

    animsprite_restart(miss_info->animation);

    character->current_anim = miss_info->animation;
    character->current_action_type = CharacterActionType_MISS;
    character->current_action_miss = miss_info;

    character->current_follow_hold = false;
    character->current_use_frame_rollback = false;

    character_internal_update_texture(character);
    character_internal_calculate_location(character);

    character->played_actions_count++;
    character->commited_animations_count++;

    return true;
}

bool character_play_extra(Character character, const char* extra_animation_name, bool prefer_sustain) {
#ifdef DEBUG
    if (!character->current_state) {
        logger_error("character->current_state was NULL");
        assert(character->current_state);
    }
#endif

    int32_t id_extra = character_internal_get_extra_id(character, extra_animation_name);
    if (id_extra < 0) {
        // unknown extra
        return false;
    }

    CharacterState* state = character->current_state;
    CharacterActionExtra* extra_info = NULL;

L_read_state:
    for (int32_t i = 0; i < state->extras_size; i++) {
        if (state->extras[i].id_extra == id_extra) {
            extra_info = &state->extras[i];
            break;
        }
    }

    if (!extra_info || !extra_info->is_valid) {
        if (state != character->default_state) {
            // attempt use default state
            state = character->default_state;
            goto L_read_state;
        }
        // character_internal_fallback_idle(character);
        return false;
    }

    // end current action
    character_internal_end_current_action(character);

    if (prefer_sustain) {
        // ignore "stopAfterBeats"
        character->current_expected_duration = FLOAT_Inf;
        character->current_waiting_animation_end = false;
        character->current_waiting_animation_end_and_idle = false;
    } else if (extra_info->stop_after_beats <= 0.0f) {
        // wait for animation completion
        character->current_expected_duration = FLOAT_Inf;
        character->current_waiting_animation_end = true;
        character->current_waiting_animation_end_and_idle = extra_info->stop_after_beats == 0.0f;
    } else {
        character->current_expected_duration = extra_info->stop_after_beats * character->beatwatcher.tick;
        character->current_waiting_animation_end = false;
        character->current_waiting_animation_end_and_idle = false;
    }

    if (prefer_sustain) {
        character->current_anim = extra_info->hold ? extra_info->hold : extra_info->base;
        character->current_anim_type = extra_info->hold ? CharacterAnimType_HOLD : CharacterAnimType_BASE;
        character->current_follow_hold = false;
    } else {
        character->current_anim = extra_info->base ? extra_info->base : extra_info->hold;
        character->current_anim_type = extra_info->base ? CharacterAnimType_BASE : CharacterAnimType_HOLD;
        character->current_follow_hold = extra_info->base && extra_info->hold;
    }

    animsprite_restart(character->current_anim);

    character->current_action_extra = extra_info;
    character->current_action_type = CharacterActionType_EXTRA;

    character->current_use_frame_rollback = false;
    character->current_idle_in_next_beat = false;

    character_internal_update_texture(character);
    character_internal_calculate_location(character);

    character->played_actions_count++;
    character->commited_animations_count++;

    return true;
}

void character_schedule_idle(Character character) {
    character->played_actions_count++;
    character->commited_animations_count++;

    character->current_expected_duration = 0.0f;
    character->current_idle_in_next_beat = true;
}


void character_set_bpm(Character character, float beats_per_minute) {
    beatwatcher_change_bpm(&character->beatwatcher, beats_per_minute);
}

void character_set_idle_speed(Character character, float speed) {
    if (character->allow_speed_change) character->idle_speed = speed;
}

void character_set_scale(Character character, float scale_factor) {
    character->character_scale = scale_factor;
    character_internal_update_texture(character);
}

void character_reset(Character character) {
    beatwatcher_reset(&character->beatwatcher, true, 100.0f);

    character->idle_speed = 1.0f;
    character->alt_enabled = false;

    drawable_set_antialiasing(character->drawable, PVRCTX_FLAG_DEFAULT);

    Modifier* modifier = drawable_get_modifier(character->drawable);
    pvr_context_helper_clear_modifier(modifier);
    modifier->x = character->draw_x;
    modifier->y = character->draw_y;

    // switch to the default state
    character_state_toggle(character, NULL);

    character->current_action_type = CharacterActionType_NONE;
    character_play_idle(character);
}

void character_enable_continuous_idle(Character character, bool enable) {
    character->continuous_idle = enable;
}

bool character_is_idle_active(Character character) {
    return character->current_action_type == CharacterActionType_IDLE;
}

void character_enable_flip_correction(Character character, bool enable) {
    statesprite_flip_texture_enable_correction(character->statesprite, enable);
}

void character_flip_orientation(Character character, bool enable) {
    character->inverted_enabled = character->inverted_size > 0 && enable;
    statesprite_flip_texture(character->statesprite, enable, unset);
}

void character_face_as_opponent(Character character, bool face_as_opponent) {
    if (face_as_opponent)
        character->flip_x = character->is_left_facing;
    else
        character->flip_x = !character->is_left_facing;

    character->inverted_enabled = character->inverted_size > 0 && character->flip_x;
    statesprite_flip_texture(character->statesprite, character->flip_x, unset);
}



int32_t character_animate(Character character, float elapsed) {
    bool has_beat = beatwatcher_poll(&character->beatwatcher);

    if (character->drawable_animation != NULL) {
        animsprite_animate(character->drawable_animation, elapsed);
        animsprite_update_drawable(character->drawable_animation, character->drawable, true);
    }

    if (character->animation_freezed) {
        if (character->current_use_frame_rollback)
            animsprite_rollback(character->current_anim, elapsed * CHARACTER_ROLLBACK_SPEED);
        else
            animsprite_animate(character->current_anim, elapsed);

        animsprite_update_statesprite(character->current_anim, character->statesprite, true);
        character_internal_calculate_location(character);
        return 1;
    }

    statesprite_animate(character->statesprite, elapsed);

    if (character->current_action_type == CharacterActionType_NONE) return 1;

    bool completed;
    float orig_elapsed = elapsed;

    if (character->current_action_type == CharacterActionType_IDLE && character->idle_speed != 1.0f) {
        elapsed *= character->idle_speed;
    }

    if (character->current_use_frame_rollback)
        completed = animsprite_rollback(character->current_anim, elapsed * CHARACTER_ROLLBACK_SPEED);
    else
        completed = animsprite_animate(character->current_anim, elapsed);

    animsprite_update_statesprite(character->current_anim, character->statesprite, true);
    character_internal_calculate_location(character);

    bool should_stop = character->current_expected_duration <= 0.0f;
    character->current_expected_duration -= orig_elapsed;

    if (character->current_idle_in_next_beat) {
        if (has_beat && should_stop) {
            character->current_action_type = CharacterActionType_NONE;
            return character_play_idle(character);
        }
        return 1;
    }

    if (character->current_anim_type == CharacterAnimType_ROLLBACK) {
        if (completed) {
            character->current_action_type = CharacterActionType_NONE;
            return character_play_idle(character);
        }
        return 0;
    }

    switch (character->current_action_type) {
        case CharacterActionType_SING:
            return character_internal_animate_sing(character, completed, should_stop);
        case CharacterActionType_MISS:
            return character_internal_animate_miss(character, completed, should_stop);
        case CharacterActionType_IDLE:
            return character_internal_animate_idle(character, completed, has_beat);
        case CharacterActionType_EXTRA:
            return character_internal_animate_extra(character, completed, should_stop);
            CASE_UNUSED(CharacterActionType_NONE)
    }

    return 0;
}

void character_draw(Character character, PVRContext pvrctx) {
    // TODO: direction ghosting

    pvr_context_save(pvrctx);
    drawable_helper_apply_in_context(character->drawable, pvrctx);

    statesprite_draw(character->statesprite, pvrctx);
    pvr_context_restore(pvrctx);
}


void character_set_z_index(Character character, float z) {
    drawable_set_z_index(character->drawable, z);
}

void character_set_z_offset(Character character, float z) {
    drawable_set_z_offset(character->drawable, z);
}

void character_animation_set(Character character, AnimSprite animsprite) {
    if (character->drawable_animation) animsprite_destroy(&character->drawable_animation);
    character->drawable_animation = animsprite ? animsprite_clone(animsprite) : NULL;
}

void character_animation_restart(Character character) {
    if (character->drawable_animation) animsprite_restart(character->drawable_animation);
}

void character_animation_end(Character character) {
    if (character->drawable_animation) {
        animsprite_force_end(character->drawable_animation);
        animsprite_update_drawable(character->drawable_animation, character->drawable, true);
    }
}

void character_set_color_offset(Character character, float r, float g, float b, float a) {
    drawable_set_offsetcolor(character->drawable, r, g, b, a);
}

void character_set_color_offset_to_default(Character character) {
    drawable_set_offsetcolor_to_default(character->drawable);
}

void character_set_alpha(Character character, float alpha) {
    drawable_set_alpha(character->drawable, alpha);
}

void character_set_visible(Character character, bool visible) {
    drawable_set_visible(character->drawable, visible);
}

Modifier* character_get_modifier(Character character) {
    return drawable_get_modifier(character->drawable);
}

Drawable character_get_drawable(Character character) {
    return character->drawable;
}

int32_t character_get_play_calls(Character character) {
    return character->played_actions_count;
}

int32_t character_get_commited_animations_count(Character character) {
    return character->commited_animations_count;
}

CharacterActionType character_get_current_action(Character character) {
    return character->current_action_type;
}

bool character_has_direction(Character character, const char* name, bool is_extra) {
    char** array = is_extra ? character->all_extras_names : character->all_directions_names;
    int32_t size = is_extra ? character->all_extras_names_size : character->all_directions_names_size;

    for (int32_t i = 0; i < size; i++) {
        if (string_equals(array[i], name)) return true;
    }

    return false;
}

void character_freeze_animation(Character character, bool enabled) {
    character->animation_freezed = enabled;
}

void character_trailing_enabled(Character character, bool enabled) {
    statesprite_trailing_enabled(character->statesprite, enabled);
}

void character_trailing_set_params(Character character, int32_t length, float trail_delay, float trail_alpha, nbool darken_colors) {
    statesprite_trailing_set_params(character->statesprite, length, trail_delay, trail_alpha, darken_colors);
}

void character_trailing_set_offsetcolor(Character character, float r, float g, float b) {
    statesprite_trailing_set_offsetcolor(character->statesprite, r, g, b);
}



static void character_internal_import_sing(CharacterActionSing* sing_info, ModelHolder modelholder, CharacterManifestSing* sing_entry, int32_t id_direction, const char* prefix, const char* suffix) {

    sing_info->base = character_internal_import_animation(modelholder, sing_entry->anim, prefix, suffix, false);
    sing_info->hold = character_internal_import_animation(modelholder, sing_entry->anim_hold, prefix, suffix, true);
    sing_info->rollback = character_internal_import_animation(modelholder, sing_entry->direction, prefix, suffix, false);

    sing_info->id_direction = id_direction;
    sing_info->follow_hold = sing_entry->follow_hold;
    sing_info->full_sustain = sing_entry->full_sustain;
    sing_info->stop_after_beats = sing_entry->stop_after_beats;
    sing_info->offset_x = sing_entry->offset_x;
    sing_info->offset_y = sing_entry->offset_y;

    sing_info->base_can_rollback = false;
    sing_info->hold_can_rollback = false;

    if (!sing_info->rollback && sing_entry->rollback) {
        if (sing_info->base != NULL && animsprite_is_frame_animation(sing_info->base)) {
            sing_info->base_can_rollback = true;
        }
        if (sing_info->hold != NULL && animsprite_is_frame_animation(sing_info->hold)) {
            sing_info->hold_can_rollback = true;
        }
    }
}

static void character_internal_import_miss(CharacterActionMiss* miss_info, ModelHolder modelholder, CharacterManifestMiss* miss_entry, int32_t id_direction) {
    miss_info->animation = character_internal_import_animation(modelholder, miss_entry->anim, NULL, NULL, false);

    miss_info->id_direction = id_direction;
    miss_info->stop_after_beats = miss_entry->stop_after_beats;
    miss_info->offset_x = miss_entry->offset_x;
    miss_info->offset_y = miss_entry->offset_y;
}

static void character_internal_import_extra(CharacterActionExtra* extra_info, ArrayList mdlhldr_rrlst, ArrayList txtr_rrlst, CharacterManifestExtra* extra_entry, int32_t id_extra, const char* prefix, const char* suffix, const char* model_src) {
    if (!extra_entry) {
        extra_info->id_extra = -1;
        extra_info->is_valid = false;
        extra_info->base = NULL;
        extra_info->hold = NULL;
        extra_info->rollback = NULL;
        extra_info->offset_x = 0.0f;
        extra_info->offset_y = 0.0f;
        return;
    }

    ModelHolder modelholder = character_internal_get_modelholder(
        mdlhldr_rrlst, extra_entry->model_src ? extra_entry->model_src : model_src, true
    );
    extra_info->id_texture = character_internal_add_texture(
        txtr_rrlst, modelholder
    );

    extra_info->stop_after_beats = extra_entry->stop_after_beats;

    // this originally was:
    //          extra_entry->ANIM_NAME != NULL && extra_entry->ANIM_NAME[0] == '\0'
    // instead of
    //          string_is_empty(extra_entry->ANIM_NAME)
    // revert if something breaks
    //

    if (string_is_empty(extra_entry->anim))
        extra_info->base = NULL;
    else
        extra_info->base = character_internal_import_animation(modelholder, extra_entry->anim, prefix, suffix, false);

    if (string_is_empty(extra_entry->anim_hold)) {
        extra_info->hold = NULL;
    } else {
        extra_info->hold = character_internal_import_animation(
            modelholder, extra_entry->anim_hold, prefix, suffix, true
        );
    }

    if (string_is_empty(extra_entry->anim_rollback)) {
        extra_info->rollback = NULL;
    } else {
        extra_info->rollback = character_internal_import_animation(modelholder, extra_entry->anim_rollback, prefix, suffix, false);
    }

    extra_info->id_extra = id_extra;
    extra_info->is_valid = extra_info->base != NULL || extra_info->hold != NULL;
    extra_info->offset_x = extra_entry->offset_x;
    extra_info->offset_y = extra_entry->offset_y;
}



static AnimSprite character_internal_import_animation(ModelHolder mdlhldr, const char* anim_name, const char* prefix, const char* suffix, bool is_sustain) {
    if (!anim_name) return NULL;
    char* tmp = string_concat(3, prefix, anim_name, suffix);

    AnimSprite animsprite = character_internal_import_animation2(mdlhldr, tmp, is_sustain);
    free_chk(tmp);

    return animsprite;
}

static AnimSprite character_internal_import_animation2(ModelHolder modelholder, const char* name, bool is_sustain) {
    AnimSprite animsprite;

    // read from the animation list
    animsprite = animsprite_init_from_animlist(modelholder_get_animlist(modelholder), name);
    if (animsprite) return animsprite;

    // animation not found, build from atlas
    Atlas atlas = modelholder_get_atlas(modelholder);
    float fps = atlas_get_glyph_fps(atlas);
    if (fps <= 0.0f) fps = FUNKIN_DEFAULT_ANIMATIONS_FRAMERATE;

    int32_t loop_count = is_sustain ? 0 /*infinite*/ : 1 /*once*/;
    animsprite = animsprite_init_from_atlas(fps, loop_count, atlas, name, true);
    if (animsprite) return animsprite;

    // ¿static animation?
    const AtlasEntry* atlas_entry = atlas_get_entry(atlas, name);
    if (!atlas_entry) return NULL;

    return animsprite_init_from_atlas_entry(atlas_entry, is_sustain, fps);
}

static AnimSprite character_internal_import_animation3(ModelHolder modelholder, const char* state_name, AnimSprite animation, bool is_sustain) {
    if (!animation) return NULL;

    const char* name = animsprite_get_name(animation);
    char* anim_name = string_concat_for_state_name(2, name, state_name);

    AnimSprite anim = character_internal_import_animation2(modelholder, anim_name, is_sustain);
    free_chk(anim_name);

    return anim;
}




static CharacterState character_internal_state_create(const char* name, int32_t sing_size, int32_t miss_size, int32_t extras_size) {
    CharacterState state = {
        .name = string_duplicate(name),
        .sing_size = sing_size,
        .miss_size = miss_size,
        .extras_size = extras_size,
        .sing = malloc_for_array(CharacterActionSing, sing_size),
        .sing_alt = malloc_for_array(CharacterActionSing, sing_size),
        .miss = malloc_for_array(CharacterActionMiss, miss_size),
        .extras = malloc_for_array(CharacterActionExtra, extras_size),
        .hey = {
            .base = NULL,
            .hold = NULL,
            .rollback = NULL,
            .stop_after_beats = 0.0f,
            .id_extra = -1,
            .id_texture = -1,
            .is_valid = false,
            .offset_x = 0.0f,
            .offset_y = 0.0f,
        },
        .idle = {.base = NULL, .hold = NULL, .rollback = NULL, .stop_after_beats = 0.0f, .id_extra = -1, .id_texture = -1, .is_valid = false, .offset_x = 0.0f, .offset_y = 0.0f},
        .idle_alt = {.base = NULL, .hold = NULL, .rollback = NULL, .stop_after_beats = 0.0f, .id_extra = -1, .id_texture = -1, .is_valid = false, .offset_x = 0.0f, .offset_y = 0.0f}
    };

    return state;
}

static void character_internal_state_of_sing(CharacterActionSing* new_singinfo, ModelHolder modelholder, const char* state_name, CharacterActionSing* sing_info) {
    new_singinfo->base = character_internal_import_animation3(
        modelholder, state_name, sing_info->base, false
    );
    new_singinfo->hold = character_internal_import_animation3(
        modelholder, state_name, sing_info->hold, true
    );
    new_singinfo->rollback = character_internal_import_animation3(
        modelholder, state_name, sing_info->rollback, false
    );

    new_singinfo->id_direction = sing_info->id_direction;
    new_singinfo->follow_hold = sing_info->follow_hold;
    new_singinfo->full_sustain = sing_info->full_sustain;
    new_singinfo->stop_after_beats = sing_info->stop_after_beats;
    new_singinfo->offset_x = sing_info->offset_x;
    new_singinfo->offset_y = sing_info->offset_y;
    new_singinfo->base_can_rollback = false;
    new_singinfo->hold_can_rollback = false;

    if (!sing_info->rollback) {
        new_singinfo->base_can_rollback = new_singinfo->base && animsprite_is_frame_animation(new_singinfo->base);
        new_singinfo->hold_can_rollback = new_singinfo->hold && animsprite_is_frame_animation(new_singinfo->hold);
    }
}

static void character_internal_state_of_extra(CharacterActionExtra* new_extrainfo, ModelHolder modelholder, const char* state_name, CharacterActionExtra* extra_info) {
    new_extrainfo->base = character_internal_import_animation3(
        modelholder, state_name, extra_info->base, false
    );
    new_extrainfo->hold = character_internal_import_animation3(
        modelholder, state_name, extra_info->hold, true
    );
    new_extrainfo->rollback = character_internal_import_animation3(
        modelholder, state_name, extra_info->rollback, false
    );

    new_extrainfo->id_extra = extra_info->id_extra;
    new_extrainfo->stop_after_beats = extra_info->stop_after_beats;
    new_extrainfo->offset_x = extra_info->offset_x;
    new_extrainfo->offset_y = extra_info->offset_y;
    new_extrainfo->is_valid = new_extrainfo->base && new_extrainfo->hold;
}



static int32_t character_internal_get_direction_id(Character character, const char* direction) {
    for (int32_t i = 0; i < character->all_directions_names_size; i++) {
        if (!string_equals(character->all_directions_names[i], direction)) continue;

        if (character->inverted_enabled) {
            for (int32_t j = 0; j < character->inverted_size; j++) {
                if (character->inverted_from[j] == i) {
                    if (character->inverted_to[j] < 0)
                        break; // no inverse direction
                    else
                        return character->inverted_to[j];
                }
            }
        }

        return i;
    }
    // unknown direction
    return -1;
}

static int32_t character_internal_get_extra_id(Character character, const char* direction) {
    for (int32_t i = 0; i < character->all_extras_names_size; i++) {
        if (string_equals(character->all_extras_names[i], direction)) return i;
    }
    return -1;
}

static int32_t character_internal_index_name(ArrayPointerList arraylist, const char* name, bool add_if_not_found) {
    if (name == NULL) return -1;

    char** array = arraypointerlist_peek_array(arraylist);
    int32_t size = arraypointerlist_size(arraylist);

    for (int32_t i = 0; i < size; i++) {
        if (string_equals(array[i], name)) return i;
    }

    if (!add_if_not_found) return -1;

    arraypointerlist_add(arraylist, string_duplicate(name));
    return size;
}

static void character_internal_update_texture(Character character) {
    int32_t id_texture = -1;
    switch (character->current_action_type) {
        case CharacterActionType_EXTRA:
        case CharacterActionType_IDLE:
            id_texture = character->current_action_extra->id_texture;
            break;
        case CharacterActionType_MISS:
            id_texture = character->current_action_miss->id_texture;
            break;
        case CharacterActionType_SING:
            id_texture = character->current_action_sing->id_texture;
            break;
            CASE_UNUSED(CharacterActionType_NONE)
    }

    float final_scale = character->character_scale;

    if (id_texture >= 0 && id_texture != character->current_texture_id) {

        CharacterTextureInfo* texture_info = arraylist_get(character->textures, id_texture);
        character->current_texture_id = id_texture;

        //
        // (Does not work)
        // Calculate the scale required to fit the sprite in the layout and/or screen
        // and later apply this scale to the character scale.
        //
        // As long as the viewport size of the layout is intended for a 1280x720 screen, there
        // will be no problems.
        //
        //
        // let scale_factor = character->layout_height / texture_info.resolution_height;
        // final_scale *= scale_factor;

        statesprite_set_texture(character->statesprite, texture_info->texture, true);
    }

    statesprite_change_draw_size_in_atlas_apply(character->statesprite, true, final_scale);
    if (character->current_anim) animsprite_update_statesprite(character->current_anim, character->statesprite, false);
}

static void character_internal_import_opposite_dir(Character character, int32_t** array_ptr, char** ltr_array) {
    int32_t* array = malloc_for_array(int32_t, character->inverted_size);
    *array_ptr = array;

    for (int32_t i = 0; i < character->inverted_size; i++) {
        array[i] = -1;
        if (ltr_array[i] == NULL) continue;

        for (int32_t j = 0; j < character->all_directions_names_size; j++) {
            if (string_equals(character->all_directions_names[j], ltr_array[i])) {
                array[i] = j;
                break;
            }
        }

        if (array[i] < 0)
            logger_error("character_init() unknown sing/miss direction: %s", ltr_array[i]);
    }
}

static void character_internal_calculate_location(Character character) {
    float draw_width = 0.0f, draw_height = 0.0f;
    Modifier* modifier = drawable_get_modifier(character->drawable);

    float draw_x = character->draw_x;
    float draw_y = character->draw_y;

    statesprite_get_draw_size(character->statesprite, &draw_width, &draw_height);
    if (character->enable_reference_size) {
        if (character->reference_width >= 0.0f) draw_width = (character->reference_width - draw_width) / 2.0f;
        if (character->reference_height >= 0.0f) draw_height = (character->reference_height - draw_height) / 2.0f;
    }

    // pick current action offsets
    float action_offset_x, action_offset_y;
    switch (character->current_action_type) {
        case CharacterActionType_SING:
            action_offset_x = character->current_action_sing->offset_x;
            action_offset_y = character->current_action_sing->offset_y;
            break;
        case CharacterActionType_MISS:
            action_offset_x = character->current_action_miss->offset_x;
            action_offset_y = character->current_action_miss->offset_y;
            break;
        case CharacterActionType_IDLE:
        case CharacterActionType_EXTRA:
            action_offset_x = character->current_action_extra->offset_x;
            action_offset_y = character->current_action_extra->offset_y;
            break;
        default:
            action_offset_x = 0.0f;
            action_offset_y = 0.0f;
            break;
    }

    // step 1: apply align
    switch (character->align_vertical) {
        case ALIGN_CENTER:
            draw_y -= draw_height / 2.0f;
            break;
        case ALIGN_END:
            draw_y -= draw_height;
            break;
            CASE_UNUSED(ALIGN_INVALID)
            CASE_UNUSED(ALIGN_START)
            CASE_UNUSED(ALIGN_BOTH)
            CASE_UNUSED(ALIGN_NONE)
    }
    switch (character->align_horizontal) {
        case ALIGN_CENTER:
            draw_x -= draw_width / 2.0f;
            break;
        case ALIGN_END:
            draw_x -= draw_width;
            break;
            CASE_UNUSED(ALIGN_INVALID)
            CASE_UNUSED(ALIGN_START)
            CASE_UNUSED(ALIGN_BOTH)
            CASE_UNUSED(ALIGN_NONE)
    }

    // step 2: calc total offsets
    float offset_x = action_offset_x + character->offset_x;
    float offset_y = action_offset_y + character->offset_y;
    if (character->flip_x) offset_x = -offset_x;

    // step 3: apply offsets
    draw_x += offset_x * character->character_scale;
    draw_y += offset_y * character->character_scale;

    // step 4: change the sprite location
    modifier->x = draw_x;
    modifier->y = draw_y;
    statesprite_set_draw_location(character->statesprite, draw_x, draw_y);
}

static void character_internal_end_current_action(Character character) {
    if (!character->current_anim || character->current_idle_in_next_beat) return;

    AnimSprite base = NULL;
    AnimSprite hold = NULL;
    AnimSprite rollback = NULL;

    switch (character->current_action_type) {
        case CharacterActionType_SING:
            rollback = character->current_action_sing->rollback;
            switch (character->current_anim_type) {
                case CharacterAnimType_BASE:
                    base = character->current_action_sing->base;
                    if (character->current_action_sing->follow_hold) hold = character->current_action_sing->hold;
                    break;
                case CharacterAnimType_HOLD:
                    hold = character->current_action_sing->hold;
                    break;
                case CharacterAnimType_ROLLBACK:
                    if (rollback) animsprite_restart(rollback);
                    break;
            }
            break;
        case CharacterActionType_MISS:
            base = character->current_action_miss->animation;
            break;
        case CharacterActionType_IDLE:
        case CharacterActionType_EXTRA:
            rollback = character->current_action_extra->rollback;
            switch (character->current_anim_type) {
                case CharacterAnimType_BASE:
                    base = character->current_action_extra->base;
                    hold = character->current_action_extra->hold;
                    break;
                case CharacterAnimType_HOLD:
                    hold = character->current_action_extra->hold;
                    break;
                case CharacterAnimType_ROLLBACK:
                    if (rollback) animsprite_restart(rollback);
                    break;
            }
            break;
            CASE_UNUSED(CharacterActionType_NONE)
    }

    if (base) animsprite_force_end3(base, character->statesprite);
    if (hold) animsprite_force_end3(hold, character->statesprite);
    if (rollback) animsprite_force_end3(rollback, character->statesprite);
}

static void character_internal_fallback_idle(Character character) {
    if (character->current_action_type == CharacterActionType_IDLE) return;
    character_internal_end_current_action(character);
    character->current_action_type = CharacterActionType_NONE;
    character_play_idle(character);
}

static int32_t character_internal_add_texture(ArrayList texture_arraylist, ModelHolder modelholder) {
    CharacterTextureInfo* array = arraylist_peek_array(texture_arraylist);
    int32_t size = arraylist_size(texture_arraylist);

    Texture texture = modelholder_get_texture(modelholder, false);
    if (!texture) return -1;

    for (int32_t i = 0; i < size; i++) {
        if (array[i].texture == texture) return i;
    }

    int32_t resolution_width = 0, resolution_height = 0;
    modelholder_get_texture_resolution(modelholder, &resolution_width, &resolution_height);

    arraylist_add(
        texture_arraylist,
        &(CharacterTextureInfo){
            .texture = modelholder_get_texture(modelholder, true),
            .resolution_width = resolution_width,
            .resolution_height = resolution_height,
        }
    );

    return size;
}

static ModelHolder character_internal_get_modelholder(ArrayList modelholder_arraylist, const char* model_src, bool is_optional) {
    if (string_is_empty(model_src)) {
        if (is_optional) {
            CharacterModelInfo* model_info = arraylist_get(modelholder_arraylist, 0);
            assert(model_info);
            return model_info->modelholder;
        } else {
            logger_error("character model not specified in the manifest");
            assert(is_optional);
        }
    }

    CharacterModelInfo* array = arraylist_peek_array(modelholder_arraylist);
    int32_t size = arraylist_size(modelholder_arraylist);

    for (int32_t i = 0; i < size; i++) {
        if (string_equals(array[i].model_src, model_src)) return array[i].modelholder;
    }

    ModelHolder modelholder = modelholder_init(model_src);
    if (!modelholder || modelholder_is_invalid(modelholder)) {
        logger_error("model not found: %s", model_src);
        assert(modelholder && !modelholder_is_invalid(modelholder));
    }

    arraylist_add(modelholder_arraylist, &(CharacterModelInfo){.model_src = model_src, .modelholder = modelholder});
    return modelholder;
}

static void character_internal_destroy_modelholder(CharacterModelInfo* modelholder_arraylist_item) {
    modelholder_destroy(&modelholder_arraylist_item->modelholder);
}

static CharacterState* character_internal_import_actions(CharacterImportContext* context, CharacterManifestActions* actions, const char* model_src, const char* state_name) {
    CharacterState state = character_internal_state_create(state_name, actions->sing_size, actions->miss_size, actions->extras_size);

    // import all sign actions
    for (int32_t i = 0; i < actions->sing_size; i++) {
        int32_t index = character_internal_index_name(
            context->all_directions_names, actions->sing[i].direction, true
        );

        ModelHolder modelholder = character_internal_get_modelholder(
            context->modelholder_arraylist, actions->sing[i].model_src ? actions->sing[i].model_src : model_src, true
        );

        state.sing[i].id_texture = state.sing_alt[i].id_texture = character_internal_add_texture(
            context->textures, modelholder
        );

        character_internal_import_sing(
            &state.sing[i],
            modelholder, &actions->sing[i],
            index,
            context->charactermanifest->sing_prefix,
            context->charactermanifest->sing_suffix
        );
        character_internal_import_sing(
            &state.sing_alt[i],
            modelholder,
            &actions->sing[i],
            index,
            context->charactermanifest->sing_alternate_prefix,
            context->charactermanifest->sing_alternate_suffix
        );
    }

    // import all miss actions
    for (int32_t i = 0; i < actions->miss_size; i++) {
        int32_t index = character_internal_index_name(
            context->all_directions_names, actions->miss[i].direction, true
        );

        ModelHolder modelholder = character_internal_get_modelholder(
            context->modelholder_arraylist, actions->miss[i].model_src ? actions->miss[i].model_src : model_src, true
        );

        state.miss[i].id_texture = character_internal_add_texture(
            context->textures, modelholder
        );

        character_internal_import_miss(
            &state.miss[i], modelholder, &actions->miss[i], index
        );
    }

    // import all extras
    for (int32_t i = 0; i < actions->extras_size; i++) {
        int32_t index = character_internal_index_name(
            context->all_extras_names, actions->extras[i].name, true
        );

        character_internal_import_extra(
            &state.extras[i],
            context->modelholder_arraylist,
            context->textures,
            &actions->extras[i],
            index,
            NULL,
            NULL,
            model_src
        );
    }

    character_internal_import_extra(
        &state.hey,
        context->modelholder_arraylist,
        context->textures,
        &actions->hey,
        -10,
        NULL,
        NULL,
        model_src
    );

    character_internal_import_extra(
        &state.idle,
        context->modelholder_arraylist,
        context->textures,
        &actions->idle,
        -11,
        context->charactermanifest->allow_alternate_idle ? context->charactermanifest->sing_prefix : NULL,
        context->charactermanifest->allow_alternate_idle ? context->charactermanifest->sing_suffix : NULL,
        model_src
    );

    if (context->charactermanifest->allow_alternate_idle) {
        character_internal_import_extra(
            &state.idle_alt,
            context->modelholder_arraylist,
            context->textures,
            &actions->idle,
            -11,
            context->charactermanifest->sing_alternate_prefix,
            context->charactermanifest->sing_alternate_suffix,
            model_src
        );
    }

    return arraylist_add(context->states, &state);
}

static int32_t character_internal_animate_sing(Character character, bool completed, bool should_stop) {
    CharacterActionSing* action = character->current_action_sing;

    if (!completed && !should_stop) {
        // keep waiting
        return 0;
    }

    // play "animHold" if necessary
    if (character->current_follow_hold && !should_stop) {
        character->commited_animations_count++;
        character->current_anim_type = CharacterAnimType_HOLD;
        character->current_follow_hold = false;
        character->current_anim = action->hold;
        animsprite_restart(character->current_anim);
        return 1;
    }

    if (character->current_waiting_animation_end) {
        if (!completed) return 0;
    } else if (!should_stop) {
        // edge case: sustain sing with "anim"+"animHold" ended. keep static
        // edge case: sustain sing with "anim" only ended. keep static
        return 1;
    }

    //
    // Note: only do rollback here for non-sustain sing actions
    //
    character->commited_animations_count++;

    // if exists a rollback animation use it
    if (action->rollback) {
        character->current_expected_duration = FLOAT_Inf;
        character->current_anim = action->rollback;
        character->current_anim_type = CharacterAnimType_ROLLBACK;
        character->current_use_frame_rollback = false;
        animsprite_restart(character->current_anim);
        return 1;
    }

    // if not, try do self rollback
    bool is_base = character->current_anim_type == CharacterAnimType_BASE;
    bool can_frame_rollback = is_base ? action->base_can_rollback : action->hold_can_rollback;

    if (!can_frame_rollback) {
        if (character->current_waiting_animation_end_and_idle) {
            // do not wait for next beat, play idle now
            character->current_action_type = CharacterActionType_NONE;
            return character_play_idle(character);
        }
        // schedule idle
        character->current_expected_duration = 0.0f;
        character->current_idle_in_next_beat = true;
        return 1;
    }

    // do frame rollback
    character->current_use_frame_rollback = true;
    character->current_anim_type = CharacterAnimType_ROLLBACK;
    return 1;
}

static int32_t character_internal_animate_miss(Character character, bool completed, bool should_stop) {
    if (!completed && !should_stop) {
        // nothing to do
        return 0;
    }

    if (character->current_waiting_animation_end) {
        if (!completed) return 0;
    } else if (!should_stop) {
        return 1;
    }

    character->commited_animations_count++;

    if (character->current_waiting_animation_end_and_idle) {
        // do not wait for next beat, play idle now
        character->current_action_type = CharacterActionType_NONE;
        return character_play_idle(character);
    }

    // schedule idle
    character->current_expected_duration = 0.0f;
    character->current_idle_in_next_beat = true;
    return 1;
}

static int32_t character_internal_animate_idle(Character character, bool completed, bool has_beat) {
    if (!completed) {

        // interrupt "animHold" when a beat is detected
        if (has_beat && !character->continuous_idle && character->current_anim_type == CharacterAnimType_HOLD) {
            character->current_action_type = CharacterActionType_NONE;
            character_play_idle(character);
            return 1;
        }

        return 0;
    }

    character->commited_animations_count++;

    if (character->current_follow_hold) {
        character->current_follow_hold = false;
        character->current_anim = character->current_action_extra->hold;
        character->current_anim_type = CharacterAnimType_HOLD;
        animsprite_restart(character->current_anim);
        return 1;
    }

    if (character->continuous_idle) {
        character->current_action_type = CharacterActionType_NONE;
        character_play_idle(character);
        return 1;
    }

    character->current_expected_duration = 0.0f;
    character->current_idle_in_next_beat = true;
    return 1;
}

static int32_t character_internal_animate_extra(Character character, bool completed, bool should_stop) {
    CharacterActionExtra* action = character->current_action_extra;

    if (!completed) return 0;

    // play "animHold" if necessary
    if (character->current_follow_hold && !should_stop) {
        character->commited_animations_count++;
        character->current_anim_type = CharacterAnimType_HOLD;
        character->current_follow_hold = false;
        character->current_anim = action->hold;
        animsprite_restart(character->current_anim);
        return 1;
    }

    if (character->current_waiting_animation_end) {
        if (!completed) return 0;
    } else if (!should_stop) {
        return 1;
    }

    character->commited_animations_count++;

    // if exists a rollback animation use it
    if (action->rollback) {
        character->current_expected_duration = FLOAT_Inf;
        character->current_anim = action->rollback;
        character->current_anim_type = CharacterAnimType_ROLLBACK;
        character->current_use_frame_rollback = false;
        animsprite_restart(character->current_anim);
        return 1;
    }

    if (character->current_waiting_animation_end_and_idle) {
        // do not wait for next beat, play idle now
        character->current_action_type = CharacterActionType_NONE;
        return character_play_idle(character);
    }

    character->current_expected_duration = 0.0f;
    character->current_idle_in_next_beat = true;
    return 1;
}
