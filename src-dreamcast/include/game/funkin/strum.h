#ifndef _strum_h
#define _strum_h

#include "animsprite.h"
#include "drawable.h"
#include "float64.h"
#include "game/ddrkeymon.h"
#include "game/gameplay/helpers/chart.h"
#include "game/gameplay/weekscript.h"
#include "modelholder.h"
#include "statesprite.h"
#include "strums.h"
#include "tweenkeyframe.h"

#include "note_types.h"
#include "notepool_types.h"
#include "strum_types.h"
#include "strums_types.h"


Strum strum_init(int32_t id, const char* name, float x, float y, float marker_dimmen, float invdimmen, float length_dimmen, bool keep_markers_scale);
void strum_destroy(Strum* strum_ptr);
int32_t strum_set_notes(Strum strum, Chart chart, DistributionStrum* strumsdefs, int32_t strumsdefs_size, int32_t player_id, NotePool notepool);
void strum_force_rebuild(Strum strum, float marker_dimmen, float invdimmen, float length_dimmen, bool keep_markers_scale);
void strum_update_draw_location(Strum strum, float x, float y);
void strum_set_scroll_speed(Strum strum, float64 speed);
void strum_set_scroll_direction(Strum strum, ScrollDirection direction);
void strum_set_marker_duration_multiplier(Strum strum, float multipler);
void strum_reset(Strum strum, float64 scroll_speed, const char* state_name);
int32_t strum_scroll(Strum strum, float64 song_timestamp, DDRKeysFIFO* ddrkeys_fifo, PlayerStats playerstats, WeekScript weekscript);
void strum_scroll_auto(Strum strum, float64 song_timestamp, PlayerStats playerstats, WeekScript weekscript);
void strum_force_key_release(Strum strum);
int32_t strum_find_penalties_note_hit(Strum strum, float64 song_timestamp, DDRKeysFIFO* ddrkeys_fifo, PlayerStats playerstats, WeekScript weekscript);
int32_t strum_find_penalties_empty_hit(Strum strum, float64 song_timestamp, DDRKeysFIFO* ddrkeys_fifo, PlayerStats playerstats);

int32_t strum_get_press_state_changes(Strum strum);
StrumPressState strum_get_press_state(Strum strum);
bool strum_get_press_state_use_alt_anim(Strum strum);
const char* strum_get_name(Strum strum);
float strum_get_marker_duration(Strum strum);
void strum_use_funkin_maker_duration(Strum strum, bool enable);
void strum_set_player_id(Strum strum, int32_t player_id);

int32_t strum_animate(Strum strum, float elapsed);
void strum_draw(Strum strum, PVRContext pvrctx);

void strum_enable_background(Strum strum, bool enable);
void strum_enable_sick_effect(Strum strum, bool enable);
void strum_state_add(Strum strum, ModelHolder mdlhldr_mrkr, ModelHolder mdlhldr_sck_ffct, ModelHolder mdlhldr_bckgrnd, const char* state_name);
int32_t strum_state_toggle(Strum strum, const char* state_name);
bool strum_state_toggle_notes(Strum strum, const char* state_name);
bool strum_state_toggle_sick_effect(Strum strum, const char* state_name);
int32_t strum_state_toggle_marker(Strum strum, const char* state_name);
bool strum_state_toggle_background(Strum strum, const char* state_name);

float strum_set_alpha_background(Strum strum, float alpha);
float strum_set_alpha_sick_effect(Strum strum, float alpha);
void strum_set_keep_aspect_ratio_background(Strum strum, bool enable);
StateSprite strum_draw_sick_effect_apart(Strum strum, bool enable);

void strum_set_extra_animation(Strum strum, StrumScriptTarget strum_script_target, StrumScriptOn strum_script_on, bool undo, AnimSprite animsprite);
void strum_set_extra_animation_continuous(Strum strum, StrumScriptTarget strum_script_target, AnimSprite animsprite);

void strum_set_note_tweenkeyframe(Strum strum, TweenKeyframe tweenkeyframe);
void strum_set_sickeffect_size_ratio(Strum strum, float size_ratio);

void strum_set_offsetcolor(Strum strum, float r, float g, float b, float a);
void strum_set_alpha(Strum strum, float alpha);
void strum_set_visible(Strum strum, bool visible);
void strum_set_draw_offset(Strum strum, float64 offset_milliseconds);
void strum_set_bpm(Strum strum, float beats_per_minute);
void strum_disable_beat_synced_idle_and_continous(Strum strum, bool disabled);
Modifier* strum_get_modifier(Strum strum);
Drawable strum_get_drawable(Strum strum);
float64 strum_get_duration(Strum strum);
void strum_animation_restart(Strum strum);
void strum_animation_end(Strum strum);

#endif
