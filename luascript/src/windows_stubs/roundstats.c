#include "commons.h"
#include "roundstats.h"

static Drawable_t drawable = {};

void roundstats_hide(RoundStats roundstats, bool hide) {
    print_stub("roundstats_hide", "roundstats=%p hide=(bool)%i", roundstats, hide);
}
void roundstats_hide_nps(RoundStats roundstats, bool hide) {
    print_stub("roundstats_hide_nps", "roundstats=%p hide=(bool)%i", roundstats, hide);
}
void roundstats_set_draw_y(RoundStats roundstats, float y) {
    print_stub("roundstats_set_draw_y", "roundstats=%p y=%f", roundstats, y);
}
void roundstats_reset(RoundStats roundstats) {
    print_stub("roundstats_reset", "roundstats=%p", roundstats);
}
Drawable roundstats_get_drawable(RoundStats roundstats) {
    print_stub("roundstats_get_drawable", "roundstats=%p", roundstats);
    return &drawable;
}
void roundstats_tweenkeyframe_set_on_beat(RoundStats roundstats, TweenKeyframe tweenkeyframe, float rollback_beats, float beat_duration) {
    print_stub("roundstats_tweenkeyframe_set_on_beat", "roundstats=%p tweenkeyframe=%p rollback_beats=%f beat_duration=%f", roundstats, tweenkeyframe, rollback_beats, beat_duration);
}
void roundstats_tweenkeyframe_set_on_hit(RoundStats roundstats, TweenKeyframe tweenkeyframe, float rollback_beats, float beat_duration) {
    print_stub("roundstats_tweenkeyframe_set_on_hit", "roundstats=%p tweenkeyframe=%p rollback_beats=%f beat_duration=%f", roundstats, tweenkeyframe, rollback_beats, beat_duration);
}
void roundstats_tweenkeyframe_set_on_miss(RoundStats roundstats, TweenKeyframe tweenkeyframe, float rollback_beats, float beat_duration) {
    print_stub("roundstats_tweenkeyframe_set_on_miss", "roundstats=%p tweenkeyframe=%p rollback_beats=%f beat_duration=%f", roundstats, tweenkeyframe, rollback_beats, beat_duration);
}
void roundstats_tweenkeyframe_set_bpm(RoundStats roundstats, float beats_per_minute) {
    print_stub("roundstats_tweenkeyframe_set_bpm", "roundstats=%p beats_per_minute=%f", roundstats, beats_per_minute);
}
