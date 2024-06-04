"use strict";

const WEEKSELECTOR_WEEKLIST_TITLE_ANIM_NAME = "weektitle";
const WEEKSELECTOR_WEEKLIST_VISIBLE_SIZE = 4;
const WEEKSELECTOR_WEEKLIST_TWEEN_DURATION = 120.0;

async function weekselector_weeklist_init(animlist, modelholder, layout, texturepool) {
    let placeholder = layout_get_placeholder(layout, "ui_weeklist");
    if (!placeholder) throw new Error("missing placeholder 'ui_weeklist'");

    let placeholder_host = layout_get_placeholder(layout, "ui_character_opponent");
    if (!placeholder_host) throw new Error("missing placeholder 'ui_weeklist'");

    let anim_selected = animsprite_init_from_animlist(animlist, "week_title_selected");


    let weeklist = {
        list_visible: new Array(WEEKSELECTOR_WEEKLIST_VISIBLE_SIZE),

        index: 0,

        x: placeholder.x,
        y: placeholder.y,
        width: placeholder.width,
        row_height: placeholder.height / 4.0,

        progress: WEEKSELECTOR_WEEKLIST_TWEEN_DURATION,
        do_reverse: false,

        drawable_host: null,
        drawable_list: null,

        host_statesprite: statesprite_init_from_texture(null),
        host_loading_count: 0,
        host_placeholder: placeholder_host,
        host_load_id: 0,

        texturepool: texturepool,
        anim_selected: anim_selected,
        week_choosen: false,
        hey_playing: false,

        beatwatcher: {}
    };

    beatwatcher_reset(weeklist.beatwatcher, true, 100.0);

    for (let i = 0; i < WEEKSELECTOR_WEEKLIST_VISIBLE_SIZE; i++) {
        weeklist.list_visible[i] = {
            icon_locked: modelholder_create_sprite(modelholder, WEEKSELECTOR_LOCKED, true),
            sprite_title: sprite_init(null),
            tweenlerp_locked: weekselector_weeklist_internal_create_tween(),
            tweenlerp_title: weekselector_weeklist_internal_create_tween(),
            is_locked: true
        };
        sprite_matrix_scale_size(weeklist.list_visible[i].icon_locked, true);
        sprite_matrix_scale_size(weeklist.list_visible[i].sprite_title, true);
    }

    weekselector_weeklist_internal_prepare_locked_tweens(weeklist);

    weeklist.drawable_list = drawable_init(
        placeholder.z, weeklist, weekselector_weeklist_draw, weekselector_weeklist_animate
    );
    placeholder.vertex = weeklist.drawable_list;

    weeklist.drawable_host = drawable_init(
        placeholder_host.z, weeklist, weekselector_weeklist_internal_host_draw, null
    );
    placeholder_host.vertex = weeklist.drawable_host;
    statesprite_set_draw_location(weeklist.host_statesprite, placeholder_host.x, placeholder_host.y);

    let last_played_week = funkinsave_get_last_played_week();
    let index = 0;
    for (let i = 0; i < weeks_array.size; i++) {
        if (weeks_array.array[i].name == last_played_week) {
            index = i;
            break;
        }
    }

    await weekselector_weeklist_select(weeklist, index);

    // obligatory step
    weeklist.progress = WEEKSELECTOR_WEEKLIST_TWEEN_DURATION;

    let percent = index > 0 ? 0.0 : 1.0;
    for (let i = 0; i < WEEKSELECTOR_WEEKLIST_VISIBLE_SIZE; i++) {
        let visible_item = weeklist.list_visible[i];
        tweenlerp_animate_percent(visible_item.tweenlerp_locked, percent);
        tweenlerp_animate_percent(visible_item.tweenlerp_title, percent);

        tweenlerp_vertex_set_properties(
            visible_item.tweenlerp_locked, visible_item.icon_locked, sprite_set_property
        );
        if (sprite_is_textured(visible_item.sprite_title)) {
            tweenlerp_vertex_set_properties(
                visible_item.tweenlerp_title, visible_item.sprite_title, sprite_set_property
            );
        }
    }

    return weeklist;
}

function weekselector_weeklist_destroy(weeklist) {

    weeklist.host_load_id = -1;

    /*
    // C/C# only
    thd_pass();

    while (weeklist.host_loading_count > 0) {
        // wait until all async operations are done
        thd_pass();
    }
    */

    for (let i = 0; i < WEEKSELECTOR_WEEKLIST_VISIBLE_SIZE; i++) {
        sprite_destroy_full(weeklist.list_visible[i].icon_locked);
        sprite_destroy_full(weeklist.list_visible[i].sprite_title);
        tweenlerp_destroy(weeklist.list_visible[i].tweenlerp_title);
        tweenlerp_destroy(weeklist.list_visible[i].tweenlerp_locked);
    }

    statesprite_destroy(weeklist.host_statesprite);
    if (weeklist.anim_selected) animsprite_destroy(weeklist.anim_selected);
    drawable_destroy(weeklist.drawable_host);
    drawable_destroy(weeklist.drawable_list);
    weeklist = undefined;
}


function weekselector_weeklist_animate(weeklist, elapsed) {
    let since_beat = elapsed;

    if (!weeklist.hey_playing && beatwatcher_poll(weeklist.beatwatcher)) {
        since_beat = weeklist.beatwatcher.since;
        if (statesprite_animation_completed(weeklist.host_statesprite)) {
            statesprite_animation_restart(weeklist.host_statesprite);
        }
    }

    if (weeklist.host_loading_count < 1) statesprite_animate(weeklist.host_statesprite, since_beat);

    for (let i = 0; i < WEEKSELECTOR_WEEKLIST_VISIBLE_SIZE; i++) {
        let visible_item = weeklist.list_visible[i];

        sprite_animate(visible_item.icon_locked, elapsed);

        if (sprite_is_textured(visible_item.sprite_title))
            sprite_animate(visible_item.sprite_title, elapsed);
    }

    if (weeklist.week_choosen && weeklist.anim_selected) {
        let visible_item = weeklist.list_visible[weeklist.do_reverse ? 2 : 1];
        animsprite_update_sprite(weeklist.anim_selected, visible_item.sprite_title, true);
        animsprite_animate(weeklist.anim_selected, elapsed);
    }

    if (weeklist.progress >= WEEKSELECTOR_WEEKLIST_TWEEN_DURATION) return 1;

    let percent = Math.min(1.0, weeklist.progress / WEEKSELECTOR_WEEKLIST_TWEEN_DURATION);
    weeklist.progress += elapsed;
    if (weeklist.do_reverse) percent = 1.0 - percent;

    for (let i = 0; i < WEEKSELECTOR_WEEKLIST_VISIBLE_SIZE; i++) {
        let visible_item = weeklist.list_visible[i];

        tweenlerp_animate_percent(visible_item.tweenlerp_locked, percent);
        tweenlerp_animate_percent(visible_item.tweenlerp_title, percent);

        tweenlerp_vertex_set_properties(
            visible_item.tweenlerp_locked, visible_item.icon_locked, sprite_set_property
        );
        if (sprite_is_textured(visible_item.sprite_title)) {
            tweenlerp_vertex_set_properties(
                visible_item.tweenlerp_title, visible_item.sprite_title, sprite_set_property
            );
        }
    }

    return 0;
}

function weekselector_weeklist_draw(weeklist, pvrctx) {
    pvr_context_save(pvrctx);

    sh4matrix_translate(pvrctx.current_matrix, weeklist.x, weeklist.y);
    pvr_context_flush(pvrctx);

    if (weeklist.week_choosen) {
        // draw only the choosen week
        let visible_item = weeklist.list_visible[weeklist.do_reverse ? 2 : 1];
        if (sprite_is_textured(visible_item.sprite_title)) {
            sprite_draw(visible_item.sprite_title, pvrctx);
        }
    } else {
        for (let i = 0; i < WEEKSELECTOR_WEEKLIST_VISIBLE_SIZE; i++) {
            let visible_item = weeklist.list_visible[i];
            if (!sprite_is_textured(visible_item.sprite_title)) continue;

            sprite_draw(visible_item.sprite_title, pvrctx);
            if (visible_item.is_locked) sprite_draw(visible_item.icon_locked, pvrctx);
        }
    }

    pvr_context_restore(pvrctx);
}


function weekselector_weeklist_toggle_choosen(weeklist) {
    weeklist.week_choosen = true;
    if (weeklist.host_loading_count > 0) return;
    if (statesprite_state_toggle(weeklist.host_statesprite, WEEKSELECTOR_MDLSELECT_HEY)) {
        weeklist.hey_playing = true;
    }
}

function weekselector_weeklist_get_selected(weeklist) {
    if (weeklist.index < 0 || weeklist.index >= weeks_array.size) return null;
    return weeks_array.array[weeklist.index];
}

function weekselector_weeklist_peek_title_sprite(weeklist) {
    return weeklist.list_visible[weeklist.do_reverse ? 2 : 1].sprite_title;
}

/*async */function weekselector_weeklist_scroll(weeklist, offset) {
    return weekselector_weeklist_select(weeklist, weeklist.index + offset);
}

async function weekselector_weeklist_select(weeklist, index) {
    if (index < 0 || index >= weeks_array.size) return false;

    weeklist.do_reverse = weeklist.index < index;
    weeklist.index = index;

    // center the selected week in the visible list
    index -= weeklist.do_reverse ? 2 : 1;

    for (let i = 0; i < WEEKSELECTOR_WEEKLIST_VISIBLE_SIZE; i++, index++) {
        if (index < 0 || index >= weeks_array.size) {
            weeklist.list_visible[i].is_locked = true;
            sprite_destroy_texture(weeklist.list_visible[i].sprite_title);
            continue;
        }

        let weekinfo = weeks_array.array[index];
        weeklist.list_visible[i].is_locked = !funkinsave_contains_unlock_directive(
            weekinfo.unlock_directive
        );

        let texture = null;
        let animsprite = null;

        if (weekinfo.week_title_model) {
            if (modelholder_utils_is_known_extension(weekinfo.week_title_model)) {
                let modelholder = await modelholder_init(weekinfo.week_title_model);
                if (modelholder) {
                    texture = modelholder_get_texture(modelholder, 1);
                    animsprite = modelholder_create_animsprite(
                        modelholder,
                        weekinfo.week_title_model_animation_name ?? WEEKSELECTOR_WEEKLIST_TITLE_ANIM_NAME,
                        true,
                        false
                    );
                    modelholder_destroy(modelholder);
                }
            } else {
                texture = await texture_init(weekinfo.week_title_model);
            }
        } else {
            let title_src = weekenumerator_get_title_texture(weekinfo);
            texture = await texture_init(title_src);
            title_src = undefined;
        }

        // add title texture to the texturepool and dispose the previous one
        texturepool_add(weeklist.texturepool, texture);

        sprite_destroy_texture(weeklist.list_visible[i].sprite_title);
        sprite_destroy_all_animations(weeklist.list_visible[i].sprite_title);

        sprite_set_texture(weeklist.list_visible[i].sprite_title, texture, true);

        if (animsprite) {
            sprite_external_animation_set(weeklist.list_visible[i].sprite_title, animsprite);
            animsprite_update_sprite(animsprite, weeklist.list_visible[i].sprite_title, true);
        }

        sprite_set_draw_size_from_source_size(weeklist.list_visible[i].sprite_title);
    }

    weeklist.host_load_id++;
    weeklist.host_loading_count++;
    main_thd_helper_spawn(true, weekselector_weeklist_internal_load_host_async, weeklist);

    weeklist.progress = 0.0;
    weekselector_weeklist_internal_prepare_title_tweens(weeklist);

    return true;
}

function weekselector_weeklist_get_selected_index(weeklist) {
    return weeklist.index;
}

function weekselector_weeklist_set_beats(weeklist, beats_per_minute) {
    beatwatcher_change_bpm(weeklist.beatwatcher, beats_per_minute);
}



function weekselector_weeklist_internal_create_tween() {
    const duration = WEEKSELECTOR_WEEKLIST_TWEEN_DURATION;
    let tweenlerp = tweenlerp_init();
    tweenlerp_add_linear(tweenlerp, SPRITE_PROP_SCALE_X, NaN, NaN, duration);
    tweenlerp_add_linear(tweenlerp, SPRITE_PROP_SCALE_Y, NaN, NaN, duration);
    tweenlerp_add_linear(tweenlerp, SPRITE_PROP_TRANSLATE_X, NaN, NaN, duration);
    tweenlerp_add_linear(tweenlerp, SPRITE_PROP_TRANSLATE_Y, NaN, NaN, duration);
    tweenlerp_add_linear(tweenlerp, SPRITE_PROP_ALPHA, NaN, NaN, duration);
    return tweenlerp;
}

function weekselector_weeklist_internal_prepare_locked_tweens(weeklist) {
    const row1 = [0.0, 0.0, 0.0, 0.0];
    const row2 = [0.0, 0.0, 0.0, 0.0];

    let tweenlerp;
    let row_height = weeklist.row_height;
    let x1, x2, y1, y2;

    let height3 = row_height * 3.0;
    let height4 = row_height * 4.0;

    // all locked icons have the same draw size
    weekselector_weeklist_internal_calc_row_sizes(
        weeklist, weeklist.list_visible[0].icon_locked, row1, row2
    );

    // void1 -> row1
    x1 = weeklist.width / 2.0;
    x2 = (weeklist.width - row1[2]) / 2.0;
    y1 = -row_height;
    y2 = (row_height - row1[3]) / 2.0;
    tweenlerp = weeklist.list_visible[0].tweenlerp_locked;
    tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_SCALE_X, 0.0, row1[0]);
    tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_SCALE_Y, 0.0, row1[1]);
    tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_TRANSLATE_X, x1, x2);
    tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_TRANSLATE_Y, y1, y2);
    tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_ALPHA, 0.0, 0.7);

    // row1 -> row2
    x1 = (weeklist.width - row1[2]) / 2.0;
    x2 = (weeklist.width - row2[2]) / 2.0;
    y1 = (row_height - row1[3]) / 2.0;
    y2 = ((weeklist.row_height * 2.0) - row2[3]) / 2.0;
    tweenlerp = weeklist.list_visible[1].tweenlerp_locked;
    tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_SCALE_X, row1[0], row2[0]);
    tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_SCALE_Y, row1[1], row2[1]);
    tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_TRANSLATE_X, x1, x2);
    tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_TRANSLATE_Y, y1, y2 + row_height);
    tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_ALPHA, 0.7, 1.0);

    // row2 -> row3
    x1 = (weeklist.width - row2[2]) / 2.0;
    x2 = (weeklist.width - row1[2]) / 2.0;
    y1 = ((weeklist.row_height * 2) - row2[3]) / 2.0;
    y2 = (row_height - row1[3]) / 2.0;
    tweenlerp = weeklist.list_visible[2].tweenlerp_locked;
    tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_SCALE_X, row2[0], row1[0]);
    tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_SCALE_Y, row2[1], row1[1]);
    tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_TRANSLATE_X, x1, x2);
    tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_TRANSLATE_Y, y1 + row_height, y2 + height3);
    tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_ALPHA, 1.0, 0.7);

    // row3 -> void2
    x1 = (weeklist.width - row1[2]) / 2.0;
    x2 = weeklist.width / 2.0;
    y1 = (row_height - row1[3]) / 2.0;
    y2 = row_height;
    tweenlerp = weeklist.list_visible[3].tweenlerp_locked;
    tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_SCALE_X, row1[0], 0.0);
    tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_SCALE_Y, row1[1], 0.0);
    tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_TRANSLATE_X, x1, x2);
    tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_TRANSLATE_Y, y1 + height3, y2 + height4);
    tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_ALPHA, 0.7, 0.0);
}

function weekselector_weeklist_internal_prepare_title_tweens(weeklist) {
    const row1 = [0.0, 0.0, 0.0, 0.0];
    const row2 = [0.0, 0.0, 0.0, 0.0];

    let tweenlerp;
    let height1 = weeklist.row_height * 1.0;
    let height3 = weeklist.row_height * 3.0;
    let height4 = weeklist.row_height * 4.0;

    // void1 -> row1
    if (weeklist.list_visible[0].sprite_title) {
        tweenlerp = weeklist.list_visible[0].tweenlerp_title;
        weekselector_weeklist_internal_calc_row_sizes(
            weeklist, weeklist.list_visible[0].sprite_title, row1, row2
        );
        let x1 = weeklist.width / 2.0;
        let x2 = (weeklist.width - row1[2]) / 2.0;
        let y1 = -height1;
        let y2 = (height1 - row1[3]) / 2.0;
        tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_SCALE_X, 0.0, row1[0]);
        tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_SCALE_Y, 0.0, row1[1]);
        tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_TRANSLATE_X, x1, x2);
        tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_TRANSLATE_Y, y1, y2);
        tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_ALPHA, 0.0, 0.3);
    }

    // row1 -> row2
    if (weeklist.list_visible[1].sprite_title) {
        tweenlerp = weeklist.list_visible[1].tweenlerp_title;
        weekselector_weeklist_internal_calc_row_sizes(
            weeklist, weeklist.list_visible[1].sprite_title, row1, row2
        );
        let x1 = (weeklist.width - row1[2]) / 2.0;
        let x2 = (weeklist.width - row2[2]) / 2.0;
        let y1 = (height1 - row1[3]) / 2.0;
        let y2 = ((weeklist.row_height * 2.0) - row2[3]) / 2.0;
        tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_SCALE_X, row1[0], row2[0]);
        tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_SCALE_Y, row1[1], row2[1]);
        tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_TRANSLATE_X, x1, x2);
        tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_TRANSLATE_Y, y1, y2 + height1);
        tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_ALPHA, 0.3, 1.0);
    }

    // row2 -> row3
    if (weeklist.list_visible[2].sprite_title) {
        tweenlerp = weeklist.list_visible[2].tweenlerp_title;
        weekselector_weeklist_internal_calc_row_sizes(
            weeklist, weeklist.list_visible[2].sprite_title, row1, row2
        );
        let x1 = (weeklist.width - row2[2]) / 2.0;
        let x2 = (weeklist.width - row1[2]) / 2.0;
        let y1 = ((weeklist.row_height * 2.0) - row2[3]) / 2.0;
        let y2 = (height1 - row1[3]) / 2.0;
        tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_SCALE_X, row2[0], row1[0]);
        tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_SCALE_Y, row2[1], row1[1]);
        tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_TRANSLATE_X, x1, x2);
        tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_TRANSLATE_Y, y1 + height1, y2 + height3);
        tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_ALPHA, 1.0, 0.3);
    }

    // row3 -> void2
    if (weeklist.list_visible[3].sprite_title) {
        tweenlerp = weeklist.list_visible[3].tweenlerp_title;
        weekselector_weeklist_internal_calc_row_sizes(
            weeklist, weeklist.list_visible[3].sprite_title, row1, row2
        );
        let x1 = (weeklist.width - row1[2]) / 2.0;
        let x2 = weeklist.width / 2.0;
        let y1 = (height1 - row1[3]) / 2.0;
        let y2 = height4 + height1;
        tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_SCALE_X, row1[0], 0.0);
        tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_SCALE_Y, row1[1], 0.0);
        tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_TRANSLATE_X, x1, x2);
        tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_TRANSLATE_Y, y1 + height3, y2);
        tweenlerp_change_bounds_by_id(tweenlerp, SPRITE_PROP_ALPHA, 0.3, 0.0);
    }

}

function weekselector_weeklist_internal_calc_row_sizes(weeklist, vertex, row1, row2) {
    const draw_size = [0.0, 0.0];

    let height1 = weeklist.row_height * 1.0;
    let height2 = weeklist.row_height * 2.0;
    let width = weeklist.width;

    sprite_get_draw_size(vertex, draw_size);

    imgutils_calc_size(draw_size[0], draw_size[1], width, height1, row1);
    imgutils_calc_size(draw_size[0], draw_size[1], width, height2, row2);

    row1[0] /= draw_size[0];// scale x
    row1[1] /= draw_size[1];// scale y
    row1[2] = draw_size[0] * row1[0];// width 1
    row1[3] = draw_size[1] * row1[1];// height 1

    row2[0] /= draw_size[0];// scale x
    row2[1] /= draw_size[1];// scale y
    row2[2] = draw_size[0] * row2[0];// width 2
    row2[3] = draw_size[1] * row2[1];// height 2
}

function weekselector_weeklist_internal_host_draw(weeklist, pvrctx) {
    if (weeklist.host_loading_count > 0 || !statesprite_is_visible(weeklist.host_statesprite)) return;
    pvr_context_save(pvrctx);
    statesprite_draw(weeklist.host_statesprite, pvrctx);
    pvr_context_restore(pvrctx);
}

async function weekselector_weeklist_internal_load_host_async(weeklist) {
    let host_load_id = weeklist.host_load_id;
    let weekinfo = weeks_array.array[weeklist.index];
    let host_flip, host_beat, modelholder;
    let anim_name_hey = WEEKSELECTOR_MDLSELECT_HEY;
    let anim_name_idle = WEEKSELECTOR_MDLSELECT_IDLE;
    let hidden = false;
    let charactermanifest = null;

    if (weekinfo.host_hide_if_week_locked)
        hidden = weeklist.list_visible[weeklist.index].is_locked && weekinfo.host_hide_if_week_locked;

    if (weekinfo.week_host_character_manifest) {
        charactermanifest = await charactermanifest_init(weekinfo.week_host_character_manifest, false);
        host_flip = charactermanifest.left_facing;// face to the right
        host_beat = charactermanifest.week_selector_enable_beat;

        if (charactermanifest.week_selector_idle_anim_name != null) {
            anim_name_idle = charactermanifest.week_selector_idle_anim_name;
        }
        if (charactermanifest.week_selector_choosen_anim_name != null) {
            anim_name_hey = charactermanifest.week_selector_choosen_anim_name;
        }

        modelholder = await modelholder_init(charactermanifest.week_selector_model);
    } else if (weekinfo.week_host_model) {
        host_flip = weekinfo.host_flip_sprite;
        host_beat = weekinfo.host_enable_beat;

        if (weekinfo.week_host_model_idle_animation_name) {
            anim_name_idle = weekinfo.week_host_model_idle_animation_name;
        }
        if (weekinfo.week_host_model_choosen_animation_name) {
            anim_name_hey = weekinfo.week_host_model_choosen_animation_name;
        }

        modelholder = await modelholder_init(weekinfo.week_host_model);
    } else {
        host_flip = weekinfo.host_flip_sprite;
        host_beat = weekinfo.host_enable_beat;

        let week_host_model = weekenumerator_get_host_model(weekinfo);
        modelholder = await modelholder_init(week_host_model);
        week_host_model = undefined;
    }

    if (weeklist.host_load_id < 0) {
        // weeklist is begin disposed
        if (modelholder) modelholder_destroy(modelholder);
        if (charactermanifest) charactermanifest_destroy(charactermanifest);
        return;
    }

    if (!modelholder) {
        console.error("weekselector_weeklist_internal_load_host_async() modelholder_init failed");
        if (host_load_id == weeklist.host_load_id) {
            statesprite_state_remove(weeklist.host_statesprite, WEEKSELECTOR_MDLSELECT_HEY);
            statesprite_state_remove(weeklist.host_statesprite, WEEKSELECTOR_MDLSELECT_IDLE);
        }
        statesprite_set_texture(weeklist.host_statesprite, null, false);
        statesprite_set_visible(weeklist.host_statesprite, false);
        weeklist.host_loading_count--;

        if (charactermanifest) charactermanifest_destroy(charactermanifest);
        return null;
    }

    let texture = modelholder_get_texture(modelholder, false);
    if (texture) texturepool_add(weeklist.texturepool, texture);

    if (host_load_id == weeklist.host_load_id) {
        statesprite_state_remove(weeklist.host_statesprite, WEEKSELECTOR_MDLSELECT_HEY);
        statesprite_state_remove(weeklist.host_statesprite, WEEKSELECTOR_MDLSELECT_IDLE);

        weekselector_mdlselect_helper_import(
            weeklist.host_statesprite,
            modelholder,
            weeklist.host_placeholder,
            host_beat,
            anim_name_idle,
            WEEKSELECTOR_MDLSELECT_IDLE
        );
        weekselector_mdlselect_helper_import(
            weeklist.host_statesprite,
            modelholder,
            weeklist.host_placeholder,
            false,
            anim_name_hey,
            WEEKSELECTOR_MDLSELECT_HEY
        );

        if (weeklist.week_choosen)
            statesprite_state_toggle(weeklist.host_statesprite, WEEKSELECTOR_MDLSELECT_HEY);
        else
            statesprite_state_toggle(weeklist.host_statesprite, WEEKSELECTOR_MDLSELECT_IDLE);

        // set defaults
        statesprite_set_alpha(weeklist.host_statesprite, 1.0);
        statesprite_set_property(weeklist.host_statesprite, SPRITE_PROP_ALPHA2, 1.0);
        statesprite_set_offsetcolor(weeklist.host_statesprite, 1.0, 1.0, 1.0, -1.0);
        statesprite_matrix_reset(weeklist.host_statesprite);

        statesprite_set_visible(weeklist.host_statesprite, !hidden && texture);
        statesprite_flip_texture(weeklist.host_statesprite, host_flip, 0);
        statesprite_state_apply(weeklist.host_statesprite, null);
        statesprite_animate(weeklist.host_statesprite, beatwatcher_remaining_until_next(weeklist.beatwatcher));
        weeklist.weeklist.host_loading_count--;
    }

    modelholder_destroy(modelholder);
    if (charactermanifest) charactermanifest_destroy(charactermanifest);
}

