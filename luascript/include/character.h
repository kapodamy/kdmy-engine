#ifndef __character_h
#define __character_h

#include <stdint.h>
#include <stdbool.h>

#include "vertexprops.h"
#include "modelholder.h"
#include "animsprite.h"
#include "modifier.h"

typedef struct _Character_t {
    int dummy;
} Character_t;

typedef Character_t* Character;
typedef int CharacterActionType;

#define CHARACTER_ACTION_TYPE_NONE 0
#define CHARACTER_ACTION_TYPE_MISS 1
#define CHARACTER_ACTION_TYPE_SING 2
#define CHARACTER_ACTION_TYPE_EXTRA 3
#define CHARACTER_ACTION_TYPE_IDLE 4

void character_use_alternate_sing_animations(Character character, bool enable);
void character_set_draw_location(Character character, float x, float y);
void character_set_draw_align(Character character, Align align_vertical, Align align_horizontal);
void character_update_reference_size(Character character, float width, float height);
void character_enable_reference_size(Character character, bool enable);
void character_set_offset(Character character, float offset_x, float offset_y);
int32_t character_state_add(Character character, ModelHolder modelholder, const char* state_name);
int32_t character_state_toggle(Character character, const char* state_name);
bool character_play_hey(Character character);
bool character_play_idle(Character character);
bool character_play_sing(Character character, const char* direction, bool prefer_sustain);
bool character_play_miss(Character character, const char* direction, bool keep_in_hold);
bool character_play_extra(Character character, const char* extra_animation_name, bool prefer_sustain);
void character_set_idle_speed(Character character, float speed);
void character_set_scale(Character character, float scale_factor);
void character_reset(Character character);
void character_enable_continuous_idle(Character character, bool enable);
bool character_is_idle_active(Character character);
void character_enable_flip_correction(Character character, bool enable);
void character_flip_orientation(Character character, bool enable);
void character_face_as_opponent(Character character, bool face_as_opponent);
void character_set_z_index(Character character, float z);
void character_set_z_offset(Character character, float z_offset);
void character_animation_set(Character character, const AnimSprite animsprite);
void character_animation_restart(Character character);
void character_animation_end(Character character);
void character_set_color_offset(Character character, float r, float g, float b, float a);
void character_set_color_offset_to_default(Character character);
void character_set_alpha(Character character, float alpha);
void character_set_visible(Character character, bool visible);
Modifier character_get_modifier(Character character);
bool character_has_direction(Character character, const char* name, bool is_extra);
int32_t character_get_play_calls(Character character);
CharacterActionType character_get_current_action(Character character);

#endif

