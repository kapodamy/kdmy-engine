#include "game/helpers/freeplaymenu_songicons.h"

#include "arraylist.h"
#include "fs.h"
#include "game/common/weekenumerator.h"
#include "game/freeplaymenu_types.h"
#include "game/funkin/healthbar.h"
#include "malloc_utils.h"
#include "modelholder.h"
#include "sprite.h"
#include "stringutils.h"


typedef struct {
    Sprite sprite;
    float offset_x;
    float offset_y;
} SongIcon;

struct FreeplaySongIcons_s {
    SongIcon* icons;
    int32_t icons_size;
};


FreeplaySongIcons freeplaymenu_songicons_init(ArrayList song_map, float max_dimmen, float font_size) {
    FreeplaySongIcons songicons = malloc_chk(sizeof(struct FreeplaySongIcons_s));
    malloc_assert(songicons, FreeplaySongIcons);

    *songicons = (struct FreeplaySongIcons_s){.icons = NULL, .icons_size = 0};
    songicons->icons_size = arraylist_size(song_map);
    songicons->icons = malloc_for_array(SongIcon, songicons->icons_size);

    const char* last_model = NULL;
    bool last_model_exists = false;
    float base_offset_y = (font_size - max_dimmen) / 2.0f;

    for (int32_t i = 0; i < songicons->icons_size; i++) {
        MappedSong* song = arraylist_get(song_map, i);
        WeekInfo* weekinfo = &weeks_array.array[song->week_index];

        const char *icon_name = NULL, *model_src = NULL;

        if (song->is_locked) {
            icon_name = weekinfo->songs[song->song_index].freeplay_locked_host_icon_name;
            model_src = weekinfo->songs[song->song_index].freeplay_locked_host_icon_model;
        }

        if (!icon_name) icon_name = weekinfo->songs[song->song_index].freeplay_host_icon_name;
        if (!model_src) model_src = weekinfo->songs[song->song_index].freeplay_host_icon_model;

        if (!icon_name) icon_name = HEALTHBAR_ICON_PREFIX_NEUTRAL;
        if (!model_src) model_src = weekinfo->songs_default_freeplay_host_icon_model;

        if (!string_equals(model_src, last_model)) {
            last_model = model_src;
            last_model_exists = fs_file_exists(model_src);
        }
        if (!last_model_exists) model_src = NULL;

        Sprite sprite = NULL;
        float offset_x = 0.0f;
        float offset_y = 0.0f;

        if (modelholder_utils_is_known_extension(model_src)) {
            ModelHolder modelholder = modelholder_init(model_src);
            if (!modelholder) goto L_continue;

            Texture texture = modelholder_get_texture(modelholder, true);
            AnimSprite anim = modelholder_create_animsprite(modelholder, icon_name, false, false);
            uint32_t color_rgb8 = modelholder_get_vertex_color(modelholder);

            if (texture)
                sprite = sprite_init(texture);
            else
                sprite = sprite_init_from_rgb8(color_rgb8);

            if (anim) {
                sprite_external_animation_set(sprite, anim);
            } else {
                const AtlasEntry* atlas_entry = modelholder_get_atlas_entry(modelholder, icon_name);
                if (atlas_entry) atlas_apply_from_entry(sprite, atlas_entry, true);
            }

            modelholder_destroy(&modelholder);
        } else if (model_src) {
            Texture texture = texture_init(model_src);
            sprite = sprite_init(texture);
        }

    L_continue:
        if (sprite) {
            float draw_width = 0.0f, draw_height = 0.0f;
            sprite_resize_draw_size(sprite, max_dimmen, max_dimmen, &draw_width, &draw_height);

            offset_x = (max_dimmen - draw_width) / 2.0f;
            offset_y = (max_dimmen - draw_height) / 2.0f;
            offset_y += base_offset_y;
        }

        songicons->icons[i] = (SongIcon){
            .sprite = sprite,
            .offset_x = offset_x,
            .offset_y = offset_y
        };
    }

    return songicons;
}

void freeplaymenu_songicons_destroy(FreeplaySongIcons* songicons_ptr) {
    if (!songicons_ptr || !*songicons_ptr) return;

    FreeplaySongIcons songicons = *songicons_ptr;

    for (int32_t i = 0; i < songicons->icons_size; i++) {
        if (songicons->icons[i].sprite) sprite_destroy_full(&songicons->icons[i].sprite);
    }
    free_chk(songicons->icons);

    free_chk(songicons);
    *songicons_ptr = NULL;
}

bool freeplaymenu_songicons_draw_item_icon(FreeplaySongIcons songicons, PVRContext pvrctx, Menu menu, int32_t idx, float x, float y, float w, float h) {
    (void)menu;
    (void)h;

    SongIcon* icon = &songicons->icons[idx];

    if (icon->sprite) {
        sprite_set_draw_location(icon->sprite, x + w, y + icon->offset_y);
        sprite_draw(icon->sprite, pvrctx);
    }

    return true;
}
