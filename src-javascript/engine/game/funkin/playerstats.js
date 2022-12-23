"use strict";

const FUNKIN_COMBO_STREAK_VISIBLE_AFTER = 10;

const FUNKIN_HEALTH_BASE = 1.0;


//
// Note:
//      values are expressed in percent, and indicates the minimum precission
//      required to apply the desired ranking
//
const FUNKIN_RANKING_SHIT = 0.00;
const FUNKIN_RANKING_BAD = 0.25;
const FUNKIN_RANKING_GOOD = 0.50;
const FUNKIN_RANKING_SICK = 0.75;

//
// Notes:
//      - amount of points to add/remove for the desires accuracy
//      - penality means pressing a key without the corresponding arrow 
//      - values taken from "https://fridaynightfunkin.fandom.com/wiki/Friday_Night_Funkin'#Score"
//      - in some mods "penality" and "miss" values can be different
//
const FUNKIN_SCORE_PENALITY = -10;
const FUNKIN_SCORE_MISS = 0;
const FUNKIN_SCORE_SHIT = 50;
const FUNKIN_SCORE_BAD = 100;
const FUNKIN_SCORE_GOOD = 200;
const FUNKIN_SCORE_SICK = 350;

//
// Notes:
//      - this indicates how much health the player will loose or gain
//      - expressed in percent (100 health units or 100% or 1.0)
//      - Funkin and Kade engine sourcecode are illegible so i have to "reinvent the wheel"
//
const FUNKIN_HEALTH_DIFF_OVER_PENALITY = -0.01;
const FUNKIN_HEALTH_DIFF_OVER_MISS = -0.2;
const FUNKIN_HEALTH_DIFF_OVER_SHIT = -0.01;
const FUNKIN_HEALTH_DIFF_OVER_BAD =  0.00;
const FUNKIN_HEALTH_DIFF_OVER_GOOD = 0.02;
const FUNKIN_HEALTH_DIFF_OVER_SICK = 0.04;
const FUNKIN_HEALTH_DIFF_OVER_SUSTAIN = 0.01;// applies to hold (gain), release (loose) and recover (gain)

//
// Rank enumerations (used in playerstats->last_ranking)
//
const PLAYERSTATS_RANK_NONE = 0;
const PLAYERSTATS_RANK_SICK = 1;
const PLAYERSTATS_RANK_GOOD = 2;
const PLAYERSTATS_RANK_BAD = 3;
const PLAYERSTATS_RANK_SHIT = 4;
const PLAYERSTATS_RANK_MISS = 5;
const PLAYERSTATS_RANK_PENALITY = 6;


function playerstats_init() {
    return {
        ranking_sick: 0,
        ranking_good: 0,
        ranking_bad: 0,
        ranking_shit: 0,

        score: 0,
        accuracy: 0,

        health: 0.5,
        health_max: FUNKIN_HEALTH_BASE,
        can_recover: 1,

        iterations: 0,// amount of hit/miss/penalties added

        hits: 0,// notes hits
        miss: 0,// notes misses
        penalties: 0,// on empty strum or pressing the wrong button

        streak: 0,// combo streak
        breaks: 0,// combo breaks count
        highest_streak: 0,// highest combo streak

        nps_current: 0,// notes per second
        nps_highest: 0,// notes per seconds (highest value)

        last_ranking: PLAYERSTATS_RANK_NONE,// last ranking added and/or calculated
        last_diff: NaN,// last note hit diference (signed value in milliseconds)
        last_accuracy: NaN,// last note hit accuracy

        // internal use only
        accuracy_accumulator: 0,
        accuracy_counter: 0,
        accuracy_counter_last: 0,

        penalize_on_empty_strum: 1,


        deads_by_fault: 0,// fault registered with negative health
    };
}

function playerstats_destroy(playerstats) {
    ModuleLuaScript.kdmyEngine_drop_shared_object(playerstats);

    playerstats = undefined;
}


function playerstats_add_hit(playerstats, multiplier, base_note_duration, hit_time_difference) {
    let hit_accuracy = 1.0 - Math.abs(hit_time_difference / base_note_duration);
    let health_gain;

    playerstats.last_diff = hit_time_difference;
    playerstats.streak++;
    playerstats.hits++;
    playerstats.nps_current++;
    playerstats.iterations++;

    if (playerstats.nps_current > playerstats.nps_highest)
        playerstats.nps_highest = playerstats.nps_current;

    if (hit_accuracy >= FUNKIN_RANKING_SICK) {
        playerstats.ranking_sick++;
        playerstats.score += FUNKIN_SCORE_SICK;
        health_gain = FUNKIN_HEALTH_DIFF_OVER_SICK;
        playerstats.last_ranking = PLAYERSTATS_RANK_SICK;
    } else if (hit_accuracy >= FUNKIN_RANKING_GOOD) {
        playerstats.ranking_good++;
        playerstats.score += FUNKIN_SCORE_GOOD;
        health_gain = FUNKIN_HEALTH_DIFF_OVER_GOOD;
        playerstats.last_ranking = PLAYERSTATS_RANK_GOOD;
    } else if (hit_accuracy >= FUNKIN_RANKING_BAD) {
        playerstats.ranking_bad++;
        playerstats.score += FUNKIN_SCORE_BAD;
        health_gain = FUNKIN_HEALTH_DIFF_OVER_BAD;
        playerstats.last_ranking = PLAYERSTATS_RANK_BAD;
    } else if (hit_accuracy >= FUNKIN_RANKING_SHIT) {
        playerstats.ranking_shit++;
        playerstats.score += FUNKIN_SCORE_SHIT;
        health_gain = FUNKIN_HEALTH_DIFF_OVER_SHIT;
        playerstats.last_ranking = PLAYERSTATS_RANK_SHIT;
    } else {
        // this never should happen (hit_accuracy < zero)
        return playerstats.last_ranking;
    }

    health_gain *= multiplier;
    if (playerstats.can_recover || (health_gain < 0 && !playerstats.can_recover)) {
        let health = health_gain + playerstats.health;
        if (playerstats.health < 0 && multiplier < 0) playerstats.deads_by_fault++;
        playerstats.health = Math.min(health, playerstats.health_max);
        if (playerstats.health > 0 && playerstats.deads_by_fault > 0) playerstats.deads_by_fault = 0;
    }

    playerstats.accuracy_accumulator += hit_accuracy;
    playerstats.accuracy_counter++;
    return playerstats.last_ranking;
}

function playerstats_add_sustain(playerstats, quarters, is_released) {
    if (Number.isNaN(quarters)) throw new NaNArgumentError();

    if (is_released) {
        playerstats.last_ranking = PLAYERSTATS_RANK_MISS;
        quarters = -quarters;
        if (playerstats.last_ranking != PLAYERSTATS_RANK_MISS) playerstats.iterations++;
    } else {
        switch (playerstats.last_ranking) {
            case PLAYERSTATS_RANK_MISS:
            case PLAYERSTATS_RANK_PENALITY:
                playerstats.last_ranking = PLAYERSTATS_RANK_GOOD;
                playerstats.iterations++;
                break;
        }
        if (!playerstats.can_recover && quarters > 0) return;

    }

    let health = playerstats.health + (quarters * FUNKIN_HEALTH_DIFF_OVER_SUSTAIN);
    playerstats.health = Math.min(health, playerstats.health_max);

    if (playerstats.health < 0)
        playerstats.deads_by_fault++;
    else if (playerstats.health > 0 && playerstats.deads_by_fault > 0)
        playerstats.deads_by_fault = 0;

    playerstats.score += quarters * FUNKIN_SCORE_MISS;
}

function playerstats_add_sustain_delayed_hit(playerstats, multiplier, hit_time_difference) {
    playerstats.last_accuracy = 0.0;
    playerstats.last_ranking = FUNKIN_RANKING_SHIT;
    playerstats.last_diff = hit_time_difference;
    playerstats.hits++;
    playerstats.streak++;
    playerstats.nps_current++;
    playerstats.iterations++;

    let health_gain = FUNKIN_HEALTH_DIFF_OVER_SHIT * multiplier;
    if (playerstats.can_recover || (health_gain < 0.0 && !playerstats.can_recover)) {
        let health = health_gain + playerstats.health;
        if (playerstats.health < 0.0 && multiplier < 0) playerstats.deads_by_fault++;
        playerstats.health = Math.min(health, playerstats.health_max);
        if (playerstats.health > 0.0 && playerstats.deads_by_fault > 0) playerstats.deads_by_fault = 0;
    }

    return playerstats.last_ranking;
}

function playerstats_add_penality(playerstats, on_empty_strum) {
    if (!playerstats.penalize_on_empty_strum && on_empty_strum) return;

    let health = FUNKIN_HEALTH_DIFF_OVER_PENALITY;
    if (!on_empty_strum) health *= 2;
    playerstats.health += health;
    if (playerstats.health < 0) playerstats.deads_by_fault++;

    playerstats.score += FUNKIN_SCORE_PENALITY;
    playerstats.last_ranking = PLAYERSTATS_RANK_PENALITY;
    playerstats.iterations++;
    playerstats.penalties++;

    if (playerstats.streak > 0) {
        if (playerstats.streak > playerstats.highest_streak) {
            playerstats.highest_streak = playerstats.streak;
        }
        playerstats.streak = 0;
        playerstats.breaks++;
    }

}

function playerstats_add_miss(playerstats, multiplier) {
    //
    // FIXME: ¿should decrease the accumulated accuracy? ¿with what criteria? 
    //

    playerstats.health += FUNKIN_HEALTH_DIFF_OVER_MISS * multiplier;

    if (playerstats.health < 0 && multiplier > 0)
        playerstats.deads_by_fault++;
    else if (playerstats.health > 0 && multiplier < 0 && playerstats.deads_by_fault > 0)
        playerstats.deads_by_fault = 0;

    playerstats.miss++;
    playerstats.score += FUNKIN_SCORE_MISS;
    playerstats.last_ranking = PLAYERSTATS_RANK_MISS;
    playerstats.iterations++;
}


function playerstats_reset(playerstats) {
    playerstats.ranking_sick = 0;
    playerstats.ranking_good = 0;
    playerstats.ranking_bad = 0;
    playerstats.ranking_shit = 0;
    playerstats.score = 0;
    playerstats.accuracy = 0;
    playerstats.iterations = 0;
    playerstats.hits = 0;
    playerstats.miss = 0;
    playerstats.penalties = 0;
    playerstats.streak = 0;
    playerstats.breaks = 0;
    playerstats.highest_streak = 0;
    playerstats.nps_current = 0;
    playerstats.nps_highest = 0;
    playerstats.accuracy_accumulator = 0;
    playerstats.accuracy_counter = 0;
    playerstats.accuracy_counter_last = 0;
    playerstats.deads_by_fault = 0;

    //
    // Non-zero fields
    //
    playerstats.last_ranking = PLAYERSTATS_RANK_NONE;
    playerstats.last_diff = NaN;
    playerstats.last_accuracy = NaN;

    playerstats.health = FUNKIN_HEALTH_BASE / 2;
    playerstats.health_max = FUNKIN_HEALTH_BASE;
}

function playerstats_reset_notes_per_seconds(playerstats) {
    playerstats.nps_current = 0;
}

function playerstats_add_extra_health(playerstats, multiplier) {
    if (Number.isNaN(multiplier)) throw new NaNArgumentError();

    if (multiplier > 1)
        playerstats.health_max = FUNKIN_HEALTH_BASE * multiplier;
    else
        playerstats.health_max = FUNKIN_HEALTH_BASE;

    playerstats.health = Math.min(playerstats.health, playerstats.health_max);
}


function playerstats_enable_penality_on_empty_strum(playerstats, enable) {
    playerstats.penalize_on_empty_strum = !!enable;
}

function playerstats_enable_health_recover(playerstats, enable) {
    playerstats.can_recover = !!enable;
}


function playerstats_get_maximum_health(playerstats) {
    return playerstats.health_max;
}

function playerstats_get_health(playerstats) {
    return playerstats.health;
}

function playerstats_get_accuracy(playerstats) {
    if (playerstats.accuracy_counter_last != playerstats.accuracy_counter) {
        playerstats.accuracy = playerstats.accuracy_accumulator / playerstats.accuracy_counter;
        playerstats.accuracy_counter_last = playerstats.accuracy_counter;

        // for convenience express the result as hundred (100.0000000%)
        playerstats.accuracy *= 100.0;
    }

    return playerstats.accuracy;
}

function playerstats_get_last_accuracy(playerstats) {
    return playerstats.last_accuracy;
}

function playerstats_get_last_ranking(playerstats) {
    return playerstats.last_ranking;
}

function playerstats_get_last_difference(playerstats) {
    return playerstats.last_diff;
}

function playerstats_get_combo_streak(playerstats) {
    return playerstats.streak;
}

function playerstats_get_highest_combo_streak(playerstats) {
    return playerstats.highest_streak;
}

function playerstats_get_combo_breaks(playerstats) {
    return playerstats.breaks;
}

function playerstats_get_notes_per_seconds(playerstats) {
    return playerstats.nps_current;
}

function playerstats_get_notes_per_seconds_highest(playerstats) {
    return playerstats.nps_highest;
}

function playerstats_get_iterations(playerstats) {
    return playerstats.iterations;
}

function playerstats_get_score(playerstats) {
    return playerstats.score;
}

function playerstats_get_hits(playerstats) {
    return playerstats.hits;
}

function playerstats_get_misses(playerstats) {
    return playerstats.miss;
}

function playerstats_get_penalties(playerstats) {
    return playerstats.penalties;
}

function playerstats_get_shits(playerstats) {
    return playerstats.ranking_shit;
}

function playerstats_get_bads(playerstats) {
    return playerstats.ranking_bad;
}

function playerstats_get_goods(playerstats) {
    return playerstats.ranking_good;
}

function playerstats_get_sicks(playerstats) {
    return playerstats.ranking_sick;
}




function playerstats_set_health(playerstats, health) {
    if (Number.isNaN(health)) throw new NaNArgumentError("health");
    playerstats.health = Math.min(health, playerstats.health_max);
    if (health >= 0) playerstats.deads_by_fault = 0;
}

function playerstats_add_health(playerstats, health, die_if_negative) {
    if (Number.isNaN(health)) throw new NaNArgumentError("health");

    health += playerstats.health;

    if (!die_if_negative && health < 0)
        health = 0;
    else if (health > playerstats.health_max)
        health = playerstats.health_max;

    if (die_if_negative && health < 0) playerstats.deads_by_fault++;

    return playerstats.health = health;
}


function playerstats_raise(playerstats, with_full_health) {
    if (playerstats.health < 0) playerstats.health = with_full_health ? playerstats.health_max : 0;
    playerstats.deads_by_fault = 0;
}

function playerstats_kill(playerstats) {
    playerstats.health = -1;
    playerstats.deads_by_fault++;
}

function playerstats_kill_if_negative_health(playerstats) {
    if (playerstats.health < 0) {
        playerstats.deads_by_fault++;
        playerstats.health = 0;
    }
}

function playerstats_is_dead(playerstats) {
    return playerstats.deads_by_fault;
}

