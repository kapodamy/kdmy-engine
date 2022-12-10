#include "commons.h"
#include "playerstats.h"

Ranking playerstats_add_hit(PlayerStats playerstats, float multiplier, float base_note_duration, double hit_time_difference) {
    print_stub("playerstats_add_hit", "playerstats=%p multiplier=%f base_note_duration=%f hit_time_difference=%f", playerstats, multiplier, base_note_duration, hit_time_difference);
    return 0;
}
void playerstats_add_sustain(PlayerStats playerstats, int32_t quarters, bool is_released) {
    print_stub("playerstats_add_sustain", "playerstats=%p quarters=%i is_released=(bool)%i", playerstats, quarters, is_released);
}
Ranking playerstats_add_sustain_delayed_hit(PlayerStats playerstats, float multiplier, float hit_time_difference) {
    print_stub("playerstats_add_sustain_delayed_hit", "playerstats=%p multiplier=%f hit_time_difference=%f", playerstats, multiplier, hit_time_difference);
    return 0;
}
void playerstats_add_penality(PlayerStats playerstats, bool on_empty_strum) {
    print_stub("playerstats_add_penality", "playerstats=%p on_empty_strum=(bool)%i", playerstats, on_empty_strum);
}
void playerstats_add_miss(PlayerStats playerstats, float multiplier) {
    print_stub("playerstats_add_miss", "playerstats=%p multiplier=%f", playerstats, multiplier);
}
void playerstats_reset(PlayerStats playerstats) {
    print_stub("playerstats_reset", "playerstats=%p", playerstats);
}
void playerstats_reset_notes_per_seconds(PlayerStats playerstats) {
    print_stub("playerstats_reset_notes_per_seconds", "playerstats=%p", playerstats);
}
void playerstats_add_extra_health(PlayerStats playerstats, float multiplier) {
    print_stub("playerstats_add_extra_health", "playerstats=%p multiplier=%f", playerstats, multiplier);
}
void playerstats_enable_penality_on_empty_strum(PlayerStats playerstats, bool enable) {
    print_stub("playerstats_enable_penality_on_empty_strum", "playerstats=%p enable=(bool)%i", playerstats, enable);
}
double playerstats_get_maximum_health(PlayerStats playerstats) {
    print_stub("playerstats_get_maximum_health", "playerstats=%p", playerstats);
    return 0;
}
double playerstats_get_health(PlayerStats playerstats) {
    print_stub("playerstats_get_health", "playerstats=%p", playerstats);
    return 0;
}
double playerstats_get_accuracy(PlayerStats playerstats) {
    print_stub("playerstats_get_accuracy", "playerstats=%p", playerstats);
    return 0;
}
double playerstats_get_last_accuracy(PlayerStats playerstats) {
    print_stub("playerstats_get_last_accuracy", "playerstats=%p", playerstats);
    return 0;
}
Ranking playerstats_get_last_ranking(PlayerStats playerstats) {
    print_stub("playerstats_get_last_ranking", "playerstats=%p", playerstats);
    return 0;
}
double playerstats_get_last_difference(PlayerStats playerstats) {
    print_stub("playerstats_get_last_difference", "playerstats=%p", playerstats);
    return 0;
}
int32_t playerstats_get_combo_streak(PlayerStats playerstats) {
    print_stub("playerstats_get_combo_streak", "playerstats=%p", playerstats);
    return 0;
}
int32_t playerstats_get_highest_combo_streak(PlayerStats playerstats) {
    print_stub("playerstats_get_highest_combo_streak", "playerstats=%p", playerstats);
    return 0;
}
int32_t playerstats_get_combo_breaks(PlayerStats playerstats) {
    print_stub("playerstats_get_combo_breaks", "playerstats=%p", playerstats);
    return 0;
}
int32_t playerstats_get_notes_per_seconds(PlayerStats playerstats) {
    print_stub("playerstats_get_notes_per_seconds", "playerstats=%p", playerstats);
    return 0;
}
int32_t playerstats_get_notes_per_seconds_highest(PlayerStats playerstats) {
    print_stub("playerstats_get_notes_per_seconds_highest", "playerstats=%p", playerstats);
    return 0;
}
int32_t playerstats_get_iterations(PlayerStats playerstats) {
    print_stub("playerstats_get_iterations", "playerstats=%p", playerstats);
    return 0;
}
int64_t playerstats_get_score(PlayerStats playerstats) {
    print_stub("playerstats_get_score", "playerstats=%p", playerstats);
    return 0;
}
int32_t playerstats_get_hits(PlayerStats playerstats) {
    print_stub("playerstats_get_hits", "playerstats=%p", playerstats);
    return 0;
}
int32_t playerstats_get_misses(PlayerStats playerstats) {
    print_stub("playerstats_get_misses", "playerstats=%p", playerstats);
    return 0;
}
int32_t playerstats_get_penalties(PlayerStats playerstats) {
    print_stub("playerstats_get_penalties", "playerstats=%p", playerstats);
    return 0;
}
int32_t playerstats_get_shits(PlayerStats playerstats) {
    print_stub("playerstats_get_shits", "playerstats=%p", playerstats);
    return 0;
}
int32_t playerstats_get_bads(PlayerStats playerstats) {
    print_stub("playerstats_get_bads", "playerstats=%p", playerstats);
    return 0;
}
int32_t playerstats_get_goods(PlayerStats playerstats) {
    print_stub("playerstats_get_goods", "playerstats=%p", playerstats);
    return 0;
}
int32_t playerstats_get_sicks(PlayerStats playerstats) {
    print_stub("playerstats_get_sicks", "playerstats=%p", playerstats);
    return 0;
}
void playerstats_set_health(PlayerStats playerstats, double health) {
    print_stub("playerstats_set_health", "playerstats=%p health=%f", playerstats, health);
}
double playerstats_add_health(PlayerStats playerstats, double health, bool die_if_negative) {
    print_stub("playerstats_add_health", "playerstats=%p health=%f die_if_negative=(bool)%i", playerstats, health, die_if_negative);
    return 0;
}
void playerstats_raise(PlayerStats playerstats, bool with_full_health) {
    print_stub("playerstats_raise", "playerstats=%p with_full_health=(bool)%i", playerstats, with_full_health);
}
void playerstats_kill(PlayerStats playerstats) {
    print_stub("playerstats_kill", "playerstats=%p", playerstats);
}
void playerstats_kill_if_negative_health(PlayerStats playerstats) {
    print_stub("playerstats_kill_if_negative_health", "playerstats=%p", playerstats);
}
bool playerstats_is_dead(PlayerStats playerstats) {
    print_stub("playerstats_is_dead", "playerstats=%p", playerstats);
    return 0;
}
