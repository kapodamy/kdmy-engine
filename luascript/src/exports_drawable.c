#include "luascript_internal.h"

#ifdef JAVASCRIPT
EM_JS_PRFX(void, drawable_set_z_index, (Drawable drawable, float z_index), {
    drawable_set_z_index(kdmyEngine_obtain(drawable), z_index);
});
EM_JS_PRFX(float, drawable_get_z_index, (Drawable drawable), {
    let ret = drawable_get_z_index(kdmyEngine_obtain(drawable));
    return ret;
});
EM_JS_PRFX(void, drawable_set_z_offset, (Drawable drawable, float offset), {
    drawable_set_z_offset(kdmyEngine_obtain(drawable), offset);
});
EM_JS_PRFX(void, drawable_set_alpha, (Drawable drawable, float alpha), {
    drawable_set_alpha(kdmyEngine_obtain(drawable), alpha);
});
EM_JS_PRFX(float, drawable_get_alpha, (Drawable drawable), {
    let ret = drawable_get_alpha(kdmyEngine_obtain(drawable));
    return ret;
});
EM_JS_PRFX(void, drawable_set_offsetcolor, (Drawable drawable, float r, float g, float b, float a), {
    drawable_set_offsetcolor(kdmyEngine_obtain(drawable), r, g, b, a);
});
EM_JS_PRFX(void, drawable_set_offsetcolor_to_default, (Drawable drawable), {
    drawable_set_offsetcolor_to_default(kdmyEngine_obtain(drawable));
});
EM_JS_PRFX(Modifier, drawable_get_modifier, (Drawable drawable), {
    let ret = drawable_get_modifier(kdmyEngine_obtain(drawable));
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(void, drawable_set_antialiasing, (Drawable drawable, PVRFlag antialiasing), {
    drawable_set_antialiasing(kdmyEngine_obtain(drawable), antialiasing);
});
EM_JS_PRFX(void, drawable_set_shader, (Drawable drawable, PSShader psshader), {
    drawable_set_shader(kdmyEngine_obtain(drawable), kdmyEngine_obtain(psshader));
});
EM_JS_PRFX(PSShader, drawable_get_shader, (Drawable drawable), {
    let ret = drawable_get_shader(kdmyEngine_obtain(drawable));
    return kdmyEngine_obtain(ret);
});
EM_JS_PRFX(void, drawable_blend_enable, (Drawable drawable, bool enabled), {
    drawable_blend_enable(kdmyEngine_obtain(drawable), enabled);
});
EM_JS_PRFX(void, drawable_blend_set, (Drawable drawable, Blend src_rgb, Blend dst_rgb, Blend src_alpha, Blend dst_alpha), {
    drawable_blend_set(kdmyEngine_obtain(drawable), src_rgb, dst_rgb, src_alpha, dst_alpha);
});
#endif


static int script_drawable_set_z_index(lua_State* L) {
    Drawable drawable = luascript_read_userdata(L, DRAWABLE);
    float z_index = (float)luaL_checknumber(L, 2);

    drawable_set_z_index(drawable, z_index);

    return 0;
}

static int script_drawable_get_z_index(lua_State* L) {
    Drawable drawable = luascript_read_userdata(L, DRAWABLE);

    float ret = drawable_get_z_index(drawable);

    lua_pushnumber(L, ret);
    return 1;
}

static int script_drawable_set_z_offset(lua_State* L) {
    Drawable drawable = luascript_read_userdata(L, DRAWABLE);
    float offset = (float)luaL_checknumber(L, 2);

    drawable_set_z_offset(drawable, offset);

    return 0;
}

static int script_drawable_set_alpha(lua_State* L) {
    Drawable drawable = luascript_read_userdata(L, DRAWABLE);
    float alpha = (float)luaL_checknumber(L, 2);

    drawable_set_alpha(drawable, alpha);

    return 0;
}

static int script_drawable_get_alpha(lua_State* L) {
    Drawable drawable = luascript_read_userdata(L, DRAWABLE);

    float ret = drawable_get_alpha(drawable);

    lua_pushnumber(L, ret);
    return 1;
}

static int script_drawable_set_offsetcolor(lua_State* L) {
    Drawable drawable = luascript_read_userdata(L, DRAWABLE);
    float r = (float)luaL_checknumber(L, 2);
    float g = (float)luaL_checknumber(L, 3);
    float b = (float)luaL_checknumber(L, 4);
    float a = (float)luaL_checknumber(L, 5);

    drawable_set_offsetcolor(drawable, r, g, b, a);

    return 0;
}

static int script_drawable_set_offsetcolor_to_default(lua_State* L) {
    Drawable drawable = luascript_read_userdata(L, DRAWABLE);

    drawable_set_offsetcolor_to_default(drawable);

    return 0;
}

static int script_drawable_get_modifier(lua_State* L) {
    Drawable drawable = luascript_read_userdata(L, DRAWABLE);

    Modifier ret = drawable_get_modifier(drawable);

    return script_modifier_new(L, ret);
}

static int script_drawable_set_antialiasing(lua_State* L) {
    Drawable drawable = luascript_read_userdata(L, DRAWABLE);
    PVRFlag antialiasing = luascript_parse_pvrflag(L, luaL_optstring(L, 2, NULL));

    drawable_set_antialiasing(drawable, antialiasing);

    return 0;
}

static int script_drawable_set_shader(lua_State* L) {
    Drawable drawable = luascript_read_userdata(L, DRAWABLE);
    PSShader psshader = luascript_read_nullable_userdata(L, 2, PSSHADER);

    drawable_set_shader(drawable, psshader);

    return 0;
}

static int script_drawable_get_shader(lua_State* L) {
    Drawable drawable = luascript_read_userdata(L, DRAWABLE);

    PSShader ret = drawable_get_shader(drawable);

    return script_psshader_new(L, ret);
}

static int script_drawable_blend_enable(lua_State* L) {
    Drawable drawable = luascript_read_userdata(L, DRAWABLE);
    bool enabled = (bool)lua_toboolean(L, 2);

    drawable_blend_enable(drawable, enabled);

    return 0;
}

static int script_drawable_blend_set(lua_State* L) {
    Drawable drawable = luascript_read_userdata(L, DRAWABLE);
    Blend src_rgb = luascript_parse_blend(L, luaL_optstring(L, 2, NULL));
    Blend dst_rgb = luascript_parse_blend(L, luaL_optstring(L, 3, NULL));
    Blend src_alpha = luascript_parse_blend(L, luaL_optstring(L, 4, NULL));
    Blend dst_alpha = luascript_parse_blend(L, luaL_optstring(L, 5, NULL));

    drawable_blend_set(drawable, src_rgb, dst_rgb, src_alpha, dst_alpha);

    return 0;
}




static const luaL_Reg DRAWABLE_FUNCTIONS[] = {
    { "set_z_index", script_drawable_set_z_index },
    { "get_z_index", script_drawable_get_z_index },
    { "set_z_offset", script_drawable_set_z_offset },
    { "set_alpha", script_drawable_set_alpha },
    { "get_alpha", script_drawable_get_alpha },
    { "set_offsetcolor", script_drawable_set_offsetcolor },
    { "set_offsetcolor_to_default", script_drawable_set_offsetcolor_to_default },
    { "get_modifier", script_drawable_get_modifier },
    { "set_antialiasing", script_drawable_set_antialiasing },
    { "set_shader", script_drawable_set_shader },
    { "get_shader", script_drawable_get_shader },
    { "blend_enable", script_drawable_blend_enable },
    { "blend_set", script_drawable_blend_set },
    { NULL, NULL }
};

int script_drawable_new(lua_State* L, Drawable drawable) {
    return luascript_userdata_new(L, DRAWABLE, drawable);
}

static int script_drawable_gc(lua_State* L) {
    return luascript_userdata_gc(L, DRAWABLE);
}

static int script_drawable_tostring(lua_State* L) {
    return luascript_userdata_tostring(L, DRAWABLE);
}

void script_drawable_register(lua_State* L) {
    luascript_register(L, DRAWABLE, script_drawable_gc, script_drawable_tostring, DRAWABLE_FUNCTIONS);
}

