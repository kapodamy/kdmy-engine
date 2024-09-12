#include "tweenlerp.h"

#include <string.h>

#include "arraylist.h"
#include "cloneutils.h"
#include "externals/luascript.h"
#include "malloc_utils.h"
#include "math2d.h"
#include "vertexprops.h"


struct TweenLerp_s {
    ArrayList arraylist;
    float64 progress;
    bool has_completed;
};

struct TweenLerpEntry_s;

typedef float (*TweenLerpCallback)(struct TweenLerpEntry_s* tweenlerp_entry, float progress_percent);

typedef struct TweenLerpEntry_s {
    float duration;
    float value;
    int32_t id;
    float start;
    float end;
    const float* brezier_points;
    float steps_bounds[3];
    int steps_count;
    Align steps_dir;
    TweenLerpCallback callback;
} TweenLerpEntry;


static float tweenlerp_internal_by_brezier(TweenLerpEntry* tweenlerp_entry, float progress);
static float tweenlerp_internal_by_linear(TweenLerpEntry* tweenlerp_entry, float progress);
static float tweenlerp_internal_by_steps(TweenLerpEntry* tweenlerp_entry, float progress);
static float tweenlerp_internal_by_cubic(TweenLerpEntry* tweenlerp_entry, float progress);
static float tweenlerp_internal_by_quad(TweenLerpEntry* tweenlerp_entry, float progress);
static float tweenlerp_internal_by_expo(TweenLerpEntry* tweenlerp_entry, float progress);
static float tweenlerp_internal_by_sin(TweenLerpEntry* tweenlerp_entry, float progress);
static bool tweenlerp_internal_animate_entry(TweenLerpEntry* tweenlerp_entry, float progress);
static bool tweenlerp_internal_animate_entry_absolute(TweenLerpEntry* tweenlerp_entry, float progress_percent);
static int32_t tweenlerp_internal_add(TweenLerp tweenlerp, int32_t id, float start, float end, float duration, AnimInterpolator interp, Align steps_dir, int32_t steps_count);


TweenLerp tweenlerp_init() {
    TweenLerp tweenlerp = malloc_chk(sizeof(struct TweenLerp_s));
    malloc_assert(tweenlerp, TweenLerp);

    *tweenlerp = (struct TweenLerp_s){
        .arraylist = arraylist_init(sizeof(TweenLerpEntry)),
        .progress = 0.0,
        .has_completed = false
    };

    return tweenlerp;
}

void tweenlerp_destroy(TweenLerp* tweenlerp_ptr) {
    TweenLerp tweenlerp = *tweenlerp_ptr;
    if (!tweenlerp) return;

    arraylist_destroy(&tweenlerp->arraylist);
    luascript_drop_shared(tweenlerp);

    free_chk(tweenlerp);
    *tweenlerp_ptr = NULL;
}

TweenLerp tweenlerp_clone(TweenLerp tweenlerp) {
    if (!tweenlerp) return NULL;

    TweenLerp copy = CLONE_STRUCT(struct TweenLerp_s, TweenLerp, tweenlerp);
    copy->arraylist = arraylist_clone(tweenlerp->arraylist);

    return copy;
}


void tweenlerp_end(TweenLerp tweenlerp) {
    foreach (TweenLerpEntry*, entry, ARRAYLIST_ITERATOR, tweenlerp->arraylist) {
        if (entry->duration > tweenlerp->progress) tweenlerp->progress = entry->duration;
        entry->value = entry->end;
    }

    tweenlerp->has_completed = true;
}

void tweenlerp_mark_as_completed(TweenLerp tweenlerp) {
    tweenlerp->has_completed = true;
}


void tweenlerp_restart(TweenLerp tweenlerp) {
    tweenlerp->progress = 0.0;
    tweenlerp->has_completed = false;
    tweenlerp_animate_percent(tweenlerp, 0.0);
}

int32_t tweenlerp_animate(TweenLerp tweenlerp, float elapsed) {
    if (tweenlerp->has_completed) return 1;

    tweenlerp_animate_timestamp(tweenlerp, tweenlerp->progress + elapsed);

    return 0; // keep last "frame" alive
}

int32_t tweenlerp_animate_timestamp(TweenLerp tweenlerp, float64 timestamp) {
    tweenlerp->progress = timestamp;

    TweenLerpEntry* array_tweens = arraylist_peek_array(tweenlerp->arraylist);
    int32_t size_tweens = arraylist_size(tweenlerp->arraylist);
    int32_t completed = 0;

    for (int32_t i = 0; i < size_tweens; i++) {
        if (tweenlerp_internal_animate_entry(&array_tweens[i], (float)tweenlerp->progress)) completed++;
    }

    if (completed >= size_tweens) tweenlerp->has_completed = true;

    return completed;
}

int32_t tweenlerp_animate_percent(TweenLerp tweenlerp, float64 percent) {
    tweenlerp->progress = 0.0; // undefined behavoir

    TweenLerpEntry* array = arraylist_peek_array(tweenlerp->arraylist);
    int32_t size = arraylist_size(tweenlerp->arraylist);
    int32_t completed = 0;

    percent = math2d_clamp_double(percent, 0.0, 1.0);

    for (int32_t i = 0; i < size; i++) {
        if (tweenlerp_internal_animate_entry_absolute(&array[i], (float)percent)) completed++;
    }

    tweenlerp->has_completed = percent >= 1.0;
    return completed;
}

bool tweenlerp_is_completed(TweenLerp tweenlerp) {
    return tweenlerp->has_completed;
}

float64 tweenlerp_get_elapsed(TweenLerp tweenlerp) {
    return tweenlerp->progress;
}

int32_t tweenlerp_get_entry_count(TweenLerp tweenlerp) {
    return arraylist_size(tweenlerp->arraylist);
}

float tweenlerp_peek_value(TweenLerp tweenlerp) {
    if (arraylist_size(tweenlerp->arraylist) < 1) return FLOAT_NaN;
    return ((TweenLerpEntry*)arraylist_get(tweenlerp->arraylist, 0))->value;
}

float tweenlerp_peek_value_by_index(TweenLerp tweenlerp, int32_t index) {
    TweenLerpEntry* entry = arraylist_get(tweenlerp->arraylist, index);
    if (!entry) return FLOAT_NaN;
    return entry->value;
}

bool tweenlerp_peek_entry_by_index(TweenLerp tweenlerp, int32_t index, int32_t* id, float* value, float* duration) {
    TweenLerpEntry* entry = arraylist_get(tweenlerp->arraylist, index);
    if (!entry) {
        *id = -1;
        *value = FLOAT_NaN;
        *duration = FLOAT_NaN;
        return false;
    }
    *id = entry->id;
    *value = entry->value;
    *duration = entry->duration;
    return true;
}

float tweenlerp_peek_value_by_id(TweenLerp tweenlerp, int32_t id) {
    TweenLerpEntry* array = arraylist_peek_array(tweenlerp->arraylist);
    int32_t size = arraylist_size(tweenlerp->arraylist);

    for (int32_t i = 0; i < size; i++) {
        if (array[i].id == id) return array[i].value;
    }

    return FLOAT_NaN;
}

bool tweenlerp_change_bounds_by_index(TweenLerp tweenlerp, int32_t index, float new_start, float new_end) {
    TweenLerpEntry* tweenlerp_entry = arraylist_get(tweenlerp->arraylist, index);

    if (!math2d_is_float_NaN(new_start)) tweenlerp_entry->start = new_start;
    if (!math2d_is_float_NaN(new_end)) tweenlerp_entry->end = new_end;

    return tweenlerp_internal_animate_entry(tweenlerp_entry, (float)tweenlerp->progress);
}

bool tweenlerp_override_start_with_end_by_index(TweenLerp tweenlerp, int32_t index) {
    TweenLerpEntry* tweenlerp_entry = arraylist_get(tweenlerp->arraylist, index);

    tweenlerp_entry->start = tweenlerp_entry->end;

    return tweenlerp_internal_animate_entry(tweenlerp_entry, (float)tweenlerp->progress);
}

bool tweenlerp_change_bounds_by_id(TweenLerp tweenlerp, int32_t id, float new_start, float new_end) {
    TweenLerpEntry* array = arraylist_peek_array(tweenlerp->arraylist);
    int32_t size = arraylist_size(tweenlerp->arraylist);

    for (int32_t i = 0; i < size; i++) {
        if (array[i].id != id) continue;
        array[i].start = new_start;
        array[i].end = new_end;
        tweenlerp_internal_animate_entry(&array[i], (float)tweenlerp->progress);
        return true;
    }

    return false;
}

bool tweenlerp_change_duration_by_index(TweenLerp tweenlerp, int32_t index, float new_duration) {
    TweenLerpEntry* tweenlerp_entry = arraylist_get(tweenlerp->arraylist, index);
    tweenlerp_entry->duration = new_duration;

    return tweenlerp_internal_animate_entry(tweenlerp_entry, (float)tweenlerp->progress);
}

bool tweenlerp_swap_bounds_by_index(TweenLerp tweenlerp, int32_t index) {
    TweenLerpEntry* tweenlerp_entry = arraylist_get(tweenlerp->arraylist, index);
    float temp = tweenlerp_entry->start;

    tweenlerp_entry->start = tweenlerp_entry->end;
    tweenlerp_entry->end = temp;

    return tweenlerp_internal_animate_entry(tweenlerp_entry, (float)tweenlerp->progress);
}


int32_t tweenlerp_add_ease(TweenLerp tweenlerp, int32_t id, float start, float end, float duration) {
    return tweenlerp_internal_add(
        tweenlerp, id, start, end, duration, AnimInterpolator_EASE, (Align)-1, -1
    );
}

int32_t tweenlerp_add_easein(TweenLerp tweenlerp, int32_t id, float start, float end, float duration) {
    return tweenlerp_internal_add(
        tweenlerp, id, start, end, duration, AnimInterpolator_EASE_IN, (Align)-1, -1
    );
}

int32_t tweenlerp_add_easeout(TweenLerp tweenlerp, int32_t id, float start, float end, float duration) {
    return tweenlerp_internal_add(
        tweenlerp, id, start, end, duration, AnimInterpolator_EASE_OUT, (Align)-1, -1
    );
}

int32_t tweenlerp_add_easeinout(TweenLerp tweenlerp, int32_t id, float start, float end, float duration) {
    return tweenlerp_internal_add(
        tweenlerp, id, start, end, duration, AnimInterpolator_EASE_IN_OUT, (Align)-1, -1
    );
}

int32_t tweenlerp_add_linear(TweenLerp tweenlerp, int32_t id, float start, float end, float duration) {
    return tweenlerp_internal_add(
        tweenlerp, id, start, end, duration, AnimInterpolator_LINEAR, (Align)-1, -1
    );
}

int32_t tweenlerp_add_steps(TweenLerp tweenlerp, int32_t id, float start, float end, float duration, int32_t steps_count, Align steps_method) {
    return tweenlerp_internal_add(
        tweenlerp, id, start, end, duration, AnimInterpolator_STEPS,
        steps_method, steps_count
    );
}

int32_t tweenlerp_add_cubic(TweenLerp tweenlerp, int32_t id, float start, float end, float duration) {
    return tweenlerp_internal_add(
        tweenlerp, id, start, end, duration, AnimInterpolator_CUBIC, (Align)-1, -1
    );
}

int32_t tweenlerp_add_quad(TweenLerp tweenlerp, int32_t id, float start, float end, float duration) {
    return tweenlerp_internal_add(
        tweenlerp, id, start, end, duration, AnimInterpolator_QUAD, (Align)-1, -1
    );
}

int32_t tweenlerp_add_expo(TweenLerp tweenlerp, int32_t id, float start, float end, float duration) {
    return tweenlerp_internal_add(
        tweenlerp, id, start, end, duration, AnimInterpolator_EXPO, (Align)-1, -1
    );
}

int32_t tweenlerp_add_sin(TweenLerp tweenlerp, int32_t id, float start, float end, float duration) {
    return tweenlerp_internal_add(
        tweenlerp, id, start, end, duration, AnimInterpolator_SIN, (Align)-1, -1
    );
}

int32_t tweenlerp_add_interpolator(TweenLerp tweenlerp, int32_t id, float start, float end, float duration, AnimInterpolator type) {
    return tweenlerp_internal_add(tweenlerp, id, start, end, duration, type, ALIGN_NONE, 2);
}


void tweenlerp_vertex_set_properties(TweenLerp tweenlerp, void* vertex, PropertySetter setter) {
    TweenLerpEntry* array = arraylist_peek_array(tweenlerp->arraylist);
    int32_t size = arraylist_size(tweenlerp->arraylist);

    for (int32_t i = 0; i < size; i++) {
        if (array[i].id < 0 || array[i].id == TEXTSPRITE_PROP_STRING) continue;
        setter(vertex, array[i].id, array[i].value);
    }
}


static float tweenlerp_internal_by_brezier(TweenLerpEntry* tweenlerp_entry, float progress) {
    return macroexecutor_calc_cubicbezier((float)progress, tweenlerp_entry->brezier_points);
}

static float tweenlerp_internal_by_linear(TweenLerpEntry* tweenlerp_entry, float progress) {
    (void)tweenlerp_entry;
    return (float)progress;
}

static float tweenlerp_internal_by_steps(TweenLerpEntry* tweenlerp_entry, float progress) {
    float completed = math2d_inverselerp(0.0f, tweenlerp_entry->duration, (float)progress);
    return macroexecutor_calc_steps(
        completed, tweenlerp_entry->steps_bounds, tweenlerp_entry->steps_count, tweenlerp_entry->steps_dir
    );
}

static float tweenlerp_internal_by_cubic(TweenLerpEntry* tweenlerp_entry, float progress) {
    (void)tweenlerp_entry;
    return math2d_lerp_cubic((float)progress);
}

static float tweenlerp_internal_by_quad(TweenLerpEntry* tweenlerp_entry, float progress) {
    (void)tweenlerp_entry;
    return math2d_lerp_quad((float)progress);
}

static float tweenlerp_internal_by_expo(TweenLerpEntry* tweenlerp_entry, float progress) {
    (void)tweenlerp_entry;
    return math2d_lerp_expo((float)progress);
}

static float tweenlerp_internal_by_sin(TweenLerpEntry* tweenlerp_entry, float progress) {
    (void)tweenlerp_entry;
    return math2d_lerp_sin((float)progress);
}

static bool tweenlerp_internal_animate_entry(TweenLerpEntry* tweenlerp_entry, float progress) {
    // calculate the completed percent
    if (progress > tweenlerp_entry->duration) progress = tweenlerp_entry->duration;

    float progress_percent = math2d_inverselerp(0.0f, tweenlerp_entry->duration, (float)progress);
    float percent = tweenlerp_entry->callback(tweenlerp_entry, progress_percent);

    tweenlerp_entry->value = math2d_lerp(tweenlerp_entry->start, tweenlerp_entry->end, percent);
    return progress >= tweenlerp_entry->duration;
}

static bool tweenlerp_internal_animate_entry_absolute(TweenLerpEntry* tweenlerp_entry, float progress_percent) {
    float percent = tweenlerp_entry->callback(tweenlerp_entry, progress_percent);
    tweenlerp_entry->value = math2d_lerp(tweenlerp_entry->start, tweenlerp_entry->end, percent);
    return percent >= 1.0;
}

static int32_t tweenlerp_internal_add(TweenLerp tweenlerp, int32_t id, float start, float end, float duration, AnimInterpolator interp, Align steps_dir, int32_t steps_count) {
    TweenLerpEntry tweenlerp_entry = {
        .steps_dir = steps_dir, .steps_count = steps_count, .steps_bounds = {0.0f, 0.0f, 0.0f},

        .brezier_points = NULL,
        .callback = NULL,

        .id = id,
        .value = start,

        .start = start,
        .end = end,
        .duration = duration
    };

    switch (interp) {
        case AnimInterpolator_EASE:
            tweenlerp_entry.callback = tweenlerp_internal_by_brezier;
            tweenlerp_entry.brezier_points = CUBIC_BREZIER_EASE;
            break;
        case AnimInterpolator_EASE_IN:
            tweenlerp_entry.callback = tweenlerp_internal_by_brezier;
            tweenlerp_entry.brezier_points = CUBIC_BREZIER_EASE_IN;
            break;
        case AnimInterpolator_EASE_OUT:
            tweenlerp_entry.callback = tweenlerp_internal_by_brezier;
            tweenlerp_entry.brezier_points = CUBIC_BREZIER_EASE_OUT;
            break;
        case AnimInterpolator_EASE_IN_OUT:
            tweenlerp_entry.callback = tweenlerp_internal_by_brezier;
            tweenlerp_entry.brezier_points = CUBIC_BREZIER_EASE_IN_OUT;
            break;
        case AnimInterpolator_STEPS:
            tweenlerp_entry.callback = tweenlerp_internal_by_steps;
            break;
        case AnimInterpolator_LINEAR:
            tweenlerp_entry.callback = tweenlerp_internal_by_linear;
            break;
        case AnimInterpolator_CUBIC:
            tweenlerp_entry.callback = tweenlerp_internal_by_cubic;
            break;
        case AnimInterpolator_QUAD:
            tweenlerp_entry.callback = tweenlerp_internal_by_quad;
            break;
        case AnimInterpolator_EXPO:
            tweenlerp_entry.callback = tweenlerp_internal_by_expo;
            break;
        case AnimInterpolator_SIN:
            tweenlerp_entry.callback = tweenlerp_internal_by_sin;
            break;
        default:
            return -1;
    }

    // if all other entries are completed, check if this entry is completed too
    if (tweenlerp->has_completed)
        tweenlerp->has_completed = tweenlerp_internal_animate_entry(&tweenlerp_entry, (float)tweenlerp->progress);

    int32_t index = arraylist_size(tweenlerp->arraylist);
    arraylist_add(tweenlerp->arraylist, &tweenlerp_entry);

    return index;
}
