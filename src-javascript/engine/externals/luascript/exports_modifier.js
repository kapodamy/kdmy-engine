"use strict";

const MODIFIER = "Modifier";

// (JS & C# only) internal engine format
const TOSTRING_INTERNAL_FORMAT = "{ " +
    "translateX: $6f, " +
    "translateY: $6f, " +
    "rotate: $6f, " +
    "skewX: $6f, " +
    "skewY: $6f, " +
    "scaleX: $6f, " +
    "scaleY: $6f, " +
    "scaleDirectionX: $6f, " +
    "scaleDirectionY: $6f, " +
    "rotatePivotEnabled: $b, " +
    "rotatePivotU: $6f, " +
    "rotatePivotV: $6f, " +
    "translateRotation: $b, " +
    "scaleSize: $b, " +
    "scaleTranslation: $b, " +
    "x: $6f, " +
    "y: $6f, " +
    "width: $6f, " +
    "height: $6f, " +
    "}";


function script_modifier_new(L, modifier) {
    return luascript_userdata_new(L, MODIFIER, modifier);
}

function script_modifier_gc(L) {
    return luascript_userdata_gc(L, MODIFIER);
}

function script_modifier_tostring(L) {
    let modifier = luascript_read_userdata(L, MODIFIER);

    LUA.lua_pushfstring(
        L,
        stringbuilder_helper_create_formatted_string2(
            TOSTRING_INTERNAL_FORMAT,
            modifier.translate_x,
            modifier.translate_y,
            modifier.rotate,
            modifier.skew_x,
            modifier.skew_y,
            modifier.scale_x,
            modifier.scale_y,
            modifier.scale_direction_x,
            modifier.scale_direction_y,
            modifier.rotate_pivot_enabled >= 1.0,
            modifier.rotate_pivot_u,
            modifier.rotate_pivot_v,
            modifier.translate_rotation >= 1.0,
            modifier.scale_size >= 1.0,
            modifier.scale_translation >= 1.0,
            modifier.x,
            modifier.y,
            modifier.width,
            modifier.height
        )
    );

    return 1;
}

function script_modifier_index(L) {
    let modifier = luascript_read_userdata(L, MODIFIER);
    let field = LUA.luaL_optstring(L, 2, null);

    switch (field) {
        case "translateX":
            LUA.lua_pushnumber(L, modifier.translate_x);
            break;
        case "translateY":
            LUA.lua_pushnumber(L, modifier.translate_y);
            break;
        case "rotate":
            LUA.lua_pushnumber(L, modifier.rotate);
            break;
        case "rotateByDegrees":
            LUA.lua_pushnumber(L, modifier.rotate * MATH2D_DEG_TO_RAD);
            break;
        case "skewX":
            LUA.lua_pushnumber(L, modifier.skew_x);
            break;
        case "skewY":
            LUA.lua_pushnumber(L, modifier.skew_y);
            break;
        case "scaleX":
            LUA.lua_pushnumber(L, modifier.scale_x);
            break;
        case "scaleY":
            LUA.lua_pushnumber(L, modifier.scale_y);
            break;
        case "scaleDirectionX":
            LUA.lua_pushnumber(L, modifier.scale_direction_x);
            break;
        case "scaleDirectionY":
            LUA.lua_pushnumber(L, modifier.scale_direction_y);
            break;
        case "rotatePivotEnabled":
            LUA.lua_pushboolean(L, (modifier.rotate_pivot_enabled == true) || (modifier.rotate_pivot_enabled >= 1.0));
            break;
        case "rotatePivotU":
            LUA.lua_pushnumber(L, modifier.rotate_pivot_u);
            break;
        case "rotatePivotV":
            LUA.lua_pushnumber(L, modifier.rotate_pivot_v);
            break;
        case "translateRotation":
            LUA.lua_pushboolean(L, (modifier.translate_rotation == true) || (modifier.translate_rotation >= 1.0));
            break;
        case "scaleSize":
            LUA.lua_pushboolean(L, modifier.scale_size >= 1.0);
            break;
        case "scaleTranslation":
            LUA.lua_pushboolean(L, (modifier.scale_translation == true) || (modifier.scale_translation >= 1.0));
            break;
        case "x":
            LUA.lua_pushnumber(L, modifier.x);
            break;
        case "y":
            LUA.lua_pushnumber(L, modifier.y);
            break;
        case "width":
            LUA.lua_pushnumber(L, modifier.width);
            break;
        case "height":
            LUA.lua_pushnumber(L, modifier.height);
        default:
            return LUA.luaL_error(L, `unknown modifier field '${field}'`);
    }

    return 1;
}

function script_modifier_newindex(L) {
    let modifier = luascript_read_userdata(L, MODIFIER);
    let field = LUA.luaL_optstring(L, 2, null);

    switch (field) {
        case "translateX":
            modifier.translate_x = LUA.luaL_checknumber(L, 3);
            break;
        case "translateY":
            modifier.translate_y = LUA.luaL_checknumber(L, 3);
            break;
        case "rotateByDegrees":
            modifier.rotate = (LUA.luaL_checknumber(L, 3) * MATH2D_DEG_TO_RAD);
            break;
        case "rotate":
            modifier.rotate = LUA.luaL_checknumber(L, 3);
            break;
        case "skewX":
            modifier.skew_x = LUA.luaL_checknumber(L, 3);
            break;
        case "skewY":
            modifier.skew_y = LUA.luaL_checknumber(L, 3);
            break;
        case "scaleX":
            modifier.scale_x = LUA.luaL_checknumber(L, 3);
            break;
        case "scaleY":
            modifier.scale_y = LUA.luaL_checknumber(L, 3);
            break;
        case "scaleDirectionX":
            modifier.scale_direction_x = LUA.luaL_checknumber(L, 3);
            break;
        case "scaleDirectionY":
            modifier.scale_direction_y = LUA.luaL_checknumber(L, 3);
            break;
        case "rotatePivotEnabled":
            modifier.rotate_pivot_enabled = LUA.lua_toboolean(L, 3) ? 1.0 : 0.0;
            break;
        case "rotatePivotU":
            modifier.rotate_pivot_u = LUA.luaL_checknumber(L, 3);
            break;
        case "rotatePivotV":
            modifier.rotate_pivot_v = LUA.luaL_checknumber(L, 3);
            break;
        case "translateRotation":
            modifier.translate_rotation = LUA.lua_toboolean(L, 3) ? 1.0 : 0.0;
            break;
        case "scaleSize":
            modifier.scale_size = LUA.lua_toboolean(L, 3) ? 1.0 : 0.0;
            break;
        case "scaleTranslation":
            modifier.scale_translation = LUA.lua_toboolean(L, 3) ? 1.0 : 0.0;
            break;
        case "x":
            modifier.x = LUA.luaL_checknumber(L, 3);
            break;
        case "y":
            modifier.y = LUA.luaL_checknumber(L, 3);
            break;
        case "width":
            modifier.width = LUA.luaL_checknumber(L, 3);
            break;
        case "height":
            modifier.height = LUA.luaL_checknumber(L, 3);
        default:
            return LUA.luaL_error(L, `unknown modifier field '${field}'`);
    }

    return 0;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

function script_modifier_register(lua) {
    LUA.luaL_newmetatable(lua, MODIFIER);

    LUA.lua_pushcfunction(lua, script_modifier_gc);
    LUA.lua_setfield(lua, -2, "__gc");

    LUA.lua_pushcfunction(lua, script_modifier_tostring);
    LUA.lua_setfield(lua, -2, "__tostring");

    LUA.lua_pushcfunction(lua, script_modifier_index);
    LUA.lua_setfield(lua, -2, "__index");

    LUA.lua_pushcfunction(lua, script_modifier_newindex);
    LUA.lua_setfield(lua, -2, "__newindex");

    LUA.lua_pop(lua, 1);
}
