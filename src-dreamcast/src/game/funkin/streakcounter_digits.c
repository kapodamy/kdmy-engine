#include "game/funkin/streakcounter_digits.h"

#include "drawable.h"
#include "malloc_utils.h"
#include "math2d.h"
#include "statesprite.h"
#include "stringutils.h"
#include "imgutils.h"

#define STREAKCOUNTER_DIGITS 4 // maximum visible digits

struct StreakCounterDigits_s {
    int8_t buffer[STREAKCOUNTER_DIGITS];
    StateSprite digits[STREAKCOUNTER_DIGITS];
    AnimSprite animations[STREAKCOUNTER_DIGITS];
    bool has_animations;
    float delay;
    AnimSprite animation_pair;
    Drawable drawable;
    char* selected_state;
};


const char* STREAKCOUNTER_UI_STREAK_ANIM = "streak_number"; // picked from UI animlist
static const char STREAKCOUNTER_NUMBERS[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

const int32_t STREAKCOUNTER_MAX_VALUE = 9999; // ¡¡this must match the STREAKCOUNTER_DIGITS!!

static const char* STREAKCOUNTER_DIGITS_INTERNAL_STATE_NAME = "streakcounter_digits-state";


StreakCounterDigits streakcounter_digits_init(float delay) {
    StreakCounterDigits streakcounter_digits = malloc_chk(sizeof(struct StreakCounterDigits_s));
    malloc_assert(streakcounter_digits, StreakCounterDigits);

    *streakcounter_digits = (struct StreakCounterDigits_s){
        //.buffer = (int8_t[]){},
        //.digits = (StateSprite[]){},
        //.animations = (AnimSprite[]){},

        .has_animations = false,

        .delay = delay,

        .animation_pair = NULL,
        .drawable = NULL,
        .selected_state = (char*)STREAKCOUNTER_DIGITS_INTERNAL_STATE_NAME
    };

    // fake drawable for pair animations
    streakcounter_digits->drawable = drawable_init(-1.0f, streakcounter_digits, NULL, NULL);
    drawable_set_visible(streakcounter_digits->drawable, false);

    for (int32_t i = 0; i < STREAKCOUNTER_DIGITS; i++) {
        streakcounter_digits->buffer[i] = -1;
        streakcounter_digits->animations[i] = NULL;
        streakcounter_digits->digits[i] = statesprite_init_from_texture(NULL);
        statesprite_set_visible(streakcounter_digits->digits[i], false);
    }

    return streakcounter_digits;
}

void streakcounter_digits_destroy(StreakCounterDigits* streakcounter_digits_ptr) {
    if (!streakcounter_digits_ptr || !*streakcounter_digits_ptr) return;

    StreakCounterDigits streakcounter_digits = *streakcounter_digits_ptr;

    for (int32_t i = 0; i < STREAKCOUNTER_DIGITS; i++) {
        statesprite_destroy(&streakcounter_digits->digits[i]);
        if (streakcounter_digits->animations[i]) animsprite_destroy(&streakcounter_digits->animations[i]);
    }
    if (streakcounter_digits->animation_pair) animsprite_destroy(&streakcounter_digits->animation_pair);
    drawable_destroy(&streakcounter_digits->drawable);

    if (/*pointer equals*/ streakcounter_digits->selected_state != STREAKCOUNTER_DIGITS_INTERNAL_STATE_NAME)
        free_chk(streakcounter_digits->selected_state);

    free_chk(streakcounter_digits);
    *streakcounter_digits_ptr = NULL;
}


int32_t streakcounter_digits_state_add(StreakCounterDigits streakcounter_digits, float max_hght, ModelHolder mdlhldr, const char* state_name) {
    float temp_width = 0.0f, temp_height = 0.0f;
    int32_t success = 0;

    for (int32_t i = 0; i < sizeof(STREAKCOUNTER_NUMBERS); i++) {
        const char number[] = {STREAKCOUNTER_NUMBERS[i], '\0'};

        char* animation_name = string_concat_for_state_name(2, number, state_name);

        for (int32_t j = 0; j < STREAKCOUNTER_DIGITS; j++) {
            StateSpriteState* statesprite_state = statesprite_state_add(
                streakcounter_digits->digits[j], mdlhldr, animation_name, animation_name
            );

            if (!statesprite_state) continue;

            imgutils_get_statesprite_original_size(statesprite_state, &temp_width, &temp_height);
            imgutils_calc_size(temp_width, temp_height, -1.0f, max_hght, &temp_width, &temp_height);

            statesprite_state->draw_width = temp_width;
            statesprite_state->draw_height = temp_height;
            statesprite_state->offset_x = 0.0;
            statesprite_state->offset_y = 0.0;
        }

        free_chk(animation_name);
    }

    return success;
}

void streakcounter_digits_state_toggle(StreakCounterDigits streakcounter_digits, const char* state_name) {
    if (/*pointer equals*/ streakcounter_digits->selected_state != STREAKCOUNTER_DIGITS_INTERNAL_STATE_NAME)
        free_chk(streakcounter_digits->selected_state);

    streakcounter_digits->selected_state = string_duplicate(state_name);
}

void streakcounter_digits_animation_restart(StreakCounterDigits streakcounter_digits) {
    for (int32_t i = 0; i < STREAKCOUNTER_DIGITS; i++) {
        statesprite_animation_restart(streakcounter_digits->digits[i]);
        if (streakcounter_digits->has_animations)
            animsprite_restart(streakcounter_digits->animations[i]);
    }
    if (streakcounter_digits->animation_pair)
        animsprite_restart(streakcounter_digits->animation_pair);
}

void streakcounter_digits_animation_end(StreakCounterDigits streakcounter_digits) {
    for (int32_t i = 0; i < STREAKCOUNTER_DIGITS; i++) {
        statesprite_animation_end(streakcounter_digits->digits[i]);
        if (streakcounter_digits->has_animations) {
            animsprite_force_end(streakcounter_digits->animations[i]);
            animsprite_update_statesprite(streakcounter_digits->animations[i], streakcounter_digits->digits[i], true);
        }
    }
    if (streakcounter_digits->animation_pair) {
        animsprite_force_end(streakcounter_digits->animation_pair);
        animsprite_update_drawable(streakcounter_digits->animation_pair, streakcounter_digits->drawable, true);
    }
}


void streakcounter_digits_animation_set(StreakCounterDigits streakcounter_digits, const AnimListItem* animlist_item) {
    streakcounter_digits->has_animations = !!animlist_item;
    for (int32_t i = 0; i < STREAKCOUNTER_DIGITS; i++) {
        if (streakcounter_digits->animations[i]) animsprite_destroy(&streakcounter_digits->animations[i]);

        if (!animlist_item) {
            streakcounter_digits->animations[i] = NULL;
            continue;
        }

        streakcounter_digits->animations[i] = animsprite_init(animlist_item);
        float delay = streakcounter_digits->delay * math2d_random_int(0, STREAKCOUNTER_DIGITS);
        animsprite_set_delay(streakcounter_digits->animations[i], delay);
    }
}

void streakcounter_digits_animation_pair_set(StreakCounterDigits streakcounter_digits, AnimSprite animsprite_to_clone) {
    if (streakcounter_digits->animation_pair) animsprite_destroy(&streakcounter_digits->animation_pair);
    streakcounter_digits->animation_pair = animsprite_to_clone ? animsprite_clone(animsprite_to_clone) : NULL;
}


int32_t streakcounter_digits_animate(StreakCounterDigits streakcounter_digits, float elapsed) {
    int32_t completed = 1;

    for (int32_t i = 0; i < STREAKCOUNTER_DIGITS; i++) {
        if (streakcounter_digits->buffer[i] < 0) continue;

        statesprite_animate(streakcounter_digits->digits[i], elapsed);

        if (!streakcounter_digits->has_animations) continue;
        if (animsprite_animate(streakcounter_digits->animations[i], elapsed)) continue;

        completed = 0;
        animsprite_update_statesprite(
            streakcounter_digits->animations[i], streakcounter_digits->digits[i], true
        );
    }

    if (!streakcounter_digits->animation_pair) return completed;

    if (!animsprite_animate(streakcounter_digits->animation_pair, elapsed)) completed = 0;
    animsprite_update_drawable(streakcounter_digits->animation_pair, streakcounter_digits->drawable, true);

    return completed;
}

float streakcounter_digits_measure(StreakCounterDigits streakcounter_digits, int32_t value) {
    float draw_width = 0.0f, draw_height = 0.0f;

    // value = math2d_clamp_int(value, 0, STREAKCOUNTER_MAX_VALUE);

    for (int32_t i = 0; i < STREAKCOUNTER_DIGITS; i++)
        streakcounter_digits->buffer[i] = -1;

    int32_t index = STREAKCOUNTER_DIGITS - 1;
    while (value > 0) {
        streakcounter_digits->buffer[index--] = (int8_t)(value % 10);
        value /= 10;
    }

    // add leading zero
    if (index > 0) streakcounter_digits->buffer[index - 1] = 0;

    float measured_width = 0;

    for (int32_t i = 0; i < STREAKCOUNTER_DIGITS; i++) {
        int8_t digit = streakcounter_digits->buffer[i];
        if (digit < 0) continue;

        if (digit >= sizeof(STREAKCOUNTER_NUMBERS)) {
            // this never should happen
            streakcounter_digits->buffer[i] = -1;
            continue;
        }

        const char number[] = {STREAKCOUNTER_NUMBERS[digit], '\0'};

        StateSprite statesprite = streakcounter_digits->digits[i];
        char* state_name = string_concat_for_state_name(
            2, number, streakcounter_digits->selected_state
        );

        if (statesprite_state_toggle(statesprite, state_name)) {
            statesprite_animation_restart(statesprite);

            if (streakcounter_digits->animations[i]) {
                animsprite_restart(streakcounter_digits->animations[i]);
                animsprite_update_statesprite(streakcounter_digits->animations[i], statesprite, true);
            }

            statesprite_get_draw_size(statesprite, &draw_width, &draw_height);
            measured_width += draw_width;
        } else {
            streakcounter_digits->buffer[i] = -1;
        }

        free_chk(state_name);
    }

    return measured_width;
}

void streakcounter_digits_set_draw_location(StreakCounterDigits streakcounter_digits, float x, float y, float gap) {
    float draw_width = 0.0f, draw_height = 0.0f;
    Modifier* modifier = drawable_get_modifier(streakcounter_digits->drawable);
    float max_width = 0.0f;
    float max_height = 0.0f;

    for (int32_t i = 0; i < STREAKCOUNTER_DIGITS; i++) {
        if (streakcounter_digits->buffer[i] < 0) continue;

        statesprite_get_draw_size(streakcounter_digits->digits[i], &draw_width, &draw_height);
        statesprite_set_draw_location(streakcounter_digits->digits[i], x, y);
        x += gap + draw_width;

        if (draw_width > max_width) max_width = draw_width;
        if (draw_height > max_height) max_height = draw_height;
    }

    modifier->x = x;
    modifier->y = y;
    modifier->width = max_width;
    modifier->width = max_height;
}

void streakcounter_digits_draw(StreakCounterDigits streakcounter_digits, PVRContext pvrctx) {
    pvr_context_save(pvrctx);
    drawable_helper_apply_in_context(streakcounter_digits->drawable, pvrctx);

    for (int32_t i = 0; i < STREAKCOUNTER_DIGITS; i++) {
        if (streakcounter_digits->buffer[i] < 0) continue;
        statesprite_draw(streakcounter_digits->digits[i], pvrctx);
    }

    pvr_context_restore(pvrctx);
}
