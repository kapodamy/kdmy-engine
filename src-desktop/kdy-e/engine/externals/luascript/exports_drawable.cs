using Engine.Externals.LuaInterop;
using Engine.Platform;
using Engine.Utils;

namespace Engine.Externals.LuaScriptInterop;

public static class ExportsDrawable {

    public const string DRAWABLE = "Drawable";


    static int script_drawable_set_z_index(LuaState L) {
        Drawable drawable = L.ReadUserdata<Drawable>(DRAWABLE);
        float z_index = (float)L.luaL_checknumber(2);

        drawable.SetZIndex(z_index);

        return 0;
    }

    static int script_drawable_get_z_index(LuaState L) {
        Drawable drawable = L.ReadUserdata<Drawable>(DRAWABLE);

        float ret = drawable.GetZIndex();

        L.lua_pushnumber(ret);
        return 1;
    }

    static int script_drawable_set_z_offset(LuaState L) {
        Drawable drawable = L.ReadUserdata<Drawable>(DRAWABLE);
        float offset = (float)L.luaL_checknumber(2);

        drawable.SetZOffset(offset);

        return 0;
    }

    static int script_drawable_set_alpha(LuaState L) {
        Drawable drawable = L.ReadUserdata<Drawable>(DRAWABLE);
        float alpha = (float)L.luaL_checknumber(2);

        drawable.SetAlpha(alpha);

        return 0;
    }

    static int script_drawable_get_alpha(LuaState L) {
        Drawable drawable = L.ReadUserdata<Drawable>(DRAWABLE);

        float ret = drawable.GetAlpha();

        L.lua_pushnumber(ret);
        return 1;
    }

    static int script_drawable_set_offsetcolor(LuaState L) {
        Drawable drawable = L.ReadUserdata<Drawable>(DRAWABLE);
        float r = (float)L.luaL_checknumber(2);
        float g = (float)L.luaL_checknumber(3);
        float b = (float)L.luaL_checknumber(4);
        float a = (float)L.luaL_checknumber(5);

        drawable.SetOffsetColor(r, g, b, a);

        return 0;
    }

    static int script_drawable_set_offsetcolor_to_default(LuaState L) {
        Drawable drawable = L.ReadUserdata<Drawable>(DRAWABLE);

        drawable.SetOffsetColorToDefault();

        return 0;
    }

    static int script_drawable_get_modifier(LuaState L) {
        Drawable drawable = L.ReadUserdata<Drawable>(DRAWABLE);

        Modifier ret = drawable.GetModifier();

        return ExportsModifier.script_modifier_new(L, ret);
    }

    static int script_drawable_set_antialiasing(LuaState L) {
        Drawable drawable = L.ReadUserdata<Drawable>(DRAWABLE);
        PVRContextFlag antialiasing = LuascriptHelpers.ParsePVRFLAG(L, L.luaL_optstring(2, null));

        drawable.SetAntialiasing(antialiasing);

        return 0;
    }

    static int script_drawable_set_shader(LuaState L) {
        Drawable drawable = L.ReadUserdata<Drawable>(DRAWABLE);
        PSShader psshader = L.ReadNullableUserdata<PSShader>(2, ExportsPSShader.PSSHADER);

        drawable.SetShader(psshader);

        return 0;
    }

    static int script_drawable_get_shader(LuaState L) {
        Drawable drawable = L.ReadUserdata<Drawable>(DRAWABLE);

        PSShader ret = drawable.GetShader();

        return ExportsPSShader.script_psshader_new(L, ret);
    }

    static int script_drawable_blend_enable(LuaState L) {
        Drawable drawable = L.ReadUserdata<Drawable>(DRAWABLE);
        bool enabled = L.lua_toboolean(2);

        drawable.BlendEnable(enabled);

        return 0;
    }

    static int script_drawable_blend_set(LuaState L) {
        Drawable drawable = L.ReadUserdata<Drawable>(DRAWABLE);
        Blend src_rgb = LuascriptHelpers.ParseBlend(L, L.luaL_optstring(2, null));
        Blend dst_rgb = LuascriptHelpers.ParseBlend(L, L.luaL_optstring(3, null));
        Blend src_alpha = LuascriptHelpers.ParseBlend(L, L.luaL_optstring(4, null));
        Blend dst_alpha = LuascriptHelpers.ParseBlend(L, L.luaL_optstring(5, null));

        drawable.BlendSet(src_rgb, dst_rgb, src_alpha, dst_alpha);

        return 0;
    }




    static readonly LuaTableFunction[] DRAWABLE_FUNCTIONS = {
        new LuaTableFunction("set_z_index", script_drawable_set_z_index),
        new LuaTableFunction("get_z_index", script_drawable_get_z_index),
        new LuaTableFunction("set_z_offset", script_drawable_set_z_offset),
        new LuaTableFunction("set_alpha", script_drawable_set_alpha),
        new LuaTableFunction("get_alpha", script_drawable_get_alpha),
        new LuaTableFunction("set_offsetcolor", script_drawable_set_offsetcolor),
        new LuaTableFunction("set_offsetcolor_to_default", script_drawable_set_offsetcolor_to_default),
        new LuaTableFunction("get_modifier", script_drawable_get_modifier),
        new LuaTableFunction("set_antialiasing", script_drawable_set_antialiasing),
        new LuaTableFunction("set_shader", script_drawable_set_shader),
        new LuaTableFunction("get_shader", script_drawable_get_shader),
        new LuaTableFunction("blend_enable", script_drawable_blend_enable),
        new LuaTableFunction("blend_set", script_drawable_blend_set),
        new LuaTableFunction(null, null)
    };

    public static int script_drawable_new(LuaState L, Drawable drawable) {
        return L.CreateUserdata(DRAWABLE, drawable);
    }

    static int script_drawable_gc(LuaState L) {
        return L.GC_userdata(DRAWABLE);
    }

    static int script_drawable_tostring(LuaState L) {
        return L.ToString_userdata(DRAWABLE);
    }

    private static readonly LuaCallback delegate_gc = script_drawable_gc;
    private static readonly LuaCallback delegate_tostring = script_drawable_tostring;

    public static void script_drawable_register(ManagedLuaState L) {
        L.RegisterMetaTable(DRAWABLE, delegate_gc, delegate_tostring, DRAWABLE_FUNCTIONS);
    }

}
