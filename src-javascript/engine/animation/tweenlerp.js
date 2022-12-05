"use strict";


function tweenlerp_init() {
    return {
        arraylist: arraylist_init(),
        progress: 0,
        has_completed: 0
    }
}

function tweenlerp_init2(animlist_item) {
    if (!animlist_item.is_tweenlerp) {
        console.error("tweenlerp_init2() the animlist item is not a tweenlerp: " + animlist_item.name);
        return null;
    }

    let tweenlerp = {
        arraylist: arraylist_init2(animlist_item.tweenlerp_entries_count),
        progress: 0,
        has_completed: 0
    }

    for (let i = 0; i < animlist_item.tweenlerp_entries_count; i++) {
        tweenlerp_internal_add(
            tweenlerp,
            animlist_item.tweenlerp_entries[i].id,
            animlist_item.tweenlerp_entries[i].start,
            animlist_item.tweenlerp_entries[i].end,
            animlist_item.tweenlerp_entries[i].duration,
            animlist_item.tweenlerp_entries[i].interp,
            animlist_item.tweenlerp_entries[i].steps_dir,
            animlist_item.tweenlerp_entries[i].steps_count
        );
    }

    return tweenlerp;
}

function tweenlerp_init3(animlist, tweenlerp_name) {
    let animlist_item = null;

    for (let i = 0; i < animlist.entries_count; i++) {
        if (animlist.entries[i].name == tweenlerp_name) {
            animlist_item = animlist.entries[i];
            break;
        }
    }

    if (!animlist_item) {
        console.warn("tweenlerp_init3() the animlist does not contains: " + tweenlerp_name);
        return null;
    }

    return tweenlerp_init2(animlist_item);
}

function tweenlerp_destroy(tweenlerp) {
    arraylist_destroy(tweenlerp.arraylist);
    ModuleLuaScript.kdmyEngine_drop_shared_object(tweenlerp);
    tweenlerp = undefined;
}

function tweenlerp_clone(tweenlerp) {
    if (!tweenlerp) return null;
    let array_size = arraylist_size(tweenlerp.arraylist);
    let copy = {
        arraylist: arraylist_init2(array_size),
        progress: tweenlerp.progress,
        has_completed: tweenlerp.has_completed
    };

    let array_old = arraylist_peek_array(tweenlerp.arraylist);
    let array_new = arraylist_peek_array(copy.arraylist);

    for (let i = 0; i < array_size; i++) {
        array_new[i] = clone_object(array_old[i]);
    }

    return copy;
}


function tweenlerp_end(tweenlerp) {
    const array = arraylist_peek_array(tweenlerp.arraylist);
    const size = arraylist_size(tweenlerp.arraylist);

    for (let i = 0; i < size; i++) {
        if (array[i].duration > tweenlerp.progress) tweenlerp.progress = array[i].duration;
        array[i].value = array[i].end;
    }

    tweenlerp.has_completed = 1;
}

function tweenlerp_mark_as_completed(tweenlerp) {
    tweenlerp.has_completed = 1;
}


function tweenlerp_restart(tweenlerp) {
    tweenlerp.progress = 0;
    tweenlerp.has_completed = 0;
    tweenlerp_animate_percent(tweenlerp, 0);
}

function tweenlerp_animate(tweenlerp, elapsed) {
    if (tweenlerp.has_completed) return 1;

    tweenlerp.progress += elapsed;

    const array = arraylist_peek_array(tweenlerp.arraylist);
    const size = arraylist_size(tweenlerp.arraylist);
    let completed = 0;

    for (let i = 0; i < size; i++) {
        if (tweenlerp_internal_animate_entry(array[i], tweenlerp.progress)) completed++;
    }

    if (completed >= size) tweenlerp.has_completed = 1;

    return 0;// keep last "frame" alive
}

function tweenlerp_animate_timestamp(tweenlerp, timestamp) {
    tweenlerp.progress = timestamp;

    const array = arraylist_peek_array(tweenlerp.arraylist);
    const size = arraylist_size(tweenlerp.arraylist);
    let completed = 0;

    for (let i = 0; i < size; i++) {
        if (tweenlerp_internal_animate_entry(array[i], timestamp)) completed++;
    }

    tweenlerp.has_completed = completed >= size;
    return completed;
}

function tweenlerp_animate_percent(tweenlerp, percent) {
    tweenlerp.progress = -1;// undefined behavoir

    const array = arraylist_peek_array(tweenlerp.arraylist);
    const size = arraylist_size(tweenlerp.arraylist);
    let completed = 0;

    percent = math2d_clamp(percent, 0.0, 1.0);

    for (let i = 0; i < size; i++) {
        if (tweenlerp_internal_animate_entry_absolute(array[i], percent)) completed++;
    }

    tweenlerp.has_completed = completed >= size;
    return completed;
}

function tweenlerp_is_completed(tweenlerp) {
    return tweenlerp.has_completed;
}

function tweenlerp_get_elapsed(tweenlerp) {
    return tweenlerp.progress;
}

function tweenlerp_get_entry_count(tweenlerp) {
    return arraylist_size(tweenlerp.arraylist);
}


function tweenlerp_peek_value(tweenlerp) {
    return arraylist_get(tweenlerp.arraylist, 0).value;
}

function tweenlerp_peek_value_by_index(tweenlerp, index) {
	let entry = arraylist_get(tweenlerp.arraylist, index);
	if(!entry) return NaN;
    return entry.value;
}

function tweenlerp_peek_entry_by_index(tweenlerp, index, output_id_value_pair) {
    let entry = arraylist_get(tweenlerp.arraylist, index);
	if (!entry) return null;
    output_id_value_pair[0] = entry.id;
    output_id_value_pair[1] = entry.value;
    return output_id_value_pair;
}

function tweenlerp_peek_value_by_id(tweenlerp, id) {
    const array = arraylist_peek_array(tweenlerp.arraylist);
    const size = arraylist_size(tweenlerp.arraylist);

    for (let i = 0; i < size; i++) {
        if (array[i].id == id) return array[i].value;
    }

    return NaN;
}

function tweenlerp_change_bounds_by_index(tweenlerp, index, new_start, new_end) {
    let tweenlerp_entry = arraylist_get(tweenlerp.arraylist, index);

    if (Number.isFinite(new_start)) tweenlerp_entry.start = new_start;
    if (Number.isFinite(new_end)) tweenlerp_entry.end = new_end;

    return tweenlerp_internal_animate_entry(tweenlerp_entry, tweenlerp.progress);
}

function tweenlerp_override_start_with_end_by_index(tweenlerp, index) {
    let tweenlerp_entry = arraylist_get(tweenlerp.arraylist, index);

    tweenlerp_entry.start = tweenlerp_entry.end;

    return tweenlerp_internal_animate_entry(tweenlerp_entry, tweenlerp.progress);
}

function tweenlerp_change_bounds_by_id(tweenlerp, id, new_start, new_end) {
    const array = arraylist_peek_array(tweenlerp.arraylist);
    const size = arraylist_size(tweenlerp.arraylist);

    for (let i = 0; i < size; i++) {
        if (array[i].id != id) continue;
        array[i].start = new_start;
        array[i].end = new_end;
        tweenlerp_internal_animate_entry(array[i], tweenlerp.progress);
        return 1;
    }

    return 0;
}

function tweenlerp_change_duration_by_index(tweenlerp, index, new_duration) {
    let tweenlerp_entry = arraylist_get(tweenlerp.arraylist, index);
    tweenlerp_entry.duration = new_duration;

    return tweenlerp_internal_animate_entry(tweenlerp_entry, tweenlerp.progress);
}

function tweenlerp_swap_bounds_by_index(tweenlerp, index) {
    let tweenlerp_entry = arraylist_get(tweenlerp.arraylist, index);
    let temp = tweenlerp_entry.start;

    tweenlerp_entry.start = tweenlerp_entry.end;
    tweenlerp_entry.end = temp;

    return tweenlerp_internal_animate_entry(tweenlerp_entry, tweenlerp.progress);
}


function tweenlerp_add_ease(tweenlerp, id, start, end, duration) {
    return tweenlerp_internal_add(
        tweenlerp, id, start, end, duration, ANIM_MACRO_INTERPOLATOR_EASE, -1, -1
    );
}

function tweenlerp_add_easein(tweenlerp, id, start, end, duration) {
    return tweenlerp_internal_add(
        tweenlerp, id, start, end, duration, ANIM_MACRO_INTERPOLATOR_EASE_IN, -1, -1
    );
}

function tweenlerp_add_easeout(tweenlerp, id, start, end, duration) {
    return tweenlerp_internal_add(
        tweenlerp, id, start, end, duration, ANIM_MACRO_INTERPOLATOR_EASE_OUT, -1, -1
    );
}

function tweenlerp_add_easeinout(tweenlerp, id, start, end, duration) {
    return tweenlerp_internal_add(
        tweenlerp, id, start, end, duration, ANIM_MACRO_INTERPOLATOR_EASE_IN_OUT, -1, -1
    );
}

function tweenlerp_add_linear(tweenlerp, id, start, end, duration) {
    return tweenlerp_internal_add(
        tweenlerp, id, start, end, duration, ANIM_MACRO_INTERPOLATOR_LINEAR, -1, -1
    );
}

function tweenlerp_add_steps(tweenlerp, id, start, end, duration, steps_count, steps_method) {
    return tweenlerp_internal_add(
        tweenlerp, id, start, end, duration, ANIM_MACRO_INTERPOLATOR_STEPS,
        steps_method, steps_count
    );
}

function tweenlerp_add_interpolator(tweenlerp, id, start, end, duration, type) {
    return tweenlerp_internal_add(tweenlerp, id, start, end, duration, type, ALIGN_NONE, 2);
}


function tweenlerp_vertex_set_properties(tweenlerp, vertex, setter) {
    let array = arraylist_peek_array(tweenlerp.arraylist);
    let size = arraylist_size(tweenlerp.arraylist);

    for (let i = 0; i < size; i++) {
        if (array[i].id < 0 || array[i].id == TEXTSPRITE_PROP_STRING) continue;
        setter(vertex, array[i].id, array[i].value);
    }
}


function tweenlerp_internal_by_brezier(tweenlerp_entry, progress) {
    return macroexecutor_calc_cubicbezier(progress, tweenlerp_entry.brezier_points);
}

function tweenlerp_internal_by_linear(tweenlerp_entry, progress) {
    return progress;
}

function tweenlerp_internal_by_steps(tweenlerp_entry, progress) {
    let completed = math2d_inverselerp(0, tweenlerp_entry.duration, progress);
    return macroexecutor_calc_steps(
        completed, tweenlerp_entry.steps_bounds, tweenlerp_entry.steps_count, tweenlerp_entry.steps_dir
    );
}

function tweenlerp_internal_animate_entry(tweenlerp_entry, progress) {
    // calculate the completed percent
    if (progress > tweenlerp_entry.duration) progress = tweenlerp_entry.duration;

    let progress_percent = math2d_inverselerp(0, tweenlerp_entry.duration, progress);
    let percent = tweenlerp_entry.callback(tweenlerp_entry, progress_percent);

    tweenlerp_entry.value = math2d_lerp(tweenlerp_entry.start, tweenlerp_entry.end, percent);
    return progress >= tweenlerp_entry.duration;
}

function tweenlerp_internal_animate_entry_absolute(tweenlerp_entry, progress_percent) {
    let percent = tweenlerp_entry.callback(tweenlerp_entry, progress_percent);
    tweenlerp_entry.value = math2d_lerp(tweenlerp_entry.start, tweenlerp_entry.end, percent);
    return percent >= 1.0;
}

function tweenlerp_internal_add(tweenlerp, id, start, end, duration, interp, steps_dir, steps_count) {
    let tweenlerp_entry = {
        steps_dir, steps_count, steps_bounds: [0.0, 0.0, 0.0],

        brezier_points: null,
        callback: null,

        id,
        value: start,

        start, end, duration
    }

    switch (interp) {
        case ANIM_MACRO_INTERPOLATOR_EASE:
            tweenlerp_entry.callback = tweenlerp_internal_by_brezier;
            tweenlerp_entry.brezier_points = CUBIC_BREZIER_EASE;
            break;
        case ANIM_MACRO_INTERPOLATOR_EASE_IN:
            tweenlerp_entry.callback = tweenlerp_internal_by_brezier;
            tweenlerp_entry.brezier_points = CUBIC_BREZIER_EASE_IN;
            break;
        case ANIM_MACRO_INTERPOLATOR_EASE_OUT:
            tweenlerp_entry.callback = tweenlerp_internal_by_brezier;
            tweenlerp_entry.brezier_points = CUBIC_BREZIER_EASE_OUT;
            break;
        case ANIM_MACRO_INTERPOLATOR_EASE_IN_OUT:
            tweenlerp_entry.callback = tweenlerp_internal_by_brezier;
            tweenlerp_entry.brezier_points = CUBIC_BREZIER_EASE_IN_OUT;
            break;
        case ANIM_MACRO_INTERPOLATOR_STEPS:
            tweenlerp_entry.callback = tweenlerp_internal_by_steps;
            break;
        case ANIM_MACRO_INTERPOLATOR_LINEAR:
            tweenlerp_entry.callback = tweenlerp_internal_by_linear;
            break;
        default:
            return -1;
    }

    // if all other entries are completed, check if this entry is completed too
    if (tweenlerp.has_completed)
        tweenlerp.has_completed = tweenlerp_internal_animate_entry(tweenlerp_entry, tweenlerp.progress);

    return arraylist_add(tweenlerp.arraylist, tweenlerp_entry) - 1;
}

