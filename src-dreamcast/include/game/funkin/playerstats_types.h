#ifndef _playerstats_types_h
#define _playerstats_types_h

#include <stdbool.h>
#include <stdint.h>

#include "float64.h"


typedef enum {
    Ranking_NONE = 0,
    Ranking_SICK,
    Ranking_GOOD,
    Ranking_BAD,
    Ranking_SHIT,
    Ranking_MISS,
    Ranking_PENALITY
} Ranking;

typedef struct PlayerStats_s {
    int32_t ranking_sick;
    int32_t ranking_good;
    int32_t ranking_bad;
    int32_t ranking_shit;
    int64_t score;
    float64 accuracy;
    float64 health;
    float64 health_max;
    bool can_recover;
    int32_t iterations;
    int32_t hits;
    int32_t miss;
    int32_t penalties;
    int32_t streak;
    int32_t breaks;
    int32_t highest_streak;
    int32_t nps_current;
    int32_t nps_highest;
    Ranking last_ranking;
    float64 last_diff;
    float64 accuracy_accumulator;
    float64 accuracy_counter;
    float64 accuracy_counter_last;
    bool penalize_on_empty_strum;
    int32_t deads_by_fault;
    float64 last_accuracy;
}* PlayerStats;

#endif
