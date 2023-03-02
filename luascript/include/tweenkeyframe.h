#ifndef __tweenkeyframe_h
#define __tweenkeyframe_h

#include <stdint.h>
#include "animlist.h"
#include "vertexprops.h"

typedef struct {
    int dummy;
} TweenKeyframe_t;
typedef TweenKeyframe_t* TweenKeyframe;

TweenKeyframe tweenkeyframe_init();
TweenKeyframe tweenkeyframe_init2(AnimListItem animlist_item);
void tweenkeyframe_destroy(TweenKeyframe* tweenkeyframe);
void tweenkeyframe_animate_percent(TweenKeyframe tweenkeyframe, double percent);
int32_t tweenkeyframe_get_ids_count(TweenKeyframe tweenkeyframe);
float tweenkeyframe_peek_value(TweenKeyframe tweenkeyframe);
float tweenkeyframe_peek_value_by_index(TweenKeyframe tweenkeyframe, int32_t index);
bool tweenkeyframe_peek_entry_by_index(TweenKeyframe tweenkeyframe, int32_t index, int32_t* out_id,  float* out_value);
float tweenkeyframe_peek_value_by_id(TweenKeyframe tweenkeyframe, int32_t id);
int32_t tweenkeyframe_add_easeout(TweenKeyframe tweenkeyframe, float at, int32_t id, float value);
int32_t tweenkeyframe_add_easeinout(TweenKeyframe tweenkeyframe, float at, int32_t id, float value);
int32_t tweenkeyframe_add_linear(TweenKeyframe tweenkeyframe, float at, int32_t id, float value);
int32_t tweenkeyframe_add_steps(TweenKeyframe tweenkeyframe, float at, int32_t id, float value, int32_t steps_count, Align steps_method);
int32_t tweenkeyframe_add_ease(TweenKeyframe tweenkeyframe, float at, int32_t id, float value);
int32_t tweenkeyframe_add_easein(TweenKeyframe tweenkeyframe, float at, int32_t id, float value);
int32_t tweenkeyframe_add_cubic(TweenKeyframe tweenkeyframe, float at, int32_t id, float value);
int32_t tweenkeyframe_add_quad(TweenKeyframe tweenkeyframe, float at, int32_t id, float value);
int32_t tweenkeyframe_add_expo(TweenKeyframe tweenkeyframe, float at, int32_t id, float value);
int32_t tweenkeyframe_add_sin(TweenKeyframe tweenkeyframe, float at, int32_t id, float value);
int32_t tweenkeyframe_add_interpolator(TweenKeyframe tweenkeyframde, float at, int32_t id, float value, AnimInterpolator type);

#endif
