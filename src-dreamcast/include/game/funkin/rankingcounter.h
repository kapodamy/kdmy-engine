#ifndef _rankingcounter_h
#define _rankingcounter_h

#include <stdbool.h>
#include <stdint.h>

#include "animsprite.h"
#include "drawable_types.h"
#include "fontholder.h"
#include "layout_types.h"
#include "modelholder.h"
#include "playerstats_types.h"
#include "pvrcontext_types.h"


typedef struct RankingCounter_s* RankingCounter;


RankingCounter rankingcounter_init(LayoutPlaceholder* plchldr_rank, LayoutPlaceholder* plchldr_accuracy, FontHolder fnthldr);
void rankingcounter_destroy(RankingCounter* rankingcounter);

int32_t rankingcounter_add_state(RankingCounter rankingcounter, ModelHolder modelholder, const char* state_name);
void rankingcounter_toggle_state(RankingCounter rankingcounter, const char* state_name);
void rankingcounter_peek_ranking(RankingCounter rankingcounter, PlayerStats playerstats);
void rankingcounter_reset(RankingCounter rankingcounter);
void rankingcounter_hide_accuracy(RankingCounter rankingcounter, bool hide);
void rankingcounter_use_percent_instead(RankingCounter rankingcounter, bool use_accuracy_percenter);
void rankingcounter_set_default_ranking_animation(RankingCounter rankingcounter, AnimList animlist);
void rankingcounter_set_default_ranking_animation2(RankingCounter rankingcounter, AnimSprite animsprite);

void rankingcounter_set_default_ranking_text_animation(RankingCounter rankingcounter, AnimList animlist);
void rankingcounter_set_default_ranking_text_animation2(RankingCounter rankingcounter, AnimSprite animsprite);

void rankingcounter_set_alpha(RankingCounter rankingcounter, float alpha);
void rankingcounter_set_offsetcolor(RankingCounter rankingcounter, float r, float g, float b, float a);
void rankingcounter_set_offsetcolor_to_default(RankingCounter rankingcounter);

void rankingcounter_animation_set(RankingCounter rankingcounter, AnimSprite animsprite);
void rankingcounter_animation_restart(RankingCounter rankingcounter);
void rankingcounter_animation_end(RankingCounter rankingcounter);

int32_t rankingcounter_animate1(RankingCounter rankingcounter, float elapsed);
int32_t rankingcounter_animate2(RankingCounter rankingcounter, float elapsed);
void rankingcounter_draw1(RankingCounter rankingcounter, PVRContext pvrctx);
void rankingcounter_draw2(RankingCounter rankingcounter, PVRContext pvrctx);

#endif
