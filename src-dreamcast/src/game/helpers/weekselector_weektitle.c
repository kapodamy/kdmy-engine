#include "game/helpers/weekselector_weektitle.h"

#include "atlas.h"
#include "drawable.h"
#include "game/helpers/weekselector_difficult.h"
#include "game/helpers/weekselector_weeklist.h"
#include "imgutils.h"
#include "layout.h"
#include "malloc_utils.h"
#include "pvrcontext.h"
#include "sprite.h"


struct WeekSelectorWeekTitle_s {
    LayoutPlaceholder* placeholder1;
    LayoutPlaceholder* placeholder2;
    LayoutPlaceholder* placeholder3;
    Sprite sprite;
    Drawable drawable;
};


static const float WEEKSELECTOR_WEEKTITLE_ALT_ALPHA = 0.5f;


WeekSelectorWeekTitle weekselector_weektitle_init(Layout layout) {
    WeekSelectorWeekTitle weektitle = malloc_chk(sizeof(struct WeekSelectorWeekTitle_s));
    malloc_assert(weektitle, WeekSelectorWeekTitle);

    *weektitle = (struct WeekSelectorWeekTitle_s){
        .placeholder1 = layout_get_placeholder(layout, "ui_week_title_no_warns"),
        .placeholder2 = layout_get_placeholder(layout, "ui_week_title_warns"),
        .placeholder3 = layout_get_placeholder(layout, "ui_difficult_selected"),
        .sprite = sprite_init(NULL),
        .drawable = NULL
    };

    weektitle->drawable = drawable_init(-1.0f, weektitle, (DelegateDraw)weekselector_weektitle_draw, NULL);
    sprite_set_alpha(weektitle->sprite, WEEKSELECTOR_WEEKTITLE_ALT_ALPHA);

    return weektitle;
}

void weekselector_weektitle_destroy(WeekSelectorWeekTitle* weektitle_ptr) {
    if (!weektitle_ptr || !*weektitle_ptr) return;

    WeekSelectorWeekTitle weektitle = *weektitle_ptr;

    sprite_destroy(&weektitle->sprite);
    drawable_destroy(&weektitle->drawable);

    free_chk(weektitle);
    *weektitle_ptr = NULL;
}

void weekselector_weektitle_draw(WeekSelectorWeekTitle weektitle, PVRContext pvrctx) {
    if (!sprite_is_textured(weektitle->sprite)) return;
    pvr_context_save(pvrctx);
    sprite_draw(weektitle->sprite, pvrctx);
    pvr_context_restore(pvrctx);
}

void weekselector_weektitle_move(WeekSelectorWeekTitle weektitle, WeekSelectorWeekList weeklist, bool use_warns_placeholder) {
    float draw_width = 0.0f, draw_height = 0.0f;
    float draw_x = 0.0f, draw_y = 0.0f;
    AtlasEntry atlas_entry;

    if (!weektitle->placeholder1 && !weektitle->placeholder2) return;

    Sprite title_sprite = weekselector_weeklist_peek_title_sprite(weeklist);
    sprite_fill_atlas_entry(title_sprite, &atlas_entry);
    sprite_get_source_size(title_sprite, &draw_width, &draw_height);

    LayoutPlaceholder* old_placeholder = use_warns_placeholder ? weektitle->placeholder1 : weektitle->placeholder2;
    LayoutPlaceholder* placeholder = use_warns_placeholder ? weektitle->placeholder2 : weektitle->placeholder1;

    if (old_placeholder) old_placeholder->vertex = NULL;
    if (!placeholder) return;

    placeholder->vertex = weektitle->drawable;
    drawable_helper_update_from_placeholder(weektitle->drawable, placeholder);

    imgutils_calc_rectangle(
        placeholder->x, placeholder->y, placeholder->width, placeholder->height,
        draw_width, draw_height, ALIGN_CENTER, ALIGN_CENTER,
        &draw_width, &draw_height, &draw_x, &draw_y
    );

    sprite_set_texture(weektitle->sprite, sprite_get_texture(title_sprite), false);
    atlas_apply_from_entry(weektitle->sprite, &atlas_entry, false);
    sprite_set_draw_size(weektitle->sprite, draw_width, draw_height);
    sprite_set_draw_location(weektitle->sprite, draw_x, draw_y);
}

void weekselector_weektitle_move_difficult(WeekSelectorWeekTitle weektitle, WeekSelectorDifficult weekdifficult) {
    float draw_width = 0.0f, draw_height = 0.0f;
    float draw_x = 0.0f, draw_y = 0.0f;

    if (weektitle->placeholder1) weektitle->placeholder1->vertex = NULL;
    if (weektitle->placeholder2) weektitle->placeholder2->vertex = NULL;
    if (weektitle->placeholder3) weektitle->placeholder3->vertex = weektitle->drawable;
    drawable_helper_update_from_placeholder(weektitle->drawable, weektitle->placeholder3);

    StateSprite difficult_statesprite = weekselector_difficult_peek_statesprite(weekdifficult);
    StateSpriteState* statesprite_state = statesprite_state_get(difficult_statesprite);
    if (!statesprite_state) {
        sprite_set_texture(weektitle->sprite, NULL, false);
        return;
    }
    imgutils_get_statesprite_original_size(statesprite_state, &draw_width, &draw_height);

    imgutils_calc_rectangle(
        weektitle->placeholder3->x, weektitle->placeholder3->y,
        weektitle->placeholder3->width, weektitle->placeholder3->height,
        draw_width, draw_height,
        weektitle->placeholder3->align_horizontal, weektitle->placeholder3->align_vertical,
        &draw_width, &draw_height, &draw_x, &draw_y
    );

    sprite_set_texture(weektitle->sprite, statesprite_state->texture, false);
    atlas_apply_from_entry(weektitle->sprite, statesprite_state->frame_info, false);
    sprite_set_draw_size(weektitle->sprite, draw_width, draw_height);
    sprite_set_draw_location(weektitle->sprite, draw_x, draw_y);
    sprite_set_alpha(weektitle->sprite, 1.0f);
}
