"use strict";

const SCRIPT_NOTE_PENALITY = 0;
const SCRIPT_NOTE_MISS = 1;
const SCRIPT_NOTE_SHIT = 2;
const SCRIPT_NOTE_BAD = 3;
const SCRIPT_NOTE_GOOD = 4;
const SCRIPT_NOTE_SICK = 5;


async function weekscript_init(src, context, is_week) {
    let lua_sourcecode = await fs_readtext(src);
    if (lua_sourcecode == null) {
        let path = await fs_get_full_path_and_override(src);
        console.error(`weekscript_init() missing file '${path}'`);
        path = undefined;
        return null;
    }

    let lua_filename = fs_get_filename_without_extension(src);
    let tmp_full_path = fs_get_full_path(src);
    let working_folder = fs_get_parent_folder(tmp_full_path);

    tmp_full_path = undefined;

    // init luascript
    let luascript = await luascript_init(lua_sourcecode, lua_filename, working_folder, context, is_week);

    // Note: the "working_folder" is deallocated by luascript_destroy() function
    lua_sourcecode = undefined;
    lua_filename = undefined;

    if (!luascript) {
        console.error(`weekscript_init() call to luascript_init() failed lua=${src}`);
        return null;
    }

    await luascriptplatform.AddLuascript(luascript);
    return luascript;
}

/*async function weekscript_init2(lua_sourcecode, context, is_week) {
    let luascript = await luascript_init(lua_sourcecode, "test.lua", null, context, is_week);

    if (!luascript) {
        console.error(`weekscript_init() call to luascript_init() failed lua=test.lua`);
        return null;
    }

    return luascript;
}*/

function weekscript_destroy(script) {
    luascriptplatform.RemoveLuascript(script);
    luascriptplatform.RestoreWindowTitleIfChanged();
    luascript_destroy(script);
}


function weekscript_notify_weekinit(script, freeplay_index) {
    return luascript_notify_weekinit(script, freeplay_index);
}

function weekscript_notify_beforeready(script, from_retry) {
    return luascript_notify_beforeready(script, from_retry);
}

function weekscript_notify_ready(script) {
    return luascript_notify_ready(script);
}


function weekscript_notify_aftercountdown(script) {
    return luascript_notify_aftercountdown(script);
}

async function weekscript_notify_frame(script, elapsed) {
    const timestamp = Math.trunc(window.performance.now());
    await luascript_notify_timer_run(script, timestamp);
    await luascript_notify_frame(script, elapsed);
}

function weekscript_notify_timersong(script, song_timestamp) {
    return luascript_notify_timersong_run(script, song_timestamp);
}

function weekscript_notify_unknownnote(script, player_id, timestamp, direction, duration, data) {
    return luascript_notify_unknownnote(script, player_id, timestamp, direction, duration, data);
}

function weekscript_notify_roundend(script, loose) {
    return luascript_notify_roundend(script, loose);
}

function weekscript_notify_weekend(script, giveup) {
    return luascript_notify_weekend(script, giveup);
}

function weekscript_notify_gameoverloop(script) {
    return luascript_notify_gameoverloop(script);
}

function weekscript_notify_gameoverdecision(script, retry_or_giveup, changed_difficult) {
    return luascript_notify_gameoverdecision(script, retry_or_giveup, changed_difficult);
}

function weekscript_notify_gameoverended(script) {
    return luascript_notify_gameoverended(script);
}

function weekscript_notify_pause(script, pause_or_resume) {
    return luascript_notify_pause(script, pause_or_resume);
}

function weekscript_notify_weekleave(script) {
    return luascript_notify_weekleave(script);
}

function weekscript_notify_beforeresults(script) {
    return luascript_notify_beforeresults(script);
}

function weekscript_notify_afterresults(script, total_attempts, songs_count, reject_completed) {
    return luascript_notify_afterresults(script, total_attempts, songs_count, reject_completed);
}

function weekscript_notify_scriptchange(script) {
    return luascript_notify_scriptchange(script);
}

function weekscript_notify_pause_optionchoosen(script, option_index) {
    return luascript_notify_pause_optionchoosen(script, option_index);
}

function weekscript_notify_pause_menuvisible(script, shown_or_hidden) {
    return luascript_notify_pause_menuvisible(script, shown_or_hidden);
}

function weekscript_notify_beat(script, beat_count, since) {
    return luascript_notify_beat(script, beat_count, since);
}

function weekscript_notify_quarter(script, quarter_count, since) {
    return luascript_notify_quarter(script, quarter_count, since);
}

function weekscript_notify_buttons(script, player_id, buttons) {
    return luascript_notify_buttons(script, player_id, buttons);
}

function weekscript_notify_after_strum_scroll(script) {
    return luascript_notify_after_strum_scroll(script);
}

function weekscript_notify_dialogue_builtin_open(script, dialog_src) {
    return luascript_notify_dialogue_builtin_open(script, dialog_src);
}

function weekscript_notify_note(script, timestamp, id, duration, data, special, player_id, state) {
    return luascript_notify_note(script, timestamp, id, duration, data, special, player_id, state);
}


function weekscript_get_luascript(script) {
    return script;
}

