#include "imgutils.h"
#include "math2d.h"


void imgutils_calc_size(float orig_width, float orig_height, float max_width, float max_height, float* result_width, float* result_height) {
    float width = 0.0f, height = 0.0f;

    if (max_width < 0.0f && max_height < 0.0f) {
        width = orig_width;
        height = orig_height;
    } else if (orig_width == 0.0f && orig_height == 0.0f) {
        if (max_width > 0.0f) width = max_width;
        if (max_height > 0.0f) height = max_height;
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

    *result_width = width;
    *result_height = height;
}

void imgutils_calc_size2(Texture texture, const AtlasEntry* atlas_entry, float max_width, float max_height, float* result_width, float* result_height) {
    float orig_width, orig_height;

    if (atlas_entry) {
        orig_width = atlas_entry->frame_width > 0.0f ? atlas_entry->frame_width : atlas_entry->width;
        orig_height = atlas_entry->frame_height > 0.0f ? atlas_entry->frame_height : atlas_entry->height;
    } else if (texture) {
        orig_width = texture->original_width;
        orig_height = texture->original_height;
    } else {
        *result_width = max_width;
        *result_height = max_height;
        return;
    }

    imgutils_calc_size(orig_width, orig_height, max_width, max_height, result_width, result_height);
}

void imgutils_calc_centered_location(float x, float y, float width, float height, float ref_width, float ref_height, float* location_x, float* location_y) {
    if (ref_width >= 0.0f) x += ((ref_width - width) / 2.0f);
    if (ref_height >= 0.0f) y += ((ref_height - height) / 2.0f);

    *location_x = x;
    *location_y = y;
}

void imgutils_get_statesprite_original_size(StateSpriteState* statesprite_state, float* output_width, float* output_height) {
    if (statesprite_state->frame_info) {
        if (statesprite_state->frame_info->frame_width > 0.0f)
            *output_width = statesprite_state->frame_info->frame_width;
        else
            *output_width = statesprite_state->frame_info->width;

        if (statesprite_state->frame_info->frame_height > 0.0f)
            *output_height = statesprite_state->frame_info->frame_height;
        else
            *output_height = statesprite_state->frame_info->height;
    } else if (statesprite_state->texture) {
        texture_get_original_dimmensions(statesprite_state->texture, output_width, output_height);
    }
}

void imgutils_calc_rectangle(float x, float y, float max_width, float max_height, float src_width, float src_height, Align align_hrzntl, Align align_vrtcl, float* d_width, float* d_height, float* d_x, float* d_y) {
    float draw_width = src_width, draw_height = src_height;
    imgutils_calc_size(src_width, src_height, max_width, max_height, &draw_width, &draw_height);

    if (max_width > 0.0f && (align_hrzntl == ALIGN_CENTER || align_hrzntl == ALIGN_END)) {
        float offset = (max_width - draw_width);
        if (align_hrzntl == ALIGN_CENTER) offset /= 2.0f;
        x += offset;
    }
    if (max_height > 0.0f && (align_vrtcl == ALIGN_CENTER || align_vrtcl == ALIGN_END)) {
        float offset = (max_height - draw_height);
        if (align_hrzntl == ALIGN_CENTER) offset /= 2.0f;
        y += offset;
    }

    if (d_width) *d_width = draw_width;
    if (d_height) *d_height = draw_height;

    if (d_x) *d_x = x;
    if (d_y) *d_y = y;
}

void imgutils_calc_rectangle_in_sprite(float x, float y, float max_width, float max_height, Align align_hrzntl, Align align_vrtcl, Sprite sprite) {
    float draw_width = 0.0f, draw_height = 0.0f;
    float draw_x = -1.0f, draw_y = -1.0f;

    sprite_get_source_size(sprite, &draw_width, &draw_height);
    imgutils_calc_rectangle(
        x, y, max_width, max_height,
        draw_width, draw_height, align_hrzntl, align_vrtcl, &draw_width, &draw_height, &draw_x, &draw_y
    );

    sprite_set_draw_size(sprite, draw_width, draw_height);
    sprite_set_draw_location(sprite, draw_x, draw_y);
}

void imgutils_calc_rectangle_in_statesprite_state(float off_x, float off_y, float max_width, float max_height, Align align_hrzntl, Align align_vrtcl, StateSpriteState* statesprite_state) {
    float draw_width = 0.0f, draw_height = 0.0f;
    float draw_x = 0.0f, draw_y = 0.0f;

    if (!statesprite_state) return;

    imgutils_get_statesprite_original_size(statesprite_state, &draw_width, &draw_height);
    imgutils_calc_rectangle(
        off_x, off_y, max_width, max_height,
        draw_width, draw_height, align_hrzntl, align_vrtcl, &draw_width, &draw_height, &draw_x, &draw_y
    );

    statesprite_state->draw_width = draw_width;
    statesprite_state->draw_height = draw_height;
    statesprite_state->offset_x = draw_x;
    statesprite_state->offset_y = draw_y;
}

void imgutils_calc_resize_sprite(Sprite sprite, float max_width, float max_height, bool cover, bool center) {
    float draw_width = 0.0f, draw_height = 0.0f;
    float location_x = 0.0f, location_y = 0.0f;
    if (cover) {
        sprite_get_source_size(sprite, &draw_width, &draw_height);
        float ratio_width = draw_width / max_width;
        float ratio_height = draw_height / max_height;

        // resize the longest dimension of the sprite
        if (ratio_width > ratio_height)
            sprite_resize_draw_size(sprite, -1.0f, max_height, &draw_width, &draw_height);
        else
            sprite_resize_draw_size(sprite, max_width, -1.0f, &draw_width, &draw_height);
    } else {
        sprite_resize_draw_size(sprite, max_width, max_height, &draw_width, &draw_height);
    }
    if (center) {
        sprite_get_draw_location(sprite, &location_x, &location_y);
        if (max_width >= 0) location_x -= (draw_width - max_width) / 2.0f;
        if (max_height >= 0) location_y -= (draw_height - max_height) / 2.0f;
        sprite_set_draw_location(sprite, location_x, location_y);
    }
}
