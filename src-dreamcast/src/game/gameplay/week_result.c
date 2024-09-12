#include "game/gameplay/week_result.h"

#include "externals/luascript.h"
#include "game/funkin/playerstats.h"
#include "game/gameplay/week.h"
#include "game/gameplay/weekscript.h"
#include "game/modding.h"
#include "layout.h"
#include "malloc_utils.h"
#include "pvrcontext.h"
#include "pvrcontext_global.h"
#include "textsprite.h"


typedef struct {
    int32_t sick;
    int32_t good;
    int32_t bads;
    int32_t shits;
    int32_t miss;
    int32_t penalties;
    int64_t score;
    float64 accuracy;
    int32_t notesperseconds;
    int32_t combobreaks;
    int32_t higheststreak;
} StatsSong;

typedef struct {
    int32_t sick;
    int32_t good;
    int32_t bads;
    int32_t shits;
    int32_t miss;
    int32_t penalties;
    int64_t score;
    float64 accuracy;
    int32_t notesperseconds;
    int32_t combobreaks;
    int32_t higheststreak;
} StatsAccumulated;

struct WeekResult_s {
    Layout active_layout;
    StatsSong stats_song;
    StatsAccumulated accumulated_stats_week;
};


static const char* WEEKRESULT_WEEK_TEXT = "~judgements~\nSicks   $i\nGoods  $i\nBads   $i\n"
                                          "\n~failures~\nShits      $i\nMissses    $i\nPenalties  $i"
                                          "\n\n~streak~\nCombo breaks   $i\nHighest combo  $i";
static const char* WEEKRESULT_WEEK_TEXT2 = "~week resume~\n"
                                           "Total attempts $i\nTotal score    $l\nAverage accuracy $3d%";

static const char* WEEKRESULT_SONG_TEXT = "~judgements~\nSicks  $i\nGoods  $i\nBads   $i\n"
                                          "\n~failures~\nShits      $i\nMissses    $i\nPenalties  $i"
                                          "\n\n~streak~\nCombo breaks   $i\nHighest combo  $i\nHighest nps    $i";
static const char* WEEKRESULT_SONG_TEXT2 = "~song resume~\nAttempts $i\nScore    $l\nAvg. accuracy $3d%";


static const char* WEEKRESULT_LAYOUT_WIDESCREEN = "/assets/common/image/week-round/results.xml";
static const char* WEEKRESULT_LAYOUT_DREAMCAST = "/assets/common/image/week-round/results~dreamcast.xml";
static const char* WEEKRESULT_MODDING_SCRIPT = "/assets/common/data/scripts/weekresult.lua";


WeekResult week_result_init() {
    WeekResult weekresult = malloc_chk(sizeof(struct WeekResult_s));
    malloc_assert(weekresult, WeekResult);

    *weekresult = (struct WeekResult_s){
        .active_layout = NULL,
        .stats_song = (StatsSong){
            .sick = 0,
            .good = 0,
            .bads = 0,
            .shits = 0,
            .miss = 0,
            .penalties = 0,
            .score = 0,
            .accuracy = 0.0,
            .notesperseconds = 0,
            .combobreaks = 0,
            .higheststreak = 0,
        },
        .accumulated_stats_week = (StatsAccumulated){
            .sick = 0,
            .good = 0,
            .bads = 0,
            .shits = 0,
            .miss = 0,
            .penalties = 0,
            .score = 0,
            .accuracy = 0.0,
            .notesperseconds = 0,
            .combobreaks = 0,
            .higheststreak = 0,
        }
    };

    return weekresult;
}

void week_result_destroy(WeekResult* weekresult_ptr) {
    if (!weekresult_ptr || !*weekresult_ptr) return;

    WeekResult weekresult = *weekresult_ptr;

    // Note: do not dispose the "active_layout" field

    free_chk(weekresult);
    *weekresult_ptr = NULL;
}

void week_result_add_stats(WeekResult weekresult, RoundContext roundcontext) {
    for (int32_t i = 0; i < roundcontext->players_size; i++) {
        if (roundcontext->players[i].type != CharacterType_PLAYER) continue;
        if (roundcontext->players[i].is_opponent) continue;
        PlayerStats playerstats = roundcontext->players[i].playerstats;

        weekresult->stats_song.sick = playerstats_get_sicks(playerstats);
        weekresult->stats_song.good = playerstats_get_goods(playerstats);
        weekresult->stats_song.bads = playerstats_get_bads(playerstats);
        weekresult->stats_song.shits = playerstats_get_shits(playerstats);
        weekresult->stats_song.miss = playerstats_get_misses(playerstats);
        weekresult->stats_song.penalties = playerstats_get_penalties(playerstats);
        weekresult->stats_song.score = playerstats_get_score(playerstats);
        weekresult->stats_song.accuracy = playerstats_get_accuracy(playerstats);
        weekresult->stats_song.notesperseconds = playerstats_get_notes_per_seconds_highest(playerstats);
        weekresult->stats_song.combobreaks = playerstats_get_combo_breaks(playerstats);

        int32_t higheststreak = playerstats_get_highest_combo_streak(playerstats);
        if (higheststreak > weekresult->stats_song.higheststreak) {
            weekresult->stats_song.higheststreak = higheststreak;
        }
    }

    weekresult->accumulated_stats_week.sick += weekresult->stats_song.sick;
    weekresult->accumulated_stats_week.good += weekresult->stats_song.good;
    weekresult->accumulated_stats_week.bads += weekresult->stats_song.bads;
    weekresult->accumulated_stats_week.shits += weekresult->stats_song.shits;
    weekresult->accumulated_stats_week.miss += weekresult->stats_song.miss;
    weekresult->accumulated_stats_week.penalties += weekresult->stats_song.penalties;
    weekresult->accumulated_stats_week.score += weekresult->stats_song.score;
    weekresult->accumulated_stats_week.accuracy += weekresult->stats_song.accuracy;
    weekresult->accumulated_stats_week.notesperseconds += weekresult->stats_song.notesperseconds;
    weekresult->accumulated_stats_week.combobreaks += weekresult->stats_song.combobreaks;

    if (weekresult->stats_song.higheststreak > weekresult->accumulated_stats_week.higheststreak) {
        weekresult->accumulated_stats_week.higheststreak = weekresult->stats_song.higheststreak;
    }
}

void week_result_helper_show_summary(WeekResult weekresult, RoundContext roundcontext, int32_t attempts, int32_t songs_count, bool freeplay, bool reject) {
    const char* src = pvr_context_is_widescreen() ? WEEKRESULT_LAYOUT_WIDESCREEN : WEEKRESULT_LAYOUT_DREAMCAST;
    Layout layout = layout_init(src);
    if (!layout) return;

    weekresult->active_layout = layout;

    if (roundcontext->script) {
        weekscript_notify_beforeresults(roundcontext->script);
        week_halt(roundcontext, true);
    }

    Modding modding = modding_init(layout, WEEKRESULT_MODDING_SCRIPT);
    modding->has_exit = false;
    modding->has_halt = false;
    modding->native_menu = NULL;
    modding_helper_notify_init(modding, MODDING_NATIVE_MENU_SCREEN, ModdingValueType_string);

    if (reject) {
        layout_trigger_any(layout, "week_not_cleared");
        modding_helper_notify_event(modding, "week_not_cleared");
    } else if (freeplay) {
        layout_trigger_any(layout, "song_cleared");
        modding_helper_notify_event(modding, "song_cleared");
    } else {
        layout_trigger_any(layout, "week_cleared");
        modding_helper_notify_event(modding, "week_cleared");
    }

    TextSprite textsprite1 = layout_get_textsprite(layout, "stats");
    TextSprite textsprite2 = layout_get_textsprite(layout, "stats2");
    float transition_delay = layout_get_attached_value_as_float(layout, "transition_delay", 0.0f);

    if (songs_count < 0) {
        // show the stats for the current (completed) song
        StatsSong* accumulated_stats = &weekresult->stats_song;

        if (textsprite1)
            textsprite_set_text_formated(
                textsprite1, WEEKRESULT_SONG_TEXT,
                // judgements
                accumulated_stats->sick, accumulated_stats->good, accumulated_stats->bads,

                // streak
                accumulated_stats->shits, accumulated_stats->miss, accumulated_stats->penalties,

                accumulated_stats->combobreaks, accumulated_stats->higheststreak, accumulated_stats->notesperseconds
            );
        if (textsprite2)
            textsprite_set_text_formated(
                textsprite2, WEEKRESULT_SONG_TEXT2,
                // week resume
                attempts, accumulated_stats->score, accumulated_stats->accuracy
            );
    } else {
        StatsAccumulated* accumulated_stats = &weekresult->accumulated_stats_week;
        float64 average_accuracy = accumulated_stats->accuracy / songs_count;

        if (textsprite1)
            textsprite_set_text_formated(
                textsprite1, WEEKRESULT_WEEK_TEXT,
                // judgements
                accumulated_stats->sick, accumulated_stats->good, accumulated_stats->bads,

                // streak
                accumulated_stats->shits, accumulated_stats->miss, accumulated_stats->penalties,

                accumulated_stats->combobreaks, accumulated_stats->higheststreak
            );
        if (textsprite2)
            textsprite_set_text_formated(
                textsprite2, WEEKRESULT_WEEK_TEXT2,
                // week resume
                attempts, accumulated_stats->score, average_accuracy
            );
    }

    bool transition = false;
    Gamepad controller = gamepad_init(-1);
    gamepad_clear_buttons(controller);

    while (true) {
        float elapsed = pvrctx_wait_ready();
        pvr_context_reset(&pvr_context);

        if (roundcontext->script) weekscript_notify_frame(roundcontext->script, elapsed);

        if (roundcontext->layout) {
            layout_animate(roundcontext->layout, elapsed);
            layout_draw(roundcontext->layout, &pvr_context);
        }

        layout_animate(layout, elapsed);
        layout_draw(layout, &pvr_context);

        ModdingHelperResult res = modding_helper_handle_custom_menu(modding, controller, elapsed);
        if (res != ModdingHelperResult_CONTINUE || modding->has_exit) break;
        if (modding->has_halt) continue;

        if (roundcontext->script && roundcontext->scriptcontext.halt_flag) {
            gamepad_clear_buttons(controller);
            continue;
        }

        if (transition) {
            if (transition_delay > 0.0f) {
                transition_delay -= elapsed;
                if (transition_delay < 0.0f) layout_trigger_any(layout, "transition");
                continue;
            }
            if (layout_animation_is_completed(layout, "transition_effect")) break;
        } else if (gamepad_get_pressed(controller)) {
            transition = true;
            layout_set_group_visibility(layout, "allstats", false);
        }
    }

    modding_helper_notify_exit2(modding);
    modding_destroy(&modding);

    weekresult->active_layout = NULL;

    gamepad_destroy(&controller);
    layout_destroy(&layout);
}

Layout week_result_get_layout(WeekResult weekresult) {
    return weekresult->active_layout;
}

void week_result_get_accumulated_stats(WeekResult weekresult, WeekResult_Stats* stats) {
    stats->sick = weekresult->accumulated_stats_week.sick;
    stats->good = weekresult->accumulated_stats_week.good;
    stats->bads = weekresult->accumulated_stats_week.bads;
    stats->shits = weekresult->accumulated_stats_week.shits;
    stats->miss = weekresult->accumulated_stats_week.miss;
    stats->penalties = weekresult->accumulated_stats_week.penalties;
    stats->score = weekresult->accumulated_stats_week.score;
    stats->accuracy = weekresult->accumulated_stats_week.accuracy;
    stats->notesperseconds = weekresult->accumulated_stats_week.notesperseconds;
    stats->combobreaks = weekresult->accumulated_stats_week.combobreaks;
    stats->higheststreak = weekresult->accumulated_stats_week.higheststreak;
}
