using System;
using Engine.Animation;
using Engine.Externals.LuaInterop;
using Engine.Game;
using Engine.Platform;
using Engine.Utils;
using static Engine.Game.Strum;

namespace Engine.Externals.LuaScriptInterop;

public static class LuascriptHelpers {

    public static Align ParseAlign(LuaState L, string align) {
        Align value = VertexProps.ParseAlign2(align);

        if (value == Align.INVALID)
            L.luaL_error($"invalid align: {align}");

        return value;
    }

    public static PVRContextFlag ParsePVRFLAG(LuaState L, string pvrflag) {
        PVRContextFlag value = VertexProps.ParseFlag2(pvrflag, PVRContextFlag.INVALID_VALUE);

        if (value == PVRContextFlag.INVALID_VALUE)
            L.luaL_error($"invalid pvrflag: {pvrflag}");

        return value;
    }

    public static int ParseForcecase(LuaState L, string forcecase) {
        int value = VertexProps.ParseTextSpriteForceCase2(forcecase);

        if (value < 0)
            L.luaL_error($"invalid forcecase: {forcecase}");

        return value;
    }

    public static int ParseWordbreak(LuaState L, string wordbreak) {
        int value = VertexProps.ParseWordbreak2(wordbreak);

        if (value < 0)
            L.luaL_error($"invalid wordbreak: {wordbreak}");

        return value;
    }

    public static AnimInterpolator ParseInterpolator(LuaState L, string interpolator) {
        switch (interpolator) {
            case "ease":
                return AnimInterpolator.EASE;
            case "ease-in":
                return AnimInterpolator.EASE_IN;
            case "ease-out":
                return AnimInterpolator.EASE_OUT;
            case "ease-in-out":
                return AnimInterpolator.EASE_IN_OUT;
            case "linear":
                return AnimInterpolator.LINEAR;
            case "steps":
                return AnimInterpolator.STEPS;
            case "cubic":
                return AnimInterpolator.CUBIC;
            case "quad":
                return AnimInterpolator.QUAD;
            case "expo":
                return AnimInterpolator.EXPO;
            case "sin":
                return AnimInterpolator.SIN;
        }

        L.luaL_error($"invalid interpolator: {interpolator}");
        return AnimInterpolator.LINEAR;
    }

    public static Blend ParseBlend(LuaState L, string blend) {
        Blend value = VertexProps.ParseBlending(blend);

        if (value == Blend.INVALID_VALUE)
            L.luaL_error($"invalid blend: {blend}");

        return value;
    }

    public static StrumScriptTarget luascript_parse_strumscripttarget(LuaState L, string strumscripttarget) {
        switch (strumscripttarget) {
            case "marker":
                return StrumScriptTarget.MARKER;
            case "sick_effect":
                return StrumScriptTarget.SICK_EFFECT;
            case "background":
                return StrumScriptTarget.BACKGROUND;
            case "strum_line":
                return StrumScriptTarget.STRUM_LINE;
            case "note":
                return StrumScriptTarget.NOTE;
            case "all":
                return StrumScriptTarget.ALL;
        }

        L.luaL_error($"invalid strumscripttarget: {strumscripttarget}");
        return StrumScriptTarget.ALL;
    }

    public static StrumScriptOn luascript_parse_strumscripton(LuaState L, string strumscripton) {
        switch (strumscripton) {
            case "hit_down":
                return StrumScriptOn.HIT_DOWN;
            case "hit_up":
                return StrumScriptOn.HIT_UP;
            case "miss":
                return StrumScriptOn.MISS;
            case "penality":
                return StrumScriptOn.PENALITY;
            case "idle":
                return StrumScriptOn.IDLE;
            case "all":
                return StrumScriptOn.ALL;
        }

        L.luaL_error($"invalid strumscripton: {strumscripton}");
        return StrumScriptOn.ALL;
    }

    public static ScrollDirection luascript_parse_scrolldirection(LuaState L, string scrolldirection) {
        switch (scrolldirection) {
            case "UPSCROLL":
                return ScrollDirection.UPSCROLL;
            case "LEFTSCROLL":
                return ScrollDirection.LEFTSCROLL;
            case "DOWNSCROLL":
                return ScrollDirection.DOWNSCROLL;
            case "RIGHTSCROLL":
                return ScrollDirection.RIGHTSCROLL;
        }

        L.luaL_error($"invalid scrolldirection: {scrolldirection}");
        return ScrollDirection.LEFTSCROLL;
    }


    public static string StringifyAlign(Align align) {
        switch (align) {
            case Align.START:
                return "START";
            case Align.END:
                return "END";
            case Align.CENTER:
                return "CENTER";
            case Align.BOTH:
                return "BOTH";
            case Align.NONE:
            default:
                return "NONE";
        }
    }

    public static string StringifyActiontype(CharacterActionType actiontype) {
        switch (actiontype) {
            case CharacterActionType.MISS:
                return "miss";
            case CharacterActionType.EXTRA:
                return "extra";
            case CharacterActionType.IDLE:
                return "idle";
            case CharacterActionType.SING:
                return "sing";
            default:
                return "none";
        }
    }

    public static object ParseModdingValue(LuaState L, int idx) {
        object value = null;
        switch (L.lua_type(-1)) {
            case LuaType.TBOOLEAN:
                value = L.lua_toboolean(-1);
                break;
            case LuaType.TNUMBER:
                value = L.lua_tonumber(-1);
                break;
            case LuaType.TSTRING:
                value = L.lua_tostring(-1);
                break;
            case LuaType.TNIL:
            case LuaType.TNONE:
                break;
            default:
                Console.Error.WriteLine("[ERROR] luascripthelpers_to_basic_value() invalid value at idx " + idx);
                break;
        }

        return value;
    }

    public static void PushModdingValue(LuaState L, object value) {
        if (value is string)
            L.lua_pushstring((string)value);
        else if (value is long)
            L.lua_pushnumber((long)value);
        else if (value is double)
            L.lua_pushnumber((double)value);
        else if (value is bool)
            L.lua_pushboolean((bool)value);
        else
            L.lua_pushnil();
    }

}

