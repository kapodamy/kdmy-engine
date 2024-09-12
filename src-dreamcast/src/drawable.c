#include "drawable.h"
#include "externals/luascript.h"
#include "malloc_utils.h"
#include "map.h"
#include "math2d.h"
#include "pvrcontext.h"
#include "vertexprops.h"


struct Drawable_s {
    int32_t id;
    float alpha;
    float alpha2;
    RGBA offsetcolor;
    Modifier modifier;
    float z_index;
    float z_offset;
    DelegateDraw callback_draw;
    DelegateAnimate callback_animate;
    void* private_data;
    bool visible;
    PVRFlag antialiasing;

    // PSShader psshader;

    bool blend_enabled;
    Blend blend_src_rgb;
    Blend blend_dst_rgb;
    Blend blend_src_alpha;
    Blend blend_dst_alpha;
};


static Map DRAWABLE_POOL = NULL;
static int32_t DRAWABLE_IDS = 0;


void __attribute__((constructor)) __ctor_drawable() {
    DRAWABLE_POOL = map_init();
}


Drawable drawable_init(float z, void* private_data, DelegateDraw callback_draw, DelegateAnimate callback_animate) {
    Drawable drawable = malloc_chk(sizeof(struct Drawable_s));
    malloc_assert(drawable, Drawable);

    *drawable = (struct Drawable_s){
        .id = DRAWABLE_IDS++,

        .alpha = 1.0f,
        .alpha2 = 1.0f,

        //.psshader = NULL,

        .blend_enabled = true,// obligatory
        .blend_src_rgb = BLEND_DEFAULT,
        .blend_dst_rgb = BLEND_DEFAULT,
        .blend_src_alpha = BLEND_DEFAULT,
        .blend_dst_alpha = BLEND_DEFAULT,

        .z_index = z,
        .z_offset = 0.0,

        .callback_draw = callback_draw,
        .callback_animate = callback_animate,
        .private_data = private_data,

        .visible = true,

        .antialiasing = PVRCTX_FLAG_DEFAULT
    };

    pvr_context_helper_clear_offsetcolor(drawable->offsetcolor);
    pvr_context_helper_clear_modifier(&drawable->modifier);

    map_add(DRAWABLE_POOL, drawable->id, drawable);
    return drawable;
}

void drawable_destroy(Drawable* drawable_ptr) {
    Drawable drawable = *drawable_ptr;
    if (!drawable) return;

    map_delete(DRAWABLE_POOL, drawable->id);

    drawable->callback_draw = NULL;
    drawable->callback_animate = NULL;
    drawable->private_data = NULL;

    luascript_drop_shared(&drawable->modifier);
    luascript_drop_shared(drawable);

    free_chk(drawable);
    *drawable_ptr = NULL;
}


void drawable_set_z_index(Drawable drawable, float z_index) {
    drawable->z_index = z_index;
}

float drawable_get_z_index(Drawable drawable) {
    return drawable->z_index + drawable->z_offset;
}

void drawable_set_z_offset(Drawable drawable, float offset) {
    drawable->z_offset = offset;
}


void drawable_set_alpha(Drawable drawable, float alpha) {
    drawable->alpha = alpha;
}

float drawable_get_alpha(Drawable drawable) {
    return drawable->alpha * drawable->alpha2;
}

void drawable_set_offsetcolor(Drawable drawable, float r, float g, float b, float a) {
    if (!math2d_is_float_NaN(r)) drawable->offsetcolor[0] = r;
    if (!math2d_is_float_NaN(g)) drawable->offsetcolor[1] = g;
    if (!math2d_is_float_NaN(b)) drawable->offsetcolor[2] = b;
    if (!math2d_is_float_NaN(a)) drawable->offsetcolor[3] = a;
}

void drawable_set_offsetcolor_to_default(Drawable drawable) {
    pvr_context_helper_clear_offsetcolor(drawable->offsetcolor);
}

void drawable_set_visible(Drawable drawable, bool visible) {
    drawable->visible = visible;
}

Modifier* drawable_get_modifier(Drawable drawable) {
    return &drawable->modifier;
}


void drawable_draw(Drawable drawable, PVRContext pvrctx) {
    return drawable->callback_draw(drawable->private_data, pvrctx);
}

void drawable_change_private_data(Drawable drawable, void* private_data) {
    drawable->private_data = private_data;
}

int32_t drawable_animate(Drawable drawable, float elapsed) {
    if (drawable->callback_animate)
        return drawable->callback_animate(drawable->private_data, elapsed);
    else
        return 0;
}

void drawable_set_property(Drawable drawable, int32_t property_id, float value) {
    switch (property_id) {
        case SPRITE_PROP_X:
            drawable->modifier.x = value;
            break;
        case SPRITE_PROP_Y:
            drawable->modifier.y = value;
            break;
        case SPRITE_PROP_WIDTH:
            drawable->modifier.width = value;
            break;
        case SPRITE_PROP_HEIGHT:
            drawable->modifier.height = value;
            break;
        case SPRITE_PROP_ROTATE:
            drawable->modifier.rotate = value * MATH2D_DEG_TO_RAD;
            break;
        case SPRITE_PROP_SCALE_X:
            drawable->modifier.scale_x = value;
            break;
        case SPRITE_PROP_SCALE_Y:
            drawable->modifier.scale_y = value;
            break;
        case SPRITE_PROP_SKEW_X:
            drawable->modifier.skew_x = value;
            break;
        case SPRITE_PROP_SKEW_Y:
            drawable->modifier.skew_y = value;
            break;
        case SPRITE_PROP_TRANSLATE_X:
            drawable->modifier.translate_x = value;
            break;
        case SPRITE_PROP_TRANSLATE_Y:
            drawable->modifier.translate_y = value;
            break;
        case SPRITE_PROP_ALPHA:
            drawable->alpha = math2d_clamp_float(value, 0.0f, 1.0f);
            break;
        case SPRITE_PROP_Z:
            drawable->z_index = value;
            break;
        case SPRITE_PROP_VERTEX_COLOR_R:
        case SPRITE_PROP_VERTEX_COLOR_G:
        case SPRITE_PROP_VERTEX_COLOR_B:
            break;
        case SPRITE_PROP_VERTEX_COLOR_OFFSET_R:
            drawable->offsetcolor[0] = value;
            break;
        case SPRITE_PROP_VERTEX_COLOR_OFFSET_G:
            drawable->offsetcolor[1] = value;
            break;
        case SPRITE_PROP_VERTEX_COLOR_OFFSET_B:
            drawable->offsetcolor[2] = value;
            break;
        case SPRITE_PROP_VERTEX_COLOR_OFFSET_A:
            drawable->offsetcolor[3] = value;
            break;
        case SPRITE_PROP_ANIMATIONLOOP:
            break;
        case SPRITE_PROP_ROTATE_PIVOT_ENABLED:
            drawable->modifier.rotate_pivot_enabled = value >= 1.0f;
            break;
        case SPRITE_PROP_ROTATE_PIVOT_U:
            drawable->modifier.rotate_pivot_u = value;
            break;
        case SPRITE_PROP_ROTATE_PIVOT_V:
            drawable->modifier.rotate_pivot_v = value;
            break;
        case SPRITE_PROP_SCALE_DIRECTION_X:
            drawable->modifier.scale_direction_x = value;
            break;
        case SPRITE_PROP_SCALE_DIRECTION_Y:
            drawable->modifier.scale_direction_y = value;
            break;
        case SPRITE_PROP_TRANSLATE_ROTATION:
            drawable->modifier.translate_rotation = value >= 1.0f;
            break;
        case SPRITE_PROP_SCALE_SIZE:
            drawable->modifier.scale_size = value >= 1.0f;
            break;
        case SPRITE_PROP_SCALE_TRANSLATION:
            drawable->modifier.scale_translation = value >= 1.0f;
            break;
        case SPRITE_PROP_Z_OFFSET:
            drawable->z_offset = value;
            break;
        case SPRITE_PROP_ANTIALIASING:
            drawable->antialiasing = (PVRFlag)(int)value;
            break;
        case SPRITE_PROP_ALPHA2:
            drawable->alpha2 = value;
            break;
    }
}

void drawable_helper_apply_in_context(Drawable drawable, PVRContext pvrctx) {
    pvr_context_apply_modifier(pvrctx, &drawable->modifier);
    pvr_context_set_global_alpha(pvrctx, drawable->alpha * drawable->alpha2);
    pvr_context_set_global_offsetcolor(pvrctx, drawable->offsetcolor);
    if (drawable->antialiasing != PVRCTX_FLAG_DEFAULT) {
        pvr_context_set_global_antialiasing(pvrctx, drawable->antialiasing);
    }
    // if (drawable->psshader) pvr_context_add_shader(pvrctx, drawable->psshader);

    pvr_context_set_vertex_blend(
        pvrctx,
        drawable->blend_enabled,
        drawable->blend_src_rgb,
        drawable->blend_dst_rgb,
        drawable->blend_src_alpha,
        drawable->blend_dst_alpha
    );
}

void drawable_helper_update_from_placeholder(Drawable drawable, const LayoutPlaceholder* layout_placeholder) {
    drawable->modifier.x = layout_placeholder->x;
    drawable->modifier.y = layout_placeholder->y;
    drawable->z_index = layout_placeholder->z;
    drawable->modifier.width = layout_placeholder->width;
    drawable->modifier.height = layout_placeholder->height;
    drawable->z_offset = 0.0f;
}


bool drawable_is_visible(Drawable drawable) {
    return drawable->visible;
}

void drawable_get_draw_location(Drawable drawable, float* x, float* y) {
    *x = drawable->modifier.x;
    *y = drawable->modifier.y;
}

void drawable_set_antialiasing(Drawable drawable, PVRFlag antialiasing) {
    drawable->antialiasing = antialiasing;
}

/*void drawable_set_shader(Drawable drawable, PSShader psshader) {
    drawable->psshader = psshader;
}

PSShader drawable_get_shader(Drawable drawable) {
    return drawable->psshader;
}*/

void drawable_blend_enable(Drawable drawable, bool enabled) {
    drawable->blend_enabled = enabled;
}

void drawable_blend_set(Drawable drawable, Blend src_rgb, Blend dst_rgb, Blend src_alpha, Blend dst_alpha) {
    drawable->blend_src_rgb = src_rgb;
    drawable->blend_dst_rgb = dst_rgb;
    drawable->blend_src_alpha = src_alpha;
    drawable->blend_dst_alpha = dst_alpha;
}
