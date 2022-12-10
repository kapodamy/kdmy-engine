#ifndef __playerstats_h
#define __playerstats_h

#include <stdbool.h>
#include <stdint.h>

#define PLAYERSTATS_RANKING_NONE 0
#define PLAYERSTATS_RANKING_SICK 1
#define PLAYERSTATS_RANKING_GOOD 2
#define PLAYERSTATS_RANKING_BAD 3
#define PLAYERSTATS_RANKING_SHIT 4
#define PLAYERSTATS_RANKING_MISS 5
#define PLAYERSTATS_RANKING_PENALITY 6
typedef int32_t Ranking;

typedef struct {
    int dummy;
} PlayerStats_t;
typedef PlayerStats_t* PlayerStats;


Ranking playerstats_add_hit(PlayerStats playerstats, float multiplier, float base_note_duration, double hit_time_difference);
void playerstats_add_sustain(PlayerStats playerstats, int32_t quarters, bool is_released);
Ranking playerstats_add_sustain_delayed_hit(PlayerStats playerstats, float multiplier, float hit_time_difference);
void playerstats_add_penality(PlayerStats playerstats, bool on_empty_strum);
void playerstats_add_miss(PlayerStats playerstats, float multiplier);
void playerstats_reset(PlayerStats playerstats);
void playerstats_reset_notes_per_seconds(PlayerStats playerstats);
void playerstats_add_extra_health(PlayerStats playerstats, float multiplier);
void playerstats_enable_penality_on_empty_strum(PlayerStats playerstats, bool enable);
double playerstats_get_maximum_health(PlayerStats playerstats);
double playerstats_get_health(PlayerStats playerstats);
double playerstats_get_accuracy(PlayerStats playerstats);
double playerstats_get_last_accuracy(PlayerStats playerstats);
Ranking playerstats_get_last_ranking(PlayerStats playerstats);
double playerstats_get_last_difference(PlayerStats playerstats);
int32_t playerstats_get_combo_streak(PlayerStats playerstats);
int32_t playerstats_get_highest_combo_streak(PlayerStats playerstats);
int32_t playerstats_get_combo_breaks(PlayerStats playerstats);
int32_t playerstats_get_notes_per_seconds(PlayerStats playerstats);
int32_t playerstats_get_notes_per_seconds_highest(PlayerStats playerstats);
int32_t playerstats_get_iterations(PlayerStats playerstats);
int64_t playerstats_get_score(PlayerStats playerstats);
int32_t playerstats_get_hits(PlayerStats playerstats);
int32_t playerstats_get_misses(PlayerStats playerstats);
int32_t playerstats_get_penalties(PlayerStats playerstats);
int32_t playerstats_get_shits(PlayerStats playerstats);
int32_t playerstats_get_bads(PlayerStats playerstats);
int32_t playerstats_get_goods(PlayerStats playerstats);
int32_t playerstats_get_sicks(PlayerStats playerstats);
void playerstats_set_health(PlayerStats playerstats, double health);
double playerstats_add_health(PlayerStats playerstats, double health, bool die_if_negative);
void playerstats_raise(PlayerStats playerstats, bool with_full_health);
void playerstats_kill(PlayerStats playerstats);
void playerstats_kill_if_negative_health(PlayerStats playerstats);
bool playerstats_is_dead(PlayerStats playerstats);

#endif
