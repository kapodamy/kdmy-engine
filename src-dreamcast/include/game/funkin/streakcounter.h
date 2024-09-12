#ifndef _streakcounter_h
#define _streakcounter_h

#include <stdbool.h>
#include <stdint.h>

#include "drawable_types.h"
#include "layout_types.h"
#include "modifier.h"
#include "playerstats_types.h"
#include "pvrcontext_types.h"


//
//  forward references
//

typedef struct ModelHolder_s* ModelHolder;
typedef struct AnimList_s* AnimList;
typedef struct AnimSprite_s* AnimSprite;


typedef struct StreakCounter_s* StreakCounter;


StreakCounter streakcounter_init(LayoutPlaceholder* placeholder, float combo_height, float number_gap, float delay);
void streakcounter_destroy(StreakCounter* streakcounter);

bool streakcounter_peek_streak(StreakCounter streakcounter, PlayerStats playerstats);
void streakcounter_reset(StreakCounter streakcounter);
void streakcounter_hide_combo_sprite(StreakCounter streakcounter, bool hide);
void streakcounter_set_combo_draw_location(StreakCounter streakcounter, float x, float y);

int32_t streakcounter_state_add(StreakCounter streakcounter, ModelHolder combo_modelholder, ModelHolder number_modelholder, const char* state_name);
bool streakcounter_state_toggle(StreakCounter streakcounter, const char* state_name);
void streakcounter_set_alpha(StreakCounter streakcounter, float alpha);
void streakcounter_set_offsetcolor(StreakCounter streakcounter, float r, float g, float b, float a);
void streakcounter_set_offsetcolor_to_default(StreakCounter streakcounter);
Modifier* streakcounter_get_modifier(StreakCounter streakcounter);
Drawable streakcounter_get_drawable(StreakCounter streakcounter);

void streakcounter_set_number_animation(StreakCounter streakcounter, AnimList animlist);
void streakcounter_set_combo_animation(StreakCounter streakcounter, AnimList animlist);

void streakcounter_animation_set(StreakCounter streakcounter, AnimSprite animsprite);
void streakcounter_animation_restart(StreakCounter streakcounter);
void streakcounter_animation_end(StreakCounter streakcounter);

int32_t streakcounter_animate(StreakCounter streakcounter, float elapsed);
void streakcounter_draw(StreakCounter streakcounter, PVRContext pvrctx);

#endif
