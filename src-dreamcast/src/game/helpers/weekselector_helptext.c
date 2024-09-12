#include "game/helpers/weekselector_helptext.h"

#include "drawable.h"
#include "game/weekselector.h"
#include "layout.h"
#include "malloc_utils.h"
#include "modelholder.h"
#include "sprite.h"
#include "textsprite.h"


struct WeekSelectorHelpText_s {
    Sprite icon;
    const char* string_1;
    const char* string_2;
    TextSprite textsprite;
    float max_width;
    float icon_width;
    float icon_padding;
    bool start;
    float offset_x;
    Drawable drawable;
};


static const uint WEEKSELECTOR_HELPTEXT_TEXTCOLOR = 0xFFFFFF; // white
static const float WEEKSELECTOR_HELPTEXT_Z_INDEX = 210.0f;     // draw on top of anything
static const float WEEKSELECTOR_HELPTEXT_ROW_PADDING_RATIO = 0.5f;


static void weekselector_helptext_draw(WeekSelectorHelpText helptext, PVRContext pvrctx);


WeekSelectorHelpText weekselector_helptext_init(ModelHolder modelholder, Layout layout, int32_t row, bool start, const char* icon_name, const char* string1, const char* string2) {
    float font_size = layout_get_attached_value_as_float(layout, "help_text_font_size", 12.0f);
    const char* font_name = layout_get_attached_value_string(layout, "help_text_font_name", NULL);

    float padding_x = layout_get_attached_value_as_float(layout, "help_text_padding_x", 0.0f);
    float padding_y = layout_get_attached_value_as_float(layout, "help_text_padding_y", 0.0f);

    FontHolder layout_font = layout_get_attached_font(layout, font_name);

    float size_width = 0.0f, size_height = 0.0f;
    layout_get_viewport_size(layout, &size_width, &size_height);

    float max_width = size_width / 2.0f;

    WeekSelectorHelpText helptext = malloc_chk(sizeof(struct WeekSelectorHelpText_s));
    malloc_assert(helptext, WeekSelectorHelpText);

    *helptext = (struct WeekSelectorHelpText_s){
        .icon = NULL,
        .string_1 = string1,
        .string_2 = string2,
        .textsprite = NULL,
        .max_width = max_width - padding_x,
        .icon_width = 0.0f,
        .icon_padding = font_size / 2.0f,
        .start = start,
        .offset_x = 0.0f,
        .drawable = NULL
    };

    helptext->drawable = drawable_init(
        WEEKSELECTOR_HELPTEXT_Z_INDEX, helptext, (DelegateDraw)weekselector_helptext_draw, NULL
    );

    if (!layout_font) return helptext;

    float row_padding_x = padding_x * WEEKSELECTOR_HELPTEXT_ROW_PADDING_RATIO;
    float draw_x = padding_x * (start ? 2.0f : 1.0f);
    float draw_y = size_height - ((font_size + row_padding_x) * row) - padding_y + row_padding_x;

    if (!start) draw_x += helptext->max_width;

    if (modelholder) {
        helptext->icon = sprite_init(modelholder_get_texture(modelholder, true));

        const AtlasEntry* atlas_entry = modelholder_get_atlas_entry(modelholder, icon_name);
        if (atlas_entry) atlas_apply_from_entry(helptext->icon, atlas_entry, true);

        sprite_set_visible(helptext->icon, false);
        sprite_resize_draw_size(helptext->icon, -1.0f, font_size, &size_width, &size_height);
        sprite_set_draw_location(helptext->icon, draw_x, draw_y);
        helptext->icon_width = size_width;
    }

    helptext->textsprite = textsprite_init2(layout_font, font_size, WEEKSELECTOR_HELPTEXT_TEXTCOLOR);
    textsprite_set_visible(helptext->textsprite, false);

    if (start) {
        if (helptext->icon) draw_x += helptext->icon_width + helptext->icon_padding;
        textsprite_set_draw_location(helptext->textsprite, draw_x, draw_y);
    } else {
        textsprite_set_align_horizontal(helptext->textsprite, ALIGN_END);
        textsprite_set_max_draw_size(helptext->textsprite, helptext->max_width, -1.0f);
        textsprite_set_draw_location(helptext->textsprite, draw_x, draw_y);
    }

    weekselector_helptext_use_alt(helptext, false);
    return helptext;
}

void weekselector_helptext_destroy(WeekSelectorHelpText* helptext_ptr) {
    WeekSelectorHelpText helptext = *helptext_ptr;
    if (!helptext) return;

    if (helptext->icon) sprite_destroy_full(&helptext->icon);
    if (helptext->textsprite) textsprite_destroy(&helptext->textsprite);
    drawable_destroy(&helptext->drawable);

    free_chk(helptext);
    *helptext_ptr = NULL;
}

void weekselector_helptext_set_visible(WeekSelectorHelpText helptext, bool visible) {
    drawable_set_visible(helptext->drawable, visible);
}

void weekselector_helptext_use_alt(WeekSelectorHelpText helptext, bool use_alt) {
    // intern strings, they are static
    textsprite_set_text_intern(helptext->textsprite, true, use_alt ? &helptext->string_2 : &helptext->string_1);
    if (!helptext->icon || helptext->start) return;
    float draw_width = 0.0f, draw_height = 0.0f;
    textsprite_get_draw_size(helptext->textsprite, &draw_width, &draw_height);
    helptext->offset_x = helptext->max_width - draw_width - helptext->icon_width - helptext->icon_padding;
}

Drawable weekselector_helptext_get_drawable(WeekSelectorHelpText helptext) {
    return helptext->drawable;
}

static void weekselector_helptext_draw(WeekSelectorHelpText helptext, PVRContext pvrctx) {
    pvr_context_save(pvrctx);
    textsprite_draw(helptext->textsprite, pvrctx);
    if (helptext->icon) {
        sh4matrix_translate_x(pvrctx->current_matrix, helptext->offset_x);
        pvr_context_flush(pvrctx);
        sprite_draw(helptext->icon, pvrctx);
    }
    pvr_context_restore(pvrctx);
}
