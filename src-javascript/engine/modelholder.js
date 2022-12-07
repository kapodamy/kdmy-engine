"use strict";

const MODELHOLDER_POOL = new Map();
var MODELHOLDER_IDS = 0;

var MODELHOLDER_STUB_ATLAS = { size: 0, glyph_fps: 0, texture_filename: null };
var MODELHOLDER_STUB_ANIMLIST = { entries_count: 0 };

async function modelholder_init(src) {
    let full_path = await fs_get_full_path_and_override(src);

    // find an instance of this 
    for (const instance of MODELHOLDER_POOL.values()) {
        if (instance.instance_src != full_path) continue;

        instance.instance_references++;
        full_path = undefined;
        return instance;
    }

    // C and C# only
    //fs_folder_stack_push();
    //fs_set_working_folder(src, 1);

    // load the model manifest (if was specified)
    let manifest_texture = null;
    let manifest_atlas = null;
    let manifest_animlist = null;
    let vertex_color = 0xFFFFFF;
    let from_manifest = string_lowercase_ends_with(src, ".json");

    if (from_manifest) {
        if (!await fs_file_exists(src)) {
            // C and C# only
            //fs_folder_stack_pop();
            return null;
        }
        let json = await json_load_from(src);
        manifest_texture = json_read_string(json, "texture", null);
        manifest_atlas = json_read_string(json, "atlas", null);
        manifest_animlist = json_read_string(json, "animlist", null);
        vertex_color = json_read_hex(json, "vertexColor", vertex_color)
        json_destroy(json);
    } else {
        let temp = src;
        let from_atlas = atlas_utils_is_known_extension(src);

        if (from_atlas) temp = fs_get_filename_without_extension(src);
        manifest_texture = string_concat(2, temp, ".png");
        manifest_atlas = from_atlas ? strdup(src) : string_concat(2, temp, ".xml");
        manifest_animlist = string_concat(2, temp, "_anims.xml");
        if (from_atlas) temp = undefined;
    }

    // JS only
    if (manifest_texture != null) manifest_texture = fs_build_path2(full_path, manifest_texture);
    if (manifest_atlas != null) manifest_atlas = fs_build_path2(full_path, manifest_atlas);
    if (manifest_animlist != null) manifest_animlist = fs_build_path2(full_path, manifest_animlist);

    let modelholder = {
        atlas: MODELHOLDER_STUB_ATLAS,
        animlist: MODELHOLDER_STUB_ANIMLIST,
        texture: null,
        vertex_color_rgb8: vertex_color,

        id: MODELHOLDER_IDS++,
        instance_references: 1,
        instance_src: full_path
    };

    if (manifest_atlas && await fs_file_exists(manifest_atlas)) {
        let atlas = await atlas_init(manifest_atlas);
        if (atlas) modelholder.atlas = atlas;
    }

    if (manifest_animlist && await fs_file_exists(manifest_animlist)) {
        let animlist = await animlist_init(manifest_animlist);
        if (animlist) modelholder.animlist = animlist;
    }

    if (manifest_texture && await fs_file_exists(manifest_texture)) {
        modelholder.texture = await texture_init(manifest_texture);
    } else {
        let altas_texture = atlas_get_texture_path(modelholder.atlas);
        if (altas_texture) {
            if ((await fs_file_exists(altas_texture))) {
                modelholder.texture = await texture_init(altas_texture);
            } else {
                console.warn(
                    "modelholder_init() missing texture '" +
                    altas_texture +
                    "' of atlas '" +
                    manifest_atlas +
                    "'"
                );
            }
        }
    }

    manifest_atlas = undefined;
    manifest_texture = undefined;
    manifest_animlist = undefined;

    // C and C# only
    //fs_folder_stack_pop();

    let no_atlas = modelholder.atlas == MODELHOLDER_STUB_ATLAS;
    let no_animlist = modelholder.animlist == MODELHOLDER_STUB_ANIMLIST;
    if (!from_manifest && !modelholder.texture && no_atlas && no_animlist) {
        console.warn(`modelholder_init() failed, missing resources of ${src}`);
        modelholder_destroy(modelholder);
        return null;
    }

    MODELHOLDER_POOL.set(modelholder.id, modelholder);
    return modelholder;
}

async function modelholder_init2(vertex_color_rgb8, atlas_src, animlist_src) {
    let modelholder = {
        atlas: MODELHOLDER_STUB_ATLAS,
        animlist: MODELHOLDER_STUB_ANIMLIST,
        texture: null,
        vertex_color_rgb8: vertex_color_rgb8,

        id: MODELHOLDER_IDS++,
        instance_references: 1,
        instance_src: null
    };
    MODELHOLDER_POOL.set(modelholder.id, modelholder);

    if (animlist_src && await fs_file_exists(animlist_src)) {
        modelholder.animlist = await animlist_init(animlist_src);
        if (!modelholder.animlist) modelholder.animlist = MODELHOLDER_STUB_ANIMLIST;
    }

    if (atlas_src && await fs_file_exists(atlas_src)) {
        modelholder.atlas = await atlas_init(atlas_src);
        if (!modelholder.atlas) modelholder.atlas = MODELHOLDER_STUB_ATLAS;

        if (modelholder.atlas) {
            let atlas_texture = atlas_get_texture_path(modelholder.atlas);
            if (atlas_texture) {
                if (await fs_file_exists(atlas_texture)) {
                    modelholder.texture = await texture_init(atlas_texture);
                } else {
                    console.error(`missing texture file: ${atlas_texture} in ${atlas_src}`);
                }
            }
        }
    }

    return modelholder;
}


function modelholder_destroy(modelholder) {

    // before continue, check if there another instances
    modelholder.instance_references--;
    if (modelholder.instance_references > 0) return;

    if (modelholder.atlas != MODELHOLDER_STUB_ATLAS && modelholder.atlas)
        atlas_destroy(modelholder.atlas);

    if (modelholder.animlist != MODELHOLDER_STUB_ANIMLIST)
        animlist_destroy(modelholder.animlist);

    if (modelholder.texture)
        texture_destroy(modelholder.texture);

    MODELHOLDER_POOL.delete(modelholder.id);
    ModuleLuaScript.kdmyEngine_drop_shared_object(modelholder);
    modelholder.instance_src = undefined;
    modelholder = undefined;
}


function modelholder_is_invalid(modelholder) {
    if (!modelholder) return 1;
    return modelholder.atlas == MODELHOLDER_STUB_ATLAS &&
        modelholder.animlist == MODELHOLDER_STUB_ANIMLIST &&
        !modelholder.texture;
}

function modelholder_has_animlist(modelholder) {
    return modelholder.animlist != MODELHOLDER_STUB_ANIMLIST;
}

function modelholder_get_atlas(modelholder) {
    return modelholder.atlas;
}

function modelholder_get_texture(modelholder, increment_reference) {
    if (!modelholder.texture)
        return null;
    else if (increment_reference)
        return texture_share_reference(modelholder.texture);
    else
        return modelholder.texture;
}

function modelholder_get_vertex_color(modelholder) {
    return modelholder.vertex_color_rgb8;
}

function modelholder_get_animlist(modelholder) {
    return modelholder.animlist;
}

function modelholder_create_sprite(modelholder, atlas_entry_name, with_animation) {
    let sprite;
    if (modelholder.texture) {
        sprite = sprite_init(texture_share_reference(modelholder.texture));
        let atlas_entry = modelholder_get_atlas_entry2(modelholder, atlas_entry_name, 0);
        if (atlas_entry) atlas_apply_from_entry(sprite, atlas_entry, 1);
    } else {
        sprite = sprite_init_from_rgb8(modelholder.vertex_color_rgb8);
    }

    if (with_animation) {
        let animsprite = modelholder_create_animsprite(modelholder, atlas_entry_name, 0, 0);
        if (animsprite) sprite_animation_add(sprite, animsprite);
    }

    return sprite;
}

function modelholder_create_animsprite(modelholder, animation_name, fallback_static, no_return_null) {
    if (modelholder.animlist != MODELHOLDER_STUB_ANIMLIST) {
        let animlist_item = animlist_get_animation(modelholder.animlist, animation_name);
        if (animlist_item) return animsprite_init(animlist_item);
    }

    let has_atlas = modelholder.atlas != MODELHOLDER_STUB_ATLAS;

    //L_build_from_atlas:
    if (/*modelholder.animlist == MODELHOLDER_STUB_ANIMLIST && */has_atlas) {
        /*if (
            !fallback_static &&
            !atlas_get_entry_with_number_suffix(modelholder.atlas, animation_name)
        ) {
            break L_build_from_atlas;
        }*/

        // try build an animation using atlas frames (loops indefinitely at 24 FPS)
        let animsprite = animsprite_init_from_atlas(
            FUNKIN_DEFAULT_ANIMATIONS_FRAMERATE, 0, modelholder.atlas, animation_name, 1
        );
        if (animsprite) return animsprite;

        if (fallback_static) {
            // Try build a static animation (single atlas frame)
            let atlas_entry = atlas_get_entry(modelholder.atlas, animation_name);
            if (atlas_entry) {
                return animsprite_init_from_atlas_entry(
                    atlas_entry, 0, FUNKIN_DEFAULT_ANIMATIONS_FRAMERATE
                );
            }
        }
    }

    if (no_return_null)
        return animsprite_init_as_empty(animation_name);
    else
        return null;
}

function modelholder_get_atlas_entry(modelholder, atlas_entry_name, return_copy) {
    if (modelholder.atlas == MODELHOLDER_STUB_ATLAS) return null;

    if (return_copy)
        return atlas_get_entry_copy(modelholder.atlas, atlas_entry_name);
    else
        return atlas_get_entry(modelholder.atlas, atlas_entry_name);
}

/**
 * Retrieves an atlas entry with the specified name. If the entry is not found, retrieves the first
 * entry with number suffix (example "dance left000"), 
 * @param {Object} modelholder The modelholder instance
 * @param {string} atlas_entry_name Entry name and/or entry name prefix
 * @param {bool} return_copy 1 to clone entry (must be free() later), otherwise, 0 to obtain an reference
 * @returns {object} the atlas entry or NULL if not found
 */
function modelholder_get_atlas_entry2(modelholder, atlas_entry_name, return_copy) {
    if (modelholder.atlas == MODELHOLDER_STUB_ATLAS) return null;

    let atlas_entry = atlas_get_entry(modelholder.atlas, atlas_entry_name);

    if (!atlas_entry)
        atlas_entry = atlas_get_entry_with_number_suffix(modelholder.atlas, atlas_entry_name);

    if (atlas_entry && return_copy) atlas_entry = clone_object(atlas_entry);

    return atlas_entry;
}

function modelholder_get_texture_resolution(modelholder, output_resolution) {
    if (modelholder.atlas == MODELHOLDER_STUB_ATLAS) {
        output_resolution[0] = FUNKIN_SCREEN_RESOLUTION_WIDTH;
        output_resolution[1] = FUNKIN_SCREEN_RESOLUTION_HEIGHT;
        return 0;
    }
    return atlas_get_texture_resolution(modelholder.atlas, output_resolution);
}

function modelholder_utils_is_known_extension(filename) {
    if (!filename) return false;
    return atlas_utils_is_known_extension(filename) || string_lowercase_ends_with(filename, ".json");
}

