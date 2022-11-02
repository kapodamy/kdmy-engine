"use strict";

async function freeplaymenu_songicons_init(song_map, max_dimmen, font_size) {
    let songicons = { icons: null, icons_size: 0 };
    songicons.icons_size = arraylist_size(song_map);
    songicons.icons = new Array(songicons.icons_size);


    let last_model = null;
    let last_model_exists = 0;
    let base_offset_y = (font_size - max_dimmen) / 2.0;

    for (let i = 0; i < songicons.icons_size; i++) {
        let track = arraylist_get(song_map, i);
        let weekinfo = weeks_array.array[track.week_index];

        let icon_name = weekinfo.songs[track.song_index].freeplay_host_icon_name;
        let model_src = weekinfo.songs[track.song_index].freeplay_host_icon_model;

        if (!icon_name) icon_name = HEALTHBAR_ICON_PREFIX_NEUTRAL;
        if (!model_src) model_src = weekinfo.songs_default_freeplay_host_icon_model;

        if (model_src != last_model) {
            last_model = model_src;
            last_model_exists = await fs_file_exists(model_src);
        }
        if (!last_model_exists) model_src = null;

        let sprite = null;
        let offset_x = 0;
        let offset_y = 0;

        L_load_model_holder:
        if (modelholder_utils_is_known_extension(model_src)) {
            let modelholder = await modelholder_init(model_src);
            if (!modelholder) break L_load_model_holder;

            let texture = modelholder_get_texture(modelholder, 1);
            let anim = modelholder_create_animsprite(modelholder, icon_name, 0, 0);
            let color_rgb8 = modelholder_get_vertex_color(modelholder);

            if (texture)
                sprite = sprite_init(texture);
            else
                sprite = sprite_init_from_rgb8(color_rgb8);

            if (anim) {
                sprite_external_animation_set(sprite, anim);
            } else {
                let atlas_entry = modelholder_get_atlas_entry(modelholder, icon_name, 0);
                if (atlas_entry) atlas_apply_from_entry(sprite, atlas_entry, 1);
            }

            modelholder_destroy(modelholder);
        } else if (model_src) {
            let texture = await texture_init(model_src);
            sprite = sprite_init(texture);
        }

        if (sprite) {
            const draw_size = [0, 0];
            sprite_resize_draw_size(sprite, max_dimmen, max_dimmen, draw_size);

            offset_x = (max_dimmen - draw_size[0]) / 2.0;
            offset_y = (max_dimmen - draw_size[1]) / 2.0;
            offset_y += base_offset_y;
        }

        songicons.icons[i] = {
            sprite: sprite,
            offset_x: offset_x,
            offset_y: offset_y
        };
    }

    return songicons;
}

function freeplaymenu_songicons_destroy(songicons) {
    for (let i = 0; i < songicons.icons_size; i++) {
        if (songicons.icons[i].sprite) sprite_destroy_full(songicons.icons[i].sprite);
    }
    songicons.icons = null;
    songicons = null;
}

function freeplaymenu_songicons_draw_item_icon(songicons, pvrctx, menu, idx, x, y, w, h) {
    let icon = songicons.icons[idx];
    if (!icon.sprite) return 1;

    sprite_set_draw_location(icon.sprite, x + w, y + icon.offset_y);
    sprite_draw(icon.sprite, pvrctx);

    return 1;
}

