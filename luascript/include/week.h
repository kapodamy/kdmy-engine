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
#include "strums.h"
#include "roundstats.h"
#include "streakcounter.h"
#include "songprogressbar.h"
#include "rankingcounter.h"
#include "countdown.h"
#include "healthbar.h"
#include "healthwatcher.h"
#include "conductor.h"
#include "missnotefx.h"

typedef struct _RoundContext_t {
    int dummy;
} RoundContext_t;

typedef RoundContext_t* RoundContext;

#define WEEK_GAMEOVEROPTION_NOMUSIC 0
#define WEEK_GAMEOVEROPTION_NOSFXDIE 1
#define WEEK_GAMEOVEROPTION_NOSFXRETRY 2
#define WEEK_GAMEOVEROPTION_ANIMDURATIONDIE 3
#define WEEK_GAMEOVEROPTION_ANIMDURATIONRETRY 4
#define WEEK_GAMEOVEROPTION_ANIMDURATIONGIVEUP 5
#define WEEK_GAMEOVEROPTION_ANIMDURATIONBEFORE 6
#define WEEK_GAMEOVEROPTION_ANIMDURATIONBEFOREFORCEEND 7
#define WEEK_GAMEOVEROPTION_SETMUSIC 8
#define WEEK_GAMEOVEROPTION_SETSFXDIE 9
#define WEEK_GAMEOVEROPTION_SETSFXRETRY 10
typedef int32_t WeekGameOverOption;


void week_unlockdirective_create(RoundContext roundcontext, const char* name, bool completed_round, bool completed_week, double value);
bool week_unlockdirective_has(RoundContext roundcontext, const char* name);
void week_unlockdirective_remove(RoundContext roundcontext, const char* name, bool completed_round, bool completed_week);
double week_unlockdirective_get(RoundContext roundcontext, const char* name);
bool week_storage_set(RoundContext roundcontext, const char* name, const uint8_t* data, uint32_t data_size);
uint32_t week_storage_get(RoundContext roundcontext, const char* name, uint8_t** data);
void week_ui_set_visibility(RoundContext roundcontext, bool visible);
Layout week_ui_get_layout(RoundContext roundcontext);
Camera week_ui_get_camera(RoundContext roundcontext);

void week_set_halt(RoundContext roundcontext, bool halt);
void week_disable_week_end_results(RoundContext roundcontext, bool disable);
void week_disable_girlfriend_cry(RoundContext roundcontext, bool disable);
void week_disable_ask_ready(RoundContext roundcontext, bool disable);
void week_disable_countdown(RoundContext roundcontext, bool disable);
void week_disable_camera_bumping(RoundContext roundcontext, bool disable);
int32_t week_ui_get_strums_count(RoundContext roundcontext);
Strums week_ui_get_strums(RoundContext roundcontext, int32_t strums_id);
RoundStats week_ui_get_roundstats(RoundContext roundcontext);
RankingCounter week_ui_get_rankingcounter(RoundContext roundcontext);
StreakCounter week_ui_get_streakcounter(RoundContext roundcontext);
TextSprite week_ui_get_round_textsprite(RoundContext roundcontext);
SongProgressbar week_ui_get_songprogressbar(RoundContext roundcontext);
Countdown week_ui_get_countdown(RoundContext roundcontext);
HealthBar week_ui_get_healthbar(RoundContext roundcontext);
Layout week_get_stage_layout(RoundContext roundcontext);
HealthWatcher week_get_healthwatcher(RoundContext roundcontext);
MissNoteFX week_get_missnotefx(RoundContext roundcontext);
void week_update_bpm(RoundContext roundcontext, float bpm);
void week_update_speed(RoundContext roundcontext, float speed);
MessageBox week_get_messagebox(RoundContext roundcontext);
Character week_get_girlfriend(RoundContext roundcontext);
int32_t week_get_character_count(RoundContext roundcontext);
Conductor week_get_conductor(RoundContext roundcontext, int32_t character_index);
Character week_get_character(RoundContext roundcontext, int32_t character_index);
PlayerStats week_get_playerstats(RoundContext roundcontext, int32_t character_index);
SongPlayer week_get_songplayer(RoundContext roundcontext);
void week_get_current_chart_info(RoundContext roundcontext, float* bpm, double* speed);
void week_get_current_song_info(RoundContext roundcontext, const char** name, const char** difficult, int32_t* index);
void week_change_character_camera_name(RoundContext roundcontext, bool opponent_or_player, const char* new_name);
void week_disable_layout_rollback(RoundContext roundcontext, bool disable);
void week_override_common_folder(RoundContext roundcontext, const char* custom_common_path);
void week_enable_credits_on_completed(RoundContext roundcontext);
void week_end(RoundContext roundcontext, bool round_or_week, bool loose_or_win);
Dialogue week_get_dialogue(RoundContext roundcontext);
void week_set_ui_shader(RoundContext roundcontext, PSShader psshader);
void week_rebuild_ui(RoundContext roundcontext);
void week_set_gameover_option(RoundContext roundcontext, WeekGameOverOption opt, float nro, const char* str);

#endif

