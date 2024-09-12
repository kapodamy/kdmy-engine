#ifndef _luascript_enums_h
#define _luascript_enums_h

#include <stdbool.h>
#include <stdint.h>


typedef struct {
    const int32_t value;
    const char* name;
} LuascriptEnumsPair;

typedef struct LuascriptEnums_s {
    const char* enum_name;
    const bool reject_on_null_or_empty;
    const LuascriptEnumsPair* pairs;
}* LuascriptEnums;


extern const LuascriptEnums LUASCRIPT_ENUMS_PVRFlag;
extern const LuascriptEnums LUASCRIPT_ENUMS_TextSpriteForceCase;
extern const LuascriptEnums LUASCRIPT_ENUMS_FontWordBreak;
extern const LuascriptEnums LUASCRIPT_ENUMS_AnimInterpolator;
extern const LuascriptEnums LUASCRIPT_ENUMS_Blend;
extern const LuascriptEnums LUASCRIPT_ENUMS_StrumScriptTarget;
extern const LuascriptEnums LUASCRIPT_ENUMS_StrumScriptOn;
extern const LuascriptEnums LUASCRIPT_ENUMS_ScrollDirection;
extern const LuascriptEnums LUASCRIPT_ENUMS_Align;
extern const LuascriptEnums LUASCRIPT_ENUMS_CharacterActionType;

#endif
