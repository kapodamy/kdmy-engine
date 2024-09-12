#ifndef _ffgraph_frame_h
#define _ffgraph_frame_h

#include <stdint.h>

typedef struct {
    uint16_t width;
    uint16_t height;
    uint8_t* data;
} YUVPlane;

typedef struct {
    uint8_t* y;
    uint8_t* u;
    uint8_t* v;
} YUVFrame;

#endif
