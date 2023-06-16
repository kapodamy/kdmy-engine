using Engine.Externals.LuaInterop;
using Engine.Game;

namespace Engine.Externals.LuaScriptInterop;

public static class ExportsHealthWatcher {
    public const string HEALTHWATCHER = "HealthWatcher";
    static int script_healthwatcher_add_opponent(LuaState L) {
        HealthWatcher healthwatcher = L.ReadUserdata<HealthWatcher>(HEALTHWATCHER);
        PlayerStats playerstats = L.ReadNullableUserdata<PlayerStats>(2, ExportsPlayerStats.PLAYERSTATS);
        bool can_recover = L.lua_toboolean(3);
        bool can_die = L.lua_toboolean(4);

        bool ret = healthwatcher.AddOpponent(playerstats, can_recover, can_die);

        L.lua_pushboolean(ret);
        return 1;
    }

    static int script_healthwatcher_add_player(LuaState L) {
        HealthWatcher healthwatcher = L.ReadUserdata<HealthWatcher>(HEALTHWATCHER);
        PlayerStats playerstats = L.ReadNullableUserdata<PlayerStats>(2, ExportsPlayerStats.PLAYERSTATS);
        bool can_recover = L.lua_toboolean(3);
        bool can_die = L.lua_toboolean(4);

        bool ret = healthwatcher.AddPlayer(playerstats, can_recover, can_die);

        L.lua_pushboolean(ret);
        return 1;
    }

    static int script_healthwatcher_has_deads(LuaState L) {
        HealthWatcher healthwatcher = L.ReadUserdata<HealthWatcher>(HEALTHWATCHER);
        bool in_players_or_opponents = L.lua_toboolean(2);

        int ret = healthwatcher.HasDeads(in_players_or_opponents);

        L.lua_pushinteger(ret);
        return 1;
    }

    static int script_healthwatcher_enable_dead(LuaState L) {
        HealthWatcher healthwatcher = L.ReadUserdata<HealthWatcher>(HEALTHWATCHER);
        PlayerStats playerstats = L.ReadNullableUserdata<PlayerStats>(2, ExportsPlayerStats.PLAYERSTATS);
        bool can_die = L.lua_toboolean(3);

        bool ret = healthwatcher.EnableDead(playerstats, can_die);

        L.lua_pushboolean(ret);
        return 1;
    }

    static int script_healthwatcher_enable_recover(LuaState L) {
        HealthWatcher healthwatcher = L.ReadUserdata<HealthWatcher>(HEALTHWATCHER);
        PlayerStats playerstats = L.ReadNullableUserdata<PlayerStats>(2, ExportsPlayerStats.PLAYERSTATS);
        bool can_recover = L.lua_toboolean(3);

        bool ret = healthwatcher.EnableRecover(playerstats, can_recover);

        L.lua_pushboolean(ret);
        return 1;
    }

    static int script_healthwatcher_clear(LuaState L) {
        HealthWatcher healthwatcher = L.ReadUserdata<HealthWatcher>(HEALTHWATCHER);

        healthwatcher.Clear();

        return 0;
    }

    static int script_healthwatcher_balance(LuaState L) {
        HealthWatcher healthwatcher = L.ReadUserdata<HealthWatcher>(HEALTHWATCHER);
        HealthBar healthbar = L.ReadNullableUserdata<HealthBar>(2, ExportsHealthBar.HEALTHBAR);

        healthwatcher.Balance(healthbar);

        return 0;
    }

    static int script_healthwatcher_reset_opponents(LuaState L) {
        HealthWatcher healthwatcher = L.ReadUserdata<HealthWatcher>(HEALTHWATCHER);

        healthwatcher.ResetOpponents();

        return 0;
    }




    static readonly LuaTableFunction[] HEALTHWATCHER_FUNCTIONS = {
        new LuaTableFunction("add_opponent", script_healthwatcher_add_opponent),
        new LuaTableFunction("add_player", script_healthwatcher_add_player),
        new LuaTableFunction("has_deads", script_healthwatcher_has_deads),
        new LuaTableFunction("enable_dead", script_healthwatcher_enable_dead),
        new LuaTableFunction("enable_recover", script_healthwatcher_enable_recover),
        new LuaTableFunction("clear", script_healthwatcher_clear),
        new LuaTableFunction("balance", script_healthwatcher_balance),
        new LuaTableFunction("reset_opponents", script_healthwatcher_reset_opponents),
        new LuaTableFunction(null, null)
    };

    public static int script_healthwatcher_new(LuaState L, HealthWatcher healthwatcher) {
        return L.CreateUserdata(HEALTHWATCHER, healthwatcher);
    }

    static int script_healthwatcher_gc(LuaState L) {
        return L.GC_userdata(HEALTHWATCHER);
    }

    static int script_healthwatcher_tostring(LuaState L) {
        return L.ToString_userdata(HEALTHWATCHER);
    }

    private static readonly LuaCallback delegate_gc = script_healthwatcher_gc;
    private static readonly LuaCallback delegate_tostring = script_healthwatcher_tostring;

    public static void script_healthwatcher_register(ManagedLuaState L) {
        L.RegisterMetaTable(HEALTHWATCHER, delegate_gc, delegate_tostring, HEALTHWATCHER_FUNCTIONS);
    }

}

