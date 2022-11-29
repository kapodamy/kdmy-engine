#ifndef __week_h
#define __week_h

#include <stdint.h>
#include <stdbool.h>

#include "layout.h"
#include "messagebox.h"
#include "character.h"
#include "songplayer.h"
#include "dialogue.h"
#include "psshader.h"

typedef struct _RoundContext_t {
    int dummy;
} RoundContext_t;

typedef RoundContext_t* RoundContext;

typedef struct {
#ifdef JAVASCRIPT
    int32_t _ptrsize;
#endif
    char* name;
    char* difficult;
    int32_t index;
} WeekTrackInfo_t;

typedef struct {
    float bpm;
    double speed;
} WeekChartInfo_t;


void week_unlockdirective_create(RoundContext roundcontext, const char* name, bool completed_round, bool completed_week, double value);
void week_unlockdirective_remove(RoundContext roundcontext, const char* name, bool completed_round, bool completed_week);
bool week_unlockdirective_has(RoundContext roundcontext, const char* name);
double week_unlockdirective_get(RoundContext roundcontext, const char* name);
void week_set_halt(RoundContext roundcontext, bool halt);
void week_ui_set_visibility(RoundContext roundcontext, bool visible);
Camera week_ui_get_camera(RoundContext roundcontext);
Layout week_ui_get_layout(RoundContext roundcontext);
Layout week_get_stage_layout(RoundContext roundcontext);
int32_t week_ui_get_strums_count(RoundContext roundcontext);
//Strums week_ui_get_strums(RoundContext roundcontext, int32_t strums_id);
//Roundstats week_ui_get_roundstats(RoundContext roundcontext);
//Rankingcounter week_ui_get_rankingcounter(RoundContext roundcontext);
//Streakcounter week_ui_get_streakcounter(RoundContext roundcontext);
TextSprite week_ui_get_trackinfo(RoundContext roundcontext);
//SongProgressbar week_ui_get_songprogressbar(RoundContext roundcontext);
void week_update_bpm(RoundContext roundcontext, float bpm);
void week_update_speed(RoundContext roundcontext, double speed);
Messagebox week_ui_get_messagebox(RoundContext roundcontext);
Character week_get_girlfriend(RoundContext roundcontext);
int32_t week_get_character_count(RoundContext roundcontext);
Character week_get_character(RoundContext roundcontext, int32_t index);
SongPlayer week_get_songplayer(RoundContext roundcontext);
void week_get_current_chart_info(RoundContext roundcontext, WeekChartInfo_t* output_info);
void week_get_current_track_info(RoundContext roundcontext, WeekTrackInfo_t* output_info);
void week_change_character_camera_name(RoundContext roundcontext, bool opponent_or_player, const char* new_name);
void week_disable_layout_rollback(RoundContext roundcontext, bool disable);
void week_override_common_folder(RoundContext roundcontext, const char* custom_common_path);
void week_enable_credits_on_completed(RoundContext roundcontext);
void week_end(RoundContext roundcontext, bool round_or_week, bool loose_or_win);
Dialogue week_get_dialogue(RoundContext roundcontext);
void week_set_ui_shader(RoundContext roundcontext, PSShader psshader);
#endif

