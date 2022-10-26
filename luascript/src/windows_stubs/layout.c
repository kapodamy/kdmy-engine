#include "commons.h"
#include "layout.h"
#include "camera.h"
#include "sprite.h"
#include "pvrctx.h"

static TextSprite_t stub_textsprite = {};
static Sprite_t stub_sprite = {};
static SoundPlayer_t stub_soundplayer = {};
static Camera_t stub_camera = {};
static _LayoutPlaceholder_t stub_placeholder = {};
static Modifier_t stub_modifier = {};

int layout_trigger_any(Layout layout, const char* action_triger_camera_interval_name) {
    print_stub("layout_trigger_any", "layout=%p action_triger_camera_interval_name=%s", layout, action_triger_camera_interval_name);
    return 100;
}
int layout_trigger_action(Layout layout, const char* target_name, const char* action_name) {
    print_stub("layout_trigger_action", "layout=%p target_name=%s action_name=%s", layout, target_name, action_name);
    return 500;
}
int layout_trigger_camera(Layout layout, const char* camera_name) {
    print_stub("layout_trigger_camera", "layout=%p camera_name=%s", layout, camera_name);
    return 200;
}
int layout_trigger_trigger(Layout layout, const char* trigger_name) {
    print_stub("layout_trigger_trigger", "layout=%p trigger_name=%s", layout, trigger_name);
    return 200;
}
int layout_contains_action(Layout layout, const char* target_name, const char* action_name) {
    print_stub("layout_contains_action", "layout=%p target_name=%s action_name=%s", layout, target_name, action_name);
    return 200;
}
void layout_stop_all_triggers(Layout layout) {
    print_stub("layout_stop_all_triggers", "layout=%p", layout);
}
void layout_stop_trigger(Layout layout, const char* target_name) {
    print_stub("layout_stop_trigger", "layout=%p target_name=%s", layout, target_name);
}
int layout_animation_is_completed(Layout layout, const char* item_name) {
    print_stub("layout_animation_is_completed", "layout=%p item_name=%s", layout, item_name);
    return 600;
}
void layout_camera_set_view(Layout layout, float x, float y, float z) {
    print_stub("layout_camera_set_view", "layout=%p x=%f y=%f z=%f", layout, x, y, z);
}
int layout_camera_is_completed(Layout layout) {
    print_stub("layout_camera_is_completed", "layout=%p", layout);
    return 300;
}
Camera layout_get_camera_helper(Layout layout) {
    print_stub("layout_get_camera_helper", "layout=%p", layout);
    return &stub_camera;
}
Camera layout_get_secondary_camera_helper(Layout layout) {
    print_stub("layout_get_secondary_camera_helper", "layout=%p", layout);
    return &stub_camera;
}
TextSprite layout_get_textsprite(Layout layout, const char* name) {
    print_stub("layout_get_textsprite", "layout=%p name=%s", layout, name);
    return &stub_textsprite;
}
Sprite layout_get_sprite(Layout layout, const char* name) {
    print_stub("layout_get_sprite", "layout=%p name=%s", layout, name);
    return &stub_sprite;
}
SoundPlayer layout_get_soundplayer(Layout layout, const char* name) {
    print_stub("layout_get_soundplayer", "layout=%p name=%s", layout, name);
    return &stub_soundplayer;
}
float* layout_get_viewport_size(Layout layout, float* size) {
    size[0] = 640.0f;
    size[1] = 480.0f;
    print_stub("layout_get_viewport_size", "layout=%p size=%p", layout, size);
    return size;
}
int layout_get_attached_value2(Layout layout, const char* name, void* result) {
    print_stub("layout_get_attached_value2", "layout=%p name=%s result=%p", layout, name, result);
    return 500;
}
int layout_get_attached_value_type(Layout layout, const char* name) {
    print_stub("layout_get_attached_value_type", "layout=%p name=%s", layout, name);
    return 999;
}
void layout_set_group_visibility(Layout layout, const char* group_name, bool visible) {
    print_stub("layout_set_group_visibility", "layout=%p group_name=%s visible=(bool)%i", layout, group_name, visible);
}
void layout_set_group_alpha(Layout layout, const char* group_name, float alpha) {
    print_stub("layout_set_group_alpha", "layout=%p group_name=%s alpha=%f", layout, group_name, alpha);
}
void layout_set_group_offsetcolor(Layout layout, const char* group_name, float r, float g, float b, float a) {
    print_stub("layout_set_group_offsetcolor", "layout=%p group_name=%s r=%f g=%f b=%f a=%f", layout, group_name, r, g, b, a);
}
void layout_suspend(Layout layout) {
    print_stub("layout_suspend", "layout=%p", layout);
}
void layout_resume(Layout layout) {
    print_stub("layout_resume", "layout=%p", layout);
}
LayoutPlaceholder layout_get_placeholder(Layout layout, const char* name) {
    print_stub("layout_get_placeholder", "layout=%p name=%s", layout, name);
    return &stub_placeholder;
}
void layout_disable_antialiasing(Layout layout, bool disable) {
    print_stub("layout_disable_antialiasing", "layout=%p disable=(bool)%i", layout, disable);
}
void layout_set_group_antialiasing(Layout layout, const char* group_name, PVRFLAG antialiasing) {
    print_stub("layout_set_group_antialiasing", "layout=%p group_name=%s antialiasing=%i", layout, group_name, antialiasing);
}
Modifier layout_get_group_modifier(Layout layout, const char* group_name) {
    print_stub("layout_get_group_modifier", "layout=%p group_name=%s", layout, group_name);
    return &stub_modifier;
}

