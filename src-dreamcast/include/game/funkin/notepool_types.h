#ifndef _notepool_types_h
#define _notepool_types_h

#include <stdbool.h>
#include <stdint.h>

//
//  forward reference
//

typedef struct ModelHolder_s* ModelHolder;
typedef struct Note_s* Note;

typedef struct {
    float hurt_ratio;
    float heal_ratio;
    bool ignore_hit;
    bool ignore_miss;
    bool can_kill_on_hit;
    char* custom_sick_effect_name;
    bool is_special;
} NoteAttribute;

typedef struct NotePool_s {
    int32_t size;
    Note* drawables;
    ModelHolder* models_custom_sick_effect;
    NoteAttribute* attributes;
}* NotePool;


#endif