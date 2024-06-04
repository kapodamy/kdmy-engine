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
    //fs_set_working_folder(src, true);

    // load the model manifest (if was specified)
    let manifest_texture = null;
    let manifest_atlas = null;
    let manifest_animlist = null;
    let fallback_texture_path = null;
    let vertex_color = 0xFFFFFF;
    let from_manifest = string_lowercase_ends_with(full_path, ".json");

    if (from_manifest) {
        if (!await fs_file_exists(full_path)) {
            // C and C# only
            //fs_folder_stack_pop();
            return null;
        }
        let json = await json_load_from(full_path);
        manifest_texture = strdup(json_read_string(json, "texture", null));
        manifest_atlas = strdup(json_read_string(json, "atlas", null));
        manifest_animlist = strdup(json_read_string(json, "animlist", null));
        vertex_color = json_read_hex(json, "vertexColor", vertex_color);
        json_destroy(json);
    } else {
        let temp = full_path;
        let from_atlas = atlas_utils_is_known_extension(full_path);

        if (from_atlas) temp = fs_get_filename_without_extension(full_path);
        let atlas_filename = from_atlas ? strdup(full_path) : string_concat(2, temp, ".xml");
        let animlist_filename = string_concat(2, temp, "_anims.xml");
        let fallback_texture_filename = string_concat(2, temp, ".png");
        if (from_atlas) temp = undefined;

        manifest_atlas = fs_build_path2(full_path, atlas_filename);
        manifest_animlist = fs_build_path2(full_path, animlist_filename);
        fallback_texture_path = fs_build_path2(full_path, fallback_texture_filename);

        atlas_filename = undefined;
        animlist_filename = undefined;
        fallback_texture_filename = undefined;
    }

    //
    // (JS Only) to avoid use "fs_set_working_folder()" function build the absolute path here
    // this is a workarround because there no thread local storage (TLS) available on javascript
    //
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

    if (modelholder.atlas && modelholder.atlas != MODELHOLDER_STUB_ATLAS && !manifest_texture) {
        let altas_texture = atlas_get_texture_path(modelholder.atlas);
        if (altas_texture && await fs_file_exists(altas_texture)) {
            modelholder.texture = await texture_init(altas_texture);
            if (!modelholder.texture) {
                console.error(
                    `modelholder_init() atlas texture not found: atlas=${manifest_atlas} texture=${altas_texture}`
                );
            }
        }
    }

    if (!modelholder.texture) {
        let texture_src = manifest_texture ?? fallback_texture_path;
        if (await fs_file_exists(texture_src)) {
            modelholder.texture = await texture_init(texture_src);
        } else {
            console.error(`modelholder_init() missing manifest texture: ${texture_src}`);
        }
    }

    fallback_texture_path = undefined;
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

    let atlas_texture = null;

    if (atlas_src && await fs_file_exists(atlas_src)) {
        modelholder.atlas = await atlas_init(atlas_src);

        if (modelholder.atlas != null)
            atlas_texture = atlas_get_texture_path(modelholder.atlas);
        else
            modelholder.atlas = MODELHOLDER_STUB_ATLAS;
    }

    if (atlas_texture && await fs_file_exists(atlas_texture)) {
        modelholder.texture = await texture_init(atlas_texture);
    } else if (atlas_src) {
        // try use atlas name instead
        fs_folder_stack_push();
        fs_set_working_folder(atlas_src, true);

        let temp = fs_get_filename_without_extension(atlas_src);
        let texture_path = string_concat(2, temp, ".png");

        if (await fs_file_exists(texture_path)) {
            modelholder.texture = await texture_init(texture_path);
        }

        if (modelholder.texture != null) {
            console.warn(`modelholder_init2() expected ${atlas_texture}, found ${texture_path}`);
        } else {
            console.error(`modelholder_init2() missing texture file: ${atlas_texture} in ${atlas_src}`);
        }

        temp = undefined;
        texture_path = undefined;
        fs_folder_stack_pop();
    }

    return modelholder;
}

function modelholder_init3(vertex_color_rgb8, texture, atlas, animlist) {
    let modelholder = {
        atlas: atlas ?? MODELHOLDER_STUB_ATLAS,
        animlist: animlist ?? MODELHOLDER_STUB_ANIMLIST,
        texture: texture,
        vertex_color_rgb8: vertex_color_rgb8,

        id: MODELHOLDER_IDS++,
        instance_references: 1,
        instance_src: null
    };

    MODELHOLDER_POOL.set(modelholder.id, modelholder);
    return modelholder;
}


function modelholder_destroy(modelholder) {

    // before continue, check if there another instances
    modelholder.instance_references--;
    if (modelholder.instance_references > 0) return;

    if (modelholder.instance_src != null) {
        if (modelholder.atlas != MODELHOLDER_STUB_ATLAS && modelholder.atlas)
            atlas_destroy(modelholder.atlas);

        if (modelholder.animlist != MODELHOLDER_STUB_ANIMLIST)
            animlist_destroy(modelholder.animlist);

        if (modelholder.texture)
            texture_destroy(modelholder.texture);
    }

    MODELHOLDER_POOL.delete(modelholder.id);
    luascript_drop_shared(modelholder);
    modelholder.instance_src = undefined;
    modelholder = undefined;
}


function modelholder_is_invalid(modelholder) {
    if (!modelholder) return true;
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
        let atlas_entry = modelholder_get_atlas_entry2(modelholder, atlas_entry_name);
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

function modelholder_get_atlas_entry(modelholder, atlas_entry_name) {
    if (modelholder.atlas == MODELHOLDER_STUB_ATLAS) return null;
    return atlas_get_entry(modelholder.atlas, atlas_entry_name);
}

/**
 * Retrieves an atlas entry with the specified name. If the entry is not found, retrieves the first
 * entry with number suffix (example "dance left000"), 
 * @param {Object} modelholder The modelholder instance
 * @param {string} atlas_entry_name Entry name and/or entry name prefix
 * @returns {object} the atlas entry or NULL if not found
 */
function modelholder_get_atlas_entry2(modelholder, atlas_entry_name) {
    if (modelholder.atlas == MODELHOLDER_STUB_ATLAS) return null;

    let atlas_entry = atlas_get_entry(modelholder.atlas, atlas_entry_name);

    if (!atlas_entry)
        atlas_entry = atlas_get_entry_with_number_suffix(modelholder.atlas, atlas_entry_name);

    return atlas_entry;
}

function modelholder_get_texture_resolution(modelholder, output_resolution) {
    if (modelholder.atlas == MODELHOLDER_STUB_ATLAS) {
        output_resolution[0] = FUNKIN_SCREEN_RESOLUTION_WIDTH;
        output_resolution[1] = FUNKIN_SCREEN_RESOLUTION_HEIGHT;
        return false;
    }
    return atlas_get_texture_resolution(modelholder.atlas, output_resolution);
}

function modelholder_utils_is_known_extension(filename) {
    if (!filename) return false;
    return atlas_utils_is_known_extension(filename) || string_lowercase_ends_with(filename, ".json");
}

