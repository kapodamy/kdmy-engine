#ifndef _weekselector_mdlselect_h
#define _weekselector_mdlselect_h

#include <stdbool.h>
#include <stdint.h>

#include "layout_types.h"
#include "pvrcontext_types.h"


//
//  forward references
//

typedef struct AnimList_s* AnimList;
typedef struct Drawable_s* Drawable;
typedef struct Layout_s* Layout;
typedef struct ModelHolder_s* ModelHolder;
typedef struct StateSprite_s* StateSprite;

extern const char* WEEKSELECTOR_MDLSELECT_HEY;
extern const char* WEEKSELECTOR_MDLSELECT_IDLE;
extern const char* WEEKSELECTOR_MDLSELECT_MODELS_BF;
extern const char* WEEKSELECTOR_MDLSELECT_MODELS_GF;

typedef struct WeekSelectorMdlSelect_s* WeekSelectorMdlSelect;


WeekSelectorMdlSelect weekselector_mdlselect_init(AnimList animlist, ModelHolder modelholder, Layout layout, bool is_boyfriend);
void weekselector_mdlselect_destroy(WeekSelectorMdlSelect* mdlselect);

void weekselector_mdlselect_draw(WeekSelectorMdlSelect mdlselect, PVRContext pvrctx);
int32_t weekselector_mdlselect_animate(WeekSelectorMdlSelect mdlselect, float elapsed);

char* weekselector_mdlselect_get_manifest(WeekSelectorMdlSelect mdlselect);
bool weekselector_mdlselect_is_selected_locked(WeekSelectorMdlSelect mdlselect);
void weekselector_mdlselect_toggle_choosen(WeekSelectorMdlSelect mdlselect);
void weekselector_mdlselect_enable_arrows(WeekSelectorMdlSelect mdlselect, bool enabled);
void weekselector_mdlselect_select_default(WeekSelectorMdlSelect mdlselect);
bool weekselector_mdlselect_select(WeekSelectorMdlSelect mdlselect, int32_t new_index);
bool weekselector_mdlselect_scroll(WeekSelectorMdlSelect mdlselect, int32_t offset);
Drawable weekselector_mdlselect_get_drawable(WeekSelectorMdlSelect mdlselect);
void weekselector_mdlselect_set_beats(WeekSelectorMdlSelect mdlselect, float bpm);

void weekselector_mdlselect_helper_import(StateSprite statesprite, ModelHolder mdlhldr, LayoutPlaceholder* placeholder, bool enable_beat, const char* anim_name, const char* name);

#endif
