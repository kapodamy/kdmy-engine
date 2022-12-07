"use strict";

const RANKINGCOUNTER_PREFIX_SHIT = "shit";
const RANKINGCOUNTER_PREFIX_BAD = "bad";
const RANKINGCOUNTER_PREFIX_GOOD = "good";
const RANKINGCOUNTER_PREFIX_SICK = "sick";
const RANKINGCOUNTER_PREFIX_MISS = "miss";
const RANKINGCOUNTER_PREFIX_PENALITY = "penality";

const RANKINGCOUNTER_TEXT_COLOR_SHIT = 0xFF0000;// red
const RANKINGCOUNTER_TEXT_COLOR_BAD = 0xFF0000;// red
const RANKINGCOUNTER_TEXT_COLOR_GOOD = 0x00FF00;// green
const RANKINGCOUNTER_TEXT_COLOR_SICK = 0x00FFFF;// cyan
const RANKINGCOUNTER_TEXT_COLOR_MISS = 0x151B54;// midnight blue
const RANKINGCOUNTER_TEXT_COLOR_PENALITY = 0x000000;// black (not implemented)

const RANKINGCOUNTER_RANKING_BUFFER_SIZE = 5;
const RANKINGCOUNTER_FORMAT_TIME = "$2dms";// prints 12.345 as "12.34ms"
const RANKINGCOUNTER_FORMAT_PERCENT = "$0d%";// prints 99.7899 as "99%"
const RANKINGCOUNTER_UI_RANKING_ANIM = "ranking";// picked from UI animlist
const RANKINGCOUNTER_UI_RANKING_TEXT_ANIM = "ranking_text";// picked from UI animlist

const RANKINGCOUNTER_TEXT_MISS = "MISS";


function rankingcounter_init(plchldr_rank, plchldr_accuracy, fnthldr) {
    //
    // Notes:
    //      * the width is optional (should no be present)
    //      * alignments are ignored
    //      * the font size is calculated for the screen to avoid huge font characters in VRAM
    //
    let ranking_height = 0;
    if (plchldr_rank && plchldr_rank.height > 0) ranking_height = plchldr_rank.height;

    let font_size = 20;
    if (plchldr_accuracy) {
        if (plchldr_accuracy.height > 0) {
            font_size = plchldr_accuracy.height;
        } else {
            // dismiss
            plchldr_accuracy = null;
        }
    }

    let rankingcounter = {
        last_iterations: 0,

        textsprite: textsprite_init2(fnthldr, font_size, 0x000000),
        show_accuracy: 0,
        enable_accuracy: 1,
        enable_accuracy_percent: 0,

        selected_state: Symbol,// in C replace this with a unique value
        drawable_animation: null,

        ranking_items: new Array(RANKINGCOUNTER_RANKING_BUFFER_SIZE),

        ranking_id: 0,
        ranking_height: ranking_height,

        drawable_accuracy: null,
        drawable_rank: null,

        correction_x: 0,
        correction_y: 0
    };

    rankingcounter.drawable_rank = drawable_init(
        -1, rankingcounter, rankingcounter_draw1, rankingcounter_animate1
    );
    rankingcounter.drawable_accuracy = drawable_init(
        -1, rankingcounter, rankingcounter_draw2, rankingcounter_animate2
    );

    if (plchldr_rank) {
        drawable_helper_update_from_placeholder(rankingcounter.drawable_rank, plchldr_rank);
        plchldr_rank.vertex = rankingcounter.drawable_rank;

        let x = plchldr_rank.x;
        if (plchldr_rank.width > 0) x -= plchldr_rank.width / 2;

        for (let i = 0; i < RANKINGCOUNTER_RANKING_BUFFER_SIZE; i++) {
            let statesprite = statesprite_init_from_texture(null);
            statesprite_set_draw_location(statesprite, x, plchldr_rank.y);
            statesprite_set_visible(statesprite, 0);

            rankingcounter.ranking_items[i] = { statesprite, animsprite: null, id: -1 };
        }
    } else {
        for (let i = 0; i < RANKINGCOUNTER_RANKING_BUFFER_SIZE; i++) {
            let statesprite = statesprite_init_from_texture(null);
            rankingcounter.ranking_items[i] = { statesprite, animsprite: null, id: -1 };
        }
    }

    if (plchldr_accuracy) {
        drawable_helper_update_from_placeholder(rankingcounter.drawable_accuracy, plchldr_accuracy);
        plchldr_accuracy.vertex = rankingcounter.drawable_accuracy;

        let x = plchldr_accuracy.x;
        if (plchldr_accuracy.width > 0) x -= plchldr_accuracy.width / 2;

        textsprite_set_draw_location(
            rankingcounter.textsprite, x, plchldr_accuracy.y
        );
        rankingcounter.correction_x = x;
        rankingcounter.correction_y = plchldr_accuracy.y;

        textsprite_set_align(rankingcounter.textsprite, ALIGN_CENTER, ALIGN_CENTER);

        // center the accuracy text on the draw location
        textsprite_set_max_draw_size(rankingcounter.textsprite, 0, 0);
    }

    return rankingcounter;
}

function rankingcounter_destroy(rankingcounter) {
    ModuleLuaScript.kdmyEngine_drop_shared_object(rankingcounter);

    textsprite_destroy(rankingcounter.textsprite);
    drawable_destroy(rankingcounter.drawable_rank);
    drawable_destroy(rankingcounter.drawable_accuracy);
    if (rankingcounter.drawable_animation) animsprite_destroy(rankingcounter.drawable_animation);

    for (let i = 0; i < RANKINGCOUNTER_RANKING_BUFFER_SIZE; i++) {
        if (rankingcounter.ranking_items[i].animsprite)
            animsprite_destroy(rankingcounter.ranking_items[i].animsprite);
        if (rankingcounter.ranking_items[i].statesprite)
            statesprite_destroy(rankingcounter.ranking_items[i].statesprite);
    }

    // destroy the attached animation of textsprite
    let old_animation = textsprite_animation_set(rankingcounter.textsprite, null);
    if (old_animation) animsprite_destroy(old_animation);

    rankingcounter.selected_state = undefined;
    rankingcounter = undefined;
}


function rankingcounter_add_state(rankingcounter, modelholder, state_name) {
    const max_height = rankingcounter.ranking_height;

    let success = 0;
    for (let i = 0; i < RANKINGCOUNTER_RANKING_BUFFER_SIZE; i++) {
        let statesprite = rankingcounter.ranking_items[i].statesprite;

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

function rankingcounter_toggle_state(rankingcounter, state_name) {
    if (rankingcounter.selected_state != Symbol) rankingcounter.selected_state = undefined;
    rankingcounter.selected_state = strdup(state_name);
}


function rankingcounter_peek_ranking(rankingcounter, playerstats) {
    let value, format, ranking, color;

    let interations = playerstats_get_iterations(playerstats);
    if (interations == rankingcounter.last_iterations) return;
    rankingcounter.last_iterations = interations;

    let rank = playerstats_get_last_ranking(playerstats);

    switch (rank) {
        case PLAYERSTATS_RANK_NONE:
            return;
        case PLAYERSTATS_RANK_SICK:
            ranking = RANKINGCOUNTER_PREFIX_SICK;
            color = RANKINGCOUNTER_TEXT_COLOR_SICK;
            break;
        case PLAYERSTATS_RANK_GOOD:
            ranking = RANKINGCOUNTER_PREFIX_GOOD;
            color = RANKINGCOUNTER_TEXT_COLOR_GOOD;
            break;
        case PLAYERSTATS_RANK_BAD:
            ranking = RANKINGCOUNTER_PREFIX_BAD;
            color = RANKINGCOUNTER_TEXT_COLOR_BAD;
            break;
        case PLAYERSTATS_RANK_SHIT:
            ranking = RANKINGCOUNTER_PREFIX_SHIT;
            color = RANKINGCOUNTER_TEXT_COLOR_SHIT;
            break;
        case PLAYERSTATS_RANK_MISS:
            ranking = RANKINGCOUNTER_PREFIX_MISS;
            color = RANKINGCOUNTER_TEXT_COLOR_MISS;
            break;
        case PLAYERSTATS_RANK_PENALITY:
            ranking = RANKINGCOUNTER_PREFIX_PENALITY;
            color = RANKINGCOUNTER_TEXT_COLOR_PENALITY;
            break;
        default:
            return;
    }

    // find an unused item
    let item = rankingcounter_internal_pick_item(
        rankingcounter.ranking_items, rankingcounter.ranking_id++
    );

    // toggle state for this item
    let state_name = string_concat_for_state_name(2, ranking, rankingcounter.selected_state);
    if (statesprite_state_toggle(item.statesprite, state_name)) {
        statesprite_animation_restart(item.statesprite);
        if (item.animsprite) animsprite_restart(item.animsprite);

        // sort visible items (old items first)
        qsort(
            rankingcounter.ranking_items,
            RANKINGCOUNTER_RANKING_BUFFER_SIZE, NaN,
            rankingcounter_internal_sort
        );
    } else {
        // no state for this item, hide it
        item.id = -1;
    }
    state_name = undefined;

    if (!rankingcounter.enable_accuracy || rank == PLAYERSTATS_RANK_PENALITY) return;

    if (rankingcounter.enable_accuracy_percent) {
        value = playerstats_get_last_accuracy(playerstats);
        format = RANKINGCOUNTER_FORMAT_PERCENT;
    } else {
        value = playerstats_get_last_difference(playerstats);
        format = RANKINGCOUNTER_FORMAT_TIME;
    }
	
	if (Number.isNaN(value)) return;

    rankingcounter.show_accuracy = 1;
    textsprite_animation_restart(rankingcounter.textsprite);
    textsprite_set_color_rgba8(rankingcounter.textsprite, color);

    if (rank == PLAYERSTATS_RANK_MISS)
        textsprite_set_text_intern(rankingcounter.textsprite, 1, RANKINGCOUNTER_TEXT_MISS);
    else
        textsprite_set_text_formated(rankingcounter.textsprite, format, value);
}

function rankingcounter_reset(rankingcounter) {
    rankingcounter.show_accuracy = 0;
    rankingcounter.last_iterations = 0;

    drawable_set_antialiasing(rankingcounter.drawable_accuracy, PVR_FLAG_DEFAULT);
    drawable_set_antialiasing(rankingcounter.drawable_rank, PVR_FLAG_DEFAULT);

    rankingcounter_set_offsetcolor_to_default(rankingcounter);

    pvrctx_helper_clear_modifier(drawable_get_modifier(rankingcounter.drawable_accuracy));
    pvrctx_helper_clear_modifier(drawable_get_modifier(rankingcounter.drawable_rank));

    // hide all visible ranking items
    for (let i = 0; i < RANKINGCOUNTER_RANKING_BUFFER_SIZE; i++)
        rankingcounter.ranking_items[i].id = -1;
}

function rankingcounter_hide_accuracy(rankingcounter, hide) {
    rankingcounter.enable_accuracy = !!hide;
}

function rankingcounter_use_percent_instead(rankingcounter, use_accuracy_percenter) {
    rankingcounter.enable_accuracy_percent = !!use_accuracy_percenter;
}

function rankingcounter_set_default_ranking_animation(rankingcounter, animlist) {
    if (!animlist) return;
    let animlist_item = animlist_get_animation(animlist, RANKINGCOUNTER_UI_RANKING_ANIM);

    if (!animlist_item) return;
    let animsprite = animsprite_init(animlist_item);

    rankingcounter_set_default_ranking_animation2(rankingcounter, animsprite);
    if (animsprite) animsprite_destroy(animsprite);
}

function rankingcounter_set_default_ranking_animation2(rankingcounter, animsprite) {
    for (let i = 0; i < RANKINGCOUNTER_RANKING_BUFFER_SIZE; i++) {
        if (rankingcounter.ranking_items[i].animsprite)
            animsprite_destroy(rankingcounter.ranking_items[i].animsprite);
        rankingcounter.ranking_items[i].animsprite = animsprite ? animsprite_clone(animsprite) : null;
    }
}


function rankingcounter_set_default_ranking_text_animation(rankingcounter, animlist) {
    if (!animlist) return;
    let animlist_item = animlist_get_animation(animlist, RANKINGCOUNTER_UI_RANKING_TEXT_ANIM);

    if (!animlist_item) return;
    let animsprite = animsprite_init(animlist_item);

    rankingcounter_set_default_ranking_text_animation2(rankingcounter, animsprite);
    if (animsprite) animsprite_destroy(animsprite);
}

function rankingcounter_set_default_ranking_text_animation2(rankingcounter, animsprite) {
    let old_animation = textsprite_animation_set(
        rankingcounter.textsprite, animsprite ? animsprite_clone(animsprite) : null
    );
    if (old_animation) animsprite_destroy(old_animation);
}



function rankingcounter_set_alpha(rankingcounter, alpha) {
    drawable_set_alpha(rankingcounter.drawable_accuracy, alpha);
    drawable_set_alpha(rankingcounter.drawable_rank, alpha);
}

function rankingcounter_set_offsetcolor(rankingcounter, r, g, b, a) {
    drawable_set_offsetcolor(rankingcounter.drawable_accuracy, r, g, b, a);
    drawable_set_offsetcolor(rankingcounter.drawable_rank, r, g, b, a);
}

function rankingcounter_set_offsetcolor_to_default(rankingcounter) {
    drawable_set_offsetcolor_to_default(rankingcounter.drawable_accuracy);
    drawable_set_offsetcolor_to_default(rankingcounter.drawable_rank);
}



function rankingcounter_animation_set(rankingcounter, animsprite) {
    if (rankingcounter.drawable_animation) animsprite_destroy(rankingcounter.drawable_animation);
    rankingcounter.drawable_animation = animsprite ? animsprite_clone(animsprite) : null;
}

function rankingcounter_animation_restart(rankingcounter) {
    for (let i = 0; i < RANKINGCOUNTER_RANKING_BUFFER_SIZE; i++) {
        if (rankingcounter.ranking_items[i].animsprite)
            animsprite_restart(rankingcounter.ranking_items[i].animsprite);
        statesprite_animation_restart(rankingcounter.ranking_items[i].statesprite);
    }

    textsprite_animation_restart(rankingcounter.textsprite);

    if (rankingcounter.drawable_animation)
        animsprite_restart(rankingcounter.drawable_animation);
}

function rankingcounter_animation_end(rankingcounter) {
    for (let i = 0; i < RANKINGCOUNTER_RANKING_BUFFER_SIZE; i++)
        rankingcounter.ranking_items[i].id = -1;

    textsprite_animation_end(rankingcounter.textsprite);

    if (rankingcounter.drawable_animation) {
        animsprite_force_end(rankingcounter.drawable_animation);
        animsprite_update_drawable(rankingcounter.drawable_animation, rankingcounter.drawable_accuracy, 0);
        animsprite_update_drawable(rankingcounter.drawable_animation, rankingcounter.drawable_rank, 1);
    }
}


function rankingcounter_animate1(rankingcounter, elapsed) {
    let total = RANKINGCOUNTER_RANKING_BUFFER_SIZE + 1;

    for (let i = 0; i < RANKINGCOUNTER_RANKING_BUFFER_SIZE; i++) {
        let item = rankingcounter.ranking_items[i];
        if (item.id < 0) {
            total--;
            continue;
        }

        let completed = statesprite_animate(item.statesprite, elapsed);

        if (item.animsprite) {
            completed = animsprite_animate(item.animsprite, elapsed);
            animsprite_update_statesprite(item.animsprite, item.statesprite, 1);
        }

        if (completed) {
            item.id = -1;
            total--;
        }
    }

    if (rankingcounter.drawable_animation) {
        if (animsprite_animate(rankingcounter.drawable_animation, elapsed)) total--;
        animsprite_update_drawable(rankingcounter.drawable_animation, rankingcounter.drawable_accuracy, 0);
        animsprite_update_drawable(rankingcounter.drawable_animation, rankingcounter.drawable_rank, 1);
    }

    return total;
}

function rankingcounter_animate2(rankingcounter, elapsed) {
    if (rankingcounter.enable_accuracy && rankingcounter.show_accuracy) {
        let completed = textsprite_animate(rankingcounter.textsprite, elapsed);
        if (completed) rankingcounter.show_accuracy = 0;
        return completed;
    }

    return 1;
}

function rankingcounter_draw1(rankingcounter, pvrctx) {
    pvr_context_save(pvrctx);
    drawable_helper_apply_in_context(rankingcounter.drawable_rank, pvrctx);

    for (let i = 0; i < RANKINGCOUNTER_RANKING_BUFFER_SIZE; i++) {
        if (rankingcounter.ranking_items[i].id < 0) continue;
        statesprite_draw(rankingcounter.ranking_items[i].statesprite, pvrctx);
    }

    pvr_context_restore(pvrctx);
}

function rankingcounter_draw2(rankingcounter, pvrctx) {
    if (!rankingcounter.enable_accuracy || !rankingcounter.show_accuracy) return;
    pvr_context_save(pvrctx);
    drawable_helper_apply_in_context(rankingcounter.drawable_accuracy, pvrctx);

    textsprite_draw(rankingcounter.textsprite, pvrctx);
    pvr_context_restore(pvrctx);
}



function rankingcounter_internal_add_state(statesprite, max_height, modelholder, prefix, state_name) {
    let animation_name = string_concat_for_state_name(2, prefix, state_name);

    let texture = modelholder_get_texture(modelholder, 0);
    let vertex_color = modelholder_get_vertex_color(modelholder);
    let animsprite = modelholder_create_animsprite(modelholder, animation_name, 0, 0);
    let atlas_entry = modelholder_get_atlas_entry2(modelholder, animation_name, 0);

    let state = statesprite_state_add2(
        statesprite, texture, animsprite, atlas_entry, vertex_color, animation_name
    );
    animation_name = undefined;

    if (!state) {
        if (animsprite) animsprite_destroy(animsprite);
        return 0;
    }

    let temp = [0, 0];
    imgutils_get_statesprite_original_size(state, temp);
    imgutils_calc_size(temp[0], temp[1], -1, max_height, temp);

    state.draw_width = temp[0];
    state.draw_height = temp[1];

    // center the sprite on the draw location
    state.offset_x = state.draw_width / -2;
    state.offset_y = state.draw_height / -2;

    return 1;
}

function rankingcounter_internal_pick_item(array, new_id) {
    for (let i = 0; i < RANKINGCOUNTER_RANKING_BUFFER_SIZE; i++) {
        if (array[i].id < 0) {
            array[i].id = new_id;
            return array[i];
        }
    }

    let oldest_id = MATH2D_MAX_INT32;
    let oldest_item = null;

    for (let i = 0; i < RANKINGCOUNTER_RANKING_BUFFER_SIZE; i++) {
        if (array[i].id < oldest_id) {
            oldest_id = array[i].id;
            oldest_item = array[i];
        }
    }

    console.assert(oldest_item != null);
    oldest_item.id = new_id;
    return oldest_item;
}

function rankingcounter_internal_sort(item1, item2) {
    return item1.id - item2.id;
}

