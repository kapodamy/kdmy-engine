using System;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using Engine.Externals;
using Engine.Externals.GLFW;
using Engine.Externals.LuaScriptInterop;
using Engine.Image;
using Engine.Platform;
using Engine.Sound;
using Engine.Utils;

namespace Engine.Video {

    public class VideoPlayer : ISetProperty {
        private const double BEHIND_TOLERANCE = 0.005;// 5ms
        private const int MAX_BUFFERED_SIZE = 1024 * 1024 * 128;// 128MiB
        private static readonly bool runtime_available;

        static VideoPlayer() {
            runtime_available = FFgraph.ffgraph_get_runtime_info() != null;
        }

        private bool is_muted;
        private int sndbridge_stream_id;
        private GCHandle gchandle;
        private IntPtr audio_filehandle;
        private IntPtr video_filehandle;
        private Sprite sprite;
        private IntPtr ffgraph;
        private IntPtr ffgraph_sndbridge;
        private IntPtr buffer_front;
        private IntPtr buffer_back;
        private int buffer_size;
        private FFGraphInfo info;
        private Thread decoder;
        private volatile bool decoder_running;
        private volatile bool current_buffer_is_front;
        private volatile bool video_track_ended;
        private volatile bool frame_available;
        private volatile bool decoder_seek_request = false;
        private WebGLTexture texture;
        private Texture texture_managed;
        private bool longest_is_video;
        private volatile bool loop_enabled;
        private double last_video_playback_time;
        private Mutex mutex;
        private IntPtr buffered_file;

        private VideoPlayer() { }

        public static VideoPlayer Init(string src) {
            string full_path = FS.GetFullPathAndOverride(src);

            if (!FS.FileExists(full_path)) { return null; }

            if (!runtime_available) {
                Console.Error.WriteLine($"[ERROR] VideoPlayer::Init() can not load '${src}'. FFgraph or FFmpeg libraries are not available");
                return null;
            }

            full_path = IO.GetAbsolutePath(full_path, true, true);

            byte[] full_path_ptr = new byte[Encoding.UTF8.GetByteCount(full_path) + 1];
            Encoding.UTF8.GetBytes(full_path, 0, full_path.Length, full_path_ptr, 0);

            byte[] buffer = PreloadCache.RetrieveBuffer(full_path);
            IntPtr buffered_file = IntPtr.Zero;
            GCHandle gchandle;
            IntPtr audio_filehandle;
            IntPtr video_filehandle;

            if (buffer != null) {
                gchandle = GCHandle.Alloc(buffer, GCHandleType.Pinned);
                audio_filehandle = AICA.filehandle_init2(gchandle.AddrOfPinnedObject(), buffer.Length);
                video_filehandle = AICA.filehandle_init2(gchandle.AddrOfPinnedObject(), buffer.Length);
            } else {
                if (FS.FileLength(src) > MAX_BUFFERED_SIZE) {
                    audio_filehandle = AICA.filehandle_init(full_path_ptr);
                    video_filehandle = AICA.filehandle_init(full_path_ptr);
                } else {
                    // load file contents in RAM
                    buffer = FS.ReadArrayBuffer(src);
                    buffered_file = Marshal.AllocHGlobal(buffer.Length);
                    Marshal.Copy(buffer, 0, buffered_file, buffer.Length);

                    audio_filehandle = AICA.filehandle_init2(buffered_file, buffer.Length);
                    video_filehandle = AICA.filehandle_init2(buffered_file, buffer.Length);
                }
                gchandle = default(GCHandle);
            }

            // Initialize FFgraph and SoundBridge
            IntPtr ffgraph = IntPtr.Zero;
            IntPtr ffgraph_sndbridge = IntPtr.Zero;
            int sndbridge_stream_id = -1;
            WebGLTexture tex = WebGLTexture.Null;
            Texture tex_managed = null;

            // initialize FFgraph
            ffgraph = FFgraph.ffgraph_init(video_filehandle, audio_filehandle);

            if (ffgraph == IntPtr.Zero) {
                Console.Error.WriteLine("videoplayer_init() ffgraph_init() failed for: " + src);
                goto L_failed;
            }

            // adquire information about audio/video streams
            FFGraphInfo info = new FFGraphInfo();
            FFgraph.ffgraph_get_streams_info(ffgraph, ref info);

            if (info.audio_has_stream) {
                // adquire ExternalDecoder of the audio track
                ffgraph_sndbridge = FFgraph.ffgraph_sndbridge_create_helper(ffgraph, true, false);

                // initialize SoundBridge stream
                sndbridge_stream_id = AICA.sndbridge_queue(ffgraph_sndbridge);
                if (sndbridge_stream_id < 0) {
                    Console.Error.WriteLine("videoplayer_init() sndbridge_queue() failed for: " + src);
                    goto L_failed;
                }
            }

            IntPtr buffer_front = IntPtr.Zero, buffer_back = IntPtr.Zero;
            int texture_data_size = 0;
            bool frame_available = false;
            if (info.video_has_stream) {
                texture_data_size = info.video_width * info.video_height * 3/*rgb*/;
                buffer_front = Marshal.AllocHGlobal(texture_data_size);
                buffer_back = Marshal.AllocHGlobal(texture_data_size);

                WebGL2RenderingContext gl = PVRContext.global_context.webopengl.gl;
                tex = gl.createTexture();
                gl.bindTexture(gl.TEXTURE_2D, tex);
                gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGB, info.video_width, info.video_height, 0, gl.RGB, gl.UNSIGNED_BYTE, IntPtr.Zero);
                gl.bindTexture(gl.TEXTURE_2D, WebGLTexture.Null);

                tex_managed = Texture.InitFromRAW(
                   tex, texture_data_size, true, info.video_width, info.video_height, info.video_width, info.video_height
                );

                // adquire first video frame
                frame_available = FFgraph.ffgraph_read_video_frame2(ffgraph, buffer_front, texture_data_size) >= 0;
            }

            bool longest_is_video = info.video_has_stream && info.video_seconds_duration > info.audio_seconds_duration;

            Sprite sprite = Sprite.InitFromRGB8(0x000000);
            sprite.SetDrawSize(info.video_width, info.video_height);
            if (tex_managed != null) sprite.SetTexture(tex_managed, false);

            return new VideoPlayer() {
                gchandle = gchandle,
                video_filehandle = video_filehandle,
                audio_filehandle = audio_filehandle,
                ffgraph_sndbridge = ffgraph_sndbridge,
                sprite = sprite,
                is_muted = false,
                sndbridge_stream_id = sndbridge_stream_id,
                ffgraph = ffgraph,
                buffer_front = buffer_front,
                buffer_back = buffer_back,
                buffer_size = texture_data_size,
                info = info,
                texture = tex,
                texture_managed = tex_managed,
                decoder = null,
                decoder_running = false,
                current_buffer_is_front = true,
                video_track_ended = !info.video_has_stream,
                frame_available = frame_available,
                longest_is_video = longest_is_video,
                loop_enabled = false,
                last_video_playback_time = 0.0,
                mutex = new Mutex(),
                buffered_file = buffered_file
            };

L_failed:
            if (ffgraph != IntPtr.Zero) FFgraph.ffgraph_destroy(ffgraph);
            if (ffgraph_sndbridge != IntPtr.Zero) FFgraph.ffgraph_sndbridge_destroy_helper(ffgraph_sndbridge);
            if (sndbridge_stream_id >= 0) AICA.sndbridge_dispose(sndbridge_stream_id);
            if (buffered_file != IntPtr.Zero) Marshal.FreeHGlobal(buffered_file);

            if (gchandle.IsAllocated) gchandle.Free();

            if (audio_filehandle != IntPtr.Zero) AICA.filehandle_destroy(audio_filehandle);
            if (video_filehandle != IntPtr.Zero) AICA.filehandle_destroy(video_filehandle);

            return null;
        }

        public void Destroy() {
            Luascript.DropShared(this);

            if (this.decoder != null && this.decoder.IsAlive) {
                this.decoder_running = false;
                this.decoder.Join();
            }

            this.decoder = null;
            this.mutex.Dispose();

            if (this.gchandle.IsAllocated) this.gchandle.Free();
            FFgraph.ffgraph_destroy(this.ffgraph);
            if (this.ffgraph_sndbridge != IntPtr.Zero) FFgraph.ffgraph_sndbridge_destroy_helper(this.ffgraph_sndbridge);
            if (this.sndbridge_stream_id >= 0) AICA.sndbridge_dispose(this.sndbridge_stream_id);

            AICA.filehandle_destroy(this.audio_filehandle);
            AICA.filehandle_destroy(this.video_filehandle);

            if (this.buffered_file != IntPtr.Zero) Marshal.FreeHGlobal(this.buffered_file);

            this.sprite.Destroy();
            if (texture_managed != null) texture_managed.Destroy();

            if (this.buffer_front != IntPtr.Zero) Marshal.FreeHGlobal(this.buffer_front);
            if (this.buffer_back != IntPtr.Zero) Marshal.FreeHGlobal(this.buffer_back);


            // free(this);
        }

        public Sprite GetSprite() {
            return this.sprite;
        }

        public void Replay() {
            Stop();
            Play();
        }

        public void Play() {
            InternalRunDecoderAsync();
        }

        public void Pause() {
            if (this.info.audio_has_stream) {
                AICA.sndbridge_pause(this.sndbridge_stream_id);
            }
            if (this.decoder != null && this.decoder.IsAlive) {
                this.decoder_running = false;
                this.decoder.Join();
            }
        }

        public void Stop() {
            if (this.info.audio_has_stream) {
                AICA.sndbridge_stop(this.sndbridge_stream_id);
            }
            if (this.decoder != null && this.decoder.IsAlive) {
                this.decoder_running = false;
                this.decoder.Join();
            }
            if (this.info.video_has_stream) {
                FFgraph.ffgraph_seek2(this.ffgraph, 0.0, false);
                this.last_video_playback_time = 0.0;
                this.video_track_ended = false;
            }
        }

        public void LoopEnable(bool enable) {
            if (this.info.video_has_stream)
                this.loop_enabled = enable;
            else
                AICA.sndbridge_loop(this.sndbridge_stream_id, enable);
        }

        public void FadeAudio(bool in_or_out, float duration) {
            if (this.info.audio_has_stream) {
                AICA.sndbridge_do_fade(this.sndbridge_stream_id, in_or_out, duration);
            }
        }


        public void SetVolume(float volume) {
            if (this.sndbridge_stream_id >= 0) {
                AICA.sndbridge_set_volume(this.sndbridge_stream_id, volume);
            }
        }

        public void SetMute(bool muted) {
            if (this.sndbridge_stream_id >= 0) {
                this.is_muted = muted;
                AICA.sndbridge_mute(this.sndbridge_stream_id, muted);
            }
        }

        public void Seek(double timestamp) {
            double duration = GetDuration();
            if (timestamp > duration) timestamp = duration;

            if (this.info.video_has_stream) {
                this.mutex.WaitOne();

                this.last_video_playback_time = timestamp;
                this.decoder_seek_request = this.decoder != null && this.decoder.IsAlive;
                FFgraph.ffgraph_seek2(this.ffgraph, timestamp / 1000.0, false);

                if (!this.decoder_seek_request) {
                    // playback is paused, read a single frame to keep the sprite updated
                    this.last_video_playback_time = FFgraph.ffgraph_read_video_frame2(
                        this.ffgraph, this.buffer_front, this.buffer_size
                    );
                    this.frame_available = this.last_video_playback_time >= 0;
                }

                this.mutex.ReleaseMutex();
            }

            if (this.sndbridge_stream_id >= 0) {
                AICA.sndbridge_seek(this.sndbridge_stream_id, timestamp);
            }
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
                default:
                    this.sprite.SetProperty(property_id, value);
                    break;
            }
        }


        public bool IsMuted() {
            return this.is_muted;
        }

        public bool IsPlaying() {
            if (this.longest_is_video)
                return this.decoder_running;
            if (this.info.audio_has_stream)
                return AICA.sndbridge_is_active(this.sndbridge_stream_id);
            else
                return false;
        }

        public Fading HasFaddingAudio() {
            if (this.sndbridge_stream_id >= 0) {
                return (Fading)AICA.sndbridge_has_fade_active(this.sndbridge_stream_id);
            }
            return Fading.NONE;
        }

        public double GetDuration() {
            if (this.longest_is_video)
                return this.info.video_seconds_duration * 1000.0;
            else
                return this.info.audio_seconds_duration * 1000.0;
        }

        public double GetPosition() {
            if (longest_is_video)
                return this.last_video_playback_time;
            else if (this.sndbridge_stream_id >= 0)
                return AICA.sndbridge_position(this.sndbridge_stream_id);
            else
                return -1.0;
        }

        public bool HasEnded() {
            bool audio_track_ended = true;
            if (this.sndbridge_stream_id >= 0)
                audio_track_ended = AICA.sndbridge_has_ended(this.sndbridge_stream_id);

            return this.video_track_ended && audio_track_ended;
        }


        public bool HasVideoTrack() {
            return this.info.video_has_stream;
        }

        public bool HasAudioTrack() {
            return this.info.audio_has_stream;
        }

        public void PollStreams() {
            // check if time to show the next frame
            if (!this.frame_available) return;

            mutex.WaitOne();

            // prepare texture swap, use the buffer which is not current
            IntPtr buffer = this.current_buffer_is_front ? this.buffer_back : this.buffer_front;

            // do texture update
            WebGL2RenderingContext gl = PVRContext.global_context.webopengl.gl;

            gl.bindTexture(gl.TEXTURE_2D, this.texture);
            gl.texSubImage2D(
                gl.TEXTURE_2D, 0, 0, 0, this.info.video_width, this.info.video_height, gl.RGB, gl.UNSIGNED_BYTE, buffer
            );
            gl.bindTexture(gl.TEXTURE_2D, WebGLTexture.Null);

            mutex.ReleaseMutex();
        }


        private void InternalDecoder() {
            IntPtr front, back;
            int size = this.buffer_size;
            bool no_audio = !this.info.audio_has_stream;
            int audio_id = this.sndbridge_stream_id;

            if (this.current_buffer_is_front) {
                front = this.buffer_front;
                back = this.buffer_back;
            } else {
                back = this.buffer_front;
                front = this.buffer_back;
            }

L_prepare:
            double time;
            double time_offset = Glfw.GetTime();
            double next_frame_time = Double.NegativeInfinity;

            if (!no_audio) AICA.sndbridge_play(audio_id);

            while (this.decoder_running) {
                if (this.video_track_ended && (no_audio || AICA.sndbridge_has_ended(audio_id))) {
                    if (!no_audio) AICA.sndbridge_stop(audio_id);
                    FFgraph.ffgraph_seek(this.ffgraph, 0.0);

                    // loop again 
                    this.video_track_ended = false;
                    goto L_prepare;
                }

                double current_time = Glfw.GetTime() - time_offset;
                if (current_time < next_frame_time) {
                    Thread.Sleep(1);
                    continue;
                }

                this.mutex.WaitOne();
                time = FFgraph.ffgraph_read_video_frame2(this.ffgraph, front, size);

                if (this.decoder_running) {
                    if (this.decoder_seek_request) {
                        // seek was requested, reset times
                        time_offset = Glfw.GetTime();
                        next_frame_time = Double.NegativeInfinity;
                        this.decoder_seek_request = false;
                    } else if ((current_time - time) > VideoPlayer.BEHIND_TOLERANCE && next_frame_time >= 0.0) {
                        // behind playback time, skip the current frame to keep in sync
                        Interlocked.Exchange(ref this.last_video_playback_time, time);
#if DEBUG
                        Console.Error.WriteLineAsync(
                            $"VideoPlayer::InternalDecoder() video out-of-sync {time}/{current_time}"
                        );
#endif
                    } else if (time == -2.0) {
                        this.video_track_ended = true;
                        this.decoder_running = this.loop_enabled;
                        Interlocked.Exchange(ref this.last_video_playback_time, this.info.video_seconds_duration);
                    } else if (time < 0) {
                        // an error ocurred
                    } else {
                        // swap buffers only if no the first frame
                        if (next_frame_time >= 0.0) {
                            this.current_buffer_is_front = !this.current_buffer_is_front;
                        }
                        Interlocked.Exchange(ref this.last_video_playback_time, time);
                        this.frame_available = true;
                        next_frame_time = time;
                        IntPtr tmp = back;
                        back = front;
                        front = tmp;
                    }
                }
                this.mutex.ReleaseMutex();
            }
        }

        private void InternalRunDecoderAsync() {
            if (!this.info.video_has_stream) {
                AICA.sndbridge_play(this.sndbridge_stream_id);
                return;
            }

            // (just in case) wait until the decoder thread ends
            if (this.decoder != null && this.decoder.IsAlive) {
                this.decoder_running = false;
                this.decoder.Join();
            }

            this.decoder = new Thread(this.InternalDecoder);
            this.decoder.IsBackground = true;

            this.frame_available = false;
            this.decoder_running = true;
            this.video_track_ended = false;
            this.decoder_seek_request = false;

            this.decoder.Start();
        }

    }
}

