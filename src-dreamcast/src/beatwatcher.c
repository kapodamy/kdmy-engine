#include <arch/timer.h>

#include "beatwatcher.h"
#include "math2d.h"


int64_t beatwatcher_global_timestamp = 0;

void beatwatcher_global_set_timestamp(int64_t timestamp) {
    beatwatcher_global_timestamp = timestamp;
}

void beatwatcher_global_set_timestamp_from_kos_timer() {
    beatwatcher_global_timestamp = (int64_t)timer_ms_gettime64();
}


void beatwatcher_reset(BeatWatcher* beatwatcher, bool count_beats_or_quarters, float beats_per_minute) {
    beatwatcher->count_beats_or_quarters = count_beats_or_quarters;
    beatwatcher->tick = math2d_beats_per_minute_to_beat_per_milliseconds(beats_per_minute);
    if (!count_beats_or_quarters) beatwatcher->tick /= 8.0f;
    beatwatcher->count = 0;
    beatwatcher->drift_count = 0;
    beatwatcher->drift_timestamp = 0;
    beatwatcher->since = 0.0f;
    beatwatcher->resyncs = 0;
    beatwatcher->last_global_timestamp = beatwatcher_global_timestamp;
}

void beatwatcher_change_bpm(BeatWatcher* beatwatcher, float beats_per_minute) {
    beatwatcher->tick = math2d_beats_per_minute_to_beat_per_milliseconds(beats_per_minute);
    beatwatcher->drift_timestamp = beatwatcher_global_timestamp;
    beatwatcher->drift_count += beatwatcher->count;

    if (!beatwatcher->count_beats_or_quarters) beatwatcher->tick /= 8.0f;
}

float beatwatcher_remaining_until_next(BeatWatcher* beatwatcher) {
    return ((beatwatcher->count + 1) * beatwatcher->tick) - beatwatcher_global_timestamp;
}

bool beatwatcher_poll(BeatWatcher* beatwatcher) {
    // all beatwatcher instances are in sync with the global beatwatcher
    if (beatwatcher->last_global_timestamp > beatwatcher_global_timestamp) {
        beatwatcher->resyncs++;
        beatwatcher->drift_timestamp -= beatwatcher->last_global_timestamp - (int64_t)beatwatcher_global_timestamp;
        beatwatcher->last_global_timestamp = beatwatcher_global_timestamp;
        beatwatcher->since = 0.0f;
        return false;
    }

    int32_t old_count = beatwatcher->count;
    int64_t timestamp = beatwatcher_global_timestamp - beatwatcher->drift_timestamp;

    beatwatcher->since = 0.0f;
    beatwatcher->count = (int32_t)(timestamp / beatwatcher->tick);
    beatwatcher->count += beatwatcher->drift_count;
    beatwatcher->last_global_timestamp = beatwatcher_global_timestamp;

    if (beatwatcher->count > old_count) {
        beatwatcher->since = timestamp - (beatwatcher->tick * old_count) - beatwatcher->tick;
        if (beatwatcher->since < 0) beatwatcher->since = 0.0f;
        return true;
    }

    return false;
}
