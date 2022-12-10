#ifndef __rankingcounter_h
#define __rankingcounter_h

#include "modelholder.h"


typedef struct {
    int dummy;
} RankingCounter_t;
typedef RankingCounter_t* RankingCounter;


int32_t rankingcounter_add_state(RankingCounter rankingcounter, ModelHolder modelholder, const char* state_name);
void rankingcounter_toggle_state(RankingCounter rankingcounter, const char* state_name);
void rankingcounter_reset(RankingCounter rankingcounter);
void rankingcounter_hide_accuracy(RankingCounter rankingcounter, bool hide);
void rankingcounter_use_percent_instead(RankingCounter rankingcounter, bool use_accuracy_percenter);
void rankingcounter_set_default_ranking_animation2(RankingCounter rankingcounter, AnimSprite animsprite);
void rankingcounter_set_default_ranking_text_animation2(RankingCounter rankingcounter, AnimSprite animsprite);
void rankingcounter_set_alpha(RankingCounter rankingcounter, float alpha);
void rankingcounter_animation_set(RankingCounter rankingcounter, AnimSprite animsprite);
void rankingcounter_animation_restart(RankingCounter rankingcounter);
void rankingcounter_animation_end(RankingCounter rankingcounter);

#endif

