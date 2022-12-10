#ifndef __songprogressbar_h
#define __songprogressbar_h

#include "animsprite.h"
#include "songplayer.h"
#include "drawable.h"

typedef struct {
    int dummy;
} SongProgressbar_t;
typedef SongProgressbar_t* SongProgressbar;

void songprogressbar_set_songplayer(SongProgressbar songprogressbar, SongPlayer songplayer);
void songprogressbar_set_duration(SongProgressbar songprogressbar, double duration);
Drawable songprogressbar_get_drawable(SongProgressbar songprogressbar);
void songprogressbar_set_visible(SongProgressbar songprogressbar, bool visible);
void songprogressbar_set_background_color(SongProgressbar songprogressbar, float r, float g, float b, float a);
void songprogressbar_set_bar_back_color(SongProgressbar songprogressbar, float r, float g, float b, float a);
void songprogressbar_set_bar_progress_color(SongProgressbar songprogressbar, float r, float g, float b, float a);
void songprogressbar_set_text_color(SongProgressbar songprogressbar, float r, float g, float b, float a);
void songprogressbar_hide_time(SongProgressbar songprogressbar, bool hidden);
void songprogressbar_show_elapsed(SongProgressbar songprogressbar, bool elapsed_or_remain_time);
void songprogressbar_manual_update_enable(SongProgressbar songprogressbar, bool enabled);
void songprogressbar_manual_set_text(SongProgressbar songprogressbar, const char* text);
double songprogressbar_manual_set_position(SongProgressbar songprogressbar, double elapsed, double duration, bool should_update_time_text);
void songprogressbar_animation_set(SongProgressbar songprogressbar, AnimSprite animsprite);
void songprogressbar_animation_restart(SongProgressbar songprogressbar);
void songprogressbar_animation_end(SongProgressbar songprogressbar);

#endif
