#include "commons.h"
#include "week.h"


static SongPlayer_t stub_songplayer = {};
static Character_t stub_character1 = {};
static Character_t stub_character2 = {};
static Character_t stub_character3 = {};
static Messagebox_t stub_messagebox = {};
static Camera_t stub_camera = {};
static Layout_t stub_layout1 = {};
static Layout_t stub_layout2 = {};
static TextSprite_t stub_textsprite = {};
static Dialogue_t stub_dialogue = {};
static char* stub_track_name = "°°test track°°";
static char* stub_track_difficult = "NORMAL";
static Strums_t stub_strums = {};
static RoundStats_t stub_roundstats = {};
static RankingCounter_t stub_rankingcounter = {};
static StreakCounter_t stub_streakcounter = {};
static SongProgressbar_t stub_songprogressbar = {};
static Countdown_t stub_countdown = {};
static HealthBar_t stub_healthbar = {};
static MissNoteFX_t stub_missnotefx = {};
static Conductor_t stub_conductor1 = {};
static Conductor_t stub_conductor2 = {};
static PlayerStats_t stub_playerstats1 = {};
static PlayerStats_t stub_playerstats2 = {};

void week_unlockdirective_create(RoundContext roundcontext, const char* name, bool completed_round, bool completed_week, double value) {
    print_stub("week_unlockdirective_create", "roundcontext=%p name=%s completed_round=(bool)%i completed_week=(bool)%i value=%f", roundcontext, name, completed_round, completed_week, value);
}
bool week_unlockdirective_has(RoundContext roundcontext, const char* name) {
    print_stub("week_unlockdirective_has", "roundcontext=%p name=%s", roundcontext, name);
    return 1;
}
void week_unlockdirective_remove(RoundContext roundcontext, const char* name, bool completed_round, bool completed_week) {
    print_stub("week_unlockdirective_remove", "roundcontext=%p name=%s completed_round=(bool)%i completed_week=(bool)%i", roundcontext, name, completed_round, completed_week);
}
double week_unlockdirective_get(RoundContext roundcontext, const char* name) {
    print_stub("week_unlockdirective_get", "roundcontext=%p name=%s", roundcontext, name);
    return 123;
}
void week_ui_set_visibility(RoundContext roundcontext, bool visible) {
    print_stub("week_ui_set_visibility", "roundcontext=%p visible=(bool)%i", roundcontext, visible);
}
Layout week_ui_get_layout(RoundContext roundcontext) {
    print_stub("week_ui_get_layout", "roundcontext=%p", roundcontext);
    return &stub_layout1;
}
Camera week_ui_get_camera(RoundContext roundcontext) {
    print_stub("week_ui_get_camera", "roundcontext=%p", roundcontext);
    return &stub_camera;
}
void week_set_halt(RoundContext roundcontext, bool halt) {
    print_stub("week_set_halt", "roundcontext=%p halt=(bool)%i", roundcontext, halt);
}
int32_t week_ui_get_strums_count(RoundContext roundcontext) {
    print_stub("week_ui_get_strums_count", "roundcontext=%p", roundcontext);
    return 1;
}
Strums week_ui_get_strums(RoundContext roundcontext, int32_t strums_id) {
    print_stub("week_ui_get_strums", "roundcontext=%p strums_id=%i", roundcontext, strums_id);
    return &stub_strums;
}
RoundStats week_ui_get_roundstats(RoundContext roundcontext) {
    print_stub("week_ui_get_roundstats", "roundcontext=%p", roundcontext);
    return &stub_roundstats;
}
RankingCounter week_ui_get_rankingcounter(RoundContext roundcontext) {
    print_stub("week_ui_get_rankingcounter", "roundcontext=%p", roundcontext);
    return &stub_rankingcounter;
}
StreakCounter week_ui_get_streakcounter(RoundContext roundcontext) {
    print_stub("week_ui_get_streakcounter", "roundcontext=%p", roundcontext);
    return &stub_streakcounter;
}
TextSprite week_ui_get_trackinfo(RoundContext roundcontext) {
    print_stub("week_ui_get_trackinfo", "roundcontext=%p", roundcontext);
    return NULL;
}
SongProgressbar week_ui_get_songprogressbar(RoundContext roundcontext) {
    print_stub("week_ui_get_songprogressbar", "roundcontext=%p", roundcontext);
    return &stub_songprogressbar;
}
Countdown week_ui_get_countdown(RoundContext roundcontext) {
    print_stub("week_ui_get_countdown", "roundcontext=%p", roundcontext);
    return &stub_countdown;
}
HealthBar week_ui_get_healthbar(RoundContext roundcontext) {
    print_stub("week_ui_get_healthbar", "roundcontext=%p", roundcontext);
    return &stub_healthbar;
}
Layout week_get_stage_layout(RoundContext roundcontext) {
    print_stub("week_get_stage_layout", "roundcontext=%p", roundcontext);
    return &stub_layout2;
}
HealthWatcher week_get_healthwatcher(RoundContext roundcontext) {
    print_stub("week_get_healthwatcher", "roundcontext=%p", roundcontext);
    return NULL;
}
MissNoteFX week_get_missnotefx(RoundContext roundcontext) {
    print_stub("week_get_missnotefx", "roundcontext=%p", roundcontext);
    return &stub_missnotefx;
}
void week_update_bpm(RoundContext roundcontext, float bpm) {
    print_stub("week_update_bpm", "roundcontext=%p bpm=%f", roundcontext, bpm);
}
void week_update_speed(RoundContext roundcontext, float speed) {
    print_stub("week_update_speed", "roundcontext=%p speed=%f", roundcontext, speed);
}
MessageBox week_get_messagebox(RoundContext roundcontext) {
    print_stub("week_get_messagebox", "roundcontext=%p", roundcontext);
    return &stub_messagebox;
}
Character week_get_girlfriend(RoundContext roundcontext) {
    print_stub("week_get_girlfriend", "roundcontext=%p", roundcontext);
    return &stub_character1;
}
int32_t week_get_character_count(RoundContext roundcontext) {
    print_stub("week_get_character_count", "roundcontext=%p", roundcontext);
    return 2;
}
Conductor week_get_conductor(RoundContext roundcontext, int32_t character_index) {
    print_stub("week_get_conductor", "roundcontext=%p character_index=%i", roundcontext, character_index);
    return character_index == 0 ? &stub_conductor1 : &stub_conductor2 ;
}
Character week_get_character(RoundContext roundcontext, int32_t character_index) {
    print_stub("week_get_character", "roundcontext=%p character_index=%i", roundcontext, character_index);
    return character_index == 0 ? &stub_character2 : &stub_character3;
}
PlayerStats week_get_playerstats(RoundContext roundcontext, int32_t character_index) {
    print_stub("week_get_playerstats", "roundcontext=%p character_index=%i", roundcontext, character_index);
    return character_index == 0 ? &stub_playerstats1 : &stub_playerstats2;
}
SongPlayer week_get_songplayer(RoundContext roundcontext) {
    print_stub("week_get_songplayer", "roundcontext=%p", roundcontext);
    return &stub_songplayer;
}
void week_get_current_chart_info(RoundContext roundcontext, float* bpm, double* speed) {
    *bpm = 100;
    *speed = 1;
    print_stub("week_get_current_chart_info", "roundcontext=%p bpm=%p speed=%p", roundcontext, bpm, speed);
}
void week_get_current_track_info(RoundContext roundcontext, const char** name, const char** difficult, int32_t* index) {
    *name = stub_track_name;
    *difficult = stub_track_difficult;
    *index = 1;
    print_stub("week_get_current_track_info", "roundcontext=%p name=%p difficult=%p index=%i", roundcontext, name, difficult, index);
}
void week_change_character_camera_name(RoundContext roundcontext, bool opponent_or_player, const char* new_name) {
    print_stub("week_change_character_camera_name", "roundcontext=%p opponent_or_player=(bool)%i new_name=%s", roundcontext, opponent_or_player, new_name);
}
void week_disable_layout_rollback(RoundContext roundcontext, bool disable) {
    print_stub("week_disable_layout_rollback", "roundcontext=%p disable=(bool)%i", roundcontext, disable);
}
void week_override_common_folder(RoundContext roundcontext, const char* custom_common_path) {
    print_stub("week_override_common_folder", "roundcontext=%p custom_common_path=%s", roundcontext, custom_common_path);
}
void week_enable_credits_on_completed(RoundContext roundcontext) {
    print_stub("week_enable_credits_on_completed", "roundcontext=%p", roundcontext);
}
void week_end(RoundContext roundcontext, bool round_or_week, bool loose_or_win) {
    print_stub("week_end", "roundcontext=%p round_or_week=(bool)%i loose_or_win=(bool)%i", roundcontext, round_or_week, loose_or_win);
}
Dialogue week_get_dialogue(RoundContext roundcontext) {
    print_stub("week_get_dialogue", "roundcontext=%p", roundcontext);
    return &stub_dialogue;
}
void week_set_ui_shader(RoundContext roundcontext, PSShader psshader) {
    print_stub("week_set_ui_shader", "roundcontext=%p psshader=%p", roundcontext, psshader);
}
void week_rebuild_ui(RoundContext roundcontext) {
    print_stub("week_rebuild_ui", "roundcontext=%p", roundcontext);
}

