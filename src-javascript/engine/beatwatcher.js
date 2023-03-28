"use strict";

var beatwatcher_global_timestamp = 0;

function beatwatcher_global_set_timestamp(timestamp) {
    beatwatcher_global_timestamp = timestamp;
}

function beatwatcher_global_set_timestamp_from_kos_timer() {
    beatwatcher_global_timestamp = timer_ms_gettime64();
}


function beatwatcher_reset(beatwatcher, count_beats_or_quarters, beats_per_minute) {
    beatwatcher.count_beats_or_quarters = count_beats_or_quarters;
    beatwatcher.tick = math2d_beats_per_minute_to_beat_per_milliseconds(beats_per_minute);
    if (!count_beats_or_quarters) beatwatcher.tick /= 8.0;
    beatwatcher.count = 0;
    beatwatcher.drift_count = 0;
    beatwatcher.drift_timestamp = 0;
    beatwatcher.since = 0;
    beatwatcher.resyncs = 0;
    beatwatcher.last_global_timestamp = beatwatcher_global_timestamp;
}

function beatwatcher_change_bpm(beatwatcher, beats_per_minute) {
    beatwatcher.tick = math2d_beats_per_minute_to_beat_per_milliseconds(beats_per_minute);
    beatwatcher.drift_timestamp = beatwatcher_global_timestamp;
    beatwatcher.drift_count += beatwatcher.count;

    if (!beatwatcher.count_beats_or_quarters) beatwatcher.tick /= 8.0;
}

function beatwatcher_remaining_until_next(beatwatcher) {
    return ((beatwatcher.count + 1) * beatwatcher.tick) - beatwatcher_global_timestamp
}

function beatwatcher_poll(beatwatcher) {
    // all beatwatcher instances are in sync with the global beatwatcher
    if (beatwatcher.last_global_timestamp > beatwatcher_global_timestamp) {
        beatwatcher.resyncs++;
        beatwatcher.drift_timestamp -= beatwatcher.last_global_timestamp - beatwatcher_global_timestamp;
        beatwatcher.last_global_timestamp = beatwatcher_global_timestamp;
        beatwatcher.since = 0;
        return 0;
    }

    let old_count = beatwatcher.count;
    let timestamp = beatwatcher_global_timestamp - beatwatcher.drift_timestamp;

    beatwatcher.since = 0;
    beatwatcher.count = Math.trunc(timestamp / beatwatcher.tick);
    beatwatcher.count += beatwatcher.drift_count;
    beatwatcher.last_global_timestamp = beatwatcher_global_timestamp;

    if (beatwatcher.count > old_count) {
        beatwatcher.since = timestamp - (beatwatcher.tick * old_count) - beatwatcher.tick;
        if (beatwatcher.since < 0) beatwatcher.since = 0;
        return 1;
    }

    return 0;
}
