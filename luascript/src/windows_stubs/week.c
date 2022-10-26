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
static char* stub_track_name = "°°test track°°";
static char* stub_track_difficult = "NORMAL";

void week_unlockdirective_create(RoundContext roundcontext, const char* name, bool completed_round, bool completed_week, int64_t value) {
    print_stub("week_unlockdirective_create", "roundcontext=%p name=%s completed_round=(bool)%i completed_week=(bool)%i value=%li", roundcontext, name, completed_round, completed_week, value);
}
void week_unlockdirective_remove(RoundContext roundcontext, const char* name, bool completed_round, bool completed_week) {
    print_stub("week_unlockdirective_remove", "roundcontext=%p name=%s completed_round=(bool)%i completed_week=(bool)%i", roundcontext, name, completed_round, completed_week);
}
bool week_unlockdirective_has(RoundContext roundcontext, const char* name) {
    print_stub("week_unlockdirective_has", "roundcontext=%p name=%s", roundcontext, name);
    return 111;
}
int64_t week_unlockdirective_get(RoundContext roundcontext, const char* name) {
    print_stub("week_unlockdirective_get", "roundcontext=%p name=%s", roundcontext, name);
    return 123456789;
}
void week_set_halt(RoundContext roundcontext, bool halt) {
    print_stub("week_set_halt", "roundcontext=%p halt=(bool)%i", roundcontext, halt);
}
void week_ui_set_visibility(RoundContext roundcontext, bool visible) {
    print_stub("week_ui_set_visibility", "roundcontext=%p visible=(bool)%i", roundcontext, visible);
}
Camera week_ui_get_camera(RoundContext roundcontext) {
    print_stub("week_ui_get_camera", "roundcontext=%p", roundcontext);
    return &stub_camera;
}
Layout week_ui_get_layout(RoundContext roundcontext) {
    print_stub("week_ui_get_layout", "roundcontext=%p", roundcontext);
    return &stub_layout1;
}
Layout week_get_stage_layout(RoundContext roundcontext) {
    print_stub("week_get_stage_layout", "roundcontext=%p", roundcontext);
    return &stub_layout2;
}
int32_t week_ui_get_strums_count(RoundContext roundcontext) {
    print_stub("week_ui_get_strums_count", "roundcontext=%p", roundcontext);
    return 456;
}
TextSprite week_ui_get_trackinfo(RoundContext roundcontext) {
    print_stub("week_ui_get_trackinfo", "roundcontext=%p", roundcontext);
    return &stub_textsprite;
}
void week_update_bpm(RoundContext roundcontext, float bpm) {
    print_stub("week_update_bpm", "roundcontext=%p bpm=%f", roundcontext, bpm);
}
void week_update_speed(RoundContext roundcontext, double speed) {
    print_stub("week_update_speed", "roundcontext=%p speed=%f", roundcontext, speed);
}
Messagebox week_ui_get_messagebox(RoundContext roundcontext) {
    print_stub("week_ui_get_messagebox", "roundcontext=%p", roundcontext);
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
Character week_get_character(RoundContext roundcontext, int32_t index) {
    print_stub("week_get_character", "roundcontext=%p index=%i", roundcontext, index);
    if (index == 0) return &stub_character2;
    if (index == 1) return &stub_character3;
    return NULL;
}
SongPlayer week_get_songplayer(RoundContext roundcontext) {
    print_stub("week_get_songplayer", "roundcontext=%p", roundcontext);
    return &stub_songplayer;
}
void week_get_current_chart_info(RoundContext roundcontext, WeekChartInfo_t* output_info) {
    output_info->bpm = 123.0f;
    output_info->speed = 456.0;
    print_stub("week_get_current_chart_info", "roundcontext=%p output_info=%p", roundcontext, output_info);
}
void week_get_current_track_info(RoundContext roundcontext, WeekTrackInfo_t* output_info) {
    output_info->index = 123;
    output_info->name = stub_track_name;
    output_info->difficult = stub_track_difficult;
    print_stub("week_get_current_track_info", "roundcontext=%p output_info=%p", roundcontext, output_info);
}
void week_change_charecter_camera_name(RoundContext roundcontext, bool opponent_or_player, const char* new_name) {
    print_stub("week_change_charecter_camera_name", "roundcontext=%p opponent_or_player=(bool)%i new_name=%s", roundcontext, opponent_or_player, new_name);
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

