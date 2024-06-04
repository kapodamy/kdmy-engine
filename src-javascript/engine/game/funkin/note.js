"use strict";

const NOTE_SUFFIX_ALONE = "alone";
const NOTE_SUFFIX_HOLD = "hold";
const NOTE_SUFFIX_TAIL = "tail";
const NOTE_ALTERNATE_MINE = "mine";

//const NOTE_SUSTAIN_BODY_HEIGHT_USE_INTEGERS = 0;
const NOTE_SUSTAIN_BODY_OVELAPING_PIXELS = 0.5;
const NOTE_STRUCT_ELEMENTS_COUNT = 4;// (used in JS) [timestamp, note_id, duration, "reserved"]


/**
 * Chart scroll velocity (do not confuse with the song's chart speed).
 * In Funkin, Kade Engine and forks this velocity is 450px/s (pixel per seconds) using
 * a render resolution of 1280x720. But here is expressed in pixel per milliseconds for convenience.
 */
const FNF_CHART_SCROLL_VELOCITY = 450.0 / 1000.0;

/**
 * Minimal sustain duration for a note, if the duration does not reach
 * the threshold, the sustain hit check will be ignored.
 * This value is a ratio based on the arrow height
 */
const FNF_NOTE_MIN_SUSTAIN_THRESHOLD = 1.10;



function note_init(name, dimmen, invdimmen) {
    let note = {
        sprite_alone: statesprite_init_from_texture(null),
        sprite_hold: statesprite_init_from_texture(null),
        sprite_tail: statesprite_init_from_texture(null),

        current_dimmen: dimmen,
        dimmen: dimmen,
        invdimmen: invdimmen,

        // assign some default values
        dimmen_alone: dimmen,
        dimmen_alone_half: dimmen / 2,

        height_hold: 44,
        height_tail: 64,

        offset_hold: 15,
        offset_tail: 15,

        direction: STRUM_UPSCROLL,

        modifier_sustain: {},

        name: strdup(name),

        alpha_sustain: 1.0,
        alpha_alone: 1.0,

        internal_states: linkedlist_init(),
        current_state_name: Symbol,// Note: in C use an unique constant value

        has_part_alone: 0,
        has_part_hold: 0,
        has_part_tail: 0
    };

    pvr_context_helper_clear_modifier(note.modifier_sustain);

    statesprite_set_visible(note.sprite_alone, 0);
    statesprite_set_visible(note.sprite_hold, 0);
    statesprite_set_visible(note.sprite_tail, 0);

    return note;
}

function note_destroy(note) {
    statesprite_destroy(note.sprite_alone);
    statesprite_destroy(note.sprite_hold);
    statesprite_destroy(note.sprite_tail);

    linkedlist_destroy2(note.internal_states, note_internal_destroy_internal_state);

    // note: do not dispose "note.current_state_name" is part of "note.internal_states"

    note.name = null;
    //free(note);
}

function note_set_scoll_direction(note, direction) {
    switch (direction) {
        case STRUM_LEFTSCROLL:
            note.modifier_sustain.rotate = -90;
            break;
        case STRUM_RIGHTSCROLL:
            note.modifier_sustain.rotate = 90;
            break;
        case STRUM_DOWNSCROLL:
            note.modifier_sustain.rotate = 180;
            break;
        case STRUM_UPSCROLL:
            note.modifier_sustain.rotate = 0;
            break;
        default:
            return;
    }

    switch (direction) {
        case STRUM_LEFTSCROLL:
        case STRUM_RIGHTSCROLL:
            note.current_dimmen = note.invdimmen;
            break;
        case STRUM_DOWNSCROLL:
        case STRUM_UPSCROLL:
            note.current_dimmen = note.dimmen;
            break;
    }

    note.direction = direction;

    if (direction != STRUM_UPSCROLL) {
        note.modifier_sustain.rotate *= MATH2D_DEG_TO_RAD;
        note.modifier_sustain.rotate_pivot_enabled = 1;
        note.modifier_sustain.rotate_pivot_u = 0;
        note.modifier_sustain.rotate_pivot_v = 0;
        note.modifier_sustain.width = 0;
        note.modifier_sustain.height = 0;
        note.modifier_sustain.translate_rotation = 1;
    }

    // recalculate all sprite states
    for (let state_note of linkedlist_iterate4(note.internal_states)) {
        let state_alone = note_internal_get_statesprite_state(note.sprite_alone, state_note.name);
        let state_hold = note_internal_get_statesprite_state(note.sprite_hold, state_note.name);
        let state_tail = note_internal_get_statesprite_state(note.sprite_tail, state_note.name);
        note_internal_resize_sprites(note, state_note, state_alone, state_hold, state_tail);
    }

    for (let note_state of linkedlist_iterate4(note.internal_states)) {
        if (note_state.name == note.current_state_name) {
            note.height_hold = note_state.height_hold;
            note.offset_hold = note_state.offset_hold;
            note.height_tail = note_state.height_tail;
            note.offset_tail = note_state.offset_tail;
            break;
        }
    }
}

function note_set_alpha(note, alpha) {
    note_set_alpha_alone(note, alpha);
    note_set_alpha_sustain(note, alpha);
}

function note_set_alpha_alone(note, alpha) {
    note.alpha_alone = alpha;
}

function note_set_alpha_sustain(note, alpha) {
    note.alpha_sustain = alpha;
}

function note_animate(note, elapsed) {
    let res = 0;
    if (note.sprite_alone) res += statesprite_animate(note.sprite_alone, elapsed);
    if (note.sprite_hold) res += statesprite_animate(note.sprite_hold, elapsed);
    if (note.sprite_tail) res += statesprite_animate(note.sprite_tail, elapsed);
    return res;
}

function note_get_name(note) {
    return note.name;// in C return the string pointer
}

function note_draw(note, pvrctx, scroll_velocity, x, y, duration_ms, only_body) {
    //
    // Note:
    //      Scroll velocity should be according to the UI layout viewport (or the screen resolution)
    //      and the song chart speed should be already applied
    //
    let length = Math.abs(duration_ms * scroll_velocity);// note length in pixels
    let sustain_length = length - note.dimmen_alone;

    // save PVR context and apply the modifier to alone, hold and tail sprites
    pvr_context_save(pvrctx);

    let dimmen_alone_half = note.has_part_alone ? note.dimmen_alone_half : 0.0;

    // draw the sustain body (hold & tail)
    if (sustain_length > 0 && note.has_part_hold && note.has_part_tail) {
        if (note.direction != STRUM_UPSCROLL) {
            switch (note.direction) {
                case STRUM_DOWNSCROLL:
                    y -= sustain_length;
                    break;
                case STRUM_RIGHTSCROLL:
                    x -= sustain_length;
                    break;
            }
            note.modifier_sustain.x = x;
            note.modifier_sustain.y = y;
        }

        let draw_x = x + note.offset_hold;
        let draw_y = y;
        let tail_correction;
        let height_hold = note.height_hold;
        let height_tail = note.height_tail;

        switch (note.direction) {
            case STRUM_RIGHTSCROLL:
            case STRUM_DOWNSCROLL:
                draw_y -= sustain_length + dimmen_alone_half;
                tail_correction = 0;
                break;
            default:
                draw_y += dimmen_alone_half;
                tail_correction = length;
                break;
        }

        // crop sprite_tail if necessary
        let hold_length = sustain_length + dimmen_alone_half;
        if (hold_length < height_tail) {
            // crop the top part of the sprite
            let offset = height_tail - hold_length;
            height_tail = hold_length;
            statesprite_crop_enable(note.sprite_tail, 1);
            statesprite_crop(note.sprite_tail, 0, offset, -1, -1);
        }

        // reserve space for the sprite_tail
        hold_length -= height_tail;
        // Rendering workaround to avoid gaps
        hold_length += NOTE_SUSTAIN_BODY_OVELAPING_PIXELS;
        height_hold -= NOTE_SUSTAIN_BODY_OVELAPING_PIXELS;

        pvr_context_save(pvrctx);
        pvr_context_apply_modifier(pvrctx, note.modifier_sustain);
        pvrctx.global_alpha = note.alpha_sustain;

        while (hold_length > height_hold) {
            statesprite_set_draw_location(note.sprite_hold, draw_x, draw_y);
            statesprite_draw(note.sprite_hold, pvrctx);
            draw_y += height_hold;
            hold_length -= height_hold;
        }

        if (hold_length > 0) {
            // crop the bottom part of sprite_hold and draw it
            statesprite_crop_enable(note.sprite_hold, 1);
            statesprite_crop(note.sprite_hold, 0, 0, -1, hold_length);

            statesprite_set_draw_location(note.sprite_hold, draw_x, draw_y);
            statesprite_draw(note.sprite_hold, pvrctx);
        }

        // draw sprite_tail
        draw_x = x + note.offset_tail;
        draw_y = y + tail_correction - height_tail;
        statesprite_set_draw_location(note.sprite_tail, draw_x, draw_y);
        statesprite_draw(note.sprite_tail, pvrctx);

        // disable crop sustain body (hold & tail)
        statesprite_crop_enable(note.sprite_hold, 0);
        statesprite_crop_enable(note.sprite_tail, 0);

        pvr_context_restore(pvrctx);
    }

    // draw sprite_alone
    if (!only_body && note.has_part_alone) {
        if (sustain_length > 0.0) {
            if (note.direction == STRUM_DOWNSCROLL)
                y += sustain_length;
            else if (note.direction == STRUM_RIGHTSCROLL)
                x += sustain_length;
        }

        pvrctx.global_alpha = note.alpha_alone;
        statesprite_set_draw_location(note.sprite_alone, x, y);
        statesprite_draw(note.sprite_alone, pvrctx);
    }

    pvr_context_restore(pvrctx);
}


function note_state_add(note, modelholder, state_name) {
    //
    // build every part of the note
    // all sprites are hidden to avoid getting drawn by the PVR graphics backend
    // we draw those sprites manually in note_draw()
    //

    let state_alone = note_internal_load_part(
        note.sprite_alone, note.name, NOTE_SUFFIX_ALONE, modelholder, state_name
    );
    let state_hold = note_internal_load_part(
        note.sprite_hold, note.name, NOTE_SUFFIX_HOLD, modelholder, state_name
    );
    let state_tail = note_internal_load_part(
        note.sprite_tail, note.name, NOTE_SUFFIX_TAIL, modelholder, state_name
    );

    if (!state_alone && !state_hold && !state_tail) return 0;

    let state_note = { name: null, height_hold: 0, offset_hold: 0, height_tail: 0, offset_tail: 0 };
    if (state_name != null) state_note.name = strdup(state_name);

    linkedlist_add_item(note.internal_states, state_note);
    note_internal_resize_sprites(note, state_note, state_alone, state_hold, state_tail);

    return 1;
}

function note_state_toggle(note, state_name) {
    if (note.current_state_name === state_name) return 4;

    let res = 0;

    for (let note_state of linkedlist_iterate4(note.internal_states)) {
        if (note_state.name == state_name) {
            note.height_hold = note_state.height_hold;
            note.offset_hold = note_state.offset_hold;
            note.height_tail = note_state.height_tail;
            note.offset_tail = note_state.offset_tail;
            note.current_state_name = note_state.name;
            res = 1;
            break;
        }
    }

    if (res < 1) return 0;

    res += note.has_part_alone = statesprite_state_toggle(note.sprite_alone, state_name);
    res += note.has_part_hold = statesprite_state_toggle(note.sprite_hold, state_name);
    res += note.has_part_tail = statesprite_state_toggle(note.sprite_tail, state_name);

    return res;
}

function note_peek_alone_statesprite(note) {
    return note.sprite_alone;
}


function note_internal_load_part(part, name, suffix, modelholder, state_name) {
    let atlas = modelholder_get_atlas(modelholder);
    let animlist = modelholder_get_animlist(modelholder);
    let texture = modelholder_get_texture(modelholder, 0);
    let rgb8_color = modelholder_get_vertex_color(modelholder);

    let animation_name = "";
    if (name) animation_name = strdup(name);
    if (suffix) animation_name += ` ${suffix}`;
    if (state_name) animation_name += ` ${state_name}`;


    //
    // Attempt to load the arrow part (texture, frame & animation)
    //
    let animsprite;

    // in C use "goto L_build_state;" instead of "break L_load;"
    L_load: {
        // read the animation from the animlist (if necessary)
        if (animlist) {
            let animlist_item = animlist_get_animation(animlist, animation_name);
            if (animlist_item) {
                animsprite = animsprite_init(animlist_item);
                break L_load;
            }
        }

        // fallback, build the animation using atlas frames directly
        let framerate = atlas_get_glyph_fps(atlas);
        if (framerate <= 0.0) framerate = FUNKIN_DEFAULT_ANIMATIONS_FRAMERATE;

        animsprite = animsprite_init_from_atlas(framerate, 0, atlas, animation_name, 1);

        if (animsprite)
            break L_load;

        // the arrow is not animated, build an static animation
        let atlas_entry = atlas_get_entry(atlas, animation_name);
        if (atlas_entry) {
            animsprite = animsprite_init_from_atlas_entry(atlas_entry, 1, framerate);
            break L_load;
        }

        /*if (state_name) {
            animation_name = undefined;

            // try again but without the state name, can fail if the texture supplied is custom
            // with a hold duration (alone + hold + tail)
            let statesprite_state = note_internal_create_arrow_part(
                part, name, suffix, atlas, animlist, texture, rgb8_color, null
            );

            // set the state name (null was used, before)
            statesprite_state.state_name = strdup(state_name);

            return statesprite_state;
        }*/

        // this part of the arrow is incomplete
        animsprite = null;
    }


    //L_build_state:
    if (!animsprite) {
        animation_name = undefined;
        return null;
    }

    let atlas_entry = atlas_get_entry_with_number_suffix(atlas, animation_name);
    if (!atlas_entry) atlas_entry = atlas_get_entry(atlas, animation_name);

    animation_name = undefined;

    let state = statesprite_state_add2(part, texture, animsprite, atlas_entry, rgb8_color, state_name);
    if (!state && animsprite) animsprite_destroy(animsprite);

    return state;
}

function note_internal_resize_sprites(note, state_note, state_alone, state_hold, state_tail) {
    //
    // confuse part, set the desired width and height of the alone sprite
    //
    let ref_width, ref_height, ref_invdimmen;
    let inverse = 0;

    switch (note.direction) {
        case STRUM_UPSCROLL:
        case STRUM_DOWNSCROLL:
            ref_width = note.invdimmen;
            ref_height = note.dimmen;
            ref_invdimmen = note.invdimmen;
            break;
        case STRUM_LEFTSCROLL:
        case STRUM_RIGHTSCROLL:
            ref_width = note.dimmen;
            ref_height = note.invdimmen;
            ref_invdimmen = note.invdimmen;
            break;
        default:
            return;
    }

    if (note.direction == STRUM_LEFTSCROLL || note.direction == STRUM_DOWNSCROLL) inverse = 1;

    let alone_size = [ref_width, ref_height];

    // obtain the alone sprite original dimmensions
    if (state_alone)
        imgutils_get_statesprite_original_size(state_alone, alone_size);
    else
        console.warn("note_internal_resize_sprites() warning state_alone was NULL");

    // resize the sustain body sprites using the same scale as alone sprite
    let scale_factor;
    if (note.direction == STRUM_UPSCROLL || note.direction == STRUM_DOWNSCROLL) {
        scale_factor = ref_height / alone_size[1];
    } else {
        scale_factor = ref_width / alone_size[0];
    }


    //
    // resize sprites
    //

    if (state_alone) {
        let draw_size = [ref_width, ref_height];
        imgutils_calc_size(alone_size[0], alone_size[1], ref_width, ref_height, draw_size);
        state_alone.draw_width = draw_size[0];
        state_alone.draw_height = draw_size[1];

        state_alone.offset_x = (ref_width - state_alone.draw_width) / 2;
        state_alone.offset_y = (ref_height - state_alone.draw_height) / 2;
    }

    if (state_hold) {
        note_internal_resize_sustain_part(state_hold, scale_factor);
        state_note.height_hold = state_hold.draw_height;
        state_note.offset_hold = (ref_invdimmen - state_hold.draw_width) / 2.0;
        if (inverse)
            state_note.offset_hold = (state_note.offset_hold + state_hold.draw_width) * -1;
    } else {
        state_note.height_hold = 0;
        state_note.offset_hold = 0;
    }

    if (state_tail) {
        note_internal_resize_sustain_part(state_tail, scale_factor);
        state_note.height_tail = state_tail.draw_height;
        state_note.offset_tail = (ref_invdimmen - state_tail.draw_width) / 2.0;
        if (inverse)
            state_note.offset_tail = (state_note.offset_tail + state_tail.draw_width) * -1;
    } else {
        state_note.height_tail = 0;
        state_note.offset_tail = 0;
    }
}

function note_internal_resize_sustain_part(statesprite_state, scale) {
    const applied_draw_size = [0, 0];
    // obtain the size of this part of the note
    imgutils_get_statesprite_original_size(statesprite_state, applied_draw_size);

    // keep at the same scale as sprite_alone
    statesprite_state.draw_width = applied_draw_size[0] * scale;
    statesprite_state.draw_height = applied_draw_size[1] * scale;
}

function note_internal_destroy_internal_state(internal_state) {
    if (internal_state.name) internal_state.name = undefined;
    internal_state = undefined;
}

function note_internal_get_statesprite_state(statesprite, state_name) {
    let states = statesprite_state_list(statesprite);
    for (let statesprite_state of linkedlist_iterate4(states)) {
        if (statesprite_state.state_name == state_name)
            return statesprite_state;
    }
    return null;
}

