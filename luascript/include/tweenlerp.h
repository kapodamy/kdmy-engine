#ifndef __tweenlerp_h
#define __tweenlerp_h

#include "animlist.h"
#include "vertexprops.h"
#include <stdint.h>
#include <stdbool.h>


typedef struct {
    int dummy;
} TweenLerp_t;

typedef TweenLerp_t* TweenLerp;

TweenLerp tweenlerp_init();
void tweenlerp_destroy(TweenLerp* tweenlerp);
void tweenlerp_end(TweenLerp tweenlerp);
void tweenlerp_mark_as_completed(TweenLerp tweenlerp);
void tweenlerp_restart(TweenLerp tweenlerp);
int32_t tweenlerp_animate(TweenLerp tweenlerp, float elapsed);
int32_t tweenlerp_animate_percent(TweenLerp tweenlerp, float percent);
bool tweenlerp_is_completed(TweenLerp tweenlerp);
double tweenlerp_get_elapsed(TweenLerp tweenlerp);
int32_t tweenlerp_get_entry_count(TweenLerp tweenlerp);
float tweenlerp_peek_value(TweenLerp tweenlerp);
float tweenlerp_peek_value_by_index(TweenLerp tweenlerp, int32_t index);
bool tweenlerp_peek_entry_by_index(TweenLerp tweenlerp, int32_t index, int32_t* out_id, float* out_value, float* out_duration);
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
int32_t tweenlerp_add_interpolator(TweenLerp tweenlerp, int32_t id, float start, float end, float duration, AnimInterpolator type);

#endif
