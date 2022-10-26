"use strict";

var STATESPRITE_IDS = 0;
const STATESPRITE_POOL = new Map();


function statesprite_draw(statesprite, pvrctx) {
    if (statesprite.draw_width < 1 || statesprite.draw_height < 1) return;
    if (statesprite.alpha <= 0) return;

    let sprite_vertex = statesprite.vertex;

    // apply offsets from the current state
    let draw_x = statesprite.draw_x + statesprite.offset_x;
    let draw_y = statesprite.draw_y + statesprite.offset_y;

    if (statesprite.vertex_dirty) {
        if (statesprite.texture) {
            let frame_width, frame_height;
            let crop_x, crop_y, crop_width, crop_height;
            let ratio_width, ratio_height;

            // complex frame size redimension
            if (statesprite.frame_width > 0) {
                ratio_width = statesprite.draw_width / statesprite.frame_width;
                frame_width = statesprite.src_width * ratio_width;
            } else {
                ratio_width = statesprite.draw_width / statesprite.src_width;
                frame_width = statesprite.draw_width;
            }
            if (statesprite.frame_height > 0) {
                ratio_height = statesprite.draw_height / statesprite.frame_height;
                frame_height = statesprite.src_height * ratio_height;
            } else {
                ratio_height = statesprite.draw_height / statesprite.src_height;
                frame_height = statesprite.draw_height;
            }

            // calculate cropping (if required)
            if (statesprite.crop_enabled) {
                if (statesprite.crop.x >= frame_width || statesprite.crop.y >= frame_height)
                    return;

                crop_x = statesprite.crop.x;
                crop_y = statesprite.crop.y;
                crop_width = crop_height = 0;

                if (statesprite.crop.width != -1 && statesprite.crop.width < frame_width)
                    crop_width = frame_width - statesprite.crop.width;

                if (statesprite.crop.height != -1 && statesprite.crop.height < frame_height)
                    crop_height = frame_height - statesprite.crop.height;
            } else {
                crop_x = crop_y = crop_width = crop_height = 0;
            }

            // draw location & size
            sprite_vertex[4] = draw_x - ((statesprite.pivot_x + statesprite.frame_x) * ratio_width);
            sprite_vertex[5] = draw_y - ((statesprite.pivot_y + statesprite.frame_y) * ratio_height);
            sprite_vertex[6] = frame_width - crop_width - crop_x;
            sprite_vertex[7] = frame_height - crop_height - crop_y;

            if (statesprite.crop_enabled) {
                crop_x /= ratio_width;
                crop_y /= ratio_height;
                crop_width /= ratio_width;
                crop_height /= ratio_height;
            }

            // source location & size
            sprite_vertex[0] = statesprite.src_x + crop_x;
            sprite_vertex[1] = statesprite.src_y + crop_y;
            sprite_vertex[2] = statesprite.src_width - crop_width - crop_x;
            sprite_vertex[3] = statesprite.src_height - crop_height - crop_y;

        } else {
            sprite_vertex[4] = draw_x;
            sprite_vertex[5] = draw_y;
            sprite_vertex[6] = statesprite.draw_width;
            sprite_vertex[7] = statesprite.draw_height;

            if (statesprite.crop_enabled) {
                let crop_width = statesprite.crop.width;
                let crop_height = statesprite.crop.height;

                sprite_vertex[4] += statesprite.crop.x;
                sprite_vertex[5] += statesprite.crop.y;
                sprite_vertex[6] -= statesprite.crop.x;
                sprite_vertex[7] -= statesprite.crop.y;

                if (crop_width != -1 && crop_width < sprite_vertex[6])
                    sprite_vertex[6] = crop_width;
                if (crop_height != -1 && crop_height < sprite_vertex[7])
                    sprite_vertex[7] = crop_height;
            }
        }

        if (statesprite.flip_x) {
            if (statesprite.flip_correction) sprite_vertex[4] += sprite_vertex[6];
            sprite_vertex[6] = -sprite_vertex[6];
        }
        if (statesprite.flip_y) {
            if (statesprite.flip_correction) sprite_vertex[5] += sprite_vertex[7];
            sprite_vertex[7] = -sprite_vertex[7];
        }

        // cache the calculated vertex
        statesprite.vertex_dirty = 0;
    }

    pvr_context_save(pvrctx);
    pvr_context_set_vertex_alpha(pvrctx, statesprite.alpha);
    pvr_context_set_vertex_offsetcolor(pvrctx, statesprite.offsetcolor);

    // apply self modifier
    pvr_context_apply_modifier2(pvrctx,
        statesprite.matrix_source,
        draw_x, draw_y, statesprite.draw_width, statesprite.draw_height
    );

    if (statesprite.antialiasing != PVR_FLAG_DEFAULT) {
        pvr_context_set_vertex_antialiasing(pvrctx, statesprite.antialiasing);
    }

    // do corner rotation (if required)
    if (statesprite.matrix_corner.angle != 0) {
        sh4matrix_corner_rotate(
            pvrctx.current_matrix, statesprite.matrix_corner,
            draw_x, draw_y, statesprite.draw_width, statesprite.draw_height
        );
        pvr_context_flush(pvrctx);
    }

    // draw the vertex
    if (statesprite.texture) {

        // upload texture to the PVR VRAM, if was not loaded yet
        texture_upload_to_pvr(statesprite.texture);

        // textured vertex
        pvr_context_draw_texture(
            pvrctx,
            statesprite.texture,
            sprite_vertex[0], sprite_vertex[1], // source location
            sprite_vertex[2], sprite_vertex[3], // source size

            sprite_vertex[4], sprite_vertex[5], // draw location
            sprite_vertex[6], sprite_vertex[7], // draw size
        );
    } else {
        // un-textured vertex
        pvr_context_draw_solid_color(
            pvrctx, statesprite.vertex_color,
            sprite_vertex[4], sprite_vertex[5],// draw location
            sprite_vertex[6], sprite_vertex[7]// draw size
        );
    }

    pvr_context_restore(pvrctx);
}


function statesprite_init_from_vertex_color(rgb8_color) {
    let statesprite = statesprite_init_from_texture(null);
    statesprite_set_vertex_color_rgb8(statesprite, rgb8_color);
    return statesprite;
}

function statesprite_init_from_texture(texture) {
    let statesprite = {};

    statesprite.matrix_source = {};
    pvrctx_helper_clear_modifier(statesprite.matrix_source);

    statesprite.src_x = 0;
    statesprite.src_y = 0;

    statesprite.vertex_color = [1.0, 1.0, 1.0];

    statesprite.offsetcolor = [];
    pvrctx_helper_clear_offsetcolor(statesprite.offsetcolor);

    statesprite.texture = texture;
    if (texture) {
        statesprite.src_width = statesprite.texture.original_width;
        statesprite.src_height = statesprite.texture.original_height;
    } else {
        statesprite.src_width = 0;
        statesprite.src_height = 0;
    }

    statesprite.draw_x = 100;
    statesprite.draw_y = 100;
    statesprite.draw_width = -1;
    statesprite.draw_height = -1;

    statesprite.alpha = 1.0;
    statesprite.visible = 1;
    statesprite.z_index = 1;
    statesprite.z_offset = 0;

    statesprite.frame_x = 0;
    statesprite.frame_y = 0;
    statesprite.frame_width = 0;
    statesprite.frame_height = 0;
    statesprite.pivot_x = 0;
    statesprite.pivot_y = 0;

    statesprite.id = SPRITE_IDS++;
    STATESPRITE_POOL.set(statesprite.id, statesprite);

    statesprite.vertex = new Array(8);
    statesprite.vertex_dirty = 1;

    statesprite.animation_external = null;

    statesprite.matrix_corner = { x: 0, y: 0, angle: 0 };

    statesprite.crop = { x: 0, y: 0, width: -1, height: -1 };
    statesprite.crop_enabled = 0;

    statesprite.flip_x = 0;
    statesprite.flip_y = 0;
    statesprite.flip_correction = 1;

    statesprite.state_list = linkedlist_init();
    statesprite.offset_x = 0;
    statesprite.offset_y = 0;
    statesprite.selected_state = null;

    statesprite.scale_texture = 1.0;
    statesprite.antialiasing = PVR_FLAG_DEFAULT;
    statesprite.atlas_to_draw_size_enabled = 0;

    return statesprite;
}

function statesprite_destroy_texture_if_stateless(statesprite) {
    if (!statesprite.texture) return 0;

    // if the texture belongs to any state, return
    // it will fail in case of duplicated refereces (is a caller problem)
    for (let state of linkedlist_iterate4(statesprite.state_list)) {
        if (state.texture && state.texture == statesprite.texture) return 0;
    }

    texture_destroy(statesprite.texture);
    statesprite.texture = null;
    return 1;
}

function statesprite_destroy(statesprite) {
    for (let state of linkedlist_iterate4(statesprite.state_list)) {
        if (state.animation) animsprite_destroy(state.animation);
        if (state.texture) texture_destroy(state.texture);
        state.state_name = undefined;
        state.frame_info = undefined;
        state = undefined;
    }

    linkedlist_destroy2(statesprite.state_list, free);

    STATESPRITE_POOL.delete(statesprite.id);
    statesprite = undefined;
}


function statesprite_set_texture(statesprite, texture, update_offset_source_size) {
    let old_texture = statesprite.texture;
    statesprite.texture = texture;

    if (update_offset_source_size && texture) {
        statesprite.src_width = statesprite.texture.original_width;
        statesprite.src_height = statesprite.texture.original_height;
        statesprite.vertex_dirty = 1;
    }

    return old_texture;
}

function statesprite_set_offset_source(statesprite, x, y, width, height) {
    statesprite.src_x = x;
    statesprite.src_y = y;
    statesprite.src_width = width;
    statesprite.src_height = height;

    if (statesprite.atlas_to_draw_size_enabled) {
        statesprite.draw_width = width * statesprite.scale_texture;
        statesprite.draw_height = height * statesprite.scale_texture;
    } else {
        if (statesprite.draw_width < 0) statesprite.draw_width = width * statesprite.scale_texture;
        if (statesprite.draw_height < 0) statesprite.draw_height = height * statesprite.scale_texture;
    }

    statesprite.vertex_dirty = 1;
}

function statesprite_set_offset_frame(statesprite, x, y, width, height) {
    statesprite.frame_x = x;
    statesprite.frame_y = y;
    statesprite.frame_width = width;
    statesprite.frame_height = height;

    statesprite.vertex_dirty = 1;

    if (statesprite.atlas_to_draw_size_enabled) {
        if (width > 0) statesprite.draw_width = width * statesprite.scale_texture;
        if (height > 0) statesprite.draw_height = height * statesprite.scale_texture;
    } else {
        if (statesprite.draw_width < 0 && width > 0)
            statesprite.draw_width = width * statesprite.scale_texture;
        if (statesprite.draw_height < 0 && height > 0)
            statesprite.draw_height = height * statesprite.scale_texture;
    }
}

function statesprite_set_offset_pivot(statesprite, x, y) {
    statesprite.pivot_x = x;
    statesprite.pivot_y = y;
    statesprite.vertex_dirty = 1;
}


function statesprite_set_draw_location(statesprite, x, y) {
    statesprite.draw_x = x;
    statesprite.draw_y = y;
    statesprite.vertex_dirty = 1;
}

function statesprite_set_draw_size(statesprite, width, height) {
    statesprite.draw_width = width;
    statesprite.draw_height = height;
    statesprite.vertex_dirty = 1;
}

function statesprite_set_alpha(statesprite, alpha) {
    statesprite.alpha = alpha;
}

function statesprite_set_visible(statesprite, visible) {
    statesprite.visible = visible;
}

function statesprite_set_z_index(statesprite, index) {
    statesprite.z_index = index;
}

function statesprite_set_z_offset(statesprite, offset) {
    statesprite.z_offset = offset;
}

function statesprite_set_property(statesprite, property_id, value) {
    switch (property_id) {
        case SPRITE_PROP_X:
            statesprite.draw_x = value;
            statesprite.vertex_dirty = 1;
            break;
        case SPRITE_PROP_Y:
            statesprite.draw_y = value;
            statesprite.vertex_dirty = 1;
            break;
        case SPRITE_PROP_WIDTH:
            statesprite.draw_width = value;
            statesprite.vertex_dirty = 1;
            break;
        case SPRITE_PROP_HEIGHT:
            statesprite.draw_height = value;
            statesprite.vertex_dirty = 1;
            break;
        case SPRITE_PROP_ROTATE:
            statesprite_matrix_rotate(statesprite, value);
            break;
        case SPRITE_PROP_SCALE_X:
            statesprite_matrix_scale(statesprite, value, null);
            break;
        case SPRITE_PROP_SCALE_Y:
            statesprite_matrix_scale(statesprite, null, value);
            break;
        case SPRITE_PROP_SKEW_X:
            statesprite_matrix_skew(statesprite, value, null);
            break;
        case SPRITE_PROP_SKEW_Y:
            statesprite_matrix_skew(statesprite, null, value);
            break;
        case SPRITE_PROP_TRANSLATE_X:
            statesprite_matrix_translate(statesprite, value, null);
            break;
        case SPRITE_PROP_TRANSLATE_Y:
            statesprite_matrix_translate(statesprite, null, value);
            break;
        case SPRITE_PROP_ALPHA:
            statesprite.alpha = math2d_clamp(value, 0, 1.0);
            break;
        case SPRITE_PROP_Z:
            statesprite.z_index = value;
            break;
        case SPRITE_PROP_VERTEX_COLOR_R:
            statesprite.vertex_color[0] = value;
            break;
        case SPRITE_PROP_VERTEX_COLOR_G:
            statesprite.vertex_color[1] = value;
            break;
        case SPRITE_PROP_VERTEX_COLOR_B:
            statesprite.vertex_color[2] = value;
            break;
        case SPRITE_PROP_VERTEX_COLOR_OFFSET_R:
            statesprite.offsetcolor[0] = value;
            break;
        case SPRITE_PROP_VERTEX_COLOR_OFFSET_G:
            statesprite.offsetcolor[1] = value;
            break;
        case SPRITE_PROP_VERTEX_COLOR_OFFSET_B:
            statesprite.offsetcolor[2] = value;
            break;
        case SPRITE_PROP_VERTEX_COLOR_OFFSET_A:
            statesprite.offsetcolor[3] = value;
            break;
        case SPRITE_PROP_ANIMATIONLOOP:
            console.log("statesprite_set_property() SPRITE_PROP_ANIMATIONLOOP is not supported");
            break;
        case SPRITE_PROP_ROTATE_PIVOT_ENABLE:
            statesprite_matrix_rotate_pivot_enable(statesprite, value >= 1.0);
            break;
        case SPRITE_PROP_ROTATE_PIVOT_U:
            statesprite.matrix_source.rotate_pivot_u = value;
            break;
        case SPRITE_PROP_ROTATE_PIVOT_V:
            statesprite.matrix_source.rotate_pivot_u = value;
            break;
        case SPRITE_PROP_SCALE_DIRECTION_X:
            statesprite.matrix_source.scale_direction_x = value;
            break;
        case SPRITE_PROP_SCALE_DIRECTION_Y:
            statesprite.matrix_source.scale_direction_y = value;
            break;
        case SPRITE_PROP_TRANSLATE_ROTATION:
            statesprite.matrix_source.translate_rotation = value >= 1.0;
            break;
        case SPRITE_PROP_SCALE_SIZE:
            statesprite.matrix_source.scale_size = value >= 1.0;
            break;
        case SPRITE_PROP_SCALE_TRANSLATION:
            statesprite.matrix_source.scale_translation = value >= 1.0;
            break;
        case SPRITE_PROP_Z_OFFSET:
            statesprite.z_offset = value;
            break;
        case SPRITE_PROP_ANTIALIASING:
            statesprite.antialiasing = Math.trunc(value);
            break;
        case SPRITE_PROP_FLIP_X:
            statesprite.flip_x = value >= 1.0;
            statesprite.vertex_dirty = true;
            break;
        case SPRITE_PROP_FLIP_Y:
            statesprite.flip_y = value >= 1.0;
            statesprite.vertex_dirty = true;
            break;
        case SPRITE_PROP_FLIP_CORRECTION:
            statesprite.flip_correction = value >= 1.0;
            statesprite.vertex_dirty = true;
            break;
    }
}

function statesprite_set_offsetcolor(statesprite, r, g, b, a) {
    if (r >= 0) statesprite.offsetcolor[0] = r;
    if (g >= 0) statesprite.offsetcolor[1] = g;
    if (b >= 0) statesprite.offsetcolor[2] = b;
    if (a >= 0) statesprite.offsetcolor[3] = a;
}

function statesprite_set_vertex_color(statesprite, r, g, b) {
    if (r >= 0) statesprite.vertex_color[0] = r;
    if (g >= 0) statesprite.vertex_color[1] = g;
    if (b >= 0) statesprite.vertex_color[2] = b;
}

function statesprite_set_vertex_color_rgb8(statesprite, rbg8_color) {
    math2d_color_bytes_to_floats(rbg8_color, 0, statesprite.vertex_color);
}


function statesprite_animation_set(statesprite, animsprite) {
    let old_animation = statesprite.animation_external;
    statesprite.animation_external = animsprite;
    return old_animation;
}

function statesprite_animation_restart(statesprite) {
    if (statesprite.animation_external) {
        animsprite_restart(statesprite.animation_external);
        animsprite_update_statesprite(statesprite.animation_external, statesprite, 1);
    }
}

function statesprite_animation_end(statesprite) {
    if (statesprite.animation_external)
        animsprite_force_end3(statesprite.animation_external, statesprite);
}

function statesprite_animation_completed(statesprite) {
    return !statesprite.animation_external || animsprite_is_completed(statesprite.animation_external);
}

function statesprite_animate(statesprite, elapsed) {
    //if (!Number.isFinite(elapsed)) throw new Error("Invalid elapsed value:" + elapsed);

    if (!statesprite.animation_external) return 1;

    let result = animsprite_animate(statesprite.animation_external, elapsed);
    animsprite_update_statesprite(statesprite.animation_external, statesprite, 1);

    return result;
}


function statesprite_get_z_index(statesprite) {
    return statesprite.z_index + statesprite.z_offset;
}

function statesprite_get_modifier(statesprite) {
    return statesprite.matrix_source;
}

function statesprite_get_draw_size(statesprite, size) {
    size[0] = statesprite.draw_width >= 0 ? statesprite.draw_width : statesprite.src_width;
    size[1] = statesprite.draw_height >= 0 ? statesprite.draw_height : statesprite.src_height;
    return size;
}

function statesprite_get_draw_location(statesprite, location) {
    location[0] = statesprite.draw_x;
    location[1] = statesprite.draw_y;
    return location;
}

function statesprite_get_alpha(statesprite) {
    return statesprite.alpha;
}

function statesprite_get_offset_source_size(statesprite, size) {
    size[0] = statesprite.frame_width > 0 ? statesprite.frame_width : statesprite.src_width;
    size[1] = statesprite.frame_height > 0 ? statesprite.frame_height : statesprite.src_height;
    return size;
}

function statesprite_is_visible(statesprite) {
    return statesprite.visible;
}


function statesprite_matrix_reset(statesprite) {
    pvrctx_helper_clear_modifier(statesprite.matrix_source);
    statesprite.flip_x = statesprite.flip_y = 0;
    statesprite.matrix_corner = { x: 0, y: 0, angle: 0 };
}

function statesprite_matrix_scale(statesprite, scale_x, scale_y) {
    if (scale_x != null) statesprite.matrix_source.scale_x = scale_x;
    if (scale_y != null) statesprite.matrix_source.scale_y = scale_y;
}

function statesprite_matrix_rotate(statesprite, degrees_angle) {
    statesprite.matrix_source.rotate = degrees_angle * MATH2D_DEG_TO_RAD;
}

function statesprite_matrix_skew(statesprite, skew_x, skew_y) {
    if (skew_x != null) statesprite.matrix_source.skew_x = skew_x;
    if (skew_y != null) statesprite.matrix_source.skew_y = skew_y;
}

function statesprite_matrix_translate(statesprite, translate_x, translate_y) {
    if (translate_x != null) statesprite.matrix_source.translate_x = translate_x;
    if (translate_y != null) statesprite.matrix_source.translate_y = translate_y;
}

function statesprite_matrix_rotate_pivot_enable(statesprite, enable) {
    statesprite.matrix_source.rotate_pivot_enabled = enable;
}

function statesprite_matrix_rotate_pivot(statesprite, u, v) {
    if (u != null) statesprite.matrix_source.rotate_pivot_u = u;
    if (v != null) statesprite.matrix_source.rotate_pivot_u = v;
}

function statesprite_matrix_scale_offset(statesprite, direction_x, direction_y) {
    if (direction_x != null) statesprite.matrix_source.scale_direction_x = direction_x;
    if (direction_y != null) statesprite.matrix_source.scale_direction_y = direction_y;
}

function statesprite_matrix_translate_rotation(statesprite, enable_translate_rotation) {
    statesprite.matrix_source.translate_rotation = enable_translate_rotation;
}

function statesprite_matrix_scale_size(statesprite, enable_scale_size) {
    statesprite.matrix_source.scale_size = enable_scale_size;
}


function statesprite_state_add(statesprite, modelholder, animation_name, state_name) {
    if (!modelholder) return null;
    let atlas_entry = modelholder_get_atlas_entry2(modelholder, animation_name, 0);
    let animsprite = modelholder_create_animsprite(modelholder, animation_name, 0, 0);
    let texture = modelholder_get_texture(modelholder, 0);
    let vertex_color = modelholder_get_vertex_color(modelholder);

    if (!atlas_entry && animsprite) {
        atlas_entry = animsprite_helper_get_first_frame_atlas_entry(animsprite);
    }

    let statesprite_state = statesprite_state_add2(
        statesprite, texture, animsprite, atlas_entry, vertex_color, state_name
    );

    if (!statesprite_state && animsprite) animsprite_destroy(animsprite);

    return statesprite_state;
}

function statesprite_state_add2(statesprite, texture, animsprt, atlas_entry, rgb8_color, state_name) {
    if (texture && !atlas_entry && !animsprt) return null;

    // in C clone the string object
    if (state_name != null)
        state_name = strdup(state_name);

    let corner_x = statesprite.matrix_corner.x;
    let corner_y = statesprite.matrix_corner.y;
    let corner_angle = statesprite.matrix_corner.angle;

    if (!texture && atlas_entry) {
        console.warn("statesprite_state_add2() atlas_entry specified but the texture was null");
        atlas_entry = null;
    }

    if (texture) texture = texture_share_reference(texture);

    let state = {
        statesprite_id: statesprite.id,
        state_name: state_name,
        texture: texture,
        animation: animsprt,
        vertex_color: rgb8_color,
        frame_info: null,
        corner_rotation: { x: corner_x, y: corner_y, angle: corner_angle },
        offset_x: statesprite.offset_x, offset_y: statesprite.offset_y,
        draw_width: statesprite.draw_width, draw_height: statesprite.draw_height
    };

    if (atlas_entry) {
        state.frame_info = {};
        clone_struct_to(atlas_entry, state.frame_info);
    }

    linkedlist_add_item(statesprite.state_list, state);

    return state;
}

function statesprite_state_toggle(statesprite, state_name) {
    if (statesprite.selected_state && statesprite.selected_state.state_name == state_name)
        return 1;

    let state = null;

    for (let item of linkedlist_iterate4(statesprite.state_list)) {
        if (item.state_name == state_name) {
            state = item;
            break;
        }
    }

    if (!state) return 0;

    statesprite_state_apply(statesprite, state);

    return 1;
}

function statesprite_state_list(statesprite) {
    return statesprite.state_list;
}

function statesprite_state_name(statesprite) {
    if (statesprite.selected_state) return statesprite.selected_state.state_name;
    return null;
}

function statesprite_state_has(statesprite, state_name) {
    for (let state of linkedlist_iterate4(statesprite.state_list)) {
        if (state.state_name == state_name) return 1;
    }
    return 0;
}

function statesprite_state_get(statesprite) {
    return statesprite.selected_state;
}

function statesprite_state_apply(statesprite, statesprite_state) {
    if (!statesprite_state && !statesprite.selected_state) return 0;

    if (statesprite_state && statesprite_state.statesprite_id != statesprite.id) {
        console.warn("statesprite_state_apply() attempt to apply a foreign state");
        return 0;
    }

    let state = statesprite_state ? statesprite_state : statesprite.selected_state;

    if (state.frame_info) {
        statesprite_set_offset_source(
            statesprite,
            state.frame_info.x, state.frame_info.y,
            state.frame_info.width, state.frame_info.height
        );
        statesprite_set_offset_frame(
            statesprite,
            state.frame_info.frame_x, state.frame_info.frame_y,
            state.frame_info.frame_width, state.frame_info.frame_height
        );
        statesprite_set_offset_pivot(
            statesprite,
            state.frame_info.pivot_x,
            state.frame_info.pivot_y
        );
    } else {
        statesprite_set_offset_pivot(
            statesprite,
            0,
            0
        );
    }

    statesprite.texture = state.texture;
    statesprite.animation_external = state.animation;
    statesprite_set_vertex_color_rgb8(statesprite, state.vertex_color);
    statesprite.offset_x = state.offset_x;
    statesprite.offset_y = state.offset_y;
    statesprite.draw_width = state.draw_width;
    statesprite.draw_height = state.draw_height;
    statesprite.matrix_corner.x = state.corner_rotation.x;
    statesprite.matrix_corner.y = state.corner_rotation.y;
    statesprite.matrix_corner.angle = state.corner_rotation.angle;

    statesprite.selected_state = state;
    statesprite.vertex_dirty = 1;
    return 1;
}

function statesprite_state_get_offsets(statesprite, output_offsets) {
    output_offsets[0] = statesprite.offset_x;
    output_offsets[1] = statesprite.offset_y;
    return output_offsets;
}

function statesprite_state_set_offsets(statesprite, offset_x, offset_y, update_state) {
    statesprite.offset_x = offset_x;
    statesprite.offset_y = offset_y;

    if (update_state && statesprite.selected_state) {
        statesprite.selected_state.offset_x = offset_x;
        statesprite.selected_state.offset_y = offset_y;
    }

    statesprite.vertex_dirty = 1;
}

function statesprite_state_remove(statesprite, state_name) {
    let i = 0;
    linkedlist_iterator_prepare(statesprite.state_list);
    for (let state of linkedlist_iterate4(statesprite.state_list)) {
        if (state.state_name == state_name) {
            if (statesprite.animation_external == state.animation) statesprite.animation_external = null;
            if (state.texture) texture_destroy(state.texture);
            if (state.animation) animsprite_destroy(state.animation);
            if (state == statesprite.selected_state) statesprite.selected_state = null;

            linkedlist_remove_item_at(statesprite.state_list, i);

            state.state_name = undefined;
            state.frame_info = undefined;
            state = undefined;
            return;
        }
        i++;
    }
}


function statesprite_resize_draw_size(statesprite, max_width, max_height, applied_size) {
    let orig_width = statesprite.frame_width > 0 ? statesprite.frame_width : statesprite.src_width;
    let orig_height = statesprite.frame_height > 0 ? statesprite.frame_height : statesprite.src_height;

    let width, height;

    if (max_width < 1 && max_height < 1) {
        width = orig_width;
        height = orig_height;
    } else if (orig_width == 0 && orig_height == 0) {
        width = max_width < 0 ? max_height : max_width;
        height = max_height < 0 ? max_width : max_height;
    } else if (max_width == 0 || max_height == 0) {
        width = height = 0;
    } else {
        if (max_width > 0 && max_height > 0) {
            let scale_x = max_width / orig_width;
            let scale_y = max_height / orig_height;

            if (scale_x > scale_y)
                max_width = -Infinity;
            else
                max_height = -Infinity;
        }

        if (max_height > 0) {
            height = max_height;
            width = (orig_width * max_height) / orig_height;
        }

        if (max_width > 0) {
            height = (orig_height * max_width) / orig_width;
            width = max_width;
        }
    }

    statesprite.draw_width = width;
    statesprite.draw_height = height;
    statesprite.vertex_dirty = 1;

    if (applied_size) {
        applied_size[0] = width;
        applied_size[1] = height;
    }

    return applied_size;
}

function statesprite_center_draw_location(statesprite, x, y, ref_width, ref_height, applied_location) {
    if (ref_width >= 0) x += ((ref_width - statesprite.draw_width) / 2);
    if (ref_height >= 0) y += ((ref_height - statesprite.draw_height) / 2);

    statesprite.draw_x = x;
    statesprite.draw_y = y;
    statesprite.vertex_dirty = 1;

    if (applied_location) {
        applied_location[0] = statesprite.draw_x;
        applied_location[1] = statesprite.draw_y;
    }

    return applied_location;
}

function statesprite_corner_rotate(statesprite, corner) {
    sh4matrix_helper_calculate_corner_rotation(statesprite.matrix_corner, corner);
}


function statesprite_crop(statesprite, dx, dy, dwidth, dheight) {
    if (dx != null) statesprite.crop.x = dx;
    if (dy != null) statesprite.crop.y = dy;
    if (dwidth != null) statesprite.crop.width = dwidth;
    if (dheight != null) statesprite.crop.height = dheight;

    let invalid = statesprite.crop.x < 0 &&
        statesprite.crop.y < 0 &&
        statesprite.crop.width == 0 &&
        statesprite.crop.height == 0;

    // disable cropping if the bounds are invalid
    if (statesprite.crop_enabled && invalid) statesprite.crop_enabled = 0;

    statesprite.vertex_dirty = 1;
    return !invalid;
}

function statesprite_is_crop_enabled(statesprite) {
    return statesprite.crop_enabled;
}

function statesprite_crop_enable(statesprite, enable) {
    statesprite.crop_enabled = enable;
}

function statesprite_flip_texture(statesprite, flip_x, flip_y) {
    if (flip_x != null) statesprite.flip_x = !!flip_x;
    if (flip_y != null) statesprite.flip_y = !!flip_y;
    statesprite.vertex_dirty = 1;
}

function statesprite_flip_texture_enable_correction(statesprite, enabled) {
    statesprite.flip_correction = !!enabled;
}

function statesprite_change_draw_size_in_atlas_apply(statesprite, enable, scale_factor) {
    if (!Number.isFinite(scale_factor)) throw new NaNArgumentError("invalid scale_factor");
    statesprite.scale_texture = scale_factor;
    statesprite.atlas_to_draw_size_enabled = !!enable;
}

function statesprite_set_antialiasing(statesprite, antialiasing) {
    statesprite.antialiasing = antialiasing;
}

