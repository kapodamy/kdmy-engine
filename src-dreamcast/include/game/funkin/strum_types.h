#ifndef _strum_types_h
#define _strum_types_h

//
//  forward reference
//

typedef struct Chart_s* Chart;


typedef enum {
    StrumPressState_NONE = 0,
    StrumPressState_HIT,
    StrumPressState_HIT_SUSTAIN,
    StrumPressState_PENALTY_NOTE,
    StrumPressState_PENALTY_HIT,
    StrumPressState_MISS,
    StrumPressState_INVALID
} StrumPressState;

typedef enum {
    NoteState_CLEAR,
    NoteState_MISS,
    NoteState_PENDING,
    NoteState_HOLD,
    NoteState_RELEASE
} NoteState;

typedef enum {
    StrumScriptOn_HIT_DOWN,
    StrumScriptOn_HIT_UP,
    StrumScriptOn_MISS,
    StrumScriptOn_PENALITY,
    StrumScriptOn_IDLE,
    StrumScriptOn_ALL
} StrumScriptOn;

typedef enum {
    StrumScriptTarget_MARKER,
    StrumScriptTarget_SICK_EFFECT,
    StrumScriptTarget_BACKGROUND,
    StrumScriptTarget_STRUM_LINE,
    StrumScriptTarget_NOTE,
    StrumScriptTarget_ALL
} StrumScriptTarget;

typedef struct Strum_s* Strum;

#endif
