#include "engine_string.h"
#include "luascript_internal.h"



#ifdef JAVASCRIPT
EM_JS_PRFX(void, camera_set_interpolator_type, (Camera camera, AnimInterpolator type), {
    camera_set_interpolator_type(kdmyEngine_obtain(camera), type);
});
EM_JS_PRFX(void, camera_set_transition_duration, (Camera camera, bool expresed_in_beats, float value), {
    camera_set_transition_duration(kdmyEngine_obtain(camera), expresed_in_beats, value);
});
EM_JS_PRFX(void, camera_set_absolute_zoom, (Camera camera, float z), {
    camera_set_absolute_zoom(kdmyEngine_obtain(camera), z);
});
EM_JS_PRFX(void, camera_set_absolute_position, (Camera camera, float x, float y), {
    camera_set_absolute_position(kdmyEngine_obtain(camera), x, y);
});
EM_JS_PRFX(void, camera_set_offset, (Camera camera, float x, float y, float z), {
    camera_set_offset(kdmyEngine_obtain(camera), x, y, z);
});
EM_JS_PRFX(float*, camera_get_offset, (Camera camera, float* xyz), {
    const HEAP_ENDIANESS = true;
    const dataView = new DataView(buffer);
    const values = [ 0, 0, 0 ];

    camera_get_offset(kdmyEngine_obtain(camera), values);
    dataView.setFloat32(xyz + 0, values[0], HEAP_ENDIANESS);
    dataView.setFloat32(xyz + 4, values[1], HEAP_ENDIANESS);
    dataView.setFloat32(xyz + 8, values[2], HEAP_ENDIANESS);
    return xyz;
});
EM_JS_PRFX(Modifier, camera_get_modifier, (Camera camera), {
    const modifier = camera_get_modifier(kdmyEngine_obtain(camera));
    return kdmyEngine_obtain(modifier);
});
EM_JS_PRFX(void, camera_move, (Camera camera, float end_x, float end_y, float end_z), {
    camera_move(kdmyEngine_obtain(camera), end_x, end_y, end_z);
});
EM_JS_PRFX(void, camera_slide, (Camera camera, float start_x, float start_y, float start_z, float end_x, float end_y, float end_z), {
    camera_slide(kdmyEngine_obtain(camera), start_x, start_y, start_z, end_x, end_y, end_z);
});
EM_JS_PRFX(void, camera_slide_x, (Camera camera, float start, float end), {
    camera_slide_x(kdmyEngine_obtain(camera), start, end);
});
EM_JS_PRFX(void, camera_slide_y, (Camera camera, float start, float end), {
    camera_slide_y(kdmyEngine_obtain(camera), start, end);
});
EM_JS_PRFX(void, camera_slide_z, (Camera camera, float start, float end), {
    camera_slide_z(kdmyEngine_obtain(camera), start, end);
});
EM_JS_PRFX(void, camera_slide_to, (Camera camera, float x, float y, float z), {
    camera_slide_to(kdmyEngine_obtain(camera), x, y, z);
});
EM_JS_PRFX(bool, camera_from_layout, (Camera camera, Layout layout, const char* camera_name), {
    return camera_from_layout(kdmyEngine_obtain(camera), kdmyEngine_obtain(layout), kdmyEngine_ptrToString(camera_name));
});
EM_JS_PRFX(void, camera_to_origin, (Camera camera, bool should_slide), {
    camera_to_origin(kdmyEngine_obtain(camera), should_slide);
});
EM_JS_PRFX(void, camera_repeat, (Camera camera), {
    camera_repeat(kdmyEngine_obtain(camera));
});
EM_JS_PRFX(void, camera_stop, (Camera camera), {
    camera_stop(kdmyEngine_obtain(camera));
});
EM_JS_PRFX(void, camera_end, (Camera camera), {
    camera_end(kdmyEngine_obtain(camera));
});
EM_JS_PRFX(bool, camera_is_completed, (Camera camera), {
    return camera_is_completed(kdmyEngine_obtain(camera));
});
EM_JS_PRFX(void, camera_debug_log_info, (Camera camera), {
    camera_debug_log_info(kdmyEngine_obtain(camera));
});
EM_JS_PRFX(void, camera_apply, (Camera camera, PVRContext pvrctx), {
    camera_apply(kdmyEngine_obtain(camera), null);
});
EM_JS_PRFX(void, camera_move_offset, (Camera camera, float end_x, float end_y, float end_z), {
    camera_move_offset(kdmyEngine_obtain(camera), end_x, end_y, end_z);
});
EM_JS_PRFX(void, camera_slide_offset, (Camera camera, float start_x, float start_y, float start_z, float end_x, float end_y, float end_z), {
    camera_slide_offset(kdmyEngine_obtain(camera), start_x, start_y, start_z, end_x, end_y, end_z);
});
EM_JS_PRFX(void, camera_slide_x_offset, (Camera camera, float start, float end), {
    camera_slide_x_offset(kdmyEngine_obtain(camera), start, end);
});
EM_JS_PRFX(void, camera_slide_y_offset, (Camera camera, float start, float end), {
    camera_slide_y_offset(kdmyEngine_obtain(camera), start, end);
});
EM_JS_PRFX(void, camera_slide_z_offset, (Camera camera, float start, float end), {
    camera_slide_z_offset(kdmyEngine_obtain(camera), start, end);
});
EM_JS_PRFX(void, camera_slide_to_offset, (Camera camera, float x, float y, float z), {
    camera_slide_to_offset(kdmyEngine_obtain(camera), x, y, z);
});
EM_JS_PRFX(void, camera_to_origin_offset, (Camera camera, bool should_slide), {
    camera_to_origin_offset(kdmyEngine_obtain(camera), should_slide);
});
EM_JS_PRFX(Layout, camera_get_parent_layout, (Camera camera), {
    let ret = camera_get_parent_layout(kdmyEngine_obtain(camera));
    return kdmyEngine_obtain(ret);
});
#endif



static int script_camera_set_interpolator_type(lua_State* L) {
    Camera camera = luascript_read_userdata(L, CAMERA);

    AnimInterpolator type = luascript_parse_interpolator(L, luaL_optstring(L, 2, NULL));

    camera_set_interpolator_type(camera, type);

    return 0;
}

static int script_camera_set_transition_duration(lua_State* L) {
    Camera camera = luascript_read_userdata(L, CAMERA);

    bool expresed_in_beats = lua_toboolean(L, 2);
    float value = (float)luaL_checknumber(L, 3);

    camera_set_transition_duration(camera, expresed_in_beats, value);

    return 0;
}

static int script_camera_set_absolute_zoom(lua_State* L) {
    Camera camera = luascript_read_userdata(L, CAMERA);

    float z = (float)luaL_checknumber(L, 2);

    camera_set_absolute_zoom(camera, z);

    return 0;
}

static int script_camera_set_absolute_position(lua_State* L) {
    Camera camera = luascript_read_userdata(L, CAMERA);

    float x = (float)luaL_optnumber(L, 2, NAN);
    float y = (float)luaL_optnumber(L, 3, NAN);

    camera_set_absolute_position(camera, x, y);

    return 0;
}

static int script_camera_set_offset(lua_State* L) {
    Camera camera = luascript_read_userdata(L, CAMERA);

    float x = (float)luaL_optnumber(L, 2, NAN);
    float y = (float)luaL_optnumber(L, 3, NAN);
    float z = (float)luaL_optnumber(L, 4, NAN);

    camera_set_offset(camera, x, y, z);

    return 0;
}

static int script_camera_get_offset(lua_State* L) {
    Camera camera = luascript_read_userdata(L, CAMERA);

    float xyz[3];

    camera_get_offset(camera, xyz);

    lua_pushnumber(L, (double)xyz[0]);
    lua_pushnumber(L, (double)xyz[1]);
    lua_pushnumber(L, (double)xyz[2]);

    return 3;
}

static int script_camera_get_modifier(lua_State* L) {
    Camera camera = luascript_read_userdata(L, CAMERA);

    Modifier modifier = camera_get_modifier(camera);
    return script_modifier_new(L, modifier);
}

static int script_camera_move(lua_State* L) {
    Camera camera = luascript_read_userdata(L, CAMERA);

    float x = (float)luaL_optnumber(L, 2, NAN);
    float y = (float)luaL_optnumber(L, 3, NAN);
    float z = (float)luaL_optnumber(L, 4, NAN);

    camera_move(camera, x, y, z);

    return 0;
}

static int script_camera_slide(lua_State* L) {
    Camera camera = luascript_read_userdata(L, CAMERA);

    float start_x = (float)luaL_optnumber(L, 2, NAN);
    float start_y = (float)luaL_optnumber(L, 3, NAN);
    float start_z = (float)luaL_optnumber(L, 4, NAN);
    float end_x = (float)luaL_optnumber(L, 5, NAN);
    float end_y = (float)luaL_optnumber(L, 6, NAN);
    float end_z = (float)luaL_optnumber(L, 7, NAN);

    camera_slide(camera, start_x, start_y, start_z, end_x, end_y, end_z);

    return 0;
}

static int script_camera_slide_x(lua_State* L) {
    Camera camera = luascript_read_userdata(L, CAMERA);

    float start = (float)luaL_optnumber(L, 2, NAN);
    float end = (float)luaL_optnumber(L, 3, NAN);

    camera_slide_x(camera, start, end);

    return 0;
}

static int script_camera_slide_y(lua_State* L) {
    Camera camera = luascript_read_userdata(L, CAMERA);

    float start = (float)luaL_optnumber(L, 2, NAN);
    float end = (float)luaL_optnumber(L, 3, NAN);

    camera_slide_y(camera, start, end);

    return 0;
}

static int script_camera_slide_z(lua_State* L) {
    Camera camera = luascript_read_userdata(L, CAMERA);

    float start = (float)luaL_optnumber(L, 2, NAN);
    float end = (float)luaL_optnumber(L, 3, NAN);

    camera_slide_z(camera, start, end);

    return 0;
}

static int script_camera_slide_to(lua_State* L) {
    Camera camera = luascript_read_userdata(L, CAMERA);

    float x = (float)luaL_optnumber(L, 2, NAN);
    float y = (float)luaL_optnumber(L, 3, NAN);
    float z = (float)luaL_optnumber(L, 4, NAN);

    camera_slide_to(camera, x, y, z);

    return 0;
}

static int script_camera_from_layout(lua_State* L) {
    Camera camera = luascript_read_userdata(L, CAMERA);

    const char* camera_name = luaL_optstring(L, 2, NULL);
    Layout layout = camera_get_parent_layout(camera);

    if (!layout) {
        lua_pushboolean(L, false);
        return 1;
    }

    bool ret = camera_from_layout(camera, layout, camera_name);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_camera_to_origin(lua_State* L) {
    Camera camera = luascript_read_userdata(L, CAMERA);

    bool should_slide = lua_toboolean(L, 2);

    camera_to_origin(camera, should_slide);

    return 0;
}

static int script_camera_repeat(lua_State* L) {
    Camera camera = luascript_read_userdata(L, CAMERA);

    camera_repeat(camera);

    return 0;
}

static int script_camera_stop(lua_State* L) {
    Camera camera = luascript_read_userdata(L, CAMERA);

    camera_stop(camera);

    return 0;
}

static int script_camera_end(lua_State* L) {
    Camera camera = luascript_read_userdata(L, CAMERA);

    camera_end(camera);

    return 0;
}

static int script_camera_is_completed(lua_State* L) {
    Camera camera = luascript_read_userdata(L, CAMERA);

    bool ret = camera_is_completed(camera);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_camera_debug_log_info(lua_State* L) {
    Camera camera = luascript_read_userdata(L, CAMERA);

    camera_debug_log_info(camera);

    return 0;
}

static int script_camera_apply(lua_State* L) {
    Camera camera = luascript_read_userdata(L, CAMERA);

    camera_apply(camera, NULL);

    return 0;
}

static int script_camera_move_offset(lua_State* L) {
    Camera camera = luascript_read_userdata(L, CAMERA);

    float end_x = (float)luaL_optnumber(L, 2, NAN);
    float end_y = (float)luaL_optnumber(L, 3, NAN);
    float end_z = (float)luaL_optnumber(L, 4, NAN);

    camera_move_offset(camera, end_x, end_y, end_z);

    return 0;
}

static int script_camera_slide_offset(lua_State* L) {
    Camera camera = luascript_read_userdata(L, CAMERA);

    float start_x = (float)luaL_optnumber(L, 2, NAN);
    float start_y = (float)luaL_optnumber(L, 3, NAN);
    float start_z = (float)luaL_optnumber(L, 4, NAN);
    float end_x = (float)luaL_optnumber(L, 5, NAN);
    float end_y = (float)luaL_optnumber(L, 6, NAN);
    float end_z = (float)luaL_optnumber(L, 7, NAN);

    camera_slide_offset(camera, start_x, start_y, start_z, end_x, end_y, end_z);

    return 0;
}

static int script_camera_slide_x_offset(lua_State* L) {
    Camera camera = luascript_read_userdata(L, CAMERA);

    float start = (float)luaL_optnumber(L, 2, NAN);
    float end = (float)luaL_optnumber(L, 3, NAN);

    camera_slide_x_offset(camera, start, end);

    return 0;
}

static int script_camera_slide_y_offset(lua_State* L) {
    Camera camera = luascript_read_userdata(L, CAMERA);

    float start = (float)luaL_optnumber(L, 2, NAN);
    float end = (float)luaL_optnumber(L, 3, NAN);

    camera_slide_y_offset(camera, start, end);

    return 0;
}

static int script_camera_slide_z_offset(lua_State* L) {
    Camera camera = luascript_read_userdata(L, CAMERA);

    float start = (float)luaL_optnumber(L, 2, NAN);
    float end = (float)luaL_optnumber(L, 3, NAN);

    camera_slide_z_offset(camera, start, end);

    return 0;
}

static int script_camera_slide_to_offset(lua_State* L) {
    Camera camera = luascript_read_userdata(L, CAMERA);

    float x = (float)luaL_optnumber(L, 2, NAN);
    float y = (float)luaL_optnumber(L, 3, NAN);
    float z = (float)luaL_optnumber(L, 4, NAN);

    camera_slide_to_offset(camera, x, y, z);

    return 0;
}

static int script_camera_to_origin_offset(lua_State* L) {
    Camera camera = luascript_read_userdata(L, CAMERA);

    bool should_slide = lua_toboolean(L, 2);

    camera_to_origin_offset(camera, should_slide);

    return 0;
}



////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const luaL_Reg CAMERA_FUNCTIONS[] = {
    {"set_interpolator_type", script_camera_set_interpolator_type},
    {"set_transition_duration", script_camera_set_transition_duration},
    {"set_absolute_zoom", script_camera_set_absolute_zoom},
    {"set_absolute_position", script_camera_set_absolute_position},
    {"set_offset", script_camera_set_offset},
    {"get_offset", script_camera_get_offset},
    {"get_modifier", script_camera_get_modifier},
    {"move", script_camera_move},
    {"slide", script_camera_slide},
    {"slide_x", script_camera_slide_x},
    {"slide_y", script_camera_slide_y},
    {"slide_z", script_camera_slide_z},
    {"slide_to", script_camera_slide_to},
    {"from_layout", script_camera_from_layout},
    {"to_origin", script_camera_to_origin},
    {"repeat", script_camera_repeat},
    {"stop", script_camera_stop},
    {"end", script_camera_end},
    {"is_completed", script_camera_is_completed},
    {"debug_log_info", script_camera_debug_log_info},
    {"apply", script_camera_apply},
    {"move_offset", script_camera_move_offset},
    {"slide_offset", script_camera_slide_offset},
    {"slide_x_offset", script_camera_slide_x_offset},
    {"slide_y_offset", script_camera_slide_y_offset},
    {"slide_z_offset", script_camera_slide_z_offset},
    {"slide_to_offset", script_camera_slide_to_offset},
    {"to_origin_offset", script_camera_to_origin_offset},
    {NULL, NULL}
};

int script_camera_new(lua_State* L, Camera camera) {
    return luascript_userdata_new(L, CAMERA, camera);
}

static int script_camera_gc(lua_State* L) {
    return luascript_userdata_gc(L, CAMERA);
}

static int script_camera_tostring(lua_State* L) {
    return luascript_userdata_tostring(L, CAMERA);
}


void script_camera_register(lua_State* L) {
    luascript_register(L, CAMERA, script_camera_gc, script_camera_tostring, CAMERA_FUNCTIONS);
}
