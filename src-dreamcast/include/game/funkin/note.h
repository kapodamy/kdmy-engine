#ifndef _note_h
#define _note_h

#include <stdbool.h>
#include <stdint.h>

#include "float64.h"
#include "pvrcontext_types.h"

#include "note_types.h"


Note note_init(const char* name, float dimmen, float invdimmen);
void note_destroy(Note* note);
void note_set_scoll_direction(Note note, ScrollDirection direction);
void note_set_alpha(Note note, float alpha);
void note_set_alpha_alone(Note note, float alpha);
void note_set_alpha_sustain(Note note, float alpha);
int32_t note_animate(Note note, float elapsed);
const char* note_get_name(Note note);
void note_draw(Note note, PVRContext pvrctx, float64 scroll_velocity, float64 x, float64 y, float64 duration_ms, bool only_body);

bool note_state_add(Note note, ModelHolder modelholder, const char* state_name);
int32_t note_state_toggle(Note note, const char* state_name);
StateSprite note_peek_alone_statesprite(Note note);



#endif
