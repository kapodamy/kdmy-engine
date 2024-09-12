#ifndef _character_types_h
#define _character_types_h


typedef enum {
    CharacterActionType_NONE,
    CharacterActionType_MISS,
    CharacterActionType_SING,
    CharacterActionType_EXTRA,
    CharacterActionType_IDLE
} CharacterActionType;

typedef struct Character_s* Character;

#endif
