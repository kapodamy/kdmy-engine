#ifndef __roundstats_h
#define __roundstats_h

#include "drawable.h"
#include "tweenkeyframe.h"

typedef struct {
    int dummy;
} RoundStats_t;
typedef RoundStats_t* RoundStats;


void roundstats_hide(RoundStats roundstats, bool hide);
void roundstats_hide_nps(RoundStats roundstats, bool hide);
void roundstats_set_draw_y(RoundStats roundstats, float y);
void roundstats_reset(RoundStats roundstats);
Drawable roundstats_get_drawable(RoundStats roundstats);
void roundstats_tweenkeyframe_set_on_beat(RoundStats roundstats, TweenKeyframe tweenkeyframe, float rollback_beats, float beat_duration);
void roundstats_tweenkeyframe_set_on_hit(RoundStats roundstats, TweenKeyframe tweenkeyframe, float rollback_beats, float beat_duration);
void roundstats_tweenkeyframe_set_on_miss(RoundStats roundstats, TweenKeyframe tweenkeyframe, float rollback_beats, float beat_duration);
void roundstats_tweenkeyframe_set_bpm(RoundStats roundstats, float beats_per_minute);

#endif
