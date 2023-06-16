using Engine.Animation;
using Engine.Externals.LuaInterop;
using Engine.Game;
using Engine.Sound;

namespace Engine.Externals.LuaScriptInterop;

public static class ExportsSongProgressbar {
    public const string SONGPROGRESSBAR = "SongProgressbar";


    static int script_songprogressbar_set_songplayer(LuaState L) {
        SongProgressbar songprogressbar = L.ReadUserdata<SongProgressbar>(SONGPROGRESSBAR);
        SongPlayer songplayer = L.ReadNullableUserdata<SongPlayer>(2, ExportsSongPlayer.SONGPLAYER);

        songprogressbar.SetSongplayer(songplayer);

        return 0;
    }

    static int script_songprogressbar_set_duration(LuaState L) {
        SongProgressbar songprogressbar = L.ReadUserdata<SongProgressbar>(SONGPROGRESSBAR);
        double duration = (double)L.luaL_checknumber(2);

        songprogressbar.SetDuration(duration);

        return 0;
    }

    static int script_songprogressbar_get_drawable(LuaState L) {
        SongProgressbar songprogressbar = L.ReadUserdata<SongProgressbar>(SONGPROGRESSBAR);

        Drawable ret = songprogressbar.GetDrawable();

        return ExportsDrawable.script_drawable_new(L, ret);
    }

    static int script_songprogressbar_set_visible(LuaState L) {
        SongProgressbar songprogressbar = L.ReadUserdata<SongProgressbar>(SONGPROGRESSBAR);
        bool visible = L.lua_toboolean(2);

        songprogressbar.SetVisible(visible);

        return 0;
    }

    static int script_songprogressbar_set_background_color(LuaState L) {
        SongProgressbar songprogressbar = L.ReadUserdata<SongProgressbar>(SONGPROGRESSBAR);
        float r = (float)L.luaL_checknumber(2);
        float g = (float)L.luaL_checknumber(3);
        float b = (float)L.luaL_checknumber(4);
        float a = (float)L.luaL_checknumber(5);

        songprogressbar.SetBackgroundColor(r, g, b, a);

        return 0;
    }

    static int script_songprogressbar_set_bar_back_color(LuaState L) {
        SongProgressbar songprogressbar = L.ReadUserdata<SongProgressbar>(SONGPROGRESSBAR);
        float r = (float)L.luaL_checknumber(2);
        float g = (float)L.luaL_checknumber(3);
        float b = (float)L.luaL_checknumber(4);
        float a = (float)L.luaL_checknumber(5);

        songprogressbar.SetBarBackColor(r, g, b, a);

        return 0;
    }

    static int script_songprogressbar_set_bar_progress_color(LuaState L) {
        SongProgressbar songprogressbar = L.ReadUserdata<SongProgressbar>(SONGPROGRESSBAR);
        float r = (float)L.luaL_checknumber(2);
        float g = (float)L.luaL_checknumber(3);
        float b = (float)L.luaL_checknumber(4);
        float a = (float)L.luaL_checknumber(5);

        songprogressbar.SetBarProgressColor(r, g, b, a);

        return 0;
    }

    static int script_songprogressbar_set_text_color(LuaState L) {
        SongProgressbar songprogressbar = L.ReadUserdata<SongProgressbar>(SONGPROGRESSBAR);
        float r = (float)L.luaL_checknumber(2);
        float g = (float)L.luaL_checknumber(3);
        float b = (float)L.luaL_checknumber(4);
        float a = (float)L.luaL_checknumber(5);

        songprogressbar.SetTextColor(r, g, b, a);

        return 0;
    }

    static int script_songprogressbar_hide_time(LuaState L) {
        SongProgressbar songprogressbar = L.ReadUserdata<SongProgressbar>(SONGPROGRESSBAR);
        bool hidden = L.lua_toboolean(2);

        songprogressbar.HideTime(hidden);

        return 0;
    }

    static int script_songprogressbar_show_elapsed(LuaState L) {
        SongProgressbar songprogressbar = L.ReadUserdata<SongProgressbar>(SONGPROGRESSBAR);
        bool elapsed_or_remain_time = L.lua_toboolean(2);

        songprogressbar.ShowElapsed(elapsed_or_remain_time);

        return 0;
    }

    static int script_songprogressbar_manual_update_enable(LuaState L) {
        SongProgressbar songprogressbar = L.ReadUserdata<SongProgressbar>(SONGPROGRESSBAR);
        bool enabled = L.lua_toboolean(2);

        songprogressbar.ManualUpdateEnable(enabled);

        return 0;
    }

    static int script_songprogressbar_manual_set_text(LuaState L) {
        SongProgressbar songprogressbar = L.ReadUserdata<SongProgressbar>(SONGPROGRESSBAR);
        string text = L.luaL_optstring(2, null);

        songprogressbar.ManualSetText(text);

        return 0;
    }

    static int script_songprogressbar_manual_set_position(LuaState L) {
        SongProgressbar songprogressbar = L.ReadUserdata<SongProgressbar>(SONGPROGRESSBAR);
        double elapsed = (double)L.luaL_checknumber(2);
        double duration = (double)L.luaL_checknumber(3);
        bool should_update_time_text = L.lua_toboolean(4);

        double ret = songprogressbar.ManualSetPosition(elapsed, duration, should_update_time_text);

        L.lua_pushnumber(ret);
        return 1;
    }

    static int script_songprogressbar_animation_set(LuaState L) {
        SongProgressbar songprogressbar = L.ReadUserdata<SongProgressbar>(SONGPROGRESSBAR);
        AnimSprite animsprite = L.ReadNullableUserdata<AnimSprite>(2, ExportsAnimSprite.ANIMSPRITE);

        songprogressbar.AnimationSet(animsprite);

        return 0;
    }

    static int script_songprogressbar_animation_restart(LuaState L) {
        SongProgressbar songprogressbar = L.ReadUserdata<SongProgressbar>(SONGPROGRESSBAR);

        songprogressbar.AnimationRestart();

        return 0;
    }

    static int script_songprogressbar_animation_end(LuaState L) {
        SongProgressbar songprogressbar = L.ReadUserdata<SongProgressbar>(SONGPROGRESSBAR);

        songprogressbar.AnimationEnd();

        return 0;
    }




    static readonly LuaTableFunction[] SONGPROGRESSBAR_FUNCTIONS = {
        new LuaTableFunction("set_songplayer", script_songprogressbar_set_songplayer),
        new LuaTableFunction("set_duration", script_songprogressbar_set_duration),
        new LuaTableFunction("get_drawable", script_songprogressbar_get_drawable),
        new LuaTableFunction("set_visible", script_songprogressbar_set_visible),
        new LuaTableFunction("set_background_color", script_songprogressbar_set_background_color),
        new LuaTableFunction("set_bar_back_color", script_songprogressbar_set_bar_back_color),
        new LuaTableFunction("set_bar_progress_color", script_songprogressbar_set_bar_progress_color),
        new LuaTableFunction("set_text_color", script_songprogressbar_set_text_color),
        new LuaTableFunction("hide_time", script_songprogressbar_hide_time),
        new LuaTableFunction("show_elapsed", script_songprogressbar_show_elapsed),
        new LuaTableFunction("manual_update_enable", script_songprogressbar_manual_update_enable),
        new LuaTableFunction("manual_set_text", script_songprogressbar_manual_set_text),
        new LuaTableFunction("manual_set_position", script_songprogressbar_manual_set_position),
        new LuaTableFunction("animation_set", script_songprogressbar_animation_set),
        new LuaTableFunction("animation_restart", script_songprogressbar_animation_restart),
        new LuaTableFunction("animation_end", script_songprogressbar_animation_end),
        new LuaTableFunction(null, null)
    };

    public static int script_songprogressbar_new(LuaState L, SongProgressbar songprogressbar) {
        return L.CreateUserdata(SONGPROGRESSBAR, songprogressbar);
    }

    static int script_songprogressbar_gc(LuaState L) {
        return L.GC_userdata(SONGPROGRESSBAR);
    }

    static int script_songprogressbar_tostring(LuaState L) {
        return L.ToString_userdata(SONGPROGRESSBAR);
    }

    private static readonly LuaCallback delegate_gc = script_songprogressbar_gc;
    private static readonly LuaCallback delegate_tostring = script_songprogressbar_tostring;

    public static void script_songprogressbar_register(ManagedLuaState L) {
        L.RegisterMetaTable(SONGPROGRESSBAR, delegate_gc, delegate_tostring, SONGPROGRESSBAR_FUNCTIONS);
    }


}

