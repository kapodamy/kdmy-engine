#include "commons.h"
#include "healthbar.h"

static Drawable_t drawable = {};

void healthbar_enable_extra_length(HealthBar healthbar, bool extra_enabled) {
    print_stub("healthbar_enable_extra_length", "healthbar=%p extra_enabled=(bool)%i", healthbar, extra_enabled);
}
void healthbar_enable_vertical(HealthBar healthbar, bool enable_vertical) {
    print_stub("healthbar_enable_vertical", "healthbar=%p enable_vertical=(bool)%i", healthbar, enable_vertical);
}
int32_t healthbar_state_opponent_add(HealthBar healthbar, ModelHolder icon_mdlhldr, ModelHolder bar_mdlhldr, const char* state_name) {
    print_stub("healthbar_state_opponent_add", "healthbar=%p icon_mdlhldr=%p bar_mdlhldr=%p state_name=%s", healthbar, icon_mdlhldr, bar_mdlhldr, state_name);
    return 0;
}
int32_t healthbar_state_opponent_add2(HealthBar healthbar, ModelHolder icon_mdlhldr, uint32_t bar_color_rgb8, const char* state_name) {
    print_stub("healthbar_state_opponent_add2", "healthbar=%p icon_mdlhldr=%p bar_color_rgb8=%u state_name=%s", healthbar, icon_mdlhldr, bar_color_rgb8, state_name);
    return 0;
}
int32_t healthbar_state_player_add(HealthBar healthbar, ModelHolder icon_mdlhldr, ModelHolder bar_mdlhldr, const char* state_name) {
    print_stub("healthbar_state_player_add", "healthbar=%p icon_mdlhldr=%p bar_mdlhldr=%p state_name=%s", healthbar, icon_mdlhldr, bar_mdlhldr, state_name);
    return 0;
}
int32_t healthbar_state_player_add2(HealthBar healthbar, ModelHolder icon_modelholder, uint32_t bar_color_rgb8, const char* state_name) {
    print_stub("healthbar_state_player_add2", "healthbar=%p icon_modelholder=%p bar_color_rgb8=%u state_name=%s", healthbar, icon_modelholder, bar_color_rgb8, state_name);
    return 0;
}
bool healthbar_state_background_add(HealthBar healthbar, ModelHolder modelholder, const char* state_name) {
    print_stub("healthbar_state_background_add", "healthbar=%p modelholder=%p state_name=%s", healthbar, modelholder, state_name);
    return 0;
}
bool healthbar_state_background_add2(HealthBar healthbar, uint32_t color_rgb8, AnimSprite animsprite, const char* state_name) {
    print_stub("healthbar_state_background_add2", "healthbar=%p color_rgb8=%u animsprite=%p state_name=%s", healthbar, color_rgb8, animsprite, state_name);
    return 0;
}
bool healthbar_load_warnings(HealthBar healthbar, ModelHolder modelholder, bool use_alt_icons) {
    print_stub("healthbar_load_warnings", "healthbar=%p modelholder=%p use_alt_icons=(bool)%i", healthbar, modelholder, use_alt_icons);
    return 0;
}
void healthbar_set_opponent_bar_color_rgb8(HealthBar healthbar, uint32_t color_rgb8) {
    print_stub("healthbar_set_opponent_bar_color_rgb8", "healthbar=%p color_rgb8=%u", healthbar, color_rgb8);
}
void healthbar_set_opponent_bar_color(HealthBar healthbar, float r, float g, float b) {
    print_stub("healthbar_set_opponent_bar_color", "healthbar=%p r=%f g=%f b=%f", healthbar, r, g ,b);
}
void healthbar_set_player_bar_color_rgb8(HealthBar healthbar, uint32_t color_rgb8) {
    print_stub("healthbar_set_player_bar_color_rgb8", "healthbar=%p color_rgb8=%u", healthbar, color_rgb8);
}
void healthbar_set_player_bar_color(HealthBar healthbar, float r, float g, float b) {
    print_stub("healthbar_set_player_bar_color", "healthbar=%p r=%f g=%f b=%f", healthbar, r, g ,b);
}
int32_t healthbar_state_toggle(HealthBar healthbar, const char* state_name) {
    print_stub("healthbar_state_toggle", "healthbar=%p state_name=%s", healthbar, state_name);
    return 0;
}
bool healthbar_state_toggle_background(HealthBar healthbar, const char* state_name) {
    print_stub("healthbar_state_toggle_background", "healthbar=%p state_name=%s", healthbar, state_name);
    return 0;
}
bool healthbar_state_toggle_player(HealthBar healthbar, const char* state_name) {
    print_stub("healthbar_state_toggle_player", "healthbar=%p state_name=%s", healthbar, state_name);
    return 0;
}
bool healthbar_state_toggle_opponent(HealthBar healthbar, const char* state_name) {
    print_stub("healthbar_state_toggle_opponent", "healthbar=%p state_name=%s", healthbar, state_name);
    return 0;
}
void healthbar_set_bump_animation_opponent(HealthBar healthbar, AnimSprite animsprite) {
    print_stub("healthbar_set_bump_animation_opponent", "healthbar=%p animsprite=%p", healthbar, animsprite);
}
void healthbar_set_bump_animation_player(HealthBar healthbar, AnimSprite animsprite) {
    print_stub("healthbar_set_bump_animation_player", "healthbar=%p animsprite=%p", healthbar, animsprite);
}
void healthbar_bump_enable(HealthBar healthbar, bool enable_bump) {
    print_stub("healthbar_bump_enable", "healthbar=%p enable_bump=(bool)%i", healthbar, enable_bump);
}
void healthbar_set_bpm(HealthBar healthbar, float beats_per_minute) {
    print_stub("healthbar_set_bpm", "healthbar=%p beats_per_minute=%f", healthbar, beats_per_minute);
}
void healthbar_set_alpha(HealthBar healthbar, float alpha) {
    print_stub("healthbar_set_alpha", "healthbar=%p alpha=%f", healthbar, alpha);
}
void healthbar_set_visible(HealthBar healthbar, bool visible) {
    print_stub("healthbar_set_visible", "healthbar=%p visible=(bool)%i", healthbar, visible);
}
Drawable healthbar_get_drawable(HealthBar healthbar) {
    print_stub("healthbar_get_drawable", "healthbar=%p", healthbar);
    return &drawable;
}
void healthbar_animation_set(HealthBar healthbar, AnimSprite animsprite) {
    print_stub("healthbar_animation_set", "healthbar=%p animsprite=%p", healthbar, animsprite);
}
void healthbar_animation_restart(HealthBar healthbar) {
    print_stub("healthbar_animation_restart", "healthbar=%p", healthbar);
}
void healthbar_animation_end(HealthBar healthbar) {
    print_stub("healthbar_animation_end", "healthbar=%p", healthbar);
}
void healthbar_disable_progress_animation(HealthBar healthbar, bool disable) {
    print_stub("healthbar_disable_progress_animation", "healthbar=%p disable=(bool)%i", healthbar, disable);
}
float healthbar_set_health_position(HealthBar healthbar, float max_health, float health, bool opponent_recover) {
    print_stub("healthbar_set_health_position", "healthbar=%p max_health=%f health=%f opponent_recover=(bool)%i", healthbar, max_health, health, opponent_recover);
    return 0;
}
void healthbar_set_health_position2(HealthBar healthbar, float percent) {
    print_stub("healthbar_set_health_position2", "healthbar=%p percent=%f", healthbar, percent);
}
void healthbar_disable_icon_overlap(HealthBar healthbar, bool disable) {
    print_stub("healthbar_disable_icon_overlap", "healthbar=%p disable=(bool)%i", healthbar, disable);
}
void healthbar_disable_warnings(HealthBar healthbar, bool disable) {
    print_stub("healthbar_disable_warnings", "healthbar=%p disable=(bool)%i", healthbar, disable);
}
void healthbar_enable_low_health_flash_warning(HealthBar healthbar, bool enable) {
    print_stub("healthbar_enable_low_health_flash_warning", "healthbar=%p enable=(bool)%i", healthbar, enable);
}
void healthbar_hide_warnings(HealthBar healthbar) {
    print_stub("healthbar_hide_warnings", "healthbar=%p", healthbar);
}
void healthbar_show_drain_warning(HealthBar healthbar, bool use_fast_drain) {
    print_stub("healthbar_show_drain_warning", "healthbar=%p use_fast_drain=(bool)%i", healthbar, use_fast_drain);
}
void healthbar_show_locked_warning(HealthBar healthbar) {
    print_stub("healthbar_show_locked_warning", "healthbar=%p", healthbar);
}
