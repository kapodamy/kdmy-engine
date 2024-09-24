#include "game/funkin/roundstats.h"

#include "animsprite.h"
#include "beatwatcher.h"
#include "drawable.h"
#include "externals/luascript.h"
#include "game/common/funkin.h"
#include "game/funkin/playerstats.h"
#include "malloc_utils.h"
#include "math2d.h"
#include "pvrcontext.h"
#include "stringbuilder.h"
#include "textsprite.h"
#include "tweenkeyframe.h"
#include "vertexprops.h"


typedef struct {
    TweenKeyframe tweenkeyframe;
    float rollback_beats;
    float rollback_duration;
    float beat_duration;
    bool rollback_active;
    float target_elapsed;
    float target_duration;
    bool is_completed;
} TweenKeyframeInfo;

struct RoundStats_s {
    TextSprite textsprite;
    float64 next_clear;
    Drawable drawable;
    AnimSprite drawable_animation;
    StringBuilder builder;
    BeatWatcher beatwatcher;
    TweenKeyframeInfo* tweenkeyframe_active;
    float tweenkeyframe_multiplier;
    int32_t last_count_hit;
    int32_t last_count_miss;
    TweenKeyframeInfo tweenkeyframe_beat;
    TweenKeyframeInfo tweenkeyframe_hit;
    TweenKeyframeInfo tweenkeyframe_miss;
    bool enable_nps;
};


static const uint32_t ROUNDSTATS_FONT_COLOR = 0xFFFFFF;          // white
static const uint32_t ROUNDSTATS_FONT_BORDER_COLOR = 0x000000FF; // black
static const char* ROUNDSTATS_SEPARATOR = " | ";


static void roundstats_internal_tweenkeyframe_set(TweenKeyframeInfo* tweenkeyframe_info, TweenKeyframe tweenkeyframe, float rollback_beats, float beat_duration);
static void roundstats_internal_tweenkeyframe_duration(TweenKeyframeInfo* tweenkeyframe_info, float beat_duration);
static bool roundstats_internal_tweenkeyframe_run(RoundStats roundstats, TweenKeyframeInfo* tweenkeyframe_info, float elapsed);
static bool roundstats_internal_tweenkeyframe_setup(TweenKeyframeInfo* tweenkeyframe_info, bool rollback);


RoundStats roundstats_init(float x, float y, float z, FontHolder fontholder, float font_size, float font_border_size, float layout_width) {
    RoundStats roundstats = malloc_chk(sizeof(struct RoundStats_s));
    malloc_assert(roundstats, RoundStats);

    *roundstats = (struct RoundStats_s){
        .textsprite = textsprite_init2(fontholder, font_size, ROUNDSTATS_FONT_COLOR),
        .next_clear = 0.0,
        .drawable = NULL,
        .drawable_animation = NULL,
        .builder = stringbuilder_init(128),

        .beatwatcher = {},

        .tweenkeyframe_active = NULL, // this can be "tweenkeyframe_hit" or "tweenkeyframe_miss"
        .tweenkeyframe_multiplier = 1.0f,

        .last_count_hit = 0,
        .last_count_miss = 0,

        //.tweenkeyframe_beat = (TweenKeyframeInfo){},
        //.tweenkeyframe_hit = (TweenKeyframeInfo){},
        //.tweenkeyframe_miss = (TweenKeyframeInfo){},

        .enable_nps = false
    };

    memset(&roundstats->tweenkeyframe_beat, 0x00, sizeof(TweenKeyframeInfo));
    memset(&roundstats->tweenkeyframe_hit, 0x00, sizeof(TweenKeyframeInfo));
    memset(&roundstats->tweenkeyframe_miss, 0x00, sizeof(TweenKeyframeInfo));

    beatwatcher_reset(&roundstats->beatwatcher, true, 100.0f);

    roundstats->drawable = drawable_init(z, roundstats, (DelegateDraw)roundstats_draw, (DelegateAnimate)roundstats_animate);

    Modifier* modifier = drawable_get_modifier(roundstats->drawable);
    modifier->x = x;
    modifier->y = y;

    textsprite_border_enable(roundstats->textsprite, true);
    textsprite_border_set_color_rgba8(roundstats->textsprite, ROUNDSTATS_FONT_BORDER_COLOR);
    textsprite_border_set_size(roundstats->textsprite, font_border_size);
    textsprite_set_visible(roundstats->textsprite, false);
    textsprite_set_draw_location(roundstats->textsprite, x, y);
    textsprite_set_max_draw_size(roundstats->textsprite, layout_width, -1.0f);
    textsprite_set_align(roundstats->textsprite, ALIGN_START, ALIGN_CENTER);

    roundstats_reset(roundstats);

    return roundstats;
}

void roundstats_destroy(RoundStats* roundstats_ptr) {
    if (!roundstats_ptr || !*roundstats_ptr) return;

    RoundStats roundstats = *roundstats_ptr;

    luascript_drop_shared(roundstats);

    drawable_destroy(&roundstats->drawable);
    if (roundstats->drawable_animation) animsprite_destroy(&roundstats->drawable_animation);
    textsprite_destroy(&roundstats->textsprite);
    stringbuilder_destroy(&roundstats->builder);

    free_chk(roundstats);
    *roundstats_ptr = NULL;
}


void roundstats_hide(RoundStats roundstats, bool hide) {
    drawable_set_visible(roundstats->drawable, !hide);
}

void roundstats_hide_nps(RoundStats roundstats, bool hide) {
    roundstats->enable_nps = hide;
}

void roundstats_set_draw_y(RoundStats roundstats, float y) {
    Modifier* modifier = drawable_get_modifier(roundstats->drawable);
    modifier->y = y;
    textsprite_set_draw_location(roundstats->textsprite, FLOAT_NaN, y);
}

void roundstats_reset(RoundStats roundstats) {
    roundstats->next_clear = 1000.0;
    roundstats->tweenkeyframe_active = NULL;
    roundstats->tweenkeyframe_multiplier = 1.0f;
    roundstats->last_count_hit = 0;
    roundstats->last_count_miss = 0;

    beatwatcher_reset(&roundstats->beatwatcher, true, 100.0f);

    drawable_set_antialiasing(roundstats->drawable, PVRCTX_FLAG_DEFAULT);

    roundstats_internal_tweenkeyframe_setup(&roundstats->tweenkeyframe_beat, false);
    roundstats_internal_tweenkeyframe_setup(&roundstats->tweenkeyframe_hit, false);
    roundstats_internal_tweenkeyframe_setup(&roundstats->tweenkeyframe_miss, false);
}

Drawable roundstats_get_drawable(RoundStats roundstats) {
    return roundstats->drawable;
}

void roundstats_peek_playerstats(RoundStats roundstats, float64 song_timestamp, PlayerStats playerstats) {
    StringBuilder builder = roundstats->builder;
    float64 accuracy = playerstats_get_accuracy(playerstats);

    stringbuilder_clear(builder);

    if (roundstats->enable_nps) {
        int32_t nps = playerstats_get_notes_per_seconds(playerstats);
        int32_t nps_max = playerstats_get_notes_per_seconds_highest(playerstats);
        stringbuilder_add_format(builder, "NPS: $i (Max $i)", nps, nps_max);
        stringbuilder_add(builder, ROUNDSTATS_SEPARATOR);
    }

    if (roundstats->next_clear > song_timestamp) {
        roundstats->next_clear += 1000.0;
        playerstats_reset_notes_per_seconds(playerstats);
    }

    stringbuilder_add_format(builder, "Score: $l", playerstats_get_score(playerstats));
    stringbuilder_add(builder, ROUNDSTATS_SEPARATOR);

    stringbuilder_add_format(builder, "CB: $i", playerstats_get_combo_breaks(playerstats));
    stringbuilder_add(builder, ROUNDSTATS_SEPARATOR);

    stringbuilder_add_format(builder, "Accuracy: $2d%", accuracy);
    stringbuilder_add(builder, ROUNDSTATS_SEPARATOR);

    if (accuracy > 0.0) {
        stringbuilder_add_format(
            builder, " $s $s",
            funkin_get_letter_rank(playerstats),
            funkin_get_wife3_accuracy(playerstats)
        );
    } else {
        stringbuilder_add(builder, FUNKIN_NO_ACCURACY);
    }

    // stringbuilder_add(builder, ROUNDSTATS_SEPARATOR);

    // now use the builded stats string
    textsprite_set_text_intern(roundstats->textsprite, true, stringbuilder_intern(builder));

    //
    // configure the active tweenkeyframe
    //
    int32_t miss_count = playerstats_get_misses(playerstats);
    int32_t hit_count = playerstats_get_hits(playerstats);
    TweenKeyframeInfo* tweenkeyframe_active = NULL;
    int32_t tweenkeyframe_multiplier = 1;

    if (miss_count > roundstats->last_count_miss) {
        tweenkeyframe_active = &roundstats->tweenkeyframe_miss;
        tweenkeyframe_multiplier = roundstats->last_count_miss - miss_count;
    } else if (hit_count > roundstats->last_count_hit) {
        tweenkeyframe_active = &roundstats->tweenkeyframe_hit;
        tweenkeyframe_multiplier = roundstats->last_count_hit - hit_count;
    } else {
        return;
    }

    roundstats->last_count_miss = miss_count;
    roundstats->last_count_hit = hit_count;

    if (tweenkeyframe_active != roundstats->tweenkeyframe_active) {
        roundstats->tweenkeyframe_active = tweenkeyframe_active;
        roundstats->tweenkeyframe_multiplier = tweenkeyframe_multiplier;
    } else {
        roundstats->tweenkeyframe_multiplier += tweenkeyframe_multiplier;
    }
    roundstats_internal_tweenkeyframe_setup(tweenkeyframe_active, false);
}


void roundstats_tweenkeyframe_set_on_beat(RoundStats roundstats, TweenKeyframe tweenkeyframe, float rollback_beats, float beat_duration) {
    roundstats_internal_tweenkeyframe_set(&roundstats->tweenkeyframe_beat, tweenkeyframe, rollback_beats, beat_duration);
    roundstats_internal_tweenkeyframe_duration(&roundstats->tweenkeyframe_beat, roundstats->beatwatcher.tick);
}

void roundstats_tweenkeyframe_set_on_hit(RoundStats roundstats, TweenKeyframe tweenkeyframe, float rollback_beats, float beat_duration) {
    roundstats_internal_tweenkeyframe_set(&roundstats->tweenkeyframe_hit, tweenkeyframe, rollback_beats, beat_duration);
    roundstats_internal_tweenkeyframe_duration(&roundstats->tweenkeyframe_hit, roundstats->beatwatcher.tick);
}

void roundstats_tweenkeyframe_set_on_miss(RoundStats roundstats, TweenKeyframe tweenkeyframe, float rollback_beats, float beat_duration) {
    roundstats_internal_tweenkeyframe_set(&roundstats->tweenkeyframe_miss, tweenkeyframe, rollback_beats, beat_duration);
    roundstats_internal_tweenkeyframe_duration(&roundstats->tweenkeyframe_miss, roundstats->beatwatcher.tick);
}

void roundstats_tweenkeyframe_set_bpm(RoundStats roundstats, float beats_per_minute) {
    beatwatcher_change_bpm(&roundstats->beatwatcher, beats_per_minute);

    roundstats_internal_tweenkeyframe_duration(&roundstats->tweenkeyframe_beat, roundstats->beatwatcher.tick);
    roundstats_internal_tweenkeyframe_duration(&roundstats->tweenkeyframe_hit, roundstats->beatwatcher.tick);
    roundstats_internal_tweenkeyframe_duration(&roundstats->tweenkeyframe_miss, roundstats->beatwatcher.tick);
}


int32_t roundstats_animate(RoundStats roundstats, float elapsed) {
    if (roundstats->drawable_animation) animsprite_animate(roundstats->drawable_animation, elapsed);

    int32_t completed = 1;
    bool ignore_beat = !roundstats->tweenkeyframe_beat.tweenkeyframe; // ignore if there no tweenkeyframe for beats
    bool beat_rollback_active = roundstats->tweenkeyframe_beat.rollback_active;

    if (roundstats->tweenkeyframe_active) {
        completed = roundstats_internal_tweenkeyframe_run(roundstats, roundstats->tweenkeyframe_active, elapsed);

        // if the active tweenkeyframe is completed, do rollback
        if (!roundstats->tweenkeyframe_active->rollback_active) {
            if (completed) {
                roundstats_internal_tweenkeyframe_setup(roundstats->tweenkeyframe_active, true);
            } else if (!ignore_beat && !beat_rollback_active) {
                // still running, ignore beat tweernlerp
                beat_rollback_active = true;
                roundstats->tweenkeyframe_beat.rollback_active = true;
            }
        }
    }

    // beat check
    if (beatwatcher_poll(&roundstats->beatwatcher)) {
        elapsed += roundstats->beatwatcher.since;
        roundstats_internal_tweenkeyframe_setup(&roundstats->tweenkeyframe_beat, false); // run again
    }

    if (!ignore_beat) {
        completed = roundstats_internal_tweenkeyframe_run(roundstats, &roundstats->tweenkeyframe_beat, elapsed);
        if (completed && !beat_rollback_active)
            roundstats_internal_tweenkeyframe_setup(&roundstats->tweenkeyframe_beat, true); // do rollback
    }

    float draw_width = 0.0f, draw_height = 0.0f;
    Modifier* modifier = drawable_get_modifier(roundstats->drawable);
    textsprite_get_draw_size(roundstats->textsprite, &draw_width, &draw_height);
    modifier->width = draw_width;
    modifier->height = draw_height;

    return completed;
}

void roundstats_draw(RoundStats roundstats, PVRContext pvrctx) {
    pvr_context_save(pvrctx);
    drawable_helper_apply_in_context(roundstats->drawable, pvrctx);
    textsprite_draw(roundstats->textsprite, pvrctx);
    pvr_context_restore(pvrctx);
}



static void roundstats_internal_tweenkeyframe_set(TweenKeyframeInfo* tweenkeyframe_info, TweenKeyframe tweenkeyframe, float rollback_beats, float beat_duration) {
    if (tweenkeyframe_info->tweenkeyframe) tweenkeyframe_destroy(&tweenkeyframe_info->tweenkeyframe);
    tweenkeyframe_info->tweenkeyframe = tweenkeyframe_clone(tweenkeyframe);
    tweenkeyframe_info->rollback_beats = rollback_beats;
    tweenkeyframe_info->beat_duration = beat_duration;
}

static void roundstats_internal_tweenkeyframe_duration(TweenKeyframeInfo* tweenkeyframe_info, float beat_duration) {
    tweenkeyframe_info->rollback_duration = tweenkeyframe_info->rollback_beats * beat_duration;
    tweenkeyframe_info->beat_duration = beat_duration * tweenkeyframe_info->beat_duration;
    roundstats_internal_tweenkeyframe_setup(tweenkeyframe_info, tweenkeyframe_info->rollback_active);
}

static bool roundstats_internal_tweenkeyframe_run(RoundStats roundstats, TweenKeyframeInfo* tweenkeyframe_info, float elapsed) {
    int32_t entry_id = 0.0f;
    float entry_value = 0.0f;

    if (!tweenkeyframe_info->tweenkeyframe || tweenkeyframe_info->is_completed) return true;

    tweenkeyframe_info->target_elapsed += elapsed;

    float percent = tweenkeyframe_info->target_elapsed / tweenkeyframe_info->target_duration;
    tweenkeyframe_info->is_completed = percent >= 1.0f;

    if (percent > 1.0f) percent = 1.0f;

    if (tweenkeyframe_info->rollback_active) percent = 1.0f - percent;

    tweenkeyframe_animate_percent(tweenkeyframe_info->tweenkeyframe, percent);

    int32_t count = tweenkeyframe_get_ids_count(tweenkeyframe_info->tweenkeyframe);

    for (int32_t i = 0; i < count; i++) {
        tweenkeyframe_peek_entry_by_index(tweenkeyframe_info->tweenkeyframe, i, &entry_id, &entry_value);
        if (entry_id == TEXTSPRITE_PROP_STRING) continue; // illegal property

        if (!vertexprops_is_property_enumerable(entry_id))
            entry_value *= roundstats->tweenkeyframe_multiplier;

        drawable_set_property(roundstats->drawable, entry_id, entry_value);
    }

    return tweenkeyframe_info->is_completed;
}

static bool roundstats_internal_tweenkeyframe_setup(TweenKeyframeInfo* tweenkeyframe_info, bool rollback) {
    tweenkeyframe_info->is_completed = false;
    tweenkeyframe_info->rollback_active = rollback;
    tweenkeyframe_info->target_elapsed = 0.0f;
    tweenkeyframe_info->target_duration = rollback ? tweenkeyframe_info->rollback_duration : tweenkeyframe_info->beat_duration;
    return tweenkeyframe_info->tweenkeyframe == NULL;
}
