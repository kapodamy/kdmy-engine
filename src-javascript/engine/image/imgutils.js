"use strict";

function imgutils_calc_size(orig_width, orig_height, max_width, max_height, result_size) {
    let width, height;

    if (max_width < 0 && max_height < 0) {
        width = orig_width;
        height = orig_height;
    } else if (orig_width == 0 && orig_height == 0) {
        if (max_width > 0) width = max_width;
        if (max_height > 0) height = max_height;
    } else if (max_width == 0 || max_height == 0) {
		width = height = 0;
    } else {
		if (max_width > 0 && max_height > 0)  {
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

    result_size[0] = width;
    result_size[1] = height;

    return result_size;
}

function imgutils_calc_size2(texture, atlas_entry, max_width, max_height, resized_result) {
    let orig_width, orig_height;

    if (atlas_entry) {
        orig_width = atlas_entry.frame_width > 0 ? atlas_entry.frame_width : atlas_entry.width;
        orig_height = atlas_entry.frame_height > 0 ? atlas_entry.frame_height : atlas_entry.height;
    } else if (texture) {
        orig_width = texture.original_width;
        orig_height = texture.original_height;
    } else {
        resized_result[0] = max_width;
        resized_result[1] = max_height;
        return resized_result;
    }

    return imgutils_calc_size(orig_width, orig_height, max_width, max_height, resized_result);
}

function imgutils_calc_centered_location(x, y, width, height, ref_width, ref_height, location_result) {
    if (ref_width >= 0) x += ((ref_width - width) / 2);
    if (ref_height >= 0) y += ((ref_height - height) / 2);

    location_result[0] = x;
    location_result[1] = y;

    return location_result;
}

function imgutils_get_statesprite_original_size(statesprite_state, output_size) {
    if (statesprite_state.frame_info) {
        if (statesprite_state.frame_info.frame_width > 0)
            output_size[0] = statesprite_state.frame_info.frame_width;
        else
            output_size[0] = statesprite_state.frame_info.width;

        if (statesprite_state.frame_info.frame_height > 0)
            output_size[1] = statesprite_state.frame_info.frame_height;
        else
            output_size[1] = statesprite_state.frame_info.height;
    } else if (statesprite_state.texture) {
        texture_get_original_dimmensions(statesprite_state.texture, output_size);
    }
}

function imgutils_calc_rectangle(x, y, max_width, max_height, src_width, src_height, align_hrzntl, align_vrtcl, d_size, d_location) {
    const draw_size = [src_width, src_height];
    imgutils_calc_size(src_width, src_height, max_width, max_height, draw_size);

    if (max_width > 0 && (align_hrzntl == ALIGN_CENTER || align_hrzntl == ALIGN_END)) {
        let offset = (max_width - draw_size[0]);
        if (align_hrzntl == ALIGN_CENTER) offset /= 2.0;
        x += offset;
    }
    if (max_height > 0 && (align_vrtcl == ALIGN_CENTER || align_vrtcl == ALIGN_END)) {
        let offset = (max_height - draw_size[1]);
        if (align_hrzntl == ALIGN_CENTER) offset /= 2.0;
        y += offset;
    }

    if (d_size) {
        d_size[0] = draw_size[0];
        d_size[1] = draw_size[1];
    }

    if (d_location) {
        d_location[0] = x;
        d_location[1] = y;
    }
}

function imgutils_calc_rectangle_in_sprite(x, y, max_width, max_height, align_hrzntl, align_vrtcl, sprite) {
    const draw_size = [0, 0];
    const draw_location = [-1, -1];

    sprite_get_source_size(sprite, draw_size);
    imgutils_calc_rectangle(
        x, y, max_width, max_height,
        draw_size[0], draw_size[1], align_hrzntl, align_vrtcl, draw_size, draw_location
    );

    sprite_set_draw_size(sprite, draw_size[0], draw_size[1]);
    sprite_set_draw_location(sprite, draw_location[0], draw_location[1]);
}

function imgutils_calc_rectangle_in_statesprite_state(off_x, off_y, max_width, max_height, align_hrzntl, align_vrtcl, statesprite_state) {
    const draw_size = [0, 0];
    const draw_location = [0, 0];

    if (!statesprite_state) return;

    imgutils_get_statesprite_original_size(statesprite_state, draw_size);
    imgutils_calc_rectangle(
        off_x, off_y, max_width, max_height,
        draw_size[0], draw_size[1], align_hrzntl, align_vrtcl, draw_size, draw_location
    );

    statesprite_state.draw_width = draw_size[0]
    statesprite_state.draw_height = draw_size[1];
    statesprite_state.offset_x = draw_location[0];
    statesprite_state.offset_y = draw_location[1];
}

