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
EM_JS_PRFX(bool, modifier_set_field_JS, (Modifier modifier, const char* name, float value), {
    const _modifier = kdmyEngine_obtain(modifier);
    const _field = kdmyEngine_ptrToString(name);

    if (_field in _modifier) {
        _modifier[_field] = value;
        return 0;
    }

    return 1;
    });
EM_JS_PRFX(bool, modifier_get_field_JS, (Modifier modifier, const char* name, float* value_ptr), {
    const _modifier = kdmyEngine_obtain(modifier);
    const _field = kdmyEngine_ptrToString(name);

    if (_field in _modifier) {
        const HEAP_ENDIANESS = true;
        const dataView = new DataView(buffer);
        const value = _modifier[_field];

        dataView.setFloat32(value_ptr, value, HEAP_ENDIANESS);
        return 0;
    }

    return 1;
    });
#endif

#ifdef JAVASCRIPT
#include <ctype.h>

static char DESUGARED_FIELD_NAME[26];
static const char* desugar_modifier_field_nameJS(const char* lua_str) {
    if (!lua_str) return NULL;

    const size_t length = sizeof(DESUGARED_FIELD_NAME) - 1;
    size_t j = 0;

    for (size_t i = 0;lua_str[i]; i++) {
        char chr = lua_str[i];
        if (isupper(chr)) {
            DESUGARED_FIELD_NAME[j++] = '_';
            if (j >= length) break;
            DESUGARED_FIELD_NAME[j++] = tolower(chr);
            if (j >= length) break;
        } else {
            DESUGARED_FIELD_NAME[j++] = chr;
            if (j >= length) break;
        }
    }

    DESUGARED_FIELD_NAME[j] = 0;
    return DESUGARED_FIELD_NAME;
}
#endif


int script_modifier_new(lua_State* L, void* parent, Modifier modifier) {
    return NEW_USERDATA(L, MODIFIER, parent, modifier, true);
}

static int script_modifier_gc(lua_State* L) {
    READ_USERDATA_UNCHECKED(L, Modifier, modifier, MODIFIER);
    _luascript_suppress_item(L, modifier, true);
    return 0;
}

static int script_modifier_tostring(lua_State* L) {
    READ_USERDATA(L, Modifier, modifier, MODIFIER);

#ifdef JAVASCRIPT
    float translate_x;
    float translate_y;
    float rotate;
    float skew_x;
    float skew_y;
    float scale_x;
    float scale_y;
    float scale_direction_x;
    float scale_direction_y;
    float rotate_pivot_enabled;
    float rotate_pivot_u;
    float rotate_pivot_v;
    float translate_rotation;
    float scale_size;
    float scale_translation;
    float x;
    float y;
    float width;
    float height;
    modifier_get_field_JS(modifier, "translate_x", &translate_x);
    modifier_get_field_JS(modifier, "translate_y", &translate_y);
    modifier_get_field_JS(modifier, "rotate", &rotate);
    modifier_get_field_JS(modifier, "skew_x", &skew_x);
    modifier_get_field_JS(modifier, "skew_y", &skew_y);
    modifier_get_field_JS(modifier, "scale_x", &scale_x);
    modifier_get_field_JS(modifier, "scale_y", &scale_y);
    modifier_get_field_JS(modifier, "scale_direction_x", &scale_direction_x);
    modifier_get_field_JS(modifier, "scale_direction_y", &scale_direction_y);
    modifier_get_field_JS(modifier, "rotate_pivot_enabled", &rotate_pivot_enabled);
    modifier_get_field_JS(modifier, "rotate_pivot_u", &rotate_pivot_u);
    modifier_get_field_JS(modifier, "rotate_pivot_v", &rotate_pivot_v);
    modifier_get_field_JS(modifier, "translate_rotation", &translate_rotation);
    modifier_get_field_JS(modifier, "scale_size", &scale_size);
    modifier_get_field_JS(modifier, "scale_translation", &scale_translation);
    modifier_get_field_JS(modifier, "x", &x);
    modifier_get_field_JS(modifier, "y", &y);
    modifier_get_field_JS(modifier, "width", &width);
    modifier_get_field_JS(modifier, "heigh", &height);

    lua_pushfstring(
        L,
        TOSTRING_FORMAT,
        (lua_Number)translate_x,
        (lua_Number)translate_y,
        (lua_Number)rotate,
        (lua_Number)skew_x,
        (lua_Number)skew_y,
        (lua_Number)scale_x,
        (lua_Number)scale_y,
        (lua_Number)scale_direction_x,
        (lua_Number)scale_direction_y,
        rotate_pivot_enabled,
        (lua_Number)rotate_pivot_u,
        (lua_Number)rotate_pivot_v,
        translate_rotation,
        scale_size,
        scale_translation,
        (lua_Number)x,
        (lua_Number)y,
        (lua_Number)width,
        (lua_Number)height
    );
#else
    lua_pushfstring(
        L,
        TOSTRING_FORMAT,
        (double)modifier->translate_x,
        (double)modifier->translate_y,
        (double)modifier->rotate,
        (double)modifier->skew_x,
        (double)modifier->skew_y,
        (double)modifier->scale_x,
        (double)modifier->scale_y,
        (double)modifier->scale_direction_x,
        (double)modifier->scale_direction_y,
        isfinite(modifier->rotate_pivot_enabled) && modifier->rotate_pivot_enabled >= 0.0f,
        (double)modifier->rotate_pivot_u,
        (double)modifier->rotate_pivot_v,
        isfinite(modifier->translate_rotation) && modifier->translate_rotation >= 1.0f,
        isfinite(modifier->scale_size) && modifier->scale_size >= 1.0f,
        isfinite(modifier->scale_translation) && modifier->scale_translation >= 1.0f,
        (double)modifier->x,
        (double)modifier->y,
        (double)modifier->width,
        (double)modifier->height
    );
#endif

    return 1;
}

static int script_modifier_index(lua_State* L) {
    READ_USERDATA(L, Modifier, modifier, MODIFIER);

    const char* field = luaL_optstring(L, 2, NULL);

#ifdef JAVASCRIPT
    field = desugar_modifier_field_nameJS(field);
    if (!field) {
        lua_pushnil(L);
        return 1;
    }
    float value;
    if (modifier_get_field_JS(modifier, field, &value))
        lua_pushnil(L);
    else
        lua_pushnumber(L, value);
#else
    if (string_equals(field, "translateX"))
        lua_pushnumber(L, (double)modifier->translate_x);
    else if (string_equals(field, "translateY"))
        lua_pushnumber(L, (double)modifier->translate_y);
    else if (string_equals(field, "rotate"))
        lua_pushnumber(L, (double)modifier->rotate);
    else if (string_equals(field, "skewX"))
        lua_pushnumber(L, (double)modifier->skew_x);
    else if (string_equals(field, "skewY"))
        lua_pushnumber(L, (double)modifier->skew_y);
    else if (string_equals(field, "scaleX"))
        lua_pushnumber(L, (double)modifier->scale_x);
    else if (string_equals(field, "scaleY"))
        lua_pushnumber(L, (double)modifier->scale_y);
    else if (string_equals(field, "scaleDirectionX"))
        lua_pushnumber(L, (double)modifier->scale_direction_x);
    else if (string_equals(field, "scaleDirectionY"))
        lua_pushnumber(L, (double)modifier->scale_direction_y);
    else if (string_equals(field, "rotatePivotEnabled"))
        lua_pushboolean(L, isfinite(modifier->rotate_pivot_enabled) && modifier->rotate_pivot_enabled != 0.0f);
    else if (string_equals(field, "rotatePivotU"))
        lua_pushnumber(L, (double)modifier->rotate_pivot_u);
    else if (string_equals(field, "rotatePivotV"))
        lua_pushnumber(L, (double)modifier->rotate_pivot_v);
    else if (string_equals(field, "translateRotation"))
        lua_pushboolean(L, isfinite(modifier->translate_rotation) && modifier->translate_rotation != 0.0f);
    else if (string_equals(field, "scaleSize"))
        lua_pushboolean(L, isfinite(modifier->scale_size) && modifier->scale_size != 0.0f);
    else if (string_equals(field, "scaleTranslation"))
        lua_pushboolean(L, isfinite(modifier->scale_translation) && modifier->scale_translation != 0.0f);
    else if (string_equals(field, "x"))
        lua_pushnumber(L, (double)modifier->x);
    else if (string_equals(field, "y"))
        lua_pushnumber(L, (double)modifier->y);
    else if (string_equals(field, "width"))
        lua_pushnumber(L, (double)modifier->width);
    else if (string_equals(field, "height"))
        lua_pushnumber(L, (double)modifier->height);
    else
        lua_pushnil(L);
#endif

    return 1;
}

static int script_modifier_newindex(lua_State* L) {
    READ_USERDATA(L, Modifier, modifier, MODIFIER);

    const char* field = luaL_optstring(L, 2, NULL);

#ifdef JAVASCRIPT
    field = desugar_modifier_field_nameJS(field);
    if (!field) {
        return luaL_argerror(L, 2, lua_pushfstring(L, "Unknown Modifier field '%s'", field));
    }
    float value = luaL_checkfloat(L, 3);
    if (modifier_set_field_JS(modifier, field, value))
        return luaL_argerror(L, 2, lua_pushfstring(L, "Unknown Modifier field '%s'", field));
#else
    if (string_equals(field, "translateX"))
        modifier->translate_x = luaL_checkfloat(L, 3);
    else if (string_equals(field, "translateY"))
        modifier->translate_y = luaL_checkfloat(L, 3);
    else if (string_equals(field, "rotate"))
        modifier->rotate = luaL_checkfloat(L, 3);
    else if (string_equals(field, "skewX"))
        modifier->skew_x = luaL_checkfloat(L, 3);
    else if (string_equals(field, "skewY"))
        modifier->skew_y = luaL_checkfloat(L, 3);
    else if (string_equals(field, "scaleX"))
        modifier->scale_x = luaL_checkfloat(L, 3);
    else if (string_equals(field, "scaleY"))
        modifier->scale_y = luaL_checkfloat(L, 3);
    else if (string_equals(field, "scaleDirectionX"))
        modifier->scale_direction_x = luaL_checkfloat(L, 3);
    else if (string_equals(field, "scaleDirectionY"))
        modifier->scale_direction_y = luaL_checkfloat(L, 3);
    else if (string_equals(field, "rotatePivotEnabled"))
        modifier->rotate_pivot_enabled = luaL_checkboolean(L, 3);
    else if (string_equals(field, "rotatePivotU"))
        modifier->rotate_pivot_u = luaL_checkfloat(L, 3);
    else if (string_equals(field, "rotatePivotV"))
        modifier->rotate_pivot_v = luaL_checkfloat(L, 3);
    else if (string_equals(field, "translateRotation"))
        modifier->translate_rotation = luaL_checkboolean(L, 3);
    else if (string_equals(field, "scaleSize"))
        modifier->scale_size = luaL_checkboolean(L, 3);
    else if (string_equals(field, "scaleTranslation"))
        modifier->scale_translation = luaL_checkboolean(L, 3);
    else if (string_equals(field, "x"))
        modifier->x = luaL_checkfloat(L, 3);
    else if (string_equals(field, "y"))
        modifier->y = luaL_checkfloat(L, 3);
    else if (string_equals(field, "width"))
        modifier->width = luaL_checkfloat(L, 3);
    else if (string_equals(field, "height"))
        modifier->height = luaL_checkfloat(L, 3);
    else
        return luaL_argerror(L, 2, lua_pushfstring(L, "unknown Modifier field '%s'", field));
#endif

    return 0;
}


inline void register_modifier(lua_State* lua) {
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
