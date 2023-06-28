"use strict";

//
// it took over four months to create this, the most expensive part of the project
//

/**  @readonly @enum {number} */
const NoteState = {
    CLEAR: 0,
    MISS: 1,
    PENDING: 2,

    HOLD: 3,
    RELEASE: 4
};

class StrumNote {
    /** @type {number} */
    timestamp;
    /** @type {number} */
    id;
    /** @type {number} */
    duration;
    /** @type {boolean} */
    alt_anim;

    /** @type {NoteState} */
    state;
    /** @type {number} */
    hit_diff;
    /** @type {number} */
    release_time;

    /** @type {number} */
    release_button;

    /** @type {number} */
    previous_quarter;

    /** @type {any} */
    custom_data;

    /** @type {bool} */
    hit_on_penality;

    constructor(chart_note, map, map_size) {
        let id = 0;
        for (let i = 0; i < map_size; i++) {
            if (map[i] == chart_note.direction) {
                id = i;
                break;
            }
        }

        // base note info
        this.id = id;
        this.timestamp = chart_note.timestamp;
        this.duration = chart_note.duration;
        this.custom_data = chart_note.data;
        this.hit_on_penality = 0;
        this.alt_anim = chart_note.alt_anim;

        this.state = NoteState.PENDING;
        this.hit_diff = NaN;
        this.release_time = -1.0;

        this.release_button = 0x00;

        this.previous_quarter = 0;
    }

    get isSustain() { return this.duration > 0; }

    get endTimestamp() { return this.timestamp + this.duration; }

    static sort_callback(note1, note2) { return note1.timestamp - note2.timestamp; }
}

const STRUM_UPSCROLL = 0;
const STRUM_LEFTSCROLL = 1;
const STRUM_DOWNSCROLL = 2;
const STRUM_RIGHTSCROLL = 3;

const STRUM_PRESS_STATE_NONE = 0;
const STRUM_PRESS_STATE_HIT = 1;
const STRUM_PRESS_STATE_HIT_SUSTAIN = 2;
const STRUM_PRESS_STATE_PENALTY_NOTE = 3;
const STRUM_PRESS_STATE_PENALTY_HIT = 4;
const STRUM_PRESS_STATE_MISS = 5;


const STRUM_MARKER_STATE_NOTHING = 0;
const STRUM_MARKER_STATE_CONFIRM = 1;
const STRUM_MARKER_STATE_PRESS = 2;

const STRUM_EXTRA_STATE_NONE = 0;
const STRUM_EXTRA_STATE_WAIT = 1;
const STRUM_EXTRA_STATE_COMPLETED = 2;


const STRUM_MARKER_SUFFIX_CONFIRM = "confirm";
const STRUM_MARKER_SUFFIX_MARKER = "marker";
const STRUM_MARKER_SUFFIX_HIT = "press";
const STRUM_SICK_EFFECT_SUFFIX = "splash";
const STRUM_BACKGROUND_SUFFIX = "background";

const STRUM_SCRIPT_TARGET_MARKER = 0;
const STRUM_SCRIPT_TARGET_SICK_EFFECT = 1;
const STRUM_SCRIPT_TARGET_BACKGROUND = 2;
const STRUM_SCRIPT_TARGET_STRUM_LINE = 3;
const STRUM_SCRIPT_TARGET_NOTE = 4;
const STRUM_SCRIPT_TARGET_ALL = 5;

const STRUM_SCRIPT_ON_HIT_DOWN = 0;
const STRUM_SCRIPT_ON_HIT_UP = 1;
const STRUM_SCRIPT_ON_MISS = 2;
const STRUM_SCRIPT_ON_PENALITY = 3;
const STRUM_SCRIPT_ON_IDLE = 4;
const STRUM_SCRIPT_ON_ALL = 5;

/** Default sprite size of the sick effect in relation to the marker sprite size */
const STRUM_DEFAULT_SICK_EFFECT_RATIO = 2;

/** Number of notes to draw after the marker */
const STRUM_DRAW_PAST_NOTES = 1.1;

/** Marker used in auto-scroll */
const STRUM_AUTO_SCROLL_MARKER_STATE = STRUM_MARKER_STATE_CONFIRM;

/** Increases the marker duration 5% */
const STRUM_ADDITIONAL_MARKER_DURATION_RATIO = 1.05;

const FUNKIN_MARKER_DURATION = 1000.0 / 6.0;


function strum_init(id, name, x, y, marker_dimmen, invdimmen, length_dimmen, keep_markers_scale) {
    const attached_animations = {
        hit_up: { action: null, undo: null },
        hit_down: { action: null, undo: null },
        penality: { action: null, undo: null },
        miss: { action: null, undo: null },
        idle: { action: null, undo: null },
        continuous: { action: null },
        last_event: STRUM_SCRIPT_ON_ALL,
        state: STRUM_EXTRA_STATE_NONE
    };

    let strum = {
        strum_id: id,
        strum_name: strdup(name),

        sick_effect_ratio: STRUM_DEFAULT_SICK_EFFECT_RATIO,

        sprite_marker_nothing: statesprite_init_from_texture(null),
        sprite_marker_confirm: statesprite_init_from_texture(null),
        sprite_marker_press: statesprite_init_from_texture(null),
        sprite_sick_effect: statesprite_init_from_texture(null),
        sprite_background: statesprite_init_from_texture(null),

        chart_notes: null,
        chart_notes_size: 0,

        drawable_notes: null,
        attribute_notes: null,
        chart_notes_id_map: null,
        chart_notes_id_map_size: 0,

        scroll_velocity_base: FNF_CHART_SCROLL_VELOCITY,
        scroll_velocity: 0,

        scroll_is_vertical: 1,
        scroll_is_inverse: 0,
        scroll_window: 0,

        last_song_timestamp: 0,
        notes_peek_index: 0,
        notes_render_index: 0,

        press_state_changes: -1,
        press_state: STRUM_PRESS_STATE_NONE,
        press_state_use_alt_anim: 0,

        marker_state: STRUM_MARKER_STATE_NOTHING,
        marker_state_changed: 0,
        marker_sick_state: STRUM_MARKER_STATE_NOTHING,

        marker_sick_state_name: null,
        selected_sick_effect_state: null,

        dimmen_length: length_dimmen,
        dimmen_opposite: invdimmen,

        dimmen_marker: marker_dimmen,
        marker_duration: 0,
        marker_duration_quarter: 0,
        minimum_sustain_duration: 0,
        marker_duration_multiplier: 1.0,

        key_test_limit: 0,

        modifier: {},

        enable_sick_effect: 1,
        enable_sick_effect_draw: 1,
        enable_background: 0,

        keep_aspect_ratio_background: 1,

        markers_scale_keep: keep_markers_scale,
        markers_scale: -1.0,

        selected_notes_state: null,

        // In C the following fields are not pointers
        animation_strum_line: clone_object(attached_animations),
        animation_note: clone_object(attached_animations),
        animation_marker: clone_object(attached_animations),
        animation_sick_effect: clone_object(attached_animations),
        animation_background: clone_object(attached_animations),

        extra_animations_have_penalties: 0,
        extra_animations_have_misses: 0,

        auto_scroll_elapsed: -1,
        sustain_queue: arraylist_init(),

        drawable: drawable_init(-1, null, null, null),
        draw_offset_milliseconds: 0,
        player_id: -1,
        inverse_offset: length_dimmen - marker_dimmen,
        use_fukin_marker_duration: 1,
        tweenkeyframe_note: null,
        use_beat_synced_idle_and_continous: 1,
        beatwatcher: {}
    };

    // set sprites location and modifier location
    strum_update_draw_location(strum, x, y);

    // set sprites size
    statesprite_set_draw_size(strum.sprite_marker_nothing, marker_dimmen, marker_dimmen);
    statesprite_set_draw_size(strum.sprite_marker_confirm, marker_dimmen, marker_dimmen);
    statesprite_set_draw_size(strum.sprite_marker_press, marker_dimmen, marker_dimmen);
    statesprite_set_draw_size(strum.sprite_sick_effect, marker_dimmen, marker_dimmen);
    statesprite_set_draw_size(strum.sprite_background, marker_dimmen, length_dimmen);


    strum_set_scroll_speed(strum, 1.0);
    strum_set_scroll_direction(strum, STRUM_UPSCROLL);
    strum_set_bpm(strum, 100);

    return strum;
}

function strum_destroy(strum) {
    ModuleLuaScript.kdmyEngine_drop_shared_object(strum);

    statesprite_destroy(strum.sprite_marker_nothing);
    statesprite_destroy(strum.sprite_marker_confirm);
    statesprite_destroy(strum.sprite_marker_press);
    statesprite_destroy(strum.sprite_sick_effect);
    statesprite_destroy(strum.sprite_background);

    strum.strum_name = undefined;

    if (strum.chart_notes) strum.chart_notes = undefined;
    if (strum.drawable_notes) strum.drawable_notes = undefined;
    if (strum.attribute_notes) strum.attribute_notes = undefined;
    if (strum.chart_notes_id_map) strum.chart_notes_id_map = undefined;

    if (strum.selected_notes_state) strum.selected_notes_state = undefined;
    if (strum.selected_sick_effect_state) strum.selected_sick_effect_state = undefined;

    strum_internal_extra_destroy_animation(strum.animation_marker);
    strum_internal_extra_destroy_animation(strum.animation_sick_effect);
    strum_internal_extra_destroy_animation(strum.animation_background);
    strum_internal_extra_destroy_animation(strum.animation_strum_line);
    strum_internal_extra_destroy_animation(strum.animation_note);

    if (strum.tweenkeyframe_note) tweenkeyframe_destroy(strum.tweenkeyframe_note);

    arraylist_destroy(strum.sustain_queue, 0);

    // Note: do not release "strum.marker_sick_state_name"

    drawable_destroy(strum.drawable);
    strum = undefined;
}

function strum_set_notes(strum, chart, strumsdefs, strumsdefs_size, player_id, notepool) {
    if (strum.chart_notes) {
        // free() previous used resources
        strum.chart_notes_id_map = undefined;
        strum.chart_notes = undefined;
        strum.chart_notes_size = 0;
        strum.key_test_limit = -Infinity;
    }

    strum_internal_reset_scrolling(strum);// obligatory

    // validate ids
    let strum_id = strum.strum_id;
    if (strum_id < 0 || strum_id >= strumsdefs_size) return -1;
    if (player_id < 0 || player_id >= chart.entries_size) return 0;

    let notes_ids = strumsdefs[strum_id].notes_ids;
    let note_ids_size = strumsdefs[strum_id].notes_ids_size;

    let player_notes = chart.entries[player_id].notes;
    let player_notes_size = chart.entries[player_id].notes_size;

    // step 1: count the notes required for this strum
    let count = 0;
    for (let i = 0; i < player_notes_size; i++) {
        let note_direction = player_notes[i].direction;
        for (let j = 0; j < note_ids_size; j++) {
            if (note_direction == notes_ids[j]) {
                count++;
                break;
            }
        }
    }

    // step 2: map all note IDs
    strum.chart_notes_id_map = new Array(note_ids_size);
    strum.chart_notes_id_map_size = note_ids_size;

    for (let i = 0; i < note_ids_size; i++)
        strum.chart_notes_id_map[i] = notes_ids[i];

    // step 3: grab notes from the chart
    strum.chart_notes = new Array(count);
    strum.chart_notes_size = count;

    let k = 0;
    for (let i = 0; i < player_notes_size; i++) {
        let ignore = 1;
        for (let j = 0; j < note_ids_size; j++) {
            if (player_notes[i].direction == notes_ids[j]) {
                ignore = 0;
                break;
            }
        }
        if (ignore) continue;

        strum.chart_notes[k++] = new StrumNote(player_notes[i], notes_ids, note_ids_size);
    }

    if (count > 0) {
        // Important: sort the notes by timestamp
        qsort(strum.chart_notes, strum.chart_notes_size, NaN, StrumNote.sort_callback);

        // calculate the key test time limit
        strum.key_test_limit = Math.max(strum.chart_notes[0].timestamp - strum.marker_duration, 0);

        // remove duplicated notes (filtered by timestamp and id)
        let j = 0;
        let last_timestamp = NaN;
        let last_id = -1;
        for (let i = 0; i < strum.chart_notes_size; i++) {
            let id = strum.chart_notes[i].id;
            let timestamp = strum.chart_notes[i].timestamp;
            if (timestamp == last_timestamp && id == last_id) {
                console.error(`strum_set_notes() duplicated note found: ts=${timestamp} id=${id}`);
            } else {
                last_timestamp = timestamp;
                last_id = id;
                strum.chart_notes[j++] = strum.chart_notes[i];// in C clone as struct
            }
        }
        if (j != strum.chart_notes_size) {
            // trim array
            strum.chart_notes_size = j;
            strum.chart_notes = realloc(strum.chart_notes, strum.chart_notes_size /* * sizeof(StrumNote) */);
        }

    } else {
        strum.key_test_limit = -Infinity;
    }


    if (strum.strum_name != null)
        strum_internal_set_note_drawables(strum, notepool);


    return count;
}

function strum_force_rebuild(strum, marker_dimmen, invdimmen, length_dimmen, keep_markers_scale) {
    //
    // Note: the caller MUST change later the scroll direction in order to take effect
    //
    console.warn("strum_force_rebuild() was called, this never should happen");

    strum.dimmen_length = length_dimmen;
    strum.dimmen_opposite = invdimmen;
    strum.dimmen_marker = marker_dimmen;
    strum.markers_scale_keep = keep_markers_scale;

    // set sprites size
    statesprite_set_draw_size(strum.sprite_marker_nothing, marker_dimmen, marker_dimmen);
    statesprite_set_draw_size(strum.sprite_marker_confirm, marker_dimmen, marker_dimmen);
    statesprite_set_draw_size(strum.sprite_marker_press, marker_dimmen, marker_dimmen);
    statesprite_set_draw_size(strum.sprite_sick_effect, marker_dimmen, marker_dimmen);
    statesprite_set_draw_size(strum.sprite_background, marker_dimmen, length_dimmen);
}

function strum_update_draw_location(strum, x, y) {
    statesprite_set_draw_location(strum.sprite_marker_nothing, x, y);
    statesprite_set_draw_location(strum.sprite_marker_confirm, x, y);
    statesprite_set_draw_location(strum.sprite_marker_press, x, y);
    statesprite_set_draw_location(strum.sprite_sick_effect, x, y);
    statesprite_set_draw_location(strum.sprite_background, x, y);

    strum.modifier.x = x;
    strum.modifier.y = y;
}

function strum_set_scroll_speed(strum, speed) {
    strum.scroll_velocity = strum.scroll_velocity_base * speed;

    // Use half of the scolling speed if the screen aspect ratio is 4:3 (dreamcast)
    if (!pvrctx_is_widescreen()) strum.scroll_velocity *= 0.5;//480.0 / 720.0;

    strum_internal_calc_marker_duration(strum, strum.scroll_velocity);
    strum_internal_calc_scroll_window(strum);
}

function strum_set_scroll_direction(strum, direction) {
    strum.scroll_is_vertical = direction == STRUM_UPSCROLL || direction == STRUM_DOWNSCROLL;
    strum.scroll_is_inverse = direction == STRUM_DOWNSCROLL || direction == STRUM_RIGHTSCROLL;

    if (strum.scroll_is_vertical) {
        strum.modifier.width = strum.dimmen_marker;
        strum.modifier.height = strum.dimmen_length;
    } else {
        strum.modifier.width = strum.dimmen_length;
        strum.modifier.height = strum.dimmen_marker;
    }

    strum_internal_calc_scroll_window(strum);
    strum_internal_states_recalculate(strum);
}

function strum_set_marker_duration_multiplier(strum, multipler) {
    strum.marker_duration_multiplier = multipler > 0 ? multipler : 1.0;

    let velocity = strum.scroll_velocity;

    strum_internal_calc_marker_duration(strum, velocity);
}

function strum_reset(strum, scroll_speed, state_name) {
    strum_internal_reset_scrolling(strum);

    drawable_set_alpha(strum.drawable, 1.0);
    drawable_set_offsetcolor_to_default(strum.drawable);

    for (let i = 0; i < strum.chart_notes_size; i++) {
        strum.chart_notes[i].state = NoteState.PENDING;
        strum.chart_notes[i].hit_diff = NaN;
        strum.chart_notes[i].release_time = -1.0;
        strum.chart_notes[i].release_button = 0x00;
        strum.chart_notes[i].previous_quarter = 0;
        strum.chart_notes[i].hit_on_penality = 0;
    }

    strum_set_scroll_speed(strum, scroll_speed);
    strum_state_toggle(strum, state_name);
}

function strum_scroll(strum, song_timestamp, ddrkeys_fifo, playerstats, weekscript) {
    /** @type {StrumNote[]} */
    const chart_notes = strum.chart_notes;
    const chart_notes_size = strum.chart_notes_size;
    const marker_duration = strum.marker_duration;

    let marker_state = -1;
    let keys_processed = 0;
    let notes_peek_index = strum.notes_peek_index;
    let song_offset_timestamp = song_timestamp + marker_duration;
    let press_state = -1;
    let press_state_use_alt_anim = false;
    let notes_ahead = 0;
    let notes_cleared = 0;


    if (strum.auto_scroll_elapsed > 0) {
        // cancel strum_scroll_auto() state keep
        strum.auto_scroll_elapsed = -1;
        strum.marker_state = STRUM_MARKER_STATE_NOTHING;
        strum.marker_state_changed = 1;
    }

    // step 1: enter in "key testing" mode on the early seconds of the song or at the end
    if (song_timestamp < strum.key_test_limit || notes_peek_index >= chart_notes_size) {
        for (let i = 0; i < ddrkeys_fifo.available; i++) {
            let ddr_key = ddrkeys_fifo.queue[i];
            if (ddr_key.discard || ddr_key.strum_id != strum.strum_id) continue;
            if (ddr_key.in_song_timestamp > song_timestamp) continue;

            keys_processed++;
            ddr_key.discard = 1;
            marker_state = ddr_key.holding ? STRUM_MARKER_STATE_CONFIRM : STRUM_MARKER_STATE_NOTHING;
        }

        if (keys_processed > 0) strum_internal_update_press_state(strum, STRUM_PRESS_STATE_NONE);

        strum_internal_check_sustain_queue(strum, song_timestamp, playerstats);
        strum.last_song_timestamp = song_timestamp;
        if (marker_state != -1) {
            strum.marker_state = marker_state;
            strum.marker_state_changed = 1;
        }
        return keys_processed;
    }

    // step 2: count all notes near the song position (under normal conditions is always 1)
    for (let i = notes_peek_index; i < chart_notes_size; i++) {
        if (chart_notes[i].timestamp > song_offset_timestamp) break;
        notes_ahead++;
    }

    // step 3: do key checking (very extensive)
    for (let i = 0; i < ddrkeys_fifo.available && notes_cleared < notes_ahead; i++) {
        let note_attributes;
        const ddr_key = ddrkeys_fifo.queue[i];
        const key_timestamp = ddr_key.in_song_timestamp;
        if (ddr_key.discard || ddr_key.strum_id != strum.strum_id) continue;

        // phase 1: check key event against every note ahead
        let lowest_diff = Infinity;
        let note = null;
        let hit_index = -1;
        let index_ahead = 0;

        L_key_check:
        if (ddr_key.holding) {

            // find a candidate for this key event
            for (let j = notes_peek_index; j < chart_notes_size; j++) {
                if (index_ahead++ > notes_ahead) break;

                // check if is trying to recover a sustain note
                if (chart_notes[j].state == NoteState.RELEASE) {
                    let start = chart_notes[j].timestamp;
                    let end = start + chart_notes[j].duration;
                    if (key_timestamp < start || key_timestamp > end) continue;

                    // the timestamp is within the bounds of the note
                    note = chart_notes[j];
                    hit_index = j;

                    // ignore recover if the next note is closer
                    j++;
                    if (j < chart_notes_size && chart_notes[j].state == NoteState.PENDING) {
                        start = chart_notes[j].timestamp - (marker_duration * 0.75);
                        if (start <= key_timestamp) {
                            note = chart_notes[j];
                            hit_index = j;
                        }
                    }

                    break;
                }

                // check if the note is on pending state and near the key timestamp
                if (chart_notes[j].state != NoteState.PENDING) continue;
                if (!strum_internal_on_note_bounds(strum, j, key_timestamp)) continue;

                // check how far are from the key, if near, remember the note
                let diff = Math.abs(chart_notes[j].timestamp - key_timestamp);
                if (diff < lowest_diff) {
                    lowest_diff = diff;
                    note = chart_notes[j];
                    hit_index = j;
                }
            }

            if (!note) {
                if (ddr_key.in_song_timestamp > song_timestamp) {
                    //console.warn(`strum: [hold] in future ts_k=${key_timestamp} ts_s=${song_timestamp}`);
                }

                // maybe the key event is a penalty hit/press or future event (do not discard)
                continue;
            }

            if (note.state == NoteState.RELEASE) {
                //console.info(`strum: [hold] sustain recover!  ts_k=${key_timestamp} ts_n=${note.timestamp}`);

                if (Number.isNaN(note.hit_diff)) {
                    // never pressed compute hit
                    note_attributes = strum.attribute_notes[note.id];
                    note.hit_diff = note.timestamp - key_timestamp;
                    if (!note_attributes.ignore_hit) {
                        playerstats_add_sustain_delayed_hit(playerstats, note_attributes.heal_ratio, note.hit_diff);
                    }
                    if (note_attributes.can_kill_on_hit) playerstats_kill_if_negative_health(playerstats);
                }

                // recover the sustain note
                note.release_time = -1;
                note.state = NoteState.HOLD;
                note.release_button = ddr_key.button;

                press_state = STRUM_PRESS_STATE_HIT;
                press_state_use_alt_anim = note.alt_anim;
                break L_key_check;
            }

            // process pending note
            note_attributes = strum.attribute_notes[note.id];
            let sick_effect_ready = strum.marker_sick_state == STRUM_MARKER_STATE_NOTHING;
            lowest_diff = note.timestamp - key_timestamp;// hit difference (for stats)

            // calculate rank
            let rank = PLAYERSTATS_RANK_NONE;
            if (!note_attributes.ignore_hit) {
                rank = playerstats_add_hit(
                    playerstats, note_attributes.heal_ratio, marker_duration, lowest_diff
                );
            }

            if (note_attributes.can_kill_on_hit) playerstats_kill_if_negative_health(playerstats);
            if (weekscript) weekscript_notify_note_hit(weekscript, strum, hit_index, playerstats);

            //console.info(`strum: [hold] note hit!  ts=${key_timestamp} diff=${lowest_diff} rank=${rank}`);

            // check if necessary display the sick effect
            if (rank == PLAYERSTATS_RANK_SICK && sick_effect_ready) {
                strum.marker_sick_state = STRUM_MARKER_STATE_PRESS;

                // if the note has a custom sick effect choose from the state list
                if (note_attributes.custom_sick_effect_name)
                    strum.marker_sick_state_name = note_attributes.custom_sick_effect_name;
                else
                    strum.marker_sick_state_name = strum.selected_sick_effect_state;
            }

            if (note.duration >= strum.minimum_sustain_duration) {
                // add note to the sustain queue to check the health loss/gain later
                arraylist_add(strum.sustain_queue, note);
                press_state = STRUM_PRESS_STATE_HIT_SUSTAIN;
            } else {
                press_state = STRUM_PRESS_STATE_HIT;
            }

            press_state_use_alt_anim = note.alt_anim;
            note.state = NoteState.HOLD;
            note.release_button = ddr_key.button;
            note.release_time = note.endTimestamp;// use end timestamp to avoid compute as miss
            note.hit_diff = lowest_diff;
        } else {
            //
            // find what note is being released. Note: this method will fail
            // if there multiple key inputs which under normal circumstances
            // never should happen
            //
            for (let j = notes_peek_index; j < chart_notes_size; j++) {
                if (chart_notes[j].state != NoteState.HOLD) continue;// ignore non-hold notes
                if (chart_notes[j].release_button != ddr_key.button) continue;
                if (index_ahead++ > notes_ahead) break;

                note = chart_notes[j];
                //console.log(`strum: [release] note found!  ts_k=${key_timestamp} ts_n=${note.timestamp}`);
                break;
            }

            press_state = STRUM_PRESS_STATE_NONE;

            if (!note) {
                if (key_timestamp <= song_timestamp) {
                    //console.log(`strum: [release] empty strum at ${key_timestamp}`);
                }

                //
                // The key was pressed for too long or the penality hit/press was released.
                // Note: strum_internal_check_sustain_queue() function handles sustain notes
                // pressed longer than required.
                //
                break L_key_check;
            }

            note.release_button = 0x00;

            // clear if the note is not sustain
            if (note.duration < strum.minimum_sustain_duration) {
                //console.info(`strum: [release] clear non-sustain ts=${note.timestamp}`);
                note.state = NoteState.CLEAR;
                note.release_time = Infinity;
                notes_cleared++;

                break L_key_check;
            }

            // check if the sustain note was released early
            let end_timestamp = note.endTimestamp;

            if (key_timestamp < end_timestamp) {
                //console.log(`strum: [release] early! left duration ${end_timestamp - key_timestamp}`);

                // early release
                note.state = NoteState.RELEASE;
                note.release_time = key_timestamp;
            } else {
                //console.log(`strum: [release] sustain clear! remain was ${end_timestamp - note.release_time}`);
                arraylist_remove(strum.sustain_queue, note);
                note.state = NoteState.CLEAR;
                note.release_time = Infinity;
                notes_cleared++;
            }
        }

        // phase 2: discard the key event
        ddr_key.discard = 1;
        keys_processed++;

        // phase 3: update the marker state (press/confirm/nothing)
        if (ddr_key.holding)
            marker_state = note ? STRUM_MARKER_STATE_PRESS : STRUM_MARKER_STATE_CONFIRM;
        else
            marker_state = STRUM_MARKER_STATE_NOTHING;
    }

    // step 4: check for missed notes or early released sustains
    song_offset_timestamp = song_timestamp - marker_duration;
    for (let i = notes_peek_index, j = 0; i < chart_notes_size && j < notes_ahead; i++, j++) {
        let note = chart_notes[i];
        let compute_miss = !strum.attribute_notes[note.id].ignore_miss;
        let end_timestamp = note.timestamp + Math.max(note.duration, marker_duration);

        switch (note.state) {
            case NoteState.CLEAR:
            case NoteState.HOLD:
            case NoteState.MISS:
                continue;
        }

        // check if the note is non-sustain
        if (note.duration < strum.minimum_sustain_duration) {
            if (song_offset_timestamp >= end_timestamp) {
                //console.info(`strum: [missed] non-sustain note ts=${note.timestamp}`);
                note.state = NoteState.MISS;
                note.release_time = -Infinity;

                if (compute_miss) {
                    playerstats_add_miss(playerstats, strum.attribute_notes[note.id].hurt_ratio);
                    if (weekscript) weekscript_notify_note_loss(weekscript, strum, i, playerstats, 0);
                    strum.extra_animations_have_misses++;
                    press_state = STRUM_PRESS_STATE_MISS;
                }
            }
            continue;
        }


        //
        // On sustain notes check how many note-quarters are lost.
        // The strum_internal_check_sustain_queue() function calculates it is loosing.
        //

        if (compute_miss) {
            if (note.state == NoteState.PENDING && weekscript)
                weekscript_notify_note_loss(weekscript, strum, i, playerstats, 0);
            if (note.state != NoteState.RELEASE)
                strum.extra_animations_have_misses++;
        }

        if (song_timestamp > end_timestamp) {
            //console.info(`strum: [missed] sustain note ts=${note.timestamp} ts_end=${end_timestamp}`);
            arraylist_remove(strum.sustain_queue, note);

            note.state = NoteState.MISS;
            note.release_time = -Infinity;
            note.release_button = 0x00;
        }

        // consider as missed if the worst possible ranking can not be assigned
        let miss_timestamp = note.timestamp + marker_duration;
        if (song_timestamp >= miss_timestamp) {
            //console.info($`strum: [miss] sustain loosing ts_n=${note.timestamp} ts_s=${song_timestamp}`);
            if (note.state == NoteState.PENDING) {
                arraylist_add(strum.sustain_queue, note);
                note.state = NoteState.RELEASE;
            }
        }
    }

    // step 5: move notes_peek_index
    for (let i = 0; i < notes_ahead && notes_peek_index < chart_notes_size; i++) {
        let note_state = chart_notes[notes_peek_index].state;
        if (note_state != NoteState.CLEAR && note_state != NoteState.MISS) break;
        notes_peek_index++;
    }

    // step 6: check for released keys of pentalties presses
    for (let i = 0; i < ddrkeys_fifo.available; i++) {
        const ddr_key = ddrkeys_fifo.queue[i];
        const key_timestamp = ddr_key.in_song_timestamp;

        if (ddr_key.discard || ddr_key.holding || ddr_key.strum_id != strum.strum_id) continue;
        if (key_timestamp > song_timestamp) continue;

        if (marker_state == -1) marker_state = STRUM_MARKER_STATE_NOTHING;
        if (press_state == -1) press_state = STRUM_PRESS_STATE_NONE;

        ddr_key.discard = 1;
        keys_processed++;
    }

    // before leave, check the sustain queue
    strum_internal_check_sustain_queue(strum, song_timestamp, playerstats);

    strum.last_song_timestamp = song_timestamp;
    strum.notes_peek_index = notes_peek_index;
    if (marker_state != -1) {
        strum.marker_state = marker_state;
        strum.marker_state_changed = 1;
    }
    strum_internal_update_press_state(strum, press_state);

    if (press_state == STRUM_PRESS_STATE_HIT || press_state == STRUM_PRESS_STATE_HIT_SUSTAIN) {
        strum.press_state_use_alt_anim = press_state_use_alt_anim;
    }

    return keys_processed;
}

function strum_scroll_auto(strum, song_timestamp, playerstats, weekscript) {
    if (strum.chart_notes_size < 1) return;

    const chart_notes = strum.chart_notes;
    const attribute_notes = strum.attribute_notes;
    const chart_notes_size = strum.chart_notes_size;
    const marker_duration = strum.marker_duration;
    const press_time = strum.marker_duration * 2;

    let notes_peek_index = strum.notes_peek_index;
    let notes_cleared = 0;

    let press_state = -1;
    let press_state_use_alt_anim = false;

    // clear all notes ahead, but keep in hold the sustain ones
    for (; notes_peek_index < chart_notes_size; notes_peek_index++) {
        /** @type {StrumNote} */
        let note = chart_notes[notes_peek_index];
        let is_sustain = note.duration >= strum.minimum_sustain_duration;
        let end_timestamp = note.endTimestamp;

        if (note.timestamp > song_timestamp) break;

        if (note.state == NoteState.PENDING || note.state == NoteState.RELEASE) {

            if (note.state == NoteState.PENDING) {
                if (attribute_notes[note.id].can_kill_on_hit) {
                    if (!attribute_notes[note.id].ignore_miss)
                        playerstats_add_miss(playerstats, attribute_notes[note.id].hurt_ratio);
                } else if (!attribute_notes[note.id].ignore_hit) {
                    let heal_ratio = attribute_notes[note.id].heal_ratio;
                    note.hit_diff = 0.0;
                    playerstats_add_hit(playerstats, heal_ratio, marker_duration, note.hit_diff);

                    // keep the marker in press state for a while
                    strum.auto_scroll_elapsed = note.duration < 1 ? press_time : note.duration;
                    strum.marker_state = STRUM_AUTO_SCROLL_MARKER_STATE;
                    strum.marker_state_changed = 1;
                    press_state = is_sustain ? STRUM_PRESS_STATE_HIT_SUSTAIN : STRUM_PRESS_STATE_HIT;
                    press_state_use_alt_anim = note.alt_anim;
                }

                if (weekscript) {
                    if (attribute_notes[note.id].can_kill_on_hit)
                        weekscript_notify_note_loss(weekscript, strum, notes_peek_index, playerstats, 0);
                    else
                        weekscript_notify_note_hit(weekscript, strum, notes_peek_index, playerstats);
                }

                if (is_sustain) arraylist_add(strum.sustain_queue, note);
                else notes_cleared++;
            }

            note.release_button = 0x00;

            if (attribute_notes[note.id].can_kill_on_hit) {
                note.state = is_sustain ? NoteState.RELEASE : NoteState.MISS;
            } else {
                note.state = is_sustain ? NoteState.HOLD : NoteState.CLEAR;
                note.release_time = is_sustain ? end_timestamp : Infinity;
            }
        } else if (note.state == NoteState.HOLD) {
            if (song_timestamp >= end_timestamp) {
                note.state = NoteState.CLEAR;
                note.release_time = Infinity;
                note.release_button = 0x00;
                notes_cleared++;

                press_state = STRUM_PRESS_STATE_NONE;
                if (is_sustain) arraylist_remove(strum.sustain_queue, note);
            }
        } else if (note.state == NoteState.RELEASE && song_timestamp >= end_timestamp) {
            note.state = NoteState.MISS;
            note.release_time = -Infinity;
            note.release_button = 0x00;
            notes_cleared++;

            press_state = STRUM_PRESS_STATE_NONE;
            if (is_sustain) arraylist_remove(strum.sustain_queue, note);
        }
    }

    strum_internal_update_press_state(strum, press_state);
    strum_internal_check_sustain_queue(strum, song_timestamp, playerstats);
    strum.last_song_timestamp = song_timestamp;

    if (press_state == STRUM_PRESS_STATE_HIT || press_state == STRUM_PRESS_STATE_HIT_SUSTAIN) {
        strum.press_state_use_alt_anim = press_state_use_alt_anim;
    }

    let count = notes_peek_index - strum.notes_peek_index;
    if (notes_cleared < count) return;

    strum.notes_peek_index = notes_peek_index;

}

function strum_force_key_release(strum) {
    strum.marker_state = STRUM_MARKER_STATE_NOTHING;
    strum.marker_state_changed = 1;
    strum_internal_update_press_state(strum, STRUM_PRESS_STATE_NONE);

    for (let i = strum.notes_peek_index; i < strum.chart_notes_size; i++) {
        let note = strum.chart_notes[i];
        if (note.state == NoteState.HOLD) {
            note.state = NoteState.RELEASE;
            note.release_time = Math.max(strum.last_song_timestamp, note.timestamp);
        }
    }

    strum_animate(strum, 0);
}

function strum_find_penalties_note_hit(strum, song_timestamp, ddrkeys_fifo, playerstats, weekscript) {
    if (ddrkeys_fifo.available < 1 || strum.chart_notes_size < 1) return 0;

    const chart_notes = strum.chart_notes;
    const chart_notes_size = strum.chart_notes_size;
    const attribute_notes = strum.attribute_notes;

    let keys_processed = 0;

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

    for (let i = Math.min(strum.notes_peek_index, chart_notes_size - 1); i < chart_notes_size; i++) {
        let note = chart_notes[i];
        if (note.state != NoteState.PENDING) continue;
        if (note.duration >= strum.minimum_sustain_duration) continue;

        for (let j = 0; j < ddrkeys_fifo.available; j++) {
            let ddr_key = ddrkeys_fifo.queue[j];
            let key_timestamp = ddr_key.in_song_timestamp;

            if (ddr_key.discard || ddr_key.strum_invisible || !ddr_key.holding) continue;
            if (ddr_key.strum_id == strum.strum_id) continue;
            if (ddr_key.in_song_timestamp > song_timestamp) continue;

            if (strum_internal_on_note_bounds(strum, i, key_timestamp)) {
                let diff = Math.abs(strum.chart_notes[i].timestamp - key_timestamp);
                diff /= strum.marker_duration;

                // ignore if the accuracy less than 50%
                if (diff < 0.50) continue;

                //console.log(`strum: [penality] hit on pending note ts=${note.timestamp}`);
                strum.extra_animations_have_penalties = 1;
                strum_internal_update_press_state(strum, STRUM_PRESS_STATE_PENALTY_HIT);

                if (!attribute_notes[note.id].ignore_miss) {
                    note.state = NoteState.MISS;
                    note.hit_on_penality = 1;
                    note.release_button = 0x00;
                    note.release_time = -Infinity;
                    playerstats_add_penality(playerstats, 0);
                }
                if (weekscript) weekscript_notify_note_loss(weekscript, strum, i, playerstats, 1);
            } else {
                // ignore key event
                continue;
            }

            ddr_key.discard = 1;
            keys_processed++;
        }
    }

    return keys_processed;
}

function strum_find_penalties_empty_hit(strum, song_timestamp, ddrkeys_fifo, playerstats) {
    if (ddrkeys_fifo.available < 1 || strum.chart_notes_size < 1) return 0;

    let keys_processed = 0;

    //
    // How this function works:
    //
    //  * Check if the player is pressing a button on a strum empty.
    //
    //  * strum_find_penalties_note_hit() function must be called before doing this.
    //

    for (let i = 0; i < ddrkeys_fifo.available; i++) {
        let ddr_key = ddrkeys_fifo.queue[i];
        let key_timestamp = ddr_key.in_song_timestamp;

        if (ddr_key.discard || ddr_key.strum_id != strum.strum_id) continue;
        if (key_timestamp > song_timestamp) continue;// maybe is a future penality

        if (ddr_key.holding) {
            //console.log(`strum: [penality] key hold on empty strum ts=${key_timestamp}`);
            playerstats_add_penality(playerstats, 1);
            strum_internal_update_press_state(strum, STRUM_PRESS_STATE_PENALTY_NOTE);
            strum.marker_state = STRUM_MARKER_STATE_CONFIRM;
            strum.extra_animations_have_penalties = 1;
        } else {
            //console.log(`strum: [penality] key release on empty strum ts=${key_timestamp}`);
            strum.marker_state = STRUM_MARKER_STATE_NOTHING;
            strum_internal_update_press_state(strum, STRUM_PRESS_STATE_NONE);
        }

        strum.marker_state_changed = 1;
        ddr_key.discard = 1;
        keys_processed++;
    }

    return keys_processed;
}


function strum_get_press_state_changes(strum) {
    return strum.press_state_changes;
}

function strum_get_press_state(strum) {
    return strum.press_state;
}

function strum_get_press_state_use_alt_anim(strum) {
    return strum.press_state_use_alt_anim;
}

function strum_get_name(strum) {
    return strum.strum_name;
}

function strum_get_marker_duration(strum) {
    return strum.marker_duration;
}

function strum_use_funkin_maker_duration(strum, enable) {
    strum.use_fukin_marker_duration = !!enable;
    strum_internal_calc_marker_duration(strum, strum.scroll_velocity);
}

function strum_set_player_id(strum, player_id) {
    strum.player_id = player_id;
}


function strum_animate(strum, elapsed) {
    if (!strum.strum_name) return 1;

    if (beatwatcher_poll(strum.beatwatcher) && strum.use_beat_synced_idle_and_continous) {
        statesprite_animation_restart(strum.sprite_marker_nothing);
        strum_internal_restart_extra_continous(strum);
    }

    let res = 0;

    //#region marker animation

    // if strum_scroll_auto() function was used, keep the marker in press state for a while
    if (strum.auto_scroll_elapsed > 0) {
        if (strum.marker_state != STRUM_AUTO_SCROLL_MARKER_STATE) {
            strum.auto_scroll_elapsed = -1;
        } else {
            strum.auto_scroll_elapsed -= elapsed;
            if (strum.auto_scroll_elapsed <= 0) {
                strum.auto_scroll_elapsed = -1;
                strum.marker_state = STRUM_MARKER_STATE_NOTHING;
                strum.marker_state_changed = 1;
                strum_internal_update_press_state(strum, STRUM_PRESS_STATE_NONE);
            }
        }
    }

    let current_marker;
    switch (strum.marker_state) {
        case STRUM_MARKER_STATE_CONFIRM:
            current_marker = strum.sprite_marker_confirm;
            break;
        case STRUM_MARKER_STATE_PRESS:
            current_marker = strum.sprite_marker_press;
            break;
        case STRUM_MARKER_STATE_NOTHING:
            current_marker = strum.sprite_marker_nothing;
            break;
        default:
            return 1;
    }

    // restart the marker animation (if was changed)
    if (strum.marker_state_changed) {
        strum.marker_state_changed = 0;
        statesprite_animation_restart(current_marker);
    }

    res += statesprite_animate(current_marker, elapsed);
    //#endregion

    // background animation
    if (strum.enable_background)
        res += statesprite_animate(strum.sprite_background, elapsed);

    //#region splash effect animation
    if (strum.enable_sick_effect) {
        // check if is necessary restart the sick effect animation
        let visible = 1;
        switch (strum.marker_sick_state) {
            case STRUM_MARKER_STATE_NOTHING:
                visible = 0;
                break;
            case STRUM_MARKER_STATE_PRESS:
                // change the state if the note has a custom sick effect
                statesprite_state_toggle(strum.sprite_sick_effect, strum.marker_sick_state_name);

                // play the splash effect animation again
                statesprite_animation_restart(strum.sprite_sick_effect);
                strum.marker_sick_state = STRUM_MARKER_STATE_CONFIRM;
                break;
        }

        // hide if ended
        if (visible && statesprite_animate(strum.sprite_sick_effect, elapsed))
            strum.marker_sick_state = STRUM_MARKER_STATE_NOTHING;
    }
    //#endregion

    //#region compute & animate extra animations
    let current_event;
    if (strum.extra_animations_have_penalties) {
        current_event = STRUM_SCRIPT_ON_PENALITY;
        strum.extra_animations_have_misses = strum.extra_animations_have_penalties = 0;
    } else if (strum.extra_animations_have_misses) {
        current_event = STRUM_SCRIPT_ON_MISS;
        strum.extra_animations_have_penalties = strum.extra_animations_have_misses = 0;
    } else if (strum.marker_state == STRUM_MARKER_STATE_PRESS) {
        current_event = STRUM_SCRIPT_ON_HIT_DOWN;
    } else {
        current_event = STRUM_SCRIPT_ON_IDLE;
    }

    res += strum_internal_extra_animate(strum, STRUM_SCRIPT_TARGET_STRUM_LINE, current_event, 0, elapsed);
    res += strum_internal_extra_animate(strum, STRUM_SCRIPT_TARGET_NOTE, current_event, 0, elapsed);
    res += strum_internal_extra_animate(strum, STRUM_SCRIPT_TARGET_MARKER, current_event, 0, elapsed);
    res += strum_internal_extra_animate(strum, STRUM_SCRIPT_TARGET_SICK_EFFECT, current_event, 0, elapsed);
    res += strum_internal_extra_animate(strum, STRUM_SCRIPT_TARGET_BACKGROUND, current_event, 0, elapsed);
    //#endregion

    //#region execute continuous extra animations
    strum_internal_extra_animate_sprite(
        strum, STRUM_SCRIPT_TARGET_STRUM_LINE, strum.animation_strum_line.continuous.action
    );
    strum_internal_extra_animate_sprite(
        strum, STRUM_SCRIPT_TARGET_NOTE, strum.animation_note.continuous.action
    );
    strum_internal_extra_animate_sprite(
        strum, STRUM_SCRIPT_TARGET_MARKER, strum.animation_marker.continuous.action
    );
    strum_internal_extra_animate_sprite(
        strum, STRUM_SCRIPT_TARGET_SICK_EFFECT, strum.animation_sick_effect.continuous.action
    );
    strum_internal_extra_animate_sprite(
        strum, STRUM_SCRIPT_TARGET_BACKGROUND, strum.animation_background.continuous.action
    );
    //#endregion

    return res;
}

function strum_draw(strum, pvrctx) {
    if (!strum.strum_name) return;

    pvr_context_save(pvrctx);
    drawable_helper_apply_in_context(strum.drawable, pvrctx);

    // draw the background
    if (strum.enable_background) {
        statesprite_draw(strum.sprite_background, pvrctx);
    }

    // draw the strum marker
    if (strum.marker_state == STRUM_MARKER_STATE_NOTHING) {
        statesprite_draw(strum.sprite_marker_nothing, pvrctx);
    }

    // draw the note sick hit effect
    if (strum.enable_sick_effect && strum.marker_sick_state == STRUM_MARKER_STATE_CONFIRM) {
        statesprite_draw(strum.sprite_sick_effect, pvrctx);
    }

    // check if there notes to draw
    if (strum.notes_render_index >= strum.chart_notes_size) {
        // nothing to draw, update the marker (if required) and return
        switch (strum.marker_state) {
            case STRUM_MARKER_STATE_CONFIRM:
                statesprite_draw(strum.sprite_marker_confirm, pvrctx);
                break;
            case STRUM_MARKER_STATE_PRESS:
                statesprite_draw(strum.sprite_marker_press, pvrctx);
                break;
        }

        pvr_context_restore(pvrctx);
        return;
    }

    // toggle state on the notes (because note objects can be shared with other strum lines)
    for (let i = 0; i < strum.chart_notes_id_map_size; i++) {
        if (strum.drawable_notes[i])
            note_state_toggle(strum.drawable_notes[i], strum.selected_notes_state);
    }


    //
    // setup everything to start drawing notes
    //

    const song_timestamp = strum.last_song_timestamp + strum.draw_offset_milliseconds;
    let notes_render_index = strum.notes_render_index;
    let draw_x = strum.modifier.x;
    let draw_y = strum.modifier.y;
    let song_window_past = song_timestamp - (strum.marker_duration * STRUM_DRAW_PAST_NOTES);
    let song_window_future = song_timestamp + strum.scroll_window;

    if (strum.scroll_is_inverse) {
        if (strum.scroll_is_vertical)
            draw_y += strum.inverse_offset;
        else
            draw_x += strum.inverse_offset;
    }

    for (let i = notes_render_index; i < strum.chart_notes_size; i++) {
        let note = strum.chart_notes[i];

        if (note.EndTimestamp <= song_window_past) {
            notes_render_index++;
            continue;
        } else if (note.timestamp >= song_window_future) {
            // future note, stop here
            break;
        }

        // do not draw cleared notes or invisible ones
        if (note.state == NoteState.CLEAR) continue;
        if (note.state == NoteState.HOLD && note.duration < 1) continue;
        if (note.state == NoteState.MISS && note.hit_on_penality) continue;
        if (!strum.drawable_notes[note.id]) continue;

        let note_duration = note.duration;
        let note_timestamp = note.timestamp;
        let body_only = 0;

        // on sustain notes only draw the remaining duration
        if (note.duration > 0) {
            if (note.state == NoteState.HOLD) {
                body_only = 1;
                note_duration -= song_timestamp - note_timestamp;
                note_timestamp = song_timestamp;
            } else if (note.release_time > 0) {
                // early release
                body_only = 1;
                note_duration -= note.release_time - note_timestamp;
                note_timestamp = note.release_time;
            }

            if (note.state != NoteState.HOLD && note_timestamp <= song_window_past) {
                // past note, show part of the sustain body 
                body_only = 1;
                let past_note_timestamp = song_timestamp - strum.marker_duration;
                note_duration -= past_note_timestamp - note_timestamp;
                note_timestamp = past_note_timestamp;

                // ignore short sustain duration
                if (body_only && note_duration < strum.marker_duration) continue;
            }

            if (note_duration <= 0) continue;
        }

        let scroll_offset;
        let x = draw_x;
        let y = draw_y;
        let drawable_note = strum.drawable_notes[note.id];

        if (strum.scroll_is_inverse)
            scroll_offset = song_timestamp - note_timestamp;
        else
            scroll_offset = note_timestamp - song_timestamp;

        scroll_offset *= strum.scroll_velocity;

        if (strum.tweenkeyframe_note) {
            let percent = Math.abs(scroll_offset / strum.scroll_window);
            tweenkeyframe_animate_percent(strum.tweenkeyframe_note, percent);
            tweenkeyframe_vertex_set_properties(
                strum.tweenkeyframe_note, note_peek_alone_statesprite(drawable_note), statesprite_set_property
            );
        }

        if (strum.scroll_is_vertical)
            y += scroll_offset;
        else
            x += scroll_offset;

        note_draw(drawable_note, pvrctx, strum.scroll_velocity, x, y, note_duration, body_only);

        if (strum.tweenkeyframe_note) {
            //
            // after the note is drawn, "attempt" to restore the original values
            // by running again the TweenKeyframe at 0%.
            //
            tweenkeyframe_animate_percent(strum.tweenkeyframe_note, 0.0);
            tweenkeyframe_vertex_set_properties(
                strum.tweenkeyframe_note, note_peek_alone_statesprite(drawable_note), statesprite_set_property
            );
        }
    }

    switch (strum.marker_state) {
        case STRUM_MARKER_STATE_CONFIRM:
            statesprite_draw(strum.sprite_marker_confirm, pvrctx);
            break;
        case STRUM_MARKER_STATE_PRESS:
            statesprite_draw(strum.sprite_marker_press, pvrctx);
            break;
    }

    strum.notes_render_index = notes_render_index;
    pvr_context_restore(pvrctx);
}


function strum_enable_background(strum, enable) {
    strum.enable_background = !!enable;
}

function strum_enable_sick_effect(strum, enable) {
    strum.enable_sick_effect = !!enable;
    statesprite_set_visible(
        strum.sprite_sick_effect, strum.enable_sick_effect_draw && strum.enable_sick_effect
    );
}

function strum_state_add(strum, mdlhldr_mrkr, mdlhldr_sck_ffct, mdlhldr_bckgrnd, state_name) {
    let state;

    // marker nothing
    strum.markers_scale = -1.0;
    state = strum_internal_state_add(
        strum.sprite_marker_nothing, mdlhldr_mrkr, strum.strum_name,
        STRUM_MARKER_SUFFIX_MARKER, state_name
    );
    if (state) strum_internal_calc_state_marker(strum, state);

    // marker confirm
    state = strum_internal_state_add(
        strum.sprite_marker_confirm, mdlhldr_mrkr, strum.strum_name,
        STRUM_MARKER_SUFFIX_CONFIRM, state_name
    );
    if (state) strum_internal_calc_state_marker(strum, state);

    // marker press
    state = strum_internal_state_add(
        strum.sprite_marker_press, mdlhldr_mrkr, strum.strum_name,
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
        strum.sprite_background, mdlhldr_bckgrnd, strum.strum_name,
        STRUM_BACKGROUND_SUFFIX, state_name
    );
    if (state) strum_internal_calc_state_background(strum, state);

}

function strum_state_toggle(strum, state_name) {
    let success = 0;
    success += strum_state_toggle_marker(strum, state_name);
    success += strum_state_toggle_sick_effect(strum, state_name);
    success += strum_state_toggle_background(strum, state_name);
    success += strum_state_toggle_notes(strum, state_name);
    return success;
}

function strum_state_toggle_notes(strum, state_name) {
    if (strum.selected_notes_state) strum.selected_notes_state = undefined;
    strum.selected_notes_state = strdup(state_name);

    return 1;
}

function strum_state_toggle_sick_effect(strum, state_name) {
    if (strum.selected_sick_effect_state) strum.selected_sick_effect_state = undefined;
    strum.selected_sick_effect_state = strdup(state_name);

    return statesprite_state_toggle(strum.sprite_sick_effect, state_name);
}

function strum_state_toggle_marker(strum, state_name) {
    let res = 0;

    // toggle marker sub-states (yeah state of states)
    res += statesprite_state_toggle(strum.sprite_marker_nothing, state_name);
    res += statesprite_state_toggle(strum.sprite_marker_confirm, state_name);
    res += statesprite_state_toggle(strum.sprite_marker_press, state_name);

    return res;
}

function strum_state_toggle_background(strum, state_name) {
    return statesprite_state_toggle(strum.sprite_background, state_name);
}


function strum_set_alpha_background(strum, alpha) {
    let old_alpha = statesprite_get_alpha(strum.sprite_background);
    statesprite_set_alpha(strum.sprite_background, alpha);
    return old_alpha;
}

function strum_set_alpha_sick_effect(strum, alpha) {
    let old_alpha = statesprite_get_alpha(strum.sprite_sick_effect);
    statesprite_set_alpha(strum.sprite_sick_effect, alpha);
    return old_alpha;
}

function strum_set_keep_aspect_ratio_background(strum, enable) {
    strum.keep_aspect_ratio_background = !!enable;
    strum_internal_calc_states(strum, strum.sprite_background, strum_internal_calc_state_background);
}

function strum_draw_sick_effect_apart(strum, enable) {
    strum.enable_sick_effect_draw = !!enable;
    statesprite_set_visible(strum.sprite_sick_effect, 0);
    return strum.sprite_sick_effect;
}


function strum_set_extra_animation(strum, strum_script_target, strum_script_on, undo, animsprite) {
    if (strum_script_target == STRUM_SCRIPT_TARGET_ALL) {
        strum_set_extra_animation(strum, STRUM_SCRIPT_TARGET_MARKER, strum_script_on, undo, animsprite);
        strum_set_extra_animation(strum, STRUM_SCRIPT_TARGET_SICK_EFFECT, strum_script_on, undo, animsprite);
        strum_set_extra_animation(strum, STRUM_SCRIPT_TARGET_BACKGROUND, strum_script_on, undo, animsprite);
        strum_set_extra_animation(strum, STRUM_SCRIPT_TARGET_STRUM_LINE, strum_script_on, undo, animsprite);
        strum_set_extra_animation(strum, STRUM_SCRIPT_TARGET_NOTE, strum_script_on, undo, animsprite);
        return;
    }

    let extra_animations = strum_internal_extra_get_holder(strum, strum_script_target);
    if (!extra_animations) {
        console.warn(`strum_set_extra_animation() unknown strum_script_target=${strum_script_target}`);
        return;
    }

    let action_undo_anims;

    switch (strum_script_on) {
        case STRUM_SCRIPT_ON_HIT_UP:
            action_undo_anims = extra_animations["hit_up"];
            break;
        case STRUM_SCRIPT_ON_HIT_DOWN:
            action_undo_anims = extra_animations["hit_down"];
            break;
        case STRUM_SCRIPT_ON_MISS:
            action_undo_anims = extra_animations["miss"];
            break;
        case STRUM_SCRIPT_ON_PENALITY:
            action_undo_anims = extra_animations["penality"];
            break;
        case STRUM_SCRIPT_ON_IDLE:
            action_undo_anims = extra_animations["idle"];
            break;
        case STRUM_SCRIPT_ON_ALL:
            strum_set_extra_animation(strum, strum_script_target, STRUM_SCRIPT_ON_HIT_UP, undo, animsprite);
            strum_set_extra_animation(strum, strum_script_target, STRUM_SCRIPT_ON_HIT_DOWN, undo, animsprite);
            strum_set_extra_animation(strum, strum_script_target, STRUM_SCRIPT_ON_MISS, undo, animsprite);
            strum_set_extra_animation(strum, strum_script_target, STRUM_SCRIPT_ON_IDLE, undo, animsprite);
            strum_set_extra_animation(strum, strum_script_target, STRUM_SCRIPT_ON_PENALITY, undo, animsprite);
            return;
        default:
            console.warn("strum_set_extra_animation() unknown strum_script_on=" + strum_script_on);
            return;
    }

    let property = undo ? "undo" : "action";

    if (action_undo_anims[property])
        animsprite_destroy(action_undo_anims[property]);

    // obligatory, make a copy of animsprite
    action_undo_anims[property] = animsprite ? animsprite_clone(animsprite) : animsprite;
}

function strum_set_extra_animation_continuous(strum, strum_script_target, animsprite) {
    let attached_animations;

    switch (strum_script_target) {
        case STRUM_SCRIPT_TARGET_MARKER:
            attached_animations = strum.animation_marker;
            break;
        case STRUM_SCRIPT_TARGET_SICK_EFFECT:
            attached_animations = strum.animation_sick_effect;
            break;
        case STRUM_SCRIPT_TARGET_BACKGROUND:
            attached_animations = strum.animation_background;
            break;
        case STRUM_SCRIPT_TARGET_STRUM_LINE:
            attached_animations = strum.animation_strum_line;
            break;
        case STRUM_SCRIPT_TARGET_NOTE:
            attached_animations = strum.animation_note;
            break;
        case STRUM_SCRIPT_TARGET_ALL:
            strum_set_extra_animation_continuous(strum, STRUM_SCRIPT_TARGET_MARKER, animsprite);
            strum_set_extra_animation_continuous(strum, STRUM_SCRIPT_TARGET_SICK_EFFECT, animsprite);
            strum_set_extra_animation_continuous(strum, STRUM_SCRIPT_TARGET_BACKGROUND, animsprite);
            strum_set_extra_animation_continuous(strum, STRUM_SCRIPT_TARGET_STRUM_LINE, animsprite);
            strum_set_extra_animation_continuous(strum, STRUM_SCRIPT_TARGET_NOTE, animsprite);
            return;
        default:
            console.warn(
                "strum_set_extra_animation_continuous() unknown strum_script_target=" +
                strum_script_target
            );
            return;
    }

    // dispose previous animation
    if (attached_animations.continuous.action)
        animsprite_destroy(attached_animations.continuous.action);

    // obligatory, make a copy of animsprite
    attached_animations.continuous.action = animsprite ? animsprite_clone(animsprite) : null;
}


function strum_set_note_tweenkeyframe(strum, tweenkeyframe) {
    if (strum.tweenkeyframe_note) tweenkeyframe_destroy(strum.tweenkeyframe_note);
    strum.tweenkeyframe_note = tweenkeyframe ? tweenkeyframe_clone(tweenkeyframe) : null;
}

function strum_set_sickeffect_size_ratio(strum, size_ratio) {
    strum.sick_effect_ratio = size_ratio;
    strum_internal_calc_states(strum, strum.sprite_sick_effect, strum_internal_calc_state_sick_effect);
}


function strum_set_offsetcolor(strum, r, g, b, a) {
    drawable_set_offsetcolor(strum.drawable, r, g, b, a);
}

function strum_set_alpha(strum, alpha) {
    drawable_set_alpha(strum.drawable, alpha);
}

function strum_set_visible(strum, visible) {
    drawable_set_visible(strum.drawable, visible);
}

function strum_set_draw_offset(strum, offset_milliseconds) {
    strum.draw_offset_milliseconds = offset_milliseconds;
}

function strum_set_bpm(strum, beats_per_minute) {
    beatwatcher_reset(strum.beatwatcher, 1, beats_per_minute);
}

function strum_disable_beat_synced_idle_and_continous(strum, disabled) {
    strum.use_beat_synced_idle_and_continous = !disabled;
}

function strum_get_modifier(strum) {
    return drawable_get_modifier(strum.drawable);
}

function strum_get_drawable(strum) {
    return strum.drawable;
}

function strum_get_duration(strum) {
    let max_duration = 0.0;
    for (let i = 0; i < strum.chart_notes_size; i++) {
        let end_timestamp = strum.chart_notes[i].endTimestamp;
        if (end_timestamp > max_duration) max_duration = end_timestamp;
    }
    return max_duration;
}

function strum_animation_restart(strum) {
    strum_internal_extra_batch(strum.animation_strum_line, animsprite_restart);
    strum_internal_extra_batch(strum.animation_note, animsprite_restart);
    strum_internal_extra_batch(strum.animation_marker, animsprite_restart);
    strum_internal_extra_batch(strum.animation_sick_effect, animsprite_restart);
    strum_internal_extra_batch(strum.animation_background, animsprite_restart);

}

function strum_animation_end(strum) {
    strum_internal_extra_batch(strum.animation_strum_line, animsprite_force_end);
    strum_internal_extra_batch(strum.animation_note, animsprite_force_end);
    strum_internal_extra_batch(strum.animation_marker, animsprite_force_end);
    strum_internal_extra_batch(strum.animation_sick_effect, animsprite_force_end);
    strum_internal_extra_batch(strum.animation_background, animsprite_force_end);
}



function strum_internal_calc_scroll_window(strum) {
    strum.scroll_window = Math.abs(strum.dimmen_length / strum.scroll_velocity);
}

function strum_internal_on_note_bounds(strum, note_index, test_timestamp) {
    let strum_note = strum.chart_notes[note_index];

    let note_timestamp = strum_note.timestamp;
    let note_duration = strum_internal_get_note_duration(strum, note_index);

    let start = note_timestamp - strum.marker_duration;
    let end = note_timestamp + note_duration;

    return test_timestamp >= start && test_timestamp <= end;
}

function strum_internal_get_note_duration(strum, note_index) {
    let note_duration = strum.chart_notes[note_index].duration;

    //
    // Note:
    //      ¿is the sustain duration visible at the current chart speed and scroll velocity?
    //      if not, the sustain length in pixels is negligible (duration too short).
    //
    //      Sustain notes should have a minimal duration according to the
    //      scroll speed, also the sustain note duration must reach the threshold.
    //

    return note_duration < strum.minimum_sustain_duration ? strum.marker_duration : note_duration;
}


function strum_internal_calc_state_dimmen(state, scroll_is_vertical, dimmen, invdimmen) {
    const draw_size = [0, 0];
    let width, height;

    if (scroll_is_vertical) {
        width = invdimmen;
        height = dimmen;
    } else {
        width = dimmen;
        height = invdimmen;
    }

    imgutils_calc_size2(state.texture, state.frame_info, width, height, draw_size);

    state.draw_width = draw_size[0];
    state.draw_height = draw_size[1];
}

function strum_internal_state_add(statesprite, modelholder, strum_name, target, state_name) {
    if (statesprite_state_has(statesprite, state_name)) return null;
    if (!modelholder) return null;

    let animation_name = `${strum_name} ${target}`;

    if (state_name != null) animation_name = `${animation_name} ${state_name}`;

    let state = statesprite_state_add(statesprite, modelholder, animation_name, state_name);
    animation_name = undefined;

    // reset the draw location offsets beacuase is picked from last state applied
    if (state) {
        state.offset_x = 0;
        state.offset_y = 0;
    }

    return state;
}

function strum_internal_calc_states(strum, statesprite, calc_callback) {
    let list = statesprite_state_list(statesprite);
    for (let state of linkedlist_iterate4(list)) {
        // reset the state offsets before continue
        state.offset_x = state.offset_y = 0;
        calc_callback(strum, state);
    }
    statesprite_state_apply(statesprite, null);
}

function strum_internal_calc_state_marker(strum, state) {
    let original_size = [0, 0];
    let width, height;

    if (strum.scroll_is_vertical) {
        width = strum.dimmen_opposite;
        height = strum.dimmen_marker;
        if (strum.scroll_is_inverse) state.offset_y += strum.inverse_offset;
    } else {
        width = strum.dimmen_marker;
        height = strum.dimmen_opposite;
        if (strum.scroll_is_inverse) state.offset_x += strum.inverse_offset;
    }

    if (strum.markers_scale_keep) {
        imgutils_get_statesprite_original_size(state, original_size);
    }

    if (strum.markers_scale < 0.0 || !strum.markers_scale_keep) {
        strum_internal_calc_state_dimmen(
            state, strum.scroll_is_vertical, strum.dimmen_marker, strum.dimmen_opposite
        );
        strum.markers_scale = state.draw_width / original_size[0];
    } else {
        //
        // resize sprite_marker_confirm and sprite_marker_press with the same
        // scale as sprite_marker_nothing
        //
        state.draw_width = original_size[0] * strum.markers_scale;
        state.draw_height = original_size[1] * strum.markers_scale;
    }


    // center marker inside of the marker bounds
    state.offset_x += (width - state.draw_width) / 2;
    state.offset_y += (height - state.draw_height) / 2;
}

function strum_internal_calc_state_sick_effect(strum, state) {
    const dimmen_sick_effect = strum.dimmen_marker * strum.sick_effect_ratio;

    let width, height;
    let inverse = strum.dimmen_length - dimmen_sick_effect;

    if (strum.scroll_is_vertical) {
        width = strum.dimmen_opposite;
        height = strum.dimmen_marker;
        if (strum.scroll_is_inverse) state.offset_y += inverse + height;
    } else {
        width = dimmen_sick_effect;
        height = strum.dimmen_marker;
        if (strum.scroll_is_inverse) state.offset_x += inverse + width;
    }

    strum_internal_calc_state_dimmen(
        state, strum.scroll_is_vertical, dimmen_sick_effect, strum.dimmen_opposite * strum.sick_effect_ratio
    );

    // center the splash with the marker
    state.offset_x += (width - state.draw_width) / 2;
    state.offset_y += (height - state.draw_height) / 2;
}

function strum_internal_calc_state_background(strum, state) {
    const temp = [0, 0];
    let corner;
    let width = strum.dimmen_opposite;
    let height = strum.dimmen_length;

    if (strum.scroll_is_vertical) {
        corner = strum.scroll_is_inverse ? CORNER_BOTTOMRIGHT : CORNER_TOPLEFT;
    } else {
        corner = strum.scroll_is_inverse ? CORNER_BOTTOMLEFT : CORNER_TOPRIGHT;
    }

    sh4matrix_helper_calculate_corner_rotation(state.corner_rotation, corner);

    if (!strum.keep_aspect_ratio_background) {
        state.draw_width = width;
        state.draw_height = height;
        return;
    }

    imgutils_calc_size2(state.texture, state.frame_info, width, height, temp);
    state.draw_width = temp[0];
    state.draw_height = temp[1];

    let offset_x = (strum.dimmen_opposite - state.draw_width) / 2;
    let offset_y = strum.dimmen_length - state.draw_height;

    switch (corner) {
        case CORNER_TOPLEFT:// upscroll
            state.offset_x = offset_x;
            break;
        case CORNER_BOTTOMRIGHT:// downscroll
            state.offset_x = offset_x;
            state.offset_y = offset_y;
            break;
        case CORNER_TOPRIGHT:// leftscroll
            state.offset_y = offset_x;
            break;
        case CORNER_BOTTOMLEFT:// rightscroll
            state.offset_x = offset_y;
            break;
    }

}

function strum_internal_states_recalculate(strum) {
    strum.markers_scale = -1.0;
    strum_internal_calc_states(strum, strum.sprite_marker_nothing, strum_internal_calc_state_marker);
    strum_internal_calc_states(strum, strum.sprite_marker_confirm, strum_internal_calc_state_marker);
    strum_internal_calc_states(strum, strum.sprite_marker_press, strum_internal_calc_state_marker);
    strum_internal_calc_states(strum, strum.sprite_sick_effect, strum_internal_calc_state_sick_effect);
    strum_internal_calc_states(strum, strum.sprite_background, strum_internal_calc_state_background);
}



function strum_internal_extra_destroy_animation(holder) {
    if (holder.hit_up.action) animsprite_destroy(holder.hit_up.action);
    if (holder.hit_down.action) animsprite_destroy(holder.hit_down.action);
    if (holder.miss.action) animsprite_destroy(holder.miss.action);
    if (holder.penality.action) animsprite_destroy(holder.penality.action);
    if (holder.idle.action) animsprite_destroy(holder.idle.action);
    if (holder.continuous.action) animsprite_destroy(holder.continuous.action);

    if (holder.hit_up.undo) animsprite_destroy(holder.hit_up.undo);
    if (holder.hit_down.undo) animsprite_destroy(holder.hit_down.undo);
    if (holder.miss.undo) animsprite_destroy(holder.miss.undo);
    if (holder.penality.undo) animsprite_destroy(holder.penality.undo);
    if (holder.idle.undo) animsprite_destroy(holder.idle.undo);
    //if (holder.continuous.undo) animsprite_destroy(holder.continuous.undo);// never used
}

function strum_internal_extra_animate(strum, target, event, undo, elapsed) {
    let holder = strum_internal_extra_get_holder(strum, target);
    let subholder;

    if (undo) {
        if (event == STRUM_SCRIPT_ON_ALL) return 1;

        // execute "undo" animation of the previous event
        subholder = strum_internal_extra_get_subholder(holder, event);

        if (subholder.undo) {
            animsprite_restart(subholder.undo);
            animsprite_animate(subholder.undo, elapsed);
            strum_internal_extra_animate_sprite(strum, target, subholder.undo);
        }
        return 1;
    }


    //
    // painful check:
    //      - differentiate between idle and keyup events
    //      - also wait for miss/penality/keyup animation end on idle events
    //
    if (event == STRUM_SCRIPT_ON_IDLE) {
        switch (holder.state) {
            case STRUM_EXTRA_STATE_WAIT:
                event = holder.last_event;
                break;
            case STRUM_EXTRA_STATE_NONE:
                if (holder.last_event == STRUM_SCRIPT_ON_HIT_DOWN) {
                    holder.state = STRUM_EXTRA_STATE_WAIT;
                    event = STRUM_SCRIPT_ON_HIT_UP;
                }
                break;
        }
    } else {
        switch (event) {
            case STRUM_SCRIPT_ON_MISS:
            case STRUM_SCRIPT_ON_PENALITY:
                holder.state = STRUM_EXTRA_STATE_WAIT;
                break;
            default:
                holder.state = STRUM_EXTRA_STATE_NONE;
                break;
        }
    }

    let changed = event != holder.last_event;
    subholder = strum_internal_extra_get_subholder(holder, event);

    if (changed)
        strum_internal_extra_animate(strum, target, holder.last_event, 1, Infinity);

    holder.last_event = event;
    let completed = 1;

    if (subholder.action) {
        if (changed)
            animsprite_restart(subholder.action);

        completed = animsprite_animate(subholder.action, elapsed);
        strum_internal_extra_animate_sprite(strum, target, subholder.action);
    }

    if (completed) {
        switch (event) {
            case STRUM_SCRIPT_ON_HIT_UP:
            case STRUM_SCRIPT_ON_MISS:
            case STRUM_SCRIPT_ON_PENALITY:
                holder.state = STRUM_EXTRA_STATE_COMPLETED;
                break;
            default:
                holder.state = STRUM_EXTRA_STATE_NONE;
                break;
        }
    }

    return completed;
}

function strum_internal_extra_animate_sprite(strum, target, animsprite) {
    if (!animsprite) return;

    switch (target) {
        case STRUM_SCRIPT_TARGET_STRUM_LINE:
            animsprite_update_drawable(strum.drawable, animsprite, 1);
            break;
        case STRUM_SCRIPT_TARGET_NOTE:
            let last_index = strum.chart_notes_id_map_size - 1;
            for (let i = 0; i < strum.chart_notes_id_map_size; i++) {
                if (strum.drawable_notes[i]) {
                    let statesprite = note_peek_alone_statesprite(strum.drawable_notes[i]);
                    animsprite_update_statesprite(animsprite, statesprite, i == last_index);
                }
            }
            break;
        case STRUM_SCRIPT_TARGET_MARKER:
            animsprite_update_statesprite(animsprite, strum.sprite_marker_confirm, 0);
            animsprite_update_statesprite(animsprite, strum.sprite_marker_nothing, 0);
            animsprite_update_statesprite(animsprite, strum.sprite_marker_press, 1);
            break;
        case STRUM_SCRIPT_TARGET_SICK_EFFECT:
            animsprite_update_statesprite(animsprite, strum.sprite_sick_effect, 1);
            break;
        case STRUM_SCRIPT_TARGET_BACKGROUND:
            animsprite_update_statesprite(animsprite, strum.sprite_background, 1);
            break;
    }
}

function strum_internal_extra_get_holder(strum, target) {
    switch (target) {
        case STRUM_SCRIPT_TARGET_STRUM_LINE:
            return strum.animation_strum_line;
        case STRUM_SCRIPT_TARGET_NOTE:
            return strum.animation_note;
        case STRUM_SCRIPT_TARGET_MARKER:
            return strum.animation_marker;
        case STRUM_SCRIPT_TARGET_SICK_EFFECT:
            return strum.animation_sick_effect;
        case STRUM_SCRIPT_TARGET_BACKGROUND:
            return strum.animation_background;
        default:
            return null;
    }
}

function strum_internal_extra_get_subholder(holder, event) {
    switch (event) {
        case STRUM_SCRIPT_ON_HIT_UP:
            return holder.hit_up;
        case STRUM_SCRIPT_ON_HIT_DOWN:
            return holder.hit_down;
        case STRUM_SCRIPT_ON_MISS:
            return holder.miss;
        case STRUM_SCRIPT_ON_PENALITY:
            return holder.penality;
        case STRUM_SCRIPT_ON_IDLE:
            return holder.idle;
        default:
            return null;
    }
}


function strum_internal_check_sustain_queue(strum, song_timestamp, playerstats) {
    let size = arraylist_size(strum.sustain_queue);
    const array = arraylist_peek_array(strum.sustain_queue);

    for (let i = 0; i < size; i++) {
        /** @type {StrumNote} */
        let note = array[i];
        let end_timestamp = note.endTimestamp;
        let note_attributes = strum.attribute_notes[note.id];
        let is_released = note.state == NoteState.RELEASE;// ¿was early released?
        let quarter = Math.trunc(
            (note.duration - (end_timestamp - song_timestamp)) / strum.marker_duration_quarter
        );

        if (quarter <= 0 || quarter <= note.previous_quarter) continue;

        if (song_timestamp > end_timestamp) {
            // the sustain note is still in hold (the player keeps holding the button)
            arraylist_remove(strum.sustain_queue, i);
            i--;
            size--;
            continue;
        }

        if (is_released && !note_attributes.ignore_miss || !is_released && !note_attributes.ignore_hit) {
            let quarters = quarter - note.previous_quarter;
            playerstats_add_sustain(playerstats, quarters, is_released);

            // Mark press state as missed after the 4 first quarters 
            if (is_released && quarter > 4) {
                let total_quarters = (note.duration / strum.marker_duration_quarter) - 2;
                // also ignore last 2 quarters
                if (quarter < total_quarters) strum_internal_update_press_state(strum, STRUM_PRESS_STATE_MISS);
            }

            //console.log(`strum: [sustain] ts=${note.timestamp} release=${is_released} quarters=${quarters}`);
        }

        note.previous_quarter = quarter;
    }
}

function strum_internal_load_sick_effect_state(strum, modelholder, state_name) {
    let state = strum_internal_state_add(
        strum.sprite_sick_effect, modelholder, strum.strum_name,
        STRUM_SICK_EFFECT_SUFFIX, state_name
    );
    if (state) strum_internal_calc_state_sick_effect(strum, state);
}

function strum_internal_set_note_drawables(strum, notepool) {
    // free() previous arrays
    if (strum.drawable_notes) strum.drawable_notes = undefined;
    if (strum.attribute_notes) strum.attribute_notes = undefined;

    strum.drawable_notes = new Array(strum.chart_notes_id_map_size);
    strum.attribute_notes = new Array(strum.chart_notes_id_map_size);

    for (let i = 0; i < strum.chart_notes_id_map_size; i++) {
        let id = strum.chart_notes_id_map[i];

        if (id < 0 || id >= notepool.size) {
            console.error("strum_internal_set_note_drawables() invalid note id found in the chart: " + id);
            strum.drawable_notes[i] = null;
            strum.attribute_notes[i] = null;
            continue;
        }

        strum.drawable_notes[i] = notepool.drawables[id];
        strum.attribute_notes[i] = notepool.attributes[id];

        if (notepool.models_custom_sick_effect[id] && notepool.attributes[id].custom_sick_effect_name) {
            strum_internal_load_sick_effect_state(
                strum,
                notepool.models_custom_sick_effect[id],
                notepool.attributes[id].custom_sick_effect_name
            );
        }
    }
}

function strum_internal_extra_batch(holder, callback) {
    if (holder.hit_up.action) callback(holder.hit_up.action);
    if (holder.hit_down.action) callback(holder.hit_down.action);
    if (holder.miss.action) callback(holder.miss.action);
    if (holder.penality.action) callback(holder.penality.action);
    if (holder.idle.action) callback(holder.idle.action);
}

function strum_internal_update_press_state(strum, press_state) {
    if (press_state != -1) {
        //if (press_state == strum.press_state) return;
        strum.press_state = press_state;
        strum.press_state_changes++;
        strum.press_state_use_alt_anim = 0;
    }
}

function strum_internal_reset_scrolling(strum) {
    strum.notes_peek_index = 0;
    strum.notes_render_index = 0;
    strum.last_song_timestamp = 0;
    strum.marker_state = STRUM_MARKER_STATE_NOTHING;
    strum.marker_state_changed = 1;
    strum.marker_sick_state = STRUM_MARKER_STATE_NOTHING;
    strum.marker_sick_state_name = null;
    strum.extra_animations_have_penalties = 0;
    strum.extra_animations_have_misses = 0;
    strum.auto_scroll_elapsed = -1;

    strum.press_state_changes = -1;
    strum.press_state = STRUM_PRESS_STATE_NONE;

    strum.animation_background.last_event = STRUM_SCRIPT_ON_ALL;
    strum.animation_marker.last_event = STRUM_SCRIPT_ON_ALL;
    strum.animation_sick_effect.last_event = STRUM_SCRIPT_ON_ALL;
    strum.animation_strum_line.last_event = STRUM_SCRIPT_ON_ALL;
    strum.animation_note.last_event = STRUM_SCRIPT_ON_ALL;

    strum.animation_background.state = STRUM_EXTRA_STATE_NONE;
    strum.animation_marker.state = STRUM_EXTRA_STATE_NONE;
    strum.animation_sick_effect.state = STRUM_EXTRA_STATE_NONE;
    strum.animation_strum_line.state = STRUM_EXTRA_STATE_NONE;
    strum.animation_note.state = STRUM_EXTRA_STATE_NONE;
}

function strum_internal_calc_marker_duration(strum, velocity) {
    if (strum.use_fukin_marker_duration)
        strum.marker_duration = FUNKIN_MARKER_DURATION;
    else
        strum.marker_duration = Math.abs(strum.dimmen_marker / velocity) * STRUM_ADDITIONAL_MARKER_DURATION_RATIO;

    strum.marker_duration *= strum.marker_duration_multiplier;

    strum.minimum_sustain_duration = strum.marker_duration * FNF_NOTE_MIN_SUSTAIN_THRESHOLD;
    strum.marker_duration_quarter = strum.marker_duration / 4.0;

    if (strum.chart_notes_size > 0)
        strum.key_test_limit = Math.max(strum.chart_notes[0].timestamp - strum.marker_duration, 0);
    else
        strum.key_test_limit = -Infinity;
}

function strum_internal_restart_extra_continous(strum) {
    if (strum.animation_strum_line.continuous.action)
        animsprite_restart(strum.animation_strum_line.continuous.action);
    if (strum.animation_note.continuous.action)
        animsprite_restart(strum.animation_note.continuous.action);
    if (strum.animation_marker.continuous.action)
        animsprite_restart(strum.animation_marker.continuous.action);
    if (strum.animation_sick_effect.continuous.action)
        animsprite_restart(strum.animation_sick_effect.continuous.action);
    if (strum.animation_background.continuous.action)
        animsprite_restart(strum.animation_background.continuous.action);
}

