"use strict";

const CAMERA_DEFAULT_INTERPOLATOR = ANIM_MACRO_INTERPOLATOR_LINEAR;
const CAMERA_PX = 0;
const CAMERA_PY = 1;
const CAMERA_PZ = 2;
const CAMERA_OX = 3;
const CAMERA_OY = 4;
const CAMERA_OZ = 5;


function camera_init(modifier, viewport_width, viewport_height) {
    let camera = {
        tweenlerp: null,
        modifier: modifier,
        progress: 600,
        duration: 600,
        scene_zoom: 1,
        beat_duration: 600,
        internal_modifier: !modifier,
        transition_completed: 1,
        force_update: 0,
        has_transition_offset: 0,
        parallax_x: 0, parallax_y: 0, parallax_z: 1,
        offset_x: 0, offset_y: 0, offset_z: 1,
        parent_layout: null,
        animation: null
    };

    if (camera.internal_modifier) {
        // no modifier provided used the internal one
        camera.modifier = {};
        pvrctx_helper_clear_modifier(camera.modifier);
        camera.modifier.width = viewport_width;
        camera.modifier.width = viewport_height;
        camera.modifier.scale_direction_x = -0.5
        camera.modifier.scale_direction_y = -0.5
    }

    camera_set_interpolator_type(camera, CAMERA_DEFAULT_INTERPOLATOR);
    //camera_enable_scene_zoom(camera, 1);

    return camera;
}

function camera_destroy(camera) {
    tweenlerp_destroy(camera.tweenlerp);

    if (camera.internal_modifier) {
        ModuleLuaScript.kdmyEngine_drop_shared_object(camera.modifier);
        camera.modifier = undefined;
    }

    if (camera.animation) animsprite_destroy(camera.animation);

    ModuleLuaScript.kdmyEngine_drop_shared_object(camera);
    camera = undefined;
}


function camera_set_bpm(camera, beats_per_minute) {
    camera.beat_duration = math2d_beats_per_minute_to_beat_per_milliseconds(beats_per_minute);
}

function camera_set_interpolator_type(camera, type) {
    let px, py, pz, ox, oy, oz;

    if (camera.tweenlerp) {
        px = tweenlerp_peek_value_by_index(camera.tweenlerp, CAMERA_PX);
        py = tweenlerp_peek_value_by_index(camera.tweenlerp, CAMERA_PY);
        pz = tweenlerp_peek_value_by_index(camera.tweenlerp, CAMERA_PZ);
        ox = tweenlerp_peek_value_by_index(camera.tweenlerp, CAMERA_OX);
        oy = tweenlerp_peek_value_by_index(camera.tweenlerp, CAMERA_OY);
        oz = tweenlerp_peek_value_by_index(camera.tweenlerp, CAMERA_OZ);
        tweenlerp_destroy(camera.tweenlerp);
    } else {
        px = camera.parallax_x;
        py = camera.parallax_y;
        pz = camera.parallax_z;
        ox = camera.offset_x;
        oy = camera.offset_y;
        oz = camera.offset_z;
    }

    camera.tweenlerp = tweenlerp_init();

    tweenlerp_add_interpolator(camera.tweenlerp, -1, px, px, 0, type);
    tweenlerp_add_interpolator(camera.tweenlerp, -1, py, py, 0, type);
    tweenlerp_add_interpolator(camera.tweenlerp, -1, pz, pz, 0, type);
    tweenlerp_add_interpolator(camera.tweenlerp, -1, ox, ox, 0, type);
    tweenlerp_add_interpolator(camera.tweenlerp, -1, oy, oy, 0, type);
    tweenlerp_add_interpolator(camera.tweenlerp, -1, oz, oz, 0, type);
    tweenlerp_end(camera.tweenlerp);

    camera.progress = camera.duration;
    camera.transition_completed = 1;
}

function camera_set_transition_duration(camera, expresed_in_beats, value) {
    if (expresed_in_beats)
        camera.duration = value * camera.beat_duration;
    else
        camera.duration = value;
}

function camera_set_absolute_zoom(camera, z) {
    if (Number.isFinite(z)) camera_internal_tweenlerp_absolute(camera, CAMERA_PZ, z);
    camera.force_update = 1;
    camera_end(camera);
}

function camera_set_absolute_position(camera, x, y) {
    if (Number.isFinite(x)) camera_internal_tweenlerp_absolute(camera, CAMERA_PX, x);
    if (Number.isFinite(y)) camera_internal_tweenlerp_absolute(camera, CAMERA_PY, y);
    camera.force_update = 1;
    camera_end(camera);
}

function camera_set_absolute(camera, x, y, z) {
    camera_set_absolute_position(camera, x, y);
    camera_set_absolute_zoom(camera, z);
}


function camera_set_offset(camera, x, y, z) {
    if (Number.isFinite(x)) {
        camera_internal_tweenlerp_absolute(camera, CAMERA_OX, x);
        camera.offset_x = x;
    }
    if (Number.isFinite(y)) {
        camera_internal_tweenlerp_absolute(camera, CAMERA_OY, y);
        camera.offset_y = y;
    }
    if (Number.isFinite(z)) {
        camera_internal_tweenlerp_absolute(camera, CAMERA_OZ, z);
        camera.offset_z = z;
    }

    camera.has_transition_offset = 0;
}

function camera_get_offset(camera, xyz) {
    xyz[0] = camera.offset_x;
    xyz[1] = camera.offset_y;
    xyz[2] = camera.offset_z;
}


function camera_replace_modifier(camera, new_modifier) {
    if (!new_modifier) throw new Error("Invalid modifier");

    if (camera.internal_modifier) {
        camera.modifier = undefined;
        camera.internal_modifier = 0;
    }

    camera.modifier = new_modifier;
}

function camera_get_modifier(camera) {
    return camera.modifier;
}


function camera_move(camera, end_x, end_y, end_z) {
    camera_internal_drop_animation(camera);

    tweenlerp_change_bounds_by_index(camera.tweenlerp, CAMERA_PX, NaN, end_x);
    tweenlerp_change_bounds_by_index(camera.tweenlerp, CAMERA_PY, NaN, end_y);
    tweenlerp_change_bounds_by_index(camera.tweenlerp, CAMERA_PZ, NaN, end_z);

    camera_end(camera);
}

function camera_move_offset(camera, end_x, end_y, end_z) {
    camera_internal_drop_animation(camera);

    tweenlerp_change_bounds_by_index(camera.tweenlerp, CAMERA_OX, NaN, end_x);
    tweenlerp_change_bounds_by_index(camera.tweenlerp, CAMERA_OY, NaN, end_y);
    tweenlerp_change_bounds_by_index(camera.tweenlerp, CAMERA_OZ, NaN, end_z);

    camera.has_transition_offset = 1;
    camera_end(camera);
}

function camera_slide(camera, start_x, start_y, start_z, end_x, end_y, end_z) {
    camera_internal_drop_animation(camera);

    tweenlerp_change_bounds_by_index(camera.tweenlerp, CAMERA_PX, start_x, end_x);
    tweenlerp_change_bounds_by_index(camera.tweenlerp, CAMERA_PY, start_y, end_y);
    tweenlerp_change_bounds_by_index(camera.tweenlerp, CAMERA_PZ, start_z, end_z);

    camera_repeat(camera);
}

function camera_slide_offset(camera, start_x, start_y, start_z, end_x, end_y, end_z) {
    camera_internal_drop_animation(camera);

    tweenlerp_change_bounds_by_index(camera.tweenlerp, CAMERA_OX, start_x, end_x);
    tweenlerp_change_bounds_by_index(camera.tweenlerp, CAMERA_OY, start_y, end_y);
    tweenlerp_change_bounds_by_index(camera.tweenlerp, CAMERA_OZ, start_z, end_z);

    camera.has_transition_offset = 1;
    camera_repeat(camera);
}

function camera_slide_x(camera, start, end) {
    camera_internal_drop_animation(camera);

    tweenlerp_change_bounds_by_index(camera.tweenlerp, CAMERA_PX, start, end);
    tweenlerp_override_start_with_end_by_index(camera.tweenlerp, CAMERA_PY);
    tweenlerp_override_start_with_end_by_index(camera.tweenlerp, CAMERA_PZ);
    camera_repeat(camera);
}

function camera_slide_x_offset(camera, start, end) {
    camera_internal_drop_animation(camera);

    tweenlerp_change_bounds_by_index(camera.tweenlerp, CAMERA_OX, start, end);
    tweenlerp_override_start_with_end_by_index(camera.tweenlerp, CAMERA_OY);
    tweenlerp_override_start_with_end_by_index(camera.tweenlerp, CAMERA_OZ);
    camera.has_transition_offset = 1;
    camera_repeat(camera);
}

function camera_slide_y(camera, start, end) {
    camera_internal_drop_animation(camera);

    tweenlerp_override_start_with_end_by_index(camera.tweenlerp, CAMERA_PX);
    tweenlerp_change_bounds_by_index(camera.tweenlerp, CAMERA_PY, start, end);
    tweenlerp_override_start_with_end_by_index(camera.tweenlerp, CAMERA_PZ);
    camera_repeat(camera);
}

function camera_slide_y_offset(camera, start, end) {
    camera_internal_drop_animation(camera);

    tweenlerp_override_start_with_end_by_index(camera.tweenlerp, CAMERA_OX);
    tweenlerp_change_bounds_by_index(camera.tweenlerp, CAMERA_OY, start, end);
    tweenlerp_override_start_with_end_by_index(camera.tweenlerp, CAMERA_OZ);
    camera.has_transition_offset = 1;
    camera_repeat(camera);
}

function camera_slide_z(camera, start, end) {
    camera_internal_drop_animation(camera);

    tweenlerp_override_start_with_end_by_index(camera.tweenlerp, CAMERA_PX);
    tweenlerp_override_start_with_end_by_index(camera.tweenlerp, CAMERA_PY);
    tweenlerp_change_bounds_by_index(camera.tweenlerp, CAMERA_PZ, start, end);
    camera_repeat(camera);
}

function camera_slide_z_offset(camera, start, end) {
    camera_internal_drop_animation(camera);

    tweenlerp_override_start_with_end_by_index(camera.tweenlerp, CAMERA_OX);
    tweenlerp_override_start_with_end_by_index(camera.tweenlerp, CAMERA_OY);
    tweenlerp_change_bounds_by_index(camera.tweenlerp, CAMERA_OZ, start, end);
    camera_repeat(camera);
}

function camera_slide_to(camera, x, y, z) {
    camera_internal_drop_animation(camera);

    let start_x = tweenlerp_peek_value_by_index(camera.tweenlerp, CAMERA_PX);
    let start_y = tweenlerp_peek_value_by_index(camera.tweenlerp, CAMERA_PY);
    let start_z = tweenlerp_peek_value_by_index(camera.tweenlerp, CAMERA_PZ);

    if (!Number.isFinite(x)) x = start_x;
    if (!Number.isFinite(y)) y = start_y;
    if (!Number.isFinite(z)) z = start_z;

    tweenlerp_change_bounds_by_index(camera.tweenlerp, CAMERA_PX, start_x, x);
    tweenlerp_change_bounds_by_index(camera.tweenlerp, CAMERA_PY, start_y, y);
    tweenlerp_change_bounds_by_index(camera.tweenlerp, CAMERA_PZ, start_z, z);

    camera_repeat(camera);
}

function camera_slide_to_offset(camera, x, y, z) {
    camera_internal_drop_animation(camera);

    let start_x = tweenlerp_peek_value_by_index(camera.tweenlerp, CAMERA_OX);
    let start_y = tweenlerp_peek_value_by_index(camera.tweenlerp, CAMERA_OY);
    let start_z = tweenlerp_peek_value_by_index(camera.tweenlerp, CAMERA_OZ);

    if (!Number.isFinite(x)) x = start_x;
    if (!Number.isFinite(y)) y = start_y;
    if (!Number.isFinite(z)) z = start_z;

    tweenlerp_change_bounds_by_index(camera.tweenlerp, CAMERA_OX, start_x, x);
    tweenlerp_change_bounds_by_index(camera.tweenlerp, CAMERA_OY, start_y, y);
    tweenlerp_change_bounds_by_index(camera.tweenlerp, CAMERA_OZ, start_z, z);

    camera.has_transition_offset = 1;

    camera_repeat(camera);
}

function camera_from_layout(camera, layout, camera_name) {
    if (!layout) {
        if (!camera.parent_layout) return 0;
        layout = camera.parent_layout;
    }

    let camera_placeholder = layout_get_camera_placeholder(layout, camera_name);
    if (!camera_placeholder) return 0;

    if (camera_placeholder.animation) {
        camera_set_animation(camera, camera_placeholder.animation);
        return 1;
    }

    if (!camera_placeholder.is_empty || camera_placeholder.has_duration) {
        camera_set_transition_duration(camera, camera_placeholder.duration_in_beats, camera_placeholder.duration);
    }

    if (camera_placeholder.is_empty) {
        return 0;
    }
    camera_internal_drop_animation(camera);

    if (camera_placeholder.has_parallax_offset_only) {
        camera_set_offset(
            camera, camera_placeholder.offset_x, camera_placeholder.offset_y, camera_placeholder.offset_z
        );
    } else if (camera_placeholder.move_offset_only) {
        camera_move_offset(
            camera, camera_placeholder.to_offset_x, camera_placeholder.to_offset_y, camera_placeholder.to_offset_z
        );
        camera.force_update = 1;
    } else if (camera_placeholder.has_offset_from) {
        camera_slide_offset(
            camera,
            camera_placeholder.from_offset_x, camera_placeholder.from_offset_y, camera_placeholder.from_offset_z,
            camera_placeholder.to_offset_x, camera_placeholder.to_offset_y, camera_placeholder.to_offset_z
        );
    } else if (camera_placeholder.has_offset_to) {
        camera_slide_to_offset(
            camera, camera_placeholder.to_offset_x, camera_placeholder.to_offset_y, camera_placeholder.to_offset_z
        );
    }

    if (camera_placeholder.move_only) {
        camera_move(
            camera, camera_placeholder.to_x, camera_placeholder.to_y, camera_placeholder.to_z
        );
        camera_set_transition_duration(
            camera, camera_placeholder.duration_in_beats, camera_placeholder.duration
        );
        camera.force_update = 1;
    } else {
        if (camera_placeholder.has_from) {
            camera_slide(
                camera,
                camera_placeholder.from_x, camera_placeholder.from_y, camera_placeholder.from_z,
                camera_placeholder.to_x, camera_placeholder.to_y, camera_placeholder.to_z
            );
        } else {
            camera_slide_to(
                camera, camera_placeholder.to_x, camera_placeholder.to_y, camera_placeholder.to_z
            );
        }
        camera_set_transition_duration(
            camera, camera_placeholder.duration_in_beats, camera_placeholder.duration
        );
    }

    return 1;
}

function camera_to_origin(camera, should_slide) {
    camera_internal_drop_animation(camera);

    if (should_slide) {
        camera_slide_to(camera, 0, 0, 1);
        camera_repeat(camera);
    } else {
        camera_move(camera, 0, 0, 1);
    }
}

function camera_to_origin_offset(camera, should_slide) {
    camera_internal_drop_animation(camera);

    if (should_slide) {
        camera_slide_to_offset(camera, 0, 0, 1);
        camera_repeat(camera);
    } else {
        camera_move_offset(camera, 0, 0, 1);
    }
    camera.has_transition_offset = 1;
}

function camera_get_parent_layout(camera) {
    return camera.parent_layout;
}

function camera_set_parent_layout(camera, layout) {
    camera.parent_layout = layout;
}

function camera_set_animation(camera, animsprite) {
    if (camera.animation) animsprite_destroy(camera.animation);
    camera.animation = animsprite ? animsprite_clone(animsprite) : null;
    camera.transition_completed = 1;
}


function camera_repeat(camera) {
    if (camera.animation != null) {
        animsprite_restart(camera.animation);
        animsprite_update_using_callback(camera.animation, camera, camera_set_property, 1);
        return;
    }

    camera.progress = 0;
    camera.transition_completed = 0;
    tweenlerp_restart(camera.tweenlerp);
}

function camera_stop(camera) {
    camera.progress = camera.duration;
    camera.transition_completed = 1;

    if (camera.animation) {
        animsprite_stop(camera.animation);
        return;
    }

    camera.parallax_x = tweenlerp_peek_value_by_index(camera.tweenlerp, CAMERA_PX);
    camera.parallax_y = tweenlerp_peek_value_by_index(camera.tweenlerp, CAMERA_PY);
    camera.parallax_z = tweenlerp_peek_value_by_index(camera.tweenlerp, CAMERA_PZ);

    if (camera.has_transition_offset) {
        camera.offset_x = tweenlerp_peek_value_by_index(camera.tweenlerp, CAMERA_OX);
        camera.offset_y = tweenlerp_peek_value_by_index(camera.tweenlerp, CAMERA_OY);
        camera.offset_z = tweenlerp_peek_value_by_index(camera.tweenlerp, CAMERA_OZ);
    }
}

function camera_end(camera) {
    if (camera.animation) {
        animsprite_force_end(camera.animation);
        animsprite_update_using_callback(camera.animation, camera, camera_set_property, 1);
        return;
    }

    tweenlerp_end(camera.tweenlerp);

    camera.progress = camera.duration;
    camera.transition_completed = 1;
    camera.has_transition_offset = 0;

    camera.parallax_x = tweenlerp_peek_value_by_index(camera.tweenlerp, CAMERA_PX);
    camera.parallax_y = tweenlerp_peek_value_by_index(camera.tweenlerp, CAMERA_PY);
    camera.parallax_z = tweenlerp_peek_value_by_index(camera.tweenlerp, CAMERA_PZ);

    if (camera.has_transition_offset) {
        camera.offset_x = tweenlerp_peek_value_by_index(camera.tweenlerp, CAMERA_OX);
        camera.offset_y = tweenlerp_peek_value_by_index(camera.tweenlerp, CAMERA_OY);
        camera.offset_z = tweenlerp_peek_value_by_index(camera.tweenlerp, CAMERA_OZ);
    }
}


function camera_animate(camera, elapsed) {
    if (camera.animation) {
        let completed = animsprite_animate(camera.animation, elapsed);
        animsprite_update_using_callback(camera.animation, camera, camera_set_property, 1);
        return completed;
    }

    if (camera.transition_completed) {
        if (camera.force_update) {
            camera.force_update = 0;
            return 0;
        }

        camera.has_transition_offset = 0;
        return 1;
    }

    if (camera.progress >= camera.duration) {
        camera_end(camera);
        return 0;// required to apply the end values
    }

    let percent = camera.progress / camera.duration;
    camera.progress += elapsed;

    tweenlerp_animate_percent(camera.tweenlerp, percent);
    return 0;
}

function camera_apply(camera, pvrctx) {
    if (camera.transition_completed || camera.animation) {
        camera.modifier.translate_x = camera.parallax_x;
        camera.modifier.translate_y = camera.parallax_y;
        camera.modifier.scale_x = camera.parallax_z;
        camera.modifier.scale_y = camera.parallax_z;
    } else {
        let x = tweenlerp_peek_value_by_index(camera.tweenlerp, CAMERA_PX);
        let y = tweenlerp_peek_value_by_index(camera.tweenlerp, CAMERA_PY);
        let z = tweenlerp_peek_value_by_index(camera.tweenlerp, CAMERA_PZ);

        if (Number.isFinite(x)) camera.modifier.translate_x = x;
        if (Number.isFinite(y)) camera.modifier.translate_y = y;
        if (Number.isFinite(z)) camera.modifier.scale_x = camera.modifier.scale_y = z;
    }

    if (pvrctx) {
        let matrix = pvrctx.current_matrix;
        camera_apply_offset(camera, matrix);
        sh4matrix_apply_modifier(matrix, camera.modifier);
        pvr_context_flush(pvrctx);
    }

}

function camera_apply_offset(camera, destination_matrix) {
    if (!camera.animation && camera.has_transition_offset) {
        let x = tweenlerp_peek_value_by_index(camera.tweenlerp, CAMERA_OX);
        let y = tweenlerp_peek_value_by_index(camera.tweenlerp, CAMERA_OY);
        let z = tweenlerp_peek_value_by_index(camera.tweenlerp, CAMERA_OZ);

        if (Number.isFinite(x)) camera.offset_x = x;
        if (Number.isFinite(y)) camera.offset_y = y;
        if (Number.isFinite(z)) camera.offset_z = z;
    }

    sh4matrix_translate(destination_matrix, camera.offset_x, camera.offset_y);
    sh4matrix_scale(destination_matrix, camera.offset_z, camera.offset_z);
}

function camera_is_completed(camera) {
    if (camera.animation) return animsprite_is_completed(camera.animation);
    if (camera.transition_completed && camera.force_update) return 0;
    return camera.progress >= camera.duration;
}

function camera_set_property(camera, id, value) {
    switch (id) {
        case SPRITE_PROP_X:
            camera.parallax_x = value;
            break;
        case SPRITE_PROP_Y:
            camera.parallax_y = value;
            break;
        case SPRITE_PROP_Z:
            camera.parallax_z = value;
            break;
        case CAMERA_PROP_OFFSET_X:
            camera.offset_x = value;
            break;
        case CAMERA_PROP_OFFSET_Y:
            camera.offset_y = value;
            break;
        case CAMERA_PROP_OFFSET_Z:
            camera.offset_z = value;
            break;
    }
}


function camera_debug_log_info(camera) {
    console.log(`camera offset: x=${camera.offset_x} y=${camera.offset_y} z=${camera.offset_z}`);
    console.log(`camera position: x=${camera.modifier.translate_x} y=${camera.modifier.translate_y} z=${camera.modifier.scale_x}`);
}

function camera_internal_tweenlerp_absolute(camera, index, value) {
    tweenlerp_change_bounds_by_index(camera.tweenlerp, index, value, value);
}

function camera_internal_drop_animation(camera) {
    if (camera.animation) {
        animsprite_destroy(camera.animation);
        camera.animation = null;
    }
}

