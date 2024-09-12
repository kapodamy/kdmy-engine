#include "game/gameplay/helpers/chart.h"

#include "arraylist.h"
#include "fs.h"
#include "game/common/funkin.h"
#include "jsonparser.h"
#include "malloc_utils.h"
#include "math2d.h"
#include "nbool.h"
#include "stringutils.h"


Chart chart_init(const char* src, const char* difficult) {
    bool is_normal = string_equals(difficult, FUNKIN_DIFFICULT_NORMAL);
    int32_t index = string_last_index_of_string(src, 0, ".json");
    if (index < 0) index = (int32_t)strlen(src);

    char* temp = string_concat(2, "-", difficult);
    difficult = string_to_lowercase(temp); // funkin uses lowercase in chart filenames
    free_chk(temp);
    char* chart_filename = string_copy_and_insert(src, index, difficult);
    free_chk((char*)difficult);

    JSONToken json;
    if (is_normal && fs_file_exists(src)) {
        json = json_load_from(src);
    } else {
        json = json_load_from(chart_filename);
    }

    if (!json) {
        logger_error("Missing chart file: %s", chart_filename);
        assert(json);
    }
    free_chk(chart_filename);

    if (!json_has_property_object(json, "song")) {
        logger_error("Unknown chart file, expecting 'funkin' format");
        assert(json_has_property_object(json, "song"));
    }
    JSONToken song = json_read_object(json, "song");

    Chart chart = calloc_for_type(struct Chart_s);
    malloc_assert(chart, Chart);

    chart->bpm = (float)json_read_number_double(song, "bpm", 100.0);
    chart->speed = (float)json_read_number_double(song, "speed", 1.0);

    JSONToken json_notes = json_read_array(song, "notes");
    int32_t json_notes_length = json_read_array_length(json_notes);

    // Important: charts with more than 4 directions (arrows) must include "directionCount"
    int32_t direction_count = (int32_t)json_read_number_long(song, "directionCount", 4);
    int32_t unknown_notes = direction_count * 2;

    ArrayList player_notes = arraylist_init(sizeof(ChartNote));
    ArrayList opponent_notes = arraylist_init(sizeof(ChartNote));
    ArrayList chart_events = arraylist_init(sizeof(ChartEventEntry));

    nbool last_must_hit_section = unset;
    bool last_alt_anim_player = false;
    bool last_alt_anim_opponent = false;
    float beat_duration = math2d_beats_per_minute_to_beat_per_milliseconds(chart->bpm);
    float quarter_duration = beat_duration / 4.0f;
    float64 timestamp_event_accumulated = 0.0;

    for (int32_t i = 0; i < json_notes_length; i++) {
        JSONToken json_section = json_read_array_item_object(json_notes, i);

        // typeOfSection appears to be unused
        /*if (json_has_property(json, "typeOfSection")) {
            if (json_has_property_number_long(json, "typeOfSection")) {
                int64_t type_of_section = json_read_number_long(json_section, "typeOfSection", 0);
                if (type_of_section != 0) {
                    logger_error("Unknown typeOfSection=" FMT_I8, type_of_section);
                    continue;
                }
            } else {
                continue;
            }
        }*/

        int64_t length_in_steps = json_read_number_long(json_section, "lengthInSteps", 16);
        bool must_hit_section = json_read_boolean(json_section, "mustHitSection", false);
        bool alt_anim = json_read_boolean(json_section, "altAnim", false);
        bool change_bpm = json_read_boolean(json_section, "changeBPM", false);
        float bpm = (float)json_read_number_double(json_section, "bpm", 100.0);

        if (change_bpm) {
            beat_duration = math2d_beats_per_minute_to_beat_per_milliseconds(bpm);
            quarter_duration = beat_duration / 4.0f;
        }

        float64 timestamp_event = timestamp_event_accumulated;
        timestamp_event_accumulated += quarter_duration * length_in_steps;

        JSONToken json_section_notes = json_read_array(json_section, "sectionNotes");
        int32_t json_section_notes_length = json_read_array_length(json_section_notes);

        for (int32_t j = 0; j < json_section_notes_length; j++) {
            bool set_camera = last_must_hit_section != must_hit_section;

            // this should have a size of 3 or 4
            JSONToken self_notes = json_read_array_item_array(json_section_notes, j);
            int32_t self_notes_length = json_read_array_length(self_notes);

            if (self_notes_length < 3) {
                logger_warn("chart_init() section=" FMT_I4 " note=" FMT_I4 " has less than 3 fields", i, j);
            }

            float64 timestamp = json_read_array_item_number_double(self_notes, 0, 0.0);
            float64 direction = json_read_array_item_number_double(self_notes, 1, 0.0);
            float64 duration = json_read_array_item_number_double(self_notes, 2, 0.0);
            float64 data;
            bool alt_note;

            // In Funkin v0.2.8 the value at index 3 can denote an alt animation
            if (json_get_array_item_type(self_notes, 3) == JSONTokenType_Boolean) {
                alt_note = json_read_array_item_boolean(self_notes, 3, false);
                data = alt_note ? DOUBLE_Inf : -DOUBLE_Inf;
            } else {
                alt_note = 0.0;
                data = json_read_array_item_number_double(self_notes, 3, DOUBLE_NaN);
            }

            if (set_camera) {
                ChartEvent target_camera;
                if (must_hit_section)
                    target_camera = ChartEvent_CAMERA_PLAYER;
                else
                    target_camera = ChartEvent_CAMERA_OPPONENT;

                set_camera = false;
                last_must_hit_section = must_hit_section;

                arraylist_add(
                    chart_events,
                    &(ChartEventEntry){
                        .timestamp = timestamp_event,
                        .command = target_camera,
                        .parameter = 0.0,
                        .parameter2 = 0.0,
                    }
                );
            }

            if (alt_anim != (must_hit_section ? last_alt_anim_player : last_alt_anim_opponent)) {
                if (must_hit_section)
                    last_alt_anim_player = alt_anim;
                else
                    last_alt_anim_opponent = alt_anim;

                ChartEvent cam = must_hit_section ? ChartEvent_ALT_ANIM_PLAYER : ChartEvent_ALT_ANIM_OPPONENT;
                arraylist_add(
                    chart_events,
                    &(ChartEventEntry){
                        .timestamp = timestamp_event,
                        .command = cam,
                        .parameter = alt_anim,
                        .parameter2 = 0.0,
                    }
                );
            }

            if (json_has_property_boolean(json_section, "changeBPM") && change_bpm) {
                arraylist_add(
                    chart_events,
                    &(ChartEventEntry){
                        .timestamp = timestamp_event,
                        .command = ChartEvent_CHANGE_BPM,
                        .parameter = bpm,
                        .parameter2 = 0.0,
                    }
                );
            }

            if (direction >= unknown_notes) {
                // custom event data
                logger_info(
                    "chart_init() custom direction detected will be used as event. "
                    "timestamp" FMT_FLT64 " direction" FMT_FLT64 " duration" FMT_FLT64 " data=" FMT_FLT64,
                    timestamp, direction, duration, data
                );

                arraylist_add(
                    chart_events,
                    &(ChartEventEntry){
                        .is_player_or_opponent = must_hit_section,
                        .timestamp = timestamp,
                        .command = ChartEvent_UNKNOWN_NOTE,
                        .parameter = direction,
                        .parameter2 = duration,
                        .parameter3 = data,
                    }
                );
                continue;
            }

            ArrayList target;
            if (direction >= direction_count) {
                // chour notes
                target = must_hit_section ? opponent_notes : player_notes;
                direction -= direction_count;
            } else {
                // self notes
                target = must_hit_section ? player_notes : opponent_notes;
            }

            arraylist_add(
                target,
                &(ChartNote){
                    .timestamp = timestamp,
                    .direction = direction,
                    .duration = duration,
                    .alt_anim = alt_note,
                    .data = data,
                }
            );
        }
    }

    chart->entries_size = 0;
    if (arraylist_size(opponent_notes) > 0) chart->entries_size++;
    if (arraylist_size(player_notes) > 0) chart->entries_size++;

    int32_t entry_index = 0;
    chart->entries = malloc_for_array(ChartEntry, chart->entries_size);

    if (arraylist_size(opponent_notes) > 0) {
        chart->entries[entry_index] = (ChartEntry){.notes = NULL, .notes_size = 0};
        arraylist_destroy2(&opponent_notes, &chart->entries[entry_index].notes_size, (void**)&chart->entries[entry_index].notes);
        entry_index++;
    }
    if (arraylist_size(player_notes) > 0) {
        chart->entries[entry_index] = (ChartEntry){.notes = NULL, .notes_size = 0};
        arraylist_destroy2(&player_notes, &chart->entries[entry_index].notes_size, (void**)&chart->entries[entry_index].notes);
    }

    arraylist_destroy2(&chart_events, &chart->events_size, (void**)&chart->events);

    json_destroy(&json);
    return chart;
}

void chart_destroy(Chart* chart_ptr) {
    if (!chart_ptr || !*chart_ptr) return;

    Chart chart = *chart_ptr;

    for (int32_t i = 0; i < chart->entries_size; i++) {
        free_chk(chart->entries[i].notes);
    }

    free_chk(chart->entries);
    free_chk(chart->events);

    free_chk(chart);
    *chart_ptr = NULL;
}
