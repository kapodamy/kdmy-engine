#ifndef __luascript_h
#define __luascript_h

#include <stdbool.h>
#include <stdint.h>

#include "linkedlist.h"
#include "modding.h"

typedef struct LuascriptObject_t {
    int lua_ref;
    void* obj_ptr;
    bool was_allocated_by_lua;
} LuascriptObject;

typedef struct Luascript_t {
    void* L;
    void* context;
    bool is_week;
    LuascriptObject* shared_array;
    size_t shared_size;
}* Luascript;


Luascript luascript_init(const char* lua_sourcecode, const char* filename, void* context, bool is_week);
void luascript_destroy(Luascript* luascript);

void luascript_drop_shared(void* obj_ptr);

void luascript_notify_weekinit(Luascript luascript, int32_t freeplay_index);
void luascript_notify_beforeready(Luascript luascript, bool from_retry);
void luascript_notify_ready(Luascript luascript);
void luascript_notify_aftercountdown(Luascript luascript);
void luascript_notify_frame(Luascript luascript, float elapsed);
void luascript_notify_roundend(Luascript luascript, bool loose);
void luascript_notify_weekend(Luascript luascript, bool giveup);
void luascript_notify_diedecision(Luascript luascript, bool retry_or_giveup, const char* difficult_changed);
void luascript_notify_pause(Luascript luascript, bool pause_or_resume);
void luascript_notify_weekleave(Luascript luascript);
void luascript_notify_afterresults(Luascript luascript, int32_t total_attempts, int32_t songs_count, bool reject_completed);
void luascript_notify_scriptchange(Luascript luascript);
void luascript_notify_pause_optionchoosen(Luascript luascript, int32_t option_index);
void luascript_notify_pause_menuvisible(Luascript luascript, bool shown_or_hidden);
void luascript_notify_note(Luascript luascript, double timestamp, int32_t id, double duration, double data, bool special, int32_t player_id, uint32_t state);
void luascript_notify_buttons(Luascript luascript, int32_t player_id, uint32_t buttons);
void luascript_notify_unknownnote(Luascript luascript, int32_t player_id, double timestamp, int32_t id, double duration, double data);
void luascript_notify_beat(Luascript luascript, int32_t current_beat, float since);
void luascript_notify_quarter(Luascript luascript, int32_t current_quarter, float since);
void luascript_notify_timer_run(Luascript luascript, double kos_timestamp);
void luascript_notify_timersong_run(Luascript luascript, double song_timestamp);
void luascript_notify_dialogue_builtin_open(Luascript luascript, const char* dialog_src);
void luascript_notify_dialogue_line_starts(Luascript luascript, int line_index, const char* state_name, const char* if_line_label);
void luascript_notify_dialogue_line_ends(Luascript luascript, int line_index, const char* state_name, const char* if_line_label);
void luascript_notify_after_strum_scroll(Luascript luascript);
void luascript_call_function(Luascript luascript, const char* function_name);
void luascript_notify_modding_menu_option_selected(Luascript luascript, void* menu, int index, const char* name);
bool luascript_notify_modding_menu_option_choosen(Luascript luascript, void* menu, int index, const char* name);
bool luascript_notify_modding_back(Luascript luascript);
void* luascript_notify_modding_exit(Luascript luascript, ModdingValueType* return_type);
void luascript_notify_modding_init(Luascript luascript, ModdingValueType arg_type, void* arg_value);
void luascript_notify_modding_event(Luascript luascript, const char* event_name);
void luascript_notify_modding_handle_custom_option(Luascript luascript, const char* option_name);
void luascript_notify_modding_window_focus(Luascript luascript, bool focused);
void luascript_notify_modding_window_minimized(Luascript luascript, bool minimized);
void luascript_notify_window_size_changed(Luascript luascript, int32_t screen_width, int32_t screen_height);
void luascript_notify_input_keyboard(Luascript luascript, int32_t key, int32_t scancode, bool is_pressed, int32_t mods);
void luascript_notify_input_mouse_position(Luascript luascript, double x, double y);
void luascript_notify_input_mouse_enter(Luascript luascript, bool entering);
void luascript_notify_input_mouse_button(Luascript luascript, int32_t button, bool is_pressed, int32_t mods);
void luascript_notify_input_mouse_scroll(Luascript luascript, double x, double y);
#endif
