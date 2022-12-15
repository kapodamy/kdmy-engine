#ifndef __strum_h
#define __strum_h

#include "modelholder.h"
#include "modifier.h"
#include "drawable.h"
#include "tweenkeyframe.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    int dummy;
} Strum_t;
typedef Strum_t* Strum;


#define STRUM_UPSCROLL 0
#define STRUM_LEFTSCROLL 1
#define STRUM_DOWNSCROLL 2
#define STRUM_RIGHTSCROLL 3
typedef int32_t ScrollDirection;

#define STRUM_PRESS_STATE_NONE 0
#define STRUM_PRESS_STATE_HIT 1
#define STRUM_PRESS_STATE_HIT_SUSTAIN 2
#define STRUM_PRESS_STATE_PENALTY_NOTE 3
#define STRUM_PRESS_STATE_PENALTY_HIT 4
typedef int32_t StrumPressState;

#define STRUM_SCRIPT_TARGET_MARKER 0
#define STRUM_SCRIPT_TARGET_SICK_EFFECT 1
#define STRUM_SCRIPT_TARGET_BACKGROUND 2
#define STRUM_SCRIPT_TARGET_STRUM_LINE 3
#define STRUM_SCRIPT_TARGET_ALL 4
typedef int32_t StrumScriptTarget;

#define STRUM_SCRIPT_ON_HIT_DOWN 0
#define STRUM_SCRIPT_ON_HIT_UP 1
#define STRUM_SCRIPT_ON_MISS 2
#define STRUM_SCRIPT_ON_PENALITY 3
#define STRUM_SCRIPT_ON_IDLE 4
#define STRUM_SCRIPT_ON_ALL 5
typedef int32_t StrumScriptOn;



void strum_update_draw_location(Strum strum, float x, float y);
void strum_set_scroll_speed(Strum strum, float speed);
void strum_set_scroll_direction(Strum strum, ScrollDirection direction);
void strum_set_marker_duration_multiplier(Strum strum, float multipler);
void strum_reset(Strum strum, float scroll_speed, const char* state_name);
void strum_force_key_release(Strum strum);
int32_t strum_get_press_state_changes(Strum strum);
StrumPressState strum_get_press_state(Strum strum);
const char* strum_get_name(Strum strum);
float strum_get_marker_duration(Strum strum);
void strum_set_player_id(Strum strum, int32_t player_id);
void strum_enable_background(Strum strum, bool enable);
void strum_enable_sick_effect(Strum strum, bool enable);
void strum_state_add(Strum strum, ModelHolder mdlhldr_mrkr, ModelHolder mdlhldr_sck_ffct, ModelHolder mdlhldr_bckgrnd, const char* state_name);
int32_t strum_state_toggle(Strum strum, const char* state_name);
int32_t strum_state_toggle_notes(Strum strum, const char* state_name);
bool strum_state_toggle_sick_effect(Strum strum, const char* state_name);
int32_t strum_state_toggle_marker(Strum strum, const char* state_name);
bool strum_state_toggle_background(Strum strum, const char* state_name);
float strum_set_alpha_background(Strum strum, float alpha);
float strum_set_alpha_sick_effect(Strum strum, float alpha);
void strum_set_keep_aspect_ratio_background(Strum strum, bool enable);
void strum_draw_sick_effect_apart(Strum strum, bool enable);
void strum_set_extra_animation(Strum strum, StrumScriptTarget strum_script_target, StrumScriptOn strum_script_on, bool undo, AnimSprite animsprite);
void strum_set_extra_animation_continuous(Strum strum, StrumScriptTarget strum_script_target, AnimSprite animsprite);
void strum_set_notesmaker_tweenkeyframe(Strum strum, TweenKeyframe tweenkeyframe, bool apply_to_marker_too);
void strum_set_sickeffect_size_ratio(Strum strum, float size_ratio);
void strum_set_alpha(Strum strum, float alpha);
void strum_set_visible(Strum strum, bool visible);
void strum_set_draw_offset(Strum strum, float offset_milliseconds);
Modifier strum_get_modifier(Strum strum);
double strum_get_duration(Strum strum);
void strum_animation_restart(Strum strum);
void strum_animation_end(Strum strum);
Drawable strum_get_drawable(Strum strum);

#endif
