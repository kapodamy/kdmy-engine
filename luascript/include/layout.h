#ifndef __layout_h
#define __layout_h

#include <stdint.h>
#include <stdbool.h>

#include "sprite.h"
#include "textsprite.h"
#include "soundplayer.h"
#include "videoplayer.h"
#include "modifier.h"
#include "psshader.h"
#include "camera.h"

// foward declaration
struct _Camera_t;
typedef struct _Camera_t Camera_t;
typedef Camera_t* Camera;

typedef struct _Layout_t {
    int dummy;
} Layout_t;

typedef Layout_t* Layout;

typedef struct _LayoutParallax_t {
    float x;
    float y;
    float z;
} LayoutParallax_t;


typedef struct {
    int32_t group_id;
    char* name;
    Align align_vertical;
    Align align_horizontal;

    float x;
    float y;
    float z;
    float height;
    float width;

    LayoutParallax_t parallax;
    bool static_camera;

    /*void* vertex;

    uint16_t actions_size;
    void* actions;*/
} LayoutPlaceholder_t;

typedef LayoutPlaceholder_t* LayoutPlaceholder;

typedef union {
    char* value_string;
    uint32_t value_boolean;
    int64_t value_integer;
    uint32_t value_unsigned;
    double value_float;
} LayoutAttachedValue;

#define LAYOUT_TYPE_NOTFOUND 0x00
#define LAYOUT_TYPE_STRING 0x01
#define LAYOUT_TYPE_FLOAT 0x02
#define LAYOUT_TYPE_INTEGER 0x04
#define LAYOUT_TYPE_HEX 0x08
#define LAYOUT_TYPE_BOOLEAN 0x10

typedef int LAYOUT_TYPE;


int layout_trigger_any(Layout layout, const char* action_triger_camera_interval_name);
int layout_trigger_action(Layout layout, const char* target_name, const char* action_name);
int layout_trigger_camera(Layout layout, const char* camera_name);
int layout_trigger_trigger(Layout layout, const char* trigger_name);
int layout_contains_action(Layout layout, const char* target_name, const char* action_name);
void layout_stop_all_triggers(Layout layout);
void layout_stop_trigger(Layout layout, const char* target_name);
int layout_animation_is_completed(Layout layout, const char* item_name);
void layout_camera_set_view(Layout layout, float x, float y, float z);
int layout_camera_is_completed(Layout layout);
Camera layout_get_camera_helper(Layout layout);
Camera layout_get_secondary_camera_helper(Layout layout);
TextSprite layout_get_textsprite(Layout layout, const char* name);
Sprite layout_get_sprite(Layout layout, const char* name);
SoundPlayer layout_get_soundplayer(Layout layout, const char* name);
VideoPlayer layout_get_videoplayer(Layout layout, const char* name);
void layout_get_viewport_size(Layout layout, float* viewport_width, float* viewport_height);
int layout_get_attached_value2(Layout layout, const char* name, void* result);
LAYOUT_TYPE layout_get_attached_value_type(Layout layout, const char* name);
void layout_set_group_visibility(Layout layout, const char* group_name, bool visible);
void layout_set_group_alpha(Layout layout, const char* group_name, float alpha);
void layout_set_group_offsetcolor(Layout layout, const char* group_name, float r, float g, float b, float a);
void layout_suspend(Layout layout);
void layout_resume(Layout layout);
LayoutPlaceholder layout_get_placeholder(Layout layout, const char* name);
void layout_disable_antialiasing(Layout layout, bool disable);
void layout_set_group_antialiasing(Layout layout, const char* group_name, PVRFlag antialiasing);
Modifier layout_get_group_modifier(Layout layout, const char* group_name);
PSShader layout_get_group_shader(Layout layout, const char* group_name);
bool layout_set_group_shader(Layout layout, const char* group_name, PSShader psshader);

#endif

