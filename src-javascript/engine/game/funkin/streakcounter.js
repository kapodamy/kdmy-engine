"use strict";

const STREAKCOUNTER_PREFIX_COMBO = "combo";
const STREAKCOUNTER_UI_COMBO_ANIM = "streak_text";// picked from UI animlist
const STREAKCOUNTER_STREAK_BUFFER_SIZE = 5;



function streakcounter_init(placeholder, combo_height, number_gap, delay) {
    //
    // Notes:
    //      * the width is optional (should no be present)
    //      * alignments are ignored
    //
    let number_height = 0;
    let reference_width = 0;
    if (placeholder) {
        if (placeholder.height > 0) number_height = placeholder.height;
        if (placeholder.width > 0) reference_width = placeholder.width;
    }

    let streakcounter = {
        drawable: null,
        modifier: null,

        last_streak: -1,

        number_height, number_gap, delay,

        ignore_combo_location: 1,

        combo_width: -1,
        combo_height: combo_height,
        combo_sprite: statesprite_init_from_texture(null),
        combo_animation: null,
        combo_enabled: combo_height > 0,

        numbers_items: new Array(STREAKCOUNTER_STREAK_BUFFER_SIZE),
        numbers_id: 0
    };

    statesprite_set_visible(streakcounter.combo_sprite, 0);

    streakcounter.drawable = drawable_init(0, streakcounter, streakcounter_draw, streakcounter_animate);

    if (placeholder) {
        placeholder.vertex = streakcounter.drawable;
        drawable_helper_update_from_placeholder(streakcounter.drawable, placeholder);
    }

    streakcounter.modifier = drawable_get_modifier(streakcounter.drawable);
    streakcounter.modifier.x += reference_width / -2;
    streakcounter.modifier.y += number_height / -2;
    streakcounter.modifier.height = Math.max(combo_height, number_height);

    for (let i = 0; i < STREAKCOUNTER_STREAK_BUFFER_SIZE; i++) {
        streakcounter.numbers_items[i] = { id: -1, digits: streakcounter_digits_init(delay) };
    }

    return streakcounter;
}

function streakcounter_destroy(streakcounter) {
    drawable_destroy(streakcounter.drawable);

    for (let i = 0; i < STREAKCOUNTER_STREAK_BUFFER_SIZE; i++)
        streakcounter_digits_destroy(streakcounter.numbers_items[i].digits);

    if (streakcounter.combo_animation) animsprite_destroy(streakcounter.combo_animation);
    statesprite_destroy(streakcounter.combo_sprite);

    streakcounter = undefined;
}


function streakcounter_peek_streak(streakcounter, playerstats) {
    let value = playerstats_get_combo_streak(playerstats);

    if (streakcounter.last_streak == value) return 0;

    if (value >= FUNKIN_COMBO_STREAK_VISIBLE_AFTER) {
        let unused_item = streakcounter_internal_pick_item(
            streakcounter.numbers_items, STREAKCOUNTER_STREAK_BUFFER_SIZE, streakcounter.numbers_id++
        );

        // center the number sprites in the draw location (y axis is already centered)
        value = math2d_clamp(value, 0, STREAKCOUNTER_MAX_VALUE);
        let draw_width = streakcounter_digits_meansure(unused_item.digits, value);
        let x = streakcounter.modifier.x;
        let y = streakcounter.modifier.y;

        if (streakcounter.combo_enabled && streakcounter.ignore_combo_location) {
            x -= ((streakcounter.combo_width + streakcounter.number_gap * 2) + draw_width) / 2;
            statesprite_set_draw_location(streakcounter.combo_sprite, x, y);
            x += streakcounter.combo_width;
        } else {
            x += draw_width / -2;
        }

        // set the draw location
        streakcounter_digits_set_draw_location(unused_item.digits, x, y, streakcounter.number_gap);

        qsort(
            streakcounter.numbers_items, STREAKCOUNTER_STREAK_BUFFER_SIZE, NaN,
            streakcounter_internal_sort
        );

        if (streakcounter.combo_enabled) {
            statesprite_animation_restart(streakcounter.combo_sprite);
            if (streakcounter.combo_animation) animsprite_restart(streakcounter.combo_animation);
        }
    }

    let streak_loose;
    if (streakcounter.last_streak >= FUNKIN_COMBO_STREAK_VISIBLE_AFTER)
        streak_loose = value < streakcounter.last_streak;
    else
        streak_loose = 0;

    streakcounter.last_streak = value;
    return streak_loose;
}

function streakcounter_reset(streakcounter) {
    streakcounter.last_streak = -1;
}

function streakcounter_hide_combo_sprite(streakcounter, hide) {
    let combo_enabled = !hide;
    if (combo_enabled && linkedlist_count(statesprite_state_list(streakcounter.combo_sprite)) < 1) {
        console.warn(
            "streakcounter_hide_combo_sprite() failed, " +
            "combo sprite does not have states to show"
        );
        return;
    }
    streakcounter.combo_enabled = combo_enabled;
}

function streakcounter_set_combo_draw_location(streakcounter, x, y) {
    streakcounter.ignore_combo_location = 0;
    statesprite_set_draw_location(streakcounter.combo_sprite, x, y);
}



function streakcounter_state_add(streakcounter, combo_modelholder, number_modelholder, state_name) {
    const temp = [0, 0];

    let success = 0;

    for (let i = 0; i < STREAKCOUNTER_STREAK_BUFFER_SIZE; i++) {
        success += streakcounter_digits_state_add(
            streakcounter.numbers_items[i].digits,
            streakcounter.number_height,
            number_modelholder,
            state_name
        );
    }

    if (!combo_modelholder) return success;

    let animation_name = string_concat_for_state_name(2, STREAKCOUNTER_PREFIX_COMBO, state_name);
    let statesprite_state = statesprite_state_add(
        streakcounter.combo_sprite, combo_modelholder, animation_name, state_name
    );
    animation_name = undefined;


    if (statesprite_state) {
        imgutils_get_statesprite_original_size(statesprite_state, temp);
        imgutils_calc_size(temp[0], temp[1], -1, streakcounter.combo_height, temp);
        statesprite_state.draw_width = temp[0];
        statesprite_state.draw_height = temp[1];
        statesprite_state.offset_x = 0;
        statesprite_state.offset_y = temp[1] / -2;
        streakcounter.combo_width = temp[0];
        success++;
    }

    return success;
}

function streakcounter_state_toggle(streakcounter, state_name) {
    for (let i = 0; i < STREAKCOUNTER_STREAK_BUFFER_SIZE; i++)
        streakcounter_digits_state_toggle(streakcounter.numbers_items[i].digits, state_name);

    if (statesprite_state_toggle(streakcounter.combo_sprite, state_name)) {
        streakcounter.combo_width = statesprite_state_get(streakcounter.combo_sprite).draw_width;
        return 1;
    }

    return 0;
}


function streakcounter_set_alpha(streakcounter, alpha) {
    drawable_set_alpha(streakcounter.drawable, alpha);
}

function streakcounter_set_offsetcolor(streakcounter, r, g, b, a) {
    drawable_set_offsetcolor(streakcounter.drawable, r, g, b, a);
}

function streakcounter_set_offsetcolor_to_default(streakcounter) {
    drawable_set_offsetcolor_to_default(streakcounter.drawable);
}

function streakcounter_get_modifier(streakcounter) {
    return drawable_get_modifier(streakcounter.drawable);
}


function streakcounter_set_number_animation(streakcounter, animlist) {
    if (!animlist) return;

    let animlist_item = animlist_get_animation(animlist, STREAKCOUNTER_UI_STREAK_ANIM);
    if (!animlist_item) return;

    for (let i = 0; i < STREAKCOUNTER_STREAK_BUFFER_SIZE; i++)
        streakcounter_digits_animation_set(streakcounter.numbers_items[i].digits, animlist_item);
}

function streakcounter_set_combo_animation(streakcounter, animlist) {
    if (!animlist) return;
    let animlist_item = animlist_get_animation(animlist, STREAKCOUNTER_UI_COMBO_ANIM);

    if (!animlist_item) return;
    streakcounter.combo_animation = animsprite_init(animlist_item);
}


function streakcounter_animation_set(streakcounter, animsprite) {
    for (let i = 0; i < STREAKCOUNTER_STREAK_BUFFER_SIZE; i++)
        streakcounter_digits_animation_pair_set(streakcounter.numbers_items[i].digits, animsprite);
}

function streakcounter_animation_restart(streakcounter) {
    for (let i = 0; i < STREAKCOUNTER_STREAK_BUFFER_SIZE; i++)
        streakcounter_digits_animation_restart(streakcounter.numbers_items[i].digits);


    statesprite_animation_restart(streakcounter.combo_sprite);
    if (streakcounter.combo_animation) animsprite_restart(streakcounter.combo_animation);
}

function streakcounter_animation_end(streakcounter) {
    for (let i = 0; i < STREAKCOUNTER_STREAK_BUFFER_SIZE; i++)
        streakcounter_digits_animation_end(streakcounter.numbers_items[i].digits);

    statesprite_animation_end(streakcounter.combo_sprite);
    if (streakcounter.combo_animation)
        animsprite_force_end3(streakcounter.combo_animation, streakcounter.combo_sprite);
}


function streakcounter_animate(streakcounter, elapsed) {
    let res = 0;

    for (let i = 0; i < STREAKCOUNTER_STREAK_BUFFER_SIZE; i++) {
        if (streakcounter.numbers_items[i].id < 0) continue;
        if (streakcounter_digits_animate(streakcounter.numbers_items[i].digits, elapsed)) {
            res++;
            streakcounter.numbers_items[i].id = -1;// animation completed, hide
        }
    }

    res += statesprite_animate(streakcounter.combo_sprite, elapsed);
    if (streakcounter.combo_animation) {
        res += animsprite_animate(streakcounter.combo_animation, elapsed);
        animsprite_update_statesprite(streakcounter.combo_animation, streakcounter.combo_sprite);
    }

    return res;
}

function streakcounter_draw(streakcounter, pvrctx) {
    if (streakcounter.last_streak < FUNKIN_COMBO_STREAK_VISIBLE_AFTER) return;

    pvr_context_save(pvrctx);
    drawable_helper_apply_in_context(streakcounter.drawable, pvrctx);

    let draw_combo = streakcounter.combo_enabled;

    for (let i = 0; i < STREAKCOUNTER_STREAK_BUFFER_SIZE; i++) {
        if (streakcounter.numbers_items[i].id < 0) continue;

        if (draw_combo) {
            draw_combo = 0;
            statesprite_draw(streakcounter.combo_sprite, pvrctx);
        }

        streakcounter_digits_draw(streakcounter.numbers_items[i].digits, pvrctx);
    }

    pvr_context_restore(pvrctx);
}



function streakcounter_internal_pick_item(array, size, new_id) {
    for (let i = 0; i < size; i++) {
        if (array[i].id < 0) {
            array[i].id = new_id;
            return array[i];
        }
    }

    let oldest_id = MATH2D_MAX_INT32;
    let oldest_item = null;

    for (let i = 0; i < size; i++) {
        if (array[i].id < oldest_id) {
            oldest_id = array[i].id;
            oldest_item = array[i];
        }
    }

    console.assert(oldest_item != null);
    oldest_item.id = new_id;
    return oldest_item;
}

function streakcounter_internal_sort(item1, item2) {
    return item1.id - item2.id;
}

