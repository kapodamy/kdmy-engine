"use strict";

const CHART_EVENT_NONE = 1;
const CHART_EVENT_CAMERA_OPPONENT = 2;
const CHART_EVENT_CAMERA_PLAYER = 3;
const CHART_EVENT_CHANGE_BPM = 4;
const CHART_EVENT_ALT_ANIM_OPPONENT = 5;
const CHART_EVENT_ALT_ANIM_PLAYER = 6;
const CHART_EVENT_UNKNOWN_NOTE = 7;

class Chart {
    /**
     * @typedef {object} ChartNote
     * @property {number} timestamp
     * @property {number} direction
     * @property {number} duration
     * @property {boolean} alt_anim
     * @property {number} data
     */
    /**
     * @typedef {object} ChartEntry
     * @property {ChartNote[]} notes
     * @property {number} notes_size
     */
    /**
     * @typedef {object} ChartEvent
     * @property {boolean} is_player_or_opponent
     * @property {number} timestamp
     * @property {number} command
     * @property {number} parameter
     * @property {number} parameter2
     * @property {number} parameter3
     */

    /** @type {ChartEntry[]} */
    entries = null;
    entries_size = 0;

    /** @type {ChartEvent[]} */
    events = null;
    events_size = 0;

    speed = 1.0;
    bpm = 100;
}


async function chart_init(src, difficult) {
    let is_normal = difficult == FUNKIN_DIFFICULT_NORMAL;
    let index = src.lastIndexOf(".json");
    if (index < 0) index = src.length;

    let temp = string_concat(2, "-", difficult);
    difficult = temp.toLowerCase();// funkin uses lowercase in chart filenames
    temp = undefined;
    let chart_filename = string_copy_and_insert(src, index, difficult);
    difficult = undefined;

    let json;
    if (is_normal && await fs_file_exists(src)) {
        json = await json_load_from(src);
    } else {
        json = await json_load_from(chart_filename);
    }

    if (!json) throw new Error("Missing chart file: " + chart_filename);
    chart_filename = undefined;

    if (!json_has_property_object(json, "song")) {
        throw new Error("Unknown chart file, expecting 'funkin' format");
    }
    let song = json_read_object(json, "song");

    let chart = new Chart();
    chart.bpm = json_read_number(song, "bpm", 100.0);
    chart.speed = json_read_number(song, "speed", 1.0);

    let json_notes = json_read_array(song, "notes");
    let json_notes_length = json_read_array_length(json_notes);

    // Important: charts with more than 4 directions (arrows) must include "directionCount"
    let direction_count = json_read_number(song, "directionCount", 4);
    let unknown_notes = direction_count * 2;

    let player_notes = arraylist_init();
    let opponent_notes = arraylist_init();
    let chart_events = arraylist_init();

    let last_must_hit_section = 2;
    let last_alt_anim_player = false;
    let last_alt_anim_opponent = false;
    let beat_duration = math2d_beats_per_minute_to_beat_per_milliseconds(chart.bpm);
    let quarter_duration = beat_duration / 4.0;
    let timestamp_event_accumulated = 0.0;

    for (let i = 0; i < json_notes_length; i++) {
        let json_section = json_read_array_item_object(json_notes, i);

        // typeOfSection appears to be unused
        /*if (json_has_property(json, "typeOfSection")) {
            if (json_has_property_number_long(json, "typeOfSection")) {
                let type_of_section = json_read_number_long(json_section, "typeOfSection", 0);
                if (type_of_section != 0) {
                    console.warn("Unknown typeOfSection", type_of_section);
                    continue;
                }
            } else {
                continue;
            }
        }*/

        let length_in_steps = json_read_number(json_section, "lengthInSteps", 16.0);
        let must_hit_section = json_read_boolean(json_section, "mustHitSection", false);
        let alt_anim = json_read_boolean(json_section, "altAnim", false);
        let change_bpm = json_read_boolean(json_section, "changeBPM", false);
        let bpm = json_read_number(json_section, "bpm", 100.0);

        if (change_bpm) {
            beat_duration = math2d_beats_per_minute_to_beat_per_milliseconds(bpm);
            quarter_duration = beat_duration / 4.0;
        }

        let timestamp_event = timestamp_event_accumulated;
        timestamp_event_accumulated += quarter_duration * length_in_steps;

        let json_section_notes = json_read_array(json_section, "sectionNotes");
        let json_section_notes_length = json_read_array_length(json_section_notes);

        for (let j = 0; j < json_section_notes_length; j++) {
            let set_camera = last_must_hit_section != must_hit_section;

            // this should have a size of 3 or 4
            let self_notes = json_read_array_item_array(json_section_notes, j);
            let self_notes_length = json_read_array_length(self_notes);

            if (self_notes_length < 3) {
                console.warn(`chart_init() section=${i} note=${j} has less than 3 fields`);
            }

            let timestamp = json_read_array_item_number(self_notes, 0, 0.0);
            let direction = json_read_array_item_number(self_notes, 1, 0.0);
            let duration = json_read_array_item_number(self_notes, 2, 0.0);
            let data;
            let alt_note;

            // In Funkin v0.2.8 the value at index 3 can denote an alt animation
            if (json_get_array_item_type(self_notes, 3) == JSON_VALUE_TYPE_BOOLEAN) {
                alt_note = json_read_array_item_boolean(self_notes, 3, false);
                data = alt_note ? Infinity : -Infinity;
            } else {
                alt_note = 0.0;
                data = json_read_array_item_number(self_notes, 3, 0.0);
            }

            if (set_camera) {
                let target_camera;
                if (must_hit_section)
                    target_camera = CHART_EVENT_CAMERA_PLAYER;
                else
                    target_camera = CHART_EVENT_CAMERA_OPPONENT;

                set_camera = false;
                last_must_hit_section = must_hit_section;

                arraylist_add(chart_events, {
                    timestamp: timestamp_event,
                    command: target_camera,
                    parameter: 0.0,
                    parameter2: 0.0
                });
            }

            if (alt_anim != (must_hit_section ? last_alt_anim_player : last_alt_anim_opponent)) {
                if (must_hit_section)
                    last_alt_anim_player = alt_anim;
                else
                    last_alt_anim_opponent = alt_anim;

                let cam = must_hit_section ? CHART_EVENT_ALT_ANIM_PLAYER : CHART_EVENT_ALT_ANIM_OPPONENT;
                arraylist_add(chart_events, {
                    timestamp: timestamp_event,
                    command: cam,
                    parameter: alt_anim,
                    parameter2: 0.0
                });
            }

            if (json_has_property_boolean(json_section, "changeBPM") && change_bpm) {
                arraylist_add(chart_events, {
                    timestamp: timestamp_event,
                    command: CHART_EVENT_CHANGE_BPM,
                    parameter: bpm,
                    parameter2: 0.0
                });
            }

            if (direction >= unknown_notes) {
                // custom event data
                console.info(
                    "chart_init() custom direction detected will be used as event. " +
                    `timestamp=${timestamp} direction:${direction} duration=${duration} data=${data}`
                );

                arraylist_add(chart_events, {
                    is_player_or_opponent: must_hit_section,
                    timestamp: timestamp,
                    command: CHART_EVENT_UNKNOWN_NOTE,
                    parameter: direction,
                    parameter2: duration,
                    parameter3: data
                });
                continue;
            }

            let target;
            if (direction >= direction_count) {
                // chour notes
                target = must_hit_section ? opponent_notes : player_notes;
                direction -= direction_count;
            } else {
                // self notes
                target = must_hit_section ? player_notes : opponent_notes;
            }
            arraylist_add(target, { timestamp, direction, duration, alt_anim, data });
        }
    }

    chart.entries_size = 0;
    if (arraylist_size(opponent_notes) > 0) chart.entries_size++;
    if (arraylist_size(player_notes) > 0) chart.entries_size++;

    let entry_index = 0;
    chart.entries = malloc_for_array(chart.entries_size);
    if (arraylist_size(opponent_notes) > 0) {
        chart.entries[entry_index] = { notes: null, notes_size: 0 };
        arraylist_destroy2(opponent_notes, chart.entries[entry_index], "notes_size", "notes");
        entry_index++;
    }
    if (arraylist_size(player_notes) > 0) {
        chart.entries[entry_index] = { notes: null, notes_size: 0 };
        arraylist_destroy2(player_notes, chart.entries[entry_index], "notes_size", "notes");
    }

    arraylist_destroy2(chart_events, chart, "events_size", "events");

    return chart;
}

function chart_destroy(chart) {
    for (let i = 0; i < chart.entries_size; i++) {
        chart.entries[i].notes = undefined;
    }

    chart.entries = undefined;
    chart.events = undefined;
    chart = undefined;
}

