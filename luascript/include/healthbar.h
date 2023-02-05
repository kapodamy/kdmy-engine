#ifndef __healthbar_h
#define __healthbar_h

#include "drawable.h"
#include "modelholder.h"


typedef struct {
    int dummy;
} HealthBar_t;
typedef HealthBar_t* HealthBar;


void healthbar_enable_extra_length(HealthBar healthbar, bool extra_enabled);
void healthbar_enable_vertical(HealthBar healthbar, bool enable_vertical);
int32_t healthbar_state_opponent_add(HealthBar healthbar, ModelHolder icon_mdlhldr, ModelHolder bar_mdlhldr, const char* state_name);
int32_t healthbar_state_opponent_add2(HealthBar healthbar, ModelHolder icon_mdlhldr, uint32_t bar_color_rgb8, const char* state_name);
int32_t healthbar_state_player_add(HealthBar healthbar, ModelHolder icon_mdlhldr, ModelHolder bar_mdlhldr, const char* state_name);
int32_t healthbar_state_player_add2(HealthBar healthbar, ModelHolder icon_modelholder, uint32_t bar_color_rgb8, const char* state_name);
bool healthbar_state_background_add(HealthBar healthbar, ModelHolder modelholder, const char* state_name);
bool healthbar_state_background_add2(HealthBar healthbar, uint32_t color_rgb8, AnimSprite animsprite, const char* state_name);
bool healthbar_load_warnings(HealthBar healthbar, ModelHolder modelholder, bool use_alt_icons);
void healthbar_set_opponent_bar_color_rgb8(HealthBar healthbar, uint32_t color_rgb8);
void healthbar_set_opponent_bar_color(HealthBar healthbar, float r, float g, float b);
void healthbar_set_player_bar_color_rgb8(HealthBar healthbar, uint32_t color_rgb8);
void healthbar_set_player_bar_color(HealthBar healthbar, float r, float g, float b);
int32_t healthbar_state_toggle(HealthBar healthbar, const char* state_name);
bool healthbar_state_toggle_background(HealthBar healthbar, const char* state_name);
bool healthbar_state_toggle_player(HealthBar healthbar, const char* state_name);
bool healthbar_state_toggle_opponent(HealthBar healthbar, const char* state_name);
void healthbar_set_bump_animation_opponent(HealthBar healthbar, AnimSprite animsprite);
void healthbar_set_bump_animation_player(HealthBar healthbar, AnimSprite animsprite);
void healthbar_bump_enable(HealthBar healthbar, bool enable_bump);
void healthbar_set_bpm(HealthBar healthbar, float beats_per_minute);
void healthbar_set_alpha(HealthBar healthbar, float alpha);
void healthbar_set_visible(HealthBar healthbar, bool visible);
Drawable healthbar_get_drawable(HealthBar healthbar);
void healthbar_animation_set(HealthBar healthbar, AnimSprite animsprite);
void healthbar_animation_restart(HealthBar healthbar);
void healthbar_animation_end(HealthBar healthbar);
void healthbar_disable_progress_animation(HealthBar healthbar, bool disable);
float healthbar_set_health_position(HealthBar healthbar, float max_health, float health, bool opponent_recover);
void healthbar_set_health_position2(HealthBar healthbar, float percent);
void healthbar_disable_icon_overlap(HealthBar healthbar, bool disable);
void healthbar_disable_warnings(HealthBar healthbar, bool disable);
void healthbar_enable_low_health_flash_warning(HealthBar healthbar, bool enable);
void healthbar_hide_warnings(HealthBar healthbar);
void healthbar_show_drain_warning(HealthBar healthbar, bool use_fast_drain);
void healthbar_show_locked_warning(HealthBar healthbar);
void healthbar_get_bar_midpoint(HealthBar healthbar, float* x, float* y);
float healthbar_get_percent(HealthBar healthbar);
#endif
