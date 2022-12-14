#ifndef __conductor_h
#define __conductor_h

#include "character.h"
#include "missnotefx.h"
#include "strums.h"

typedef struct {
    int dummy;
} Conductor_t;
typedef Conductor_t* Conductor;


Conductor conductor_init();
void conductor_destroy(Conductor* conductor);
void conductor_poll_reset(Conductor conductor);
void conductor_set_character(Conductor conductor, Character character);
void conductor_use_strum_line(Conductor conductor, Strum strum);
void conductor_use_strums(Conductor conductor, Strums strums);
bool conductor_disable_strum_line(Conductor conductor, Strum strum, bool should_disable);
bool conductor_remove_strum(Conductor conductor, Strum strum);
void conductor_clear_mapping(Conductor conductor);
void conductor_map_strum_to_player_sing_add(Conductor conductor, Strum strum, const char* sing_direction_name);
void conductor_map_strum_to_player_extra_add(Conductor conductor, Strum strum, const char* extra_animation_name);
void conductor_map_strum_to_player_sing_remove(Conductor conductor, Strum strum, const char* sing_direction_name);
void conductor_map_strum_to_player_extra_remove(Conductor conductor, Strum strum, const char* extra_animation_name);
int32_t conductor_map_automatically(Conductor conductor, bool should_map_extras);
void conductor_poll(Conductor conductor);
void conductor_disable(Conductor conductor, bool disable);
void conductor_play_idle(Conductor conductor);
void conductor_play_hey(Conductor conductor);
Character conductor_get_character(Conductor conductor);

#endif

