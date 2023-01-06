"use strict";

const WEEKRESULT_WEEK_TEXT = "~judgements~\nSicks   $i\nGoods  $i\nBads   $i\n" +
    "\n~failures~\nShits      $i\nMissses    $i\nPenalties  $i" +
    "\n\n~streak~\nCombo breaks   $i\nHighest combo  $i";
const WEEKRESULT_WEEK_TEXT2 = "~week resume~\n" +
    "Total attempts $i\nTotal score    $l\nAverage accuracy $3d%";

const WEEKRESULT_TRACK_TEXT = "~judgements~\nSicks  $i\nGoods  $i\nBads   $i\n" +
    "\n~failures~\nShits      $i\nMissses    $i\nPenalties  $i" +
    "\n\n~streak~\nCombo breaks   $i\nHighest combo  $i\nHighest nps    $i";
const WEEKRESULT_TRACK_TEXT2 = "~track resume~\nAttempts $i\nScore    $l\nAvg. accuracy $3d%";


const WEEKRESULT_LAYOUT_WIDESCREEN = "/assets/common/image/week-round/results.xml";
const WEEKRESULT_LAYOUT_DREAMCAST = "/assets/common/image/week-round/results~dreamcast.xml";


function week_result_init() {
    let weekresult = {
        stats_track: {
            sick: 0,
            good: 0,
            bads: 0,
            shits: 0,
            miss: 0,
            penalties: 0,
            score: 0,
            accuracy: 0,
            notesperseconds: 0,
            combobreaks: 0,
            higheststreak: 0,
        },
        accumulated_stats_week: {
            sick: 0,
            good: 0,
            bads: 0,
            shits: 0,
            miss: 0,
            penalties: 0,
            score: 0,
            accuracy: 0,
            notesperseconds: 0,
            combobreaks: 0,
            higheststreak: 0,
        }
    };

    return weekresult;
}

function week_result_destroy(weekresult) {
    weekresult = undefined;
}

function week_result_add_stats(weekresult, roundcontext) {
    for (let i = 0; i < roundcontext.players_size; i++) {
        if (roundcontext.players[i].type != CHARACTERTYPE.PLAYER) continue;
        if (roundcontext.players[i].is_opponent) continue;
        let playerstats = roundcontext.players[i].playerstats;

        weekresult.stats_track.sick = playerstats_get_sicks(playerstats);
        weekresult.stats_track.good = playerstats_get_goods(playerstats);
        weekresult.stats_track.bads = playerstats_get_bads(playerstats);
        weekresult.stats_track.shits = playerstats_get_shits(playerstats);
        weekresult.stats_track.miss = playerstats_get_misses(playerstats);
        weekresult.stats_track.penalties = playerstats_get_penalties(playerstats);
        weekresult.stats_track.score = playerstats_get_score(playerstats);
        weekresult.stats_track.accuracy = playerstats_get_accuracy(playerstats);
        weekresult.stats_track.notesperseconds = playerstats_get_notes_per_seconds_highest(playerstats);
        weekresult.stats_track.combobreaks = playerstats_get_combo_breaks(playerstats);

        let higheststreak = playerstats_get_highest_combo_streak(playerstats);
        if (higheststreak > weekresult.stats_track.higheststreak) {
            weekresult.stats_track.higheststreak = higheststreak;
        }
    }

    weekresult.accumulated_stats_week.sick += weekresult.stats_track.sick;
    weekresult.accumulated_stats_week.good += weekresult.stats_track.good;
    weekresult.accumulated_stats_week.bads += weekresult.stats_track.bads;
    weekresult.accumulated_stats_week.shits += weekresult.stats_track.shits;
    weekresult.accumulated_stats_week.miss += weekresult.stats_track.miss;
    weekresult.accumulated_stats_week.penalties += weekresult.stats_track.penalties;
    weekresult.accumulated_stats_week.score += weekresult.stats_track.score;
    weekresult.accumulated_stats_week.accuracy += weekresult.stats_track.accuracy;
    weekresult.accumulated_stats_week.notesperseconds += weekresult.stats_track.notesperseconds;
    weekresult.accumulated_stats_week.combobreaks += weekresult.stats_track.combobreaks;

    if (weekresult.stats_track.higheststreak > weekresult.accumulated_stats_week.higheststreak) {
        weekresult.accumulated_stats_week.higheststreak = weekresult.stats_track.higheststreak;
    }

}

async function week_result_helper_show_summary(weekresult, roundcontext, attempts, tracks_count, is_cleared) {
	let src = pvrctx_is_widescreen() ? WEEKRESULT_LAYOUT_WIDESCREEN : WEEKRESULT_LAYOUT_DREAMCAST;
    let layout = await layout_init(src);
    if (!layout) return;

    if (is_cleared)
        layout_trigger_any(layout, "week_not_cleared");
    else
        layout_trigger_any(layout, "week_cleared");

    let textsprite1 = layout_get_textsprite(layout, "stats");
    let textsprite2 = layout_get_textsprite(layout, "stats2");
    let transition_delay = layout_get_attached_value_as_float(layout, "transition_delay", 0);

    if (tracks_count < 0) {
        // show the stats for the current (completed) track
        let accumulated_stats = weekresult.stats_track;

        if (textsprite1)
            textsprite_set_text_formated(textsprite1, WEEKRESULT_TRACK_TEXT,
                // judgements
                accumulated_stats.sick,
                accumulated_stats.good,
                accumulated_stats.bads,

                // streak
                accumulated_stats.shits,
                accumulated_stats.miss,
                accumulated_stats.penalties,

                accumulated_stats.combobreaks,
                accumulated_stats.higheststreak,
                accumulated_stats.notesperseconds
            );
        if (textsprite2)
            textsprite_set_text_formated(textsprite2, WEEKRESULT_TRACK_TEXT2,
                // week resume
                attempts,
                accumulated_stats.score,
                accumulated_stats.accuracy
            );
    } else {
        let accumulated_stats = weekresult.accumulated_stats_week;
        let average_accuracy = accumulated_stats.accuracy / tracks_count;

        if (textsprite1)
            textsprite_set_text_formated(textsprite1, WEEKRESULT_WEEK_TEXT,
                // judgements
                accumulated_stats.sick,
                accumulated_stats.good,
                accumulated_stats.bads,

                // streak
                accumulated_stats.shits,
                accumulated_stats.miss,
                accumulated_stats.penalties,

                accumulated_stats.combobreaks,
                accumulated_stats.higheststreak
            );
        if (textsprite2)
            textsprite_set_text_formated(textsprite2, WEEKRESULT_WEEK_TEXT2,
                // week resume
                attempts,
                accumulated_stats.score,
                average_accuracy
            );
    }

    let transition = 0;
    let controller = gamepad_init(-1);
    gamepad_clear_buttons(controller);

    while (1) {
        let elapsed = await pvrctx_wait_ready();
        pvr_context_reset(pvr_context);

        if (roundcontext.script) await weekscript_notify_frame(roundcontext.script, elapsed);

        if (roundcontext.layout) {
            layout_animate(roundcontext.layout, elapsed);
            layout_draw(roundcontext.layout, pvr_context);
        }

        layout_animate(layout, elapsed);
        layout_draw(layout, pvr_context);

        if (transition) {
            if (transition_delay > 0) {
                transition_delay -= elapsed;
                if (transition_delay < 0) layout_trigger_any(layout, "transition");
                continue;
            }
            if (layout_animation_is_completed(layout, "transition_effect")) break;
        } else if (gamepad_get_pressed(controller)) {
            transition = 1;
            layout_set_group_visibility(layout, "allstats", 0);
        }
    }

    layout_destroy(layout);
}

