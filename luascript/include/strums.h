#ifndef __strums_h
#define __strums_h

#include "strum.h"
#include "drawable.h"


typedef struct {
    int dummy;
} Strums_t;
typedef Strums_t* Strums;


Drawable strums_get_drawable(Strums strums);
void strums_set_scroll_speed(Strums strums, float speed);
void strums_set_scroll_direction(Strums strums, ScrollDirection direction);
void strums_set_marker_duration_multiplier(Strums strums, float multipler);
void strums_set_bpm(Strums strums, float bpm);
void strums_disable_beat_synced_idle_and_continous(Strums strums, bool disabled);
void strums_reset(Strums strums, float scroll_speed, const char* state_name);
void strums_force_key_release(Strums strums);
float strums_set_alpha(Strums strums, float alpha);
void strums_enable_background(Strums strums, bool enable);
void strums_set_keep_aspect_ratio_background(Strums strums, bool enable);
void strums_set_alpha_background(Strums strums, float alpha);
void strums_set_alpha_sick_effect(Strums strums, float alpha);
void strums_set_draw_offset(Strums strums, float offset_milliseconds);
void strums_state_add(Strums strums, ModelHolder mdlhldr_mrkr, ModelHolder mdlhldr_sck_ffct, ModelHolder mdlhldr_bckgrnd, const char* state_name);
int32_t strums_state_toggle(Strums strums, const char* state_name);
int32_t strums_state_toggle_notes(Strums strums, const char* state_name);
void strums_state_toggle_marker_and_sick_effect(Strums strums, const char* state_name);
int32_t strums_get_lines_count(Strums strums);
Strum strums_get_strum_line(Strums strums, int32_t index);
void strums_enable_post_sick_effect_draw(Strums strums, bool enable);
void strums_animation_set(Strums strums, AnimSprite animsprite);
void strums_animation_restart(Strums strums);
void strums_animation_end(Strums strums);

#endif
