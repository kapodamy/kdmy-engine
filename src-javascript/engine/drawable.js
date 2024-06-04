"use strict";

const DRAWABLE_POOL = new Map();
var DRAWABLE_IDS = 0;

function drawable_init(z, private_data, callback_draw, callback_animate) {
    let drawable = {
        id: DRAWABLE_IDS++,

        alpha: 1.0,
        alpha2: 1.0,
        offsetcolor: [],
        modifier: {},
        psshader: null,

        blend_enabled: true,
        blend_src_rgb: BLEND_DEFAULT,
        blend_dst_rgb: BLEND_DEFAULT,
        blend_src_alpha: BLEND_DEFAULT,
        blend_dst_alpha: BLEND_DEFAULT,

        z_index: z,
        z_offset: 0.0,

        callback_draw, callback_animate, private_data,

        visible: true,

        antialiasing: PVRCTX_FLAG_DEFAULT
    };

    pvr_context_helper_clear_offsetcolor(drawable.offsetcolor);
    pvr_context_helper_clear_modifier(drawable.modifier);

    DRAWABLE_POOL.set(drawable.id, drawable);
    return drawable;
}

function drawable_destroy(drawable) {
    DRAWABLE_POOL.delete(drawable.id);

    drawable.callback_draw = null;
    drawable.callback_animate = null;
    drawable.private_data = null;

    luascript_drop_shared(drawable.modifier);
    luascript_drop_shared(drawable);
    drawable = undefined;
}


function drawable_set_z_index(drawable, z_index) {
    drawable.z_index = z_index;
}

function drawable_get_z_index(drawable) {
    return drawable.z_index + drawable.z_offset;
}

function drawable_set_z_offset(drawable, offset) {
    drawable.z_offset = offset;
}


function drawable_set_alpha(drawable, alpha) {
    drawable.alpha = alpha;
}

function drawable_get_alpha(drawable) {
    return drawable.alpha * drawable.alpha2;
}

function drawable_set_offsetcolor(drawable, r, g, b, a) {
    if (r != null && !isNaN(r)) drawable.offsetcolor[0] = r;
    if (g != null && !isNaN(g)) drawable.offsetcolor[1] = g;
    if (b != null && !isNaN(b)) drawable.offsetcolor[2] = b;
    if (a != null && !isNaN(a)) drawable.offsetcolor[3] = a;
}

function drawable_set_offsetcolor_to_default(drawable) {
    pvr_context_helper_clear_offsetcolor(drawable.offsetcolor);
}

function drawable_set_visible(drawable, visible) {
    drawable.visible = visible;
}

function drawable_get_modifier(drawable) {
    return drawable.modifier;
}


function drawable_draw(drawable, pvrctx) {
    return drawable.callback_draw(drawable.private_data, pvrctx);
}

function drawable_change_private_data(drawable, private_data) {
    drawable.private_data = private_data;
}

function drawable_animate(drawable, elapsed) {
    if (drawable.callback_animate)
        return drawable.callback_animate(drawable.private_data, elapsed);
    else
        return 0;
}

function drawable_set_property(drawable, property_id, value) {
    switch (property_id) {
        case SPRITE_PROP_X:
            drawable.modifier.x = value;
            break;
        case SPRITE_PROP_Y:
            drawable.modifier.y = value;
            break;
        case SPRITE_PROP_WIDTH:
            drawable.modifier.width = value;
            break;
        case SPRITE_PROP_HEIGHT:
            drawable.modifier.height = value;
            break;
        case SPRITE_PROP_ROTATE:
            drawable.modifier.rotate = value * MATH2D_DEG_TO_RAD;
            break;
        case SPRITE_PROP_SCALE_X:
            drawable.modifier.scale_x = value;
            break;
        case SPRITE_PROP_SCALE_Y:
            drawable.modifier.scale_y = value;
            break;
        case SPRITE_PROP_SKEW_X:
            drawable.modifier.skew_x = value;
            break;
        case SPRITE_PROP_SKEW_Y:
            drawable.modifier.skew_y = value;
            break;
        case SPRITE_PROP_TRANSLATE_X:
            drawable.modifier.translate_x = value;
            break;
        case SPRITE_PROP_TRANSLATE_Y:
            drawable.modifier.translate_y = value;
            break;
        case SPRITE_PROP_ALPHA:
            drawable.alpha = math2d_clamp_float(value, 0.0, 1.0);
            break;
        case SPRITE_PROP_Z:
            drawable.z_index = value;
            break;
        case SPRITE_PROP_VERTEX_COLOR_R:
        case SPRITE_PROP_VERTEX_COLOR_G:
        case SPRITE_PROP_VERTEX_COLOR_B:
            break;
        case SPRITE_PROP_VERTEX_COLOR_OFFSET_R:
            drawable.offsetcolor[0] = value;
            break;
        case SPRITE_PROP_VERTEX_COLOR_OFFSET_G:
            drawable.offsetcolor[1] = value;
            break;
        case SPRITE_PROP_VERTEX_COLOR_OFFSET_B:
            drawable.offsetcolor[2] = value;
            break;
        case SPRITE_PROP_VERTEX_COLOR_OFFSET_A:
            drawable.offsetcolor[3] = value;
            break;
        case SPRITE_PROP_ANIMATIONLOOP:
            break;
        case SPRITE_PROP_ROTATE_PIVOT_ENABLED:
            drawable.modifier.rotate_pivot_enabled = value >= 1.0;
            break;
        case SPRITE_PROP_ROTATE_PIVOT_U:
            drawable.modifier.rotate_pivot_u = value;
            break;
        case SPRITE_PROP_ROTATE_PIVOT_V:
            drawable.modifier.rotate_pivot_v = value;
            break;
        case SPRITE_PROP_SCALE_DIRECTION_X:
            drawable.modifier.scale_direction_x = value;
            break;
        case SPRITE_PROP_SCALE_DIRECTION_Y:
            drawable.modifier.scale_direction_y = value;
            break;
        case SPRITE_PROP_TRANSLATE_ROTATION:
            drawable.modifier.translate_rotation = value >= 1.0;
            break;
        case SPRITE_PROP_SCALE_SIZE:
            drawable.modifier.scale_size = value >= 1.0;
            break;
        case SPRITE_PROP_SCALE_TRANSLATION:
            drawable.modifier.scale_translation = value >= 1.0;
            break;
        case SPRITE_PROP_Z_OFFSET:
            drawable.z_offset = value;
            break;
        case SPRITE_PROP_ANTIALIASING:
            drawable.antialiasing = Math.trunc(value);
            break;
        case SPRITE_PROP_ALPHA2:
            drawable.alpha2 = value;
            break;
    }
}

function drawable_helper_apply_in_context(drawable, pvrctx) {
    pvr_context_apply_modifier(pvrctx, drawable.modifier);
    pvr_context_set_global_alpha(pvrctx, drawable.alpha * drawable.alpha2);
    pvr_context_set_global_offsetcolor(pvrctx, drawable.offsetcolor);
    if (drawable.antialiasing != PVRCTX_FLAG_DEFAULT) {
        pvr_context_set_global_antialiasing(pvrctx, drawable.antialiasing);
    }
    if (drawable.psshader) pvr_context_add_shader(pvrctx, drawable.psshader);

    pvr_context_set_vertex_blend(
        pvrctx,
        drawable.blend_enabled,
        drawable.blend_src_rgb,
        drawable.blend_dst_rgb,
        drawable.blend_src_alpha,
        drawable.blend_dst_alpha
    );
}

function drawable_helper_update_from_placeholder(drawable, layout_placeholder) {
    drawable.modifier.x = layout_placeholder.x;
    drawable.modifier.y = layout_placeholder.y;
    drawable.z_index = layout_placeholder.z;
    drawable.modifier.width = layout_placeholder.width;
    drawable.modifier.height = layout_placeholder.height;
    drawable.z_offset = 0.0;
}


function drawable_is_visible(drawable) {
    return drawable.visible;
}

function drawable_get_draw_location(drawable, output_draw_location) {
    output_draw_location[0] = drawable.modifier.x;
    output_draw_location[1] = drawable.modifier.y;
    return output_draw_location;
}

function drawable_set_antialiasing(drawable, antialiasing) {
    drawable.antialiasing = antialiasing;
}

function drawable_set_shader(drawable, psshader) {
    drawable.psshader = psshader;
}

function drawable_get_shader(drawable) {
    return drawable.psshader;
}

function drawable_blend_enable(drawable, enabled) {
    drawable.blend_enabled = enabled;
}

function drawable_blend_set(drawable, src_rgb, dst_rgb, src_alpha, dst_alpha) {
    drawable.blend_src_rgb = src_rgb;
    drawable.blend_dst_rgb = dst_rgb;
    drawable.blend_src_alpha = src_alpha;
    drawable.blend_dst_alpha = dst_alpha;
}

