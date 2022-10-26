#ifndef __luascript_h
#define __luascript_h

#include <stdbool.h>
#include <stdint.h>

#include "linkedlist.h"

typedef struct _Luascript_t {
    void* L;
    Linkedlist shared;
    Linkedlist allocated;
    void* context;
} *Luascript;

Luascript luascript_init(const char* lua_sourcecode, const char* filename, void* context, bool is_week);
void luascript_destroy(Luascript* luascript);
void luascript_drop_shared(Luascript luascript);

void luascript_notify_weekinit(Luascript luascript, bool from_restart);
void luascript_notify_beforeready(Luascript luascript, bool from_retry);
void luascript_notify_ready(Luascript luascript);
void luascript_notify_aftercountdown(Luascript luascript);
void luascript_notify_frame(Luascript luascript, float elapsed);
void luascript_notify_roundend(Luascript luascript, bool loose);
void luascript_notify_weekend(Luascript luascript, bool giveup);
void luascript_notify_diedecision(Luascript luascript, bool retry_or_giveup);
void luascript_notify_pause(Luascript luascript, bool pause_or_resume);
void luascript_notify_weekleave(Luascript luascript);
void luascript_notify_afterresults(Luascript luascript);
void luascript_notify_scriptchange(Luascript luascript);
void luascript_notify_pause_optionselected(Luascript luascript, int32_t option_index, uint32_t buttons);
void luascript_notify_pause_menuvisible(Luascript luascript, bool shown_or_hidden);
void luascript_notify_note(Luascript luascript, double timestamp, int32_t id, double duration, double data, bool special, int32_t player_id, uint32_t state);
void luascript_notify_buttons(Luascript luascript, int32_t player_id, uint32_t buttons);
void luascript_notify_unknownnote(Luascript luascript, int32_t player_id, double timestamp, int32_t id, double duration, double data);
void luascript_notify_beat(Luascript luascript, int32_t current_beat, float since);
void luascript_notify_quarter(Luascript luascript, int32_t current_quarter, float since);
void luascript_notify_timer_run(Luascript luascript, double timestamp);
#endif

