#ifndef __dialogue_h
#define __dialogue_h

#include "modifier.h"
#include "pvrctx.h"
#include <stdbool.h>

typedef struct _Dialogue_t {
    int dummy;
} Dialogue_t;

typedef Dialogue_t* Dialogue;

bool dialogue_apply_state(Dialogue dialogue, const char* state_name);
bool dialogue_is_completed(Dialogue dialogue);
bool dialogue_is_hidden(Dialogue dialogue);
bool dialogue_show_dialog(Dialogue dialogue, const char* dialog_src);
void dialogue_close(Dialogue dialogue);
void dialogue_hide(Dialogue dialogue, bool hidden);
Modifier dialogue_get_modifier(Dialogue dialogue);
void dialogue_set_offsetcolor(Dialogue dialogue, float r, float g, float b, float a);
void dialogue_set_alpha(Dialogue dialogue, float alpha);
void dialogue_set_antialiasing(Dialogue dialogue, PVRFlag antialiasing);



#endif
