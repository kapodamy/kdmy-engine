"use strict";


function conductor_init() {
    return {
        character: null,
        disable: 0,
        mappings: arraylist_init2(4),
        last_penality_strum: null,
        last_sustain_strum: null,
        missnotefx: null,
        has_misses: 0,
        has_hits: 0,
        play_calls: 0
    };
}

function conductor_destroy(conductor) {
    ModuleLuaScript.kdmyEngine_drop_shared_object(conductor);

    conductor_clear_mapping(conductor);
    arraylist_destroy(conductor.mappings, 0);
    conductor = undefined;
}


function conductor_poll_reset(conductor) {
    for (let map of arraylist_iterate4(conductor.mappings)) {
        map.last_change_count = -1;
    }
    conductor.last_penality_strum = null;
    conductor.last_sustain_strum = null;
    conductor.has_misses = 0;
    conductor.has_hits = 0;
}

function conductor_set_character(conductor, character) {
    if (conductor.character != character) conductor.last_sustain_strum = conductor.last_penality_strum = null;
    conductor.character = character;

    if (character != null)
        conductor.play_calls = character_get_play_calls(character) - 1;
}

function conductor_use_strum_line(conductor, strum) {
    if (!strum) return;

    // check if is already in use
    for (let map of arraylist_iterate4(conductor.mappings)) {
        if (map.strum == strum) {
            map.is_disabled = 0;
            return;
        }
    }

    arraylist_add(conductor.mappings, {
        strum: strum,
        last_change_count: -1,
        directions: arraylist_init2(4),
        is_disabled: 0
    });
}

function conductor_use_strums(conductor, strums) {
    if (!strums) return;

    let size = strums_get_lines_count(strums);
    for (let i = 0; i < size; i++) {
        let strum = strums_get_strum_line(strums, i);
        conductor_use_strum_line(conductor, strum);
    }
}

function conductor_disable_strum_line(conductor, strum, should_disable) {
    if (!strum) return 0;
    for (let map of arraylist_iterate4(conductor.mappings)) {
        if (map.strum == strum) {
            map.is_disabled = !!should_disable;
            return 1;
        }
    }
    return 0;
}

function conductor_remove_strum(conductor, strum) {
    let array = arraylist_peek_array(conductor.mappings);
    let size = arraylist_size(conductor.mappings);
    for (let i = 0; i < size; i++) {
        if (array[i].strum == strum) {
            conductor_internal_disposed_mapped_strum(array[i]);
            arraylist_remove_at(conductor.mappings, i);
            return 1;
        }
    }
    return 0;
}

function conductor_clear_mapping(conductor) {
    for (let map of arraylist_iterate4(conductor.mappings)) {
        conductor_internal_disposed_mapped_strum(map);
    }
    arraylist_clear(conductor.mappings);
    conductor.last_sustain_strum = conductor.last_penality_strum = null;
}


function conductor_map_strum_to_player_sing_add(conductor, strum, sing_direction_name) {
    conductor_intenal_add_mapping(conductor, strum, sing_direction_name, 0);
}

function conductor_map_strum_to_player_extra_add(conductor, strum, extra_animation_name) {
    conductor_intenal_add_mapping(conductor, strum, extra_animation_name, 1);
}

function conductor_map_strum_to_player_sing_remove(conductor, strum, sing_direction_name) {
    conductor_intenal_remove_mapping(conductor, strum, sing_direction_name, 0);
}

function conductor_map_strum_to_player_extra_remove(conductor, strum, extra_animation_name) {
    conductor_intenal_remove_mapping(conductor, strum, extra_animation_name, 1);
}


function conductor_map_automatically(conductor, should_map_extras) {
    if (!conductor.character) {
        console.error("conductor_map_automatically() failed, no character set");
        return 0;
    }

    let mappings = arraylist_peek_array(conductor.mappings);
    let size = arraylist_size(conductor.mappings);

    if (size < 1) {
        console.error("conductor_map_automatically() failed, no strums are set");
        return 0;
    }

    let count = 0;
    for (let i = 0; i < size; i++) {
        let strum_name = strum_get_name(mappings[i].strum);

        if (character_has_direction(conductor.character, strum_name, 0)) {
            // strum_name --> sing_direction
            conductor_map_strum_to_player_sing_add(conductor, mappings[i].strum, strum_name);
            count++;
            continue;
        } else if (should_map_extras && character_has_direction(conductor.character, strum_name, 1)) {
            // strum_mame --> extra_animation
            conductor_map_strum_to_player_extra_add(conductor, mappings[i].strum, strum_name);
            count++;
            break;
        }
    }

    return count;
}

function conductor_set_missnotefx(conductor, missnotefx) {
    conductor.missnotefx = missnotefx;
}

function conductor_poll(conductor) {
    if (conductor.disable || !conductor.character) return;

    let array = arraylist_peek_array(conductor.mappings);
    let size = arraylist_size(conductor.mappings);
    let success = 0;

    conductor.has_hits = 0;
    conductor.has_misses = 0;

    for (let i = 0; i < size; i++) {
        if (array[i].is_disabled) continue;

        let press_changes = strum_get_press_state_changes(array[i].strum);
        let press_state = strum_get_press_state(array[i].strum);

        // check if the strums was updated
        if (press_changes == array[i].last_change_count) continue;
        array[i].last_change_count = press_changes;

        switch (press_state) {
            case STRUM_PRESS_STATE_MISS:
                conductor.has_misses = true;
            case STRUM_PRESS_STATE_NONE:
                if (conductor.last_penality_strum == array[i].strum) {
                    // stop the penality animation in the next beat
                    conductor_internal_execute_miss(conductor.character, array[i].directions, 0);
                    conductor.last_penality_strum = null;
                } else if (!conductor.last_sustain_strum || conductor.last_sustain_strum == array[i].strum) {
                    conductor.last_sustain_strum = null;
                    if (!character_is_idle_active(conductor.character)) {
                        //
                        // only play the idle animation if the last action
                        // was done by the conductor itself
                        //
                        if (conductor.play_calls == character_get_play_calls(conductor.character)) {
                            character_play_idle(conductor.character);
                            conductor.play_calls = character_get_play_calls(conductor.character);
                        }
                    }
                }
                break;
            case STRUM_PRESS_STATE_HIT:
                //conductor.current_duration = strum_get_marker_duration(array[i].strum);
                conductor.last_sustain_strum = null;
                conductor.last_penality_strum = null;
                conductor.has_misses = 0;
                conductor.has_hits = 1;
                success += conductor_internal_execute_sing(conductor.character, array[i].directions, 0);
                break;
            case STRUM_PRESS_STATE_HIT_SUSTAIN:
                conductor.last_sustain_strum = array[i].strum;
                conductor.last_penality_strum = null;
                conductor.has_misses = 0;
                conductor.has_hits = 1;
                success += conductor_internal_execute_sing(conductor.character, array[i].directions, 1);
                break;
            case STRUM_PRESS_STATE_PENALTY_NOTE:
                // button press on empty strum
                conductor.has_misses = 1;
                conductor.last_sustain_strum = null;
                conductor.last_penality_strum = array[i].strum;
                success += conductor_internal_execute_miss(conductor.character, array[i].directions, 1);
                if (conductor.missnotefx) missnotefx_play_effect(conductor.missnotefx);
                break;
            case STRUM_PRESS_STATE_PENALTY_HIT:
                // wrong note button
                conductor.last_sustain_strum = null;
                conductor.last_penality_strum = array[i].strum;
                success += conductor_internal_execute_miss(conductor.character, array[i].directions, 0);
                if (conductor.missnotefx) missnotefx_play_effect(conductor.missnotefx);
                break;
            default:
                break;
        }
    }

    if (success > 0) {
        // remember the amount of played animations if latter is necessary play idle
        conductor.play_calls = character_get_play_calls(conductor.character);
    }

}

function conductor_has_hits(conductor) {
    return conductor.has_hits;
}

function conductor_has_misses(conductor) {
    return conductor.has_misses;
}

function conductor_disable(conductor, disable) {
    conductor.disable = !!disable;
    conductor.last_sustain_strum = conductor.last_penality_strum = null;
}


function conductor_play_idle(conductor) {
    if (!conductor.character) return;
    conductor.last_sustain_strum = conductor.last_penality_strum = null;

    if (character_play_idle(conductor.character) > 0)
        conductor.play_calls = character_get_play_calls(conductor.character);
}

function conductor_play_hey(conductor) {
    if (!conductor.character) return;
    conductor.last_sustain_strum = conductor.last_penality_strum = null;

    if (character_play_hey(conductor.character))
        conductor.play_calls = character_get_play_calls(conductor.character);
}

function conductor_get_character(conductor) {
    return conductor.character;
}


function conductor_internal_disposed_mapped_strum(mapped_strum) {
    for (let mapped_sing of arraylist_iterate4(mapped_strum.directions)) {
        mapped_sing.name = undefined;
        mapped_sing = undefined;
    }
    arraylist_destroy(mapped_strum.directions, 0);
    mapped_strum.strum = null;
    mapped_strum = undefined;
}

function conductor_intenal_add_mapping(conductor, strum, name, is_extra) {
    let mapped_strum = null;
    for (let map of arraylist_iterate4(conductor.mappings)) {
        if (map.strum == strum) {
            mapped_strum = map;
            break;
        }
    }

    if (!mapped_strum) {
        mapped_strum = {
            directions: arraylist_init2(4),
            is_disabled: 0,
            last_change_count: -1,
            strum: strum
        };
        arraylist_add(conductor.mappings, mapped_strum);
    }

    // check if already is added
    for (let item of arraylist_iterate4(mapped_strum.directions)) {
        if (item.name == name && item.is_extra == is_extra) {
            return;
        }
    }

    name = strdup(name);
    arraylist_add(mapped_strum.directions, { name, is_extra });
}

function conductor_intenal_remove_mapping(conductor, strum, name, is_extra) {
    for (let mapped_strum of arraylist_iterate4(conductor.mappings)) {
        if (mapped_strum.strum != strum) continue;

        let directions = arraylist_peek_array(mapped_strum.directions);
        let size = arraylist_size(mapped_strum.directions);

        for (let i = 0; i < size; i++) {
            if (directions[i].name == name && directions[i].is_extra == is_extra) {
                directions[i].name = undefined;
                directions[i] = undefined;
                arraylist_remove_at(mapped_strum.directions, i);

                return;
            }
        }
    }
}

function conductor_internal_execute_sing(character, mapping, is_sustain) {
    let done = 0;
    for (let direction of arraylist_iterate4(mapping)) {
        if (direction.is_extra) {
            if (character_play_extra(character, direction.name, is_sustain)) done++;
        } else {
            if (character_play_sing(character, direction.name, is_sustain)) done++;
        }
    }
    return done;
}

function conductor_internal_execute_miss(character, mapping, keep_in_hold) {
    let done = 0;
    for (let direction of arraylist_iterate4(mapping)) {
        if (direction.is_extra) {
            if (character_play_extra(character, direction.name, keep_in_hold)) done++;
        } else {
            if (character_play_miss(character, direction.name, keep_in_hold) == 1) done++;
        }
    }
    return done;
}

