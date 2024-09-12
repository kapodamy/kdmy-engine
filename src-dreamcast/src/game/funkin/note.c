#include "game/funkin/note.h"

#include "game/common/funkin.h"
#include "imgutils.h"
#include "malloc_utils.h"
#include "statesprite.h"
#include "stringutils.h"
#include "unused_switch_case.h"


#define NOTE_SUFFIX_ALONE "alone"
#define NOTE_SUFFIX_HOLD "hold"
#define NOTE_SUFFIX_TAIL "tail"
#define NOTE_ALTERNATE_MINE "mine"

typedef struct {
    char* name;
    float height_hold;
    float offset_hold;
    float height_tail;
    float offset_tail;
} NoteState;

struct Note_s {
    StateSprite sprite_alone;
    StateSprite sprite_hold;
    StateSprite sprite_tail;
    float current_dimmen;
    float dimmen;
    float invdimmen;
    float dimmen_alone;
    float dimmen_alone_half;
    float height_hold;
    float height_tail;
    float offset_hold;
    float offset_tail;
    ScrollDirection direction;
    Modifier modifier_sustain;
    char* name;
    float alpha_sustain;
    float alpha_alone;
    LinkedList internal_states;
    char* current_state_name;
    bool has_part_alone;
    bool has_part_hold;
    bool has_part_tail;
};

static const float NOTE_SUSTAIN_BODY_OVELAPING_PIXELS = 0.5f;
static const char* NOTE_INTERNAL_STATE_NAME = "note-state";

static StateSpriteState* note_internal_load_part(StateSprite part, const char* name, const char* suffix, ModelHolder modelholder, const char* state_name);
static void note_internal_resize_sprites(Note note, NoteState* state_note, StateSpriteState* state_alone, StateSpriteState* state_hold, StateSpriteState* state_tail);
static void note_internal_resize_sustain_part(StateSpriteState* statesprite_state, float scale);
static void note_internal_destroy_internal_state(NoteState* internal_state);
static StateSpriteState* note_internal_get_statesprite_state(StateSprite statesprite, const char* state_name);


Note note_init(const char* name, float dimmen, float invdimmen) {
    Note note = malloc_chk(sizeof(struct Note_s));
    malloc_assert(note, Note);

    *note = (struct Note_s){
        .sprite_alone = statesprite_init_from_texture(NULL),
        .sprite_hold = statesprite_init_from_texture(NULL),
        .sprite_tail = statesprite_init_from_texture(NULL),

        .current_dimmen = dimmen,
        .dimmen = dimmen,
        .invdimmen = invdimmen,

        // assign some default values
        .dimmen_alone = dimmen,
        .dimmen_alone_half = dimmen / 2.0f,

        .height_hold = 44.0f,
        .height_tail = 64.0f,

        .offset_hold = 15.0f,
        .offset_tail = 15.0f,

        .direction = ScrollDirection_UPSCROLL,

        // .modifier_sustain = (Modifier){},

        .name = string_duplicate(name),

        .alpha_sustain = 1.0f,
        .alpha_alone = 1.0f,

        .internal_states = linkedlist_init(),
        .current_state_name = (char*)NOTE_INTERNAL_STATE_NAME,

        .has_part_alone = false,
        .has_part_hold = false,
        .has_part_tail = false
    };

    pvr_context_helper_clear_modifier(&note->modifier_sustain);

    statesprite_set_visible(note->sprite_alone, false);
    statesprite_set_visible(note->sprite_hold, false);
    statesprite_set_visible(note->sprite_tail, false);

    return note;
}

void note_destroy(Note* note_ptr) {
    if (!note_ptr || !*note_ptr) return;

    Note note = *note_ptr;

    statesprite_destroy(&note->sprite_alone);
    statesprite_destroy(&note->sprite_hold);
    statesprite_destroy(&note->sprite_tail);

    linkedlist_destroy2(&note->internal_states, (FreeDelegate)note_internal_destroy_internal_state);

    // note: do not dispose "note->current_state_name" is part of "note->internal_states"

    free_chk(note->name);

    free_chk(note);
    *note_ptr = NULL;
}

void note_set_scoll_direction(Note note, ScrollDirection direction) {
    switch (direction) {
        case ScrollDirection_LEFTSCROLL:
            note->modifier_sustain.rotate = -90.0f;
            break;
        case ScrollDirection_RIGHTSCROLL:
            note->modifier_sustain.rotate = 90.0f;
            break;
        case ScrollDirection_DOWNSCROLL:
            note->modifier_sustain.rotate = 180.0f;
            break;
        case ScrollDirection_UPSCROLL:
            note->modifier_sustain.rotate = 0.0f;
            break;
        default:
            return;
    }

    switch (direction) {
        case ScrollDirection_LEFTSCROLL:
        case ScrollDirection_RIGHTSCROLL:
            note->current_dimmen = note->invdimmen;
            break;
        case ScrollDirection_DOWNSCROLL:
        case ScrollDirection_UPSCROLL:
            note->current_dimmen = note->dimmen;
            break;
    }

    note->direction = direction;

    if (direction != ScrollDirection_UPSCROLL) {
        note->modifier_sustain.rotate *= MATH2D_DEG_TO_RAD;
        note->modifier_sustain.rotate_pivot_enabled = true;
        note->modifier_sustain.rotate_pivot_u = 0.0f;
        note->modifier_sustain.rotate_pivot_v = 0.0f;
        note->modifier_sustain.width = 0.0f;
        note->modifier_sustain.height = 0.0f;
        note->modifier_sustain.translate_rotation = true;
    }

    // recalculate all sprite states
    foreach (NoteState*, state_note, LINKEDLIST_ITERATOR, note->internal_states) {
        StateSpriteState* state_alone = note_internal_get_statesprite_state(note->sprite_alone, state_note->name);
        StateSpriteState* state_hold = note_internal_get_statesprite_state(note->sprite_hold, state_note->name);
        StateSpriteState* state_tail = note_internal_get_statesprite_state(note->sprite_tail, state_note->name);
        note_internal_resize_sprites(note, state_note, state_alone, state_hold, state_tail);
    }

    foreach (NoteState*, note_state, LINKEDLIST_ITERATOR, note->internal_states) {
        if (string_equals(note_state->name, note->current_state_name)) {
            note->height_hold = note_state->height_hold;
            note->offset_hold = note_state->offset_hold;
            note->height_tail = note_state->height_tail;
            note->offset_tail = note_state->offset_tail;
            break;
        }
    }
}

void note_set_alpha(Note note, float alpha) {
    note_set_alpha_alone(note, alpha);
    note_set_alpha_sustain(note, alpha);
}

void note_set_alpha_alone(Note note, float alpha) {
    note->alpha_alone = alpha;
}

void note_set_alpha_sustain(Note note, float alpha) {
    note->alpha_sustain = alpha;
}

int32_t note_animate(Note note, float elapsed) {
    int32_t res = 0;
    if (note->sprite_alone) res += statesprite_animate(note->sprite_alone, elapsed);
    if (note->sprite_hold) res += statesprite_animate(note->sprite_hold, elapsed);
    if (note->sprite_tail) res += statesprite_animate(note->sprite_tail, elapsed);
    return res;
}

const char* note_get_name(Note note) {
    return note->name;
}

void note_draw(Note note, PVRContext pvrctx, float64 scroll_velocity, float64 x, float64 y, float64 duration_ms, bool only_body) {
    //
    // Note:
    //      Scroll velocity should be according to the UI layout viewport (or the screen resolution)
    //      and the song chart speed should be already applied
    //
    float64 length = float64_fabs(duration_ms * scroll_velocity); // note length in pixels
    float64 sustain_length = length - note->dimmen_alone;

    // save PVR context and apply the modifier to alone, hold and tail sprites
    pvr_context_save(pvrctx);

    float dimmen_alone_half = note->has_part_alone ? note->dimmen_alone_half : 0.0f;

    // draw the sustain body (hold & tail)
    if (sustain_length > 0.0 && note->has_part_hold && note->has_part_tail) {
        if (note->direction != ScrollDirection_UPSCROLL) {
            switch (note->direction) {
                case ScrollDirection_DOWNSCROLL:
                    y -= sustain_length;
                    break;
                case ScrollDirection_RIGHTSCROLL:
                    x -= sustain_length;
                    break;
                case ScrollDirection_LEFTSCROLL:
                    break;
                    CASE_UNUSED(ScrollDirection_UPSCROLL);
            }
            note->modifier_sustain.x = (float)x;
            note->modifier_sustain.y = (float)y;
        }

        float64 draw_x = x + note->offset_hold;
        float64 draw_y = y;
        float64 tail_correction;
        float64 height_hold = note->height_hold;
        float64 height_tail = note->height_tail;

        switch (note->direction) {
            case ScrollDirection_RIGHTSCROLL:
            case ScrollDirection_DOWNSCROLL:
                draw_y -= sustain_length + dimmen_alone_half;
                tail_correction = 0.0;
                break;
            default:
                draw_y += dimmen_alone_half;
                tail_correction = length;
                break;
        }

        // crop sprite_tail if necessary
        float64 hold_length = sustain_length + dimmen_alone_half;
        if (hold_length < height_tail) {
            // crop the top part of the sprite
            float64 offset = height_tail - hold_length;
            height_tail = hold_length;
            statesprite_crop_enable(note->sprite_tail, true);
            statesprite_crop(note->sprite_tail, 0.0f, (float)offset, -1.0f, -1.0f);
        }

        // reserve space for the sprite_tail
        hold_length -= height_tail;
        // Rendering workaround to avoid gaps
        hold_length += NOTE_SUSTAIN_BODY_OVELAPING_PIXELS;
        height_hold -= NOTE_SUSTAIN_BODY_OVELAPING_PIXELS;

        pvr_context_save(pvrctx);
        pvr_context_apply_modifier(pvrctx, &note->modifier_sustain);
        pvrctx->global_alpha = note->alpha_sustain;

        while (hold_length > height_hold) {
            statesprite_set_draw_location(note->sprite_hold, (float)draw_x, (float)draw_y);
            statesprite_draw(note->sprite_hold, pvrctx);
            draw_y += height_hold;
            hold_length -= height_hold;
        }

        if (hold_length > 0.0) {
            // crop the bottom part of sprite_hold and draw it
            statesprite_crop_enable(note->sprite_hold, true);
            statesprite_crop(note->sprite_hold, 0.0, 0.0, -1.0, (float)hold_length);

            statesprite_set_draw_location(note->sprite_hold, (float)draw_x, (float)draw_y);
            statesprite_draw(note->sprite_hold, pvrctx);
        }

        // draw sprite_tail
        draw_x = x + note->offset_tail;
        draw_y = y + tail_correction - height_tail;
        statesprite_set_draw_location(note->sprite_tail, (float)draw_x, (float)draw_y);
        statesprite_draw(note->sprite_tail, pvrctx);

        // disable crop sustain body (hold & tail)
        statesprite_crop_enable(note->sprite_hold, false);
        statesprite_crop_enable(note->sprite_tail, false);

        pvr_context_restore(pvrctx);
    }

    // draw sprite_alone
    if (!only_body && note->has_part_alone) {
        if (sustain_length > 0.0) {
            if (note->direction == ScrollDirection_DOWNSCROLL)
                y += sustain_length;
            else if (note->direction == ScrollDirection_RIGHTSCROLL)
                x += sustain_length;
        }

        pvrctx->global_alpha = note->alpha_alone;
        statesprite_set_draw_location(note->sprite_alone, (float)x, (float)y);
        statesprite_draw(note->sprite_alone, pvrctx);
    }

    pvr_context_restore(pvrctx);
}


bool note_state_add(Note note, ModelHolder modelholder, const char* state_name) {
    //
    // build every part of the note
    // all sprites are hidden to avoid getting drawn by the PVR graphics backend
    // we draw those sprites manually in note_draw()
    //

    StateSpriteState* state_alone = note_internal_load_part(
        note->sprite_alone, note->name, NOTE_SUFFIX_ALONE, modelholder, state_name
    );
    StateSpriteState* state_hold = note_internal_load_part(
        note->sprite_hold, note->name, NOTE_SUFFIX_HOLD, modelholder, state_name
    );
    StateSpriteState* state_tail = note_internal_load_part(
        note->sprite_tail, note->name, NOTE_SUFFIX_TAIL, modelholder, state_name
    );

    if (!state_alone && !state_hold && !state_tail) return false;

    NoteState* state_note = malloc_chk(sizeof(NoteState));
    malloc_assert(state_note, NoteState);

    *state_note = (NoteState){.name = NULL, .height_hold = 0.0f, .offset_hold = 0.0f, .height_tail = 0.0f, .offset_tail = 0.0f};
    if (state_name != NULL) state_note->name = string_duplicate(state_name);

    linkedlist_add_item(note->internal_states, state_note);
    note_internal_resize_sprites(note, state_note, state_alone, state_hold, state_tail);

    return true;
}

int32_t note_state_toggle(Note note, const char* state_name) {
    if (string_equals(note->current_state_name, state_name)) return 4;

    int32_t res = 0;

    foreach (NoteState*, note_state, LINKEDLIST_ITERATOR, note->internal_states) {
        if (string_equals(note_state->name, state_name)) {
            note->height_hold = note_state->height_hold;
            note->offset_hold = note_state->offset_hold;
            note->height_tail = note_state->height_tail;
            note->offset_tail = note_state->offset_tail;
            note->current_state_name = note_state->name;
            res = 1;
            break;
        }
    }

    if (res < 1) return 0;

    res += (note->has_part_alone = statesprite_state_toggle(note->sprite_alone, state_name)) ? 1 : 0;
    res += (note->has_part_hold = statesprite_state_toggle(note->sprite_hold, state_name)) ? 1 : 0;
    res += (note->has_part_tail = statesprite_state_toggle(note->sprite_tail, state_name)) ? 1 : 0;

    return res;
}

StateSprite note_peek_alone_statesprite(Note note) {
    return note->sprite_alone;
}


static StateSpriteState* note_internal_load_part(StateSprite part, const char* name, const char* suffix, ModelHolder modelholder, const char* state_name) {
    Atlas atlas = modelholder_get_atlas(modelholder);
    AnimList animlist = modelholder_get_animlist(modelholder);
    Texture texture = modelholder_get_texture(modelholder, false);
    uint32_t rgb8_color = modelholder_get_vertex_color(modelholder);

    char* animation_name = string_concat_for_state_name(3, name, suffix, state_name);

    //
    // Attempt to load the arrow part (texture, frame & animation)
    //
    AnimSprite animsprite;

    // read the animation from the animlist (if necessary)
    if (animlist) {
        const AnimListItem* animlist_item = animlist_get_animation(animlist, animation_name);
        if (animlist_item) {
            animsprite = animsprite_init(animlist_item);
            goto L_build_state;
        }
    }

    // fallback, build the animation using atlas frames directly
    float framerate = atlas_get_glyph_fps(atlas);
    if (framerate <= 0.0f) framerate = FUNKIN_DEFAULT_ANIMATIONS_FRAMERATE;

    animsprite = animsprite_init_from_atlas(framerate, 0, atlas, animation_name, true);

    if (animsprite)
        goto L_build_state;

    // the arrow is not animated, build an static animation
    const AtlasEntry* atlas_entry = atlas_get_entry(atlas, animation_name);
    if (atlas_entry) {
        animsprite = animsprite_init_from_atlas_entry(atlas_entry, true, framerate);
        goto L_build_state;
    }

    /*if (state_name) {
        free_chk(animation_name);

        // try again but without the state name, can fail if the texture supplied is custom
        // with a hold duration (alone + hold + tail)
        StateSpriteState* statesprite_state = note_internal_create_arrow_part(
            part, name, suffix, atlas, animlist, texture, rgb8_color, NULL
        );

        // set the state name (NULL was used, before)
        statesprite_state->state_name = string_duplicate(state_name);

        return statesprite_state;
    }*/

    // this part of the arrow is incomplete
    animsprite = NULL;


L_build_state:
    if (!animsprite) {
        free_chk(animation_name);
        return NULL;
    }

    atlas_entry = atlas_get_entry_with_number_suffix(atlas, animation_name);
    if (!atlas_entry) atlas_entry = atlas_get_entry(atlas, animation_name);

    free_chk(animation_name);

    StateSpriteState* state = statesprite_state_add2(part, texture, animsprite, atlas_entry, rgb8_color, state_name);
    if (!state && animsprite) animsprite_destroy(&animsprite);

    return state;
}

static void note_internal_resize_sprites(Note note, NoteState* state_note, StateSpriteState* state_alone, StateSpriteState* state_hold, StateSpriteState* state_tail) {
    //
    // confuse part, set the desired width and height of the alone sprite
    //
    float ref_width, ref_height, ref_invdimmen;
    bool inverse = false;

    switch (note->direction) {
        case ScrollDirection_UPSCROLL:
        case ScrollDirection_DOWNSCROLL:
            ref_width = note->invdimmen;
            ref_height = note->dimmen;
            ref_invdimmen = note->invdimmen;
            break;
        case ScrollDirection_LEFTSCROLL:
        case ScrollDirection_RIGHTSCROLL:
            ref_width = note->dimmen;
            ref_height = note->invdimmen;
            ref_invdimmen = note->invdimmen;
            break;
        default:
            return;
    }

    if (note->direction == ScrollDirection_LEFTSCROLL || note->direction == ScrollDirection_DOWNSCROLL) inverse = true;

    float alone_width = ref_width, alone_height = ref_height;

    // obtain the alone sprite original dimmensions
    if (state_alone)
        imgutils_get_statesprite_original_size(state_alone, &alone_width, &alone_height);
    else
        logger_warn("note_internal_resize_sprites() warning state_alone was NULL");

    // resize the sustain body sprites using the same scale as alone sprite
    float scale_factor;
    if (note->direction == ScrollDirection_UPSCROLL || note->direction == ScrollDirection_DOWNSCROLL) {
        scale_factor = ref_height / alone_height;
    } else {
        scale_factor = ref_width / alone_width;
    }


    //
    // resize sprites
    //

    if (state_alone) {
        float draw_width = ref_width, draw_height = ref_height;
        imgutils_calc_size(alone_width, alone_height, ref_width, ref_height, &draw_width, &draw_height);
        state_alone->draw_width = draw_width;
        state_alone->draw_height = draw_height;

        state_alone->offset_x = (ref_width - state_alone->draw_width) / 2.0f;
        state_alone->offset_y = (ref_height - state_alone->draw_height) / 2.0f;
    }

    if (state_hold) {
        note_internal_resize_sustain_part(state_hold, scale_factor);
        state_note->height_hold = state_hold->draw_height;
        state_note->offset_hold = (ref_invdimmen - state_hold->draw_width) / 2.0f;
        if (inverse)
            state_note->offset_hold = (state_note->offset_hold + state_hold->draw_width) * -1.0f;
    } else {
        state_note->height_hold = 0.0f;
        state_note->offset_hold = 0.0f;
    }

    if (state_tail) {
        note_internal_resize_sustain_part(state_tail, scale_factor);
        state_note->height_tail = state_tail->draw_height;
        state_note->offset_tail = (ref_invdimmen - state_tail->draw_width) / 2.0f;
        if (inverse)
            state_note->offset_tail = (state_note->offset_tail + state_tail->draw_width) * -1.0f;
    } else {
        state_note->height_tail = 0.0f;
        state_note->offset_tail = 0.0f;
    }
}

static void note_internal_resize_sustain_part(StateSpriteState* statesprite_state, float scale) {
    float applied_draw_width = 0.0f, applied_draw_height = 0.0f;
    // obtain the size of this part of the note
    imgutils_get_statesprite_original_size(statesprite_state, &applied_draw_width, &applied_draw_height);

    // keep at the same scale as sprite_alone
    statesprite_state->draw_width = applied_draw_width * scale;
    statesprite_state->draw_height = applied_draw_height * scale;
}

static void note_internal_destroy_internal_state(NoteState* internal_state) {
    if (internal_state->name) free_chk(internal_state->name);
    free_chk(internal_state);
}

static StateSpriteState* note_internal_get_statesprite_state(StateSprite statesprite, const char* state_name) {
    LinkedList states = statesprite_state_list(statesprite);
    foreach (StateSpriteState*, statesprite_state, LINKEDLIST_ITERATOR, states) {
        if (string_equals(statesprite_state->state_name, state_name))
            return statesprite_state;
    }
    return NULL;
}
