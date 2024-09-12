#ifndef _streakcounter_digits_h
#define _streakcounter_digits_h

#include <stdbool.h>
#include <stdint.h>

#include "animlist.h"
#include "pvrcontext_types.h"


extern const char* STREAKCOUNTER_UI_STREAK_ANIM;
extern const int32_t STREAKCOUNTER_MAX_VALUE;


//
//  forward references
//

typedef struct ModelHolder_s* ModelHolder;
typedef struct AnimSprite_s* AnimSprite;


typedef struct StreakCounterDigits_s* StreakCounterDigits;

StreakCounterDigits streakcounter_digits_init(float delay);
void streakcounter_digits_destroy(StreakCounterDigits* streakcounter_digits);

int32_t streakcounter_digits_state_add(StreakCounterDigits streakcounter_digits, float max_hght, ModelHolder mdlhldr, const char* state_name);
void streakcounter_digits_state_toggle(StreakCounterDigits streakcounter_digits, const char* state_name);
void streakcounter_digits_animation_restart(StreakCounterDigits streakcounter_digits);
void streakcounter_digits_animation_end(StreakCounterDigits streakcounter_digits);

void streakcounter_digits_animation_set(StreakCounterDigits streakcounter_digits, const AnimListItem* animlist_item);
void streakcounter_digits_animation_pair_set(StreakCounterDigits streakcounter_digits, AnimSprite animsprite_to_clone);

int32_t streakcounter_digits_animate(StreakCounterDigits streakcounter_digits, float elapsed);
float streakcounter_digits_measure(StreakCounterDigits streakcounter_digits, int32_t value);
void streakcounter_digits_set_draw_location(StreakCounterDigits streakcounter_digits, float x, float y, float gap);
void streakcounter_digits_draw(StreakCounterDigits streakcounter_digits, PVRContext pvrctx);

#endif
