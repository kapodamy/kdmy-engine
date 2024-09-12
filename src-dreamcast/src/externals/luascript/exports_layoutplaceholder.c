#include "luascript_internal.h"

#include "layout_types.h"
#include "luascript_enums.h"
#include "malloc_utils.h"
#include "stringbuilder.h"
#include "stringutils.h"


static const char* LAYOUTPLACEHOLDER_INTERNAL_TOSTRING =
    "{ "
    "groupId: $i, "
    "name: $s, "
    "alignVertical: $s, "
    "alignHorizontal: $s, "
    "x: $6f, "
    "y: $6f, "
    "z: $6f, "
    "height: $6f, "
    "width: $6f, "
    "parallaxX: $6f, "
    "parallaxY: $6f, "
    "parallaxZ: $6f, "
    "staticCamera: $b "
    "}";


static int script_layoutplaceholder_index(lua_State* L) {
    LayoutPlaceholder* layoutplaceholder = luascript_read_userdata(L, LAYOUTPLACEHOLDER);
    const char* field = luaL_optstring(L, 2, NULL);

    if (string_equals(field, "groupId"))
        lua_pushinteger(L, (lua_Integer)layoutplaceholder->group_id);
    else if (string_equals(field, "name"))
        lua_pushstring(L, layoutplaceholder->name);
    else if (string_equals(field, "alignVertical"))
        luascript_helper_pushenum(L, LUASCRIPT_ENUMS_Align, layoutplaceholder->align_vertical);
    else if (string_equals(field, "alignHorizontal"))
        luascript_helper_pushenum(L, LUASCRIPT_ENUMS_Align, layoutplaceholder->align_horizontal);
    else if (string_equals(field, "x"))
        lua_pushnumber(L, (lua_Number)layoutplaceholder->x);
    else if (string_equals(field, "y"))
        lua_pushnumber(L, (lua_Number)layoutplaceholder->y);
    else if (string_equals(field, "z"))
        lua_pushnumber(L, (lua_Number)layoutplaceholder->z);
    else if (string_equals(field, "height"))
        lua_pushnumber(L, (lua_Number)layoutplaceholder->height);
    else if (string_equals(field, "width"))
        lua_pushnumber(L, (lua_Number)layoutplaceholder->width);
    else if (string_equals(field, "parallaxX"))
        lua_pushnumber(L, (lua_Number)layoutplaceholder->parallax.x);
    else if (string_equals(field, "parallaxY"))
        lua_pushnumber(L, (lua_Number)layoutplaceholder->parallax.y);
    else if (string_equals(field, "parallaxZ"))
        lua_pushnumber(L, (lua_Number)layoutplaceholder->parallax.z);
    else if (string_equals(field, "staticCamera"))
        lua_pushboolean(L, layoutplaceholder->static_camera);
    else
        return luaL_error(L, "unknown LayoutPlaceholder field '%s'", field);

    return 1;
}

static int script_layoutplaceholder_newindex(lua_State* L) {
    LayoutPlaceholder* layoutplaceholder = luascript_read_userdata(L, LAYOUTPLACEHOLDER);
    const char* field = luaL_optstring(L, 2, NULL);

    if (string_equals(field, "groupId"))
        layoutplaceholder->group_id = luaL_checkinteger(L, 3);
    else if (string_equals(field, "name"))
        return luaL_error(L, "the field 'name' of LayoutPlaceholder is read-only");
    else if (string_equals(field, "alignVertical"))
        layoutplaceholder->align_vertical = luascript_helper_optenum(L, 3, LUASCRIPT_ENUMS_Align);
    else if (string_equals(field, "alignHorizontal"))
        layoutplaceholder->align_horizontal = luascript_helper_optenum(L, 3, LUASCRIPT_ENUMS_Align);
    else if (string_equals(field, "x"))
        layoutplaceholder->x = (float)luaL_checknumber(L, 3);
    else if (string_equals(field, "y"))
        layoutplaceholder->y = (float)luaL_checknumber(L, 3);
    else if (string_equals(field, "z"))
        layoutplaceholder->z = (float)luaL_checknumber(L, 3);
    else if (string_equals(field, "height"))
        layoutplaceholder->height = (float)luaL_checknumber(L, 3);
    else if (string_equals(field, "width"))
        layoutplaceholder->width = (float)luaL_checknumber(L, 3);
    else if (string_equals(field, "parallaxX"))
        layoutplaceholder->parallax.x = (float)luaL_checknumber(L, 3);
    else if (string_equals(field, "parallaxY"))
        layoutplaceholder->parallax.y = (float)luaL_checknumber(L, 3);
    else if (string_equals(field, "parallaxZ"))
        layoutplaceholder->parallax.z = (float)luaL_checknumber(L, 3);
    else if (string_equals(field, "staticCamera"))
        layoutplaceholder->static_camera = lua_toboolean(L, 3);
    else
        return luaL_error(L, "unknown LayoutPlaceholder field '%s'", field);

    return 0;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

int script_layoutplaceholder_new(lua_State* L, LayoutPlaceholder* layoutplaceholder) {
    return luascript_userdata_new(L, LAYOUTPLACEHOLDER, layoutplaceholder);
}

static int script_layoutplaceholder_gc(lua_State* L) {
    return luascript_userdata_gc(L, LAYOUTPLACEHOLDER);
}

static int script_layoutplaceholder_tostring(lua_State* L) {
    LayoutPlaceholder* layoutplaceholder = luascript_read_userdata(L, LAYOUTPLACEHOLDER);

    char* str = stringbuilder_helper_create_formatted_string(
        LAYOUTPLACEHOLDER_INTERNAL_TOSTRING,
        layoutplaceholder->group_id,
        layoutplaceholder->name,
        luascript_helper_enums_stringify(LUASCRIPT_ENUMS_Align, layoutplaceholder->align_vertical),
        luascript_helper_enums_stringify(LUASCRIPT_ENUMS_Align, layoutplaceholder->align_horizontal),
        layoutplaceholder->x,
        layoutplaceholder->y,
        layoutplaceholder->z,
        layoutplaceholder->height,
        layoutplaceholder->width,
        layoutplaceholder->parallax.x,
        layoutplaceholder->parallax.y,
        layoutplaceholder->parallax.z,
        layoutplaceholder->static_camera
    );

    lua_pushstring(L, str);
    free_chk(str);

    return 1;
}

void script_layoutplaceholder_register(lua_State* L) {
    luaL_newmetatable(L, LAYOUTPLACEHOLDER);

    lua_pushcfunction(L, script_layoutplaceholder_gc);
    lua_setfield(L, -2, "__gc");

    lua_pushcfunction(L, script_layoutplaceholder_tostring);
    lua_setfield(L, -2, "__tostring");

    lua_pushcfunction(L, script_layoutplaceholder_index);
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, script_layoutplaceholder_newindex);
    lua_setfield(L, -2, "__newindex");
}
