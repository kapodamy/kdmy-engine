"use strict";


function tweenkeyframe_init() {
    return {
        arraylist_keyframes: arraylist_init(),
        arraylist_values: arraylist_init(),
        default_interpolator: ANIM_MACRO_INTERPOLATOR_LINEAR
    }
}

function tweenkeyframe_init2(animlist_item) {
    if (!animlist_item.is_tweenkeyframe) {
        console.error("tweenkeyframe_init2() the animlist item is not a tweenkeyframe: " + animlist_item.name);
        return null;
    }

    let tweenkeyframe = {
        arraylist_keyframes: arraylist_init2(animlist_item.tweenkeyframe_entries_count),
        arraylist_values: arraylist_init(),
        default_interpolator: animlist_item.tweenkeyframe_default_interpolator
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
        console.warn("tweenkeyframe_init3() the animlist does not contains: " + tweenkeyframe_name);
        return null;
    }

    return tweenkeyframe_init2(animlist_item);
}

function tweenkeyframe_destroy(tweenkeyframe) {
    arraylist_destroy(tweenkeyframe.arraylist_keyframes, 0);
    arraylist_destroy(tweenkeyframe.arraylist_values, 0);
    ModuleLuaScript.kdmyEngine_drop_shared_object(tweenkeyframe);
    tweenkeyframe = undefined;
}

function tweenkeyframe_clone(tweenkeyframe) {
    if (!tweenkeyframe) return null;

    let copy = {
        arraylist_keyframes: arraylist_clone(tweenkeyframe.arraylist_keyframes),
        arraylist_values: arraylist_clone(tweenkeyframe.arraylist_values),
        progress: tweenkeyframe.progress,
        has_completed: tweenkeyframe.has_completed
    };

    //  (JS & C# only) clone steps_bounds
    for (let entry of arraylist_iterate4(copy.arraylist_keyframes)) {
        entry.steps_bounds = [entry.steps_bounds[0], entry.steps_bounds[1], entry.steps_bounds[2]];
    }

    return copy;
}


function tweenkeyframe_animate_percent(tweenkeyframe, percent) {
    const array = arraylist_peek_array(tweenkeyframe.arraylist_keyframes);
    const size = arraylist_size(tweenkeyframe.arraylist_keyframes);

    percent = math2d_clamp(percent, 0.0, 1.0);

    for (let i = 0; i < size; i++) {
        let keyframe_entry = array[i];
        if (percent >= keyframe_entry.percent_start && percent <= keyframe_entry.percent_end) {
            tweenkeyframe_internal_animate_entry(keyframe_entry, percent);
        } else if (percent >= keyframe_entry.percent_end) {
            keyframe_entry.keyframe_value.value = keyframe_entry.value_end;
        }
    }

}


function tweenkeyframe_get_ids_count(tweenkeyframe) {
    return arraylist_size(tweenkeyframe.arraylist_values);
}


function tweenkeyframe_peek_value(tweenkeyframe) {
    if (arraylist_size(tweenkeyframe.arraylist_values) < 1) return NaN;
    return arraylist_get(tweenkeyframe.arraylist_values, 0).value;
}

function tweenkeyframe_peek_value_by_index(tweenkeyframe, index) {
    let keyframe_value = arraylist_get(tweenkeyframe.arraylist_values, index);
    if (!keyframe_value) return NaN;

    return keyframe_value.value;
}

function tweenkeyframe_peek_entry_by_index(tweenkeyframe, index, output_id_value_pair) {
    let keyframe_value = arraylist_get(tweenkeyframe.arraylist_values, index);
    if (!keyframe_value) return null;

    output_id_value_pair[0] = keyframe_value.id;
    output_id_value_pair[1] = keyframe_value.value;
    return output_id_value_pair;
}

function tweenkeyframe_peek_value_by_id(tweenkeyframe, id) {
    for (let keyframe_value of tweenkeyframe.arraylist_values) {
        if (keyframe_value.id == id) return keyframe_value.value;
    }

    return NaN;
}


function tweenkeyframe_add_easeout(tweenkeyframe, at, id, value) {
    return tweenkeyframe_internal_add(
        tweenkeyframe, at, id, value, ANIM_MACRO_INTERPOLATOR_EASE_OUT, -1, -1
    );
}

function tweenkeyframe_add_easeinout(tweenkeyframe, at, id, value) {
    return tweenkeyframe_internal_add(
        tweenkeyframe, at, id, value, ANIM_MACRO_INTERPOLATOR_EASE_IN_OUT, -1, -1
    );
}

function tweenkeyframe_add_linear(tweenkeyframe, at, id, value) {
    return tweenkeyframe_internal_add(
        tweenkeyframe, at, id, value, ANIM_MACRO_INTERPOLATOR_LINEAR, -1, -1
    );
}

function tweenkeyframe_add_steps(tweenkeyframe, at, id, value, steps_count, steps_method) {
    return tweenkeyframe_internal_add(
        tweenkeyframe, at, id, value, ANIM_MACRO_INTERPOLATOR_STEPS,
        steps_method, steps_count
    );
}

function tweenkeyframe_add_interpolator(tweenkeyframe, at, id, value, type) {
    return tweenkeyframe_internal_add(
        tweenkeyframe, at, id, value, type, -1, -1
    );
}


function tweenkeyframe_vertex_set_properties(tweenkeyframe, vertex, setter) {
    const array = arraylist_peek_array(tweenkeyframe.arraylist_values);
    const size = arraylist_size(tweenkeyframe.arraylist_values);

    for (let i = 0; i < size; i++) {
        if (array[i].id < 0 || array[i].id == TEXTSPRITE_PROP_STRING) continue;
        if (Number.isNaN(array[i].value)) continue;
        setter(vertex, array[i].id, array[i].value);
    }
}


function tweenkeyframe_internal_by_brezier(tweenkeyframe_entry, percent) {
    return macroexecutor_calc_cubicbezier(percent, tweenkeyframe_entry.brezier_points);
}

function tweenkeyframe_internal_by_linear(tweenkeyframe_entry, percent) {
    return percent;
}

function tweenkeyframe_internal_by_steps(tweenkeyframe_entry, percent) {
    return macroexecutor_calc_steps(
        percent, tweenkeyframe_entry.steps_bounds, tweenkeyframe_entry.steps_count, tweenkeyframe_entry.steps_dir
    );
}


function tweenkeyframe_internal_animate_entry(tweenkeyframe_entry, percent) {
    let interp_percent = tweenkeyframe_entry.callback(tweenkeyframe_entry, percent);
    let value = math2d_lerp(tweenkeyframe_entry.value_start, tweenkeyframe_entry.value_end, interp_percent);
    tweenkeyframe_entry.keyframe_value.value = value;
}

function tweenkeyframe_internal_add(tweenkeyframe, at, id, value, interp, steps_dir, steps_count) {
    let keyframe_entry = null;
    let keyframe_entry_index = 0;

    // find a duplicated entry and replace
    for (let entry of arraylist_iterate4(tweenkeyframe.arraylist_keyframes)) {
        if (entry.id == id && entry.percent_start == at) {
            keyframe_entry = entry;
            break;
        }
        keyframe_entry_index++;
    }

    if (!keyframe_entry) {
        let new_keyframe_entry = {
            steps_dir, steps_count, steps_bounds: [0.0, 0.0, 0.0],

            brezier_points: null,
            callback: null,

            id,

            value_start: value,
            value_end: value,

            percent_start: at,
            percent_end: 1.0,

            keyframe_value: null
        };

        arraylist_add(tweenkeyframe.arraylist_keyframes, new_keyframe_entry);
        keyframe_entry = new_keyframe_entry;
    }

    switch (interp) {
        case ANIM_MACRO_INTERPOLATOR_EASE:
            keyframe_entry.callback = tweenkeyframe_internal_by_brezier;
            keyframe_entry.brezier_points = CUBIC_BREZIER_EASE;
            break;
        case ANIM_MACRO_INTERPOLATOR_EASE_IN:
            keyframe_entry.callback = tweenkeyframe_internal_by_brezier;
            keyframe_entry.brezier_points = CUBIC_BREZIER_EASE_IN;
            break;
        case ANIM_MACRO_INTERPOLATOR_EASE_OUT:
            keyframe_entry.callback = tweenkeyframe_internal_by_brezier;
            keyframe_entry.brezier_points = CUBIC_BREZIER_EASE_OUT;
            break;
        case ANIM_MACRO_INTERPOLATOR_EASE_IN_OUT:
            keyframe_entry.callback = tweenkeyframe_internal_by_brezier;
            keyframe_entry.brezier_points = CUBIC_BREZIER_EASE_IN_OUT;
            break;
        case ANIM_MACRO_INTERPOLATOR_STEPS:
            keyframe_entry.callback = tweenkeyframe_internal_by_steps;
            break;
        case ANIM_MACRO_INTERPOLATOR_LINEAR:
            keyframe_entry.callback = tweenkeyframe_internal_by_linear;
            break;
        case -1:
            console.assert(tweenkeyframe.default_interpolator != -1, "invalid default interpolator");
            interp = tweenkeyframe.default_interpolator;
            return tweenkeyframe_internal_add(tweenkeyframe, at, id, value, interp, steps_dir, steps_count);
        default:
            arraylist_remove(tweenkeyframe.arraylist_keyframes, keyframe_entry);
            return -1;
    }

    // store id for values
    let value_index = 0;

    L_find_or_add_id: {
        for (let value_entry of arraylist_iterate4(tweenkeyframe.arraylist_values)) {
            if (value_entry.id == id) {
                keyframe_entry.keyframe_value = value_entry;
                break L_find_or_add_id;
            }
            value_index++;
        }

        let new_value_entry = { id: keyframe_entry.id, value: NaN };
        value_index = arraylist_add(tweenkeyframe.arraylist_values, new_value_entry) - 1;
        keyframe_entry.keyframe_value = new_value_entry;
    }

    tweenkeyframe_internal_calculate_ends(tweenkeyframe);
    return value_index;
}


function tweenkeyframe_internal_calculate_ends(tweenkeyframe) {
    arraylist_sort(tweenkeyframe.arraylist_keyframes, tweenkeyframe_internal_sort);

    const array = arraylist_peek_array(tweenkeyframe.arraylist_keyframes);
    let last_index = arraylist_size(tweenkeyframe.arraylist_keyframes) - 1;

    for (let value of arraylist_iterate4(tweenkeyframe.arraylist_values)) {
        let last_entry = null;

        for (let i = last_index; i >= 0; i--) {
            if (array[i].id != value.id) continue;

            if (last_entry) {
                array[i].value_end = last_entry.value_start;
                array[i].percent_end = last_entry.percent_start;
            } else {
                array[i].value_end = array[i].value_start;
                array[i].percent_end = 1.0;
            }
            last_entry = array[i];
        }
    }
}

function tweenkeyframe_internal_sort(a, b) {
    let combo_a = a.percent_start + a.id;
    let combo_b = b.percent_start + b.id;
    return combo_a - combo_b;
}

