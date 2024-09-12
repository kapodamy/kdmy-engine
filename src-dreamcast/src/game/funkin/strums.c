#include "game/funkin/strums.h"

#include "arraylist.h"
#include "drawable.h"
#include "externals/luascript.h"
#include "game/common/funkin.h"
#include "game/funkin/strum.h"
#include "imgutils.h"
#include "logger.h"
#include "malloc_utils.h"

typedef struct {
    float64 timestamp;
    StateSprite statesprite;
    int32_t from_strum_index;
    int32_t to_strum_index;
    bool is_visible;
} Decorator;

struct Strums_s {
    int32_t size;
    Strum* lines;
    StateSprite* sick_effects;
    int32_t player_id;
    float x;
    float y;
    float gap;
    float invdimmen;
    bool is_vertical;
    bool is_inverse;
    DDRKeysFIFO* ddrkeys_fifo;
    Drawable drawable;
    AnimSprite drawable_animation;
    Modifier* modifier;
    PlayerStats playerstats;
    WeekScript weekscript;
    float64 decorators_scroll_velocity;
    float64 decorators_last_song_timestamp;
    ArrayList decorators;
    float decorators_alpha;
    float64 decorators_offset_milliseconds;
};


//
// buttons binding
//
const GamepadButtons BUTTONS_BIND_4[] = {
    GAMEPAD_DALL_LEFT | GAMEPAD_X, GAMEPAD_DALL_DOWN | GAMEPAD_A,
    GAMEPAD_DALL_UP | GAMEPAD_Y, GAMEPAD_DALL_RIGHT | GAMEPAD_B
};

const GamepadButtons BUTTONS_BIND_5[] = {
    GAMEPAD_DALL_LEFT | GAMEPAD_X, GAMEPAD_DALL_DOWN | GAMEPAD_A,
    GAMEPAD_T_LR | GAMEPAD_B_LR | GAMEPAD_SELECT,
    GAMEPAD_DALL_UP | GAMEPAD_Y, GAMEPAD_DALL_RIGHT | GAMEPAD_B
};

const GamepadButtons BUTTONS_BIND_6[] = {
    GAMEPAD_DPAD_LEFT | GAMEPAD_DPAD2_LEFT | GAMEPAD_X,
    GAMEPAD_DPAD_UP | GAMEPAD_DPAD2_UP | GAMEPAD_Y,
    GAMEPAD_DPAD_RIGHT | GAMEPAD_DPAD2_RIGHT | GAMEPAD_TRIGGER_LEFT,
    GAMEPAD_DPAD3_LEFT | GAMEPAD_DPAD4_LEFT | GAMEPAD_TRIGGER_RIGHT,
    GAMEPAD_DPAD3_UP | GAMEPAD_DPAD4_UP | GAMEPAD_B,
    GAMEPAD_DPAD3_RIGHT | GAMEPAD_DPAD4_RIGHT | GAMEPAD_A
};

const GamepadButtons BUTTONS_BIND_7[] = {
    GAMEPAD_DPAD_LEFT | GAMEPAD_DPAD2_LEFT | GAMEPAD_TRIGGER_LEFT,
    GAMEPAD_DPAD_UP | GAMEPAD_DPAD2_UP | GAMEPAD_Y,
    GAMEPAD_DPAD_RIGHT | GAMEPAD_DPAD2_RIGHT | GAMEPAD_B,
    GAMEPAD_SELECT | GAMEPAD_B_LR,
    GAMEPAD_DPAD3_LEFT | GAMEPAD_DPAD4_LEFT | GAMEPAD_X,
    GAMEPAD_DPAD3_UP | GAMEPAD_DPAD4_UP | GAMEPAD_A,
    GAMEPAD_DPAD3_RIGHT | GAMEPAD_DPAD4_RIGHT | GAMEPAD_TRIGGER_RIGHT
};

const GamepadButtons BUTTONS_BIND_8[] = {
    GAMEPAD_DPAD_LEFT | GAMEPAD_DPAD2_LEFT | GAMEPAD_X,
    GAMEPAD_DPAD_DOWN | GAMEPAD_DPAD2_DOWN | GAMEPAD_A,
    GAMEPAD_DPAD_UP | GAMEPAD_DPAD2_UP | GAMEPAD_Y,
    GAMEPAD_DPAD_RIGHT | GAMEPAD_DPAD2_RIGHT | GAMEPAD_B,
    GAMEPAD_DPAD3_LEFT | GAMEPAD_DPAD4_LEFT | GAMEPAD_TRIGGER_LEFT,
    GAMEPAD_DPAD3_DOWN | GAMEPAD_DPAD4_DOWN | GAMEPAD_BUMPER_LEFT,
    GAMEPAD_DPAD3_UP | GAMEPAD_DPAD4_UP | GAMEPAD_BUMPER_RIGHT,
    GAMEPAD_DPAD3_RIGHT | GAMEPAD_DPAD4_RIGHT | GAMEPAD_TRIGGER_RIGHT,
};

const GamepadButtons BUTTONS_BIND_9[] = {
    GAMEPAD_DPAD_LEFT | GAMEPAD_DPAD2_LEFT | GAMEPAD_X,
    GAMEPAD_DPAD_DOWN | GAMEPAD_DPAD2_DOWN | GAMEPAD_A,
    GAMEPAD_DPAD_UP | GAMEPAD_DPAD2_UP | GAMEPAD_Y,
    GAMEPAD_DPAD_RIGHT | GAMEPAD_DPAD2_RIGHT | GAMEPAD_B,
    GAMEPAD_SELECT | GAMEPAD_APAD_UP | GAMEPAD_APAD_DOWN | GAMEPAD_APAD_LEFT | GAMEPAD_APAD_RIGHT,
    GAMEPAD_DPAD3_LEFT | GAMEPAD_DPAD4_LEFT | GAMEPAD_TRIGGER_LEFT,
    GAMEPAD_DPAD3_DOWN | GAMEPAD_DPAD4_DOWN | GAMEPAD_BUMPER_LEFT,
    GAMEPAD_DPAD3_UP | GAMEPAD_DPAD4_UP | GAMEPAD_BUMPER_RIGHT,
    GAMEPAD_DPAD3_RIGHT | GAMEPAD_DPAD4_RIGHT | GAMEPAD_TRIGGER_RIGHT,
};


const Distribution STRUMS_DEFAULT_DISTRIBUTION = {
    .notes_size = 4,
    .notes = (DistributionNote[]){
        (DistributionNote){.name = "left"},
        (DistributionNote){.name = "down"},
        (DistributionNote){.name = "up"},
        (DistributionNote){.name = "right"}
    },

    .strums_size = 4,
    .strums = (DistributionStrum[]){
        (DistributionStrum){.name = "left", .notes_ids = (int32_t[]){0}, .notes_ids_size = 1},
        (DistributionStrum){.name = "down", .notes_ids = (int32_t[]){1}, .notes_ids_size = 1},
        (DistributionStrum){.name = "up", .notes_ids = (int32_t[]){2}, .notes_ids_size = 1},
        (DistributionStrum){.name = "right", .notes_ids = (int32_t[]){3}, .notes_ids_size = 1},
    },
    .strum_binds = (GamepadButtons*)BUTTONS_BIND_4,
    .states = (DistributionStrumState[]){
        (DistributionStrumState){
            .name = NULL,
            .model_marker = FUNKIN_COMMON_NOTES,
            .model_sick_effect = FUNKIN_COMMON_NOTES_SPLASH,
            .model_background = FUNKIN_COMMON_STRUM_BACKGROUND,
            .model_notes = NULL,
        },
    },
    .states_size = 1
};

__attribute__((constructor)) void init_default_strums_distribution() {
    for (int32_t i = 0; i < STRUMS_DEFAULT_DISTRIBUTION.notes_size; i++) {
        DistributionNote* note = &STRUMS_DEFAULT_DISTRIBUTION.notes[i];
        note->sick_effect_state_name = NULL;
        note->model_src = FUNKIN_COMMON_NOTES;
        note->custom_sick_effect_model_src = NULL;
        note->damage_ratio = 1.0f;
        note->heal_ratio = 1.0f;
        note->ignore_hit = false;
        note->ignore_miss = false;
        note->can_kill_on_hit = false;
    }
}



static void strums_internal_calc_decorator_bounds(Strums strums, StateSpriteState* state, int32_t from_strum_index, int32_t to_strum_index);
static void strums_internal_update_draw_location(Strums strums);


Strums strums_init(float x, float y, float z, float dimmen, float invdimmen, float length, float gap, int32_t player_id, bool is_vertical, bool keep_marker_scale, DistributionStrum* strumsdefs, int32_t strumsdefs_size) {
    Strums strums = malloc_chk(sizeof(struct Strums_s));
    malloc_assert(strums, Strums);

    *strums = (struct Strums_s){
        .size = strumsdefs_size,
        .lines = malloc_for_array(Strum, strumsdefs_size),
        .sick_effects = malloc_for_array(StateSprite, strumsdefs_size),

        .player_id = player_id,
        .x = x,
        .y = y,
        .gap = gap,
        .invdimmen = invdimmen,
        .is_vertical = is_vertical,

        .ddrkeys_fifo = NULL,
        .playerstats = NULL,
        .weekscript = NULL,

        .drawable = NULL,
        .drawable_animation = NULL,
        .modifier = NULL
    };

    strums->drawable = drawable_init(z, strums, (DelegateDraw)strums_draw, (DelegateAnimate)strums_animate);
    strums->modifier = drawable_get_modifier(strums->drawable);
    strums->modifier->x = x;
    strums->modifier->x = y;

    strums->is_vertical = is_vertical;
    strums->is_inverse = false;

    float space = gap + invdimmen;

    for (int32_t i = 0; i < strums->size; i++) {
        const char* name = strumsdefs[i].name;
        strums->sick_effects[i] = NULL;

        strums->lines[i] = strum_init(i, name, x, y, dimmen, invdimmen, length, keep_marker_scale);
        strum_set_player_id(strums->lines[i], player_id);

        if (is_vertical)
            x += space;
        else
            y += space;
    }

    float invlength = gap * strums->size;
    if (strums->size > 0) invlength -= gap;
    invlength += invdimmen * strums->size;

    if (is_vertical) {
        strums->modifier->width = invlength;
        strums->modifier->height = dimmen;
    } else {
        strums->modifier->width = dimmen;
        strums->modifier->height = invlength;
    }

    strums->decorators_scroll_velocity = 1.0;
    strums->decorators = arraylist_init2(sizeof(Decorator), 1);
    strums->decorators_alpha = 1.0f;
    strums->decorators_offset_milliseconds = 0.0;

    return strums;
}

void strums_destroy(Strums* strums_ptr) {
    if (!strums_ptr || !*strums_ptr) return;

    Strums strums = *strums_ptr;

    luascript_drop_shared(strums);

    for (int32_t i = 0; i < strums->size; i++)
        strum_destroy(&strums->lines[i]);

    drawable_destroy(&strums->drawable);
    if (strums->drawable_animation) animsprite_destroy(&strums->drawable_animation);

    foreach (Decorator*, decorator, ARRAYLIST_ITERATOR, strums->decorators)
        statesprite_destroy(&decorator->statesprite);
    arraylist_destroy(&strums->decorators);

    free_chk(strums->lines);
    free_chk(strums->sick_effects);

    free_chk(strums);
    *strums_ptr = NULL;
}

void strums_set_params(Strums strums, DDRKeymon ddrkeymon, PlayerStats playerstats, WeekScript weekscript) {
    strums->ddrkeys_fifo = ddrkeymon ? ddrkeymon_get_fifo(ddrkeymon) : NULL;
    strums->playerstats = playerstats;
    strums->weekscript = weekscript;
}

Drawable strums_get_drawable(Strums strums) {
    return strums->drawable;
}


void strums_set_notes(Strums strums, Chart chart, DistributionStrum* strumsdefs, int32_t strumsdefs_size, NotePool notepool) {
    for (int32_t i = 0; i < strums->size; i++) {
        int32_t success = strum_set_notes(
            strums->lines[i], chart, strumsdefs, strumsdefs_size, strums->player_id, notepool
        );
        if (success < 0) {
            logger_error("Error setting the notes for strum id=" FMT_I4 " player_id=" FMT_I4, i, strums->player_id);
            assert(success >= 0);
        }
    }
}

void strums_set_scroll_speed(Strums strums, float64 speed) {
    for (int32_t i = 0; i < strums->size; i++)
        strum_set_scroll_speed(strums->lines[i], speed);
}

void strums_set_scroll_direction(Strums strums, ScrollDirection direction) {
    bool is_vertical = direction == ScrollDirection_UPSCROLL || direction == ScrollDirection_DOWNSCROLL;

    strums->is_inverse = direction == ScrollDirection_DOWNSCROLL || direction == ScrollDirection_RIGHTSCROLL;
    if (is_vertical != strums->is_vertical) {
        float temp = strums->modifier->width;
        strums->modifier->width = strums->modifier->height;
        strums->modifier->height = temp;
        strums->is_vertical = is_vertical;
        strums_internal_update_draw_location(strums);
    }

    for (int32_t i = 0; i < strums->size; i++) strum_set_scroll_direction(strums->lines[i], direction);
}

void strums_set_marker_duration_multiplier(Strums strums, float multipler) {
    for (int32_t i = 0; i < strums->size; i++) strum_set_marker_duration_multiplier(strums->lines[i], multipler);
}

void strums_set_bpm(Strums strums, float beats_per_minute) {
    for (int32_t i = 0; i < strums->size; i++) strum_set_bpm(strums->lines[i], beats_per_minute);
}

void strums_disable_beat_synced_idle_and_continous(Strums strums, bool disabled) {
    for (int32_t i = 0; i < strums->size; i++) strum_disable_beat_synced_idle_and_continous(strums->lines[i], disabled);
}

void strums_reset(Strums strums, float64 scroll_speed, const char* state_name) {
    for (int32_t i = 0; i < strums->size; i++) {
        strum_reset(strums->lines[i], scroll_speed, state_name);
    }
    drawable_set_antialiasing(strums->drawable, PVRCTX_FLAG_DEFAULT);
    strums_decorators_set_scroll_speed(strums, scroll_speed);
    strums_decorators_set_visible(strums, -1.0, true);
    strums->decorators_last_song_timestamp = 0.0;
}

int32_t strums_scroll_full(Strums strums, float64 song_timestamp) {
    strums->decorators_last_song_timestamp = song_timestamp;

    if (!strums->ddrkeys_fifo) {
        // this never should happen, use strums_scroll_auto() instead
        for (int32_t i = 0; i < strums->size; i++)
            strum_scroll_auto(strums->lines[i], song_timestamp, strums->playerstats, strums->weekscript);
        return 0;
    }

    int32_t keys_processed = 0;

    // step 1: scroll all lines
    for (int32_t i = 0; i < strums->size; i++) {
        keys_processed += strum_scroll(
            strums->lines[i], song_timestamp, strums->ddrkeys_fifo, strums->playerstats, strums->weekscript
        );
    }

    // step 2: find penalties hits between lines
    for (int32_t i = 0; i < strums->size; i++) {
        keys_processed += strum_find_penalties_note_hit(
            strums->lines[i], song_timestamp, strums->ddrkeys_fifo, strums->playerstats, strums->weekscript
        );
    }

    // step 3: find penalties hits on empty lines
    for (int32_t i = 0; i < strums->size; i++) {
        keys_processed += strum_find_penalties_empty_hit(
            strums->lines[i], song_timestamp, strums->ddrkeys_fifo, strums->playerstats
        );
    }

    if (keys_processed > 0) {
        ddrkeymon_purge(strums->ddrkeys_fifo);
    }

    return keys_processed;
}

void strums_scroll_auto(Strums strums, float64 song_timestamp) {
    for (int32_t i = 0; i < strums->size; i++)
        strum_scroll_auto(strums->lines[i], song_timestamp, strums->playerstats, strums->weekscript);

    if (strums->ddrkeys_fifo) {
        // drop all key events, they are useless
        strums->ddrkeys_fifo->available = 0;
    }

    strums->decorators_last_song_timestamp = song_timestamp;
}

void strums_force_key_release(Strums strums) {
    for (int32_t i = 0; i < strums->size; i++) {
        strum_force_key_release(strums->lines[i]);
    }
}


int32_t strums_animate(Strums strums, float elapsed) {
    int32_t res = 0;
    if (strums->drawable_animation) {
        res += animsprite_animate(strums->drawable_animation, elapsed);
        animsprite_update_drawable(strums->drawable_animation, strums->drawable, true);
    }

    for (int32_t i = 0; i < strums->size; i++)
        res += strum_animate(strums->lines[i], elapsed);

    Decorator* decorators_array = arraylist_peek_array(strums->decorators);
    int32_t decorators_size = arraylist_size(strums->decorators);
    for (int32_t i = 0; i < decorators_size; i++)
        res += statesprite_animate(decorators_array[i].statesprite, elapsed);

    return res;
}

void strums_draw(Strums strums, PVRContext pvrctx) {
    pvr_context_save(pvrctx);

    drawable_helper_apply_in_context(strums->drawable, pvrctx);

    for (int32_t i = 0; i < strums->size; i++) {
        if (!drawable_is_visible(strum_get_drawable(strums->lines[i]))) continue;
        strum_draw(strums->lines[i], pvrctx);
    }

    for (int32_t i = 0; i < strums->size; i++) {
        if (!drawable_is_visible(strum_get_drawable(strums->lines[i]))) continue;
        if (strums->sick_effects[i] && statesprite_is_visible(strums->sick_effects[i]))
            statesprite_draw(strums->sick_effects[i], pvrctx);
    }

    pvr_context_save(pvrctx);
    pvr_context_set_global_alpha(pvrctx, strums->decorators_alpha);

    int32_t decorators_size = arraylist_size(strums->decorators);
    Decorator* decorators_array = arraylist_peek_array(strums->decorators);
    float64 song_timestamp = strums->decorators_last_song_timestamp + strums->decorators_offset_milliseconds;
    float draw_x = strums->modifier->x;
    float draw_y = strums->modifier->y;

    //
    // Draw all decorators and let the PVRContext decide what is visible, because
    // decorators can have different dimmensions. The marker duration and scroll window
    // does not apply here.
    //
    for (int32_t i = 0; i < decorators_size; i++) {
        float64 decorator_timestamp = decorators_array[i].timestamp;
        float64 scroll_offset;
        float x = draw_x;
        float y = draw_y;

        if (strums->is_inverse) {
            scroll_offset = song_timestamp - decorator_timestamp;
        } else {
            scroll_offset = decorator_timestamp - song_timestamp;
        }

        scroll_offset *= strums->decorators_scroll_velocity;

        if (strums->is_vertical)
            y = (float)(y + scroll_offset);
        else
            x = (float)(x + scroll_offset);

        StateSprite statesprite = decorators_array[i].statesprite;
        statesprite_set_draw_location(statesprite, x, y);
        statesprite_draw(statesprite, pvrctx);
    }

    pvr_context_restore(pvrctx);
    pvr_context_restore(pvrctx);
}


float strums_set_alpha(Strums strums, float alpha) {
    float old = drawable_get_alpha(strums->drawable);
    drawable_set_alpha(strums->drawable, alpha);
    return old;
}

void strums_enable_background(Strums strums, bool enable) {
    for (int32_t i = 0; i < strums->size; i++)
        strum_enable_background(strums->lines[i], enable);
}

void strums_set_keep_aspect_ratio_background(Strums strums, bool enable) {
    for (int32_t i = 0; i < strums->size; i++)
        strum_set_keep_aspect_ratio_background(strums->lines[i], enable);
}

void strums_set_alpha_background(Strums strums, float alpha) {
    for (int32_t i = 0; i < strums->size; i++)
        strum_set_alpha_background(strums->lines[i], alpha);
}

void strums_set_alpha_sick_effect(Strums strums, float alpha) {
    for (int32_t i = 0; i < strums->size; i++)
        strum_set_alpha_sick_effect(strums->lines[i], alpha);
}

void strums_set_draw_offset(Strums strums, float64 offset_milliseconds) {
    for (int32_t i = 0; i < strums->size; i++)
        strum_set_draw_offset(strums->lines[i], offset_milliseconds);
    strums->decorators_offset_milliseconds = offset_milliseconds;
}


void strums_state_add(Strums strums, ModelHolder mdlhldr_mrkr, ModelHolder mdlhldr_sck_ffct, ModelHolder mdlhldr_bckgrnd, const char* state_name) {
    for (int32_t i = 0; i < strums->size; i++)
        strum_state_add(strums->lines[i], mdlhldr_mrkr, mdlhldr_sck_ffct, mdlhldr_bckgrnd, state_name);
}

int32_t strums_state_toggle(Strums strums, const char* state_name) {
    int32_t toggles = 0;
    for (int32_t i = 0; i < strums->size; i++)
        toggles += strum_state_toggle(strums->lines[i], state_name);
    return toggles;
}

int32_t strums_state_toggle_notes(Strums strums, const char* state_name) {
    int32_t toggles = 0;
    for (int32_t i = 0; i < strums->size; i++)
        toggles += strum_state_toggle_notes(strums->lines[i], state_name);
    return toggles;
}

void strums_state_toggle_marker_and_sick_effect(Strums strums, const char* state_name) {
    for (int32_t i = 0; i < strums->size; i++) {
        strum_state_toggle_marker(strums->lines[i], state_name);
        strum_state_toggle_sick_effect(strums->lines[i], state_name);
    }
}

int32_t strums_get_lines_count(Strums strums) {
    return strums->size;
}

Strum strums_get_strum_line(Strums strums, int32_t index) {
    if (index < 0 || index >= strums->size) return NULL;
    return strums->lines[index];
}

void strums_enable_post_sick_effect_draw(Strums strums, bool enable) {
    for (int32_t i = 0; i < strums->size; i++) {
        if (enable)
            strums->sick_effects[i] = strum_draw_sick_effect_apart(strums->lines[i], enable);
        else
            strums->sick_effects[i] = NULL;
    }
}

void strums_use_funkin_maker_duration(Strums strums, bool enable) {
    for (int32_t i = 0; i < strums->size; i++) {
        strum_use_funkin_maker_duration(strums->lines[i], enable);
    }
}


void strums_set_offsetcolor(Strums strums, float r, float g, float b, float a) {
    drawable_set_offsetcolor(strums->drawable, r, g, b, a);
}

void strums_set_visible(Strums strums, bool visible) {
    drawable_set_visible(strums->drawable, visible);
}

Modifier* strums_get_modifier(Strums strums) {
    return drawable_get_modifier(strums->drawable);
}


void strums_animation_set(Strums strums, AnimSprite animsprite) {
    if (strums->drawable_animation) animsprite_destroy(&strums->drawable_animation);
    strums->drawable_animation = animsprite ? animsprite_clone(animsprite) : NULL;
}

void strums_animation_restart(Strums strums) {
    if (strums->drawable_animation) animsprite_restart(strums->drawable_animation);

    for (int32_t i = 0; i < strums->size; i++)
        strum_animation_restart(strums->lines[i]);

    Decorator* decorators_array = arraylist_peek_array(strums->decorators);
    int32_t decorators_size = arraylist_size(strums->decorators);
    for (int32_t i = 0; i < decorators_size; i++)
        statesprite_animation_restart(decorators_array[i].statesprite);
}

void strums_animation_end(Strums strums) {
    if (strums->drawable_animation) {
        animsprite_force_end(strums->drawable_animation);
        animsprite_update_drawable(strums->drawable_animation, strums->drawable, true);
    }

    for (int32_t i = 0; i < strums->size; i++)
        strum_animation_end(strums->lines[i]);

    Decorator* decorators_array = arraylist_peek_array(strums->decorators);
    int32_t decorators_size = arraylist_size(strums->decorators);
    for (int32_t i = 0; i < decorators_size; i++)
        statesprite_animation_end(decorators_array[i].statesprite);
}


int32_t strums_decorators_get_count(Strums strums) {
    return arraylist_size(strums->decorators);
}

bool strums_decorators_add(Strums strums, ModelHolder modelholder, const char* animation_name, float64 timestamp) {
    return strums_decorators_add2(strums, modelholder, animation_name, timestamp, 0, strums->size - 1);
}

bool strums_decorators_add2(Strums strums, ModelHolder modelholder, const char* animation_name, float64 timestamp, int32_t from_strum_index, int32_t to_strum_index) {
    if (!modelholder || timestamp < 0.0 || math2d_is_double_NaN(timestamp)) return false;
    if (from_strum_index < 0 || to_strum_index >= strums->size || to_strum_index < from_strum_index) return false;

    StateSprite statesprite = statesprite_init_from_vertex_color(modelholder_get_vertex_color(modelholder));
    StateSpriteState* state = statesprite_state_add(statesprite, modelholder, animation_name, NULL);

    if (!state) {
        statesprite_destroy(&statesprite);
        return false;
    }

    strums_internal_calc_decorator_bounds(strums, state, from_strum_index, to_strum_index);
    statesprite_state_apply(statesprite, state);

    Decorator decorator = (Decorator){
        .timestamp = timestamp,
        .statesprite = statesprite,
        .to_strum_index = to_strum_index,
        .from_strum_index = from_strum_index,
        .is_visible = true
    };

    arraylist_add(strums->decorators, &decorator);

    return true;
}

void strums_decorators_set_scroll_speed(Strums strums, float64 speed) {
    strums->decorators_scroll_velocity = FUNKIN_CHART_SCROLL_VELOCITY * speed;

    // Use half of the scolling speed if the screen aspect ratio is 4:3 (dreamcast)
    if (!pvr_context_is_widescreen()) strums->decorators_scroll_velocity *= 0.5;
}

void strums_decorators_set_alpha(Strums strums, float alpha) {
    strums->decorators_alpha = alpha;
}

void strums_decorators_set_visible(Strums strums, float64 decorator_timestamp, bool visible) {
    foreach (Decorator*, decorator, ARRAYLIST_ITERATOR, strums->decorators) {
        if (decorator_timestamp < 0.0 || math2d_doubles_are_near_equal(decorator->timestamp, decorator_timestamp)) {
            decorator->is_visible = visible;
        }
    }
}


void strums_force_rebuild(Strums strums, float x, float y, float z, float dimmen, float invdimmen, float length_dimmen, float gap, bool is_vertical, bool keep_markers_scale) {
    drawable_set_z_index(strums->drawable, z);

    strums->modifier->x = x;
    strums->modifier->y = y;
    strums->is_vertical = is_vertical;
    strums->is_inverse = false;
    strums->gap = gap;
    strums->invdimmen = invdimmen;

    float invlength = gap * strums->size;
    if (strums->size > 0) invlength -= gap;
    invlength += invdimmen * strums->size;

    if (is_vertical) {
        strums->modifier->width = invlength;
        strums->modifier->height = dimmen;
    } else {
        strums->modifier->width = dimmen;
        strums->modifier->height = invlength;
    }

    strums_internal_update_draw_location(strums);

    for (int32_t i = 0; i < strums->size; i++) {
        strum_force_rebuild(strums->lines[i], dimmen, invdimmen, length_dimmen, keep_markers_scale);
    }
}


static void strums_internal_calc_decorator_bounds(Strums strums, StateSpriteState* state, int32_t from_strum_index, int32_t to_strum_index) {
    float offset_start = (strums->gap + strums->invdimmen) * from_strum_index;
    float offset_end = (strums->gap + strums->invdimmen) * to_strum_index;
    float length = offset_end - offset_start;

    float x, y;
    float width, height;
    Align horizontal, vertical;

    if (strums->is_vertical) {
        x = offset_start;
        y = 0.0f;
        width = length;
        height = -1.0f;
        horizontal = ALIGN_CENTER;
        vertical = strums->is_inverse ? ALIGN_END : ALIGN_START;
    } else {
        x = 0.0f;
        y = offset_start;
        width = -1.0f;
        height = length;
        horizontal = strums->is_inverse ? ALIGN_END : ALIGN_START;
        vertical = ALIGN_CENTER;
    }

    if (state->texture) {
        imgutils_calc_rectangle_in_statesprite_state(x, y, width, height, horizontal, vertical, state);
    } else {
        state->offset_x = x;
        state->offset_y = y;
        state->draw_width = strums->is_vertical ? length : strums->invdimmen;
        state->draw_height = strums->is_vertical ? strums->invdimmen : length;
    }
}

static void strums_internal_update_draw_location(Strums strums) {
    float space = strums->gap + strums->invdimmen;
    float x = strums->x;
    float y = strums->y;

    for (int32_t i = 0; i < strums->size; i++) {
        strum_update_draw_location(strums->lines[i], x, y);

        if (strums->is_vertical)
            x += space;
        else
            y += space;
    }

    foreach (Decorator*, decorator, ARRAYLIST_ITERATOR, strums->decorators) {
        StateSpriteState* state = statesprite_state_get(decorator->statesprite);
        strums_internal_calc_decorator_bounds(strums, state, decorator->from_strum_index, decorator->to_strum_index);
        statesprite_state_apply(decorator->statesprite, state);
    }
}
