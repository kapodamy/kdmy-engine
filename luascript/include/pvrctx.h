#ifndef __pvrctx_h
#define __pvrctx_h

typedef struct _PVRContext_t {
    int dummy;
} PVRContext_t;

typedef PVRContext_t* PVRContext;

typedef int PVRFlag;

#define PVR_FLAG_DEFAULT 2
#define PVR_FLAG_ENABLE 1
#define PVR_FLAG_DISABLE 0

#endif

