#include "luascript_internal.h"


#ifdef JAVASCRIPT

EM_JS_PRFX(int, layout_trigger_any, (Layout layout, const char* action_triger_camera_interval_name), {
    return layout_trigger_any(kdmyEngine_obtain(layout), kdmyEngine_ptrToString(action_triger_camera_interval_name));
});
EM_JS_PRFX(int, layout_trigger_action, (Layout layout, const char* target_name, const char* action_name), {
    return layout_trigger_action(kdmyEngine_obtain(layout), kdmyEngine_ptrToString(target_name), kdmyEngine_ptrToString(action_name));
});
EM_JS_PRFX(int, layout_trigger_camera, (Layout layout, const char* camera_name), {
    return layout_trigger_camera(kdmyEngine_obtain(layout), kdmyEngine_ptrToString(camera_name));
});
EM_JS_PRFX(int, layout_trigger_trigger, (Layout layout, const char* trigger_name), {
    return layout_trigger_trigger(kdmyEngine_obtain(layout), kdmyEngine_ptrToString(trigger_name));
});
EM_JS_PRFX(int, layout_contains_action, (Layout layout, const char* target_name, const char* action_name), {
    return layout_contains_action(kdmyEngine_obtain(layout), kdmyEngine_ptrToString(target_name), kdmyEngine_ptrToString(action_name));
});
EM_JS_PRFX(void, layout_stop_all_triggers, (Layout layout), {
    return layout_stop_all_triggers(kdmyEngine_obtain(layout));
});
EM_JS_PRFX(void, layout_stop_trigger, (Layout layout, const char* trigger_name), {
    return layout_stop_trigger(kdmyEngine_obtain(layout), kdmyEngine_ptrToString(trigger_name));
});
EM_JS_PRFX(int, layout_animation_is_completed, (Layout layout, const char* item_name), {
    return layout_animation_is_completed(kdmyEngine_obtain(layout), kdmyEngine_ptrToString(item_name));
});
EM_JS_PRFX(void, layout_camera_set_view, (Layout layout, float x, float y, float z), {
    layout_camera_set_view(kdmyEngine_obtain(layout), x, y, z);
});
EM_JS_PRFX(int, layout_camera_is_completed, (Layout layout), {
    return layout_camera_is_completed(kdmyEngine_obtain(layout));
});
EM_JS_PRFX(Camera, layout_get_camera_helper, (Layout layout), {
    const camera = layout_get_camera_helper(kdmyEngine_obtain(layout));
    return kdmyEngine_obtain(camera);
});
EM_JS_PRFX(Camera, layout_get_secondary_camera_helper, (Layout layout), {
    const camera = layout_get_secondary_camera_helper(kdmyEngine_obtain(layout));
    return kdmyEngine_obtain(camera);
});
EM_JS_PRFX(TextSprite, layout_get_textsprite, (Layout layout, const char* name), {
    const textsprite = layout_get_textsprite(kdmyEngine_obtain(layout), kdmyEngine_ptrToString(name));
    return kdmyEngine_obtain(textsprite);
});
EM_JS_PRFX(Sprite, layout_get_sprite, (Layout layout, const char* name), {
    const sprite = layout_get_sprite(kdmyEngine_obtain(layout), kdmyEngine_ptrToString(name));
    return kdmyEngine_obtain(sprite);
});
EM_JS_PRFX(SoundPlayer, layout_get_soundplayer, (Layout layout, const char* name), {
    const soundplayer = layout_get_soundplayer(kdmyEngine_obtain(layout), kdmyEngine_ptrToString(name));
    return kdmyEngine_obtain(soundplayer);
});
EM_JS_PRFX(VideoPlayer, layout_get_videoplayer, (Layout layout, const char* name), {
    let ret = layout_get_videoplayer(kdmyEngine_obtain(layout), kdmyEngine_ptrToString(name));
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(void, layout_get_viewport_size, (Layout layout, float* viewport_width, float* viewport_height), {
    const values = [ 0, 0 ];
    layout_get_viewport_size(kdmyEngine_obtain(layout), values);
    
    kdmyEngine_set_float32(viewport_width, values[0]);
    kdmyEngine_set_float32(viewport_height, values[1]);
});
EM_JS_PRFX(int, layout_get_attached_value2, (Layout layout, const char* name, void* result), {
    const value = [null];
    let type = layout_get_attached_value2(kdmyEngine_obtain(layout), kdmyEngine_ptrToString(name), value);

    switch (type) {
        case LAYOUT_TYPE_NOTFOUND:
            // nothing to do
            break;
        case LAYOUT_TYPE_STRING:
            kdmyEngine_dataView.setUint32(result, kdmyEngine_stringToPtr(value[0]), kdmyEngine_endianess);
            break;
        case LAYOUT_TYPE_FLOAT:
            kdmyEngine_dataView.setFloat64(result, value[0], kdmyEngine_endianess);
            break;
        case LAYOUT_TYPE_INTEGER:
            kdmyEngine_dataView.setBigInt64(result, BigInt(value[0]), kdmyEngine_endianess);
            break;
        case LAYOUT_TYPE_HEX:
            kdmyEngine_set_uint32(result, value[0]);
            break;
        case LAYOUT_TYPE_BOOLEAN:
            kdmyEngine_set_int32(result, value[0] ? 1 : 0);
            break;
        default:
            console.warn("Unknown layout type-value ", type, value[0]);
            break;
    }

    return type;
});
EM_JS_PRFX(LAYOUT_TYPE, layout_get_attached_value_type, (Layout layout, const char* name), {
    return layout_get_attached_value_type(kdmyEngine_obtain(layout), kdmyEngine_ptrToString(name));
});
EM_JS_PRFX(void, layout_set_group_visibility, (Layout layout, const char* group_name, bool visible), {
    layout_set_group_visibility(kdmyEngine_obtain(layout), kdmyEngine_ptrToString(group_name), visible);
});
EM_JS_PRFX(void, layout_set_group_alpha, (Layout layout, const char* group_name, float alpha), {
    layout_set_group_alpha(kdmyEngine_obtain(layout), kdmyEngine_ptrToString(group_name), alpha);
});
EM_JS_PRFX(void, layout_set_group_offsetcolor, (Layout layout, const char* group_name, float r, float g, float b, float a), {
    layout_set_group_offsetcolor(kdmyEngine_obtain(layout), kdmyEngine_ptrToString(group_name), r, g, b, a);
});
EM_JS_PRFX(void, layout_suspend, (Layout layout), {
    layout_suspend(kdmyEngine_obtain(layout));
});
EM_JS_PRFX(void, layout_resume, (Layout layout), {
    layout_resume(kdmyEngine_obtain(layout));
});
EM_JS_PRFX(LayoutPlaceholder, layout_get_placeholder, (Layout layout, const char* group_name), {
    let placeholder = layout_get_placeholder(kdmyEngine_obtain(layout), kdmyEngine_ptrToString(group_name));
    return kdmyEngine_obtain(placeholder);
});
EM_JS_PRFX(void, layout_disable_antialiasing, (Layout layout, bool antialiasing), {
    layout_disable_antialiasing(kdmyEngine_obtain(layout), antialiasing);
});
EM_JS_PRFX(void, layout_set_group_antialiasing, (Layout layout, const char* group_name, PVRFlag antialiasing), {
    layout_set_group_antialiasing(kdmyEngine_obtain(layout), kdmyEngine_ptrToString(group_name), antialiasing);
});
EM_JS_PRFX(Modifier, layout_get_group_modifier, (Layout layout, const char* group_name), {
    let modifier = layout_get_group_modifier(kdmyEngine_obtain(layout), kdmyEngine_ptrToString(group_name));
    return kdmyEngine_obtain(modifier);
});
EM_JS_PRFX(PSShader, layout_get_group_shader, (Layout layout, const char* group_name), {
    let psshader = layout_get_group_shader(kdmyEngine_obtain(layout), kdmyEngine_ptrToString(group_name));
    return kdmyEngine_obtain(psshader);
});
EM_JS_PRFX(bool, layout_set_group_shader, (Layout layout, const char* group_name, PSShader psshader), {
    return layout_set_group_shader(
        kdmyEngine_obtain(layout), kdmyEngine_ptrToString(group_name), kdmyEngine_obtain(psshader)
    );
});
EM_JS_PRFX(void, layout_screen_to_layout_coordinates, (Layout layout, float screen_x, float screen_y, bool calc_with_camera, float* layout_x, float* layout_y), {
    const output_coords = [0, 0];
    layout_screen_to_layout_coordinates(kdmyEngine_obtain(layout), screen_x, screen_y, calc_with_camera, output_coords);

    kdmyEngine_set_float32(layout_x, output_coords[0]);
    kdmyEngine_set_float32(layout_y, output_coords[1]);
});
#endif



static int script_layout_trigger_any(lua_State* L) {
    Layout layout = luascript_read_userdata(L, LAYOUT);
    const char* action_triger_camera_trigger_name = luaL_optstring(L, 2, NULL);

    int ret = layout_trigger_any(layout, action_triger_camera_trigger_name);
    lua_pushinteger(L, ret);

    return 1;
}

static int script_layout_trigger_action(lua_State* L) {
    Layout layout = luascript_read_userdata(L, LAYOUT);
    const char* target_name = luaL_optstring(L, 2, NULL);
    const char* action_name = luaL_optstring(L, 3, NULL);

    int ret = layout_trigger_action(layout, target_name, action_name);
    lua_pushinteger(L, ret);

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

    int ret = layout_trigger_trigger(layout, trigger_name);
    lua_pushinteger(L, ret);

    return 1;
}

static int script_layout_contains_action(lua_State* L) {
    Layout layout = luascript_read_userdata(L, LAYOUT);
    const char* target_name = luaL_optstring(L, 2, NULL);
    const char* action_name = luaL_optstring(L, 3, NULL);

    int ret = layout_contains_action(layout, target_name, action_name);
    lua_pushinteger(L, ret);

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

    int ret = layout_animation_is_completed(layout, item_name);
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

    int ret = layout_camera_is_completed(layout);
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

    lua_pushnumber(L, viewport_width);
    lua_pushnumber(L, viewport_height);

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
    int value_type = layout_get_attached_value2(layout, name, &value);

    switch (value_type) {
        case LAYOUT_TYPE_STRING:
            lua_pushstring(L, value.value_string);
            break;
        case LAYOUT_TYPE_FLOAT:
            lua_pushnumber(L, value.value_float);
            break;
        case LAYOUT_TYPE_INTEGER:
            lua_pushinteger(L, value.value_integer);
            break;
        case LAYOUT_TYPE_HEX:
            lua_pushinteger(L, value.value_unsigned);
            break;
        case LAYOUT_TYPE_BOOLEAN:
            lua_pushboolean(L, value.value_boolean);
            break;
        case LAYOUT_TYPE_NOTFOUND:
        default:
            lua_pushvalue(L, 3); // default value (taken from the arguments)
            break;
    }

#ifdef JAVASCRIPT
    if (value_type == LAYOUT_TYPE_STRING) {
        // the javascript side returned a copy of the string, dispose it
        free(value.value_string);
    }
#endif

    return 1;
}

static int script_layout_set_group_visibility(lua_State* L) {
    Layout layout = luascript_read_userdata(L, LAYOUT);
    const char* group_name = luaL_optstring(L, 2, NULL);
    int visible = lua_toboolean(L, 3);

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
    float r = (float)luaL_optnumber(L, 3, NAN);
    float g = (float)luaL_optnumber(L, 4, NAN);
    float b = (float)luaL_optnumber(L, 5, NAN);
    float a = (float)luaL_optnumber(L, 6, NAN);

    layout_set_group_offsetcolor(layout, group_name, r, g, b, a);

    return 0;
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

    LayoutPlaceholder layoutplaceholder = layout_get_placeholder(layout, name);

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
    PVRFlag antialiasing = luascript_parse_pvrflag(L, luaL_optstring(L, 3, NULL));

    layout_set_group_antialiasing(layout, group_name, antialiasing);

    return 0;
}

static int script_layout_get_group_modifier(lua_State* L) {
    Layout layout = luascript_read_userdata(L, LAYOUT);

    const char* group_name = luaL_optstring(L, 2, NULL);

    Modifier modifier = layout_get_group_modifier(layout, group_name);

    return script_modifier_new(L, modifier);
}

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

static int script_layout_screen_to_layout_coordinates(lua_State* L) {
    Layout layout = luascript_read_userdata(L, LAYOUT);

    float screen_x = (float)luaL_checknumber(L, 2);
    float screen_y = (float)luaL_checknumber(L, 3);
    bool calc_with_camera = lua_toboolean(L, 4);

    float layout_x, layout_y;

    layout_screen_to_layout_coordinates(layout, screen_x, screen_y, calc_with_camera, &layout_x, &layout_y);

    lua_pushnumber(L, layout_x);
    lua_pushnumber(L, layout_y);
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
    {"suspend", script_layout_suspend},
    {"resume", script_layout_resume},
    {"get_placeholder", script_layout_get_placeholder},
    {"disable_antialiasing", script_layout_disable_antialiasing},
    {"set_group_antialiasing", script_layout_set_group_antialiasing},
    {"get_group_modifier", script_layout_get_group_modifier},
    {"get_group_shader", script_layout_get_group_shader},
    {"set_group_shader", script_layout_set_group_shader},
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
