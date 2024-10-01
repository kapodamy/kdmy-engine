"use strict";

const FONTCACHE_FOLDER = ".fontcache";

/**@type {Cache}*/
var fontcache_storage = null;
var fontcache_failed = false;

/**
 * @param {FontAtlas} fontatlas
 * @param {number} font_height 
 * @param {number} revision
 * @param {string} suffix 
 * @returns {Promise<FontCharMap>}
 */
async function fontcache_fontcache_load(fontatlas, font_height, revision, suffix) {
    if (!fontcache_storage && !fontcache_failed) fontcache_internal_init();
    if (fontcache_failed) return null;

    let xxhash = fontatlas_xxhash(fontatlas);
    let cache_path = fontcache_internal_create_path(xxhash, suffix);

    let req = new Request(cache_path, { method: "GET", cache: "force-cache", mode: "same-origin" });
    let cache = null;

    try {
        let res = await fontcache_storage.match(req);
        if (!res) return null;
        cache = await res.arrayBuffer();

    } catch (e) {
        console.error("fontcache_store() failed for " + cache_path, e);
    }

    return fontatlas_deserialize_atlas(fontatlas, cache, font_height, revision);
}

/**
 * @param {FontAtlas} fontatlas
 * @param {FontCharMap} charmap
 * @param {number} font_height 
 * @param {number} revision
 * @param {string} suffix 
 * @returns {Promise<void>}
 */
async function fontcache_store(fontatlas, charmap, font_height, revision, suffix) {
    if (!fontcache_storage && !fontcache_failed) fontcache_internal_init();
    if (fontcache_failed) return;

    let xxhash = fontatlas_xxhash(fontatlas);
    let cache_path = fontcache_internal_create_path(xxhash, suffix);

    let cache = fontatlas_serialize_atlas(fontatlas, charmap, font_height, revision);

    let date = new Date();
    date = new Date(date.setDate(date.getMonth() + 1));

    let req = new Request(cache_path, { method: "GET" });
    let res = new Response(cache, {
        status: 200,
        statusText: "OK",
        headers: {
            "Content-Type": "application/octet-stream",
            "Expires": date.toUTCString()
        }
    });

    try {
        await fontcache_storage.put(req, res);
    } catch (e) {
        console.error("fontcache_store() failed for " + cache_path, e);
    }
}


function fontcache_internal_create_path(xxhash, suffix) {
    let xxhash_str = xxhash.toString(16).padStart(BigUint64Array.BYTES_PER_ELEMENT * 2, '0');

    return `${IO_BASE_URL}/${FONTCACHE_FOLDER}/${xxhash_str}${suffix}.bin`;
}

async function fontcache_internal_init() {
    try {
        fontcache_storage = await window.caches.open("fontcache");
    } catch (e) {
        console.error("fontcache_internal_init() cache open failed", e);
        fontcache_failed = true;
    }
}

