"use strict";

const LAYOUTPLACEHOLDER = "LayoutPlaceholder";

const LAYOUTPLACEHOLDER_INTERNAL_TOSTRING = "{ " +
    "groupId: $i, " +
    "name: $s, " +
    "alignVertical: $s, " +
    "alignHorizontal: $s, " +
    "x: $6f, " +
    "y: $6f, " +
    "z: $6f, " +
    "height: $6f, " +
    "width: $6f, " +
    "parallaxX: $6f, " +
    "parallaxY: $6f, " +
    "parallaxZ: $6f, " +
    "staticCamera: $b " +
    "}";


function script_layoutplaceholder_index(L) {
    let layoutplaceholder = luascript_read_userdata(L, LAYOUTPLACEHOLDER);
    let field = LUA.luaL_optstring(L, 2, null);

    switch (field) {
        case "groupId":
            LUA.lua_pushinteger(L, layoutplaceholder.group_id);
            break;
        case "name":
            LUA.lua_pushstring(L, layoutplaceholder.name);
            break;
        case "alignVertical":
            luascript_helper_pushenum(L, LUASCRIPT_ENUMS_Align, layoutplaceholder.align_vertical);
            break;
        case "alignHorizontal":
            luascript_helper_pushenum(L, LUASCRIPT_ENUMS_Align, layoutplaceholder.align_horizontal);
            break;
        case "x":
            LUA.lua_pushnumber(L, layoutplaceholder.x);
            break;
        case "y":
            LUA.lua_pushnumber(L, layoutplaceholder.y);
            break;
        case "z":
            LUA.lua_pushnumber(L, layoutplaceholder.z);
            break;
        case "height":
            LUA.lua_pushnumber(L, layoutplaceholder.height);
            break;
        case "width":
            LUA.lua_pushnumber(L, layoutplaceholder.width);
            break;
        case "parallaxX":
            LUA.lua_pushnumber(L, layoutplaceholder.parallax.x);
            break;
        case "parallaxY":
            LUA.lua_pushnumber(L, layoutplaceholder.parallax.y);
            break;
        case "parallaxZ":
            LUA.lua_pushnumber(L, layoutplaceholder.parallax.z);
            break;
        case "staticCamera":
            LUA.lua_pushboolean(L, layoutplaceholder.static_camera);
            break;
        default:
            layoutplaceholder = null;
            break;
    }

    if (!layoutplaceholder) {
        return LUA.luaL_error(L, `unknown LayoutPlaceholder field '${field}'`);
    }

    return 1;
}

function script_layoutplaceholder_newindex(L) {
    let layoutplaceholder = luascript_read_userdata(L, LAYOUTPLACEHOLDER);
    let field = LUA.luaL_optstring(L, 2, null);

    switch (field) {
        case "groupId":
            layoutplaceholder.group_id = LUA.luaL_checkinteger(L, 3);
            break;
        case "name":
            return LUA.luaL_error(L, "the field 'name' of LayoutPlaceholder is read-only");
            break;
        case "alignVertical":
            layoutplaceholder.align_vertical = luascript_helper_optenum(L, 3, LUASCRIPT_ENUMS_Align);
            break;
        case "alignHorizontal":
            layoutplaceholder.align_horizontal = luascript_helper_optenum(L, 3, LUASCRIPT_ENUMS_Align);
            break;
        case "x":
            layoutplaceholder.x = LUA.luaL_checknumber(L, 3);
            break;
        case "y":
            layoutplaceholder.y = LUA.luaL_checknumber(L, 3);
            break;
        case "z":
            layoutplaceholder.z = LUA.luaL_checknumber(L, 3);
            break;
        case "height":
            layoutplaceholder.height = LUA.luaL_checknumber(L, 3);
            break;
        case "width":
            layoutplaceholder.width = LUA.luaL_checknumber(L, 3);
            break;
        case "parallaxX":
            layoutplaceholder.parallax.x = LUA.luaL_checknumber(L, 3);
            break;
        case "parallaxY":
            layoutplaceholder.parallax.y = LUA.luaL_checknumber(L, 3);
            break;
        case "parallaxZ":
            layoutplaceholder.parallax.z = LUA.luaL_checknumber(L, 3);
            break;
        case "staticCamera":
            layoutplaceholder.static_camera = LUA.lua_toboolean(L, 3);
            break;
        default:
            return LUA.luaL_error(L, `unknown LayoutPlaceholder field '${field}'`);
    }

    return 0;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

function script_layoutplaceholder_new(L, layoutplaceholder) {
    return luascript_userdata_new(L, LAYOUTPLACEHOLDER, layoutplaceholder);
}

function script_layoutplaceholder_gc(L) {
    return luascript_userdata_gc(L, LAYOUTPLACEHOLDER);
}

function script_layoutplaceholder_tostring(L) {
    let layoutplaceholder = luascript_read_userdata(L, LAYOUTPLACEHOLDER);

    let str = stringbuilder_helper_create_formatted_string(
        LAYOUTPLACEHOLDER_INTERNAL_TOSTRING,
        layoutplaceholder.group_id,
        layoutplaceholder.name,
        luascript_helper_enums_stringify(LUASCRIPT_ENUMS_Align, layoutplaceholder.align_vertical),
        luascript_helper_enums_stringify(LUASCRIPT_ENUMS_Align, layoutplaceholder.align_horizontal),
        layoutplaceholder.x,
        layoutplaceholder.y,
        layoutplaceholder.z,
        layoutplaceholder.height,
        layoutplaceholder.width,
        layoutplaceholder.parallax.x,
        layoutplaceholder.parallax.y,
        layoutplaceholder.parallax.z,
        layoutplaceholder.static_camera
    );

    LUA.lua_pushstring(L, str);
    str = undefined;

    return 1;
}

function script_layoutplaceholder_register(L) {
    LUA.luaL_newmetatable(L, LAYOUTPLACEHOLDER);

    LUA.lua_pushcfunction(L, script_layoutplaceholder_gc);
    LUA.lua_setfield(L, -2, "__gc");

    LUA.lua_pushcfunction(L, script_layoutplaceholder_tostring);
    LUA.lua_setfield(L, -2, "__tostring");

    LUA.lua_pushcfunction(L, script_layoutplaceholder_index);
    LUA.lua_setfield(L, -2, "__index");

    LUA.lua_pushcfunction(L, script_layoutplaceholder_newindex);
    LUA.lua_setfield(L, -2, "__newindex");
}
