#include "luascript_enums.h"

#include "game/funkin/character.h"
#include "game/funkin/note_types.h"
#include "game/funkin/strum_types.h"
#include "pvrcontext_types.h"
#include "stringutils.h"
#include "textsprite.h"
#include "vertexprops.h"


const LuascriptEnums LUASCRIPT_ENUMS_PVRFlag = &(struct LuascriptEnums_s){
    .enum_name = "PVRFlag",
    .reject_on_null_or_empty = false,
    .pairs = (LuascriptEnumsPair[]){
        {.value = PVRCTX_FLAG_DEFAULT, .name = "default"},
        {.value = PVRCTX_FLAG_ENABLE, .name = "enable"},
        {.value = PVRCTX_FLAG_DISABLE, .name = "disable"},
        {.name = NULL}
    }
};

const LuascriptEnums LUASCRIPT_ENUMS_TextSpriteForceCase = &(struct LuascriptEnums_s){
    .enum_name = "TextSpriteForceCase",
    .reject_on_null_or_empty = false,
    .pairs = (LuascriptEnumsPair[]){
        {.value = TEXTSPRITE_FORCE_NONE, .name = "none"},
        {.value = TEXTSPRITE_FORCE_UPPERCASE, .name = "upper"},
        {.value = TEXTSPRITE_FORCE_LOWERCASE, .name = "lower"},
        {.name = NULL}
    }
};

const LuascriptEnums LUASCRIPT_ENUMS_FontWordBreak = &(struct LuascriptEnums_s){
    .enum_name = "FontWordBreak",
    .reject_on_null_or_empty = false,
    .pairs = (LuascriptEnumsPair[]){
        {.value = FONT_WORDBREAK_NONE, .name = "none"},
        {.value = FONT_WORDBREAK_LOOSE, .name = "loose"},
        {.value = FONT_WORDBREAK_BREAK, .name = "break"},
        {.name = NULL}
    }
};

const LuascriptEnums LUASCRIPT_ENUMS_AnimInterpolator = &(struct LuascriptEnums_s){
    .enum_name = "AnimInterpolator",
    .reject_on_null_or_empty = true,
    .pairs = (LuascriptEnumsPair[]){
        {.value = AnimInterpolator_EASE, .name = "ease"},
        {.value = AnimInterpolator_EASE_IN, .name = "ease-in"},
        {.value = AnimInterpolator_EASE_OUT, .name = "ease-out"},
        {.value = AnimInterpolator_EASE_IN_OUT, .name = "ease-in-out"},
        {.value = AnimInterpolator_LINEAR, .name = "linear"},
        {.value = AnimInterpolator_STEPS, .name = "steps"},
        {.value = AnimInterpolator_CUBIC, .name = "cubic"},
        {.value = AnimInterpolator_QUAD, .name = "quad"},
        {.value = AnimInterpolator_EXPO, .name = "expo"},
        {.value = AnimInterpolator_SIN, .name = "sin"},
        {.name = NULL}
    }
};

const LuascriptEnums LUASCRIPT_ENUMS_Blend = &(struct LuascriptEnums_s){
    .enum_name = "Blend",
    .reject_on_null_or_empty = false,
    .pairs = (LuascriptEnumsPair[]){
        {.value = BLEND_DEFAULT, .name = "BLEND_DEFAULT"},
        {.value = BLEND_ZERO, .name = "BLEND_ZERO"},
        {.value = BLEND_ONE, .name = "BLEND_ONE"},
#ifndef _arch_dreamcast
        {.value = BLEND_SRC_COLOR, .name = "BLEND_SRC_COLOR"},
        {.value = BLEND_ONE_MINUS_SRC_COLOR, .name = "BLEND_ONE_MINUS_SRC_COLOR"},
#endif
        {.value = BLEND_DST_COLOR, .name = "BLEND_DST_COLOR"},
        {.value = BLEND_ONE_MINUS_DST_COLOR, .name = "BLEND_ONE_MINUS_DST_COLOR"},
        {.value = BLEND_SRC_ALPHA, .name = "BLEND_SRC_ALPHA"},
        {.value = BLEND_ONE_MINUS_SRC_ALPHA, .name = "BLEND_ONE_MINUS_SRC_ALPHA"},
        {.value = BLEND_DST_ALPHA, .name = "BLEND_DST_ALPHA"},
        {.value = BLEND_ONE_MINUS_DST_ALPHA, .name = "BLEND_ONE_MINUS_DST_ALPHA"},
#ifndef _arch_dreamcast
        {.value = BLEND_CONSTANT_COLOR, .name = "BLEND_CONSTANT_COLOR"},
        {.value = BLEND_ONE_MINUS_CONSTANT_COLOR, .name = "BLEND_ONE_MINUS_CONSTANT_COLOR"},
        {.value = BLEND_CONSTANT_ALPHA, .name = "BLEND_CONSTANT_ALPHA"},
        {.value = BLEND_ONE_MINUS_CONSTANT_ALPHA, .name = "BLEND_ONE_MINUS_CONSTANT_ALPHA"},
        {.value = BLEND_SRC_ALPHA_SATURATE, .name = "BLEND_SRC_ALPHA_SATURATE"},
#endif
        {.name = NULL}
    }
};

const LuascriptEnums LUASCRIPT_ENUMS_StrumScriptTarget = &(struct LuascriptEnums_s){
    .enum_name = "StrumScriptTarget",
    .reject_on_null_or_empty = true,
    .pairs = (LuascriptEnumsPair[]){
        {.value = StrumScriptTarget_MARKER, .name = "marker"},
        {.value = StrumScriptTarget_SICK_EFFECT, .name = "sick_effect"},
        {.value = StrumScriptTarget_BACKGROUND, .name = "background"},
        {.value = StrumScriptTarget_STRUM_LINE, .name = "strum_line"},
        {.value = StrumScriptTarget_NOTE, .name = "note"},
        {.value = StrumScriptTarget_ALL, .name = "all"},
        {.name = NULL}
    }
};

const LuascriptEnums LUASCRIPT_ENUMS_StrumScriptOn = &(struct LuascriptEnums_s){
    .enum_name = "StrumScriptOn",
    .reject_on_null_or_empty = true,
    .pairs = (LuascriptEnumsPair[]){
        {.value = StrumScriptOn_HIT_DOWN, .name = "hit_down"},
        {.value = StrumScriptOn_HIT_UP, .name = "hit_up"},
        {.value = StrumScriptOn_MISS, .name = "miss"},
        {.value = StrumScriptOn_PENALITY, .name = "penality"},
        {.value = StrumScriptOn_IDLE, .name = "idle"},
        {.value = StrumScriptOn_ALL, .name = "all"},
        {.name = NULL}
    }
};

const LuascriptEnums LUASCRIPT_ENUMS_ScrollDirection = &(struct LuascriptEnums_s){
    .enum_name = "ScrollDirection",
    .reject_on_null_or_empty = true,
    .pairs = (LuascriptEnumsPair[]){
        {.value = ScrollDirection_UPSCROLL, .name = "UPSCROLL"},
        {.value = ScrollDirection_DOWNSCROLL, .name = "DOWNSCROLL"},
        {.value = ScrollDirection_LEFTSCROLL, .name = "LEFTSCROLL"},
        {.value = ScrollDirection_RIGHTSCROLL, .name = "RIGHTSCROLL"},
        {.name = NULL}
    }
};

const LuascriptEnums LUASCRIPT_ENUMS_Align = &(struct LuascriptEnums_s){
    .enum_name = "Align",
    .reject_on_null_or_empty = false,
    .pairs = (LuascriptEnumsPair[]){
        {.value = ALIGN_NONE, .name = "none"},
        {.value = ALIGN_START, .name = "start"},
        {.value = ALIGN_END, .name = "end"},
        {.value = ALIGN_CENTER, .name = "center"},
        {.value = ALIGN_BOTH, .name = "both"},
        {.name = NULL}
    }
};

const LuascriptEnums LUASCRIPT_ENUMS_CharacterActionType = &(struct LuascriptEnums_s){
    .enum_name = "CharacterActionType",
    .reject_on_null_or_empty = false,
    .pairs = (LuascriptEnumsPair[]){
        {.value = CharacterActionType_NONE, .name = "none"},
        {.value = CharacterActionType_MISS, .name = "miss"},
        {.value = CharacterActionType_EXTRA, .name = "extra"},
        {.value = CharacterActionType_IDLE, .name = "idle"},
        {.value = CharacterActionType_SING, .name = "sing"},
        {.name = NULL}
    }
};
