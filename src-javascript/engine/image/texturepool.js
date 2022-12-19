"use strict";

function texturepool_init(max_size) {
    // JS only
    max_size *= 8;
    return { max_bytes: max_size, used_bytes: 0, list: linkedlist_init() };
}

function texturepool_destroy(texpool) {
    for (const tex of linkedlist_iterate4(texpool.list)) {
        texture_cache(tex, 0);
        texture_destroy(tex);
    }
    linkedlist_destroy(texpool.list);
    texpool = undefined;
}

function texturepool_add(texpool, texture) {
    if (!texture || texture.src_filename) return;// illegal operation

    // check if already present
    for (const tex of linkedlist_iterate4(texpool.list)) {
        if (tex == texture) return;
    }

    // Important: disable SH4 interrupts first, this is a critical part
    // let old_irq = irq_disable();

    // check if can afford this texture
    let used_bytes = texpool.used_bytes + texture_get_size(texture);

    while (used_bytes > texpool.max_bytes) {
        // release textures at the start of the list
        let tex = linkedlist_get_first_item(texpool.list);
        if (!tex) break;// empty list

        used_bytes -= texture_get_size(tex);
        if (used_bytes < texpool.max_bytes) break;
    }

    // adquire
    texpool.used_bytes = used_bytes;
    texture_cache(texture, 1);
    texture_share_reference(texture);
    linkedlist_add_item(texpool.list, texture);

    // now restore CPU's interrupts
    // irq_restore(old_irq);
}

