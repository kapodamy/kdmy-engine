#include "game/funkin/streakcounter.h"

#include "animlist.h"
#include "animsprite.h"
#include "drawable.h"
#include "externals/luascript.h"
#include "game/common/funkin.h"
#include "game/funkin/streakcounter_digits.h"
#include "imgutils.h"
#include "linkedlist.h"
#include "logger.h"
#include "malloc_utils.h"
#include "modelholder.h"
#include "statesprite.h"
#include "stringutils.h"

#define STREAKCOUNTER_STREAK_BUFFER_SIZE 5


typedef struct {
    int32_t id;
    StreakCounterDigits digits;
} NumberItem;

struct StreakCounter_s {
    Drawable drawable;
    Modifier* modifier;
    int32_t last_streak;
    float number_height;
    float number_gap;
    bool ignore_combo_location;
    float combo_width;
    float combo_height;
    StateSprite combo_sprite;
    AnimSprite combo_animation;
    bool combo_enabled;
    NumberItem numbers_items[STREAKCOUNTER_STREAK_BUFFER_SIZE];
    int32_t numbers_id;
};

static const char* STREAKCOUNTER_PREFIX_COMBO = "combo";
static const char* STREAKCOUNTER_UI_COMBO_ANIM = "streak_text"; // picked from UI animlist


static NumberItem* streakcounter_internal_pick_item(NumberItem* array, int32_t size, int32_t new_id);
static int streakcounter_internal_sort(const void* item1_ptr, const void* item2_ptr);


StreakCounter streakcounter_init(LayoutPlaceholder* placeholder, float combo_height, float number_gap, float delay) {
    //
    // Notes:
    //      * the width is optional (should no be present)
    //      * alignments are ignored
    //
    float number_height = 0.0f;
    float reference_width = 0.0f;
    if (placeholder) {
        if (placeholder->height > 0.0f) number_height = placeholder->height;
        if (placeholder->width > 0.0f) reference_width = placeholder->width;
    }

    StreakCounter streakcounter = malloc_chk(sizeof(struct StreakCounter_s));
    malloc_assert(streakcounter, StreakCounter);

    *streakcounter = (struct StreakCounter_s){
        .drawable = NULL,
        //.modifier = NULL,

        .last_streak = -1,

        .number_height = number_height,
        .number_gap = number_gap,

        .ignore_combo_location = true,

        .combo_width = -1,
        .combo_height = combo_height,
        .combo_sprite = statesprite_init_from_texture(NULL),
        .combo_animation = NULL,
        .combo_enabled = combo_height > 0.0f,

        //.numbers_items = (NumberItem[]){},

        .numbers_id = 0
    };

    statesprite_set_visible(streakcounter->combo_sprite, false);

    streakcounter->drawable = drawable_init(0.0f, streakcounter, (DelegateDraw)streakcounter_draw, (DelegateAnimate)streakcounter_animate);

    if (placeholder) {
        placeholder->vertex = streakcounter->drawable;
        drawable_helper_update_from_placeholder(streakcounter->drawable, placeholder);
    }

    streakcounter->modifier = drawable_get_modifier(streakcounter->drawable);
    streakcounter->modifier->x += reference_width / -2.0f;
    streakcounter->modifier->y += number_height / -2.0f;
    streakcounter->modifier->height = math2d_max_float(combo_height, number_height);

    for (int32_t i = 0; i < STREAKCOUNTER_STREAK_BUFFER_SIZE; i++) {
        streakcounter->numbers_items[i] = (NumberItem){.id = -1, .digits = streakcounter_digits_init(delay)};
    }

    return streakcounter;
}

void streakcounter_destroy(StreakCounter* streakcounter_ptr) {
    if (!streakcounter_ptr || !*streakcounter_ptr) return;

    StreakCounter streakcounter = *streakcounter_ptr;

    luascript_drop_shared(streakcounter);

    drawable_destroy(&streakcounter->drawable);

    for (int32_t i = 0; i < STREAKCOUNTER_STREAK_BUFFER_SIZE; i++)
        streakcounter_digits_destroy(&streakcounter->numbers_items[i].digits);

    if (streakcounter->combo_animation) animsprite_destroy(&streakcounter->combo_animation);
    statesprite_destroy(&streakcounter->combo_sprite);

    free_chk(streakcounter);
    *streakcounter_ptr = NULL;
}


bool streakcounter_peek_streak(StreakCounter streakcounter, PlayerStats playerstats) {
    int32_t value = playerstats_get_combo_streak(playerstats);

    if (streakcounter->last_streak == value) return false;

    if (value >= FUNKIN_COMBO_STREAK_VISIBLE_AFTER) {
        NumberItem* unused_item = streakcounter_internal_pick_item(
            streakcounter->numbers_items, STREAKCOUNTER_STREAK_BUFFER_SIZE, streakcounter->numbers_id++
        );

        // center the number sprites in the draw location (y axis is already centered)
        value = math2d_clamp_int(value, 0, STREAKCOUNTER_MAX_VALUE);
        float draw_width = streakcounter_digits_measure(unused_item->digits, value);
        float x = streakcounter->modifier->x;
        float y = streakcounter->modifier->y;

        if (streakcounter->combo_enabled && streakcounter->ignore_combo_location) {
            x -= ((streakcounter->combo_width + streakcounter->number_gap * 2.0f) + draw_width) / 2.0f;
            statesprite_set_draw_location(streakcounter->combo_sprite, x, y);
            x += streakcounter->combo_width;
        } else {
            x += draw_width / -2.0f;
        }

        // set the draw location
        streakcounter_digits_set_draw_location(unused_item->digits, x, y, streakcounter->number_gap);

        qsort(
            streakcounter->numbers_items, STREAKCOUNTER_STREAK_BUFFER_SIZE, sizeof(NumberItem),
            streakcounter_internal_sort
        );

        if (streakcounter->combo_enabled) {
            statesprite_animation_restart(streakcounter->combo_sprite);
            if (streakcounter->combo_animation) animsprite_restart(streakcounter->combo_animation);
        }
    }

    bool streak_loose;
    if (streakcounter->last_streak >= FUNKIN_COMBO_STREAK_VISIBLE_AFTER)
        streak_loose = value < streakcounter->last_streak;
    else
        streak_loose = false;

    streakcounter->last_streak = value;
    return streak_loose;
}

void streakcounter_reset(StreakCounter streakcounter) {
    streakcounter->last_streak = -1;
}

void streakcounter_hide_combo_sprite(StreakCounter streakcounter, bool hide) {
    bool combo_enabled = !hide;
    if (combo_enabled && linkedlist_count(statesprite_state_list(streakcounter->combo_sprite)) < 1) {
        logger_warn("streakcounter_hide_combo_sprite() failed, combo sprite does not have states to show");
        return;
    }
    streakcounter->combo_enabled = combo_enabled;
}

void streakcounter_set_combo_draw_location(StreakCounter streakcounter, float x, float y) {
    streakcounter->ignore_combo_location = false;
    statesprite_set_draw_location(streakcounter->combo_sprite, x, y);
}


int32_t streakcounter_state_add(StreakCounter streakcounter, ModelHolder combo_modelholder, ModelHolder number_modelholder, const char* state_name) {
    float temp_width = 0.0f, temp_height = 0.0f;

    int32_t success = 0;

    for (int32_t i = 0; i < STREAKCOUNTER_STREAK_BUFFER_SIZE; i++) {
        success += streakcounter_digits_state_add(
            streakcounter->numbers_items[i].digits,
            streakcounter->number_height,
            number_modelholder,
            state_name
        );
    }

    if (!combo_modelholder) return success;

    char* animation_name = string_concat_for_state_name(2, STREAKCOUNTER_PREFIX_COMBO, state_name);
    StateSpriteState* statesprite_state = statesprite_state_add(
        streakcounter->combo_sprite, combo_modelholder, animation_name, state_name
    );
    free_chk(animation_name);


    if (statesprite_state) {
        imgutils_get_statesprite_original_size(statesprite_state, &temp_width, &temp_height);
        imgutils_calc_size(temp_width, temp_height, -1.0f, streakcounter->combo_height, &temp_width, &temp_height);
        statesprite_state->draw_width = temp_width;
        statesprite_state->draw_height = temp_height;
        statesprite_state->offset_x = 0.0f;
        statesprite_state->offset_y = temp_height / -2.0f;
        streakcounter->combo_width = temp_width;
        success++;
    }

    return success;
}

bool streakcounter_state_toggle(StreakCounter streakcounter, const char* state_name) {
    for (int32_t i = 0; i < STREAKCOUNTER_STREAK_BUFFER_SIZE; i++)
        streakcounter_digits_state_toggle(streakcounter->numbers_items[i].digits, state_name);

    if (statesprite_state_toggle(streakcounter->combo_sprite, state_name)) {
        streakcounter->combo_width = statesprite_state_get(streakcounter->combo_sprite)->draw_width;
        return true;
    }

    return false;
}


void streakcounter_set_alpha(StreakCounter streakcounter, float alpha) {
    drawable_set_alpha(streakcounter->drawable, alpha);
}

void streakcounter_set_offsetcolor(StreakCounter streakcounter, float r, float g, float b, float a) {
    drawable_set_offsetcolor(streakcounter->drawable, r, g, b, a);
}

void streakcounter_set_offsetcolor_to_default(StreakCounter streakcounter) {
    drawable_set_offsetcolor_to_default(streakcounter->drawable);
}

Modifier* streakcounter_get_modifier(StreakCounter streakcounter) {
    return drawable_get_modifier(streakcounter->drawable);
}

Drawable streakcounter_get_drawable(StreakCounter streakcounter) {
    return streakcounter->drawable;
}


void streakcounter_set_number_animation(StreakCounter streakcounter, AnimList animlist) {
    if (!animlist) return;

    const AnimListItem* animlist_item = animlist_get_animation(animlist, STREAKCOUNTER_UI_STREAK_ANIM);
    if (!animlist_item) return;

    for (int32_t i = 0; i < STREAKCOUNTER_STREAK_BUFFER_SIZE; i++)
        streakcounter_digits_animation_set(streakcounter->numbers_items[i].digits, animlist_item);
}

void streakcounter_set_combo_animation(StreakCounter streakcounter, AnimList animlist) {
    if (!animlist) return;
    const AnimListItem* animlist_item = animlist_get_animation(animlist, STREAKCOUNTER_UI_COMBO_ANIM);

    if (!animlist_item) return;
    streakcounter->combo_animation = animsprite_init(animlist_item);
}


void streakcounter_animation_set(StreakCounter streakcounter, AnimSprite animsprite) {
    for (int32_t i = 0; i < STREAKCOUNTER_STREAK_BUFFER_SIZE; i++)
        streakcounter_digits_animation_pair_set(streakcounter->numbers_items[i].digits, animsprite);
}

void streakcounter_animation_restart(StreakCounter streakcounter) {
    for (int32_t i = 0; i < STREAKCOUNTER_STREAK_BUFFER_SIZE; i++)
        streakcounter_digits_animation_restart(streakcounter->numbers_items[i].digits);


    statesprite_animation_restart(streakcounter->combo_sprite);
    if (streakcounter->combo_animation) animsprite_restart(streakcounter->combo_animation);
}

void streakcounter_animation_end(StreakCounter streakcounter) {
    for (int32_t i = 0; i < STREAKCOUNTER_STREAK_BUFFER_SIZE; i++)
        streakcounter_digits_animation_end(streakcounter->numbers_items[i].digits);

    statesprite_animation_end(streakcounter->combo_sprite);
    if (streakcounter->combo_animation)
        animsprite_force_end3(streakcounter->combo_animation, streakcounter->combo_sprite);
}


int32_t streakcounter_animate(StreakCounter streakcounter, float elapsed) {
    int32_t res = 0;

    for (int32_t i = 0; i < STREAKCOUNTER_STREAK_BUFFER_SIZE; i++) {
        if (streakcounter->numbers_items[i].id < 0) continue;
        if (streakcounter_digits_animate(streakcounter->numbers_items[i].digits, elapsed)) {
            res++;
            streakcounter->numbers_items[i].id = -1; // animation completed, hide
        }
    }

    res += statesprite_animate(streakcounter->combo_sprite, elapsed);
    if (streakcounter->combo_animation) {
        res += animsprite_animate(streakcounter->combo_animation, elapsed);
        animsprite_update_statesprite(streakcounter->combo_animation, streakcounter->combo_sprite, true);
    }

    return res;
}

void streakcounter_draw(StreakCounter streakcounter, PVRContext pvrctx) {
    if (streakcounter->last_streak < FUNKIN_COMBO_STREAK_VISIBLE_AFTER) return;

    pvr_context_save(pvrctx);
    drawable_helper_apply_in_context(streakcounter->drawable, pvrctx);

    bool draw_combo = streakcounter->combo_enabled;

    for (int32_t i = 0; i < STREAKCOUNTER_STREAK_BUFFER_SIZE; i++) {
        if (streakcounter->numbers_items[i].id < 0) continue;

        if (draw_combo) {
            draw_combo = false;
            statesprite_draw(streakcounter->combo_sprite, pvrctx);
        }

        streakcounter_digits_draw(streakcounter->numbers_items[i].digits, pvrctx);
    }

    pvr_context_restore(pvrctx);
}



static NumberItem* streakcounter_internal_pick_item(NumberItem* array, int32_t size, int32_t new_id) {
    for (int32_t i = 0; i < size; i++) {
        if (array[i].id < 0) {
            array[i].id = new_id;
            return &array[i];
        }
    }

    int32_t oldest_id = MATH2D_MAX_INT32;
    NumberItem* oldest_item = NULL;

    for (int32_t i = 0; i < size; i++) {
        if (array[i].id < oldest_id) {
            oldest_id = array[i].id;
            oldest_item = &array[i];
        }
    }

    assert(oldest_item != NULL);
    oldest_item->id = new_id;
    return oldest_item;
}

static int streakcounter_internal_sort(const void* item1_ptr, const void* item2_ptr) {
    const NumberItem* item1 = (const NumberItem*)item1_ptr;
    const NumberItem* item2 = (const NumberItem*)item2_ptr;

    return item1->id - item2->id;
}
