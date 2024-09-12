#include "game/funkin/conductor.h"

#include "arraylist.h"
#include "drawable.h"
#include "externals/luascript.h"
#include "game/funkin/character.h"
#include "game/funkin/missnotefx.h"
#include "game/funkin/strum.h"
#include "game/funkin/strums.h"
#include "logger.h"
#include "malloc_utils.h"
#include "math2d.h"
#include "pvrcontext.h"
#include "stringutils.h"


typedef struct {
    char* name;
    bool is_extra;
} MapDirection;

typedef struct {
    Strum strum;
    ArrayList /*<MapDirection>*/ directions;
    bool is_disabled;
    int32_t last_change_count;
} Mapping;

struct Conductor_s {
    Character character;
    ArrayList /*<Mapping>*/ mappings;
    bool disable;
    Strum last_penality_strum;
    Strum last_sustain_strum;
    MissNoteFX missnotefx;
    bool has_misses;
    bool has_hits;
    int32_t play_calls;
};


static void conductor_internal_disposed_mapped_strum(Mapping* mapped_strum);
static void conductor_intenal_add_mapping(Conductor conductor, Strum strum, const char* name, bool is_extra);
static void conductor_intenal_remove_mapping(Conductor conductor, Strum strum, const char* name, bool is_extra);
static int32_t conductor_internal_execute_sing(Character character, ArrayList mapping, bool is_sustain);
static int32_t conductor_internal_execute_miss(Character character, ArrayList mapping, bool keep_in_hold);


Conductor conductor_init() {
    Conductor conductor = malloc_chk(sizeof(struct Conductor_s));
    malloc_assert(conductor, Conductor);

    *conductor = (struct Conductor_s){
        .character = NULL,
        .disable = false,
        .mappings = arraylist_init2(sizeof(Mapping), 4),
        .last_penality_strum = NULL,
        .last_sustain_strum = NULL,
        .missnotefx = NULL,
        .has_misses = false,
        .has_hits = false,
        .play_calls = 0
    };

    return conductor;
}

void conductor_destroy(Conductor* conductor_ptr) {
    if (!conductor_ptr || !*conductor_ptr) return;

    Conductor conductor = *conductor_ptr;

    luascript_drop_shared(conductor);

    conductor_clear_mapping(conductor);
    arraylist_destroy(&conductor->mappings);

    free_chk(conductor);
    *conductor_ptr = NULL;
}


void conductor_poll_reset(Conductor conductor) {
    foreach (Mapping*, map, ARRAYLIST_ITERATOR, conductor->mappings) {
        map->last_change_count = -1;
    }
    conductor->last_penality_strum = NULL;
    conductor->last_sustain_strum = NULL;
    conductor->has_misses = false;
    conductor->has_hits = false;
}

void conductor_set_character(Conductor conductor, Character character) {
    if (conductor->character != character) conductor->last_sustain_strum = conductor->last_penality_strum = NULL;
    conductor->character = character;

    if (character != NULL)
        conductor->play_calls = character_get_play_calls(character) - 1;
}

void conductor_use_strum_line(Conductor conductor, Strum strum) {
    if (!strum) return;

    // check if is already in use
    foreach (Mapping*, map, ARRAYLIST_ITERATOR, conductor->mappings) {
        if (map->strum == strum) {
            map->is_disabled = false;
            return;
        }
    }

    arraylist_add(
        conductor->mappings,
        &(Mapping){
            .strum = strum,
            .last_change_count = -1,
            .directions = arraylist_init2(sizeof(MapDirection), 4),
            .is_disabled = false,
        }
    );
}

void conductor_use_strums(Conductor conductor, Strums strums) {
    if (!strums) return;

    int32_t size = strums_get_lines_count(strums);
    for (int32_t i = 0; i < size; i++) {
        Strum strum = strums_get_strum_line(strums, i);
        conductor_use_strum_line(conductor, strum);
    }
}

bool conductor_disable_strum_line(Conductor conductor, Strum strum, bool should_disable) {
    if (!strum) return false;
    foreach (Mapping*, map, ARRAYLIST_ITERATOR, conductor->mappings) {
        if (map->strum == strum) {
            map->is_disabled = !!should_disable;
            return true;
        }
    }
    return false;
}

bool conductor_remove_strum(Conductor conductor, Strum strum) {
    Mapping* array = arraylist_peek_array(conductor->mappings);
    int32_t size = arraylist_size(conductor->mappings);
    for (int32_t i = 0; i < size; i++) {
        if (array[i].strum == strum) {
            conductor_internal_disposed_mapped_strum(&array[i]);
            arraylist_remove_at(conductor->mappings, i);
            return true;
        }
    }
    return false;
}

void conductor_clear_mapping(Conductor conductor) {
    foreach (Mapping*, map, ARRAYLIST_ITERATOR, conductor->mappings) {
        conductor_internal_disposed_mapped_strum(map);
    }
    arraylist_clear(conductor->mappings);
    conductor->last_sustain_strum = conductor->last_penality_strum = NULL;
}


void conductor_map_strum_to_player_sing_add(Conductor conductor, Strum strum, const char* sing_direction_name) {
    conductor_intenal_add_mapping(conductor, strum, sing_direction_name, false);
}

void conductor_map_strum_to_player_extra_add(Conductor conductor, Strum strum, const char* extra_animation_name) {
    conductor_intenal_add_mapping(conductor, strum, extra_animation_name, true);
}

void conductor_map_strum_to_player_sing_remove(Conductor conductor, Strum strum, const char* sing_direction_name) {
    conductor_intenal_remove_mapping(conductor, strum, sing_direction_name, false);
}

void conductor_map_strum_to_player_extra_remove(Conductor conductor, Strum strum, const char* extra_animation_name) {
    conductor_intenal_remove_mapping(conductor, strum, extra_animation_name, true);
}


int32_t conductor_map_automatically(Conductor conductor, bool should_map_extras) {
    if (!conductor->character) {
        logger_error("conductor_map_automatically() failed, no character set");
        return 0;
    }

    Mapping* mappings = arraylist_peek_array(conductor->mappings);
    int32_t size = arraylist_size(conductor->mappings);

    if (size < 1) {
        logger_error("conductor_map_automatically() failed, no strums are set");
        return 0;
    }

    int32_t count = 0;
    for (int32_t i = 0; i < size; i++) {
        const char* strum_name = strum_get_name(mappings[i].strum);

        if (character_has_direction(conductor->character, strum_name, false)) {
            // strum_name --> sing_direction
            conductor_map_strum_to_player_sing_add(conductor, mappings[i].strum, strum_name);
            count++;
            continue;
        } else if (should_map_extras && character_has_direction(conductor->character, strum_name, true)) {
            // strum_mame --> extra_animation
            conductor_map_strum_to_player_extra_add(conductor, mappings[i].strum, strum_name);
            count++;
            break;
        }
    }

    return count;
}

void conductor_set_missnotefx(Conductor conductor, MissNoteFX missnotefx) {
    conductor->missnotefx = missnotefx;
}

void conductor_poll(Conductor conductor) {
    if (conductor->disable || !conductor->character) return;

    Mapping* array = arraylist_peek_array(conductor->mappings);
    int32_t size = arraylist_size(conductor->mappings);
    int32_t success = 0;

    conductor->has_hits = false;
    conductor->has_misses = false;

    for (int32_t i = 0; i < size; i++) {
        if (array[i].is_disabled) continue;

        int32_t press_changes = strum_get_press_state_changes(array[i].strum);
        StrumPressState press_state = strum_get_press_state(array[i].strum);
        bool press_state_use_alt_anim = strum_get_press_state_use_alt_anim(array[i].strum);

        // check if the strums was updated
        if (press_changes == array[i].last_change_count) {
            // if was not updated, check if still holding a note (sustain or not)
            switch (press_state) {
                case StrumPressState_HIT:
                case StrumPressState_HIT_SUSTAIN:
                    conductor->has_hits = true;
                    break;
                default:
                    break;
            }
            continue;
        }
        array[i].last_change_count = press_changes;

        if (press_state_use_alt_anim) character_use_alternate_sing_animations(conductor->character, true);

        switch (press_state) {
            case StrumPressState_MISS:
                conductor->has_misses = true;
                // fall through
            case StrumPressState_NONE:
                if (conductor->last_penality_strum == array[i].strum) {
                    // stop the penality animation in the next beat
                    character_schedule_idle(conductor->character);
                    conductor->play_calls = character_get_play_calls(conductor->character);
                    conductor->last_penality_strum = NULL;
                } else if (conductor->last_sustain_strum == array[i].strum) {
                    conductor->last_sustain_strum = NULL;
                    if (!character_is_idle_active(conductor->character)) {
                        //
                        // only play the idle animation if the last action
                        // was done by the conductor itself
                        //
                        if (conductor->play_calls == character_get_play_calls(conductor->character)) {
                            character_play_idle(conductor->character);
                            conductor->play_calls = character_get_play_calls(conductor->character);
                        }
                    }
                }
                break;
            case StrumPressState_HIT:
                conductor->last_sustain_strum = NULL;
                conductor->last_penality_strum = NULL;
                conductor->has_misses = false;
                conductor->has_hits = true;
                success += conductor_internal_execute_sing(conductor->character, array[i].directions, false);
                break;
            case StrumPressState_HIT_SUSTAIN:
                conductor->last_sustain_strum = array[i].strum;
                conductor->last_penality_strum = NULL;
                conductor->has_misses = false;
                conductor->has_hits = true;
                success += conductor_internal_execute_sing(conductor->character, array[i].directions, true);
                break;
            case StrumPressState_PENALTY_NOTE:
                // button press on empty strum
                conductor->has_misses = true;
                conductor->last_sustain_strum = NULL;
                conductor->last_penality_strum = array[i].strum;
                success += conductor_internal_execute_miss(conductor->character, array[i].directions, true);
                if (conductor->missnotefx) missnotefx_play_effect(conductor->missnotefx);
                break;
            case StrumPressState_PENALTY_HIT:
                // wrong note button
                conductor->last_sustain_strum = NULL;
                conductor->last_penality_strum = array[i].strum;
                success += conductor_internal_execute_miss(conductor->character, array[i].directions, false);
                if (conductor->missnotefx) missnotefx_play_effect(conductor->missnotefx);
                break;
            default:
                break;
        }

        if (press_state_use_alt_anim) character_use_alternate_sing_animations(conductor->character, false);
    }

    if (success > 0) {
        // remember the amount of played animations if latter is necessary play idle
        conductor->play_calls = character_get_play_calls(conductor->character);
    }
}

bool conductor_has_hits(Conductor conductor) {
    return conductor->has_hits;
}

bool conductor_has_misses(Conductor conductor) {
    return conductor->has_misses;
}

void conductor_disable(Conductor conductor, bool disable) {
    conductor->disable = disable;
    conductor->last_sustain_strum = conductor->last_penality_strum = NULL;
}


void conductor_play_idle(Conductor conductor) {
    if (!conductor->character) return;
    conductor->last_sustain_strum = conductor->last_penality_strum = NULL;

    if (character_play_idle(conductor->character) > 0)
        conductor->play_calls = character_get_play_calls(conductor->character);
}

void conductor_play_hey(Conductor conductor) {
    if (!conductor->character) return;
    conductor->last_sustain_strum = conductor->last_penality_strum = NULL;

    if (character_play_hey(conductor->character))
        conductor->play_calls = character_get_play_calls(conductor->character);
}

Character conductor_get_character(Conductor conductor) {
    return conductor->character;
}


static void conductor_internal_disposed_mapped_strum(Mapping* mapped_strum) {
    foreach (MapDirection*, mapped_sing, ARRAYLIST_ITERATOR, mapped_strum->directions) {
        free_chk(mapped_sing->name);
    }
    arraylist_destroy(&mapped_strum->directions);
    mapped_strum->strum = NULL;
}

static void conductor_intenal_add_mapping(Conductor conductor, Strum strum, const char* name, bool is_extra) {
    Mapping* mapped_strum = NULL;
    foreach (Mapping*, map, ARRAYLIST_ITERATOR, conductor->mappings) {
        if (map->strum == strum) {
            mapped_strum = map;
            break;
        }
    }

    if (!mapped_strum) {
        mapped_strum = arraylist_add(
            conductor->mappings,
            &(Mapping){
                .directions = arraylist_init2(sizeof(MapDirection), 4),
                .is_disabled = false,
                .last_change_count = -1,
                .strum = strum,
            }
        );
    }

    // check if already is added
    foreach (MapDirection*, item, ARRAYLIST_ITERATOR, mapped_strum->directions) {
        if (string_equals(item->name, name) && item->is_extra == is_extra) {
            return;
        }
    }

    name = string_duplicate(name);
    arraylist_add(mapped_strum->directions, &(MapDirection){.name = (char*)name, .is_extra = is_extra});
}

static void conductor_intenal_remove_mapping(Conductor conductor, Strum strum, const char* name, bool is_extra) {
    foreach (Mapping*, mapped_strum, ARRAYLIST_ITERATOR, conductor->mappings) {
        if (mapped_strum->strum != strum) continue;

        MapDirection* directions = arraylist_peek_array(mapped_strum->directions);
        int32_t size = arraylist_size(mapped_strum->directions);

        for (int32_t i = 0; i < size; i++) {
            if (string_equals(directions[i].name, name) && directions[i].is_extra == is_extra) {
                free_chk(directions[i].name);
                arraylist_remove_at(mapped_strum->directions, i);
                return;
            }
        }
    }
}

static int32_t conductor_internal_execute_sing(Character character, ArrayList mapping, bool is_sustain) {
    int32_t done = 0;
    foreach (MapDirection*, direction, ARRAYLIST_ITERATOR, mapping) {
        if (direction->is_extra) {
            if (character_play_extra(character, direction->name, is_sustain)) done++;
        } else {
            if (character_play_sing(character, direction->name, is_sustain)) done++;
        }
    }
    return done;
}

static int32_t conductor_internal_execute_miss(Character character, ArrayList mapping, bool keep_in_hold) {
    int32_t done = 0;
    foreach (MapDirection*, direction, ARRAYLIST_ITERATOR, mapping) {
        if (direction->is_extra) {
            if (character_play_extra(character, direction->name, keep_in_hold)) done++;
        } else {
            if (character_play_miss(character, direction->name, keep_in_hold)) done++;
        }
    }
    return done;
}
