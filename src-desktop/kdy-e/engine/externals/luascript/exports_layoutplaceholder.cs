using Engine.Externals.LuaInterop;
using Engine.Utils;

namespace Engine.Externals.LuaScriptInterop;

public static class ExportsLayoutPlaceholder {

    public const string LAYOUTPLACEHOLDER = "LayoutPlaceholder";


    // (JS & C# only) internal engine format
    const string INTERNAL_TOSTRING = "{ " +
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


    static int script_layoutplaceholder_index(LuaState L) {
        LayoutPlaceholder layoutplaceholder = L.ReadUserdata<LayoutPlaceholder>(LAYOUTPLACEHOLDER); ;
        string field = L.luaL_optstring(2, null);

        switch (field) {
            case "groupId":
                L.lua_pushinteger(layoutplaceholder.group_id);
                break;
            case "name":
                L.lua_pushstring(layoutplaceholder.name);
                break;
            case "alignVertical":
                LuascriptHelpers.pushenum(L, LuascriptEnums.Align, (int)layoutplaceholder.align_vertical);
                break;
            case "alignHorizontal":
                LuascriptHelpers.pushenum(L, LuascriptEnums.Align, (int)layoutplaceholder.align_horizontal);
                break;
            case "x":
                L.lua_pushnumber(layoutplaceholder.x);
                break;
            case "y":
                L.lua_pushnumber(layoutplaceholder.y);
                break;
            case "z":
                L.lua_pushnumber(layoutplaceholder.z);
                break;
            case "height":
                L.lua_pushnumber(layoutplaceholder.height);
                break;
            case "width":
                L.lua_pushnumber(layoutplaceholder.width);
                break;
            case "parallaxX":
                L.lua_pushnumber(layoutplaceholder.parallax.x);
                break;
            case "parallaxY":
                L.lua_pushnumber(layoutplaceholder.parallax.y);
                break;
            case "parallaxZ":
                L.lua_pushnumber(layoutplaceholder.parallax.z);
                break;
            case "staticCamera":
                L.lua_pushboolean(layoutplaceholder.static_camera);
                break;
            default:
                layoutplaceholder = null;
                break;
        }

        if (layoutplaceholder == null) {
            return L.luaL_error($"unknown LayoutPlaceholder field '{field}'");
        }

        return 1;
    }

    static int script_layoutplaceholder_newindex(LuaState L) {
        LayoutPlaceholder layoutplaceholder = L.ReadUserdata<LayoutPlaceholder>(LAYOUTPLACEHOLDER); ;
        string field = L.luaL_optstring(2, null);

        switch (field) {
            case "groupId":
                layoutplaceholder.group_id = (int)L.luaL_checkinteger(3);
                break;
            case "name":
                return L.luaL_error("the field 'name' of LayoutPlaceholder is read-only");
            case "alignVertical":
                layoutplaceholder.align_vertical = (Align)LuascriptHelpers.optenum(L, 3, LuascriptEnums.Align);
                break;
            case "alignHorizontal":
                layoutplaceholder.align_horizontal = (Align)LuascriptHelpers.optenum(L, 3, LuascriptEnums.Align);
                break;
            case "x":
                layoutplaceholder.x = (float)L.luaL_checknumber(3);
                break;
            case "y":
                layoutplaceholder.y = (float)L.luaL_checknumber(3);
                break;
            case "z":
                layoutplaceholder.z = (float)L.luaL_checknumber(3);
                break;
            case "height":
                layoutplaceholder.height = (float)L.luaL_checknumber(3);
                break;
            case "width":
                layoutplaceholder.width = (float)L.luaL_checknumber(3);
                break;
            case "parallaxX":
                layoutplaceholder.parallax.x = (float)L.luaL_checknumber(3);
                break;
            case "parallaxY":
                layoutplaceholder.parallax.y = (float)L.luaL_checknumber(3);
                break;
            case "parallaxZ":
                layoutplaceholder.parallax.z = (float)L.luaL_checknumber(3);
                break;
            case "staticCamera":
                layoutplaceholder.static_camera = L.lua_toboolean(3);
                break;
            default:
                layoutplaceholder = null;
                break;
        }


        if (layoutplaceholder == null) {
            return L.luaL_error($"unknown LayoutPlaceholder field '{field}'");
        }

        return 0;
    }

    public static int script_layoutplaceholder_new(LuaState L, LayoutPlaceholder layoutplaceholder) {
        return L.CreateUserdata(LAYOUTPLACEHOLDER, layoutplaceholder);
    }

    static int script_layoutplaceholder_gc(LuaState L) {
        return L.GC_userdata(LAYOUTPLACEHOLDER);
    }

    static int script_layoutplaceholder_tostring(LuaState L) {
        LayoutPlaceholder layoutplaceholder = L.ReadUserdata<LayoutPlaceholder>(LAYOUTPLACEHOLDER);

        L.lua_pushstring(StringUtils.CreateFormattedString(INTERNAL_TOSTRING,
            layoutplaceholder.group_id,
            layoutplaceholder.name,
            LuascriptHelpers.EnumsStringify(LuascriptEnums.Align, (int)layoutplaceholder.align_vertical),
            LuascriptHelpers.EnumsStringify(LuascriptEnums.Align, (int)layoutplaceholder.align_horizontal),
            layoutplaceholder.x,
            layoutplaceholder.y,
            layoutplaceholder.z,
            layoutplaceholder.height,
            layoutplaceholder.width,
            layoutplaceholder.parallax.x,
            layoutplaceholder.parallax.y,
            layoutplaceholder.parallax.z,
            layoutplaceholder.static_camera
        ));

        return 1;
    }

    private static readonly LuaCallback delegate_gc = script_layoutplaceholder_gc;
    private static readonly LuaCallback delegate_tostring = script_layoutplaceholder_tostring;
    private static readonly LuaCallback delegate_index = script_layoutplaceholder_index;
    private static readonly LuaCallback delegate_newindex = script_layoutplaceholder_newindex;


    public static void script_layoutplaceholder_register(ManagedLuaState L) {
        L.RegisterStructMetaTable(LAYOUTPLACEHOLDER, delegate_gc, delegate_tostring, delegate_index, delegate_newindex);
    }


}
