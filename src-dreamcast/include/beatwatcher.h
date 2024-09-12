#ifndef _beatwatcher_h
#define _beatwatcher_h

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool count_beats_or_quarters;
    float tick;
    int32_t count;
    int32_t drift_count;
    int64_t drift_timestamp;
    float since;
    int32_t resyncs;
    int64_t last_global_timestamp;
} BeatWatcher;


extern int64_t beatwatcher_global_timestamp;
void beatwatcher_global_set_timestamp(int64_t timestamp);
void beatwatcher_global_set_timestamp_from_kos_timer();

void beatwatcher_reset(BeatWatcher* beatwatcher, bool count_beats_or_quarters, float beats_per_minute);
void beatwatcher_change_bpm(BeatWatcher* beatwatcher, float beats_per_minute);
float beatwatcher_remaining_until_next(BeatWatcher* beatwatcher);
bool beatwatcher_poll(BeatWatcher* beatwatcher);

#endif
