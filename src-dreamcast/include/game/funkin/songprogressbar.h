#ifndef _songprogressbar_h
#define _songprogressbar_h

#include <stdint.h>
#include <stdbool.h>

#include "fontholder.h"
#include "float64.h"
#include "drawable_types.h"
#include "vertexprops_types.h"


//
//  forward reference
//

typedef struct SongPlayer_s* SongPlayer;
typedef struct AnimSprite_s* AnimSprite;


typedef struct SongProgressbar_s* SongProgressbar;

SongProgressbar songprogressbar_init(float x, float y, float z, float width, float height, Align align, float border_size, bool is_vertical, bool show_time, FontHolder fontholder, float font_size, float font_border_size, uint32_t color_rgba8_text, uint32_t color_rgba8_background, uint32_t color_rgba8_back, uint32_t color_rgba8_progress);
void songprogressbar_destroy(SongProgressbar* songprogressbar);

void songprogressbar_set_songplayer(SongProgressbar songprogressbar, SongPlayer songplayer);
void songprogressbar_set_duration(SongProgressbar songprogressbar, float64 duration);
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
float songprogressbar_manual_set_position(SongProgressbar songprogressbar, float64 elapsed, float64 duration, bool should_update_time_text);
void songprogressbar_animation_set(SongProgressbar songprogressbar, AnimSprite animsprite);
void songprogressbar_animation_restart(SongProgressbar songprogressbar);
void songprogressbar_animation_end(SongProgressbar songprogressbar);

int32_t songprogressbar_animate(SongProgressbar songprogressbar, float elapsed);
void songprogressbar_draw(SongProgressbar songprogressbar, PVRContext pvrctx);

#endif
