
/**
 * @typedef {object} ModuleFontAtlas
 * @property {function(boolean):void} enable_sdf
 * @property {function(ArrayBuffer, number):FontAtlas} init
 * @property {function(FontAtlas):void} destroy
 * @property {function(FontAtlas, number, number, Uint32Array):FontCharMap} atlas_build
 * @property {function(FontAtlas, number, number):FontCharMap} atlas_build_complete
 * @property {function(FontCharMap):void} atlas_destroy_texture_only
 * @property {function(FontCharMap):void} atlas_destroy
 * @property {function():string} get_version
 */

/**
 * @typedef {object} FontAtlas
 * @property {number} fontatlas_ptr
 * @property {number} font_data_ptr
 */

/**
 * @typedef {object} FontCharMap
 * @property {FontCharData[]} char_array
 * @property {number} char_array_size
 * @property {Uint8Array} texture
 * @property {number} texture_width
 * @property {number} texture_height
 * @property {number} texture_byte_size
 * @property {number} ascender
 * @property {number} line_height
 * @property {number} __ptr
 */

/**
 * @typedef {object} FontCharData
 * @property {number} codepoint
 * @property {number} offset_y
 * @property {number} offset_x
 * @property {number} advancex
 * @property {number} advancey
 * @property {number} width
 * @property {number} height
 * @property {FontCharDataKerning[]} kernings
 * @property {number} kernings_size
 * @property {FontCharDataAtlasEntry} atlas_entry
 * @property {boolean} has_atlas_entry
 */

/**
 * @typedef {object} FontCharDataAtlasEntry
 * @property {number} x
 * @property {number} y
*/

/**
 * @typedef {object} FontCharDataKerning
 * @property {number} codepoint
 * @property {number} x
 */


/**@type {Uint8Array} */
var HEAPU8;

/**@type {WebAssembly.Memory} */
var wasmMemory;

var Module = {};

/**
 * @param {number} sz 
 * @returns {number}
 */
function _malloc(sz) { return 1; }

/**
 * @param {number} ptr 
 */
function _free(ptr) { }


/** @param {number} stub */
function _fontatlas_enable_sdf(stub) { }

/** 
* @param {...number} stubs
* @returns {number} */
function _fontatlas_init(...stubs) { return 0; }

/** 
* @param {number} stub
*/
function _fontatlas_destroy_JS(stub) { }

/** 
* @param {...number} stubs
* @returns {number} */
function _fontatlas_atlas_build(...stubs) { return 0; }

/** 
 * @param {...number} stubs
 * @returns {number} */
function _fontatlas_atlas_build_complete(...stubs) { return 0; }

/** @param {number} stub */
function _fontatlas_atlas_destroy_texture_only(stub) { }

/** @param {number} stub */
function _fontatlas_atlas_destroy(stub) { }

/** @returns {number} */
function _fontatlas_get_version() { return 0; }

/** @param {number} stub */
function malloc_for_array(stub) { return new Array(stub); }
