#ifndef _weekselector_weektitle_h
#define _weekselector_weektitle_h

#include <stdbool.h>
#include <stdint.h>

#include "pvrcontext_types.h"

//
//  forward references
//

typedef struct Layout_s* Layout;
typedef struct WeekSelectorWeekList_s* WeekSelectorWeekList;
typedef struct WeekSelectorDifficult_s* WeekSelectorDifficult;


typedef struct WeekSelectorWeekTitle_s* WeekSelectorWeekTitle;

WeekSelectorWeekTitle weekselector_weektitle_init(Layout layout);
void weekselector_weektitle_destroy(WeekSelectorWeekTitle* weektitle);
void weekselector_weektitle_draw(WeekSelectorWeekTitle weektitle, PVRContext pvrctx);
void weekselector_weektitle_move(WeekSelectorWeekTitle weektitle, WeekSelectorWeekList weeklist, bool use_warns_placeholder);
void weekselector_weektitle_move_difficult(WeekSelectorWeekTitle weektitle, WeekSelectorDifficult weekdifficult);

#endif
