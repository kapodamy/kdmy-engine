#include "game/gameplay/weekscript.h"

#include <arch/timer.h>

#include "externals/luascript.h"
#include "externals/luascript_platform.h"
#include "fs.h"
#include "logger.h"
#include "malloc_utils.h"


WeekScript weekscript_init(const char* src, void* context, bool is_week) {
    char* lua_sourcecode = fs_readtext(src);
    if (lua_sourcecode == NULL) {
        char* path = fs_get_full_path_and_override(src);
        logger_error("weekscript_init() missing file '%s'", path);
        free_chk(path);
        return NULL;
    }

    char* lua_filename = fs_get_filename_without_extension(src);
    char* tmp_full_path = fs_get_full_path(src);
    char* working_folder = fs_get_parent_folder(tmp_full_path);

    free_chk(tmp_full_path);

    // init luascript
    Luascript luascript = luascript_init(lua_sourcecode, lua_filename, working_folder, context, is_week);

    // Note: the "working_folder" is deallocated by luascript_destroy() function
    free_chk(lua_sourcecode);
    free_chk(lua_filename);

    if (!luascript) {
        logger_error("weekscript_init() call to luascript_init() failed lua=%s", src);
        return NULL;
    }

    luascript_platform_add_luascript(luascript);
    return luascript;
}

/*WeekScript weekscript_init2(const char* lua_sourcecode, void* context, bool is_week) {
    Luascript luascript = luascript_init(lua_sourcecode, "test.lua", null, context, is_week);

    if (!luascript) {
        logger_error("weekscript_init() call to luascript_init() failed lua=test.lua");
        return NULL;
    }

    return luascript;
}*/

void weekscript_destroy(WeekScript* script_ptr) {
    if (!script_ptr || !*script_ptr) return;

    WeekScript script = *script_ptr;

    luascript_platform_remove_luascript(script);
    luascript_platform_restore_window_title_if_changed();

    luascript_destroy(script_ptr);
}


void weekscript_notify_weekinit(WeekScript script, int32_t freeplay_index) {
    luascript_notify_weekinit(script, freeplay_index);
}

void weekscript_notify_beforeready(WeekScript script, bool from_retry) {
    luascript_notify_beforeready(script, from_retry);
}

void weekscript_notify_ready(WeekScript script) {
    luascript_notify_ready(script);
}


void weekscript_notify_aftercountdown(WeekScript script) {
    luascript_notify_aftercountdown(script);
}

void weekscript_notify_frame(WeekScript script, float elapsed) {
    uint64_t timestamp = timer_ms_gettime64();
    luascript_notify_timer_run(script, timestamp);
    luascript_notify_frame(script, elapsed);
}

void weekscript_notify_timersong(WeekScript script, float64 song_timestamp) {
    luascript_notify_timersong_run(script, song_timestamp);
}

void weekscript_notify_unknownnote(WeekScript script, int32_t player_id, float64 timestamp, int32_t direction, float64 duration, float64 data) {
    luascript_notify_unknownnote(script, player_id, timestamp, direction, duration, data);
}

void weekscript_notify_roundend(WeekScript script, bool loose) {
    luascript_notify_roundend(script, loose);
}

void weekscript_notify_weekend(WeekScript script, bool giveup) {
    luascript_notify_weekend(script, giveup);
}

void weekscript_notify_gameoverloop(WeekScript script) {
    luascript_notify_gameoverloop(script);
}

void weekscript_notify_gameoverdecision(WeekScript script, bool retry_or_giveup, const char* changed_difficult) {
    luascript_notify_gameoverdecision(script, retry_or_giveup, changed_difficult);
}

void weekscript_notify_gameoverended(WeekScript script) {
    luascript_notify_gameoverended(script);
}

void weekscript_notify_pause(WeekScript script, bool pause_or_resume) {
    luascript_notify_pause(script, pause_or_resume);
}

void weekscript_notify_weekleave(WeekScript script) {
    luascript_notify_weekleave(script);
}

void weekscript_notify_beforeresults(WeekScript script) {
    luascript_notify_beforeresults(script);
}

void weekscript_notify_afterresults(WeekScript script, int32_t total_attempts, int32_t songs_count, bool reject_completed) {
    luascript_notify_afterresults(script, total_attempts, songs_count, reject_completed);
}

void weekscript_notify_scriptchange(WeekScript script) {
    luascript_notify_scriptchange(script);
}

void weekscript_notify_pause_optionchoosen(WeekScript script, int32_t option_index) {
    luascript_notify_pause_optionchoosen(script, option_index);
}

void weekscript_notify_pause_menuvisible(WeekScript script, bool shown_or_hidden) {
    luascript_notify_pause_menuvisible(script, shown_or_hidden);
}

void weekscript_notify_beat(WeekScript script, int32_t beat_count, float since) {
    luascript_notify_beat(script, beat_count, since);
}

void weekscript_notify_quarter(WeekScript script, int32_t quarter_count, float since) {
    luascript_notify_quarter(script, quarter_count, since);
}

void weekscript_notify_buttons(WeekScript script, int32_t player_id, GamepadButtons buttons) {
    luascript_notify_buttons(script, player_id, buttons);
}

void weekscript_notify_after_strum_scroll(WeekScript script) {
    luascript_notify_after_strum_scroll(script);
}

void weekscript_notify_dialogue_builtin_open(WeekScript script, const char* dialog_src) {
    luascript_notify_dialogue_builtin_open(script, dialog_src);
}

void weekscript_notify_note(WeekScript script, float64 timestamp, int32_t id, float64 duration, float64 data, bool special, int32_t player_id, Ranking state) {
    luascript_notify_note(script, timestamp, id, duration, data, special, player_id, state);
}


Luascript weekscript_get_luascript(WeekScript script) {
    return script;
}
