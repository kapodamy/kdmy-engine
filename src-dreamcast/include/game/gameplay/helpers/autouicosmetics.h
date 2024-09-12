#ifndef _autouicosmetics_h
#define _autouicosmetics_h

#include "drawable_types.h"
#include "layout_types.h"


typedef struct AutoUICosmetics_s{
    Drawable drawable_self;
    int32_t layout_group_id;
    Drawable drawable_streak;
    Drawable drawable_rank;
    Drawable drawable_accuracy;
}* AutoUICosmetics;

extern LayoutPlaceholder AUTOUICOSMETICS_PLACEHOLDER_STREAK;
extern LayoutPlaceholder AUTOUICOSMETICS_PLACEHOLDER_RANK;
extern LayoutPlaceholder AUTOUICOSMETICS_PLACEHOLDER_ACCURACY;

AutoUICosmetics autouicosmetics_init();
void autouicosmetics_destroy(AutoUICosmetics* autouicosmetics_ptr);

bool autouicosmetics_prepare_placeholders(AutoUICosmetics autouicosmetics, Layout layout);
void autouicosmetics_pick_drawables(AutoUICosmetics autouicosmetics);
void autouicosmetics_draw(AutoUICosmetics autouicosmetics, PVRContext pvrctx);
int32_t autouicosmetics_animate(AutoUICosmetics autouicosmetics, float elapsed);

#endif
