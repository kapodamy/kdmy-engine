#ifndef __animsprite_h
#define __animsprite_h

#include "animlist.h"
#include "atlas.h"


typedef struct _AnimSprite_t {
    int dummy;
} AnimSprite_t;

typedef AnimSprite_t* AnimSprite;

AnimSprite animsprite_init_from_atlas(float frame_rate, int32_t loop, Atlas atlas, const char* prefix, bool has_number_suffix);
AnimSprite animsprite_init_from_animlist(AnimList animlist, const char* animation_name);
AnimSprite animsprite_init_as_empty(const char* name);
AnimSprite animsprite_init(AnimListItem animlist_item);
void animsprite_destroy(AnimSprite* animsprite);
void animsprite_set_loop(AnimSprite animsprite, int32_t loop);
void animsprite_restart(AnimSprite animsprite);
const char* animsprite_get_name(AnimSprite animsprite);
bool animsprite_is_frame_animation(AnimSprite animsprite);
void animsprite_set_delay(AnimSprite animsprite, float delay_milliseconds);

#endif
