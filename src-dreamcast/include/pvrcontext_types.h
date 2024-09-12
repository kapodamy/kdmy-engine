#ifndef _pvrcontext_types_h
#define _pvrcontext_types_h


#include "color2d.h"


//
//  forward references
//

typedef struct PVRContext_s* PVRContext;


extern const RGBA PVR_DEFAULT_OFFSET_COLOR;


typedef enum {
    PVRCTX_FLAG_DISABLE = 0,
    PVRCTX_FLAG_ENABLE = 1,
    PVRCTX_FLAG_DEFAULT = 2
} PVRFlag;

typedef enum {
    VERTEX_NONE = -1,
    VERTEX_SPRITE = 0,
    VERTEX_TEXTSPRITE = 1,
    VERTEX_STATESPRITE = 2,
    VERTEX_DRAWABLE = 3,
    VERTEX_SHAPE = 4
} PVRVertex;

#endif
