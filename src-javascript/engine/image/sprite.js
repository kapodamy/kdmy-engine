"use strict";

var SPRITE_IDS = 0;
const SPRITE_POOL = new Map();


function sprite_draw(sprite, pvrctx) {
    if (sprite.draw_width < 1 || sprite.draw_height < 1) return;
    if (sprite.alpha <= 0) return;

    let sprite_vertex = sprite.vertex;
    let render_alpha = sprite.alpha * sprite.alpha2;

    let draw_x = sprite.draw_x;
    let draw_y = sprite.draw_y;

    if (sprite.vertex_dirty) {
        let draw_width = sprite.draw_width;
        let draw_height = sprite.draw_height;

        // flip vertex (if required)
        if (sprite.flip_x) {
            if (sprite.flip_correction) draw_x += draw_width;
            draw_width = -draw_width;
        }
        if (sprite.flip_y) {
            if (sprite.flip_correction) draw_y += draw_height;
            draw_height = -draw_height;
        }

        if (sprite.texture) {
            let frame_width, frame_height;
            let crop_x, crop_y, crop_width, crop_height;
            let ratio_width, ratio_height;

            // complex frame size redimension
            if (sprite.frame_width > 0) {
                ratio_width = draw_width / sprite.frame_width;
                frame_width = sprite.src_width * ratio_width;
            } else {
                ratio_width = draw_width / sprite.src_width;
                frame_width = draw_width;
            }
            if (sprite.frame_height > 0) {
                ratio_height = draw_height / sprite.frame_height;
                frame_height = sprite.src_height * ratio_height;
            } else {
                ratio_height = draw_height / sprite.src_height;
                frame_height = draw_height;
            }

            // calculate cropping (if required)
            if (sprite.crop_enabled) {
                if (sprite.crop.x >= frame_width || sprite.crop.y >= frame_height)
                    return;

                crop_x = sprite.crop.x;
                crop_y = sprite.crop.y;
                crop_width = crop_height = 0;

                if (sprite.crop.width != -1 && sprite.crop.width < frame_width)
                    crop_width = frame_width - sprite.crop.width;

                if (sprite.crop.height != -1 && sprite.crop.height < frame_height)
                    crop_height = frame_height - sprite.crop.height;
            } else {
                crop_x = crop_y = crop_width = crop_height = 0;
            }

            // draw location & size
            sprite_vertex[4] = draw_x - ((sprite.pivot_x + sprite.frame_x) * ratio_width);
            sprite_vertex[5] = draw_y - ((sprite.pivot_y + sprite.frame_y) * ratio_height);
            sprite_vertex[6] = frame_width - crop_width - crop_x;
            sprite_vertex[7] = frame_height - crop_height - crop_y;

            if (sprite.crop_enabled) {
                crop_x /= ratio_width;
                crop_y /= ratio_height;
                crop_width /= ratio_width;
                crop_height /= ratio_height;
            }

            // source location & size
            sprite_vertex[0] = sprite.src_x + crop_x;
            sprite_vertex[1] = sprite.src_y + crop_y;
            sprite_vertex[2] = sprite.src_width - crop_width - crop_x;
            sprite_vertex[3] = sprite.src_height - crop_height - crop_y;
        } else {
            sprite_vertex[4] = draw_x;
            sprite_vertex[5] = draw_y;
            sprite_vertex[6] = draw_width;
            sprite_vertex[7] = draw_height;

            if (sprite.crop_enabled) {
                let crop_width = sprite.crop.width;
                let crop_height = sprite.crop.height;

                sprite_vertex[4] += sprite.crop.x;
                sprite_vertex[5] += sprite.crop.y;
                sprite_vertex[6] -= sprite.crop.x;
                sprite_vertex[7] -= sprite.crop.y;

                if (crop_width != -1 && crop_width < sprite_vertex[6])
                    sprite_vertex[6] = crop_width;
                if (crop_height != -1 && crop_height < sprite_vertex[7])
                    sprite_vertex[7] = crop_height;
            }
        }

        // cache the calculated vertex
        sprite.vertex_dirty = 0;
    }

    pvr_context_save(pvrctx);
    if (sprite.psshader) pvr_context_add_shader(pvrctx, sprite.psshader);

    pvr_context_set_vertex_blend(
        pvrctx,
        sprite.blend_enabled,
        sprite.blend_src_rgb,
        sprite.blend_dst_rgb,
        sprite.blend_src_alpha,
        sprite.blend_dst_alpha
    );

    if (sprite.antialiasing != PVR_FLAG_DEFAULT) {
        pvr_context_set_vertex_antialiasing(pvrctx, sprite.antialiasing);
    }

    let matrix = pvrctx.current_matrix;

    // apply self modifier
    sh4matrix_apply_modifier2(
        matrix, sprite.matrix_source, draw_x, draw_y, sprite.draw_width, sprite.draw_height
    );

    // do corner rotation (if required)
    if (sprite.matrix_corner.angle != 0) {
        sh4matrix_corner_rotate(
            matrix, sprite.matrix_corner,
            draw_x, draw_y, sprite.draw_width, sprite.draw_height
        );
    }

    // draw sprites trail if necessary
    if (!sprite.trailing_disabled && sprite.trailing_used > 0) {
        /**@type {RGBA}*/ let trailing_offsetcolor = [
            sprite.trailing_offsetcolor[0], sprite.trailing_offsetcolor[1], sprite.trailing_offsetcolor[2], 1.0
        ];
        if (sprite.offsetcolor[3] >= 0) {
            trailing_offsetcolor[0] *= sprite.offsetcolor[0];
            trailing_offsetcolor[1] *= sprite.offsetcolor[1];
            trailing_offsetcolor[2] *= sprite.offsetcolor[2];
            trailing_offsetcolor[3] *= sprite.offsetcolor[3];
        }

        pvr_context_set_vertex_offsetcolor(pvrctx, trailing_offsetcolor);
        pvr_context_set_vertex_textured_darken(pvrctx, sprite.trailing_darken);

        for (let i = 0; i < sprite.trailing_used; i++) {
            const trail = sprite.trailing_buffer[i];
            pvr_context_set_vertex_alpha(pvrctx, trail.alpha * render_alpha);

            if (sprite.texture) {
                pvr_context_draw_texture(
                    pvrctx,
                    sprite.texture,
                    trail.sx, trail.sy, trail.sw, trail.sh,
                    trail.dx, trail.dy, trail.dw, trail.dh
                );
            } else {
                pvr_context_draw_solid_color(
                    pvrctx,
                    sprite.vertex_color,
                    trail.dx, trail.dy, trail.dw, trail.dh
                );
            }
        }

        // restore previous values
        pvr_context_set_vertex_textured_darken(pvrctx, 0);
    }

    pvr_context_set_vertex_alpha(pvrctx, render_alpha);
    pvr_context_set_vertex_offsetcolor(pvrctx, sprite.offsetcolor);

    // draw the vertex
    if (sprite.texture) {
        // upload texture to the PVR VRAM, if was not loaded yet
        texture_upload_to_pvr(sprite.texture);

        // textured vertex
        pvr_context_draw_texture(
            pvrctx,
            sprite.texture,
            sprite_vertex[0], sprite_vertex[1], // source location
            sprite_vertex[2], sprite_vertex[3], // source size

            sprite_vertex[4], sprite_vertex[5], // draw location
            sprite_vertex[6], sprite_vertex[7] // draw size
        );
    } else {
        // un-textured vertex
        pvr_context_draw_solid_color(
            pvrctx, sprite.vertex_color,
            sprite_vertex[4], sprite_vertex[5],// draw location
            sprite_vertex[6], sprite_vertex[7]// draw size
        );
    }

    pvr_context_restore(pvrctx);
}

function sprite_init_from_rgb8(solid_rgb8_color) {
    let percent_color = [0, 0, 0];

    for (let i = 0; i < 3; i++)
        percent_color[2 - i] = ((solid_rgb8_color >> (i * 8)) & 0xFF) / 0xFF;

    let sprite = sprite_init(null);
    sprite_set_vertex_color(sprite, percent_color[0], percent_color[1], percent_color[2]);

    return sprite;
}

function sprite_init(src_texture) {
    let sprite = {};

    sprite.matrix_source = {};
    pvrctx_helper_clear_modifier(sprite.matrix_source);

    sprite.flip_x = 0;
    sprite.flip_y = 0;
    sprite.flip_correction = 1;
    sprite.matrix_corner = { x: 0, y: 0, angle: 0 };

    sprite.src_x = 0;
    sprite.src_y = 0;

    sprite.vertex_color = [1.0, 1.0, 1.0];

    sprite.offsetcolor = [];
    pvrctx_helper_clear_offsetcolor(sprite.offsetcolor);

    sprite.texture = src_texture;
    if (src_texture) {
        sprite.src_width = sprite.texture.original_width;
        sprite.src_height = sprite.texture.original_height;
    } else {
        sprite.src_width = 0;
        sprite.src_height = 0;
    }

    sprite.draw_x = 0;
    sprite.draw_y = 0;
    sprite.draw_width = -1;
    sprite.draw_height = -1;

    sprite.alpha = 1.0;
    sprite.alpha2 = 1.0;
    sprite.visible = 1;
    sprite.z_index = 1;
    sprite.z_offset = 0;

    sprite.frame_x = 0;
    sprite.frame_y = 0;
    sprite.frame_width = 0;
    sprite.frame_height = 0;

    sprite.pivot_x = 0;
    sprite.pivot_y = 0;

    sprite.id = SPRITE_IDS++;
    SPRITE_POOL.set(sprite.id, sprite);

    sprite.animation_list = linkedlist_init();
    sprite.animation_selected = null;
    sprite.animation_external = null;

    sprite.vertex = new Array(8).fill(0.0);
    sprite.vertex_dirty = 1;

    sprite.crop = { x: 0, y: 0, width: -1, height: -1 };
    sprite.crop_enabled = 0;

    sprite.antialiasing = PVR_FLAG_DEFAULT;

    sprite.psshader = null;

    sprite.blend_enabled = 1;
    sprite.blend_src_rgb = BLEND_DEFAULT;
    sprite.blend_dst_rgb = BLEND_DEFAULT;
    sprite.blend_src_alpha = BLEND_DEFAULT;
    sprite.blend_dst_alpha = BLEND_DEFAULT;

    sprite.trailing_buffer = Array(10);
    sprite.trailing_used = 0;
    sprite.trailing_length = 10;
    sprite.trailing_alpha = 0.9;
    sprite.trailing_delay = 0;
    sprite.trailing_darken = 1;
    sprite.trailing_disabled = 1;
    sprite.trailing_progress = 0.0;
    sprite.trailing_offsetcolor = [1, 1, 1];

    sprite_internal_JS_init_trail_array(sprite, 0, sprite.trailing_length);

    return sprite;
}

function sprite_destroy(sprite) {
    linkedlist_destroy(sprite.animation_list);

    sprite.animation_list = null;
    sprite.animation_external = null;
    sprite.texture = null;

    SPRITE_POOL.delete(sprite.id);
    ModuleLuaScript.kdmyEngine_drop_shared_object(sprite.matrix_source);
    ModuleLuaScript.kdmyEngine_drop_shared_object(sprite);
    sprite.trailing_buffer = undefined;
    sprite = undefined;
}

function sprite_destroy_all_animations(sprite) {
    if (sprite.animation_list) {

        for (let animsprite of linkedlist_iterate4(sprite.animation_list))
            if (animsprite) animsprite_destroy(animsprite);

        linkedlist_destroy(sprite.animation_list);
        sprite.animation_list = linkedlist_init();
        sprite.animation_selected = null;
    }

    if (sprite.animation_external) {
        animsprite_destroy(sprite.animation_external);
        sprite.animation_external = null;
    }
}

function sprite_destroy_texture(sprite) {
    if (sprite.texture) {
        texture_destroy(sprite.texture);
        sprite.texture = null;
    }
}

function sprite_destroy_full(sprite) {
    if (!sprite) return;
    sprite_destroy_texture(sprite);
    sprite_destroy_all_animations(sprite);
    sprite_destroy(sprite);
}

function sprite_set_texture(sprite, texture, update_offset_source_size) {
    let old_texture = sprite.texture;
    sprite.texture = texture;

    if (update_offset_source_size && texture) {
        sprite.src_x = 0;
        sprite.src_y = 0;
        sprite.src_width = sprite.texture.original_width;
        sprite.src_height = sprite.texture.original_height;
        sprite.vertex_dirty = 1;
    }

    return old_texture;
}

function sprite_matrix_get_modifier(sprite) {
    return sprite.matrix_source;
}

function sprite_matrix_scale(sprite, scale_x, scale_y) {
    if (scale_x != null) sprite.matrix_source.scale_x = scale_x;
    if (scale_y != null) sprite.matrix_source.scale_y = scale_y;
}

function sprite_matrix_rotate(sprite, degrees_angle) {
    sprite.matrix_source.rotate = degrees_angle * MATH2D_DEG_TO_RAD;
}

function sprite_matrix_skew(sprite, skew_x, skew_y) {
    if (skew_x != null) sprite.matrix_source.skew_x = skew_x;
    if (skew_y != null) sprite.matrix_source.skew_y = skew_y;
}

function sprite_matrix_translate(sprite, translate_x, translate_y) {
    if (translate_x != null) sprite.matrix_source.translate_x = translate_x;
    if (translate_y != null) sprite.matrix_source.translate_y = translate_y;
}

function sprite_set_offset_source(sprite, x, y, width, height) {
    sprite.src_x = x;
    sprite.src_y = y;
    sprite.src_width = width;
    sprite.src_height = height;

    sprite.vertex_dirty = 1;

    if (sprite.draw_width < 0) sprite.draw_width = width;
    if (sprite.draw_height < 0) sprite.draw_height = height;
}

function sprite_set_offset_frame(sprite, x, y, width, height) {
    sprite.frame_x = x;
    sprite.frame_y = y;
    sprite.frame_width = width;
    sprite.frame_height = height;

    sprite.vertex_dirty = 1;

    if (sprite.draw_width < 0) sprite.draw_width = width;
    if (sprite.draw_height < 0) sprite.draw_height = height;
}

function sprite_set_offset_pivot(sprite, x, y) {
    sprite.pivot_x = x;
    sprite.pivot_y = y;

    sprite.vertex_dirty = 1;
}

function sprite_flip_rendered_texture(sprite, flip_x, flip_y) {
    if (flip_x != null) sprite.flip_x = flip_x;
    if (flip_y != null) sprite.flip_y = flip_y;
    sprite.vertex_dirty = 1;
}

function sprite_flip_rendered_texture_enable_correction(sprite, enabled) {
    sprite.flip_correction = !!enabled;
}

function sprite_matrix_reset(sprite) {
    pvrctx_helper_clear_modifier(sprite.matrix_source);
    sprite.flip_x = 0;
    sprite.flip_y = 0;
    sprite.matrix_corner = { x: 0, y: 0, angle: 0 };
}

function sprite_set_draw_location(sprite, x, y) {
    sprite.draw_x = x;
    sprite.draw_y = y;
    sprite.vertex_dirty = 1;
}

function sprite_set_draw_size(sprite, width, height) {
    if (Number.isFinite(width)) sprite.draw_width = width;
    if (Number.isFinite(height)) sprite.draw_height = height;
    sprite.vertex_dirty = 1;

    if (!sprite.texture) {
        if (Number.isFinite(width)) sprite.src_width = width;
        if (Number.isFinite(height)) sprite.src_height = height;
    }
}

function sprite_set_draw_size_from_source_size(sprite) {
    sprite.draw_width = sprite.src_width;
    sprite.draw_height = sprite.src_height;
    sprite.vertex_dirty = 1;
}

function sprite_set_alpha(sprite, alpha) {
    sprite.alpha = alpha;
}

function sprite_get_alpha(sprite) {
    return sprite.alpha * sprite.alpha2;
}

function sprite_set_visible(sprite, visible) {
    sprite.visible = visible;
}

function sprite_set_z_index(sprite, index) {
    sprite.z_index = index;
}

function sprite_set_z_offset(sprite, offset) {
    sprite.z_offset = offset;
}

function sprite_get_z_index(sprite) {
    return sprite.z_index + sprite.z_offset;
}

function sprite_animation_add(sprite, animsprite) {
    if (!animsprite) return;

    // ¿should duplicate first?
    linkedlist_add_item(sprite.animation_list, animsprite);
}

function sprite_animation_remove(sprite, animation_name) {
    if (animation_name == null) {
        linkedlist_destroy(sprite.animation_list);
        sprite.animation_list = linkedlist_init();
        return;
    }

    let iterator = { item: null };
    let anim = null;
    let i = 0;

    while (linkedlist_iterate2(sprite.animation_list, iterator)) {
        if (iterator.item.name == animation_name) {
            linkedlist_remove_item_at(sprite.animation_list, i);
            return;
        }
        i++;
    }
}

function sprite_animation_restart(sprite) {
    if (!sprite.animation_selected) return;
    animsprite_restart(sprite.animation_selected);
    animsprite_update_sprite(sprite.animation_selected, sprite, 1);
}

function sprite_animation_play(sprite, animation_name) {
    //sprite.animation_selected = null;
    if (!animation_name) return 0;

    let animation = sprite_animation_get_attached(sprite, animation_name);

    if (!animation) {
        console.warn("sprite_animation_play() animation no added: " + animation_name);
        return 0;
    }

    sprite.animation_selected = animation;
    return 1;
}

function sprite_animation_play_by_index(sprite, index) {
    //sprite.animation_selected = null;

    if (index < 0 || index >= linkedlist_count(sprite.animation_list)) {
        console.warn("sprite_animation_play_by_index() rejected, index was out-of-bounds");
        return;
    }

    sprite.animation_selected = linkedlist_get_by_index(sprite.animation_list, index);
}

function sprite_animation_play_by_animsprite(sprite, animsprite, only_if_attached) {
    if (!animsprite) {
        if (!only_if_attached)
            sprite.animation_selected = null;

        return 0;
    }

    if (!only_if_attached) {
        //
        // Warning:
        //         using this way, the animation won't be disposed
        //         calling sprite_destroy_all_animations() or sprite_destroy_full()
        //
        sprite.animation_selected = animsprite;
        return 1;
    }

    for (let attached_animsprite of linkedlist_iterate4(sprite.animation_list)) {
        if (attached_animsprite == animsprite) {
            sprite.animation_selected = attached_animsprite;
            return 1;
        }
    }

    return 0;
}

function sprite_animation_play_first(sprite) {
    // if there no animations on the list the "animation_selected" will be null.
    // This means no animation selected
    sprite.animation_selected = linkedlist_get_first_item(sprite.animation_list);
}

function sprite_animation_stop(sprite) {
    if (sprite.animation_selected) animsprite_stop(sprite.animation_selected);
}

function sprite_animation_end(sprite) {
    if (sprite.animation_selected) {
        animsprite_force_end(sprite.animation_selected);
        animsprite_update_sprite(sprite.animation_selected, sprite, 1);
    }
}

function sprite_animate(sprite, elapsed) {
    if (elapsed == null || isNaN(elapsed)) throw new Error("Invalid elapsed value:" + elapsed);

    let result = 1;

    if (sprite.animation_selected) {
        result = animsprite_animate(sprite.animation_selected, elapsed);
        animsprite_update_sprite(sprite.animation_selected, sprite, 1);
    }

    if (sprite.animation_external) {
        result = animsprite_animate(sprite.animation_external, elapsed);
        animsprite_update_sprite(sprite.animation_external, sprite, 0);
    }

    if (sprite.trailing_disabled) return result;

    // check delay for next trail
    let wait = sprite.trailing_progress < sprite.trailing_delay;
    sprite.trailing_progress += elapsed;

    if (wait) return result;
    sprite.trailing_progress -= sprite.trailing_delay;

    // compute trailing using the cached vertex
    let insert_vertex = true;
    if (sprite.trailing_used > 0) {
        // check if the last vertex equals to the current vertex
        const trail = sprite.trailing_buffer[0];
        let source = trail.sx == sprite.vertex[0] && trail.sy == sprite.vertex[1] && trail.sw == sprite.vertex[2] && trail.sh == sprite.vertex[3];
        let draw = trail.dx == sprite.vertex[4] && trail.dy == sprite.vertex[5] && trail.dw == sprite.vertex[6] && trail.dh == sprite.vertex[7];
        let color = sprite.vertex_color[0] == trail.r && sprite.vertex_color[1] == trail.g && sprite.vertex_color[2] == trail.b;
        insert_vertex = !source || !draw || !color;

        if (insert_vertex) {
            // do right shift (this should be optimized to shift only used trails)
            for (let i = sprite.trailing_length - 1, j = sprite.trailing_length - 2; j >= 0; j--, i--) {
                clone_struct_to(sprite.trailing_buffer[j], sprite.trailing_buffer[i], NaN);
            }
        } else {
            sprite.trailing_used--;
        }
    }

    if (insert_vertex) {
        // add new trail
        const trail = sprite.trailing_buffer[0];
        trail.sx = sprite.vertex[0];
        trail.sy = sprite.vertex[1];
        trail.sw = sprite.vertex[2];
        trail.sh = sprite.vertex[3];
        trail.dx = sprite.vertex[4];
        trail.dy = sprite.vertex[5];
        trail.dw = sprite.vertex[6];
        trail.dh = sprite.vertex[7];
        trail.r = sprite.vertex_color[0];
        trail.g = sprite.vertex_color[1];
        trail.b = sprite.vertex_color[2];
        if (sprite.trailing_used < sprite.trailing_length) sprite.trailing_used++;
    }

    // update alpha of each trail
    for (let i = 0; i < sprite.trailing_used; i++) {
        sprite.trailing_buffer[i].alpha = (1.0 - (i / sprite.trailing_used)) * sprite.trailing_alpha;
    }

    return result;
}

function sprite_animation_get_attached(sprite, animation_name) {
    if (linkedlist_count(sprite.animation_list) < 1) {
        console.warn("No animation attached, can not play: " + animation_name);
        return null;
    }

    let iterator = { item: null };

    while (linkedlist_iterate2(sprite.animation_list, iterator)) {
        if (animation_name == null || iterator.item.name == animation_name)
            return iterator.item;
    }

    return null;
}


function sprite_get_source_size(sprite, size) {
    size[0] = sprite.frame_width > 0 ? sprite.frame_width : sprite.src_width;
    size[1] = sprite.frame_height > 0 ? sprite.frame_height : sprite.src_height;
    return size;
}

function sprite_get_draw_size(sprite, size) {
    size[0] = sprite.draw_width >= 0 ? sprite.draw_width : sprite.src_width;
    size[1] = sprite.draw_height >= 0 ? sprite.draw_height : sprite.src_height;
    return size;
}

function sprite_get_draw_location(sprite, location) {
    location[0] = sprite.draw_x;
    location[1] = sprite.draw_y;
    return location;
}


function sprite_set_property(sprite, property_id, value) {
    switch (property_id) {
        case SPRITE_PROP_X:
            sprite.draw_x = value;
            sprite.vertex_dirty = 1;
            break;
        case SPRITE_PROP_Y:
            sprite.draw_y = value;
            sprite.vertex_dirty = 1;
            break;
        case SPRITE_PROP_WIDTH:
            sprite.draw_width = value;
            sprite.vertex_dirty = 1;
            break;
        case SPRITE_PROP_HEIGHT:
            sprite.draw_height = value;
            sprite.vertex_dirty = 1;
            break;
        case SPRITE_PROP_ROTATE:
            sprite_matrix_rotate(sprite, value);
            break;
        case SPRITE_PROP_SCALE_X:
            sprite_matrix_scale(sprite, value, null);
            break;
        case SPRITE_PROP_SCALE_Y:
            sprite_matrix_scale(sprite, null, value);
            break;
        case SPRITE_PROP_SKEW_X:
            sprite_matrix_skew(sprite, value, null);
            break;
        case SPRITE_PROP_SKEW_Y:
            sprite_matrix_skew(sprite, null, value);
            break;
        case SPRITE_PROP_TRANSLATE_X:
            sprite_matrix_translate(sprite, value, null);
            break;
        case SPRITE_PROP_TRANSLATE_Y:
            sprite_matrix_translate(sprite, null, value);
            break;
        case SPRITE_PROP_ALPHA:
            sprite.alpha = math2d_clamp(value, 0, 1.0);
            break;
        case SPRITE_PROP_Z:
            sprite.z_index = value;
            break;
        case SPRITE_PROP_VERTEX_COLOR_R:
            sprite_set_vertex_color(sprite, value, -1, -1);
            break;
        case SPRITE_PROP_VERTEX_COLOR_G:
            sprite_set_vertex_color(sprite, -1, value, -1);
            break;
        case SPRITE_PROP_VERTEX_COLOR_B:
            sprite_set_vertex_color(sprite, -1, -1, value);
            break;
        case SPRITE_PROP_VERTEX_COLOR_OFFSET_R:
            sprite.offsetcolor[0] = value;
            break;
        case SPRITE_PROP_VERTEX_COLOR_OFFSET_G:
            sprite.offsetcolor[1] = value;
            break;
        case SPRITE_PROP_VERTEX_COLOR_OFFSET_B:
            sprite.offsetcolor[2] = value;
            break;
        case SPRITE_PROP_VERTEX_COLOR_OFFSET_A:
            sprite.offsetcolor[3] = value;
            break;
        case SPRITE_PROP_ANIMATIONLOOP:
            if (sprite.animation_selected)
                animsprite_set_loop(sprite.animation_selected, value);
            break;
        case SPRITE_PROP_ROTATE_PIVOT_ENABLED:
            sprite_matrix_rotate_pivot_enable(sprite, value >= 1.0);
            break;
        case SPRITE_PROP_ROTATE_PIVOT_U:
            sprite_matrix_rotate_pivot(sprite, value, null);
            break;
        case SPRITE_PROP_ROTATE_PIVOT_V:
            sprite_matrix_rotate_pivot(sprite, null, value);
            break;
        case SPRITE_PROP_SCALE_DIRECTION_X:
            sprite.matrix_source.scale_direction_x = value;
            break;
        case SPRITE_PROP_SCALE_DIRECTION_Y:
            sprite.matrix_source.scale_direction_y = value;
            break;
        case SPRITE_PROP_TRANSLATE_ROTATION:
            sprite.matrix_source.translate_rotation = value >= 1.0;
            break;
        case SPRITE_PROP_SCALE_SIZE:
            sprite.matrix_source.scale_size = value >= 1.0;
            break;
        case SPRITE_PROP_SCALE_TRANSLATION:
            sprite.matrix_source.scale_translation = value >= 1.0;
            break;
        case SPRITE_PROP_Z_OFFSET:
            sprite.z_offset = value;
            break;
        case SPRITE_PROP_FLIP_X:
            sprite.flip_x = value >= 1.0;
            sprite.vertex_dirty = true;
            break;
        case SPRITE_PROP_FLIP_Y:
            sprite.flip_y = value >= 1.0;
            sprite.vertex_dirty = true;
            break;
        case SPRITE_PROP_ANTIALIASING:
            sprite.antialiasing = Math.trunc(value);
            break;
        case SPRITE_PROP_FLIP_CORRECTION:
            sprite.flip_correction = value >= 1.0;
            sprite.vertex_dirty = true;
            break;
        case SPRITE_PROP_ALPHA2:
            sprite.alpha2 = value;
            break;
    }
}


function sprite_set_vertex_color_rgb8(sprite, rbg8_color) {
    math2d_color_bytes_to_floats(rbg8_color, 0, sprite.vertex_color);
}

function sprite_set_vertex_color(sprite, r, g, b) {
    if (r >= 0) sprite.vertex_color[0] = r;
    if (g >= 0) sprite.vertex_color[1] = g;
    if (b >= 0) sprite.vertex_color[2] = b;
}

function sprite_get_vertex_color_rgb8(sprite) {
    return math2d_color_floats_to_bytes(sprite.vertex_color, 0);
}


function sprite_set_offsetcolor_rgba8(sprite, rgba8_color) {
    math2d_color_bytes_to_floats(rgba8_color, 1, sprite.offsetcolor);
}

function sprite_set_offsetcolor(sprite, r, g, b, a) {
    if (r >= 0) sprite.offsetcolor[0] = r;
    if (g >= 0) sprite.offsetcolor[1] = g;
    if (b >= 0) sprite.offsetcolor[2] = b;
    if (a >= 0) sprite.offsetcolor[3] = a;
}


function sprite_external_animation_set(sprite, animsprite) {
    let old_animation = sprite.animation_external;
    sprite.animation_external = animsprite;
    return old_animation;
}

function sprite_external_animation_restart(sprite) {
    if (!sprite.animation_external) return;
    animsprite_restart(sprite.animation_external);
    animsprite_update_sprite(sprite.animation_external, sprite, 0);
}

function sprite_external_animation_end(sprite) {
    if (sprite.animation_external)
        animsprite_force_end2(sprite.animation_external, sprite);
}


function sprite_resize_draw_size(sprite, max_width, max_height, applied_draw_size) {
    let orig_size = [0, 0];
    sprite_get_source_size(sprite, orig_size);

    let width, height;

    if (max_width < 1 && max_height < 1) {
        width = orig_size[0];
        height = orig_size[1];
    } else if (orig_size[0] == 0 && orig_size[1] == 0) {
        width = max_width < 0 ? max_height : max_width;
        height = max_height < 0 ? max_width : max_height;
    } else if (max_width == 0 || max_height == 0) {
        width = height = 0;
    } else {
        if (max_width > 0 && max_height > 0) {
            let scale_x = max_width / orig_size[0];
            let scale_y = max_height / orig_size[1];

            if (scale_x > scale_y)
                max_width = -Infinity;
            else
                max_height = -Infinity;
        }

        if (max_height > 0) {
            height = max_height;
            width = (orig_size[0] * max_height) / orig_size[1];
        }

        if (max_width > 0) {
            height = (orig_size[1] * max_width) / orig_size[0];
            width = max_width;
        }
    }

    sprite.draw_width = width;
    sprite.draw_height = height;
    sprite.vertex_dirty = 1;

    if (applied_draw_size) {
        applied_draw_size[0] = width;
        applied_draw_size[1] = height;
    }

    return applied_draw_size;
}

function sprite_center_draw_location(sprite, x, y, ref_width, ref_height, applied_draw_location) {
    let draw_size = [0, 0];
    sprite_get_draw_size(sprite, draw_size);

    if (ref_width >= 0) x += ((ref_width - draw_size[0]) / 2);
    if (ref_height >= 0) y += ((ref_height - draw_size[1]) / 2);

    sprite.draw_x = x;
    sprite.draw_y = y;
    sprite.vertex_dirty = 1;

    if (applied_draw_location) {
        applied_draw_location[0] = sprite.draw_x;
        applied_draw_location[1] = sprite.draw_y;
    }

    return applied_draw_location;
}


function sprite_is_textured(sprite) {
    return sprite.texture ? 1 : 0;
}

function sprite_is_visible(sprite) {
    return sprite.visible;
}


function sprite_matrix_rotate_pivot_enable(sprite, enable) {
    sprite.matrix_source.rotate_pivot_enabled = enable;
}

function sprite_matrix_rotate_pivot(sprite, u, v) {
    if (u != null) sprite.matrix_source.rotate_pivot_u = u;
    if (v != null) sprite.matrix_source.rotate_pivot_v = v;
}

function sprite_matrix_scale_offset(sprite, direction_x, direction_y) {
    if (direction_x != null) sprite.matrix_source.scale_direction_x = direction_x;
    if (direction_y != null) sprite.matrix_source.scale_direction_y = direction_y;
}

function sprite_matrix_translate_rotation(sprite, enable_translate_rotation) {
    sprite.matrix_source.translate_rotation = enable_translate_rotation;
}

function sprite_matrix_scale_size(sprite, enable_scale_size) {
    sprite.matrix_source.scale_size = enable_scale_size;
}

function sprite_matrix_corner_rotation(sprite, corner) {
    sh4matrix_helper_calculate_corner_rotation(sprite.matrix_corner, corner);
}

function sprite_set_draw_x(sprite, value) {
    sprite.draw_x = value;
    sprite.vertex_dirty = 1;
}

function sprite_set_draw_y(sprite, value) {
    sprite.draw_y = value;
    sprite.vertex_dirty = 1;
}


function sprite_crop(sprite, dx, dy, dwidth, dheight) {
    if (Number.isFinite(dx)) sprite.crop.x = dx;
    if (Number.isFinite(dy)) sprite.crop.y = dy;
    if (Number.isFinite(dwidth)) sprite.crop.width = dwidth;
    if (Number.isFinite(dheight)) sprite.crop.height = dheight;

    let invalid = sprite.crop.x < 0 && sprite.crop.y < 0 && sprite.crop.width == 0 && sprite.crop.height == 0;

    // disable cropping if the bounds are invalid
    if (sprite.crop_enabled && invalid) sprite.crop_enabled = 0;

    sprite.vertex_dirty = 1;
    return !invalid;
}

function sprite_is_crop_enabled(sprite) {
    return sprite.crop_enabled;
}

function sprite_crop_enable(sprite, enable) {
    sprite.crop_enabled = enable;
}


function sprite_fill_atlas_entry(sprite, target_atlas_entry) {
    target_atlas_entry.name = null;
    target_atlas_entry.x = sprite.src_x;
    target_atlas_entry.y = sprite.src_y;
    target_atlas_entry.width = sprite.src_width;
    target_atlas_entry.height = sprite.src_height;
    target_atlas_entry.frame_x = sprite.frame_x;
    target_atlas_entry.frame_y = sprite.frame_y;
    target_atlas_entry.frame_width = sprite.frame_width;
    target_atlas_entry.frame_height = sprite.frame_height;
    target_atlas_entry.pivot_x = sprite.pivot_x;
    target_atlas_entry.pivot_y = sprite.pivot_y;
}

function sprite_get_texture(sprite) {
    return sprite.texture;
}

function sprite_set_antialiasing(sprite, antialiasing) {
    sprite.antialiasing = antialiasing;
}

function sprite_set_shader(sprite, psshader) {
    sprite.psshader = psshader;
}

function sprite_get_shader(sprite) {
    return sprite.psshader;
}

function sprite_blend_enable(sprite, enabled) {
    sprite.blend_enabled = enabled;
}

function sprite_blend_set(sprite, src_rgb, dst_rgb, src_alpha, dst_alpha) {
    sprite.blend_src_rgb = src_rgb;
    sprite.blend_dst_rgb = dst_rgb;
    sprite.blend_src_alpha = src_alpha;
    sprite.blend_dst_alpha = dst_alpha;
}

function sprite_trailing_enabled(sprite, enabled) {
    sprite.trailing_disabled = !enabled;
    if (sprite.trailing_disabled) sprite.trailing_used = 0;
}

function sprite_trailing_set_params(sprite, length, trail_delay, trail_alpha, darken_colors) {
    if (length > 0) {
        sprite.trailing_buffer = realloc(sprite.trailing_buffer, length);
        console.assert(sprite.trailing_buffer, "cannot reallocate the sprite trailing buffer");

        sprite_internal_JS_init_trail_array(sprite, sprite.trailing_length, length);

        sprite.trailing_length = length;
        if (sprite.trailing_used > length) sprite.trailing_used = length;
    }
    if (darken_colors != null) sprite.trailing_darken = darken_colors ? 1 : 0;
    if (!Number.isNaN(trail_delay)) sprite.trailing_delay = trail_delay;
    if (!Number.isNaN(trail_alpha)) sprite.trailing_alpha = trail_alpha;

    // force update
    sprite.trailing_progress = sprite.trailing_delay;
}

function sprite_trailing_set_offsetcolor(sprite, r, g, b) {
    if (!Number.isNaN(r)) sprite.trailing_offsetcolor[0] = r;
    if (!Number.isNaN(g)) sprite.trailing_offsetcolor[1] = g;
    if (!Number.isNaN(b)) sprite.trailing_offsetcolor[2] = b;
}

function sprite_internal_JS_init_trail_array(sprite, index, length) {
    // (JS only) initialize added trailings
    for (let i = index; i < length; i++) {
        sprite.trailing_buffer[i] = {
            sx: 0, sy: 0, sw: 0, sh: 0, dx: 0, dy: 0, dw: 0, dh: 0, r: 0, g: 0, b: 0, alpha: 0
        };
    }
}

