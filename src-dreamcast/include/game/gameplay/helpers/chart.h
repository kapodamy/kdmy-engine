#ifndef _chart_h
#define _chart_h

#include <stdbool.h>
#include <stdint.h>

#include "float64.h"


typedef enum {
    ChartEvent_NONE = 1,
    ChartEvent_CAMERA_OPPONENT = 2,
    ChartEvent_CAMERA_PLAYER = 3,
    ChartEvent_CHANGE_BPM = 4,
    ChartEvent_ALT_ANIM_OPPONENT = 5,
    ChartEvent_ALT_ANIM_PLAYER = 6,
    ChartEvent_UNKNOWN_NOTE = 7
} ChartEvent;

typedef struct {
    float64 timestamp;
    float64 direction;
    float64 duration;
    bool alt_anim;
    float64 data;
} ChartNote;

typedef struct {
    ChartNote* notes;
    int32_t notes_size;
} ChartEntry;

typedef struct {
    bool is_player_or_opponent;
    float64 timestamp;
    ChartEvent command;
    float64 parameter;
    float64 parameter2;
    float64 parameter3;
} ChartEventEntry;

typedef struct Chart_s {
    ChartEntry* entries;
    int32_t entries_size;

    ChartEventEntry* events;
    int32_t events_size;

    float speed;
    float bpm;
}* Chart;


Chart chart_init(const char* src, const char* difficult);
void chart_destroy(Chart* chart);

#endif
