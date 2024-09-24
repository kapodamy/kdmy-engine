#ifndef _roundstats_h
#define _roundstats_h

#include <stdbool.h>
#include <stdint.h>

#include "drawable_types.h"
#include "fontholder.h"
#include "playerstats_types.h"
#include "pvrcontext_types.h"


//
//  forward reference
//

typedef struct TweenKeyframe_s* TweenKeyframe;


typedef struct RoundStats_s* RoundStats;


RoundStats roundstats_init(float x, float y, float z, FontHolder fontholder, float font_size, float font_border_size, float layout_width);
void roundstats_destroy(RoundStats* roundstats);
void roundstats_hide(RoundStats roundstats, bool hide);
void roundstats_hide_nps(RoundStats roundstats, bool hide);
void roundstats_set_draw_y(RoundStats roundstats, float y);
void roundstats_reset(RoundStats roundstats);
Drawable roundstats_get_drawable(RoundStats roundstats);
void roundstats_peek_playerstats(RoundStats roundstats, float64 song_timestamp, PlayerStats playerstats);

void roundstats_tweenkeyframe_set_on_beat(RoundStats roundstats, TweenKeyframe tweenkeyframe, float rollback_beats, float beat_duration);
void roundstats_tweenkeyframe_set_on_hit(RoundStats roundstats, TweenKeyframe tweenkeyframe, float rollback_beats, float beat_duration);
void roundstats_tweenkeyframe_set_on_miss(RoundStats roundstats, TweenKeyframe tweenkeyframe, float rollback_beats, float beat_duration);
void roundstats_tweenkeyframe_set_bpm(RoundStats roundstats, float beats_per_minute);

int32_t roundstats_animate(RoundStats roundstats, float elapsed);

void roundstats_draw(RoundStats roundstats, PVRContext pvrctx);

#endif
