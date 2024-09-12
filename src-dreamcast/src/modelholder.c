#include "modelholder.h"
#include "externals/luascript.h"
#include "fs.h"
#include "game/common/funkin.h"
#include "jsonparser.h"
#include "malloc_utils.h"
#include "map.h"
#include "stringbuilder.h"
#include "stringutils.h"


struct ModelHolder_s {
    char* instance_src;
    int32_t instance_references;
    Atlas atlas;
    AnimList animlist;
    Texture texture;
    uint vertex_color_rgb8;
    int32_t id;
};


static Map MODELHOLDER_POOL = NULL;
static int32_t MODELHOLDER_IDS = 0;

static const struct Atlas_s MODELHOLDER_STUB_ATLAS = {.size = 0, .glyph_fps = 0, .texture_filename = NULL};
static const struct AnimList_s MODELHOLDER_STUB_ANIMLIST = {.entries_count = 0};


void __attribute__((constructor)) __ctor_modelholder() {
    MODELHOLDER_POOL = map_init();
}


ModelHolder modelholder_init(const char* src) {
    char* full_path = fs_get_full_path_and_override(src);

    // find an instance of this
    foreach (ModelHolder, instance, MAP_ITERATOR, MODELHOLDER_POOL) {
        if (!string_equals_ignore_case(instance->instance_src, full_path)) continue;

        instance->instance_references++;
        free_chk(full_path);
        return instance;
    }

    fs_folder_stack_push();
    fs_set_working_folder(src, true);

    // load the model manifest (if was specified)
    char* manifest_texture = NULL;
    char* manifest_atlas = NULL;
    char* manifest_animlist = NULL;
    char* fallback_texture_path = NULL;
    uint32_t vertex_color = 0xFFFFFF;
    bool from_manifest = string_lowercase_ends_with(full_path, ".json");

    if (from_manifest) {
        if (!fs_file_exists(full_path)) {
            fs_folder_stack_pop();
            return NULL;
        }
        JSONToken json = json_load_from(full_path);
        manifest_texture = string_duplicate(json_read_string(json, "texture", NULL));
        manifest_atlas = string_duplicate(json_read_string(json, "atlas", NULL));
        manifest_animlist = string_duplicate(json_read_string(json, "animlist", NULL));
        vertex_color = json_read_hex(json, "vertexColor", vertex_color);
        json_destroy(&json);
    } else {
        char* temp = full_path;
        bool from_atlas = atlas_utils_is_known_extension(full_path);

        if (from_atlas) temp = fs_get_filename_without_extension(full_path);
        char* atlas_filename = from_atlas ? string_duplicate(full_path) : string_concat(2, temp, ".xml");
        char* animlist_filename = string_concat(2, temp, "_anims.xml");
        char* fallback_texture_filename = string_concat(2, temp, ".png");
        if (from_atlas) free_chk(temp);

        manifest_atlas = fs_build_path2(full_path, atlas_filename);
        manifest_animlist = fs_build_path2(full_path, animlist_filename);
        fallback_texture_path = fs_build_path2(full_path, fallback_texture_filename);

        free_chk(atlas_filename);
        free_chk(animlist_filename);
        free_chk(fallback_texture_filename);
    }

    ModelHolder modelholder = malloc_chk(sizeof(struct ModelHolder_s));
    malloc_assert(modelholder, ModelHolder);

    *modelholder = (struct ModelHolder_s){
        .atlas = (Atlas)&MODELHOLDER_STUB_ATLAS,
        .animlist = (AnimList)&MODELHOLDER_STUB_ANIMLIST,
        .texture = NULL,
        .vertex_color_rgb8 = vertex_color,

        .id = MODELHOLDER_IDS++,
        .instance_references = 1,
        .instance_src = full_path
    };

    if (manifest_atlas && fs_file_exists(manifest_atlas)) {
        Atlas atlas = atlas_init(manifest_atlas);
        if (atlas) modelholder->atlas = atlas;
    }

    if (manifest_animlist && fs_file_exists(manifest_animlist)) {
        AnimList animlist = animlist_init(manifest_animlist);
        if (animlist) modelholder->animlist = animlist;
    }

    if (modelholder->atlas && modelholder->atlas != &MODELHOLDER_STUB_ATLAS && !manifest_texture) {
        const char* altas_texture = atlas_get_texture_path(modelholder->atlas);
        if (altas_texture && fs_file_exists(altas_texture)) {
            modelholder->texture = texture_init(altas_texture);
            if (!modelholder->texture) {
                logger_error(
                    "modelholder_init() atlas texture not found: atlas=%s texture=%s", manifest_atlas, altas_texture
                );
            }
        }
    }

    if (!modelholder->texture) {
        char* texture_src = manifest_texture ? manifest_texture : fallback_texture_path;
        if (fs_file_exists(texture_src)) {
            modelholder->texture = texture_init(texture_src);
        } else {
            logger_error("modelholder_init() missing manifest texture: %s", texture_src);
        }
    }

    free_chk(fallback_texture_path);
    free_chk(manifest_atlas);
    free_chk(manifest_texture);
    free_chk(manifest_animlist);

    fs_folder_stack_pop();

    bool no_atlas = modelholder->atlas == &MODELHOLDER_STUB_ATLAS;
    bool no_animlist = modelholder->animlist == &MODELHOLDER_STUB_ANIMLIST;
    if (!from_manifest && !modelholder->texture && no_atlas && no_animlist) {
        logger_warn("modelholder_init() failed, missing resources of %s", src);
        modelholder_destroy(&modelholder);
        return NULL;
    }

    map_add(MODELHOLDER_POOL, modelholder->id, modelholder);
    return modelholder;
}

ModelHolder modelholder_init2(uint32_t vertex_color_rgb8, const char* atlas_src, const char* animlist_src) {
    char* full_atlas_src = fs_get_full_path_and_override(atlas_src);
    char* full_animlist_src = fs_get_full_path_and_override(animlist_src);

    StringBuilder stringbuilder = stringbuilder_init(128);
    stringbuilder_add_format(stringbuilder, "$s|$s|0x$I", full_atlas_src, full_animlist_src, vertex_color_rgb8);
    char* fake_src = stringbuilder_finalize(&stringbuilder);

    free_chk(full_atlas_src);
    free_chk(full_animlist_src);

    // find an instance of this
    foreach (ModelHolder, instance, MAP_ITERATOR, MODELHOLDER_POOL) {
        if (!string_equals_ignore_case(instance->instance_src, fake_src)) continue;

        instance->instance_references++;
        free_chk(fake_src);
        return instance;
    }

    ModelHolder modelholder = malloc_chk(sizeof(struct ModelHolder_s));
    malloc_assert(modelholder, ModelHolder);

    *modelholder = (struct ModelHolder_s){
        .atlas = (Atlas)&MODELHOLDER_STUB_ATLAS,
        .animlist = (AnimList)&MODELHOLDER_STUB_ANIMLIST,
        .texture = NULL,
        .vertex_color_rgb8 = vertex_color_rgb8,

        .id = MODELHOLDER_IDS++,
        .instance_references = 1,
        .instance_src = fake_src
    };
    map_add(MODELHOLDER_POOL, modelholder->id, modelholder);

    if (animlist_src && fs_file_exists(animlist_src)) {
        modelholder->animlist = animlist_init(animlist_src);
        if (!modelholder->animlist) modelholder->animlist = (AnimList)&MODELHOLDER_STUB_ANIMLIST;
    }

    const char* atlas_texture = NULL;

    if (atlas_src && fs_file_exists(atlas_src)) {
        modelholder->atlas = atlas_init(atlas_src);

        if (modelholder->atlas != NULL)
            atlas_texture = atlas_get_texture_path(modelholder->atlas);
        else
            modelholder->atlas = (Atlas)&MODELHOLDER_STUB_ATLAS;
    }

    if (atlas_texture && fs_file_exists(atlas_texture)) {
        modelholder->texture = texture_init(atlas_texture);
    } else if (atlas_src) {
        // try use atlas name instead
        fs_folder_stack_push();
        fs_set_working_folder(atlas_src, true);

        char* temp = fs_get_filename_without_extension(atlas_src);
        char* texture_path = string_concat(2, temp, ".png");

        if (fs_file_exists(texture_path)) {
            modelholder->texture = texture_init(texture_path);
        }

        if (modelholder->texture != NULL) {
            logger_warn("modelholder_init2() expected %s, found %s", atlas_texture, texture_path);
        } else {
            logger_error("modelholder_init2() missing texture file: %s in %s", atlas_texture, atlas_src);
        }

        free_chk(temp);
        free_chk(texture_path);
        fs_folder_stack_pop();
    }

    return modelholder;
}

ModelHolder modelholder_init3(uint32_t vertex_color_rgb8, const Texture texture, const Atlas atlas, const AnimList animlist) {
    ModelHolder modelholder = malloc_chk(sizeof(struct ModelHolder_s));
    malloc_assert(modelholder, ModelHolder);

    *modelholder = (struct ModelHolder_s){
        .atlas = (Atlas)(atlas ? atlas : &MODELHOLDER_STUB_ATLAS),
        .animlist = (AnimList)(animlist ? animlist : &MODELHOLDER_STUB_ANIMLIST),
        .texture = (Texture)texture,
        .vertex_color_rgb8 = vertex_color_rgb8,

        .id = MODELHOLDER_IDS++,
        .instance_references = 1,
        .instance_src = NULL
    };

    map_add(MODELHOLDER_POOL, modelholder->id, modelholder);
    return modelholder;
}


void modelholder_destroy(ModelHolder* modelholder_ptr) {
    ModelHolder modelholder = *modelholder_ptr;
    if (!modelholder) return;

    // before continue, check if there another instances
    modelholder->instance_references--;
    if (modelholder->instance_references > 0) return;

    if (modelholder->instance_src != NULL) {
        if (modelholder->atlas != &MODELHOLDER_STUB_ATLAS && modelholder->atlas)
            atlas_destroy(&modelholder->atlas);

        if (modelholder->animlist != &MODELHOLDER_STUB_ANIMLIST)
            animlist_destroy(&modelholder->animlist);

        if (modelholder->texture)
            texture_destroy(&modelholder->texture);
    }

    map_delete(MODELHOLDER_POOL, modelholder->id);
    luascript_drop_shared(modelholder);
    free_chk(modelholder->instance_src);

    free_chk(modelholder);
    *modelholder_ptr = NULL;
}


bool modelholder_is_invalid(ModelHolder modelholder) {
    if (!modelholder) return true;
    return modelholder->atlas == &MODELHOLDER_STUB_ATLAS &&
           modelholder->animlist == &MODELHOLDER_STUB_ANIMLIST &&
           !modelholder->texture;
}

bool modelholder_has_animlist(ModelHolder modelholder) {
    return modelholder->animlist != &MODELHOLDER_STUB_ANIMLIST;
}

Atlas modelholder_get_atlas(ModelHolder modelholder) {
    return modelholder->atlas;
}

Texture modelholder_get_texture(ModelHolder modelholder, bool increment_reference) {
    if (!modelholder->texture)
        return NULL;
    else if (increment_reference)
        return texture_share_reference(modelholder->texture);
    else
        return modelholder->texture;
}

uint32_t modelholder_get_vertex_color(ModelHolder modelholder) {
    return modelholder->vertex_color_rgb8;
}

AnimList modelholder_get_animlist(ModelHolder modelholder) {
    return modelholder->animlist;
}

Sprite modelholder_create_sprite(ModelHolder modelholder, const char* atlas_entry_name, bool with_animation) {
    Sprite sprite;
    if (modelholder->texture) {
        sprite = sprite_init(texture_share_reference(modelholder->texture));
        const AtlasEntry* atlas_entry = modelholder_get_atlas_entry2(modelholder, atlas_entry_name);
        if (atlas_entry) atlas_apply_from_entry(sprite, atlas_entry, true);
    } else {
        sprite = sprite_init_from_rgb8(modelholder->vertex_color_rgb8);
    }

    if (with_animation) {
        AnimSprite animsprite = modelholder_create_animsprite(modelholder, atlas_entry_name, false, false);
        if (animsprite) sprite_animation_add(sprite, animsprite);
    }

    return sprite;
}

AnimSprite modelholder_create_animsprite(ModelHolder modelholder, const char* animation_name, bool fallback_static, bool no_return_null) {
    if (modelholder->animlist != &MODELHOLDER_STUB_ANIMLIST) {
        const AnimListItem* animlist_item = animlist_get_animation(modelholder->animlist, animation_name);
        if (animlist_item) return animsprite_init(animlist_item);
    }

    bool has_atlas = modelholder->atlas != &MODELHOLDER_STUB_ATLAS;

    if (/*modelholder->animlist == &MODELHOLDER_STUB_ANIMLIST && */ has_atlas) {
        /*if (
            !fallback_static &&
            !atlas_get_entry_with_number_suffix(modelholder->atlas, animation_name)
        ) {
            goto L_return;
        }*/

        // try build an animation using atlas frames (loops indefinitely at 24 FPS)
        AnimSprite animsprite = animsprite_init_from_atlas(
            FUNKIN_DEFAULT_ANIMATIONS_FRAMERATE, false, modelholder->atlas, animation_name, true
        );
        if (animsprite) return animsprite;

        if (fallback_static) {
            // Try build a static animation (single atlas frame)
            const AtlasEntry* atlas_entry = atlas_get_entry(modelholder->atlas, animation_name);
            if (atlas_entry) {
                return animsprite_init_from_atlas_entry(
                    atlas_entry, false, FUNKIN_DEFAULT_ANIMATIONS_FRAMERATE
                );
            }
        }
    }

    // L_return:
    if (no_return_null)
        return animsprite_init_as_empty(animation_name);
    else
        return NULL;
}

const AtlasEntry* modelholder_get_atlas_entry(ModelHolder modelholder, const char* atlas_entry_name) {
    if (modelholder->atlas == &MODELHOLDER_STUB_ATLAS) return NULL;
    return atlas_get_entry(modelholder->atlas, atlas_entry_name);
}

/**
 * Retrieves an atlas entry with the specified name. If the entry is not found, retrieves the first
 * entry with number suffix (example "dance left000"),
 * @param {Object} modelholder The modelholder instance
 * @param {string} atlas_entry_name Entry name and/or entry name prefix
 * @returns {object} the atlas entry or NULL if not found
 */
const AtlasEntry* modelholder_get_atlas_entry2(ModelHolder modelholder, const char* atlas_entry_name) {
    if (modelholder->atlas == &MODELHOLDER_STUB_ATLAS) return NULL;

    const AtlasEntry* atlas_entry = atlas_get_entry(modelholder->atlas, atlas_entry_name);

    if (!atlas_entry)
        atlas_entry = atlas_get_entry_with_number_suffix(modelholder->atlas, atlas_entry_name);

    return atlas_entry;
}

bool modelholder_get_texture_resolution(ModelHolder modelholder, int32_t* resolution_width, int32_t* resolution_height) {
    if (modelholder->atlas == &MODELHOLDER_STUB_ATLAS) {
        *resolution_width = FUNKIN_SCREEN_RESOLUTION_WIDTH;
        *resolution_height = FUNKIN_SCREEN_RESOLUTION_HEIGHT;
        return false;
    }
    return atlas_get_texture_resolution(modelholder->atlas, resolution_width, resolution_height);
}

bool modelholder_utils_is_known_extension(const char* filename) {
    if (!filename) return false;
    return atlas_utils_is_known_extension(filename) || string_lowercase_ends_with(filename, ".json");
}
