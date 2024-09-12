#ifndef _dialogue_h
#define _dialogue_h

#include <stdbool.h>
#include <stdint.h>

#include "pvrcontext_types.h"
#include "modifier.h"


//
//  forward reference
//

typedef struct Luascript_s* WeekScript;
typedef struct Drawable_s* Drawable;


typedef struct Dialogue_s* Dialogue;


Dialogue dialogue_init(const char* src, float viewport_width, float viewport_height);
void dialogue_destroy(Dialogue* dialogue_ptr);
bool dialogue_apply_state(Dialogue dialogue, const char* state_name);
bool dialogue_apply_state2(Dialogue dialogue, const char* state_name, const char* if_line_label);
int32_t dialogue_animate(Dialogue dialogue, float elapsed);
void dialogue_poll(Dialogue dialogue, bool elapsed);
void dialogue_draw(Dialogue dialogue, PVRContext pvrctx);
bool dialogue_is_completed(Dialogue dialogue);
bool dialogue_is_hidden(Dialogue dialogue);
bool dialogue_show_dialog(Dialogue dialogue, const char* src_dialog);
bool dialogue_show_dialog2(Dialogue dialogue, const char* text_dialog_content);
void dialogue_close(Dialogue dialogue);
void dialogue_hide(Dialogue dialogue, bool hidden);
void dialogue_suspend(Dialogue dialogue);
void dialogue_resume(Dialogue dialogue);
Modifier* dialogue_get_modifier(Dialogue dialogue);
Drawable dialogue_get_drawable(Dialogue dialogue);
void dialogue_set_offsetcolor(Dialogue dialogue, float r, float g, float b, float a);
void dialogue_set_antialiasing(Dialogue dialogue, PVRFlag pvrflag);
void dialogue_set_alpha(Dialogue dialogue, float alpha);
void dialogue_set_script(Dialogue dialogue, WeekScript weekscript);

#endif
