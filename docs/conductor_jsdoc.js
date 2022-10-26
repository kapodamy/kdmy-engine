
/**
 * @typedef {object} MapDirection
 * @property {string} name
 * @property {bool} is_extra
 */

/**
 * @typedef {object} Mapping
 * @property {object} strum
 * @property {MapDirection[]} directions
 * @property {number} last_timestamp
 * @property {bool} is_disabled
 */

/**
 * @typedef {object} Conductor
 * @property {object} character
 * @property {Mapping[]} mappings
 * @property {bool} disable
 * @property {string} last_penality_direction
 */

