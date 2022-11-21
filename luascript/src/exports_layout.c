#include "luascript_internal.h"



#ifdef JAVASCRIPT

typedef struct {
    int32_t group_id;

    int32_t align_vertical;
    int32_t align_horizontal;

    float x;
    float y;
    float z;

    float height;
    float width;

    float parallax_x;
    float parallax_y;
    float parallax_z;

    int32_t static_camera;
} LayoutPlaceholderJS_t;


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
EM_JS_PRFX(float*, layout_get_viewport_size, (Layout layout, float* size), {
    const HEAP_ENDIANESS = true;
    const values = [0, 0];
    const dataView = new DataView(buffer);

    layout_get_viewport_size(kdmyEngine_obtain(layout), values);
    dataView.setFloat32(size + 0, values[0], HEAP_ENDIANESS);
    dataView.setFloat32(size + 4, values[1], HEAP_ENDIANESS);

    return size;
    });
EM_JS_PRFX(int, layout_get_attached_value2, (Layout layout, const char* name, void* result), {
    const dataView = new DataView(buffer);
    const HEAP_ENDIANESS = true;

    const value = [null];
    let type = layout_get_attached_value2(kdmyEngine_obtain(layout), kdmyEngine_ptrToString(name), value);

    switch (type) {
        case LAYOUT_TYPE_NOTFOUND:
            // nothing to do
            break;
        case LAYOUT_TYPE_STRING:
            let str_ptr = kdmyEngine_stringToPtr(value[0]);
            dataView.setUint32(result, str_ptr, HEAP_ENDIANESS);
            break;
        case LAYOUT_TYPE_FLOAT:
            dataView.setFloat64(result, value[0], HEAP_ENDIANESS);
            break;
        case LAYOUT_TYPE_INTEGER:
            dataView.setBigInt64(result, value[0], HEAP_ENDIANESS);
            break;
        case LAYOUT_TYPE_HEX:
            dataView.setUint32(result, value[0], HEAP_ENDIANESS);
            break;
        case LAYOUT_TYPE_BOOLEAN:
            // bool value should have a size of 4bytes
            dataView.setUint32(result, value[0], HEAP_ENDIANESS);
            break;
        default:
            console.warn("Unknown layout type-value ", type, value[0]);
            throw new Error("Unimplemented type: " + type);
    }

    return type;
    });
EM_JS_PRFX(int, layout_get_attached_value_type, (Layout layout, const char* name), {
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
EM_JS_PRFX(bool, layout_get_placeholder_JS, (Layout layout, const char* group_name, LayoutPlaceholderJS_t* output), {
    const dataView = new DataView(buffer);
    const HEAP_ENDIANESS = true;

    let placeholder = layout_get_placeholder(kdmyEngine_obtain(layout), kdmyEngine_ptrToString(group_name));
    if (!placeholder) return 0;

    dataView.setInt32(output, placeholder.group_id, HEAP_ENDIANESS);output += 4;

    dataView.setInt32(output, placeholder.align_vertical, HEAP_ENDIANESS);output += 4;
    dataView.setInt32(output, placeholder.align_horizontal, HEAP_ENDIANESS);output += 4;

    dataView.setFloat32(output, placeholder.x, HEAP_ENDIANESS);output += 4;
    dataView.setFloat32(output, placeholder.y, HEAP_ENDIANESS);output += 4;
    dataView.setFloat32(output, placeholder.z, HEAP_ENDIANESS);output += 4;

    dataView.setFloat32(output, placeholder.height, HEAP_ENDIANESS);output += 4;
    dataView.setFloat32(output, placeholder.width, HEAP_ENDIANESS);output += 4;

    dataView.setFloat32(output, placeholder.parallax_x, HEAP_ENDIANESS);output += 4;
    dataView.setFloat32(output, placeholder.parallax_y, HEAP_ENDIANESS);output += 4;
    dataView.setFloat32(output, placeholder.parallax_z, HEAP_ENDIANESS);output += 4;

    dataView.setInt32(output, placeholder.static_camera ? 1 : 0, HEAP_ENDIANESS);

    return 1;
    });
EM_JS_PRFX(void, layout_disable_antialiasing, (Layout layout, bool antialiasing), {
    layout_disable_antialiasing(kdmyEngine_obtain(layout), antialiasing);
    });
EM_JS_PRFX(void, layout_set_group_antialiasing, (Layout layout, const char* group_name, PVRFLAG antialiasing), {
    layout_set_group_antialiasing(kdmyEngine_obtain(layout), kdmyEngine_ptrToString(group_name), antialiasing);
    });
EM_JS_PRFX(Modifier, layout_get_group_modifier, (Layout layout, const char* group_name), {
    let modifier = layout_get_group_modifier(kdmyEngine_obtain(layout), kdmyEngine_ptrToString(group_name));
    return kdmyEngine_obtain(modifier);
    });
#endif




typedef union {
    const int value_boolean;
    const int64_t value_integer;
    const uint32_t value_unsigned;
    const double value_float;
#ifdef JAVASCRIPT
    char* value_string;
#else
    const char* value_string;
#endif
} LayoutAttachedValue;

#define LuaL_add_table_field(L, field_name, setter, value)  lua_pushstring(L, field_name); \
                                                            setter(L, (value)); \
                                                            lua_settable(L, -3);

static int script_layout_trigger_any(lua_State* L) {
    READ_USERDATA(L, Layout, layout, LAYOUT);
    const char* action_triger_camera_trigger_name = luaL_optstring(L, 2, NULL);

    int ret = layout_trigger_any(layout, action_triger_camera_trigger_name);
    lua_pushinteger(L, ret);

    return 1;
}

static int script_layout_trigger_action(lua_State* L) {
    READ_USERDATA(L, Layout, layout, LAYOUT);
    const char* target_name = luaL_optstring(L, 2, NULL);
    const char* action_name = luaL_optstring(L, 3, NULL);

    int ret = layout_trigger_action(layout, target_name, action_name);
    lua_pushinteger(L, ret);

    return 1;
}

static int script_layout_trigger_camera(lua_State* L) {
    READ_USERDATA(L, Layout, layout, LAYOUT);
    const char* camera_name = luaL_optstring(L, 2, NULL);

    int ret = layout_trigger_camera(layout, camera_name);
    lua_pushboolean(L, ret);

    return 1;
}

static int script_layout_trigger_trigger(lua_State* L) {
    READ_USERDATA(L, Layout, layout, LAYOUT);
    const char* trigger_name = luaL_optstring(L, 2, NULL);

    int ret = layout_trigger_trigger(layout, trigger_name);
    lua_pushinteger(L, ret);

    return 1;
}

static int script_layout_contains_action(lua_State* L) {
    READ_USERDATA(L, Layout, layout, LAYOUT);
    const char* target_name = luaL_optstring(L, 2, NULL);
    const char* action_name = luaL_optstring(L, 3, NULL);

    int ret = layout_contains_action(layout, target_name, action_name);
    lua_pushinteger(L, ret);

    return 1;
}

static int script_layout_stop_all_triggers(lua_State* L) {
    READ_USERDATA(L, Layout, layout, LAYOUT);
    layout_stop_all_triggers(layout);
    return 0;
}

static int script_layout_stop_trigger(lua_State* L) {
    READ_USERDATA(L, Layout, layout, LAYOUT);
    const char* trigger_name = luaL_optstring(L, 2, NULL);

    layout_stop_trigger(layout, trigger_name);

    return 0;
}

static int script_layout_animation_is_completed(lua_State* L) {
    READ_USERDATA(L, Layout, layout, LAYOUT);
    const char* item_name = luaL_optstring(L, 2, NULL);

    int ret = layout_animation_is_completed(layout, item_name);
    lua_pushinteger(L, (lua_Integer)ret);

    return 1;
}

static int script_layout_camera_set_view(lua_State* L) {
    READ_USERDATA(L, Layout, layout, LAYOUT);
    float x = luaL_checkfloat(L, 2);
    float y = luaL_checkfloat(L, 3);
    float z = luaL_checkfloat(L, 4);

    layout_camera_set_view(layout, x, y, z);

    return 0;
}

static int script_layout_camera_is_completed(lua_State* L) {
    READ_USERDATA(L, Layout, layout, LAYOUT);

    int ret = layout_camera_is_completed(layout);
    lua_pushboolean(L, ret);

    return 1;
}

static int script_layout_get_camera_helper(lua_State* L) {
    READ_USERDATA(L, Layout, layout, LAYOUT);

    Camera camera = layout_get_camera_helper(layout);

    return script_camera_new(L, layout, camera);
}

static int script_layout_get_secondary_camera_helper(lua_State* L) {
    READ_USERDATA(L, Layout, layout, LAYOUT);

    Camera camera = layout_get_secondary_camera_helper(layout);

    return script_camera_new(L, layout, camera);
}

static int script_layout_get_textsprite(lua_State* L) {
    READ_USERDATA(L, Layout, layout, LAYOUT);

    const char* name = luaL_optstring(L, 2, NULL);

    TextSprite textsprite = layout_get_textsprite(layout, name);

    return script_textsprite_new(L, layout, textsprite);
}

static int script_layout_get_soundplayer(lua_State* L) {
    READ_USERDATA(L, Layout, layout, LAYOUT);

    const char* name = luaL_optstring(L, 2, NULL);

    SoundPlayer soundplayer = layout_get_soundplayer(layout, name);

    return script_soundplayer_new(L, layout, soundplayer);
}

static int script_layout_get_sprite(lua_State* L) {
    READ_USERDATA(L, Layout, layout, LAYOUT);

    const char* name = luaL_optstring(L, 2, NULL);

    Sprite sprite = layout_get_sprite(layout, name);

    return script_sprite_new(L, layout, sprite);
}

static int script_layout_get_viewport_size(lua_State* L) {
    READ_USERDATA(L, Layout, layout, LAYOUT);

    float size[2];
    layout_get_viewport_size(layout, size);

    lua_pushnumber(L, size[0]);
    lua_pushnumber(L, size[1]);

    return 2;
}

static int script_layout_get_attached_value(lua_State* L) {
    READ_USERDATA(L, Layout, layout, LAYOUT);
    const char* name = luaL_optstring(L, 2, NULL);

    if (name == NULL) {
        lua_pushnil(L);
        return 1;
    }

    LayoutAttachedValue value;
    int value_type = layout_get_attached_value2(layout, name, &value);

    switch (value_type) {
    case LAYOUT_TYPE_STRING:
        if (value.value_string) {
            lua_pushstring(L, value.value_string);
#ifdef JAVASCRIPT
            // the javascript side returned a copy of the string, dispose it
            free(value.value_string);
#endif
        }
        else {
            lua_pushnil(L);
        }
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
        lua_pushvalue(L, 3);// default value (taken from the arguments)
        break;
    }

    return 1;
}

static int script_layout_set_group_visibility(lua_State* L) {
    READ_USERDATA(L, Layout, layout, LAYOUT);
    const char* group_name = luaL_optstring(L, 2, NULL);
    int visible = luaL_checkboolean(L, 3);

    layout_set_group_visibility(layout, group_name, visible);

    return 0;
}

static int script_layout_set_group_alpha(lua_State* L) {
    READ_USERDATA(L, Layout, layout, LAYOUT);
    const char* group_name = luaL_optstring(L, 2, NULL);
    float alpha = luaL_checkfloat(L, 3);

    layout_set_group_alpha(layout, group_name, alpha);

    return 0;
}

static int script_layout_set_group_offsetcolor(lua_State* L) {
    READ_USERDATA(L, Layout, layout, LAYOUT);
    const char* group_name = luaL_optstring(L, 2, NULL);
    float r = luaL_optionalfloat(L, 3);
    float g = luaL_optionalfloat(L, 4);
    float b = luaL_optionalfloat(L, 5);
    float a = luaL_optionalfloat(L, 6);

    layout_set_group_offsetcolor(layout, group_name, r, g, b, a);

    return 0;
}

static int script_layout_suspend(lua_State* L) {
    READ_USERDATA(L, Layout, layout, LAYOUT);
    layout_suspend(layout);
    return 0;
}

static int script_layout_resume(lua_State* L) {
    READ_USERDATA(L, Layout, layout, LAYOUT);
    layout_resume(layout);
    return 0;
}

static int script_layout_get_placeholder(lua_State* L) {
    READ_USERDATA(L, Layout, layout, LAYOUT);

    const char* name = luaL_optstring(L, 2, NULL);

#ifdef JAVASCRIPT
    LayoutPlaceholderJS_t placeholder;
    if (layout_get_placeholder_JS(layout, name, &placeholder)) {
        lua_createtable(L, 0, 12);

        LuaL_add_table_field(L, "group_id", lua_pushinteger, placeholder.group_id);

        LuaL_add_table_field(L, "align_vertical", lua_pushstring, _get_align(placeholder.align_vertical));
        LuaL_add_table_field(L, "align_horizontal", lua_pushstring, _get_align(placeholder.align_horizontal));

        LuaL_add_table_field(L, "x", lua_pushnumber, placeholder.x);
        LuaL_add_table_field(L, "y", lua_pushnumber, placeholder.y);
        LuaL_add_table_field(L, "z", lua_pushnumber, placeholder.z);

        LuaL_add_table_field(L, "height", lua_pushnumber, placeholder.height);
        LuaL_add_table_field(L, "width", lua_pushnumber, placeholder.width);

        LuaL_add_table_field(L, "parallax_x", lua_pushnumber, placeholder.parallax_x);
        LuaL_add_table_field(L, "parallax_y", lua_pushnumber, placeholder.parallax_y);
        LuaL_add_table_field(L, "parallax_z", lua_pushnumber, placeholder.parallax_z);

        LuaL_add_table_field(L, "static_camera", lua_pushboolean, placeholder.static_camera);
    }
    else {
        lua_pushnil(L);
    }
#else
    LayoutPlaceholder placeholder = layout_get_placeholder(layout, name);
    if (placeholder) {
        lua_createtable(L, 0, 12);

        LuaL_add_table_field(L, "group_id", lua_pushinteger, placeholder->group_id);

        LuaL_add_table_field(L, "align_vertical", lua_pushstring, _get_align(placeholder->align_vertical));
        LuaL_add_table_field(L, "align_horizontal", lua_pushstring, _get_align(placeholder->align_horizontal));

        LuaL_add_table_field(L, "x", lua_pushnumber, placeholder->x);
        LuaL_add_table_field(L, "y", lua_pushnumber, placeholder->y);
        LuaL_add_table_field(L, "z", lua_pushnumber, placeholder->z);

        LuaL_add_table_field(L, "height", lua_pushnumber, placeholder->height);
        LuaL_add_table_field(L, "width", lua_pushnumber, placeholder->width);

        LuaL_add_table_field(L, "parallax_x", lua_pushnumber, placeholder->parallax.x);
        LuaL_add_table_field(L, "parallax_y", lua_pushnumber, placeholder->parallax.y);
        LuaL_add_table_field(L, "parallax_z", lua_pushnumber, placeholder->parallax.z);

        LuaL_add_table_field(L, "static_camera", lua_pushboolean, placeholder->static_camera);
    }
    else {
        lua_pushnil(L);
    }
#endif

    return 1;
}

static int script_layout_disable_antialiasing(lua_State* L) {
    READ_USERDATA(L, Layout, layout, LAYOUT);

    bool disable = luaL_checkboolean(L, 3);

    layout_disable_antialiasing(layout, disable);

    return 0;
}

static int script_layout_set_group_antialiasing(lua_State* L) {
    READ_USERDATA(L, Layout, layout, LAYOUT);

    const char* group_name = luaL_optstring(L, 2, NULL);
    PVRFLAG antialiasing = _parse_pvrflag(L, luaL_optstring(L, 3, NULL));

    layout_set_group_antialiasing(layout, group_name, antialiasing);

    return 0;
}

static int script_layout_get_group_modifier(lua_State* L) {
    READ_USERDATA(L, Layout, layout, LAYOUT);

    const char* group_name = luaL_optstring(L, 2, NULL);

    Modifier modifier = layout_get_group_modifier(layout, group_name);

    return script_modifier_new(L, layout, modifier);
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
    {NULL, NULL}
};


int script_layout_new(lua_State* L, Layout layout) {
    return NEW_USERDATA(L, LAYOUT, NULL, layout, true);
}

static int script_layout_gc(lua_State* L) {
    READ_USERDATA_UNCHECKED(L, Layout, layout, LAYOUT);
    _luascript_suppress_item(L, layout, true);
    return 0;
}

static int script_layout_tostring(lua_State* L) {
    READ_USERDATA(L, Layout, layout, LAYOUT);
    lua_pushstring(L, "[Layout]");
    return 1;
}


inline void register_layout(lua_State* L) {
    _luascript_register(
        L,
        LAYOUT,
        script_layout_gc,
        script_layout_tostring,
        LAYOUT_FUNCTIONS
    );
}

