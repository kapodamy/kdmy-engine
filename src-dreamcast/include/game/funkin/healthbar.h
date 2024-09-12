#ifndef _healthbar_h
#define _healthbar_h

#include <stdbool.h>
#include <stdint.h>

#include "modelholder.h"

typedef struct Healthbar_s* Healthbar;


#define HEALTHBAR_DEFAULT_COLOR_BACKGROUND 0x000000 // black
#define HEALTHBAR_DEFAULT_COLOR_DAD 0xFF0000        // red
#define HEALTHBAR_DEFAULT_COLOR_BOYFRIEND 0x00FF00  // green

extern const int32_t HEALTHBAR_DEFAULT_DIMMEN;    // 20px in a 1280x720 screen
extern const float HEALTHBAR_DEFAULT_BORDER_SIZE; // 4px in a 1280x720 screen

extern const int32_t HEALTHBAR_DEFAULT_ICON_SIZE; // 80px in a 1280x720 screen

extern const char* HEALTHBAR_ICON_PREFIX_NEUTRAL;

extern const float HEALTHBAR_RATIO_SIZE_NORMAL; // 600px in a 1280x720 screen
// extern const float HEALTHBAR_RATIO_SIZE_LONG; // 1180px in a 1280x70 screen

#define HEALTHBAR_WARNING_MODEL "/assets/common/image/week-round/healthbar_warns.xml"


Healthbar healthbar_init(float x, float y, float z, float length, float dimmen, float border, float icon_overlap, float warn_height, float lock_height);
void healthbar_destroy(Healthbar* healthbar);

void healthbar_set_layout_size(Healthbar healthbar, float width, float height);
void healthbar_enable_extra_length(Healthbar healthbar, bool extra_enabled);
void healthbar_enable_vertical(Healthbar healthbar, bool enable_vertical);

int32_t healthbar_state_opponent_add(Healthbar healthbar, ModelHolder icon_mdlhldr, ModelHolder bar_mdlhldr, const char* state_name);
int32_t healthbar_state_opponent_add2(Healthbar healthbar, ModelHolder icon_mdlhldr, uint32_t bar_color_rgb8, const char* state_name);
int32_t healthbar_state_player_add(Healthbar healthbar, ModelHolder icon_mdlhldr, ModelHolder bar_mdlhldr, const char* state_name);
int32_t healthbar_state_player_add2(Healthbar healthbar, ModelHolder icon_modelholder, uint32_t bar_color_rgb8, const char* state_name);

bool healthbar_state_background_add(Healthbar healthbar, ModelHolder modelholder, const char* state_name);
bool healthbar_state_background_add2(Healthbar healthbar, uint32_t color_rgb8, AnimSprite animsprite, const char* state_name);
bool healthbar_load_warnings(Healthbar healthbar, ModelHolder modelholder, bool use_alt_icons);

void healthbar_set_opponent_bar_color_rgb8(Healthbar healthbar, uint32_t color_rgb8);
void healthbar_set_opponent_bar_color(Healthbar healthbar, float r, float g, float b);
void healthbar_set_player_bar_color_rgb8(Healthbar healthbar, uint32_t color_rgb8);
void healthbar_set_player_bar_color(Healthbar healthbar, float r, float g, float b);

int32_t healthbar_state_toggle(Healthbar healthbar, const char* state_name);
bool healthbar_state_toggle_background(Healthbar healthbar, const char* state_name);
bool healthbar_state_toggle_player(Healthbar healthbar, const char* state_name);
bool healthbar_state_toggle_opponent(Healthbar healthbar, const char* state_name);

void healthbar_set_bump_animation(Healthbar healthbar, AnimList animlist);
void healthbar_set_bump_animation_opponent(Healthbar healthbar, AnimSprite animsprite);
void healthbar_set_bump_animation_player(Healthbar healthbar, AnimSprite animsprite);
void healthbar_bump_enable(Healthbar healthbar, bool enable_bump);

void healthbar_set_bpm(Healthbar healthbar, float beats_per_minute);

void healthbar_set_offsetcolor(Healthbar healthbar, float r, float g, float b, float a);
void healthbar_set_alpha(Healthbar healthbar, float alpha);
void healthbar_set_visible(Healthbar healthbar, bool visible);
Modifier* healthbar_get_modifier(Healthbar healthbar);
Drawable healthbar_get_drawable(Healthbar healthbar);

void healthbar_animation_set(Healthbar healthbar, AnimSprite animsprite);
void healthbar_animation_restart(Healthbar healthbar);
void healthbar_animation_end(Healthbar healthbar);

int32_t healthbar_animate(Healthbar healthbar, float elapsed);
void healthbar_draw(Healthbar healthbar, PVRContext pvrctx);

void healthbar_disable_progress_animation(Healthbar healthbar, bool disable);
float healthbar_set_health_position(Healthbar healthbar, float max_health, float health, bool opponent_recover);
void healthbar_set_health_position2(Healthbar healthbar, float percent);

void healthbar_disable_icon_overlap(Healthbar healthbar, bool disable);
void healthbar_disable_warnings(Healthbar healthbar, bool disable);
void healthbar_enable_low_health_flash_warning(Healthbar healthbar, bool enable);

void healthbar_hide_warnings(Healthbar healthbar);
void healthbar_show_drain_warning(Healthbar healthbar, bool use_fast_drain);
void healthbar_show_locked_warning(Healthbar healthbar);

void healthbar_get_bar_midpoint(Healthbar healthbar, float* x, float* y);
float healthbar_get_percent(Healthbar healthbar);

#endif
