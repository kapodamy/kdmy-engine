#include "commons.h"
#include "conductor.h"

static Conductor_t conductor = {};
static Character_t character = {};

Conductor conductor_init() {
    print_stub("conductor_init", "");
    return &conductor;
}
void conductor_destroy(Conductor* conductor) {
    print_stub("conductor_destroy", "conductor=%p", conductor);
}
void conductor_poll_reset(Conductor conductor) {
    print_stub("conductor_poll_reset", "conductor=%p", conductor);
}
void conductor_set_character(Conductor conductor, Character character) {
    print_stub("conductor_set_character", "conductor=%p character=%p", conductor, character);
}
void conductor_use_strum_line(Conductor conductor, Strum strum) {
    print_stub("conductor_use_strum_line", "conductor=%p strum=%p", conductor, strum);
}
void conductor_use_strums(Conductor conductor, Strums strums) {
    print_stub("conductor_use_strums", "conductor=%p strums=%p", conductor, strums);
}
bool conductor_disable_strum_line(Conductor conductor, Strum strum, bool should_disable) {
    print_stub("conductor_disable_strum_line", "conductor=%p strum=%p should_disable=(bool)%i", conductor, strum, should_disable);
    return 0;
}
bool conductor_remove_strum(Conductor conductor, Strum strum) {
    print_stub("conductor_remove_strum", "conductor=%p strum=%p", conductor, strum);
    return 0;
}
void conductor_clear_mapping(Conductor conductor) {
    print_stub("conductor_clear_mapping", "conductor=%p", conductor);
}
void conductor_map_strum_to_player_sing_add(Conductor conductor, Strum strum, const char* sing_direction_name) {
    print_stub("conductor_map_strum_to_player_sing_add", "conductor=%p strum=%p sing_direction_name=%s", conductor, strum, sing_direction_name);
}
void conductor_map_strum_to_player_extra_add(Conductor conductor, Strum strum, const char* extra_animation_name) {
    print_stub("conductor_map_strum_to_player_extra_add", "conductor=%p strum=%p extra_animation_name=%s", conductor, strum, extra_animation_name);
}
void conductor_map_strum_to_player_sing_remove(Conductor conductor, Strum strum, const char* sing_direction_name) {
    print_stub("conductor_map_strum_to_player_sing_remove", "conductor=%p strum=%p sing_direction_name=%s", conductor, strum, sing_direction_name);
}
void conductor_map_strum_to_player_extra_remove(Conductor conductor, Strum strum, const char* extra_animation_name) {
    print_stub("conductor_map_strum_to_player_extra_remove", "conductor=%p strum=%p extra_animation_name=%s", conductor, strum, extra_animation_name);
}
int32_t conductor_map_automatically(Conductor conductor, bool should_map_extras) {
    print_stub("conductor_map_automatically", "conductor=%p should_map_extras=(bool)%i", conductor, should_map_extras);
    return 0;
}
void conductor_poll(Conductor conductor) {
    print_stub("conductor_poll", "conductor=%p", conductor);
}
void conductor_disable(Conductor conductor, bool disable) {
    print_stub("conductor_disable", "conductor=%p disable=(bool)%i", conductor, disable);
}
void conductor_play_idle(Conductor conductor) {
    print_stub("conductor_play_idle", "conductor=%p", conductor);
}
void conductor_play_hey(Conductor conductor) {
    print_stub("conductor_play_hey", "conductor=%p", conductor);
}
Character conductor_get_character(Conductor conductor) {
    print_stub("conductor_get_character", "conductor=%p", conductor);
    return &character;
}
