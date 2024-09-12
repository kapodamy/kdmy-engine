#ifndef _macroexecutor_h
#define _macroexecutor_h

#include "drawable.h"
#include "macroexecutor_types.h"
#include "sprite.h"
#include "statesprite.h"
#include "textsprite.h"

typedef struct MacroExecutor_s* MacroExecutor;


MacroExecutor macroexecutor_init(const AnimListItem* animlist_item);
void macroexecutor_destroy(MacroExecutor* macroexecutor);
void macroexecutor_set_restart_in_frame(MacroExecutor macroexecutor, int32_t frame_index, bool allow_change_size);
void macroexecutor_set_speed(MacroExecutor macroexecutor, float speed);
void macroexecutor_restart(MacroExecutor macroexecutor);
int32_t macroexecutor_animate(MacroExecutor macroexecutor, float elapsed);
void macroexecutor_state_apply(MacroExecutor macroexecutor, Sprite sprite, bool no_stack_changes);
void macroexecutor_state_apply2(MacroExecutor macroexecutor, StateSprite statesprite, bool no_stack_changes);
void macroexecutor_state_apply3(MacroExecutor macroexecutor, Drawable drawable, bool no_stack_changes);
void macroexecutor_state_apply4(MacroExecutor macroexecutor, TextSprite textsprite, bool no_stack_changes);
void macroexecutor_state_apply5(MacroExecutor macroexecutor, void* private_data, PropertySetter setter_callback, bool no_stack_changes);
void macroexecutor_state_apply_minimal(MacroExecutor macroexecutor, Sprite sprite);
void macroexecutor_state_to_modifier(MacroExecutor macroexecutor, Modifier* modifier, bool no_stack_changes);
void macroexecutor_state_from_modifier(MacroExecutor macroexecutor, Modifier* modifier);
bool macroexecutor_is_completed(MacroExecutor macroexecutor);
void macroexecutor_force_end(MacroExecutor macroexecutor, Sprite sprite);
MacroExecutor macroexecutor_clone(MacroExecutor macroexecutor);
int32_t macroexecutor_get_frame_count(MacroExecutor macroexecutor);
const AtlasEntry* macroexecutor_get_frame(MacroExecutor macroexecutor, int32_t index);

#endif
