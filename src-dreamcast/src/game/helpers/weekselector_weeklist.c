#include "game/helpers/weekselector_weeklist.h"

#include <kos/thread.h>

#include "animlist.h"
#include "animsprite.h"
#include "beatwatcher.h"
#include "drawable.h"
#include "game/common/funkinsave.h"
#include "game/gameplay/helpers/charactermanifest.h"
#include "game/helpers/weekselector_mdlselect.h"
#include "game/main.h"
#include "game/weekselector.h"
#include "imgutils.h"
#include "layout.h"
#include "logger.h"
#include "malloc_utils.h"
#include "math2d.h"
#include "modelholder.h"
#include "pvrcontext.h"
#include "sprite.h"
#include "statesprite.h"
#include "stringutils.h"
#include "textsprite.h"
#include "tweenlerp.h"
#include "vertexprops.h"


#define WEEKSELECTOR_WEEKLIST_VISIBLE_SIZE 4


typedef struct {
    Sprite icon_locked;
    Sprite sprite_title;
    TweenLerp tweenlerp_locked;
    TweenLerp tweenlerp_title;
    bool is_locked;
} Item;

struct WeekSelectorWeekList_s {
    Item list_visible[WEEKSELECTOR_WEEKLIST_VISIBLE_SIZE];
    int32_t index;
    float x;
    float y;
    float width;
    float row_height;
    float64 progress;
    bool do_reverse;
    Drawable drawable_host;
    Drawable drawable_list;
    StateSprite host_statesprite;
    LayoutPlaceholder* host_placeholder;
    volatile int32_t running_threads;
    volatile int32_t load_host_id;
    AnimSprite anim_selected;
    bool week_choosen;
    bool hey_playing;
    BeatWatcher beatwatcher;
};


static const char* WEEKSELECTOR_WEEKLIST_TITLE_ANIM_NAME = "weektitle";
static const float WEEKSELECTOR_WEEKLIST_TWEEN_DURATION = 120.0f;


static TweenLerp weekselector_weeklist_internal_create_tween();
static void weekselector_weeklist_internal_prepare_locked_tweens(WeekSelectorWeekList weeklist);
static void weekselector_weeklist_internal_prepare_title_tweens(WeekSelectorWeekList weeklist);
static void weekselector_weeklist_internal_calc_row_sizes(WeekSelectorWeekList weeklist, Sprite vertex, float* row1, float* row2);
static void weekselector_weeklist_internal_host_draw(WeekSelectorWeekList weeklist, PVRContext pvrctx);
static void* weekselector_weeklist_internal_load_host_async(WeekSelectorWeekList weeklist);


WeekSelectorWeekList weekselector_weeklist_init(AnimList animlist, ModelHolder modelholder, Layout layout) {
    LayoutPlaceholder* placeholder = layout_get_placeholder(layout, "ui_weeklist");
    assert(placeholder);

    LayoutPlaceholder* placeholder_host = layout_get_placeholder(layout, "ui_character_opponent");
    assert(placeholder_host);

    AnimSprite anim_selected = animsprite_init_from_animlist(animlist, "week_title_selected");

    WeekSelectorWeekList weeklist = malloc_chk(sizeof(struct WeekSelectorWeekList_s));
    malloc_assert(weeklist, WeekSelectorWeekList_s);

    *weeklist = (struct WeekSelectorWeekList_s){
        //.list_visible =(Item[]){},

        .index = 0,

        .x = placeholder->x,
        .y = placeholder->y,
        .width = placeholder->width,
        .row_height = placeholder->height / 4.0f,

        .progress = WEEKSELECTOR_WEEKLIST_TWEEN_DURATION,
        .do_reverse = false,

        .drawable_host = NULL,
        .drawable_list = NULL,

        .host_statesprite = statesprite_init_from_texture(NULL),
        .host_placeholder = placeholder_host,
        .running_threads = 0,
        .load_host_id = 0,

        .anim_selected = anim_selected,
        .week_choosen = false,
        .hey_playing = false,

        //.beatwatcher = (BeatWatcher){}
    };

    beatwatcher_reset(&weeklist->beatwatcher, true, 100.0f);

    for (int32_t i = 0; i < WEEKSELECTOR_WEEKLIST_VISIBLE_SIZE; i++) {
        weeklist->list_visible[i] = (Item){
            .icon_locked = modelholder_create_sprite(modelholder, WEEKSELECTOR_LOCKED, true),
            .sprite_title = sprite_init(NULL),
            .tweenlerp_locked = weekselector_weeklist_internal_create_tween(),
            .tweenlerp_title = weekselector_weeklist_internal_create_tween(),
            .is_locked = true
        };
        sprite_matrix_scale_size(weeklist->list_visible[i].icon_locked, true);
        sprite_matrix_scale_size(weeklist->list_visible[i].sprite_title, true);
    }

    weekselector_weeklist_internal_prepare_locked_tweens(weeklist);

    weeklist->drawable_list = drawable_init(
        placeholder->z, weeklist, (DelegateDraw)weekselector_weeklist_draw, (DelegateAnimate)weekselector_weeklist_animate
    );
    placeholder->vertex = weeklist->drawable_list;

    weeklist->drawable_host = drawable_init(
        placeholder_host->z, weeklist, (DelegateDraw)weekselector_weeklist_internal_host_draw, NULL
    );
    placeholder_host->vertex = weeklist->drawable_host;
    statesprite_set_draw_location(weeklist->host_statesprite, placeholder_host->x, placeholder_host->y);

    const char* last_played_week = funkinsave_get_last_played_week();
    int32_t index = 0;
    for (int32_t i = 0; i < weeks_array.size; i++) {
        if (string_equals(weeks_array.array[i].name, last_played_week)) {
            index = i;
            break;
        }
    }

    weekselector_weeklist_select(weeklist, index);

    // obligatory step
    weeklist->progress = WEEKSELECTOR_WEEKLIST_TWEEN_DURATION;

    float percent = index > 0 ? 0.0f : 1.0f;
    for (int32_t i = 0; i < WEEKSELECTOR_WEEKLIST_VISIBLE_SIZE; i++) {
        Item* visible_item = &weeklist->list_visible[i];
        tweenlerp_animate_percent(visible_item->tweenlerp_locked, percent);
        tweenlerp_animate_percent(visible_item->tweenlerp_title, percent);

        tweenlerp_vertex_set_properties(
            visible_item->tweenlerp_locked, visible_item->icon_locked, (PropertySetter)sprite_set_property
        );
        if (sprite_is_textured(visible_item->sprite_title)) {
            tweenlerp_vertex_set_properties(
                visible_item->tweenlerp_title, visible_item->sprite_title, (PropertySetter)sprite_set_property
            );
        }
    }

    return weeklist;
}

void weekselector_weeklist_destroy(WeekSelectorWeekList* weeklist_ptr) {
    if (!weeklist_ptr || !*weeklist_ptr) return;

    WeekSelectorWeekList weeklist = *weeklist_ptr;

    weeklist->load_host_id = -1;
    while (weeklist->running_threads > 0) {
        // wait until all async operations are done
        thd_pass();
    }

    for (int32_t i = 0; i < WEEKSELECTOR_WEEKLIST_VISIBLE_SIZE; i++) {
        sprite_destroy_full(&weeklist->list_visible[i].icon_locked);
        sprite_destroy_full(&weeklist->list_visible[i].sprite_title);
        tweenlerp_destroy(&weeklist->list_visible[i].tweenlerp_title);
        tweenlerp_destroy(&weeklist->list_visible[i].tweenlerp_locked);
    }

    statesprite_destroy(&weeklist->host_statesprite);
    if (weeklist->anim_selected) animsprite_destroy(&weeklist->anim_selected);
    drawable_destroy(&weeklist->drawable_host);
    drawable_destroy(&weeklist->drawable_list);

    free_chk(weeklist);
    *weeklist_ptr = NULL;
}


int32_t weekselector_weeklist_animate(WeekSelectorWeekList weeklist, float elapsed) {
    float since_beat = elapsed;

    if (!weeklist->hey_playing && beatwatcher_poll(&weeklist->beatwatcher)) {
        since_beat = weeklist->beatwatcher.since;
        if (statesprite_animation_completed(weeklist->host_statesprite)) {
            statesprite_animation_restart(weeklist->host_statesprite);
        }
    }

    if (weeklist->running_threads < 1) statesprite_animate(weeklist->host_statesprite, since_beat);

    for (int32_t i = 0; i < WEEKSELECTOR_WEEKLIST_VISIBLE_SIZE; i++) {
        Item* visible_item = &weeklist->list_visible[i];

        sprite_animate(visible_item->icon_locked, elapsed);

        if (sprite_is_textured(visible_item->sprite_title))
            sprite_animate(visible_item->sprite_title, elapsed);
    }

    if (weeklist->week_choosen && weeklist->anim_selected) {
        Item* visible_item = &weeklist->list_visible[weeklist->do_reverse ? 2 : 1];
        animsprite_update_sprite(weeklist->anim_selected, visible_item->sprite_title, true);
        animsprite_animate(weeklist->anim_selected, elapsed);
    }

    if (weeklist->progress >= WEEKSELECTOR_WEEKLIST_TWEEN_DURATION) return 1;

    float percent = math2d_min_float(1.0f, (float)(weeklist->progress / WEEKSELECTOR_WEEKLIST_TWEEN_DURATION));
    weeklist->progress += elapsed;
    if (weeklist->do_reverse) percent = 1.0f - percent;

    for (int32_t i = 0; i < WEEKSELECTOR_WEEKLIST_VISIBLE_SIZE; i++) {
        Item* visible_item = &weeklist->list_visible[i];

        tweenlerp_animate_percent(visible_item->tweenlerp_locked, percent);
        tweenlerp_animate_percent(visible_item->tweenlerp_title, percent);

        tweenlerp_vertex_set_properties(
            visible_item->tweenlerp_locked, visible_item->icon_locked, (PropertySetter)sprite_set_property
        );
        if (sprite_is_textured(visible_item->sprite_title)) {
            tweenlerp_vertex_set_properties(
                visible_item->tweenlerp_title, visible_item->sprite_title, (PropertySetter)sprite_set_property
            );
        }
    }

    return 0;
}

void weekselector_weeklist_draw(WeekSelectorWeekList weeklist, PVRContext pvrctx) {
    pvr_context_save(pvrctx);

    sh4matrix_translate(pvrctx->current_matrix, weeklist->x, weeklist->y);
    pvr_context_flush(pvrctx);

    if (weeklist->week_choosen) {
        // draw only the choosen week
        Item* visible_item = &weeklist->list_visible[weeklist->do_reverse ? 2 : 1];
        if (sprite_is_textured(visible_item->sprite_title)) {
            sprite_draw(visible_item->sprite_title, pvrctx);
        }
    } else {
        for (int32_t i = 0; i < WEEKSELECTOR_WEEKLIST_VISIBLE_SIZE; i++) {
            Item* visible_item = &weeklist->list_visible[i];
            if (!sprite_is_textured(visible_item->sprite_title)) continue;

            sprite_draw(visible_item->sprite_title, pvrctx);
            if (visible_item->is_locked) sprite_draw(visible_item->icon_locked, pvrctx);
        }
    }

    pvr_context_restore(pvrctx);
}


void weekselector_weeklist_toggle_choosen(WeekSelectorWeekList weeklist) {
    weeklist->week_choosen = true;
    if (weeklist->running_threads > 0) return;
    if (statesprite_state_toggle(weeklist->host_statesprite, WEEKSELECTOR_MDLSELECT_HEY)) {
        weeklist->hey_playing = true;
    }
}

WeekInfo* weekselector_weeklist_get_selected(WeekSelectorWeekList weeklist) {
    if (weeklist->index < 0 || weeklist->index >= weeks_array.size) return NULL;
    return &weeks_array.array[weeklist->index];
}

Sprite weekselector_weeklist_peek_title_sprite(WeekSelectorWeekList weeklist) {
    return weeklist->list_visible[weeklist->do_reverse ? 2 : 1].sprite_title;
}

bool weekselector_weeklist_scroll(WeekSelectorWeekList weeklist, int32_t offset) {
    return weekselector_weeklist_select(weeklist, weeklist->index + offset);
}

bool weekselector_weeklist_select(WeekSelectorWeekList weeklist, int32_t index) {
    if (index < 0 || index >= weeks_array.size) return false;

    weeklist->do_reverse = weeklist->index < index;
    weeklist->index = index;

    // center the selected week in the visible list
    index -= weeklist->do_reverse ? 2 : 1;

    for (int32_t i = 0; i < WEEKSELECTOR_WEEKLIST_VISIBLE_SIZE; i++, index++) {
        if (index < 0 || index >= weeks_array.size) {
            weeklist->list_visible[i].is_locked = true;
            sprite_destroy_texture(weeklist->list_visible[i].sprite_title);
            continue;
        }

        WeekInfo* weekinfo = &weeks_array.array[index];
        weeklist->list_visible[i].is_locked = !funkinsave_contains_unlock_directive(
            weekinfo->unlock_directive
        );

        Texture texture = NULL;
        AnimSprite animsprite = NULL;

        if (weekinfo->week_title_model) {
            if (modelholder_utils_is_known_extension(weekinfo->week_title_model)) {
                ModelHolder modelholder = modelholder_init(weekinfo->week_title_model);
                if (modelholder) {
                    texture = modelholder_get_texture(modelholder, true);
                    animsprite = modelholder_create_animsprite(
                        modelholder,
                        weekinfo->week_title_model_animation_name ? weekinfo->week_title_model_animation_name : WEEKSELECTOR_WEEKLIST_TITLE_ANIM_NAME,
                        true,
                        false
                    );
                    modelholder_destroy(&modelholder);
                }
            } else {
                texture = texture_init(weekinfo->week_title_model);
            }
        } else {
            char* title_src = weekenumerator_get_title_texture(weekinfo);
            texture = texture_init(title_src);
            free_chk(title_src);
        }

        sprite_destroy_texture(weeklist->list_visible[i].sprite_title);
        sprite_destroy_all_animations(weeklist->list_visible[i].sprite_title);

        sprite_set_texture(weeklist->list_visible[i].sprite_title, texture, true);

        if (animsprite) {
            sprite_external_animation_set(weeklist->list_visible[i].sprite_title, animsprite);
            animsprite_update_sprite(animsprite, weeklist->list_visible[i].sprite_title, true);
        }

        sprite_set_draw_size_from_source_size(weeklist->list_visible[i].sprite_title);
    }

    weeklist->load_host_id++;
    weeklist->running_threads++;
    main_thd_helper_spawn(true, (AsyncThread)weekselector_weeklist_internal_load_host_async, weeklist);

    weeklist->progress = 0.0;
    weekselector_weeklist_internal_prepare_title_tweens(weeklist);

    return true;
}

int32_t weekselector_weeklist_get_selected_index(WeekSelectorWeekList weeklist) {
    return weeklist->index;
}

void weekselector_weeklist_set_beats(WeekSelectorWeekList weeklist, float beats_per_minute) {
    beatwatcher_change_bpm(&weeklist->beatwatcher, beats_per_minute);
}



static TweenLerp weekselector_weeklist_internal_create_tween() {
    float duration = WEEKSELECTOR_WEEKLIST_TWEEN_DURATION;
    TweenLerp tweenlerp = tweenlerp_init();
    tweenlerp_add_linear(tweenlerp, SPRITE_PROP_SCALE_X, FLOAT_NaN, FLOAT_NaN, duration);
    tweenlerp_add_linear(tweenlerp, SPRITE_PROP_SCALE_Y, FLOAT_NaN, FLOAT_NaN, duration);
    tweenlerp_add_linear(tweenlerp, SPRITE_PROP_TRANSLATE_X, FLOAT_NaN, FLOAT_NaN, duration);
    tweenlerp_add_linear(tweenlerp, SPRITE_PROP_TRANSLATE_Y, FLOAT_NaN, FLOAT_NaN, duration);
    tweenlerp_add_linear(tweenlerp, SPRITE_PROP_ALPHA, FLOAT_NaN, FLOAT_NaN, duration);
    return tweenlerp;
}

static void weekselector_weeklist_internal_prepare_locked_tweens(WeekSelectorWeekList weeklist) {
    float row1[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    float row2[4] = {0.0f, 0.0f, 0.0f, 0.0f};

    TweenLerp tweenlerp;
    float row_height = weeklist->row_height;
    float x1, x2, y1, y2;

    float height3 = row_height * 3.0f;
    float height4 = row_height * 4.0f;

    // all locked icons have the same draw size
    weekselector_weeklist_internal_calc_row_sizes(
        weeklist, weeklist->list_visible[0].icon_locked, row1, row2
    );

    // void1 -> row1
    x1 = weeklist->width / 2.0f;
    x2 = (weeklist->width - row1[2]) / 2.0f;
    y1 = -row_height;
    y2 = (row_height - row1[3]) / 2.0f;
    tweenlerp = weeklist->list_visible[0].tweenlerp_locked;
    tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_SCALE_X, 0.0f, row1[0]);
    tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_SCALE_Y, 0.0f, row1[1]);
    tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_TRANSLATE_X, x1, x2);
    tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_TRANSLATE_Y, y1, y2);
    tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_ALPHA, 0.0f, 0.7f);

    // row1 -> row2
    x1 = (weeklist->width - row1[2]) / 2.0f;
    x2 = (weeklist->width - row2[2]) / 2.0f;
    y1 = (row_height - row1[3]) / 2.0f;
    y2 = ((weeklist->row_height * 2.0f) - row2[3]) / 2.0f;
    tweenlerp = weeklist->list_visible[1].tweenlerp_locked;
    tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_SCALE_X, row1[0], row2[0]);
    tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_SCALE_Y, row1[1], row2[1]);
    tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_TRANSLATE_X, x1, x2);
    tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_TRANSLATE_Y, y1, y2 + row_height);
    tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_ALPHA, 0.7f, 1.0f);

    // row2 -> row3
    x1 = (weeklist->width - row2[2]) / 2.0f;
    x2 = (weeklist->width - row1[2]) / 2.0f;
    y1 = ((weeklist->row_height * 2.0f) - row2[3]) / 2.0f;
    y2 = (row_height - row1[3]) / 2.0f;
    tweenlerp = weeklist->list_visible[2].tweenlerp_locked;
    tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_SCALE_X, row2[0], row1[0]);
    tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_SCALE_Y, row2[1], row1[1]);
    tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_TRANSLATE_X, x1, x2);
    tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_TRANSLATE_Y, y1 + row_height, y2 + height3);
    tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_ALPHA, 1.0f, 0.7f);

    // row3 -> void2
    x1 = (weeklist->width - row1[2]) / 2.0f;
    x2 = weeklist->width / 2.0f;
    y1 = (row_height - row1[3]) / 2.0f;
    y2 = row_height;
    tweenlerp = weeklist->list_visible[3].tweenlerp_locked;
    tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_SCALE_X, row1[0], 0.0f);
    tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_SCALE_Y, row1[1], 0.0f);
    tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_TRANSLATE_X, x1, x2);
    tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_TRANSLATE_Y, y1 + height3, y2 + height4);
    tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_ALPHA, 0.7f, 0.0f);
}

static void weekselector_weeklist_internal_prepare_title_tweens(WeekSelectorWeekList weeklist) {
    float row1[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    float row2[4] = {0.0f, 0.0f, 0.0f, 0.0f};

    TweenLerp tweenlerp;
    float height1 = weeklist->row_height * 1.0f;
    float height3 = weeklist->row_height * 3.0f;
    float height4 = weeklist->row_height * 4.0f;

    // void1 -> row1
    if (weeklist->list_visible[0].sprite_title) {
        tweenlerp = weeklist->list_visible[0].tweenlerp_title;
        weekselector_weeklist_internal_calc_row_sizes(
            weeklist, weeklist->list_visible[0].sprite_title, row1, row2
        );
        float x1 = weeklist->width / 2.0f;
        float x2 = (weeklist->width - row1[2]) / 2.0f;
        float y1 = -height1;
        float y2 = (height1 - row1[3]) / 2.0f;
        tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_SCALE_X, 0.0f, row1[0]);
        tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_SCALE_Y, 0.0f, row1[1]);
        tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_TRANSLATE_X, x1, x2);
        tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_TRANSLATE_Y, y1, y2);
        tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_ALPHA, 0.0f, 0.3f);
    }

    // row1 -> row2
    if (weeklist->list_visible[1].sprite_title) {
        tweenlerp = weeklist->list_visible[1].tweenlerp_title;
        weekselector_weeklist_internal_calc_row_sizes(
            weeklist, weeklist->list_visible[1].sprite_title, row1, row2
        );
        float x1 = (weeklist->width - row1[2]) / 2.0f;
        float x2 = (weeklist->width - row2[2]) / 2.0f;
        float y1 = (height1 - row1[3]) / 2.0f;
        float y2 = ((weeklist->row_height * 2.0f) - row2[3]) / 2.0f;
        tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_SCALE_X, row1[0], row2[0]);
        tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_SCALE_Y, row1[1], row2[1]);
        tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_TRANSLATE_X, x1, x2);
        tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_TRANSLATE_Y, y1, y2 + height1);
        tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_ALPHA, 0.3f, 1.0f);
    }

    // row2 -> row3
    if (weeklist->list_visible[2].sprite_title) {
        tweenlerp = weeklist->list_visible[2].tweenlerp_title;
        weekselector_weeklist_internal_calc_row_sizes(
            weeklist, weeklist->list_visible[2].sprite_title, row1, row2
        );
        float x1 = (weeklist->width - row2[2]) / 2.0f;
        float x2 = (weeklist->width - row1[2]) / 2.0f;
        float y1 = ((weeklist->row_height * 2.0f) - row2[3]) / 2.0f;
        float y2 = (height1 - row1[3]) / 2.0f;
        tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_SCALE_X, row2[0], row1[0]);
        tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_SCALE_Y, row2[1], row1[1]);
        tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_TRANSLATE_X, x1, x2);
        tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_TRANSLATE_Y, y1 + height1, y2 + height3);
        tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_ALPHA, 1.0f, 0.3f);
    }

    // row3 -> void2
    if (weeklist->list_visible[3].sprite_title) {
        tweenlerp = weeklist->list_visible[3].tweenlerp_title;
        weekselector_weeklist_internal_calc_row_sizes(
            weeklist, weeklist->list_visible[3].sprite_title, row1, row2
        );
        float x1 = (weeklist->width - row1[2]) / 2.0f;
        float x2 = weeklist->width / 2.0f;
        float y1 = (height1 - row1[3]) / 2.0f;
        float y2 = height4 + height1;
        tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_SCALE_X, row1[0], 0.0f);
        tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_SCALE_Y, row1[1], 0.0f);
        tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_TRANSLATE_X, x1, x2);
        tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_TRANSLATE_Y, y1 + height3, y2);
        tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_ALPHA, 0.3f, 0.0f);
    }
}

static void weekselector_weeklist_internal_calc_row_sizes(WeekSelectorWeekList weeklist, Sprite vertex, float* row1, float* row2) {
    float draw_width = 0.0f, draw_height = 0.0f;

    float height1 = weeklist->row_height * 1.0f;
    float height2 = weeklist->row_height * 2.0f;
    float width = weeklist->width;

    sprite_get_draw_size(vertex, &draw_width, &draw_height);

    imgutils_calc_size(draw_width, draw_height, width, height1, &row1[0], &row1[1]);
    imgutils_calc_size(draw_width, draw_height, width, height2, &row2[0], &row2[1]);

    row1[0] /= draw_width;           // scale x
    row1[1] /= draw_height;          // scale y
    row1[2] = draw_width * row1[0];  // width 1
    row1[3] = draw_height * row1[1]; // height 1

    row2[0] /= draw_width;           // scale x
    row2[1] /= draw_height;          // scale y
    row2[2] = draw_width * row2[0];  // width 2
    row2[3] = draw_height * row2[1]; // height 2
}

static void weekselector_weeklist_internal_host_draw(WeekSelectorWeekList weeklist, PVRContext pvrctx) {
    if (weeklist->running_threads > 0 || !statesprite_is_visible(weeklist->host_statesprite)) return;
    pvr_context_save(pvrctx);
    statesprite_draw(weeklist->host_statesprite, pvrctx);
    pvr_context_restore(pvrctx);
}

static void* weekselector_weeklist_internal_load_host_async(WeekSelectorWeekList weeklist) {
    int32_t load_host_id = weeklist->load_host_id;
    WeekInfo* weekinfo = &weeks_array.array[weeklist->index];
    bool host_flip, host_beat;
    ModelHolder modelholder;
    const char* anim_name_hey = WEEKSELECTOR_MDLSELECT_HEY;
    const char* anim_name_idle = WEEKSELECTOR_MDLSELECT_IDLE;
    bool hidden = false;
    CharacterManifest charactermanifest = NULL;

    if (weekinfo->host_hide_if_week_locked)
        hidden = weeklist->list_visible[weeklist->index].is_locked && weekinfo->host_hide_if_week_locked;

    if (weekinfo->week_host_character_manifest) {
        charactermanifest = charactermanifest_init(weekinfo->week_host_character_manifest, false);
        host_flip = charactermanifest->left_facing; // face to the right
        host_beat = charactermanifest->week_selector_enable_beat;

        if (charactermanifest->week_selector_idle_anim_name != NULL) {
            anim_name_idle = charactermanifest->week_selector_idle_anim_name;
        }
        if (charactermanifest->week_selector_choosen_anim_name != NULL) {
            anim_name_hey = charactermanifest->week_selector_choosen_anim_name;
        }

        modelholder = modelholder_init(charactermanifest->week_selector_model);
    } else if (weekinfo->week_host_model) {
        host_flip = weekinfo->host_flip_sprite;
        host_beat = weekinfo->host_enable_beat;

        if (weekinfo->week_host_model_idle_animation_name) {
            anim_name_idle = weekinfo->week_host_model_idle_animation_name;
        }
        if (weekinfo->week_host_model_choosen_animation_name) {
            anim_name_hey = weekinfo->week_host_model_choosen_animation_name;
        }

        modelholder = modelholder_init(weekinfo->week_host_model);
    } else {
        host_flip = weekinfo->host_flip_sprite;
        host_beat = weekinfo->host_enable_beat;

        char* week_host_model = weekenumerator_get_host_model(weekinfo);
        modelholder = modelholder_init(week_host_model);
        free_chk(week_host_model);
    }

    if (weeklist->load_host_id != load_host_id) {
        // weeklist is loading another host or begin disposed
        goto L_return;
    }

    if (!modelholder) {
        logger_error("weekselector_weeklist_internal_load_host_async() modelholder_init failed");
        if (weeklist->load_host_id == load_host_id) {
            statesprite_state_remove(weeklist->host_statesprite, WEEKSELECTOR_MDLSELECT_HEY);
            statesprite_state_remove(weeklist->host_statesprite, WEEKSELECTOR_MDLSELECT_IDLE);
        }
        statesprite_set_texture(weeklist->host_statesprite, NULL, false);
        statesprite_set_visible(weeklist->host_statesprite, false);

        goto L_return;
    }

    Texture texture = modelholder_get_texture(modelholder, false);

    if (weeklist->load_host_id == load_host_id) {
        statesprite_state_remove(weeklist->host_statesprite, WEEKSELECTOR_MDLSELECT_HEY);
        statesprite_state_remove(weeklist->host_statesprite, WEEKSELECTOR_MDLSELECT_IDLE);

        weekselector_mdlselect_helper_import(
            weeklist->host_statesprite,
            modelholder,
            weeklist->host_placeholder,
            host_beat,
            anim_name_idle,
            WEEKSELECTOR_MDLSELECT_IDLE
        );
        weekselector_mdlselect_helper_import(
            weeklist->host_statesprite,
            modelholder,
            weeklist->host_placeholder,
            false,
            anim_name_hey,
            WEEKSELECTOR_MDLSELECT_HEY
        );

        if (weeklist->week_choosen)
            statesprite_state_toggle(weeklist->host_statesprite, WEEKSELECTOR_MDLSELECT_HEY);
        else
            statesprite_state_toggle(weeklist->host_statesprite, WEEKSELECTOR_MDLSELECT_IDLE);

        // set defaults
        statesprite_set_alpha(weeklist->host_statesprite, 1.0f);
        statesprite_set_property(weeklist->host_statesprite, SPRITE_PROP_ALPHA2, 1.0f);
        statesprite_set_offsetcolor(weeklist->host_statesprite, 1.0f, 1.0f, 1.0f, -1.0f);
        statesprite_matrix_reset(weeklist->host_statesprite);

        statesprite_set_visible(weeklist->host_statesprite, !hidden && texture);
        statesprite_flip_texture(weeklist->host_statesprite, host_flip, false);
        statesprite_state_apply(weeklist->host_statesprite, NULL);
        statesprite_animate(weeklist->host_statesprite, beatwatcher_remaining_until_next(&weeklist->beatwatcher));
    }

L_return:
    if (modelholder) modelholder_destroy(&modelholder);
    if (charactermanifest) charactermanifest_destroy(&charactermanifest);

    weeklist->running_threads--;
    return NULL;
}
