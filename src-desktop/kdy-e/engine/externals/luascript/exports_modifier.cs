using System;
using Engine.Externals.LuaInterop;
using Engine.Utils;

namespace Engine.Externals.LuaScriptInterop;

internal static class ExportsModifier {

    private const string MODIFIER = "Modifier";
    
    private const string TOSTRING_INTERNAL_FORMAT = "{ " +
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
    private const double DEG_TO_RAD = Math.PI / 180.0;
    private const double RAD_TO_DEG = 180.0 / Math.PI;


    internal static int script_modifier_new(LuaState L, Modifier modifier) {
        return L.CreateUserdata(MODIFIER, modifier);
    }

    static int script_modifier_gc(LuaState L) {
        return L.GC_userdata(MODIFIER);
    }

    static int script_modifier_tostring(LuaState L) {
        Modifier modifier = L.ReadUserdata<Modifier>(MODIFIER);

        string str = StringUtils.CreateFormattedString(
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
            modifier.rotate_pivot_enabled == true,
            modifier.rotate_pivot_u,
            modifier.rotate_pivot_v,
            modifier.translate_rotation == true,
            modifier.scale_size == true,
            modifier.scale_translation == true,
            modifier.x,
            modifier.y,
            modifier.width,
            modifier.height
        );

        L.lua_pushstring(str);
        //free(str);

        return 1;
    }

    static int script_modifier_index(LuaState L) {
        Modifier modifier = L.ReadUserdata<Modifier>(MODIFIER);
        string field = L.luaL_optstring(2, null);

        switch (field) {
            case "translateX":
                L.lua_pushnumber(modifier.translate_x);
                break;
            case "translateY":
                L.lua_pushnumber(modifier.translate_y);
                break;
            case "rotate":
                L.lua_pushnumber(modifier.rotate);
                break;
            case "rotateByDegrees":
                L.lua_pushnumber(modifier.rotate / RAD_TO_DEG);
                break;
            case "skewX":
                L.lua_pushnumber(modifier.skew_x);
                break;
            case "skewY":
                L.lua_pushnumber(modifier.skew_y);
                break;
            case "scaleX":
                L.lua_pushnumber(modifier.scale_x);
                break;
            case "scaleY":
                L.lua_pushnumber(modifier.scale_y);
                break;
            case "scaleDirectionX":
                L.lua_pushnumber(modifier.scale_direction_x);
                break;
            case "scaleDirectionY":
                L.lua_pushnumber(modifier.scale_direction_y);
                break;
            case "rotatePivotEnabled":
                L.lua_pushboolean(modifier.rotate_pivot_enabled == true);
                break;
            case "rotatePivotU":
                L.lua_pushnumber(modifier.rotate_pivot_u);
                break;
            case "rotatePivotV":
                L.lua_pushnumber(modifier.rotate_pivot_v);
                break;
            case "translateRotation":
                L.lua_pushboolean(modifier.translate_rotation == true);
                break;
            case "scaleSize":
                L.lua_pushboolean(modifier.scale_size == true);
                break;
            case "scaleTranslation":
                L.lua_pushboolean(modifier.scale_translation == true);
                break;
            case "x":
                L.lua_pushnumber(modifier.x);
                break;
            case "y":
                L.lua_pushnumber(modifier.y);
                break;
            case "width":
                L.lua_pushnumber(modifier.width);
                break;
            case "height":
                L.lua_pushnumber(modifier.height);
                break;
            default:
                return L.luaL_error($"unknown modifier field '{field}'");
        }

        return 1;
    }

    static int script_modifier_newindex(LuaState L) {
        Modifier modifier = L.ReadUserdata<Modifier>(MODIFIER);
        string field = L.luaL_optstring(2, null);

        switch (field) {
            case "translateX":
                modifier.translate_x = (float)L.luaL_checknumber(3);
                break;
            case "translateY":
                modifier.translate_y = (float)L.luaL_checknumber(3);
                break;
            case "rotate":
                modifier.rotate = (float)L.luaL_checknumber(3);
                break;
            case "rotateByDegrees":
                modifier.rotate = (float)(L.luaL_checknumber(3) * DEG_TO_RAD);
                break;
            case "skewX":
                modifier.skew_x = (float)L.luaL_checknumber(3);
                break;
            case "skewY":
                modifier.skew_y = (float)L.luaL_checknumber(3);
                break;
            case "scaleX":
                modifier.scale_x = (float)L.luaL_checknumber(3);
                break;
            case "scaleY":
                modifier.scale_y = (float)L.luaL_checknumber(3);
                break;
            case "scaleDirectionX":
                modifier.scale_direction_x = (float)L.luaL_checknumber(3);
                break;
            case "scaleDirectionY":
                modifier.scale_direction_y = (float)L.luaL_checknumber(3);
                break;
            case "rotatePivotEnabled":
                modifier.rotate_pivot_enabled = L.lua_toboolean(3);
                break;
            case "rotatePivotU":
                modifier.rotate_pivot_u = (float)L.luaL_checknumber(3);
                break;
            case "rotatePivotV":
                modifier.rotate_pivot_v = (float)L.luaL_checknumber(3);
                break;
            case "translateRotation":
                modifier.translate_rotation = L.lua_toboolean(3);
                break;
            case "scaleSize":
                modifier.scale_size = L.lua_toboolean(3);
                break;
            case "scaleTranslation":
                modifier.scale_translation = L.lua_toboolean(3);
                break;
            case "x":
                modifier.x = (float)L.luaL_checknumber(3);
                break;
            case "y":
                modifier.y = (float)L.luaL_checknumber(3);
                break;
            case "width":
                modifier.width = (float)L.luaL_checknumber(3);
                break;
            case "height":
                modifier.height = (float)L.luaL_checknumber(3);
                break;
            default:
                return L.luaL_error($"unknown modifier field '{field}'");
        }

        return 0;
    }


    private static readonly LuaCallback delegate_gc = script_modifier_gc;
    private static readonly LuaCallback delegate_tostring = script_modifier_tostring;
    private static readonly LuaCallback delegate_index = script_modifier_index;
    private static readonly LuaCallback delegate_newindex = script_modifier_newindex;

    internal static void script_modifier_register(ManagedLuaState lua) {
        lua.RegisterStructMetaTable(
            MODIFIER,
            delegate_gc,
            delegate_tostring,
            delegate_index,
            delegate_newindex
        );
    }

}
