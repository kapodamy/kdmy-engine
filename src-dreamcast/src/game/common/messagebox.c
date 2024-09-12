#include "game/common/messagebox.h"

#include "atlas.h"
#include "drawable.h"
#include "externals/luascript.h"
#include "game/common/funkin.h"
#include "imgutils.h"
#include "layout.h"
#include "logger.h"
#include "malloc_utils.h"
#include "sprite.h"
#include "textsprite.h"


struct MessageBox_s {
    Sprite image;
    Layout layout;
    Atlas atlas;
    Drawable drawable;
    bool hide_animation;
    bool small;
    bool full_title;
};


static const char* MESSAGEBOX_LAYOUT = "/assets/common/image/messagebox.xml";


static void messagebox_internal_set_text(MessageBox messagebox, const char* name, const char* text);
static void messagebox_internal_set_icon(MessageBox messagebox, const char* name, const char* icon_name);


MessageBox messagebox_init() {
    Layout layout = layout_init(MESSAGEBOX_LAYOUT);
    if (!layout) {
        logger_error("can not load %s", MESSAGEBOX_LAYOUT);
        assert(layout);
    }

    Atlas atlas = NULL;
    const char* atlas_src = layout_get_attached_value_string(layout, "btn-atlas", NULL);
    if (atlas_src) atlas = atlas_init(atlas_src);

    MessageBox messagebox = malloc_chk(sizeof(struct MessageBox_s));
    malloc_assert(messagebox, MessageBox);

    *messagebox = (struct MessageBox_s){
        .image = sprite_init(NULL),
        .layout = layout,
        .atlas = atlas,
        .drawable = NULL,
        .hide_animation = false,
        .small = false,
        .full_title = false
    };

    messagebox->drawable = drawable_init(-1, messagebox, (DelegateDraw)messagebox_draw, (DelegateAnimate)messagebox_animate);
    drawable_set_visible(messagebox->drawable, false);
    layout_external_vertex_create_entries(messagebox->layout, 1);

    messagebox_set_button_single_icon(messagebox, "start");
    messagebox_set_buttons_icons(messagebox, "a", "b");
    messagebox_show_buttons_icons(messagebox, false);

    return messagebox;
}

void messagebox_destroy(MessageBox* messagebox_ptr) {
    MessageBox messagebox = *messagebox_ptr;
    if (!messagebox) return;

    luascript_drop_shared(messagebox);
    layout_destroy(&messagebox->layout);
    if (messagebox->atlas) atlas_destroy(&messagebox->atlas);
    drawable_destroy(&messagebox->drawable);
    sprite_destroy_full(&messagebox->image);

    free_chk(messagebox);
    *messagebox_ptr = NULL;
}


void messagebox_set_buttons_text(MessageBox messagebox, const char* left_text, const char* right_text) {
    layout_set_group_visibility(messagebox->layout, "btn1", false);
    layout_set_group_visibility(messagebox->layout, "btn2", true);

    layout_trigger_any(
        messagebox->layout, messagebox->small ? "rest-fntsize-small" : "rest-fntsize-normal"
    );

    messagebox_internal_set_text(messagebox, "btn1text", left_text);
    messagebox_internal_set_text(messagebox, "btn2text", right_text);

    layout_set_group_visibility(messagebox->layout, "btn1g", left_text != NULL);
    layout_set_group_visibility(messagebox->layout, "btn2g", right_text != NULL);
}

void messagebox_set_button_single(MessageBox messagebox, const char* center_text) {
    layout_set_group_visibility(messagebox->layout, "btn1", true);
    layout_set_group_visibility(messagebox->layout, "btn2", false);

    layout_trigger_any(
        messagebox->layout, messagebox->small ? "rest-fntsize-small" : "rest-fntsize-normal"
    );

    messagebox_internal_set_text(messagebox, "btntext", center_text);
}

void messagebox_set_buttons_icons(MessageBox messagebox, const char* left_icon_name, const char* right_icon_name) {
    messagebox_show_buttons_icons(messagebox, true);
    messagebox_internal_set_icon(messagebox, "btn2icn1", left_icon_name);
    messagebox_internal_set_icon(messagebox, "btn2icn2", right_icon_name);
    layout_trigger_any(messagebox->layout, "btn-resize");
}

void messagebox_set_button_single_icon(MessageBox messagebox, const char* center_icon_name) {
    messagebox_show_buttons_icons(messagebox, true);
    messagebox_internal_set_icon(messagebox, "btn1icn", center_icon_name);
    layout_trigger_any(messagebox->layout, "btn-resize");
}

void messagebox_set_title(MessageBox messagebox, const char* text) {
    TextSprite textsprite = layout_get_textsprite(messagebox->layout, "title");
    if (textsprite) textsprite_set_text_intern(textsprite, false, &text);
}

void messagebox_set_image_background_color(MessageBox messagebox, uint8_t color_rgb8) {
    Sprite sprite = layout_get_sprite(messagebox->layout, "imgbg");
    if (sprite) sprite_set_vertex_color_rgb8(sprite, color_rgb8);
}

void messagebox_set_image_background_color_default(MessageBox messagebox) {
    layout_trigger_any(messagebox->layout, "def-imgcolor");
}

void messagebox_set_message(MessageBox messagebox, const char* text) {
    TextSprite textsprite = layout_get_textsprite(messagebox->layout, "message");
    if (textsprite) textsprite_set_text_intern(textsprite, false, &text);
}

void messagebox_set_message_formated(MessageBox messagebox, const char* format, ...) {
    TextSprite textsprite = layout_get_textsprite(messagebox->layout, "message");
    if (textsprite) {
        va_list values;
        va_start(values, format);
        textsprite_set_text_formated2(textsprite, format, values);
        va_end(values);
    }
}

void messagebox_hide_image_background(MessageBox messagebox, bool hide) {
    layout_trigger_any(messagebox->layout, hide ? "no-imgbg" : "with-imgbg");
}

void messagebox_hide_image(MessageBox messagebox, bool hide) {
    layout_trigger_any(messagebox->layout, hide ? "no-img" : "with-img");
    layout_set_group_visibility(messagebox->layout, "imgg", !hide);
}

void messagebox_hide_buttons(MessageBox messagebox) {
    layout_set_group_visibility(messagebox->layout, "btn1", false);
    layout_set_group_visibility(messagebox->layout, "btn2", false);
    layout_trigger_any(
        messagebox->layout, messagebox->small ? "rest-fntsize-small" : "rest-fntsize-normal"
    );
}

void messagebox_show_buttons_icons(MessageBox messagebox, bool show) {
    layout_trigger_any(messagebox->layout, show ? "btnicn-show" : "btnicn-hide");

    if (!show) {
        messagebox_internal_set_icon(messagebox, "btn2icn1", NULL);
        messagebox_internal_set_icon(messagebox, "btn2icn2", NULL);
        messagebox_internal_set_icon(messagebox, "btn1icn", NULL);
    }
}

void messagebox_use_small_size(MessageBox messagebox, bool small_or_normal) {
    if (messagebox->small == small_or_normal) return;
    if (small_or_normal) messagebox_hide_image(messagebox, true);
    layout_trigger_any(messagebox->layout, small_or_normal ? "size-small" : "size-normal");
    messagebox->small = small_or_normal;
    messagebox_use_full_title(messagebox, messagebox->full_title);
}

void messagebox_use_full_title(MessageBox messagebox, bool enable) {
    messagebox->full_title = enable;
    if (enable)
        layout_trigger_any(messagebox->layout, messagebox->small ? "full-title-small" : "full-title-big");
    else
        layout_trigger_any(messagebox->layout, "no-full-title");
}

void messagebox_set_image_sprite(MessageBox messagebox, Sprite sprite) {
    if (!sprite) {
        layout_external_vertex_set_entry(messagebox->layout, 0, VERTEX_SPRITE, sprite, -1);
        return;
    }

    const LayoutPlaceholder* placeholder = layout_get_placeholder(messagebox->layout, "img");
    if (!placeholder) return;

    float draw_width = 0.0f, draw_height = 0.0f;
    float draw_x = 0.0f, draw_y = 0.0f;

    sprite_get_source_size(sprite, &draw_width, &draw_height);
    imgutils_calc_rectangle(
        placeholder->x, placeholder->y, placeholder->width, placeholder->height,
        draw_width, draw_height,
        ALIGN_CENTER, ALIGN_CENTER,
        &draw_width, &draw_height, &draw_x, &draw_y
    );

    sprite_set_draw_size(sprite, draw_width, draw_height);
    sprite_set_draw_location(sprite, draw_x, draw_y);
    sprite_set_z_index(sprite, placeholder->z);
    sprite_set_visible(sprite, true);
    layout_external_vertex_set_entry(messagebox->layout, 0, VERTEX_SPRITE, sprite, -1);
}

void messagebox_set_image_from_texture(MessageBox messagebox, const char* filename) {
    Texture texture = texture_init(filename);

    sprite_set_visible(messagebox->image, !!texture);
    sprite_destroy_texture(messagebox->image);
    sprite_destroy_all_animations(messagebox->image);

    if (texture) {
        sprite_set_texture(messagebox->image, texture, true);
        sprite_set_draw_size_from_source_size(messagebox->image);
    } else {
        logger_warn("messagebox_set_image_from_texture() can not load: %s", filename);
    }

    messagebox_set_image_sprite(messagebox, messagebox->image);
}

void messagebox_set_image_from_atlas(MessageBox messagebox, const char* filename, const char* entry_name, bool is_animation) {
    Atlas atlas = NULL;
    Texture texture = NULL;

    sprite_destroy_texture(messagebox->image);
    sprite_destroy_all_animations(messagebox->image);

    atlas = atlas_init(filename);
    if (!atlas) goto L_dipose_and_return;

    texture = texture_init(filename);
    if (!texture) goto L_dipose_and_return;

    sprite_set_texture(messagebox->image, texture, true);

    if (is_animation) {
        AnimSprite animsprite = animsprite_init_from_atlas(
            FUNKIN_DEFAULT_ANIMATIONS_FRAMERATE, 0, atlas, entry_name, true
        );
        if (!animsprite) goto L_dipose_and_return;

        sprite_external_animation_set(messagebox->image, animsprite);
        sprite_animate(messagebox->image, 0.0f); // brief animate
    } else {
        const AtlasEntry* atlas_entry = atlas_get_entry(atlas, entry_name);
        if (!atlas_entry) goto L_dipose_and_return;

        atlas_apply_from_entry(messagebox->image, atlas_entry, true);
    }

    sprite_set_visible(messagebox->image, true);
    messagebox_set_image_sprite(messagebox, messagebox->image);
    return;

L_dipose_and_return:
    if (atlas) atlas_destroy(&atlas);
    if (texture) texture_destroy(&texture);

    logger_warn(
        "messagebox_set_image_from_atlas() can not load the atlas, texture or entry: %s", filename
    );

    sprite_set_visible(messagebox->image, false);
    messagebox_set_image_sprite(messagebox, messagebox->image);
}

void messagebox_hide(MessageBox messagebox, bool animated) {
    if (animated) {
        layout_trigger_camera(messagebox->layout, "hide-anim");
        layout_animate(messagebox->layout, 0.0f); // brief animate
        messagebox->hide_animation = true;
    } else {
        drawable_set_visible(messagebox->drawable, false);
    }
}

void messagebox_show(MessageBox messagebox, bool animated) {
    drawable_set_visible(messagebox->drawable, true);
    messagebox->hide_animation = false;

    if (animated)
        layout_trigger_camera(messagebox->layout, "show-anim");
    else
        layout_trigger_camera(messagebox->layout, "show-static");

    layout_animate(messagebox->layout, 0.0f); // brief animate
}

void messagebox_set_z_index(MessageBox messagebox, float z_index) {
    drawable_set_z_index(messagebox->drawable, z_index);
}


Drawable messagebox_get_drawable(MessageBox messagebox) {
    return messagebox->drawable;
}

Modifier* messagebox_get_modifier(MessageBox messagebox) {
    return drawable_get_modifier(messagebox->drawable);
}

int32_t messagebox_animate(MessageBox messagebox, float elapsed) {
    if (messagebox->hide_animation && layout_camera_is_completed(messagebox->layout)) {
        messagebox->hide_animation = false;
        drawable_set_visible(messagebox->drawable, false);
        return 1;
    }

    return layout_animate(messagebox->layout, elapsed);
}

void messagebox_draw(MessageBox messagebox, PVRContext pvrctx) {
    if (!drawable_is_visible(messagebox->drawable)) return;
    drawable_helper_apply_in_context(messagebox->drawable, pvrctx);
    layout_draw(messagebox->layout, pvrctx);
}



static void messagebox_internal_set_text(MessageBox messagebox, const char* name, const char* text) {
    TextSprite textsprite = layout_get_textsprite(messagebox->layout, name);
    if (!textsprite) return;

    textsprite_set_text_intern(textsprite, false, &text);

    float draw_width = 0.0f, draw_height = 0.0f;
    textsprite_get_draw_size(textsprite, &draw_width, &draw_height);

    float width = draw_width;
    textsprite_get_max_draw_size(textsprite, &draw_width, &draw_height);

    if (width <= draw_width) return;

    float font_size = (draw_width * textsprite_get_font_size(textsprite)) / width;
    textsprite_set_font_size(textsprite, font_size);
}

static void messagebox_internal_set_icon(MessageBox messagebox, const char* name, const char* icon_name) {
    Sprite sprite = layout_get_sprite(messagebox->layout, name);
    if (!sprite) return;

    bool visible = icon_name != NULL && messagebox->atlas != NULL;
    if (visible && atlas_apply(messagebox->atlas, sprite, icon_name, true)) visible = false;

    sprite_set_visible(sprite, visible);
}
