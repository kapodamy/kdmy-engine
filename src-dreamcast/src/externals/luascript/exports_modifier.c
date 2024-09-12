#include "luascript_internal.h"

#include "modifier.h"

#include "malloc_utils.h"
#include "math2d.h"
#include "stringbuilder.h"
#include "stringutils.h"


static const char* TOSTRING_INTERNAL_FORMAT =
    "{ "
    "translateX: $6f, "
    "translateY: $6f, "
    "rotate: $6f, "
    "skewX: $6f, "
    "skewY: $6f, "
    "scaleX: $6f, "
    "scaleY: $6f, "
    "scaleDirectionX: $6f, "
    "scaleDirectionY: $6f, "
    "rotatePivotEnabled: $b, "
    "rotatePivotU: $6f, "
    "rotatePivotV: $6f, "
    "translateRotation: $b, "
    "scaleSize: $b, "
    "scaleTranslation: $b, "
    "x: $6f, "
    "y: $6f, "
    "width: $6f, "
    "height: $6f, "
    "}";


int script_modifier_new(lua_State* L, Modifier* modifier) {
    return luascript_userdata_new(L, MODIFIER, modifier);
}

static int script_modifier_gc(lua_State* L) {
    return luascript_userdata_gc(L, MODIFIER);
}

static int script_modifier_tostring(lua_State* L) {
    Modifier* modifier = luascript_read_userdata(L, MODIFIER);

    char* str = stringbuilder_helper_create_formatted_string(
        TOSTRING_INTERNAL_FORMAT,
        modifier->translate_x,
        modifier->translate_y,
        modifier->rotate,
        modifier->skew_x,
        modifier->skew_y,
        modifier->scale_x,
        modifier->scale_y,
        modifier->scale_direction_x,
        modifier->scale_direction_y,
        modifier->rotate_pivot_enabled == true,
        modifier->rotate_pivot_u,
        modifier->rotate_pivot_v,
        modifier->translate_rotation == true,
        modifier->scale_size == true,
        modifier->scale_translation == true,
        modifier->x,
        modifier->y,
        modifier->width,
        modifier->height
    );

    lua_pushstring(L, str);
    free_chk(str);

    return 1;
}

static int script_modifier_index(lua_State* L) {
    Modifier* modifier = luascript_read_userdata(L, MODIFIER);
    const char* field = luaL_optstring(L, 2, NULL);

    if (string_equals(field, "translateX"))
        lua_pushnumber(L, (lua_Number)modifier->translate_x);
    else if (string_equals(field, "translateY"))
        lua_pushnumber(L, (lua_Number)modifier->translate_y);
    else if (string_equals(field, "rotate"))
        lua_pushnumber(L, (lua_Number)modifier->rotate);
    else if (string_equals(field, "rotateByDegrees"))
        lua_pushnumber(L, (lua_Number)modifier->rotate * MATH2D_DEG_TO_RAD);
    else if (string_equals(field, "skewX"))
        lua_pushnumber(L, (lua_Number)modifier->skew_x);
    else if (string_equals(field, "skewY"))
        lua_pushnumber(L, (lua_Number)modifier->skew_y);
    else if (string_equals(field, "scaleX"))
        lua_pushnumber(L, (lua_Number)modifier->scale_x);
    else if (string_equals(field, "scaleY"))
        lua_pushnumber(L, (lua_Number)modifier->scale_y);
    else if (string_equals(field, "scaleDirectionX"))
        lua_pushnumber(L, (lua_Number)modifier->scale_direction_x);
    else if (string_equals(field, "scaleDirectionY"))
        lua_pushnumber(L, (lua_Number)modifier->scale_direction_y);
    else if (string_equals(field, "rotatePivotEnabled"))
        lua_pushboolean(L, modifier->rotate_pivot_enabled == true);
    else if (string_equals(field, "rotatePivotU"))
        lua_pushnumber(L, (lua_Number)modifier->rotate_pivot_u);
    else if (string_equals(field, "rotatePivotV"))
        lua_pushnumber(L, (lua_Number)modifier->rotate_pivot_v);
    else if (string_equals(field, "translateRotation"))
        lua_pushboolean(L, modifier->translate_rotation == true);
    else if (string_equals(field, "scaleSize"))
        lua_pushboolean(L, modifier->scale_size == true);
    else if (string_equals(field, "scaleTranslation"))
        lua_pushboolean(L, modifier->scale_translation == true);
    else if (string_equals(field, "x"))
        lua_pushnumber(L, (lua_Number)modifier->x);
    else if (string_equals(field, "y"))
        lua_pushnumber(L, (lua_Number)modifier->y);
    else if (string_equals(field, "width"))
        lua_pushnumber(L, (lua_Number)modifier->width);
    else if (string_equals(field, "height"))
        lua_pushnumber(L, (lua_Number)modifier->height);
    else
        return luaL_error(L, "unknown modifier field '%s'", field);

    return 1;
}

static int script_modifier_newindex(lua_State* L) {
    Modifier* modifier = luascript_read_userdata(L, MODIFIER);
    const char* field = luaL_optstring(L, 2, NULL);

    if (string_equals(field, "translateX"))
        modifier->translate_x = (float)luaL_checknumber(L, 3);
    else if (string_equals(field, "translateY"))
        modifier->translate_y = (float)luaL_checknumber(L, 3);
    else if (string_equals(field, "rotateByDegrees"))
        modifier->rotate = (float)(luaL_checknumber(L, 3) * MATH2D_DEG_TO_RAD);
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


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

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
