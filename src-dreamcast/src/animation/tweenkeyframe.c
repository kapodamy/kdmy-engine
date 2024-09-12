#include "tweenkeyframe.h"
#include "arraylist.h"
#include "externals/luascript.h"
#include "logger.h"
#include "malloc_utils.h"
#include "math2d.h"
#include "stringutils.h"
#include "vertexprops.h"

struct Step_s;

typedef float (*Callback)(struct Step_s* step, float progress_percent);

struct TweenKeyframe_s {
    ArrayList entries;
};

typedef struct Step_s {
    float percent_start;
    float percent_end;
    float value_start;
    float value_end;
    const float* brezier_points;
    float steps_bounds[3];
    int32_t steps_count;
    Align steps_dir;
    Callback callback;
} Step;

typedef struct {
    int32_t id;
    float value;
    ArrayList steps;
} Entry;


static float tweenkeyframe_internal_by_brezier(Step* step, float percent);
static float tweenkeyframe_internal_by_linear(Step* step, float percent);
static float tweenkeyframe_internal_by_steps(Step* step, float percent);
static float tweenkeyframe_internal_by_cubic(Step* step, float percent);
static float tweenkeyframe_internal_by_quad(Step* step, float percent);
static float tweenkeyframe_internal_by_expo(Step* step, float percent);
static float tweenkeyframe_internal_by_sin(Step* step, float percent);
static float tweenkeyframe_internal_animate_entry(Step* step, float percent);
static int32_t tweenkeyframe_internal_add(TweenKeyframe tweenkeyframe, float at, int32_t id, float value, AnimInterpolator interp, Align steps_dir, int32_t steps_count);
static void tweenkeyframe_internal_calculate_ends(Entry* entry);
static int tweenkeyframe_internal_sort(const void* a_ptr, const void* b_ptr);


TweenKeyframe tweenkeyframe_init() {
    TweenKeyframe tweenkeyframe = malloc_chk(sizeof(struct TweenKeyframe_s));
    malloc_assert(tweenkeyframe, TweenKeyframe);

    *tweenkeyframe = (struct TweenKeyframe_s){
        .entries = arraylist_init(sizeof(Entry))
    };

    return tweenkeyframe;
}

TweenKeyframe tweenkeyframe_init2(const AnimListItem* animlist_item) {
    if (!animlist_item->is_tweenkeyframe) {
        logger_error("tweenkeyframe_init2() the animlist item is not a tweenkeyframe: %s", animlist_item->name);
        return NULL;
    }

    TweenKeyframe tweenkeyframe = malloc_chk(sizeof(struct TweenKeyframe_s));
    malloc_assert(tweenkeyframe, TweenKeyframe);

    *tweenkeyframe = (struct TweenKeyframe_s){
        .entries = arraylist_init2(sizeof(Entry), animlist_item->tweenkeyframe_entries_count)
    };

    for (int32_t i = 0; i < animlist_item->tweenkeyframe_entries_count; i++) {
        tweenkeyframe_internal_add(
            tweenkeyframe,
            animlist_item->tweenkeyframe_entries[i].at,
            animlist_item->tweenkeyframe_entries[i].id,
            animlist_item->tweenkeyframe_entries[i].value,
            animlist_item->tweenkeyframe_entries[i].interpolator,
            animlist_item->tweenkeyframe_entries[i].steps_dir,
            animlist_item->tweenkeyframe_entries[i].steps_count
        );
    }

    return tweenkeyframe;
}

TweenKeyframe tweenkeyframe_init3(AnimList animlist, const char* tweenkeyframe_name) {
    AnimListItem* animlist_item = NULL;

    for (int32_t i = 0; i < animlist->entries_count; i++) {
        if (string_equals(animlist->entries[i].name, tweenkeyframe_name)) {
            animlist_item = &animlist->entries[i];
            break;
        }
    }

    if (!animlist_item) {
        logger_warn("tweenkeyframe_init3() the animlist does not contains: %s", tweenkeyframe_name);
        return NULL;
    }

    return tweenkeyframe_init2(animlist_item);
}

void tweenkeyframe_destroy(TweenKeyframe* tweenkeyframe_ptr) {
    if (!tweenkeyframe_ptr || !*tweenkeyframe_ptr) return;

    TweenKeyframe tweenkeyframe = *tweenkeyframe_ptr;

    foreach (Entry*, entry, ARRAYLIST_ITERATOR, tweenkeyframe->entries) {
        arraylist_destroy(&entry->steps);
    }

    arraylist_destroy(&tweenkeyframe->entries);
    luascript_drop_shared(tweenkeyframe);

    free_chk(tweenkeyframe);
    *tweenkeyframe_ptr = NULL;
}

TweenKeyframe tweenkeyframe_clone(TweenKeyframe tweenkeyframe) {
    if (!tweenkeyframe) return NULL;

    TweenKeyframe copy = malloc_chk(sizeof(struct TweenKeyframe_s));
    malloc_assert(copy, TweenKeyframe);

    *copy = (struct TweenKeyframe_s){
        .entries = arraylist_clone(tweenkeyframe->entries)
    };

    foreach (Entry*, entry, ARRAYLIST_ITERATOR, copy->entries) {
        entry->steps = arraylist_clone(entry->steps);
    }

    return copy;
}


void tweenkeyframe_animate_percent(TweenKeyframe tweenkeyframe, float64 percent) {
    Entry* array = arraylist_peek_array(tweenkeyframe->entries);
    int32_t entries_size = arraylist_size(tweenkeyframe->entries);

    float final_percent = (float)math2d_clamp_double(percent, 0.0, 1.0);

    for (int32_t i = 0; i < entries_size; i++) {
        Entry* entry = &array[i];
        Step* steps = arraylist_peek_array(entry->steps);
        int32_t steps_size = arraylist_size(entry->steps);

        for (int32_t j = 0; j < steps_size; j++) {
            Step* step = &steps[i];

            if (percent >= step->percent_start && percent <= step->percent_end) {
                entry->value = tweenkeyframe_internal_animate_entry(step, final_percent);
            } else if (percent >= step->percent_end) {
                entry->value = step->value_end;
            }
        }
    }
}


int32_t tweenkeyframe_get_ids_count(TweenKeyframe tweenkeyframe) {
    return arraylist_size(tweenkeyframe->entries);
}


float tweenkeyframe_peek_value(TweenKeyframe tweenkeyframe) {
    if (arraylist_size(tweenkeyframe->entries) < 1) return FLOAT_NaN;
    Entry* entry = arraylist_get(tweenkeyframe->entries, 0);
    return entry->value;
}

float tweenkeyframe_peek_value_by_index(TweenKeyframe tweenkeyframe, int32_t index) {
    Entry* entry = arraylist_get(tweenkeyframe->entries, index);
    if (!entry) return FLOAT_NaN;

    return entry->value;
}

bool tweenkeyframe_peek_entry_by_index(TweenKeyframe tweenkeyframe, int32_t index, int32_t* id, float* value) {
    Entry* entry = arraylist_get(tweenkeyframe->entries, index);
    if (!entry) {
        *id = -1;
        *value = FLOAT_NaN;
        return false;
    }

    *id = entry->id;
    *value = entry->value;
    return true;
}

float tweenkeyframe_peek_value_by_id(TweenKeyframe tweenkeyframe, int32_t id) {
    foreach (Entry*, entry, ARRAYLIST_ITERATOR, tweenkeyframe->entries) {
        if (entry->id == id) return entry->value;
    }

    return FLOAT_NaN;
}


int32_t tweenkeyframe_add_ease(TweenKeyframe tweenkeyframe, float at, int32_t id, float value) {
    return tweenkeyframe_internal_add(
        tweenkeyframe, at, id, value, AnimInterpolator_EASE, (Align)-1, -1
    );
}

int32_t tweenkeyframe_add_easein(TweenKeyframe tweenkeyframe, float at, int32_t id, float value) {
    return tweenkeyframe_internal_add(
        tweenkeyframe, at, id, value, AnimInterpolator_EASE_IN, (Align)-1, -1
    );
}

int32_t tweenkeyframe_add_easeout(TweenKeyframe tweenkeyframe, float at, int32_t id, float value) {
    return tweenkeyframe_internal_add(
        tweenkeyframe, at, id, value, AnimInterpolator_EASE_OUT, (Align)-1, -1
    );
}

int32_t tweenkeyframe_add_easeinout(TweenKeyframe tweenkeyframe, float at, int32_t id, float value) {
    return tweenkeyframe_internal_add(
        tweenkeyframe, at, id, value, AnimInterpolator_EASE_IN_OUT, (Align)-1, -1
    );
}

int32_t tweenkeyframe_add_linear(TweenKeyframe tweenkeyframe, float at, int32_t id, float value) {
    return tweenkeyframe_internal_add(
        tweenkeyframe, at, id, value, AnimInterpolator_LINEAR, (Align)-1, -1
    );
}

int32_t tweenkeyframe_add_steps(TweenKeyframe tweenkeyframe, float at, int32_t id, float value, int32_t steps_count, Align steps_method) {
    return tweenkeyframe_internal_add(
        tweenkeyframe, at, id, value, AnimInterpolator_STEPS,
        steps_method, steps_count
    );
}

int32_t tweenkeyframe_add_cubic(TweenKeyframe tweenkeyframe, float at, int32_t id, float value) {
    return tweenkeyframe_internal_add(
        tweenkeyframe, at, id, value, AnimInterpolator_CUBIC, (Align)-1, -1
    );
}

int32_t tweenkeyframe_add_quad(TweenKeyframe tweenkeyframe, float at, int32_t id, float value) {
    return tweenkeyframe_internal_add(
        tweenkeyframe, at, id, value, AnimInterpolator_QUAD, (Align)-1, -1
    );
}

int32_t tweenkeyframe_add_expo(TweenKeyframe tweenkeyframe, float at, int32_t id, float value) {
    return tweenkeyframe_internal_add(
        tweenkeyframe, at, id, value, AnimInterpolator_EXPO, (Align)-1, -1
    );
}

int32_t tweenkeyframe_add_sin(TweenKeyframe tweenkeyframe, float at, int32_t id, float value) {
    return tweenkeyframe_internal_add(
        tweenkeyframe, at, id, value, AnimInterpolator_SIN, (Align)-1, -1
    );
}

int32_t tweenkeyframe_add_interpolator(TweenKeyframe tweenkeyframe, float at, int32_t id, float value, AnimInterpolator type) {
    return tweenkeyframe_internal_add(
        tweenkeyframe, at, id, value, type, (Align)-1, -1
    );
}


void tweenkeyframe_vertex_set_properties(TweenKeyframe tweenkeyframe, void* vertex, PropertySetter setter) {
    Entry* array = arraylist_peek_array(tweenkeyframe->entries);
    int32_t size = arraylist_size(tweenkeyframe->entries);

    for (int32_t i = 0; i < size; i++) {
        if (array[i].id < 0 || array[i].id == TEXTSPRITE_PROP_STRING) continue;
        if (math2d_is_float_NaN(array[i].value)) continue;
        setter(vertex, array[i].id, array[i].value);
    }
}


static float tweenkeyframe_internal_by_brezier(Step* step, float percent) {
    return macroexecutor_calc_cubicbezier(percent, step->brezier_points);
}

static float tweenkeyframe_internal_by_linear(Step* step, float percent) {
    (void)step;
    return percent;
}

static float tweenkeyframe_internal_by_steps(Step* step, float percent) {
    return macroexecutor_calc_steps(
        percent, step->steps_bounds, step->steps_count, step->steps_dir
    );
}

static float tweenkeyframe_internal_by_cubic(Step* step, float percent) {
    (void)step;
    return math2d_lerp_cubic(percent);
}

static float tweenkeyframe_internal_by_quad(Step* step, float percent) {
    (void)step;
    return math2d_lerp_quad(percent);
}

static float tweenkeyframe_internal_by_expo(Step* step, float percent) {
    (void)step;
    return math2d_lerp_expo(percent);
}

static float tweenkeyframe_internal_by_sin(Step* step, float percent) {
    (void)step;
    return math2d_lerp_sin(percent);
}


static float tweenkeyframe_internal_animate_entry(Step* step, float percent) {
    float interp_percent = step->callback(step, percent);
    float value = math2d_lerp(step->value_start, step->value_end, interp_percent);

    return value;
}

static int32_t tweenkeyframe_internal_add(TweenKeyframe tweenkeyframe, float at, int32_t id, float value, AnimInterpolator interp, Align steps_dir, int32_t steps_count) {
    Entry* entry = NULL;
    Step* step = NULL;

    // find the requested entry
    foreach (Entry*, existing_entry, ARRAYLIST_ITERATOR, tweenkeyframe->entries) {
        if (existing_entry->id == id) {
            entry = existing_entry;
            break;
        }
    }

    // create new entry (if necessary)
    if (!entry) {
        entry = arraylist_add(
            tweenkeyframe->entries,
            &(Entry){
                .id = id,
                .value = 0.0f,
                .steps = arraylist_init(sizeof(Step)),
            }
        );
    }

    // find a duplicated step and replace if exists
    foreach (Step*, existing_step, ARRAYLIST_ITERATOR, entry->steps) {
        if (existing_step->percent_start == at) {
            step = existing_step;
            break;
        }
    }

    // create new step (if necessary)
    if (!step) {
        step = arraylist_add(
            entry->steps,
            &(Step){
                .steps_dir = steps_dir,
                .steps_count = steps_count,
                .steps_bounds = {0.0f, 0.0f, 0.0f},

                .brezier_points = NULL,
                .callback = NULL,

                .value_start = value,
                .value_end = value,

                .percent_start = at,
                .percent_end = 1.0f,
            }
        );
    }

    switch (interp) {
        case AnimInterpolator_EASE:
            step->callback = tweenkeyframe_internal_by_brezier;
            step->brezier_points = CUBIC_BREZIER_EASE;
            break;
        case AnimInterpolator_EASE_IN:
            step->callback = tweenkeyframe_internal_by_brezier;
            step->brezier_points = CUBIC_BREZIER_EASE_IN;
            break;
        case AnimInterpolator_EASE_OUT:
            step->callback = tweenkeyframe_internal_by_brezier;
            step->brezier_points = CUBIC_BREZIER_EASE_OUT;
            break;
        case AnimInterpolator_EASE_IN_OUT:
            step->callback = tweenkeyframe_internal_by_brezier;
            step->brezier_points = CUBIC_BREZIER_EASE_IN_OUT;
            break;
        case AnimInterpolator_STEPS:
            step->callback = tweenkeyframe_internal_by_steps;
            break;
        case AnimInterpolator_LINEAR:
            step->callback = tweenkeyframe_internal_by_linear;
            break;
        case AnimInterpolator_CUBIC:
            step->callback = tweenkeyframe_internal_by_cubic;
            break;
        case AnimInterpolator_QUAD:
            step->callback = tweenkeyframe_internal_by_quad;
            break;
        case AnimInterpolator_EXPO:
            step->callback = tweenkeyframe_internal_by_expo;
            break;
        case AnimInterpolator_SIN:
            step->callback = tweenkeyframe_internal_by_sin;
            break;
        default:
#if DEBUG
            logger_error("tweenkeyframe_internal_add() unknown interpolator provided");
#endif
            // this never should happen
            arraylist_remove(entry->steps, step);
            return -1;
    }

    tweenkeyframe_internal_calculate_ends(entry);

    return arraylist_index_of(tweenkeyframe->entries, entry);
}


static void tweenkeyframe_internal_calculate_ends(Entry* entry) {
    // sort steps by starting percent
    arraylist_sort(entry->steps, tweenkeyframe_internal_sort);

    Step* steps = arraylist_peek_array(entry->steps);
    int32_t steps_count = arraylist_size(entry->steps);

    for (int32_t i = 0, j = 1; i < steps_count; i++, j++) {
        if (j < steps_count) {
            steps[i].value_end = steps[j].value_start;
            steps[i].percent_end = steps[j].percent_start;
        } else {
            steps[i].value_end = steps[i].value_start;
            steps[i].percent_end = 1.0f;
        }
    }
}

static int tweenkeyframe_internal_sort(const void* a_ptr, const void* b_ptr) {
    const Step* a = (const Step*)a_ptr;
    const Step* b = (const Step*)b_ptr;

    return math2d_float_comparer(a->percent_start, b->percent_start);
}
