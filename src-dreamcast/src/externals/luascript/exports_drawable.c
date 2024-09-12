#include "luascript_internal.h"

#include "drawable.h"
#include "luascript_enums.h"


static int script_drawable_set_z_index(lua_State* L) {
    Drawable drawable = luascript_read_userdata(L, DRAWABLE);
    float z_index = (float)luaL_checknumber(L, 2);

    drawable_set_z_index(drawable, z_index);

    return 0;
}

static int script_drawable_get_z_index(lua_State* L) {
    Drawable drawable = luascript_read_userdata(L, DRAWABLE);

    float ret = drawable_get_z_index(drawable);

    lua_pushnumber(L, (lua_Number)ret);
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

    lua_pushnumber(L, (lua_Number)ret);
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

    Modifier* ret = drawable_get_modifier(drawable);

    return script_modifier_new(L, ret);
}

static int script_drawable_set_antialiasing(lua_State* L) {
    Drawable drawable = luascript_read_userdata(L, DRAWABLE);
    PVRFlag antialiasing = luascript_helper_optenum(L, 2, LUASCRIPT_ENUMS_PVRFlag);

    drawable_set_antialiasing(drawable, antialiasing);

    return 0;
}

#ifndef _arch_dreamcast
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
#endif

static int script_drawable_blend_enable(lua_State* L) {
    Drawable drawable = luascript_read_userdata(L, DRAWABLE);
    bool enabled = (bool)lua_toboolean(L, 2);

    drawable_blend_enable(drawable, enabled);

    return 0;
}

static int script_drawable_blend_set(lua_State* L) {
    Drawable drawable = luascript_read_userdata(L, DRAWABLE);
    Blend src_rgb = luascript_helper_optenum(L, 2, LUASCRIPT_ENUMS_Blend);
    Blend dst_rgb = luascript_helper_optenum(L, 3, LUASCRIPT_ENUMS_Blend);
    Blend src_alpha = luascript_helper_optenum(L, 4, LUASCRIPT_ENUMS_Blend);
    Blend dst_alpha = luascript_helper_optenum(L, 5, LUASCRIPT_ENUMS_Blend);

    drawable_blend_set(drawable, src_rgb, dst_rgb, src_alpha, dst_alpha);

    return 0;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const luaL_Reg DRAWABLE_FUNCTIONS[] = {
    {"set_z_index", script_drawable_set_z_index},
    {"get_z_index", script_drawable_get_z_index},
    {"set_z_offset", script_drawable_set_z_offset},
    {"set_alpha", script_drawable_set_alpha},
    {"get_alpha", script_drawable_get_alpha},
    {"set_offsetcolor", script_drawable_set_offsetcolor},
    {"set_offsetcolor_to_default", script_drawable_set_offsetcolor_to_default},
    {"get_modifier", script_drawable_get_modifier},
    {"set_antialiasing", script_drawable_set_antialiasing},
#ifndef _arch_dreamcast
    {"set_shader", script_drawable_set_shader},
    {"get_shader", script_drawable_get_shader},
#endif
    {"blend_enable", script_drawable_blend_enable},
    {"blend_set", script_drawable_blend_set},
    {NULL, NULL}
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
