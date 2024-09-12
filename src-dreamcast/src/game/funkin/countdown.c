#include "game/funkin/countdown.h"

#include "animlist.h"
#include "animsprite.h"
#include "beatwatcher.h"
#include "drawable.h"
#include "externals/luascript.h"
#include "fs.h"
#include "imgutils.h"
#include "malloc_utils.h"
#include "modelholder.h"
#include "soundplayer.h"
#include "statesprite.h"
#include "tweenkeyframe.h"


struct Countdown_s {
    bool valid_model;
    bool valid_state;
    StateSprite statesprite;
    float beat_duration;
    float height;
    TweenKeyframe default_animation;
    bool default_animate;
    float animation_speed;
    bool static_ready;
    bool ready_only;
    int32_t progress;
    float64 timer;
    SoundPlayer sound_three;
    SoundPlayer sound_two;
    SoundPlayer sound_one;
    SoundPlayer sound_go;
    Drawable drawable;
};


static const char* COUNTDOWN_READY_CONFIRM = "ready-confirm";
static const char* COUNTDOWN_THREE = "three"; // in the funkin only the sounds "2", "1", "go!" have sprites
static const char* COUNTDOWN_READY = "ready";
static const char* COUNTDOWN_SET = "set";
static const char* COUNTDOWN_GO = "go";
static const char* COUNTDOWN_DEFAULT_ANIMATION = "countdown";


static void countdown_internal_toggle(Countdown countdown, const char* state_name);


Countdown countdown_init(ModelHolder modelholder, float height) {
    fs_folder_stack_push();
    fs_set_working_folder("/assets/common/sound/", false);

    Countdown countdown = malloc_chk(sizeof(struct Countdown_s));
    malloc_assert(countdown, Countdown);

    *countdown = (struct Countdown_s){
        .valid_model = !modelholder_is_invalid(modelholder),
        .valid_state = false,

        .statesprite = statesprite_init_from_texture(NULL),
        .beat_duration = math2d_beats_per_minute_to_beat_per_milliseconds(100.0f),

        .height = height,
        .default_animation = NULL,
        .default_animate = false,
        .animation_speed = 1.0f,

        .static_ready = false,
        .ready_only = false,
        .progress = 4,
        .timer = 0.0,

        .sound_three = soundplayer_init("intro3.ogg"),
        .sound_two = soundplayer_init("intro2.ogg"),
        .sound_one = soundplayer_init("intro1.ogg"),
        .sound_go = soundplayer_init("introGo.ogg"),

        .drawable = NULL
    };

    countdown->drawable = drawable_init(200.0f, countdown, (DelegateDraw)countdown_draw, (DelegateAnimate)countdown_animate);

    fs_folder_stack_pop();

    if (!countdown->valid_model) return countdown;

    statesprite_set_visible(countdown->statesprite, false);
    statesprite_state_add(
        countdown->statesprite, modelholder, COUNTDOWN_READY_CONFIRM, COUNTDOWN_READY_CONFIRM
    );
    statesprite_state_add(
        countdown->statesprite, modelholder, COUNTDOWN_THREE, COUNTDOWN_THREE
    );
    statesprite_state_add(
        countdown->statesprite, modelholder, COUNTDOWN_READY, COUNTDOWN_READY
    );
    statesprite_state_add(
        countdown->statesprite, modelholder, COUNTDOWN_SET, COUNTDOWN_SET
    );
    statesprite_state_add(
        countdown->statesprite, modelholder, COUNTDOWN_GO, COUNTDOWN_GO
    );

    return countdown;
}

void countdown_destroy(Countdown* countdown_ptr) {
    if (!countdown_ptr || !*countdown_ptr) return;

    Countdown countdown = *countdown_ptr;

    luascript_drop_shared(countdown);

    statesprite_destroy(&countdown->statesprite);
    if (countdown->default_animation) tweenkeyframe_destroy(&countdown->default_animation);
    if (countdown->sound_three) soundplayer_destroy(&countdown->sound_three);
    if (countdown->sound_two) soundplayer_destroy(&countdown->sound_two);
    if (countdown->sound_one) soundplayer_destroy(&countdown->sound_one);
    if (countdown->sound_go) soundplayer_destroy(&countdown->sound_go);
    drawable_destroy(&countdown->drawable);

    free_chk(countdown);
    *countdown_ptr = NULL;
}


void countdown_set_layout_viewport(Countdown countdown, float width, float height) {
    if (!countdown->valid_model) return;
    float draw_width = 0.0f, draw_height = 0.0f;

    LinkedList state_list = statesprite_state_list(countdown->statesprite);

    foreach (StateSpriteState*, state, LINKEDLIST_ITERATOR, state_list) {
        imgutils_get_statesprite_original_size(state, &draw_width, &draw_height);
        imgutils_calc_size(draw_width, draw_height, -1.0f, countdown->height, &draw_width, &draw_height);
        state->draw_width = draw_width;
        state->draw_height = draw_height;
        state->offset_x = state->draw_width / -2.0f;
        state->offset_y = state->draw_height / -2.0f;
    }

    statesprite_set_draw_location(countdown->statesprite, width / 2.0f, height / 2.0f);
}

void countdown_set_bpm(Countdown countdown, float bpm) {
    countdown->beat_duration = math2d_beats_per_minute_to_beat_per_milliseconds(bpm);
    countdown->animation_speed = 1000.0f / countdown->beat_duration;
}

void countdown_set_default_animation(Countdown countdown, AnimList animlist) {
    if (!animlist) return;
    const AnimListItem* animlist_item = animlist_get_animation(animlist, COUNTDOWN_DEFAULT_ANIMATION);
    if (!animlist_item) return;

    countdown->default_animation = tweenkeyframe_init2(animlist_item);
}

void countdown_set_default_animation2(Countdown countdown, TweenKeyframe tweenkeyframe) {
    if (countdown->default_animation) tweenkeyframe_destroy(&countdown->default_animation);
    countdown->default_animation = tweenkeyframe ? tweenkeyframe_clone(tweenkeyframe) : NULL;
}


Drawable countdown_get_drawable(Countdown countdown) {
    return countdown->drawable;
}


bool countdown_ready(Countdown countdown) {
    if (!countdown->valid_model) return false;

    countdown->progress = 0;
    countdown->ready_only = true;
    countdown->static_ready = false;
    countdown->valid_state = statesprite_state_toggle(countdown->statesprite, COUNTDOWN_READY_CONFIRM);

    // restore statesprite alpha
    statesprite_set_alpha(countdown->statesprite, 1.0f);

    if (!countdown->valid_state) {
        countdown->static_ready = true;
        countdown->valid_state = statesprite_state_toggle(countdown->statesprite, COUNTDOWN_READY);
    }

    if (countdown->valid_state) {
        statesprite_animation_restart(countdown->statesprite);
        statesprite_animate(countdown->statesprite, 0.0f);
    }

    return countdown->valid_state;
}

bool countdown_start(Countdown countdown) {
    if (countdown->sound_three) soundplayer_replay(countdown->sound_three);
    if (!countdown->valid_model) return false;
    countdown_internal_toggle(countdown, COUNTDOWN_THREE);
    countdown->ready_only = false;
    countdown->static_ready = false;
    countdown->progress = 0;
    countdown->timer = 0.0;

    return countdown->valid_state;
}

bool countdown_has_ended(Countdown countdown) {
    return (countdown->progress > 3) && (!countdown->ready_only);
}


int32_t countdown_animate(Countdown countdown, float elapsed) {
    if (countdown->static_ready) return 1;
    if (countdown->ready_only) return statesprite_animate(countdown->statesprite, elapsed);
    if (countdown->progress > 4) return 1;

    int32_t completed;

    if (countdown->default_animate) {
        float64 percent = countdown->timer / countdown->beat_duration;
        if (percent > 1.0) percent = 1.0;

        tweenkeyframe_animate_percent(countdown->default_animation, percent);
        tweenkeyframe_vertex_set_properties(
            countdown->default_animation, countdown->statesprite, (PropertySetter)statesprite_set_property
        );
        completed = percent >= 1.0 ? 1 : 0;
    } else {
        completed = statesprite_animate(countdown->statesprite, elapsed * countdown->animation_speed);
    }

    if (countdown->timer >= countdown->beat_duration) {
        countdown->timer -= countdown->beat_duration; // this keeps the timer in sync
        countdown->progress++;

        switch (countdown->progress) {
            case 1:
                if (countdown->sound_three) soundplayer_stop(countdown->sound_three);
                if (countdown->sound_two) soundplayer_replay(countdown->sound_two);
                countdown_internal_toggle(countdown, COUNTDOWN_READY);
                break;
            case 2:
                if (countdown->sound_two) soundplayer_stop(countdown->sound_two);
                if (countdown->sound_one) soundplayer_replay(countdown->sound_one);
                countdown_internal_toggle(countdown, COUNTDOWN_SET);
                break;
            case 3:
                if (countdown->sound_one) soundplayer_stop(countdown->sound_one);
                if (countdown->sound_go) soundplayer_replay(countdown->sound_go);
                countdown_internal_toggle(countdown, COUNTDOWN_GO);
                break;
        }

        statesprite_animation_restart(countdown->statesprite);
    }

    countdown->timer += elapsed;

    return completed;
}

void countdown_draw(Countdown countdown, PVRContext pvrctx) {
    if (!countdown->valid_model || !countdown->valid_state) return;
    if (!countdown->ready_only && countdown->progress > 3) return;

    statesprite_draw(countdown->statesprite, pvrctx);
}



static void countdown_internal_toggle(Countdown countdown, const char* state_name) {
    countdown->valid_state = statesprite_state_toggle(countdown->statesprite, state_name);
    if (!countdown->valid_state) return;

    if (statesprite_state_get(countdown->statesprite)->animation) {
        countdown->default_animate = false;
    } else if (countdown->default_animation) {
        countdown->default_animate = true;
    } else {
        countdown->default_animate = false;
    }
}
