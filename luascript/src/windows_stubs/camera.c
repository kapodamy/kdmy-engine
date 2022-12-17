#include "camera.h"
#include "animlist.h"
#include "modifier.h"

#include "commons.h"

static Modifier_t stub_modifier = {};
static Layout_t stub_layout = {};

void camera_set_interpolator_type(Camera camera, AnimInterpolator type) {
    print_stub("camera_set_interpolator_type", "camera=%p type=%i", camera, type);
}
void camera_set_transition_duration(Camera camera, bool expresed_in_beats, float value) {
    print_stub("camera_set_transition_duration", "camera=%p expresed_in_beats=(bool)%i value=%f", camera, expresed_in_beats, value);
}
void camera_set_absolute_zoom(Camera camera, float z) {
    print_stub("camera_set_absolute_zoom", "camera=%p z=%f", camera, z);
}
void camera_set_absolute_position(Camera camera, float x, float y) {
    print_stub("camera_set_absolute_position", "camera=%p x=%f y=%f", camera, x, y);
}
void camera_set_offset(Camera camera, float x, float y, float z) {
    print_stub("camera_set_offset", "camera=%p x=%f y=%f z=%f", camera, x, y, z);
}
void camera_get_offset(Camera camera, float* x, float* y, float* z) {
    print_stub("camera_get_offset", "camera=%p x=%p y=%p z=%p", camera, x, y, z);
    *x = 10;
    *y = 20;
    *z = 30;
}
Modifier camera_get_modifier(Camera camera) {
    print_stub("camera_get_modifier", "camera=%p", camera);
    return &stub_modifier;
}
void camera_move(Camera camera, float end_x, float end_y, float end_z) {
    print_stub("camera_move", "camera=%p end_x=%f end_y=%f end_z=%f", camera, end_x, end_y, end_z);
}
void camera_slide(Camera camera, float start_x, float start_y, float start_z, float end_x, float end_y, float end_z) {
    print_stub("camera_slide", "camera=%p start_x=%f start_y=%f start_z=%f end_x=%f end_y=%f end_z=%f", camera, start_x, start_y, start_z, end_x, end_y, end_z);
}
void camera_slide_x(Camera camera, float start, float end) {
    print_stub("camera_slide_x", "camera=%p start=%f end=%f", camera, start, end);
}
void camera_slide_y(Camera camera, float start, float end) {
    print_stub("camera_slide_y", "camera=%p start=%f end=%f", camera, start, end);
}
void camera_slide_z(Camera camera, float start, float end) {
    print_stub("camera_slide_z", "camera=%p start=%f end=%f", camera, start, end);
}
void camera_slide_to(Camera camera, float x, float y, float z) {
    print_stub("camera_slide_to", "camera=%p x=%f y=%f z=%f", camera, x, y, z);
}
bool camera_from_layout(Camera camera, Layout layout, const char* camera_name) {
    print_stub("camera_from_layout", "camera=%p layout=%p camera_name=%s", camera, layout, camera_name);
    return 0;
}
void camera_to_origin(Camera camera, bool should_slide) {
    print_stub("camera_to_origin", "camera=%p should_slide=(bool)%i", camera, should_slide);
}
void camera_repeat(Camera camera) {
    print_stub("camera_repeat", "camera=%p", camera);
}
void camera_stop(Camera camera) {
    print_stub("camera_stop", "camera=%p", camera);
}
void camera_end(Camera camera) {
    print_stub("camera_end", "camera=%p", camera);
}
bool camera_is_completed(Camera camera) {
    print_stub("camera_is_completed", "camera=%p", camera);
    return 0;
}
void camera_debug_log_info(Camera camera) {
    print_stub("camera_debug_log_info", "camera=%p", camera);
}
void camera_apply(Camera camera, PVRContext pvrctx) {
    print_stub("camera_apply", "camera=%p pvrctx=%p", camera, pvrctx);
}
void camera_move_offset(Camera camera, float end_x, float end_y, float end_z) {
    print_stub("camera_move_offset", "camera=%p end_x=%f end_y=%f end_z=%f", camera, end_x, end_y, end_z);
}
void camera_slide_offset(Camera camera, float start_x, float start_y, float start_z, float end_x, float end_y, float end_z) {
    print_stub("camera_slide_offset", "camera=%p start_x=%f start_y=%f start_z=%f end_x=%f end_y=%f end_z=%f", camera, start_x, start_y, start_z, end_x, end_y, end_z);
}
void camera_slide_x_offset(Camera camera, float start, float end) {
    print_stub("camera_slide_x_offset", "camera=%p start=%f end=%f", camera, start, end);
}
void camera_slide_y_offset(Camera camera, float start, float end) {
    print_stub("camera_slide_y_offset", "camera=%p start=%f end=%f", camera, start, end);
}
void camera_slide_z_offset(Camera camera, float start, float end) {
    print_stub("camera_slide_z_offset", "camera=%p start=%f end=%f", camera, start, end);
}
void camera_slide_to_offset(Camera camera, float x, float y, float z) {
    print_stub("camera_slide_to_offset", "camera=%p x=%f y=%f z=%f", camera, x, y, z);
}
void camera_to_origin_offset(Camera camera, bool should_slide) {
    print_stub("camera_to_origin_offset", "camera=%p should_slide=(bool)%i", camera, should_slide);
}
Layout camera_get_parent_layout(Camera camera) {
    print_stub("camera_get_parent_layout", "camera=%p", camera);
    return &stub_layout;
}
void camera_set_animation(Camera camera, AnimSprite animsprite) {
    print_stub("camera_set_animation", "camera=%p animsprite=%p", camera, animsprite);
}
