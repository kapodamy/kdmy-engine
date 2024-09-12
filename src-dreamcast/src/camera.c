#include "camera.h"
#include "externals/luascript.h"
#include "float64.h"
#include "layout.h"
#include "logger.h"
#include "malloc_utils.h"
#include "math2d.h"
#include "pvrcontext.h"
#include "tweenlerp.h"
#include "vertexprops.h"


struct Camera_s {
    TweenLerp tweenlerp;
    Modifier* modifier;
    float64 progress;
    float64 duration;

    // bool scene_zoom;

    float64 beat_duration;
    bool internal_modifier;
    bool transition_completed;
    bool force_update;
    bool has_transition_offset;
    float parallax_x;
    float parallax_y;
    float parallax_z;
    float offset_x;
    float offset_y;
    float offset_z;
    Layout parent_layout;
    AnimSprite animation;
    bool enable_offset_zoom;
    float half_viewport_width;
    float half_viewport_height;
};


static const AnimInterpolator CAMERA_DEFAULT_INTERPOLATOR = AnimInterpolator_LINEAR;
static const int32_t CAMERA_PX = 0;
static const int32_t CAMERA_PY = 1;
static const int32_t CAMERA_PZ = 2;
static const int32_t CAMERA_OX = 3;
static const int32_t CAMERA_OY = 4;
static const int32_t CAMERA_OZ = 5;


static void camera_internal_tweenlerp_absolute(Camera camera, int32_t index, float value);
static void camera_internal_drop_animation(Camera camera);


Camera camera_init(Modifier* modifier, float viewport_width, float viewport_height) {
    Camera camera = malloc_chk(sizeof(struct Camera_s));
    malloc_assert(camera, Camera);

    *camera = (struct Camera_s){
        .tweenlerp = NULL,
        .modifier = modifier,
        .progress = 600.0,
        .duration = 600.0,

        //.scene_zoom = true,

        .beat_duration = 600.0,
        .internal_modifier = !modifier,
        .transition_completed = true,
        .force_update = false,
        .has_transition_offset = true,
        .parallax_x = 0.0f,
        parallax_y : 0.0f,
        parallax_z : 1.0f,
        .offset_x = 0.0f,
        offset_y : 0.0f,
        offset_z : 1.0f,
        .parent_layout = NULL,
        .animation = NULL,
        .enable_offset_zoom = true,
        .half_viewport_width = viewport_width / 2.0f,
        .half_viewport_height = viewport_height / 2.0f
    };

    if (camera->internal_modifier) {
        // no modifier provided used the internal one
        camera->modifier = malloc_chk(sizeof(Modifier));
        malloc_assert(camera->modifier, Modifier);

        pvr_context_helper_clear_modifier(camera->modifier);
        camera->modifier->width = viewport_width;
        camera->modifier->width = viewport_height;
        camera->modifier->scale_direction_x = -0.5f;
        camera->modifier->scale_direction_y = -0.5f;
    }

    camera_set_interpolator_type(camera, CAMERA_DEFAULT_INTERPOLATOR);

    return camera;
}

void camera_destroy(Camera* camera_ptr) {
    Camera camera = *camera_ptr;
    if (!camera) return;

    tweenlerp_destroy(&camera->tweenlerp);

    if (camera->internal_modifier) {
        luascript_drop_shared(camera->modifier);
        free_chk(camera->modifier);
    }

    if (camera->animation) animsprite_destroy(&camera->animation);

    luascript_drop_shared(camera);

    free_chk(camera);
    *camera_ptr = NULL;
}


void camera_set_bpm(Camera camera, float beats_per_minute) {
    camera->beat_duration = math2d_beats_per_minute_to_beat_per_milliseconds(beats_per_minute);
}

void camera_set_interpolator_type(Camera camera, AnimInterpolator type) {
    float px, py, pz, ox, oy, oz;

    if (camera->tweenlerp) {
        px = tweenlerp_peek_value_by_index(camera->tweenlerp, CAMERA_PX);
        py = tweenlerp_peek_value_by_index(camera->tweenlerp, CAMERA_PY);
        pz = tweenlerp_peek_value_by_index(camera->tweenlerp, CAMERA_PZ);
        ox = tweenlerp_peek_value_by_index(camera->tweenlerp, CAMERA_OX);
        oy = tweenlerp_peek_value_by_index(camera->tweenlerp, CAMERA_OY);
        oz = tweenlerp_peek_value_by_index(camera->tweenlerp, CAMERA_OZ);
        tweenlerp_destroy(&camera->tweenlerp);
    } else {
        px = camera->parallax_x;
        py = camera->parallax_y;
        pz = camera->parallax_z;
        ox = camera->offset_x;
        oy = camera->offset_y;
        oz = camera->offset_z;
    }

    camera->tweenlerp = tweenlerp_init();

    tweenlerp_add_interpolator(camera->tweenlerp, -1, px, px, 0.0f, type);
    tweenlerp_add_interpolator(camera->tweenlerp, -1, py, py, 0.0f, type);
    tweenlerp_add_interpolator(camera->tweenlerp, -1, pz, pz, 0.0f, type);
    tweenlerp_add_interpolator(camera->tweenlerp, -1, ox, ox, 0.0f, type);
    tweenlerp_add_interpolator(camera->tweenlerp, -1, oy, oy, 0.0f, type);
    tweenlerp_add_interpolator(camera->tweenlerp, -1, oz, oz, 0.0f, type);
    tweenlerp_end(camera->tweenlerp);

    camera->progress = camera->duration;
    camera->transition_completed = true;
}

void camera_set_transition_duration(Camera camera, bool expresed_in_beats, float value) {
    if (expresed_in_beats)
        camera->duration = value * camera->beat_duration;
    else
        camera->duration = value;
}

void camera_set_absolute_zoom(Camera camera, float z) {
    if (!math2d_is_float_NaN(z)) camera_internal_tweenlerp_absolute(camera, CAMERA_PZ, z);
    camera->force_update = true;
    camera_end(camera);
}

void camera_set_absolute_position(Camera camera, float x, float y) {
    if (!math2d_is_float_NaN(x)) camera_internal_tweenlerp_absolute(camera, CAMERA_PX, x);
    if (!math2d_is_float_NaN(y)) camera_internal_tweenlerp_absolute(camera, CAMERA_PY, y);
    camera->force_update = true;
    camera_end(camera);
}

void camera_set_absolute(Camera camera, float x, float y, float z) {
    camera_set_absolute_position(camera, x, y);
    camera_set_absolute_zoom(camera, z);
}


void camera_set_offset(Camera camera, float x, float y, float z) {
    if (!math2d_is_float_NaN(x)) {
        camera_internal_tweenlerp_absolute(camera, CAMERA_OX, x);
        camera->offset_x = x;
    }
    if (!math2d_is_float_NaN(y)) {
        camera_internal_tweenlerp_absolute(camera, CAMERA_OY, y);
        camera->offset_y = y;
    }
    if (!math2d_is_float_NaN(z)) {
        camera_internal_tweenlerp_absolute(camera, CAMERA_OZ, z);
        camera->offset_z = z;
    }

    camera->has_transition_offset = false;
}

void camera_get_offset(Camera camera, float* x, float* y, float* z) {
    *x = camera->offset_x;
    *y = camera->offset_y;
    *z = camera->offset_z;
}


void camera_replace_modifier(Camera camera, Modifier* new_modifier) {
    assert(new_modifier != NULL);

    if (camera->internal_modifier) {
        free_chk(camera->modifier);
        camera->internal_modifier = false;
    }

    camera->modifier = new_modifier;
}

Modifier* camera_get_modifier(Camera camera) {
    return camera->modifier;
}


void camera_move(Camera camera, float end_x, float end_y, float end_z) {
    camera_internal_drop_animation(camera);

    tweenlerp_change_bounds_by_index(camera->tweenlerp, CAMERA_PX, FLOAT_NaN, end_x);
    tweenlerp_change_bounds_by_index(camera->tweenlerp, CAMERA_PY, FLOAT_NaN, end_y);
    tweenlerp_change_bounds_by_index(camera->tweenlerp, CAMERA_PZ, FLOAT_NaN, end_z);

    camera_end(camera);
}

void camera_move_offset(Camera camera, float end_x, float end_y, float end_z) {
    camera_internal_drop_animation(camera);

    tweenlerp_change_bounds_by_index(camera->tweenlerp, CAMERA_OX, FLOAT_NaN, end_x);
    tweenlerp_change_bounds_by_index(camera->tweenlerp, CAMERA_OY, FLOAT_NaN, end_y);
    tweenlerp_change_bounds_by_index(camera->tweenlerp, CAMERA_OZ, FLOAT_NaN, end_z);

    camera->has_transition_offset = true;
    camera_end(camera);
}

void camera_slide(Camera camera, float start_x, float start_y, float start_z, float end_x, float end_y, float end_z) {
    camera_internal_drop_animation(camera);

    tweenlerp_change_bounds_by_index(camera->tweenlerp, CAMERA_PX, start_x, end_x);
    tweenlerp_change_bounds_by_index(camera->tweenlerp, CAMERA_PY, start_y, end_y);
    tweenlerp_change_bounds_by_index(camera->tweenlerp, CAMERA_PZ, start_z, end_z);

    camera_repeat(camera);
}

void camera_slide_offset(Camera camera, float start_x, float start_y, float start_z, float end_x, float end_y, float end_z) {
    camera_internal_drop_animation(camera);

    tweenlerp_change_bounds_by_index(camera->tweenlerp, CAMERA_OX, start_x, end_x);
    tweenlerp_change_bounds_by_index(camera->tweenlerp, CAMERA_OY, start_y, end_y);
    tweenlerp_change_bounds_by_index(camera->tweenlerp, CAMERA_OZ, start_z, end_z);

    camera->has_transition_offset = true;
    camera_repeat(camera);
}

void camera_slide_x(Camera camera, float start, float end) {
    camera_internal_drop_animation(camera);

    tweenlerp_change_bounds_by_index(camera->tweenlerp, CAMERA_PX, start, end);
    tweenlerp_override_start_with_end_by_index(camera->tweenlerp, CAMERA_PY);
    tweenlerp_override_start_with_end_by_index(camera->tweenlerp, CAMERA_PZ);
    camera_repeat(camera);
}

void camera_slide_x_offset(Camera camera, float start, float end) {
    camera_internal_drop_animation(camera);

    tweenlerp_change_bounds_by_index(camera->tweenlerp, CAMERA_OX, start, end);
    tweenlerp_override_start_with_end_by_index(camera->tweenlerp, CAMERA_OY);
    tweenlerp_override_start_with_end_by_index(camera->tweenlerp, CAMERA_OZ);
    camera->has_transition_offset = true;
    camera_repeat(camera);
}

void camera_slide_y(Camera camera, float start, float end) {
    camera_internal_drop_animation(camera);

    tweenlerp_override_start_with_end_by_index(camera->tweenlerp, CAMERA_PX);
    tweenlerp_change_bounds_by_index(camera->tweenlerp, CAMERA_PY, start, end);
    tweenlerp_override_start_with_end_by_index(camera->tweenlerp, CAMERA_PZ);
    camera_repeat(camera);
}

void camera_slide_y_offset(Camera camera, float start, float end) {
    camera_internal_drop_animation(camera);

    tweenlerp_override_start_with_end_by_index(camera->tweenlerp, CAMERA_OX);
    tweenlerp_change_bounds_by_index(camera->tweenlerp, CAMERA_OY, start, end);
    tweenlerp_override_start_with_end_by_index(camera->tweenlerp, CAMERA_OZ);
    camera->has_transition_offset = true;
    camera_repeat(camera);
}

void camera_slide_z(Camera camera, float start, float end) {
    camera_internal_drop_animation(camera);

    tweenlerp_override_start_with_end_by_index(camera->tweenlerp, CAMERA_PX);
    tweenlerp_override_start_with_end_by_index(camera->tweenlerp, CAMERA_PY);
    tweenlerp_change_bounds_by_index(camera->tweenlerp, CAMERA_PZ, start, end);
    camera_repeat(camera);
}

void camera_slide_z_offset(Camera camera, float start, float end) {
    camera_internal_drop_animation(camera);

    tweenlerp_override_start_with_end_by_index(camera->tweenlerp, CAMERA_OX);
    tweenlerp_override_start_with_end_by_index(camera->tweenlerp, CAMERA_OY);
    tweenlerp_change_bounds_by_index(camera->tweenlerp, CAMERA_OZ, start, end);
    camera_repeat(camera);
}

void camera_slide_to(Camera camera, float x, float y, float z) {
    camera_internal_drop_animation(camera);

    float start_x = tweenlerp_peek_value_by_index(camera->tweenlerp, CAMERA_PX);
    float start_y = tweenlerp_peek_value_by_index(camera->tweenlerp, CAMERA_PY);
    float start_z = tweenlerp_peek_value_by_index(camera->tweenlerp, CAMERA_PZ);

    if (math2d_is_float_NaN(x)) x = start_x;
    if (math2d_is_float_NaN(y)) y = start_y;
    if (math2d_is_float_NaN(z)) z = start_z;

    tweenlerp_change_bounds_by_index(camera->tweenlerp, CAMERA_PX, start_x, x);
    tweenlerp_change_bounds_by_index(camera->tweenlerp, CAMERA_PY, start_y, y);
    tweenlerp_change_bounds_by_index(camera->tweenlerp, CAMERA_PZ, start_z, z);

    camera_repeat(camera);
}

void camera_slide_to_offset(Camera camera, float x, float y, float z) {
    camera_internal_drop_animation(camera);

    float start_x = tweenlerp_peek_value_by_index(camera->tweenlerp, CAMERA_OX);
    float start_y = tweenlerp_peek_value_by_index(camera->tweenlerp, CAMERA_OY);
    float start_z = tweenlerp_peek_value_by_index(camera->tweenlerp, CAMERA_OZ);

    if (math2d_is_float_NaN(x)) x = start_x;
    if (math2d_is_float_NaN(y)) y = start_y;
    if (math2d_is_float_NaN(z)) z = start_z;

    tweenlerp_change_bounds_by_index(camera->tweenlerp, CAMERA_OX, start_x, x);
    tweenlerp_change_bounds_by_index(camera->tweenlerp, CAMERA_OY, start_y, y);
    tweenlerp_change_bounds_by_index(camera->tweenlerp, CAMERA_OZ, start_z, z);

    camera->has_transition_offset = true;

    camera_repeat(camera);
}

bool camera_from_layout(Camera camera, Layout layout, const char* camera_name) {
    if (!layout) {
        if (!camera->parent_layout) return false;
        layout = camera->parent_layout;
    }

    const CameraPlaceholder* camera_placeholder = layout_get_camera_placeholder(layout, camera_name);
    if (!camera_placeholder) return false;

    camera->enable_offset_zoom = camera_placeholder->enable_offset_zoom;

    if (camera_placeholder->animation) {
        camera_set_animation(camera, camera_placeholder->animation);
        return true;
    }

    if (!camera_placeholder->is_empty || camera_placeholder->has_duration) {
        camera_set_transition_duration(camera, camera_placeholder->duration_in_beats, camera_placeholder->duration);
    }

    if (camera_placeholder->is_empty) {
        return false;
    }
    camera_internal_drop_animation(camera);

    if (camera_placeholder->has_parallax_offset_only) {
        camera_set_offset(
            camera, camera_placeholder->offset_x, camera_placeholder->offset_y, camera_placeholder->offset_z
        );
    } else if (camera_placeholder->move_offset_only) {
        camera_move_offset(
            camera, camera_placeholder->to_offset_x, camera_placeholder->to_offset_y, camera_placeholder->to_offset_z
        );
        camera->force_update = true;
    } else if (camera_placeholder->has_offset_from) {
        camera_slide_offset(
            camera,
            camera_placeholder->from_offset_x, camera_placeholder->from_offset_y, camera_placeholder->from_offset_z,
            camera_placeholder->to_offset_x, camera_placeholder->to_offset_y, camera_placeholder->to_offset_z
        );
    } else if (camera_placeholder->has_offset_to) {
        camera_slide_to_offset(
            camera, camera_placeholder->to_offset_x, camera_placeholder->to_offset_y, camera_placeholder->to_offset_z
        );
    }

    if (camera_placeholder->move_only) {
        camera_move(
            camera, camera_placeholder->to_x, camera_placeholder->to_y, camera_placeholder->to_z
        );
        camera_set_transition_duration(
            camera, camera_placeholder->duration_in_beats, camera_placeholder->duration
        );
        camera->force_update = true;
    } else {
        if (camera_placeholder->has_from) {
            camera_slide(
                camera,
                camera_placeholder->from_x, camera_placeholder->from_y, camera_placeholder->from_z,
                camera_placeholder->to_x, camera_placeholder->to_y, camera_placeholder->to_z
            );
        } else {
            camera_slide_to(
                camera, camera_placeholder->to_x, camera_placeholder->to_y, camera_placeholder->to_z
            );
        }
        camera_set_transition_duration(
            camera, camera_placeholder->duration_in_beats, camera_placeholder->duration
        );
    }

    return true;
}

void camera_to_origin(Camera camera, bool should_slide) {
    camera_internal_drop_animation(camera);

    if (should_slide) {
        camera_slide_to(camera, 0.0f, 0.0f, 1.0f);
        camera_repeat(camera);
    } else {
        camera_move(camera, 0.0f, 0.0f, 1.0f);
    }
}

void camera_to_origin_offset(Camera camera, bool should_slide) {
    camera_internal_drop_animation(camera);

    if (should_slide) {
        camera_slide_to_offset(camera, 0.0f, 0.0f, 1.0f);
        camera_repeat(camera);
    } else {
        camera_move_offset(camera, 0.0f, 0.0f, 1.0f);
    }
    camera->has_transition_offset = true;
}

Layout camera_get_parent_layout(Camera camera) {
    return camera->parent_layout;
}

void camera_set_parent_layout(Camera camera, Layout layout) {
    camera->parent_layout = layout;
}

void camera_set_animation(Camera camera, AnimSprite animsprite) {
    if (camera->animation) animsprite_destroy(&camera->animation);
    camera->animation = animsprite ? animsprite_clone(animsprite) : NULL;
    camera->transition_completed = true;
}


void camera_repeat(Camera camera) {
    if (camera->animation != NULL) {
        animsprite_restart(camera->animation);
        animsprite_update_using_callback(camera->animation, camera, (PropertySetter)camera_set_property, true);
        return;
    }

    camera->progress = 0.0;
    camera->transition_completed = false;
    tweenlerp_restart(camera->tweenlerp);
}

void camera_stop(Camera camera) {
    camera->progress = camera->duration;
    camera->transition_completed = true;

    if (camera->animation) {
        animsprite_stop(camera->animation);
        return;
    }

    camera->parallax_x = tweenlerp_peek_value_by_index(camera->tweenlerp, CAMERA_PX);
    camera->parallax_y = tweenlerp_peek_value_by_index(camera->tweenlerp, CAMERA_PY);
    camera->parallax_z = tweenlerp_peek_value_by_index(camera->tweenlerp, CAMERA_PZ);

    if (camera->has_transition_offset) {
        camera->offset_x = tweenlerp_peek_value_by_index(camera->tweenlerp, CAMERA_OX);
        camera->offset_y = tweenlerp_peek_value_by_index(camera->tweenlerp, CAMERA_OY);
        camera->offset_z = tweenlerp_peek_value_by_index(camera->tweenlerp, CAMERA_OZ);
    }
}

void camera_end(Camera camera) {
    if (camera->animation) {
        animsprite_force_end(camera->animation);
        animsprite_update_using_callback(camera->animation, camera, (PropertySetter)camera_set_property, true);
        return;
    }

    tweenlerp_end(camera->tweenlerp);

    camera->progress = camera->duration;
    camera->transition_completed = true;
    camera->has_transition_offset = false;

    camera->parallax_x = tweenlerp_peek_value_by_index(camera->tweenlerp, CAMERA_PX);
    camera->parallax_y = tweenlerp_peek_value_by_index(camera->tweenlerp, CAMERA_PY);
    camera->parallax_z = tweenlerp_peek_value_by_index(camera->tweenlerp, CAMERA_PZ);

    if (camera->has_transition_offset) {
        camera->offset_x = tweenlerp_peek_value_by_index(camera->tweenlerp, CAMERA_OX);
        camera->offset_y = tweenlerp_peek_value_by_index(camera->tweenlerp, CAMERA_OY);
        camera->offset_z = tweenlerp_peek_value_by_index(camera->tweenlerp, CAMERA_OZ);
    }
}


int32_t camera_animate(Camera camera, float elapsed) {
    if (camera->animation) {
        int32_t completed = animsprite_animate(camera->animation, elapsed);
        animsprite_update_using_callback(camera->animation, camera, (PropertySetter)camera_set_property, true);
        return completed;
    }

    if (camera->transition_completed) {
        if (camera->force_update) {
            camera->force_update = false;
            return 0;
        }

        camera->has_transition_offset = false;
        return 1;
    }

    if (camera->progress >= camera->duration) {
        camera_end(camera);
        return 0; // required to apply the end values
    }

    float64 percent = camera->progress / camera->duration;
    camera->progress += elapsed;

    tweenlerp_animate_percent(camera->tweenlerp, percent);
    return 0;
}

void camera_apply(Camera camera, PVRContext pvrctx) {
    if (camera->transition_completed || camera->animation) {
        camera->modifier->translate_x = camera->parallax_x;
        camera->modifier->translate_y = camera->parallax_y;
        camera->modifier->scale_x = camera->parallax_z;
        camera->modifier->scale_y = camera->parallax_z;
    } else {
        float x = tweenlerp_peek_value_by_index(camera->tweenlerp, CAMERA_PX);
        float y = tweenlerp_peek_value_by_index(camera->tweenlerp, CAMERA_PY);
        float z = tweenlerp_peek_value_by_index(camera->tweenlerp, CAMERA_PZ);

        if (!math2d_is_float_NaN(x)) camera->modifier->translate_x = x;
        if (!math2d_is_float_NaN(y)) camera->modifier->translate_y = y;
        if (!math2d_is_float_NaN(z)) camera->modifier->scale_x = camera->modifier->scale_y = z;
    }

    if (pvrctx) {
        float* matrix = pvrctx->current_matrix;
        camera_apply_offset(camera, matrix);
        sh4matrix_apply_modifier(matrix, camera->modifier);
        pvr_context_flush(pvrctx);
    }
}

void camera_apply_offset(Camera camera, float* destination_matrix) {
    if (!camera->animation && camera->has_transition_offset) {
        float x = tweenlerp_peek_value_by_index(camera->tweenlerp, CAMERA_OX);
        float y = tweenlerp_peek_value_by_index(camera->tweenlerp, CAMERA_OY);
        float z = tweenlerp_peek_value_by_index(camera->tweenlerp, CAMERA_OZ);

        if (!math2d_is_float_NaN(x)) camera->offset_x = x;
        if (!math2d_is_float_NaN(y)) camera->offset_y = y;
        if (!math2d_is_float_NaN(z)) camera->offset_z = z;
    }

    sh4matrix_translate(destination_matrix, camera->offset_x, camera->offset_y);

    if (camera->enable_offset_zoom && camera->offset_z != 1.0f) {
        sh4matrix_scale_size(
            destination_matrix,
            camera->half_viewport_width, camera->half_viewport_height,
            camera->offset_z, camera->offset_z
        );
    } else {
        sh4matrix_scale(destination_matrix, camera->offset_z, camera->offset_z);
    }
}

bool camera_is_completed(Camera camera) {
    if (camera->animation) return animsprite_is_completed(camera->animation);
    if (camera->transition_completed && camera->force_update) return false;
    return camera->progress >= camera->duration;
}

void camera_disable_offset_zoom(Camera camera, bool disabled) {
    camera->enable_offset_zoom = !disabled;
}

void camera_change_viewport(Camera camera, float width, float height) {
    if (camera->internal_modifier) {
        camera->modifier->width = width;
        camera->modifier->height = height;
    }
    camera->half_viewport_width = width / 2.0f;
    camera->half_viewport_height = height / 2.0f;
}

void camera_set_property(Camera camera, int32_t id, float value) {
    switch (id) {
        case SPRITE_PROP_X:
            camera->parallax_x = value;
            break;
        case SPRITE_PROP_Y:
            camera->parallax_y = value;
            break;
        case SPRITE_PROP_Z:
            camera->parallax_z = value;
            break;
        case CAMERA_PROP_OFFSET_X:
            camera->offset_x = value;
            break;
        case CAMERA_PROP_OFFSET_Y:
            camera->offset_y = value;
            break;
        case CAMERA_PROP_OFFSET_Z:
            camera->offset_z = value;
            break;
        case CAMERA_PROP_OFFSET_ZOOM:
            camera->enable_offset_zoom = value >= 1.0f;
            break;
    }
}


void camera_debug_log_info(Camera camera) {
    logger_log("camera offset: x=%f y=%f z=%f", camera->offset_x, camera->offset_y, camera->offset_z);
    logger_log("camera position: x=%f y=%f z=%f", camera->modifier->translate_x, camera->modifier->translate_y, camera->modifier->scale_x);
}

static void camera_internal_tweenlerp_absolute(Camera camera, int32_t index, float value) {
    tweenlerp_change_bounds_by_index(camera->tweenlerp, index, value, value);
}

static void camera_internal_drop_animation(Camera camera) {
    if (camera->animation) {
        animsprite_destroy(&camera->animation);
        camera->animation = NULL;
    }
}
