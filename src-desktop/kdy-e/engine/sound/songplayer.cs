using System;
using Engine.Externals.LuaScriptInterop;
using Engine.Platform;
using Engine.Utils;
using KallistiOS.THD;

namespace Engine.Sound {

    public class SongPlayerInfo {
        public double timestamp;
        public bool completed;
    }

    public class SongPlayer {
        private const string SUFFIX_INSTRUMENTAL = "-inst";
        private const string SUFFIX_VOICES = "-voices";
        private const string SUFFIX_ALTERNATIVE = "-alt";
        private const string NAME_INSTRUMENTAL = "Inst";
        private const string NAME_VOICES = "Voices";
        public const char TRACKS_SEPARATOR = '|';

        private static readonly SongPlayer SILENCE = new SongPlayer();

        private bool paused = true;
        private int playbacks_size = 0;
        private SoundPlayer[] playbacks = null;
        private int index_instrumental = -1;
        private int index_voices = -1;

        private SongPlayer() { }

        public static SongPlayer Init(string src, bool prefer_alternative) {
            string path_voices, path_instrumental;
            bool is_not_splitted = HelperGetTracks(
                src, prefer_alternative, out path_voices, out path_instrumental
            );

            if (path_voices == null && path_instrumental == null && !is_not_splitted) {
                Console.Error.WriteLine("[ERROR] songplayer_init() fallback failed, missing file: " + src);
                Console.Error.WriteLine("[ERROR] songplayer_init() cannot load any file, there will only be silence.");

                return SongPlayer.SILENCE;
            }

            SongPlayer songplayer;

            if (is_not_splitted) {
                songplayer = SongPlayer.Init2(true, src, null);
            } else {
                songplayer = SongPlayer.Init2(false, path_voices, path_instrumental);
                //free(path_voices);
                //free(path_instrumental);
            }

            return songplayer;
        }

        public static SongPlayer Init2(bool is_not_splitted, string path_voices, string path_instrumental) {

            SongPlayer songplayer = new SongPlayer();

            if (is_not_splitted) {
                SoundPlayer player = SoundPlayer.Init(path_voices ?? path_instrumental);
                if (player != null) {
                    songplayer.playbacks = new SoundPlayer[1];
                    songplayer.playbacks_size = 1;
                    songplayer.playbacks[0] = player;
                    songplayer.index_instrumental = 0;
                    songplayer.index_voices = 0;
                }
            } else {
                SoundPlayer player_voices = null;
                SoundPlayer player_instrumentals = null;

                if (path_voices != null) {
                    player_voices = SoundPlayer.Init(path_voices);
                }

                if (path_instrumental != null) {
                    player_instrumentals = SoundPlayer.Init(path_instrumental);
                }

                if (player_voices != null) {
                    songplayer.index_voices = songplayer.playbacks_size;
                    songplayer.playbacks_size++;
                }
                if (player_instrumentals != null) {
                    songplayer.index_instrumental = songplayer.playbacks_size;
                    songplayer.playbacks_size++;
                }

                int index = 0;
                songplayer.playbacks = new SoundPlayer[songplayer.playbacks_size];

                if (player_voices != null) songplayer.playbacks[index++] = player_voices;
                if (player_instrumentals != null) songplayer.playbacks[index++] = player_instrumentals;

            }

            return songplayer;
        }

        public void Destroy() {
            Luascript.DropShared(this);
            if (this == SongPlayer.SILENCE) return;

            for (int i = 0 ; i < this.playbacks_size ; i++) {
                this.playbacks[i].Destroy();
            }

            //free(this.playbacks);
            //free(songplayer);
        }

        public double Play(SongPlayerInfo songinfo) {
            if (this.playbacks_size < 1 || !this.paused) return 0.0;

            double lowest_timestamp = double.PositiveInfinity;
            int reference_index = 0;
            double timestamp;

            for (int i = 0 ; i < this.playbacks_size ; i++) {
                timestamp = this.playbacks[i].GetDuration();
                if (timestamp < lowest_timestamp) {
                    lowest_timestamp = timestamp;
                    reference_index = i;
                }
            }

            int completed = 0;
            bool playback_success = false;
            for (int i = 0 ; i < this.playbacks_size ; i++) {
                if (this.playbacks[i].HasEnded()) completed++;
                this.playbacks[i].Play();
            }

            // wait until the first audio samples are played

            SoundPlayer reference = this.playbacks[reference_index];
            timestamp = 0.0;

            while (playback_success && timestamp == reference.GetDuration()) thd.pass();

            this.paused = false;
            double current_timestamp = reference.GetPosition();

            songinfo.timestamp = current_timestamp * this.playbacks_size;
            songinfo.completed = completed >= this.playbacks_size;

            return current_timestamp - timestamp;
        }

        public void Pause() {
            if (this.playbacks_size < 1 || this.paused) return;
            for (int i = 0 ; i < this.playbacks_size ; i++) this.playbacks[i].Pause();
            this.paused = true;
        }

        public void Seek(double timestamp) {

            for (int i = 0 ; i < this.playbacks_size ; i++)
                this.playbacks[i].Seek(timestamp);
        }

        public void Poll(SongPlayerInfo songinfo) {
            //
            // IMPORTANT: in the C version poll every stream (similar to animate() ) in
            // order to send audio samples to the AICA.
            //
            int ended = 0;
            double timestamp = 0;

            for (int i = 0 ; i < this.playbacks_size ; i++) {
                double position = this.playbacks[i].GetPosition();
                if (this.playbacks[i].HasEnded()) ended++;
                timestamp += position;
            }

            songinfo.timestamp = timestamp / this.playbacks_size;
            songinfo.completed = ended >= this.playbacks_size;
        }

        public double GetDuration() {
            double duration = 0;

            for (int i = 0 ; i < this.playbacks_size ; i++) {
                /**@type {HTMLAudioElement} */
                double playback_duration = this.playbacks[i].GetDuration();

                if (playback_duration < 0) continue;
                if (playback_duration > duration) duration = playback_duration;
            }

            return duration;
        }

        public bool ChangeSong(string src, bool prefer_no_copyright) {
            Console.Error.WriteLine("songplayer_changesong() not implemented");
            return false;
        }

        public bool IsCompleted() {
            int completed = 0;
            for (int i = 0 ; i < this.playbacks_size ; i++) {
                if (this.playbacks[i].HasEnded()) completed++;
            }

            return completed >= this.playbacks_size;
        }

        public double GetTimestamp() {
            double timestamp = 0;
            for (int i = 0 ; i < this.playbacks_size ; i++) {
                timestamp += this.playbacks[i].GetPosition();
            }

            return timestamp / this.playbacks_size;
        }

        public void MuteTrack(bool vocals_or_instrumental, bool muted) {
            if (this.playbacks_size < 1) return;
            int target = vocals_or_instrumental ? this.index_voices : this.index_instrumental;
            if (target < 0) return;
            this.playbacks[target].SetMute(muted);
        }

        public void Mute(bool muted) {
            if (this.playbacks_size < 1) return;
            for (int i = 0 ; i < this.playbacks_size ; i++) this.playbacks[i].SetMute(muted);
        }

        public void SetVolume(float volume) {
            if (this.playbacks_size < 1) return;
            for (int i = 0 ; i < this.playbacks_size ; i++) this.playbacks[i].SetVolume(volume);
        }

        public void SetVolumeTrack(bool vocals_or_instrumental, float volume) {
            if (this.playbacks_size < 1) return;
            int target = vocals_or_instrumental ? this.index_voices : this.index_instrumental;
            if (target < 0) return;
            this.playbacks[target].SetVolume(volume);
        }


        public static bool HelperGetTracks(string src, bool prefer_alternative, out string path_voices, out string path_instrumental) {
            path_voices = null;
            path_instrumental = null;
            bool is_not_splitted = false;
            int parts = src.IndexOf('|');

            if (String.IsNullOrWhiteSpace(src)) return is_not_splitted;

            int separator_index = src.IndexOf(TRACKS_SEPARATOR);
            if (separator_index >= 0) {
                // parse "voices.ogg|inst.ogg" format
                path_voices = SongPlayer.InternalSeparePaths(0, separator_index, src, true);
                path_instrumental = SongPlayer.InternalSeparePaths(separator_index + 1, src.Length, src, true);

                return is_not_splitted;
            }

            int dot_index = src.LastIndexOf('.');
            if (dot_index < 0) throw new Exception("missing file extension : " + src);


            if (prefer_alternative) {
                src = src.Insert(dot_index, SongPlayer.SUFFIX_ALTERNATIVE);
            }

            if (FS.FileExists(src)) {
                is_not_splitted = true;
            } else {
                // check if the song is splited in voices and instrumental
                string voices, instrumental;

                if (dot_index == 0 || src[dot_index - 1] == FS.CHAR_SEPARATOR) {
                    if (dot_index > 0) {
                        string folder_path = src.SubstringKDY(0, dot_index - 1);
                        if (!FS.FolderExists(folder_path)) {
                            Console.Error.WriteLine("[WARN] songplayer_init() folder not found: " + src);
                        }
                        //free(folder_path);
                    }
                    // src points to a folder, load files with names "Voices.ogg" and "Inst.ogg"
                    voices = src.Insert(dot_index, SongPlayer.NAME_VOICES);
                    instrumental = src.Insert(dot_index, SongPlayer.NAME_INSTRUMENTAL);
                } else {
                    // absolute filenames "songame-voices.ogg" and "songname-inst.ogg"
                    voices = src.Insert(dot_index, SongPlayer.SUFFIX_VOICES);
                    instrumental = src.Insert(dot_index, SongPlayer.SUFFIX_INSTRUMENTAL);
                }

                if (FS.FileExists(voices)) {
                    path_voices = voices;
                } else {
                    Console.Error.WriteLine("[WARN] songplayer_init() missing voices: " + voices);
                    //free(voices);
                }

                if (FS.FileExists(instrumental)) {
                    path_instrumental = instrumental;
                } else {
                    Console.Error.WriteLine("[WARN] songplayer_init() missing instrumental: " + instrumental);
                    //free(instrumental);
                }
            }

            if (path_instrumental == null && path_voices == null && !is_not_splitted) {
                //if (prefer_alternative) free(src);
            }

            return is_not_splitted;
        }

        public static string HelperGetTracksFullPath(string src) {
            int separator_index = src.IndexOf(SongPlayer.TRACKS_SEPARATOR);

            if (separator_index < 0) {
                string path = FS.GetFullPath(src);
                //free(src);
                return path;
            }

            // parse "voices.ogg|inst.ogg" format
            string path_voices = SongPlayer.InternalSeparePaths(0, separator_index, src, false);
            string path_instrumental = SongPlayer.InternalSeparePaths(separator_index + 1, src.Length, src, false);

            //free(src);

            if (path_voices != null) {
                string tmp = FS.GetFullPath(path_voices);
                //free(path_voices);
                path_voices = tmp;
            }

            if (path_instrumental != null) {
                string tmp = FS.GetFullPath(path_instrumental);
                //free(path_instrumental);
                path_instrumental = tmp;
            }

            src = StringUtils.Concat(path_voices, SongPlayer.TRACKS_SEPARATOR.ToString(), path_instrumental);

            //free(path_voices);
            //free(path_instrumental);
            return src;
        }

        private static string InternalSeparePaths(int start, int end, string src, bool check_if_exists) {
            if (start == end) return null;

            string path = src.SubstringKDY(start, end);

            if (!check_if_exists || FS.FileExists(path)) return path;

            Console.Error.WriteLine("[WARN] songplayer_internal_separe_paths() missing: " + path);

            //free(path);
            return null;
        }

    }
}
