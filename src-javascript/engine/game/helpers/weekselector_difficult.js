"use strict";

const WEEKSELECTOR_DIFFICULTY_UI_NO_WARNS = "ui_difficult_no_warns";
const WEEKSELECTOR_DIFFICULTY_UI_WARNS = "ui_difficult_warns";

async function weekselector_difficult_init(animlist, modelholder, layout) {
    let weekdifficult = {
        animsprite: animsprite_init_from_animlist(animlist, WEEKSELECTOR_ARROW_SPRITE_NAME),

        index: -1,
        list: null,
        list_size: 0,

        sprite_commons: statesprite_init_from_texture(null),
        sprite_customs: statesprite_init_from_texture(null),

        icon_locked: modelholder_create_sprite(modelholder, WEEKSELECTOR_LOCKED, 1),
        icon_left: modelholder_create_sprite(modelholder, WEEKSELECTOR_ARROW_SPRITE_NAME, 1),
        icon_right: modelholder_create_sprite(modelholder, WEEKSELECTOR_ARROW_SPRITE_NAME, 1),

        placeholder1: layout_get_placeholder(layout, WEEKSELECTOR_DIFFICULTY_UI_NO_WARNS),
        placeholder2: layout_get_placeholder(layout, WEEKSELECTOR_DIFFICULTY_UI_WARNS),

        has_left: 1,
        has_right: 1,
        is_common: 0,
        is_locked: 0,

        drawable: null
    };

    sprite_flip_rendered_texture(weekdifficult.icon_right, 1, 0);

    weekdifficult.drawable = drawable_init(
        -1, weekdifficult, weekselector_difficult_draw, weekselector_difficult_animate
    );
    weekselector_difficult_visible(weekdifficult, 1);

    statesprite_set_visible(weekdifficult.sprite_commons, 0);
    statesprite_set_visible(weekdifficult.sprite_customs, 0);

    sprite_set_visible(weekdifficult.icon_locked, 0);
    sprite_set_visible(weekdifficult.icon_left, 0);
    sprite_set_visible(weekdifficult.icon_right, 0);

    // prebuild the list
    statesprite_state_add(
        weekdifficult.sprite_commons, modelholder, FUNKIN_DIFFICULT_EASY, FUNKIN_DIFFICULT_EASY
    );
    statesprite_state_add(
        weekdifficult.sprite_commons, modelholder, FUNKIN_DIFFICULT_NORMAL, FUNKIN_DIFFICULT_NORMAL
    );
    statesprite_state_add(
        weekdifficult.sprite_commons, modelholder, FUNKIN_DIFFICULT_HARD, FUNKIN_DIFFICULT_HARD
    );

    return weekdifficult;
}

function weekselector_difficult_destroy(weekdifficult) {
    if (weekdifficult.animsprite) animsprite_destroy(weekdifficult.animsprite);

    drawable_destroy(weekdifficult.drawable);

    statesprite_destroy(weekdifficult.sprite_commons);
    statesprite_destroy(weekdifficult.sprite_customs);

    sprite_destroy_full(weekdifficult.icon_locked);
    sprite_destroy_full(weekdifficult.icon_left);
    sprite_destroy_full(weekdifficult.icon_right);

    // do not disposed the names on the list
    weekdifficult.list = undefined;
    weekdifficult = undefined;
}


function weekselector_difficult_draw(weekdifficult, pvrctx) {
    pvr_context_save(pvrctx);

    if (weekdifficult.is_locked) sprite_draw(weekdifficult.icon_locked, pvrctx);
    sprite_draw(weekdifficult.icon_left, pvrctx);
    sprite_draw(weekdifficult.icon_right, pvrctx);

    if (weekdifficult.is_locked)
        pvr_context_set_global_alpha(pvrctx, WEEKSELECTOR_PREVIEW_DISABLED_ALPHA);

    if (weekdifficult.is_common)
        statesprite_draw(weekdifficult.sprite_commons, pvrctx);
    else
        statesprite_draw(weekdifficult.sprite_customs, pvrctx);

    if (weekdifficult.is_locked) {
        pvr_context_set_global_alpha(pvrctx, 1.0);
        sprite_draw(weekdifficult.icon_locked, pvrctx);
    }

    pvr_context_restore(pvrctx);
}

function weekselector_difficult_animate(weekdifficult, elapsed) {
    if (weekdifficult.animsprite) animsprite_animate(weekdifficult.animsprite, elapsed);

    sprite_animate(weekdifficult.icon_left, elapsed);
    sprite_animate(weekdifficult.icon_right, elapsed);

    statesprite_animate(weekdifficult.sprite_commons, elapsed);
    statesprite_animate(weekdifficult.sprite_customs, elapsed);

    if (weekdifficult.has_left)
        animsprite_update_sprite(weekdifficult.animsprite, weekdifficult.icon_left, 0);
    if (weekdifficult.has_right)
        animsprite_update_sprite(weekdifficult.animsprite, weekdifficult.icon_right, 0);
}


function weekselector_difficult_select(weekdifficult, new_index) {
    if (new_index < 0 || new_index >= weekdifficult.list_size) return 0;

    weekdifficult.is_common = weekdifficult.list[new_index].is_common;
    weekdifficult.is_locked = weekdifficult.list[new_index].is_locked;
    weekdifficult.index = new_index;
    weekdifficult.has_left = new_index > 0;
    weekdifficult.has_right = new_index < (weekdifficult.list_size - 1);

    statesprite_state_toggle(
        weekdifficult.is_common ? weekdifficult.sprite_commons : weekdifficult.sprite_customs,
        weekdifficult.list[new_index].name
    );
    sprite_set_alpha(
        weekdifficult.icon_left, weekdifficult.has_left ? 1.0 : WEEKSELECTOR_ARROW_DISABLED_ALPHA
    );
    sprite_set_alpha(
        weekdifficult.icon_right, weekdifficult.has_right ? 1.0 : WEEKSELECTOR_ARROW_DISABLED_ALPHA
    );

    return 1;
}

function weekselector_difficult_select_default(weekdifficult) {
    for (let i = 0; i < weekdifficult.list_size; i++) {
        if (weekdifficult.list[i].name == FUNKIN_DIFFICULT_NORMAL) {
            weekselector_difficult_select(weekdifficult, i);
            return;
        }
    }
    weekselector_difficult_select(weekdifficult, 0);
}

function weekselector_difficult_scroll(weekdifficult, offset) {
    return weekselector_difficult_select(weekdifficult, weekdifficult.index + offset);
}

function weekselector_difficult_visible(weekdifficult, visible) {
    drawable_set_visible(weekdifficult.drawable, visible);
}

function weekselector_difficult_get_selected(weekdifficult) {
    if (weekdifficult.index < 0 || weekdifficult.index >= weekdifficult.list_size) return null;
    let difficult = weekdifficult.list[weekdifficult.index];
    return difficult.is_locked ? null : difficult.name;
}

function weekselector_difficult_peek_statesprite(weekdifficult) {
    if (weekdifficult.index < 0 || weekdifficult.index >= weekdifficult.list_size) return null;

    if (weekdifficult.list[weekdifficult.index].is_common)
        return weekdifficult.sprite_commons;
    else
        return weekdifficult.sprite_customs;
}

function weekselector_difficult_relayout(weekdifficult, use_alt_placeholder) {
    let old_placeholder = use_alt_placeholder ? weekdifficult.placeholder1 : weekdifficult.placeholder2;
    let placeholder = use_alt_placeholder ? weekdifficult.placeholder2 : weekdifficult.placeholder1;
    if (!placeholder) return;

    if (old_placeholder) old_placeholder.vertex = null;
    placeholder.vertex = weekdifficult.drawable;
    drawable_helper_update_from_placeholder(weekdifficult.drawable, placeholder);

    // resize&locate left arrow
    imgutils_calc_rectangle_in_sprite(
        placeholder.x, placeholder.y, placeholder.width, placeholder.height,
        ALIGN_START, ALIGN_CENTER, weekdifficult.icon_left
    );

    // resize&locate right arrow
    imgutils_calc_rectangle_in_sprite(
        placeholder.x, placeholder.y, placeholder.width, placeholder.height,
        ALIGN_END, ALIGN_CENTER, weekdifficult.icon_right
    );

    // calculte the horizontal space between arrows
    let draw_size = [0, 0];
    sprite_get_draw_size(weekdifficult.icon_left, draw_size);
    let difficult_width = placeholder.width - (draw_size[0] * 2);

    // resize&locate commons difficults
    for (let state of linkedlist_iterate4(statesprite_state_list(weekdifficult.sprite_commons))) {
        imgutils_calc_rectangle_in_statesprite_state(
            draw_size[0], 0, difficult_width, placeholder.height, ALIGN_CENTER, ALIGN_CENTER, state
        );
    }
    statesprite_set_draw_location(weekdifficult.sprite_commons, placeholder.x, placeholder.y);
    statesprite_state_apply(weekdifficult.sprite_commons, null);

    // resize&locate custom difficults
    for (let state of linkedlist_iterate4(statesprite_state_list(weekdifficult.sprite_customs))) {
        imgutils_calc_rectangle_in_statesprite_state(
            draw_size[0], 0, difficult_width, placeholder.height, ALIGN_CENTER, ALIGN_CENTER, state
        );
    }
    statesprite_set_draw_location(weekdifficult.sprite_customs, placeholder.x, placeholder.y);
    statesprite_state_apply(weekdifficult.sprite_customs, null);

    // resize&locate locked icon
    imgutils_calc_rectangle_in_sprite(
        placeholder.x, placeholder.y, placeholder.width, placeholder.height,
        ALIGN_CENTER, ALIGN_CENTER, weekdifficult.icon_locked
    );
}

function weekselector_difficult_selected_is_locked(weekdifficult) {
    if (weekdifficult.index < 0 || weekdifficult.index >= weekdifficult.list_size) return 1;
    return weekdifficult.list[weekdifficult.index].is_locked;
}


async function weekselector_difficult_load(weekdifficult, weekinfo, default_difficult) {
    if (weekdifficult.animsprite) animsprite_restart(weekdifficult.animsprite);

    // dispose previous list and custom difficults
    weekdifficult.list = undefined;
    statesprite_destroy(weekdifficult.sprite_customs);
    weekdifficult.sprite_customs = statesprite_init_from_texture(null);
    statesprite_set_visible(weekdifficult.sprite_customs, 0);

    if (!weekinfo) return;

    weekdifficult.list_size = 3;
    if (weekinfo.custom_difficults_model) weekdifficult.list_size += weekinfo.custom_difficults_size;

    let index = 0;
    weekdifficult.list = new Array(weekdifficult.list_size);

    if (weekinfo.has_difficulty_easy)
        weekdifficult.list[index++] = { name: FUNKIN_DIFFICULT_EASY, is_locked: 0, is_common: 1 };
    if (weekinfo.has_difficulty_normal)
        weekdifficult.list[index++] = { name: FUNKIN_DIFFICULT_NORMAL, is_locked: 0, is_common: 1 };
    if (weekinfo.has_difficulty_hard)
        weekdifficult.list[index++] = { name: FUNKIN_DIFFICULT_HARD, is_locked: 0, is_common: 1 };

    if (weekinfo.custom_difficults_model) {
        let modelholder = await modelholder_init(weekinfo.custom_difficults_model);

        if (modelholder) {
            for (let i = 0; i < weekinfo.custom_difficults_size; i++) {
                let difficult = weekinfo.custom_difficults[i];
                weekdifficult.list[index++] = {
                    name: difficult.name,
                    is_locked: !funkinsave_contains_unlock_directive(difficult.unlock_directive),
                    is_common: 0
                };
                statesprite_state_add(
                    weekdifficult.sprite_customs, modelholder, difficult.name, difficult.name
                );
            }
            modelholder_destroy(modelholder);
        }
    }

    console.assert(index <= weekdifficult.list_size, "weekselector_difficult_load() list overflow");
    weekdifficult.list_size = index;// trim the list

    weekselector_difficult_relayout(weekdifficult, 0);

    // select default difficult
    let selected = default_difficult == null ? FUNKIN_DIFFICULT_NORMAL : default_difficult;
    let last_difficult_played = funkinsave_get_last_played_difficult();// read-only string
    if (!default_difficult && last_difficult_played) selected = last_difficult_played;

    // select normal difficult
    for (let i = 0; i < weekdifficult.list_size; i++) {
        if (weekdifficult.list[i].name == selected) {
            weekselector_difficult_select(weekdifficult, i);
            return;
        }
    }

    // fallback to first one
    weekselector_difficult_select(weekdifficult, 0);
}

