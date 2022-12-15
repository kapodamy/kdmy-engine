#ifndef __countdown_h
#define __countdown_h

#include "drawable.h"
#include "tweenkeyframe.h"


typedef struct {
    int dummy;
} Countdown_t;
typedef Countdown_t* Countdown;


void countdown_set_default_animation2(Countdown countdown, TweenKeyframe tweenkeyframe);
void countdown_set_bpm(Countdown countdown, float bpm);
Drawable countdown_get_drawable(Countdown countdown);
bool countdown_ready(Countdown countdown);
bool countdown_start(Countdown countdown);
bool countdown_has_ended(Countdown countdown);

#endif
