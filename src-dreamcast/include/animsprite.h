#ifndef _animsprite_h
#define _animsprite_h

#include "animlist.h"
#include "modifier.h"

//
//  forward references
//

typedef struct Sprite_s* Sprite;
typedef struct StateSprite_s* StateSprite;
typedef struct TextSprite_s* TextSprite;
typedef struct Drawable_s* Drawable;
typedef struct TweenLerp_s* TweenLerp;


typedef struct AnimSprite_s* AnimSprite;


AnimSprite animsprite_init_from_atlas(float frame_rate, int32_t loop, Atlas atlas, const char* prefix, bool has_number_suffix);
AnimSprite animsprite_init_from_atlas_entry(const AtlasEntry* atlas_entry, bool loop_indefinitely, float frame_rate);
AnimSprite animsprite_init_from_animlist(AnimList animlist, const char* animation_name);
AnimSprite animsprite_init_from_tweenlerp(const char* name, int32_t loop, TweenLerp tweenlerp);
AnimSprite animsprite_init_as_empty(const char* name);
AnimSprite animsprite_init(const AnimListItem* animlist_item);
void animsprite_destroy(AnimSprite* animsprite);
AnimSprite animsprite_clone(AnimSprite animsprite);
void animsprite_set_loop(AnimSprite animsprite, int32_t loop);
void animsprite_restart(AnimSprite animsprite);
int32_t animsprite_animate(AnimSprite animsprite, float elapsed);
const char* animsprite_get_name(AnimSprite animsprite);
bool animsprite_is_completed(AnimSprite animsprite);
bool animsprite_is_frame_animation(AnimSprite animsprite);
bool animsprite_has_looped(AnimSprite animsprite);
void animsprite_disable_loop(AnimSprite animsprite);
void animsprite_stop(AnimSprite animsprite);
void animsprite_force_end(AnimSprite animsprite);
void animsprite_force_end2(AnimSprite animsprite, Sprite sprite);
void animsprite_force_end3(AnimSprite animsprite, StateSprite statesprite);
void animsprite_set_delay(AnimSprite animsprite, float delay_milliseconds);
void animsprite_update_sprite(AnimSprite animsprite, Sprite sprite, bool stack_changes);
void animsprite_update_statesprite(AnimSprite animsprite, StateSprite statesprite, bool stack_changes);
void animsprite_update_textsprite(AnimSprite animsprite, TextSprite textsprite, bool stack_changes);
void animsprite_update_modifier(AnimSprite animsprite, Modifier* modifier, bool stack_changes);
void animsprite_update_drawable(AnimSprite animsprite, Drawable drawable, bool stack_changes);
void animsprite_update_using_callback(AnimSprite animsprite, void* private_data, PropertySetter setter_callback, bool stack_changes);
bool animsprite_rollback(AnimSprite animsprite, float elapsed);
const AtlasEntry* animsprite_helper_get_first_frame_atlas_entry(AnimSprite animsprite);
void animsprite_allow_override_sprite_size(AnimSprite animsprite, bool enable);

#endif
