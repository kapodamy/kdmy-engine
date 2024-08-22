"use strict";


function script_week_unlockdirective_create(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;

    let name = LUA.luaL_optstring(L, 1, null);
    let completed_round = LUA.lua_toboolean(L, 2);
    let completed_week = LUA.lua_toboolean(L, 3);
    let value = LUA.luaL_checknumber(L, 4);

    week_unlockdirective_create(roundcontext, name, completed_round, completed_week, value);

    return 0;
}

function script_week_unlockdirective_remove(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;

    let name = LUA.luaL_optstring(L, 1, null);
    let completed_round = LUA.lua_toboolean(L, 2);
    let completed_week = LUA.lua_toboolean(L, 3);

    week_unlockdirective_remove(roundcontext, name, completed_round, completed_week);

    return 0;
}

function script_week_unlockdirective_get(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;

    let name = LUA.luaL_optstring(L, 1, null);

    if (!week_unlockdirective_has(roundcontext, name)) {
        LUA.lua_pushnil(L);
        return 1;
    }

    let ret = week_unlockdirective_get(roundcontext, name);

    LUA.lua_pushnumber(L, ret);
    return 1;
}

function script_week_ui_set_visibility(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;

    let visible = LUA.lua_toboolean(L, 1);

    week_ui_set_visibility(roundcontext, visible);

    return 0;
}

function script_week_ui_get_layout(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;


    let ret = week_ui_get_layout(roundcontext);

    return script_layout_new(L, ret);
}

function script_week_ui_get_camera(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;


    let ret = week_ui_get_camera(roundcontext);

    return script_camera_new(L, ret);
}

function script_week_set_halt(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;

    let halt = LUA.lua_toboolean(L, 1);

    week_set_halt(roundcontext, halt);

    return 0;
}

function script_week_disable_week_end_results(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;

    let disable = LUA.lua_toboolean(L, 1);

    week_disable_week_end_results(roundcontext, disable);

    return 0;
}

function script_week_disable_girlfriend_cry(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;

    let disable = LUA.lua_toboolean(L, 1);

    week_disable_girlfriend_cry(roundcontext, disable);

    return 0;
}

function script_week_disable_ask_ready(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;

    let disable = LUA.lua_toboolean(L, 1);

    week_disable_ask_ready(roundcontext, disable);

    return 0;
}

function script_week_disable_countdown(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;

    let disable = LUA.lua_toboolean(L, 1);

    week_disable_countdown(roundcontext, disable);

    return 0;
}

function script_week_disable_camera_bumping(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;

    let disable = LUA.lua_toboolean(L, 1);

    week_disable_camera_bumping(roundcontext, disable);

    return 0;
}

function script_week_ui_get_strums_count(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;


    let ret = week_ui_get_strums_count(roundcontext);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_week_ui_get_strums(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;

    let strums_id = LUA.luaL_checkinteger(L, 1);

    let ret = week_ui_get_strums(roundcontext, strums_id);

    return script_strums_new(L, ret);
}

function script_week_ui_get_roundstats(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;


    let ret = week_ui_get_roundstats(roundcontext);

    return script_roundstats_new(L, ret);
}

function script_week_ui_get_rankingcounter(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;


    let ret = week_ui_get_rankingcounter(roundcontext);

    return script_rankingcounter_new(L, ret);
}

function script_week_ui_get_streakcounter(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;


    let ret = week_ui_get_streakcounter(roundcontext);

    return script_streakcounter_new(L, ret);
}

function script_week_ui_get_round_textsprite(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;


    let ret = week_ui_get_round_textsprite(roundcontext);

    return script_textsprite_new(L, ret);
}

function script_week_ui_get_songprogressbar(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;


    let ret = week_ui_get_songprogressbar(roundcontext);

    return script_songprogressbar_new(L, ret);
}

function script_week_ui_get_countdown(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;


    let ret = week_ui_get_countdown(roundcontext);

    return script_countdown_new(L, ret);
}

function script_week_ui_get_healthbar(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;


    let ret = week_ui_get_healthbar(roundcontext);

    return script_healthbar_new(L, ret);
}

function script_week_get_stage_layout(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;


    let ret = week_get_stage_layout(roundcontext);

    return script_layout_new(L, ret);
}

function script_week_get_healthwatcher(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;


    let ret = week_get_healthwatcher(roundcontext);

    return script_healthwatcher_new(L, ret);
}

function script_week_get_missnotefx(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;


    let ret = week_get_missnotefx(roundcontext);

    return script_missnotefx_new(L, ret);
}

function script_week_update_bpm(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;

    let bpm = LUA.luaL_checknumber(L, 1);

    week_update_bpm(roundcontext, bpm);

    return 0;
}

function script_week_update_speed(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;

    let speed = LUA.luaL_checknumber(L, 1);

    week_update_speed(roundcontext, speed);

    return 0;
}

function script_week_get_messagebox(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;


    let ret = week_get_messagebox(roundcontext);

    return script_messagebox_new(L, ret);
}

function script_week_get_girlfriend(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;


    let ret = week_get_girlfriend(roundcontext);

    return script_character_new(L, ret);
}

function script_week_get_character_count(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;


    let ret = week_get_character_count(roundcontext);

    LUA.lua_pushinteger(L, ret);
    return 1;
}

function script_week_get_conductor(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;

    let character_index = LUA.luaL_checkinteger(L, 1);

    let ret = week_get_conductor(roundcontext, character_index);

    return script_conductor_new(L, ret);
}

function script_week_get_character(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;

    let character_index = LUA.luaL_checkinteger(L, 1);

    let ret = week_get_character(roundcontext, character_index);

    return script_character_new(L, ret);
}

function script_week_get_playerstats(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;

    let character_index = LUA.luaL_checkinteger(L, 1);

    let ret = week_get_playerstats(roundcontext, character_index);

    return script_playerstats_new(L, ret);
}

function script_week_get_songplayer(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;


    let ret = week_get_songplayer(roundcontext);

    return script_songplayer_new(L, ret);
}

function script_week_get_current_chart_info(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;

    const chartinfo = { bpm: 0.0, speed: 0.0 };
    week_get_current_chart_info(roundcontext, chartinfo);

    LUA.lua_pushnumber(L, chartinfo.bpm);
    LUA.lua_pushnumber(L, chartinfo.speed);
    return 2;
}

function script_week_get_current_song_info(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;

    const songinfo = { name: null, difficult: null, index: 0 };
    week_get_current_song_info(roundcontext, songinfo);

    LUA.lua_pushstring(L, songinfo.name);
    LUA.lua_pushstring(L, songinfo.difficult);
    LUA.lua_pushinteger(L, songinfo.index);

    return 3;
}

function script_week_change_character_camera_name(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;

    let opponent_or_player = LUA.lua_toboolean(L, 1);
    let new_name = LUA.luaL_optstring(L, 2, null);

    week_change_character_camera_name(roundcontext, opponent_or_player, new_name);

    return 0;
}

function script_week_disable_layout_rollback(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;

    let disable = LUA.lua_toboolean(L, 1);

    week_disable_layout_rollback(roundcontext, disable);

    return 0;
}

function script_week_override_common_folder(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;

    let custom_common_path = LUA.luaL_optstring(L, 1, null);

    luascript_change_working_folder(L);
    week_override_common_folder(roundcontext, custom_common_path);
    luascript_restore_working_folder(L);

    return 0;
}

function script_week_enable_credits_on_completed(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;


    week_enable_credits_on_completed(roundcontext);

    return 0;
}

function script_week_end(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;

    let round_or_week = LUA.lua_toboolean(L, 1);
    let loose_or_win = LUA.lua_toboolean(L, 2);

    week_end(roundcontext, round_or_week, loose_or_win);

    return 0;
}

function script_week_get_dialogue(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;


    let dialogue = week_get_dialogue(roundcontext);

    return script_dialogue_new(L, dialogue);
}

function script_week_set_ui_shader(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;

    let psshader = luascript_read_nullable_userdata(L, 1, PSSHADER);

    week_set_ui_shader(roundcontext, psshader);

    return 0;
}

async function script_week_rebuild_ui(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;


    await week_rebuild_ui(roundcontext);

    return 0;
}

function script_week_storage_get(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;

    let name = LUA.luaL_optstring(L, 1, null);
    const out_data = [null];

    let ret = week_storage_get(roundcontext, name, out_data);

    if (!out_data[0])
        LUA.lua_pushnil(L);
    else
        LUA.lua_pushlstring(L, out_data[0], ret);

    return 1;
}

function script_week_storage_set(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;

    let name = LUA.luaL_optstring(L, 1, null);
    let data;
    let data_size = [0];

    if (LUA.lua_isnil(L, 2)) {
        data = null;
        data_size[0] = 0;
    } else {
        data = LUA.luaL_checklstring(L, 3, data_size);
    }

    let ret = week_storage_set(roundcontext, name, data, data_size[0]);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

async function script_week_gameover_no_music(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;

    await week_set_gameover_option(roundcontext, WEEK_GAMEOVER_NOMUSIC, NaN, null);

    return 0;
}

async function script_week_gameover_no_sfx_die(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;

    await week_set_gameover_option(roundcontext, WEEK_GAMEOVER_NOSFXDIE, NaN, null);

    return 0;
}

async function script_week_gameover_no_sfx_retry(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;

    await week_set_gameover_option(roundcontext, WEEK_GAMEOVER_NOSFXRETRY, NaN, null);

    return 0;
}

async function script_week_gameover_set_die_anim_duration(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;

    let duration_ms = LUA.luaL_checknumber(L, 1);

    await week_set_gameover_option(roundcontext, WEEK_GAMEOVER_ANIMDURATIONDIE, duration_ms, null);

    return 0;
}

async function script_week_gameover_set_retry_anim_duration(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;

    let duration_ms = LUA.luaL_checknumber(L, 1);

    await week_set_gameover_option(roundcontext, WEEK_GAMEOVER_ANIMDURATIONRETRY, duration_ms, null);

    return 0;
}

async function script_week_gameover_set_giveup_anim_duration(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;

    let duration_ms = LUA.luaL_checknumber(L, 1);

    await week_set_gameover_option(roundcontext, WEEK_GAMEOVER_ANIMDURATIONGIVEUP, duration_ms, null);

    return 0;
}

async function script_week_gameover_set_before_anim_duration(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;

    let duration_ms = LUA.luaL_checknumber(L, 1);

    await week_set_gameover_option(roundcontext, WEEK_GAMEOVER_ANIMDURATIONBEFORE, duration_ms, null);

    return 0;
}

async function script_week_gameover_set_before_force_end_anim_duration(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;

    let duration_ms = LUA.luaL_checknumber(L, 1);

    await week_set_gameover_option(roundcontext, WEEK_GAMEOVER_ANIMDURATIONBEFOREFORCEEND, duration_ms, null);

    return 0;
}

async function script_week_gameover_set_music(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;

    let filename = LUA.luaL_optstring(L, 1, null);

    await week_set_gameover_option(roundcontext, WEEK_GAMEOVER_SETMUSIC, NaN, filename);

    return 0;
}

async function script_week_gameover_set_sfx_die(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;

    let filename = LUA.luaL_optstring(L, 1, null);

    await week_set_gameover_option(roundcontext, WEEK_GAMEOVER_SETSFXDIE, NaN, filename);

    return 0;
}

async function script_week_gameover_set_sfx_confirm(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;

    let filename = LUA.luaL_optstring(L, 1, null);

    await week_set_gameover_option(roundcontext, WEEK_GAMEOVER_SETSFXRETRY, NaN, filename);

    return 0;
}

function script_week_get_accumulated_stats(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;

    let stats = {
        sick: 0,
        good: 0,
        bads: 0,
        shits: 0,
        miss: 0,
        penalties: 0,
        score: 0,
        accuracy: 0.0,
        notesperseconds: 0,
        combobreaks: 0,
        higheststreak: 0
    };

    week_get_accumulated_stats(roundcontext, stats);

    LUA.lua_createtable(L, 0, 11);
    luascript_helper_add_table_field(L, "sick", LUA.lua_pushinteger, stats.sick);
    luascript_helper_add_table_field(L, "good", LUA.lua_pushinteger, stats.good);
    luascript_helper_add_table_field(L, "bads", LUA.lua_pushinteger, stats.bads);
    luascript_helper_add_table_field(L, "shits", LUA.lua_pushinteger, stats.shits);
    luascript_helper_add_table_field(L, "miss", LUA.lua_pushinteger, stats.miss);
    luascript_helper_add_table_field(L, "penalties", LUA.lua_pushinteger, stats.penalties);
    luascript_helper_add_table_field(L, "score", LUA.lua_pushinteger, stats.score);
    luascript_helper_add_table_field(L, "accuracy", LUA.lua_pushnumber, stats.accuracy);
    luascript_helper_add_table_field(L, "notesPerSeconds", LUA.lua_pushinteger, stats.notesperseconds);
    luascript_helper_add_table_field(L, "comboBreaks", LUA.lua_pushinteger, stats.combobreaks);
    luascript_helper_add_table_field(L, "highestStreak", LUA.lua_pushinteger, stats.higheststreak);

    return 1;
}

function script_week_get_gameover_layout(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;

    let ret = week_get_layout_of(roundcontext, 'g');

    return script_layout_new(L, ret);
}

function script_week_get_pause_layout(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;

    let ret = week_get_layout_of(roundcontext, 'p');

    return script_layout_new(L, ret);
}

function script_week_get_results_layout(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;

    let ret = week_get_layout_of(roundcontext, 'r');

    return script_layout_new(L, ret);
}

function script_week_set_pause_background_music(L) {
    let luascript = luascript_get_instance(L);
    let roundcontext = luascript.context;

    let filename = LUA.luaL_optstring(L, 1, null);

    week_set_pause_background_music(roundcontext, filename);

    return 0;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/**@type {luaL_Reg[]}*/const WEEK_FUNCTIONS = [
    ["week_ui_set_visibility", script_week_ui_set_visibility],
    ["week_ui_get_layout", script_week_ui_get_layout],
    ["week_ui_get_camera", script_week_ui_get_camera],
    ["week_set_halt", script_week_set_halt],
    ["week_disable_week_end_results", script_week_disable_week_end_results],
    ["week_disable_girlfriend_cry", script_week_disable_girlfriend_cry],
    ["week_disable_ask_ready", script_week_disable_ask_ready],
    ["week_disable_countdown", script_week_disable_countdown],
    ["week_disable_camera_bumping", script_week_disable_camera_bumping],
    ["week_ui_get_strums_count", script_week_ui_get_strums_count],
    ["week_ui_get_strums", script_week_ui_get_strums],
    ["week_ui_get_roundstats", script_week_ui_get_roundstats],
    ["week_ui_get_rankingcounter", script_week_ui_get_rankingcounter],
    ["week_ui_get_streakcounter", script_week_ui_get_streakcounter],
    ["week_ui_get_round_textsprite", script_week_ui_get_round_textsprite],
    ["week_ui_get_songprogressbar", script_week_ui_get_songprogressbar],
    ["week_ui_get_countdown", script_week_ui_get_countdown],
    ["week_ui_get_healthbar", script_week_ui_get_healthbar],
    ["week_get_stage_layout", script_week_get_stage_layout],
    ["week_get_healthwatcher", script_week_get_healthwatcher],
    ["week_get_missnotefx", script_week_get_missnotefx],
    ["week_update_bpm", script_week_update_bpm],
    ["week_update_speed", script_week_update_speed],
    ["week_get_messagebox", script_week_get_messagebox],
    ["week_get_girlfriend", script_week_get_girlfriend],
    ["week_get_character_count", script_week_get_character_count],
    ["week_get_conductor", script_week_get_conductor],
    ["week_get_character", script_week_get_character],
    ["week_get_playerstats", script_week_get_playerstats],
    ["week_get_songplayer", script_week_get_songplayer],
    ["week_get_current_chart_info", script_week_get_current_chart_info],
    ["week_get_current_song_info", script_week_get_current_song_info],
    ["week_change_character_camera_name", script_week_change_character_camera_name],
    ["week_disable_layout_rollback", script_week_disable_layout_rollback],
    ["week_override_common_folder", script_week_override_common_folder],
    ["week_enable_credits_on_completed", script_week_enable_credits_on_completed],
    ["week_end", script_week_end],
    ["week_get_dialogue", script_week_get_dialogue],
    ["week_set_ui_shader", script_week_set_ui_shader],
    ["week_rebuild_ui", script_week_rebuild_ui],
    ["week_unlockdirective_create", script_week_unlockdirective_create],
    ["week_unlockdirective_get", script_week_unlockdirective_get],
    ["week_unlockdirective_remove", script_week_unlockdirective_remove],
    ["week_storage_set", script_week_storage_set],
    ["week_storage_get", script_week_storage_get],
    ["week_gameover_no_music", script_week_gameover_no_music],
    ["week_gameover_no_sfx_die", script_week_gameover_no_sfx_die],
    ["week_gameover_no_sfx_retry", script_week_gameover_no_sfx_retry],
    ["week_gameover_set_die_anim_duration", script_week_gameover_set_die_anim_duration],
    ["week_gameover_set_retry_anim_duration", script_week_gameover_set_retry_anim_duration],
    ["week_gameover_set_giveup_anim_duration", script_week_gameover_set_giveup_anim_duration],
    ["week_gameover_set_music", script_week_gameover_set_music],
    ["week_gameover_set_sfx_die", script_week_gameover_set_sfx_die],
    ["week_gameover_set_sfx_confirm", script_week_gameover_set_sfx_confirm],
    ["week_gameover_set_before_anim_duration", script_week_gameover_set_before_anim_duration],
    ["week_gameover_set_before_force_end_anim_duration", script_week_gameover_set_before_force_end_anim_duration],
    ["week_get_accumulated_stats", script_week_get_accumulated_stats],
    ["week_get_gameover_layout", script_week_get_gameover_layout],
    ["week_get_pause_layout", script_week_get_pause_layout],
    ["week_get_results_layout", script_week_get_results_layout],
    ["week_set_pause_background_music", script_week_set_pause_background_music],
    [null, null]
];


function script_week_register(L) {
    for (let i = 0; ; i++) {
        if (WEEK_FUNCTIONS[i][0] == null || WEEK_FUNCTIONS[i][1] == null) break;
        LUA.lua_pushcfunction(L, WEEK_FUNCTIONS[i][1]);
        LUA.lua_setglobal(L, WEEK_FUNCTIONS[i][0]);
    }
}
