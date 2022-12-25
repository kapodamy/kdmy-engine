using System;
using System.Runtime.InteropServices;
using System.Text;
using Engine.Externals;
using Engine.Externals.LuaScriptInterop;
using Engine.Platform;
using Engine.Utils;

namespace Engine.Sound {

    public class SoundPlayer {

        private int stream_id;
        private IntPtr filehandle;
        private GCHandle gchandle;
        private bool is_muted;

        private SoundPlayer() {

        }

        public static SoundPlayer Init(string src) {
            string full_path = FS.GetFullPathAndOverride(src);

            if (!FS.FileExists(full_path)) { return null; }

            if (!full_path.EndsWith(".ogg", StringComparison.OrdinalIgnoreCase)) {
                Console.Error.WriteLine("songplayer_init() format not supported: " + src);
                return null;
            }

            full_path = IO.GetAbsolutePath(full_path, true, true);

            byte[] buffer = PreloadCache.RetrieveBuffer(full_path);
            GCHandle gchandle;
            IntPtr filehandle;

            if (buffer != null) {
                gchandle = GCHandle.Alloc(buffer, GCHandleType.Pinned);
                filehandle = AICA.filehandle_init2(gchandle.AddrOfPinnedObject(), buffer.Length);
            } else {
                byte[] full_path_ptr = new byte[Encoding.UTF8.GetByteCount(full_path) + 1];
                Encoding.UTF8.GetBytes(full_path, 0, full_path.Length, full_path_ptr, 0);

                filehandle = AICA.filehandle_init(full_path_ptr);
                if (filehandle == IntPtr.Zero) {
                    Console.Error.WriteLine("soundplayer_init() filehandle_init failed for: " + src);
                    return null;
                }

                gchandle = new GCHandle();
            }

            int stream_id = AICA.sndbridge_queue_ogg(filehandle);
            if (stream_id < 0) {
                AICA.filehandle_destroy(filehandle);
                Console.Error.WriteLine("soundplayer_init() sndbridge_queue_ogg failed for: " + src);
                return null;
            }

            return new SoundPlayer() { filehandle = filehandle, stream_id = stream_id, gchandle = gchandle };
        }

        public void Destroy() {
            Luascript.DropShared(this);
            if (this.filehandle == IntPtr.Zero) return;
            AICA.sndbridge_dispose(this.stream_id);
            AICA.filehandle_destroy(this.filehandle);
            this.filehandle = IntPtr.Zero;
            if (this.gchandle.IsAllocated) this.gchandle.Free();
        }


        public void Replay() {
            AICA.sndbridge_stop(this.stream_id);
            AICA.sndbridge_play(this.stream_id);
        }

        public void Play() {
            AICA.sndbridge_play(this.stream_id);
        }

        public void Pause() {
            AICA.sndbridge_pause(this.stream_id);
        }

        public void Stop() {
            AICA.sndbridge_stop(this.stream_id);
        }

        public void LoopEnable(bool enable) {
            AICA.sndbridge_loop(this.stream_id, enable);
        }

        public void Fade(bool in_or_out, float duration) {
            AICA.sndbridge_do_fade(this.stream_id, in_or_out, duration);
        }


        public void SetVolume(float volume) {
            AICA.sndbridge_set_volume(this.stream_id, volume);
        }

        public void SetMute(bool muted) {
            this.is_muted = muted;
            AICA.sndbridge_mute(this.stream_id, muted);
        }

        public void Seek(double timestamp) {
            AICA.sndbridge_seek(this.stream_id, timestamp);
        }

        public void SetProperty(int property_id, float value) {
            switch (property_id) {
                case VertexProps.MEDIA_PROP_VOLUME:
                    this.SetVolume(value);
                    break;
                case VertexProps.MEDIA_PROP_SEEK:
                    this.Seek(value);
                    break;
                case VertexProps.MEDIA_PROP_PLAYBACK:
                    switch (value) {
                        case VertexProps.PLAYBACK_PLAY:
                            this.Play();
                            break;
                        case VertexProps.PLAYBACK_PAUSE:
                            this.Pause();
                            break;
                        case VertexProps.PLAYBACK_STOP:
                            this.Stop();
                            break;
                        case VertexProps.PLAYBACK_MUTE:
                            this.SetMute(true);
                            break;
                        case VertexProps.PLAYBACK_UNMUTE:
                            this.SetMute(false);
                            break;
                    }
                    break;
            }
        }


        public bool IsMuted() {
            return this.is_muted;
        }

        public bool IsPlaying() {
            return AICA.sndbridge_is_active(this.stream_id);
        }

        public bool IsFadding() {
            return AICA.sndbridge_is_fade_active(this.stream_id);
        }

        public double GetDuration() {
            return AICA.sndbridge_duration(this.stream_id);
        }

        public double GetPosition() {
            return AICA.sndbridge_position(this.stream_id);
        }

        public bool HasEnded() {
            return AICA.sndbridge_has_ended(this.stream_id);
        }

    }
}
