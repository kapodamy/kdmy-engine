using System;
using System.Runtime.InteropServices;
using System.Threading;
using Engine.Externals;
using Engine.Externals.FFGraphInterop;
using Engine.Externals.GLFW;
using Engine.Externals.LuaScriptInterop;
using Engine.Externals.SoundBridge;
using Engine.Image;
using Engine.Platform;
using Engine.Sound;
using Engine.Utils;

namespace Engine.Video;

public class VideoPlayer : ISetProperty {
    private const double BEHIND_TOLERANCE = 0.005;// 5ms
    private const int MAX_BUFFERED_SIZE = 1024 * 1024 * 128;// 128MiB
    private static readonly bool runtime_available;

    static VideoPlayer() {
        runtime_available = FFGraph.GetRuntimeInfo() != null;
    }

    private bool is_muted;
    private Stream sndbridge_stream;
    private ISourceHandle audio_sourcehandle;
    private ISourceHandle video_sourcehandle;
    private Sprite sprite;
    private FFGraph ffgraph;
    private IDecoder ffgraph_sndbridge;
    private nint buffer_front;
    private nint buffer_back;
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

    private VideoPlayer() { }

    public static VideoPlayer Init(string src) {
        string full_path = FS.GetFullPathAndOverride(src);

        if (!FS.FileExists(full_path)) { return null; }

        if (!runtime_available) {
            //free(full_path);
            Logger.Error($"videoplayer_init() can not load '{src}'. FFgraph or FFmpeg libraries are not available");
            return null;
        }

        full_path = IO.GetAbsolutePath(full_path, true, false, true);

        byte[] buffer = PreloadCache.RetrieveBuffer(full_path);
        ISourceHandle audio_sourcehandle;
        ISourceHandle video_sourcehandle;

        if (buffer != null) {
            audio_sourcehandle = FileHandleUtil.Init(buffer, 0, buffer.Length);
            video_sourcehandle = FileHandleUtil.Init(buffer, 0, buffer.Length);
        } else {
            if (FS.FileLength(src) > MAX_BUFFERED_SIZE) {
                audio_sourcehandle = FileHandleUtil.Init(full_path, true);
                video_sourcehandle = FileHandleUtil.Init(full_path, true);
            } else {
                // load file contents in RAM
                buffer = FS.ReadArrayBuffer(src);

                audio_sourcehandle = FileHandleUtil.Init(buffer, 0, buffer.Length);
                video_sourcehandle = FileHandleUtil.Init(buffer, 0, buffer.Length);
            }
        }

        //free(full_path);

        // Initialize FFgraph and SoundBridge
        FFGraph ffgraph = null;
        IDecoder ffgraph_sndbridge = null;
        Stream sndbridge_stream = null;
        WebGLTexture tex = WebGLTexture.Null;
        Texture tex_managed = null;

        // initialize FFgraph
        ffgraph = FFGraph.Init(video_sourcehandle, audio_sourcehandle);

        if (ffgraph == null) {
            Logger.Error($"videoplayer_init() ffgraph_init() failed for: {src}");
            goto L_failed;
        }

        // adquire information about audio/video streams
        FFGraphInfo info = ffgraph.GetStreamsInfo();

        if (info.audio_has_stream) {
            // adquire ExternalDecoder of the audio track
            ffgraph_sndbridge = new FFGraphSoundBridgeDecoder(ffgraph, true, false);

            // initialize SoundBridge stream
            StreamResult ret = SoundBridge.Enqueue(ffgraph_sndbridge, out sndbridge_stream);
            if (ret != StreamResult.Success) {
                Logger.Error($"videoplayer_init() SoundBridge::Enqueue() {ret} for: {src}");
                goto L_failed;
            }
        }

        nint buffer_front = 0x00, buffer_back = 0x00;
        int texture_data_size = 0;
        bool frame_available = false;
        if (info.video_has_stream) {
            texture_data_size = info.video_width * info.video_height * 3/*rgb*/;
            buffer_front = Marshal.AllocHGlobal(texture_data_size);
            buffer_back = Marshal.AllocHGlobal(texture_data_size);

            WebGL2RenderingContext gl = PVRContext.global_context.webopengl.gl;
            tex = gl.createTexture();
            gl.bindTexture(gl.TEXTURE_2D, tex);
            gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGB, info.video_width, info.video_height, 0, gl.RGB, gl.UNSIGNED_BYTE, 0x00);
            gl.bindTexture(gl.TEXTURE_2D, WebGLTexture.Null);

            tex_managed = Texture.InitFromRAW(
               tex, texture_data_size, true, info.video_width, info.video_height, info.video_width, info.video_height
            );

            // adquire first video frame
            frame_available = ffgraph.ReadVideoFrame2(buffer_front, texture_data_size) >= 0;
        }

        bool longest_is_video = info.video_has_stream && info.video_seconds_duration > info.audio_seconds_duration;

        Sprite sprite = Sprite.InitFromRGB8(0x000000);
        sprite.SetDrawSize(info.video_width, info.video_height);
        if (tex_managed != null) sprite.SetTexture(tex_managed, false);

        return new VideoPlayer() {
            video_sourcehandle = video_sourcehandle,
            audio_sourcehandle = audio_sourcehandle,
            ffgraph_sndbridge = ffgraph_sndbridge,
            sprite = sprite,
            is_muted = false,
            sndbridge_stream = sndbridge_stream,
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
            mutex = new Mutex()
        };

L_failed:
        if (ffgraph != null) ffgraph.Dispose();
        if (ffgraph_sndbridge != null) ffgraph_sndbridge.Dispose();
        if (sndbridge_stream != 0x00) sndbridge_stream.Dispose();

        if (audio_sourcehandle != null) audio_sourcehandle.Dispose();
        if (video_sourcehandle != null) video_sourcehandle.Dispose();

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

        this.ffgraph.Dispose();
        if (this.ffgraph_sndbridge != null) this.ffgraph_sndbridge.Dispose();
        if (this.sndbridge_stream != null) this.sndbridge_stream.Dispose();

        this.audio_sourcehandle.Dispose();
        this.video_sourcehandle.Dispose();

        this.sprite.Destroy();
        if (this.texture_managed != null) this.texture_managed.Destroy();

        if (this.buffer_front != 0x00) Marshal.FreeHGlobal(this.buffer_front);
        if (this.buffer_back != 0x00) Marshal.FreeHGlobal(this.buffer_back);


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
        if (this.sndbridge_stream != null) {
            this.sndbridge_stream.Pause();
        }
        if (this.decoder != null && this.decoder.IsAlive) {
            this.decoder_running = false;
            this.decoder.Join();
        }
    }

    public void Stop() {
        if (this.sndbridge_stream != null) {
            this.sndbridge_stream.Stop();
        }
        if (this.decoder != null && this.decoder.IsAlive) {
            this.decoder_running = false;
            this.decoder.Join();
        }
        if (this.info.video_has_stream) {
            this.ffgraph.Seek2(0.0, false);
            this.last_video_playback_time = 0.0;
            this.video_track_ended = false;
        }
    }

    public void LoopEnable(bool enable) {
        if (this.info.video_has_stream)
            this.loop_enabled = enable;
        else if (this.sndbridge_stream != null)
            this.sndbridge_stream.SetLooped(enable);
    }

    public void FadeAudio(bool in_or_out, float duration) {
        if (this.sndbridge_stream != null) {
            this.sndbridge_stream.DoFade(in_or_out, duration / 1000f);
        }
    }


    public void SetVolume(float volume) {
        if (this.sndbridge_stream != null) {
            this.sndbridge_stream.SetVolume(volume);
        }
    }

    public void SetMute(bool muted) {
        if (this.sndbridge_stream != null) {
            this.is_muted = muted;
            this.sndbridge_stream.Mute(muted);
        }
    }

    public void Seek(double timestamp) {
        double duration = GetDuration();
        if (timestamp > duration) timestamp = duration;

        if (this.info.video_has_stream) {
            this.mutex.WaitOne();

            this.last_video_playback_time = timestamp;
            this.decoder_seek_request = this.decoder != null && this.decoder.IsAlive;
            this.ffgraph.Seek2(timestamp / 1000.0, false);

            if (!this.decoder_seek_request) {
                // playback is paused, read a single frame to keep the sprite updated
                this.current_buffer_is_front = false;
                this.last_video_playback_time = this.ffgraph.ReadVideoFrame2(
                    this.buffer_front, this.buffer_size
                );
                this.frame_available = this.last_video_playback_time >= 0.0;
            }

            this.mutex.ReleaseMutex();
        }

        if (this.sndbridge_stream != null) {
            this.sndbridge_stream.Seek(timestamp / 1000.0);
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
        if (this.sndbridge_stream != null)
            return this.sndbridge_stream.IsActive;
        else
            return false;
    }

    public Fading HasFadingAudio() {
        if (this.sndbridge_stream != null) {
            return (Fading)this.sndbridge_stream.ActiveFade;
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
        else if (this.sndbridge_stream != null)
            return this.sndbridge_stream.Position * 1000.0;
        else
            return -1.0;
    }

    public bool HasEnded() {
        bool audio_track_ended = true;
        if (this.sndbridge_stream != null)
            audio_track_ended = this.sndbridge_stream.HasEnded;

        return this.video_track_ended && audio_track_ended;
    }


    public bool HasVideoTrack() {
        return this.info.video_has_stream;
    }

    public bool HasAudioTrack() {
        // WARNING: audio playback can be unavailable
        return this.info.audio_has_stream;
    }

    public void PollStreams() {
        // check if time to show the next frame
        if (!this.frame_available) return;

        this.mutex.WaitOne();

        // prepare texture swap, use the buffer which is not current
        nint buffer = this.current_buffer_is_front ? this.buffer_back : this.buffer_front;
        this.frame_available = false;

        // do texture update
        WebGL2RenderingContext gl = PVRContext.global_context.webopengl.gl;

        gl.bindTexture(gl.TEXTURE_2D, this.texture);
        gl.texSubImage2D(
            gl.TEXTURE_2D, 0, 0, 0, this.info.video_width, this.info.video_height, gl.RGB, gl.UNSIGNED_BYTE, buffer
        );
        gl.bindTexture(gl.TEXTURE_2D, WebGLTexture.Null);

        this.mutex.ReleaseMutex();
    }


    private void InternalDecoder() {
        nint front, back;
        int size = this.buffer_size;
        bool no_audio = this.sndbridge_stream == null;
        Stream audio = this.sndbridge_stream;

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

        if (!no_audio) audio.Play();

        while (this.decoder_running) {
            // if loop is enabled, check if both audio and video ended and play again
            if (this.video_track_ended && (no_audio || audio.HasEnded)) {
                if (!no_audio) audio.Stop();
                this.ffgraph.Seek(0.0);

                // loop again 
                this.video_track_ended = false;
                goto L_prepare;
            }

            this.mutex.WaitOne();
            time = this.ffgraph.ReadVideoFrame2(front, size);

            double current_time = Glfw.GetTime() - time_offset;
            if (current_time < next_frame_time) {
                Thread.Sleep(1);
                continue;
            }

            if (this.decoder_running) {
                if (this.decoder_seek_request) {
                    // seek was requested, reset times
                    time_offset = Glfw.GetTime();
                    next_frame_time = Double.NegativeInfinity;
                    this.decoder_seek_request = false;
                } else if (time <= -2.0) {
                    this.video_track_ended = true;
                    this.decoder_running = this.loop_enabled;
                    Interlocked.Exchange(ref this.last_video_playback_time, this.info.video_seconds_duration);
                } else if ((current_time - time) > VideoPlayer.BEHIND_TOLERANCE && next_frame_time >= 0.0) {
                    // behind playback time, skip the current frame to keep in sync
                    Interlocked.Exchange(ref this.last_video_playback_time, time);
#if DEBUG
                    Console.Error.WriteLineAsync(
                        $"VideoPlayer::InternalDecoder() video out-of-sync {time}/{current_time}"
                    );
#endif
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
                    nint tmp = back;
                    back = front;
                    front = tmp;
                }
            }
            this.mutex.ReleaseMutex();
        }

        // self stop
        this.decoder_running = false;
    }

    private void InternalRunDecoderAsync() {
        if (!this.info.video_has_stream && this.sndbridge_stream != null) {
            this.sndbridge_stream.Play();
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

