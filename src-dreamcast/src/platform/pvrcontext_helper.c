#include "pvrcontext.h"
#include "vertexprops.h"
#include "math2d.h"


static const RGBA PVRCONTEXT_DEFAULT_OFFSET_COLOR = {0.0f, 0.0f, 0.0f, -1.0f};


void pvr_context_helper_clear_modifier(Modifier* modifier) {
    modifier->translate_x = 0.0f;
    modifier->translate_y = 0.0f;

    modifier->rotate = 0.0f;

    modifier->skew_x = 0.0f;
    modifier->skew_y = 0.0f;

    modifier->scale_x = 1.0f;
    modifier->scale_y = 1.0f;

    modifier->scale_direction_x = 0.0f;
    modifier->scale_direction_y = 0.0f;

    modifier->rotate_pivot_enabled = false;
    modifier->rotate_pivot_u = 0.5f;
    modifier->rotate_pivot_v = 0.5f;

    modifier->translate_rotation = false;
    modifier->scale_size = false;
    modifier->scale_translation = false;

    modifier->x = 0.0f;
    modifier->y = 0.0f;
    modifier->width = -1.0f;
    modifier->height = -1.0f;
}

void pvr_context_helper_invalidate_modifier(Modifier* modifier) {
    modifier->translate_x = FLOAT_NaN;
    modifier->translate_y = FLOAT_NaN;

    modifier->rotate = FLOAT_NaN;

    modifier->skew_x = FLOAT_NaN;
    modifier->skew_y = FLOAT_NaN;

    modifier->scale_x = FLOAT_NaN;
    modifier->scale_y = FLOAT_NaN;

    modifier->scale_direction_x = FLOAT_NaN;
    modifier->scale_direction_y = FLOAT_NaN;

    modifier->rotate_pivot_enabled = unset;
    modifier->rotate_pivot_u = FLOAT_NaN;
    modifier->rotate_pivot_v = FLOAT_NaN;

    modifier->translate_rotation = unset;
    modifier->scale_size = unset;
    modifier->scale_translation = unset;

    modifier->x = FLOAT_NaN;
    modifier->y = FLOAT_NaN;
    modifier->width = FLOAT_NaN;
    modifier->height = FLOAT_NaN;
}

void pvr_context_helper_clear_offsetcolor(RGBA offsetcolor) {
    memcpy(offsetcolor, PVRCONTEXT_DEFAULT_OFFSET_COLOR, sizeof(PVRCONTEXT_DEFAULT_OFFSET_COLOR));
}

void pvr_context_helper_set_modifier_property(Modifier* modifier, int32_t property, float value) {
    switch (property) {
        case SPRITE_PROP_ROTATE:
            modifier->rotate = value * MATH2D_DEG_TO_RAD;
            break;
        case SPRITE_PROP_SCALE_X:
            modifier->scale_x = value;
            break;
        case SPRITE_PROP_SCALE_Y:
            modifier->scale_y = value;
            break;
        case SPRITE_PROP_TRANSLATE_X:
            modifier->translate_x = value;
            break;
        case SPRITE_PROP_TRANSLATE_Y:
            modifier->translate_y = value;
            break;
        case SPRITE_PROP_SKEW_X:
            modifier->skew_x = value;
            break;
        case SPRITE_PROP_SKEW_Y:
            modifier->skew_y = value;
            break;
        case SPRITE_PROP_ROTATE_PIVOT_ENABLED:
            modifier->rotate_pivot_enabled = value >= 1.0f ? true : false;
            break;
        case SPRITE_PROP_ROTATE_PIVOT_U:
            modifier->rotate_pivot_u = value;
            break;
        case SPRITE_PROP_ROTATE_PIVOT_V:
            modifier->rotate_pivot_v = value;
            break;
        case SPRITE_PROP_SCALE_DIRECTION_X:
            modifier->scale_direction_x = value;
            break;
        case SPRITE_PROP_SCALE_DIRECTION_Y:
            modifier->scale_direction_y = value;
            break;
        case SPRITE_PROP_TRANSLATE_ROTATION:
            modifier->translate_rotation = value >= 1.0f ? true : false;
            break;
        case SPRITE_PROP_SCALE_SIZE:
            modifier->scale_size = value >= 1.0f ? true : false;
            break;
        case SPRITE_PROP_SCALE_TRANSLATION:
            modifier->scale_translation = value >= 1.0f ? true : false;
            break;
    }
}

void pvr_context_helper_copy_modifier(const Modifier* modifier_source, Modifier* modifier_dest) {
    if (!math2d_is_float_NaN(modifier_source->translate_x))
        modifier_dest->translate_x = modifier_source->translate_x;
    if (!math2d_is_float_NaN(modifier_source->translate_y))
        modifier_dest->translate_y = modifier_source->translate_y;

    if (!math2d_is_float_NaN(modifier_source->rotate))
        modifier_dest->rotate = modifier_source->rotate;

    if (!math2d_is_float_NaN(modifier_source->skew_x))
        modifier_dest->skew_x = modifier_source->skew_x;
    if (!math2d_is_float_NaN(modifier_source->skew_y))
        modifier_dest->skew_y = modifier_source->skew_y;

    if (!math2d_is_float_NaN(modifier_source->scale_x))
        modifier_dest->scale_x = modifier_source->scale_x;
    if (!math2d_is_float_NaN(modifier_source->scale_y))
        modifier_dest->scale_y = modifier_source->scale_y;

    if (!math2d_is_float_NaN(modifier_source->scale_direction_x))
        modifier_dest->scale_direction_x = modifier_source->scale_direction_x;
    if (!math2d_is_float_NaN(modifier_source->scale_direction_y))
        modifier_dest->scale_direction_y = modifier_source->scale_direction_y;

    if (modifier_source->rotate_pivot_enabled != unset)
        modifier_dest->rotate_pivot_enabled = modifier_source->rotate_pivot_enabled;
    if (!math2d_is_float_NaN(modifier_source->rotate_pivot_u))
        modifier_dest->rotate_pivot_u = modifier_source->rotate_pivot_u;
    if (!math2d_is_float_NaN(modifier_source->rotate_pivot_v))
        modifier_dest->rotate_pivot_v = modifier_source->rotate_pivot_v;

    if (modifier_source->translate_rotation != unset)
        modifier_dest->translate_rotation = modifier_source->translate_rotation;
    if (modifier_source->scale_size != unset)
        modifier_dest->scale_size = modifier_source->scale_size;
    if (modifier_source->scale_translation != unset)
        modifier_dest->scale_translation = modifier_source->scale_translation;

    if (!math2d_is_float_NaN(modifier_source->x))
        modifier_dest->x = modifier_source->x;
    if (!math2d_is_float_NaN(modifier_source->y))
        modifier_dest->y = modifier_source->y;
    if (!math2d_is_float_NaN(modifier_source->width))
        modifier_dest->width = -modifier_source->width;
    if (!math2d_is_float_NaN(modifier_source->height))
        modifier_dest->height = -modifier_source->height;
}
