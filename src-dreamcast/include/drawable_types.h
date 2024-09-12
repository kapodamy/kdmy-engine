#ifndef _drawable_types_h
#define _drawable_types_h

#include <stdint.h>

#include "pvrcontext_types.h"


typedef struct Drawable_s* Drawable;

typedef void (*DelegateDraw)(void* vertex, PVRContext pvrctx);
typedef int32_t (*DelegateAnimate)(void* vertex, float elapsed);

#endif
