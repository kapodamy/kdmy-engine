#include "game/funkin/strum.h"

#include "arraypointerlist.h"
#include "beatwatcher.h"
#include "externals/luascript.h"
#include "game/common/funkin.h"
#include "game/funkin/note.h"
#include "imgutils.h"
#include "logger.h"
#include "malloc_utils.h"
#include "math2d.h"
#include "sprite.h"
#include "stringutils.h"
#include "unused_switch_case.h"


//
// it took over four months to create this, the most expensive part of the project
//

typedef enum {
    StrumMarkerState_INVALID = -1,
    StrumMarkerState_NOTHING,
    StrumMarkerState_CONFIRM,
    StrumMarkerState_PRESS
} StrumMarkerState;

typedef enum {
    StrumExtraState_NONE,
    StrumExtraState_WAIT,
    StrumExtraState_COMPLETED
} StrumExtraState;

typedef struct {
    AnimSprite action;
    AnimSprite undo;
} Action;

typedef struct {
    Action hit_up;
    Action hit_down;
    Action penality;
    Action miss;
    Action idle;
    Action continuous;
    StrumScriptOn last_event;
    StrumExtraState state;
} AttachedAnimations;

typedef struct {
    float64 timestamp;
    int32_t id;
    float64 duration;
    bool alt_anim;
    NoteState state;
    float64 hit_diff;
    float64 release_time;
    GamepadButtons release_button;
    int32_t previous_quarter;
    float64 custom_data;
    bool hit_on_penality;
} StrumNote;


struct Strum_s {
    int32_t strum_id;
    char* strum_name;
    float sick_effect_ratio;
    StateSprite sprite_marker_nothing;
    StateSprite sprite_marker_confirm;
    StateSprite sprite_marker_press;
    StateSprite sprite_sick_effect;
    StateSprite sprite_background;
    StrumNote* chart_notes;
    int32_t chart_notes_size;
    Note* drawable_notes;
    NoteAttribute** attribute_notes;
    int32_t* chart_notes_id_map;
    int32_t chart_notes_id_map_size;
    float64 scroll_velocity_base;
    float64 scroll_velocity;
    bool scroll_is_vertical;
    bool scroll_is_inverse;
    float64 scroll_window;
    float64 last_song_timestamp;
    int32_t notes_peek_index;
    int32_t notes_render_index;
    int32_t press_state_changes;
    StrumPressState press_state;
    bool press_state_use_alt_anim;
    StrumMarkerState marker_state;
    bool marker_state_changed;
    StrumMarkerState marker_sick_state;
    char* marker_sick_state_name;
    char* selected_sick_effect_state;
    float dimmen_length;
    float dimmen_opposite;
    float dimmen_marker;
    float marker_duration;
    float marker_duration_quarter;
    float minimum_sustain_duration;
    float marker_duration_multiplier;
    float64 key_test_limit;
    Modifier modifier;
    bool enable_sick_effect;
    bool enable_sick_effect_draw;
    bool enable_background;
    bool keep_aspect_ratio_background;
    bool markers_scale_keep;
    float markers_scale;
    char* selected_notes_state;
    AttachedAnimations animation_strum_line;
    AttachedAnimations animation_note;
    AttachedAnimations animation_marker;
    AttachedAnimations animation_sick_effect;
    AttachedAnimations animation_background;
    bool extra_animations_have_penalties;
    bool extra_animations_have_misses;
    float64 auto_scroll_elapsed;
    ArrayPointerList sustain_queue;
    Drawable drawable;
    float64 draw_offset_milliseconds;
    int32_t player_id;
    float inverse_offset;
    bool use_fukin_marker_duration;
    TweenKeyframe tweenkeyframe_note;
    bool use_beat_synced_idle_and_continous;
    BeatWatcher beatwatcher;
};


static StrumNote strumnote_init(ChartNote chart_note, int32_t* map, int32_t map_size) {
    int32_t id = 0;
    for (int32_t i = 0; i < map_size; i++) {
        if (map[i] == chart_note.direction) {
            id = i;
            break;
        }
    }

    StrumNote note;

    // base note info
    note.id = id;
    note.timestamp = chart_note.timestamp;
    note.duration = chart_note.duration;
    note.custom_data = chart_note.data;
    note.hit_on_penality = false;
    note.alt_anim = chart_note.alt_anim;

    note.state = NoteState_PENDING;
    note.hit_diff = DOUBLE_NaN;
    note.release_time = -1.0;

    note.release_button = 0x00;

    note.previous_quarter = 0;

    return note;
}

static inline bool strumnote_isSustain(StrumNote* note) { return note->duration > 0.0; }

static inline float64 strumnote_endTimestamp(StrumNote* note) { return note->timestamp + note->duration; }

static int strumnote_sort(const void* note1_ptr, const void* note2_ptr) {
    StrumNote* note1 = (StrumNote*)note1_ptr;
    StrumNote* note2 = (StrumNote*)note2_ptr;

    return math2d_double_comparer(note1->timestamp, note2->timestamp);
}

static AttachedAnimations attachedanimations_init() {
    return (AttachedAnimations){
        .hit_up = (Action){.action = NULL, .undo = NULL},
        .hit_down = (Action){.action = NULL, .undo = NULL},
        .penality = (Action){.action = NULL, .undo = NULL},
        .miss = (Action){.action = NULL, .undo = NULL},
        .idle = (Action){.action = NULL, .undo = NULL},
        .continuous = (Action){.action = NULL},
        .last_event = StrumScriptOn_ALL,
        .state = StrumExtraState_NONE
    };
}


static const char* STRUM_MARKER_SUFFIX_CONFIRM = "confirm";
static const char* STRUM_MARKER_SUFFIX_MARKER = "marker";
static const char* STRUM_MARKER_SUFFIX_HIT = "press";
static const char* STRUM_SICK_EFFECT_SUFFIX = "splash";
static const char* STRUM_BACKGROUND_SUFFIX = "background";


/** Default sprite size of the sick effect in relation to the marker sprite size */
static const float STRUM_DEFAULT_SICK_EFFECT_RATIO = 2.0f;

/** Number of notes to draw after the marker */
static const float STRUM_DRAW_PAST_NOTES = 1.1f;

/** Marker used in auto-scroll */
static const StrumMarkerState STRUM_AUTO_SCROLL_MARKER_STATE = StrumMarkerState_CONFIRM;

/** Increases the marker duration 5% */
static const float STRUM_ADDITIONAL_MARKER_DURATION_RATIO = 1.05f;


static void strum_internal_calc_scroll_window(Strum strum);
static bool strum_internal_on_note_bounds(Strum strum, int32_t note_index, float64 test_timestamp);
static float64 strum_internal_get_note_duration(Strum strum, int32_t note_index);
static void strum_internal_calc_state_dimmen(StateSpriteState* state, bool scroll_is_vertical, float dimmen, float invdimmen);
static StateSpriteState* strum_internal_state_add(StateSprite statesprite, ModelHolder modelholder, const char* strum_name, const char* target, const char* state_name);
static void strum_internal_calc_states(Strum strum, StateSprite statesprite, void (*calc_callback)(Strum, StateSpriteState*));
static void strum_internal_calc_state_marker(Strum strum, StateSpriteState* state);
static void strum_internal_calc_state_sick_effect(Strum strum, StateSpriteState* state);
static void strum_internal_calc_state_background(Strum strum, StateSpriteState* state);
static void strum_internal_states_recalculate(Strum strum);
static void strum_internal_extra_destroy_animation(AttachedAnimations* holder);
static int32_t strum_internal_extra_animate(Strum strum, StrumScriptTarget target, StrumScriptOn event, bool undo, float elapsed);
static void strum_internal_extra_animate_sprite(Strum strum, StrumScriptTarget target, AnimSprite animsprite);
static AttachedAnimations* strum_internal_extra_get_holder(Strum strum, StrumScriptTarget target);
static Action* strum_internal_extra_get_subholder(AttachedAnimations* holder, StrumScriptOn event);
static void strum_internal_check_sustain_queue(Strum strum, float64 song_timestamp, PlayerStats playerstats);
static void strum_internal_load_sick_effect_state(Strum strum, ModelHolder modelholder, const char* state_name);
static void strum_internal_set_note_drawables(Strum strum, NotePool notepool);
static void strum_internal_extra_batch(AttachedAnimations* holder, void (*callback)(AnimSprite));
static void strum_internal_update_press_state(Strum strum, StrumPressState press_state);
static void strum_internal_reset_scrolling(Strum strum);
static void strum_internal_calc_marker_duration(Strum strum, float64 velocity);
static void strum_internal_restart_extra_continous(Strum strum);
static void strum_internal_notify_note_hit(Strum strum, WeekScript script, int32_t strum_note_index, PlayerStats playerstats);
static void strum_internal_notify_note_loss(Strum strum, WeekScript script, int32_t strum_note_idx, bool is_penalty);


Strum strum_init(int32_t id, const char* name, float x, float y, float marker_dimmen, float invdimmen, float length_dimmen, bool keep_markers_scale) {
    Strum strum = malloc_chk(sizeof(struct Strum_s));
    malloc_assert(strum, Strum);

    *strum = (struct Strum_s){
        .strum_id = id,
        .strum_name = string_duplicate(name),

        .sick_effect_ratio = STRUM_DEFAULT_SICK_EFFECT_RATIO,

        .sprite_marker_nothing = statesprite_init_from_texture(NULL),
        .sprite_marker_confirm = statesprite_init_from_texture(NULL),
        .sprite_marker_press = statesprite_init_from_texture(NULL),
        .sprite_sick_effect = statesprite_init_from_texture(NULL),
        .sprite_background = statesprite_init_from_texture(NULL),

        .chart_notes = NULL,
        .chart_notes_size = 0,

        .drawable_notes = NULL,
        .attribute_notes = NULL,
        .chart_notes_id_map = NULL,
        .chart_notes_id_map_size = 0,

        .scroll_velocity_base = FUNKIN_CHART_SCROLL_VELOCITY,
        .scroll_velocity = 0.0,

        .scroll_is_vertical = true,
        .scroll_is_inverse = false,
        .scroll_window = 0.0,

        .last_song_timestamp = 0.0,
        .notes_peek_index = 0,
        .notes_render_index = 0,

        .press_state_changes = -1,
        .press_state = StrumPressState_NONE,
        .press_state_use_alt_anim = false,

        .marker_state = StrumMarkerState_NOTHING,
        .marker_state_changed = false,
        .marker_sick_state = StrumMarkerState_NOTHING,

        .marker_sick_state_name = NULL,
        .selected_sick_effect_state = NULL,

        .dimmen_length = length_dimmen,
        .dimmen_opposite = invdimmen,

        .dimmen_marker = marker_dimmen,
        .marker_duration = 0.0f,
        .marker_duration_quarter = 0.0f,
        .minimum_sustain_duration = 0.0f,
        .marker_duration_multiplier = 1.0f,

        .key_test_limit = 0.0,

        //.modifier = (Modifier){},

        .enable_sick_effect = true,
        .enable_sick_effect_draw = true,
        .enable_background = false,

        .keep_aspect_ratio_background = true,

        .markers_scale_keep = keep_markers_scale,
        .markers_scale = -1.0f,

        .selected_notes_state = NULL,

        .animation_strum_line = attachedanimations_init(),
        .animation_note = attachedanimations_init(),
        .animation_marker = attachedanimations_init(),
        .animation_sick_effect = attachedanimations_init(),
        .animation_background = attachedanimations_init(),

        .extra_animations_have_penalties = false,
        .extra_animations_have_misses = false,

        .auto_scroll_elapsed = -1.0,
        .sustain_queue = arraypointerlist_init(),

        .drawable = drawable_init(-1, NULL, NULL, NULL),
        .draw_offset_milliseconds = 0.0,
        .player_id = -1,
        .inverse_offset = length_dimmen - marker_dimmen,
        .use_fukin_marker_duration = true,
        .tweenkeyframe_note = NULL,
        .use_beat_synced_idle_and_continous = true,
        //.beatwatcher = (BeatWatcher){}
    };

    // set sprites location and modifier location
    strum_update_draw_location(strum, x, y);

    // set sprites size
    statesprite_set_draw_size(strum->sprite_marker_nothing, marker_dimmen, marker_dimmen);
    statesprite_set_draw_size(strum->sprite_marker_confirm, marker_dimmen, marker_dimmen);
    statesprite_set_draw_size(strum->sprite_marker_press, marker_dimmen, marker_dimmen);
    statesprite_set_draw_size(strum->sprite_sick_effect, marker_dimmen, marker_dimmen);
    statesprite_set_draw_size(strum->sprite_background, marker_dimmen, length_dimmen);


    strum_set_scroll_speed(strum, 1.0);
    strum_set_scroll_direction(strum, ScrollDirection_UPSCROLL);
    strum_set_bpm(strum, 100.0f);

    return strum;
}

void strum_destroy(Strum* strum_ptr) {
    if (!strum_ptr || !*strum_ptr) return;

    Strum strum = *strum_ptr;

    luascript_drop_shared(strum);

    statesprite_destroy(&strum->sprite_marker_nothing);
    statesprite_destroy(&strum->sprite_marker_confirm);
    statesprite_destroy(&strum->sprite_marker_press);
    statesprite_destroy(&strum->sprite_sick_effect);
    statesprite_destroy(&strum->sprite_background);

    free_chk(strum->strum_name);

    if (strum->chart_notes) free_chk(strum->chart_notes);
    if (strum->drawable_notes) free_chk(strum->drawable_notes);
    if (strum->attribute_notes) free_chk(strum->attribute_notes);
    if (strum->chart_notes_id_map) free_chk(strum->chart_notes_id_map);

    if (strum->selected_notes_state) free_chk(strum->selected_notes_state);
    if (strum->selected_sick_effect_state) free_chk(strum->selected_sick_effect_state);

    strum_internal_extra_destroy_animation(&strum->animation_marker);
    strum_internal_extra_destroy_animation(&strum->animation_sick_effect);
    strum_internal_extra_destroy_animation(&strum->animation_background);
    strum_internal_extra_destroy_animation(&strum->animation_strum_line);
    strum_internal_extra_destroy_animation(&strum->animation_note);

    if (strum->tweenkeyframe_note) tweenkeyframe_destroy(&strum->tweenkeyframe_note);

    arraypointerlist_destroy(&strum->sustain_queue);

    // Note: do not release "strum->marker_sick_state_name"

    drawable_destroy(&strum->drawable);

    free_chk(strum);
    *strum_ptr = NULL;
}

int32_t strum_set_notes(Strum strum, Chart chart, DistributionStrum* strumsdefs, int32_t strumsdefs_size, int32_t player_id, NotePool notepool) {
    if (strum->chart_notes) {
        // free previous used resources
        free_chk(strum->chart_notes_id_map);
        free_chk(strum->chart_notes);
        strum->chart_notes_size = 0;
        strum->key_test_limit = -DOUBLE_Inf;
    }

    strum_internal_reset_scrolling(strum); // obligatory

    // validate ids
    int32_t strum_id = strum->strum_id;
    if (strum_id < 0 || strum_id >= strumsdefs_size) return -1;
    if (player_id < 0 || player_id >= chart->entries_size) return 0;

    int32_t* notes_ids = strumsdefs[strum_id].notes_ids;
    int32_t note_ids_size = strumsdefs[strum_id].notes_ids_size;

    ChartNote* player_notes = chart->entries[player_id].notes;
    int32_t player_notes_size = chart->entries[player_id].notes_size;

    // step 1: count the notes required for this strum
    int32_t count = 0;
    for (int32_t i = 0; i < player_notes_size; i++) {
        int32_t note_direction = (int32_t)player_notes[i].direction;
        for (int32_t j = 0; j < note_ids_size; j++) {
            if (note_direction == notes_ids[j]) {
                count++;
                break;
            }
        }
    }

    if (count < 1) {
        strum->chart_notes = NULL;
        strum->chart_notes_id_map = NULL;
        strum->chart_notes_id_map_size = 0;
        strum->key_test_limit = -DOUBLE_Inf;

        if (strum->strum_name != NULL)
            strum_internal_set_note_drawables(strum, notepool);

        return 0;
    }

    // step 2: map all note IDs
    strum->chart_notes_id_map = malloc_for_array(int32_t, note_ids_size);
    strum->chart_notes_id_map_size = note_ids_size;

    for (int32_t i = 0; i < note_ids_size; i++)
        strum->chart_notes_id_map[i] = notes_ids[i];

    // step 3: grab notes from the chart
    strum->chart_notes = malloc_for_array(StrumNote, count);
    strum->chart_notes_size = count;

    int32_t k = 0;
    for (int32_t i = 0; i < player_notes_size; i++) {
        bool ignore = true;
        for (int32_t j = 0; j < note_ids_size; j++) {
            if (player_notes[i].direction == notes_ids[j]) {
                ignore = false;
                break;
            }
        }
        if (ignore) continue;

        strum->chart_notes[k++] = strumnote_init(player_notes[i], notes_ids, note_ids_size);
    }

    // Important: sort the notes by timestamp
    qsort(strum->chart_notes, (size_t)strum->chart_notes_size, sizeof(StrumNote), strumnote_sort);

    // calculate the key test time limit
    strum->key_test_limit = math2d_max_double(strum->chart_notes[0].timestamp - strum->marker_duration, 0.0);

    // remove duplicated notes (filtered by timestamp and id)
    int32_t index = 0;
    float64 last_timestamp = DOUBLE_NaN;
    int32_t last_id = -1;

    for (int32_t i = 0; i < strum->chart_notes_size; i++) {
        int32_t id = strum->chart_notes[i].id;
        float64 timestamp = strum->chart_notes[i].timestamp;
        if (timestamp == last_timestamp && id == last_id) {
            logger_error("strum_set_notes() duplicated note found: ts=" FMT_FLT64 " id=" FMT_I4, timestamp, id);
        } else {
            last_timestamp = timestamp;
            last_id = id;
            strum->chart_notes[index++] = strum->chart_notes[i]; // clone struct
        }
    }

    if (index != strum->chart_notes_size) {
        // trim array
        strum->chart_notes_size = index;
        strum->chart_notes = realloc_for_array(strum->chart_notes, strum->chart_notes_size, StrumNote);
    }

    if (strum->strum_name != NULL)
        strum_internal_set_note_drawables(strum, notepool);

    return count;
}

void strum_force_rebuild(Strum strum, float marker_dimmen, float invdimmen, float length_dimmen, bool keep_markers_scale) {
    //
    // Note: the caller MUST change later the scroll direction in order to take effect
    //
    logger_warn("strum_force_rebuild() was called, this never should happen");

    strum->dimmen_length = length_dimmen;
    strum->dimmen_opposite = invdimmen;
    strum->dimmen_marker = marker_dimmen;
    strum->markers_scale_keep = keep_markers_scale;

    // set sprites size
    statesprite_set_draw_size(strum->sprite_marker_nothing, marker_dimmen, marker_dimmen);
    statesprite_set_draw_size(strum->sprite_marker_confirm, marker_dimmen, marker_dimmen);
    statesprite_set_draw_size(strum->sprite_marker_press, marker_dimmen, marker_dimmen);
    statesprite_set_draw_size(strum->sprite_sick_effect, marker_dimmen, marker_dimmen);
    statesprite_set_draw_size(strum->sprite_background, marker_dimmen, length_dimmen);
}

void strum_update_draw_location(Strum strum, float x, float y) {
    statesprite_set_draw_location(strum->sprite_marker_nothing, x, y);
    statesprite_set_draw_location(strum->sprite_marker_confirm, x, y);
    statesprite_set_draw_location(strum->sprite_marker_press, x, y);
    statesprite_set_draw_location(strum->sprite_sick_effect, x, y);
    statesprite_set_draw_location(strum->sprite_background, x, y);

    strum->modifier.x = x;
    strum->modifier.y = y;
}

void strum_set_scroll_speed(Strum strum, float64 speed) {
    strum->scroll_velocity = strum->scroll_velocity_base * speed;

    // Use half of the scolling speed if the screen aspect ratio is 4:3 (dreamcast)
    if (!pvr_context_is_widescreen()) strum->scroll_velocity *= 0.5; // 480.0 / 720.0;

    strum_internal_calc_marker_duration(strum, strum->scroll_velocity);
    strum_internal_calc_scroll_window(strum);
}

void strum_set_scroll_direction(Strum strum, ScrollDirection direction) {
    strum->scroll_is_vertical = direction == ScrollDirection_UPSCROLL || direction == ScrollDirection_DOWNSCROLL;
    strum->scroll_is_inverse = direction == ScrollDirection_DOWNSCROLL || direction == ScrollDirection_RIGHTSCROLL;

    if (strum->scroll_is_vertical) {
        strum->modifier.width = strum->dimmen_marker;
        strum->modifier.height = strum->dimmen_length;
    } else {
        strum->modifier.width = strum->dimmen_length;
        strum->modifier.height = strum->dimmen_marker;
    }

    strum_internal_calc_scroll_window(strum);
    strum_internal_states_recalculate(strum);
}

void strum_set_marker_duration_multiplier(Strum strum, float multipler) {
    strum->marker_duration_multiplier = multipler > 0.0f ? multipler : 1.0f;

    float64 velocity = strum->scroll_velocity;

    strum_internal_calc_marker_duration(strum, velocity);
}

void strum_reset(Strum strum, float64 scroll_speed, const char* state_name) {
    strum_internal_reset_scrolling(strum);

    drawable_set_alpha(strum->drawable, 1.0f);
    drawable_set_offsetcolor_to_default(strum->drawable);

    for (int32_t i = 0; i < strum->chart_notes_size; i++) {
        strum->chart_notes[i].state = NoteState_PENDING;
        strum->chart_notes[i].hit_diff = DOUBLE_NaN;
        strum->chart_notes[i].release_time = -1.0;
        strum->chart_notes[i].release_button = 0x00;
        strum->chart_notes[i].previous_quarter = 0;
        strum->chart_notes[i].hit_on_penality = false;
    }

    strum_set_scroll_speed(strum, scroll_speed);
    strum_state_toggle(strum, state_name);
}

int32_t strum_scroll(Strum strum, float64 song_timestamp, DDRKeysFIFO* ddrkeys_fifo, PlayerStats playerstats, WeekScript weekscript) {
    StrumNote* chart_notes = strum->chart_notes;
    const int32_t chart_notes_size = strum->chart_notes_size;
    const float marker_duration = strum->marker_duration;

    StrumMarkerState marker_state = StrumMarkerState_INVALID;
    int32_t keys_processed = 0;
    int32_t notes_peek_index = strum->notes_peek_index;
    float64 song_offset_timestamp = song_timestamp + marker_duration;
    StrumPressState press_state = StrumPressState_INVALID;
    bool press_state_use_alt_anim = false;
    int32_t notes_ahead = 0;
    int32_t notes_cleared = 0;


    if (strum->auto_scroll_elapsed > 0.0) {
        // cancel strum_scroll_auto() state keep
        strum->auto_scroll_elapsed = -1.0;
        strum->marker_state = StrumMarkerState_NOTHING;
        strum->marker_state_changed = true;
    }

    // step 1: enter in "key testing" mode on the early seconds of the song or at the end
    if (song_timestamp < strum->key_test_limit || notes_peek_index >= chart_notes_size) {
        for (int32_t i = 0; i < ddrkeys_fifo->available; i++) {
            DDRKey* ddr_key = &ddrkeys_fifo->queue[i];
            if (ddr_key->discard || ddr_key->strum_id != strum->strum_id) continue;
            if (ddr_key->in_song_timestamp > song_timestamp) continue;

            keys_processed++;
            ddr_key->discard = true;
            marker_state = ddr_key->holding ? StrumMarkerState_CONFIRM : StrumMarkerState_NOTHING;
        }

        if (keys_processed > 0) strum_internal_update_press_state(strum, StrumPressState_NONE);

        strum_internal_check_sustain_queue(strum, song_timestamp, playerstats);
        strum->last_song_timestamp = song_timestamp;
        if (marker_state != StrumMarkerState_INVALID) {
            strum->marker_state = marker_state;
            strum->marker_state_changed = true;
        }
        return keys_processed;
    }

    // step 2: count all notes near the song position (under normal conditions is always 1)
    for (int32_t i = notes_peek_index; i < chart_notes_size; i++) {
        if (chart_notes[i].timestamp > song_offset_timestamp) break;
        notes_ahead++;
    }

    // step 3: do key checking (very extensive)
    for (int32_t i = 0; i < ddrkeys_fifo->available && notes_cleared < notes_ahead; i++) {
        NoteAttribute* note_attributes;
        DDRKey* ddr_key = &ddrkeys_fifo->queue[i];
        const float64 key_timestamp = ddr_key->in_song_timestamp;
        if (ddr_key->discard || ddr_key->strum_id != strum->strum_id) continue;

        // phase 1: check key event against every note ahead
        float64 lowest_diff = DOUBLE_Inf;
        StrumNote* note = NULL;
        int32_t hit_index = -1;
        int32_t index_ahead = 0;


        if (ddr_key->holding) {

            // find a candidate for this key event
            for (int32_t j = notes_peek_index; j < chart_notes_size; j++) {
                if (index_ahead++ > notes_ahead) break;

                // check if is trying to recover a sustain note
                if (chart_notes[j].state == NoteState_RELEASE) {
                    float64 start = chart_notes[j].timestamp;
                    float64 end = start + chart_notes[j].duration;
                    if (key_timestamp < start || key_timestamp > end) continue;

                    // the timestamp is within the bounds of the note
                    note = &chart_notes[j];
                    hit_index = j;

                    // ignore recover if the next note is closer
                    j++;
                    if (j < chart_notes_size && chart_notes[j].state == NoteState_PENDING) {
                        start = chart_notes[j].timestamp - (marker_duration * 0.75f);
                        if (start <= key_timestamp) {
                            note = &chart_notes[j];
                            hit_index = j;
                        }
                    }

                    break;
                }

                // check if the note is on pending state and near the key timestamp
                if (chart_notes[j].state != NoteState_PENDING) continue;
                if (!strum_internal_on_note_bounds(strum, j, key_timestamp)) continue;

                // check how far are from the key, if near, remember the note
                float64 diff = float64_fabs(chart_notes[j].timestamp - key_timestamp);
                if (diff < lowest_diff) {
                    lowest_diff = diff;
                    note = &chart_notes[j];
                    hit_index = j;
                }
            }

            if (!note) {
                if (ddr_key->in_song_timestamp > song_timestamp) {
                    // logger_warn("strum: [hold] in future ts_k=" FMT_FLT64 " ts_s=" FMT_FLT64, key_timestamp, song_timestamp);
                }

                // maybe the key event is a penalty hit/press or future event (do not discard)
                continue;
            }

            if (note->state == NoteState_RELEASE) {
                // logger_info("strum: [hold] sustain recover!  ts_k=" FMT_FLT64 " ts_n=" FMT_FLT64, key_timestamp, note->timestamp);

                if (math2d_is_double_NaN(note->hit_diff)) {
                    // never pressed compute hit
                    note_attributes = strum->attribute_notes[note->id];
                    note->hit_diff = note->timestamp - key_timestamp;
                    if (!note_attributes->ignore_hit) {
                        playerstats_add_sustain_delayed_hit(playerstats, note_attributes->heal_ratio, note->hit_diff);
                    }
                    if (note_attributes->can_kill_on_hit) playerstats_kill_if_negative_health(playerstats);
                }

                // recover the sustain note
                note->release_time = -1.0;
                note->state = NoteState_HOLD;
                note->release_button = ddr_key->button;

                press_state = StrumPressState_HIT;
                press_state_use_alt_anim = note->alt_anim;
                goto L_discard_key_event;
            }

            // process pending note
            note_attributes = strum->attribute_notes[note->id];
            bool sick_effect_ready = strum->marker_sick_state == StrumMarkerState_NOTHING;
            lowest_diff = note->timestamp - key_timestamp; // hit difference (for stats)

            // calculate rank
            Ranking rank = Ranking_NONE;
            if (!note_attributes->ignore_hit) {
                rank = playerstats_add_hit(
                    playerstats, note_attributes->heal_ratio, marker_duration, lowest_diff
                );
            }

            if (note_attributes->can_kill_on_hit) playerstats_kill_if_negative_health(playerstats);
            if (weekscript) strum_internal_notify_note_hit(strum, weekscript, hit_index, playerstats);

            // logger_info("strum: [hold] note hit!  ts=" FMT_FLT64 " diff=" FMT_FLT64 " rank=0x%x", key_timestamp, lowest_diff, (int)rank);

            // check if necessary display the sick effect
            if (rank == Ranking_SICK && sick_effect_ready) {
                strum->marker_sick_state = StrumMarkerState_PRESS;

                // if the note has a custom sick effect choose from the state list
                if (note_attributes->custom_sick_effect_name)
                    strum->marker_sick_state_name = note_attributes->custom_sick_effect_name;
                else
                    strum->marker_sick_state_name = strum->selected_sick_effect_state;
            }

            if (note->duration >= strum->minimum_sustain_duration) {
                // add note to the sustain queue to check the health loss/gain later
                arraypointerlist_add(strum->sustain_queue, note);
                press_state = StrumPressState_HIT_SUSTAIN;
            } else {
                press_state = StrumPressState_HIT;
            }

            press_state_use_alt_anim = note->alt_anim;
            note->state = NoteState_HOLD;
            note->release_button = ddr_key->button;
            note->release_time = strumnote_endTimestamp(note); // use end timestamp to avoid compute as miss
            note->hit_diff = lowest_diff;
        } else {
            //
            // find what note is being released. Note: this method will fail
            // if there multiple key inputs which under normal circumstances
            // never should happen
            //
            for (int32_t j = notes_peek_index; j < chart_notes_size; j++) {
                if (chart_notes[j].state != NoteState_HOLD) continue; // ignore non-hold notes
                if (chart_notes[j].release_button != ddr_key->button) continue;
                if (index_ahead++ > notes_ahead) break;

                note = &chart_notes[j];
                // logger_log("strum: [release] note found!  ts_k=" FMT_FLT64 " ts_n=" FMT_FLT64, key_timestamp, note->timestamp);
                break;
            }

            press_state = StrumPressState_NONE;

            if (!note) {
                if (key_timestamp <= song_timestamp) {
                    // logger_log("strum: [release] empty strum at " FMT_FLT64, key_timestamp);
                }

                //
                // The key was pressed for too long or the penality hit/press was released.
                // Note: strum_internal_check_sustain_queue() function handles sustain notes
                // pressed longer than required.
                //
                goto L_discard_key_event;
            }

            note->release_button = 0x00;

            // clear if the note is not sustain
            if (note->duration < strum->minimum_sustain_duration) {
                // logger_info("strum: [release] clear non-sustain ts=" FMT_FLT64, note->timestamp);
                note->state = NoteState_CLEAR;
                note->release_time = DOUBLE_Inf;
                notes_cleared++;

                goto L_discard_key_event;
            }

            // check if the sustain note was released early
            float64 end_timestamp = strumnote_endTimestamp(note);

            if (key_timestamp < end_timestamp) {
                // logger_log("strum: [release] early! left duration " FMT_FLT64, end_timestamp - key_timestamp);

                // early release
                note->state = NoteState_RELEASE;
                note->release_time = key_timestamp;
            } else {
                // logger_log("strum: [release] sustain clear! remain was " FMT_FLT64, end_timestamp - note.release_time);
                arraypointerlist_remove(strum->sustain_queue, note);
                note->state = NoteState_CLEAR;
                note->release_time = DOUBLE_Inf;
                notes_cleared++;
            }
        }

        // phase 2: discard the key event
    L_discard_key_event:
        ddr_key->discard = true;
        keys_processed++;

        // phase 3: update the marker state (press/confirm/nothing)
        if (ddr_key->holding)
            marker_state = note ? StrumMarkerState_PRESS : StrumMarkerState_CONFIRM;
        else
            marker_state = StrumMarkerState_NOTHING;
    }

    // step 4: check for missed notes or early released sustains
    song_offset_timestamp = song_timestamp - marker_duration;
    for (int32_t i = notes_peek_index, j = 0; i < chart_notes_size && j < notes_ahead; i++, j++) {
        StrumNote* note = &chart_notes[i];
        bool compute_miss = !strum->attribute_notes[note->id]->ignore_miss;
        float64 end_timestamp = note->timestamp + math2d_max_double(note->duration, marker_duration);

        switch (note->state) {
            case NoteState_CLEAR:
            case NoteState_HOLD:
            case NoteState_MISS:
                continue;
                CASE_UNUSED(NoteState_PENDING);
                CASE_UNUSED(NoteState_RELEASE);
        }

        // check if the note is non-sustain
        if (note->duration < strum->minimum_sustain_duration) {
            if (song_offset_timestamp >= end_timestamp) {
                // logger_info("strum: [missed] non-sustain note ts=" FMT_FLT64, note->timestamp);
                note->state = NoteState_MISS;
                note->release_time = -DOUBLE_Inf;

                if (compute_miss) {
                    playerstats_add_miss(playerstats, strum->attribute_notes[note->id]->hurt_ratio);
                    if (weekscript) strum_internal_notify_note_loss(strum, weekscript, i, false);
                    strum->extra_animations_have_misses = true;
                    press_state = StrumPressState_MISS;
                }
            }
            continue;
        }


        //
        // On sustain notes check how many note-quarters are lost.
        // The strum_internal_check_sustain_queue() function calculates it is loosing.
        //

        if (compute_miss) {
            if (note->state == NoteState_PENDING && weekscript)
                strum_internal_notify_note_loss(strum, weekscript, i, false);
            if (note->state != NoteState_RELEASE)
                strum->extra_animations_have_misses = true;
        }

        if (song_timestamp > end_timestamp) {
            // logger_info("strum: [missed] sustain note ts=" FMT_FLT64 " ts_end=" FMT_FLT64, note->timestamp, end_timestamp);
            arraypointerlist_remove(strum->sustain_queue, note);

            note->state = NoteState_MISS;
            note->release_time = -DOUBLE_Inf;
            note->release_button = 0x00;
        }

        // consider as missed if the worst possible ranking can not be assigned
        float64 miss_timestamp = note->timestamp + marker_duration;
        if (song_timestamp >= miss_timestamp) {
            // logger_info("strum: [miss] sustain loosing ts_n=" FMT_FLT64 " ts_s=" FMT_FLT64, note->timestamp, song_timestamp);
            if (note->state == NoteState_PENDING) {
                arraypointerlist_add(strum->sustain_queue, note);
                note->state = NoteState_RELEASE;
            }
        }
    }

    // step 5: move notes_peek_index
    for (int32_t i = 0; i < notes_ahead && notes_peek_index < chart_notes_size; i++) {
        NoteState note_state = chart_notes[notes_peek_index].state;
        if (note_state != NoteState_CLEAR && note_state != NoteState_MISS) break;
        notes_peek_index++;
    }

    // step 6: check for released keys of pentalties presses
    for (int32_t i = 0; i < ddrkeys_fifo->available; i++) {
        DDRKey* ddr_key = &ddrkeys_fifo->queue[i];
        const float64 key_timestamp = ddr_key->in_song_timestamp;

        if (ddr_key->discard || ddr_key->holding || ddr_key->strum_id != strum->strum_id) continue;
        if (key_timestamp > song_timestamp) continue;

        if (marker_state == StrumMarkerState_INVALID) marker_state = StrumMarkerState_NOTHING;
        if (press_state == StrumPressState_INVALID) press_state = StrumPressState_NONE;

        ddr_key->discard = true;
        keys_processed++;
    }

    // before leave, check the sustain queue
    strum_internal_check_sustain_queue(strum, song_timestamp, playerstats);

    strum->last_song_timestamp = song_timestamp;
    strum->notes_peek_index = notes_peek_index;
    if (marker_state != StrumMarkerState_INVALID) {
        strum->marker_state = marker_state;
        strum->marker_state_changed = true;
    }
    strum_internal_update_press_state(strum, press_state);

    if (press_state == StrumPressState_HIT || press_state == StrumPressState_HIT_SUSTAIN) {
        strum->press_state_use_alt_anim = press_state_use_alt_anim;
    }

    return keys_processed;
}

void strum_scroll_auto(Strum strum, float64 song_timestamp, PlayerStats playerstats, WeekScript weekscript) {
    if (strum->chart_notes_size < 1) return;

    StrumNote* chart_notes = strum->chart_notes;
    NoteAttribute** attribute_notes = strum->attribute_notes;
    const int32_t chart_notes_size = strum->chart_notes_size;
    const float marker_duration = strum->marker_duration;
    const float press_time = strum->marker_duration * 2.0f;

    int32_t notes_peek_index = strum->notes_peek_index;
    int32_t notes_cleared = 0;

    StrumPressState press_state = StrumPressState_INVALID;
    bool press_state_use_alt_anim = false;

    // clear all notes ahead, but keep in hold the sustain ones
    for (; notes_peek_index < chart_notes_size; notes_peek_index++) {
        StrumNote* note = &chart_notes[notes_peek_index];
        bool is_sustain = note->duration >= strum->minimum_sustain_duration;
        float64 end_timestamp = strumnote_endTimestamp(note);

        if (note->timestamp > song_timestamp) break;

        if (note->state == NoteState_PENDING || note->state == NoteState_RELEASE) {

            if (note->state == NoteState_PENDING) {
                if (attribute_notes[note->id]->can_kill_on_hit) {
                    if (!attribute_notes[note->id]->ignore_miss)
                        playerstats_add_miss(playerstats, attribute_notes[note->id]->hurt_ratio);
                } else if (!attribute_notes[note->id]->ignore_hit) {
                    float heal_ratio = attribute_notes[note->id]->heal_ratio;
                    note->hit_diff = 0.0;
                    playerstats_add_hit(playerstats, heal_ratio, marker_duration, note->hit_diff);

                    // keep the marker in press state for a while
                    strum->auto_scroll_elapsed = note->duration < 1.0 ? press_time : note->duration;
                    strum->marker_state = STRUM_AUTO_SCROLL_MARKER_STATE;
                    strum->marker_state_changed = true;
                    press_state = is_sustain ? StrumPressState_HIT_SUSTAIN : StrumPressState_HIT;
                    press_state_use_alt_anim = note->alt_anim;
                }

                if (weekscript) {
                    if (attribute_notes[note->id]->can_kill_on_hit)
                        strum_internal_notify_note_loss(strum, weekscript, notes_peek_index, false);
                    else
                        strum_internal_notify_note_hit(strum, weekscript, notes_peek_index, playerstats);
                }

                if (is_sustain)
                    arraypointerlist_add(strum->sustain_queue, note);
                else
                    notes_cleared++;
            }

            note->release_button = 0x00;

            if (attribute_notes[note->id]->can_kill_on_hit) {
                note->state = is_sustain ? NoteState_RELEASE : NoteState_MISS;
            } else {
                note->state = is_sustain ? NoteState_HOLD : NoteState_CLEAR;
                note->release_time = is_sustain ? end_timestamp : DOUBLE_Inf;
            }
        } else if (note->state == NoteState_HOLD) {
            if (song_timestamp >= end_timestamp) {
                note->state = NoteState_CLEAR;
                note->release_time = DOUBLE_Inf;
                note->release_button = 0x00;
                notes_cleared++;

                press_state = StrumPressState_NONE;
                if (is_sustain) arraypointerlist_remove(strum->sustain_queue, note);
            }
        } else if (note->state == NoteState_RELEASE && song_timestamp >= end_timestamp) {
            note->state = NoteState_MISS;
            note->release_time = -DOUBLE_Inf;
            note->release_button = 0x00;
            notes_cleared++;

            press_state = StrumPressState_NONE;
            if (is_sustain) arraypointerlist_remove(strum->sustain_queue, note);
        }
    }

    strum_internal_update_press_state(strum, press_state);
    strum_internal_check_sustain_queue(strum, song_timestamp, playerstats);
    strum->last_song_timestamp = song_timestamp;

    if (press_state == StrumPressState_HIT || press_state == StrumPressState_HIT_SUSTAIN) {
        strum->press_state_use_alt_anim = press_state_use_alt_anim;
    }

    int32_t count = notes_peek_index - strum->notes_peek_index;
    if (notes_cleared < count) return;

    strum->notes_peek_index = notes_peek_index;
}

void strum_force_key_release(Strum strum) {
    strum->marker_state = StrumMarkerState_NOTHING;
    strum->marker_state_changed = true;
    strum_internal_update_press_state(strum, StrumPressState_NONE);

    for (int32_t i = strum->notes_peek_index; i < strum->chart_notes_size; i++) {
        StrumNote* note = &strum->chart_notes[i];
        if (note->state == NoteState_HOLD) {
            note->state = NoteState_RELEASE;
            note->release_time = math2d_max_double(strum->last_song_timestamp, note->timestamp);
        }
    }

    strum_animate(strum, 0.0f);
}

int32_t strum_find_penalties_note_hit(Strum strum, float64 song_timestamp, DDRKeysFIFO* ddrkeys_fifo, PlayerStats playerstats, WeekScript weekscript) {
    if (ddrkeys_fifo->available < 1 || strum->chart_notes_size < 1) return 0;

    StrumNote* chart_notes = strum->chart_notes;
    const int32_t chart_notes_size = strum->chart_notes_size;
    NoteAttribute** attribute_notes = strum->attribute_notes;

    int32_t keys_processed = 0;

    //
    // How this function works:
    //
    //  * Pick key events from neighbor strums and check if hits notes on this strum. To accomplish
    //    this strum_scroll() function must be called before doing this.
    //
    //  * Ignore key events of "invisible" strums which are used in mechanics like "press space"
    //
    //  * Do not highlight the strum (STRUM_MARKER_*) for foreign key events
    //
    //  * Only pending notes are checked, sustain notes and early released sustains are ignored.
    //
    //  * ignore notes with less of 50% of accuracy
    //

    for (int32_t i = math2d_min_int(strum->notes_peek_index, chart_notes_size - 1); i < chart_notes_size; i++) {
        StrumNote* note = &chart_notes[i];
        if (note->state != NoteState_PENDING) continue;
        if (note->duration >= strum->minimum_sustain_duration) continue;

        for (int32_t j = 0; j < ddrkeys_fifo->available; j++) {
            DDRKey* ddr_key = &ddrkeys_fifo->queue[j];
            float64 key_timestamp = ddr_key->in_song_timestamp;

            if (ddr_key->discard || ddr_key->strum_invisible || !ddr_key->holding) continue;
            if (ddr_key->strum_id == strum->strum_id) continue;
            if (ddr_key->in_song_timestamp > song_timestamp) continue;

            if (strum_internal_on_note_bounds(strum, i, key_timestamp)) {
                float64 diff = float64_fabs(strum->chart_notes[i].timestamp - key_timestamp);
                diff /= strum->marker_duration;

                // ignore if the accuracy less than 50%
                if (diff < 0.50) continue;

                // logger_log("strum: [penality] hit on pending note ts=" FMT_FLT64, note->timestamp);
                strum->extra_animations_have_penalties = true;
                strum_internal_update_press_state(strum, StrumPressState_PENALTY_HIT);

                if (!attribute_notes[note->id]->ignore_miss) {
                    note->state = NoteState_MISS;
                    note->hit_on_penality = true;
                    note->release_button = 0x00;
                    note->release_time = -DOUBLE_Inf;
                    playerstats_add_penality(playerstats, false);
                }
                if (weekscript) strum_internal_notify_note_loss(strum, weekscript, i, true);
            } else {
                // ignore key event
                continue;
            }

            ddr_key->discard = true;
            keys_processed++;
        }
    }

    return keys_processed;
}

int32_t strum_find_penalties_empty_hit(Strum strum, float64 song_timestamp, DDRKeysFIFO* ddrkeys_fifo, PlayerStats playerstats) {
    if (ddrkeys_fifo->available < 1 || strum->chart_notes_size < 1) return 0;

    int32_t keys_processed = 0;

    //
    // How this function works:
    //
    //  * Check if the player is pressing a button on a strum empty.
    //
    //  * strum_find_penalties_note_hit() function must be called before doing this.
    //

    for (int32_t i = 0; i < ddrkeys_fifo->available; i++) {
        DDRKey* ddr_key = &ddrkeys_fifo->queue[i];
        float64 key_timestamp = ddr_key->in_song_timestamp;

        if (ddr_key->discard || ddr_key->strum_id != strum->strum_id) continue;
        if (key_timestamp > song_timestamp) continue; // maybe is a future penality

        if (ddr_key->holding) {
            // logger_log("strum: [penality] key hold on empty strum ts=" FMT_FLT64, key_timestamp);
            playerstats_add_penality(playerstats, true);
            strum_internal_update_press_state(strum, StrumPressState_PENALTY_NOTE);
            strum->marker_state = StrumMarkerState_CONFIRM;
            strum->extra_animations_have_penalties = true;
        } else {
            // logger_log("strum: [penality] key release on empty strum ts=" FMT_FLT64, key_timestamp);
            strum->marker_state = StrumMarkerState_NOTHING;
            strum_internal_update_press_state(strum, StrumPressState_NONE);
        }

        strum->marker_state_changed = true;
        ddr_key->discard = true;
        keys_processed++;
    }

    return keys_processed;
}


int32_t strum_get_press_state_changes(Strum strum) {
    return strum->press_state_changes;
}

StrumPressState strum_get_press_state(Strum strum) {
    return strum->press_state;
}

bool strum_get_press_state_use_alt_anim(Strum strum) {
    return strum->press_state_use_alt_anim;
}

const char* strum_get_name(Strum strum) {
    return strum->strum_name;
}

float strum_get_marker_duration(Strum strum) {
    return strum->marker_duration;
}

void strum_use_funkin_maker_duration(Strum strum, bool enable) {
    strum->use_fukin_marker_duration = enable;
    strum_internal_calc_marker_duration(strum, strum->scroll_velocity);
}

void strum_set_player_id(Strum strum, int32_t player_id) {
    strum->player_id = player_id;
}


int32_t strum_animate(Strum strum, float elapsed) {
    if (!strum->strum_name) return 1;

    if (beatwatcher_poll(&strum->beatwatcher) && strum->use_beat_synced_idle_and_continous) {
        statesprite_animation_restart(strum->sprite_marker_nothing);
        strum_internal_restart_extra_continous(strum);
    }

    int32_t res = 0;

    // marker animation
    // if strum_scroll_auto() function was used, keep the marker in press state for a while
    if (strum->auto_scroll_elapsed > 0.0) {
        if (strum->marker_state != STRUM_AUTO_SCROLL_MARKER_STATE) {
            strum->auto_scroll_elapsed = -1.0;
        } else {
            strum->auto_scroll_elapsed -= elapsed;
            if (strum->auto_scroll_elapsed <= 0.0) {
                strum->auto_scroll_elapsed = -1.0;
                strum->marker_state = StrumMarkerState_NOTHING;
                strum->marker_state_changed = true;
                strum_internal_update_press_state(strum, StrumPressState_NONE);
            }
        }
    }

    StateSprite current_marker;
    switch (strum->marker_state) {
        case StrumMarkerState_CONFIRM:
            current_marker = strum->sprite_marker_confirm;
            break;
        case StrumMarkerState_PRESS:
            current_marker = strum->sprite_marker_press;
            break;
        case StrumMarkerState_NOTHING:
            current_marker = strum->sprite_marker_nothing;
            break;
        default:
            return 1;
    }

    // restart the marker animation (if was changed)
    if (strum->marker_state_changed) {
        strum->marker_state_changed = false;
        statesprite_animation_restart(current_marker);
    }

    res += statesprite_animate(current_marker, elapsed);

    // background animation
    if (strum->enable_background)
        res += statesprite_animate(strum->sprite_background, elapsed);

    // splash effect animation
    if (strum->enable_sick_effect) {
        // check if is necessary restart the sick effect animation
        bool visible = true;
        switch (strum->marker_sick_state) {
            case StrumMarkerState_NOTHING:
                visible = false;
                break;
            case StrumMarkerState_PRESS:
                // change the state if the note has a custom sick effect
                statesprite_state_toggle(strum->sprite_sick_effect, strum->marker_sick_state_name);

                // play the splash effect animation again
                statesprite_animation_restart(strum->sprite_sick_effect);
                strum->marker_sick_state = StrumMarkerState_CONFIRM;
                break;
                CASE_UNUSED(StrumMarkerState_INVALID);
                CASE_UNUSED(StrumMarkerState_CONFIRM);
        }

        // hide if ended
        if (visible && statesprite_animate(strum->sprite_sick_effect, elapsed))
            strum->marker_sick_state = StrumMarkerState_NOTHING;
    }

    // compute & animate extra animations
    StrumScriptOn current_event;
    if (strum->extra_animations_have_penalties) {
        current_event = StrumScriptOn_PENALITY;
        strum->extra_animations_have_misses = strum->extra_animations_have_penalties = false;
    } else if (strum->extra_animations_have_misses) {
        current_event = StrumScriptOn_MISS;
        strum->extra_animations_have_penalties = strum->extra_animations_have_misses = false;
    } else if (strum->marker_state == StrumMarkerState_PRESS) {
        current_event = StrumScriptOn_HIT_DOWN;
    } else {
        current_event = StrumScriptOn_IDLE;
    }

    res += strum_internal_extra_animate(strum, StrumScriptTarget_STRUM_LINE, current_event, false, elapsed);
    res += strum_internal_extra_animate(strum, StrumScriptTarget_NOTE, current_event, false, elapsed);
    res += strum_internal_extra_animate(strum, StrumScriptTarget_MARKER, current_event, false, elapsed);
    res += strum_internal_extra_animate(strum, StrumScriptTarget_SICK_EFFECT, current_event, false, elapsed);
    res += strum_internal_extra_animate(strum, StrumScriptTarget_BACKGROUND, current_event, false, elapsed);

    // execute continuous extra animations
    strum_internal_extra_animate_sprite(
        strum, StrumScriptTarget_STRUM_LINE, strum->animation_strum_line.continuous.action
    );
    strum_internal_extra_animate_sprite(
        strum, StrumScriptTarget_NOTE, strum->animation_note.continuous.action
    );
    strum_internal_extra_animate_sprite(
        strum, StrumScriptTarget_MARKER, strum->animation_marker.continuous.action
    );
    strum_internal_extra_animate_sprite(
        strum, StrumScriptTarget_SICK_EFFECT, strum->animation_sick_effect.continuous.action
    );
    strum_internal_extra_animate_sprite(
        strum, StrumScriptTarget_BACKGROUND, strum->animation_background.continuous.action
    );

    return res;
}

void strum_draw(Strum strum, PVRContext pvrctx) {
    if (!strum->strum_name) return;

    pvr_context_save(pvrctx);
    drawable_helper_apply_in_context(strum->drawable, pvrctx);

    // draw the background
    if (strum->enable_background) {
        statesprite_draw(strum->sprite_background, pvrctx);
    }

    // draw the strum marker
    if (strum->marker_state == StrumMarkerState_NOTHING) {
        statesprite_draw(strum->sprite_marker_nothing, pvrctx);
    }

    // draw the note sick hit effect
    if (strum->enable_sick_effect && strum->marker_sick_state == StrumMarkerState_CONFIRM) {
        statesprite_draw(strum->sprite_sick_effect, pvrctx);
    }

    // check if there notes to draw
    if (strum->notes_render_index >= strum->chart_notes_size) {
        // nothing to draw, update the marker (if required) and return
        switch (strum->marker_state) {
            case StrumMarkerState_CONFIRM:
                statesprite_draw(strum->sprite_marker_confirm, pvrctx);
                break;
            case StrumMarkerState_PRESS:
                statesprite_draw(strum->sprite_marker_press, pvrctx);
                break;
                CASE_UNUSED(StrumMarkerState_INVALID);
                CASE_UNUSED(StrumMarkerState_NOTHING);
        }

        pvr_context_restore(pvrctx);
        return;
    }

    // toggle state on the notes (because note objects can be shared with other strum lines)
    for (int32_t i = 0; i < strum->chart_notes_id_map_size; i++) {
        if (strum->drawable_notes[i])
            note_state_toggle(strum->drawable_notes[i], strum->selected_notes_state);
    }


    //
    // setup everything to start drawing notes
    //

    const float64 song_timestamp = strum->last_song_timestamp + strum->draw_offset_milliseconds;
    int32_t notes_render_index = strum->notes_render_index;
    float draw_x = strum->modifier.x;
    float draw_y = strum->modifier.y;
    float64 song_window_past = song_timestamp - (strum->marker_duration * STRUM_DRAW_PAST_NOTES);
    float64 song_window_future = song_timestamp + strum->scroll_window;

    if (strum->scroll_is_inverse) {
        if (strum->scroll_is_vertical)
            draw_y += strum->inverse_offset;
        else
            draw_x += strum->inverse_offset;
    }

    for (int32_t i = notes_render_index; i < strum->chart_notes_size; i++) {
        StrumNote* note = &strum->chart_notes[i];

        if (strumnote_endTimestamp(note) <= song_window_past) {
            notes_render_index++;
            continue;
        } else if (note->timestamp >= song_window_future) {
            // future note, stop here
            break;
        }

        // do not draw cleared notes or invisible ones
        if (note->state == NoteState_CLEAR) continue;
        if (note->state == NoteState_HOLD && note->duration < 1) continue;
        if (note->state == NoteState_MISS && note->hit_on_penality) continue;
        if (!strum->drawable_notes[note->id]) continue;

        float64 note_duration = note->duration;
        float64 note_timestamp = note->timestamp;
        bool body_only = false;

        // on sustain notes only draw the remaining duration
        if (note->duration > 0.0) {
            if (note->state == NoteState_HOLD) {
                body_only = true;
                note_duration -= song_timestamp - note_timestamp;
                note_timestamp = song_timestamp;
            } else if (note->release_time > 0.0) {
                // early release
                body_only = true;
                note_duration -= note->release_time - note_timestamp;
                note_timestamp = note->release_time;
            }

            if (note->state != NoteState_HOLD && note_timestamp <= song_window_past) {
                // past note, show part of the sustain body
                body_only = true;
                float64 past_note_timestamp = song_timestamp - strum->marker_duration;
                note_duration -= past_note_timestamp - note_timestamp;
                note_timestamp = past_note_timestamp;

                // ignore short sustain duration
                if (body_only && note_duration < strum->marker_duration) continue;
            }

            if (note_duration <= 0.0) continue;
        }

        float64 scroll_offset;
        float64 x = draw_x;
        float64 y = draw_y;
        Note drawable_note = strum->drawable_notes[note->id];

        if (strum->scroll_is_inverse)
            scroll_offset = song_timestamp - note_timestamp;
        else
            scroll_offset = note_timestamp - song_timestamp;

        scroll_offset *= strum->scroll_velocity;

        if (strum->tweenkeyframe_note) {
            float64 percent = float64_fabs(scroll_offset / strum->scroll_window);
            tweenkeyframe_animate_percent(strum->tweenkeyframe_note, percent);
            tweenkeyframe_vertex_set_properties(
                strum->tweenkeyframe_note, note_peek_alone_statesprite(drawable_note), (PropertySetter)statesprite_set_property
            );
        }

        if (strum->scroll_is_vertical)
            y += scroll_offset;
        else
            x += scroll_offset;

        note_draw(drawable_note, pvrctx, strum->scroll_velocity, x, y, note_duration, body_only);

        if (strum->tweenkeyframe_note) {
            //
            // after the note is drawn, "attempt" to restore the original values
            // by running again the TweenKeyframe at 0%.
            //
            tweenkeyframe_animate_percent(strum->tweenkeyframe_note, 0.0);
            tweenkeyframe_vertex_set_properties(
                strum->tweenkeyframe_note, note_peek_alone_statesprite(drawable_note), (PropertySetter)statesprite_set_property
            );
        }
    }

    switch (strum->marker_state) {
        case StrumMarkerState_CONFIRM:
            statesprite_draw(strum->sprite_marker_confirm, pvrctx);
            break;
        case StrumMarkerState_PRESS:
            statesprite_draw(strum->sprite_marker_press, pvrctx);
            break;
            CASE_UNUSED(StrumMarkerState_INVALID);
            CASE_UNUSED(StrumMarkerState_NOTHING);
    }

    strum->notes_render_index = notes_render_index;
    pvr_context_restore(pvrctx);
}


void strum_enable_background(Strum strum, bool enable) {
    strum->enable_background = enable;
}

void strum_enable_sick_effect(Strum strum, bool enable) {
    strum->enable_sick_effect = enable;
    statesprite_set_visible(
        strum->sprite_sick_effect, strum->enable_sick_effect_draw && strum->enable_sick_effect
    );
}

void strum_state_add(Strum strum, ModelHolder mdlhldr_mrkr, ModelHolder mdlhldr_sck_ffct, ModelHolder mdlhldr_bckgrnd, const char* state_name) {
    StateSpriteState* state;

    // marker nothing
    strum->markers_scale = -1.0;
    state = strum_internal_state_add(
        strum->sprite_marker_nothing, mdlhldr_mrkr, strum->strum_name,
        STRUM_MARKER_SUFFIX_MARKER, state_name
    );
    if (state) strum_internal_calc_state_marker(strum, state);

    // marker confirm
    state = strum_internal_state_add(
        strum->sprite_marker_confirm, mdlhldr_mrkr, strum->strum_name,
        STRUM_MARKER_SUFFIX_CONFIRM, state_name
    );
    if (state) strum_internal_calc_state_marker(strum, state);

    // marker press
    state = strum_internal_state_add(
        strum->sprite_marker_press, mdlhldr_mrkr, strum->strum_name,
        STRUM_MARKER_SUFFIX_HIT, state_name
    );
    if (state) strum_internal_calc_state_marker(strum, state);

    // sick effect state
    strum_internal_load_sick_effect_state(
        strum,
        mdlhldr_sck_ffct,
        state_name
    );

    // background state
    state = strum_internal_state_add(
        strum->sprite_background, mdlhldr_bckgrnd, strum->strum_name,
        STRUM_BACKGROUND_SUFFIX, state_name
    );
    if (state) strum_internal_calc_state_background(strum, state);
}

int32_t strum_state_toggle(Strum strum, const char* state_name) {
    int32_t success = 0;
    success += strum_state_toggle_marker(strum, state_name) ? 1 : 0;
    success += strum_state_toggle_sick_effect(strum, state_name) ? 1 : 0;
    success += strum_state_toggle_background(strum, state_name) ? 1 : 0;
    success += strum_state_toggle_notes(strum, state_name);
    return success;
}

bool strum_state_toggle_notes(Strum strum, const char* state_name) {
    if (strum->selected_notes_state) free_chk(strum->selected_notes_state);
    strum->selected_notes_state = string_duplicate(state_name);

    return true;
}

bool strum_state_toggle_sick_effect(Strum strum, const char* state_name) {
    if (strum->selected_sick_effect_state) free_chk(strum->selected_sick_effect_state);
    strum->selected_sick_effect_state = string_duplicate(state_name);

    return statesprite_state_toggle(strum->sprite_sick_effect, state_name);
}

int32_t strum_state_toggle_marker(Strum strum, const char* state_name) {
    int32_t res = 0;

    // toggle marker sub-states (yeah state of states)
    res += statesprite_state_toggle(strum->sprite_marker_nothing, state_name) ? 1 : 0;
    res += statesprite_state_toggle(strum->sprite_marker_confirm, state_name) ? 1 : 0;
    res += statesprite_state_toggle(strum->sprite_marker_press, state_name) ? 1 : 0;

    return res;
}

bool strum_state_toggle_background(Strum strum, const char* state_name) {
    return statesprite_state_toggle(strum->sprite_background, state_name);
}


float strum_set_alpha_background(Strum strum, float alpha) {
    float old_alpha = statesprite_get_alpha(strum->sprite_background);
    statesprite_set_alpha(strum->sprite_background, alpha);
    return old_alpha;
}

float strum_set_alpha_sick_effect(Strum strum, float alpha) {
    float old_alpha = statesprite_get_alpha(strum->sprite_sick_effect);
    statesprite_set_alpha(strum->sprite_sick_effect, alpha);
    return old_alpha;
}

void strum_set_keep_aspect_ratio_background(Strum strum, bool enable) {
    strum->keep_aspect_ratio_background = enable;
    strum_internal_calc_states(strum, strum->sprite_background, strum_internal_calc_state_background);
}

StateSprite strum_draw_sick_effect_apart(Strum strum, bool enable) {
    strum->enable_sick_effect_draw = enable;
    statesprite_set_visible(strum->sprite_sick_effect, false);
    return strum->sprite_sick_effect;
}


void strum_set_extra_animation(Strum strum, StrumScriptTarget strum_script_target, StrumScriptOn strum_script_on, bool undo, AnimSprite animsprite) {
    if (strum_script_target == StrumScriptTarget_ALL) {
        strum_set_extra_animation(strum, StrumScriptTarget_MARKER, strum_script_on, undo, animsprite);
        strum_set_extra_animation(strum, StrumScriptTarget_SICK_EFFECT, strum_script_on, undo, animsprite);
        strum_set_extra_animation(strum, StrumScriptTarget_BACKGROUND, strum_script_on, undo, animsprite);
        strum_set_extra_animation(strum, StrumScriptTarget_STRUM_LINE, strum_script_on, undo, animsprite);
        strum_set_extra_animation(strum, StrumScriptTarget_NOTE, strum_script_on, undo, animsprite);
        return;
    }

    AttachedAnimations* extra_animations = strum_internal_extra_get_holder(strum, strum_script_target);
    if (!extra_animations) {
        logger_warn("strum_set_extra_animation() unknown strum_script_target=0x%x", (int)strum_script_target);
        return;
    }

    Action* action_undo_anims;

    switch (strum_script_on) {
        case StrumScriptOn_HIT_UP:
            action_undo_anims = &extra_animations->hit_up;
            break;
        case StrumScriptOn_HIT_DOWN:
            action_undo_anims = &extra_animations->hit_down;
            break;
        case StrumScriptOn_MISS:
            action_undo_anims = &extra_animations->miss;
            break;
        case StrumScriptOn_PENALITY:
            action_undo_anims = &extra_animations->penality;
            break;
        case StrumScriptOn_IDLE:
            action_undo_anims = &extra_animations->idle;
            break;
        case StrumScriptOn_ALL:
            strum_set_extra_animation(strum, strum_script_target, StrumScriptOn_HIT_UP, undo, animsprite);
            strum_set_extra_animation(strum, strum_script_target, StrumScriptOn_HIT_DOWN, undo, animsprite);
            strum_set_extra_animation(strum, strum_script_target, StrumScriptOn_MISS, undo, animsprite);
            strum_set_extra_animation(strum, strum_script_target, StrumScriptOn_IDLE, undo, animsprite);
            strum_set_extra_animation(strum, strum_script_target, StrumScriptOn_PENALITY, undo, animsprite);
            return;
        default:
            logger_warn("strum_set_extra_animation() unknown strum_script_on='0x%x", (int)strum_script_on);
            return;
    }

    AnimSprite* anim = undo ? &action_undo_anims->undo : &action_undo_anims->action;

    if (*anim)
        animsprite_destroy(anim);

    // obligatory, make a copy of animsprite
    *anim = animsprite ? animsprite_clone(animsprite) : animsprite;
}

void strum_set_extra_animation_continuous(Strum strum, StrumScriptTarget strum_script_target, AnimSprite animsprite) {
    AttachedAnimations* attached_animations;

    switch (strum_script_target) {
        case StrumScriptTarget_MARKER:
            attached_animations = &strum->animation_marker;
            break;
        case StrumScriptTarget_SICK_EFFECT:
            attached_animations = &strum->animation_sick_effect;
            break;
        case StrumScriptTarget_BACKGROUND:
            attached_animations = &strum->animation_background;
            break;
        case StrumScriptTarget_STRUM_LINE:
            attached_animations = &strum->animation_strum_line;
            break;
        case StrumScriptTarget_NOTE:
            attached_animations = &strum->animation_note;
            break;
        case StrumScriptTarget_ALL:
            strum_set_extra_animation_continuous(strum, StrumScriptTarget_MARKER, animsprite);
            strum_set_extra_animation_continuous(strum, StrumScriptTarget_SICK_EFFECT, animsprite);
            strum_set_extra_animation_continuous(strum, StrumScriptTarget_BACKGROUND, animsprite);
            strum_set_extra_animation_continuous(strum, StrumScriptTarget_STRUM_LINE, animsprite);
            strum_set_extra_animation_continuous(strum, StrumScriptTarget_NOTE, animsprite);
            return;
        default:
            logger_warn(
                "strum_set_extra_animation_continuous() unknown strum_script_target=0x%x",
                (int)strum_script_target
            );
            return;
    }

    // dispose previous animation
    if (attached_animations->continuous.action)
        animsprite_destroy(&attached_animations->continuous.action);

    // obligatory, make a copy of animsprite
    attached_animations->continuous.action = animsprite ? animsprite_clone(animsprite) : NULL;
}


void strum_set_note_tweenkeyframe(Strum strum, TweenKeyframe tweenkeyframe) {
    if (strum->tweenkeyframe_note) tweenkeyframe_destroy(&strum->tweenkeyframe_note);
    strum->tweenkeyframe_note = tweenkeyframe ? tweenkeyframe_clone(tweenkeyframe) : NULL;
}

void strum_set_sickeffect_size_ratio(Strum strum, float size_ratio) {
    strum->sick_effect_ratio = size_ratio;
    strum_internal_calc_states(strum, strum->sprite_sick_effect, strum_internal_calc_state_sick_effect);
}


void strum_set_offsetcolor(Strum strum, float r, float g, float b, float a) {
    drawable_set_offsetcolor(strum->drawable, r, g, b, a);
}

void strum_set_alpha(Strum strum, float alpha) {
    drawable_set_alpha(strum->drawable, alpha);
}

void strum_set_visible(Strum strum, bool visible) {
    drawable_set_visible(strum->drawable, visible);
}

void strum_set_draw_offset(Strum strum, float64 offset_milliseconds) {
    strum->draw_offset_milliseconds = offset_milliseconds;
}

void strum_set_bpm(Strum strum, float beats_per_minute) {
    beatwatcher_reset(&strum->beatwatcher, true, beats_per_minute);
}

void strum_disable_beat_synced_idle_and_continous(Strum strum, bool disabled) {
    strum->use_beat_synced_idle_and_continous = !disabled;
}

Modifier* strum_get_modifier(Strum strum) {
    return drawable_get_modifier(strum->drawable);
}

Drawable strum_get_drawable(Strum strum) {
    return strum->drawable;
}

float64 strum_get_duration(Strum strum) {
    float64 max_duration = 0.0;
    for (int32_t i = 0; i < strum->chart_notes_size; i++) {
        float64 end_timestamp = strumnote_endTimestamp(&strum->chart_notes[i]);
        if (end_timestamp > max_duration) max_duration = end_timestamp;
    }
    return max_duration;
}

void strum_animation_restart(Strum strum) {
    strum_internal_extra_batch(&strum->animation_strum_line, animsprite_restart);
    strum_internal_extra_batch(&strum->animation_note, animsprite_restart);
    strum_internal_extra_batch(&strum->animation_marker, animsprite_restart);
    strum_internal_extra_batch(&strum->animation_sick_effect, animsprite_restart);
    strum_internal_extra_batch(&strum->animation_background, animsprite_restart);
}

void strum_animation_end(Strum strum) {
    strum_internal_extra_batch(&strum->animation_strum_line, animsprite_force_end);
    strum_internal_extra_batch(&strum->animation_note, animsprite_force_end);
    strum_internal_extra_batch(&strum->animation_marker, animsprite_force_end);
    strum_internal_extra_batch(&strum->animation_sick_effect, animsprite_force_end);
    strum_internal_extra_batch(&strum->animation_background, animsprite_force_end);
}



static void strum_internal_calc_scroll_window(Strum strum) {
    strum->scroll_window = float64_fabs(strum->dimmen_length / strum->scroll_velocity);
}

static bool strum_internal_on_note_bounds(Strum strum, int32_t note_index, float64 test_timestamp) {
    StrumNote* strum_note = &strum->chart_notes[note_index];

    float64 note_timestamp = strum_note->timestamp;
    float64 note_duration = strum_internal_get_note_duration(strum, note_index);

    float64 start = note_timestamp - strum->marker_duration;
    float64 end = note_timestamp + note_duration;

    return test_timestamp >= start && test_timestamp <= end;
}

static float64 strum_internal_get_note_duration(Strum strum, int32_t note_index) {
    float64 note_duration = strum->chart_notes[note_index].duration;

    //
    // Note:
    //      ¿is the sustain duration visible at the current chart speed and scroll velocity?
    //      if not, the sustain length in pixels is negligible (duration too short).
    //
    //      Sustain notes should have a minimal duration according to the
    //      scroll speed, also the sustain note duration must reach the threshold.
    //

    return note_duration < strum->minimum_sustain_duration ? strum->marker_duration : note_duration;
}


static void strum_internal_calc_state_dimmen(StateSpriteState* state, bool scroll_is_vertical, float dimmen, float invdimmen) {
    float draw_width = 0.0f, draw_height = 0.0f;
    float width, height;

    if (scroll_is_vertical) {
        width = invdimmen;
        height = dimmen;
    } else {
        width = dimmen;
        height = invdimmen;
    }

    imgutils_calc_size2(state->texture, state->frame_info, width, height, &draw_width, &draw_height);

    state->draw_width = draw_width;
    state->draw_height = draw_height;
}

static StateSpriteState* strum_internal_state_add(StateSprite statesprite, ModelHolder modelholder, const char* strum_name, const char* target, const char* state_name) {
    if (statesprite_state_has(statesprite, state_name)) return NULL;
    if (!modelholder) return NULL;

    char* animation_name = string_concat_for_state_name(3, strum_name, target, state_name);
    StateSpriteState* state = statesprite_state_add(statesprite, modelholder, animation_name, state_name);
    free_chk(animation_name);

    // reset the draw location offsets becuase is picked from last state applied
    if (state) {
        state->offset_x = 0.0f;
        state->offset_y = 0.0f;
    }

    return state;
}

static void strum_internal_calc_states(Strum strum, StateSprite statesprite, void (*calc_callback)(Strum, StateSpriteState*)) {
    LinkedList list = statesprite_state_list(statesprite);
    foreach (StateSpriteState*, state, LINKEDLIST_ITERATOR, list) {
        // reset the state offsets before continue
        state->offset_x = state->offset_y = 0.0f;
        calc_callback(strum, state);
    }
    statesprite_state_apply(statesprite, NULL);
}

static void strum_internal_calc_state_marker(Strum strum, StateSpriteState* state) {
    float original_width = 0.0f, original_height = 0.0f;
    float width, height;

    if (strum->scroll_is_vertical) {
        width = strum->dimmen_opposite;
        height = strum->dimmen_marker;
        if (strum->scroll_is_inverse) state->offset_y += strum->inverse_offset;
    } else {
        width = strum->dimmen_marker;
        height = strum->dimmen_opposite;
        if (strum->scroll_is_inverse) state->offset_x += strum->inverse_offset;
    }

    if (strum->markers_scale_keep) {
        imgutils_get_statesprite_original_size(state, &original_width, &original_height);
    }

    if (strum->markers_scale < 0.0f || !strum->markers_scale_keep) {
        strum_internal_calc_state_dimmen(
            state, strum->scroll_is_vertical, strum->dimmen_marker, strum->dimmen_opposite
        );
        strum->markers_scale = state->draw_width / original_width;
    } else {
        //
        // resize sprite_marker_confirm and sprite_marker_press with the same
        // scale as sprite_marker_nothing
        //
        state->draw_width = original_width * strum->markers_scale;
        state->draw_height = original_height * strum->markers_scale;
    }


    // center marker inside of the marker bounds
    state->offset_x += (width - state->draw_width) / 2.0f;
    state->offset_y += (height - state->draw_height) / 2.0f;
}

static void strum_internal_calc_state_sick_effect(Strum strum, StateSpriteState* state) {
    const float dimmen_sick_effect = strum->dimmen_marker * strum->sick_effect_ratio;

    float width, height;
    float inverse = strum->dimmen_length - dimmen_sick_effect;

    if (strum->scroll_is_vertical) {
        width = strum->dimmen_opposite;
        height = strum->dimmen_marker;
        if (strum->scroll_is_inverse) state->offset_y += inverse + height;
    } else {
        width = dimmen_sick_effect;
        height = strum->dimmen_marker;
        if (strum->scroll_is_inverse) state->offset_x += inverse + width;
    }

    strum_internal_calc_state_dimmen(
        state, strum->scroll_is_vertical, dimmen_sick_effect, strum->dimmen_opposite * strum->sick_effect_ratio
    );

    // center the splash with the marker
    state->offset_x += (width - state->draw_width) / 2.0f;
    state->offset_y += (height - state->draw_height) / 2.0f;
}

static void strum_internal_calc_state_background(Strum strum, StateSpriteState* state) {
    float temp_width = 0.0f, temp_height = 0.0f;
    Corner corner;
    float width = strum->dimmen_opposite;
    float height = strum->dimmen_length;

    if (strum->scroll_is_vertical) {
        corner = strum->scroll_is_inverse ? Corner_BottomRight : Corner_TopLeft;
    } else {
        corner = strum->scroll_is_inverse ? Corner_BottomLeft : Corner_TopRight;
    }

    sh4matrix_helper_calculate_corner_rotation(&state->corner_rotation, corner);

    if (!strum->keep_aspect_ratio_background) {
        state->draw_width = width;
        state->draw_height = height;
        return;
    }

    imgutils_calc_size2(state->texture, state->frame_info, width, height, &temp_width, &temp_height);
    state->draw_width = temp_width;
    state->draw_height = temp_height;

    float offset_x = (strum->dimmen_opposite - state->draw_width) / 2.0;
    float offset_y = strum->dimmen_length - state->draw_height;

    switch (corner) {
        case Corner_TopLeft: // upscroll
            state->offset_x = offset_x;
            break;
        case Corner_BottomRight: // downscroll
            state->offset_x = offset_x;
            state->offset_y = offset_y;
            break;
        case Corner_TopRight: // leftscroll
            state->offset_y = offset_x;
            break;
        case Corner_BottomLeft: // rightscroll
            state->offset_x = offset_y;
            break;
    }
}

static void strum_internal_states_recalculate(Strum strum) {
    strum->markers_scale = -1.0f;
    strum_internal_calc_states(strum, strum->sprite_marker_nothing, strum_internal_calc_state_marker);
    strum_internal_calc_states(strum, strum->sprite_marker_confirm, strum_internal_calc_state_marker);
    strum_internal_calc_states(strum, strum->sprite_marker_press, strum_internal_calc_state_marker);
    strum_internal_calc_states(strum, strum->sprite_sick_effect, strum_internal_calc_state_sick_effect);
    strum_internal_calc_states(strum, strum->sprite_background, strum_internal_calc_state_background);
}



static void strum_internal_extra_destroy_animation(AttachedAnimations* holder) {
    if (holder->hit_up.action) animsprite_destroy(&holder->hit_up.action);
    if (holder->hit_down.action) animsprite_destroy(&holder->hit_down.action);
    if (holder->miss.action) animsprite_destroy(&holder->miss.action);
    if (holder->penality.action) animsprite_destroy(&holder->penality.action);
    if (holder->idle.action) animsprite_destroy(&holder->idle.action);
    if (holder->continuous.action) animsprite_destroy(&holder->continuous.action);

    if (holder->hit_up.undo) animsprite_destroy(&holder->hit_up.undo);
    if (holder->hit_down.undo) animsprite_destroy(&holder->hit_down.undo);
    if (holder->miss.undo) animsprite_destroy(&holder->miss.undo);
    if (holder->penality.undo) animsprite_destroy(&holder->penality.undo);
    if (holder->idle.undo) animsprite_destroy(&holder->idle.undo);
    // if (holder->continuous.undo) animsprite_destroy(&holder->continuous.undo);// never used
}

static int32_t strum_internal_extra_animate(Strum strum, StrumScriptTarget target, StrumScriptOn event, bool undo, float elapsed) {
    AttachedAnimations* holder = strum_internal_extra_get_holder(strum, target);
    Action* subholder;

    if (undo) {
        if (event == StrumScriptOn_ALL) return 1;

        // execute "undo" animation of the previous event
        subholder = strum_internal_extra_get_subholder(holder, event);

        if (subholder->undo) {
            animsprite_restart(subholder->undo);
            animsprite_animate(subholder->undo, elapsed);
            strum_internal_extra_animate_sprite(strum, target, subholder->undo);
        }
        return 1;
    }


    //
    // painful check:
    //      - differentiate between idle and keyup events
    //      - also wait for miss/penality/keyup animation end on idle events
    //
    if (event == StrumScriptOn_IDLE) {
        switch (holder->state) {
            case StrumExtraState_WAIT:
                event = holder->last_event;
                break;
            case StrumExtraState_NONE:
                if (holder->last_event == StrumScriptOn_HIT_DOWN) {
                    holder->state = StrumExtraState_WAIT;
                    event = StrumScriptOn_HIT_UP;
                }
                break;
                CASE_UNUSED(StrumExtraState_COMPLETED);
        }
    } else {
        switch (event) {
            case StrumScriptOn_MISS:
            case StrumScriptOn_PENALITY:
                holder->state = StrumExtraState_WAIT;
                break;
            default:
                holder->state = StrumExtraState_NONE;
                break;
        }
    }

    StrumScriptOn changed = event != holder->last_event;
    subholder = strum_internal_extra_get_subholder(holder, event);

    if (changed)
        strum_internal_extra_animate(strum, target, holder->last_event, true, FLOAT_Inf);

    holder->last_event = event;
    int32_t completed = 1;

    if (subholder->action) {
        if (changed)
            animsprite_restart(subholder->action);

        completed = animsprite_animate(subholder->action, elapsed);
        strum_internal_extra_animate_sprite(strum, target, subholder->action);
    }

    if (completed) {
        switch (event) {
            case StrumScriptOn_HIT_UP:
            case StrumScriptOn_MISS:
            case StrumScriptOn_PENALITY:
                holder->state = StrumExtraState_COMPLETED;
                break;
            default:
                holder->state = StrumExtraState_NONE;
                break;
        }
    }

    return completed;
}

static void strum_internal_extra_animate_sprite(Strum strum, StrumScriptTarget target, AnimSprite animsprite) {
    if (!animsprite) return;

    switch (target) {
        case StrumScriptTarget_STRUM_LINE:
            animsprite_update_drawable(animsprite, strum->drawable, true);
            break;
        case StrumScriptTarget_NOTE:
            int32_t last_index = strum->chart_notes_id_map_size - 1;
            for (int32_t i = 0; i < strum->chart_notes_id_map_size; i++) {
                if (strum->drawable_notes[i]) {
                    StateSprite statesprite = note_peek_alone_statesprite(strum->drawable_notes[i]);
                    animsprite_update_statesprite(animsprite, statesprite, i == last_index);
                }
            }
            break;
        case StrumScriptTarget_MARKER:
            animsprite_update_statesprite(animsprite, strum->sprite_marker_confirm, false);
            animsprite_update_statesprite(animsprite, strum->sprite_marker_nothing, false);
            animsprite_update_statesprite(animsprite, strum->sprite_marker_press, true);
            break;
        case StrumScriptTarget_SICK_EFFECT:
            animsprite_update_statesprite(animsprite, strum->sprite_sick_effect, true);
            break;
        case StrumScriptTarget_BACKGROUND:
            animsprite_update_statesprite(animsprite, strum->sprite_background, true);
            break;
            CASE_UNUSED(StrumScriptTarget_ALL);
    }
}

static AttachedAnimations* strum_internal_extra_get_holder(Strum strum, StrumScriptTarget target) {
    switch (target) {
        case StrumScriptTarget_STRUM_LINE:
            return &strum->animation_strum_line;
        case StrumScriptTarget_NOTE:
            return &strum->animation_note;
        case StrumScriptTarget_MARKER:
            return &strum->animation_marker;
        case StrumScriptTarget_SICK_EFFECT:
            return &strum->animation_sick_effect;
        case StrumScriptTarget_BACKGROUND:
            return &strum->animation_background;
        default:
            return NULL;
    }
}

static Action* strum_internal_extra_get_subholder(AttachedAnimations* holder, StrumScriptOn event) {
    switch (event) {
        case StrumScriptOn_HIT_UP:
            return &holder->hit_up;
        case StrumScriptOn_HIT_DOWN:
            return &holder->hit_down;
        case StrumScriptOn_MISS:
            return &holder->miss;
        case StrumScriptOn_PENALITY:
            return &holder->penality;
        case StrumScriptOn_IDLE:
            return &holder->idle;
        default:
            return NULL;
    }
}


static void strum_internal_check_sustain_queue(Strum strum, float64 song_timestamp, PlayerStats playerstats) {
    int32_t size = arraypointerlist_size(strum->sustain_queue);
    StrumNote** array = arraypointerlist_peek_array(strum->sustain_queue);

    for (int32_t i = 0; i < size; i++) {
        StrumNote* note = array[i];
        float64 end_timestamp = strumnote_endTimestamp(note);
        NoteAttribute* note_attributes = strum->attribute_notes[note->id];
        bool is_released = note->state == NoteState_RELEASE; // ¿was early released?
        int32_t quarter = (int32_t)float64_trunc(
            (note->duration - (end_timestamp - song_timestamp)) / strum->marker_duration_quarter
        );

        if (quarter <= 0 || quarter <= note->previous_quarter) continue;

        if (song_timestamp > end_timestamp) {
            // the sustain note is still in hold (the player keeps holding the button)
            arraypointerlist_remove_at(strum->sustain_queue, i);
            i--;
            size--;
            continue;
        }

        if ((is_released && !note_attributes->ignore_miss) || (!is_released && !note_attributes->ignore_hit)) {
            int32_t quarters = quarter - note->previous_quarter;
            playerstats_add_sustain(playerstats, quarters, is_released);

            // Mark press state as missed after the 4 first quarters
            if (is_released && quarter > 4) {
                int32_t total_quarters = (int32_t)(note->duration / strum->marker_duration_quarter) - 2;
                // also ignore last 2 quarters
                if (quarter < total_quarters) strum_internal_update_press_state(strum, StrumPressState_MISS);
            }

            // logger_log("strum: [sustain] ts=" FMT_FLT64 " release=%s quarters=" FMT_I4, note->timestamp, is_released ? "true" : "false", quarters);
        }

        note->previous_quarter = quarter;
    }
}

static void strum_internal_load_sick_effect_state(Strum strum, ModelHolder modelholder, const char* state_name) {
    StateSpriteState* state = strum_internal_state_add(
        strum->sprite_sick_effect, modelholder, strum->strum_name,
        STRUM_SICK_EFFECT_SUFFIX, state_name
    );
    if (state) strum_internal_calc_state_sick_effect(strum, state);
}

static void strum_internal_set_note_drawables(Strum strum, NotePool notepool) {
    // free previous arrays
    if (strum->drawable_notes) free_chk(strum->drawable_notes);
    if (strum->attribute_notes) free_chk(strum->attribute_notes);

    strum->drawable_notes = malloc_for_array(Note, strum->chart_notes_id_map_size);
    strum->attribute_notes = malloc_for_array(NoteAttribute*, strum->chart_notes_id_map_size);

    for (int32_t i = 0; i < strum->chart_notes_id_map_size; i++) {
        int32_t id = strum->chart_notes_id_map[i];

        if (id < 0 || id >= notepool->size) {
            logger_error("strum_internal_set_note_drawables() invalid note id found in the chart: " FMT_I4, id);
            strum->drawable_notes[i] = NULL;
            strum->attribute_notes[i] = NULL;
            continue;
        }

        strum->drawable_notes[i] = notepool->drawables[id];
        strum->attribute_notes[i] = &notepool->attributes[id];

        if (notepool->models_custom_sick_effect[id] && notepool->attributes[id].custom_sick_effect_name) {
            strum_internal_load_sick_effect_state(
                strum,
                notepool->models_custom_sick_effect[id],
                notepool->attributes[id].custom_sick_effect_name
            );
        }
    }
}

static void strum_internal_extra_batch(AttachedAnimations* holder, void (*callback)(AnimSprite)) {
    if (holder->hit_up.action) callback(holder->hit_up.action);
    if (holder->hit_down.action) callback(holder->hit_down.action);
    if (holder->miss.action) callback(holder->miss.action);
    if (holder->penality.action) callback(holder->penality.action);
    if (holder->idle.action) callback(holder->idle.action);
}

static void strum_internal_update_press_state(Strum strum, StrumPressState press_state) {
    if (press_state != StrumPressState_INVALID) {
        // if (press_state == strum->press_state) return;
        strum->press_state = press_state;
        strum->press_state_changes++;
        strum->press_state_use_alt_anim = false;
    }
}

static void strum_internal_reset_scrolling(Strum strum) {
    strum->notes_peek_index = 0;
    strum->notes_render_index = 0;
    strum->last_song_timestamp = 0.0;
    strum->marker_state = StrumMarkerState_NOTHING;
    strum->marker_state_changed = true;
    strum->marker_sick_state = StrumMarkerState_NOTHING;
    strum->marker_sick_state_name = NULL;
    strum->extra_animations_have_penalties = false;
    strum->extra_animations_have_misses = false;
    strum->auto_scroll_elapsed = -1.0;

    strum->press_state_changes = -1;
    strum->press_state = StrumPressState_NONE;

    strum->animation_background.last_event = StrumScriptOn_ALL;
    strum->animation_marker.last_event = StrumScriptOn_ALL;
    strum->animation_sick_effect.last_event = StrumScriptOn_ALL;
    strum->animation_strum_line.last_event = StrumScriptOn_ALL;
    strum->animation_note.last_event = StrumScriptOn_ALL;

    strum->animation_background.state = StrumExtraState_NONE;
    strum->animation_marker.state = StrumExtraState_NONE;
    strum->animation_sick_effect.state = StrumExtraState_NONE;
    strum->animation_strum_line.state = StrumExtraState_NONE;
    strum->animation_note.state = StrumExtraState_NONE;
}

static void strum_internal_calc_marker_duration(Strum strum, float64 velocity) {
    if (strum->use_fukin_marker_duration)
        strum->marker_duration = FUNKIN_MARKER_DURATION;
    else
        strum->marker_duration = (float)(float64_fabs(strum->dimmen_marker / velocity) * STRUM_ADDITIONAL_MARKER_DURATION_RATIO);

    strum->marker_duration *= strum->marker_duration_multiplier;

    strum->minimum_sustain_duration = strum->marker_duration * FUNKIN_NOTE_MIN_SUSTAIN_THRESHOLD;
    strum->marker_duration_quarter = strum->marker_duration / 4.0f;

    if (strum->chart_notes_size > 0)
        strum->key_test_limit = math2d_max_double(strum->chart_notes[0].timestamp - strum->marker_duration, 0.0);
    else
        strum->key_test_limit = -DOUBLE_Inf;
}

static void strum_internal_restart_extra_continous(Strum strum) {
    if (strum->animation_strum_line.continuous.action)
        animsprite_restart(strum->animation_strum_line.continuous.action);
    if (strum->animation_note.continuous.action)
        animsprite_restart(strum->animation_note.continuous.action);
    if (strum->animation_marker.continuous.action)
        animsprite_restart(strum->animation_marker.continuous.action);
    if (strum->animation_sick_effect.continuous.action)
        animsprite_restart(strum->animation_sick_effect.continuous.action);
    if (strum->animation_background.continuous.action)
        animsprite_restart(strum->animation_background.continuous.action);
}


static void strum_internal_notify_note_hit(Strum strum, WeekScript script, int32_t strum_note_index, PlayerStats playerstats) {
    StrumNote* strum_note = &strum->chart_notes[strum_note_index];
    float64 timestamp = strum_note->timestamp;
    float64 duration = strum_note->duration;
    int32_t note_id = strum->chart_notes_id_map[strum_note->id];
    bool special = strum->attribute_notes[strum_note->id]->is_special;
    float64 data = strum_note->custom_data;
    int32_t player_id = strum->player_id;
    Ranking ranking = playerstats->last_ranking;

    weekscript_notify_note(script, timestamp, note_id, duration, data, special, player_id, ranking);
}

static void strum_internal_notify_note_loss(Strum strum, WeekScript script, int32_t strum_note_idx, bool is_penalty) {
    StrumNote* strum_note = &strum->chart_notes[strum_note_idx];
    bool ignore_miss = strum->attribute_notes[strum_note->id]->ignore_miss;

    if (ignore_miss) return;

    float64 timestamp = strum_note->timestamp;
    float64 duration = strum_note->duration;
    int32_t note_id = strum->chart_notes_id_map[strum_note->id];
    float64 data = strum_note->custom_data;
    bool special = strum->attribute_notes[strum_note->id]->is_special;
    int32_t player_id = strum->player_id;
    Ranking state = is_penalty ? Ranking_PENALITY : Ranking_MISS;

    weekscript_notify_note(script, timestamp, note_id, duration, data, special, player_id, state);
}
