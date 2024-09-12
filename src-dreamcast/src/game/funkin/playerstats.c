#include "game/funkin/playerstats.h"
#include "externals/luascript.h"
#include "game/common/funkin.h"
#include "malloc_utils.h"
#include "math2d.h"
#include "unused_switch_case.h"


PlayerStats playerstats_init() {
    PlayerStats playerstats = malloc_chk(sizeof(struct PlayerStats_s));
    malloc_assert(playerstats, PlayerStats);

    *playerstats = (struct PlayerStats_s){
        .ranking_sick = 0,
        .ranking_good = 0,
        .ranking_bad = 0,
        .ranking_shit = 0,

        .score = 0,
        .accuracy = 0,

        .health = 0.5,
        .health_max = FUNKIN_HEALTH_BASE,
        .can_recover = 1,

        .iterations = 0, // amount of hit/miss/penalties added

        .hits = 0,      // notes hits
        .miss = 0,      // notes misses
        .penalties = 0, // on empty strum or pressing the wrong button

        .streak = 0,         // combo streak
        .breaks = 0,         // combo breaks count
        .highest_streak = 0, // highest combo streak

        .nps_current = 0, // notes per second
        .nps_highest = 0, // notes per seconds (highest value)

        .last_ranking = Ranking_NONE, // last ranking added and/or calculated
        .last_diff = DOUBLE_NaN,      // last note hit diference (signed value in milliseconds)
        .last_accuracy = DOUBLE_NaN,  // last note hit accuracy

        // internal use only
        .accuracy_accumulator = 0,
        .accuracy_counter = 0,
        .accuracy_counter_last = 0,

        .penalize_on_empty_strum = 1,


        .deads_by_fault = 0, // fault registered with negative health
    };

    return playerstats;
}

void playerstats_destroy(PlayerStats* playerstats_ptr) {
    if (!playerstats_ptr || !*playerstats_ptr) return;

    PlayerStats playerstats = *playerstats_ptr;

    luascript_drop_shared(playerstats);

    free_chk(playerstats);
    *playerstats_ptr = NULL;
}


Ranking playerstats_add_hit(PlayerStats playerstats, float multiplier, float base_note_duration, float64 hit_time_difference) {
    float hit_accuracy = (float)(1.0 - float64_fabs(hit_time_difference / base_note_duration));
    float health_gain;

    playerstats->last_diff = hit_time_difference;
    playerstats->streak++;
    playerstats->hits++;
    playerstats->nps_current++;
    playerstats->iterations++;

    if (playerstats->nps_current > playerstats->nps_highest)
        playerstats->nps_highest = playerstats->nps_current;

    if (hit_accuracy >= FUNKIN_RANKING_SICK) {
        playerstats->ranking_sick++;
        playerstats->score += FUNKIN_SCORE_SICK;
        health_gain = FUNKIN_HEALTH_DIFF_OVER_SICK;
        playerstats->last_ranking = Ranking_SICK;
    } else if (hit_accuracy >= FUNKIN_RANKING_GOOD) {
        playerstats->ranking_good++;
        playerstats->score += FUNKIN_SCORE_GOOD;
        health_gain = FUNKIN_HEALTH_DIFF_OVER_GOOD;
        playerstats->last_ranking = Ranking_GOOD;
    } else if (hit_accuracy >= FUNKIN_RANKING_BAD) {
        playerstats->ranking_bad++;
        playerstats->score += FUNKIN_SCORE_BAD;
        health_gain = FUNKIN_HEALTH_DIFF_OVER_BAD;
        playerstats->last_ranking = Ranking_BAD;
    } else if (hit_accuracy >= FUNKIN_RANKING_SHIT) {
        playerstats->ranking_shit++;
        playerstats->score += FUNKIN_SCORE_SHIT;
        health_gain = FUNKIN_HEALTH_DIFF_OVER_SHIT;
        playerstats->last_ranking = Ranking_SHIT;
    } else {
        // this never should happen (hit_accuracy < zero)
        return playerstats->last_ranking;
    }

    health_gain *= multiplier;
    if (playerstats->can_recover || (health_gain < 0.0 && !playerstats->can_recover)) {
        float64 health = health_gain + playerstats->health;
        if (playerstats->health < 0.0 && multiplier < 0.0) playerstats->deads_by_fault++;
        playerstats->health = math2d_min_double(health, playerstats->health_max);
        if (playerstats->health > 0.0 && playerstats->deads_by_fault > 0) playerstats->deads_by_fault = 0;
    }

    playerstats->accuracy_accumulator += hit_accuracy;
    playerstats->accuracy_counter++;
    return playerstats->last_ranking;
}

void playerstats_add_sustain(PlayerStats playerstats, int32_t quarters, bool is_released) {
    if (is_released) {
        playerstats->last_ranking = Ranking_MISS;
        quarters = -quarters;
        if (playerstats->last_ranking != Ranking_MISS) playerstats->iterations++;
    } else {
        switch (playerstats->last_ranking) {
            case Ranking_MISS:
            case Ranking_PENALITY:
                playerstats->last_ranking = Ranking_GOOD;
                playerstats->iterations++;
                break;
                CASE_UNUSED(Ranking_NONE)
                CASE_UNUSED(Ranking_SICK)
                CASE_UNUSED(Ranking_GOOD)
                CASE_UNUSED(Ranking_BAD)
                CASE_UNUSED(Ranking_SHIT)
        }
        if (!playerstats->can_recover && quarters > 0) return;
    }

    float64 health = playerstats->health + (quarters * FUNKIN_HEALTH_DIFF_OVER_SUSTAIN);
    playerstats->health = math2d_min_double(health, playerstats->health_max);

    if (playerstats->health < 0.0)
        playerstats->deads_by_fault++;
    else if (playerstats->health > 0.0 && playerstats->deads_by_fault > 0)
        playerstats->deads_by_fault = 0;

    playerstats->score += (int32_t)(quarters * FUNKIN_SCORE_MISS);
}

Ranking playerstats_add_sustain_delayed_hit(PlayerStats playerstats, float multiplier, float64 hit_time_difference) {
    playerstats->last_accuracy = 0.0;
    playerstats->last_ranking = FUNKIN_RANKING_SHIT;
    playerstats->last_diff = hit_time_difference;
    playerstats->hits++;
    playerstats->streak++;
    playerstats->nps_current++;
    playerstats->iterations++;

    float64 health_gain = FUNKIN_HEALTH_DIFF_OVER_SHIT * multiplier;
    if (playerstats->can_recover || (health_gain < 0.0 && !playerstats->can_recover)) {
        float64 health = health_gain + playerstats->health;
        if (playerstats->health < 0.0 && multiplier < 0) playerstats->deads_by_fault++;
        playerstats->health = math2d_min_double(health, playerstats->health_max);
        if (playerstats->health > 0.0 && playerstats->deads_by_fault > 0) playerstats->deads_by_fault = 0;
    }

    return playerstats->last_ranking;
}

void playerstats_add_penality(PlayerStats playerstats, bool on_empty_strum) {
    if (!playerstats->penalize_on_empty_strum && on_empty_strum) return;

    float64 health = FUNKIN_HEALTH_DIFF_OVER_PENALITY;
    if (!on_empty_strum) health *= 2.0;
    playerstats->health += health;
    if (playerstats->health < 0) playerstats->deads_by_fault++;

    playerstats->score += FUNKIN_SCORE_PENALITY;
    playerstats->last_ranking = Ranking_PENALITY;
    playerstats->iterations++;
    playerstats->penalties++;

    if (playerstats->streak > 0) {
        if (playerstats->streak > playerstats->highest_streak) {
            playerstats->highest_streak = playerstats->streak;
        }
        playerstats->streak = 0;
        playerstats->breaks++;
    }
}

void playerstats_add_miss(PlayerStats playerstats, float multiplier) {
    //
    // ¿should decrease the accumulated accuracy? ¿with what criteria?
    //

    playerstats->health += FUNKIN_HEALTH_DIFF_OVER_MISS * multiplier;

    if (playerstats->health < 0.0 && multiplier > 0.0f)
        playerstats->deads_by_fault++;
    else if (playerstats->health > 0 && multiplier < 0.0f && playerstats->deads_by_fault > 0)
        playerstats->deads_by_fault = 0;

    playerstats->miss++;
    playerstats->score += FUNKIN_SCORE_MISS;
    playerstats->last_ranking = Ranking_MISS;
    playerstats->iterations++;
}


void playerstats_reset(PlayerStats playerstats) {
    playerstats->ranking_sick = 0;
    playerstats->ranking_good = 0;
    playerstats->ranking_bad = 0;
    playerstats->ranking_shit = 0;
    playerstats->score = 0;
    playerstats->accuracy = 0.0;
    playerstats->iterations = 0;
    playerstats->hits = 0;
    playerstats->miss = 0;
    playerstats->penalties = 0;
    playerstats->streak = 0;
    playerstats->breaks = 0;
    playerstats->highest_streak = 0;
    playerstats->nps_current = 0;
    playerstats->nps_highest = 0;
    playerstats->accuracy_accumulator = 0.0;
    playerstats->accuracy_counter = 0.0;
    playerstats->accuracy_counter_last = 0.0;
    playerstats->deads_by_fault = 0;

    //
    // Non-zero fields
    //
    playerstats->last_ranking = Ranking_NONE;
    playerstats->last_diff = DOUBLE_NaN;
    playerstats->last_accuracy = DOUBLE_NaN;

    playerstats->health = FUNKIN_HEALTH_BASE / 2.0;
    playerstats->health_max = FUNKIN_HEALTH_BASE;
}

void playerstats_reset_notes_per_seconds(PlayerStats playerstats) {
    playerstats->nps_current = 0;
}

void playerstats_add_extra_health(PlayerStats playerstats, float multiplier) {
    math2d_float_assert_NaN(multiplier);

    if (multiplier > 1)
        playerstats->health_max = FUNKIN_HEALTH_BASE * multiplier;
    else
        playerstats->health_max = FUNKIN_HEALTH_BASE;

    playerstats->health = math2d_min_double(playerstats->health, playerstats->health_max);
}


void playerstats_enable_penality_on_empty_strum(PlayerStats playerstats, bool enable) {
    playerstats->penalize_on_empty_strum = enable;
}

void playerstats_enable_health_recover(PlayerStats playerstats, bool enable) {
    playerstats->can_recover = enable;
}


float64 playerstats_get_maximum_health(PlayerStats playerstats) {
    return playerstats->health_max;
}

float64 playerstats_get_health(PlayerStats playerstats) {
    return playerstats->health;
}

float64 playerstats_get_accuracy(PlayerStats playerstats) {
    if (playerstats->accuracy_counter_last != playerstats->accuracy_counter) {
        playerstats->accuracy = playerstats->accuracy_accumulator / playerstats->accuracy_counter;
        playerstats->accuracy_counter_last = playerstats->accuracy_counter;

        // for convenience express the result as hundred (100.0000000%)
        playerstats->accuracy *= 100.0;
    }

    return playerstats->accuracy;
}

float64 playerstats_get_last_accuracy(PlayerStats playerstats) {
    return playerstats->last_accuracy;
}

Ranking playerstats_get_last_ranking(PlayerStats playerstats) {
    return playerstats->last_ranking;
}

float64 playerstats_get_last_difference(PlayerStats playerstats) {
    return playerstats->last_diff;
}

int32_t playerstats_get_combo_streak(PlayerStats playerstats) {
    return playerstats->streak;
}

int32_t playerstats_get_highest_combo_streak(PlayerStats playerstats) {
    return playerstats->highest_streak;
}

int32_t playerstats_get_combo_breaks(PlayerStats playerstats) {
    return playerstats->breaks;
}

int32_t playerstats_get_notes_per_seconds(PlayerStats playerstats) {
    return playerstats->nps_current;
}

int32_t playerstats_get_notes_per_seconds_highest(PlayerStats playerstats) {
    return playerstats->nps_highest;
}

int32_t playerstats_get_iterations(PlayerStats playerstats) {
    return playerstats->iterations;
}

int64_t playerstats_get_score(PlayerStats playerstats) {
    return playerstats->score;
}

int32_t playerstats_get_hits(PlayerStats playerstats) {
    return playerstats->hits;
}

int32_t playerstats_get_misses(PlayerStats playerstats) {
    return playerstats->miss;
}

int32_t playerstats_get_penalties(PlayerStats playerstats) {
    return playerstats->penalties;
}

int32_t playerstats_get_shits(PlayerStats playerstats) {
    return playerstats->ranking_shit;
}

int32_t playerstats_get_bads(PlayerStats playerstats) {
    return playerstats->ranking_bad;
}

int32_t playerstats_get_goods(PlayerStats playerstats) {
    return playerstats->ranking_good;
}

int32_t playerstats_get_sicks(PlayerStats playerstats) {
    return playerstats->ranking_sick;
}




void playerstats_set_health(PlayerStats playerstats, float64 health) {
    math2d_double_assert_NaN(health);
    playerstats->health = math2d_min_double(health, playerstats->health_max);
    if (health >= 0.0) playerstats->deads_by_fault = 0;
}

float64 playerstats_add_health(PlayerStats playerstats, float64 health, bool die_if_negative) {
    math2d_double_assert_NaN(health);

    health += playerstats->health;

    if (!die_if_negative && health < 0.0)
        health = 0.0;
    else if (health > playerstats->health_max)
        health = playerstats->health_max;

    if (die_if_negative && health < 0.0) playerstats->deads_by_fault++;

    return playerstats->health = health;
}


void playerstats_raise(PlayerStats playerstats, bool with_full_health) {
    if (playerstats->health < 0.0) playerstats->health = with_full_health ? playerstats->health_max : 0.0;
    playerstats->deads_by_fault = 0;
}

void playerstats_kill(PlayerStats playerstats) {
    playerstats->health = -1.0;
    playerstats->deads_by_fault++;
}

void playerstats_kill_if_negative_health(PlayerStats playerstats) {
    if (playerstats->health < 0.0) {
        playerstats->deads_by_fault++;
        playerstats->health = 0.0;
    }
}

int32_t playerstats_is_dead(PlayerStats playerstats) {
    return playerstats->deads_by_fault;
}
