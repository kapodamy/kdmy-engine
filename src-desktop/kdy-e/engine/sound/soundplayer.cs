using System;
using Engine.Externals.LuaScriptInterop;
using Engine.Externals.SoundBridge;
using Engine.Platform;
using Engine.Utils;

namespace Engine.Sound;

public enum Fading : int {
    NONE = 0,
    IN = 1,
    OUT = 2
}

public class SoundPlayer {

    private Stream stream;
    private ISourceHandle sourcehandle;
    private bool is_muted;

    private SoundPlayer() {

    }

    public static SoundPlayer Init(string src) {
        string full_path = FS.GetFullPathAndOverride(src);

        if (!FS.FileExists(full_path)) { return null; }

        if (
            !full_path.EndsWith(".ogg", StringComparison.OrdinalIgnoreCase) &&
            !full_path.EndsWith(".logg", StringComparison.OrdinalIgnoreCase)
            ) {
            Logger.Error($"songplayer_init() format not supported: {src}");
            return null;
        }

        full_path = IO.GetAbsolutePath(full_path, true, false, true);

        byte[] buffer = PreloadCache.RetrieveBuffer(full_path);
        ISourceHandle sourcehandle;

        if (buffer != null) {
            sourcehandle = FileHandleUtil.Init(buffer, 0, buffer.Length);
        } else {
            sourcehandle = FileHandleUtil.Init(full_path, true);
            if (sourcehandle == null) {
                Logger.Error($"soundplayer_init() filehandle_init1 failed for: {src}");
                return null;
            }
        }

        Stream stream;
        StreamResult res = SoundBridge.Enqueue(sourcehandle, out stream);
        if (res != StreamResult.Success) {
            sourcehandle.Dispose();
            Logger.Error($"soundplayer_init() SoundBridge::Enqueue() returned {res} for: {src}");
            return null;
        }

        return new SoundPlayer() { sourcehandle = sourcehandle, stream = stream, is_muted = false };
    }

    public void Destroy() {
        if (this.stream == null) return;

        Luascript.DropShared(this);

        this.stream.Dispose();
        this.sourcehandle.Dispose();

        this.stream = null;
        this.sourcehandle = null;
    }


    public void Replay() {
        this.stream.Stop();
        this.stream.Play();
    }

    public void Play() {
        this.stream.Play();
    }

    public void Pause() {
        this.stream.Pause();
    }

    public void Stop() {
        this.stream.Stop();
    }

    public void LoopEnable(bool enable) {
        this.stream.SetLooped(enable);
    }

    public void Fade(bool in_or_out, float duration) {
        this.stream.DoFade(in_or_out, duration / 1000f);
    }


    public void SetVolume(float volume) {
        this.stream.SetVolume(volume);
    }

    public void SetMute(bool muted) {
        this.is_muted = muted;
        this.stream.Mute(muted);
    }

    public void Seek(double timestamp) {
        this.stream.Seek(timestamp / 1000.0);
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
        return this.stream.IsActive;
    }

    public Fading HasFading() {
        return (Fading)this.stream.ActiveFade;
    }

    public double GetDuration() {
        return this.stream.Duration * 1000.0;
    }

    public double GetPosition() {
        return this.stream.Position * 1000.0;
    }

    public bool HasEnded() {
        return this.stream.HasEnded;
    }

}
