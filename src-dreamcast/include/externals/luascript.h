#ifndef _luascript_h
#define _luascript_h

#include <stdbool.h>
#include <stdint.h>

#include "game/modding_types.h"


//
//  forward reference
//

typedef struct Menu_s* Menu;


typedef struct Luascript_s* Luascript;

Luascript luascript_init(const char* lua_sourcecode, const char* filename, char* working_folder, void* context, bool is_week);
void luascript_destroy(Luascript* luascript);

void luascript_drop_shared(void* obj_ptr);
bool luascript_eval(Luascript luascript, const char* eval_string);
const char* luascript_get_version();

void luascript_notify_weekinit(Luascript luascript, int32_t freeplay_index);
void luascript_notify_beforeready(Luascript luascript, bool from_retry);
void luascript_notify_ready(Luascript luascript);
void luascript_notify_aftercountdown(Luascript luascript);
void luascript_notify_frame(Luascript luascript, float elapsed);
void luascript_notify_roundend(Luascript luascript, bool loose);
void luascript_notify_weekend(Luascript luascript, bool giveup);
void luascript_notify_gameoverloop(Luascript luascript);
void luascript_notify_gameoverdecision(Luascript luascript, bool retry_or_giveup, const char* changed_difficult);
void luascript_notify_gameoverended(Luascript luascript);
void luascript_notify_pause(Luascript luascript, bool pause_or_resume);
void luascript_notify_weekleave(Luascript luascript);
void luascript_notify_beforeresults(Luascript luascript);
void luascript_notify_afterresults(Luascript luascript, int32_t total_attempts, int32_t songs_count, bool reject_completed);
void luascript_notify_scriptchange(Luascript luascript);
void luascript_notify_pause_optionchoosen(Luascript luascript, int32_t option_index);
void luascript_notify_pause_menuvisible(Luascript luascript, bool shown_or_hidden);
void luascript_notify_note(Luascript luascript, float64 timestamp, int32_t id, float64 duration, float64 data, bool special, int32_t player_id, uint32_t state);
void luascript_notify_buttons(Luascript luascript, int32_t player_id, uint32_t buttons);
void luascript_notify_unknownnote(Luascript luascript, int32_t player_id, float64 timestamp, int32_t id, float64 duration, float64 data);
void luascript_notify_beat(Luascript luascript, int32_t current_beat, float since);
void luascript_notify_quarter(Luascript luascript, int32_t current_quarter, float since);
void luascript_notify_timer_run(Luascript luascript, float64 kos_timestamp);
void luascript_notify_timersong_run(Luascript luascript, float64 song_timestamp);
void luascript_notify_dialogue_builtin_open(Luascript luascript, const char* dialog_src);
void luascript_notify_dialogue_line_starts(Luascript luascript, int line_index, const char* state_name, const char* if_line_label);
void luascript_notify_dialogue_line_ends(Luascript luascript, int line_index, const char* state_name, const char* if_line_label);
void luascript_notify_after_strum_scroll(Luascript luascript);
void luascript_call_function(Luascript luascript, const char* function_name);
void luascript_notify_modding_menu_option_selected(Luascript luascript, Menu menu, int index, const char* name);
bool luascript_notify_modding_menu_option_choosen(Luascript luascript, Menu menu, int index, const char* name);
bool luascript_notify_modding_back(Luascript luascript);
void* luascript_notify_modding_exit(Luascript luascript, ModdingValueType* return_type);
void luascript_notify_modding_init(Luascript luascript, const void* arg_value, const ModdingValueType arg_type);
void luascript_notify_modding_event(Luascript luascript, const char* event_name);
void luascript_notify_modding_handle_custom_option(Luascript luascript, const char* option_name);
void luascript_notify_modding_window_focus(Luascript luascript, bool focused);
void luascript_notify_modding_window_minimized(Luascript luascript, bool minimized);
void luascript_notify_window_size_changed(Luascript luascript, int32_t screen_width, int32_t screen_height);
void luascript_notify_input_keyboard(Luascript luascript, int32_t key, int32_t scancode, bool is_pressed, int32_t mods);
void luascript_notify_input_mouse_position(Luascript luascript, float64 x, float64 y);
void luascript_notify_input_mouse_enter(Luascript luascript, bool entering);
void luascript_notify_input_mouse_button(Luascript luascript, int32_t button, bool is_pressed, int32_t mods);
void luascript_notify_input_mouse_scroll(Luascript luascript, float64 x, float64 y);

#endif
