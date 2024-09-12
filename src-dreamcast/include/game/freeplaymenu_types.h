#ifndef _freeplaymenu_types_h
#define _freeplaymenu_types_h

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    int32_t song_index;
    int32_t week_index;
    bool is_locked;
    int32_t gameplaymanifest_index;
} MappedSong;

#endif
