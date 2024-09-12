#ifndef _ddrkeymon_h
#define _ddrkeymon_h

#include <stdbool.h>
#include <stdint.h>

#include "float64.h"
#include "gamepad.h"


#define DDRKEYMON_FIFO_LENGTH 64


typedef struct {
    float64 in_song_timestamp;
    int32_t strum_id;
    bool holding;
    bool discard;
    bool strum_invisible;
    GamepadButtons button;
} DDRKey;

typedef struct {
    DDRKey queue[DDRKEYMON_FIFO_LENGTH];
    int32_t available;
} DDRKeysFIFO;

typedef struct DDRKeymon_s* DDRKeymon;


DDRKeymon ddrkeymon_init(Gamepad gamepad, GamepadButtons* strum_binds, int32_t strum_binds_size);
void ddrkeymon_destroy(DDRKeymon* ddrkeymon_ptr);

DDRKeysFIFO* ddrkeymon_get_fifo(DDRKeymon ddrkeymon);
void ddrkeymon_purge(DDRKeysFIFO* ddrkeys_fifo);
void ddrkeymon_purge2(DDRKeysFIFO* ddrkeys_fifo, bool force_drop_first);
void ddrkeymon_clear(DDRKeymon ddrkeymon);
void ddrkeymon_start(DDRKeymon ddrkeymon, float64 offset_timestamp);
void ddrkeymon_stop(DDRKeymon ddrkeymon);
float64 ddrkeymon_peek_timestamp(DDRKeymon ddrkeymon);
void ddrkeymon_resync(DDRKeymon ddrkeymon, float64 offset_timestamp);


#endif
