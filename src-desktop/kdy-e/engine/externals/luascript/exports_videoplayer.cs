using Engine.Externals.LuaInterop;
using Engine.Image;
using Engine.Video;

namespace Engine.Externals.LuaScriptInterop;

public static class ExportsVideoPlayer {

    public const string VIDEOPLAYER = "VideoPlayer";

    static int script_videoplayer_get_sprite(LuaState L) {
        VideoPlayer videoplayer = L.ReadUserdata<VideoPlayer>(VIDEOPLAYER);

        Sprite ret = videoplayer.GetSprite();

        return ExportsSprite.script_sprite_new(L, ret);
    }

    static int script_videoplayer_replay(LuaState L) {
        VideoPlayer videoplayer = L.ReadUserdata<VideoPlayer>(VIDEOPLAYER);

        videoplayer.Replay();
        return 0;
    }

    static int script_videoplayer_play(LuaState L) {
        VideoPlayer videoplayer = L.ReadUserdata<VideoPlayer>(VIDEOPLAYER);

        videoplayer.Play();
        return 0;
    }

    static int script_videoplayer_pause(LuaState L) {
        VideoPlayer videoplayer = L.ReadUserdata<VideoPlayer>(VIDEOPLAYER);

        videoplayer.Pause();
        return 0;
    }

    static int script_videoplayer_stop(LuaState L) {
        VideoPlayer videoplayer = L.ReadUserdata<VideoPlayer>(VIDEOPLAYER);
        videoplayer.Stop();

        return 0;
    }

    static int script_videoplayer_loop_enable(LuaState L) {
        VideoPlayer videoplayer = L.ReadUserdata<VideoPlayer>(VIDEOPLAYER);
        bool enable = L.lua_toboolean(2);
        videoplayer.LoopEnable(enable);

        return 0;
    }

    static int script_videoplayer_fade_audio(LuaState L) {
        VideoPlayer videoplayer = L.ReadUserdata<VideoPlayer>(VIDEOPLAYER);
        bool in_or_out = L.lua_toboolean(2);
        float duration = (float)L.luaL_checknumber(3);

        videoplayer.FadeAudio(in_or_out, duration);

        return 0;
    }

    static int script_videoplayer_set_volume(LuaState L) {
        VideoPlayer videoplayer = L.ReadUserdata<VideoPlayer>(VIDEOPLAYER);
        float volume = (float)L.luaL_checknumber(2);
        videoplayer.SetVolume(volume);

        return 0;
    }

    static int script_videoplayer_set_mute(LuaState L) {
        VideoPlayer videoplayer = L.ReadUserdata<VideoPlayer>(VIDEOPLAYER);
        bool muted = L.lua_toboolean(2);
        videoplayer.SetMute(muted);

        return 0;
    }

    static int script_videoplayer_seek(LuaState L) {
        VideoPlayer videoplayer = L.ReadUserdata<VideoPlayer>(VIDEOPLAYER);
        double timestamp = (double)L.luaL_checknumber(2);
        videoplayer.Seek(timestamp);

        return 0;
    }

    static int script_videoplayer_is_muted(LuaState L) {
        VideoPlayer videoplayer = L.ReadUserdata<VideoPlayer>(VIDEOPLAYER);

        bool ret = videoplayer.IsMuted();

        L.lua_pushboolean(ret);
        return 1;
    }

    static int script_videoplayer_is_playing(LuaState L) {
        VideoPlayer videoplayer = L.ReadUserdata<VideoPlayer>(VIDEOPLAYER);

        bool ret = videoplayer.IsPlaying();

        L.lua_pushboolean(ret);
        return 1;
    }

    static int script_videoplayer_get_duration(LuaState L) {
        VideoPlayer videoplayer = L.ReadUserdata<VideoPlayer>(VIDEOPLAYER);

        double ret = videoplayer.GetDuration();

        L.lua_pushnumber(ret);
        return 1;
    }

    static int script_videoplayer_get_position(LuaState L) {
        VideoPlayer videoplayer = L.ReadUserdata<VideoPlayer>(VIDEOPLAYER);

        double ret = videoplayer.GetPosition();

        L.lua_pushnumber(ret);
        return 1;
    }

    static int script_videoplayer_has_ended(LuaState L) {
        VideoPlayer videoplayer = L.ReadUserdata<VideoPlayer>(VIDEOPLAYER);

        bool ret = videoplayer.HasEnded();
        L.lua_pushboolean(ret);
        return 1;
    }

    static int script_videoplayer_has_video_track(LuaState L) {
        VideoPlayer videoplayer = L.ReadUserdata<VideoPlayer>(VIDEOPLAYER);

        bool ret = videoplayer.HasVideoTrack();
        L.lua_pushboolean(ret);
        return 1;
    }

    static int script_videoplayer_has_audio_track(LuaState L) {
        VideoPlayer videoplayer = L.ReadUserdata<VideoPlayer>(VIDEOPLAYER);

        bool ret = videoplayer.HasAudioTrack();

        L.lua_pushboolean(ret);
        return 1;
    }




    static readonly LuaTableFunction[] VIDEOPLAYER_FUNCTIONS = {
        new LuaTableFunction("get_sprite", script_videoplayer_get_sprite ),
        new LuaTableFunction("replay", script_videoplayer_replay ),
        new LuaTableFunction("play", script_videoplayer_play ),
        new LuaTableFunction("pause", script_videoplayer_pause ),
        new LuaTableFunction("stop", script_videoplayer_stop ),
        new LuaTableFunction("loop_enable", script_videoplayer_loop_enable ),
        new LuaTableFunction("fade_audio", script_videoplayer_fade_audio ),
        new LuaTableFunction("set_volume", script_videoplayer_set_volume ),
        new LuaTableFunction("set_mute", script_videoplayer_set_mute ),
        new LuaTableFunction("seek", script_videoplayer_seek ),
        new LuaTableFunction("is_muted", script_videoplayer_is_muted ),
        new LuaTableFunction("is_playing", script_videoplayer_is_playing ),
        new LuaTableFunction("get_duration", script_videoplayer_get_duration ),
        new LuaTableFunction("get_position", script_videoplayer_get_position ),
        new LuaTableFunction("has_ended", script_videoplayer_has_ended ),
        new LuaTableFunction("has_video_track", script_videoplayer_has_video_track ),
        new LuaTableFunction("has_audio_track", script_videoplayer_has_audio_track ),
        new LuaTableFunction(null, null)
    };

    public static int script_videoplayer_new(LuaState L, VideoPlayer videoplayer) {
        return L.CreateUserdata(VIDEOPLAYER, videoplayer);
    }

    static int script_videoplayer_gc(LuaState L) {
        return L.GC_userdata(VIDEOPLAYER);
    }

    static int script_videoplayer_tostring(LuaState L) {
        return L.ToString_userdata(VIDEOPLAYER);
    }

    private static readonly LuaCallback delegate_gc = script_videoplayer_gc;
    private static readonly LuaCallback delegate_tostring = script_videoplayer_tostring;

    public static void script_videoplayer_register(ManagedLuaState L) {
        L.RegisterMetaTable(VIDEOPLAYER, delegate_gc, delegate_tostring, VIDEOPLAYER_FUNCTIONS);
    }

}

