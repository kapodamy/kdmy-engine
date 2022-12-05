#include "luascript_internal.h"



#ifdef JAVASCRIPT
EM_JS_PRFX(Modifier, sprite_matrix_get_modifier, (Sprite sprite), {
    const modifier = sprite_matrix_get_modifier(kdmyEngine_obtain(sprite));
    return kdmyEngine_obtain(modifier);
});
EM_JS_PRFX(void, sprite_set_offset_source, (Sprite sprite, float x, float y, float width, float height), {
    sprite_set_offset_source(kdmyEngine_obtain(sprite), x, y, width, height);
});
EM_JS_PRFX(void, sprite_set_offset_frame, (Sprite sprite, float x, float y, float width, float height), {
    sprite_set_offset_frame(kdmyEngine_obtain(sprite), x, y, width, height);
});
EM_JS_PRFX(void, sprite_set_offset_pivot, (Sprite sprite, float x, float y), {
    sprite_set_offset_pivot(kdmyEngine_obtain(sprite), x, y);
});
EM_JS_PRFX(void, sprite_matrix_reset, (Sprite sprite), {
    sprite_matrix_reset(kdmyEngine_obtain(sprite));
});
EM_JS_PRFX(void, sprite_set_draw_location, (Sprite sprite, float x, float y), {
    sprite_set_draw_location(kdmyEngine_obtain(sprite), x, y);
});
EM_JS_PRFX(void, sprite_set_draw_size, (Sprite sprite, float width, float height), {
    sprite_set_draw_size(kdmyEngine_obtain(sprite), width, height);
});
EM_JS_PRFX(void, sprite_set_draw_size_from_source_size, (Sprite sprite), {
    sprite_set_draw_size_from_source_size(kdmyEngine_obtain(sprite));
});
EM_JS_PRFX(void, sprite_set_alpha, (Sprite sprite, float alpha), {
    sprite_set_alpha(kdmyEngine_obtain(sprite), alpha);
});
EM_JS_PRFX(void, sprite_set_visible, (Sprite sprite, bool visible), {
    sprite_set_visible(kdmyEngine_obtain(sprite), visible);
});
EM_JS_PRFX(void, sprite_set_z_index, (Sprite sprite, float index), {
    sprite_set_z_index(kdmyEngine_obtain(sprite), index);
});
EM_JS_PRFX(void, sprite_set_z_offset, (Sprite sprite, float offset), {
    sprite_set_z_offset(kdmyEngine_obtain(sprite), offset);
});
EM_JS_PRFX(float*, sprite_get_source_size, (Sprite sprite, float* size), {
    const HEAP_ENDIANESS = true;
    const dataView = new DataView(buffer);
    const values = [ 0, 0 ];

    sprite_get_source_size(kdmyEngine_obtain(sprite), values);
    dataView.setFloat32(size + 0, values[0], HEAP_ENDIANESS);
    dataView.setFloat32(size + 4, values[1], HEAP_ENDIANESS);

    return size;
});
EM_JS_PRFX(void, sprite_set_vertex_color, (Sprite sprite, float r, float g, float b), {
    sprite_set_vertex_color(kdmyEngine_obtain(sprite), r, g, b);
});
EM_JS_PRFX(void, sprite_set_offsetcolor, (Sprite sprite, float r, float g, float b, float a), {
    sprite_set_offsetcolor(kdmyEngine_obtain(sprite), r, g, b, a);
});
EM_JS_PRFX(bool, sprite_is_textured, (Sprite sprite), {
    return sprite_is_textured(kdmyEngine_obtain(sprite));
});
EM_JS_PRFX(bool, sprite_crop, (Sprite sprite, float dx, float dy, float dwidth, float dheight), {
    return sprite_crop(kdmyEngine_obtain(sprite), dx, dy, dwidth, dheight);
});
EM_JS_PRFX(bool, sprite_is_crop_enabled, (Sprite sprite), {
    return sprite_is_crop_enabled(kdmyEngine_obtain(sprite));
});
EM_JS_PRFX(void, sprite_crop_enable, (Sprite sprite, bool enable), {
    sprite_crop_enable(kdmyEngine_obtain(sprite), enable);
});
EM_JS_PRFX(float*, sprite_resize_draw_size, (Sprite sprite, float max_width, float max_height, float* applied_draw_size), {
    const HEAP_ENDIANESS = true;
    const dataView = new DataView(buffer);
    const values = [ 0, 0 ];

    sprite_resize_draw_size(kdmyEngine_obtain(sprite), max_width, max_height, values);
    dataView.setFloat32(applied_draw_size + 0, values[0], HEAP_ENDIANESS);
    dataView.setFloat32(applied_draw_size + 4, values[1], HEAP_ENDIANESS);

    return applied_draw_size;
});
EM_JS_PRFX(float*, sprite_center_draw_location, (Sprite sprite, float x, float y, float ref_width, float ref_height, float* applied_draw_location), {
    const HEAP_ENDIANESS = true;
    const dataView = new DataView(buffer);
    const values = [ 0, 0 ];

    sprite_center_draw_location(kdmyEngine_obtain(sprite), x, y, ref_width, ref_height, values);
    dataView.setFloat32(applied_draw_location + 0, values[0], HEAP_ENDIANESS);
    dataView.setFloat32(applied_draw_location + 4, values[1], HEAP_ENDIANESS);

    return applied_draw_location;
});
EM_JS_PRFX(void, sprite_set_antialiasing, (Sprite sprite, PVRFLAG antialiasing), {
    sprite_set_antialiasing(kdmyEngine_obtain(sprite), antialiasing);
});

EM_JS_PRFX(void, sprite_flip_rendered_texture, (Sprite sprite, int flip_x, int flip_y), {
    sprite_flip_rendered_texture(kdmyEngine_obtain(sprite), flip_x < 0 ? null : flip_x, flip_y < 0 ? null : flip_y);
});
EM_JS_PRFX(void, sprite_flip_rendered_texture_enable_correction, (Sprite sprite, bool enabled), {
    sprite_flip_rendered_texture_enable_correction(kdmyEngine_obtain(sprite), enabled);
});
EM_JS_PRFX(void, sprite_set_shader, (Sprite sprite, PSShader psshader), {
    sprite_set_shader(kdmyEngine_obtain(sprite), kdmyEngine_obtain(psshader));
});
EM_JS_PRFX(PSShader, sprite_get_shader, (Sprite sprite), {
    let psshader = sprite_get_shader(kdmyEngine_obtain(sprite));
    return kdmyEngine_obtain(psshader);
});
#endif



static int script_sprite_matrix_get_modifier(lua_State* L) {
    Sprite sprite = luascript_read_userdata(L, SPRITE);

    Modifier modifier = sprite_matrix_get_modifier(sprite);
    return script_modifier_new(L, modifier);
}

static int script_sprite_set_offset_source(lua_State* L) {
    Sprite sprite = luascript_read_userdata(L, SPRITE);

    float x = (float)luaL_checknumber(L, 2);
    float y = (float)luaL_checknumber(L, 3);
    float width = (float)luaL_checknumber(L, 4);
    float height = (float)luaL_checknumber(L, 5);

    sprite_set_offset_source(sprite, x, y, width, height);

    return 0;
}

static int script_sprite_set_offset_frame(lua_State* L) {
    Sprite sprite = luascript_read_userdata(L, SPRITE);

    float x = (float)luaL_checknumber(L, 2);
    float y = (float)luaL_checknumber(L, 3);
    float width = (float)luaL_checknumber(L, 4);
    float height = (float)luaL_checknumber(L, 5);

    sprite_set_offset_frame(sprite, x, y, width, height);

    return 0;
}

static int script_sprite_set_offset_pivot(lua_State* L) {
    Sprite sprite = luascript_read_userdata(L, SPRITE);

    float x = (float)luaL_checknumber(L, 2);
    float y = (float)luaL_checknumber(L, 3);

    sprite_set_offset_pivot(sprite, x, y);
    return 0;
}

static int script_sprite_matrix_reset(lua_State* L) {
    Sprite sprite = luascript_read_userdata(L, SPRITE);

    sprite_matrix_reset(sprite);

    return 0;
}

static int script_sprite_set_draw_location(lua_State* L) {
    Sprite sprite = luascript_read_userdata(L, SPRITE);

    float x = (float)luaL_checknumber(L, 2);
    float y = (float)luaL_checknumber(L, 3);

    sprite_set_draw_location(sprite, x, y);

    return 0;
}

static int script_sprite_set_draw_size(lua_State* L) {
    Sprite sprite = luascript_read_userdata(L, SPRITE);

    float width = (float)luaL_checknumber(L, 2);
    float height = (float)luaL_checknumber(L, 3);

    sprite_set_draw_size(sprite, width, height);

    return 0;
}

static int script_sprite_set_draw_size_from_source_size(lua_State* L) {
    Sprite sprite = luascript_read_userdata(L, SPRITE);

    sprite_set_draw_size_from_source_size(sprite);

    return 0;
}

static int script_sprite_set_alpha(lua_State* L) {
    Sprite sprite = luascript_read_userdata(L, SPRITE);

    float alpha = (float)luaL_checknumber(L, 2);

    sprite_set_alpha(sprite, alpha);

    return 0;
}

static int script_sprite_set_visible(lua_State* L) {
    Sprite sprite = luascript_read_userdata(L, SPRITE);

    bool visible = lua_toboolean(L, 2);

    sprite_set_visible(sprite, visible);

    return 0;
}

static int script_sprite_set_z_index(lua_State* L) {
    Sprite sprite = luascript_read_userdata(L, SPRITE);

    float index = (float)luaL_checknumber(L, 2);

    sprite_set_z_index(sprite, index);

    return 0;
}

static int script_sprite_set_z_offset(lua_State* L) {
    Sprite sprite = luascript_read_userdata(L, SPRITE);

    float index = (float)luaL_checknumber(L, 2);

    sprite_set_z_offset(sprite, index);

    return 0;
}

static int script_sprite_get_source_size(lua_State* L) {
    Sprite sprite = luascript_read_userdata(L, SPRITE);

    float size[2];
    sprite_get_source_size(sprite, size);

    lua_pushnumber(L, size[0]);
    lua_pushnumber(L, size[1]);

    return 2;
}

static int script_sprite_set_vertex_color(lua_State* L) {
    Sprite sprite = luascript_read_userdata(L, SPRITE);

    float r = (float)luaL_optnumber(L, 2, NAN);
    float g = (float)luaL_optnumber(L, 3, NAN);
    float b = (float)luaL_optnumber(L, 4, NAN);

    sprite_set_vertex_color(sprite, r, g, b);

    return 0;
}

static int script_sprite_set_offsetcolor(lua_State* L) {
    Sprite sprite = luascript_read_userdata(L, SPRITE);

    float r = (float)luaL_optnumber(L, 2, NAN);
    float g = (float)luaL_optnumber(L, 3, NAN);
    float b = (float)luaL_optnumber(L, 4, NAN);
    float a = (float)luaL_optnumber(L, 5, NAN);

    sprite_set_offsetcolor(sprite, r, g, b, a);

    return 0;
}

static int script_sprite_is_textured(lua_State* L) {
    Sprite sprite = luascript_read_userdata(L, SPRITE);

    bool ret = sprite_is_textured(sprite);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_sprite_crop(lua_State* L) {
    Sprite sprite = luascript_read_userdata(L, SPRITE);

    float dx = (float)luaL_optnumber(L, 2, NAN);
    float dy = (float)luaL_optnumber(L, 3, NAN);
    float dwidth = (float)luaL_optnumber(L, 4, NAN);
    float dheight = (float)luaL_optnumber(L, 5, NAN);

    bool ret = sprite_crop(sprite, dx, dy, dwidth, dheight);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_sprite_is_crop_enabled(lua_State* L) {
    Sprite sprite = luascript_read_userdata(L, SPRITE);

    bool ret = sprite_is_crop_enabled(sprite);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_sprite_crop_enable(lua_State* L) {
    Sprite sprite = luascript_read_userdata(L, SPRITE);

    bool enable = lua_toboolean(L, 2);

    sprite_crop_enable(sprite, enable);

    return 0;
}

static int script_sprite_resize_draw_size(lua_State* L) {
    Sprite sprite = luascript_read_userdata(L, SPRITE);

    float max_width = (float)luaL_checknumber(L, 2);
    float max_height = (float)luaL_checknumber(L, 3);

    float applied_draw_size[2];
    sprite_resize_draw_size(sprite, max_width, max_height, applied_draw_size);

    lua_pushnumber(L, applied_draw_size[0]);
    lua_pushnumber(L, applied_draw_size[1]);
    return 2;
}

static int script_sprite_center_draw_location(lua_State* L) {
    Sprite sprite = luascript_read_userdata(L, SPRITE);

    float x = (float)luaL_checknumber(L, 2);
    float y = (float)luaL_checknumber(L, 3);
    float ref_width = (float)luaL_checknumber(L, 4);
    float ref_height = (float)luaL_checknumber(L, 5);

    float applied_draw_location[2];
    sprite_center_draw_location(sprite, x, y, ref_width, ref_height, applied_draw_location);

    lua_pushnumber(L, applied_draw_location[0]);
    lua_pushnumber(L, applied_draw_location[1]);
    return 2;
}

static int script_sprite_set_antialiasing(lua_State* L) {
    Sprite sprite = luascript_read_userdata(L, SPRITE);

    PVRFLAG antialiasing = luascript_parse_pvrflag(L, luaL_checkstring(L, 2));

    sprite_set_antialiasing(sprite, antialiasing);

    return 0;
}

static int script_sprite_flip_rendered_texture(lua_State* L) {
    Sprite sprite = luascript_read_userdata(L, SPRITE);

    int flip_x = -1, flip_y = -1;

    if (!lua_isnil(L, 2)) flip_x = lua_toboolean(L, 2) ? 1 : 0;
    if (!lua_isnil(L, 3)) flip_y = lua_toboolean(L, 3) ? 1 : 0;

    sprite_flip_rendered_texture(sprite, flip_x, flip_y);

    return 0;
}

static int script_sprite_flip_rendered_texture_enable_correction(lua_State* L) {
    Sprite sprite = luascript_read_userdata(L, SPRITE);

    bool enabled = lua_toboolean(L, 2);

    sprite_flip_rendered_texture_enable_correction(sprite, enabled);

    return 0;
}

static int script_sprite_set_shader(lua_State* L) {
    Sprite sprite = luascript_read_userdata(L, SPRITE);

    PSShader psshader = luascript_read_nullable_userdata(L, 2, PSSHADER);

    sprite_set_shader(sprite, psshader);

    return 0;
}

static int script_sprite_get_shader(lua_State* L) {
    Sprite sprite = luascript_read_userdata(L, SPRITE);

    PSShader psshader = sprite_get_shader(sprite);

    return script_psshader_new(L, psshader);
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const luaL_Reg SPRITE_FUNCTIONS[] = {
    {"matrix_get_modifier", script_sprite_matrix_get_modifier},
    {"set_offset_source", script_sprite_set_offset_source},
    {"set_offset_frame", script_sprite_set_offset_frame},
    {"set_offset_pivot", script_sprite_set_offset_pivot},
    {"matrix_reset", script_sprite_matrix_reset},
    {"set_draw_location", script_sprite_set_draw_location},
    {"set_draw_size", script_sprite_set_draw_size},
    {"set_draw_size_from_source_size", script_sprite_set_draw_size_from_source_size},
    {"set_alpha", script_sprite_set_alpha},
    {"set_visible", script_sprite_set_visible},
    {"set_z_index", script_sprite_set_z_index},
    {"set_z_offset", script_sprite_set_z_offset},
    {"get_source_size", script_sprite_get_source_size},
    {"set_vertex_color", script_sprite_set_vertex_color},
    {"set_offsetcolor", script_sprite_set_offsetcolor},
    {"is_textured", script_sprite_is_textured},
    {"crop", script_sprite_crop},
    {"is_crop_enabled", script_sprite_is_crop_enabled},
    {"crop_enable", script_sprite_crop_enable},
    {"resize_draw_size", script_sprite_resize_draw_size},
    {"center_draw_location", script_sprite_center_draw_location},
    {"set_antialiasing", script_sprite_set_antialiasing},
    {"flip_rendered_texture", script_sprite_flip_rendered_texture},
    {"flip_rendered_texture_enable_correction", script_sprite_flip_rendered_texture_enable_correction},
    {"set_shader", script_sprite_set_shader},
    {"get_shader", script_sprite_get_shader},
    {NULL, NULL}
};


int script_sprite_new(lua_State* L, Sprite sprite) {
    return luascript_userdata_new(L, SPRITE, sprite);
}

static int script_sprite_gc(lua_State* L) {
    return luascript_userdata_gc(L, SPRITE);
}

static int script_sprite_tostring(lua_State* L) {
    return luascript_userdata_tostring(L, SPRITE);
}


void script_sprite_register(lua_State* L) {
    luascript_register(L, SPRITE, script_sprite_gc, script_sprite_tostring, SPRITE_FUNCTIONS);
}
