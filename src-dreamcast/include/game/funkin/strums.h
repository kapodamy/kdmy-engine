#ifndef _strums_h
#define _strums_h

#include <stdbool.h>
#include <stdint.h>

#include "game/ddrkeymon.h"
#include "game/gameplay/helpers/chart.h"
#include "game/gameplay/weekscript.h"
#include "modifier.h"
#include "pvrcontext_types.h"

#include "note_types.h"
#include "notepool_types.h"
#include "strums_types.h"


//
// buttons binding
//
extern const GamepadButtons BUTTONS_BIND_4[];
extern const GamepadButtons BUTTONS_BIND_5[];
extern const GamepadButtons BUTTONS_BIND_6[];
extern const GamepadButtons BUTTONS_BIND_7[];
extern const GamepadButtons BUTTONS_BIND_8[];
extern const GamepadButtons BUTTONS_BIND_9[];

extern const Distribution STRUMS_DEFAULT_DISTRIBUTION;

Strums strums_init(float x, float y, float z, float dimmen, float invdimmen, float length, float gap, int32_t player_id, bool is_vertical, bool keep_marker_scale, DistributionStrum* strumsdefs, int32_t strumsdefs_size);
void strums_destroy(Strums* strums);
void strums_set_params(Strums strums, DDRKeymon ddrkeymon, PlayerStats playerstats, WeekScript weekscript);
Drawable strums_get_drawable(Strums strums);

void strums_set_notes(Strums strums, Chart chart, DistributionStrum* strumsdefs, int32_t strumsdefs_size, NotePool notepool);
void strums_set_scroll_speed(Strums strums, float64 speed);
void strums_set_scroll_direction(Strums strums, ScrollDirection direction);
void strums_set_marker_duration_multiplier(Strums strums, float multipler);
void strums_set_bpm(Strums strums, float beats_per_minute);
void strums_disable_beat_synced_idle_and_continous(Strums strums, bool disabled);
void strums_reset(Strums strums, float64 scroll_speed, const char* state_name);
int32_t strums_scroll_full(Strums strums, float64 song_timestamp);
void strums_scroll_auto(Strums strums, float64 song_timestamp);
void strums_force_key_release(Strums strums);

int32_t strums_animate(Strums strums, float elapsed);
void strums_draw(Strums strums, PVRContext pvrctx);

float strums_set_alpha(Strums strums, float alpha);
void strums_enable_background(Strums strums, bool enable);
void strums_set_keep_aspect_ratio_background(Strums strums, bool enable);
void strums_set_alpha_background(Strums strums, float alpha);
void strums_set_alpha_sick_effect(Strums strums, float alpha);
void strums_set_draw_offset(Strums strums, float64 offset_milliseconds);

void strums_state_add(Strums strums, ModelHolder mdlhldr_mrkr, ModelHolder mdlhldr_sck_ffct, ModelHolder mdlhldr_bckgrnd, const char* state_name);
int32_t strums_state_toggle(Strums strums, const char* state_name);
int32_t strums_state_toggle_notes(Strums strums, const char* state_name);
void strums_state_toggle_marker_and_sick_effect(Strums strums, const char* state_name);
int32_t strums_get_lines_count(Strums strums);
Strum strums_get_strum_line(Strums strums, int32_t index);
void strums_enable_post_sick_effect_draw(Strums strums, bool enable);
void strums_use_funkin_maker_duration(Strums strums, bool enable);

void strums_set_offsetcolor(Strums strums, float r, float g, float b, float a);
void strums_set_visible(Strums strums, bool visible);
Modifier* strums_get_modifier(Strums strums);

void strums_animation_set(Strums strums, AnimSprite animsprite);
void strums_animation_restart(Strums strums);
void strums_animation_end(Strums strums);

int32_t strums_decorators_get_count(Strums strums);
bool strums_decorators_add(Strums strums, ModelHolder modelholder, const char* animation_name, float64 timestamp);
bool strums_decorators_add2(Strums strums, ModelHolder modelholder, const char* animation_name, float64 timestamp, int32_t from_strum_index, int32_t to_strum_index);
void strums_decorators_set_scroll_speed(Strums strums, float64 speed);
void strums_decorators_set_alpha(Strums strums, float alpha);
void strums_decorators_set_visible(Strums strums, float64 decorator_timestamp, bool visible);

void strums_force_rebuild(Strums strums, float x, float y, float z, float dimmen, float invdimmen, float length_dimmen, float gap, bool is_vertical, bool keep_markers_scale);

#endif