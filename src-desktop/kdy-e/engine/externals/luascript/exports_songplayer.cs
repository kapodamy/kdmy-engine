using Engine.Externals.LuaInterop;
using Engine.Sound;

namespace Engine.Externals.LuaScriptInterop {

    public static class ExportsSongPlayer {
        private const string SONGPLAYER = "SongPlayer";

        static int script_songplayer_changesong(LuaState L) {
            SongPlayer songplayer = L.ReadUserdata<SongPlayer>(SONGPLAYER);

            string src = L.luaL_checkstring(2);
            bool prefer_no_copyright = L.luaL_checkboolean(3);

            bool ret = songplayer.ChangeSong(src, prefer_no_copyright);

            L.lua_pushboolean(ret);
            return 1;
        }

        static int script_songplayer_play(LuaState L) {
            SongPlayer songplayer = L.ReadUserdata<SongPlayer>(SONGPLAYER);

            SongPlayerInfo songinfo_dummy = new SongPlayerInfo();
            songplayer.Play(songinfo_dummy);

            return 0;
        }

        static int script_songplayer_pause(LuaState L) {
            SongPlayer songplayer = L.ReadUserdata<SongPlayer>(SONGPLAYER);

            songplayer.Pause();

            return 0;
        }

        static int script_songplayer_seek(LuaState L) {
            SongPlayer songplayer = L.ReadUserdata<SongPlayer>(SONGPLAYER);

            float timestamp = L.luaL_checkfloat(2);

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

            bool vocals_or_instrumental = L.luaL_checkboolean(2);
            bool muted = L.luaL_checkboolean(3);

            songplayer.MuteTrack(vocals_or_instrumental, muted);

            return 0;
        }

        static int script_songplayer_mute(LuaState L) {
            SongPlayer songplayer = L.ReadUserdata<SongPlayer>(SONGPLAYER);

            bool muted = L.luaL_checkboolean(2);

            songplayer.Mute(muted);

            return 0;
        }


        ////////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////

        static readonly LuaTableFunction[] SONGPLAYER_FUNCTIONS = {
            new LuaTableFunction() { name = "changesong", func = script_songplayer_changesong},
            new LuaTableFunction() { name = "play", func = script_songplayer_play},
            new LuaTableFunction() { name = "pause", func = script_songplayer_pause},
            new LuaTableFunction() { name = "seek", func = script_songplayer_seek},
            new LuaTableFunction() { name = "get_duration", func = script_songplayer_get_duration},
            new LuaTableFunction() { name = "is_completed", func = script_songplayer_is_completed},
            new LuaTableFunction() { name = "get_timestamp", func = script_songplayer_get_timestamp},
            new LuaTableFunction() { name = "mute_track", func = script_songplayer_mute_track},
            new LuaTableFunction() { name = "mute", func = script_songplayer_mute},
            new LuaTableFunction() { name = null, func = null}
        };


        internal static int script_songplayer_new(LuaState L, SongPlayer songplayer) {
            return L.CreateUserdata(SONGPLAYER, songplayer, true);
        }

        static int script_songplayer_gc(LuaState L) {
            return L.DestroyUserdata(SONGPLAYER);
        }

        static int script_songplayer_tostring(LuaState L) {
            L.lua_pushstring("[SongPlayer]");
            return 1;
        }


        private static readonly LuaCallback gc = script_songplayer_gc;
        private static readonly LuaCallback tostring = script_songplayer_tostring;


        internal static void register_songplayer(ManagedLuaState lua) {
            lua.RegisterMetaTable(
                SONGPLAYER,
                gc,
                tostring,
                SONGPLAYER_FUNCTIONS
            );
        }

    }
}
