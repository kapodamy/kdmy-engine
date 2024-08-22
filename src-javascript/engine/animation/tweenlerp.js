"use strict";


function tweenlerp_init() {
    return {
        arraylist: arraylist_init(),
        progress: 0.0,
        has_completed: false
    }
}

function tweenlerp_destroy(tweenlerp) {
    arraylist_destroy(tweenlerp.arraylist);
    luascript_drop_shared(tweenlerp);
    tweenlerp = undefined;
}

function tweenlerp_clone(tweenlerp) {
    let copy = clone_object_shallow(tweenlerp);
    copy.arraylist = arraylist_clone(tweenlerp.arraylist);

    // (JS only) clone steps_bounds
    for (let entry of arraylist_iterate4(copy.arraylist)) {
        entry.steps_bounds = clone_array(entry.steps_bounds, entry.steps_bounds.length);
    }

    return copy;
}


function tweenlerp_end(tweenlerp) {
    for (let entry of arraylist_iterate4(tweenlerp.arraylist)) {
        if (entry.duration > tweenlerp.progress) tweenlerp.progress = entry.duration;
        entry.value = entry.end;
    }

    tweenlerp.has_completed = true;
}

function tweenlerp_mark_as_completed(tweenlerp) {
    tweenlerp.has_completed = true;
}


function tweenlerp_restart(tweenlerp) {
    tweenlerp.progress = 0.0;
    tweenlerp.has_completed = false;
    tweenlerp_animate_percent(tweenlerp, 0.0);
}

function tweenlerp_animate(tweenlerp, elapsed) {
    if (tweenlerp.has_completed) return 1;

    tweenlerp_animate_timestamp(tweenlerp, tweenlerp.progress + elapsed);

    return 0;// keep last "frame" alive
}

function tweenlerp_animate_timestamp(tweenlerp, timestamp) {
    tweenlerp.progress = timestamp;

    const array_tweens = arraylist_peek_array(tweenlerp.arraylist);
    const size_tweens = arraylist_size(tweenlerp.arraylist);
    let completed = 0;

    for (let i = 0; i < size_tweens; i++) {
        if (tweenlerp_internal_animate_entry(array_tweens[i], tweenlerp.progress)) completed++;
    }

    if (completed >= size_tweens) tweenlerp.has_completed = true;

    return completed;
}

function tweenlerp_animate_percent(tweenlerp, percent) {
    tweenlerp.progress = 0.0;// undefined behavoir

    const array = arraylist_peek_array(tweenlerp.arraylist);
    const size = arraylist_size(tweenlerp.arraylist);
    let completed = 0;

    percent = math2d_clamp_float(percent, 0.0, 1.0);

    for (let i = 0; i < size; i++) {
        if (tweenlerp_internal_animate_entry_absolute(array[i], percent)) completed++;
    }

    tweenlerp.has_completed = percent >= 1.0;
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
    if (arraylist_size(tweenlerp.arraylist) < 1) return NaN;
    return arraylist_get(tweenlerp.arraylist, 0).value;
}

function tweenlerp_peek_value_by_index(tweenlerp, index) {
    let entry = arraylist_get(tweenlerp.arraylist, index);
    if (!entry) return NaN;
    return entry.value;
}

function tweenlerp_peek_entry_by_index(tweenlerp, index, output_id_value_duration_pair) {
    let entry = arraylist_get(tweenlerp.arraylist, index);
    if (!entry) {
        output_id_value_duration_pair[0] = -1;
        output_id_value_duration_pair[1] = NaN;
        output_id_value_duration_pair[2] = NaN;
        return false;
    }
    output_id_value_duration_pair[0] = entry.id;
    output_id_value_duration_pair[1] = entry.value;
    output_id_value_duration_pair[2] = entry.duration;
    return true;
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

    if (!Number.isNaN(new_start)) tweenlerp_entry.start = new_start;
    if (!Number.isNaN(new_end)) tweenlerp_entry.end = new_end;

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
        return true;
    }

    return false;
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
        tweenlerp, id, start, end, duration, ANIM_INTERPOLATOR_EASE, -1, -1
    );
}

function tweenlerp_add_easein(tweenlerp, id, start, end, duration) {
    return tweenlerp_internal_add(
        tweenlerp, id, start, end, duration, ANIM_INTERPOLATOR_EASE_IN, -1, -1
    );
}

function tweenlerp_add_easeout(tweenlerp, id, start, end, duration) {
    return tweenlerp_internal_add(
        tweenlerp, id, start, end, duration, ANIM_INTERPOLATOR_EASE_OUT, -1, -1
    );
}

function tweenlerp_add_easeinout(tweenlerp, id, start, end, duration) {
    return tweenlerp_internal_add(
        tweenlerp, id, start, end, duration, ANIM_INTERPOLATOR_EASE_IN_OUT, -1, -1
    );
}

function tweenlerp_add_linear(tweenlerp, id, start, end, duration) {
    return tweenlerp_internal_add(
        tweenlerp, id, start, end, duration, ANIM_INTERPOLATOR_LINEAR, -1, -1
    );
}

function tweenlerp_add_steps(tweenlerp, id, start, end, duration, steps_count, steps_method) {
    return tweenlerp_internal_add(
        tweenlerp, id, start, end, duration, ANIM_INTERPOLATOR_STEPS,
        steps_method, steps_count
    );
}

function tweenlerp_add_cubic(tweenlerp, id, start, end, duration) {
    return tweenlerp_internal_add(
        tweenlerp, id, start, end, duration, ANIM_INTERPOLATOR_CUBIC, -1, -1
    );
}

function tweenlerp_add_quad(tweenlerp, id, start, end, duration) {
    return tweenlerp_internal_add(
        tweenlerp, id, start, end, duration, ANIM_INTERPOLATOR_QUAD, -1, -1
    );
}

function tweenlerp_add_expo(tweenlerp, id, start, end, duration) {
    return tweenlerp_internal_add(
        tweenlerp, id, start, end, duration, ANIM_INTERPOLATOR_EXPO, -1, -1
    );
}

function tweenlerp_add_sin(tweenlerp, id, start, end, duration) {
    return tweenlerp_internal_add(
        tweenlerp, id, start, end, duration, ANIM_INTERPOLATOR_SIN, -1, -1
    );
}

function tweenlerp_add_interpolator(tweenlerp, id, start, end, duration, type) {
    return tweenlerp_internal_add(tweenlerp, id, start, end, duration, type, ALIGN_NONE, 2);
}


function tweenlerp_vertex_set_properties(tweenlerp, vertex, setter) {
    const array = arraylist_peek_array(tweenlerp.arraylist);
    const size = arraylist_size(tweenlerp.arraylist);

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
    let completed = math2d_inverselerp(0.0, tweenlerp_entry.duration, progress);
    return macroexecutor_calc_steps(
        completed, tweenlerp_entry.steps_bounds, tweenlerp_entry.steps_count, tweenlerp_entry.steps_dir
    );
}

function tweenlerp_internal_by_cubic(tweenlerp_entry, progress) {
    return math2d_lerp_cubic(progress);
}

function tweenlerp_internal_by_quad(tweenlerp_entry, progress) {
    return math2d_lerp_quad(progress);
}

function tweenlerp_internal_by_expo(tweenlerp_entry, progress) {
    return math2d_lerp_expo(progress);
}

function tweenlerp_internal_by_sin(tweenlerp_entry, progress) {
    return math2d_lerp_sin(progress);
}

function tweenlerp_internal_animate_entry(tweenlerp_entry, progress) {
    // calculate the completed percent
    if (progress > tweenlerp_entry.duration) progress = tweenlerp_entry.duration;

    let progress_percent = math2d_inverselerp(0.0, tweenlerp_entry.duration, progress);
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
    };

    switch (interp) {
        case ANIM_INTERPOLATOR_EASE:
            tweenlerp_entry.callback = tweenlerp_internal_by_brezier;
            tweenlerp_entry.brezier_points = CUBIC_BREZIER_EASE;
            break;
        case ANIM_INTERPOLATOR_EASE_IN:
            tweenlerp_entry.callback = tweenlerp_internal_by_brezier;
            tweenlerp_entry.brezier_points = CUBIC_BREZIER_EASE_IN;
            break;
        case ANIM_INTERPOLATOR_EASE_OUT:
            tweenlerp_entry.callback = tweenlerp_internal_by_brezier;
            tweenlerp_entry.brezier_points = CUBIC_BREZIER_EASE_OUT;
            break;
        case ANIM_INTERPOLATOR_EASE_IN_OUT:
            tweenlerp_entry.callback = tweenlerp_internal_by_brezier;
            tweenlerp_entry.brezier_points = CUBIC_BREZIER_EASE_IN_OUT;
            break;
        case ANIM_INTERPOLATOR_STEPS:
            tweenlerp_entry.callback = tweenlerp_internal_by_steps;
            break;
        case ANIM_INTERPOLATOR_LINEAR:
            tweenlerp_entry.callback = tweenlerp_internal_by_linear;
            break;
        case ANIM_INTERPOLATOR_CUBIC:
            tweenlerp_entry.callback = tweenlerp_internal_by_cubic;
            break;
        case ANIM_INTERPOLATOR_QUAD:
            tweenlerp_entry.callback = tweenlerp_internal_by_quad;
            break;
        case ANIM_INTERPOLATOR_EXPO:
            tweenlerp_entry.callback = tweenlerp_internal_by_expo;
            break;
        case ANIM_INTERPOLATOR_SIN:
            tweenlerp_entry.callback = tweenlerp_internal_by_sin;
            break;
        default:
            return -1;
    }

    // if all other entries are completed, check if this entry is completed too
    if (tweenlerp.has_completed)
        tweenlerp.has_completed = tweenlerp_internal_animate_entry(tweenlerp_entry, tweenlerp.progress);

    let index = arraylist_size(tweenlerp.arraylist);
    arraylist_add(tweenlerp.arraylist, tweenlerp_entry);

    return index;
}

