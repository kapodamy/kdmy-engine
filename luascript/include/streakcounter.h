#ifndef __streakcounter_h
#define __streakcounter_h

#include "drawable.h"
#include "modelholder.h"
#include "playerstats.h"

typedef struct {
    int dummy;
} StreakCounter_t;
typedef StreakCounter_t* StreakCounter;


void streakcounter_reset(StreakCounter streakcounter);
void streakcounter_hide_combo_sprite(StreakCounter streakcounter, bool hide);
void streakcounter_set_combo_draw_location(StreakCounter streakcounter, float x, float y);
int32_t streakcounter_state_add(StreakCounter streakcounter, ModelHolder combo_modelholder, ModelHolder number_modelholder, const char* state_name);
bool streakcounter_state_toggle(StreakCounter streakcounter, const char* state_name);
void streakcounter_set_alpha(StreakCounter streakcounter, float alpha);
Drawable streakcounter_get_drawable(StreakCounter streakcounter);
void streakcounter_animation_set(StreakCounter streakcounter, AnimSprite animsprite);
void streakcounter_animation_restart(StreakCounter streakcounter);
void streakcounter_animation_end(StreakCounter streakcounter);

#endif
