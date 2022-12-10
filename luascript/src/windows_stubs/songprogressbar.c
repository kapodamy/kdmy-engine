#include "commons.h"
#include "songprogressbar.h"

static Drawable_t drawable = {};

void songprogressbar_set_songplayer(SongProgressbar songprogressbar, SongPlayer songplayer) {
    print_stub("songprogressbar_set_songplayer", "songprogressbar=%p songplayer=%p", songprogressbar, songplayer);
}
void songprogressbar_set_duration(SongProgressbar songprogressbar, double duration) {
    print_stub("songprogressbar_set_duration", "songprogressbar=%p duration=%f", songprogressbar, duration);
}
Drawable songprogressbar_get_drawable(SongProgressbar songprogressbar) {
    print_stub("songprogressbar_get_drawable", "songprogressbar=%p", songprogressbar);
    return &drawable;
}
void songprogressbar_set_visible(SongProgressbar songprogressbar, bool visible) {
    print_stub("songprogressbar_set_visible", "songprogressbar=%p visible=(bool)%i", songprogressbar, visible);
}
void songprogressbar_set_background_color(SongProgressbar songprogressbar, float r, float g, float b, float a) {
    print_stub("songprogressbar_set_background_color", "songprogressbar=%p r=%f g=%f b=%f a=%f", songprogressbar, r, g, b, a);
}
void songprogressbar_set_bar_back_color(SongProgressbar songprogressbar, float r, float g, float b, float a) {
    print_stub("songprogressbar_set_bar_back_color", "songprogressbar=%p r=%f g=%f b=%f a=%f", songprogressbar, r, g, b, a);
}
void songprogressbar_set_bar_progress_color(SongProgressbar songprogressbar, float r, float g, float b, float a) {
    print_stub("songprogressbar_set_bar_progress_color", "songprogressbar=%p r=%f g=%f b=%f a=%f", songprogressbar, r, g, b, a);
}
void songprogressbar_set_text_color(SongProgressbar songprogressbar, float r, float g, float b, float a) {
    print_stub("songprogressbar_set_text_color", "songprogressbar=%p r=%f g=%f b=%f a=%f", songprogressbar, r, g, b, a);
}
void songprogressbar_hide_time(SongProgressbar songprogressbar, bool hidden) {
    print_stub("songprogressbar_hide_time", "songprogressbar=%p hidden=(bool)%i", songprogressbar, hidden);
}
void songprogressbar_show_elapsed(SongProgressbar songprogressbar, bool elapsed_or_remain_time) {
    print_stub("songprogressbar_show_elapsed", "songprogressbar=%p elapsed_or_remain_time=(bool)%i", songprogressbar, elapsed_or_remain_time);
}
void songprogressbar_manual_update_enable(SongProgressbar songprogressbar, bool enabled) {
    print_stub("songprogressbar_manual_update_enable", "songprogressbar=%p enabled=(bool)%i", songprogressbar, enabled);
}
void songprogressbar_manual_set_text(SongProgressbar songprogressbar, const char* text) {
    print_stub("songprogressbar_manual_set_text", "songprogressbar=%p text=%s", songprogressbar, text);
}
double songprogressbar_manual_set_position(SongProgressbar songprogressbar, double elapsed, double duration, bool should_update_time_text) {
    print_stub("songprogressbar_manual_set_position", "songprogressbar=%p elapsed=%f duration=%f should_update_time_text=(bool)%i", songprogressbar, elapsed, duration, should_update_time_text);
    return 0;
}
void songprogressbar_animation_set(SongProgressbar songprogressbar, AnimSprite animsprite) {
    print_stub("songprogressbar_animation_set", "songprogressbar=%p animsprite=%p", songprogressbar, animsprite);
}
void songprogressbar_animation_restart(SongProgressbar songprogressbar) {
    print_stub("songprogressbar_animation_restart", "songprogressbar=%p", songprogressbar);
}
void songprogressbar_animation_end(SongProgressbar songprogressbar) {
    print_stub("songprogressbar_animation_end", "songprogressbar=%p", songprogressbar);
}
