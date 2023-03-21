#include "character.h"
#include "commons.h"

static Modifier_t stub_modifier = {};


void character_use_alternate_sing_animations(Character character, bool enable) {
    print_stub("character_use_alternate_sing_animations", "character=%p enable=(bool)%i", character, enable);
}
void character_set_draw_location(Character character, float x, float y) {
    print_stub("character_set_draw_location", "character=%p x=%f y=%f", character, x, y);
}
void character_set_draw_align(Character character, Align align_vertical, Align align_horizontal) {
    print_stub("character_set_draw_align", "character=%p align_vertical=%hhu align_horizontal=%hhu", character, align_vertical, align_horizontal);
}
void character_update_reference_size(Character character, float width, float height) {
    print_stub("character_update_reference_size", "character=%p width=%f height=%f", character, width, height);
}
void character_enable_reference_size(Character character, bool enable) {
    print_stub("character_enable_reference_size", "character=%p enable=(bool)%i", character, enable);
}
void character_set_offset(Character character, float offset_x, float offset_y) {
    print_stub("character_set_offset", "character=%p offset_x=%f offset_y=%f", character, offset_x, offset_y);
}
int32_t character_state_add(Character character, ModelHolder modelholder, const char* state_name) {
    print_stub("character_state_add", "character=%p modelholder=%p state_name=%s", character, modelholder, state_name);
    return 0;
}
int32_t character_state_toggle(Character character, const char* state_name) {
    print_stub("character_state_toggle", "character=%p state_name=%s", character, state_name);
    return 0;
}
bool character_play_hey(Character character) {
    print_stub("character_play_hey", "character=%p", character);
    return 0;
}
bool character_play_idle(Character character) {
    print_stub("character_play_idle", "character=%p", character);
    return 0;
}
bool character_play_sing(Character character, const char* direction, bool prefer_sustain) {
    print_stub("character_play_sing", "character=%p direction=%s prefer_sustain=(bool)%i", character, direction, prefer_sustain);
    return 0;
}
bool character_play_miss(Character character, const char* direction, bool keep_in_hold) {
    print_stub("character_play_miss", "character=%p direction=%s keep_in_hold=(bool)%i", character, direction, keep_in_hold);
    return 0;
}
bool character_play_extra(Character character, const char* extra_animation_name, bool prefer_sustain) {
    print_stub("character_play_extra", "character=%p extra_animation_name=%s prefer_sustain=(bool)%i", character, extra_animation_name, prefer_sustain);
    return 0;
}
void character_set_idle_speed(Character character, float speed) {
    print_stub("character_set_idle_speed", "character=%p speed=%f", character, speed);
}
void character_set_scale(Character character, float scale_factor) {
    print_stub("character_set_scale", "character=%p scale_factor=%f", character, scale_factor);
}
void character_reset(Character character) {
    print_stub("character_reset", "character=%p", character);
}
void character_enable_continuous_idle(Character character, bool enable) {
    print_stub("character_enable_continuous_idle", "character=%p enable=(bool)%i", character, enable);
}
bool character_is_idle_active(Character character) {
    print_stub("character_is_idle_active", "character=%p", character);
    return 0;
}
void character_enable_flip_correction(Character character, bool enable) {
    print_stub("character_enable_flip_correction", "character=%p enable=%i", character, enable);
}
void character_flip_orientation(Character character, bool enable) {
    print_stub("character_flip_orientation", "character=%p enable=%i", character, enable);
}
void character_face_as_opponent(Character character, bool face_as_opponent) {
    print_stub("character_face_as_opponent", "character=%p face_as_opponent=(bool)%i", character, face_as_opponent);
}
void character_set_z_index(Character character, float z) {
    print_stub("character_set_z_index", "character=%p z=%f", character, z);
}
void character_set_z_offset(Character character, float z_offset) {
    print_stub("character_set_z_offset", "character=%p z_offset=%f", character, z_offset);
}
void character_animation_set(Character character, const AnimSprite animsprite) {
    print_stub("character_animation_set", "character=%p animsprite=%p", character, animsprite);
}
void character_animation_restart(Character character) {
    print_stub("character_animation_restart", "character=%p", character);
}
void character_animation_end(Character character) {
    print_stub("character_animation_end", "character=%p", character);
}
void character_set_color_offset(Character character, float r, float g, float b, float a) {
    print_stub("character_set_color_offset", "character=%p r=%f g=%f b=%f a=%f", character, r, g, b, a);
}
void character_set_color_offset_to_default(Character character) {
    print_stub("character_set_color_offset_to_default", "character=%p", character);
}
void character_set_alpha(Character character, float alpha) {
    print_stub("character_set_alpha", "character=%p alpha=%f", character, alpha);
}
void character_set_visible(Character character, bool visible) {
    print_stub("character_set_visible", "character=%p visible=(bool)%i", character, visible);
}
Modifier character_get_modifier(Character character) {
    print_stub("character_get_modifier", "character=%p", character);
    return &stub_modifier;
}
bool character_has_direction(Character character, const char* name, bool is_extra) {
    print_stub("character_has_direction", "character=%p name=%s is_extra=(bool)%i", character, name, is_extra);
    return 0;
}
int32_t character_get_play_calls(Character character) {
    print_stub("character_get_play_calls", "character=%p", character);
    return 12345;
}
int32_t character_get_commited_animations_count(Character character) {
    print_stub("character_get_commited_animations_count", "character=%p", character);
    return 12345;
}
CharacterActionType character_get_current_action(Character character) {
    print_stub("character_get_current_action", "character=%p", character);
    return CHARACTER_ACTION_TYPE_IDLE;
}
void character_freeze_animation(Character character, bool enabled) {
    print_stub("character_get_current_action", "character=%p enabled=(bool)%i", character, enabled);
}
void character_trailing_enabled(Character character, bool enabled) {
    print_stub("character_trailing_enabled", "character=%p enabled=(bool)%i", character, enabled);
}
void character_trailing_set_params(Character character, int32_t length, float trail_delay, float trail_alpha, bool* darken_colors) {
    print_stub("character_trailing_set_params", "character=%p length=%i trail_delay=%f trail_alpha=%f darken_colors=%p", character, length, trail_delay, trail_alpha, darken_colors);
}
void character_trailing_set_offsetcolor(Character character, float r, float g, float b){
    print_stub("character_trailing_set_offsetcolor", "character=%p r=%f g=%f b=%f", character, r ,g ,b);
}
