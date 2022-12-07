#include <math.h>
#include "luascript_internal.h"
#include "engine_string.h"

const float DEG_TO_RAD = (float)(3.14159265358979323846 / 180.0);

const char TOSTRING_FORMAT[] = "{ "
"translateX: %.6f, "
"translateY: %.6f, "
"rotate: %.6f, "
"skewX: %.6f, "
"skewY: %.6f, "
"scaleX: %.6f, "
"scaleY: %.6f, "
"scaleDirectionX: %.6f, "
"scaleDirectionY: %.6f, "
"rotatePivotEnabled: %i, "
"rotatePivotU: %.6f, "
"rotatePivotV: %.6f, "
"translateRotation: %i, "
"scaleSize: %i, "
"scaleTranslation: %i, "
"x: %.6f, "
"y: %.6f, "
"width: %.6f, "
"height: %.6f, "
"}";

#ifdef JAVASCRIPT

static Modifier_t temp_modifier = {};

static Modifier kdmy_read_modifier(Modifier modifier) {
    temp_modifier.translate_x = kdmy_read_prop_float(modifier, "translate_x");
    temp_modifier.translate_y = kdmy_read_prop_float(modifier, "translate_y");
    temp_modifier.rotate = kdmy_read_prop_float(modifier, "rotate");
    temp_modifier.skew_x = kdmy_read_prop_float(modifier, "skew_x");
    temp_modifier.skew_y = kdmy_read_prop_float(modifier, "skew_y");
    temp_modifier.scale_x = kdmy_read_prop_float(modifier, "scale_x");
    temp_modifier.scale_y = kdmy_read_prop_float(modifier, "scale_y");
    temp_modifier.scale_direction_x = kdmy_read_prop_float(modifier, "scale_direction_x");
    temp_modifier.scale_direction_y = kdmy_read_prop_float(modifier, "scale_direction_y");
    temp_modifier.rotate_pivot_enabled = kdmy_read_prop_floatboolean(modifier, "rotate_pivot_enabled");
    temp_modifier.rotate_pivot_u = kdmy_read_prop_float(modifier, "rotate_pivot_u");
    temp_modifier.rotate_pivot_v = kdmy_read_prop_float(modifier, "rotate_pivot_v");
    temp_modifier.translate_rotation = kdmy_read_prop_floatboolean(modifier, "translate_rotation");
    temp_modifier.scale_translation = kdmy_read_prop_floatboolean(modifier, "scale_translation");
    temp_modifier.scale_size = kdmy_read_prop_floatboolean(modifier, "scale_size");
    temp_modifier.x = kdmy_read_prop_float(modifier, "x");
    temp_modifier.y = kdmy_read_prop_float(modifier, "y");
    temp_modifier.width = kdmy_read_prop_float(modifier, "width");
    temp_modifier.height = kdmy_read_prop_float(modifier, "height");

    return &temp_modifier;
}

#endif


int script_modifier_new(lua_State* L, Modifier modifier) {
   return luascript_userdata_new(L, MODIFIER, modifier);
}

static int script_modifier_gc(lua_State* L) {
    return luascript_userdata_gc(L, MODIFIER);
}

static int script_modifier_tostring(lua_State* L) {
    Modifier modifier = luascript_read_userdata(L, MODIFIER);

#ifdef JAVASCRIPT
    modifier = kdmy_read_modifier(modifier);
#endif

    lua_pushfstring(
        L,
        TOSTRING_FORMAT,
        modifier->translate_x,
        modifier->translate_y,
        modifier->rotate,
        modifier->skew_x,
        modifier->skew_y,
        modifier->scale_x,
        modifier->scale_y,
        modifier->scale_direction_x,
        modifier->scale_direction_y,
        modifier->rotate_pivot_enabled >= 1.0f,
        modifier->rotate_pivot_u,
        modifier->rotate_pivot_v,
        modifier->translate_rotation >= 1.0f,
        modifier->scale_size >= 1.0f,
        modifier->scale_translation >= 1.0f,
        modifier->x,
        modifier->y,
        modifier->width,
        modifier->height
    );

    return 1;
}

static int script_modifier_index(lua_State* L) {
    Modifier modifier = luascript_read_userdata(L, MODIFIER);
    const char* field = luaL_optstring(L, 2, NULL);

#ifdef JAVASCRIPT
    modifier = kdmy_read_modifier(modifier);
#endif

    if (string_equals(field, "translateX"))
        lua_pushnumber(L, modifier->translate_x);
    else if (string_equals(field, "translateY"))
        lua_pushnumber(L, modifier->translate_y);
    else if (string_equals(field, "rotate"))
        lua_pushnumber(L, modifier->rotate);
    else if (string_equals(field, "skewX"))
        lua_pushnumber(L, modifier->skew_x);
    else if (string_equals(field, "skewY"))
        lua_pushnumber(L, modifier->skew_y);
    else if (string_equals(field, "scaleX"))
        lua_pushnumber(L, modifier->scale_x);
    else if (string_equals(field, "scaleY"))
        lua_pushnumber(L, modifier->scale_y);
    else if (string_equals(field, "scaleDirectionX"))
        lua_pushnumber(L, modifier->scale_direction_x);
    else if (string_equals(field, "scaleDirectionY"))
        lua_pushnumber(L, modifier->scale_direction_y);
    else if (string_equals(field, "rotatePivotEnabled"))
        lua_pushboolean(L, modifier->rotate_pivot_enabled >= 1.0f);
    else if (string_equals(field, "rotatePivotU"))
        lua_pushnumber(L, modifier->rotate_pivot_u);
    else if (string_equals(field, "rotatePivotV"))
        lua_pushnumber(L, modifier->rotate_pivot_v);
    else if (string_equals(field, "translateRotation"))
        lua_pushboolean(L, modifier->translate_rotation >= 1.0f);
    else if (string_equals(field, "scaleSize"))
        lua_pushboolean(L, modifier->scale_size >= 1.0f);
    else if (string_equals(field, "scaleTranslation"))
        lua_pushboolean(L, modifier->scale_translation >= 1.0f);
    else if (string_equals(field, "x"))
        lua_pushnumber(L, modifier->x);
    else if (string_equals(field, "y"))
        lua_pushnumber(L, modifier->y);
    else if (string_equals(field, "width"))
        lua_pushnumber(L, modifier->width);
    else if (string_equals(field, "height"))
        lua_pushnumber(L, modifier->height);
    else
        return luaL_error(L, "unknown modifier field '%s'", field);

    return 1;
}

static int script_modifier_newindex(lua_State* L) {
    Modifier modifier = luascript_read_userdata(L, MODIFIER);
    const char* field = luaL_optstring(L, 2, NULL);

#ifdef JAVASCRIPT
    modifier = kdmy_read_modifier(modifier);
#endif

    if (string_equals(field, "translateX"))
        modifier->translate_x = (float)luaL_checknumber(L, 3);
    else if (string_equals(field, "translateY"))
        modifier->translate_y = (float)luaL_checknumber(L, 3);
    else if (string_equals(field, "rotate"))
        modifier->rotate = (float)luaL_checknumber(L, 3);
    else if (string_equals(field, "skewX"))
        modifier->skew_x = (float)luaL_checknumber(L, 3);
    else if (string_equals(field, "skewY"))
        modifier->skew_y = (float)luaL_checknumber(L, 3);
    else if (string_equals(field, "scaleX"))
        modifier->scale_x = (float)luaL_checknumber(L, 3);
    else if (string_equals(field, "scaleY"))
        modifier->scale_y = (float)luaL_checknumber(L, 3);
    else if (string_equals(field, "scaleDirectionX"))
        modifier->scale_direction_x = (float)luaL_checknumber(L, 3);
    else if (string_equals(field, "scaleDirectionY"))
        modifier->scale_direction_y = (float)luaL_checknumber(L, 3);
    else if (string_equals(field, "rotatePivotEnabled"))
        modifier->rotate_pivot_enabled = lua_toboolean(L, 3);
    else if (string_equals(field, "rotatePivotU"))
        modifier->rotate_pivot_u = (float)luaL_checknumber(L, 3);
    else if (string_equals(field, "rotatePivotV"))
        modifier->rotate_pivot_v = (float)luaL_checknumber(L, 3);
    else if (string_equals(field, "translateRotation"))
        modifier->translate_rotation = lua_toboolean(L, 3);
    else if (string_equals(field, "scaleSize"))
        modifier->scale_size = lua_toboolean(L, 3);
    else if (string_equals(field, "scaleTranslation"))
        modifier->scale_translation = lua_toboolean(L, 3);
    else if (string_equals(field, "x"))
        modifier->x = (float)luaL_checknumber(L, 3);
    else if (string_equals(field, "y"))
        modifier->y = (float)luaL_checknumber(L, 3);
    else if (string_equals(field, "width"))
        modifier->width = (float)luaL_checknumber(L, 3);
    else if (string_equals(field, "height"))
        modifier->height = (float)luaL_checknumber(L, 3);
    else
        return luaL_error(L, "unknown modifier field '%s'", field);

    return 0;
}


void script_modifier_register(lua_State* lua) {
    luaL_newmetatable(lua, MODIFIER);

    lua_pushcfunction(lua, script_modifier_gc);
    lua_setfield(lua, -2, "__gc");

    lua_pushcfunction(lua, script_modifier_tostring);
    lua_setfield(lua, -2, "__tostring");

    lua_pushcfunction(lua, script_modifier_index);
    lua_setfield(lua, -2, "__index");

    lua_pushcfunction(lua, script_modifier_newindex);
    lua_setfield(lua, -2, "__newindex");

    lua_pop(lua, 1);
}
