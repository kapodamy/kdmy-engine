#ifndef _character_h
#define _character_h

#include <stdbool.h>
#include <stdint.h>

#include "character_types.h"

#include "drawable_types.h"
#include "modifier.h"
#include "nbool.h"
#include "vertexprops_types.h"

//
//  forward references
//

typedef struct ModelHolder_s* ModelHolder;
typedef struct AnimSprite_s* AnimSprite;
typedef struct CharacterManifest_s* CharacterManifest;


Character character_init(CharacterManifest charactermanifest);
void character_destroy(Character* character);

void character_use_alternate_sing_animations(Character character, bool enable);
void character_set_draw_location(Character character, float x, float y);
void character_set_draw_align(Character character, Align align_vertical, Align align_horizontal);
void character_update_reference_size(Character character, float width, float height);
void character_enable_reference_size(Character character, bool enable);
void character_set_offset(Character character, float offset_x, float offset_y);

bool character_state_add(Character character, ModelHolder modelholder, const char* state_name);
bool character_state_toggle(Character character, const char* state_name);

bool character_play_hey(Character character);
int32_t character_play_idle(Character character);
bool character_play_sing(Character character, const char* direction, bool prefer_sustain);
bool character_play_miss(Character character, const char* direction, bool keep_in_hold);
bool character_play_extra(Character character, const char* extra_animation_name, bool prefer_sustain);
void character_schedule_idle(Character character);

void character_set_bpm(Character character, float beats_per_minute);
void character_set_idle_speed(Character character, float speed);
void character_set_scale(Character character, float scale_factor);
void character_reset(Character character);
void character_enable_continuous_idle(Character character, bool enable);
bool character_is_idle_active(Character character);
void character_enable_flip_correction(Character character, bool enable);
void character_flip_orientation(Character character, bool enable);
void character_face_as_opponent(Character character, bool face_as_opponent);

int32_t character_animate(Character character, float elapsed);
void character_draw(Character character, PVRContext pvrctx);

void character_set_z_index(Character character, float z);
void character_set_z_offset(Character character, float z);
void character_animation_set(Character character, AnimSprite animsprite);
void character_animation_restart(Character character);
void character_animation_end(Character character);
void character_set_color_offset(Character character, float r, float g, float b, float a);
void character_set_color_offset_to_default(Character character);
void character_set_alpha(Character character, float alpha);
void character_set_visible(Character character, bool visible);
Modifier* character_get_modifier(Character character);
Drawable character_get_drawable(Character character);
int32_t character_get_play_calls(Character character);
int32_t character_get_commited_animations_count(Character character);
CharacterActionType character_get_current_action(Character character);
bool character_has_direction(Character character, const char* name, bool is_extra);
void character_freeze_animation(Character character, bool enabled);
void character_trailing_enabled(Character character, bool enabled);
void character_trailing_set_params(Character character, int32_t length, float trail_delay, float trail_alpha, nbool darken_colors);
void character_trailing_set_offsetcolor(Character character, float r, float g, float b);

#endif
