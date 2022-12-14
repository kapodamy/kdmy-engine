#include "luascript_internal.h"

#ifdef JAVASCRIPT
EM_JS_PRFX(bool, healthwatcher_add_opponent, (HealthWatcher healthwatcher, PlayerStats playerstats, bool can_recover, bool can_die), {
    let ret = healthwatcher_add_opponent(kdmyEngine_obtain(healthwatcher), kdmyEngine_obtain(playerstats), can_recover, can_die);
    return ret ? 1 : 0;
});
EM_JS_PRFX(bool, healthwatcher_add_player, (HealthWatcher healthwatcher, PlayerStats playerstats, bool can_recover, bool can_die), {
    let ret = healthwatcher_add_player(kdmyEngine_obtain(healthwatcher), kdmyEngine_obtain(playerstats), can_recover, can_die);
    return ret ? 1 : 0;
});
EM_JS_PRFX(int32_t, healthwatcher_has_deads, (HealthWatcher healthwatcher, bool in_players_or_opponents), {
    let ret = healthwatcher_has_deads(kdmyEngine_obtain(healthwatcher), in_players_or_opponents);
    return ret;
});
EM_JS_PRFX(bool, healthwatcher_enable_dead, (HealthWatcher healthwatcher, PlayerStats playerstats, bool can_die), {
    let ret = healthwatcher_enable_dead(kdmyEngine_obtain(healthwatcher), kdmyEngine_obtain(playerstats), can_die);
    return ret ? 1 : 0;
});
EM_JS_PRFX(bool, healthwatcher_enable_recover, (HealthWatcher healthwatcher, PlayerStats playerstats, bool can_recover), {
    let ret = healthwatcher_enable_recover(kdmyEngine_obtain(healthwatcher), kdmyEngine_obtain(playerstats), can_recover);
    return ret ? 1 : 0;
});
EM_JS_PRFX(void, healthwatcher_clear, (HealthWatcher healthwatcher), {
    healthwatcher_clear(kdmyEngine_obtain(healthwatcher));
});
EM_JS_PRFX(void, healthwatcher_balance, (HealthWatcher healthwatcher, HealthBar healthbar), {
    healthwatcher_balance(kdmyEngine_obtain(healthwatcher), kdmyEngine_obtain(healthbar));
});
EM_JS_PRFX(void, healthwatcher_reset_opponents, (HealthWatcher healthwatcher), {
    healthwatcher_reset_opponents(kdmyEngine_obtain(healthwatcher));
});
#endif


static int script_healthwatcher_add_opponent(lua_State* L) {
    HealthWatcher healthwatcher = luascript_read_userdata(L, HEALTHWATCHER);
    PlayerStats playerstats = luascript_read_nullable_userdata(L, 2, PLAYERSTATS);
    bool can_recover = (bool)lua_toboolean(L, 3);
    bool can_die = (bool)lua_toboolean(L, 4);

    bool ret = healthwatcher_add_opponent(healthwatcher, playerstats, can_recover, can_die);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_healthwatcher_add_player(lua_State* L) {
    HealthWatcher healthwatcher = luascript_read_userdata(L, HEALTHWATCHER);
    PlayerStats playerstats = luascript_read_nullable_userdata(L, 2, PLAYERSTATS);
    bool can_recover = (bool)lua_toboolean(L, 3);
    bool can_die = (bool)lua_toboolean(L, 4);

    bool ret = healthwatcher_add_player(healthwatcher, playerstats, can_recover, can_die);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_healthwatcher_has_deads(lua_State* L) {
    HealthWatcher healthwatcher = luascript_read_userdata(L, HEALTHWATCHER);
    bool in_players_or_opponents = lua_toboolean(L, 2);

    int32_t ret = healthwatcher_has_deads(healthwatcher, in_players_or_opponents);

    lua_pushinteger(L, (lua_Integer)ret);
    return 1;
}

static int script_healthwatcher_enable_dead(lua_State* L) {
    HealthWatcher healthwatcher = luascript_read_userdata(L, HEALTHWATCHER);
    PlayerStats playerstats = luascript_read_nullable_userdata(L, 2, PLAYERSTATS);
    bool can_die = (bool)lua_toboolean(L, 3);

    bool ret = healthwatcher_enable_dead(healthwatcher, playerstats, can_die);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_healthwatcher_enable_recover(lua_State* L) {
    HealthWatcher healthwatcher = luascript_read_userdata(L, HEALTHWATCHER);
    PlayerStats playerstats = luascript_read_nullable_userdata(L, 2, PLAYERSTATS);
    bool can_recover = (bool)lua_toboolean(L, 3);

    bool ret = healthwatcher_enable_recover(healthwatcher, playerstats, can_recover);

    lua_pushboolean(L, ret);
    return 1;
}

static int script_healthwatcher_clear(lua_State* L) {
    HealthWatcher healthwatcher = luascript_read_userdata(L, HEALTHWATCHER);

    healthwatcher_clear(healthwatcher);

    return 0;
}

static int script_healthwatcher_balance(lua_State* L) {
    HealthWatcher healthwatcher = luascript_read_userdata(L, HEALTHWATCHER);
    HealthBar healthbar = luascript_read_nullable_userdata(L, 2, HEALTHBAR);

    healthwatcher_balance(healthwatcher, healthbar);

    return 0;
}

static int script_healthwatcher_reset_opponents(lua_State* L) {
    HealthWatcher healthwatcher = luascript_read_userdata(L, HEALTHWATCHER);

    healthwatcher_reset_opponents(healthwatcher);

    return 0;
}




static const luaL_Reg HEALTHWATCHER_FUNCTIONS[] = {
    { "add_opponent", script_healthwatcher_add_opponent },
    { "add_player", script_healthwatcher_add_player },
    { "has_deads", script_healthwatcher_has_deads },
    { "enable_dead", script_healthwatcher_enable_dead },
    { "enable_recover", script_healthwatcher_enable_recover },
    { "clear", script_healthwatcher_clear },
    { "balance", script_healthwatcher_balance },
    { "reset_opponents", script_healthwatcher_reset_opponents },
    { NULL, NULL }
};

int script_healthwatcher_new(lua_State* L, HealthWatcher healthwatcher) {
    return luascript_userdata_new(L, HEALTHWATCHER, healthwatcher);
}

static int script_healthwatcher_gc(lua_State* L) {
    return luascript_userdata_gc(L, HEALTHWATCHER);
}

static int script_healthwatcher_tostring(lua_State* L) {
    return luascript_userdata_tostring(L, HEALTHWATCHER);
}

void script_healthwatcher_register(lua_State* L) {
    luascript_register(L, HEALTHWATCHER, script_healthwatcher_gc, script_healthwatcher_tostring, HEALTHWATCHER_FUNCTIONS);
}

