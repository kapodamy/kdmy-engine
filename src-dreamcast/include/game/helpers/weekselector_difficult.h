#ifndef _weekselector_difficult_h
#define _weekselector_difficult_h

#include <stdbool.h>
#include <stdint.h>

#include "pvrcontext_types.h"
#include "game/common/weekenumerator.h"

//
//  forward references
//

typedef struct AnimList_s* AnimList;
typedef struct Drawable_s* Drawable;
typedef struct Layout_s* Layout;
typedef struct ModelHolder_s* ModelHolder;
typedef struct StateSprite_s* StateSprite;
typedef struct StateSprite_s* StateSprite;


typedef struct WeekSelectorDifficult_s* WeekSelectorDifficult;


WeekSelectorDifficult weekselector_difficult_init(AnimList animlist, ModelHolder modelholder, Layout layout);
void weekselector_difficult_destroy(WeekSelectorDifficult* weekdifficult);

void weekselector_difficult_draw(WeekSelectorDifficult weekdifficult, PVRContext pvrctx);
int32_t weekselector_difficult_animate(WeekSelectorDifficult weekdifficult, float elapsed);

bool weekselector_difficult_select(WeekSelectorDifficult weekdifficult, int32_t new_index);
void weekselector_difficult_select_default(WeekSelectorDifficult weekdifficult);
bool weekselector_difficult_scroll(WeekSelectorDifficult weekdifficult, int32_t offset);
void weekselector_difficult_visible(WeekSelectorDifficult weekdifficult, bool visible);
const char* weekselector_difficult_get_selected(WeekSelectorDifficult weekdifficult);
StateSprite weekselector_difficult_peek_statesprite(WeekSelectorDifficult weekdifficult);
void weekselector_difficult_relayout(WeekSelectorDifficult weekdifficult, bool use_alt_placeholder);
bool weekselector_difficult_selected_is_locked(WeekSelectorDifficult weekdifficult);

void weekselector_difficult_load(WeekSelectorDifficult weekdifficult, WeekInfo* weekinfo, const char* default_difficult);

#endif
