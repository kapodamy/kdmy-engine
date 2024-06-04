"use strict";

const MESSAGEBOX_LAYOUT = "/assets/common/image/messagebox.xml";

async function messagebox_init() {
    let layout = await layout_init(MESSAGEBOX_LAYOUT);
    if (!layout) throw new Error("can not load " + MESSAGEBOX_LAYOUT);

    let atlas = null;
    let atlas_src = layout_get_attached_value(layout, "btn-atlas", LAYOUT_TYPE_STRING, null);
    if (atlas_src) atlas = await atlas_init(atlas_src);

    let messagebox = {
        image: sprite_init(null),
        layout, atlas,
        drawable: null,
        hide_animation: false,
        small: false,
        full_title: false
    };

    messagebox.drawable = drawable_init(-1, messagebox, messagebox_draw, messagebox_animate);
    drawable_set_visible(messagebox.drawable, false);
    layout_external_vertex_create_entries(messagebox.layout, 1);

    messagebox_set_button_single_icon(messagebox, "start");
    messagebox_set_buttons_icons(messagebox, "a", "b");
    messagebox_show_buttons_icons(messagebox, false);

    return messagebox;
}

function messagebox_destroy(messagebox) {
    luascript_drop_shared(messagebox);
    layout_destroy(messagebox.layout);
    if (messagebox.atlas) atlas_destroy(messagebox.atlas);
    drawable_destroy(messagebox.drawable);
    sprite_destroy_full(messagebox.image);

    messagebox = undefined;
}


function messagebox_set_buttons_text(messagebox, left_text, right_text) {
    layout_set_group_visibility(messagebox.layout, "btn1", false);
    layout_set_group_visibility(messagebox.layout, "btn2", true);

    layout_trigger_any(
        messagebox.layout, messagebox.small ? "rest-fntsize-small" : "rest-fntsize-normal"
    );

    messagebox_internal_set_text(messagebox, "btn1text", left_text);
    messagebox_internal_set_text(messagebox, "btn2text", right_text);

    layout_set_group_visibility(messagebox.layout, "btn1g", left_text != null);
    layout_set_group_visibility(messagebox.layout, "btn2g", right_text != null);
}

function messagebox_set_button_single(messagebox, center_text) {
    layout_set_group_visibility(messagebox.layout, "btn1", true);
    layout_set_group_visibility(messagebox.layout, "btn2", false);

    layout_trigger_any(
        messagebox.layout, messagebox.small ? "rest-fntsize-small" : "rest-fntsize-normal"
    );

    messagebox_internal_set_text(messagebox, "btntext", center_text);
}

function messagebox_set_buttons_icons(messagebox, left_icon_name, right_icon_name) {
    messagebox_show_buttons_icons(messagebox, true);
    messagebox_internal_set_icon(messagebox, "btn2icn1", left_icon_name);
    messagebox_internal_set_icon(messagebox, "btn2icn2", right_icon_name);
    layout_trigger_any(messagebox.layout, "btn-resize");
}

function messagebox_set_button_single_icon(messagebox, center_icon_name) {
    messagebox_show_buttons_icons(messagebox, true);
    messagebox_internal_set_icon(messagebox, "btn1icn", center_icon_name);
    layout_trigger_any(messagebox.layout, "btn-resize");
}

function messagebox_set_title(messagebox, text) {
    let textsprite = layout_get_textsprite(messagebox.layout, "title");
    if (textsprite) textsprite_set_text_intern(textsprite, false, text);
}

function messagebox_set_image_background_color(messagebox, color_rgb8) {
    let sprite = layout_get_sprite(messagebox.layout, "imgbg");
    if (sprite) sprite_set_vertex_color_rgb8(sprite, color_rgb8);
}

function messagebox_set_image_background_color_default(messagebox) {
    layout_trigger_any(messagebox.layout, "def-imgcolor");
}

function messagebox_set_message(messagebox, text) {
    let textsprite = layout_get_textsprite(messagebox.layout, "message");
    if (textsprite) textsprite_set_text_intern(textsprite, false, text);
}

function messagebox_set_message_formated(messagebox, format, ...values) {
    let textsprite = layout_get_textsprite(messagebox.layout, "message");
    if (textsprite) textsprite_set_text_formated2(textsprite, format, values);
}

function messagebox_hide_image_background(messagebox, hide) {
    layout_trigger_any(messagebox.layout, hide ? "no-imgbg" : "with-imgbg");
}

function messagebox_hide_image(messagebox, hide) {
    layout_trigger_any(messagebox.layout, hide ? "no-img" : "with-img");
    layout_set_group_visibility(messagebox.layout, "imgg", !hide);
}

function messagebox_hide_buttons(messagebox) {
    layout_set_group_visibility(messagebox.layout, "btn1", false);
    layout_set_group_visibility(messagebox.layout, "btn2", false);
    layout_trigger_any(
        messagebox.layout, messagebox.small ? "rest-fntsize-small" : "rest-fntsize-normal"
    );
}

function messagebox_show_buttons_icons(messagebox, show) {
    layout_trigger_any(messagebox.layout, show ? "btnicn-show" : "btnicn-hide");

    if (!show) {
        messagebox_internal_set_icon(messagebox, "btn2icn1", null);
        messagebox_internal_set_icon(messagebox, "btn2icn2", null);
        messagebox_internal_set_icon(messagebox, "btn1icn", null);
    }
}

function messagebox_use_small_size(messagebox, small_or_normal) {
    if (messagebox.small == small_or_normal) return;
    if (small_or_normal) messagebox_hide_image(messagebox, true);
    layout_trigger_any(messagebox.layout, small_or_normal ? "size-small" : "size-normal");
    messagebox.small = small_or_normal;
    messagebox_use_full_title(messagebox, messagebox.full_title);
}

function messagebox_use_full_title(messagebox, enable) {
    messagebox.full_title = !!enable;
    if (enable)
        layout_trigger_any(messagebox.layout, messagebox.small ? "full-title-small" : "full-title-big");
    else
        layout_trigger_any(messagebox.layout, "no-full-title");
}

function messagebox_set_image_sprite(messagebox, sprite) {
    if (!sprite) {
        layout_external_vertex_set_entry(messagebox.layout, 0, VERTEX_SPRITE, sprite, -1);
        return;
    }

    let placeholder = layout_get_placeholder(messagebox.layout, "img");
    if (!placeholder) return;

    const draw_size = [0.0, 0.0];
    const draw_location = [0.0, 0.0];

    sprite_get_source_size(sprite, draw_size);
    imgutils_calc_rectangle(
        placeholder.x, placeholder.y, placeholder.width, placeholder.height,
        draw_size[0], draw_size[1],
        ALIGN_CENTER, ALIGN_CENTER,
        draw_size, draw_location
    );

    sprite_set_draw_size(sprite, draw_size[0], draw_size[1]);
    sprite_set_draw_location(sprite, draw_location[0], draw_location[1]);
    sprite_set_z_index(sprite, placeholder.z);
    sprite_set_visible(sprite, true);
    layout_external_vertex_set_entry(messagebox.layout, 0, VERTEX_SPRITE, sprite, -1);
}

async function messagebox_set_image_from_texture(messagebox, filename) {
    let texture = await texture_init(filename);

    sprite_set_visible(messagebox.image, !!texture);
    sprite_destroy_texture(messagebox.image);
    sprite_destroy_all_animations(messagebox.image);

    if (texture) {
        sprite_set_texture(messagebox.image, texture, true);
        sprite_set_draw_size_from_source_size(messagebox.image);
    } else {
        console.warn("messagebox_set_image_from_texture() can not load: " + filename);
    }

    messagebox_set_image_sprite(messagebox, messagebox.image);
}

async function messagebox_set_image_from_atlas(messagebox, filename, entry_name, is_animation) {
    let atlas = null;
    let texture = null;

    sprite_destroy_texture(messagebox.image);
    sprite_destroy_all_animations(messagebox.image);

    L_resource_load: {
        atlas = await atlas_init(filename);
        if (!atlas) break L_resource_load;

        texture = await texture_init(filename);
        if (!texture) break L_resource_load;

        sprite_set_texture(messagebox.image, texture, true);

        if (is_animation) {
            let animsprite = animsprite_init_from_atlas(
                FUNKIN_DEFAULT_ANIMATIONS_FRAMERATE, 0, atlas, entry_name, true
            );
            if (!animsprite) break L_resource_load;

            sprite_external_animation_set(messagebox.image, animsprite);
            sprite_animate(messagebox.image, 0.0);// brief animate
        } else {
            let atlas_entry = atlas_get_entry(atlas, entry_name);
            if (!atlas_entry) break L_resource_load;

            atlas_apply_from_entry(messagebox.image, atlas_entry, true);
        }

        sprite_set_visible(messagebox.image, true);
        messagebox_set_image_sprite(messagebox, messagebox.image);
        return;
    }

    if (atlas) atlas_destroy(atlas);
    if (texture) texture_destroy(texture);

    console.warn(
        "messagebox_set_image_from_atlas() can not load the atlas, texture or entry: " + filename
    );

    sprite_set_visible(messagebox.image, false);
    messagebox_set_image_sprite(messagebox, messagebox.image);
}

function messagebox_hide(messagebox, animated) {
    if (animated) {
        layout_trigger_camera(messagebox.layout, "hide-anim");
        layout_animate(messagebox.layout, 0.0);// brief animate
        messagebox.hide_animation = true;
    } else {
        drawable_set_visible(messagebox.drawable, false);
    }
}

function messagebox_show(messagebox, animated) {
    drawable_set_visible(messagebox.drawable, true);
    messagebox.hide_animation = false;

    if (animated)
        layout_trigger_camera(messagebox.layout, "show-anim");
    else
        layout_trigger_camera(messagebox.layout, "show-static");

    layout_animate(messagebox.layout, 0.0);// brief animate
}

function messagebox_set_z_index(messagebox, z_index) {
    drawable_set_z_index(messagebox.drawable, z_index);
}


function messagebox_get_drawable(messagebox) {
    return messagebox.drawable;
}

function messagebox_get_modifier(messagebox) {
    return drawable_get_modifier(messagebox.drawable);
}

function messagebox_animate(messagebox, elapsed) {
    if (messagebox.hide_animation && layout_camera_is_completed(messagebox.layout)) {
        messagebox.hide_animation = false;
        drawable_set_visible(messagebox.drawable, false);
        return 1;
    }

    return layout_animate(messagebox.layout, elapsed);
}

function messagebox_draw(messagebox, pvrctx) {
    if (!drawable_is_visible(messagebox.drawable)) return;
    drawable_helper_apply_in_context(messagebox.drawable, pvrctx);
    layout_draw(messagebox.layout, pvrctx);
}



function messagebox_internal_set_text(messagebox, name, text) {
    let textsprite = layout_get_textsprite(messagebox.layout, name);
    if (!textsprite) return;

    textsprite_set_text_intern(textsprite, false, text);

    let draw_size = [0.0, 0.0];
    textsprite_get_draw_size(textsprite, draw_size);

    let width = draw_size[0];
    textsprite_get_max_draw_size(textsprite, draw_size);

    if (width <= draw_size[0]) return;

    let font_size = (draw_size[0] * textsprite_get_font_size(textsprite)) / width;
    textsprite_set_font_size(textsprite, font_size);
}

function messagebox_internal_set_icon(messagebox, name, icon_name) {
    let sprite = layout_get_sprite(messagebox.layout, name);
    if (!sprite) return;

    let visible = icon_name != null && messagebox.atlas != null;
    if (visible && atlas_apply(messagebox.atlas, sprite, icon_name, true)) visible = false;

    sprite_set_visible(sprite, visible);
}

