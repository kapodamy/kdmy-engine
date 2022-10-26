"use strict";

const WEEKSELECTOR_HELPTEXT_TEXTCOLOR = 0xFFFFFF;// white
const WEEKSELECTOR_HELPTEXT_Z_INDEX = 15;// draw on top of anything
const WEEKSELECTOR_HELPTEXT_ROW_PADDING_RATIO = 0.5;

function weekselector_helptext_init(modelholder, layout, row, start, icon_name, string1, string2) {
    let font_size = layout_get_attached_value_as_float(layout, "help_text_font_size", 12);
    let font_name = layout_get_attached_value(layout, "help_text_font_name", LAYOUT_TYPE_STRING, null);

    let padding_x = layout_get_attached_value_as_float(layout, "help_text_padding_x", 0);
    let padding_y = layout_get_attached_value_as_float(layout, "help_text_padding_y", 0);

    let layout_font = layout_get_attached_font(layout, font_name);

    let size = [0, 0];
    layout_get_viewport_size(layout, size);

    let max_width = size[0] / 2.0;

    let helptext = {
        icon: null,
        string_1: string1,
        string_2: string2,
        textsprite: null,
        max_width: max_width - padding_x,
        icon_width: 0,
        icon_padding: font_size / 2.0,
        start: start,
        offset_x: 0,
        drawable: null
    };

    helptext.drawable = drawable_init(
        WEEKSELECTOR_HELPTEXT_Z_INDEX, helptext, weekselector_helptext_draw, null
    );

    if (!layout_font) return helptext;

    let row_padding_x = padding_x * WEEKSELECTOR_HELPTEXT_ROW_PADDING_RATIO;
    let draw_x = padding_x * (start ? 2 : 1);
    let draw_y = size[1] - ((font_size + row_padding_x) * row) - padding_y + row_padding_x;

    if (!start) draw_x += helptext.max_width;

    if (modelholder) {
        helptext.icon = sprite_init(modelholder_get_texture(modelholder, 1));

        let atlas_entry = modelholder_get_atlas_entry(modelholder, icon_name, false);
        if (atlas_entry) atlas_apply_from_entry(helptext.icon, atlas_entry, 1);

        sprite_set_visible(helptext.icon, 0);
        sprite_resize_draw_size(helptext.icon, -1, font_size, size);
        sprite_set_draw_location(helptext.icon, draw_x, draw_y);
        helptext.icon_width = size[0];
    }

    helptext.textsprite = textsprite_init2(layout_font, font_size, WEEKSELECTOR_HELPTEXT_TEXTCOLOR);
    textsprite_set_visible(helptext.textsprite, 0);

    if (start) {
        if (helptext.icon) draw_x += helptext.icon_width + helptext.icon_padding;
        textsprite_set_draw_location(helptext.textsprite, draw_x, draw_y);
    } else {
        textsprite_set_align_horizontal(helptext.textsprite, ALIGN_END);
        textsprite_set_max_draw_size(helptext.textsprite, helptext.max_width, -1);
        textsprite_set_draw_location(helptext.textsprite, draw_x, draw_y);
    }

    weekselector_helptext_use_alt(helptext, 0);
    return helptext;
}

function weekselector_helptext_destroy(helptext) {
    if (helptext.icon) sprite_destroy_full(helptext.icon);
    if (helptext.textsprite) textsprite_destroy(helptext.textsprite);
    drawable_destroy(helptext.drawable);
    helptext = undefined;
}

function weekselector_helptext_set_visible(helptext, visible) {
    drawable_set_visible(helptext.drawable, visible);
}

function weekselector_helptext_use_alt(helptext, use_alt) {
    // intern strings, they are static
    textsprite_set_text_intern(helptext.textsprite, 1, use_alt ? helptext.string_2 : helptext.string_1);
    if (!helptext.icon || helptext.start) return;
    let draw_size = [0, 0];
    textsprite_get_draw_size(helptext.textsprite, draw_size);
    helptext.offset_x = helptext.max_width - draw_size[0] - helptext.icon_width - helptext.icon_padding;
}

function weekselector_helptext_get_drawable(helptext) {
    return helptext.drawable;
}

function weekselector_helptext_draw(helptext, pvrctx) {
    pvr_context_save(pvrctx);
    textsprite_draw(helptext.textsprite, pvrctx);
    if (helptext.icon) {
        sh4matrix_translate_x(pvrctx.current_matrix, helptext.offset_x);
        pvr_context_flush(pvrctx);
        sprite_draw(helptext.icon, pvrctx);
    }
    pvr_context_restore(pvrctx);
}

