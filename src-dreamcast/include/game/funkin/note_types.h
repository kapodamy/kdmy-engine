#ifndef _note_types_h
#define _note_types_h

//
//  forward reference
//

typedef struct StateSprite_s* StateSprite;
typedef struct ModelHolder_s* ModelHolder;


typedef enum {
    ScrollDirection_UPSCROLL,
    ScrollDirection_DOWNSCROLL,
    ScrollDirection_LEFTSCROLL,
    ScrollDirection_RIGHTSCROLL
} ScrollDirection;


typedef struct Note_s* Note;

#endif
