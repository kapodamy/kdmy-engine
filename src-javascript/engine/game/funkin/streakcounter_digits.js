"use strict";

const STREAKCOUNTER_UI_STREAK_ANIM = "streak_number";// picked from UI animlist
const STREAKCOUNTER_NUMBERS = ['0', '1', '2', '3', '4', '5', '6', '7', '8', '9'];
const STREAKCOUNTER_DIGITS = 4;// maximum visible digits
const STREAKCOUNTER_MAX_VALUE = 9999;// ¡¡this must match the STREAKCOUNTER_DIGITS!!


function streakcounter_digits_init(delay) {
    let streakcounter_digits = {
        buffer: new Array(STREAKCOUNTER_DIGITS),
        digits: new Array(STREAKCOUNTER_DIGITS),
        animations: new Array(STREAKCOUNTER_DIGITS),
        has_animations: 0,

        delay,

        animation_pair: null,
        drawable: null,
        selected_state: Symbol// In C use a unique string pointer
    };

    // fake drawable for pair animations
    streakcounter_digits.drawable = drawable_init(-1, streakcounter_digits, null, null);
    drawable_set_visible(streakcounter_digits.drawable, 0);

    for (let i = 0; i < STREAKCOUNTER_DIGITS; i++) {
        streakcounter_digits.buffer[i] = -1;
        streakcounter_digits.animations[i] = null;
        streakcounter_digits.digits[i] = statesprite_init_from_texture(null);
        statesprite_set_visible(streakcounter_digits.digits[i], 0);
    }

    return streakcounter_digits;
}

function streakcounter_digits_destroy(streakcounter_digits) {
    for (let i = 0; i < STREAKCOUNTER_DIGITS; i++) {
        statesprite_destroy(streakcounter_digits.digits[i]);
        if (streakcounter_digits.animations[i]) animsprite_destroy(streakcounter_digits.animations[i]);
    }
    if (streakcounter_digits.animation_pair) animsprite_destroy(streakcounter_digits.animation_pair);
    drawable_destroy(streakcounter_digits.drawable);
    if (streakcounter_digits.selected_state != Symbol) streakcounter_digits.selected_state = undefined;
    streakcounter_digits = undefined;
}


function streakcounter_digits_state_add(streakcounter_digits, max_hght, mdlhldr, state_name) {
    const temp = [0, 0];
    let success = 0;

    for (let i = 0; i < STREAKCOUNTER_NUMBERS.length; i++) {
        // C only
        //const char number[] = { STREAKCOUNTER_NUMBERS[i], '\0' }"

        // JS & C# only
        const number = STREAKCOUNTER_NUMBERS[i];

        let animation_name = string_concat_for_state_name(2, number, state_name);

        for (let j = 0; j < STREAKCOUNTER_DIGITS; j++) {
            let statesprite_state = statesprite_state_add(
                streakcounter_digits.digits[j], mdlhldr, animation_name, animation_name
            );

            if (!statesprite_state) continue;

            imgutils_get_statesprite_original_size(statesprite_state, temp);
            imgutils_calc_size(temp[0], temp[1], -1, max_hght, temp);

            statesprite_state.draw_width = temp[0];
            statesprite_state.draw_height = temp[1];
            statesprite_state.offset_x = 0;
            statesprite_state.offset_y = 0;
        }

        animation_name = undefined;
    }

    return success;
}

function streakcounter_digits_state_toggle(streakcounter_digits, state_name) {
    if (streakcounter_digits.selected_state != Symbol) streakcounter_digits.selected_state = undefined;
    streakcounter_digits.selected_state = strdup(state_name);
}

function streakcounter_digits_animation_restart(streakcounter_digits) {
    for (let i = 0; i < STREAKCOUNTER_DIGITS; i++) {
        statesprite_animation_restart(streakcounter_digits.digits[i]);
        if (streakcounter_digits.has_animations)
            animsprite_restart(streakcounter_digits.animations[i]);
    }
    if (streakcounter_digits.animation_pair)
        animsprite_restart(streakcounter_digits.animation_pair);
}

function streakcounter_digits_animation_end(streakcounter_digits) {
    for (let i = 0; i < STREAKCOUNTER_DIGITS; i++) {
        statesprite_animation_end(streakcounter_digits.digits[i]);
        if (streakcounter_digits.has_animations)
            animsprite_force_end(streakcounter_digits.animations[i]);
    }
    if (streakcounter_digits.animation_pair)
        animsprite_force_end(streakcounter_digits.animation_pair);
}


function streakcounter_digits_animation_set(streakcounter_digits, animlist_item) {
    streakcounter_digits.has_animations = !!animlist_item;
    for (let i = 0; i < STREAKCOUNTER_DIGITS; i++) {
        if (streakcounter_digits.animations[i]) animsprite_destroy(streakcounter_digits.animations[i]);

        if (!animlist_item) {
            streakcounter_digits.animations[i] = null;
            continue;
        }

        streakcounter_digits.animations[i] = animsprite_init(animlist_item);
        let delay = streakcounter_digits.delay * math2d_random_int(0, STREAKCOUNTER_DIGITS);
        animsprite_set_delay(streakcounter_digits.animations[i], delay);
    }
}

function streakcounter_digits_animation_pair_set(streakcounter_digits, animsprite_to_clone) {
    if (streakcounter_digits.animation_pair) animsprite_destroy(streakcounter_digits.animation_pair);
    streakcounter_digits.animation_pair = animsprite_to_clone ? animsprite_clone(animsprite_to_clone) : null;
}


function streakcounter_digits_animate(streakcounter_digits, elapsed) {
    let completed = 1;

    for (let i = 0; i < STREAKCOUNTER_DIGITS; i++) {
        if (streakcounter_digits.buffer[i] < 0) continue;

        statesprite_animate(streakcounter_digits.digits[i], elapsed);

        if (!streakcounter_digits.has_animations) continue;
        if (animsprite_animate(streakcounter_digits.animations[i], elapsed)) continue;

        completed = 0;
        animsprite_update_statesprite(
            streakcounter_digits.animations[i], streakcounter_digits.digits[i], 1
        );
    }

    if (!streakcounter_digits.animation_pair) return completed;

    if (!animsprite_animate(streakcounter_digits.animation_pair, elapsed)) completed = 0;
    animsprite_update_drawable(streakcounter_digits.animation_pair, streakcounter_digits.drawable, 1);

    return completed;
}

function streakcounter_digits_meansure(streakcounter_digits, value) {
    const draw_size = [0, 0];

    //value = math2d_clamp(value, 0, STREAKCOUNTER_MAX_VALUE);

    for (let i = 0; i < STREAKCOUNTER_DIGITS; i++)
        streakcounter_digits.buffer[i] = -1;

    let index = STREAKCOUNTER_DIGITS - 1;
    while (value > 0) {
        streakcounter_digits.buffer[index--] = Math.trunc(value % 10);
        value = Math.trunc(value / 10);
    }

    // add leading zero
    if (index > 0) streakcounter_digits.buffer[index - 1] = 0;

    let meansured_width = 0;

    for (let i = 0; i < STREAKCOUNTER_DIGITS; i++) {
        let digit = streakcounter_digits.buffer[i];
        if (digit < 0) continue;

        if (digit >= STREAKCOUNTER_NUMBERS.length) {
            // this never should happen
            streakcounter_digits.buffer[i] = -1;
            continue;
        }

        // C only
        //const char number[] = { STREAKCOUNTER_NUMBERS[i], '\0' }"

        // JS & C# only
        const number = STREAKCOUNTER_NUMBERS[i];

        let statesprite = streakcounter_digits.digits[i];
        let state_name = string_concat_for_state_name(
            2, number, streakcounter_digits.selected_state
        );

        if (statesprite_state_toggle(statesprite, state_name)) {
            statesprite_animation_restart(statesprite);

            if (streakcounter_digits.animations[i]) {
                animsprite_restart(streakcounter_digits.animations[i]);
                animsprite_update_statesprite(streakcounter_digits.animations[i], statesprite, 1);
            }

            statesprite_get_draw_size(statesprite, draw_size);
            meansured_width += draw_size[0];
        } else {
            streakcounter_digits.buffer[i] = -1;
        }

        state_name = undefined;
    }

    return meansured_width;
}

function streakcounter_digits_set_draw_location(streakcounter_digits, x, y, gap) {
    const draw_size = [0, 0];
    const modifier = drawable_get_modifier(streakcounter_digits.drawable);
    let max_width = 0;
    let max_height = 0;

    for (let i = 0; i < STREAKCOUNTER_DIGITS; i++) {
        if (streakcounter_digits.buffer[i] < 0) continue;

        statesprite_get_draw_size(streakcounter_digits.digits[i], draw_size);
        statesprite_set_draw_location(streakcounter_digits.digits[i], x, y);
        x += gap + draw_size[0];

        if (draw_size[0] > max_width) max_width = draw_size[0];
        if (draw_size[1] > max_height) max_height = draw_size[1];
    }

    modifier.x = x;
    modifier.y = y;
    modifier.width = max_width;
    modifier.width = max_height;
}

function streakcounter_digits_draw(streakcounter_digits, pvrctx) {
    pvr_context_save(pvrctx);
    drawable_helper_apply_in_context(streakcounter_digits.drawable, pvrctx);

    for (let i = 0; i < STREAKCOUNTER_DIGITS; i++) {
        if (streakcounter_digits.buffer[i] < 0) continue;
        statesprite_draw(streakcounter_digits.digits[i], pvrctx);
    }

    pvr_context_restore(pvrctx);
}

