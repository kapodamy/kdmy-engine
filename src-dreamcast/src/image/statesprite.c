#include "statesprite.h"
#include "malloc_utils.h"
#include "map.h"
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

struct StateSprite_s {
    float src_x;
    float src_y;
    Modifier matrix_source;
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
    float vertex[8];
    bool vertex_dirty;
    AnimSprite animation_external;
    CornerRotation matrix_corner;
    CropInfo crop;
    bool crop_enabled;
    bool flip_x;
    bool flip_y;
    bool flip_correction;
    LinkedList state_list;
    float offset_x;
    float offset_y;
    StateSpriteState* selected_state;
    float scale_texture;
    PVRFlag antialiasing;
    bool atlas_to_draw_size_enabled;

    // PSShader psshader;

    bool blend_enabled;
    Blend blend_src_rgb;
    Blend blend_dst_rgb;
    Blend blend_src_alpha;
    Blend blend_dst_alpha;

    Trail* trailing_buffer;
    int trailing_used;
    int trailing_length;
    float trailing_alpha;
    float trailing_delay;
    float trailing_progress;
    bool trailing_darken;
    bool trailing_disabled;
    RGBA trailing_offsetcolor;
};

static int32_t STATESPRITE_IDS = 0;
static Map STATESPRITE_POOL = NULL;


void __attribute__((constructor)) __ctor_statesprite() {
    STATESPRITE_POOL = map_init();
}


void statesprite_draw(StateSprite statesprite, PVRContext pvrctx) {
    if (statesprite->draw_width < 1.0f || statesprite->draw_height < 1.0f) return;
    if (statesprite->alpha <= 0.0f) return;

    float* sprite_vertex = statesprite->vertex;
    float render_alpha = statesprite->alpha * statesprite->alpha2;

    // apply offsets from the current state
    float draw_x = statesprite->draw_x + statesprite->offset_x;
    float draw_y = statesprite->draw_y + statesprite->offset_y;

    if (statesprite->vertex_dirty) {
        float draw_width = statesprite->draw_width;
        float draw_height = statesprite->draw_height;

        if (statesprite->flip_x) {
            if (statesprite->flip_correction) draw_x += draw_width;
            draw_width = -draw_width;
        }
        if (statesprite->flip_y) {
            if (statesprite->flip_correction) draw_y += draw_height;
            draw_height = -draw_height;
        }

        if (statesprite->texture) {
            float frame_width, frame_height;
            float crop_x, crop_y, crop_width, crop_height;
            float ratio_width, ratio_height;

            // complex frame size redimension
            if (statesprite->frame_width > 0.0f) {
                ratio_width = draw_width / statesprite->frame_width;
                frame_width = statesprite->src_width * ratio_width;
            } else {
                ratio_width = draw_width / statesprite->src_width;
                frame_width = draw_width;
            }
            if (statesprite->frame_height > 0.0f) {
                ratio_height = draw_height / statesprite->frame_height;
                frame_height = statesprite->src_height * ratio_height;
            } else {
                ratio_height = draw_height / statesprite->src_height;
                frame_height = draw_height;
            }

            // calculate cropping (if required)
            if (statesprite->crop_enabled) {
                if (statesprite->crop.x >= frame_width || statesprite->crop.y >= frame_height)
                    return;

                crop_x = statesprite->crop.x;
                crop_y = statesprite->crop.y;
                crop_width = crop_height = 0.0f;

                if (statesprite->crop.width != -1.0f && statesprite->crop.width < frame_width)
                    crop_width = frame_width - statesprite->crop.width;

                if (statesprite->crop.height != -1.0f && statesprite->crop.height < frame_height)
                    crop_height = frame_height - statesprite->crop.height;
            } else {
                crop_x = crop_y = crop_width = crop_height = 0.0f;
            }

            // draw location & size
            sprite_vertex[4] = draw_x - ((statesprite->pivot_x + statesprite->frame_x) * ratio_width);
            sprite_vertex[5] = draw_y - ((statesprite->pivot_y + statesprite->frame_y) * ratio_height);
            sprite_vertex[6] = frame_width - crop_width - crop_x;
            sprite_vertex[7] = frame_height - crop_height - crop_y;

            if (statesprite->crop_enabled) {
                crop_x /= ratio_width;
                crop_y /= ratio_height;
                crop_width /= ratio_width;
                crop_height /= ratio_height;
            }

            // source location & size
            sprite_vertex[0] = statesprite->src_x + crop_x;
            sprite_vertex[1] = statesprite->src_y + crop_y;
            sprite_vertex[2] = statesprite->src_width - crop_width - crop_x;
            sprite_vertex[3] = statesprite->src_height - crop_height - crop_y;

        } else {
            sprite_vertex[4] = draw_x;
            sprite_vertex[5] = draw_y;
            sprite_vertex[6] = draw_width;
            sprite_vertex[7] = draw_height;

            if (statesprite->crop_enabled) {
                float crop_width = statesprite->crop.width;
                float crop_height = statesprite->crop.height;

                sprite_vertex[4] += statesprite->crop.x;
                sprite_vertex[5] += statesprite->crop.y;
                sprite_vertex[6] -= statesprite->crop.x;
                sprite_vertex[7] -= statesprite->crop.y;

                if (crop_width != -1.0f && crop_width < sprite_vertex[6])
                    sprite_vertex[6] = crop_width;
                if (crop_height != -1.0f && crop_height < sprite_vertex[7])
                    sprite_vertex[7] = crop_height;
            }
        }

        // cache the calculated vertex
        statesprite->vertex_dirty = false;
    }

    pvr_context_save(pvrctx);
    // if (statesprite->psshader) pvr_context_add_shader(pvrctx, statesprite->psshader);

    pvr_context_set_vertex_blend(
        pvrctx,
        statesprite->blend_enabled,
        statesprite->blend_src_rgb,
        statesprite->blend_dst_rgb,
        statesprite->blend_src_alpha,
        statesprite->blend_dst_alpha
    );

    // apply self modifier
    pvr_context_apply_modifier2(pvrctx, &statesprite->matrix_source, draw_x, draw_y, statesprite->draw_width, statesprite->draw_height);

    if (statesprite->antialiasing != PVRCTX_FLAG_DEFAULT) {
        pvr_context_set_vertex_antialiasing(pvrctx, statesprite->antialiasing);
    }

    // do corner rotation (if required)
    if (statesprite->matrix_corner.angle != 0.0f) {
        sh4matrix_corner_rotate(
            pvrctx->current_matrix, statesprite->matrix_corner,
            draw_x, draw_y, statesprite->draw_width, statesprite->draw_height
        );
        pvr_context_flush(pvrctx);
    }

    // draw sprites trail if necessary
    if (!statesprite->trailing_disabled && statesprite->trailing_used > 0) {
        RGBA trailing_offsetcolor = {
            statesprite->trailing_offsetcolor[0], statesprite->trailing_offsetcolor[1], statesprite->trailing_offsetcolor[2], 1.0f
        };
        if (statesprite->trailing_offsetcolor[3] >= 0.0f) {
            trailing_offsetcolor[0] *= statesprite->offsetcolor[0];
            trailing_offsetcolor[1] *= statesprite->offsetcolor[1];
            trailing_offsetcolor[2] *= statesprite->offsetcolor[2];
            trailing_offsetcolor[3] *= statesprite->offsetcolor[3];
        }

        pvr_context_set_vertex_offsetcolor(pvrctx, trailing_offsetcolor);
        pvr_context_set_vertex_textured_darken(pvrctx, statesprite->trailing_darken);

        for (int32_t i = 0; i < statesprite->trailing_used; i++) {
            Trail* trail = &statesprite->trailing_buffer[i];
            pvr_context_set_vertex_alpha(pvrctx, trail->alpha * render_alpha);

            if (statesprite->texture) {
                pvr_context_draw_texture(
                    pvrctx,
                    statesprite->texture,
                    trail->sx, trail->sy, trail->sw, trail->sh,
                    trail->dx, trail->dy, trail->dw, trail->dh
                );
            } else {
                pvr_context_draw_solid_color(
                    pvrctx,
                    statesprite->vertex_color,
                    trail->dx, trail->dy, trail->dw, trail->dh
                );
            }
        }

        // restore previous values
        pvr_context_set_vertex_textured_darken(pvrctx, false);
    }

    pvr_context_set_vertex_alpha(pvrctx, render_alpha);
    pvr_context_set_vertex_offsetcolor(pvrctx, statesprite->offsetcolor);

    // draw the vertex
    if (statesprite->texture) {

        // upload texture to the PVR VRAM, if was not loaded yet
        texture_upload_to_pvr(statesprite->texture);

        // textured vertex
        pvr_context_draw_texture(
            pvrctx,
            statesprite->texture,
            sprite_vertex[0], sprite_vertex[1], // source location
            sprite_vertex[2], sprite_vertex[3], // source size

            sprite_vertex[4], sprite_vertex[5], // draw location
            sprite_vertex[6], sprite_vertex[7]  // draw size
        );
    } else {
        // un-textured vertex
        pvr_context_draw_solid_color(
            pvrctx, statesprite->vertex_color,
            sprite_vertex[4], sprite_vertex[5], // draw location
            sprite_vertex[6], sprite_vertex[7]  // draw size
        );
    }

    pvr_context_restore(pvrctx);
}


StateSprite statesprite_init_from_vertex_color(uint32_t rgb8_color) {
    StateSprite statesprite = statesprite_init_from_texture(NULL);
    statesprite_set_vertex_color_rgb8(statesprite, rgb8_color);
    return statesprite;
}

StateSprite statesprite_init_from_texture(Texture texture) {
    StateSprite statesprite = malloc_chk(sizeof(struct StateSprite_s));
    malloc_assert(statesprite, StateSprite);

    statesprite->src_x = 0.0f;
    statesprite->src_y = 0.0f;

    pvr_context_helper_clear_modifier(&statesprite->matrix_source);

    statesprite->vertex_color[0] = 1.0f;
    statesprite->vertex_color[1] = 1.0f;
    statesprite->vertex_color[2] = 1.0f;

    pvr_context_helper_clear_offsetcolor(statesprite->offsetcolor);

    statesprite->texture = texture;
    if (texture) {
        statesprite->src_width = statesprite->texture->original_width;
        statesprite->src_height = statesprite->texture->original_height;
    } else {
        statesprite->src_width = 0.0f;
        statesprite->src_height = 0.0f;
    }

    statesprite->draw_x = 100.0f;
    statesprite->draw_y = 100.0f;
    statesprite->draw_width = -1.0f;
    statesprite->draw_height = -1.0f;

    statesprite->alpha = 1.0f;
    statesprite->alpha2 = 1.0f;
    statesprite->visible = true;
    statesprite->z_index = 1.0f;
    statesprite->z_offset = 0.0f;

    statesprite->frame_x = 0.0f;
    statesprite->frame_y = 0.0f;
    statesprite->frame_width = 0.0f;
    statesprite->frame_height = 0.0f;
    statesprite->pivot_x = 0.0f;
    statesprite->pivot_y = 0.0f;

    statesprite->id = STATESPRITE_IDS++;
    map_add(STATESPRITE_POOL, statesprite->id, statesprite);

    memset(statesprite->vertex, 0x00, sizeof(statesprite->vertex));
    statesprite->vertex_dirty = true;

    statesprite->animation_external = NULL;

    statesprite->matrix_corner = (CornerRotation){.x = 0.0f, .y = 0.0f, .angle = 0.0f};

    statesprite->crop = (CropInfo){.x = 0.0f, .y = 0.0f, .width = -1.0f, .height = -1.0f};
    statesprite->crop_enabled = false;

    statesprite->flip_x = false;
    statesprite->flip_y = false;
    statesprite->flip_correction = true;

    statesprite->state_list = linkedlist_init();
    statesprite->offset_x = 0.0f;
    statesprite->offset_y = 0.0f;
    statesprite->selected_state = NULL;

    statesprite->scale_texture = 1.0f;
    statesprite->antialiasing = PVRCTX_FLAG_DEFAULT;
    statesprite->atlas_to_draw_size_enabled = false;

    // statesprite->psshader = NULL;

    statesprite->blend_enabled = true; // obligatory
    statesprite->blend_src_rgb = BLEND_DEFAULT;
    statesprite->blend_dst_rgb = BLEND_DEFAULT;
    statesprite->blend_src_alpha = BLEND_DEFAULT;
    statesprite->blend_dst_alpha = BLEND_DEFAULT;

    statesprite->trailing_buffer = malloc_for_array(Trail, 10);
    statesprite->trailing_used = 0;
    statesprite->trailing_length = 10;
    statesprite->trailing_alpha = 0.9f;
    statesprite->trailing_delay = 0.0f;
    statesprite->trailing_darken = true;
    statesprite->trailing_disabled = true;
    statesprite->trailing_progress = 0.0;
    statesprite->trailing_offsetcolor[0] = 1.0f;
    statesprite->trailing_offsetcolor[1] = 1.0f;
    statesprite->trailing_offsetcolor[2] = 1.0f;

    return statesprite;
}

bool statesprite_destroy_texture_if_stateless(StateSprite statesprite) {
    if (!statesprite->texture) return false;

    //
    // If the texture belongs to any state, return.
    // Note: can fail in case of duplicated references (caller problem)
    //
    foreach (StateSpriteState*, state, LINKEDLIST_ITERATOR, statesprite->state_list) {
        if (state->texture && state->texture == statesprite->texture) return false;
    }

    texture_destroy(&statesprite->texture);
    statesprite->texture = NULL;
    return true;
}

void statesprite_destroy(StateSprite* statesprite_ptr) {
    StateSprite statesprite = *statesprite_ptr;
    if (!statesprite) return;

    foreach (StateSpriteState*, state, LINKEDLIST_ITERATOR, statesprite->state_list) {
        if (state->animation) animsprite_destroy(&state->animation);
        if (state->texture) texture_destroy(&state->texture);
        free_chk((char*)state->state_name);
        free_chk(state->frame_info);
        free_chk(state);
    }

    linkedlist_destroy(&statesprite->state_list);

    map_delete(STATESPRITE_POOL, statesprite->id);
    free_chk(statesprite->trailing_buffer);

    free_chk(statesprite);
    *statesprite_ptr = NULL;
}


Texture statesprite_set_texture(StateSprite statesprite, Texture texture, bool update_offset_source_size) {
    Texture old_texture = statesprite->texture;
    statesprite->texture = texture;

    if (update_offset_source_size && texture) {
        statesprite->src_x = 0.0f;
        statesprite->src_y = 0.0f;
        statesprite->src_width = statesprite->texture->original_width;
        statesprite->src_height = statesprite->texture->original_height;
        statesprite->vertex_dirty = true;
    }

    return old_texture;
}

void statesprite_set_offset_source(StateSprite statesprite, float x, float y, float width, float height) {
    statesprite->src_x = x;
    statesprite->src_y = y;
    statesprite->src_width = width;
    statesprite->src_height = height;

    if (statesprite->atlas_to_draw_size_enabled) {
        statesprite->draw_width = width * statesprite->scale_texture;
        statesprite->draw_height = height * statesprite->scale_texture;
    } else {
        if (statesprite->draw_width < 0.0f) statesprite->draw_width = width * statesprite->scale_texture;
        if (statesprite->draw_height < 0.0f) statesprite->draw_height = height * statesprite->scale_texture;
    }

    statesprite->vertex_dirty = true;
}

void statesprite_set_offset_frame(StateSprite statesprite, float x, float y, float width, float height) {
    statesprite->frame_x = x;
    statesprite->frame_y = y;
    statesprite->frame_width = width;
    statesprite->frame_height = height;

    statesprite->vertex_dirty = true;

    if (statesprite->atlas_to_draw_size_enabled) {
        if (width > 0.0f) statesprite->draw_width = width * statesprite->scale_texture;
        if (height > 0.0f) statesprite->draw_height = height * statesprite->scale_texture;
    } else {
        if (statesprite->draw_width < 0.0f && width > 0.0f)
            statesprite->draw_width = width * statesprite->scale_texture;
        if (statesprite->draw_height < 0.0f && height > 0.0f)
            statesprite->draw_height = height * statesprite->scale_texture;
    }
}

void statesprite_set_offset_pivot(StateSprite statesprite, float x, float y) {
    statesprite->pivot_x = x;
    statesprite->pivot_y = y;
    statesprite->vertex_dirty = true;
}


void statesprite_set_draw_location(StateSprite statesprite, float x, float y) {
    statesprite->draw_x = x;
    statesprite->draw_y = y;
    statesprite->vertex_dirty = true;
}

void statesprite_set_draw_size(StateSprite statesprite, float width, float height) {
    statesprite->draw_width = width;
    statesprite->draw_height = height;
    statesprite->vertex_dirty = true;
}

void statesprite_set_alpha(StateSprite statesprite, float alpha) {
    statesprite->alpha = alpha;
}

void statesprite_set_visible(StateSprite statesprite, bool visible) {
    statesprite->visible = visible;
}

void statesprite_set_z_index(StateSprite statesprite, float index) {
    statesprite->z_index = index;
}

void statesprite_set_z_offset(StateSprite statesprite, float offset) {
    statesprite->z_offset = offset;
}

void statesprite_set_property(StateSprite statesprite, int32_t property_id, float value) {
    switch (property_id) {
        case SPRITE_PROP_X:
            statesprite->draw_x = value;
            statesprite->vertex_dirty = true;
            break;
        case SPRITE_PROP_Y:
            statesprite->draw_y = value;
            statesprite->vertex_dirty = true;
            break;
        case SPRITE_PROP_WIDTH:
            statesprite->draw_width = value;
            statesprite->vertex_dirty = true;
            break;
        case SPRITE_PROP_HEIGHT:
            statesprite->draw_height = value;
            statesprite->vertex_dirty = true;
            break;
        case SPRITE_PROP_ROTATE:
            statesprite_matrix_rotate(statesprite, value);
            break;
        case SPRITE_PROP_SCALE_X:
            statesprite_matrix_scale(statesprite, value, FLOAT_NaN);
            break;
        case SPRITE_PROP_SCALE_Y:
            statesprite_matrix_scale(statesprite, FLOAT_NaN, value);
            break;
        case SPRITE_PROP_SKEW_X:
            statesprite_matrix_skew(statesprite, value, FLOAT_NaN);
            break;
        case SPRITE_PROP_SKEW_Y:
            statesprite_matrix_skew(statesprite, FLOAT_NaN, value);
            break;
        case SPRITE_PROP_TRANSLATE_X:
            statesprite_matrix_translate(statesprite, value, FLOAT_NaN);
            break;
        case SPRITE_PROP_TRANSLATE_Y:
            statesprite_matrix_translate(statesprite, FLOAT_NaN, value);
            break;
        case SPRITE_PROP_ALPHA:
            statesprite->alpha = math2d_clamp_float(value, 0.0f, 1.0f);
            break;
        case SPRITE_PROP_Z:
            statesprite->z_index = value;
            break;
        case SPRITE_PROP_VERTEX_COLOR_R:
            statesprite->vertex_color[0] = value;
            break;
        case SPRITE_PROP_VERTEX_COLOR_G:
            statesprite->vertex_color[1] = value;
            break;
        case SPRITE_PROP_VERTEX_COLOR_B:
            statesprite->vertex_color[2] = value;
            break;
        case SPRITE_PROP_VERTEX_COLOR_OFFSET_R:
            statesprite->offsetcolor[0] = value;
            break;
        case SPRITE_PROP_VERTEX_COLOR_OFFSET_G:
            statesprite->offsetcolor[1] = value;
            break;
        case SPRITE_PROP_VERTEX_COLOR_OFFSET_B:
            statesprite->offsetcolor[2] = value;
            break;
        case SPRITE_PROP_VERTEX_COLOR_OFFSET_A:
            statesprite->offsetcolor[3] = value;
            break;
        case SPRITE_PROP_ANIMATIONLOOP:
            logger_log("statesprite_set_property() SPRITE_PROP_ANIMATIONLOOP is not supported");
            break;
        case SPRITE_PROP_ROTATE_PIVOT_ENABLED:
            statesprite_matrix_rotate_pivot_enable(statesprite, value >= 1.0f);
            break;
        case SPRITE_PROP_ROTATE_PIVOT_U:
            statesprite->matrix_source.rotate_pivot_u = value;
            break;
        case SPRITE_PROP_ROTATE_PIVOT_V:
            statesprite->matrix_source.rotate_pivot_u = value;
            break;
        case SPRITE_PROP_SCALE_DIRECTION_X:
            statesprite->matrix_source.scale_direction_x = value;
            break;
        case SPRITE_PROP_SCALE_DIRECTION_Y:
            statesprite->matrix_source.scale_direction_y = value;
            break;
        case SPRITE_PROP_TRANSLATE_ROTATION:
            statesprite->matrix_source.translate_rotation = value >= 1.0f;
            break;
        case SPRITE_PROP_SCALE_SIZE:
            statesprite->matrix_source.scale_size = value >= 1.0f;
            break;
        case SPRITE_PROP_SCALE_TRANSLATION:
            statesprite->matrix_source.scale_translation = value >= 1.0f;
            break;
        case SPRITE_PROP_Z_OFFSET:
            statesprite->z_offset = value;
            break;
        case SPRITE_PROP_ANTIALIASING:
            statesprite->antialiasing = (PVRFlag)(int)value;
            break;
        case SPRITE_PROP_FLIP_X:
            statesprite->flip_x = value >= 1.0f;
            statesprite->vertex_dirty = true;
            break;
        case SPRITE_PROP_FLIP_Y:
            statesprite->flip_y = value >= 1.0f;
            statesprite->vertex_dirty = true;
            break;
        case SPRITE_PROP_FLIP_CORRECTION:
            statesprite->flip_correction = value >= 1.0f;
            statesprite->vertex_dirty = true;
            break;
        case SPRITE_PROP_ALPHA2:
            statesprite->alpha2 = value;
            break;
    }
}

void statesprite_set_offsetcolor(StateSprite statesprite, float r, float g, float b, float a) {
    if (r >= 0.0f) statesprite->offsetcolor[0] = r;
    if (g >= 0.0f) statesprite->offsetcolor[1] = g;
    if (b >= 0.0f) statesprite->offsetcolor[2] = b;
    if (a >= 0.0f) statesprite->offsetcolor[3] = a;
}

void statesprite_set_vertex_color(StateSprite statesprite, float r, float g, float b) {
    if (r >= 0.0f) statesprite->vertex_color[0] = r;
    if (g >= 0.0f) statesprite->vertex_color[1] = g;
    if (b >= 0.0f) statesprite->vertex_color[2] = b;
}

void statesprite_set_vertex_color_rgb8(StateSprite statesprite, uint32_t rbg8_color) {
    math2d_color_bytes_to_floats(rbg8_color, false, statesprite->vertex_color);
}


AnimSprite statesprite_animation_set(StateSprite statesprite, AnimSprite animsprite) {
    AnimSprite old_animation = statesprite->animation_external;
    statesprite->animation_external = animsprite;
    return old_animation;
}

void statesprite_animation_restart(StateSprite statesprite) {
    if (statesprite->animation_external) {
        animsprite_restart(statesprite->animation_external);
        animsprite_update_statesprite(statesprite->animation_external, statesprite, true);
    }
}

void statesprite_animation_end(StateSprite statesprite) {
    if (statesprite->animation_external)
        animsprite_force_end3(statesprite->animation_external, statesprite);
}

bool statesprite_animation_completed(StateSprite statesprite) {
    return !statesprite->animation_external || animsprite_is_completed(statesprite->animation_external);
}

int32_t statesprite_animate(StateSprite statesprite, float elapsed) {
    // math2d_float_assert_NaN(elapsed);

    int32_t result = 1;

    if (statesprite->animation_external) {
        result = animsprite_animate(statesprite->animation_external, elapsed);
        animsprite_update_statesprite(statesprite->animation_external, statesprite, true);
    }

    if (statesprite->trailing_disabled) return result;

    // check delay for next trail
    bool wait = statesprite->trailing_progress < statesprite->trailing_delay;
    statesprite->trailing_progress += elapsed;

    if (wait) return result;
    statesprite->trailing_progress -= statesprite->trailing_delay;

    // compute trailing using the cached vertex
    bool insert_vertex = true;
    if (statesprite->trailing_used > 0) {
        // check if the last vertex equals to the current vertex
        Trail* trail = &statesprite->trailing_buffer[0];
        bool source = trail->sx == statesprite->vertex[0] && trail->sy == statesprite->vertex[1] && trail->sw == statesprite->vertex[2] && trail->sh == statesprite->vertex[3];
        bool draw = trail->dx == statesprite->vertex[4] && trail->dy == statesprite->vertex[5] && trail->dw == statesprite->vertex[6] && trail->dh == statesprite->vertex[7];
        bool color = statesprite->vertex_color[0] == trail->r && statesprite->vertex_color[1] == trail->g && statesprite->vertex_color[2] == trail->b;
        insert_vertex = !source || !draw || !color;

        if (insert_vertex) {
            // do right shift (this should be optimized to shift only used trails)
            for (int32_t i = statesprite->trailing_length - 1, j = statesprite->trailing_length - 2; j >= 0; j--, i--) {
                statesprite->trailing_buffer[j] = statesprite->trailing_buffer[i];
            }
        } else {
            statesprite->trailing_used--;
        }
    }

    if (insert_vertex) {
        // add new trail
        Trail* trail = &statesprite->trailing_buffer[0];
        trail->sx = statesprite->vertex[0];
        trail->sy = statesprite->vertex[1];
        trail->sw = statesprite->vertex[2];
        trail->sh = statesprite->vertex[3];
        trail->dx = statesprite->vertex[4];
        trail->dy = statesprite->vertex[5];
        trail->dw = statesprite->vertex[6];
        trail->dh = statesprite->vertex[7];
        trail->r = statesprite->vertex_color[0];
        trail->g = statesprite->vertex_color[1];
        trail->b = statesprite->vertex_color[2];
        if (statesprite->trailing_used < statesprite->trailing_length) statesprite->trailing_used++;
    }

    // update alpha of each trail
    for (int32_t i = 0; i < statesprite->trailing_used; i++) {
        statesprite->trailing_buffer[i].alpha = (1.0f - (i / statesprite->trailing_used)) * statesprite->trailing_alpha;
    }

    return result;
}


float statesprite_get_z_index(StateSprite statesprite) {
    return statesprite->z_index + statesprite->z_offset;
}

Modifier* statesprite_get_modifier(StateSprite statesprite) {
    return &statesprite->matrix_source;
}

void statesprite_get_draw_size(StateSprite statesprite, float* width, float* height) {
    *width = statesprite->draw_width >= 0 ? statesprite->draw_width : statesprite->src_width;
    *height = statesprite->draw_height >= 0 ? statesprite->draw_height : statesprite->src_height;
}

void statesprite_get_draw_location(StateSprite statesprite, float* x, float* y) {
    *x = statesprite->draw_x;
    *y = statesprite->draw_y;
}

float statesprite_get_alpha(StateSprite statesprite) {
    return statesprite->alpha * statesprite->alpha2;
}

void statesprite_get_offset_source_size(StateSprite statesprite, float* width, float* height) {
    *width = statesprite->frame_width > 0 ? statesprite->frame_width : statesprite->src_width;
    *height = statesprite->frame_height > 0 ? statesprite->frame_height : statesprite->src_height;
}

bool statesprite_is_visible(StateSprite statesprite) {
    return statesprite->visible;
}


void statesprite_matrix_reset(StateSprite statesprite) {
    pvr_context_helper_clear_modifier(&statesprite->matrix_source);
    statesprite->flip_x = statesprite->flip_y = false;
    statesprite->matrix_corner = (CornerRotation){.x = 0.0f, .y = 0.0f, .angle = 0.0f};
}

void statesprite_matrix_scale(StateSprite statesprite, float scale_x, float scale_y) {
    if (!math2d_is_float_NaN(scale_x)) statesprite->matrix_source.scale_x = scale_x;
    if (!math2d_is_float_NaN(scale_y)) statesprite->matrix_source.scale_y = scale_y;
}

void statesprite_matrix_rotate(StateSprite statesprite, float degrees_angle) {
    statesprite->matrix_source.rotate = degrees_angle * MATH2D_DEG_TO_RAD;
}

void statesprite_matrix_skew(StateSprite statesprite, float skew_x, float skew_y) {
    if (!math2d_is_float_NaN(skew_x)) statesprite->matrix_source.skew_x = skew_x;
    if (!math2d_is_float_NaN(skew_y)) statesprite->matrix_source.skew_y = skew_y;
}

void statesprite_matrix_translate(StateSprite statesprite, float translate_x, float translate_y) {
    if (!math2d_is_float_NaN(translate_x)) statesprite->matrix_source.translate_x = translate_x;
    if (!math2d_is_float_NaN(translate_y)) statesprite->matrix_source.translate_y = translate_y;
}

void statesprite_matrix_rotate_pivot_enable(StateSprite statesprite, bool enable) {
    statesprite->matrix_source.rotate_pivot_enabled = enable;
}

void statesprite_matrix_rotate_pivot(StateSprite statesprite, float u, float v) {
    if (!math2d_is_float_NaN(u)) statesprite->matrix_source.rotate_pivot_u = u;
    if (!math2d_is_float_NaN(v)) statesprite->matrix_source.rotate_pivot_u = v;
}

void statesprite_matrix_scale_offset(StateSprite statesprite, float direction_x, float direction_y) {
    if (!math2d_is_float_NaN(direction_x)) statesprite->matrix_source.scale_direction_x = direction_x;
    if (!math2d_is_float_NaN(direction_y)) statesprite->matrix_source.scale_direction_y = direction_y;
}

void statesprite_matrix_translate_rotation(StateSprite statesprite, bool enable_translate_rotation) {
    statesprite->matrix_source.translate_rotation = enable_translate_rotation;
}

void statesprite_matrix_scale_size(StateSprite statesprite, bool enable_scale_size) {
    statesprite->matrix_source.scale_size = enable_scale_size;
}


StateSpriteState* statesprite_state_add(StateSprite statesprite, ModelHolder modelholder, const char* animation_name, const char* state_name) {
    if (!modelholder) return NULL;
    const AtlasEntry* atlas_entry = modelholder_get_atlas_entry2(modelholder, animation_name);
    AnimSprite animsprite = modelholder_create_animsprite(modelholder, animation_name, false, false);
    Texture texture = modelholder_get_texture(modelholder, false);
    uint32_t vertex_color = modelholder_get_vertex_color(modelholder);

    if (!atlas_entry && animsprite) {
        atlas_entry = animsprite_helper_get_first_frame_atlas_entry(animsprite);
    }

    StateSpriteState* statesprite_state = statesprite_state_add2(
        statesprite, texture, animsprite, atlas_entry, vertex_color, state_name
    );

    if (!statesprite_state && animsprite) animsprite_destroy(&animsprite);

    return statesprite_state;
}

StateSpriteState* statesprite_state_add2(StateSprite statesprite, Texture texture, AnimSprite animsprt, const AtlasEntry* atlas_entry, uint32_t rgb8_color, const char* state_name) {
    if (texture && !atlas_entry && !animsprt) return NULL;

    if (state_name != NULL)
        state_name = string_duplicate(state_name);

    float corner_x = statesprite->matrix_corner.x;
    float corner_y = statesprite->matrix_corner.y;
    float corner_angle = statesprite->matrix_corner.angle;

    if (!texture && atlas_entry) {
        logger_warn("statesprite_state_add2() atlas_entry specified but the texture was null");
        atlas_entry = NULL;
    }

    if (texture) texture = texture_share_reference(texture);

    StateSpriteState* state = malloc_chk(sizeof(StateSpriteState));
    malloc_assert(state, StateSpriteState);

    *state = (StateSpriteState){
        .statesprite_id = statesprite->id,
        .state_name = state_name,
        .texture = texture,
        .animation = animsprt,
        .vertex_color = rgb8_color,
        .frame_info = NULL,
        .corner_rotation = {.x = corner_x, .y = corner_y, .angle = corner_angle},
        .offset_x = statesprite->offset_x,
        .offset_y = statesprite->offset_y,
        .draw_width = statesprite->draw_width,
        .draw_height = statesprite->draw_height
    };

    if (atlas_entry) {
        state->frame_info = malloc_chk(sizeof(AtlasEntry));
        malloc_assert(state->frame_info, AtlasEntry);
        *state->frame_info = *atlas_entry;
    }

    linkedlist_add_item(statesprite->state_list, state);

    return state;
}

bool statesprite_state_toggle(StateSprite statesprite, const char* state_name) {
    if (statesprite->selected_state && string_equals(statesprite->selected_state->state_name, state_name))
        return true;

    StateSpriteState* state = NULL;

    foreach (StateSpriteState*, item, LINKEDLIST_ITERATOR, statesprite->state_list) {
        if (string_equals(item->state_name, state_name)) {
            state = item;
            break;
        }
    }

    if (!state) return false;

    statesprite_state_apply(statesprite, state);

    return true;
}

LinkedList statesprite_state_list(StateSprite statesprite) {
    return statesprite->state_list;
}

const char* statesprite_state_name(StateSprite statesprite) {
    if (statesprite->selected_state) return statesprite->selected_state->state_name;
    return NULL;
}

bool statesprite_state_has(StateSprite statesprite, const char* state_name) {
    foreach (StateSpriteState*, state, LINKEDLIST_ITERATOR, statesprite->state_list) {
        if (string_equals(state->state_name, state_name)) return true;
    }
    return false;
}

StateSpriteState* statesprite_state_get(StateSprite statesprite) {
    return statesprite->selected_state;
}

bool statesprite_state_apply(StateSprite statesprite, const StateSpriteState* statesprite_state) {
    if (!statesprite_state && !statesprite->selected_state) return false;

    if (statesprite_state && statesprite_state->statesprite_id != statesprite->id) {
        logger_warn("statesprite_state_apply() attempt to apply a foreign state");
        return false;
    }

    const StateSpriteState* state = statesprite_state ? statesprite_state : statesprite->selected_state;

    if (state->frame_info) {
        statesprite_set_offset_source(
            statesprite,
            state->frame_info->x, state->frame_info->y,
            state->frame_info->width, state->frame_info->height
        );
        statesprite_set_offset_frame(
            statesprite,
            state->frame_info->frame_x, state->frame_info->frame_y,
            state->frame_info->frame_width, state->frame_info->frame_height
        );
        statesprite_set_offset_pivot(
            statesprite,
            state->frame_info->pivot_x,
            state->frame_info->pivot_y
        );
    } else {
        statesprite_set_offset_pivot(
            statesprite,
            0.0f,
            0.0f
        );
    }

    statesprite->texture = state->texture;
    statesprite->animation_external = state->animation;
    statesprite_set_vertex_color_rgb8(statesprite, state->vertex_color);
    statesprite->offset_x = state->offset_x;
    statesprite->offset_y = state->offset_y;
    statesprite->draw_width = state->draw_width;
    statesprite->draw_height = state->draw_height;
    statesprite->matrix_corner.x = state->corner_rotation.x;
    statesprite->matrix_corner.y = state->corner_rotation.y;
    statesprite->matrix_corner.angle = state->corner_rotation.angle;

    statesprite->selected_state = (StateSpriteState*)state;
    statesprite->vertex_dirty = true;
    return true;
}

void statesprite_state_get_offsets(StateSprite statesprite, float* offset_x, float* offset_y) {
    *offset_x = statesprite->offset_x;
    *offset_y = statesprite->offset_y;
}

void statesprite_state_set_offsets(StateSprite statesprite, float offset_x, float offset_y, bool update_state) {
    statesprite->offset_x = offset_x;
    statesprite->offset_y = offset_y;

    if (update_state && statesprite->selected_state) {
        statesprite->selected_state->offset_x = offset_x;
        statesprite->selected_state->offset_y = offset_y;
    }

    statesprite->vertex_dirty = true;
}

void statesprite_state_remove(StateSprite statesprite, const char* state_name) {
    int32_t i = 0;

    foreach (StateSpriteState*, state, LINKEDLIST_ITERATOR, statesprite->state_list) {
        if (string_equals(state->state_name, state_name)) {
            if (statesprite->animation_external == state->animation) statesprite->animation_external = NULL;
            if (state->texture) texture_destroy(&state->texture);
            if (state->animation) animsprite_destroy(&state->animation);
            if (state == statesprite->selected_state) statesprite->selected_state = NULL;

            linkedlist_remove_item_at(statesprite->state_list, i);

            free_chk((char*)state->state_name);
            free_chk(state->frame_info);
            free_chk(state);
            return;
        }
        i++;
    }
}


void statesprite_resize_draw_size(StateSprite statesprite, float max_width, float max_height, float* applied_width, float* applied_height) {
    float orig_width = statesprite->frame_width > 0.0f ? statesprite->frame_width : statesprite->src_width;
    float orig_height = statesprite->frame_height > 0.0f ? statesprite->frame_height : statesprite->src_height;

    float width = 0.0f, height = 0.0f;

    if (max_width < 1 && max_height < 1.0f) {
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

    statesprite->draw_width = width;
    statesprite->draw_height = height;
    statesprite->vertex_dirty = true;

    if (applied_width) *applied_width = width;
    if (applied_height) *applied_height = height;
}

void statesprite_center_draw_location(StateSprite statesprite, float x, float y, float ref_width, float ref_height, float* applied_x, float* applied_y) {
    if (ref_width >= 0.0f) x += ((ref_width - statesprite->draw_width) / 2.0f);
    if (ref_height >= 0.0f) y += ((ref_height - statesprite->draw_height) / 2.0f);

    statesprite->draw_x = x;
    statesprite->draw_y = y;
    statesprite->vertex_dirty = true;

    if (applied_x) *applied_x = statesprite->draw_x;
    if (applied_y) *applied_y = statesprite->draw_y;
}

void statesprite_corner_rotate(StateSprite statesprite, Corner corner) {
    sh4matrix_helper_calculate_corner_rotation(&statesprite->matrix_corner, corner);
}


bool statesprite_crop(StateSprite statesprite, float dx, float dy, float dwidth, float dheight) {
    if (!math2d_is_float_NaN(dx)) statesprite->crop.x = dx;
    if (!math2d_is_float_NaN(dy)) statesprite->crop.y = dy;
    if (!math2d_is_float_NaN(dwidth)) statesprite->crop.width = dwidth;
    if (!math2d_is_float_NaN(dheight)) statesprite->crop.height = dheight;

    bool invalid = statesprite->crop.x < 0.0f &&
                   statesprite->crop.y < 0.0f &&
                   statesprite->crop.width == 0.0f &&
                   statesprite->crop.height == 0.0f;

    // disable cropping if the bounds are invalid
    if (statesprite->crop_enabled && invalid) statesprite->crop_enabled = false;

    statesprite->vertex_dirty = true;
    return !invalid;
}

bool statesprite_is_crop_enabled(StateSprite statesprite) {
    return statesprite->crop_enabled;
}

void statesprite_crop_enable(StateSprite statesprite, bool enable) {
    statesprite->crop_enabled = enable;
}

void statesprite_flip_texture(StateSprite statesprite, nbool flip_x, nbool flip_y) {
    if (flip_x != unset) statesprite->flip_x = flip_x;
    if (flip_y != unset) statesprite->flip_y = flip_y;
    statesprite->vertex_dirty = true;
}

void statesprite_flip_texture_enable_correction(StateSprite statesprite, bool enabled) {
    statesprite->flip_correction = enabled;
}

void statesprite_change_draw_size_in_atlas_apply(StateSprite statesprite, bool enable, float scale_factor) {
    math2d_double_assert_NaN(scale_factor);
    statesprite->scale_texture = scale_factor;
    statesprite->atlas_to_draw_size_enabled = !!enable;
}

void statesprite_set_antialiasing(StateSprite statesprite, PVRFlag antialiasing) {
    statesprite->antialiasing = antialiasing;
}

/*void statesprite_set_shader(StateSprite statesprite, PSShader psshader) {
    statesprite->psshader = psshader;
}

PSShader statesprite_get_shader(StateSprite statesprite) {
    return statesprite->psshader;
}*/

void statesprite_blend_enable(StateSprite statesprite, bool enabled) {
    statesprite->blend_enabled = enabled;
}

void statesprite_blend_set(StateSprite statesprite, Blend src_rgb, Blend dst_rgb, Blend src_alpha, Blend dst_alpha) {
    statesprite->blend_src_rgb = src_rgb;
    statesprite->blend_dst_rgb = dst_rgb;
    statesprite->blend_src_alpha = src_alpha;
    statesprite->blend_dst_alpha = dst_alpha;
}

void statesprite_trailing_enabled(StateSprite statesprite, bool enabled) {
    statesprite->trailing_disabled = !enabled;
}

void statesprite_trailing_set_params(StateSprite statesprite, int32_t length, float trail_delay, float trail_alpha, nbool darken_colors) {
    if (length > 0) {
        statesprite->trailing_buffer = realloc_for_array(statesprite->trailing_buffer, length, Trail);
        statesprite->trailing_length = length;
    }
    if (darken_colors != unset) statesprite->trailing_darken = darken_colors ? true : false;
    if (statesprite->trailing_used > length) statesprite->trailing_used = length;
    if (!math2d_is_float_NaN(trail_delay)) statesprite->trailing_delay = trail_delay;
    if (!math2d_is_float_NaN(trail_alpha)) statesprite->trailing_alpha = trail_alpha;

    // force update
    statesprite->trailing_progress = -FLOAT_Inf;
}

void statesprite_trailing_set_offsetcolor(StateSprite statesprite, float r, float g, float b) {
    if (!math2d_is_float_NaN(r)) statesprite->trailing_offsetcolor[0] = r;
    if (!math2d_is_float_NaN(g)) statesprite->trailing_offsetcolor[1] = g;
    if (!math2d_is_float_NaN(b)) statesprite->trailing_offsetcolor[2] = b;
}
