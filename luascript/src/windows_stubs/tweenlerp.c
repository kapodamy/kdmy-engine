#include "commons.h"
#include "tweenlerp.h"

static TweenLerp_t tweenlerp = {};

TweenLerp tweenlerp_init() {
    print_stub("tweenlerp_init", "_stub0=%p", &tweenlerp);
    return &tweenlerp;
}
void tweenlerp_destroy(TweenLerp* tweenlerp) {
    print_stub("tweenlerp_destroy", "tweenlerp=%p", tweenlerp);
}
void tweenlerp_end(TweenLerp tweenlerp) {
    print_stub("tweenlerp_end", "tweenlerp=%p", tweenlerp);
}
void tweenlerp_mark_as_completed(TweenLerp tweenlerp) {
    print_stub("tweenlerp_mark_as_completed", "tweenlerp=%p", tweenlerp);
}
void tweenlerp_restart(TweenLerp tweenlerp) {
    print_stub("tweenlerp_restart", "tweenlerp=%p", tweenlerp);
}
int32_t tweenlerp_animate(TweenLerp tweenlerp, float elapsed) {
    print_stub("tweenlerp_animate", "tweenlerp=%p elapsed=%f", tweenlerp, elapsed);
    return 0;
}
int32_t tweenlerp_animate_percent(TweenLerp tweenlerp, float percent) {
    print_stub("tweenlerp_animate_percent", "tweenlerp=%p percent=%f", tweenlerp, percent);
    return 0;
}
bool tweenlerp_is_completed(TweenLerp tweenlerp) {
    print_stub("tweenlerp_is_completed", "tweenlerp=%p", tweenlerp);
    return 0;
}
double tweenlerp_get_elapsed(TweenLerp tweenlerp) {
    print_stub("tweenlerp_get_elapsed", "tweenlerp=%p", tweenlerp);
    return 0;
}
int32_t tweenlerp_get_entry_count(TweenLerp tweenlerp) {
    print_stub("tweenlerp_get_entry_count", "tweenlerp=%p", tweenlerp);
    return 0;
}
float tweenlerp_peek_value(TweenLerp tweenlerp) {
    print_stub("tweenlerp_peek_value", "tweenlerp=%p", tweenlerp);
    return 0;
}
float tweenlerp_peek_value_by_index(TweenLerp tweenlerp, int32_t index) {
    print_stub("tweenlerp_peek_value_by_index", "tweenlerp=%p index=%i", tweenlerp, index);
    return 0;
}
bool tweenlerp_peek_entry_by_index(TweenLerp tweenlerp, int32_t index, int32_t* out_id, float* out_value, float* out_duration) {
    *out_id = 123;
    *out_value = 456;
    *out_duration = 789;
    print_stub("tweenlerp_peek_entry_by_index", "tweenlerp=%p index=%i out_id=%p out_value=%p out_duration=%p", tweenlerp, index, out_id, out_value, out_duration);
    return 0;
}
float tweenlerp_peek_value_by_id(TweenLerp tweenlerp, int32_t id) {
    print_stub("tweenlerp_peek_value_by_id", "tweenlerp=%p id=%i", tweenlerp, id);
    return 0;
}
bool tweenlerp_change_bounds_by_index(TweenLerp tweenlerp, int32_t index, float new_start, float new_end) {
    print_stub("tweenlerp_change_bounds_by_index", "tweenlerp=%p index=%i new_start=%f new_end=%f", tweenlerp, index, new_start, new_end);
    return 0;
}
bool tweenlerp_override_start_with_end_by_index(TweenLerp tweenlerp, int32_t index) {
    print_stub("tweenlerp_override_start_with_end_by_index", "tweenlerp=%p index=%i", tweenlerp, index);
    return 0;
}
bool tweenlerp_change_bounds_by_id(TweenLerp tweenlerp, int32_t id, float new_start, float new_end) {
    print_stub("tweenlerp_change_bounds_by_id", "tweenlerp=%p id=%i new_start=%f new_end=%f", tweenlerp, id, new_start, new_end);
    return 0;
}
bool tweenlerp_change_duration_by_index(TweenLerp tweenlerp, int32_t index, float new_duration) {
    print_stub("tweenlerp_change_duration_by_index", "tweenlerp=%p index=%i new_duration=%f", tweenlerp, index, new_duration);
    return 0;
}
bool tweenlerp_swap_bounds_by_index(TweenLerp tweenlerp, int32_t index) {
    print_stub("tweenlerp_swap_bounds_by_index", "tweenlerp=%p index=%i", tweenlerp, index);
    return 0;
}
int32_t tweenlerp_add_ease(TweenLerp tweenlerp, int32_t id, float start, float end, float duration) {
    print_stub("tweenlerp_add_ease", "tweenlerp=%p id=%i start=%f end=%f duration=%f", tweenlerp, id, start, end, duration);
    return 0;
}
int32_t tweenlerp_add_easein(TweenLerp tweenlerp, int32_t id, float start, float end, float duration) {
    print_stub("tweenlerp_add_easein", "tweenlerp=%p id=%i start=%f end=%f duration=%f", tweenlerp, id, start, end, duration);
    return 0;
}
int32_t tweenlerp_add_easeout(TweenLerp tweenlerp, int32_t id, float start, float end, float duration) {
    print_stub("tweenlerp_add_easeout", "tweenlerp=%p id=%i start=%f end=%f duration=%f", tweenlerp, id, start, end, duration);
    return 0;
}
int32_t tweenlerp_add_easeinout(TweenLerp tweenlerp, int32_t id, float start, float end, float duration) {
    print_stub("tweenlerp_add_easeinout", "tweenlerp=%p id=%i start=%f end=%f duration=%f", tweenlerp, id, start, end, duration);
    return 0;
}
int32_t tweenlerp_add_linear(TweenLerp tweenlerp, int32_t id, float start, float end, float duration) {
    print_stub("tweenlerp_add_linear", "tweenlerp=%p id=%i start=%f end=%f duration=%f", tweenlerp, id, start, end, duration);
    return 0;
}
int32_t tweenlerp_add_steps(TweenLerp tweenlerp, int32_t id, float start, float end, float duration, int32_t steps_count, Align steps_method) {
    print_stub("tweenlerp_add_steps", "tweenlerp=%p id=%i start=%f end=%f duration=%f steps_count=%i steps_method=%hhu", tweenlerp, id, start, end, duration, steps_count, steps_method);
    return 0;
}
int32_t tweenlerp_add_cubic(TweenLerp tweenlerp, int32_t id, float start, float end, float duration) {
    print_stub("tweenlerp_add_cubic", "tweenlerp=%p id=%i start=%f end=%f duration=%f", tweenlerp, id, start, end, duration);
    return 0;
}
int32_t tweenlerp_add_quad(TweenLerp tweenlerp, int32_t id, float start, float end, float duration) {
    print_stub("tweenlerp_add_quad", "tweenlerp=%p id=%i start=%f end=%f duration=%f", tweenlerp, id, start, end, duration);
    return 0;
}
int32_t tweenlerp_add_expo(TweenLerp tweenlerp, int32_t id, float start, float end, float duration) {
    print_stub("tweenlerp_add_expo", "tweenlerp=%p id=%i start=%f end=%f duration=%f", tweenlerp, id, start, end, duration);
    return 0;
}
int32_t tweenlerp_add_sin(TweenLerp tweenlerp, int32_t id, float start, float end, float duration) {
    print_stub("tweenlerp_add_sin", "tweenlerp=%p id=%i start=%f end=%f duration=%f", tweenlerp, id, start, end, duration);
    return 0;
}
int32_t tweenlerp_add_interpolator(TweenLerp tweenlerp, int32_t id, float start, float end, float duration, AnimInterpolator type) {
    print_stub("tweenlerp_add_interpolator", "tweenlerp=%p id=%i start=%f end=%f duration=%f type=%p", tweenlerp, id, start, end, duration, type);
    return 0;
}
