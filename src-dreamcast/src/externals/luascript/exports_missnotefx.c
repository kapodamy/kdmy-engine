#include "luascript_internal.h"

#include "game/funkin/missnotefx.h"


static int script_missnotefx_disable(lua_State* L) {
    MissNoteFX missnotefx = luascript_read_userdata(L, MISSNOTEFX);
    bool disabled = (bool)lua_toboolean(L, 2);

    missnotefx_disable(missnotefx, disabled);

    return 0;
}

static int script_missnotefx_stop(lua_State* L) {
    MissNoteFX missnotefx = luascript_read_userdata(L, MISSNOTEFX);

    missnotefx_stop(missnotefx);

    return 0;
}

static int script_missnotefx_play_effect(lua_State* L) {
    MissNoteFX missnotefx = luascript_read_userdata(L, MISSNOTEFX);

    missnotefx_play_effect(missnotefx);

    return 0;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const luaL_Reg MISSNOTEFX_FUNCTIONS[] = {
    {"missnotefx_disable", script_missnotefx_disable},
    {"missnotefx_stop", script_missnotefx_stop},
    {"missnotefx_play_effect", script_missnotefx_play_effect},
    {NULL, NULL}
};

int script_missnotefx_new(lua_State* L, MissNoteFX missnotefx) {
    return luascript_userdata_new(L, MISSNOTEFX, missnotefx);
}

static int script_missnotefx_gc(lua_State* L) {
    return luascript_userdata_gc(L, MISSNOTEFX);
}

static int script_missnotefx_tostring(lua_State* L) {
    return luascript_userdata_tostring(L, MISSNOTEFX);
}

void script_missnotefx_register(lua_State* L) {
    luascript_register(L, MISSNOTEFX, script_missnotefx_gc, script_missnotefx_tostring, MISSNOTEFX_FUNCTIONS);
}
