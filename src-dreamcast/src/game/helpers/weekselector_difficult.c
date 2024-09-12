#include "game/helpers/weekselector_difficult.h"

#include "animlist.h"
#include "drawable.h"
#include "game/common/funkin.h"
#include "game/common/funkinsave.h"
#include "game/weekselector.h"
#include "imgutils.h"
#include "layout.h"
#include "malloc_utils.h"
#include "modelholder.h"
#include "statesprite.h"
#include "stringutils.h"


typedef struct {
    const char* name;
    bool is_locked;
    bool is_common;
} Item;

struct WeekSelectorDifficult_s {
    AnimSprite animsprite;
    int32_t index;
    Item* list;
    int32_t list_size;
    StateSprite sprite_commons;
    StateSprite sprite_customs;
    Sprite icon_locked;
    Sprite icon_left;
    Sprite icon_right;
    LayoutPlaceholder* placeholder1;
    LayoutPlaceholder* placeholder2;
    bool has_left;
    bool has_right;
    bool is_common;
    bool is_locked;
    Drawable drawable;
};


static const char* WEEKSELECTOR_DIFFICULTY_UI_NO_WARNS = "ui_difficult_no_warns";
static const char* WEEKSELECTOR_DIFFICULTY_UI_WARNS = "ui_difficult_warns";


WeekSelectorDifficult weekselector_difficult_init(AnimList animlist, ModelHolder modelholder, Layout layout) {
    WeekSelectorDifficult weekdifficult = malloc_chk(sizeof(struct WeekSelectorDifficult_s));
    malloc_assert(weekdifficult, WeekSelectorDifficult);

    *weekdifficult = (struct WeekSelectorDifficult_s){
        .animsprite = animsprite_init_from_animlist(animlist, WEEKSELECTOR_ARROW_SPRITE_NAME),

        .index = -1,
        .list = NULL,
        .list_size = 0,

        .sprite_commons = statesprite_init_from_texture(NULL),
        .sprite_customs = statesprite_init_from_texture(NULL),

        .icon_locked = modelholder_create_sprite(modelholder, WEEKSELECTOR_LOCKED, true),
        .icon_left = modelholder_create_sprite(modelholder, WEEKSELECTOR_ARROW_SPRITE_NAME, true),
        .icon_right = modelholder_create_sprite(modelholder, WEEKSELECTOR_ARROW_SPRITE_NAME, true),

        .placeholder1 = layout_get_placeholder(layout, WEEKSELECTOR_DIFFICULTY_UI_NO_WARNS),
        .placeholder2 = layout_get_placeholder(layout, WEEKSELECTOR_DIFFICULTY_UI_WARNS),

        .has_left = true,
        .has_right = true,
        .is_common = false,
        .is_locked = false,

        .drawable = NULL
    };

    sprite_flip_rendered_texture(weekdifficult->icon_right, true, false);

    weekdifficult->drawable = drawable_init(
        -1.0f, weekdifficult, (DelegateDraw)weekselector_difficult_draw, (DelegateAnimate)weekselector_difficult_animate
    );
    weekselector_difficult_visible(weekdifficult, true);

    statesprite_set_visible(weekdifficult->sprite_commons, false);
    statesprite_set_visible(weekdifficult->sprite_customs, false);

    sprite_set_visible(weekdifficult->icon_locked, false);
    sprite_set_visible(weekdifficult->icon_left, false);
    sprite_set_visible(weekdifficult->icon_right, false);

    // prebuild the list
    statesprite_state_add(
        weekdifficult->sprite_commons, modelholder, FUNKIN_DIFFICULT_EASY, FUNKIN_DIFFICULT_EASY
    );
    statesprite_state_add(
        weekdifficult->sprite_commons, modelholder, FUNKIN_DIFFICULT_NORMAL, FUNKIN_DIFFICULT_NORMAL
    );
    statesprite_state_add(
        weekdifficult->sprite_commons, modelholder, FUNKIN_DIFFICULT_HARD, FUNKIN_DIFFICULT_HARD
    );

    return weekdifficult;
}

void weekselector_difficult_destroy(WeekSelectorDifficult* weekdifficult_ptr) {
    if (!weekdifficult_ptr || !*weekdifficult_ptr) return;

    WeekSelectorDifficult weekdifficult = *weekdifficult_ptr;

    if (weekdifficult->animsprite) animsprite_destroy(&weekdifficult->animsprite);

    drawable_destroy(&weekdifficult->drawable);

    statesprite_destroy(&weekdifficult->sprite_commons);
    statesprite_destroy(&weekdifficult->sprite_customs);

    sprite_destroy_full(&weekdifficult->icon_locked);
    sprite_destroy_full(&weekdifficult->icon_left);
    sprite_destroy_full(&weekdifficult->icon_right);

    // do not disposed the names on the list
    free_chk(weekdifficult->list);

    free_chk(weekdifficult);
    *weekdifficult_ptr = NULL;
}


void weekselector_difficult_draw(WeekSelectorDifficult weekdifficult, PVRContext pvrctx) {
    pvr_context_save(pvrctx);

    if (weekdifficult->is_locked) sprite_draw(weekdifficult->icon_locked, pvrctx);
    sprite_draw(weekdifficult->icon_left, pvrctx);
    sprite_draw(weekdifficult->icon_right, pvrctx);

    if (weekdifficult->is_locked)
        pvr_context_set_global_alpha(pvrctx, WEEKSELECTOR_PREVIEW_DISABLED_ALPHA);

    if (weekdifficult->is_common)
        statesprite_draw(weekdifficult->sprite_commons, pvrctx);
    else
        statesprite_draw(weekdifficult->sprite_customs, pvrctx);

    if (weekdifficult->is_locked) {
        pvr_context_set_global_alpha(pvrctx, 1.0f);
        sprite_draw(weekdifficult->icon_locked, pvrctx);
    }

    pvr_context_restore(pvrctx);
}

int32_t weekselector_difficult_animate(WeekSelectorDifficult weekdifficult, float elapsed) {
    if (weekdifficult->animsprite) animsprite_animate(weekdifficult->animsprite, elapsed);

    sprite_animate(weekdifficult->icon_left, elapsed);
    sprite_animate(weekdifficult->icon_right, elapsed);

    statesprite_animate(weekdifficult->sprite_commons, elapsed);
    statesprite_animate(weekdifficult->sprite_customs, elapsed);

    if (weekdifficult->has_left)
        animsprite_update_sprite(weekdifficult->animsprite, weekdifficult->icon_left, false);
    if (weekdifficult->has_right)
        animsprite_update_sprite(weekdifficult->animsprite, weekdifficult->icon_right, true);

    return 0;
}


bool weekselector_difficult_select(WeekSelectorDifficult weekdifficult, int32_t new_index) {
    if (new_index < 0 || new_index >= weekdifficult->list_size) return false;

    weekdifficult->is_common = weekdifficult->list[new_index].is_common;
    weekdifficult->is_locked = weekdifficult->list[new_index].is_locked;
    weekdifficult->index = new_index;
    weekdifficult->has_left = new_index > 0;
    weekdifficult->has_right = new_index < (weekdifficult->list_size - 1);

    statesprite_state_toggle(
        weekdifficult->is_common ? weekdifficult->sprite_commons : weekdifficult->sprite_customs,
        weekdifficult->list[new_index].name
    );
    sprite_set_alpha(
        weekdifficult->icon_left, weekdifficult->has_left ? 1.0f : WEEKSELECTOR_ARROW_DISABLED_ALPHA
    );
    sprite_set_alpha(
        weekdifficult->icon_right, weekdifficult->has_right ? 1.0f : WEEKSELECTOR_ARROW_DISABLED_ALPHA
    );

    return true;
}

void weekselector_difficult_select_default(WeekSelectorDifficult weekdifficult) {
    for (int32_t i = 0; i < weekdifficult->list_size; i++) {
        if (string_equals(weekdifficult->list[i].name, FUNKIN_DIFFICULT_NORMAL)) {
            weekselector_difficult_select(weekdifficult, i);
            return;
        }
    }
    weekselector_difficult_select(weekdifficult, 0);
}

bool weekselector_difficult_scroll(WeekSelectorDifficult weekdifficult, int32_t offset) {
    return weekselector_difficult_select(weekdifficult, weekdifficult->index + offset);
}

void weekselector_difficult_visible(WeekSelectorDifficult weekdifficult, bool visible) {
    drawable_set_visible(weekdifficult->drawable, visible);
}

const char* weekselector_difficult_get_selected(WeekSelectorDifficult weekdifficult) {
    if (weekdifficult->index < 0 || weekdifficult->index >= weekdifficult->list_size) return NULL;
    Item* difficult = &weekdifficult->list[weekdifficult->index];
    return difficult->is_locked ? NULL : difficult->name;
}

StateSprite weekselector_difficult_peek_statesprite(WeekSelectorDifficult weekdifficult) {
    if (weekdifficult->index < 0 || weekdifficult->index >= weekdifficult->list_size) return NULL;

    if (weekdifficult->list[weekdifficult->index].is_common)
        return weekdifficult->sprite_commons;
    else
        return weekdifficult->sprite_customs;
}

void weekselector_difficult_relayout(WeekSelectorDifficult weekdifficult, bool use_alt_placeholder) {
    LayoutPlaceholder* old_placeholder = use_alt_placeholder ? weekdifficult->placeholder1 : weekdifficult->placeholder2;
    LayoutPlaceholder* placeholder = use_alt_placeholder ? weekdifficult->placeholder2 : weekdifficult->placeholder1;
    if (!placeholder) return;

    if (old_placeholder) old_placeholder->vertex = NULL;
    placeholder->vertex = weekdifficult->drawable;
    drawable_helper_update_from_placeholder(weekdifficult->drawable, placeholder);

    // resize&locate left arrow
    imgutils_calc_rectangle_in_sprite(
        placeholder->x, placeholder->y, placeholder->width, placeholder->height,
        ALIGN_START, ALIGN_CENTER, weekdifficult->icon_left
    );

    // resize&locate right arrow
    imgutils_calc_rectangle_in_sprite(
        placeholder->x, placeholder->y, placeholder->width, placeholder->height,
        ALIGN_END, ALIGN_CENTER, weekdifficult->icon_right
    );

    // calculte the horizontal space between arrows
    float draw_width = 0.0f, draw_height = 0.0f;
    sprite_get_draw_size(weekdifficult->icon_left, &draw_width, &draw_height);
    float difficult_width = placeholder->width - (draw_width * 2.0f);

    // resize&locate commons difficults
    foreach (StateSpriteState*, state, LINKEDLIST_ITERATOR, statesprite_state_list(weekdifficult->sprite_commons)) {
        imgutils_calc_rectangle_in_statesprite_state(
            draw_width, 0.0f, difficult_width, placeholder->height, ALIGN_CENTER, ALIGN_CENTER, state
        );
    }
    statesprite_set_draw_location(weekdifficult->sprite_commons, placeholder->x, placeholder->y);
    statesprite_state_apply(weekdifficult->sprite_commons, NULL);

    // resize&locate custom difficults
    foreach (StateSpriteState*, state, LINKEDLIST_ITERATOR, statesprite_state_list(weekdifficult->sprite_customs)) {
        imgutils_calc_rectangle_in_statesprite_state(
            draw_width, 0.0f, difficult_width, placeholder->height, ALIGN_CENTER, ALIGN_CENTER, state
        );
    }
    statesprite_set_draw_location(weekdifficult->sprite_customs, placeholder->x, placeholder->y);
    statesprite_state_apply(weekdifficult->sprite_customs, NULL);

    // resize&locate locked icon
    imgutils_calc_rectangle_in_sprite(
        placeholder->x, placeholder->y, placeholder->width, placeholder->height,
        ALIGN_CENTER, ALIGN_CENTER, weekdifficult->icon_locked
    );
}

bool weekselector_difficult_selected_is_locked(WeekSelectorDifficult weekdifficult) {
    if (weekdifficult->index < 0 || weekdifficult->index >= weekdifficult->list_size) return true;
    return weekdifficult->list[weekdifficult->index].is_locked;
}


void weekselector_difficult_load(WeekSelectorDifficult weekdifficult, WeekInfo* weekinfo, const char* default_difficult) {
    if (weekdifficult->animsprite) animsprite_restart(weekdifficult->animsprite);

    // dispose previous list and custom difficults
    free_chk(weekdifficult->list);
    statesprite_destroy(&weekdifficult->sprite_customs);
    weekdifficult->sprite_customs = statesprite_init_from_texture(NULL);
    statesprite_set_visible(weekdifficult->sprite_customs, false);

    if (!weekinfo) {
        weekdifficult->list = NULL;
        return;
    }

    weekdifficult->list_size = 3;
    if (weekinfo->custom_difficults_model) weekdifficult->list_size += weekinfo->custom_difficults_size;

    int32_t index = 0;
    weekdifficult->list = malloc_for_array(Item, weekdifficult->list_size);

    if (weekinfo->has_difficulty_easy)
        weekdifficult->list[index++] = (Item){.name = FUNKIN_DIFFICULT_EASY, .is_locked = false, .is_common = true};
    if (weekinfo->has_difficulty_normal)
        weekdifficult->list[index++] = (Item){.name = FUNKIN_DIFFICULT_NORMAL, .is_locked = false, .is_common = true};
    if (weekinfo->has_difficulty_hard)
        weekdifficult->list[index++] = (Item){.name = FUNKIN_DIFFICULT_HARD, .is_locked = false, .is_common = true};

    if (weekinfo->custom_difficults_model) {
        ModelHolder modelholder = modelholder_init(weekinfo->custom_difficults_model);

        if (modelholder) {
            for (int32_t i = 0; i < weekinfo->custom_difficults_size; i++) {
                WeekInfoCustomDifficult* difficult = &weekinfo->custom_difficults[i];
                weekdifficult->list[index++] = (Item){
                    .name = difficult->name,
                    .is_locked = !funkinsave_contains_unlock_directive(difficult->unlock_directive),
                    .is_common = false
                };
                statesprite_state_add(
                    weekdifficult->sprite_customs, modelholder, difficult->name, difficult->name
                );
            }
            modelholder_destroy(&modelholder);
        }
    }

    assert(index <= weekdifficult->list_size);
    weekdifficult->list_size = index; // trim the list

    weekselector_difficult_relayout(weekdifficult, false);

    // select default difficult
    const char* selected = default_difficult == NULL ? FUNKIN_DIFFICULT_NORMAL : default_difficult;
    const char* last_difficult_played = funkinsave_get_last_played_difficult(); // read-only string
    if (!default_difficult && last_difficult_played) selected = last_difficult_played;

    // select normal difficult
    for (int32_t i = 0; i < weekdifficult->list_size; i++) {
        if (string_equals(weekdifficult->list[i].name, selected)) {
            weekselector_difficult_select(weekdifficult, i);
            return;
        }
    }

    // fallback to first one
    weekselector_difficult_select(weekdifficult, 0);
}