#include "luascript_internal.h"

#include "game/gameplay/dialogue.h"
#include "luascript_enums.h"


static int script_dialogue_apply_state(lua_State* L) {
    Dialogue dialogue = luascript_read_userdata(L, DIALOGUE);

    const char* state_name = luaL_optstring(L, 2, NULL);

    bool ret = dialogue_apply_state(dialogue, state_name);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_dialogue_apply_state2(lua_State* L) {
    Dialogue dialogue = luascript_read_userdata(L, DIALOGUE);

    const char* state_name = luaL_optstring(L, 2, NULL);
    const char* if_line_label = luaL_optstring(L, 3, NULL);

    bool ret = dialogue_apply_state2(dialogue, state_name, if_line_label);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_dialogue_is_completed(lua_State* L) {
    Dialogue dialogue = luascript_read_userdata(L, DIALOGUE);

    bool ret = dialogue_is_completed(dialogue);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_dialogue_is_hidden(lua_State* L) {
    Dialogue dialogue = luascript_read_userdata(L, DIALOGUE);

    bool ret = dialogue_is_hidden(dialogue);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_dialogue_show_dialog(lua_State* L) {
    Dialogue dialogue = luascript_read_userdata(L, DIALOGUE);

    const char* dialog_src = luaL_checkstring(L, 2);

    luascript_change_working_folder(L);
    bool ret = dialogue_show_dialog(dialogue, dialog_src);
    luascript_restore_working_folder(L);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_dialogue_show_dialog2(lua_State* L) {
    Dialogue dialogue = luascript_read_userdata(L, DIALOGUE);

    const char* text_dialog_content = luaL_checkstring(L, 2);

    bool ret = dialogue_show_dialog2(dialogue, text_dialog_content);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_dialogue_close(lua_State* L) {
    Dialogue dialogue = luascript_read_userdata(L, DIALOGUE);

    dialogue_close(dialogue);

    return 0;
}

static int script_dialogue_hide(lua_State* L) {
    Dialogue dialogue = luascript_read_userdata(L, DIALOGUE);

    bool hidden = lua_toboolean(L, 2);

    dialogue_hide(dialogue, hidden);

    return 0;
}

static int script_dialogue_get_modifier(lua_State* L) {
    Dialogue dialogue = luascript_read_userdata(L, DIALOGUE);

    Modifier* modifier = dialogue_get_modifier(dialogue);

    return script_modifier_new(L, modifier);
}

static int script_dialogue_set_offsetcolor(lua_State* L) {
    Dialogue dialogue = luascript_read_userdata(L, DIALOGUE);

    float r = (float)luaL_checknumber(L, 2);
    float g = (float)luaL_checknumber(L, 3);
    float b = (float)luaL_checknumber(L, 4);
    float a = (float)luaL_checknumber(L, 5);

    dialogue_set_offsetcolor(dialogue, r, g, b, a);

    return 0;
}

static int script_dialogue_set_alpha(lua_State* L) {
    Dialogue dialogue = luascript_read_userdata(L, DIALOGUE);

    float alpha = (float)luaL_checknumber(L, 2);

    dialogue_set_alpha(dialogue, alpha);

    return 0;
}

static int script_dialogue_set_set_antialiasing(lua_State* L) {
    Dialogue dialogue = luascript_read_userdata(L, DIALOGUE);

    PVRFlag antialiasing = luascript_helper_optenum(L, 2, LUASCRIPT_ENUMS_PVRFlag);

    dialogue_set_antialiasing(dialogue, antialiasing);

    return 0;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const luaL_Reg DIALOGUE_FUNCTIONS[] = {
    {"apply_state", script_dialogue_apply_state},
    {"apply_state2", script_dialogue_apply_state2},
    {"is_completed", script_dialogue_is_completed},
    {"is_hidden", script_dialogue_is_hidden},
    {"show_dialog", script_dialogue_show_dialog},
    {"show_dialog2", script_dialogue_show_dialog2},
    {"close", script_dialogue_close},
    {"hide", script_dialogue_hide},
    {"get_modifier", script_dialogue_get_modifier},
    {"set_offsetcolor", script_dialogue_set_offsetcolor},
    {"set_alpha", script_dialogue_set_alpha},
    {"set_antialiasing", script_dialogue_set_set_antialiasing},
    {NULL, NULL}
};


int script_dialogue_new(lua_State* L, Dialogue dialogue) {
    return luascript_userdata_new(L, DIALOGUE, dialogue);
}

static int script_dialoge_gc(lua_State* L) {
    return luascript_userdata_gc(L, DIALOGUE);
}

static int script_dialogue_tostring(lua_State* L) {
    return luascript_userdata_tostring(L, DIALOGUE);
}

void script_dialogue_register(lua_State* L) {
    luascript_register(L, DIALOGUE, script_dialoge_gc, script_dialogue_tostring, DIALOGUE_FUNCTIONS);
}
