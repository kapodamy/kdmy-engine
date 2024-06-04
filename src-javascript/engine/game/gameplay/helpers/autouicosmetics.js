"use strict";

const AUTOUICOSMETICS_PLACEHOLDER_STREAK = {
    name: null,
    type: VERTEX_DRAWABLE,
    vertex: null,
    width: 0.0,
    height: 80.0
};
const AUTOUICOSMETICS_PLACEHOLDER_RANK = {
    name: null,
    type: VERTEX_DRAWABLE,
    vertex: null,
    width: -1.0,
    height: 110.0
};
const AUTOUICOSMETICS_PLACEHOLDER_ACCURACY = {
    name: null,
    type: VERTEX_DRAWABLE,
    vertex: null,
    width: -1.0,
    height: 32.0
};


function autouicosmetics_init() {
    let autouicosmetics = {
        drawable_self: null,
        drawable_streak: null,
        drawable_rank: null,
        drawable_accuracy: null
    };
    autouicosmetics.drawable_self = drawable_init(-1.0, autouicosmetics, autouicosmetics_draw, autouicosmetics_animate);
    return autouicosmetics;
}

function autouicosmetics_destroy(autouicosmetics) {
    drawable_destroy(autouicosmetics.drawable_self);

    autouicosmetics.drawable_streak = null;
    autouicosmetics.drawable_rank = null;
    autouicosmetics.drawable_accuracy = null;

    AUTOUICOSMETICS_PLACEHOLDER_STREAK.vertex = null;
    AUTOUICOSMETICS_PLACEHOLDER_RANK.vertex = null;
    AUTOUICOSMETICS_PLACEHOLDER_ACCURACY.vertex = null;

    autouicosmetics = undefined;
}


function autouicosmetics_prepare_placeholders(autouicosmetics, layout) {
    autouicosmetics.drawable_streak = null;
    autouicosmetics.drawable_rank = null;
    autouicosmetics.drawable_accuracy = null;

    const ui_autoplace = layout_get_placeholder(layout, "ui_autoplace_cosmetics");
    if (!ui_autoplace) {
        autouicosmetics.layout_group_id = -1;
        return false;
    }

    drawable_helper_update_from_placeholder(autouicosmetics.drawable_self, ui_autoplace);
    autouicosmetics.layout_group_id = ui_autoplace.group_id;

    //
    // prepare stubs
    //
    AUTOUICOSMETICS_PLACEHOLDER_STREAK.z = ui_autoplace.z + 0.1;
    AUTOUICOSMETICS_PLACEHOLDER_RANK.z = ui_autoplace.z + 0.2;
    AUTOUICOSMETICS_PLACEHOLDER_ACCURACY.z = ui_autoplace.z + 0.3;

    AUTOUICOSMETICS_PLACEHOLDER_STREAK.group_id = ui_autoplace.group_id;
    AUTOUICOSMETICS_PLACEHOLDER_RANK.group_id = ui_autoplace.group_id;
    AUTOUICOSMETICS_PLACEHOLDER_ACCURACY.group_id = ui_autoplace.group_id;

    AUTOUICOSMETICS_PLACEHOLDER_STREAK.x = ui_autoplace.x + ui_autoplace.width / 4.0;
    AUTOUICOSMETICS_PLACEHOLDER_STREAK.y = ui_autoplace.y + ui_autoplace.height / 2.0;
    AUTOUICOSMETICS_PLACEHOLDER_RANK.x = ui_autoplace.x;
    AUTOUICOSMETICS_PLACEHOLDER_RANK.y = ui_autoplace.y;
    AUTOUICOSMETICS_PLACEHOLDER_ACCURACY.x = ui_autoplace.x + ui_autoplace.width;
    AUTOUICOSMETICS_PLACEHOLDER_ACCURACY.y = ui_autoplace.y + ui_autoplace.height;

    return true;
}

function autouicosmetics_pick_drawables(autouicosmetics) {
    autouicosmetics.drawable_streak = AUTOUICOSMETICS_PLACEHOLDER_STREAK.vertex;
    AUTOUICOSMETICS_PLACEHOLDER_STREAK.vertex = null;

    autouicosmetics.drawable_rank = AUTOUICOSMETICS_PLACEHOLDER_RANK.vertex;
    AUTOUICOSMETICS_PLACEHOLDER_RANK.vertex = null;

    autouicosmetics.drawable_accuracy = AUTOUICOSMETICS_PLACEHOLDER_ACCURACY.vertex;
    AUTOUICOSMETICS_PLACEHOLDER_ACCURACY.vertex = null;
}

function autouicosmetics_draw(autouicosmetics, pvrctx) {
    if (autouicosmetics.drawable_streak) drawable_draw(autouicosmetics.drawable_streak, pvrctx);
    if (autouicosmetics.drawable_rank) drawable_draw(autouicosmetics.drawable_rank, pvrctx);
    if (autouicosmetics.drawable_accuracy) drawable_draw(autouicosmetics.drawable_accuracy, pvrctx);
}

function autouicosmetics_animate(autouicosmetics, elapsed) {
    let completed = 0;
    if (autouicosmetics.drawable_streak) completed += drawable_animate(autouicosmetics.drawable_streak, elapsed);
    if (autouicosmetics.drawable_rank) completed += drawable_animate(autouicosmetics.drawable_rank, elapsed);
    if (autouicosmetics.drawable_accuracy) completed += drawable_animate(autouicosmetics.drawable_accuracy, elapsed);
    return completed;
}

