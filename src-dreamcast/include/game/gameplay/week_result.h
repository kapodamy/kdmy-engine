#ifndef _week_result_h
#define _week_result_h

#include "week_types.h"

typedef struct WeekResult_s* WeekResult;

typedef struct WeekResult_Stats_s {
    int32_t sick;
    int32_t good;
    int32_t bads;
    int32_t shits;
    int32_t miss;
    int32_t penalties;
    int64_t score;
    float64 accuracy;
    int32_t notesperseconds;
    int32_t combobreaks;
    int32_t higheststreak;
} WeekResult_Stats;


WeekResult week_result_init();
void week_result_destroy(WeekResult* weekresult);
void week_result_add_stats(WeekResult weekresult, RoundContext roundcontext);
void week_result_helper_show_summary(WeekResult weekresult, RoundContext roundcontext, int32_t attempts, int32_t songs_count, bool freeplay, bool reject);

Layout week_result_get_layout(WeekResult weekresult);
void week_result_get_accumulated_stats(WeekResult weekresult, WeekResult_Stats* stats);

#endif
