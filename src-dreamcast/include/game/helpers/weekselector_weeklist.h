#ifndef _weekselector_weeklist_h
#define _weekselector_weeklist_h

#include "game/common/weekenumerator.h"
#include "layout_types.h"
#include "pvrcontext_types.h"

//
//  forward references
//

typedef struct AnimList_s* AnimList;
typedef struct ModelHolder_s* ModelHolder;
typedef struct Sprite_s* Sprite;


typedef struct WeekSelectorWeekList_s* WeekSelectorWeekList;


WeekSelectorWeekList weekselector_weeklist_init(AnimList animlist, ModelHolder modelholder, Layout layout);
void weekselector_weeklist_destroy(WeekSelectorWeekList* weeklist);

int32_t weekselector_weeklist_animate(WeekSelectorWeekList weeklist, float elapsed);
void weekselector_weeklist_draw(WeekSelectorWeekList weeklist, PVRContext pvrctx);

void weekselector_weeklist_toggle_choosen(WeekSelectorWeekList weeklist);
WeekInfo* weekselector_weeklist_get_selected(WeekSelectorWeekList weeklist);
Sprite weekselector_weeklist_peek_title_sprite(WeekSelectorWeekList weeklist);
bool weekselector_weeklist_scroll(WeekSelectorWeekList weeklist, int32_t offset);
bool weekselector_weeklist_select(WeekSelectorWeekList weeklist, int32_t index);
int32_t weekselector_weeklist_get_selected_index(WeekSelectorWeekList weeklist);
void weekselector_weeklist_set_beats(WeekSelectorWeekList weeklist, float beats_per_minute);

#endif
