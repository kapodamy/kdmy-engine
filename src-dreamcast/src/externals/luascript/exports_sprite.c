#include "luascript_internal.h"

#include "luascript_enums.h"
#include "sprite.h"


static int script_sprite_matrix_get_modifier(lua_State* L) {
    Sprite sprite = luascript_read_userdata(L, SPRITE);

    Modifier* modifier = sprite_matrix_get_modifier(sprite);
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

    float source_width, source_height;
    sprite_get_source_size(sprite, &source_width, &source_height);

    lua_pushnumber(L, (lua_Number)source_width);
    lua_pushnumber(L, (lua_Number)source_height);

    return 2;
}

static int script_sprite_set_vertex_color(lua_State* L) {
    Sprite sprite = luascript_read_userdata(L, SPRITE);

    float r = (float)luaL_optnumber(L, 2, FLOAT_NaN);
    float g = (float)luaL_optnumber(L, 3, FLOAT_NaN);
    float b = (float)luaL_optnumber(L, 4, FLOAT_NaN);

    sprite_set_vertex_color(sprite, r, g, b);

    return 0;
}

static int script_sprite_set_offsetcolor(lua_State* L) {
    Sprite sprite = luascript_read_userdata(L, SPRITE);

    float r = (float)luaL_optnumber(L, 2, FLOAT_NaN);
    float g = (float)luaL_optnumber(L, 3, FLOAT_NaN);
    float b = (float)luaL_optnumber(L, 4, FLOAT_NaN);
    float a = (float)luaL_optnumber(L, 5, FLOAT_NaN);

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

    float dx = (float)luaL_optnumber(L, 2, FLOAT_NaN);
    float dy = (float)luaL_optnumber(L, 3, FLOAT_NaN);
    float dwidth = (float)luaL_optnumber(L, 4, FLOAT_NaN);
    float dheight = (float)luaL_optnumber(L, 5, FLOAT_NaN);

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

    float applied_draw_x, applied_draw_y;
    sprite_resize_draw_size(sprite, max_width, max_height, &applied_draw_x, &applied_draw_y);

    lua_pushnumber(L, (lua_Number)applied_draw_x);
    lua_pushnumber(L, (lua_Number)applied_draw_y);
    return 2;
}

static int script_sprite_center_draw_location(lua_State* L) {
    Sprite sprite = luascript_read_userdata(L, SPRITE);

    float x = (float)luaL_checknumber(L, 2);
    float y = (float)luaL_checknumber(L, 3);
    float ref_width = (float)luaL_checknumber(L, 4);
    float ref_height = (float)luaL_checknumber(L, 5);

    float applied_draw_x, applied_draw_y;
    sprite_center_draw_location(sprite, x, y, ref_width, ref_height, &applied_draw_x, &applied_draw_y);

    lua_pushnumber(L, (lua_Number)applied_draw_x);
    lua_pushnumber(L, (lua_Number)applied_draw_y);
    return 2;
}

static int script_sprite_set_antialiasing(lua_State* L) {
    Sprite sprite = luascript_read_userdata(L, SPRITE);

    PVRFlag antialiasing = luascript_helper_optenum(L, 2, LUASCRIPT_ENUMS_PVRFlag);

    sprite_set_antialiasing(sprite, antialiasing);

    return 0;
}

static int script_sprite_flip_rendered_texture(lua_State* L) {
    Sprite sprite = luascript_read_userdata(L, SPRITE);

    nbool flip_x = luascript_helper_optnbool(L, 2);
    nbool flip_y = luascript_helper_optnbool(L, 3);

    sprite_flip_rendered_texture(sprite, flip_x, flip_y);

    return 0;
}

static int script_sprite_flip_rendered_texture_enable_correction(lua_State* L) {
    Sprite sprite = luascript_read_userdata(L, SPRITE);

    bool enabled = lua_toboolean(L, 2);

    sprite_flip_rendered_texture_enable_correction(sprite, enabled);

    return 0;
}

#ifndef _arch_dreamcast
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
#endif

static int script_sprite_blend_enable(lua_State* L) {
    Sprite sprite = luascript_read_userdata(L, SPRITE);
    bool enabled = (bool)lua_toboolean(L, 2);

    sprite_blend_enable(sprite, enabled);

    return 0;
}

static int script_sprite_blend_set(lua_State* L) {
    Sprite sprite = luascript_read_userdata(L, SPRITE);
    Blend src_rgb = luascript_helper_optenum(L, 2, LUASCRIPT_ENUMS_Blend);
    Blend dst_rgb = luascript_helper_optenum(L, 3, LUASCRIPT_ENUMS_Blend);
    Blend src_alpha = luascript_helper_optenum(L, 4, LUASCRIPT_ENUMS_Blend);
    Blend dst_alpha = luascript_helper_optenum(L, 5, LUASCRIPT_ENUMS_Blend);

    sprite_blend_set(sprite, src_rgb, dst_rgb, src_alpha, dst_alpha);

    return 0;
}

static int script_sprite_trailing_enabled(lua_State* L) {
    Sprite sprite = luascript_read_userdata(L, SPRITE);
    bool enabled = (bool)lua_toboolean(L, 2);

    sprite_trailing_enabled(sprite, enabled);

    return 0;
}

static int script_sprite_trailing_set_params(lua_State* L) {
    Sprite sprite = luascript_read_userdata(L, SPRITE);
    int32_t length = (int32_t)luaL_checkinteger(L, 2);
    float trail_delay = (float)luaL_checknumber(L, 3);
    float trail_alpha = (float)luaL_checknumber(L, 4);
    bool darken_colors = luascript_helper_optnbool(L, 5);

    sprite_trailing_set_params(sprite, length, trail_delay, trail_alpha, darken_colors);

    return 0;
}

static int script_sprite_trailing_set_offsetcolor(lua_State* L) {
    Sprite sprite = luascript_read_userdata(L, SPRITE);
    float r = (float)luaL_optnumber(L, 2, FLOAT_NaN);
    float g = (float)luaL_optnumber(L, 3, FLOAT_NaN);
    float b = (float)luaL_optnumber(L, 4, FLOAT_NaN);

    sprite_trailing_set_offsetcolor(sprite, r, g, b);

    return 0;
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
#ifndef _arch_dreamcast
    {"set_shader", script_sprite_set_shader},
    {"get_shader", script_sprite_get_shader},
#endif
    {"blend_enable", script_sprite_blend_enable},
    {"blend_set", script_sprite_blend_set},
    {"trailing_enabled", script_sprite_trailing_enabled},
    {"trailing_set_params", script_sprite_trailing_set_params},
    {"trailing_set_offsetcolor", script_sprite_trailing_set_offsetcolor},
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
