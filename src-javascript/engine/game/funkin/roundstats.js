"use strict"

const ROUNDSTATS_FONT_COLOR = 0xFFFFFF;// white
const ROUNDSTATS_FONT_BORDER_COLOR = 0x000000FF;// black
const ROUNDSTATS_FONT_BORDER_SIZE = 2.5;
const ROUNDSTATS_SEPARATOR = " | ";

function roundstats_init(x, y, z, fontholder, font_size, layout_width) {
    const STRUCT = {
        tweenkeyframe: null,
        rollback_beats: 0,
        duration_rollback: 0,
        beat_duration: 0,
        rollback_active: 0,
        target_elapsed: 0,
        target_duration: 0,
        is_completed: 0
    };

    let roundstats = {
        textsprite: textsprite_init2(fontholder, font_size, ROUNDSTATS_FONT_COLOR),
        next_clear: 0,
        drawable: null,
        drawable_animation: null,
        builder: stringbuilder_init(128),

        beatwatcher: {},

        tweenkeyframe_active: null,// this can be "tweenkeyframe_hit" or "tweenkeyframe_miss"
        tweenkeyframe_multiplier: 1,

        last_count_hit: 0,
        last_count_miss: 0,

        // in C clear all struct fields
        tweenkeyframe_beat: clone_struct(STRUCT),
        tweenkeyframe_hit: clone_struct(STRUCT),
        tweenkeyframe_miss: clone_struct(STRUCT),
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
    roundstats.tweenkeyframe_active = null;
    roundstats.tweenkeyframe_multiplier = 1;
    roundstats.last_count_hit = 0;
    roundstats.last_count_miss = 0;

    beatwatcher_reset(roundstats.beatwatcher, 1, 100);

    drawable_set_antialiasing(roundstats.drawable, PVR_FLAG_DEFAULT);

    roundstats_internal_tweenkeyframe_setup(roundstats.tweenkeyframe_beat, 0);
    roundstats_internal_tweenkeyframe_setup(roundstats.tweenkeyframe_hit, 0);
    roundstats_internal_tweenkeyframe_setup(roundstats.tweenkeyframe_miss, 0);
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
    // configure the active tweenkeyframe
    //
    let miss_count = playerstats_get_misses(playerstats);
    let hit_count = playerstats_get_hits(playerstats);
    let tweenkeyframe_active = null;
    let tweenkeyframe_multiplier = 1;

    if (miss_count > roundstats.last_count_miss) {
        tweenkeyframe_active = roundstats.tweenkeyframe_miss;
        tweenkeyframe_multiplier = roundstats.last_count_miss - miss_count;
    } else if (hit_count > roundstats.last_count_hit) {
        tweenkeyframe_active = roundstats.tweenkeyframe_hit;
        tweenkeyframe_multiplier = roundstats.last_count_hit - hit_count;
    } else {
        return;
    }

    roundstats.last_count_miss = miss_count;
    roundstats.last_count_hit = hit_count;

    if (tweenkeyframe_active != roundstats.tweenkeyframe_active) {
        roundstats.tweenkeyframe_active = tweenkeyframe_active;
        roundstats.tweenkeyframe_multiplier = tweenkeyframe_multiplier;
    } else {
        roundstats.tweenkeyframe_multiplier += tweenkeyframe_multiplier;
    }
    roundstats_internal_tweenkeyframe_setup(tweenkeyframe_active, 0);
}


function roundstats_tweenkeyframe_set_on_beat(roundstats, tweenkeyframe, rollback_beats, beat_duration) {
    roundstats_internal_tweenkeyframe_set(roundstats.tweenkeyframe_beat, tweenkeyframe, rollback_beats, beat_duration);
    roundstats_internal_tweenkeyframe_duration(roundstats.tweenkeyframe_beat, roundstats.beatwatcher.tick);
}

function roundstats_tweenkeyframe_set_on_hit(roundstats, tweenkeyframe, rollback_beats, beat_duration) {
    roundstats_internal_tweenkeyframe_set(roundstats.tweenkeyframe_hit, tweenkeyframe, rollback_beats, beat_duration);
    roundstats_internal_tweenkeyframe_duration(roundstats.tweenkeyframe_hit, roundstats.beatwatcher.tick);
}

function roundstats_tweenkeyframe_set_on_miss(roundstats, tweenkeyframe, rollback_beats, beat_duration) {
    roundstats_internal_tweenkeyframe_set(roundstats.tweenkeyframe_miss, tweenkeyframe, rollback_beats, beat_duration);
    roundstats_internal_tweenkeyframe_duration(roundstats.tweenkeyframe_miss, roundstats.beatwatcher.tick);
}

function roundstats_tweenkeyframe_set_bpm(roundstats, beats_per_minute) {
    beatwatcher_change_bpm(roundstats.beatwatcher, beats_per_minute);

    roundstats_internal_tweenkeyframe_duration(roundstats.tweenkeyframe_beat, roundstats.beatwatcher.tick);
    roundstats_internal_tweenkeyframe_duration(roundstats.tweenkeyframe_hit, roundstats.beatwatcher.tick);
    roundstats_internal_tweenkeyframe_duration(roundstats.tweenkeyframe_miss, roundstats.beatwatcher.tick);
}


function roundstats_animate(roundstats, elapsed) {
    if (roundstats.drawable_animation) animsprite_animate(roundstats.drawable_animation, elapsed);

    let completed = 1;
    let ignore_beat = !roundstats.tweenkeyframe_beat.tweenkeyframe;// ignore if there no tweenkeyframe for beats
    let beat_rollback_active = roundstats.tweenkeyframe_beat.rollback_active;

    if (roundstats.tweenkeyframe_active) {
        completed = roundstats_internal_tweenkeyframe_run(roundstats, roundstats.tweenkeyframe_active, elapsed);

        // if the active tweenkeyframe is completed, do rollback
        if (!roundstats.tweenkeyframe_active.rollback_active) {
            if (completed) {
                roundstats_internal_tweenkeyframe_setup(roundstats.tweenkeyframe_active, 1);
            } else if (!ignore_beat && !beat_rollback_active) {
                // still running, ignore beat tweernlerp
                beat_rollback_active = 1;
                roundstats.tweenkeyframe_beat.rollback_active = 1;
            }
        }
    }

    // beat check
    if (beatwatcher_poll(roundstats.beatwatcher)) {
        elapsed += roundstats.beatwatcher.since;
        roundstats_internal_tweenkeyframe_setup(roundstats.tweenkeyframe_beat, 0);// run again
    }

    if (!ignore_beat) {
        completed = roundstats_internal_tweenkeyframe_run(roundstats, roundstats.tweenkeyframe_beat, elapsed);
        if (completed && !beat_rollback_active)
            roundstats_internal_tweenkeyframe_setup(roundstats.tweenkeyframe_beat, 1);// do rollback
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



function roundstats_internal_tweenkeyframe_set(tweenkeyframe_info, tweenkeyframe, rollback_beats, beat_duration) {
    if (tweenkeyframe_info.tweenkeyframe) tweenkeyframe_destroy(tweenkeyframe_info.tweenkeyframe);
    tweenkeyframe_info.tweenkeyframe = tweenkeyframe_clone(tweenkeyframe);
    tweenkeyframe_info.rollback_beats = rollback_beats;
    tweenkeyframe_info.beat_duration = beat_duration;
}

function roundstats_internal_tweenkeyframe_duration(tweenkeyframe_info, beat_duration) {
    tweenkeyframe_info.duration_rollback = tweenkeyframe_info.rollback_beats * beat_duration;
    tweenkeyframe_info.beat_duration = beat_duration * tweenkeyframe_info.beat_duration;
    roundstats_internal_tweenkeyframe_setup(tweenkeyframe_info, tweenkeyframe_info.rollback_active);
}

function roundstats_internal_tweenkeyframe_run(roundstats, tweenkeyframe_info, elapsed) {
    const entry = [/*id*/0, /*value*/0];

    if (!tweenkeyframe_info.tweenkeyframe || tweenkeyframe_info.is_completed) return 1;

    tweenkeyframe_info.target_elapsed += elapsed;

    let percent = tweenkeyframe_info.target_elapsed / tweenkeyframe_info.target_duration;
    tweenkeyframe_info.is_completed = percent >= 1.0;

    if (percent > 1.0) percent = 1.0;

    if (tweenkeyframe_info.rollback_active) percent = 1.0 - percent;

    tweenkeyframe_animate_percent(tweenkeyframe_info.tweenkeyframe, percent);

    let count = tweenkeyframe_get_ids_count(tweenkeyframe_info.tweenkeyframe);

    for (let i = 0; i < count; i++) {
        tweenkeyframe_peek_entry_by_index(tweenkeyframe_info.tweenkeyframe, i, entry);
        if (entry[0] == TEXTSPRITE_PROP_STRING) continue;// illegal property

        if (!vertexprops_is_property_enumerable(entry[0]))
            entry[1] *= roundstats.tweenkeyframe_multiplier;

        textsprite_set_property(roundstats.drawable, entry[0], entry[1]);
    }

    return tweenkeyframe_info.is_completed;
}

function roundstats_internal_tweenkeyframe_setup(tweenkeyframe_info, rollback) {
    tweenkeyframe_info.is_completed = 0;
    tweenkeyframe_info.rollback_active = rollback;
    tweenkeyframe_info.target_elapsed = 0;
    tweenkeyframe_info.target_duration = rollback ? tweenkeyframe_info.rollback_duration : tweenkeyframe_info.beat_duration;
    return tweenkeyframe_info.tweenkeyframe == null;
}

