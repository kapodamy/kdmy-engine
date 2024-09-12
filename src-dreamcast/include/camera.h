#ifndef _camera_h
#define _camera_h

#include "animsprite.h"
#include "layout_types.h"
#include "macroexecutor_types.h"
#include "pvrcontext_types.h"


typedef struct Camera_s* Camera;

Camera camera_init(Modifier* modifier, float viewport_width, float viewport_height);
void camera_destroy(Camera* camera_ptr);

void camera_set_bpm(Camera camera, float beats_per_minute);
void camera_set_interpolator_type(Camera camera, AnimInterpolator type);
void camera_set_transition_duration(Camera camera, bool expresed_in_beats, float value);
void camera_set_absolute_zoom(Camera camera, float z);
void camera_set_absolute_position(Camera camera, float x, float y);
void camera_set_absolute(Camera camera, float x, float y, float z);

void camera_set_offset(Camera camera, float x, float y, float z);
void camera_get_offset(Camera camera, float* x, float* y, float* z);

void camera_replace_modifier(Camera camera, Modifier* new_modifier);
Modifier* camera_get_modifier(Camera camera);

void camera_move(Camera camera, float end_x, float end_y, float end_z);
void camera_move_offset(Camera camera, float end_x, float end_y, float end_z);
void camera_slide(Camera camera, float start_x, float start_y, float start_z, float end_x, float end_y, float end_z);
void camera_slide_offset(Camera camera, float start_x, float start_y, float start_z, float end_x, float end_y, float end_z);
void camera_slide_x(Camera camera, float start, float end);
void camera_slide_x_offset(Camera camera, float start, float end);
void camera_slide_y(Camera camera, float start, float end);
void camera_slide_y_offset(Camera camera, float start, float end);
void camera_slide_z(Camera camera, float start, float end);
void camera_slide_z_offset(Camera camera, float start, float end);
void camera_slide_to(Camera camera, float x, float y, float z);
void camera_slide_to_offset(Camera camera, float x, float y, float z);
bool camera_from_layout(Camera camera, Layout layout, const char* camera_name);
void camera_to_origin(Camera camera, bool should_slide);
void camera_to_origin_offset(Camera camera, bool should_slide);
Layout camera_get_parent_layout(Camera camera);
void camera_set_parent_layout(Camera camera, Layout layout);
void camera_set_animation(Camera camera, AnimSprite animsprite);

void camera_repeat(Camera camera);
void camera_stop(Camera camera);
void camera_end(Camera camera);

int32_t camera_animate(Camera camera, float elapsed);
void camera_apply(Camera camera, PVRContext pvrctx);
void camera_apply_offset(Camera camera, float* destination_matrix);
bool camera_is_completed(Camera camera);
void camera_disable_offset_zoom(Camera camera, bool disabled);
void camera_change_viewport(Camera camera, float width, float height);
void camera_set_property(Camera camera, int32_t id, float value);

void camera_debug_log_info(Camera camera);

#endif
