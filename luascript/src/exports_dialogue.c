#include "engine_string.h"
#include "luascript_internal.h"



#ifdef JAVASCRIPT
EM_JS_PRFX(bool, dialogue_apply_state, (Dialogue dialogue, const char* state_name), {
    return dialogue_apply_state(kdmyEngine_obtain(dialogue), kdmyEngine_ptrToString(state_name));
});
EM_JS_PRFX(bool, dialogue_apply_state2, (Dialogue dialogue, const char* state_name, const char* if_line_label), {
    return dialogue_apply_state2(kdmyEngine_obtain(dialogue), kdmyEngine_ptrToString(state_name), kdmyEngine_ptrToString(if_line_label));
});
EM_JS_PRFX(bool, dialogue_is_completed, (Dialogue dialogue), {
    return dialogue_is_completed(kdmyEngine_obtain(dialogue));
});
EM_JS_PRFX(bool, dialogue_is_hidden, (Dialogue dialogue), {
    return dialogue_is_hidden(kdmyEngine_obtain(dialogue));
});
EM_ASYNC_JS_PRFX(bool, dialogue_show_dialog, (Dialogue dialogue, const char* dialog_src), {
    return await dialogue_show_dialog(kdmyEngine_obtain(dialogue), kdmyEngine_ptrToString(dialog_src));
});
EM_JS_PRFX(void, dialogue_close, (Dialogue dialogue), {
    dialogue_close(kdmyEngine_obtain(dialogue));
});
EM_JS_PRFX(void, dialogue_hide, (Dialogue dialogue, bool hidden), {
    dialogue_hide(kdmyEngine_obtain(dialogue), hidden);
});
EM_JS_PRFX(Modifier, dialogue_get_modifier, (Dialogue dialogue), {
    const modifier = dialogue_get_modifier(kdmyEngine_obtain(dialogue));
    return kdmyEngine_obtain(modifier);
});
EM_JS_PRFX(void, dialogue_set_offsetcolor, (Dialogue dialogue, float r, float g, float b, float a), {
    dialogue_set_offsetcolor(kdmyEngine_obtain(dialogue), r, g, b, a);
});
EM_JS_PRFX(void, dialogue_set_alpha, (Dialogue dialogue, float alpha), {
    dialogue_set_alpha(kdmyEngine_obtain(dialogue), alpha);
});
EM_JS_PRFX(void, dialogue_set_antialiasing, (Dialogue dialogue, PVRFlag antialiasing), {
    dialogue_set_antialiasing(kdmyEngine_obtain(dialogue), antialiasing);
});
#endif


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

    const char* dialog_src = luaL_optstring(L, 2, NULL);

    bool ret = dialogue_show_dialog(dialogue, dialog_src);

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

    Modifier modifier = dialogue_get_modifier(dialogue);

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

    PVRFlag antialiasing = luascript_parse_pvrflag(L, luaL_optstring(L, 2, NULL));

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
