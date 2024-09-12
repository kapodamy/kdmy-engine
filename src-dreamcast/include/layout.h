#ifndef _layout_h
#define _layout_h

#include "layout_types.h"
#include "sh4matrix_types.h"


#define layout_get_attached_value_string(LAYOUT, ATTACHED_VALUE_NAME, DEFAULT_VALUE) layout_get_attached_value(LAYOUT, ATTACHED_VALUE_NAME, AttachedValueType_STRING, LYT_T(string, DEFAULT_VALUE)).as_string
#define layout_get_attached_value_double(LAYOUT, ATTACHED_VALUE_NAME, DEFAULT_VALUE) layout_get_attached_value(LAYOUT, ATTACHED_VALUE_NAME, AttachedValueType_FLOAT, LYT_T(double, DEFAULT_VALUE)).as_double
#define layout_get_attached_value_long(LAYOUT, ATTACHED_VALUE_NAME, DEFAULT_VALUE) layout_get_attached_value(LAYOUT, ATTACHED_VALUE_NAME, AttachedValueType_INTEGER, LYT_T(long, DEFAULT_VALUE)).as_long
#define layout_get_attached_value_hex(LAYOUT, ATTACHED_VALUE_NAME, DEFAULT_VALUE) layout_get_attached_value(LAYOUT, ATTACHED_VALUE_NAME, AttachedValueType_HEX, LYT_T(unsigned, DEFAULT_VALUE)).as_unsigned
#define layout_get_attached_value_boolean(LAYOUT, ATTACHED_VALUE_NAME, DEFAULT_VALUE) layout_get_attached_value(LAYOUT, ATTACHED_VALUE_NAME, AttachedValueType_BOOLEAN, LYT_T(boolean, DEFAULT_VALUE)).as_boolean


//
//  forward references
//

typedef struct TextSprite_s* TextSprite;
typedef struct Sprite_s* Sprite;
typedef struct SoundPlayer_s* SoundPlayer;
typedef struct VideoPlayer_s* VideoPlayer;
typedef struct Camera_s* Camera;
typedef struct FontHolder_s* FontHolder;
typedef struct Modifier_s Modifier;

extern bool LAYOUT_DEBUG_PRINT_TRIGGER_CALLS;

Layout layout_init(const char* src);
void layout_destroy(Layout* layout);

int32_t layout_trigger_any(Layout layout, const char* action_triger_camera_interval_name);
int32_t layout_trigger_action(Layout layout, const char* target_name, const char* action_name);
int32_t layout_contains_action(Layout layout, const char* target_name, const char* action_name);
int32_t layout_animation_is_completed(Layout layout, const char* item_name);
void layout_update_render_size(Layout layout, float screen_width, float screen_height);
void layout_screen_to_layout_coordinates(Layout layout, float screen_x, float screen_y, bool calc_with_camera, float* layout_x, float* layout_y);
void layout_camera_set_view(Layout layout, float x, float y, float depth);
bool layout_camera_is_completed(Layout layout);
bool layout_trigger_camera(Layout layout, const char* camera_name);
int32_t layout_trigger_trigger(Layout layout, const char* trigger_name);
void layout_stop_trigger(Layout layout, const char* trigger_name);
void layout_stop_all_triggers(Layout layout);
bool layout_set_placeholder_drawable_by_id(Layout layout, int32_t id, Drawable drawable);
void layout_sync_triggers_with_global_beatwatcher(Layout layout, bool enable);
void layout_set_bpm(Layout layout, float beats_per_minute);

LayoutPlaceholder* layout_get_placeholder(Layout layout, const char* name);
int32_t layout_get_placeholder_id(Layout layout, const char* name);

TextSprite layout_get_textsprite(Layout layout, const char* name);
Sprite layout_get_sprite(Layout layout, const char* name);
SoundPlayer layout_get_soundplayer(Layout layout, const char* name);
VideoPlayer layout_get_videoplayer(Layout layout, const char* name);
Camera layout_get_camera_helper(Layout layout);
Camera layout_get_secondary_camera_helper(Layout layout);

FontHolder layout_get_attached_font(Layout layout, const char* font_name);
LayoutAttachedValue layout_get_attached_value(Layout layout, const char* name, AttachedValueType expected_type, LayoutAttachedValue default_value);
AttachedValueType layout_get_attached_value2(Layout layout, const char* name, void* result_ptr);
float layout_get_attached_value_as_float(Layout layout, const char* name, float default_value);
AttachedValueType layout_get_attached_value_type(Layout layout, const char* name);
Modifier* layout_get_modifier_viewport(Layout layout);
Modifier* layout_get_modifier_camera(Layout layout);
void layout_get_viewport_size(Layout layout, float* viewport_width, float* viewport_height);
void layout_external_vertex_create_entries(Layout layout, int32_t amount);
bool layout_external_vertex_set_entry(Layout layout, int32_t index, PVRVertex vertex_type, void* vertex, int32_t group_id);
int32_t layout_external_create_group(Layout layout, const char* group_name, int32_t parent_group_id);
bool layout_set_group_static_to_camera(Layout layout, const char* group_name, bool enable);
bool layout_set_group_static_to_camera_by_id(Layout layout, int32_t group_id, bool enable);
bool layout_set_group_static_to_screen_by_id(Layout layout, int32_t group_id, SH4Matrix sh4matrix);
bool layout_external_vertex_set_entry_static(Layout layout, int32_t vertex_index, bool enable);
void layout_set_group_visibility(Layout layout, const char* group_name, bool visible);
void layout_set_group_visibility_by_id(Layout layout, int32_t group_id, bool visible);
void layout_set_group_alpha(Layout layout, const char* group_name, float alpha);
void layout_set_group_alpha_by_id(Layout layout, int32_t group_id, float alpha);
void layout_set_group_antialiasing(Layout layout, const char* group_name, PVRFlag antialiasing);
void layout_set_group_antialiasing_by_id(Layout layout, int32_t group_id, PVRFlag antialiasing);
void layout_set_group_offsetcolor(Layout layout, const char* group_name, float r, float g, float b, float a);
Modifier* layout_get_group_modifier(Layout layout, const char* group_name);
Modifier* layout_get_group_modifier_by_id(Layout layout, int32_t group_id);
int32_t layout_get_group_id(Layout layout, const char* group_name);
bool layout_get_group_visibility(Layout layout, const char* group_name);
bool layout_get_group_visibility_by_id(Layout layout, int32_t group_id);
const CameraPlaceholder* layout_get_camera_placeholder(Layout layout, const char* camera_name);
bool layout_set_single_item_to_draw(Layout layout, const char* item_name);
void layout_suspend(Layout layout);
void layout_resume(Layout layout);
void layout_disable_antialiasing(Layout layout, bool disable);
bool layout_is_antialiasing_disabled(Layout layout);
PVRFlag layout_get_layout_antialiasing(Layout layout);
void layout_set_layout_antialiasing(Layout layout, PVRFlag flag);

/*void layout_set_shader(Layout layout, PSShader psshader);
PSShader layout_get_group_shader(Layout layout, const char* group_name);
bool layout_set_group_shader(Layout layout, const char* group_name, PSShader psshader);*/

int32_t layout_animate(Layout layout, float elapsed);
void layout_draw(Layout layout, PVRContext pvrctx);

#endif
