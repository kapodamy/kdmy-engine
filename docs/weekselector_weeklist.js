
/**
 * @typedef {object} WeekListVisibleItem
 * @property {bool} is_locked 
 * @property {object} icon_locked 
 * @property {object} sprite_title
 * @property {object} tweenlerp_locked
 * @property {object} tweenlerp_title
 */

/**
 * @typedef {object} WeekList
 * @property {number} index
 
 * @property {WeekListVisibleItem[]} list_visible
 * 
 * @property {number} x
 * @property {number} y
 * @property {number} width
 * @property {number} row_height
 * 
 * @property {number} progress
 * @property {bool} do_reverse
 * 
 * @property {object} drawable_list
 * @property {object} drawable_host
 * 
 * @property {object} host_statesprite
 * @property {bool} host_loading
 * @property {object} host_placeholder
 * @property {number} host_load_id
 * 
 * @property {object} texturepool
 * 
 * @property {object} anim_selected
 * @property {bool} week_choosen
 * 
 * @property {number} beat_duration
 * @property {number} beat_progress
 * 
*/
