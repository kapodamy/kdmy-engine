#include "sprite.h"
#include "externals/luascript.h"
#include "linkedlist.h"
#include "malloc_utils.h"
#include "map.h"
#include "pvrcontext.h"
#include "stringutils.h"
#include "vertexprops.h"


typedef struct {
    float sx, sy, sw, sh, dx, dy, dw, dh, alpha, r, g, b;
} Trail;

typedef struct {
    float x;
    float y;
    float width;
    float height;
} CropInfo;

struct Sprite_s {
    bool flip_x;
    bool flip_y;
    bool flip_correction;
    CornerRotation matrix_corner;
    Modifier matrix_source;
    float src_x;
    float src_y;
    RGBA vertex_color;
    RGBA offsetcolor;
    Texture texture;
    float src_width;
    float src_height;
    float draw_x;
    float draw_y;
    float draw_width;
    float draw_height;
    float alpha;
    float alpha2;
    bool visible;
    float z_index;
    float z_offset;
    float frame_x;
    float frame_y;
    float frame_width;
    float frame_height;
    float pivot_x;
    float pivot_y;
    int32_t id;
    LinkedList animation_list;
    AnimSprite animation_selected;
    AnimSprite animation_external;

    float vertex[8];
    bool vertex_dirty;

    CropInfo crop;
    bool crop_enabled;

    PVRFlag antialiasing;

    // PSShader psshader;

    bool blend_enabled;
    Blend blend_src_rgb;
    Blend blend_dst_rgb;
    Blend blend_src_alpha;
    Blend blend_dst_alpha;

    Trail* trailing_buffer;
    int32_t trailing_used;
    int32_t trailing_length;
    float trailing_alpha;
    float trailing_delay;
    float trailing_progress;
    bool trailing_darken;
    bool trailing_disabled;
    RGBA trailing_offsetcolor;
};


static int32_t SPRITE_IDS = 0;
static Map SPRITE_POOL = NULL;


void __attribute__((constructor)) __ctor_sprite() {
    SPRITE_POOL = map_init();
}


void sprite_draw(Sprite sprite, PVRContext pvrctx) {
    if (sprite->draw_width < 1.0f || sprite->draw_height < 1.0f) return;
    if (sprite->alpha <= 0.0f) return;

    float* sprite_vertex = sprite->vertex;
    float render_alpha = sprite->alpha * sprite->alpha2;

    float draw_x = sprite->draw_x;
    float draw_y = sprite->draw_y;

    if (sprite->vertex_dirty) {
        float draw_width = sprite->draw_width;
        float draw_height = sprite->draw_height;

        // flip vertex (if required)
        if (sprite->flip_x) {
            if (sprite->flip_correction) draw_x += draw_width;
            draw_width = -draw_width;
        }
        if (sprite->flip_y) {
            if (sprite->flip_correction) draw_y += draw_height;
            draw_height = -draw_height;
        }

        if (sprite->texture) {
            float frame_width, frame_height;
            float crop_x, crop_y, crop_width, crop_height;
            float ratio_width, ratio_height;

            // complex frame size redimension
            if (sprite->frame_width > 0.0f) {
                ratio_width = draw_width / sprite->frame_width;
                frame_width = sprite->src_width * ratio_width;
            } else {
                ratio_width = draw_width / sprite->src_width;
                frame_width = draw_width;
            }
            if (sprite->frame_height > 0.0f) {
                ratio_height = draw_height / sprite->frame_height;
                frame_height = sprite->src_height * ratio_height;
            } else {
                ratio_height = draw_height / sprite->src_height;
                frame_height = draw_height;
            }

            // calculate cropping (if required)
            if (sprite->crop_enabled) {
                if (sprite->crop.x >= frame_width || sprite->crop.y >= frame_height)
                    return;

                crop_x = sprite->crop.x;
                crop_y = sprite->crop.y;
                crop_width = crop_height = 0.0f;

                if (sprite->crop.width != -1.0f && sprite->crop.width < frame_width)
                    crop_width = frame_width - sprite->crop.width;

                if (sprite->crop.height != -1.0f && sprite->crop.height < frame_height)
                    crop_height = frame_height - sprite->crop.height;
            } else {
                crop_x = crop_y = crop_width = crop_height = 0.0f;
            }

            // draw location & size
            sprite_vertex[4] = draw_x - ((sprite->pivot_x + sprite->frame_x) * ratio_width);
            sprite_vertex[5] = draw_y - ((sprite->pivot_y + sprite->frame_y) * ratio_height);
            sprite_vertex[6] = frame_width - crop_width - crop_x;
            sprite_vertex[7] = frame_height - crop_height - crop_y;

            if (sprite->crop_enabled) {
                crop_x /= ratio_width;
                crop_y /= ratio_height;
                crop_width /= ratio_width;
                crop_height /= ratio_height;
            }

            // source location & size
            sprite_vertex[0] = sprite->src_x + crop_x;
            sprite_vertex[1] = sprite->src_y + crop_y;
            sprite_vertex[2] = sprite->src_width - crop_width - crop_x;
            sprite_vertex[3] = sprite->src_height - crop_height - crop_y;
        } else {
            sprite_vertex[4] = draw_x;
            sprite_vertex[5] = draw_y;
            sprite_vertex[6] = draw_width;
            sprite_vertex[7] = draw_height;

            if (sprite->crop_enabled) {
                float crop_width = sprite->crop.width;
                float crop_height = sprite->crop.height;

                sprite_vertex[4] += sprite->crop.x;
                sprite_vertex[5] += sprite->crop.y;
                sprite_vertex[6] -= sprite->crop.x;
                sprite_vertex[7] -= sprite->crop.y;

                if (crop_width != -1.0f && crop_width < sprite_vertex[6])
                    sprite_vertex[6] = crop_width;
                if (crop_height != -1.0f && crop_height < sprite_vertex[7])
                    sprite_vertex[7] = crop_height;
            }
        }

        // cache the calculated vertex
        sprite->vertex_dirty = false;
    }

    pvr_context_save(pvrctx);
    // if (sprite->psshader) pvr_context_add_shader(pvrctx, sprite->psshader);

    pvr_context_set_vertex_blend(
        pvrctx,
        sprite->blend_enabled,
        sprite->blend_src_rgb,
        sprite->blend_dst_rgb,
        sprite->blend_src_alpha,
        sprite->blend_dst_alpha
    );

    if (sprite->antialiasing != PVRCTX_FLAG_DEFAULT) {
        pvr_context_set_vertex_antialiasing(pvrctx, sprite->antialiasing);
    }

    float* matrix = pvrctx->current_matrix;

    // apply self modifier
    sh4matrix_apply_modifier2(
        matrix, &sprite->matrix_source, draw_x, draw_y, sprite->draw_width, sprite->draw_height
    );

    // do corner rotation (if required)
    if (sprite->matrix_corner.angle != 0.0f) {
        sh4matrix_corner_rotate(
            matrix, sprite->matrix_corner,
            draw_x, draw_y, sprite->draw_width, sprite->draw_height
        );
    }

    // draw sprites trail if necessary
    if (!sprite->trailing_disabled && sprite->trailing_used > 0) {
        RGBA trailing_offsetcolor = {
            sprite->trailing_offsetcolor[0], sprite->trailing_offsetcolor[1], sprite->trailing_offsetcolor[2], 1.0f
        };
        if (sprite->offsetcolor[3] >= 0.0f) {
            trailing_offsetcolor[0] *= sprite->offsetcolor[0];
            trailing_offsetcolor[1] *= sprite->offsetcolor[1];
            trailing_offsetcolor[2] *= sprite->offsetcolor[2];
            trailing_offsetcolor[3] *= sprite->offsetcolor[3];
        }

        pvr_context_set_vertex_offsetcolor(pvrctx, trailing_offsetcolor);
        pvr_context_set_vertex_textured_darken(pvrctx, sprite->trailing_darken);

        for (int32_t i = 0; i < sprite->trailing_used; i++) {
            Trail* trail = &sprite->trailing_buffer[i];
            pvr_context_set_vertex_alpha(pvrctx, trail->alpha * render_alpha);

            if (sprite->texture) {
                pvr_context_draw_texture(
                    pvrctx,
                    sprite->texture,
                    trail->sx, trail->sy, trail->sw, trail->sh,
                    trail->dx, trail->dy, trail->dw, trail->dh
                );
            } else {
                pvr_context_draw_solid_color(
                    pvrctx,
                    sprite->vertex_color,
                    trail->dx, trail->dy, trail->dw, trail->dh
                );
            }
        }

        // restore previous values
        pvr_context_set_vertex_textured_darken(pvrctx, false);
    }

    pvr_context_set_vertex_alpha(pvrctx, render_alpha);
    pvr_context_set_vertex_offsetcolor(pvrctx, sprite->offsetcolor);

    // draw the vertex
    if (sprite->texture) {
        // upload texture to the PVR VRAM, if was not loaded yet
        texture_upload_to_pvr(sprite->texture);

        // textured vertex
        pvr_context_draw_texture(
            pvrctx,
            sprite->texture,
            sprite_vertex[0], sprite_vertex[1], // source location
            sprite_vertex[2], sprite_vertex[3], // source size

            sprite_vertex[4], sprite_vertex[5], // draw location
            sprite_vertex[6], sprite_vertex[7]  // draw size
        );
    } else {
        // un-textured vertex
        pvr_context_draw_solid_color(
            pvrctx, sprite->vertex_color,
            sprite_vertex[4], sprite_vertex[5], // draw location
            sprite_vertex[6], sprite_vertex[7]  // draw size
        );
    }

    pvr_context_restore(pvrctx);
}

Sprite sprite_init_from_rgb8(uint32_t solid_rgb8_color) {
    float percent_color[3] = {0.0f, 0.0f, 0.0f};
    math2d_color_bytes_to_floats(solid_rgb8_color, false, percent_color);

    Sprite sprite = sprite_init(NULL);
    sprite_set_vertex_color(sprite, percent_color[0], percent_color[1], percent_color[2]);

    return sprite;
}

Sprite sprite_init(Texture src_texture) {
    Sprite sprite = malloc_chk(sizeof(struct Sprite_s));
    malloc_assert(sprite, Sprite);

    sprite->flip_x = false;
    sprite->flip_y = false;
    sprite->flip_correction = true;
    sprite->matrix_corner = (CornerRotation){.x = 0.0f, .y = 0.0f, .angle = 0.0f};

    pvr_context_helper_clear_modifier(&sprite->matrix_source);

    sprite->src_x = 0.0f;
    sprite->src_y = 0.0f;

    sprite->vertex_color[0] = 1.0f;
    sprite->vertex_color[1] = 1.0f;
    sprite->vertex_color[2] = 1.0f;

    pvr_context_helper_clear_offsetcolor(sprite->offsetcolor);

    sprite->texture = src_texture;
    if (src_texture) {
        sprite->src_width = sprite->texture->original_width;
        sprite->src_height = sprite->texture->original_height;
    } else {
        sprite->src_width = 0.0f;
        sprite->src_height = 0.0f;
    }

    sprite->draw_x = 0.0f;
    sprite->draw_y = 0.0f;
    sprite->draw_width = -1.0f;
    sprite->draw_height = -1.0f;

    sprite->alpha = 1.0f;
    sprite->alpha2 = 1.0f;
    sprite->visible = true;
    sprite->z_index = 1.0;
    sprite->z_offset = 0.0f;

    sprite->frame_x = 0.0f;
    sprite->frame_y = 0.0f;
    sprite->frame_width = 0.0f;
    sprite->frame_height = 0.0f;

    sprite->pivot_x = 0.0f;
    sprite->pivot_y = 0.0f;

    sprite->id = SPRITE_IDS++;
    map_add(SPRITE_POOL, sprite->id, sprite);

    sprite->animation_list = linkedlist_init();
    sprite->animation_selected = NULL;
    sprite->animation_external = NULL;

    memset(sprite->vertex, 0x00, sizeof(sprite->vertex));
    sprite->vertex_dirty = true;

    sprite->crop = (CropInfo){.x = 0.0f, .y = 0.0f, .width = -1.0f, .height = -1.0f};
    sprite->crop_enabled = false;

    sprite->antialiasing = PVRCTX_FLAG_DEFAULT;

    // sprite->psshader = NULL;

    sprite->blend_enabled = true; // obligatory
    sprite->blend_src_rgb = BLEND_DEFAULT;
    sprite->blend_dst_rgb = BLEND_DEFAULT;
    sprite->blend_src_alpha = BLEND_DEFAULT;
    sprite->blend_dst_alpha = BLEND_DEFAULT;

    sprite->trailing_buffer = malloc_for_array(Trail, 10);
    sprite->trailing_used = 0;
    sprite->trailing_length = 10;
    sprite->trailing_alpha = 0.9f;
    sprite->trailing_delay = 0.0f;
    sprite->trailing_darken = true;
    sprite->trailing_disabled = true;
    sprite->trailing_progress = 0.0f;
    sprite->trailing_offsetcolor[0] = 1.0f;
    sprite->trailing_offsetcolor[1] = 1.0f;
    sprite->trailing_offsetcolor[2] = 1.0f;

    return sprite;
}

void sprite_destroy(Sprite* sprite_ptr) {
    Sprite sprite = *sprite_ptr;
    if (!sprite) return;

    linkedlist_destroy(&sprite->animation_list);

    sprite->animation_list = NULL;
    sprite->animation_external = NULL;
    sprite->texture = NULL;

    map_delete(SPRITE_POOL, sprite->id);
    luascript_drop_shared(&sprite->matrix_source);
    luascript_drop_shared(sprite);
    free_chk(sprite->trailing_buffer);

    free_chk(sprite);
    *sprite_ptr = NULL;
}

void sprite_destroy_all_animations(Sprite sprite) {
    if (sprite->animation_list) {

        foreach (AnimSprite, animsprite, LINKEDLIST_ITERATOR, sprite->animation_list)
            if (animsprite) animsprite_destroy(&animsprite);

        linkedlist_destroy(&sprite->animation_list);
        sprite->animation_list = linkedlist_init();
        sprite->animation_selected = NULL;
    }

    if (sprite->animation_external) {
        animsprite_destroy(&sprite->animation_external);
        sprite->animation_external = NULL;
    }
}

void sprite_destroy_texture(Sprite sprite) {
    if (sprite->texture) {
        texture_destroy(&sprite->texture);
        sprite->texture = NULL;
    }
}

void sprite_destroy_full(Sprite* sprite_ptr) {
    if (!*sprite_ptr) return;
    sprite_destroy_texture(*sprite_ptr);
    sprite_destroy_all_animations(*sprite_ptr);
    sprite_destroy(sprite_ptr);
}

Texture sprite_set_texture(Sprite sprite, Texture texture, bool update_offset_source_size) {
    Texture old_texture = sprite->texture;
    sprite->texture = texture;

    if (update_offset_source_size && texture) {
        sprite->src_x = 0.0;
        sprite->src_y = 0.0;
        sprite->src_width = sprite->texture->original_width;
        sprite->src_height = sprite->texture->original_height;
        sprite->vertex_dirty = true;
    }

    return old_texture;
}

Modifier* sprite_matrix_get_modifier(Sprite sprite) {
    return &sprite->matrix_source;
}

void sprite_matrix_scale(Sprite sprite, float scale_x, float scale_y) {
    if (!math2d_is_float_NaN(scale_x)) sprite->matrix_source.scale_x = scale_x;
    if (!math2d_is_float_NaN(scale_y)) sprite->matrix_source.scale_y = scale_y;
}

void sprite_matrix_rotate(Sprite sprite, float degrees_angle) {
    sprite->matrix_source.rotate = degrees_angle * MATH2D_DEG_TO_RAD;
}

void sprite_matrix_skew(Sprite sprite, float skew_x, float skew_y) {
    if (!math2d_is_float_NaN(skew_x)) sprite->matrix_source.skew_x = skew_x;
    if (!math2d_is_float_NaN(skew_y)) sprite->matrix_source.skew_y = skew_y;
}

void sprite_matrix_translate(Sprite sprite, float translate_x, float translate_y) {
    if (!math2d_is_float_NaN(translate_x)) sprite->matrix_source.translate_x = translate_x;
    if (!math2d_is_float_NaN(translate_y)) sprite->matrix_source.translate_y = translate_y;
}

void sprite_set_offset_source(Sprite sprite, float x, float y, float width, float height) {
    sprite->src_x = x;
    sprite->src_y = y;
    sprite->src_width = width;
    sprite->src_height = height;

    sprite->vertex_dirty = true;

    if (sprite->draw_width < 0.0f) sprite->draw_width = width;
    if (sprite->draw_height < 0.0f) sprite->draw_height = height;
}

void sprite_set_offset_frame(Sprite sprite, float x, float y, float width, float height) {
    sprite->frame_x = x;
    sprite->frame_y = y;
    sprite->frame_width = width;
    sprite->frame_height = height;

    sprite->vertex_dirty = true;

    if (sprite->draw_width < 0.0f) sprite->draw_width = width;
    if (sprite->draw_height < 0.0f) sprite->draw_height = height;
}

void sprite_set_offset_pivot(Sprite sprite, float x, float y) {
    sprite->pivot_x = x;
    sprite->pivot_y = y;

    sprite->vertex_dirty = true;
}

void sprite_flip_rendered_texture(Sprite sprite, nbool flip_x, nbool flip_y) {
    if (flip_x != unset) sprite->flip_x = flip_x;
    if (flip_y != unset) sprite->flip_y = flip_y;
    sprite->vertex_dirty = true;
}

void sprite_flip_rendered_texture_enable_correction(Sprite sprite, bool enabled) {
    sprite->flip_correction = enabled;
}

void sprite_matrix_reset(Sprite sprite) {
    pvr_context_helper_clear_modifier(&sprite->matrix_source);
    sprite->flip_x = false;
    sprite->flip_y = false;
    sprite->matrix_corner = (CornerRotation){.x = 0.0f, .y = 0.0f, .angle = 0.0f};
}

void sprite_set_draw_location(Sprite sprite, float x, float y) {
    sprite->draw_x = x;
    sprite->draw_y = y;
    sprite->vertex_dirty = true;
}

void sprite_set_draw_size(Sprite sprite, float width, float height) {
    if (!math2d_is_float_NaN(width)) sprite->draw_width = width;
    if (!math2d_is_float_NaN(height)) sprite->draw_height = height;
    sprite->vertex_dirty = true;

    if (!sprite->texture) {
        if (!math2d_is_float_NaN(width)) sprite->src_width = width;
        if (!math2d_is_float_NaN(height)) sprite->src_height = height;
    }
}

void sprite_set_draw_size_from_source_size(Sprite sprite) {
    sprite->draw_width = sprite->src_width;
    sprite->draw_height = sprite->src_height;
    sprite->vertex_dirty = true;
}

void sprite_set_alpha(Sprite sprite, float alpha) {
    sprite->alpha = alpha;
}

float sprite_get_alpha(Sprite sprite) {
    return sprite->alpha * sprite->alpha2;
}

void sprite_set_visible(Sprite sprite, bool visible) {
    sprite->visible = visible;
}

void sprite_set_z_index(Sprite sprite, float index) {
    sprite->z_index = index;
}

void sprite_set_z_offset(Sprite sprite, float offset) {
    sprite->z_offset = offset;
}

float sprite_get_z_index(Sprite sprite) {
    return sprite->z_index + sprite->z_offset;
}

void sprite_animation_add(Sprite sprite, AnimSprite animsprite) {
    if (!animsprite) return;

    // ¿should duplicate first?
    linkedlist_add_item(sprite->animation_list, animsprite);
}

void sprite_animation_remove(Sprite sprite, const char* animation_name) {
    if (animation_name == NULL) {
        linkedlist_destroy(&sprite->animation_list);
        sprite->animation_list = linkedlist_init();
        return;
    }

    int32_t i = 0;
    foreach (AnimSprite, anim, LINKEDLIST_ITERATOR, sprite->animation_list) {
        if (string_equals(animsprite_get_name(anim), animation_name)) {
            linkedlist_remove_item_at(sprite->animation_list, i);
            return;
        }
        i++;
    }
}

void sprite_animation_restart(Sprite sprite) {
    if (!sprite->animation_selected) return;
    animsprite_restart(sprite->animation_selected);
    animsprite_update_sprite(sprite->animation_selected, sprite, true);
}

bool sprite_animation_play(Sprite sprite, const char* animation_name) {
    // sprite->animation_selected = NULL;
    if (!animation_name) return false;

    AnimSprite animation = sprite_animation_get_attached(sprite, animation_name);

    if (!animation) {
        logger_warn("sprite_animation_play() animation no added: %s", animation_name);
        return false;
    }

    sprite->animation_selected = animation;
    return true;
}

void sprite_animation_play_by_index(Sprite sprite, int32_t index) {
    // sprite->animation_selected = NULL;

    if (index < 0 || index >= linkedlist_count(sprite->animation_list)) {
        logger_warn("sprite_animation_play_by_index() rejected, index was out-of-bounds");
        return;
    }

    sprite->animation_selected = linkedlist_get_by_index(sprite->animation_list, index);
}

bool sprite_animation_play_by_animsprite(Sprite sprite, AnimSprite animsprite, bool only_if_attached) {
    if (!animsprite) {
        if (!only_if_attached)
            sprite->animation_selected = NULL;

        return false;
    }

    if (!only_if_attached) {
        //
        // Warning:
        //         using this way, the animation won't be disposed
        //         calling sprite_destroy_all_animations() or sprite_destroy_full()
        //
        sprite->animation_selected = animsprite;
        return true;
    }

    foreach (AnimSprite, attached_animsprite, LINKEDLIST_ITERATOR, sprite->animation_list) {
        if (attached_animsprite == animsprite) {
            sprite->animation_selected = attached_animsprite;
            return true;
        }
    }

    return false;
}

void sprite_animation_play_first(Sprite sprite) {
    // if there no animations on the list the "animation_selected" will be NULL.
    // This means no animation selected
    sprite->animation_selected = linkedlist_get_first_item(sprite->animation_list);
}

void sprite_animation_stop(Sprite sprite) {
    if (sprite->animation_selected) animsprite_stop(sprite->animation_selected);
}

void sprite_animation_end(Sprite sprite) {
    if (sprite->animation_selected) {
        animsprite_force_end(sprite->animation_selected);
        animsprite_update_sprite(sprite->animation_selected, sprite, true);
    }
}

int32_t sprite_animate(Sprite sprite, float elapsed) {
    math2d_float_assert_NaN(elapsed);

    int32_t result = 1;

    if (sprite->animation_selected) {
        result = animsprite_animate(sprite->animation_selected, elapsed);
        animsprite_update_sprite(sprite->animation_selected, sprite, true);
    }

    if (sprite->animation_external) {
        result = animsprite_animate(sprite->animation_external, elapsed);
        animsprite_update_sprite(sprite->animation_external, sprite, false);
    }

    if (sprite->trailing_disabled) return result;

    // check delay for next trail
    bool wait = sprite->trailing_progress < sprite->trailing_delay;
    sprite->trailing_progress += elapsed;

    if (wait) return result;
    sprite->trailing_progress -= sprite->trailing_delay;

    // compute trailing using the cached vertex
    bool insert_vertex = true;
    if (sprite->trailing_used > 0) {
        // check if the last vertex equals to the current vertex
        Trail* trail = &sprite->trailing_buffer[0];
        bool source = trail->sx == sprite->vertex[0] && trail->sy == sprite->vertex[1] && trail->sw == sprite->vertex[2] && trail->sh == sprite->vertex[3];
        bool draw = trail->dx == sprite->vertex[4] && trail->dy == sprite->vertex[5] && trail->dw == sprite->vertex[6] && trail->dh == sprite->vertex[7];
        bool color = sprite->vertex_color[0] == trail->r && sprite->vertex_color[1] == trail->g && sprite->vertex_color[2] == trail->b;
        insert_vertex = !source || !draw || !color;

        if (insert_vertex) {
            // do right shift (this should be optimized to shift only used trails)
            for (int32_t i = sprite->trailing_length - 1, j = sprite->trailing_length - 2; j >= 0; j--, i--) {
                sprite->trailing_buffer[j] = sprite->trailing_buffer[i];
            }
        } else {
            sprite->trailing_used--;
        }
    }

    if (insert_vertex) {
        // add new trail
        Trail* trail = &sprite->trailing_buffer[0];
        trail->sx = sprite->vertex[0];
        trail->sy = sprite->vertex[1];
        trail->sw = sprite->vertex[2];
        trail->sh = sprite->vertex[3];
        trail->dx = sprite->vertex[4];
        trail->dy = sprite->vertex[5];
        trail->dw = sprite->vertex[6];
        trail->dh = sprite->vertex[7];
        trail->r = sprite->vertex_color[0];
        trail->g = sprite->vertex_color[1];
        trail->b = sprite->vertex_color[2];
        if (sprite->trailing_used < sprite->trailing_length) sprite->trailing_used++;
    }

    // update alpha of each trail
    for (int32_t i = 0; i < sprite->trailing_used; i++) {
        sprite->trailing_buffer[i].alpha = (1.0f - (i / sprite->trailing_used)) * sprite->trailing_alpha;
    }

    return result;
}

AnimSprite sprite_animation_get_attached(Sprite sprite, const char* animation_name) {
    if (linkedlist_count(sprite->animation_list) < 1) {
        logger_warn("sprite_animation_get_attached() no animation attached, can not play: %s", animation_name);
        return NULL;
    }

    foreach (AnimSprite, anim, LINKEDLIST_ITERATOR, sprite->animation_list) {
        if (animation_name == NULL || string_equals(animsprite_get_name(anim), animation_name))
            return anim;
    }

    return NULL;
}


void sprite_get_source_size(Sprite sprite, float* width, float* height) {
    *width = sprite->frame_width > 0.0f ? sprite->frame_width : sprite->src_width;
    *height = sprite->frame_height > 0.0f ? sprite->frame_height : sprite->src_height;
}

void sprite_get_draw_size(Sprite sprite, float* width, float* height) {
    *width = sprite->draw_width >= 0.0f ? sprite->draw_width : sprite->src_width;
    *height = sprite->draw_height >= 0.0f ? sprite->draw_height : sprite->src_height;
}

void sprite_get_draw_location(Sprite sprite, float* x, float* y) {
    *x = sprite->draw_x;
    *y = sprite->draw_y;
}


void sprite_set_property(Sprite sprite, int32_t property_id, float value) {
    switch (property_id) {
        case SPRITE_PROP_X:
            sprite->draw_x = value;
            sprite->vertex_dirty = true;
            break;
        case SPRITE_PROP_Y:
            sprite->draw_y = value;
            sprite->vertex_dirty = true;
            break;
        case SPRITE_PROP_WIDTH:
            sprite->draw_width = value;
            sprite->vertex_dirty = true;
            break;
        case SPRITE_PROP_HEIGHT:
            sprite->draw_height = value;
            sprite->vertex_dirty = true;
            break;
        case SPRITE_PROP_ROTATE:
            sprite_matrix_rotate(sprite, value);
            break;
        case SPRITE_PROP_SCALE_X:
            sprite_matrix_scale(sprite, value, FLOAT_NaN);
            break;
        case SPRITE_PROP_SCALE_Y:
            sprite_matrix_scale(sprite, FLOAT_NaN, value);
            break;
        case SPRITE_PROP_SKEW_X:
            sprite_matrix_skew(sprite, value, FLOAT_NaN);
            break;
        case SPRITE_PROP_SKEW_Y:
            sprite_matrix_skew(sprite, FLOAT_NaN, value);
            break;
        case SPRITE_PROP_TRANSLATE_X:
            sprite_matrix_translate(sprite, value, FLOAT_NaN);
            break;
        case SPRITE_PROP_TRANSLATE_Y:
            sprite_matrix_translate(sprite, FLOAT_NaN, value);
            break;
        case SPRITE_PROP_ALPHA:
            sprite->alpha = math2d_clamp_float(value, 0.0f, 1.0f);
            break;
        case SPRITE_PROP_Z:
            sprite->z_index = value;
            break;
        case SPRITE_PROP_VERTEX_COLOR_R:
            sprite_set_vertex_color(sprite, value, -1.0f, -1.0f);
            break;
        case SPRITE_PROP_VERTEX_COLOR_G:
            sprite_set_vertex_color(sprite, -1.0f, value, -1.0f);
            break;
        case SPRITE_PROP_VERTEX_COLOR_B:
            sprite_set_vertex_color(sprite, -1.0f, -1.0f, value);
            break;
        case SPRITE_PROP_VERTEX_COLOR_OFFSET_R:
            sprite->offsetcolor[0] = value;
            break;
        case SPRITE_PROP_VERTEX_COLOR_OFFSET_G:
            sprite->offsetcolor[1] = value;
            break;
        case SPRITE_PROP_VERTEX_COLOR_OFFSET_B:
            sprite->offsetcolor[2] = value;
            break;
        case SPRITE_PROP_VERTEX_COLOR_OFFSET_A:
            sprite->offsetcolor[3] = value;
            break;
        case SPRITE_PROP_ANIMATIONLOOP:
            if (sprite->animation_selected)
                animsprite_set_loop(sprite->animation_selected, (int32_t)value);
            break;
        case SPRITE_PROP_ROTATE_PIVOT_ENABLED:
            sprite_matrix_rotate_pivot_enable(sprite, value >= 1.0f);
            break;
        case SPRITE_PROP_ROTATE_PIVOT_U:
            sprite_matrix_rotate_pivot(sprite, value, FLOAT_NaN);
            break;
        case SPRITE_PROP_ROTATE_PIVOT_V:
            sprite_matrix_rotate_pivot(sprite, FLOAT_NaN, value);
            break;
        case SPRITE_PROP_SCALE_DIRECTION_X:
            sprite->matrix_source.scale_direction_x = value;
            break;
        case SPRITE_PROP_SCALE_DIRECTION_Y:
            sprite->matrix_source.scale_direction_y = value;
            break;
        case SPRITE_PROP_TRANSLATE_ROTATION:
            sprite->matrix_source.translate_rotation = value >= 1.0f;
            break;
        case SPRITE_PROP_SCALE_SIZE:
            sprite->matrix_source.scale_size = value >= 1.0f;
            break;
        case SPRITE_PROP_SCALE_TRANSLATION:
            sprite->matrix_source.scale_translation = value >= 1.0f;
            break;
        case SPRITE_PROP_Z_OFFSET:
            sprite->z_offset = value;
            break;
        case SPRITE_PROP_FLIP_X:
            sprite->flip_x = value >= 1.0f;
            sprite->vertex_dirty = true;
            break;
        case SPRITE_PROP_FLIP_Y:
            sprite->flip_y = value >= 1.0f;
            sprite->vertex_dirty = true;
            break;
        case SPRITE_PROP_ANTIALIASING:
            sprite->antialiasing = (PVRFlag)(int)value;
            break;
        case SPRITE_PROP_FLIP_CORRECTION:
            sprite->flip_correction = value >= 1.0f;
            sprite->vertex_dirty = true;
            break;
        case SPRITE_PROP_ALPHA2:
            sprite->alpha2 = value;
            break;
    }
}


void sprite_set_vertex_color_rgb8(Sprite sprite, uint32_t rbg8_color) {
    math2d_color_bytes_to_floats(rbg8_color, false, sprite->vertex_color);
}

void sprite_set_vertex_color_rgba8(Sprite sprite, uint32_t rbga8_color) {
    sprite_set_vertex_color_rgb8(sprite, rbga8_color >> 8);
    sprite->alpha = (rbga8_color & 0xFF) / 255.0f;
}

void sprite_set_vertex_color(Sprite sprite, float r, float g, float b) {
    if (r >= 0.0f) sprite->vertex_color[0] = r;
    if (g >= 0.0f) sprite->vertex_color[1] = g;
    if (b >= 0.0f) sprite->vertex_color[2] = b;
}

uint32_t sprite_get_vertex_color_rgb8(Sprite sprite) {
    return math2d_color_floats_to_bytes(sprite->vertex_color, false);
}


void sprite_set_offsetcolor_rgba8(Sprite sprite, uint32_t rgba8_color) {
    math2d_color_bytes_to_floats(rgba8_color, true, sprite->offsetcolor);
}

void sprite_set_offsetcolor(Sprite sprite, float r, float g, float b, float a) {
    if (r >= 0.0f) sprite->offsetcolor[0] = r;
    if (g >= 0.0f) sprite->offsetcolor[1] = g;
    if (b >= 0.0f) sprite->offsetcolor[2] = b;
    if (a >= 0.0f) sprite->offsetcolor[3] = a;
}


AnimSprite sprite_external_animation_set(Sprite sprite, AnimSprite animsprite) {
    AnimSprite old_animation = sprite->animation_external;
    sprite->animation_external = animsprite;
    return old_animation;
}

void sprite_external_animation_restart(Sprite sprite) {
    if (!sprite->animation_external) return;
    animsprite_restart(sprite->animation_external);
    animsprite_update_sprite(sprite->animation_external, sprite, false);
}

void sprite_external_animation_end(Sprite sprite) {
    if (sprite->animation_external)
        animsprite_force_end2(sprite->animation_external, sprite);
}


void sprite_resize_draw_size(Sprite sprite, float max_width, float max_height, float* applied_width, float* applied_height) {
    float orig_width = 0.0f, orig_height = 0.0f;
    sprite_get_source_size(sprite, &orig_width, &orig_height);

    float width = 0.0f, height = 0.0f;

    if (max_width < 1.0f && max_height < 1.0f) {
        width = orig_width;
        height = orig_height;
    } else if (orig_width == 0.0f && orig_height == 0.0f) {
        width = max_width < 0.0f ? max_height : max_width;
        height = max_height < 0.0f ? max_width : max_height;
    } else if (max_width == 0.0f || max_height == 0.0f) {
        width = height = 0.0f;
    } else {
        if (max_width > 0.0f && max_height > 0.0f) {
            float scale_x = max_width / orig_width;
            float scale_y = max_height / orig_height;

            if (scale_x > scale_y)
                max_width = -FLOAT_Inf;
            else
                max_height = -FLOAT_Inf;
        }

        if (max_height > 0.0f) {
            height = max_height;
            width = (orig_width * max_height) / orig_height;
        }

        if (max_width > 0.0f) {
            height = (orig_height * max_width) / orig_width;
            width = max_width;
        }
    }

    sprite->draw_width = width;
    sprite->draw_height = height;
    sprite->vertex_dirty = true;

    if (applied_width) *applied_width = width;
    if (applied_height) *applied_height = height;
}

void sprite_center_draw_location(Sprite sprite, float x, float y, float ref_width, float ref_height, float* applied_x, float* applied_y) {
    float draw_width = 0.0f, draw_height = 0.0f;
    sprite_get_draw_size(sprite, &draw_width, &draw_height);

    if (ref_width >= 0.0f) x += ((ref_width - draw_width) / 2.0f);
    if (ref_height >= 0.0f) y += ((ref_height - draw_height) / 2.0f);

    sprite->draw_x = x;
    sprite->draw_y = y;
    sprite->vertex_dirty = true;

    if (applied_x) *applied_x = sprite->draw_x;
    if (applied_y) *applied_y = sprite->draw_y;
}


bool sprite_is_textured(Sprite sprite) {
    return sprite->texture ? true : false;
}

bool sprite_is_visible(Sprite sprite) {
    return sprite->visible;
}


void sprite_matrix_rotate_pivot_enable(Sprite sprite, bool enable) {
    sprite->matrix_source.rotate_pivot_enabled = enable;
}

void sprite_matrix_rotate_pivot(Sprite sprite, float u, float v) {
    if (!math2d_is_float_NaN(u)) sprite->matrix_source.rotate_pivot_u = u;
    if (!math2d_is_float_NaN(v)) sprite->matrix_source.rotate_pivot_v = v;
}

void sprite_matrix_scale_offset(Sprite sprite, float direction_x, float direction_y) {
    if (!math2d_is_float_NaN(direction_x)) sprite->matrix_source.scale_direction_x = direction_x;
    if (!math2d_is_float_NaN(direction_y)) sprite->matrix_source.scale_direction_y = direction_y;
}

void sprite_matrix_translate_rotation(Sprite sprite, bool enable_translate_rotation) {
    sprite->matrix_source.translate_rotation = enable_translate_rotation;
}

void sprite_matrix_scale_size(Sprite sprite, bool enable_scale_size) {
    sprite->matrix_source.scale_size = enable_scale_size;
}

void sprite_matrix_corner_rotation(Sprite sprite, Corner corner) {
    sh4matrix_helper_calculate_corner_rotation(&sprite->matrix_corner, corner);
}

void sprite_set_draw_x(Sprite sprite, float value) {
    sprite->draw_x = value;
    sprite->vertex_dirty = true;
}

void sprite_set_draw_y(Sprite sprite, float value) {
    sprite->draw_y = value;
    sprite->vertex_dirty = true;
}


bool sprite_crop(Sprite sprite, bool dx, bool dy, bool dwidth, bool dheight) {
    if (!math2d_is_float_NaN(dx)) sprite->crop.x = dx;
    if (!math2d_is_float_NaN(dy)) sprite->crop.y = dy;
    if (!math2d_is_float_NaN(dwidth)) sprite->crop.width = dwidth;
    if (!math2d_is_float_NaN(dheight)) sprite->crop.height = dheight;

    bool invalid = sprite->crop.x < 0.0f && sprite->crop.y < 0.0f && sprite->crop.width == 0 && sprite->crop.height == 0.0f;

    // disable cropping if the bounds are invalid
    if (sprite->crop_enabled && invalid) sprite->crop_enabled = false;

    sprite->vertex_dirty = true;
    return !invalid;
}

bool sprite_is_crop_enabled(Sprite sprite) {
    return sprite->crop_enabled;
}

void sprite_crop_enable(Sprite sprite, bool enable) {
    sprite->crop_enabled = enable;
}


void sprite_fill_atlas_entry(Sprite sprite, AtlasEntry* target_atlas_entry) {
    target_atlas_entry->x = sprite->src_x;
    target_atlas_entry->y = sprite->src_y;
    target_atlas_entry->width = sprite->src_width;
    target_atlas_entry->height = sprite->src_height;
    target_atlas_entry->frame_x = sprite->frame_x;
    target_atlas_entry->frame_y = sprite->frame_y;
    target_atlas_entry->frame_width = sprite->frame_width;
    target_atlas_entry->frame_height = sprite->frame_height;
    target_atlas_entry->pivot_x = sprite->pivot_x;
    target_atlas_entry->pivot_y = sprite->pivot_y;
}

Texture sprite_get_texture(Sprite sprite) {
    return sprite->texture;
}

void sprite_set_antialiasing(Sprite sprite, PVRFlag antialiasing) {
    sprite->antialiasing = antialiasing;
}

/*void sprite_set_shader(Sprite sprite, PSShader psshader) {
    sprite->psshader = psshader;
}

PSShader sprite_get_shader(Sprite sprite) {
    return sprite->psshader;
}*/

void sprite_blend_enable(Sprite sprite, bool enabled) {
    sprite->blend_enabled = enabled;
}

void sprite_blend_set(Sprite sprite, Blend src_rgb, Blend dst_rgb, Blend src_alpha, Blend dst_alpha) {
    sprite->blend_src_rgb = src_rgb;
    sprite->blend_dst_rgb = dst_rgb;
    sprite->blend_src_alpha = src_alpha;
    sprite->blend_dst_alpha = dst_alpha;
}

void sprite_trailing_enabled(Sprite sprite, bool enabled) {
    sprite->trailing_disabled = !enabled;
    if (sprite->trailing_disabled) sprite->trailing_used = 0;
}

void sprite_trailing_set_params(Sprite sprite, int32_t length, float trail_delay, float trail_alpha, nbool darken_colors) {
    if (length > 0) {
        sprite->trailing_buffer = realloc_for_array(sprite->trailing_buffer, length, Trail);
        sprite->trailing_length = length;
        if (sprite->trailing_used > length) sprite->trailing_used = length;
    }
    if (darken_colors != unset) sprite->trailing_darken = darken_colors;
    if (!math2d_is_float_NaN(trail_delay)) sprite->trailing_delay = trail_delay;
    if (!math2d_is_float_NaN(trail_alpha)) sprite->trailing_alpha = trail_alpha;

    // force update
    sprite->trailing_progress = sprite->trailing_delay;
}

void sprite_trailing_set_offsetcolor(Sprite sprite, float r, float g, float b) {
    if (!math2d_is_float_NaN(r)) sprite->trailing_offsetcolor[0] = r;
    if (!math2d_is_float_NaN(g)) sprite->trailing_offsetcolor[1] = g;
    if (!math2d_is_float_NaN(b)) sprite->trailing_offsetcolor[2] = b;
}
