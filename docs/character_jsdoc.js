


/**
 * @typedef {object} Animsprite
 * @property {number} id
 */

/**
 * @typedef {object} Texture
 * @property {number} id
 */

/**
 * @typedef {object} CharacterTextureInfo
 * @property {object} texture
 * @property {number} resolution_width
 * @property {number} resolution_height
 */

/**
 * @typedef {object} CharacterActionMiss
 * 
 * @property {number} id_direction
 * @property {Animsprite} animation
 * @property {bool} beat_stop
 * @property {number} id_texture
 * @property {number} offset_x
 * @property {number} offset_y
 */

/**
 * @typedef {object} CharacterActionSing
 * 
 * @property {number} id_direction
 * @property {Animsprite} base
 * @property {Animsprite} hold
 * @property {Animsprite} rollback
 * @property {bool} base_can_rollback
 * @property {bool} hold_can_rollback
 * @property {bool} follow_hold
 * @property {bool} full_sustain
 * @property {number} id_texture
 * @property {number} offset_x
 * @property {number} offset_y
 *
 */

/**
 * @typedef {object} CharacterActionExtra
 * 
 * @property {number} id_extra
 * @property {bool} beat_stop
 * @property {bool} is_valid
 * @property {bool} static_until_beat
 * @property {Animsprite} base
 * @property {Animsprite} hold
 * @property {Animsprite} rollback
 * @property {number} id_texture
 * @property {number} offset_x
 * @property {number} offset_y
 */

/**
 * @typedef {object} CharacterState
 * @property {string} name
 * @property {CharacterActionSing[]} sing
 * @property {CharacterActionSing[]} sing_alt
 * @property {CharacterActionMiss[]} miss
 * @property {CharacterActionExtra[]} extras
 * @property {CharacterActionExtra} hey
 * @property {CharacterActionExtra} idle
 */

/**
 * @typedef {object} BeatWatcher
 * @property {bool} count_beats_or_quarters
 * @property {number} tick
 * @property {number} count
 * @property {number} drift_count
 * @property {number} drift_timestamp
 * @property {number} since
 */

/**
 * @typedef {object} Character
 * @property {object} statesprite
 * 
 * @property {number} current_texture_id
 * @property {ArrayList<CharacterTextureInfo>} textures
 * 
 * @property {number} sing_size
 * @property {number} miss_size
 * @property {number} extras_size
 * @property {number} all_extras_names_size
 * @property {number} all_directions_names_size
 * @property {string[]} all_directions_names
 * @property {string[]} all_extras_names
 *
 * @property {CharacterState} default_state
 * @property {ArrayList<CharacterState>} states
 *
 * @property {CharacterState} current_state
 *
 * @property {Animsprite} current_anim
 * @property {bool} current_use_frame_rollback
 * @property {CharacterAnimType} current_anim_type
 * @property {CharacterActionExtra} current_action_extra
 * @property {CharacterActionMiss} current_action_miss
 * @property {CharacterActionSing} current_action_sing
 * @property {CharacterActionType} current_action_type
 * @property {number} current_stop_on_beat
 * @property {bool} current_sing_follow_hold
 * @property {bool} current_sing_non_sustain
 *
 * 
 * @property {bool} alt_enabled
 * 
 * @property {bool} continuous_idle
 * 
 * @property {BeatWatcher} beatwatcher
 * 
 * @property {number} speed
 * @property {bool} allow_speed_change
 * 
 * @property {object} drawable
 * @property {object} drawable_animation
 *
 * @property {number} draw_x
 * @property {number} draw_y
 *
 * @property {number} align_vertical
 * @property {number} align_horizontal
 * @property {number} manifest_align_vertical
 * @property {number} manifest_align_horizontal
 *
 * @property {number} reference_width
 * @property {number} reference_height
 * @property {bool} enable_reference_size
 *
 * @property {number} offset_x
 * @property {number} offset_y
 *
 * @property {bool} is_left_facing
 *
 * @property {bool} inverted_enabled
 * @property {bool} inverted_correction_enabled
 * @property {number} inverted_size
 * @property {number[]} inverted_from
 * @property {number[]} inverted_to
 *
 * @property {number} character_scale
 */

