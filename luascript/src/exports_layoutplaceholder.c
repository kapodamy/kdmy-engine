#include "luascript_internal.h"
#include "engine_string.h"

#ifdef JAVASCRIPT
static LayoutPlaceholder_t temp_layoutplaceholder = {};

static LayoutPlaceholder kdmy_read_layoutplaceholder(LayoutPlaceholder layoutplaceholder) {
    if (!layoutplaceholder) return NULL;

    void* parallax = kdmy_read_prop_object(layoutplaceholder, "parallax");

    temp_layoutplaceholder.group_id = kdmy_read_prop_integer(layoutplaceholder, "group_id");
    temp_layoutplaceholder.name = kdmy_read_prop_string(layoutplaceholder, "name");
    temp_layoutplaceholder.align_vertical = kdmy_read_prop_integer(layoutplaceholder, "align_vertical");
    temp_layoutplaceholder.align_horizontal = kdmy_read_prop_integer(layoutplaceholder, "align_horizontal");
    temp_layoutplaceholder.x = kdmy_read_prop_float(layoutplaceholder, "x");
    temp_layoutplaceholder.y = kdmy_read_prop_float(layoutplaceholder, "y");
    temp_layoutplaceholder.z = kdmy_read_prop_float(layoutplaceholder, "z");
    temp_layoutplaceholder.height = kdmy_read_prop_float(layoutplaceholder, "height");
    temp_layoutplaceholder.width = kdmy_read_prop_float(layoutplaceholder, "width");
    temp_layoutplaceholder.parallax.x = kdmy_read_prop_float(parallax, "x");
    temp_layoutplaceholder.parallax.y = kdmy_read_prop_float(parallax, "y");
    temp_layoutplaceholder.parallax.z = kdmy_read_prop_float(parallax, "z");
    temp_layoutplaceholder.static_camera = kdmy_read_prop_boolean(layoutplaceholder, "static_camera");

    kdmy_forget_obtained(parallax);

    return &temp_layoutplaceholder;
}

#endif

const char TOSTRING[] = "{ "
    "groupId: %i, "
    "name: %s, "
    "alignVertical: %s, "
    "alignHorizontal: %s, "
    "x: %.6f, "
    "y: %.6f, "
    "z: %.6f, "
    "height: %.6f, "
    "width: %.6f, "
    "parallaxX: %.6f, "
    "parallaxY: %.6f, "
    "parallaxZ: %.6f, "
    "staticCamera: %s "
    "}";


static int script_layoutplaceholder_index(lua_State* L) {
    LayoutPlaceholder layoutplaceholder = luascript_read_userdata(L, LAYOUTPLACEHOLDER);
    const char* field = luaL_optstring(L, 2, NULL);

#ifdef JAVASCRIPT
    layoutplaceholder = kdmy_read_layoutplaceholder(layoutplaceholder);
#endif

    if (string_equals(field, "groupId"))
        lua_pushinteger(L, layoutplaceholder->group_id);
    else if (string_equals(field, "name"))
        lua_pushstring(L, layoutplaceholder->name);
    else if (string_equals(field, "alignVertical"))
        lua_pushstring(L, luascript_stringify_align(layoutplaceholder->align_vertical));
    else if (string_equals(field, "alignHorizontal"))
        lua_pushstring(L, luascript_stringify_align(layoutplaceholder->align_horizontal));
    else if (string_equals(field, "x"))
        lua_pushnumber(L, layoutplaceholder->x);
    else if (string_equals(field, "y"))
        lua_pushnumber(L, layoutplaceholder->y);
    else if (string_equals(field, "z"))
        lua_pushnumber(L, layoutplaceholder->z);
    else if (string_equals(field, "height"))
        lua_pushnumber(L, layoutplaceholder->height);
    else if (string_equals(field, "width"))
        lua_pushnumber(L, layoutplaceholder->width);
    else if (string_equals(field, "parallaxX"))
        lua_pushnumber(L, layoutplaceholder->parallax.x);
    else if (string_equals(field, "parallaxY"))
        lua_pushnumber(L, layoutplaceholder->parallax.y);
    else if (string_equals(field, "parallaxZ"))
        lua_pushnumber(L, layoutplaceholder->parallax.z);
    else if (string_equals(field, "staticCamera"))
        lua_pushboolean(L, layoutplaceholder->static_camera);
    else
        layoutplaceholder = NULL;

#ifdef JAVASCRIPT
    free(layoutplaceholder->name);
#endif

    if (!layoutplaceholder) {
        return luaL_error(L, "unknown LayoutPlaceholder field '%s'", field);
    }

    return 1;
}

static int script_layoutplaceholder_newindex(lua_State* L) {
    LayoutPlaceholder layoutplaceholder = luascript_read_userdata(L, LAYOUTPLACEHOLDER);
    const char* field = luaL_optstring(L, 2, NULL);

#ifdef JAVASCRIPT

    void* parallax = kdmy_read_prop_object(layoutplaceholder, "parallax");

    if (string_equals(field, "groupId"))
        kdmy_write_prop_integer(layoutplaceholder, "group_id", luaL_checkinteger(L, 3));
    else if (string_equals(field, "name"))
        return luaL_error(L, "the field 'name' of LayoutPlaceholder is read-only");
    else if (string_equals(field, "alignVertical"))
        kdmy_write_prop_integer(layoutplaceholder, "align_vertical", luascript_parse_align(L, luaL_optstring(L, 3, NULL)));
    else if (string_equals(field, "alignHorizontal"))
        kdmy_write_prop_integer(layoutplaceholder, "align_horizontal", luascript_parse_align(L, luaL_optstring(L, 3, NULL)));
    else if (string_equals(field, "x"))
        kdmy_write_prop_float(layoutplaceholder, "x", (float)luaL_checknumber(L, 3));
    else if (string_equals(field, "y"))
        kdmy_write_prop_float(layoutplaceholder, "y", (float)luaL_checknumber(L, 3));
    else if (string_equals(field, "z"))
        kdmy_write_prop_float(layoutplaceholder, "z", (float)luaL_checknumber(L, 3));
    else if (string_equals(field, "height"))
        kdmy_write_prop_float(layoutplaceholder, "height", (float)luaL_checknumber(L, 3));
    else if (string_equals(field, "width"))
        kdmy_write_prop_float(layoutplaceholder, "width", (float)luaL_checknumber(L, 3));
    else if (string_equals(field, "parallaxX"))
        kdmy_write_prop_float(parallax, "x", (float)luaL_checknumber(L, 3));
    else if (string_equals(field, "parallaxY"))
        kdmy_write_prop_float(parallax, "y", (float)luaL_checknumber(L, 3));
    else if (string_equals(field, "parallaxZ"))
        kdmy_write_prop_float(parallax, "z", (float)luaL_checknumber(L, 3));
    else if (string_equals(field, "staticCamera"))
        kdmy_write_prop_boolean(layoutplaceholder, "static_camera", lua_toboolean(L, 3));
    else
        layoutplaceholder = NULL;

    kdmy_forget_obtained(parallax);

#else

    if (string_equals(field, "groupId"))
        layoutplaceholder->group_id = luaL_checkinteger(L, 3);
    else if (string_equals(field, "name"))
        return luaL_error(L, "the field 'name' of LayoutPlaceholder is read-only");
    else if (string_equals(field, "alignVertical"))
        layoutplaceholder->align_vertical = luascript_parse_align(L, luaL_optstring(L, 3, NULL));
    else if (string_equals(field, "alignHorizontal"))
        layoutplaceholder->align_horizontal = luascript_parse_align(L, luaL_optstring(L, 3, NULL));
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
        layoutplaceholder = NULL;
#endif

    if (!layoutplaceholder) {
        return luaL_error(L, "unknown LayoutPlaceholder field '%s'", field);
    }

    return 0;
}

int script_layoutplaceholder_new(lua_State* L, LayoutPlaceholder layoutplaceholder) {
    return luascript_userdata_new(L, LAYOUTPLACEHOLDER, layoutplaceholder);
}

static int script_layoutplaceholder_gc(lua_State* L) {
    return luascript_userdata_gc(L, LAYOUTPLACEHOLDER);
}

static int script_layoutplaceholder_tostring(lua_State* L) {
    LayoutPlaceholder layoutplaceholder = luascript_read_userdata(L, LAYOUTPLACEHOLDER);

#ifdef JAVASCRIPT
    layoutplaceholder = kdmy_read_layoutplaceholder(layoutplaceholder);
#endif

    lua_pushfstring(L, TOSTRING,
        layoutplaceholder->group_id,
        layoutplaceholder->name,
        luascript_stringify_align(layoutplaceholder->align_vertical),
        luascript_stringify_align(layoutplaceholder->align_horizontal),
        layoutplaceholder->x,
        layoutplaceholder->y,
        layoutplaceholder->z,
        layoutplaceholder->height,
        layoutplaceholder->width,
        layoutplaceholder->parallax.x,
        layoutplaceholder->parallax.y,
        layoutplaceholder->parallax.z,
        layoutplaceholder->static_camera ? "true" : "false"
    );

#ifdef JAVASCRIPT
    free(layoutplaceholder->name);
#endif

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

