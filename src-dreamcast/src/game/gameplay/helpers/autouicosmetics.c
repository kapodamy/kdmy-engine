#include "game/gameplay/helpers/autouicosmetics.h"

#include "drawable.h"
#include "layout.h"
#include "logger.h"
#include "malloc_utils.h"


LayoutPlaceholder AUTOUICOSMETICS_PLACEHOLDER_STREAK = {
    .name = NULL,
    .type = VERTEX_DRAWABLE,
    .vertex = NULL,
    .width = 0.0f,
    .height = 80.0f
};
LayoutPlaceholder AUTOUICOSMETICS_PLACEHOLDER_RANK = {
    .name = NULL,
    .type = VERTEX_DRAWABLE,
    .vertex = NULL,
    .width = -1.0f,
    .height = 110.0f
};
LayoutPlaceholder AUTOUICOSMETICS_PLACEHOLDER_ACCURACY = {
    .name = NULL,
    .type = VERTEX_DRAWABLE,
    .vertex = NULL,
    .width = -1.0f,
    .height = 32.0f
};


AutoUICosmetics autouicosmetics_init() {
    AutoUICosmetics autouicosmetics = malloc_chk(sizeof(struct AutoUICosmetics_s));
    malloc_assert(autouicosmetics, AutoUICosmetics);

    *autouicosmetics = (struct AutoUICosmetics_s){
        .drawable_self = NULL,
        .drawable_streak = NULL,
        .drawable_rank = NULL,
        .drawable_accuracy = NULL
    };
    autouicosmetics->drawable_self = drawable_init(-1.0f, autouicosmetics, (DelegateDraw)autouicosmetics_draw, (DelegateAnimate)autouicosmetics_animate);
    return autouicosmetics;
}

void autouicosmetics_destroy(AutoUICosmetics* autouicosmetics_ptr) {
    if (!autouicosmetics_ptr || !*autouicosmetics_ptr) return;

    AutoUICosmetics autouicosmetics = *autouicosmetics_ptr;

    drawable_destroy(&autouicosmetics->drawable_self);

    autouicosmetics->drawable_streak = NULL;
    autouicosmetics->drawable_rank = NULL;
    autouicosmetics->drawable_accuracy = NULL;

    AUTOUICOSMETICS_PLACEHOLDER_STREAK.vertex = NULL;
    AUTOUICOSMETICS_PLACEHOLDER_RANK.vertex = NULL;
    AUTOUICOSMETICS_PLACEHOLDER_ACCURACY.vertex = NULL;

    free_chk(autouicosmetics);
    *autouicosmetics_ptr = NULL;
}


bool autouicosmetics_prepare_placeholders(AutoUICosmetics autouicosmetics, Layout layout) {
    autouicosmetics->drawable_streak = NULL;
    autouicosmetics->drawable_rank = NULL;
    autouicosmetics->drawable_accuracy = NULL;

    LayoutPlaceholder* ui_autoplace = layout_get_placeholder(layout, "ui_autoplace_cosmetics");
    if (!ui_autoplace) {
        autouicosmetics->layout_group_id = -1;
        return false;
    }

    drawable_helper_update_from_placeholder(autouicosmetics->drawable_self, ui_autoplace);
    autouicosmetics->layout_group_id = ui_autoplace->group_id;

    //
    // prepare stubs
    //
    AUTOUICOSMETICS_PLACEHOLDER_STREAK.z = ui_autoplace->z + 0.1f;
    AUTOUICOSMETICS_PLACEHOLDER_RANK.z = ui_autoplace->z + 0.2f;
    AUTOUICOSMETICS_PLACEHOLDER_ACCURACY.z = ui_autoplace->z + 0.3f;

    AUTOUICOSMETICS_PLACEHOLDER_STREAK.group_id = ui_autoplace->group_id;
    AUTOUICOSMETICS_PLACEHOLDER_RANK.group_id = ui_autoplace->group_id;
    AUTOUICOSMETICS_PLACEHOLDER_ACCURACY.group_id = ui_autoplace->group_id;

    AUTOUICOSMETICS_PLACEHOLDER_STREAK.x = ui_autoplace->x + ui_autoplace->width / 4.0f;
    AUTOUICOSMETICS_PLACEHOLDER_STREAK.y = ui_autoplace->y + ui_autoplace->height / 2.0f;
    AUTOUICOSMETICS_PLACEHOLDER_RANK.x = ui_autoplace->x;
    AUTOUICOSMETICS_PLACEHOLDER_RANK.y = ui_autoplace->y;
    AUTOUICOSMETICS_PLACEHOLDER_ACCURACY.x = ui_autoplace->x + ui_autoplace->width;
    AUTOUICOSMETICS_PLACEHOLDER_ACCURACY.y = ui_autoplace->y + ui_autoplace->height;

    return true;
}

void autouicosmetics_pick_drawables(AutoUICosmetics autouicosmetics) {
    autouicosmetics->drawable_streak = AUTOUICOSMETICS_PLACEHOLDER_STREAK.vertex;
    AUTOUICOSMETICS_PLACEHOLDER_STREAK.vertex = NULL;

    autouicosmetics->drawable_rank = AUTOUICOSMETICS_PLACEHOLDER_RANK.vertex;
    AUTOUICOSMETICS_PLACEHOLDER_RANK.vertex = NULL;

    autouicosmetics->drawable_accuracy = AUTOUICOSMETICS_PLACEHOLDER_ACCURACY.vertex;
    AUTOUICOSMETICS_PLACEHOLDER_ACCURACY.vertex = NULL;
}

void autouicosmetics_draw(AutoUICosmetics autouicosmetics, PVRContext pvrctx) {
    if (autouicosmetics->drawable_streak) drawable_draw(autouicosmetics->drawable_streak, pvrctx);
    if (autouicosmetics->drawable_rank) drawable_draw(autouicosmetics->drawable_rank, pvrctx);
    if (autouicosmetics->drawable_accuracy) drawable_draw(autouicosmetics->drawable_accuracy, pvrctx);
}

int32_t autouicosmetics_animate(AutoUICosmetics autouicosmetics, float elapsed) {
    int32_t completed = 0;
    if (autouicosmetics->drawable_streak) completed += drawable_animate(autouicosmetics->drawable_streak, elapsed);
    if (autouicosmetics->drawable_rank) completed += drawable_animate(autouicosmetics->drawable_rank, elapsed);
    if (autouicosmetics->drawable_accuracy) completed += drawable_animate(autouicosmetics->drawable_accuracy, elapsed);
    return completed;
}
