"use strict";


/** 
 * @typedef {object} LuascriptEnumsPair
 * @property {number} value
 * @property {string} name
*/
/** 
 * @typedef {object} LuascriptEnums
 * @property {string} enum_name
 * @property {boolean} reject_on_null_or_empty
 * @property {LuascriptEnumsPair[]} pairs
 */


/**@type {LuascriptEnums}*/const LUASCRIPT_ENUMS_PVRFlag = {
    enum_name: "PVRFlag",
    reject_on_null_or_empty: false,
    pairs: [
        { value: PVRCTX_FLAG_DEFAULT, name: "default" },
        { value: PVRCTX_FLAG_ENABLE, name: "enable" },
        { value: PVRCTX_FLAG_DISABLE, name: "disable" },
        { name: null, value: -1 }
    ]
};

/**@type {LuascriptEnums}*/const LUASCRIPT_ENUMS_TextSpriteForceCase = {
    enum_name: "TextSpriteForceCase",
    reject_on_null_or_empty: false,
    pairs: [
        { value: TEXTSPRITE_FORCE_NONE, name: "none" },
        { value: TEXTSPRITE_FORCE_UPPERCASE, name: "upper" },
        { value: TEXTSPRITE_FORCE_LOWERCASE, name: "lower" },
        { name: null, value: -1 }
    ]
};

/**@type {LuascriptEnums}*/const LUASCRIPT_ENUMS_FontWordBreak = {
    enum_name: "FontWordBreak",
    reject_on_null_or_empty: false,
    pairs: [
        { value: FONT_WORDBREAK_NONE, name: "none" },
        { value: FONT_WORDBREAK_LOOSE, name: "loose" },
        { value: FONT_WORDBREAK_BREAK, name: "break" },
        { name: null, value: -1 }
    ]
};

/**@type {LuascriptEnums}*/const LUASCRIPT_ENUMS_AnimInterpolator = {
    enum_name: "AnimInterpolator",
    reject_on_null_or_empty: true,
    pairs: [
        { value: ANIM_INTERPOLATOR_EASE, name: "ease" },
        { value: ANIM_INTERPOLATOR_EASE_IN, name: "ease-in" },
        { value: ANIM_INTERPOLATOR_EASE_OUT, name: "ease-out" },
        { value: ANIM_INTERPOLATOR_EASE_IN_OUT, name: "ease-in-out" },
        { value: ANIM_INTERPOLATOR_LINEAR, name: "linear" },
        { value: ANIM_INTERPOLATOR_STEPS, name: "steps" },
        { value: ANIM_INTERPOLATOR_CUBIC, name: "cubic" },
        { value: ANIM_INTERPOLATOR_QUAD, name: "quad" },
        { value: ANIM_INTERPOLATOR_EXPO, name: "expo" },
        { value: ANIM_INTERPOLATOR_SIN, name: "sin" },
        { name: null, value: -1 }
    ]
};

/**@type {LuascriptEnums}*/const LUASCRIPT_ENUMS_Blend = {
    enum_name: "Blend",
    reject_on_null_or_empty: false,
    pairs: [
        { value: BLEND_DEFAULT, name: "BLEND_DEFAULT" },
        { value: BLEND_ZERO, name: "BLEND_ZERO" },
        { value: BLEND_ONE, name: "BLEND_ONE" },
        { value: BLEND_SRC_COLOR, name: "BLEND_SRC_COLOR" },
        { value: BLEND_ONE_MINUS_SRC_COLOR, name: "BLEND_ONE_MINUS_SRC_COLOR" },
        { value: BLEND_DST_COLOR, name: "BLEND_DST_COLOR" },
        { value: BLEND_ONE_MINUS_DST_COLOR, name: "BLEND_ONE_MINUS_DST_COLOR" },
        { value: BLEND_SRC_ALPHA, name: "BLEND_SRC_ALPHA" },
        { value: BLEND_ONE_MINUS_SRC_ALPHA, name: "BLEND_ONE_MINUS_SRC_ALPHA" },
        { value: BLEND_DST_ALPHA, name: "BLEND_DST_ALPHA" },
        { value: BLEND_ONE_MINUS_DST_ALPHA, name: "BLEND_ONE_MINUS_DST_ALPHA" },
        { value: BLEND_CONSTANT_COLOR, name: "BLEND_CONSTANT_COLOR" },
        { value: BLEND_ONE_MINUS_CONSTANT_COLOR, name: "BLEND_ONE_MINUS_CONSTANT_COLOR" },
        { value: BLEND_CONSTANT_ALPHA, name: "BLEND_CONSTANT_ALPHA" },
        { value: BLEND_ONE_MINUS_CONSTANT_ALPHA, name: "BLEND_ONE_MINUS_CONSTANT_ALPHA" },
        { value: BLEND_SRC_ALPHA_SATURATE, name: "BLEND_SRC_ALPHA_SATURATE" },
        { name: null, value: -1 }
    ]
};

/**@type {LuascriptEnums}*/const LUASCRIPT_ENUMS_StrumScriptTarget = {
    enum_name: "StrumTarget",
    reject_on_null_or_empty: true,
    pairs: [
        { value: STRUM_SCRIPT_TARGET_MARKER, name: "marker" },
        { value: STRUM_SCRIPT_TARGET_SICK_EFFECT, name: "sick_effect" },
        { value: STRUM_SCRIPT_TARGET_BACKGROUND, name: "background" },
        { value: STRUM_SCRIPT_TARGET_STRUM_LINE, name: "strum_line" },
        { value: STRUM_SCRIPT_TARGET_NOTE, name: "note" },
        { value: STRUM_SCRIPT_TARGET_ALL, name: "all" },
        { name: null, value: -1 }
    ]
};

/**@type {LuascriptEnums}*/const LUASCRIPT_ENUMS_StrumScriptOn = {
    enum_name: "StrumOn",
    reject_on_null_or_empty: true,
    pairs: [
        { value: STRUM_SCRIPT_ON_HIT_DOWN, name: "hit_down" },
        { value: STRUM_SCRIPT_ON_HIT_UP, name: "hit_up" },
        { value: STRUM_SCRIPT_ON_MISS, name: "miss" },
        { value: STRUM_SCRIPT_ON_PENALITY, name: "penality" },
        { value: STRUM_SCRIPT_ON_IDLE, name: "idle" },
        { value: STRUM_SCRIPT_ON_ALL, name: "all" },
        { name: null, value: -1 }
    ]
};

/**@type {LuascriptEnums}*/const LUASCRIPT_ENUMS_ScrollDirection = {
    enum_name: "ScrollDirection",
    reject_on_null_or_empty: true,
    pairs: [
        { value: STRUM_UPSCROLL, name: "UPSCROLL" },
        { value: STRUM_DOWNSCROLL, name: "DOWNSCROLL" },
        { value: STRUM_LEFTSCROLL, name: "LEFTSCROLL" },
        { value: STRUM_RIGHTSCROLL, name: "RIGHTSCROLL" },
        { name: null, value: -1 }
    ]
};

/**@type {LuascriptEnums}*/const LUASCRIPT_ENUMS_Align = {
    enum_name: "Align",
    reject_on_null_or_empty: false,

    pairs: [
        { value: ALIGN_NONE, name: "none" },
        { value: ALIGN_START, name: "start" },
        { value: ALIGN_END, name: "end" },
        { value: ALIGN_CENTER, name: "center" },
        { value: ALIGN_BOTH, name: "both" },
        { name: null, value: -1 }
    ]
};

/**@type {LuascriptEnums}*/const LUASCRIPT_ENUMS_CharacterActionType = {
    enum_name: "CharacterActionType",
    reject_on_null_or_empty: false,

    pairs: [
        { value: CharacterActionType.NONE, name: "none" },
        { value: CharacterActionType.MISS, name: "miss" },
        { value: CharacterActionType.EXTRA, name: "extra" },
        { value: CharacterActionType.IDLE, name: "idle" },
        { value: CharacterActionType.SING, name: "sing" },
        { name: null, value: -1 }
    ]
};

