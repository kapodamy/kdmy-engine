#include "luascript_internal.h"

#include "game/funkin/rankingcounter.h"


static int script_rankingcounter_add_state(lua_State* L) {
    RankingCounter rankingcounter = luascript_read_userdata(L, RANKINGCOUNTER);
    ModelHolder modelholder = luascript_read_nullable_userdata(L, 2, MODELHOLDER);
    const char* state_name = luaL_optstring(L, 3, NULL);

    int32_t ret = rankingcounter_add_state(rankingcounter, modelholder, state_name);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_rankingcounter_toggle_state(lua_State* L) {
    RankingCounter rankingcounter = luascript_read_userdata(L, RANKINGCOUNTER);
    const char* state_name = luaL_optstring(L, 2, NULL);

    rankingcounter_toggle_state(rankingcounter, state_name);

    return 0;
}

static int script_rankingcounter_reset(lua_State* L) {
    RankingCounter rankingcounter = luascript_read_userdata(L, RANKINGCOUNTER);

    rankingcounter_reset(rankingcounter);

    return 0;
}

static int script_rankingcounter_hide_accuracy(lua_State* L) {
    RankingCounter rankingcounter = luascript_read_userdata(L, RANKINGCOUNTER);
    bool hide = (bool)lua_toboolean(L, 2);

    rankingcounter_hide_accuracy(rankingcounter, hide);

    return 0;
}

static int script_rankingcounter_use_percent_instead(lua_State* L) {
    RankingCounter rankingcounter = luascript_read_userdata(L, RANKINGCOUNTER);
    bool use_accuracy_percenter = (bool)lua_toboolean(L, 2);

    rankingcounter_use_percent_instead(rankingcounter, use_accuracy_percenter);

    return 0;
}

static int script_rankingcounter_set_default_ranking_animation2(lua_State* L) {
    RankingCounter rankingcounter = luascript_read_userdata(L, RANKINGCOUNTER);
    AnimSprite animsprite = luascript_read_nullable_userdata(L, 2, ANIMSPRITE);

    rankingcounter_set_default_ranking_animation2(rankingcounter, animsprite);

    return 0;
}

static int script_rankingcounter_set_default_ranking_text_animation2(lua_State* L) {
    RankingCounter rankingcounter = luascript_read_userdata(L, RANKINGCOUNTER);
    AnimSprite animsprite = luascript_read_nullable_userdata(L, 2, ANIMSPRITE);

    rankingcounter_set_default_ranking_text_animation2(rankingcounter, animsprite);

    return 0;
}

static int script_rankingcounter_set_alpha(lua_State* L) {
    RankingCounter rankingcounter = luascript_read_userdata(L, RANKINGCOUNTER);
    float alpha = (float)luaL_checknumber(L, 2);

    rankingcounter_set_alpha(rankingcounter, alpha);

    return 0;
}

static int script_rankingcounter_animation_set(lua_State* L) {
    RankingCounter rankingcounter = luascript_read_userdata(L, RANKINGCOUNTER);
    AnimSprite animsprite = luascript_read_nullable_userdata(L, 2, ANIMSPRITE);

    rankingcounter_animation_set(rankingcounter, animsprite);

    return 0;
}

static int script_rankingcounter_animation_restart(lua_State* L) {
    RankingCounter rankingcounter = luascript_read_userdata(L, RANKINGCOUNTER);

    rankingcounter_animation_restart(rankingcounter);

    return 0;
}

static int script_rankingcounter_animation_end(lua_State* L) {
    RankingCounter rankingcounter = luascript_read_userdata(L, RANKINGCOUNTER);

    rankingcounter_animation_end(rankingcounter);

    return 0;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static const luaL_Reg RANKINGCOUNTER_FUNCTIONS[] = {
    {"add_state", script_rankingcounter_add_state},
    {"toggle_state", script_rankingcounter_toggle_state},
    {"reset", script_rankingcounter_reset},
    {"hide_accuracy", script_rankingcounter_hide_accuracy},
    {"use_percent_instead", script_rankingcounter_use_percent_instead},
    {"set_default_ranking_animation2", script_rankingcounter_set_default_ranking_animation2},
    {"set_default_ranking_text_animation2", script_rankingcounter_set_default_ranking_text_animation2},
    {"set_alpha", script_rankingcounter_set_alpha},
    {"animation_set", script_rankingcounter_animation_set},
    {"animation_restart", script_rankingcounter_animation_restart},
    {"animation_end", script_rankingcounter_animation_end},
    {NULL, NULL}
};

int script_rankingcounter_new(lua_State* L, RankingCounter rankingcounter) {
    return luascript_userdata_new(L, RANKINGCOUNTER, rankingcounter);
}

static int script_rankingcounter_gc(lua_State* L) {
    return luascript_userdata_gc(L, RANKINGCOUNTER);
}

static int script_rankingcounter_tostring(lua_State* L) {
    return luascript_userdata_tostring(L, RANKINGCOUNTER);
}

void script_rankingcounter_register(lua_State* L) {
    luascript_register(L, RANKINGCOUNTER, script_rankingcounter_gc, script_rankingcounter_tostring, RANKINGCOUNTER_FUNCTIONS);
}
