using Engine.Externals.LuaInterop;
using Engine.Sound;

namespace Engine.Externals.LuaScriptInterop;

public static class ExportsSongPlayer {
    public const string SONGPLAYER = "SongPlayer";

    static int script_songplayer_changesong(LuaState L) {
        SongPlayer songplayer = L.ReadUserdata<SongPlayer>(SONGPLAYER);

        string src = L.luaL_checkstring(2);
        bool prefer_alternative = L.lua_toboolean(3);

        bool ret = songplayer.ChangeSong(src, prefer_alternative);

        L.lua_pushboolean(ret);
        return 1;
    }

    static int script_songplayer_play(LuaState L) {
        SongPlayer songplayer = L.ReadUserdata<SongPlayer>(SONGPLAYER);

        SongPlayerInfo songinfo_dummy = new SongPlayerInfo();
        songplayer.Play(ref songinfo_dummy);

        return 0;
    }

    static int script_songplayer_pause(LuaState L) {
        SongPlayer songplayer = L.ReadUserdata<SongPlayer>(SONGPLAYER);

        songplayer.Pause();

        return 0;
    }

    static int script_songplayer_seek(LuaState L) {
        SongPlayer songplayer = L.ReadUserdata<SongPlayer>(SONGPLAYER);

        double timestamp = L.luaL_checknumber(2);

        songplayer.Seek(timestamp);

        return 0;
    }

    static int script_songplayer_get_duration(LuaState L) {
        SongPlayer songplayer = L.ReadUserdata<SongPlayer>(SONGPLAYER);

        double duration = songplayer.GetDuration();

        L.lua_pushnumber(duration);
        return 1;
    }

    static int script_songplayer_is_completed(LuaState L) {
        SongPlayer songplayer = L.ReadUserdata<SongPlayer>(SONGPLAYER);

        bool ret = songplayer.IsCompleted();

        L.lua_pushboolean(ret);
        return 1;
    }

    static int script_songplayer_get_timestamp(LuaState L) {
        SongPlayer songplayer = L.ReadUserdata<SongPlayer>(SONGPLAYER);

        double timestamp = songplayer.GetTimestamp();

        L.lua_pushnumber(timestamp);
        return 1;
    }

    static int script_songplayer_mute_track(LuaState L) {
        SongPlayer songplayer = L.ReadUserdata<SongPlayer>(SONGPLAYER);

        bool vocals_or_instrumental = L.lua_toboolean(2);
        bool muted = L.lua_toboolean(3);

        songplayer.MuteTrack(vocals_or_instrumental, muted);

        return 0;
    }

    static int script_songplayer_mute(LuaState L) {
        SongPlayer songplayer = L.ReadUserdata<SongPlayer>(SONGPLAYER);

        bool muted = L.lua_toboolean(2);

        songplayer.Mute(muted);

        return 0;
    }

    static int script_songplayer_set_volume_track(LuaState L) {
        SongPlayer songplayer = L.ReadUserdata<SongPlayer>(SONGPLAYER);
        bool vocals_or_instrumental = L.lua_toboolean(2);
        float volume = (float)L.luaL_checknumber(3);

        songplayer.SetVolumeTrack(vocals_or_instrumental, volume);

        return 0;
    }

    static int script_songplayer_set_volume(LuaState L) {
        SongPlayer songplayer = L.ReadUserdata<SongPlayer>(SONGPLAYER);
        float volume = (float)L.luaL_checknumber(2);

        songplayer.SetVolume(volume);

        return 0;
    }


    ////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////

    static readonly LuaTableFunction[] SONGPLAYER_FUNCTIONS = {
        new LuaTableFunction("changesong", script_songplayer_changesong),
        new LuaTableFunction("play", script_songplayer_play),
        new LuaTableFunction("pause", script_songplayer_pause),
        new LuaTableFunction("seek", script_songplayer_seek),
        new LuaTableFunction("get_duration", script_songplayer_get_duration),
        new LuaTableFunction("is_completed", script_songplayer_is_completed),
        new LuaTableFunction("get_timestamp", script_songplayer_get_timestamp),
        new LuaTableFunction("mute_track", script_songplayer_mute_track),
        new LuaTableFunction("mute", script_songplayer_mute),
        new LuaTableFunction("set_volume_track", script_songplayer_set_volume_track),
        new LuaTableFunction("set_volume", script_songplayer_set_volume),
        new LuaTableFunction(null, null)
    };


    internal static int script_songplayer_new(LuaState L, SongPlayer songplayer) {
        return L.CreateUserdata(SONGPLAYER, songplayer);
    }

    static int script_songplayer_gc(LuaState L) {
        return L.GC_userdata(SONGPLAYER);
    }

    static int script_songplayer_tostring(LuaState L) {
        return L.ToString_userdata(SONGPLAYER);
    }


    private static readonly LuaCallback delegate_gc = script_songplayer_gc;
    private static readonly LuaCallback delegate_tostring = script_songplayer_tostring;


    internal static void script_songplayer_register(ManagedLuaState lua) {
        lua.RegisterMetaTable(SONGPLAYER, delegate_gc, delegate_tostring, SONGPLAYER_FUNCTIONS);
    }

}
