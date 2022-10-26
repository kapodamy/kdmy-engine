#ifndef _modding_h

#include "layout.h"

typedef struct {
    int stub;
} ModdingContext_t;

typedef ModdingContext_t* ModdingContext;

Layout modding_get_layout(ModdingContext moddingcontext);
void modding_exit(ModdingContext moddingcontext);

#endif

