"use strict";

const CHARACTER_ROLLBACK_SPEED = 8.0;

/** @readonly @enum {number} */
const CharacterActionType = {
    NONE: 0,
    MISS: 1,
    SING: 2,
    EXTRA: 3,
    IDLE: 4
}

/** @readonly @enum {number} */
const CharacterAnimType = {
    BASE: 0,
    HOLD: 1,
    ROLLBACK: 2
}


async function character_init(charactermanifest) {
    const import_context = {
        all_directions_names: arraylist_init(),
        all_extras_names: arraylist_init(),
        charactermanifest: charactermanifest,
        modelholder_arraylist: arraylist_init2(4),
        textures: arraylist_init2(4),
        states: arraylist_init2(4)
    };

    await character_internal_get_modelholder(
        import_context.modelholder_arraylist, charactermanifest.model_character, 0
    );


    let character = {
        // DEBUG ONLY
        //name: charactermanifest.model_character,
        //is_bf: charactermanifest.model_character.includes("BOYFRIEND"),

        statesprite: statesprite_init_from_texture(null),

        current_texture_id: -1,
        textures: import_context.textures,

        all_extras_names_size: -1, all_extras_names: null,
        all_directions_names_size: -1, all_directions_names: null,

        states: import_context.states,
        default_state: null,
        current_state: null,

        current_action_extra: null,
        current_action_miss: null,
        current_action_sing: null,

        current_anim_type: CharacterAnimType.BASE,
        current_action_type: CharacterActionType.NONE,
        current_stop_on_beat: -1,
        current_anim: null,
        current_use_frame_rollback: 0,
        current_sing_follow_hold: 0,
        current_sing_non_sustain: 0,

        alt_enabled: 0,
        continuous_idle: charactermanifest.continuous_idle,

        beatwatcher: { count: 0, count_beats_or_quarters: 0, drift_count: 0, drift_timestamp: 0, since: 0, tick: 0 },

        idle_speed: 1.0,
        allow_speed_change: charactermanifest.actions_apply_chart_speed,

        drawable: null,
        drawable_animation: null,

        draw_x: 0, draw_y: 1,
        align_vertical: charactermanifest.align_vertical,
        align_horizontal: charactermanifest.align_horizontal,

        manifest_align_vertical: charactermanifest.align_vertical,
        manifest_align_horizontal: charactermanifest.align_horizontal,

        reference_width: 0, reference_height: 0,
        enable_reference_size: 0,

        offset_x: charactermanifest.offset_x, offset_y: charactermanifest.offset_y,

        is_left_facing: charactermanifest.left_facing,

        inverted_enabled: 0,

        inverted_size: -1,
        inverted_from: null,
        inverted_to: null,

        character_scale: 1.0,
        played_actions_count: 0,
        animation_freezed: 0
    };

    beatwatcher_reset(character.beatwatcher, 1, 100);

    if (charactermanifest.has_reference_size) {
        character.reference_width = charactermanifest.reference_width;
        character.reference_height = charactermanifest.reference_height;
    }

    statesprite_set_visible(character.statesprite, 0);
    statesprite_flip_texture_enable_correction(character.statesprite, 0);

    // import default actions
    character.default_state = await character_internal_import_actions(import_context, charactermanifest.actions, null, null);
    character.current_state = character.default_state;

    // import additional actions as states
    for (let i = 0; i < charactermanifest.additional_states_size; i++) {
        let additionalstate = charactermanifest.additional_states[i];
        await character_internal_import_actions(import_context, additionalstate.actions, additionalstate.model, additionalstate.name);
    }

    character.drawable = drawable_init(0, character, character_draw, character_animate);

    arraylist_destroy3(import_context.modelholder_arraylist, character_internal_destroy_modelholder);
    arraylist_destroy2(import_context.all_directions_names, character, "all_directions_names_size", "all_directions_names");
    arraylist_destroy2(import_context.all_extras_names, character, "all_extras_names_size", "all_extras_names");

    character.inverted_size = charactermanifest.opposite_directions.sizes;
    if (character.inverted_size > 0 && charactermanifest.actions.sing_size > 0) {
        character_internal_import_opposite_dir(
            character, "inverted_from", charactermanifest.opposite_directions.from
        );
        character_internal_import_opposite_dir(
            character, "inverted_to", charactermanifest.opposite_directions.to
        );
    }

    character_state_toggle(character, null);
    character_play_idle(character);
    //character_face_as_opponent(character, character.is_left_facing);

    return character;
}

function character_destroy(character) {
    ModuleLuaScript.kdmyEngine_drop_shared_object(character);

    statesprite_destroy(character.statesprite);

    let states_size = arraylist_size(character.states);

    for (let i = 0; i < states_size; i++) {
        let state = arraylist_get(character.states, i);

        for (let j = 0; j < state.sing_size; j++) {
            if (state.sing[j].base) animsprite_destroy(state.sing[j].base);
            if (state.sing[j].hold) animsprite_destroy(state.sing[j].hold);
            if (state.sing[j].rollback) animsprite_destroy(state.sing[j].rollback);

            if (state.sing_alt[j].base) animsprite_destroy(state.sing_alt[j].base);
            if (state.sing_alt[j].hold) animsprite_destroy(state.sing_alt[j].hold);
            if (state.sing_alt[j].rollback) animsprite_destroy(state.sing_alt[j].rollback);
        }

        for (let j = 0; j < state.miss_size; j++) {
            if (state.miss[j].animation) animsprite_destroy(state.miss[j].animation);
        }

        for (let j = 0; j < state.extras_size; j++) {
            if (state.extras[j].base) animsprite_destroy(state.extras[j].base);
            if (state.extras[j].hold) animsprite_destroy(state.extras[j].hold);
            if (state.extras[j].rollback) animsprite_destroy(state.extras[j].rollback);
        }

        if (state.hey.base) animsprite_destroy(state.hey.base);
        if (state.hey.hold) animsprite_destroy(state.hey.hold);
        if (state.hey.rollback) animsprite_destroy(state.hey.rollback);

        if (state.idle.base) animsprite_destroy(state.idle.base);
        if (state.idle.hold) animsprite_destroy(state.idle.hold);
        if (state.idle.rollback) animsprite_destroy(state.idle.rollback);

        if (state.idle_alt.base) animsprite_destroy(state.idle_alt.base);
        if (state.idle_alt.hold) animsprite_destroy(state.idle_alt.hold);
        if (state.idle_alt.rollback) animsprite_destroy(state.idle_alt.rollback);

        state.name = undefined;
        state.sing = undefined;
        state.sing_alt = undefined;
        state.miss = undefined;
        state.extras = undefined;
        state = undefined;
    }

    arraylist_destroy(character.states, 0);

    for (let i = 0; i < character.all_directions_names_size; i++) {
        character.all_directions_names[i] = undefined;
    }
    for (let i = 0; i < character.all_extras_names_size; i++) {
        character.all_extras_names[i] = undefined;
    }

    drawable_destroy(character.drawable);
    if (character.drawable_animation) animsprite_destroy(character.drawable_animation);

    if (character.inverted_size > 0) {
        character.inverted_from = undefined;
        character.inverted_to = undefined;
    }

    character.all_directions_names = undefined;
    character.all_extras_names = undefined;

    for (let texture_info of arraylist_iterate4(character.textures))
        texture_destroy(texture_info.texture);
    arraylist_destroy(character.textures, 0);

    character = undefined;
}


function character_use_alternate_sing_animations(character, enable) {
    character.alt_enabled = !!enable;
}

function character_set_draw_location(character, x, y) {
    let modifier = drawable_get_modifier(character.drawable);
    if (Number.isFinite(x)) character.draw_x = modifier.x = x;
    if (Number.isFinite(y)) character.draw_y = modifier.y = y;
    character_internal_calculate_location(character);
}

function character_set_draw_align(character, align_vertical, align_horizontal) {
    if (align_vertical == ALIGN_BOTH || align_vertical == ALIGN_NONE)
        character.align_vertical = character.manifest_align_vertical;
    else
        character.align_vertical = align_vertical;

    if (align_horizontal == ALIGN_BOTH || align_horizontal == ALIGN_NONE)
        character.align_horizontal = character.manifest_align_horizontal;
    else
        character.align_horizontal = align_horizontal;

    character_internal_calculate_location(character);
}

function character_update_reference_size(character, width, height) {
    if (width >= 0) character.reference_width = width;
    if (height >= 0) character.reference_height = height;
    character_internal_calculate_location(character);
}

function character_enable_reference_size(character, enable) {
    character.enable_reference_size = !!enable;
    character_internal_calculate_location(character);
}

function character_set_offset(character, offset_x, offset_y) {
    character.offset_x = offset_x;
    character.offset_y = offset_y;
}


function character_state_add(character, modelholder, state_name) {
    //
    // expensive operation, call it before any gameplay
    //
    if (!modelholder) return 0;

    let states_size = arraylist_size(character.states);
    for (let i = 0; i < states_size; i++) {
        let existing_state = arraylist_get(character.states, i);
        if (existing_state.name == state_name) return 0;
    }

    let id_texture = character_internal_add_texture(character.textures, modelholder);
    let default_state = arraylist_get(character.states, 0);
    let state = character_internal_state_create(
        state_name, default_state.sing_size, default_state.miss_size, default_state.extras_size
    );


    for (let i = 0; i < default_state.sing_size; i++) {
        state.sing[i].id_texture = id_texture;
        character_internal_state_of_sing(state.sing[i], modelholder, state_name, default_state.sing[i]);

        state.sing_alt[i].id_texture = id_texture;
        character_internal_state_of_sing(state.sing_alt[i], modelholder, state_name, default_state.sing_alt[i]);
    }

    for (let i = 0; i < default_state.miss_size; i++) {
        let animation = character_internal_import_animation3(modelholder, state_name, default_state.miss[i].animation, 0);

        state.miss[i].id_direction = default_state.miss[i].id_direction;
        state.miss[i].stop_after_beats = default_state.miss[i].stop_after_beats;
        state.miss[i].animation = animation;
        state.miss[i].id_texture = id_texture;
        state.miss[i].offset_x = default_state.miss[i].offset_x;
        state.miss[i].offset_y = default_state.miss[i].offset_y;
    }

    for (let i = 0; i < default_state.extras_size; i++) {
        state.extras[i].id_texture = id_texture;
        character_internal_state_of_extra(state.extras[i], modelholder, state_name, default_state.extras[i]);
    }

    state.hey.id_texture = id_texture;
    character_internal_state_of_extra(state.hey, modelholder, state_name, default_state.hey);

    state.idle.id_texture = id_texture;
    character_internal_state_of_extra(state.idle, modelholder, state_name, default_state.idle);

    return 1;
}

function character_state_toggle(character, state_name) {
    let states_size = arraylist_size(character.states);

    for (let i = 0; i < states_size; i++) {
        let state = arraylist_get(character.states, i);
        if (state.name == state_name) {
            character.current_state = state;
            character_internal_update_texture(character);
            return 1;
        }
    }

    return 0;
}


function character_play_hey(character) {
    console.assert(character.current_state != null, "character.current_state was NULL");

    let extra_info = character.current_state.hey;

    if (!extra_info.is_valid && character.current_state != character.default_state) {
        // attempt use default state
        extra_info = character.default_state.hey;
    }

    if (!extra_info.is_valid) {
        character_internal_fallback_idle(character);
        return 0;
    }

    // end current action
    character_internal_end_current_action(character);

    if (extra_info.base) {
        character.current_anim = extra_info.base;
        character.current_anim_type = CharacterAnimType.BASE;
    } else {
        character.current_anim = extra_info.hold;
        character.current_anim_type = CharacterAnimType.HOLD;
    }

    animsprite_restart(character.current_anim);

    character.current_action_extra = extra_info;
    character.current_action_type = CharacterActionType.EXTRA;
    character.current_use_frame_rollback = 0;

    character_internal_set_beat_stop(character, extra_info.stop_after_beats);

    character_internal_update_texture(character);
    character.played_actions_count++;

    return 1;
}

function character_play_idle(character) {
    console.assert(character.current_state != null, "character.current_state was NULL");

    character.played_actions_count++;

    // rollback the current action (if possible)
    switch (character.current_action_type) {
        case CharacterActionType.SING:
            switch (character.current_anim_type) {
                case CharacterAnimType.BASE:
                    character.current_sing_follow_hold = 0;
                    return 2;
                case CharacterAnimType.HOLD:
                    if (character.current_action_sing.hold_can_rollback) {
                        character.current_use_frame_rollback = character.current_action_sing.hold_can_rollback;
                        return 2;
                    } else if (character.current_action_sing.rollback) {
                        character.current_anim = character.current_action_sing.rollback;
                        animsprite_restart(character.current_anim);
                        return 2;
                    }
                    break;
            }
            break;
        case CharacterActionType.MISS:
            if (character.current_stop_on_beat >= 0) return 2;
            break;
        case CharacterActionType.EXTRA:
        case CharacterActionType.IDLE:
            break;
    }

    let state = character.current_state;
    let extra_info = null;

    L_read_state: for (; ;) {
        extra_info = state.idle;
        if (character.alt_enabled && state.idle_alt.is_valid) extra_info = state.idle_alt;

        if (!extra_info.is_valid) {
            if (state != character.default_state) {
                // attempt use default state
                state = character.default_state;
                continue L_read_state;
            }
            character.played_actions_count--;
            return 0;
        }

        break;// JS only
    }

    if (character.current_action_type != CharacterActionType.IDLE) {
        // end current action
        character_internal_end_current_action(character);
    }

    if (extra_info.base) {
        character.current_anim = extra_info.base;
        character.current_anim_type = CharacterAnimType.BASE;
    } else {
        character.current_anim = extra_info.hold;
        character.current_anim_type = CharacterAnimType.HOLD;
    }

    animsprite_restart(character.current_anim);

    character.current_action_extra = extra_info;
    character.current_action_type = CharacterActionType.IDLE;
    character.current_use_frame_rollback = 0;
    character.current_stop_on_beat = -1;// extra_info.stop_after_beats ignored

    character_internal_update_texture(character);

    return 1;
}

function character_play_sing(character, direction, prefer_sustain) {
    console.assert(character.current_state != null, "character.current_state was NULL");

    let id_direction = character_internal_get_direction_id(character, direction);
    if (id_direction < 0) {
        // unknown direction
        character_internal_fallback_idle(character);
        return 0;
    };

    let sing_info = null;
    let state = character.current_state;

    L_read_state: for (; ;) {
        let array = character.alt_enabled ? state.sing_alt : state.sing;
        for (let i = 0; i < state.sing_size; i++) {
            if (array[i].id_direction == id_direction) {
                sing_info = array[i];
                break;
            }
        }

        if (!sing_info || (!sing_info.base && !sing_info.hold)) {
            // attempt to use the non-alt sing direction
            if (character.alt_enabled) {
                for (let i = 0; i < state.sing_size; i++) {
                    if (state.sing[i].id_direction == id_direction) {
                        sing_info = state.sing[i];
                        break;
                    }
                }
            }

            if (!sing_info || (!sing_info.base && !sing_info.hold)) {
                if (state != character.default_state) {
                    // attempt use default state
                    state = character.default_state;
                    continue L_read_state;
                }
                //throw new Error("unknown sing direction: " + direction);
                character_internal_fallback_idle(character);
                return 0;
            }
        }

        break;// JS only
    }

    // end current action
    character_internal_end_current_action(character);

    let base_used;
    if (prefer_sustain) {
        base_used = sing_info.full_sustain;
        character.current_anim = sing_info.full_sustain ? sing_info.base : sing_info.hold;
    } else {
        base_used = 1;
        character.current_anim = sing_info.base;
    }

    // check if the current animation is not present
    if (!character.current_anim) {
        character.current_anim = base_used ? sing_info.hold : sing_info.base;
        base_used = !base_used;
    }

    animsprite_restart(character.current_anim);

    character.current_action_sing = sing_info;
    character.current_action_type = CharacterActionType.SING;
    character.current_anim_type = prefer_sustain ? CharacterAnimType.HOLD : CharacterAnimType.BASE;
    character.current_use_frame_rollback = 0;
    character.current_stop_on_beat = prefer_sustain ? -1 : (character.beatwatcher.count + 2);

    // specific sing action fields
    character.current_sing_follow_hold = prefer_sustain ? 0 : sing_info.follow_hold;
    character.current_sing_non_sustain = !prefer_sustain;

    character_internal_update_texture(character);
    character.played_actions_count++;

    return 1;
}

function character_play_miss(character, direction, keep_in_hold) {
    console.assert(character.current_state != null, "character.current_state was NULL");

    let id_direction = character_internal_get_direction_id(character, direction);
    if (id_direction < 0) {
        // unknown direction
        character_internal_fallback_idle(character);
        return 0;
    };

    let miss_info = null;
    let state = character.current_state;

    L_read_state: for (; ;) {
        for (let i = 0; i < state.miss_size; i++) {
            if (state.miss[i].id_direction == id_direction) {
                miss_info = state.miss[i];
                break;
            }
        }

        if (!miss_info || !miss_info.animation) {
            if (state != character.default_state) {
                // attempt use default state
                state = character.default_state;
                continue L_read_state;
            }
            character_internal_fallback_idle(character);
            return 0;
        }

        break;// JS only
    }

    // end current action
    character_internal_end_current_action(character);

    if (character.current_action_type == CharacterActionType.MISS && miss_info == character.current_action_miss) {
        character_internal_set_beat_stop(character, keep_in_hold ? -1 : miss_info.stop_after_beats);

        // do not replay this action
        return 2;
    }

    animsprite_restart(miss_info.animation);

    character.current_anim = miss_info.animation;
    character.current_action_type = CharacterActionType.MISS;
    character.current_action_miss = miss_info;
    character.current_use_frame_rollback = 0;

    character_internal_set_beat_stop(character, keep_in_hold ? -1 : miss_info.stop_after_beats);

    character_internal_update_texture(character);
    character.played_actions_count++;

    return 1;
}

function character_play_extra(character, extra_animation_name, prefer_sustain) {
    console.assert(character.current_state != null, "character.current_state was NULL");

    let id_extra = character_internal_get_extra_id(character, extra_animation_name);
    if (id_extra < 0) {
        // unknown extra
        character_internal_fallback_idle(character);
        return 0;
    }

    let state = character.current_state;
    let extra_info = null;
    L_read_state: for (; ;) {
        for (let i = 0; i < state.extras_size; i++) {
            if (state.extras[i].id_extra == id_extra) {
                extra_info = state.extras[i];
                break;
            }
        }

        if (!extra_info || !extra_info.is_valid) {
            if (state != character.default_state) {
                // attempt use default state
                state = character.default_state;
                continue L_read_state;
            }
            character_internal_fallback_idle(character);
            return 0;
        }

        break;// JS only
    }

    // end current action
    character_internal_end_current_action(character);

    if ((extra_info.hold && prefer_sustain) || (!extra_info.base && extra_info.hold)) {
        character.current_anim = extra_info.hold;
        character.current_anim_type = CharacterAnimType.HOLD;
    } else {
        character.current_anim = extra_info.base;
        character.current_anim_type = CharacterAnimType.BASE;
    }

    animsprite_restart(character.current_anim);

    character.current_action_extra = extra_info;
    character.current_action_type = CharacterActionType.EXTRA;
    character.current_use_frame_rollback = 0;

    character_internal_set_beat_stop(character, extra_info.stop_after_beats);

    character_internal_update_texture(character);
    character.played_actions_count++;

    return 1;
}


function character_set_bpm(character, beats_per_minute) {
    beatwatcher_change_bpm(character.beatwatcher, beats_per_minute);
}

function character_set_idle_speed(character, speed) {
    if (character.allow_speed_change) character.idle_speed = speed;
}

function character_set_scale(character, scale_factor) {
    character.character_scale = scale_factor;
    character_internal_update_texture(character);
}

function character_reset(character) {
    beatwatcher_reset(character.beatwatcher, 1, 100);

    character.idle_speed = 1.0;
    character.current_action_type = CharacterActionType.NONE;
    character.current_stop_on_beat = -1;
    character.alt_enabled = 0;

    drawable_set_antialiasing(character.drawable, PVR_FLAG_DEFAULT);

    let modifier = drawable_get_modifier(character.drawable);
    pvrctx_helper_clear_modifier(modifier);
    modifier.x = character.draw_x;
    modifier.y = character.draw_y;

    // switch to the default state
    character_state_toggle(character, null);

    character_play_idle(character);
}

function character_enable_continuous_idle(character, enable) {
    character.continuous_idle = enable;
}

function character_is_idle_active(character) {
    return character.current_action_type == CharacterActionType.IDLE;
}

function character_enable_flip_correction(character, enable) {
    statesprite_flip_texture_enable_correction(character.statesprite, enable);
}

function character_flip_orientation(character, enable) {
    character.inverted_enabled = !!enable;
    statesprite_flip_texture(character.statesprite, character.inverted_enabled, null);
}

function character_face_as_opponent(character, face_as_opponent) {
    let flip_x;
    if (face_as_opponent)
        flip_x = character.is_left_facing;
    else
        flip_x = !character.is_left_facing;

    character.inverted_enabled = character.inverted_size > 0 && flip_x;
    statesprite_flip_texture(character.statesprite, flip_x, null);
}



function character_animate(character, elapsed) {
    beatwatcher_poll(character.beatwatcher);

    if (character.drawable_animation != null) {
        animsprite_animate(character.drawable_animation, elapsed);
        animsprite_update_drawable(character.drawable_animation, character.drawable, 1);
    }

    if (character.animation_freezed) {
        if (character.current_use_frame_rollback)
            animsprite_rollback(character.current_anim, elapsed * CHARACTER_ROLLBACK_SPEED);
        else
            animsprite_animate(character.current_anim, elapsed);

        animsprite_update_statesprite(character.current_anim, character.statesprite, 1);
        character_internal_calculate_location(character);
        return 1;
    }

    // required to compute trailing fx
    statesprite_animate(character.statesprite, elapsed);

    let completed;
    const current_action_type = character.current_action_type;
    let has_beat_stop = character.beatwatcher.count >= character.current_stop_on_beat;

    if (!character.current_anim) {
        if (character.current_stop_on_beat < 0) return 1;
        if (has_beat_stop) {
            character.current_action_type = CharacterActionType.NONE;
            character_play_idle(character);
            return 1;
        }
        return 0;
    }

    if (current_action_type == CharacterActionType.NONE) return 1;
    if (current_action_type == CharacterActionType.IDLE && character.idle_speed != 1.0) {
        elapsed *= character.idle_speed;
    }

    if (character.current_use_frame_rollback)
        completed = animsprite_rollback(character.current_anim, elapsed * CHARACTER_ROLLBACK_SPEED);
    else
        completed = animsprite_animate(character.current_anim, elapsed);

    animsprite_update_statesprite(character.current_anim, character.statesprite, 1);
    character_internal_calculate_location(character);

    if (!completed) {
        let check_beat_stop = character.current_stop_on_beat >= 0;
        if (check_beat_stop && has_beat_stop && current_action_type != CharacterActionType.SING) {
            completed = 1;
        } else if (has_beat_stop && current_action_type == CharacterActionType.SING && character.current_sing_non_sustain) {
            //
            // In non-sustain sing actions, beat stops ends the
            // action. This can no be the expected behaviour, but
            // base sing animations have to be shorter than a beat.
            //
            completed = 1;
        } else {
            // wait until the current action animation is completed
            return 1;
        }
    }

    if (character.continuous_idle && current_action_type == CharacterActionType.IDLE) {
        // follow hold animation (if exists)
        if (character.current_anim_type == CharacterAnimType.BASE && character.current_action_extra.hold) {
            character.current_anim = character.current_action_extra.hold;
            character.current_anim_type = CharacterAnimType.HOLD;
        }

        // play the idle animation again
        animsprite_restart(character.current_anim);
        return 1;
    }

    let switch_to_idle = 0;
    let follow_rollback = null;
    /**@type {bool}*/let follow_frame_rollback = 0;

    // guess the next animation in the action
    switch (character.current_anim_type) {
        case CharacterAnimType.BASE:
            // check if the hold animation should be played
            let follow_hold = null;
            switch (current_action_type) {
                case CharacterActionType.SING:
                    follow_hold = character.current_sing_follow_hold ? character.current_action_sing.hold : null;
                    follow_rollback = character.current_action_sing.rollback;
                    follow_frame_rollback = character.current_action_sing.base_can_rollback;
                    break;
                case CharacterActionType.EXTRA:
                case CharacterActionType.IDLE:
                    follow_hold = character.current_action_extra.hold;
                    follow_rollback = character.current_action_extra.rollback;
                    break;
            }

            if (!follow_hold || !has_beat_stop) {
                switch_to_idle = 1;
                break;
            }

            // queue hold animation
            animsprite_restart(follow_hold);
            character.current_anim = follow_hold;
            character.current_anim_type = CharacterAnimType.HOLD;
            return 0;
        case CharacterAnimType.HOLD:
            // check if should rollback the current animation or play the rollback animation
            switch (current_action_type) {
                case CharacterActionType.SING:
                    follow_rollback = character.current_action_sing.rollback;
                    follow_frame_rollback = character.current_action_sing.hold_can_rollback;
                    break;
                case CharacterActionType.MISS:
                    // never reached
                    break;
                case CharacterActionType.EXTRA:
                case CharacterActionType.IDLE:
                    follow_rollback = character.current_action_extra.rollback;
                    break;
            }
        case CharacterAnimType.ROLLBACK:
            switch_to_idle = 1;
            break;
    }

    // check if is necessary do a rollback
    if (!switch_to_idle && (follow_rollback || follow_frame_rollback)) {
        if (follow_rollback) character.current_anim = follow_rollback;

        character.current_use_frame_rollback = follow_frame_rollback;
        character.current_anim_type = CharacterAnimType.ROLLBACK;
        return 1;
    }


    //
    // Check if necessary switch to idle action
    //

    // re-schedule idle action (if current action)
    if (current_action_type == CharacterActionType.IDLE) {
        if (character.current_stop_on_beat < 0) {
            // no re-scheduled, do it now
            character_internal_set_beat_stop(character);
        }
        if (character.current_stop_on_beat > character.beatwatcher.count) {
            // wait for the next beat
            return 1;
        }
    }

    // handle special cases
    switch (current_action_type) {
        case CharacterActionType.SING:
            if (character.current_anim_type == CharacterAnimType.HOLD) return 1;
            break;
        case CharacterActionType.MISS:
            // keep the sprite static until another action is executed
            if (character.current_stop_on_beat < 0) return 1;
            // keep the sprite static until next beat stop
            if (character.beatwatcher.count < character.current_stop_on_beat) return 0;
            break;
        case CharacterActionType.EXTRA:
            // keep the sprite static if "current_stop_on_beat" is not set
            if (character.current_stop_on_beat < 0) return 1;
            // keep the sprite static until next beat stop, useless "static_until_beat" is false
            if (!has_beat_stop && character.current_action_extra.static_until_beat) return 1;
            break;
    }

    // switch current action to idle
    character.current_action_type = CharacterActionType.NONE;
    return character_play_idle(character);

}

function character_draw(character, pvrctx) {
    // TODO: direction ghosting

    pvr_context_save(pvrctx);
    drawable_helper_apply_in_context(character.drawable, pvrctx);

    statesprite_draw(character.statesprite, pvrctx);
    pvr_context_restore(pvrctx);
}


function character_set_z_index(character, z) {
    drawable_set_z_index(character.drawable, z);
}

function character_set_z_offset(character, z) {
    drawable_set_z_offset(character.drawable, z);
}

function character_animation_set(character, animsprite) {
    if (character.drawable_animation) animsprite_destroy(character.drawable_animation);
    character.drawable_animation = animsprite ? animsprite_clone(animsprite) : null;
}

function character_animation_restart(character) {
    if (character.drawable_animation) animsprite_restart(character.drawable_animation);
}

function character_animation_end(character) {
    if (character.drawable_animation) {
        animsprite_force_end(character.drawable_animation);
        animsprite_update_drawable(character.drawable_animation, character.drawable, 1);
    }
}

function character_set_color_offset(character, r, g, b, a) {
    drawable_set_offsetcolor(character.drawable, r, g, b, a);
}

function character_set_color_offset_to_default(character) {
    drawable_set_offsetcolor_to_default(character.drawable);
}

function character_set_alpha(character, alpha) {
    drawable_set_alpha(character.drawable, alpha);
}

function character_set_visible(character, visible) {
    drawable_set_visible(character.drawable, visible);
}

function character_get_modifier(character) {
    return drawable_get_modifier(character.drawable);
}

function character_get_drawable(character) {
    return character.drawable;
}

function character_get_play_calls(character) {
    return character.played_actions_count;
}

function character_get_current_action(character) {
    return character.current_action_type;
}

function character_has_direction(character, name, is_extra) {
    let array = is_extra ? character.all_extras_names : character.all_directions_names;
    let size = is_extra ? character.all_extras_names_size : character.all_directions_names_size;

    for (let i = 0; i < size; i++) {
        if (array[i] === name) return 1;
    }

    return 0;
}

function character_freeze_animation(character, enabled) {
    character.animation_freezed = enabled;
}

function character_trailing_enabled(character, enabled) {
    statesprite_trailing_enabled(character.statesprite, enabled);
}

function character_trailing_set_params(character, length, trail_delay, trail_alpha, darken_colors) {
    statesprite_trailing_set_params(character.statesprite, length, trail_delay, trail_alpha, darken_colors);
}

function character_trailing_set_offsetcolor(character, r, g, b) {
    statesprite_trailing_set_offsetcolor(character.statesprite, r, g, b);
}



function character_internal_import_sing(sing_info, modelholder, sing_entry, id_direction, prefix, suffix) {

    sing_info.base = character_internal_import_animation(modelholder, sing_entry.anim, prefix, suffix, 0);
    sing_info.hold = character_internal_import_animation(modelholder, sing_entry.anim_hold, prefix, suffix, 1);
    sing_info.rollback = character_internal_import_animation(modelholder, sing_entry.direction, prefix, suffix, 0);

    sing_info.id_direction = id_direction;
    sing_info.follow_hold = sing_entry.follow_hold;
    sing_info.full_sustain = sing_entry.full_sustain;
    sing_info.offset_x = sing_entry.offset_x;
    sing_info.offset_y = sing_entry.offset_y;

    sing_info.base_can_rollback = 0;
    sing_info.hold_can_rollback = 0;

    if (!sing_info.rollback && sing_entry.rollback) {
        if (sing_info.base != null && animsprite_is_frame_animation(sing_info.base)) {
            sing_info.base_can_rollback = 1;
        }
        if (sing_info.hold != null && animsprite_is_frame_animation(sing_info.hold)) {
            sing_info.hold_can_rollback = 1;
        }
    }

}

function character_internal_import_miss(miss_info, modelholder, miss_entry, id_direction) {
    miss_info.animation = character_internal_import_animation(modelholder, miss_entry.anim, null, null, 0);

    miss_info.id_direction = id_direction;
    miss_info.stop_after_beats = miss_entry.stop_after_beats;
    miss_info.offset_x = miss_entry.offset_x;
    miss_info.offset_y = miss_entry.offset_y;
}

async function character_internal_import_extra(extra_info, mdlhldr_rrlst, txtr_rrlst, extra_entry, id_extra, prefix, suffix, model_src) {
    if (!extra_entry) {
        extra_info.id_extra = -1;
        extra_info.is_valid = false;
        extra_info.base = null;
        extra_info.hold = null;
        extra_info.rollback = null;
        extra_info.offset_x = 0;
        extra_info.offset_y = 0;
        return;
    }

    let modelholder = await character_internal_get_modelholder(
        mdlhldr_rrlst, extra_entry.model_src ?? model_src, 1
    );
    extra_info.id_texture = character_internal_add_texture(
        txtr_rrlst, modelholder
    );

    extra_info.stop_after_beats = extra_entry.stop_after_beats;

    if (extra_entry.anim != null && extra_entry.anim.length < 1)
        extra_info.base = null;
    else
        extra_info.base = character_internal_import_animation(modelholder, extra_entry.anim, prefix, suffix, 0);

    if (extra_entry.anim_hold != null && extra_entry.anim_hold.length < 1) {
        extra_info.hold = null;
    } else {
        extra_info.hold = character_internal_import_animation(
            modelholder, extra_entry.anim_hold, prefix, suffix, 1
        );
    }

    if (extra_entry.anim_rollback != null && extra_entry.anim_rollback.length < 1) {
        extra_info.rollback = null;
    } else {
        extra_info.rollback = character_internal_import_animation(modelholder, extra_entry.anim_rollback, prefix, suffix, 0);
    }

    extra_info.id_extra = id_extra;
    extra_info.is_valid = extra_info.base != null || extra_info.hold != null;
    extra_info.static_until_beat = extra_entry.static_until_beat;
    extra_info.offset_x = extra_entry.offset_x;
    extra_info.offset_y = extra_entry.offset_y;
}



function character_internal_import_animation(mdlhldr, anim_name, prefix, suffix, is_sustain) {
    if (!anim_name) return null;
    anim_name = string_concat(3, prefix, anim_name, suffix);

    let animsprite = character_internal_import_animation2(mdlhldr, anim_name, is_sustain);
    if (!anim_name) anim_name = undefined;

    return animsprite;
}

function character_internal_import_animation2(modelholder, name, is_sustain) {
    let animsprite;

    // read from the animation list
    animsprite = animsprite_init_from_animlist(modelholder_get_animlist(modelholder), name);
    if (animsprite) return animsprite;

    // animation not found, build from atlas
    let atlas = modelholder_get_atlas(modelholder);
    let fps = atlas_get_glyph_fps(atlas);
    if (fps <= 0) fps = FUNKIN_DEFAULT_ANIMATIONS_FRAMERATE;

    let loop_count = is_sustain ? 0 /*infinite*/ : 1/*once*/;
    animsprite = animsprite_init_from_atlas(fps, loop_count, atlas, name, 1);
    if (animsprite) return animsprite;

    // ¿static animation?
    let atlas_entry = atlas_get_entry(atlas, name);
    if (!atlas_entry) return null;

    return animsprite_init_from_atlas_entry(atlas_entry, is_sustain, fps);
}

function character_internal_import_animation3(modelholder, state_name, animation, is_sustain) {
    if (!animation) return null;

    let name = animsprite_get_name(animation);
    let anim_name = string_concat_for_state_name(2, name, state_name);

    let anim = character_internal_import_animation2(modelholder, anim_name, is_sustain);
    anim_name = undefined;

    return anim;
}




function character_internal_state_create(name, sing_size, miss_size, extras_size) {
    let state = {
        name: strdup(name),
        sing_size: sing_size,
        miss_size: miss_size,
        extras_size: extras_size,
        sing: new Array(sing_size),
        sing_alt: new Array(sing_size),
        miss: new Array(miss_size),
        extras: new Array(extras_size),
        hey: {
            base: null, hold: null, rollback: null,
            beat_stop: 0, id_extra: -1, id_texture: -1, is_valid: 0, static_until_beat: 0,
            offset_x: 0, offset_y: 0
        },
        idle: {
            base: null, hold: null, rollback: null,
            beat_stop: 0, id_extra: -1, id_texture: -1, is_valid: 0, static_until_beat: 0,
            offset_x: 0, offset_y: 0
        },
        idle_alt: {
            base: null, hold: null, rollback: null,
            beat_stop: 0, id_extra: -1, id_texture: -1, is_valid: 0, static_until_beat: 0,
            offset_x: 0, offset_y: 0
        }
    };

    // JS only (C# needs implementation, array items can not be null)
    clone_struct_as_array_items(state.sing, sing_size, {});
    clone_struct_as_array_items(state.sing_alt, sing_size, {});
    clone_struct_as_array_items(state.miss, miss_size, {});
    clone_struct_as_array_items(state.extras, extras_size, {});

    return state;
}

function character_internal_state_of_sing(new_singinfo, modelholder, state_name, sing_info) {
    new_singinfo.base = character_internal_import_animation3(
        modelholder, state_name, sing_info.base, 0
    );
    new_singinfo.hold = character_internal_import_animation3(
        modelholder, state_name, sing_info.hold, 1
    );
    new_singinfo.rollback = character_internal_import_animation3(
        modelholder, state_name, sing_info.rollback, 0
    );

    new_singinfo.id_direction = sing_info.id_direction;
    new_singinfo.follow_hold = sing_info.follow_hold;
    new_singinfo.full_sustain = sing_info.full_sustain;
    new_singinfo.offset_x = sing_info.offset_x;
    new_singinfo.offset_y = sing_info.offset_y;
    new_singinfo.base_can_rollback = 0;
    new_singinfo.hold_can_rollback = 0;

    if (!sing_info.rollback) {
        new_singinfo.base_can_rollback = new_singinfo.base && animsprite_is_frame_animation(new_singinfo.base);
        new_singinfo.hold_can_rollback = new_singinfo.hold && animsprite_is_frame_animation(new_singinfo.hold);
    }

}

function character_internal_state_of_extra(new_extrainfo, modelholder, state_name, extra_info) {
    new_extrainfo.base = character_internal_import_animation3(
        modelholder, state_name, extra_info.base, 0
    );
    new_extrainfo.hold = character_internal_import_animation3(
        modelholder, state_name, extra_info.hold, 1
    );
    new_extrainfo.rollback = character_internal_import_animation3(
        modelholder, state_name, extra_info.rollback, 0
    );

    new_extrainfo.id_extra = extra_info.id_extra;
    new_extrainfo.stop_after_beats = extra_info.stop_after_beats;
    new_extrainfo.offset_x = extra_info.offset_x;
    new_extrainfo.offset_y = extra_info.offset_y;
    new_extrainfo.static_until_beat = extra_info.static_until_beat;
    new_extrainfo.is_valid = new_extrainfo.base && new_extrainfo.hold;
}



function character_internal_get_direction_id(character, direction) {
    for (let i = 0; i < character.all_directions_names_size; i++) {
        if (character.all_directions_names[i] != direction) continue;

        if (character.inverted_enabled) {
            for (let j = 0; j < character.inverted_size; j++) {
                if (character.inverted_from[j] == i) {
                    if (character.inverted_to[j] < 0)
                        break;// no inverse direction
                    else
                        return character.inverted_to[j];
                }
            }
        }

        return i;
    }
    // unknown direction
    return -1;
}

function character_internal_get_extra_id(character, direction) {
    for (let i = 0; i < character.all_extras_names_size; i++) {
        if (character.all_extras_names[i] == direction) return i;
    }
    return -1;
}

function character_internal_index_name(arraylist, name, add_if_not_found) {
    if (name == null) return -1;

    let array = arraylist_peek_array(arraylist);
    let size = arraylist_size(arraylist);

    for (let i = 0; i < size; i++) {
        if (array[i] == name) return i;
    }

    if (!add_if_not_found) return -1;

    arraylist_add(arraylist, strdup(name));
    return size;
}

function character_internal_update_texture(character) {
    let id_texture = -1;
    switch (character.current_action_type) {
        case CharacterActionType.EXTRA:
        case CharacterActionType.IDLE:
            id_texture = character.current_action_extra.id_texture;
            break;
        case CharacterActionType.MISS:
            id_texture = character.current_action_miss.id_texture;
            break;
        case CharacterActionType.SING:
            id_texture = character.current_action_sing.id_texture;
            break;
    }

    let final_scale = character.character_scale;

    if (id_texture >= 0 && id_texture != character.current_texture_id) {

        let texture_info = arraylist_get(character.textures, id_texture);
        character.current_texture_id = id_texture;

        //
        // (Does not work) 
        // Calculate the scale required to fit the sprite in the layout and/or screen
        // and later apply this scale to the character scale.
        //
        // As long as the viewport size of the layout is intended for a 1280x720 screen, there 
        // will be no problems.
        //
        //
        //let scale_factor = character.layout_height / texture_info.resolution_height;
        //final_scale *= scale_factor;

        statesprite_set_texture(character.statesprite, texture_info.texture, 1);
    }

    statesprite_change_draw_size_in_atlas_apply(character.statesprite, 1, final_scale);
    if (character.current_anim) animsprite_update_statesprite(character.current_anim, character.statesprite, 0);
}

function character_internal_import_opposite_dir(character, array_ptr, ltr_array) {
    // JS only
    let array = new Array(character.inverted_size);
    character[array_ptr] = array;

    // C only
    // char** array = malloc(character->inverted_size * sizeof(void*));
    // *array_ptr = array;

    for (let i = 0; i < character.inverted_size; i++) {
        array[i] = -1;
        if (ltr_array[i] == null) continue;

        for (let j = 0; j < character.all_directions_names_size; j++) {
            if (character.all_directions_names[j] == ltr_array[i]) {
                array[i] = j;
                break;
            }
        }

        if (array[i] < 0)
            console.error("character_init() unknown sing/miss direction: " + ltr_array[i]);
    }
}

function character_internal_calculate_location(character) {
    const draw_size = [0, 0];
    const modifier = drawable_get_modifier(character.drawable);

    let draw_x = character.draw_x;
    let draw_y = character.draw_y;

    statesprite_get_draw_size(character.statesprite, draw_size);
    if (character.enable_reference_size) {
        if (character.reference_width >= 0) draw_size[0] = (character.reference_width - draw_size[0]) / 2.0;
        if (character.reference_height >= 0) draw_size[1] = (character.reference_height - draw_size[1]) / 2.0;
    }

    // pick current action offsets
    let action_offset_x, action_offset_y;
    switch (character.current_action_type) {
        case CharacterActionType.SING:
            action_offset_x = character.current_action_sing.offset_x;
            action_offset_y = character.current_action_sing.offset_y;
            break;
        case CharacterActionType.MISS:
            action_offset_x = character.current_action_miss.offset_x;
            action_offset_y = character.current_action_miss.offset_y;
            break;
        case CharacterActionType.IDLE:
        case CharacterActionType.EXTRA:
            action_offset_x = character.current_action_extra.offset_x;
            action_offset_y = character.current_action_extra.offset_y;
            break;
        default:
            action_offset_x = 0;
            action_offset_y = 0;
            break;
    }

    // step 1: apply align
    switch (character.align_vertical) {
        case ALIGN_CENTER:
            draw_y -= draw_size[1] / 2.0;
            break;
        case ALIGN_END:
            draw_y -= draw_size[1];
            break;
    }
    switch (character.align_horizontal) {
        case ALIGN_CENTER:
            draw_x -= draw_size[0] / 2.0;
            break;
        case ALIGN_END:
            draw_x -= draw_size[0];
            break;
    }

    // step 2: apply offsets
    draw_x += (action_offset_x + character.offset_x) * character.character_scale;
    draw_y += (action_offset_y + character.offset_y) * character.character_scale;

    // step 4: change the sprite location
    modifier.x = draw_x;
    modifier.y = draw_y;
    statesprite_set_draw_location(character.statesprite, draw_x, draw_y);
}

function character_internal_end_current_action(character) {
    if (!character.current_anim) return;

    let base = null;
    let hold = null;
    let rollback = null;

    switch (character.current_action_type) {
        case CharacterActionType.SING:
            rollback = character.current_action_sing.rollback;
            switch (character.current_anim_type) {
                case CharacterAnimType.BASE:
                    base = character.current_action_sing.base;
                    if (character.current_action_sing.follow_hold) hold = character.current_action_sing.hold;
                    break;
                case CharacterAnimType.HOLD:
                    hold = character.current_action_sing.hold;
                    break;
                case CharacterAnimType.ROLLBACK:
                    if (rollback) animsprite_restart(rollback);
                    break;
            }
            break;
        case CharacterActionType.MISS:
            base = character.current_action_miss.animation;
            break;
        case CharacterActionType.IDLE:
        case CharacterActionType.EXTRA:
            rollback = character.current_action_extra.rollback;
            switch (character.current_anim_type) {
                case CharacterAnimType.BASE:
                    base = character.current_action_extra.base;
                    hold = character.current_action_extra.hold;
                    break;
                case CharacterAnimType.HOLD:
                    hold = character.current_action_extra.hold;
                    break;
                case CharacterAnimType.ROLLBACK:
                    if (rollback) animsprite_restart(rollback);
                    break;
            }
            break;
    }

    if (base) animsprite_force_end3(base, character.statesprite);
    if (hold) animsprite_force_end3(hold, character.statesprite);
    if (rollback) animsprite_force_end3(rollback, character.statesprite);
}

function character_internal_fallback_idle(character) {
    if (character.current_action_type == CharacterActionType.IDLE) return;
    character_internal_end_current_action(character);
    character_play_idle(character);
}

function character_internal_add_texture(texture_arraylist, modelholder) {
    let array = arraylist_peek_array(texture_arraylist);
    let size = arraylist_size(texture_arraylist);

    const texture = modelholder_get_texture(modelholder, 0);
    if (!texture) return -1;

    for (let i = 0; i < size; i++) {
        if (array[i].texture == texture) return i;
    }

    const resolution = [0, 0];
    modelholder_get_texture_resolution(modelholder, resolution);

    arraylist_add(texture_arraylist, {
        texture: modelholder_get_texture(modelholder, 1),
        resolution_width: resolution[0],
        resolution_height: resolution[1],
    });

    return size;
}

async function character_internal_get_modelholder(modelholder_arraylist, model_src, is_optional) {
    if (!model_src || model_src.length < 1) {
        if (is_optional)
            return arraylist_get(modelholder_arraylist, 0).modelholder;
        else
            throw new Error("character model not specified in the manifest");
    }

    let array = arraylist_peek_array(modelholder_arraylist);
    let size = arraylist_size(modelholder_arraylist);

    for (let i = 0; i < size; i++) {
        if (array[i].model_src == model_src) return array[i].modelholder;
    }

    const modelholder = await modelholder_init(model_src);
    if (!modelholder || modelholder_is_invalid(modelholder)) throw new Error("model not found: " + model_src);

    arraylist_add(modelholder_arraylist, { model_src, modelholder });
    return modelholder;
}

function character_internal_destroy_modelholder(modelholder_arraylist_item) {
    modelholder_arraylist_item.model_src = undefined;
    modelholder_destroy(modelholder_arraylist_item.modelholder);
}

function character_internal_set_beat_stop(/**@type {object}*/character,/**@type {number} */ stop_after_beats) {
    if (stop_after_beats < 1) {
        character.current_stop_on_beat = -1;
        return;
    }

    character.current_stop_on_beat = character.beatwatcher.count + stop_after_beats;
    if (beatwatcher_remaining_until_next(character.beatwatcher) <= (character.beatwatcher.tick * 0.5))
        character.current_stop_on_beat++;
}

async function character_internal_import_actions(context, actions, model_src, state_name) {
    let state = character_internal_state_create(state_name, actions.sing_size, actions.miss_size, actions.extras_size);
    arraylist_add(context.states, state);

    // import all sign actions
    for (let i = 0; i < actions.sing_size; i++) {
        let index = character_internal_index_name(
            context.all_directions_names, actions.sing[i].direction, 1
        );

        let modelholder = await character_internal_get_modelholder(
            context.modelholder_arraylist, actions.sing[i].model_src ?? model_src, 1
        );

        state.sing[i].id_texture = state.sing_alt[i].id_texture = character_internal_add_texture(
            context.textures, modelholder
        );

        character_internal_import_sing(
            state.sing[i],
            modelholder, actions.sing[i],
            index,
            context.charactermanifest.sing_suffix
        );
        character_internal_import_sing(
            state.sing_alt[i],
            modelholder,
            actions.sing[i],
            index,
            context.charactermanifest.sing_alternate_suffix
        );
    }

    // import all miss actions
    for (let i = 0; i < actions.miss_size; i++) {
        let index = character_internal_index_name(
            context.all_directions_names, actions.miss[i].direction, 1
        );

        let modelholder = await character_internal_get_modelholder(
            context.modelholder_arraylist, actions.miss[i].model_src ?? model_src, 1
        );

        state.miss[i].id_texture = character_internal_add_texture(
            context.textures, modelholder
        );

        character_internal_import_miss(
            state.miss[i], modelholder, actions.miss[i], index
        );
    }

    // import all extras
    for (let i = 0; i < actions.extras_size; i++) {
        let index = character_internal_index_name(
            context.all_extras_names, actions.extras[i].name, 1
        );

        await character_internal_import_extra(
            state.extras[i],
            context.modelholder_arraylist,
            context.textures,
            actions.extras[i],
            index,
            null,
            null,
            model_src
        );
    }

    await character_internal_import_extra(
        state.hey,
        context.modelholder_arraylist,
        context.textures,
        actions.hey,
        -10,
        null,
        null,
        model_src
    );

    await character_internal_import_extra(
        state.idle,
        context.modelholder_arraylist,
        context.textures,
        actions.idle,
        -11,
        context.charactermanifest.allow_alternate_idle ? context.charactermanifest.sing_prefix : null,
        context.charactermanifest.allow_alternate_idle ? context.charactermanifest.sing_suffix : null,
        model_src
    );

    if (context.charactermanifest.allow_alternate_idle) {
        await character_internal_import_extra(
            state.idle_alt,
            context.modelholder_arraylist,
            context.textures,
            actions.idle,
            -11,
            context.charactermanifest.sing_alternate_prefix,
            context.charactermanifest.sing_alternate_suffix,
            model_src
        );
    }

    return state;
}

