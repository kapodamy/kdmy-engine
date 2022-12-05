using Engine.Animation;
using Engine.Externals.LuaInterop;
using Engine.Game;
using Engine.Platform;
using Engine.Utils;

namespace Engine.Externals.LuaScriptInterop {

    public static class LuascriptHelpers {

        public static Align ParseAlign(LuaState L, string align) {
            switch (align) {
                case null:
                case "":
                case "NONE":
                    return Align.NONE;
                case "START":
                    return Align.START;
                case "CENTER":
                    return Align.CENTER;
                case "END":
                    return Align.END;
                case "BOTH":
                    return Align.BOTH;
            }

            L.luaL_error($"invalid align: {align}");
            return Align.INVALID;
        }

        public static PVRContextFlag ParsePVRFLAG(LuaState L, string pvrflag) {
            switch (pvrflag) {
                case null:
                case "":
                case "DEFAULT":
                    return PVRContextFlag.DEFAULT;
                case "ENABLE":
                    return PVRContextFlag.ENABLE;
                case "DISABLE":
                    return PVRContextFlag.DISABLE;
            }

            L.luaL_error($"invalid pvrflag: {pvrflag}");
            return PVRContextFlag.INVALID_VALUE;
        }

        public static int ParseForcecase(LuaState L, string forcecase) {
            switch (forcecase) {
                case null:
                case "":
                case "none":
                    return VertexProps.TEXTSPRITE_FORCE_NONE;
                case "upper":
                    return VertexProps.TEXTSPRITE_FORCE_UPPERCASE;
                case "lower":
                    return VertexProps.TEXTSPRITE_FORCE_LOWERCASE;
            }

            L.luaL_error($"invalid forcecase: {forcecase}");
            return -1;
        }

        public static int ParseWordbreak(LuaState L, string wordbreak) {
            switch (wordbreak) {
                case null:
                case "":
                case "none":
                    return VertexProps.FONT_WORDBREAK_NONE;
                case "loose":
                    return VertexProps.FONT_WORDBREAK_LOOSE;
                case "break":
                    return VertexProps.FONT_WORDBREAK_BREAK;
            }

            L.luaL_error($"invalid wordbreak: {wordbreak}");
            return -1;
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
            }

            L.luaL_error($"invalid interpolator: {interpolator}");
            return AnimInterpolator.LINEAR;
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

    }

}

