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
    const values = [0, 0];

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
EM_JS_PRFX(bool, sprite_crop, (Sprite sprite, float  dx, float dy, float dwidth, float  dheight), {
    return sprite_crop(kdmyEngine_obtain(sprite), dx, dy, dwidth, dheight);
    });
EM_JS_PRFX(bool, sprite_is_crop_enabled, (Sprite sprite), {
    return sprite_is_crop_enabled(kdmyEngine_obtain(sprite));
    });
EM_JS_PRFX(void, sprite_crop_enable, (Sprite sprite, bool enable), {
    sprite_crop_enable(kdmyEngine_obtain(sprite), enable);
    });
EM_JS_PRFX(float*, sprite_resize_draw_size, (Sprite sprite, float max_width, float  max_height, float* applied_draw_size), {
    const HEAP_ENDIANESS = true;
    const dataView = new DataView(buffer);
    const values = [0, 0];

    sprite_resize_draw_size(kdmyEngine_obtain(sprite), max_width, max_height, values);
    dataView.setFloat32(applied_draw_size + 0, values[0], HEAP_ENDIANESS);
    dataView.setFloat32(applied_draw_size + 4, values[1], HEAP_ENDIANESS);

    return applied_draw_size;
    });
EM_JS_PRFX(float*, sprite_center_draw_location, (Sprite sprite, float x, float y, float ref_width, float ref_height, float* applied_draw_location), {
    const HEAP_ENDIANESS = true;
    const dataView = new DataView(buffer);
    const values = [0, 0];

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
    READ_USERDATA(L, Sprite, sprite, SPRITE);

    Modifier modifier = sprite_matrix_get_modifier(sprite);
    return script_modifier_new(L, sprite, modifier);
}

static int script_sprite_set_offset_source(lua_State* L) {
    READ_USERDATA(L, Sprite, sprite, SPRITE);

    float x = luaL_checkfloat(L, 2);
    float y = luaL_checkfloat(L, 3);
    float width = luaL_checkfloat(L, 4);
    float height = luaL_checkfloat(L, 5);

    sprite_set_offset_source(sprite, x, y, width, height);

    return 0;
}

static int script_sprite_set_offset_frame(lua_State* L) {
    READ_USERDATA(L, Sprite, sprite, SPRITE);

    float x = luaL_checkfloat(L, 2);
    float y = luaL_checkfloat(L, 3);
    float width = luaL_checkfloat(L, 4);
    float height = luaL_checkfloat(L, 5);

    sprite_set_offset_frame(sprite, x, y, width, height);

    return 0;
}

static int script_sprite_set_offset_pivot(lua_State* L) {
    READ_USERDATA(L, Sprite, sprite, SPRITE);

    float x = luaL_checkfloat(L, 2);
    float y = luaL_checkfloat(L, 3);

    sprite_set_offset_pivot(sprite, x, y);
    return 0;
}

static int script_sprite_matrix_reset(lua_State* L) {
    READ_USERDATA(L, Sprite, sprite, SPRITE);

    sprite_matrix_reset(sprite);

    return 0;
}

static int script_sprite_set_draw_location(lua_State* L) {
    READ_USERDATA(L, Sprite, sprite, SPRITE);

    float x = luaL_checkfloat(L, 2);
    float y = luaL_checkfloat(L, 3);

    sprite_set_draw_location(sprite, x, y);

    return 0;
}

static int script_sprite_set_draw_size(lua_State* L) {
    READ_USERDATA(L, Sprite, sprite, SPRITE);

    float width = luaL_checkfloat(L, 2);
    float height = luaL_checkfloat(L, 3);

    sprite_set_draw_size(sprite, width, height);

    return 0;
}

static int script_sprite_set_draw_size_from_source_size(lua_State* L) {
    READ_USERDATA(L, Sprite, sprite, SPRITE);

    sprite_set_draw_size_from_source_size(sprite);

    return 0;
}

static int script_sprite_set_alpha(lua_State* L) {
    READ_USERDATA(L, Sprite, sprite, SPRITE);

    float alpha = luaL_checkfloat(L, 2);

    sprite_set_alpha(sprite, alpha);

    return 0;
}

static int script_sprite_set_visible(lua_State* L) {
    READ_USERDATA(L, Sprite, sprite, SPRITE);

    bool visible = luaL_checkboolean(L, 2);

    sprite_set_visible(sprite, visible);

    return 0;
}

static int script_sprite_set_z_index(lua_State* L) {
    READ_USERDATA(L, Sprite, sprite, SPRITE);

    float index = luaL_checkfloat(L, 2);

    sprite_set_z_index(sprite, index);

    return 0;
}

static int script_sprite_set_z_offset(lua_State* L) {
    READ_USERDATA(L, Sprite, sprite, SPRITE);

    float index = luaL_checkfloat(L, 2);

    sprite_set_z_offset(sprite, index);

    return 0;
}

static int script_sprite_get_source_size(lua_State* L) {
    READ_USERDATA(L, Sprite, sprite, SPRITE);

    float size[2];
    sprite_get_source_size(sprite, size);

    lua_pushnumber(L, size[0]);
    lua_pushnumber(L, size[1]);

    return 2;
}

static int script_sprite_set_vertex_color(lua_State* L) {
    READ_USERDATA(L, Sprite, sprite, SPRITE);

    float r = luaL_optionalfloat(L, 2);
    float g = luaL_optionalfloat(L, 3);
    float b = luaL_optionalfloat(L, 4);

    sprite_set_vertex_color(sprite, r, g, b);

    return 0;
}

static int script_sprite_set_offsetcolor(lua_State* L) {
    READ_USERDATA(L, Sprite, sprite, SPRITE);

    float r = luaL_optionalfloat(L, 2);
    float g = luaL_optionalfloat(L, 3);
    float b = luaL_optionalfloat(L, 4);
    float a = luaL_optionalfloat(L, 5);

    sprite_set_offsetcolor(sprite, r, g, b, a);

    return 0;
}

static int script_sprite_is_textured(lua_State* L) {
    READ_USERDATA(L, Sprite, sprite, SPRITE);

    bool ret = sprite_is_textured(sprite);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_sprite_crop(lua_State* L) {
    READ_USERDATA(L, Sprite, sprite, SPRITE);

    float dx = luaL_optionalfloat(L, 2);
    float dy = luaL_optionalfloat(L, 3);
    float dwidth = luaL_optionalfloat(L, 4);
    float dheight = luaL_optionalfloat(L, 5);

    bool ret = sprite_crop(sprite, dx, dy, dwidth, dheight);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_sprite_is_crop_enabled(lua_State* L) {
    READ_USERDATA(L, Sprite, sprite, SPRITE);

    bool ret = sprite_is_crop_enabled(sprite);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_sprite_crop_enable(lua_State* L) {
    READ_USERDATA(L, Sprite, sprite, SPRITE);

    bool enable = luaL_checkboolean(L, 2);

    sprite_crop_enable(sprite, enable);

    return 0;
}

static int script_sprite_resize_draw_size(lua_State* L) {
    READ_USERDATA(L, Sprite, sprite, SPRITE);

    float max_width = luaL_checkfloat(L, 2);
    float max_height = luaL_checkfloat(L, 3);

    float applied_draw_size[2];
    sprite_resize_draw_size(sprite, max_width, max_height, applied_draw_size);

    lua_pushnumber(L, applied_draw_size[0]);
    lua_pushnumber(L, applied_draw_size[1]);
    return 2;
}

static int script_sprite_center_draw_location(lua_State* L) {
    READ_USERDATA(L, Sprite, sprite, SPRITE);

    float x = luaL_checkfloat(L, 2);
    float y = luaL_checkfloat(L, 3);
    float ref_width = luaL_checkfloat(L, 4);
    float ref_height = luaL_checkfloat(L, 5);

    float applied_draw_location[2];
    sprite_center_draw_location(sprite, x, y, ref_width, ref_height, applied_draw_location);

    lua_pushnumber(L, applied_draw_location[0]);
    lua_pushnumber(L, applied_draw_location[1]);
    return 2;
}

static int script_sprite_set_antialiasing(lua_State* L) {
    READ_USERDATA(L, Sprite, sprite, SPRITE);

    PVRFLAG antialiasing = _parse_pvrflag(L, luaL_checkstring(L, 2));

    sprite_set_antialiasing(sprite, antialiasing);

    return 0;
}

static int script_sprite_flip_rendered_texture(lua_State* L) {
    READ_USERDATA(L, Sprite, sprite, SPRITE);

    int flip_x = -1, flip_y = -1;

    if (!lua_isnil(L, 2)) flip_x = luaL_checkboolean(L, 2) ? 1 : 0;
    if (!lua_isnil(L, 3)) flip_y = luaL_checkboolean(L, 3) ? 1 : 0;

    sprite_flip_rendered_texture(sprite, flip_x, flip_y);

    return 0;
}

static int script_sprite_flip_rendered_texture_enable_correction(lua_State* L) {
    READ_USERDATA(L, Sprite, sprite, SPRITE);

    bool enabled = luaL_checkboolean(L, 2);

    sprite_flip_rendered_texture_enable_correction(sprite, enabled);

    return 0;
}

static int script_sprite_set_shader(lua_State* L) {
    READ_USERDATA(L, Sprite, sprite, SPRITE);

    PSShader psshader = LUA_TOUSERDATA_OR_NULL(L, 2, PSShader, PSSHADER);

    sprite_set_shader(sprite, psshader);

    return 0;
}

static int script_sprite_get_shader(lua_State* L) {
    READ_USERDATA(L, Sprite, sprite, SPRITE);

    PSShader psshader = sprite_get_shader(sprite);

    if (!psshader) {
        lua_pushnil(L);
        return 1;
    }

    return script_psshader_new(L, sprite, psshader);
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


int script_sprite_new(lua_State* L, Layout layout, Sprite sprite) {
    return NEW_USERDATA(L, SPRITE, layout, sprite, true);
}

static int script_sprite_gc(lua_State* L) {
    READ_USERDATA_UNCHECKED(L, Sprite, sprite, SPRITE);
    _luascript_suppress_item(L, sprite, true);
    return 0;
}

static int script_sprite_tostring(lua_State* L) {
    READ_USERDATA(L, Sprite, sprite, SPRITE);
    lua_pushstring(L, "[Sprite]");
    return 1;
}


inline void register_sprite(lua_State* L) {
    _luascript_register(
        L,
        SPRITE,
        script_sprite_gc,
        script_sprite_tostring,
        SPRITE_FUNCTIONS
    );
}

