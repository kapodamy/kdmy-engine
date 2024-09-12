#ifndef _tweenkeyframe_h
#define _tweenkeyframe_h

#include <stdbool.h>
#include <stdint.h>

#include "animlist.h"
#include "macroexecutor_types.h"
#include "vertexprops_types.h"
#include "float64.h"


typedef struct TweenKeyframe_s* TweenKeyframe;


TweenKeyframe tweenkeyframe_init();
TweenKeyframe tweenkeyframe_init2(const AnimListItem* animlist_item);
TweenKeyframe tweenkeyframe_init3(AnimList animlist, const char* tweenkeyframe_name);
void tweenkeyframe_destroy(TweenKeyframe* tweenkeyframe);
TweenKeyframe tweenkeyframe_clone(TweenKeyframe tweenkeyframe);

void tweenkeyframe_animate_percent(TweenKeyframe tweenkeyframe, float64 percent);

int32_t tweenkeyframe_get_ids_count(TweenKeyframe tweenkeyframe);

float tweenkeyframe_peek_value(TweenKeyframe tweenkeyframe);
float tweenkeyframe_peek_value_by_index(TweenKeyframe tweenkeyframe, int32_t index);
bool tweenkeyframe_peek_entry_by_index(TweenKeyframe tweenkeyframe, int32_t index, int32_t* id, float* value);
float tweenkeyframe_peek_value_by_id(TweenKeyframe tweenkeyframe, int32_t id);

int32_t tweenkeyframe_add_ease(TweenKeyframe tweenkeyframe, float at, int32_t id, float value);
int32_t tweenkeyframe_add_easein(TweenKeyframe tweenkeyframe, float at, int32_t id, float value);
int32_t tweenkeyframe_add_easeout(TweenKeyframe tweenkeyframe, float at, int32_t id, float value);
int32_t tweenkeyframe_add_easeinout(TweenKeyframe tweenkeyframe, float at, int32_t id, float value);
int32_t tweenkeyframe_add_linear(TweenKeyframe tweenkeyframe, float at, int32_t id, float value);
int32_t tweenkeyframe_add_steps(TweenKeyframe tweenkeyframe, float at, int32_t id, float value, int32_t steps_count, Align steps_method);
int32_t tweenkeyframe_add_cubic(TweenKeyframe tweenkeyframe, float at, int32_t id, float value);
int32_t tweenkeyframe_add_quad(TweenKeyframe tweenkeyframe, float at, int32_t id, float value);
int32_t tweenkeyframe_add_expo(TweenKeyframe tweenkeyframe, float at, int32_t id, float value);
int32_t tweenkeyframe_add_sin(TweenKeyframe tweenkeyframe, float at, int32_t id, float value);
int32_t tweenkeyframe_add_interpolator(TweenKeyframe tweenkeyframe, float at, int32_t id, float value, AnimInterpolator type);

void tweenkeyframe_vertex_set_properties(TweenKeyframe tweenkeyframe, void* vertex, PropertySetter setter);

#endif
