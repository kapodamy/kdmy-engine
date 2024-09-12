#ifndef _week_pause_h
#define _week_pause_h

#include "week_types.h"


typedef struct WeekPause_s* WeekPause;


WeekPause week_pause_init(const char* exit_to_weekselector_label);
void week_pause_destroy(WeekPause* weekpause_ptr);

void week_pause_external_set_text(WeekPause weekpause, int32_t index, const char* text);
void week_pause_external_set_visible(WeekPause weekpause, int32_t index, bool visible);
void week_pause_external_set_menu(WeekPause weekpause, const char* menumanifest_src);
void week_pause_change_background_music(WeekPause weekpause, const char* filename);
int32_t week_pause_helper_show(WeekPause weekpause, RoundContext roundcontext, int32_t dettached_index);
Layout week_pause_get_layout(WeekPause weekpause);

#endif
