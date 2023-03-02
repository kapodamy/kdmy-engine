#include "commons.h"
#include "tweenkeyframe.h"

static TweenKeyframe_t tweenlkeyframe = {};

TweenKeyframe tweenkeyframe_init() {
    print_stub("tweenkeyframe_init", "init return %p", &tweenlkeyframe);
    return &tweenlkeyframe;
}
TweenKeyframe tweenkeyframe_init2(AnimListItem animlist_item) {
    print_stub("tweenkeyframe_init2", "animlist_item=%p init2 return %p", animlist_item, &tweenlkeyframe);
    return &tweenlkeyframe;
}
void tweenkeyframe_destroy(TweenKeyframe* tweenkeyframe) {
    print_stub("tweenkeyframe_destroy", "tweenkeyframe=%p", tweenkeyframe);
}
void tweenkeyframe_animate_percent(TweenKeyframe tweenkeyframe, double percent) {
    print_stub("tweenkeyframe_animate_percent", "tweenkeyframe=%p percent=%f", tweenkeyframe, percent);
}
int32_t tweenkeyframe_get_ids_count(TweenKeyframe tweenkeyframe) {
    print_stub("tweenkeyframe_get_ids_count", "tweenkeyframe=%p", tweenkeyframe);
    return 0;
}
float tweenkeyframe_peek_value(TweenKeyframe tweenkeyframe) {
    print_stub("tweenkeyframe_peek_value", "tweenkeyframe=%p", tweenkeyframe);
    return 0;
}
float tweenkeyframe_peek_value_by_index(TweenKeyframe tweenkeyframe, int32_t index) {
    print_stub("tweenkeyframe_peek_value_by_index", "tweenkeyframe=%p index=%i", tweenkeyframe, index);
    return 0;
}
bool tweenkeyframe_peek_entry_by_index(TweenKeyframe tweenkeyframe, int32_t index, int32_t* out_id,  float* out_value) {
    *out_id = 123;
    *out_value = 45.6f;
    print_stub("tweenkeyframe_peek_entry_by_index", "tweenkeyframe=%p index=%i out_id=%p out_value=%p", tweenkeyframe, index, out_id, out_value);
    return 0;
}
float tweenkeyframe_peek_value_by_id(TweenKeyframe tweenkeyframe, int32_t id) {
    print_stub("tweenkeyframe_peek_value_by_id", "tweenkeyframe=%p id=%i", tweenkeyframe, id);
    return 0;
}
int32_t tweenkeyframe_add_easeout(TweenKeyframe tweenkeyframe, float at, int32_t id, float value) {
    print_stub("tweenkeyframe_add_easeout", "tweenkeyframe=%p at=%f id=%i value=%f", tweenkeyframe, at, id, value);
    return 0;
}
int32_t tweenkeyframe_add_easeinout(TweenKeyframe tweenkeyframe, float at, int32_t id, float value) {
    print_stub("tweenkeyframe_add_easeinout", "tweenkeyframe=%p at=%f id=%i value=%f", tweenkeyframe, at, id, value);
    return 0;
}
int32_t tweenkeyframe_add_linear(TweenKeyframe tweenkeyframe, float at, int32_t id, float value) {
    print_stub("tweenkeyframe_add_linear", "tweenkeyframe=%p at=%f id=%i value=%f", tweenkeyframe, at, id, value);
    return 0;
}
int32_t tweenkeyframe_add_steps(TweenKeyframe tweenkeyframe, float at, int32_t id, float value, int32_t steps_count, Align steps_method) {
    print_stub("tweenkeyframe_add_steps", "tweenkeyframe=%p at=%f id=%i value=%f steps_count=%i steps_method=%hhu", tweenkeyframe, at, id, value, steps_count, steps_method);
    return 0;
}
int32_t tweenkeyframe_add_cubic(TweenKeyframe tweenkeyframe, float at, int32_t id, float value) {
    print_stub("tweenkeyframe_add_cubic", "tweenkeyframe=%p at=%f id=%i value=%f", tweenkeyframe, at, id, value);
    return 0;
}
int32_t tweenkeyframe_add_ease(TweenKeyframe tweenkeyframe, float at, int32_t id, float value) {
    print_stub("tweenkeyframe_add_ease", "tweenkeyframe=%p at=%f id=%i value=%f", tweenkeyframe, at, id, value);
    return 0;
}
int32_t tweenkeyframe_add_easein(TweenKeyframe tweenkeyframe, float at, int32_t id, float value) {
    print_stub("tweenkeyframe_add_easein", "tweenkeyframe=%p at=%f id=%i value=%f", tweenkeyframe, at, id, value);
    return 0;
}
int32_t tweenkeyframe_add_quad(TweenKeyframe tweenkeyframe, float at, int32_t id, float value) {
    print_stub("tweenkeyframe_add_quad", "tweenkeyframe=%p at=%f id=%i value=%f", tweenkeyframe, at, id, value);
    return 0;
}
int32_t tweenkeyframe_add_expo(TweenKeyframe tweenkeyframe, float at, int32_t id, float value) {
    print_stub("tweenkeyframe_add_expo", "tweenkeyframe=%p at=%f id=%i value=%f", tweenkeyframe, at, id, value);
    return 0;
}
int32_t tweenkeyframe_add_sin(TweenKeyframe tweenkeyframe, float at, int32_t id, float value) {
    print_stub("tweenkeyframe_add_sin", "tweenkeyframe=%p at=%f id=%i value=%f", tweenkeyframe, at, id, value);
    return 0;
}
int32_t tweenkeyframe_add_interpolator(TweenKeyframe tweenkeyframde, float at, int32_t id, float value, AnimInterpolator type) {
    print_stub("tweenkeyframe_add_interpolator", "tweenkeyframde=%p at=%f id=%i value=%f type=%i", tweenkeyframde, at, id, value, type);
    return 0;
}
