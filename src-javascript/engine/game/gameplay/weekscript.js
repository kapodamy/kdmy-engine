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
        return null;
    }

    // javascript only
    if (!ModuleLuaScript.isRuntimeInitialized) {
        let timeout = timer_ms_gettime64() + 10000;// 10 seconds
        console.log("weekscript_init() ModuleLuaScript not is ready, waiting");

        while (!ModuleLuaScript.isRuntimeInitialized && timer_ms_gettime64() < timeout) {
            await thd_sleep(100);
        }
        if (!ModuleLuaScript.isRuntimeInitialized) {
            console.error("weekscript_init() failed, timeout waiting for ModuleLuaScript");
            return null;
        }
    }

    if (!ModuleLuaScript.hasGlobalsSet) {
        ModuleLuaScript._luascript_set_engine_globals_JS(
            ModuleLuaScript.kdmyEngine_stringToPtr(ENGINE_NAME),
            ModuleLuaScript.kdmyEngine_stringToPtr(ENGINE_VERSION)
        );
        ModuleLuaScript.hasGlobalsSet = 1;
    }

    // javascript only
    let lua_sourcecode_ptr = ModuleLuaScript.kdmyEngine_stringToPtr(lua_sourcecode);
    let lua_filename_ptr = ModuleLuaScript.kdmyEngine_stringToPtr(fs_get_filename_without_extension(src));
    let roundcontext_ptr = ModuleLuaScript.kdmyEngine_obtain(context);

    // init luascript
    let luascript = ModuleLuaScript._luascript_init(
        lua_sourcecode_ptr, lua_filename_ptr, roundcontext_ptr, is_week ? 1 : 0
    );

    // javascript only
    let promise = ModuleLuaScript.kdmyEngine_yieldAsync();
    if (promise) luascript = await promise;
    ModuleLuaScript.kdmyEngine_deallocate(lua_sourcecode_ptr);
    ModuleLuaScript.kdmyEngine_deallocate(lua_filename_ptr);

    if (!luascript) {
        console.error(`weekscript_init() call to luascript_init() failed lua=${src}`);
        return null;
    }

    await luascriptplatform.AddLuascript(luascript);
    return luascript;
}

/*async function weekscript_init2(lua_sourcecode, context, is_week) {
    // javascript only
    if (!ModuleLuaScript.isRuntimeInitialized) {
        let timeout = timer_ms_gettime64() + 10000;// 10 seconds
        console.log("weekscript_init() ModuleLuaScript not is ready, waiting");

        while (!ModuleLuaScript.isRuntimeInitialized && timer_ms_gettime64() < timeout) {
            await thd_sleep(100);
        }
        if (!ModuleLuaScript.isRuntimeInitialized) {
            console.error("weekscript_init() failed, timeout waiting for ModuleLuaScript");
            return null;
        }
    }

    if (!ModuleLuaScript.hasGlobalsSet) {
        ModuleLuaScript._luascript_set_engine_globals_JS(
            ModuleLuaScript.kdmyEngine_stringToPtr(ENGINE_NAME),
            ModuleLuaScript.kdmyEngine_stringToPtr(ENGINE_VERSION)
        );
    }

    if (ModuleLuaScript.hasInstanceActive) {
        throw new Error("ModuleLuaScript is already in use or was not disposed correctly");
    }

    // javascript only
    let lua_sourcecode_ptr = ModuleLuaScript.kdmyEngine_stringToPtr(lua_sourcecode);
    let lua_filename_ptr = ModuleLuaScript.kdmyEngine_stringToPtr("test.lua");
    let roundcontext_ptr = ModuleLuaScript.kdmyEngine_obtain(context);

    let luascript = ModuleLuaScript._luascript_init(
        lua_sourcecode_ptr, lua_filename_ptr, roundcontext_ptr, is_week ? 1 : 0
    );

    if (!luascript) {
        console.error(`weekscript_init() call to luascript_init() failed lua=test.lua`);
        return null;
    }

    // javascript only
    ModuleLuaScript.kdmyEngine_deallocate(lua_sourcecode_ptr);
    ModuleLuaScript.kdmyEngine_deallocate(lua_filename_ptr);

    ModuleLuaScript.hasInstanceActive = true;
    return luascript;
}*/

function weekscript_destroy(script) {
    luascriptplatform.RemoveLuascript(script);
    luascriptplatform.RestoreWindowTitleIfChanged();
    ModuleLuaScript._luascript_destroy_JS(script);
}


function weekscript_notify_weekinit(script, freeplay_index) {
    return _weekscript_call(ModuleLuaScript._luascript_notify_weekinit, script, freeplay_index);
}

function weekscript_notify_beforeready(script, from_retry) {
    return _weekscript_call(ModuleLuaScript._luascript_notify_beforeready, script, from_retry ? 1 : 0);
}

function weekscript_notify_ready(script) {
    return _weekscript_call(ModuleLuaScript._luascript_notify_ready, script);
}


function weekscript_notify_aftercountdown(script) {
    return _weekscript_call(ModuleLuaScript._luascript_notify_aftercountdown, script);
}

async function weekscript_notify_frame(script, elapsed) {
    const timestamp = Math.trunc(window.performance.now());
    await _weekscript_call(ModuleLuaScript._luascript_notify_timer_run, script, timestamp);
    await _weekscript_call(ModuleLuaScript._luascript_notify_frame, script, elapsed);
}

async function weekscript_notify_timersong(script, song_timestamp) {
    await _weekscript_call(ModuleLuaScript._luascript_notify_timersong_run, script, song_timestamp);
}

function weekscript_notify_note_hit(script, strum, strum_note_index, playerstats) {
    /** @type {StrumNote} */
    let strum_note = strum.chart_notes[strum_note_index];
    let timestamp = strum_note.timestamp;
    let duration = strum_note.duration;
    let note_id = strum.chart_notes_id_map[strum_note.id];
    let special = strum.attribute_notes[strum_note.id].is_special;
    let data = strum_note.custom_data;
    let player_id = strum.player_id;

    let state;
    switch (playerstats.last_ranking) {
        case PLAYERSTATS_RANK_SICK:
            state = SCRIPT_NOTE_SICK;
            break;
        case PLAYERSTATS_RANK_GOOD:
            state = SCRIPT_NOTE_GOOD;
            break;
        case PLAYERSTATS_RANK_BAD:
            state = SCRIPT_NOTE_BAD;
            break;
        case PLAYERSTATS_RANK_SHIT:
            state = SCRIPT_NOTE_SHIT;
            break;
        default:
            return;
    }

    _weekscript_notify_note(script, timestamp, note_id, duration, data, special, player_id, state);
}

function weekscript_notify_note_loss(script, strum, strum_note_idx, plyrstts, is_penalty) {
    /** @type {StrumNote} */
    let strum_note = strum.chart_notes[strum_note_idx];
    let ignore_miss = strum.attribute_notes[strum_note.id].ignore_miss;

    if (ignore_miss) return;

    let timestamp = strum_note.timestamp;
    let duration = strum_note.duration;
    let note_id = strum.chart_notes_id_map[strum_note.id];
    let data = strum_note.custom_data;
    let special = strum.attribute_notes[strum_note.id].is_special;
    let player_id = strum.player_id;
    let state = is_penalty ? SCRIPT_NOTE_PENALITY : SCRIPT_NOTE_MISS;

    _weekscript_notify_note(script, timestamp, note_id, duration, data, special, player_id, state);
}

function weekscript_notify_unknownnote(script, player_id, timestamp, direction, duration, data) {
    return _weekscript_call(
        ModuleLuaScript._luascript_notify_unknownnote, script, player_id, timestamp, direction, duration, data
    );
}

function weekscript_notify_roundend(script, loose) {
    return _weekscript_call(ModuleLuaScript._luascript_notify_roundend, script, loose ? 1 : 0);
}

function weekscript_notify_weekend(script, giveup) {
    return _weekscript_call(ModuleLuaScript._luascript_notify_weekend, script, giveup ? 1 : 0);
}

function weekscript_notify_gameoverloop(script) {
    return _weekscript_call(ModuleLuaScript._luascript_notify_gameoverloop, script);
}

function weekscript_notify_gameoverdecision(script, retry_or_giveup, changed_difficult) {
    return _weekscript_call(
        ModuleLuaScript._luascript_notify_gameoverdecision, script,
        retry_or_giveup ? 1 : 0, ModuleLuaScript.kdmyEngine_stringToPtr(changed_difficult)
    );
}

function weekscript_notify_gameoverended(script) {
    return _weekscript_call(ModuleLuaScript._luascript_notify_weekend, script);
}

function weekscript_notify_pause(script, pause_or_resume) {
    return _weekscript_call(ModuleLuaScript._luascript_notify_pause, script, pause_or_resume ? 1 : 0);
}

function weekscript_notify_weekleave(script) {
    return _weekscript_call(ModuleLuaScript._luascript_notify_weekleave, script);
}

function weekscript_notify_afterresults(script, total_attempts, songs_count, reject_completed) {
    return _weekscript_call(
        ModuleLuaScript._luascript_notify_afterresults, script, total_attempts, songs_count, reject_completed ? 1 : 0
    );
}

function weekscript_notify_scriptchange(script) {
    return _weekscript_call(ModuleLuaScript._luascript_notify_scriptchange, script);
}

function weekscript_notify_pause_optionchoosen(script, option_index) {
    return _weekscript_call(
        ModuleLuaScript._luascript_notify_pause_optionchoosen, script, option_index
    );
}

function weekscript_notify_pause_menuvisible(script, shown_or_hidden) {
    return _weekscript_call(ModuleLuaScript._luascript_notify_pause_menuvisible, script, shown_or_hidden ? 1 : 0);
}

function weekscript_notify_beat(script, beat_count, since) {
    return _weekscript_call(ModuleLuaScript._luascript_notify_beat, script, beat_count, since);
}

function weekscript_notify_quarter(script, quarter_count, since) {
    return _weekscript_call(ModuleLuaScript._luascript_notify_quarter, script, quarter_count, since);
}

function weekscript_notify_buttons(script, player_id, buttons) {
    return _weekscript_call(ModuleLuaScript._luascript_notify_buttons, script, player_id, buttons);
}

function weekscript_notify_after_strum_scroll(script) {
    return _weekscript_call(ModuleLuaScript._luascript_notify_after_strum_scroll, script);
}

function weekscript_notify_dialogue_builtin_open(script, dialog_src) {
    // (JS only) allocate string in WASM memory, the caller deallocates it.
    let dialog_src_ptr = ModuleLuaScript.kdmyEngine_stringToPtr(dialog_src);
    return _weekscript_call(ModuleLuaScript._luascript_notify_dialogue_builtin_open, script, dialog_src_ptr);
}

function weekscript_get_luascript(script) {
    return script;
}


function _weekscript_notify_note(script, timestamp, id, duration, data, special, player_id, state) {
    let promise = _weekscript_call(
        ModuleLuaScript._luascript_notify_note, script, timestamp, id, duration, data, special, player_id, state
    );
    if (promise) console.warn("luascript_notify_note() in JS can run async");
}

function _weekscript_call(fn, ...args) {
    if (!args[0]) return;

    let ret = fn.apply(null, args);
    let promise = ModuleLuaScript.kdmyEngine_yieldAsync();
    return promise === undefined ? ret : promise;
}

