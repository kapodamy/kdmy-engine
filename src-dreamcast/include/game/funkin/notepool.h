#ifndef _notepool_h
#define _notepool_h

#include <stdbool.h>
#include <stdint.h>

#include "note_types.h"
#include "notepool_types.h"
#include "strums_types.h"


NotePool notepool_init(DistributionNote* notes, int32_t notes_size, float dimmen, float invdimmen, ScrollDirection scroll_direction);
void notepool_destroy(NotePool* notepool);
void notepool_change_scroll_direction(NotePool notepool, ScrollDirection scroll_direction);
void notepool_change_alpha_alone(NotePool notepool, float alpha);
void notepool_change_alpha_sustain(NotePool notepool, float alpha);
void notepool_change_alpha(NotePool notepool, float alpha);

int32_t notepool_add_state(NotePool notepool, ModelHolder modelholder, const char* state_name);

#endif