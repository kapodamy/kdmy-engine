"use strict";

var TEXTURE_IDS = 0;
const TEXTURE_POOL = new Map();
var texture_defer_enabled = 0;


async function texture_init(src) {
    return texture_init_deferred(src, texture_defer_enabled);
}

async function texture_init_deferred(src, deffered) {
    let loaded_texture = await texture_get_from_global_pool(src);
    if (loaded_texture) {
        // This texture is already in use, bump the reference count and reuse it
        loaded_texture.references++;
        return loaded_texture;
    }

    let tex;
    try {
        tex = await fs_readimagebitmap(src);
    } catch (e) {
        console.error(e);
        return null;
    }

    let texture = texture_init_from_raw(tex.data, tex.size, 0, tex.data.width, tex.data.height, 0, 0);

    // remember the filename to avoid duplicated textures
    texture.src_filename = fs_get_full_path(src);

    // original size if was readed from an standard image file (png, jpg, bmp)
    texture.original_width = tex.original_width;
    texture.original_height = tex.original_height;

    if (!deffered) texture_upload_to_pvr(texture);

    return texture;
}

function texture_init_from_raw(ptr, size, in_vram, width, height, orig_width, orig_height) {
    let texture = {
        size: size,
        data_vram: null,
        data_ram: null,

        width: width,
        height: height,
        original_width: orig_width,
        original_height: orig_height,

        id: TEXTURE_IDS++,
        src_filename: null,
        references: 1,
        cache_references: 0,
        has_mipmaps: false
    }

    if (in_vram)
        texture.data_vram = ptr;
    else
        texture.data_ram = ptr;

    if (!in_vram && texture.data_ram instanceof DDS) {
        texture.has_mipmaps = ptr.Mipmaps.length > 0;
    }

    TEXTURE_POOL.set(texture.id, texture);

    return texture;
}


function texture_share_reference(texture) {
    texture.references++;
    return texture;
}

async function texture_cache(texture, adquire) {
    if (adquire)
        texture.cache_references++;
    else if (texture.cache_references > 0)
        texture.cache_references--;

    //
    // This functionality is only available in the Dreamcast
    // OpenGL and WebGL has lot of limitations
    //

    // C only
    /*
    
    // if there not cached references, remove from the SH-4 RAM
    if (texture->cache_references < 1 && texture->data_ram && texture->data_vram) {
         free(texture->data_ram);
         texture->data_ram = null;
        return;
    }

    // there chache references and the texture still exists on SH-4 side
    if (texture->cache_references > 0 && texture->data_ram) return;

    // download the texture from the PVR VRAM, this in other platforms is nearly impossible
    texture->data_ram = malloc(texture->size);
    memcpy(texture->data_ram, texture->data_vram, texture->size);

    // now remove from the PVR VRAM
    pvr_mem_free(texture->data_vram);
    texture->data_vram = null;

    */
}

function texture_upload_to_pvr(texture) {
    if (texture.data_vram) return;

    if (!texture.data_ram) {
        // null data loaded
        return;
    }

    // C only
    /*
    texture->data_vram = pvr_mem_malloc(texture->size);
    if (!texture->data_vram) {
        printf("No enough video memory\n");
        return;
    }

    pvr_txr_load(texture->data_ram, texture->data_vram, texture->size);

    if (texture.cache->references > 0) return;

    free(texture->data_ram);
    texture->data_ram = null;
    return;
    */

    texture.data_vram = webopengl_create_texture(pvr_context.webopengl, texture.width, texture.height, texture.data_ram);

    if (texture.cache_references > 0) return;

    texture.data_ram.close();
    texture.data_ram = null;
}

function texture_get_size(texture) {
    return texture.size;
}

function texture_get_original_dimmensions(texture, output_dimmensions) {
    output_dimmensions[0] = texture.original_width;
    output_dimmensions[1] = texture.original_height;
    return output_dimmensions;
}

function texture_get_dimmensions(texture, output_dimmensions) {
    output_dimmensions[0] = texture.width;
    output_dimmensions[1] = texture.height;
    return output_dimmensions;

}


function texture_destroy_force(texture) {
    if (!texture) return;
    texture.references = 0;
    texture.cache_references = 0;
    texture_destroy(texture);
}

function texture_destroy(texture) {
    if (!texture) return 1;

    texture.references--;

    // check if the texture is shared somewhere
    if (texture.references > 0) {
        // C only
        /*

        // if still in use and cached, release from the PVR
        if (texture->data_ram && texture->references == texture->cache_references) {
            pvr_mem_free(texture->data_vram);
            texture->data_vram = null;
        }

        */
        return 0;
    }

    // C only
    /*
    
    if (texture->data_ram) {
        free(texture->data_ram);
    }
    if (texture->data_vram) {
        pvr_mem_free(texture->data_vram);
    }

    TEXTURE_POOL.delete(texture.id);

    string_destroy(texture->src_filename);

    free(texture);
    return 1;

    */

    // JS only
    if (texture.ram) {
        texture.data_ram.close();
    }
    if (texture.data_vram) {
        webopengl_destroy_texture(pvr_context.webopengl, texture);
    }

    texture.ram = null;
    texture.vram = null;
    texture.src_filename = undefined;
    TEXTURE_POOL.delete(texture.id);
    return 1;
}


async function texture_get_from_global_pool(src) {
    let texture = null;
    if (src) {
        let filename = await fs_get_full_path_and_override(src);
        for (let loaded_texture of TEXTURE_POOL.values()) {
            if (loaded_texture.src_filename == filename) {
                texture = loaded_texture;
                break;
            }
        }
        filename = undefined;
    }
    return texture;
}

function texture_disable_defering(disable) {
    texture_defer_enabled = disable ? 0 : 1;
}
