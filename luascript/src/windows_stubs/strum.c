#include "commons.h"
#include "strum.h"

static Modifier_t modifier = {};
static Drawable_t drawable = {};

void strum_update_draw_location(Strum strum, float x, float y) {
    print_stub("strum_update_draw_location", "strum=%p x=%f y=%f", strum, x, y);
}
void strum_set_scroll_speed(Strum strum, float speed) {
    print_stub("strum_set_scroll_speed", "strum=%p speed=%f", strum, speed);
}
void strum_set_scroll_direction(Strum strum, ScrollDirection direction) {
    print_stub("strum_set_scroll_direction", "strum=%p direction=%p", strum, direction);
}
void strum_set_marker_duration_multiplier(Strum strum, float multipler) {
    print_stub("strum_set_marker_duration_multiplier", "strum=%p multipler=%f", strum, multipler);
}
void strum_reset(Strum strum, float scroll_speed, const char* state_name) {
    print_stub("strum_reset", "strum=%p scroll_speed=%f state_name=%s", strum, scroll_speed, state_name);
}
void strum_force_key_release(Strum strum) {
    print_stub("strum_force_key_release", "strum=%p", strum);
}
int32_t strum_get_press_state_changes(Strum strum) {
    print_stub("strum_get_press_state_changes", "strum=%p", strum);
    return 0;
}
StrumPressState strum_get_press_state(Strum strum) {
    print_stub("strum_get_press_state", "strum=%p", strum);
    return 0;
}
const char* strum_get_name(Strum strum) {
    print_stub("strum_get_name", "strum=%p", strum);
    return "abc123";
}
float strum_get_marker_duration(Strum strum) {
    print_stub("strum_get_marker_duration", "strum=%p", strum);
    return 0;
}
void strum_set_player_id(Strum strum, int32_t player_id) {
    print_stub("strum_set_player_id", "strum=%p player_id=%i", strum, player_id);
}
void strum_enable_background(Strum strum, bool enable) {
    print_stub("strum_enable_background", "strum=%p enable=(bool)%i", strum, enable);
}
void strum_enable_sick_effect(Strum strum, bool enable) {
    print_stub("strum_enable_sick_effect", "strum=%p enable=(bool)%i", strum, enable);
}
void strum_state_add(Strum strum, ModelHolder mdlhldr_mrkr, ModelHolder mdlhldr_sck_ffct, ModelHolder mdlhldr_bckgrnd, const char* state_name) {
    print_stub("strum_state_add", "strum=%p mdlhldr_mrkr=%p mdlhldr_sck_ffct=%p mdlhldr_bckgrnd=%p state_name=%s", strum, mdlhldr_mrkr, mdlhldr_sck_ffct, mdlhldr_bckgrnd, state_name);
}
int32_t strum_state_toggle(Strum strum, const char* state_name) {
    print_stub("strum_state_toggle", "strum=%p state_name=%s", strum, state_name);
    return 0;
}
int32_t strum_state_toggle_notes(Strum strum, const char* state_name) {
    print_stub("strum_state_toggle_notes", "strum=%p state_name=%s", strum, state_name);
    return 0;
}
bool strum_state_toggle_sick_effect(Strum strum, const char* state_name) {
    print_stub("strum_state_toggle_sick_effect", "strum=%p state_name=%s", strum, state_name);
    return 0;
}
int32_t strum_state_toggle_marker(Strum strum, const char* state_name) {
    print_stub("strum_state_toggle_marker", "strum=%p state_name=%s", strum, state_name);
    return 0;
}
bool strum_state_toggle_background(Strum strum, const char* state_name) {
    print_stub("strum_state_toggle_background", "strum=%p state_name=%s", strum, state_name);
    return 0;
}
float strum_set_alpha_background(Strum strum, float alpha) {
    print_stub("strum_set_alpha_background", "strum=%p alpha=%f", strum, alpha);
    return 0;
}
float strum_set_alpha_sick_effect(Strum strum, float alpha) {
    print_stub("strum_set_alpha_sick_effect", "strum=%p alpha=%f", strum, alpha);
    return 0;
}
void strum_set_keep_aspect_ratio_background(Strum strum, bool enable) {
    print_stub("strum_set_keep_aspect_ratio_background", "strum=%p enable=(bool)%i", strum, enable);
}
void strum_draw_sick_effect_apart(Strum strum, bool enable) {
    print_stub("strum_draw_sick_effect_apart", "strum=%p enable=(bool)%i", strum, enable);
}
void strum_set_extra_animation(Strum strum, StrumScriptTarget strum_script_target, StrumScriptOn strum_script_on, bool undo, AnimSprite animsprite) {
    print_stub("strum_set_extra_animation", "strum=%p strum_script_target=%p strum_script_on=%p undo=(bool)%i animsprite=%p", strum, strum_script_target, strum_script_on, undo, animsprite);
}
void strum_set_extra_animation_continuous(Strum strum, StrumScriptTarget strum_script_target, AnimSprite animsprite) {
    print_stub("strum_set_extra_animation_continuous", "strum=%p strum_script_target=%p animsprite=%p", strum, strum_script_target, animsprite);
}
void strum_set_notesmaker_tweenlerp(Strum strum, TweenLerp tweenlerp, bool apply_to_marker_too) {
    print_stub("strum_set_notesmaker_tweenlerp", "strum=%p tweenlerp=%p apply_to_marker_too=(bool)%i", strum, tweenlerp, apply_to_marker_too);
}
void strum_set_sickeffect_size_ratio(Strum strum, float size_ratio) {
    print_stub("strum_set_sickeffect_size_ratio", "strum=%p size_ratio=%f", strum, size_ratio);
}
void strum_set_alpha(Strum strum, float alpha) {
    print_stub("strum_set_alpha", "strum=%p alpha=%f", strum, alpha);
}
void strum_set_visible(Strum strum, bool visible) {
    print_stub("strum_set_visible", "strum=%p visible=(bool)%i", strum, visible);
}
void strum_set_draw_offset(Strum strum, float offset_milliseconds) {
    print_stub("strum_set_draw_offset", "strum=%p offset_milliseconds=%f", strum, offset_milliseconds);
}
Modifier strum_get_modifier(Strum strum) {
    print_stub("strum_get_modifier", "strum=%p", strum);
    return &modifier;
}
double strum_get_duration(Strum strum) {
    print_stub("strum_get_duration", "strum=%p", strum);
    return 0;
}
void strum_animation_restart(Strum strum) {
    print_stub("strum_animation_restart", "strum=%p", strum);
}
void strum_animation_end(Strum strum) {
    print_stub("strum_animation_end", "strum=%p", strum);
}
Drawable strum_get_drawable(Strum strum) {
    print_stub("strum_get_drawable", "strum=%p", strum);
    return &drawable;
}
