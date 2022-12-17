#ifndef __camera_h
#define __camera_h

#include <stdint.h>
#include <stdbool.h>

#include "layout.h"
#include "animlist.h"
#include "modifier.h"
#include "pvrctx.h"
#include "animsprite.h"

typedef struct _Camera_t {
    int dummy;
} Camera_t;

typedef Camera_t *Camera;

void camera_set_interpolator_type(Camera camera, AnimInterpolator type);
void camera_set_transition_duration(Camera camera, bool expresed_in_beats, float value);
void camera_set_absolute_zoom(Camera camera, float z);
void camera_set_absolute_position(Camera camera, float x, float y);
void camera_set_offset(Camera camera, float x, float y, float z);
void camera_get_offset(Camera camera, float* x, float* y, float* z);
Modifier camera_get_modifier(Camera camera);
void camera_move(Camera camera, float end_x, float end_y, float end_z);
void camera_slide(Camera camera, float start_x, float start_y, float start_z, float end_x, float end_y, float end_z);
void camera_slide_x(Camera camera, float start, float end);
void camera_slide_y(Camera camera, float start, float end);
void camera_slide_z(Camera camera, float start, float end);
void camera_slide_to(Camera camera, float x, float y, float z);
bool camera_from_layout(Camera camera, Layout layout, const char* camera_name);
void camera_to_origin(Camera camera, bool should_slide);
void camera_repeat(Camera camera);
void camera_stop(Camera camera);
void camera_end(Camera camera);
bool camera_is_completed(Camera camera);
void camera_debug_log_info(Camera camera);
void camera_apply(Camera camera, PVRContext pvrctx);
void camera_move_offset(Camera camera, float end_x, float end_y, float end_z);
void camera_slide_offset(Camera camera, float start_x, float start_y, float start_z, float end_x, float end_y, float end_z);
void camera_slide_x_offset(Camera camera, float start, float end);
void camera_slide_y_offset(Camera camera, float start, float end);
void camera_slide_z_offset(Camera camera, float start, float end);
void camera_slide_to_offset(Camera camera, float x, float y, float z);
void camera_to_origin_offset(Camera camera, bool should_slide);
Layout camera_get_parent_layout(Camera camera);
void camera_set_animation(Camera camera, AnimSprite animsprite);

#endif

