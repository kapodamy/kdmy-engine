using System;
using Engine.Platform;
using KallistiOS;
using KallistiOS.THD;

namespace Engine.Sound {

    public class SongPlayerInfo {
        public double timestamp;
        public bool completed;
    }

    public class SongPlayer {
        private const string SUFFIX_INSTRUMENTAL = "-inst";
        private const string SUFFIX_VOICES = "-voices";
        private const string SUFFIX_NOCOPYRIGHT = "-nocopy";

        private static readonly SongPlayer SILENCE = new SongPlayer();

        private bool paused = true;
        private int playbacks_size = 0;
        private SoundPlayer[] playbacks = null;
        private int index_instrumental = -1;
        private int index_voices = -1;

        private SongPlayer() { }

        public static SongPlayer Init(string src, bool prefer_no_copyright) {
            if (String.IsNullOrWhiteSpace(src)) return SongPlayer.SILENCE;

            int dot_index = src.LastIndexOf('.');
            if (dot_index < 1) throw new Exception("missing file extension : " + src);

            bool is_not_splitted = false;
            string path_voices = null;
            string path_instrumental = null;

            if (prefer_no_copyright) {
                src = src.Insert(dot_index, SongPlayer.SUFFIX_NOCOPYRIGHT);
            }

            if (FS.FileExists(src)) {
                is_not_splitted = true;
            } else {
                // check if the song is splited in voices and instrumental
                string voices = src.Insert(dot_index, SongPlayer.SUFFIX_VOICES);
                string instrumental = src.Insert(dot_index, SongPlayer.SUFFIX_INSTRUMENTAL);

                if (FS.FileExists(voices)) {
                    path_voices = voices;
                } else {
                    Console.Error.WriteLine("songplayer_init() missing voices: " + voices);
                    //free(voices);
                }

                if (FS.FileExists(instrumental)) {
                    path_instrumental = instrumental;
                } else {
                    Console.Error.WriteLine("songplayer_init() missing instrumental: " + instrumental);
                    //free(instrumental);
                }
            }

            if (path_instrumental == null && path_voices == null && !is_not_splitted) {
                Console.Error.WriteLine("songplayer_init() fallback failed, missing file: " + src);
                Console.Error.WriteLine("songplayer_init() cannot load any file, there will only be silence.");

                //if (prefer_no_copyright) free(src);
                return SongPlayer.SILENCE;
            }

            SongPlayer songplayer = new SongPlayer();

            if (is_not_splitted) {
                SoundPlayer player = SoundPlayer.Init(src);
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
                    //free(path_voices);
                }

                if (path_instrumental != null) {
                    player_instrumentals = SoundPlayer.Init(path_instrumental);
                    //free(path_instrumental);
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

    }
}
