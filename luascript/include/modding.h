#ifndef _modding_h

#include "layout.h"

typedef struct {
    int stub;
} ModdingContext_t;

typedef ModdingContext_t* ModdingContext;

void modding_unlockdirective_create(ModdingContext moddingcontext, const char* name, double value);
bool modding_unlockdirective_has(ModdingContext roundcontext, const char* name);
void modding_unlockdirective_remove(ModdingContext moddingcontext, const char* name);
double modding_unlockdirective_get(ModdingContext moddingcontext, const char* name);
void modding_set_ui_visibility(ModdingContext moddingcontext, bool visible);

Layout modding_get_layout(ModdingContext moddingcontext);
void modding_set_halt(ModdingContext moddingcontext, bool halt);
void modding_exit(ModdingContext moddingcontext);

#endif

