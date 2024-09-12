#ifndef _weekround_h
#define _weekround_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "float64.h"

//
//  forward references
//

typedef struct SongPlayer_s* SongPlayer;
typedef struct Character_s* Character;
typedef struct MessageBox_s* MessageBox;
typedef struct TextSprite_s* TextSprite;
typedef struct Dialogue_s* Dialogue;
typedef struct Conductor_s* Conductor;
typedef struct HealthWatcher_s* HealthWatcher;
typedef struct MissNoteFX_s* MissNoteFX;
typedef struct PlayerStats_s* PlayerStats;
typedef struct Countdown_s* Countdown;
typedef struct Healthbar_s* Healthbar;
typedef struct RankingCounter_s* RankingCounter;
typedef struct RoundStats_s* RoundStats;
typedef struct SongProgressbar_s* SongProgressbar;
typedef struct StreakCounter_s* StreakCounter;
typedef struct Strums_s* Strums;
typedef struct Layout_s* Layout;
typedef struct Camera_s* Camera;
typedef struct WeekInfo_s WeekInfo;
typedef struct WeekResult_Stats_s WeekResult_Stats;


typedef struct RoundContext_s* RoundContext;


extern const char* WEEKROUND_CAMERA_PLAYER;
extern const char* WEEKROUND_CAMERA_OPONNENT;
extern const char* WEEKROUND_CHARACTER_PREFIX;

int32_t week_main(WeekInfo* weekinfo, bool alt_tracks, const char* difficult, const char* default_bf, const char* default_gf, const char* gameplaymanifest_src, int32_t single_song_index, const char* exit_to_weekselector_label);

void week_update_bpm(RoundContext roundcontext, float bpm);
void week_update_speed(RoundContext roundcontext, float64 speed);
void week_disable_layout_rollback(RoundContext roundcontext, bool disable);
void week_override_common_folder(RoundContext roundcontext, const char* custom_common_path);
void week_halt(RoundContext roundcontext, bool peek_global_beatwatcher);
void week_unlockdirective_create(RoundContext roundcontext, const char* name, bool completed_round, bool completed_week, float64 value);
void week_unlockdirective_remove(RoundContext roundcontext, const char* name, bool completed_round, bool completed_week);
float64 week_unlockdirective_get(RoundContext roundcontext, const char* name);
bool week_unlockdirective_has(RoundContext roundcontext, const char* name);
size_t week_storage_get(RoundContext roundcontext, const char* name, uint8_t** out_data);
bool week_storage_set(RoundContext roundcontext, const char* name, const uint8_t* data, size_t data_size);
SongPlayer week_get_songplayer(RoundContext roundcontext);
Layout week_get_stage_layout(RoundContext roundcontext);
Layout week_ui_get_layout(RoundContext roundcontext);
Camera week_ui_get_camera(RoundContext roundcontext);
void week_set_halt(RoundContext roundcontext, bool halt);
void week_disable_week_end_results(RoundContext roundcontext, bool disable);
void week_disable_girlfriend_cry(RoundContext roundcontext, bool disable);
void week_disable_ask_ready(RoundContext roundcontext, bool disable);
void week_disable_countdown(RoundContext roundcontext, bool disable);
void week_disable_camera_bumping(RoundContext roundcontext, bool disable);
Character week_get_girlfriend(RoundContext roundcontext);
int32_t week_get_character_count(RoundContext roundcontext);
Character week_get_character(RoundContext roundcontext, int32_t index);
MessageBox week_get_messagebox(RoundContext roundcontext);
int32_t week_ui_get_strums_count(RoundContext roundcontext);
TextSprite week_ui_get_round_textsprite(RoundContext roundcontext);
void week_ui_set_visibility(RoundContext roundcontext, bool visible);
void week_get_current_chart_info(RoundContext roundcontext, float* bpm, float64* speed);
void week_get_current_song_info(RoundContext roundcontext, const char** name, const char** difficult, int32_t* index);
void week_change_character_camera_name(RoundContext roundcontext, bool opponent_or_player, const char* new_name);
void week_enable_credits_on_completed(RoundContext roundcontext);
void week_end(RoundContext roundcontext, bool round_or_week, bool loose_or_win);
Dialogue week_get_dialogue(RoundContext roundcontext);
Conductor week_get_conductor(RoundContext roundcontext, int32_t character_index);
HealthWatcher week_get_healthwatcher(RoundContext roundcontext);
MissNoteFX week_get_missnotefx(RoundContext roundcontext);
PlayerStats week_get_playerstats(RoundContext roundcontext, int32_t character_index);
void week_rebuild_ui(RoundContext roundcontext);
Countdown week_ui_get_countdown(RoundContext roundcontext);
Healthbar week_ui_get_healthbar(RoundContext roundcontext);
RankingCounter week_ui_get_rankingcounter(RoundContext roundcontext);
RoundStats week_ui_get_roundstats(RoundContext roundcontext);
SongProgressbar week_ui_get_songprogressbar(RoundContext roundcontext);
StreakCounter week_ui_get_streakcounter(RoundContext roundcontext);
Strums week_ui_get_strums(RoundContext roundcontext, int32_t strums_id);
void week_set_gameover_option(RoundContext roundcontext, int32_t opt, float nro, const char* str);
void week_get_accumulated_stats(RoundContext roundcontext, WeekResult_Stats* stats);
Layout week_get_layout_of(RoundContext roundcontext, char g_p_r);
void week_set_pause_background_music(RoundContext roundcontext, const char* filename);

char* week_internal_concat_suffix(const char* name, int32_t number_suffix);
void week_camera_focus_guess(RoundContext roundcontext, const char* target_name, int32_t character_index);



#endif
