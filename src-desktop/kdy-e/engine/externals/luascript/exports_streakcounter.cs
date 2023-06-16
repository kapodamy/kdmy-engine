using Engine.Animation;
using Engine.Externals.LuaInterop;
using Engine.Game;

namespace Engine.Externals.LuaScriptInterop;

public static class ExportsStreakCounter {
    public const string STREAKCOUNTER = "StreakCounter";

    static int script_streakcounter_reset(LuaState L) {
        StreakCounter streakcounter = L.ReadUserdata<StreakCounter>(STREAKCOUNTER);

        streakcounter.Reset();

        return 0;
    }

    static int script_streakcounter_hide_combo_sprite(LuaState L) {
        StreakCounter streakcounter = L.ReadUserdata<StreakCounter>(STREAKCOUNTER);
        bool hide = L.lua_toboolean(2);

        streakcounter.HideComboSprite(hide);

        return 0;
    }

    static int script_streakcounter_set_combo_draw_location(LuaState L) {
        StreakCounter streakcounter = L.ReadUserdata<StreakCounter>(STREAKCOUNTER);
        float x = (float)L.luaL_checknumber(2);
        float y = (float)L.luaL_checknumber(3);

        streakcounter.SetComboDrawLocation(x, y);

        return 0;
    }

    static int script_streakcounter_state_add(LuaState L) {
        StreakCounter streakcounter = L.ReadUserdata<StreakCounter>(STREAKCOUNTER);
        ModelHolder combo_modelholder = L.ReadNullableUserdata<ModelHolder>(2, ExportsModelHolder.MODELHOLDER);
        ModelHolder number_modelholder = L.ReadNullableUserdata<ModelHolder>(3, ExportsModelHolder.MODELHOLDER);
        string state_name = L.luaL_optstring(4, null);

        int ret = streakcounter.StateAdd(combo_modelholder, number_modelholder, state_name);

        L.lua_pushinteger(ret);
        return 1;
    }

    static int script_streakcounter_state_toggle(LuaState L) {
        StreakCounter streakcounter = L.ReadUserdata<StreakCounter>(STREAKCOUNTER);
        string state_name = L.luaL_optstring(2, null);

        bool ret = streakcounter.StateToggle(state_name);

        L.lua_pushboolean(ret);
        return 1;
    }

    static int script_streakcounter_set_alpha(LuaState L) {
        StreakCounter streakcounter = L.ReadUserdata<StreakCounter>(STREAKCOUNTER);
        float alpha = (float)L.luaL_checknumber(2);

        streakcounter.SetAlpha(alpha);

        return 0;
    }

    static int script_streakcounter_get_drawable(LuaState L) {
        StreakCounter streakcounter = L.ReadUserdata<StreakCounter>(STREAKCOUNTER);

        Drawable ret = streakcounter.GetDrawable();

        return ExportsDrawable.script_drawable_new(L, ret);
    }

    static int script_streakcounter_animation_set(LuaState L) {
        StreakCounter streakcounter = L.ReadUserdata<StreakCounter>(STREAKCOUNTER);
        AnimSprite animsprite = L.ReadNullableUserdata<AnimSprite>(2, ExportsAnimSprite.ANIMSPRITE);

        streakcounter.AnimationSet(animsprite);

        return 0;
    }

    static int script_streakcounter_animation_restart(LuaState L) {
        StreakCounter streakcounter = L.ReadUserdata<StreakCounter>(STREAKCOUNTER);

        streakcounter.AnimationRestart();

        return 0;
    }

    static int script_streakcounter_animation_end(LuaState L) {
        StreakCounter streakcounter = L.ReadUserdata<StreakCounter>(STREAKCOUNTER);

        streakcounter.AnimationEnd();

        return 0;
    }




    static readonly LuaTableFunction[] STREAKCOUNTER_FUNCTIONS = {
        new LuaTableFunction("reset", script_streakcounter_reset),
        new LuaTableFunction("hide_combo_sprite", script_streakcounter_hide_combo_sprite),
        new LuaTableFunction("set_combo_draw_location", script_streakcounter_set_combo_draw_location),
        new LuaTableFunction("state_add", script_streakcounter_state_add),
        new LuaTableFunction("state_toggle", script_streakcounter_state_toggle),
        new LuaTableFunction("set_alpha", script_streakcounter_set_alpha),
        new LuaTableFunction("get_drawable", script_streakcounter_get_drawable),
        new LuaTableFunction("animation_set", script_streakcounter_animation_set),
        new LuaTableFunction("animation_restart", script_streakcounter_animation_restart),
        new LuaTableFunction("animation_end", script_streakcounter_animation_end),
        new LuaTableFunction(null, null)
    };

    public static int script_streakcounter_new(LuaState L, StreakCounter streakcounter) {
        return L.CreateUserdata(STREAKCOUNTER, streakcounter);
    }

    static int script_streakcounter_gc(LuaState L) {
        return L.GC_userdata(STREAKCOUNTER);
    }

    static int script_streakcounter_tostring(LuaState L) {
        return L.ToString_userdata(STREAKCOUNTER);
    }

    private static readonly LuaCallback delegate_gc = script_streakcounter_gc;
    private static readonly LuaCallback delegate_tostring = script_streakcounter_tostring;

    public static void script_streakcounter_register(ManagedLuaState L) {
        L.RegisterMetaTable(STREAKCOUNTER, delegate_gc, delegate_tostring, STREAKCOUNTER_FUNCTIONS);
    }


}

