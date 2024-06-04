"use strict";


function tweenkeyframe_init() {
    let tweenkeyframe = {
        entries: arraylist_init(/*sizeof(Entry)*/)
    };

    return tweenkeyframe;
}

function tweenkeyframe_init2(animlist_item) {
    if (!animlist_item.is_tweenkeyframe) {
        console.error("tweenkeyframe_init2() the animlist item is not a tweenkeyframe: %s", animlist_item.name);
        return null;
    }

    let tweenkeyframe = {
        entries: arraylist_init2(animlist_item.tweenkeyframe_entries_count)
    };

    for (let i = 0; i < animlist_item.tweenkeyframe_entries_count; i++) {
        tweenkeyframe_internal_add(
            tweenkeyframe,
            animlist_item.tweenkeyframe_entries[i].at,
            animlist_item.tweenkeyframe_entries[i].id,
            animlist_item.tweenkeyframe_entries[i].value,
            animlist_item.tweenkeyframe_entries[i].interpolator,
            animlist_item.tweenkeyframe_entries[i].steps_dir,
            animlist_item.tweenkeyframe_entries[i].steps_count
        );
    }

    return tweenkeyframe;
}

function tweenkeyframe_init3(animlist, tweenkeyframe_name) {
    let animlist_item = null;

    for (let i = 0; i < animlist.entries_count; i++) {
        if (animlist.entries[i].name == tweenkeyframe_name) {
            animlist_item = animlist.entries[i];
            break;
        }
    }

    if (!animlist_item) {
        console.warn("tweenkeyframe_init3() the animlist does not contains: %s", tweenkeyframe_name);
        return null;
    }

    return tweenkeyframe_init2(animlist_item);
}

function tweenkeyframe_destroy(tweenkeyframe) {
    if (!tweenkeyframe) return;

    for (let entry of arraylist_iterate4(tweenkeyframe.entries)) {
        arraylist_destroy(entry.steps, false);
    }

    arraylist_destroy(tweenkeyframe.entries, false);
    luascript_drop_shared(tweenkeyframe);

    tweenkeyframe = undefined;
}

function tweenkeyframe_clone(tweenkeyframe) {
    if (!tweenkeyframe) return null;

    let copy = {
        entries: arraylist_clone(tweenkeyframe.entries)
    };

    for (let entry of arraylist_iterate4(copy.entries)) {
        entry.steps = arraylist_clone(entry.steps);
    }

    return copy;
}


function tweenkeyframe_animate_percent(tweenkeyframe, percent) {
    let array = arraylist_peek_array(tweenkeyframe.entries);
    let entries_size = arraylist_size(tweenkeyframe.entries);

    let final_percent = math2d_clamp_double(percent, 0.0, 1.0);

    for (let i = 0; i < entries_size; i++) {
        let entry = array[i];
        let steps = arraylist_peek_array(entry.steps);
        let steps_size = arraylist_size(entry.steps);

        for (let j = 0; j < steps_size; j++) {
            let step = steps[i];

            if (percent >= step.percent_start && percent <= step.percent_end) {
                entry.value = tweenkeyframe_internal_animate_entry(step, final_percent);
            } else if (percent >= step.percent_end) {
                entry.value = step.value_end;
            }
        }
    }
}


function tweenkeyframe_get_ids_count(tweenkeyframe) {
    return arraylist_size(tweenkeyframe.entries);
}


function tweenkeyframe_peek_value(tweenkeyframe) {
    if (arraylist_size(tweenkeyframe.entries) < 1) return NaN;
    let entry = arraylist_get(tweenkeyframe.entries, 0);
    return entry.value;
}

function tweenkeyframe_peek_value_by_index(tweenkeyframe, index) {
    let entry = arraylist_get(tweenkeyframe.entries, index);
    if (!entry) return NaN;

    return entry.value;
}

function tweenkeyframe_peek_entry_by_index(tweenkeyframe, index, output_id_value_pair) {
    let entry = arraylist_get(tweenkeyframe.entries, index);
    if (!entry) {
        output_id_value_pair[0] = -1;
        output_id_value_pair[1] = NaN;
        return false;
    }

    output_id_value_pair[0] = entry.id;
    output_id_value_pair[1] = entry.value;
    return true;
}

function tweenkeyframe_peek_value_by_id(tweenkeyframe, id) {
    for (let entry of arraylist_iterate4(tweenkeyframe.entries)) {
        if (entry.id == id) return entry.value;
    }

    return NaN;
}


function tweenkeyframe_add_ease(tweenkeyframe, at, id, value) {
    return tweenkeyframe_internal_add(
        tweenkeyframe, at, id, value, ANIM_INTERPOLATOR_EASE, -1, -1
    );
}

function tweenkeyframe_add_easein(tweenkeyframe, at, id, value) {
    return tweenkeyframe_internal_add(
        tweenkeyframe, at, id, value, ANIM_INTERPOLATOR_EASE_IN, -1, -1
    );
}

function tweenkeyframe_add_easeout(tweenkeyframe, at, id, value) {
    return tweenkeyframe_internal_add(
        tweenkeyframe, at, id, value, ANIM_INTERPOLATOR_EASE_OUT, -1, -1
    );
}

function tweenkeyframe_add_easeinout(tweenkeyframe, at, id, value) {
    return tweenkeyframe_internal_add(
        tweenkeyframe, at, id, value, ANIM_INTERPOLATOR_EASE_IN_OUT, -1, -1
    );
}

function tweenkeyframe_add_linear(tweenkeyframe, at, id, value) {
    return tweenkeyframe_internal_add(
        tweenkeyframe, at, id, value, ANIM_INTERPOLATOR_LINEAR, -1, -1
    );
}

function tweenkeyframe_add_steps(tweenkeyframe, at, id, value, steps_count, steps_method) {
    return tweenkeyframe_internal_add(
        tweenkeyframe, at, id, value, ANIM_INTERPOLATOR_STEPS,
        steps_method, steps_count
    );
}

function tweenkeyframe_add_cubic(tweenkeyframe, at, id, value) {
    return tweenkeyframe_internal_add(
        tweenkeyframe, at, id, value, ANIM_INTERPOLATOR_CUBIC, -1, -1
    );
}

function tweenkeyframe_add_quad(tweenkeyframe, at, id, value) {
    return tweenkeyframe_internal_add(
        tweenkeyframe, at, id, value, ANIM_INTERPOLATOR_QUAD, -1, -1
    );
}

function tweenkeyframe_add_expo(tweenkeyframe, at, id, value) {
    return tweenkeyframe_internal_add(
        tweenkeyframe, at, id, value, ANIM_INTERPOLATOR_EXPO, -1, -1
    );
}

function tweenkeyframe_add_sin(tweenkeyframe, at, id, value) {
    return tweenkeyframe_internal_add(
        tweenkeyframe, at, id, value, ANIM_INTERPOLATOR_SIN, -1, -1
    );
}

function tweenkeyframe_add_interpolator(tweenkeyframe, at, id, value, type) {
    return tweenkeyframe_internal_add(
        tweenkeyframe, at, id, value, type, -1, -1
    );
}


function tweenkeyframe_vertex_set_properties(tweenkeyframe, vertex, setter) {
    let array = arraylist_peek_array(tweenkeyframe.entries);
    let size = arraylist_size(tweenkeyframe.entries);

    for (let i = 0; i < size; i++) {
        if (array[i].id < 0 || array[i].id == TEXTSPRITE_PROP_STRING) continue;
        if (Number.isNaN(array[i].value)) continue;
        setter(vertex, array[i].id, array[i].value);
    }
}


function tweenkeyframe_internal_by_brezier(step, percent) {
    return macroexecutor_calc_cubicbezier(percent, step.brezier_points);
}

function tweenkeyframe_internal_by_linear(step, percent) {
    return percent;
}

function tweenkeyframe_internal_by_steps(step, percent) {
    return macroexecutor_calc_steps(
        percent, step.steps_bounds, step.steps_count, step.steps_dir
    );
}

function tweenkeyframe_internal_by_cubic(step, percent) {
    return math2d_lerp_cubic(percent);
}

function tweenkeyframe_internal_by_quad(step, percent) {
    return math2d_lerp_quad(percent);
}

function tweenkeyframe_internal_by_expo(step, percent) {
    return math2d_lerp_expo(percent);
}

function tweenkeyframe_internal_by_sin(step, percent) {
    return math2d_lerp_sin(percent);
}


function tweenkeyframe_internal_animate_entry(step, percent) {
    let interp_percent = step.callback(step, percent);
    let value = math2d_lerp(step.value_start, step.value_end, interp_percent);

    return value;
}

function tweenkeyframe_internal_add(tweenkeyframe, at, id, value, interp, steps_dir, steps_count) {
    let entry = null;
    let step = null;

    // find the requested entry
    for (let existing_entry of arraylist_iterate4(tweenkeyframe.entries)) {
        if (existing_entry.id == id) {
            entry = existing_entry;
            break;
        }
    }

    // create new entry (if necessary)
    if (!entry) {
        entry = arraylist_add(tweenkeyframe.entries, {
            id: id,
            value: 0.0,
            steps: arraylist_init(/*sizeof(Step)*/),
        });
    }

    // find a duplicated step and replace if exists
    for (let existing_step of arraylist_iterate4(entry.steps)) {
        if (existing_step.percent_start == at) {
            step = existing_step;
            break;
        }
    }

    // create new step (if necessary)
    if (!step) {
        step = arraylist_add(entry.steps, {
            steps_dir: steps_dir,
            steps_count: steps_count,
            steps_bounds: [0.0, 0.0, 0.0],

            brezier_points: null,
            callback: null,

            value_start: value,
            value_end: value,

            percent_start: at,
            percent_end: 1.0
        });
    }

    switch (interp) {
        case ANIM_INTERPOLATOR_EASE:
            step.callback = tweenkeyframe_internal_by_brezier;
            step.brezier_points = CUBIC_BREZIER_EASE;
            break;
        case ANIM_INTERPOLATOR_EASE_IN:
            step.callback = tweenkeyframe_internal_by_brezier;
            step.brezier_points = CUBIC_BREZIER_EASE_IN;
            break;
        case ANIM_INTERPOLATOR_EASE_OUT:
            step.callback = tweenkeyframe_internal_by_brezier;
            step.brezier_points = CUBIC_BREZIER_EASE_OUT;
            break;
        case ANIM_INTERPOLATOR_EASE_IN_OUT:
            step.callback = tweenkeyframe_internal_by_brezier;
            step.brezier_points = CUBIC_BREZIER_EASE_IN_OUT;
            break;
        case ANIM_INTERPOLATOR_STEPS:
            step.callback = tweenkeyframe_internal_by_steps;
            break;
        case ANIM_INTERPOLATOR_LINEAR:
            step.callback = tweenkeyframe_internal_by_linear;
            break;
        case ANIM_INTERPOLATOR_CUBIC:
            step.callback = tweenkeyframe_internal_by_cubic;
            break;
        case ANIM_INTERPOLATOR_QUAD:
            step.callback = tweenkeyframe_internal_by_quad;
            break;
        case ANIM_INTERPOLATOR_EXPO:
            step.callback = tweenkeyframe_internal_by_expo;
            break;
        case ANIM_INTERPOLATOR_SIN:
            step.callback = tweenkeyframe_internal_by_sin;
            break;
        default:
            if (DEBUG) {
                console.error("tweenkeyframe_internal_add() unknown interpolator provided");
            }
            // this never should happen
            arraylist_remove(entry.steps, step);
            return -1;
    }

    tweenkeyframe_internal_calculate_ends(entry);

    return arraylist_index_of(tweenkeyframe.entries, entry);
}


function tweenkeyframe_internal_calculate_ends(entry) {
    // sort steps by starting percent
    arraylist_sort(entry.steps, tweenkeyframe_internal_sort);

    let steps = arraylist_peek_array(entry.steps);
    let steps_count = arraylist_size(entry.steps);

    for (let i = 0, j = 1; i < steps_count; i++, j++) {
        if (j < steps_count) {
            steps[i].value_end = steps[j].value_start;
            steps[i].percent_end = steps[j].percent_start;
        } else {
            steps[i].value_end = steps[i].value_start;
            steps[i].percent_end = 1.0;
        }
    }
}

function tweenkeyframe_internal_sort(a, b) {
    return a.percent_start - b.percent_start;
}


