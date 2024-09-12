#include "luascript_internal.h"

#include "layout.h"
#include "luascript_enums.h"
#include "math2d.h"


static int script_layout_trigger_any(lua_State* L) {
    Layout layout = luascript_read_userdata(L, LAYOUT);
    const char* action_triger_camera_trigger_name = luaL_optstring(L, 2, NULL);

    int32_t ret = layout_trigger_any(layout, action_triger_camera_trigger_name);
    lua_pushinteger(L, (lua_Integer)ret);

    return 1;
}

static int script_layout_trigger_action(lua_State* L) {
    Layout layout = luascript_read_userdata(L, LAYOUT);
    const char* target_name = luaL_optstring(L, 2, NULL);
    const char* action_name = luaL_optstring(L, 3, NULL);

    int32_t ret = layout_trigger_action(layout, target_name, action_name);
    lua_pushinteger(L, (lua_Integer)ret);

    return 1;
}

static int script_layout_trigger_camera(lua_State* L) {
    Layout layout = luascript_read_userdata(L, LAYOUT);
    const char* camera_name = luaL_optstring(L, 2, NULL);

    int ret = layout_trigger_camera(layout, camera_name);
    lua_pushboolean(L, ret);

    return 1;
}

static int script_layout_trigger_trigger(lua_State* L) {
    Layout layout = luascript_read_userdata(L, LAYOUT);
    const char* trigger_name = luaL_optstring(L, 2, NULL);

    int32_t ret = layout_trigger_trigger(layout, trigger_name);
    lua_pushinteger(L, (lua_Integer)ret);

    return 1;
}

static int script_layout_contains_action(lua_State* L) {
    Layout layout = luascript_read_userdata(L, LAYOUT);
    const char* target_name = luaL_optstring(L, 2, NULL);
    const char* action_name = luaL_optstring(L, 3, NULL);

    int32_t ret = layout_contains_action(layout, target_name, action_name);
    lua_pushinteger(L, (lua_Integer)ret);

    return 1;
}

static int script_layout_stop_all_triggers(lua_State* L) {
    Layout layout = luascript_read_userdata(L, LAYOUT);
    layout_stop_all_triggers(layout);
    return 0;
}

static int script_layout_stop_trigger(lua_State* L) {
    Layout layout = luascript_read_userdata(L, LAYOUT);
    const char* trigger_name = luaL_optstring(L, 2, NULL);

    layout_stop_trigger(layout, trigger_name);

    return 0;
}

static int script_layout_animation_is_completed(lua_State* L) {
    Layout layout = luascript_read_userdata(L, LAYOUT);
    const char* item_name = luaL_optstring(L, 2, NULL);

    int32_t ret = layout_animation_is_completed(layout, item_name);
    lua_pushinteger(L, (lua_Integer)ret);

    return 1;
}

static int script_layout_camera_set_view(lua_State* L) {
    Layout layout = luascript_read_userdata(L, LAYOUT);
    float x = (float)luaL_checknumber(L, 2);
    float y = (float)luaL_checknumber(L, 3);
    float z = (float)luaL_checknumber(L, 4);

    layout_camera_set_view(layout, x, y, z);

    return 0;
}

static int script_layout_camera_is_completed(lua_State* L) {
    Layout layout = luascript_read_userdata(L, LAYOUT);

    int32_t ret = layout_camera_is_completed(layout);
    lua_pushboolean(L, ret);

    return 1;
}

static int script_layout_get_camera_helper(lua_State* L) {
    Layout layout = luascript_read_userdata(L, LAYOUT);

    Camera camera = layout_get_camera_helper(layout);

    return script_camera_new(L, camera);
}

static int script_layout_get_secondary_camera_helper(lua_State* L) {
    Layout layout = luascript_read_userdata(L, LAYOUT);

    Camera camera = layout_get_secondary_camera_helper(layout);

    return script_camera_new(L, camera);
}

static int script_layout_get_textsprite(lua_State* L) {
    Layout layout = luascript_read_userdata(L, LAYOUT);

    const char* name = luaL_optstring(L, 2, NULL);

    TextSprite textsprite = layout_get_textsprite(layout, name);

    return script_textsprite_new(L, textsprite);
}

static int script_layout_get_soundplayer(lua_State* L) {
    Layout layout = luascript_read_userdata(L, LAYOUT);

    const char* name = luaL_optstring(L, 2, NULL);

    SoundPlayer soundplayer = layout_get_soundplayer(layout, name);

    return script_soundplayer_new(L, soundplayer);
}

static int script_layout_get_videoplayer(lua_State* L) {
    Layout layout = luascript_read_userdata(L, LAYOUT);
    const char* name = luaL_optstring(L, 2, NULL);

    VideoPlayer ret = layout_get_videoplayer(layout, name);

    return script_videoplayer_new(L, ret);
}

static int script_layout_get_sprite(lua_State* L) {
    Layout layout = luascript_read_userdata(L, LAYOUT);

    const char* name = luaL_optstring(L, 2, NULL);

    Sprite sprite = layout_get_sprite(layout, name);

    return script_sprite_new(L, sprite);
}

static int script_layout_get_viewport_size(lua_State* L) {
    Layout layout = luascript_read_userdata(L, LAYOUT);

    float viewport_width, viewport_height;
    layout_get_viewport_size(layout, &viewport_width, &viewport_height);

    lua_pushnumber(L, (lua_Number)viewport_width);
    lua_pushnumber(L, (lua_Number)viewport_height);

    return 2;
}

static int script_layout_get_attached_value(lua_State* L) {
    Layout layout = luascript_read_userdata(L, LAYOUT);
    const char* name = luaL_optstring(L, 2, NULL);

    if (name == NULL) {
        lua_pushnil(L);
        return 1;
    }

    LayoutAttachedValue value;
    AttachedValueType value_type = layout_get_attached_value2(layout, name, &value);

    switch (value_type) {
        case AttachedValueType_STRING:
            lua_pushstring(L, value.as_string);
            break;
        case AttachedValueType_FLOAT:
            lua_pushnumber(L, (lua_Number)value.as_double);
            break;
        case AttachedValueType_INTEGER:
            lua_pushinteger(L, (lua_Integer)value.as_long);
            break;
        case AttachedValueType_HEX:
            lua_pushinteger(L, (lua_Integer)((lua_Unsigned)value.as_unsigned));
            break;
        case AttachedValueType_BOOLEAN:
            lua_pushboolean(L, value.as_boolean);
            break;
        case AttachedValueType_NOTFOUND:
        default:
            lua_pushvalue(L, 3); // default value (taken from the arguments)
            break;
    }

    return 1;
}

static int script_layout_set_group_visibility(lua_State* L) {
    Layout layout = luascript_read_userdata(L, LAYOUT);
    const char* group_name = luaL_optstring(L, 2, NULL);
    int32_t visible = lua_toboolean(L, 3);

    layout_set_group_visibility(layout, group_name, visible);

    return 0;
}

static int script_layout_set_group_alpha(lua_State* L) {
    Layout layout = luascript_read_userdata(L, LAYOUT);
    const char* group_name = luaL_optstring(L, 2, NULL);
    float alpha = (float)luaL_checknumber(L, 3);

    layout_set_group_alpha(layout, group_name, alpha);

    return 0;
}

static int script_layout_set_group_offsetcolor(lua_State* L) {
    Layout layout = luascript_read_userdata(L, LAYOUT);
    const char* group_name = luaL_optstring(L, 2, NULL);
    float r = (float)luaL_optnumber(L, 3, FLOAT_NaN);
    float g = (float)luaL_optnumber(L, 4, FLOAT_NaN);
    float b = (float)luaL_optnumber(L, 5, FLOAT_NaN);
    float a = (float)luaL_optnumber(L, 6, FLOAT_NaN);

    layout_set_group_offsetcolor(layout, group_name, r, g, b, a);

    return 0;
}

static int script_layout_get_group_visibility(lua_State* L) {
    Layout layout = luascript_read_userdata(L, LAYOUT);
    const char* group_name = luaL_optstring(L, 2, NULL);

    bool ret = layout_get_group_visibility(layout, group_name);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_layout_suspend(lua_State* L) {
    Layout layout = luascript_read_userdata(L, LAYOUT);
    layout_suspend(layout);
    return 0;
}

static int script_layout_resume(lua_State* L) {
    Layout layout = luascript_read_userdata(L, LAYOUT);
    layout_resume(layout);
    return 0;
}

static int script_layout_get_placeholder(lua_State* L) {
    Layout layout = luascript_read_userdata(L, LAYOUT);
    const char* name = luaL_optstring(L, 2, NULL);

    LayoutPlaceholder* layoutplaceholder = layout_get_placeholder(layout, name);

    return script_layoutplaceholder_new(L, layoutplaceholder);
}

static int script_layout_disable_antialiasing(lua_State* L) {
    Layout layout = luascript_read_userdata(L, LAYOUT);

    bool disable = lua_toboolean(L, 3);

    layout_disable_antialiasing(layout, disable);

    return 0;
}

static int script_layout_set_group_antialiasing(lua_State* L) {
    Layout layout = luascript_read_userdata(L, LAYOUT);

    const char* group_name = luaL_optstring(L, 2, NULL);
    PVRFlag antialiasing = luascript_helper_optenum(L, 3, LUASCRIPT_ENUMS_PVRFlag);

    layout_set_group_antialiasing(layout, group_name, antialiasing);

    return 0;
}

static int script_layout_get_group_modifier(lua_State* L) {
    Layout layout = luascript_read_userdata(L, LAYOUT);

    const char* group_name = luaL_optstring(L, 2, NULL);

    Modifier* modifier = layout_get_group_modifier(layout, group_name);

    return script_modifier_new(L, modifier);
}

#ifndef _arch_dreamcast
static int script_layout_get_group_shader(lua_State* L) {
    Layout layout = luascript_read_userdata(L, LAYOUT);

    const char* name = luaL_optstring(L, 2, NULL);

    PSShader psshader = layout_get_group_shader(layout, name);

    return script_psshader_new(L, psshader);
}

static int script_layout_set_group_shader(lua_State* L) {
    Layout layout = luascript_read_userdata(L, LAYOUT);

    const char* name = luaL_optstring(L, 2, NULL);
    PSShader psshader = luascript_read_nullable_userdata(L, 3, PSSHADER);

    bool ret = layout_set_group_shader(layout, name, psshader);

    lua_pushboolean(L, ret);

    return 1;
}
#endif

static int script_layout_screen_to_layout_coordinates(lua_State* L) {
    Layout layout = luascript_read_userdata(L, LAYOUT);

    float screen_x = (float)luaL_checknumber(L, 2);
    float screen_y = (float)luaL_checknumber(L, 3);
    bool calc_with_camera = lua_toboolean(L, 4);

    float layout_x, layout_y;

    layout_screen_to_layout_coordinates(layout, screen_x, screen_y, calc_with_camera, &layout_x, &layout_y);

    lua_pushnumber(L, (lua_Number)layout_x);
    lua_pushnumber(L, (lua_Number)layout_y);
    return 2;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const luaL_Reg LAYOUT_FUNCTIONS[] = {
    {"trigger_any", script_layout_trigger_any},
    {"trigger_action", script_layout_trigger_action},
    {"trigger_camera", script_layout_trigger_camera},
    {"trigger_trigger", script_layout_trigger_trigger},
    {"contains_action", script_layout_contains_action},
    {"stop_all_triggers", script_layout_stop_all_triggers},
    {"stop_trigger", script_layout_stop_trigger},
    {"animation_is_completed", script_layout_animation_is_completed},
    {"camera_set_view", script_layout_camera_set_view},
    {"camera_is_completed", script_layout_camera_is_completed},
    {"get_camera_helper", script_layout_get_camera_helper},
    {"get_secondary_camera_helper", script_layout_get_secondary_camera_helper},
    {"get_textsprite", script_layout_get_textsprite},
    {"get_sprite", script_layout_get_sprite},
    {"get_soundplayer", script_layout_get_soundplayer},
    {"get_videoplayer", script_layout_get_videoplayer},
    {"get_viewport_size", script_layout_get_viewport_size},
    {"get_attached_value", script_layout_get_attached_value},
    {"set_group_visibility", script_layout_set_group_visibility},
    {"set_group_alpha", script_layout_set_group_alpha},
    {"set_group_offsetcolor", script_layout_set_group_offsetcolor},
    {"get_group_visibility", script_layout_get_group_visibility},
    {"suspend", script_layout_suspend},
    {"resume", script_layout_resume},
    {"get_placeholder", script_layout_get_placeholder},
    {"disable_antialiasing", script_layout_disable_antialiasing},
    {"set_group_antialiasing", script_layout_set_group_antialiasing},
    {"get_group_modifier", script_layout_get_group_modifier},
#ifndef _arch_dreamcast
    {"get_group_shader", script_layout_get_group_shader},
    {"set_group_shader", script_layout_set_group_shader},
#endif
    {"screen_to_layout_coordinates", script_layout_screen_to_layout_coordinates},
    {NULL, NULL}
};


int script_layout_new(lua_State* L, Layout layout) {
    return luascript_userdata_new(L, LAYOUT, layout);
}

static int script_layout_gc(lua_State* L) {
    return luascript_userdata_gc(L, LAYOUT);
}

static int script_layout_tostring(lua_State* L) {
    return luascript_userdata_tostring(L, LAYOUT);
}


void script_layout_register(lua_State* L) {
    luascript_register(L, LAYOUT, script_layout_gc, script_layout_tostring, LAYOUT_FUNCTIONS);
}
