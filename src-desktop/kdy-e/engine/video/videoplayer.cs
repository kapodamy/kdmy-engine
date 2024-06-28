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
    private nint buffer_front, buffer_back;
    private int buffer_size;
    private FFGraphInfo info;
    private Thread decoder;
    private Mutex mutex;
    private volatile bool decoder_running;
    private volatile bool current_buffer_is_front;
    private volatile bool video_track_ended;
    private volatile bool frame_available;
    private Texture texture_managed;
    private bool longest_is_video;
    private volatile bool loop_enabled;
    private double last_video_playback_time;
    private volatile int stride_front_y, stride_front_u, stride_front_v;
    private volatile int stride_back_y, stride_back_u, stride_back_v;

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
            if (FS.FileLength(src) < FileHandleUtil.MAX_SIZE_IN_MEMORY) {
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
        FFGraph ffgraph;
        IDecoder ffgraph_sndbridge = null;
        Stream sndbridge_stream = null;
        Texture texture = null;
        nint buffer_front = 0x00, buffer_back = 0x00;
        int buffer_size = 0;
        int stride_y = 0, stride_u = 0, stride_v = 0;
        bool frame_available = false;

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

        if (info.video_has_stream) {
            bool is_rgb = !info.video_is_yuv420p_or_rgb24;
            int width = info.video_width, height = info.video_height;
            int half_width = width / 2, half_height = height / 2;
            int frame_size = width * height;

            // read first video frame
            FFGraphFrame frame;
            frame_available = ffgraph.ReadVideoFrame(out frame) >= 0.0;

            if (is_rgb) {
                frame_size *= 3;
            } else if (frame_available) {
                // calculate Y size
                frame_size += (width + frame.y_stride) * height;
                // calculate U size
                frame_size += (half_width + frame.u_stride) * half_height;
                // calculate V size
                frame_size += (half_width + frame.v_stride) * half_height;

                stride_y = frame.y_stride;
                stride_u = frame.u_stride;
                stride_v = frame.v_stride;
            }

            WebGLTexture tex = VideoPlayer.InternalCreateTexture(width, height, is_rgb);

            texture = Texture.InitFromRAW(
               tex, frame_size, true, width, height, width, height
            );

            if (info.video_is_yuv420p_or_rgb24) {
                texture.data_vram_crhoma_planes = new WebGLTexture[] {
                    VideoPlayer.InternalCreateTexture(half_width, half_height, false),
                    VideoPlayer.InternalCreateTexture(half_width, half_height, false)
                };
            }

            // allocate buffers
            buffer_front = Marshal.AllocHGlobal(frame_size);
            buffer_back = Marshal.AllocHGlobal(frame_size);
            buffer_size = frame_size;

            // copy to front buffer
            if (frame_available) {
                if (info.video_is_yuv420p_or_rgb24)
                    frame.CopyYUVToBuffer(buffer_front, buffer_size);
                else
                    frame.CopyRGB24ToBuffer(buffer_front, buffer_size);
            }
        }

        bool longest_is_video = info.video_has_stream && info.video_seconds_duration > info.audio_seconds_duration;

        Sprite sprite = Sprite.InitFromRGB8(0x000000);
        sprite.SetDrawSize(info.video_width, info.video_height);
        if (texture != null) sprite.SetTexture(texture, false);

        VideoPlayer videoplayer = new VideoPlayer() {
            video_sourcehandle = video_sourcehandle,
            audio_sourcehandle = audio_sourcehandle,
            ffgraph_sndbridge = ffgraph_sndbridge,
            sprite = sprite,
            is_muted = false,
            sndbridge_stream = sndbridge_stream,
            ffgraph = ffgraph,
            buffer_front = buffer_front,
            buffer_back = buffer_back,
            buffer_size = buffer_size,
            info = info,
            texture_managed = texture,
            decoder = null,
            mutex = new Mutex(),
            decoder_running = false,
            current_buffer_is_front = false,// front buffer holds the first frame
            video_track_ended = !info.video_has_stream,
            frame_available = frame_available,
            longest_is_video = longest_is_video,
            loop_enabled = false,
            last_video_playback_time = 0.0,
            stride_front_y = stride_y,
            stride_front_u = stride_u,
            stride_front_v = stride_v,
            stride_back_y = stride_y,
            stride_back_u = stride_u,
            stride_back_v = stride_v
        };

        videoplayer.PollStreams();
        return videoplayer;

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

        this.mutex.Dispose();

        this.decoder = null;

        if (this.sndbridge_stream != null) this.sndbridge_stream.Dispose();
        if (this.ffgraph_sndbridge != null) this.ffgraph_sndbridge.Dispose();
        this.ffgraph.Dispose();

        if (this.audio_sourcehandle != null) this.audio_sourcehandle.Dispose();
        if (this.video_sourcehandle != null) this.video_sourcehandle.Dispose();

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
            this.decoder = null;
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
            this.ffgraph.SeekVideo(0.0);
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

        double time = timestamp / 1000.0;

        if (this.info.video_has_stream) {
            bool video_playing = this.decoder != null && this.decoder.IsAlive;

            this.decoder_running = false;

            if (video_playing) {
                this.decoder.Join();
            }

            if (this.sndbridge_stream != null) {
                this.sndbridge_stream.Pause();
                this.sndbridge_stream.Seek(time);
            }

            this.decoder = null;
            this.last_video_playback_time = time;
            this.ffgraph.SeekVideo(time);

            if (video_playing) {
                InternalRunDecoderAsync();
            } else {
                // read a single frame to keep the sprite updated while is paused
                time = InternalReadVideoFrame();

                this.frame_available = time >= 0.0;
                this.last_video_playback_time = time;

                if (time >= 0.0) this.current_buffer_is_front = !this.current_buffer_is_front;
            }
        } else if (this.sndbridge_stream != null) {
            // no video stream, seek audio stream only
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
                switch ((int)value) {
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
        if (this.longest_is_video)
            return this.last_video_playback_time * 1000.0;
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
        return this.sndbridge_stream != null && this.info.audio_has_stream;
    }

    public void PollStreams() {
        // check if time to show the next frame
        if (!this.frame_available) return;

        this.mutex.WaitOne();

        // prepare texture swap, use the buffer which is not current
        bool is_front = this.current_buffer_is_front;
        nint buffer = is_front ? this.buffer_back : this.buffer_front;
        this.frame_available = false;

        // do texture update
        WebGL2RenderingContext gl = PVRContext.global_context.webopengl.gl;
        Texture tex = this.texture_managed;
        int width = this.info.video_width, height = this.info.video_height;

        if (this.info.video_is_yuv420p_or_rgb24) {
            int half_width = width / 2, half_height = height / 2;

            int stride_y, stride_u, stride_v;
            if (is_front) {
                stride_y = this.stride_back_y;
                stride_u = this.stride_back_u;
                stride_v = this.stride_back_v;
            } else {
                stride_y = this.stride_front_y;
                stride_u = this.stride_front_u;
                stride_v = this.stride_front_v;
            }

            VideoPlayer.InternalUploadPlane(
                gl, gl.TEXTURE0, tex.data_vram, stride_y, width, height, buffer
            );
            buffer += stride_y * height;

            VideoPlayer.InternalUploadPlane(
                gl, gl.TEXTURE1, tex.data_vram_crhoma_planes[0], stride_u, half_width, half_height, buffer
            );
            buffer += stride_u * half_height;

            VideoPlayer.InternalUploadPlane(
                gl, gl.TEXTURE2, tex.data_vram_crhoma_planes[1], stride_u, half_width, half_height, buffer
            );
            gl.bindTexture(gl.TEXTURE_2D, WebGLTexture.Null);
            //buffer += stride_v * half_height;

            // restore UNPACK_ROW_LENGTH value
            gl.pixelStorei(gl.UNPACK_ROW_LENGTH, 0);
        } else {
            // rgb texture
            gl.activeTexture(gl.TEXTURE0);
            gl.bindTexture(gl.TEXTURE_2D, this.texture_managed.data_vram);
            gl.texSubImage2D(
                gl.TEXTURE_2D, 0, 0, 0, width, height, gl.RGB, gl.UNSIGNED_BYTE, buffer
            );
            gl.bindTexture(gl.TEXTURE_2D, WebGLTexture.Null);
        }

        this.current_buffer_is_front = !is_front;
        this.mutex.ReleaseMutex();
    }


    private void InternalDecoder() {
        Stream audio = this.sndbridge_stream;

L_prepare:
        double time = 0.0;
        double time_offset = Glfw.GetTime();
        double next_frame_time = Double.NegativeInfinity;
        bool play_audio = this.sndbridge_stream != null;
        bool fetch_frame = true;
        bool first_frame = true;
        bool has_error = false;

        while (this.decoder_running) {
            // if loop is enabled check if both audio and video ended and play again
            if (this.video_track_ended) {
                if (audio != null && !audio.HasEnded) {
                    // wait for audio track
                    Thread.Sleep(5);
                    continue;
                }

                // rewind streams
                this.ffgraph.SeekVideo(0.0);
                if (audio != null) audio.Stop();

                // loop again
                this.video_track_ended = false;
                goto L_prepare;
            }

            if (fetch_frame) {
                fetch_frame = false;
                time = this.InternalReadVideoFrame();
            }

            if (play_audio) {
                play_audio = false;
                this.sndbridge_stream.Play();
            }

            double current_time;

            if (first_frame) {
                first_frame = false;
                time_offset = Glfw.GetTime() - time;
                current_time = 0.0;
            } else {
                current_time = Glfw.GetTime() - time_offset;

                if (current_time < next_frame_time) {
                    // wait until the presentation time is reached
                    Thread.Sleep(1);
                    continue;
                }
            }

            if (!this.decoder_running) {
                // call to Stop, Pause, Seek or Destroy method was done
                break;
            }

            this.mutex.WaitOne();

            if (time <= -2.0) {
                this.video_track_ended = true;
                this.decoder_running = this.loop_enabled;
                this.frame_available = false;
                Interlocked.Exchange(ref this.last_video_playback_time, next_frame_time);
            } else if (time < 0.0) {
#if DEBUG
                Console.Error.WriteLineAsync($"VideoPlayer::InternalDecoder() FFGraph::ReadVideoFrameToBuffer() failed.");
#endif
                this.frame_available = false;

                if (has_error) {
                    // video playback failed
                    this.video_track_ended = true;
                    this.decoder_running = this.loop_enabled;
                    Console.Error.WriteLineAsync($"VideoPlayer::InternalDecoder() video playback ended by error.");
                }
                has_error = true;
                fetch_frame = true;// try again
            } else {
#if DEBUG && false
                double delta = current_time - time;
                if (delta > 0.005/*ms*/ && next_frame_time >= 0.0) {
                    Console.Error.WriteLineAsync($"VideoPlayer::InternalDecoder() out-of-sync by {time - current_time}ms.");
                }
#endif
                Interlocked.Exchange(ref this.last_video_playback_time, time);
                this.frame_available = true;

                next_frame_time = time;
                has_error = false;
                fetch_frame = true;
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

        this.decoder.Start();
    }

    private double InternalReadVideoFrame() {
        FFGraphFrame frame;
        double time = this.ffgraph.ReadVideoFrame(out frame);
        bool is_front = this.current_buffer_is_front;

        if (time >= 0.0) {
            nint buffer = is_front ? this.buffer_front : this.buffer_back;

            if (this.info.video_is_yuv420p_or_rgb24)
                frame.CopyYUVToBuffer(buffer, this.buffer_size);
            else
                frame.CopyRGB24ToBuffer(buffer, this.buffer_size);

            if (is_front) {
                this.stride_front_y = frame.y_stride;
                this.stride_front_u = frame.u_stride;
                this.stride_front_v = frame.v_stride;
            } else {
                this.stride_back_y = frame.y_stride;
                this.stride_back_u = frame.u_stride;
                this.stride_back_v = frame.v_stride;
            }
        }

        return time;
    }


    private static WebGLTexture InternalCreateTexture(int width, int height, bool is_rgb) {
        WebGL2RenderingContext gl = PVRContext.global_context.webopengl.gl;

        GLenum fmt = is_rgb ? gl.RGB : gl.R8;
        WebGLTexture tex = gl.createTexture();

        gl.activeTexture(gl.TEXTURE0);
        gl.bindTexture(gl.TEXTURE_2D, tex);
        //gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
        //gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
        gl.texImage2D(gl.TEXTURE_2D, 0, fmt, width, height, 0, gl.RED, gl.UNSIGNED_BYTE, 0x00);
        gl.bindTexture(gl.TEXTURE_2D, WebGLTexture.Null);

        return tex;
    }

    private static void InternalUploadPlane(WebGL2RenderingContext gl, GLenum unit, WebGLTexture tex, int stride, int width, int height, nint buffer) {
        gl.activeTexture(unit);
        gl.bindTexture(gl.TEXTURE_2D, tex);
        gl.pixelStorei(gl.UNPACK_ROW_LENGTH, stride);
        gl.texSubImage2D(gl.TEXTURE_2D, 0, 0, 0, width, height, gl.RED, gl.UNSIGNED_BYTE, buffer);
        gl.bindTexture(gl.TEXTURE_2D, WebGLTexture.Null);
    }

}

