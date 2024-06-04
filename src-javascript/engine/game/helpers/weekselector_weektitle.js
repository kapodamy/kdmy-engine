"use strict";

const WEEKSELECTOR_WEEKTITLE_ALT_ALPHA = 0.5;

function weekselector_weektitle_init(layout) {
    let weektitle = {
        placeholder1: layout_get_placeholder(layout, "ui_week_title_no_warns"),
        placeholder2: layout_get_placeholder(layout, "ui_week_title_warns"),
        placeholder3: layout_get_placeholder(layout, "ui_difficult_selected"),
        sprite: sprite_init(null),
        drawable: null
    };

    weektitle.drawable = drawable_init(-1.0, weektitle, weekselector_weektitle_draw, null);
    sprite_set_alpha(weektitle.sprite, WEEKSELECTOR_WEEKTITLE_ALT_ALPHA);

    return weektitle;
}

function weekselector_weektitle_destroy(weektitle) {
    sprite_destroy(weektitle.sprite);
    drawable_destroy(weektitle.drawable);
    weektitle = undefined;
}

function weekselector_weektitle_draw(weektitle, pvrctx) {
    if (!sprite_is_textured(weektitle.sprite)) return;
    pvr_context_save(pvrctx);
    sprite_draw(weektitle.sprite, pvrctx);
    pvr_context_restore(pvrctx);
}

function weekselector_weektitle_move(weektitle, weeklist, use_warns_placeholder) {
    const draw_size = [0.0, 0.0];
    const draw_location = [0.0, 0.0];
    const atlas_entry = {};

    if (!weektitle.placeholder1 && !weektitle.placeholder2) return;

    let title_sprite = weekselector_weeklist_peek_title_sprite(weeklist);
    sprite_fill_atlas_entry(title_sprite, atlas_entry);
    sprite_get_source_size(title_sprite, draw_size);

    let old_placeholder = use_warns_placeholder ? weektitle.placeholder1 : weektitle.placeholder2;
    let placeholder = use_warns_placeholder ? weektitle.placeholder2 : weektitle.placeholder1;

    if (old_placeholder) old_placeholder.vertex = null;
    if (!placeholder) return;

    placeholder.vertex = weektitle.drawable;
    drawable_helper_update_from_placeholder(weektitle.drawable, placeholder);

    imgutils_calc_rectangle(
        placeholder.x, placeholder.y, placeholder.width, placeholder.height,
        draw_size[0], draw_size[1], ALIGN_CENTER, ALIGN_CENTER,
        draw_size, draw_location
    );

    sprite_set_texture(weektitle.sprite, sprite_get_texture(title_sprite), false);
    atlas_apply_from_entry(weektitle.sprite, atlas_entry, false);
    sprite_set_draw_size(weektitle.sprite, draw_size[0], draw_size[1]);
    sprite_set_draw_location(weektitle.sprite, draw_location[0], draw_location[1]);
}

function weekselector_weektitle_move_difficult(weektitle, weekdifficult) {
    const draw_size = [0.0, 0.0];
    const draw_location = [0.0, 0.0];

    if (weektitle.placeholder1) weektitle.placeholder1.vertex = null;
    if (weektitle.placeholder2) weektitle.placeholder2.vertex = null;
    if (weektitle.placeholder3) weektitle.placeholder3.vertex = weektitle.drawable;
    drawable_helper_update_from_placeholder(weektitle.drawable, weektitle.placeholder3);

    let difficult_statesprite = weekselector_difficult_peek_statesprite(weekdifficult);
    let statesprite_state = statesprite_state_get(difficult_statesprite);
    if (!statesprite_state) {
        sprite_set_texture(weektitle.sprite, null, false);
        return;
    }
    imgutils_get_statesprite_original_size(statesprite_state, draw_size);

    imgutils_calc_rectangle(
        weektitle.placeholder3.x, weektitle.placeholder3.y,
        weektitle.placeholder3.width, weektitle.placeholder3.height,
        draw_size[0], draw_size[1],
        weektitle.placeholder3.align_horizontal, weektitle.placeholder3.align_vertical,
        draw_size, draw_location
    );

    sprite_set_texture(weektitle.sprite, statesprite_state.texture, false);
    atlas_apply_from_entry(weektitle.sprite, statesprite_state.frame_info, false);
    sprite_set_draw_size(weektitle.sprite, draw_size[0], draw_size[1]);
    sprite_set_draw_location(weektitle.sprite, draw_location[0], draw_location[1]);
    sprite_set_alpha(weektitle.sprite, 1.0);
}

