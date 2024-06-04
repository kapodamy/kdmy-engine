"use strict";

const SONGPROGRESSBAR = "SongProgressbar";


function script_songprogressbar_set_songplayer(L) {
    let songprogressbar = luascript_read_userdata(L, SONGPROGRESSBAR);
    let songplayer = luascript_read_nullable_userdata(L, 2, SONGPLAYER);

    songprogressbar_set_songplayer(songprogressbar, songplayer);

    return 0;
}

function script_songprogressbar_set_duration(L) {
    let songprogressbar = luascript_read_userdata(L, SONGPROGRESSBAR);
    let duration = LUA.luaL_checknumber(L, 2);

    songprogressbar_set_duration(songprogressbar, duration);

    return 0;
}

function script_songprogressbar_get_drawable(L) {
    let songprogressbar = luascript_read_userdata(L, SONGPROGRESSBAR);

    let ret = songprogressbar_get_drawable(songprogressbar);

    return script_drawable_new(L, ret);
}

function script_songprogressbar_set_visible(L) {
    let songprogressbar = luascript_read_userdata(L, SONGPROGRESSBAR);
    let visible = LUA.lua_toboolean(L, 2);

    songprogressbar_set_visible(songprogressbar, visible);

    return 0;
}

function script_songprogressbar_set_background_color(L) {
    let songprogressbar = luascript_read_userdata(L, SONGPROGRESSBAR);
    let r = LUA.luaL_checknumber(L, 2);
    let g = LUA.luaL_checknumber(L, 3);
    let b = LUA.luaL_checknumber(L, 4);
    let a = LUA.luaL_checknumber(L, 5);

    songprogressbar_set_background_color(songprogressbar, r, g, b, a);

    return 0;
}

function script_songprogressbar_set_bar_back_color(L) {
    let songprogressbar = luascript_read_userdata(L, SONGPROGRESSBAR);
    let r = LUA.luaL_checknumber(L, 2);
    let g = LUA.luaL_checknumber(L, 3);
    let b = LUA.luaL_checknumber(L, 4);
    let a = LUA.luaL_checknumber(L, 5);

    songprogressbar_set_bar_back_color(songprogressbar, r, g, b, a);

    return 0;
}

function script_songprogressbar_set_bar_progress_color(L) {
    let songprogressbar = luascript_read_userdata(L, SONGPROGRESSBAR);
    let r = LUA.luaL_checknumber(L, 2);
    let g = LUA.luaL_checknumber(L, 3);
    let b = LUA.luaL_checknumber(L, 4);
    let a = LUA.luaL_checknumber(L, 5);

    songprogressbar_set_bar_progress_color(songprogressbar, r, g, b, a);

    return 0;
}

function script_songprogressbar_set_text_color(L) {
    let songprogressbar = luascript_read_userdata(L, SONGPROGRESSBAR);
    let r = LUA.luaL_checknumber(L, 2);
    let g = LUA.luaL_checknumber(L, 3);
    let b = LUA.luaL_checknumber(L, 4);
    let a = LUA.luaL_checknumber(L, 5);

    songprogressbar_set_text_color(songprogressbar, r, g, b, a);

    return 0;
}

function script_songprogressbar_hide_time(L) {
    let songprogressbar = luascript_read_userdata(L, SONGPROGRESSBAR);
    let hidden = LUA.lua_toboolean(L, 2);

    songprogressbar_hide_time(songprogressbar, hidden);

    return 0;
}

function script_songprogressbar_show_elapsed(L) {
    let songprogressbar = luascript_read_userdata(L, SONGPROGRESSBAR);
    let elapsed_or_remain_time = LUA.lua_toboolean(L, 2);

    songprogressbar_show_elapsed(songprogressbar, elapsed_or_remain_time);

    return 0;
}

function script_songprogressbar_manual_update_enable(L) {
    let songprogressbar = luascript_read_userdata(L, SONGPROGRESSBAR);
    let enabled = LUA.lua_toboolean(L, 2);

    songprogressbar_manual_update_enable(songprogressbar, enabled);

    return 0;
}

function script_songprogressbar_manual_set_text(L) {
    let songprogressbar = luascript_read_userdata(L, SONGPROGRESSBAR);
    let text = LUA.luaL_optstring(L, 2, null);

    songprogressbar_manual_set_text(songprogressbar, text);

    return 0;
}

function script_songprogressbar_manual_set_position(L) {
    let songprogressbar = luascript_read_userdata(L, SONGPROGRESSBAR);
    let elapsed = LUA.luaL_checknumber(L, 2);
    let duration = LUA.luaL_checknumber(L, 3);
    let should_update_time_text = LUA.lua_toboolean(L, 4);

    let ret = songprogressbar_manual_set_position(songprogressbar, elapsed, duration, should_update_time_text);

    LUA.lua_pushnumber(L, ret);
    return 1;
}

function script_songprogressbar_animation_set(L) {
    let songprogressbar = luascript_read_userdata(L, SONGPROGRESSBAR);
    let animsprite = luascript_read_nullable_userdata(L, 2, ANIMSPRITE);

    songprogressbar_animation_set(songprogressbar, animsprite);

    return 0;
}

function script_songprogressbar_animation_restart(L) {
    let songprogressbar = luascript_read_userdata(L, SONGPROGRESSBAR);

    songprogressbar_animation_restart(songprogressbar);

    return 0;
}

function script_songprogressbar_animation_end(L) {
    let songprogressbar = luascript_read_userdata(L, SONGPROGRESSBAR);

    songprogressbar_animation_end(songprogressbar);

    return 0;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/**@type {luaL_Reg[]}*/const SONGPROGRESSBAR_FUNCTIONS = [
    ["set_songplayer", script_songprogressbar_set_songplayer],
    ["set_duration", script_songprogressbar_set_duration],
    ["get_drawable", script_songprogressbar_get_drawable],
    ["set_visible", script_songprogressbar_set_visible],
    ["set_background_color", script_songprogressbar_set_background_color],
    ["set_bar_back_color", script_songprogressbar_set_bar_back_color],
    ["set_bar_progress_color", script_songprogressbar_set_bar_progress_color],
    ["set_text_color", script_songprogressbar_set_text_color],
    ["hide_time", script_songprogressbar_hide_time],
    ["show_elapsed", script_songprogressbar_show_elapsed],
    ["manual_update_enable", script_songprogressbar_manual_update_enable],
    ["manual_set_text", script_songprogressbar_manual_set_text],
    ["manual_set_position", script_songprogressbar_manual_set_position],
    ["animation_set", script_songprogressbar_animation_set],
    ["animation_restart", script_songprogressbar_animation_restart],
    ["animation_end", script_songprogressbar_animation_end],
    [null, null]
];

function script_songprogressbar_new(L, songprogressbar) {
    return luascript_userdata_new(L, SONGPROGRESSBAR, songprogressbar);
}

function script_songprogressbar_gc(L) {
    return luascript_userdata_gc(L, SONGPROGRESSBAR);
}

function script_songprogressbar_tostring(L) {
    return luascript_userdata_tostring(L, SONGPROGRESSBAR);
}

function script_songprogressbar_register(L) {
    luascript_register(L, SONGPROGRESSBAR, script_songprogressbar_gc, script_songprogressbar_tostring, SONGPROGRESSBAR_FUNCTIONS);
}

