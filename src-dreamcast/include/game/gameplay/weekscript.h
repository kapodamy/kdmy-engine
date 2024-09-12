#ifndef _weekscript_h
#define _weekscript_h

#include <stdbool.h>
#include <stdint.h>

#include "float64.h"
#include "game/funkin/playerstats.h"
#include "gamepad.h"

//
// Forward references
//

typedef struct PlayerStats_s* PlayerStats;
typedef struct Strum_s* Strum;
typedef struct Luascript_s* Luascript;

typedef struct Luascript_s* WeekScript;


WeekScript weekscript_init(const char* src, void* context, bool is_week);

// WeekScript weekscript_init2(const char* lua_sourcecode, void* context, bool is_week);

void weekscript_destroy(WeekScript* script);


void weekscript_notify_weekinit(WeekScript script, int32_t freeplay_index);
void weekscript_notify_beforeready(WeekScript script, bool from_retry);
void weekscript_notify_ready(WeekScript script);

void weekscript_notify_aftercountdown(WeekScript script);
void weekscript_notify_frame(WeekScript script, float elapsed);
void weekscript_notify_timersong(WeekScript script, float64 song_timestamp);
void weekscript_notify_unknownnote(WeekScript script, int32_t player_id, float64 timestamp, int32_t direction, float64 duration, float64 data);
void weekscript_notify_roundend(WeekScript script, bool loose);
void weekscript_notify_weekend(WeekScript script, bool giveup);
void weekscript_notify_gameoverloop(WeekScript script);
void weekscript_notify_gameoverdecision(WeekScript script, bool retry_or_giveup, const char* changed_difficult);
void weekscript_notify_gameoverended(WeekScript script);
void weekscript_notify_pause(WeekScript script, bool pause_or_resume);
void weekscript_notify_weekleave(WeekScript script);
void weekscript_notify_beforeresults(WeekScript script);
void weekscript_notify_afterresults(WeekScript script, int32_t total_attempts, int32_t songs_count, bool reject_completed);
void weekscript_notify_scriptchange(WeekScript script);
void weekscript_notify_pause_optionchoosen(WeekScript script, int32_t option_index);
void weekscript_notify_pause_menuvisible(WeekScript script, bool shown_or_hidden);
void weekscript_notify_beat(WeekScript script, int32_t beat_count, float since);
void weekscript_notify_quarter(WeekScript script, int32_t quarter_count, float since);
void weekscript_notify_buttons(WeekScript script, int32_t player_id, GamepadButtons buttons);
void weekscript_notify_after_strum_scroll(WeekScript script);
void weekscript_notify_dialogue_builtin_open(WeekScript script, const char* dialog_src);
void weekscript_notify_note(WeekScript script, float64 timestamp, int32_t id, float64 duration, float64 data, bool special, int32_t player_id, Ranking state);

Luascript weekscript_get_luascript(WeekScript script);

#endif
