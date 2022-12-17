#include "commons.h"
#include "strums.h"

static Strum_t strum = {};
static Drawable_t drawable = {};

Drawable strums_get_drawable(Strums strums) {
    print_stub("strums_get_drawable", "strums=%p", strums);
    return &drawable;
}
void strums_set_scroll_speed(Strums strums, float speed) {
    print_stub("strums_set_scroll_speed", "strums=%p speed=%f", strums, speed);
}
void strums_set_scroll_direction(Strums strums, ScrollDirection direction) {
    print_stub("strums_set_scroll_direction", "strums=%p direction=%i", strums, direction);
}
void strums_set_marker_duration_multiplier(Strums strums, float multipler) {
    print_stub("strums_set_marker_duration_multiplier", "strums=%p multipler=%f", strums, multipler);
}
void strums_set_bpm(Strums strums, float bpm) {
    print_stub("strums_set_bpm", "strums=%p bpm=%f", strums, bpm);
}
void strums_disable_beat_synced_idle_and_continous(Strums strums, bool disabled) {
    print_stub("strums_disable_beat_synced_idle_and_continous", "strums=%p disabled=(bool)%i", strums, disabled);
}
void strums_reset(Strums strums, float scroll_speed, const char* state_name) {
    print_stub("strums_reset", "strums=%p scroll_speed=%f state_name=%s", strums, scroll_speed, state_name);
}
void strums_force_key_release(Strums strums) {
    print_stub("strums_force_key_release", "strums=%p", strums);
}
float strums_set_alpha(Strums strums, float alpha) {
    print_stub("strums_set_alpha", "strums=%p alpha=%f", strums, alpha);
    return 0;
}
void strums_enable_background(Strums strums, bool enable) {
    print_stub("strums_enable_background", "strums=%p enable=(bool)%i", strums, enable);
}
void strums_set_keep_aspect_ratio_background(Strums strums, bool enable) {
    print_stub("strums_set_keep_aspect_ratio_background", "strums=%p enable=(bool)%i", strums, enable);
}
void strums_set_alpha_background(Strums strums, float alpha) {
    print_stub("strums_set_alpha_background", "strums=%p alpha=%f", strums, alpha);
}
void strums_set_alpha_sick_effect(Strums strums, float alpha) {
    print_stub("strums_set_alpha_sick_effect", "strums=%p alpha=%f", strums, alpha);
}
void strums_set_draw_offset(Strums strums, float offset_milliseconds) {
    print_stub("strums_set_draw_offset", "strums=%p offset_milliseconds=%f", strums, offset_milliseconds);
}
void strums_state_add(Strums strums, ModelHolder mdlhldr_mrkr, ModelHolder mdlhldr_sck_ffct, ModelHolder mdlhldr_bckgrnd, const char* state_name) {
    print_stub("strums_state_add", "strums=%p mdlhldr_mrkr=%p mdlhldr_sck_ffct=%p mdlhldr_bckgrnd=%p state_name=%s", strums, mdlhldr_mrkr, mdlhldr_sck_ffct, mdlhldr_bckgrnd, state_name);
}
int32_t strums_state_toggle(Strums strums, const char* state_name) {
    print_stub("strums_state_toggle", "strums=%p state_name=%s", strums, state_name);
    return 0;
}
int32_t strums_state_toggle_notes(Strums strums, const char* state_name) {
    print_stub("strums_state_toggle_notes", "strums=%p state_name=%s", strums, state_name);
    return 0;
}
void strums_state_toggle_marker_and_sick_effect(Strums strums, const char* state_name) {
    print_stub("strums_state_toggle_marker_and_sick_effect", "strums=%p state_name=%s", strums, state_name);
}
int32_t strums_get_lines_count(Strums strums) {
    print_stub("strums_get_lines_count", "strums=%p", strums);
    return 0;
}
Strum strums_get_strum_line(Strums strums, int32_t index) {
    print_stub("strums_get_strum_line", "strums=%p index=%i", strums, index);
    return &strum;
}
void strums_enable_post_sick_effect_draw(Strums strums, bool enable) {
    print_stub("strums_enable_post_sick_effect_draw", "strums=%p enable=(bool)%i", strums, enable);
}
void strums_animation_set(Strums strums, AnimSprite animsprite) {
    print_stub("strums_animation_set", "strums=%p animsprite=%p", strums, animsprite);
}
void strums_animation_restart(Strums strums) {
    print_stub("strums_animation_restart", "strums=%p", strums);
}
void strums_animation_end(Strums strums) {
    print_stub("strums_animation_end", "strums=%p", strums);
}
