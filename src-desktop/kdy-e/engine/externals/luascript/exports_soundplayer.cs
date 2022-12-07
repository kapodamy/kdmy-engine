using Engine.Externals.LuaInterop;
using Engine.Sound;

namespace Engine.Externals.LuaScriptInterop {

    public static class ExportsSoundPlayer {
        private const string SOUNDPLAYER = "SoundPlayer";


        static int script_soundplayer_play(LuaState L) {
            SoundPlayer soundplayer = L.ReadUserdata<SoundPlayer>(SOUNDPLAYER);

            soundplayer.Play();

            return 0;
        }

        static int script_soundplayer_pause(LuaState L) {
            SoundPlayer soundplayer = L.ReadUserdata<SoundPlayer>(SOUNDPLAYER);

            soundplayer.Pause();

            return 0;
        }

        static int script_soundplayer_stop(LuaState L) {
            SoundPlayer soundplayer = L.ReadUserdata<SoundPlayer>(SOUNDPLAYER);

            soundplayer.Stop();
            return 0;
        }

        static int script_soundplayer_loop_enable(LuaState L) {
            SoundPlayer soundplayer = L.ReadUserdata<SoundPlayer>(SOUNDPLAYER);

            bool enable = L.lua_toboolean(2);

            soundplayer.LoopEnable(enable);

            return 0;
        }

        static int script_soundplayer_fade(LuaState L) {
            SoundPlayer soundplayer = L.ReadUserdata<SoundPlayer>(SOUNDPLAYER);

            bool in_or_out = L.lua_toboolean(2);
            float duration = (float)L.luaL_checknumber(3);

            soundplayer.Fade(in_or_out, duration);

            return 0;
        }

        static int script_soundplayer_set_volume(LuaState L) {
            SoundPlayer soundplayer = L.ReadUserdata<SoundPlayer>(SOUNDPLAYER);

            float volume = (float)L.luaL_checknumber(2);

            soundplayer.SetVolume(volume);

            return 0;
        }

        static int script_soundplayer_set_mute(LuaState L) {
            SoundPlayer soundplayer = L.ReadUserdata<SoundPlayer>(SOUNDPLAYER);

            bool muted = L.lua_toboolean(2);

            soundplayer.SetMute(muted);

            return 0;
        }

        static int script_soundplayer_is_muted(LuaState L) {
            SoundPlayer soundplayer = L.ReadUserdata<SoundPlayer>(SOUNDPLAYER);

            bool ret = soundplayer.IsMuted();

            L.lua_pushboolean(ret);
            return 1;
        }

        static int script_soundplayer_is_playing(LuaState L) {
            SoundPlayer soundplayer = L.ReadUserdata<SoundPlayer>(SOUNDPLAYER);

            bool ret = soundplayer.IsPlaying();

            L.lua_pushboolean(ret);
            return 1;
        }

        static int script_soundplayer_get_duration(LuaState L) {
            SoundPlayer soundplayer = L.ReadUserdata<SoundPlayer>(SOUNDPLAYER);

            double ret = soundplayer.GetDuration();

            L.lua_pushnumber(ret);
            return 1;
        }

        static int script_soundplayer_get_position(LuaState L) {
            SoundPlayer soundplayer = L.ReadUserdata<SoundPlayer>(SOUNDPLAYER);

            double ret = soundplayer.GetPosition();

            L.lua_pushnumber(ret);
            return 1;
        }

        static int script_soundplayer_seek(LuaState L) {
            SoundPlayer soundplayer = L.ReadUserdata<SoundPlayer>(SOUNDPLAYER);

            double timestamp = L.luaL_checknumber(2);

            soundplayer.Seek(timestamp);

            return 0;
        }


        ////////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////

        static readonly LuaTableFunction[] SOUNDPLAYER_FUNCTIONS = {
            new LuaTableFunction("play", script_soundplayer_play),
            new LuaTableFunction("pause", script_soundplayer_pause),
            new LuaTableFunction("stop", script_soundplayer_stop),
            new LuaTableFunction("loop_enable", script_soundplayer_loop_enable),
            new LuaTableFunction("fade", script_soundplayer_fade),
            new LuaTableFunction("set_volume", script_soundplayer_set_volume),
            new LuaTableFunction("set_mute", script_soundplayer_set_mute),
            new LuaTableFunction("is_muted", script_soundplayer_is_muted),
            new LuaTableFunction("is_playing", script_soundplayer_is_playing),
            new LuaTableFunction("get_duration", script_soundplayer_get_duration),
            new LuaTableFunction("get_position", script_soundplayer_get_position),
            new LuaTableFunction("seek", script_soundplayer_seek),
            new LuaTableFunction(null, null)
        };


        internal static int script_soundplayer_new(LuaState L, SoundPlayer soundplayer) {
            return L.CreateUserdata(SOUNDPLAYER, soundplayer);
        }

        static int script_soundplayer_gc(LuaState L) {
            return L.GC_userdata(SOUNDPLAYER);
        }

        static int script_soundplayer_tostring(LuaState L) {
            return L.ToString_userdata(SOUNDPLAYER);
        }


        private static readonly LuaCallback delegate_gc = script_soundplayer_gc;
        private static readonly LuaCallback delegate_tostring = script_soundplayer_tostring;

        internal static void script_soundplayer_register(ManagedLuaState lua) {
            lua.RegisterMetaTable(SOUNDPLAYER, delegate_gc, delegate_tostring, SOUNDPLAYER_FUNCTIONS);
        }

    }
}
