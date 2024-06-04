"use strict";

//
// buttons binding
//
const BUTTONS_BIND_4 = [
    GAMEPAD_DALL_LEFT, GAMEPAD_DALL_DOWN,
    GAMEPAD_DALL_UP, GAMEPAD_DALL_RIGHT
];

const BUTTONS_BIND_5 = [
    GAMEPAD_DALL_LEFT, GAMEPAD_DALL_DOWN,
    GAMEPAD_T_LR | GAMEPAD_B_LR | GAMEPAD_SELECT,
    GAMEPAD_DALL_UP, GAMEPAD_DALL_RIGHT
];

const BUTTONS_BIND_6 = [
    GAMEPAD_DPAD_LEFT | GAMEPAD_DPAD2_LEFT, GAMEPAD_DPAD_UP | GAMEPAD_DPAD2_UP, GAMEPAD_DPAD_RIGHT | GAMEPAD_DPAD2_RIGHT,
    GAMEPAD_DPAD3_LEFT | GAMEPAD_DPAD4_LEFT, GAMEPAD_DPAD3_UP | GAMEPAD_DPAD4_UP, GAMEPAD_DPAD3_RIGHT | GAMEPAD_DPAD4_RIGHT
];

const BUTTONS_BIND_7 = [
    GAMEPAD_DPAD_LEFT | GAMEPAD_DPAD2_LEFT, GAMEPAD_DPAD_UP | GAMEPAD_DPAD2_UP, GAMEPAD_DPAD_RIGHT | GAMEPAD_DPAD2_RIGHT,
    GAMEPAD_T_LR | GAMEPAD_B_LR | GAMEPAD_SELECT,
    GAMEPAD_DPAD3_LEFT | GAMEPAD_DPAD4_LEFT, GAMEPAD_DPAD3_UP | GAMEPAD_DPAD4_UP, GAMEPAD_DPAD3_RIGHT | GAMEPAD_DPAD4_RIGHT
];

const BUTTONS_BIND_8 = [
    GAMEPAD_DPAD_LEFT | GAMEPAD_DPAD2_LEFT, GAMEPAD_DPAD_DOWN | GAMEPAD_DPAD2_DOWN, GAMEPAD_DPAD_UP | GAMEPAD_DPAD2_UP, GAMEPAD_DPAD_RIGHT | GAMEPAD_DPAD2_RIGHT,
    GAMEPAD_DPAD3_LEFT | GAMEPAD_DPAD4_LEFT, GAMEPAD_DPAD3_DOWN | GAMEPAD_DPAD4_DOWN, GAMEPAD_DPAD3_UP | GAMEPAD_DPAD4_UP, GAMEPAD_DPAD3_RIGHT | GAMEPAD_DPAD4_RIGHT,
];

const BUTTONS_BIND_9 = [
    GAMEPAD_DPAD_LEFT | GAMEPAD_DPAD2_LEFT, GAMEPAD_DPAD_DOWN | GAMEPAD_DPAD2_DOWN, GAMEPAD_DPAD_UP | GAMEPAD_DPAD2_UP, GAMEPAD_DPAD_RIGHT | GAMEPAD_DPAD2_RIGHT,
    GAMEPAD_T_LR | GAMEPAD_B_LR | GAMEPAD_SELECT,
    GAMEPAD_DPAD3_LEFT | GAMEPAD_DPAD4_LEFT, GAMEPAD_DPAD3_DOWN | GAMEPAD_DPAD4_DOWN, GAMEPAD_DPAD3_UP | GAMEPAD_DPAD4_UP, GAMEPAD_DPAD3_RIGHT | GAMEPAD_DPAD4_RIGHT,
];


const FUNKIN_COMMON_NOTES_SPLASH = "/assets/common/image/week-round/noteSplashes.xml";
const FUNKIN_COMMON_NOTES = "/assets/common/image/week-round/notes.xml";
const FUNKIN_COMMON_STRUM_BACKGROUND = "/assets/common/image/week-round/strumBackground.xml";// STUB

const STRUMS_DEFAULT_DISTRIBUTION = {
    notes_size: 4,
    notes: [
        { name: "left" }, { name: "down" }, { name: "up" }, { name: "right" }
    ],

    strums_size: 4,
    strums: [
        { name: "left", notes_ids: [0], notes_ids_size: 1 },
        { name: "down", notes_ids: [1], notes_ids_size: 1 },
        { name: "up", notes_ids: [2], notes_ids_size: 1 },
        { name: "right", notes_ids: [3], notes_ids_size: 1 }
    ],
    strum_binds: BUTTONS_BIND_4,
    states: [
        {
            name: null,
            model_marker: FUNKIN_COMMON_NOTES,
            model_sick_effect: FUNKIN_COMMON_NOTES_SPLASH,
            model_background: FUNKIN_COMMON_STRUM_BACKGROUND,
            model_notes: null
        }
    ],
    states_size: 1
};

/* __attribute__((constructor)) void init_default_strums_distribution() */ {
    for (let i = 0; i < STRUMS_DEFAULT_DISTRIBUTION.notes.length; i++) {
        let note = STRUMS_DEFAULT_DISTRIBUTION.notes[i];
        note.sick_effect_state_name = null;
        note.model_src = FUNKIN_COMMON_NOTES;
        note.custom_sick_effect_model_src = null;
        note.damage_ratio = 1.0;
        note.heal_ratio = 1.0;
        note.ignore_hit = 0;
        note.ignore_miss = 0;
        note.can_kill_on_hit = 0;
    }
}


//
//  the following default values corresponds to a screen resolution of 1280x720
//
const STRUMS_MARKER_DIMMEN = 50;
const STRUMS_MARKER_INVDIMMEN = 50;
const STRUMS_STRUM_LENGTH = 450;
const STRUMS_GAP = 5;
const STRUMS_SCROLL_DIRECTION = STRUM_UPSCROLL;


async function strums_init(x, y, z, dimmen, invdimmen, length, gap, player_id, is_vertical, keep_marker_scale, strumsdefs, strumsdefs_size) {
    let strums = {
        size: strumsdefs_size,
        lines: new Array(strumsdefs_size),
        sick_effects: new Array(strumsdefs_size),

        player_id, x, y, gap, invdimmen, is_vertical,

        ddrkeys_fifo: null, playerstats: null, weekscript: null,

        drawable: null,
        drawable_animation: null,
        modifier: null
    };

    strums.drawable = drawable_init(z, strums, strums_draw, strums_animate);
    strums.modifier = drawable_get_modifier(strums.drawable);
    strums.modifier.x = x;
    strums.modifier.x = y;

    strums.is_vertical = is_vertical;
    strums.is_inverse = 0;

    let space = gap + invdimmen;

    for (let i = 0; i < strums.size; i++) {
        const name = strumsdefs[i].name;
        strums.sick_effects[i] = null;

        strums.lines[i] = strum_init(i, name, x, y, dimmen, invdimmen, length, keep_marker_scale);
        strum_set_player_id(strums.lines[i], player_id);

        if (is_vertical) x += space;
        else y += space;
    }

    let invlength = gap * strums.size;
    if (strums.size > 0) invlength -= gap;
    invlength += invdimmen * strums.size;

    if (is_vertical) {
        strums.modifier.width = invlength;
        strums.modifier.height = dimmen;
    } else {
        strums.modifier.width = dimmen;
        strums.modifier.height = invlength;
    }

    strums.decorators_scroll_velocity = 1.0;
    strums.decorators = arraylist_init2(1);
    strums.decorators_alpha = 1.0;
    strums.decorators_offset_milliseconds = 0.0;

    return strums;
}

function strums_destroy(strums) {
    ModuleLuaScript.kdmyEngine_drop_shared_object(strums);

    for (let i = 0; i < strums.size; i++)
        strum_destroy(strums.lines[i]);

    drawable_destroy(strums.drawable);
    if (strums.drawable_animation) animsprite_destroy(strums.drawable_animation);

    for (let decorator of arraylist_iterate4(strums.decorators))
        statesprite_destroy(decorator.statesprite);
    arraylist_destroy(strums.decorators, 0);

    strums.lines = undefined;
    strums.sick_effects = undefined;
    strums = undefined;
}

function strums_set_params(strums, ddrkeymon, playerstats, weekscript) {
    strums.ddrkeys_fifo = ddrkeymon ? ddrkeymon_get_fifo(ddrkeymon) : null;
    strums.playerstats = playerstats;
    strums.weekscript = weekscript;
}

function strums_get_drawable(strums) {
    return strums.drawable;
}


function strums_set_notes(strums, chart, strumsdefs, strumsdefs_size, notepool) {
    for (let i = 0; i < strums.size; i++) {
        let success = strum_set_notes(
            strums.lines[i], chart, strumsdefs, strumsdefs_size, strums.player_id, notepool
        );
        if (success < 0)
            throw new Error(`Error setting the notes for strum id=${i} player_id=${strums.player_id}`);
    }
}

function strums_set_scroll_speed(strums, speed) {
    for (let i = 0; i < strums.size; i++)
        strum_set_scroll_speed(strums.lines[i], speed);
}

function strums_set_scroll_direction(strums, direction) {
    let is_vertical = direction == STRUM_UPSCROLL || direction == STRUM_DOWNSCROLL;

    strums.is_inverse = direction == STRUM_DOWNSCROLL || direction == STRUM_RIGHTSCROLL;
    if (is_vertical != strums.is_vertical) {
        let temp = strums.modifier.width;
        strums.modifier.width = strums.modifier.height;
        strums.modifier.height = temp;
        strums.is_vertical = is_vertical;
        strums_internal_update_draw_location(strums);
    }

    for (let i = 0; i < strums.size; i++) strum_set_scroll_direction(strums.lines[i], direction);
}

function strums_set_marker_duration_multiplier(strums, multipler) {
    for (let i = 0; i < strums.size; i++) strum_set_marker_duration_multiplier(strums.lines[i], multipler);
}

function strums_set_bpm(strums, beats_per_minute) {
    for (let i = 0; i < strums.size; i++)  strum_set_bpm(strums.lines[i], beats_per_minute);
}

function strums_disable_beat_synced_idle_and_continous(strums, disabled) {
    for (let i = 0; i < strums.size; i++)  strum_disable_beat_synced_idle_and_continous(strums.lines[i], disabled);
}

function strums_reset(strums, scroll_speed, state_name) {
    for (let i = 0; i < strums.size; i++) {
        strum_reset(strums.lines[i], scroll_speed, state_name);
    }
    drawable_set_antialiasing(strums.drawable, PVRCTX_FLAG_DEFAULT);
    strums_decorators_set_scroll_speed(strums, scroll_speed);
    strums_decorators_set_visible(strums, -1.0, 1);
    strums.decorators_last_song_timestamp = 0.0;
}

function strums_scroll_full(strums, song_timestamp) {
    strums.decorators_last_song_timestamp = song_timestamp;

    if (!strums.ddrkeys_fifo) {
        // this never should happen, use strums_scroll_auto() instead
        for (let i = 0; i < strums.size; i++)
            strum_scroll_auto(strums.lines[i], song_timestamp, strums.playerstats, strums.weekscript);
        return 0;
    }

    let keys_processed = 0;

    // step 1: scroll all lines
    for (let i = 0; i < strums.size; i++) {
        keys_processed += strum_scroll(
            strums.lines[i], song_timestamp, strums.ddrkeys_fifo, strums.playerstats, strums.weekscript
        );
    }

    // step 2: find penalties hits between lines
    for (let i = 0; i < strums.size; i++) {
        keys_processed += strum_find_penalties_note_hit(
            strums.lines[i], song_timestamp, strums.ddrkeys_fifo, strums.playerstats, strums.weekscript
        );
    }

    // step 3: find penalties hits on empty lines
    for (let i = 0; i < strums.size; i++) {
        keys_processed += strum_find_penalties_empty_hit(
            strums.lines[i], song_timestamp, strums.ddrkeys_fifo, strums.playerstats
        );
    }

    if (keys_processed > 0) {
        ddrkeymon_purge(strums.ddrkeys_fifo);
    }

    return keys_processed;
}

function strums_scroll_auto(strums, song_timestamp) {
    for (let i = 0; i < strums.size; i++)
        strum_scroll_auto(strums.lines[i], song_timestamp, strums.playerstats, strums.weekscript);

    if (strums.ddrkeys_fifo) {
        // drop all key events, they are useless
        strums.ddrkeys_fifo.available = 0;
    }

    strums.decorators_last_song_timestamp = song_timestamp;
}

function strums_force_key_release(strums) {
    for (let i = 0; i < strums.size; i++) {
        strum_force_key_release(strums.lines[i]);
    }
}


function strums_animate(strums, elapsed) {
    let res = 0;
    if (strums.drawable_animation) {
        res += animsprite_animate(strums.drawable_animation, elapsed);
        animsprite_update_drawable(strums.drawable_animation, strums.drawable, 1);
    }

    for (let i = 0; i < strums.size; i++)
        res += strum_animate(strums.lines[i], elapsed);

    const decorators_array = arraylist_peek_array(strums.decorators);
    const decorators_size = arraylist_size(strums.decorators);
    for (let i = 0; i < decorators_size; i++)
        res += statesprite_animate(decorators_array[i].statesprite, elapsed);

    return res;
}

function strums_draw(strums, pvrctx) {
    pvr_context_save(pvrctx);

    drawable_helper_apply_in_context(strums.drawable, pvrctx);

    for (let i = 0; i < strums.size; i++) {
        if (!drawable_is_visible(strum_get_drawable(strums.lines[i]))) continue;
        strum_draw(strums.lines[i], pvrctx);
    }

    for (let i = 0; i < strums.size; i++) {
        if (!drawable_is_visible(strum_get_drawable(strums.lines[i]))) continue;
        if (strums.sick_effects[i] && statesprite_is_visible(strums.sick_effects[i]))
            statesprite_draw(strums.sick_effects[i], pvrctx);
    }

    pvr_context_save(pvrctx);
    pvr_context_set_global_alpha(pvrctx, strums.decorators_alpha);

    const decorators_size = arraylist_size(strums.decorators);
    const decorators_array = arraylist_peek_array(strums.decorators);
    let song_timestamp = strums.decorators_last_song_timestamp + strums.decorators_offset_milliseconds;
    let draw_x = strums.modifier.x;
    let draw_y = strums.modifier.y;

    //
    // Draw all decorators and let the PVRContext decide what is visible, because
    // decorators can have different dimmensions. The marker duration and scroll window
    // does not apply here.
    //
    for (let i = 0; i < decorators_size; i++) {
        let decorator_timestamp = decorators_array[i].timestamp;
        let scroll_offset;
        let x = draw_x;
        let y = draw_y;

        if (strums.is_inverse) {
            scroll_offset = song_timestamp - decorator_timestamp;
        } else {
            scroll_offset = decorator_timestamp - song_timestamp;
        }

        scroll_offset *= strums.decorators_scroll_velocity;

        if (strums.is_vertical)
            y += scroll_offset;
        else
            x += scroll_offset;

        const statesprite = decorators_array[i].statesprite;
        statesprite_set_draw_location(statesprite, x, y);
        statesprite_draw(statesprite, pvrctx);
    }

    pvr_context_restore(pvrctx);
    pvr_context_restore(pvrctx);
}


function strums_set_alpha(strums, alpha) {
    let old = drawable_get_alpha(strums.drawable);
    drawable_set_alpha(strums.drawable, alpha);
    return old;
}

function strums_enable_background(strums, enable) {
    for (let i = 0; i < strums.size; i++)
        strum_enable_background(strums.lines[i], enable);
}

function strums_set_keep_aspect_ratio_background(strums, enable) {
    for (let i = 0; i < strums.size; i++)
        strum_set_keep_aspect_ratio_background(strums.lines[i], enable);
}

function strums_set_alpha_background(strums, alpha) {
    for (let i = 0; i < strums.size; i++)
        strum_set_alpha_background(strums.lines[i], alpha);

}

function strums_set_alpha_sick_effect(strums, alpha) {
    for (let i = 0; i < strums.size; i++)
        strum_set_alpha_sick_effect(strums.lines[i], alpha);
}

function strums_set_draw_offset(strums, offset_milliseconds) {
    for (let i = 0; i < strums.size; i++)
        strum_set_draw_offset(strums.lines[i], offset_milliseconds);
    strums.decorators_offset_milliseconds = offset_milliseconds;
}


function strums_state_add(strums, mdlhldr_mrkr, mdlhldr_sck_ffct, mdlhldr_bckgrnd, state_name) {
    for (let i = 0; i < strums.size; i++)
        strum_state_add(strums.lines[i], mdlhldr_mrkr, mdlhldr_sck_ffct, mdlhldr_bckgrnd, state_name);
}

function strums_state_toggle(strums, state_name) {
    let toggles = 0;
    for (let i = 0; i < strums.size; i++)
        toggles += strum_state_toggle(strums.lines[i], state_name);
    return toggles;
}

function strums_state_toggle_notes(strums, state_name) {
    for (let i = 0; i < strums.size; i++)
        strum_state_toggle_notes(strums.lines[i], state_name);
}

function strums_state_toggle_marker_and_sick_effect(strums, state_name) {
    for (let i = 0; i < strums.size; i++) {
        strum_state_toggle_marker(strums.lines[i], state_name);
        strum_state_toggle_sick_effect(strums.lines[i], state_name);
    }
}

function strums_get_lines_count(strums) {
    return strums.size;
}

function strums_get_strum_line(strums, index) {
    if (index < 0 || index >= strums.size) return null;
    return strums.lines[index];
}

function strums_enable_post_sick_effect_draw(strums, enable) {
    for (let i = 0; i < strums.size; i++) {
        if (enable)
            strums.sick_effects[i] = strum_draw_sick_effect_apart(strums.lines[i], enable);
        else
            strums.sick_effects[i] = null;
    }
}

function strums_use_funkin_maker_duration(strums, enable) {
    for (let i = 0; i < strums.size; i++) {
        strum_use_funkin_maker_duration(strums.lines[i], enable);
    }
}


function strums_set_offsetcolor(strums, r, g, b, a) {
    drawable_set_offsetcolor(strums.drawable, r, g, b, a);
}

function strums_set_visible(strums, visible) {
    drawable_set_visible(strums.drawable, visible);
}

function strums_get_modifier(strums) {
    return drawable_get_modifier(strums.drawable);
}


function strums_animation_set(strums, animsprite) {
    if (strums.drawable_animation) animsprite_destroy(strums.drawable_animation);
    strums.drawable_animation = animsprite ? animsprite_clone(animsprite) : null;
}

function strums_animation_restart(strums) {
    if (strums.drawable_animation) animsprite_restart(strums.drawable_animation);

    for (let i = 0; i < strums.size; i++)
        strum_animation_restart(strums.lines[i]);

    const decorators_array = arraylist_peek_array(strums.decorators);
    const decorators_size = arraylist_size(strums.decorators);
    for (let i = 0; i < decorators_size; i++)
        statesprite_animation_restart(decorators_array[i].statesprite);
}

function strums_animation_end(strums) {
    if (strums.drawable_animation) {
        animsprite_force_end(strums.drawable_animation);
        animsprite_update_drawable(strums.drawable_animation, strums.drawable, 1);
    }

    for (let i = 0; i < strums.size; i++)
        strum_animation_end(strums.lines[i]);

    const decorators_array = arraylist_peek_array(strums.decorators);
    const decorators_size = arraylist_size(strums.decorators);
    for (let i = 0; i < decorators_size; i++)
        statesprite_animation_end(decorators_array[i].statesprite);
}


function strums_decorators_get_count(strums) {
    return arraylist_size(strums.decorators);
}

function strums_decorators_add(strums, modelholder, animation_name, timestamp) {
    return strums_decorators_add2(strums, modelholder, animation_name, timestamp, 0, strums.size - 1);
}

function strums_decorators_add2(strums, modelholder, animation_name, timestamp, from_strum_index, to_strum_index) {
    if (!modelholder || timestamp < 0 || Number.isNaN(timestamp)) return 0;
    if (from_strum_index < 0 || to_strum_index >= strums.size || to_strum_index < from_strum_index) return 0;

    let statesprite = statesprite_init_from_vertex_color(modelholder_get_vertex_color(modelholder));
    let state = statesprite_state_add(statesprite, modelholder, animation_name, null);

    if (!state) {
        statesprite_destroy(statesprite);
        return 0;
    }

    strums_internal_calc_decorator_bounds(strums, state, from_strum_index, to_strum_index);
    statesprite_state_apply(statesprite, state);

    let decorator = {
        timestamp: timestamp,
        statesprite: statesprite,
        to_strum_index: to_strum_index,
        from_strum_index: from_strum_index,
        is_visible: 1
    };

    arraylist_add(strums.decorators, decorator);

    return 1;
}

function strums_decorators_set_scroll_speed(strums, speed) {
    strums.decorators_scroll_velocity = FNF_CHART_SCROLL_VELOCITY * speed;

    // Use half of the scolling speed if the screen aspect ratio is 4:3 (dreamcast)
    if (!pvr_context_is_widescreen()) strums.decorators_scroll_velocity *= 0.5;
}

function strums_decorators_set_alpha(strums, alpha) {
    strums.decorators_alpha = alpha;
}

function strums_decorators_set_visible(strums, decorator_timestamp, visible) {
    for (let decorator of arraylist_iterate4(strums.decorators)) {
        if (decorator_timestamp < 0.0 || math2d_doubles_are_near_equal(decorator.timestamp, decorator_timestamp)) {
            decorator.is_visible = visible;
        }
    }
}


function strums_force_rebuild(strums, x, y, z, dimmen, invdimmen, length_dimmen, gap, is_vertical, keep_markers_scale) {
    drawable_set_z_index(strums.drawable, z);

    strums.modifier.x = x;
    strums.modifier.y = y;
    strums.is_vertical = is_vertical;
    strums.is_inverse = 0;
    strums.gap = gap;
    strums.invdimmen = invdimmen;

    let invlength = gap * strums.size;
    if (strums.size > 0) invlength -= gap;
    invlength += invdimmen * strums.size;

    if (is_vertical) {
        strums.modifier.width = invlength;
        strums.modifier.height = dimmen;
    } else {
        strums.modifier.width = dimmen;
        strums.modifier.height = invlength;
    }

    strums_internal_update_draw_location(strums);

    for (let i = 0; i < strums.size; i++) {
        strum_force_rebuild(strums.lines[i], dimmen, invdimmen, length_dimmen, keep_markers_scale);
    }
}

function strums_internal_calc_decorator_bounds(strums, state, from_strum_index, to_strum_index) {
    let offset_start = (strums.gap + strums.invdimmen) * from_strum_index;
    let offset_end = (strums.gap + strums.invdimmen) * to_strum_index;
    let length = offset_end - offset_start;

    let x, y;
    let width, height;
    let horizontal, vertical;

    if (strums.is_vertical) {
        x = offset_start;
        y = 0.0;
        width = length;
        height = -1.0;
        horizontal = ALIGN_CENTER;
        vertical = strums.is_inverse ? ALIGN_END : ALIGN_START;
    } else {
        x = 0.0;
        y = offset_start;
        width = -1;
        height = length;
        horizontal = strums.is_inverse ? ALIGN_END : ALIGN_START;
        vertical = ALIGN_CENTER;
    }

    if (state.texture) {
        imgutils_calc_rectangle_in_statesprite_state(x, y, width, height, horizontal, vertical, state);
    } else {
        state.offset_x = x;
        state.offset_y = y;
        state.draw_width = strums.is_vertical ? length : strums.invdimmen;
        state.draw_height = strums.is_vertical ? strums.invdimmen : length;
    }
}

function strums_internal_update_draw_location(strums) {
    let space = strums.gap + strums.invdimmen;
    let x = strums.x;
    let y = strums.y;

    for (let i = 0; i < strums.size; i++) {
        strum_update_draw_location(strums.lines[i], x, y);

        if (strums.is_vertical) x += space;
        else y += space;
    }

    for (let decorator of arraylist_iterate4(strums.decorators)) {
        let state = statesprite_state_get(decorator.statesprite);
        strums_internal_calc_decorator_bounds(strums, state, decorator.from_strum_index, decorator.to_strum_index);
        statesprite_state_apply(decorator.statesprite, state);
    }
}

