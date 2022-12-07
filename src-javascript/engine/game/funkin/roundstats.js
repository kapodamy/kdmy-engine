"use strict"

const ROUNDSTATS_FONT_COLOR = 0xFFFFFF;// white
const ROUNDSTATS_FONT_BORDER_COLOR = 0x000000FF;// black
const ROUNDSTATS_FONT_BORDER_SIZE = 1;// black
const ROUNDSTATS_SEPARATOR = " | ";

function roundstats_init(x, y, z, fontholder, font_size, layout_width) {
    const STRUCT = {
        tweenlerp: null,
        rollback_beats: 0,
        duration_rollback: 0,
        beat_duration: 0,
        rollback_active: 0
    };

    let roundstats = {
        textsprite: textsprite_init2(fontholder, font_size, ROUNDSTATS_FONT_COLOR),
        next_clear: 0,
        drawable: null,
        drawable_animation: null,
        builder: stringbuilder_init(128),

        beatwatcher: {},

        tweenlerp_active: null,// this can be "tweenlerp_hit" or "tweenlerp_miss"
        tweenlerp_multiplier: 1,

        last_count_hit: 0,
        last_count_miss: 0,

        // in C clear all struct fields
        tweenlerp_beat: clone_struct(STRUCT),
        tweenlerp_hit: clone_struct(STRUCT),
        tweenlerp_miss: clone_struct(STRUCT),
        enable_nps: 0
    };

    beatwatcher_reset(roundstats.beatwatcher, 1, 100);

    roundstats.drawable = drawable_init(z, roundstats, roundstats_draw, roundstats_animate);

    let modifier = drawable_get_modifier(roundstats.drawable);
    modifier.x = x;
    modifier.y = y;

    textsprite_border_enable(roundstats.textsprite, 1);
    textsprite_border_set_color_rgba8(roundstats.textsprite, ROUNDSTATS_FONT_BORDER_COLOR);
    textsprite_border_set_size(roundstats.textsprite, ROUNDSTATS_FONT_BORDER_SIZE);
    textsprite_set_visible(roundstats.textsprite, 0);
    textsprite_set_draw_location(roundstats.textsprite, x, y);
    textsprite_set_max_draw_size(roundstats.textsprite, layout_width, -1);
    textsprite_set_align(roundstats.textsprite, ALIGN_START, ALIGN_CENTER);

    roundstats_reset(roundstats);

    return roundstats;
}

function roundstats_destroy(roundstats) {
    ModuleLuaScript.kdmyEngine_drop_shared_object(roundstats);

    drawable_destroy(roundstats.drawable);
    if (roundstats.drawable_animation) animsprite_destroy(roundstats.drawable_animation);
    textsprite_destroy(roundstats.textsprite);
    stringbuilder_destroy(roundstats.builder);
    roundstats = undefined;
}


function roundstats_hide(roundstats, hide) {
    drawable_set_visible(roundstats.drawable, !hide);
}

function roundstats_hide_nps(roundstats, hide) {
    roundstats.enable_nps = !!hide;
}

function roundstats_set_draw_y(roundstats, y) {
    const modifier = drawable_get_modifier(roundstats.drawable);
    modifier.y = y;
    textsprite_set_draw_location(roundstats.textsprite, null, y);
}

function roundstats_reset(roundstats) {
    roundstats.next_clear = 1000;
    roundstats.tweenlerp_active = null;
    roundstats.tweenlerp_multiplier = 1;
    roundstats.last_count_hit = 0;
    roundstats.last_count_miss = 0;

    beatwatcher_reset(roundstats.beatwatcher, 1, 100);

    drawable_set_antialiasing(roundstats.drawable, PVR_FLAG_DEFAULT);

    roundstats_internal_tweenlerp_setup(roundstats.tweenlerp_beat, 0);
    roundstats_internal_tweenlerp_setup(roundstats.tweenlerp_hit, 0);
    roundstats_internal_tweenlerp_setup(roundstats.tweenlerp_miss, 0);
}

function roundstats_get_drawable(roundstats) {
    return roundstats.drawable;
}

function roundstats_peek_playerstats(roundstats, song_timestamp, playerstats) {
    let builder = roundstats.builder;
    let accuracy = playerstats_get_accuracy(playerstats);

    stringbuilder_clear(builder);

    if (roundstats.enable_nps) {
        let nps = playerstats_get_notes_per_seconds(playerstats);
        let nps_max = playerstats_get_notes_per_seconds_highest(playerstats);
        stringbuilder_add_format(builder, "NPS: $i (Max $i)", nps, nps_max);
        stringbuilder_add(builder, ROUNDSTATS_SEPARATOR);
    }

    if (roundstats.next_clear > song_timestamp) {
        roundstats.next_clear += 1000;
        playerstats_reset_notes_per_seconds(playerstats);
    }

    stringbuilder_add_format(builder, "Score: $l", playerstats_get_score(playerstats));
    stringbuilder_add(builder, ROUNDSTATS_SEPARATOR);

    stringbuilder_add_format(builder, "CB: $i", playerstats_get_combo_breaks(playerstats));
    stringbuilder_add(builder, ROUNDSTATS_SEPARATOR);

    stringbuilder_add_format(builder, "Accuracy: $2d%", accuracy);
    stringbuilder_add(builder, ROUNDSTATS_SEPARATOR);

    if (accuracy > 0) {
        stringbuilder_add_format(
            builder, " $s $s",
            funkin_get_letter_rank(playerstats),
            funking_get_wife3_accuracy(playerstats)
        );
    } else {
        stringbuilder_add(builder, FUNKIN_NO_ACCURACY);
    }

    //stringbuilder_add(builder, ROUNDSTATS_SEPARATOR);

    // now use the builded stats string
    textsprite_set_text_intern(roundstats.textsprite, 1, stringbuilder_intern(builder));

    //
    // configure the active tweenlerp
    //
    let miss_count = playerstats_get_misses(playerstats);
    let hit_count = playerstats_get_hits(playerstats);
    let tweenlerp_active = null;
    let tweenlerp_multiplier = 1;

    if (miss_count > roundstats.last_count_miss) {
        tweenlerp_active = roundstats.tweenlerp_miss;
        tweenlerp_multiplier = roundstats.last_count_miss - miss_count;
    } else if (hit_count > roundstats.last_count_hit) {
        tweenlerp_active = roundstats.tweenlerp_hit;
        tweenlerp_multiplier = roundstats.last_count_hit - hit_count;
    } else {
        return;
    }

    roundstats.last_count_miss = miss_count;
    roundstats.last_count_hit = hit_count;

    if (tweenlerp_active != roundstats.tweenlerp_active) {
        roundstats.tweenlerp_active = tweenlerp_active;
        roundstats.tweenlerp_multiplier = tweenlerp_multiplier;
        roundstats_internal_tweenlerp_setup(tweenlerp_active, 0);
    } else {
        roundstats.tweenlerp_multiplier += tweenlerp_multiplier;
        if (roundstats.tweenlerp_active.tweenlerp) tweenlerp_restart(roundstats.tweenlerp_active.tweenlerp);
    }
}


function roundstats_tweenlerp_set_on_beat(roundstats, tweenlerp, rollback_beats, beat_duration) {
    roundstats_internal_tweenlerp_set(roundstats.tweenlerp_beat, tweenlerp, rollback_beats, beat_duration);
    roundstats_internal_tweenlerp_duration(roundstats.tweenlerp_beat, roundstats.beatwatcher.tick);
}

function roundstats_tweenlerp_set_on_hit(roundstats, tweenlerp, rollback_beats, beat_duration) {
    roundstats_internal_tweenlerp_set(roundstats.tweenlerp_hit, tweenlerp, rollback_beats, beat_duration);
    roundstats_internal_tweenlerp_duration(roundstats.tweenlerp_hit, roundstats.beatwatcher.tick);
}

function roundstats_tweenlerp_set_on_miss(roundstats, tweenlerp, rollback_beats, beat_duration) {
    roundstats_internal_tweenlerp_set(roundstats.tweenlerp_miss, tweenlerp, rollback_beats, beat_duration);
    roundstats_internal_tweenlerp_duration(roundstats.tweenlerp_miss, roundstats.beatwatcher.tick);
}

function roundstats_tweenlerp_set_bpm(roundstats, beats_per_minute) {
    beatwatcher_change_bpm(roundstats.beatwatcher, beats_per_minute);

    roundstats_internal_tweenlerp_duration(roundstats.tweenlerp_beat, roundstats.beatwatcher.tick);
    roundstats_internal_tweenlerp_duration(roundstats.tweenlerp_hit, roundstats.beatwatcher.tick);
    roundstats_internal_tweenlerp_duration(roundstats.tweenlerp_miss, roundstats.beatwatcher.tick);
}


function roundstats_animate(roundstats, elapsed) {
    if (roundstats.drawable_animation) animsprite_animate(roundstats.drawable_animation, elapsed);

    let completed = 1;
    let ignore_beat = !roundstats.tweenlerp_beat.tweenlerp;// ignore if there no tweenlerp for beats
    let beat_rollback_active = roundstats.tweenlerp_beat.rollback_active;

    if (roundstats.tweenlerp_active) {
        completed = roundstats_internal_tweenlerp_run(roundstats, roundstats.tweenlerp_active, elapsed);

        // if the active tweenlerp is completed, do rollback
        if (!roundstats.tweenlerp_active.rollback_active) {
            if (completed) {
                roundstats_internal_tweenlerp_setup(roundstats.tweenlerp_active, 1);
            } else if (!ignore_beat && !beat_rollback_active) {
                // still running, ignore beat tweernlerp
                beat_rollback_active = 1;
                roundstats.tweenlerp_beat.rollback_active = 1;
                tweenlerp_mark_as_completed(roundstats.tweenlerp_beat.tweenlerp);
            }
        }
    }

    // beat check
    if (beatwatcher_poll(roundstats.beatwatcher)) {
        elapsed += roundstats.beatwatcher.since;
        roundstats_internal_tweenlerp_setup(roundstats.tweenlerp_beat, 0);// run again
    }

    if (!ignore_beat) {
        completed = roundstats_internal_tweenlerp_run(roundstats, roundstats.tweenlerp_beat, elapsed);
        if (completed && !beat_rollback_active)
            roundstats_internal_tweenlerp_setup(roundstats.tweenlerp_beat, 1);// do rollback
    }

    const draw_size = [0, 0];
    const modifier = drawable_get_modifier(roundstats.drawable);
    textsprite_get_draw_size(roundstats.textsprite, draw_size);
    modifier.width = draw_size[0];
    modifier.height = draw_size[1];

    return completed;
}

function roundstats_draw(roundstats, pvrctx) {
    pvr_context_save(pvrctx);
    drawable_helper_apply_in_context(roundstats.drawable, pvrctx);
    textsprite_draw(roundstats.textsprite, pvrctx);
    pvr_context_restore(pvrctx);
}



function roundstats_internal_tweenlerp_set(tweenlerp_info, tweenlerp, rollback_beats, beat_duration) {
    if (tweenlerp_info.tweenlerp) tweenlerp_destroy(tweenlerp_info.tweenlerp);
    tweenlerp_info.tweenlerp = tweenlerp_clone(tweenlerp);
    tweenlerp_info.rollback_beats = rollback_beats;
    tweenlerp_info.beat_duration = beat_duration;
}

function roundstats_internal_tweenlerp_duration(tweenlerp_info, beat_duration) {
    tweenlerp_info.duration_rollback = tweenlerp_info.rollback_beats * beat_duration;
    tweenlerp_info.beat_duration = beat_duration * tweenlerp_info.beat_duration;
    roundstats_internal_tweenlerp_setup(tweenlerp_info, tweenlerp_info.rollback_active);
}

function roundstats_internal_tweenlerp_run(roundstats, tweenlerp_info, elapsed) {
    const entry = [/*id*/0, /*value*/0, /*duration*/0];

    if (!tweenlerp_info.tweenlerp) return 1;
    if (tweenlerp_is_completed(tweenlerp_info.tweenlerp)) return 1;

    let completed = tweenlerp_animate(tweenlerp_info.tweenlerp, elapsed);
    if (completed) return 1;

    let count = tweenlerp_get_entry_count(tweenlerp_info.tweenlerp);

    for (let i = 0; i < count; i++) {
        tweenlerp_peek_entry_by_index(tweenlerp_info.tweenlerp, i, entry);
        if (entry[0] == TEXTSPRITE_PROP_STRING) continue;// illegal property

        if (!vertexprops_is_property_enumerable(entry[0]))
            entry[1] *= roundstats.tweenlerp_multiplier;

        textsprite_set_property(roundstats.drawable, entry[0], entry[1]);
    }

    return 0;
}

function roundstats_internal_tweenlerp_setup(tweenlerp_info, rollback) {
    const entry = [/*id*/0, /*value*/0, /*duration*/0];

    tweenlerp_info.rollback_active = rollback;

    if (!tweenlerp_info.tweenlerp) return 1;

    let count = tweenlerp_get_entry_count(tweenlerp_info.tweenlerp);
    let new_duration = rollback ? tweenlerp_info.duration_rollback : tweenlerp_info.beat_duration;

    for (let i = 0; i < count; i++) {
        tweenlerp_swap_bounds_by_index(tweenlerp_info.tweenlerp, i);
        tweenlerp_peek_entry_by_index(tweenlerp_info.tweenlerp, i, entry);

        // only modify entries with negative duration
        if (entry[2] < 0) tweenlerp_change_duration_by_index(tweenlerp_info.tweenlerp, i, new_duration);
    }

    tweenlerp_restart(tweenlerp_info.tweenlerp);
    return 0;
}

