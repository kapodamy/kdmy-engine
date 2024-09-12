#ifndef _tweenlerp_h
#define _tweenlerp_h

#include <stdbool.h>
#include <stdint.h>

#include "macroexecutor_types.h"
#include "vertexprops_types.h"
#include "float64.h"


typedef struct TweenLerp_s* TweenLerp;


TweenLerp tweenlerp_init();
void tweenlerp_destroy(TweenLerp* tweenlerp);
TweenLerp tweenlerp_clone(TweenLerp tweenlerp);

void tweenlerp_end(TweenLerp tweenlerp);
void tweenlerp_mark_as_completed(TweenLerp tweenlerp);

void tweenlerp_restart(TweenLerp tweenlerp);
int32_t tweenlerp_animate(TweenLerp tweenlerp, float elapsed);
int32_t tweenlerp_animate_timestamp(TweenLerp tweenlerp, float64 timestamp);
int32_t tweenlerp_animate_percent(TweenLerp tweenlerp, float64 percent);
bool tweenlerp_is_completed(TweenLerp tweenlerp);
float64 tweenlerp_get_elapsed(TweenLerp tweenlerp);
int32_t tweenlerp_get_entry_count(TweenLerp tweenlerp);
float tweenlerp_peek_value(TweenLerp tweenlerp);
float tweenlerp_peek_value_by_index(TweenLerp tweenlerp, int32_t index);
bool tweenlerp_peek_entry_by_index(TweenLerp tweenlerp, int32_t index, int32_t* id, float* value, float* duration);
float tweenlerp_peek_value_by_id(TweenLerp tweenlerp, int32_t id);
bool tweenlerp_change_bounds_by_index(TweenLerp tweenlerp, int32_t index, float new_start, float new_end);
bool tweenlerp_override_start_with_end_by_index(TweenLerp tweenlerp, int32_t index);
bool tweenlerp_change_bounds_by_id(TweenLerp tweenlerp, int32_t id, float new_start, float new_end);
bool tweenlerp_change_duration_by_index(TweenLerp tweenlerp, int32_t index, float new_duration);
bool tweenlerp_swap_bounds_by_index(TweenLerp tweenlerp, int32_t index);
int32_t tweenlerp_add_ease(TweenLerp tweenlerp, int32_t id, float start, float end, float duration);
int32_t tweenlerp_add_easein(TweenLerp tweenlerp, int32_t id, float start, float end, float duration);
int32_t tweenlerp_add_easeout(TweenLerp tweenlerp, int32_t id, float start, float end, float duration);
int32_t tweenlerp_add_easeinout(TweenLerp tweenlerp, int32_t id, float start, float end, float duration);
int32_t tweenlerp_add_linear(TweenLerp tweenlerp, int32_t id, float start, float end, float duration);
int32_t tweenlerp_add_steps(TweenLerp tweenlerp, int32_t id, float start, float end, float duration, int32_t steps_count, Align steps_method);
int32_t tweenlerp_add_cubic(TweenLerp tweenlerp, int32_t id, float start, float end, float duration);
int32_t tweenlerp_add_quad(TweenLerp tweenlerp, int32_t id, float start, float end, float duration);
int32_t tweenlerp_add_expo(TweenLerp tweenlerp, int32_t id, float start, float end, float duration);
int32_t tweenlerp_add_sin(TweenLerp tweenlerp, int32_t id, float start, float end, float duration);
int32_t tweenlerp_add_interpolator(TweenLerp tweenlerp, int32_t id, float start, float end, float duration, AnimInterpolator type);

void tweenlerp_vertex_set_properties(TweenLerp tweenlerp, void* vertex, PropertySetter setter);

#endif
