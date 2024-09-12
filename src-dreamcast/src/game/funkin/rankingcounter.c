#include "game/funkin/rankingcounter.h"

#include "externals/luascript.h"
#include "game/funkin/playerstats.h"
#include "imgutils.h"
#include "malloc_utils.h"
#include "math2d.h"
#include "statesprite.h"
#include "stringutils.h"
#include "textsprite.h"
#include "drawable.h"


#define RANKINGCOUNTER_RANKING_BUFFER_SIZE 5


typedef struct {
    StateSprite statesprite;
    AnimSprite animsprite;
    int32_t id;
} RankingItem;

struct RankingCounter_s {
    int32_t last_iterations;
    TextSprite textsprite;
    bool show_accuracy;
    bool enable_accuracy;
    bool enable_accuracy_percent;
    char* selected_state;
    AnimSprite drawable_animation;
    RankingItem ranking_items[RANKINGCOUNTER_RANKING_BUFFER_SIZE];
    int32_t ranking_id;
    float ranking_height;
    Drawable drawable_accuracy;
    Drawable drawable_rank;
    float correction_x;
    float correction_y;
};


static const char* RANKINGCOUNTER_PREFIX_SHIT = "shit";
static const char* RANKINGCOUNTER_PREFIX_BAD = "bad";
static const char* RANKINGCOUNTER_PREFIX_GOOD = "good";
static const char* RANKINGCOUNTER_PREFIX_SICK = "sick";
static const char* RANKINGCOUNTER_PREFIX_MISS = "miss";
static const char* RANKINGCOUNTER_PREFIX_PENALITY = "penality";

static const uint32_t RANKINGCOUNTER_TEXT_COLOR_SHIT = 0xFF0000;     // red
static const uint32_t RANKINGCOUNTER_TEXT_COLOR_BAD = 0xFF0000;      // red
static const uint32_t RANKINGCOUNTER_TEXT_COLOR_GOOD = 0x00FF00;     // green
static const uint32_t RANKINGCOUNTER_TEXT_COLOR_SICK = 0x00FFFF;     // cyan
static const uint32_t RANKINGCOUNTER_TEXT_COLOR_MISS = 0x151B54;     // midnight blue
static const uint32_t RANKINGCOUNTER_TEXT_COLOR_PENALITY = 0x000000; // black (not implemented)

static const char* RANKINGCOUNTER_FORMAT_TIME = "$2dms";                 // prints 12.345 as "12.34ms"
static const char* RANKINGCOUNTER_FORMAT_PERCENT = "$0d%";               // prints 99.7899 as "99%"
static const char* RANKINGCOUNTER_UI_RANKING_ANIM = "ranking";           // picked from UI animlist
static const char* RANKINGCOUNTER_UI_RANKING_TEXT_ANIM = "ranking_text"; // picked from UI animlist

static const char* RANKINGCOUNTER_TEXT_MISS = "MISS";

static const char* RANKINGCOUNTER_INTERNAL_STATE_NAME = "rankingcounter-state";


static int32_t rankingcounter_internal_add_state(StateSprite statesprite, float max_height, ModelHolder modelholder, const char* prefix, const char* state_name);
static RankingItem* rankingcounter_internal_pick_item(RankingItem* array, int32_t new_id);
static int rankingcounter_internal_sort(const void* item1_ptr, const void* item2_ptr);


RankingCounter rankingcounter_init(LayoutPlaceholder* plchldr_rank, LayoutPlaceholder* plchldr_accuracy, FontHolder fnthldr) {
    //
    // Notes:
    //      * the width is optional (should no be present)
    //      * alignments are ignored
    //      * the font size is calculated for the screen to avoid huge font characters in VRAM
    //
    float ranking_height = 0.0f;
    if (plchldr_rank && plchldr_rank->height > 0.0f) ranking_height = plchldr_rank->height;

    float font_size = 20.0f;
    if (plchldr_accuracy) {
        if (plchldr_accuracy->height > 0.0f) {
            font_size = plchldr_accuracy->height;
        } else {
            // dismiss
            plchldr_accuracy = NULL;
        }
    }

    RankingCounter rankingcounter = malloc_chk(sizeof(struct RankingCounter_s));
    malloc_assert(rankingcounter, RankingCounter);

    *rankingcounter = (struct RankingCounter_s){
        .last_iterations = 0,

        .textsprite = textsprite_init2(fnthldr, font_size, 0x000000),
        .show_accuracy = false,
        .enable_accuracy = true,
        .enable_accuracy_percent = false,

        .selected_state = (char*)RANKINGCOUNTER_INTERNAL_STATE_NAME,
        .drawable_animation = NULL,

        .ranking_items = {},

        .ranking_id = 0,
        .ranking_height = ranking_height,

        .drawable_accuracy = NULL,
        .drawable_rank = NULL,

        .correction_x = 0.0f,
        .correction_y = 0.0f
    };

    rankingcounter->drawable_rank = drawable_init(
        -1.0f, rankingcounter, (DelegateDraw)rankingcounter_draw1, (DelegateAnimate)rankingcounter_animate1
    );
    rankingcounter->drawable_accuracy = drawable_init(
        -1.0f, rankingcounter, (DelegateDraw)rankingcounter_draw2, (DelegateAnimate)rankingcounter_animate2
    );

    if (plchldr_rank) {
        drawable_helper_update_from_placeholder(rankingcounter->drawable_rank, plchldr_rank);
        plchldr_rank->vertex = rankingcounter->drawable_rank;

        float x = plchldr_rank->x;
        if (plchldr_rank->width > 0.0f) x -= plchldr_rank->width / 2.0f;

        for (int32_t i = 0; i < RANKINGCOUNTER_RANKING_BUFFER_SIZE; i++) {
            StateSprite statesprite = statesprite_init_from_texture(NULL);
            statesprite_set_draw_location(statesprite, x, plchldr_rank->y);
            statesprite_set_visible(statesprite, false);

            rankingcounter->ranking_items[i] = (RankingItem){
                .statesprite = statesprite,
                .animsprite = NULL,
                .id = -1,
            };
        }
    } else {
        for (int32_t i = 0; i < RANKINGCOUNTER_RANKING_BUFFER_SIZE; i++) {
            StateSprite statesprite = statesprite_init_from_texture(NULL);
            rankingcounter->ranking_items[i] = (RankingItem){
                .statesprite = statesprite,
                .animsprite = NULL,
                .id = -1,
            };
        }
    }

    if (plchldr_accuracy) {
        drawable_helper_update_from_placeholder(rankingcounter->drawable_accuracy, plchldr_accuracy);
        plchldr_accuracy->vertex = rankingcounter->drawable_accuracy;

        float x = plchldr_accuracy->x;
        if (plchldr_accuracy->width > 0.0f) x -= plchldr_accuracy->width / 2.0f;

        textsprite_set_draw_location(
            rankingcounter->textsprite, x, plchldr_accuracy->y
        );
        rankingcounter->correction_x = x;
        rankingcounter->correction_y = plchldr_accuracy->y;

        textsprite_set_align(rankingcounter->textsprite, ALIGN_CENTER, ALIGN_CENTER);

        // center the accuracy text on the draw location
        textsprite_set_max_draw_size(rankingcounter->textsprite, 0.0f, 0.0f);
    }

    return rankingcounter;
}

void rankingcounter_destroy(RankingCounter* rankingcounter_ptr) {
    if (!rankingcounter_ptr || !*rankingcounter_ptr) return;

    RankingCounter rankingcounter = *rankingcounter_ptr;

    luascript_drop_shared(rankingcounter);

    // destroy the attached animation of textsprite
    AnimSprite old_animation = textsprite_animation_set(rankingcounter->textsprite, NULL);
    if (old_animation) animsprite_destroy(&old_animation);

    textsprite_destroy(&rankingcounter->textsprite);
    drawable_destroy(&rankingcounter->drawable_rank);
    drawable_destroy(&rankingcounter->drawable_accuracy);
    if (rankingcounter->drawable_animation) animsprite_destroy(&rankingcounter->drawable_animation);

    for (int32_t i = 0; i < RANKINGCOUNTER_RANKING_BUFFER_SIZE; i++) {
        if (rankingcounter->ranking_items[i].animsprite)
            animsprite_destroy(&rankingcounter->ranking_items[i].animsprite);
        if (rankingcounter->ranking_items[i].statesprite)
            statesprite_destroy(&rankingcounter->ranking_items[i].statesprite);
    }

    if (/*pointer equals*/ rankingcounter->selected_state != RANKINGCOUNTER_INTERNAL_STATE_NAME)
        free_chk(rankingcounter->selected_state);

    free_chk(rankingcounter);
    *rankingcounter_ptr = NULL;
}


int32_t rankingcounter_add_state(RankingCounter rankingcounter, ModelHolder modelholder, const char* state_name) {
    const float max_height = rankingcounter->ranking_height;

    int32_t success = 0;
    for (int32_t i = 0; i < RANKINGCOUNTER_RANKING_BUFFER_SIZE; i++) {
        StateSprite statesprite = rankingcounter->ranking_items[i].statesprite;

        success += rankingcounter_internal_add_state(
            statesprite, max_height, modelholder, RANKINGCOUNTER_PREFIX_SHIT, state_name
        );
        success += rankingcounter_internal_add_state(
            statesprite, max_height, modelholder, RANKINGCOUNTER_PREFIX_BAD, state_name
        );
        success += rankingcounter_internal_add_state(
            statesprite, max_height, modelholder, RANKINGCOUNTER_PREFIX_GOOD, state_name
        );
        success += rankingcounter_internal_add_state(
            statesprite, max_height, modelholder, RANKINGCOUNTER_PREFIX_SICK, state_name
        );
        success += rankingcounter_internal_add_state(
            statesprite, max_height, modelholder, RANKINGCOUNTER_PREFIX_MISS, state_name
        );
        success += rankingcounter_internal_add_state(
            statesprite, max_height, modelholder, RANKINGCOUNTER_PREFIX_PENALITY, state_name
        );
    }

    return success / RANKINGCOUNTER_RANKING_BUFFER_SIZE;
}

void rankingcounter_toggle_state(RankingCounter rankingcounter, const char* state_name) {
    if (/*pointer equals*/ rankingcounter->selected_state != RANKINGCOUNTER_INTERNAL_STATE_NAME)
        free_chk(rankingcounter->selected_state);

    rankingcounter->selected_state = string_duplicate(state_name);
}


void rankingcounter_peek_ranking(RankingCounter rankingcounter, PlayerStats playerstats) {
    float64 value;
    const char* format;
    const char* ranking;
    uint color;

    int32_t interations = playerstats_get_iterations(playerstats);
    if (interations == rankingcounter->last_iterations) return;
    rankingcounter->last_iterations = interations;

    Ranking rank = playerstats_get_last_ranking(playerstats);

    switch (rank) {
        case Ranking_NONE:
            return;
        case Ranking_SICK:
            ranking = RANKINGCOUNTER_PREFIX_SICK;
            color = RANKINGCOUNTER_TEXT_COLOR_SICK;
            break;
        case Ranking_GOOD:
            ranking = RANKINGCOUNTER_PREFIX_GOOD;
            color = RANKINGCOUNTER_TEXT_COLOR_GOOD;
            break;
        case Ranking_BAD:
            ranking = RANKINGCOUNTER_PREFIX_BAD;
            color = RANKINGCOUNTER_TEXT_COLOR_BAD;
            break;
        case Ranking_SHIT:
            ranking = RANKINGCOUNTER_PREFIX_SHIT;
            color = RANKINGCOUNTER_TEXT_COLOR_SHIT;
            break;
        case Ranking_MISS:
            ranking = RANKINGCOUNTER_PREFIX_MISS;
            color = RANKINGCOUNTER_TEXT_COLOR_MISS;
            break;
        case Ranking_PENALITY:
            ranking = RANKINGCOUNTER_PREFIX_PENALITY;
            color = RANKINGCOUNTER_TEXT_COLOR_PENALITY;
            break;
        default:
            return;
    }

    // find an unused item
    RankingItem* item = rankingcounter_internal_pick_item(
        rankingcounter->ranking_items, rankingcounter->ranking_id++
    );

    // toggle state for this item
    char* state_name = string_concat_for_state_name(2, ranking, rankingcounter->selected_state);
    if (statesprite_state_toggle(item->statesprite, state_name)) {
        statesprite_animation_restart(item->statesprite);
        if (item->animsprite) animsprite_restart(item->animsprite);

        // sort visible items (old items first)
        qsort(
            rankingcounter->ranking_items,
            RANKINGCOUNTER_RANKING_BUFFER_SIZE, sizeof(RankingItem),
            rankingcounter_internal_sort
        );
    } else {
        // no state for this item, hide it
        item->id = -1;
    }
    free_chk(state_name);

    if (!rankingcounter->enable_accuracy || rank == Ranking_PENALITY) return;

    if (rankingcounter->enable_accuracy_percent) {
        value = playerstats_get_last_accuracy(playerstats);
        format = RANKINGCOUNTER_FORMAT_PERCENT;
    } else {
        value = playerstats_get_last_difference(playerstats);
        format = RANKINGCOUNTER_FORMAT_TIME;
    }

    if (math2d_is_double_NaN(value)) return;

    rankingcounter->show_accuracy = true;
    textsprite_animation_restart(rankingcounter->textsprite);
    textsprite_set_color_rgba8(rankingcounter->textsprite, color);

    if (rank == Ranking_MISS)
        textsprite_set_text_intern(rankingcounter->textsprite, true, &RANKINGCOUNTER_TEXT_MISS);
    else
        textsprite_set_text_formated(rankingcounter->textsprite, format, value);
}

void rankingcounter_reset(RankingCounter rankingcounter) {
    rankingcounter->show_accuracy = false;
    rankingcounter->last_iterations = 0;

    drawable_set_antialiasing(rankingcounter->drawable_accuracy, PVRCTX_FLAG_DEFAULT);
    drawable_set_antialiasing(rankingcounter->drawable_rank, PVRCTX_FLAG_DEFAULT);

    rankingcounter_set_offsetcolor_to_default(rankingcounter);

    pvr_context_helper_clear_modifier(drawable_get_modifier(rankingcounter->drawable_accuracy));
    pvr_context_helper_clear_modifier(drawable_get_modifier(rankingcounter->drawable_rank));

    // hide all visible ranking items
    for (int32_t i = 0; i < RANKINGCOUNTER_RANKING_BUFFER_SIZE; i++)
        rankingcounter->ranking_items[i].id = -1;
}

void rankingcounter_hide_accuracy(RankingCounter rankingcounter, bool hide) {
    rankingcounter->enable_accuracy = hide;
}

void rankingcounter_use_percent_instead(RankingCounter rankingcounter, bool use_accuracy_percenter) {
    rankingcounter->enable_accuracy_percent = use_accuracy_percenter;
}

void rankingcounter_set_default_ranking_animation(RankingCounter rankingcounter, AnimList animlist) {
    if (!animlist) return;
    const AnimListItem* animlist_item = animlist_get_animation(animlist, RANKINGCOUNTER_UI_RANKING_ANIM);

    if (!animlist_item) return;
    AnimSprite animsprite = animsprite_init(animlist_item);

    rankingcounter_set_default_ranking_animation2(rankingcounter, animsprite);
    if (animsprite) animsprite_destroy(&animsprite);
}

void rankingcounter_set_default_ranking_animation2(RankingCounter rankingcounter, AnimSprite animsprite) {
    for (int32_t i = 0; i < RANKINGCOUNTER_RANKING_BUFFER_SIZE; i++) {
        if (rankingcounter->ranking_items[i].animsprite)
            animsprite_destroy(&rankingcounter->ranking_items[i].animsprite);
        rankingcounter->ranking_items[i].animsprite = animsprite ? animsprite_clone(animsprite) : NULL;
    }
}


void rankingcounter_set_default_ranking_text_animation(RankingCounter rankingcounter, AnimList animlist) {
    if (!animlist) return;
    const AnimListItem* animlist_item = animlist_get_animation(animlist, RANKINGCOUNTER_UI_RANKING_TEXT_ANIM);

    if (!animlist_item) return;
    AnimSprite animsprite = animsprite_init(animlist_item);

    AnimSprite old_animation = textsprite_animation_set(rankingcounter->textsprite, animsprite);
    if (old_animation) animsprite_destroy(&old_animation);
}

void rankingcounter_set_default_ranking_text_animation2(RankingCounter rankingcounter, AnimSprite animsprite) {
    AnimSprite old_animation = textsprite_animation_set(
        rankingcounter->textsprite, animsprite ? animsprite_clone(animsprite) : NULL
    );
    if (old_animation) animsprite_destroy(&old_animation);
}



void rankingcounter_set_alpha(RankingCounter rankingcounter, float alpha) {
    drawable_set_alpha(rankingcounter->drawable_accuracy, alpha);
    drawable_set_alpha(rankingcounter->drawable_rank, alpha);
}

void rankingcounter_set_offsetcolor(RankingCounter rankingcounter, float r, float g, float b, float a) {
    drawable_set_offsetcolor(rankingcounter->drawable_accuracy, r, g, b, a);
    drawable_set_offsetcolor(rankingcounter->drawable_rank, r, g, b, a);
}

void rankingcounter_set_offsetcolor_to_default(RankingCounter rankingcounter) {
    drawable_set_offsetcolor_to_default(rankingcounter->drawable_accuracy);
    drawable_set_offsetcolor_to_default(rankingcounter->drawable_rank);
}



void rankingcounter_animation_set(RankingCounter rankingcounter, AnimSprite animsprite) {
    if (rankingcounter->drawable_animation) animsprite_destroy(&rankingcounter->drawable_animation);
    rankingcounter->drawable_animation = animsprite ? animsprite_clone(animsprite) : NULL;
}

void rankingcounter_animation_restart(RankingCounter rankingcounter) {
    for (int32_t i = 0; i < RANKINGCOUNTER_RANKING_BUFFER_SIZE; i++) {
        if (rankingcounter->ranking_items[i].animsprite)
            animsprite_restart(rankingcounter->ranking_items[i].animsprite);
        statesprite_animation_restart(rankingcounter->ranking_items[i].statesprite);
    }

    textsprite_animation_restart(rankingcounter->textsprite);

    if (rankingcounter->drawable_animation)
        animsprite_restart(rankingcounter->drawable_animation);
}

void rankingcounter_animation_end(RankingCounter rankingcounter) {
    for (int32_t i = 0; i < RANKINGCOUNTER_RANKING_BUFFER_SIZE; i++)
        rankingcounter->ranking_items[i].id = -1;

    textsprite_animation_end(rankingcounter->textsprite);

    if (rankingcounter->drawable_animation) {
        animsprite_force_end(rankingcounter->drawable_animation);
        animsprite_update_drawable(rankingcounter->drawable_animation, rankingcounter->drawable_accuracy, false);
        animsprite_update_drawable(rankingcounter->drawable_animation, rankingcounter->drawable_rank, true);
    }
}


int32_t rankingcounter_animate1(RankingCounter rankingcounter, float elapsed) {
    int32_t total = RANKINGCOUNTER_RANKING_BUFFER_SIZE + 1;

    for (int32_t i = 0; i < RANKINGCOUNTER_RANKING_BUFFER_SIZE; i++) {
        RankingItem* item = &rankingcounter->ranking_items[i];
        if (item->id < 0) {
            total--;
            continue;
        }

        int32_t completed = statesprite_animate(item->statesprite, elapsed);

        if (item->animsprite) {
            completed = animsprite_animate(item->animsprite, elapsed);
            animsprite_update_statesprite(item->animsprite, item->statesprite, true);
        }

        if (completed) {
            item->id = -1;
            total--;
        }
    }

    if (rankingcounter->drawable_animation) {
        if (animsprite_animate(rankingcounter->drawable_animation, elapsed)) total--;
        animsprite_update_drawable(rankingcounter->drawable_animation, rankingcounter->drawable_accuracy, false);
        animsprite_update_drawable(rankingcounter->drawable_animation, rankingcounter->drawable_rank, true);
    }

    return total;
}

int32_t rankingcounter_animate2(RankingCounter rankingcounter, float elapsed) {
    if (rankingcounter->enable_accuracy && rankingcounter->show_accuracy) {
        int32_t completed = textsprite_animate(rankingcounter->textsprite, elapsed);
        if (completed) rankingcounter->show_accuracy = false;
        return completed;
    }

    return 1;
}

void rankingcounter_draw1(RankingCounter rankingcounter, PVRContext pvrctx) {
    pvr_context_save(pvrctx);
    drawable_helper_apply_in_context(rankingcounter->drawable_rank, pvrctx);

    for (int32_t i = 0; i < RANKINGCOUNTER_RANKING_BUFFER_SIZE; i++) {
        if (rankingcounter->ranking_items[i].id < 0) continue;
        statesprite_draw(rankingcounter->ranking_items[i].statesprite, pvrctx);
    }

    pvr_context_restore(pvrctx);
}

void rankingcounter_draw2(RankingCounter rankingcounter, PVRContext pvrctx) {
    if (!rankingcounter->enable_accuracy || !rankingcounter->show_accuracy) return;
    pvr_context_save(pvrctx);
    drawable_helper_apply_in_context(rankingcounter->drawable_accuracy, pvrctx);

    textsprite_draw(rankingcounter->textsprite, pvrctx);
    pvr_context_restore(pvrctx);
}



static int32_t rankingcounter_internal_add_state(StateSprite statesprite, float max_height, ModelHolder modelholder, const char* prefix, const char* state_name) {
    char* animation_name = string_concat_for_state_name(2, prefix, state_name);

    Texture texture = modelholder_get_texture(modelholder, false);
    uint32_t vertex_color = modelholder_get_vertex_color(modelholder);
    AnimSprite animsprite = modelholder_create_animsprite(modelholder, animation_name, false, false);
    const AtlasEntry* atlas_entry = modelholder_get_atlas_entry2(modelholder, animation_name);

    StateSpriteState* state = statesprite_state_add2(
        statesprite, texture, animsprite, atlas_entry, vertex_color, animation_name
    );
    free_chk(animation_name);

    if (!state) {
        if (animsprite) animsprite_destroy(&animsprite);
        return 0;
    }

    float temp_width = 0.0f, temp_height = 0.0f;
    imgutils_get_statesprite_original_size(state, &temp_width, &temp_height);
    imgutils_calc_size(temp_width, temp_height, -1.0f, max_height, &temp_width, &temp_height);

    state->draw_width = temp_width;
    state->draw_height = temp_height;

    // center the sprite on the draw location
    state->offset_x = state->draw_width / -2.0f;
    state->offset_y = state->draw_height / -2.0f;

    return 1;
}

static RankingItem* rankingcounter_internal_pick_item(RankingItem* array, int32_t new_id) {
    for (int32_t i = 0; i < RANKINGCOUNTER_RANKING_BUFFER_SIZE; i++) {
        if (array[i].id < 0) {
            array[i].id = new_id;
            return &array[i];
        }
    }

    int32_t oldest_id = MATH2D_MAX_INT32;
    RankingItem* oldest_item = NULL;

    for (int32_t i = 0; i < RANKINGCOUNTER_RANKING_BUFFER_SIZE; i++) {
        if (array[i].id < oldest_id) {
            oldest_id = array[i].id;
            oldest_item = &array[i];
        }
    }

    assert(oldest_item != NULL);
    oldest_item->id = new_id;
    return oldest_item;
}

static int rankingcounter_internal_sort(const void* item1_ptr, const void* item2_ptr) {
    const RankingItem* item1 = (const RankingItem*)item1_ptr;
    const RankingItem* item2 = (const RankingItem*)item2_ptr;

    return item1->id - item2->id;
}
