"use strict";

const DIALOGUE = "Dialogue";


async function script_dialogue_apply_state(L) {
    let dialogue = luascript_read_userdata(L, DIALOGUE);

    let state_name = LUA.luaL_optstring(L, 2, null);

    let ret = await dialogue_apply_state(dialogue, state_name);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

async function script_dialogue_apply_state2(L) {
    let dialogue = luascript_read_userdata(L, DIALOGUE);

    let state_name = LUA.luaL_optstring(L, 2, null);
    let if_line_label = LUA.luaL_optstring(L, 3, null);

    let ret = await dialogue_apply_state2(dialogue, state_name, if_line_label);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_dialogue_is_completed(L) {
    let dialogue = luascript_read_userdata(L, DIALOGUE);

    let ret = dialogue_is_completed(dialogue);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

function script_dialogue_is_hidden(L) {
    let dialogue = luascript_read_userdata(L, DIALOGUE);

    let ret = dialogue_is_hidden(dialogue);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

async function script_dialogue_show_dialog(L) {
    let dialogue = luascript_read_userdata(L, DIALOGUE);
    let dialog_src = LUA.luaL_checkstring(L, 2);

    luascript_change_working_folder(L);
    let ret = await dialogue_show_dialog(dialogue, dialog_src);
    luascript_restore_working_folder(L);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

async function script_dialogue_show_dialog2(L) {
    let dialogue = luascript_read_userdata(L, DIALOGUE);

    let text_dialog_content = LUA.luaL_checkstring(L, 2);

    let ret = await dialogue_show_dialog2(dialogue, text_dialog_content);

    LUA.lua_pushboolean(L, ret);
    return 1;
}

async function script_dialogue_close(L) {
    let dialogue = luascript_read_userdata(L, DIALOGUE);

    await dialogue_close(dialogue);

    return 0;
}

function script_dialogue_hide(L) {
    let dialogue = luascript_read_userdata(L, DIALOGUE);

    let hidden = LUA.lua_toboolean(L, 2);

    dialogue_hide(dialogue, hidden);

    return 0;
}

function script_dialogue_get_modifier(L) {
    let dialogue = luascript_read_userdata(L, DIALOGUE);

    let modifier = dialogue_get_modifier(dialogue);

    return script_modifier_new(L, modifier);
}

function script_dialogue_set_offsetcolor(L) {
    let dialogue = luascript_read_userdata(L, DIALOGUE);

    let r = LUA.luaL_checknumber(L, 2);
    let g = LUA.luaL_checknumber(L, 3);
    let b = LUA.luaL_checknumber(L, 4);
    let a = LUA.luaL_checknumber(L, 5);

    dialogue_set_offsetcolor(dialogue, r, g, b, a);

    return 0;
}

function script_dialogue_set_alpha(L) {
    let dialogue = luascript_read_userdata(L, DIALOGUE);

    let alpha = LUA.luaL_checknumber(L, 2);

    dialogue_set_alpha(dialogue, alpha);

    return 0;
}

function script_dialogue_set_set_antialiasing(L) {
    let dialogue = luascript_read_userdata(L, DIALOGUE);

    let antialiasing = luascript_helper_optenum(L, 2, LUASCRIPT_ENUMS_PVRFlag);

    dialogue_set_antialiasing(dialogue, antialiasing);

    return 0;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/**@type {luaL_Reg[]}*/const DIALOGUE_FUNCTIONS = [
    ["apply_state", script_dialogue_apply_state],
    ["apply_state2", script_dialogue_apply_state2],
    ["is_completed", script_dialogue_is_completed],
    ["is_hidden", script_dialogue_is_hidden],
    ["show_dialog", script_dialogue_show_dialog],
    ["show_dialog2", script_dialogue_show_dialog2],
    ["close", script_dialogue_close],
    ["hide", script_dialogue_hide],
    ["get_modifier", script_dialogue_get_modifier],
    ["set_offsetcolor", script_dialogue_set_offsetcolor],
    ["set_alpha", script_dialogue_set_alpha],
    ["set_antialiasing", script_dialogue_set_set_antialiasing],
    [null, null]
];


function script_dialogue_new(L, dialogue) {
    return luascript_userdata_new(L, DIALOGUE, dialogue);
}

function script_dialoge_gc(L) {
    return luascript_userdata_gc(L, DIALOGUE);
}

function script_dialogue_tostring(L) {
    return luascript_userdata_tostring(L, DIALOGUE);
}

function script_dialogue_register(L) {
    luascript_register(L, DIALOGUE, script_dialoge_gc, script_dialogue_tostring, DIALOGUE_FUNCTIONS);
}
