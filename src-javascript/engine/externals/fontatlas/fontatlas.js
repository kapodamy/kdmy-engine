"use strict";


/**
 * @param {boolean} enable 
 */
function fontatlas_enable_sdf(enable) {
    FontAtlas.enable_sdf(enable);
}

/**
 * 
 * @param {ArrayBuffer} font_data 
 * @param {number} font_data_size 
 * @returns {FontAtlas}
 */
function fontatlas_init(font_data, font_data_size) {
    return FontAtlas.init(font_data, font_data_size);
}

/** 
 * @param {FontAtlas} fontatlas
 */
function fontatlas_destroy(fontatlas) {
    FontAtlas.destroy(fontatlas);
}

/**
 * @param {FontAtlas} fontatlas 
 * @param {number} font_height 
 * @param {number} gaps 
 * @param {Uint32Array|Array<number>} characters_to_add 
 * @returns {FontCharMap}
 */
function fontatlas_atlas_build(fontatlas, font_height, gaps, characters_to_add) {
    if (characters_to_add instanceof Uint32Array)
        return FontAtlas.atlas_build(fontatlas, font_height, gaps, characters_to_add);
    else
        return FontAtlas.atlas_build(fontatlas, font_height, gaps, new Uint32Array(characters_to_add));
}

/**
 * @param {FontAtlas} fontatlas 
 * @param {number} font_height 
 * @param {number} gaps 
 * @returns {FontCharMap}
 */
function fontatlas_atlas_build_complete(fontatlas, font_height, gaps) {
    return FontAtlas.atlas_build_complete(fontatlas, font_height, gaps);
}

/**
 * @param {FontCharMap} fontcharmap 
 */
function fontatlas_atlas_destroy_texture_only(fontcharmap) {
    FontAtlas.atlas_destroy_texture_only(fontcharmap);
}

/**
 * @param {FontCharMap} fontcharmap 
 */
function fontatlas_atlas_destroy(fontcharmap) {
    FontAtlas.atlas_destroy(fontcharmap);
}

/**
 * @returns {string}
 */
function fontatlas_get_version() {
    return FontAtlas.get_version();
}
