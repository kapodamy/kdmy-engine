#ifndef _countdown_h
#define _countdown_h

#include <stdbool.h>
#include <stdint.h>

#include "drawable_types.h"
#include "pvrcontext_types.h"


//
//  forward references
//

typedef struct AnimList_s* AnimList;
typedef struct TweenKeyframe_s* TweenKeyframe;
typedef struct ModelHolder_s* ModelHolder;


typedef struct Countdown_s* Countdown;


Countdown countdown_init(ModelHolder modelholder, float height);
void countdown_destroy(Countdown* countdown_ptr);

void countdown_set_layout_viewport(Countdown countdown, float width, float height);
void countdown_set_bpm(Countdown countdown, float bpm);
void countdown_set_default_animation(Countdown countdown, AnimList animlist);
void countdown_set_default_animation2(Countdown countdown, TweenKeyframe tweenkeyframe);

Drawable countdown_get_drawable(Countdown countdown);

bool countdown_ready(Countdown countdown);
bool countdown_start(Countdown countdown);
bool countdown_has_ended(Countdown countdown);

int32_t countdown_animate(Countdown countdown, float elapsed);
void countdown_draw(Countdown countdown, PVRContext pvrctx);

#endif
