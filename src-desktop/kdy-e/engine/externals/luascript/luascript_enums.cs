

namespace Engine.Externals.LuaScriptInterop; 

public readonly struct LuascriptEnumsPair {
    public readonly int value;
    public readonly string name;

    public LuascriptEnumsPair(int value, string name) {
        this.value = value;
        this.name = name;
    }
}

public class LuascriptEnums {
    public string enum_name;
    public bool reject_on_null_or_empty;
    public LuascriptEnumsPair[] pairs;

    public static readonly LuascriptEnums PVRFlag = new LuascriptEnums() {
        enum_name = "PVRFlag",
        reject_on_null_or_empty = false,
        pairs = new LuascriptEnumsPair[] {
             new ((int)Engine.Platform.PVRFlag.DEFAULT, "default"),
             new ((int)Engine.Platform.PVRFlag.ENABLE, "enable"),
             new ((int)Engine.Platform.PVRFlag.DISABLE, "disable")
        }
    };

    public static readonly LuascriptEnums TextSpriteForceCase = new LuascriptEnums() {
        enum_name = "TextSpriteForceCase",
        reject_on_null_or_empty = false,
        pairs = new LuascriptEnumsPair[] {
            new(Engine.Utils.VertexProps.TEXTSPRITE_FORCE_NONE, "none"),
            new(Engine.Utils.VertexProps.TEXTSPRITE_FORCE_UPPERCASE, "upper"),
            new(Engine.Utils.VertexProps.TEXTSPRITE_FORCE_LOWERCASE, "lower")
        }
    };

    public static readonly LuascriptEnums FontWordBreak = new LuascriptEnums() {
        enum_name = "FontWordBreak",
        reject_on_null_or_empty = false,
        pairs = new LuascriptEnumsPair[] {
            new (Engine.Utils.VertexProps.FONT_WORDBREAK_NONE, "none"),
            new (Engine.Utils.VertexProps.FONT_WORDBREAK_LOOSE, "loose"),
            new (Engine.Utils.VertexProps.FONT_WORDBREAK_BREAK, "break")
        }
    };

    public static readonly LuascriptEnums AnimInterpolator = new LuascriptEnums() {
        enum_name = "AnimInterpolator",
        reject_on_null_or_empty = true,
        pairs = new LuascriptEnumsPair[] {
            new ((int)Engine.Animation.AnimInterpolator.EASE, "ease"),
            new ((int)Engine.Animation.AnimInterpolator.EASE_IN, "ease-in"),
            new ((int)Engine.Animation.AnimInterpolator.EASE_OUT, "ease-out"),
            new ((int)Engine.Animation.AnimInterpolator.EASE_IN_OUT, "ease-in-out"),
            new ((int)Engine.Animation.AnimInterpolator.LINEAR, "linear"),
            new ((int)Engine.Animation.AnimInterpolator.STEPS, "steps"),
            new ((int)Engine.Animation.AnimInterpolator.CUBIC, "cubic"),
            new ((int)Engine.Animation.AnimInterpolator.QUAD, "quad"),
            new ((int)Engine.Animation.AnimInterpolator.EXPO, "expo"),
            new ((int)Engine.Animation.AnimInterpolator.SIN, "sin")
        }
    };

    public static readonly LuascriptEnums Blend = new LuascriptEnums() {
        enum_name = "Blend",
        reject_on_null_or_empty = false,
        pairs = new LuascriptEnumsPair[] {
            new ((int)Engine.Utils.Blend.DEFAULT, "BLEND_DEFAULT"),
            new ((int)Engine.Utils.Blend.ZERO, "BLEND_ZERO"),
            new ((int)Engine.Utils.Blend.ONE, "BLEND_ONE"),
            new ((int)Engine.Utils.Blend.SRC_COLOR, "BLEND_SRC_COLOR"),
            new ((int)Engine.Utils.Blend.ONE_MINUS_SRC_COLOR, "BLEND_ONE_MINUS_SRC_COLOR"),
            new ((int)Engine.Utils.Blend.DST_COLOR, "BLEND_DST_COLOR"),
            new ((int)Engine.Utils.Blend.ONE_MINUS_DST_COLOR, "BLEND_ONE_MINUS_DST_COLOR"),
            new ((int)Engine.Utils.Blend.SRC_ALPHA, "BLEND_SRC_ALPHA"),
            new ((int)Engine.Utils.Blend.ONE_MINUS_SRC_ALPHA, "BLEND_ONE_MINUS_SRC_ALPHA"),
            new ((int)Engine.Utils.Blend.DST_ALPHA, "BLEND_DST_ALPHA"),
            new ((int)Engine.Utils.Blend.ONE_MINUS_DST_ALPHA, "BLEND_ONE_MINUS_DST_ALPHA"),
            new ((int)Engine.Utils.Blend.CONSTANT_COLOR, "BLEND_CONSTANT_COLOR"),
            new ((int)Engine.Utils.Blend.ONE_MINUS_CONSTANT_COLOR, "BLEND_ONE_MINUS_CONSTANT_COLOR"),
            new ((int)Engine.Utils.Blend.CONSTANT_ALPHA, "BLEND_CONSTANT_ALPHA"),
            new ((int)Engine.Utils.Blend.ONE_MINUS_CONSTANT_ALPHA, "BLEND_ONE_MINUS_CONSTANT_ALPHA"),
            new ((int)Engine.Utils.Blend.SRC_ALPHA_SATURATE, "BLEND_SRC_ALPHA_SATURATE")
        }
    };

    public static readonly LuascriptEnums StrumScriptTarget = new LuascriptEnums() {
        enum_name = "StrumTarget",
        reject_on_null_or_empty = true,
        pairs = new LuascriptEnumsPair[] {
            new ((int)Engine.Game.Strum.StrumScriptTarget.MARKER, "marker"),
            new ((int)Engine.Game.Strum.StrumScriptTarget.SICK_EFFECT, "sick_effect"),
            new ((int)Engine.Game.Strum.StrumScriptTarget.BACKGROUND, "background"),
            new ((int)Engine.Game.Strum.StrumScriptTarget.STRUM_LINE, "strum_line"),
            new ((int)Engine.Game.Strum.StrumScriptTarget.NOTE, "note"),
            new ((int)Engine.Game.Strum.StrumScriptTarget.ALL, "all")
        }
    };

    public static readonly LuascriptEnums StrumScriptOn = new LuascriptEnums() {
        enum_name = "StrumOn",
        reject_on_null_or_empty = true,
        pairs = new LuascriptEnumsPair[] {
            new ((int)Engine.Game.Strum.StrumScriptOn.HIT_DOWN, "hit_down"),
            new ((int)Engine.Game.Strum.StrumScriptOn.HIT_UP, "hit_up"),
            new ((int)Engine.Game.Strum.StrumScriptOn.MISS, "miss"),
            new ((int)Engine.Game.Strum.StrumScriptOn.PENALITY, "penality"),
            new ((int)Engine.Game.Strum.StrumScriptOn.IDLE, "idle"),
            new ((int)Engine.Game.Strum.StrumScriptOn.ALL, "all")
        }
    };

    public static readonly LuascriptEnums ScrollDirection = new LuascriptEnums() {
        enum_name = "ScrollDirection",
        reject_on_null_or_empty = true,
        pairs = new LuascriptEnumsPair[] {
            new ((int)Engine.Game.ScrollDirection.UPSCROLL, "UPSCROLL"),
            new ((int)Engine.Game.ScrollDirection.DOWNSCROLL, "DOWNSCROLL"),
            new ((int)Engine.Game.ScrollDirection.LEFTSCROLL, "LEFTSCROLL"),
            new ((int)Engine.Game.ScrollDirection.RIGHTSCROLL, "RIGHTSCROLL")
        }
    };

    public static readonly LuascriptEnums Align = new LuascriptEnums() {
        enum_name = "Align",
        reject_on_null_or_empty = false,
        pairs = new LuascriptEnumsPair[] {
            new ((int)Engine.Utils.Align.NONE, "none"),
            new ((int)Engine.Utils.Align.START, "start"),
            new ((int)Engine.Utils.Align.END, "end"),
            new ((int)Engine.Utils.Align.CENTER, "center"),
            new ((int)Engine.Utils.Align.BOTH, "both")
        }
    };

    public static readonly LuascriptEnums CharacterActionType = new LuascriptEnums() {
        enum_name = "CharacterActionType",
        reject_on_null_or_empty = false,
        pairs = new LuascriptEnumsPair[] {
            new ((int)Engine.Game.CharacterActionType.NONE, "none"),
            new ((int)Engine.Game.CharacterActionType.MISS, "miss"),
            new ((int)Engine.Game.CharacterActionType.EXTRA, "extra"),
            new ((int)Engine.Game.CharacterActionType.IDLE, "idle"),
            new ((int)Engine.Game.CharacterActionType.SING, "sing")
        }
    };

}
