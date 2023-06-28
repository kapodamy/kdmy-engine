using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Engine.Externals.SoundBridge.Interop;

namespace Engine.Externals.SoundBridge;

using CbTimeInfo = PaStreamCallbackTimeInfo;
using CbFlags = PaStreamCallbackFlags;


public enum StreamFading : int {
    NONE = 0,
    IN = 1,
    OUT = 2
}

public enum StreamResult : int {
    Success = 0,
    DecoderFailed = -2,
    CreateFailed = -3,
    BackendFailed = -4
}


public class Stream : IDisposable {
    internal int id;
    internal IDecoder decoder;
    internal uint bytes_per_sample_per_channel;
    internal uint channels;
    internal uint sample_rate;

    internal nint pastream;

    internal volatile float volume;
    internal volatile bool muted;
    internal volatile bool looped;
    internal volatile bool completed;
    internal volatile bool fetching;
    internal volatile bool keep_alive;
    internal volatile bool halt;
    internal volatile bool callback_running;
    internal volatile bool looped_needs_time_reset;

    internal double last_sample_time;
    internal double played_time;
    internal double duration_seconds;
    internal volatile uint sample_position;

    internal int fade_duration;
    internal int fade_progress;
    internal bool fade_in_or_out;

    internal bool has_loop_points;
    internal uint loop_end_samples;
    internal uint loop_start_samples;
    internal double loop_start_seconds;

    internal uint buffer_used;
    internal unsafe float* buffer;

    private readonly GCHandle gchandle;

    internal Stream() {
        gchandle = GCHandle.Alloc(this, GCHandleType.Normal);
    }


    public double Duration {
        get
        {
            return this.duration_seconds;
        }
    }

    public double Position {
        get
        {
            double position = this.played_time;
            int err = PortAudio.Pa_IsStreamStopped(this.pastream);

            switch (err) {
                case 1:
                    break;
                case 0:
                    if (!this.completed && this.fetching && !this.halt) {
                        position += PortAudio.Pa_GetStreamTime(this.pastream) - this.last_sample_time;
                    }
                    break;
                default:
                    SoundBridge.PrintPaError($"sndbridge_position() failed on stream {this.id}: ", (PaError)err);
                    return -1.0;
            }

            // if (position >= stream.duration) return stream.duration;
            return position;
        }
    }


    public void Seek(double seconds) {
        if (seconds < 0.0 || !Double.IsFinite(seconds)) seconds = 0.0;

        PaError ret;

        int status = PortAudio.Pa_IsStreamActive(this.pastream);
        if (status != 1 && status != 0) {
            SoundBridge.PrintPaError($"sndbridge_seek() status failed on stream {this.id}: ", (PaError)status);
            return;
        }

        if (status == 1 && !this.keep_alive) {
            ret = PortAudio.Pa_AbortStream(this.pastream);
            if (ret != PaError.paNoError) {
                SoundBridge.PrintPaError($"sndbridge_seek() failed to abort stream {this.id}: ", ret);
            }
        }

        if (this.keep_alive) SoundBridge.wait_and_halt(this, status == 1);

        this.buffer_used = 0;
        this.played_time = seconds;
        this.looped_needs_time_reset = false;
        this.sample_position = (uint)(this.played_time * this.sample_rate);

        this.decoder.Seek(seconds);

        if (seconds >= this.duration_seconds) {
            this.completed = true;
            this.fetching = true;
            return;
        }

        this.completed = false;
        this.fetching = false;

        if (status == 1) {
            if (this.keep_alive) {
                this.halt = false;
                return;
            }

            ret = PortAudio.Pa_StartStream(this.pastream);
            if (ret != PaError.paNoError) {
                SoundBridge.PrintPaError($"sndbridge_seek() failed to resume stream {this.id}: ", ret);
            }
        }
    }

    public void Play() {
        int err = PortAudio.Pa_IsStreamActive(this.pastream);
        switch (err) {
            case 1:
                if (this.halt) {
                    this.Stop();
                    this.halt = false;
                    this.looped_needs_time_reset = false;
                }
                return;
            case 0:
                break;
            default:
                SoundBridge.PrintPaError($"sndbridge_play() status failed on stream {this.id}: ", (PaError)err);
                return;
        }

        // seek to the beginning if the playback has ended
        if (this.played_time >= this.duration_seconds || this.completed) {
            this.Stop();
        }

        this.fade_duration = 0;
        this.buffer_used = 0;
        this.fetching = false;
        this.completed = false;
        this.buffer_used = 0;
        this.halt = false;

        PaError ret = PortAudio.Pa_StartStream(this.pastream);
        if (ret != PaError.paNoError) {
            SoundBridge.PrintPaError($"sndbridge_play() start failed on stream {this.id}: ", ret);
            return;
        }
    }

    public void Pause() {
        int err = PortAudio.Pa_IsStreamStopped(this.pastream);
        switch (err) {
            case 1:
                return;
            case 0:
                if (this.halt) return;
                break;
            default:
                SoundBridge.PrintPaError($"sndbridge_pause() status failed on stream {this.id}: ", (PaError)err);
                return;
        }

        if (this.fetching) {
            this.played_time += PortAudio.Pa_GetStreamTime(this.pastream) - this.last_sample_time;
        }

        if (!this.keep_alive) {
            PaError ret = PortAudio.Pa_AbortStream(this.pastream);
            if (ret != PaError.paNoError) {
                SoundBridge.PrintPaError($"sndbridge_pause() stop failed on stream {this.id}: ", ret);
            }
        }

        if (this.keep_alive) SoundBridge.wait_and_halt(this, true);
        this.buffer_used = 0;
        this.fade_duration = 0;
        this.fetching = false;
        this.completed = false;
        this.looped_needs_time_reset = false;
        this.sample_position = (uint)(this.played_time * this.sample_rate);
        this.decoder.Seek(this.played_time);
    }

    public void Stop() {
        int err = PortAudio.Pa_IsStreamStopped(this.pastream);
        switch (err) {
            case 1:
                goto L_reset_stream;
            case 0:
                if (this.keep_alive) goto L_reset_stream;
                break;
            default:
                SoundBridge.PrintPaError("sndbridge_stop() status failed on stream {this.id}: ", (PaError)err);
                return;
        }

        PaError ret = PortAudio.Pa_AbortStream(this.pastream);
        if (ret != PaError.paNoError) {
            SoundBridge.PrintPaError("sndbridge_stop() close failed on stream {this.id}: ", ret);
            return;
        }

L_reset_stream:
        if (this.keep_alive) SoundBridge.wait_and_halt(this, true);
        this.buffer_used = 0;
        this.played_time = 0.0;
        this.fade_duration = 0;
        this.completed = false;
        this.fetching = false;
        this.looped_needs_time_reset = false;
        this.sample_position = 0;
        this.decoder.Seek(0.0);
    }


    public void SetVolume(float volume) {
        this.volume = volume;
        this.fade_duration = 0;
    }

    public void Mute(bool muted) {
        this.muted = muted;
        this.fade_duration = 0;
    }

    public void DoFade(bool fade_in_or_out, float seconds) {
        if (seconds <= Single.Epsilon) {
            // cancel any active fade
            this.fade_duration = 0;
            return;
        }

        this.fade_in_or_out = fade_in_or_out;
        this.fade_progress = 0;
        this.fade_duration = (int)(seconds * this.sample_rate);
    }

    public bool IsActive {
        get
        {
            if (this.keep_alive) return !this.halt;

            return PortAudio.Pa_IsStreamActive(this.pastream) == 1;
        }
    }

    public StreamFading ActiveFade {
        get
        {
            if (PortAudio.Pa_IsStreamActive(this.pastream) != 1) return StreamFading.NONE;

            if (this.fade_duration < 1) return StreamFading.NONE;

            return this.fade_in_or_out ? StreamFading.IN : StreamFading.OUT;
        }
    }

    public bool HasEnded { get => this.completed; }

    public void SetLooped(bool enable) {
        this.looped = enable;
    }


    public void Dispose() {
        if (this.pastream != 0x00) {
            PortAudio.Pa_CloseStream(this.pastream);
        }

        unsafe {
            NativeMemory.Free(this.buffer);
            this.buffer = null;
        }

        this.decoder.Dispose();
        this.gchandle.Free();

        SoundBridge.stream_count--;

        // SoundBridge.DisposeBackend();
    }


    public static implicit operator nint(Stream stream) {
        return GCHandle.ToIntPtr(stream.gchandle);
    }

}

public static class SoundBridge {

    private const uint SAMPLE_BUFFER_SIZE = (8 * 1024 * 2);
    private const uint BUFFER_SIZE_BYTES = (SAMPLE_BUFFER_SIZE * sizeof(float));
    private const int HOLE = -3; // retured by ov_read and op_read, means bad/missing packets in the ogg
    private const float PI_HALF = MathF.PI / 2f;
    private const string DESIRED_API_NAME = "DSound";
    private const PaHostApiTypeId DESIRED_API_ENUM = PaHostApiTypeId.paDirectSound;
    private const float MIN_DURATION_KEEP_ALIVE = 99990.500f; // 500 milliseconds
    private const int WAIT_AND_HALT_TIMEOUT = 1000; // 1000 milliseconds


    private static volatile int STREAM_IDS;
    private static volatile bool backend_needs_initialize;
    private static volatile float master_volume;
    private static volatile bool master_mute;
    internal static volatile uint stream_count;


    static SoundBridge() {
        backend_needs_initialize = true;
        STREAM_IDS = 0;
        master_volume = 1f;
        master_mute = false;
    }


    internal static void wait_and_halt(Stream stream, bool halt) {
        int elapsed = 0;
        while (stream.callback_running && elapsed < SoundBridge.WAIT_AND_HALT_TIMEOUT) {
            PortAudio.Pa_Sleep(10);
            elapsed += 10;
        }
        stream.halt = halt;
    }

    internal static void PrintPaError(string str, PaError err) {
        Logger.Error($"soundbridge: {str}{PortAudio.Pa_GetErrorText(err)}");
    }


    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static unsafe void completed_cb(nint userdata) {
        Stream stream = (Stream)GCHandle.FromIntPtr(userdata).Target;
        stream.completed = true;
        stream.callback_running = false;
        stream.played_time += PortAudio.Pa_GetStreamTime(stream.pastream) - stream.last_sample_time;

#if DEBUG && SNDBRIDGE_DEBUG_COMPLETE
        double position = (PortAudio.Pa_GetStreamTime(stream.pastream) - stream.last_sample_time);
        Logger.Info($"sndbridge: END stream_id={stream.id} position_ms={position * 1000.0}");
#endif
    }

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static unsafe PaStreamCallbackResult read_cb(void* _i, void* output, uint frameCount, CbTimeInfo* ti, CbFlags _f, nint userdata) {
        Stream stream = (Stream)GCHandle.FromIntPtr(userdata).Target;
        float* output_float = (float*)output;

        stream.callback_running = true;

        if (stream.halt) {
            if (stream.looped_needs_time_reset) {
                stream.looped_needs_time_reset = false;
                stream.played_time += stream.last_sample_time;
            }

            // playback ended, silence output to keep alive the stream
            NativeMemory.Clear(output, frameCount * stream.bytes_per_sample_per_channel);
            stream.callback_running = false;
            return PaStreamCallbackResult.paContinue;
        }

        // read samples
        uint readed_frames = SoundBridge.processing_read_samples(stream, frameCount, output_float);

        if (stream.muted || SoundBridge.master_mute) {
            NativeMemory.Clear(output, frameCount * stream.bytes_per_sample_per_channel);
            goto L_prepare_return;
        }

        // change volume if required
        SoundBridge.processing_change_volume(stream, frameCount, output_float);

        // do fade if required
        SoundBridge.processing_apply_fade(stream, frameCount, output_float);

L_prepare_return:
        stream.callback_running = false;

        if (readed_frames < frameCount) {
            if (!stream.keep_alive) return PaStreamCallbackResult.paComplete;

            if (frameCount > 0) frameCount--;

            stream.completed = true;
            stream.halt = true;
            stream.fetching = false;
            stream.looped_needs_time_reset = true;

            stream.played_time += ti->outputBufferDacTime - stream.last_sample_time;
            stream.last_sample_time = (double)frameCount / stream.sample_rate;

            return PaStreamCallbackResult.paContinue;
        }

        if (!stream.fetching) {
            stream.fetching = true;
            stream.last_sample_time = ti->outputBufferDacTime;
        }

        return PaStreamCallbackResult.paContinue;
    }


    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    private static float calc_s_curve(float percent) {
        if (percent <= 0f) return 0f;
        if (percent >= 1f) return 1f;
        return (MathF.Sin((MathF.PI * percent) - PI_HALF) / 2.0f) + 0.5f;
    }




    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    private static unsafe uint processing_read_samples(Stream stream, uint frameCount, float* output) {
        //Debug.Assert(frameCount < SoundBridge.SAMPLE_BUFFER_SIZE);

        if (stream.looped_needs_time_reset) {
            stream.played_time = stream.loop_start_seconds;// this is zero if loop points are absent
            stream.looped_needs_time_reset = false;
        }

        // check if necessary refill the internal buffer;
        uint buffer_used = stream.buffer_used;
        if (buffer_used >= frameCount) goto L_copy_and_return;

        uint available_space = (SoundBridge.SAMPLE_BUFFER_SIZE / stream.channels) - buffer_used;
        uint minimun_can_read = 64 * stream.channels;
        float* buf = stream.buffer + (buffer_used * stream.channels);

        while (available_space > minimun_can_read) {
            int readed = stream.decoder.Read(buf, available_space);
            if (readed < 1) {
                if (readed == SoundBridge.HOLE) continue;

                // EOF reached, if looped seek to the start
                if (stream.looped) {
                    if (stream.has_loop_points) {
                        // this never should happen
                        stream.decoder.Seek(stream.loop_start_seconds);
                        stream.sample_position = stream.loop_start_samples;
                        stream.fetching = false;
                        stream.looped_needs_time_reset = true;
                        continue;
                    }

                    stream.decoder.Seek(0.0);
                    stream.fetching = false;
                    stream.looped_needs_time_reset = true;
                    continue;
                }
                break;
            }

            // check loop points
            if (stream.looped && stream.has_loop_points && readed > 0) {
                uint total_readed = stream.sample_position + (uint)readed;

                if (total_readed < stream.loop_end_samples) {
                    stream.sample_position = total_readed;
                } else {
                    // discard last samples
                    int to_discard = (int)(total_readed - stream.loop_end_samples);
                    if (to_discard > 0) {
                        if (to_discard > readed)
                            readed = 0;
                        else
                            readed -= to_discard;
                    }

                    // seek to the loop start position
                    stream.decoder.Seek(stream.loop_start_seconds);
                    stream.fetching = false;
                    stream.sample_position = stream.loop_start_samples;
                    stream.looped_needs_time_reset = true;
                }
            }

            buf += readed * stream.channels;
            available_space -= (uint)readed;
            buffer_used += (uint)readed;
        }

        if (buffer_used < frameCount) frameCount = buffer_used;

L_copy_and_return:
        buffer_used -= frameCount;

        uint required_bytes = frameCount * stream.bytes_per_sample_per_channel;
        uint remaining_bytes = buffer_used * stream.bytes_per_sample_per_channel;
        stream.buffer_used = buffer_used;

        NativeMemory.Copy(stream.buffer, output, required_bytes);
        NativeMemory.Copy(required_bytes + (byte*)stream.buffer, stream.buffer, remaining_bytes);

        return frameCount;
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    private static unsafe void processing_apply_fade(Stream stream, uint frameCount, float* output) {
        if (stream.fade_duration < 1) return;

        uint channels = stream.channels;
        float fade_duration = stream.fade_duration;
        int fade_progress = stream.fade_progress;

        int remaining = stream.fade_duration - stream.fade_progress;
        bool has_short_fade = remaining < frameCount;
        uint to_process = has_short_fade ? (uint)remaining : frameCount;
        int offset = 0;

        if (stream.fade_in_or_out) {
            for (uint s = 0 ; s < to_process ; s++) {
                float percent = fade_progress / fade_duration;
#if SNDBRIDGE_SCURVE
                percent = calc_s_curve(percent);
#endif
                fade_progress++;
                for (int c = 0 ; c < channels ; c++) output[offset++] *= percent;
            }
        } else {
            for (uint s = 0 ; s < to_process ; s++) {
                float percent = 1f - (fade_progress / fade_duration);
#if SNDBRIDGE_SCURVE
                percent = calc_s_curve(percent);
#endif
                fade_progress++;
                for (int c = 0 ; c < channels ; c++) output[offset++] *= percent;
            }

            if (has_short_fade) {
                // silence the remaining samples
                uint remaining_bytes = (frameCount - to_process) * stream.bytes_per_sample_per_channel;
                uint silence_offset = to_process * stream.bytes_per_sample_per_channel;
                NativeMemory.Clear((byte*)output + silence_offset, remaining_bytes);
            }
        }

        if (fade_progress >= stream.fade_duration) {
            stream.fade_duration = 0;
            if (!stream.fade_in_or_out) stream.volume = 0f;
        } else {
            stream.fade_progress = fade_progress;
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    private static unsafe void processing_change_volume(Stream stream, uint frameCount, float* output) {
        if (stream.volume == 1f && SoundBridge.master_volume == 1f) return;

        float final_volume = stream.volume * SoundBridge.master_volume;
        uint total_samples = frameCount * stream.channels;

        for (uint i = 0 ; i < total_samples ; i++) {
            output[i] *= final_volume;
        }
    }


    private static unsafe void CreatePaStream(Stream stream) {
        // step 1: adquire default output device
        bool has_desired = false;
        PaError err = PaError.paNoError;
        int output_device_index = PortAudio.Pa_GetDefaultOutputDevice();
        nint pastream = 0x00;

        stream.pastream = 0x00;

        // attempt to use the desired api (DESIRED_API_ENUM)
        int count = PortAudio.Pa_GetHostApiCount();
        for (int index = 0 ; index < count ; index++) {
            PaHostApiInfo* info = PortAudio.Pa_GetHostApiInfo(index);

            if (info->type != SoundBridge.DESIRED_API_ENUM) continue;
            if (info->defaultOutputDevice != PortAudio.paNoDevice) {
                has_desired = true;
                output_device_index = info->defaultOutputDevice;
            }
            break;
        }

L_check_device:
        if (output_device_index == PortAudio.paNoDevice) {
            /*if (has_desired) {
                has_desired = false;
                output_device_index = PortAudio.Pa_GetDefaultOutputDevice();
                Logger.Warn($"soundbridge: no output devices available for {DESIRED_API_NAME}");
                goto L_check_device;
            }*/
            Logger.Warn("soundbridge: no output devices available");
            return;
        }

        // step 2: obtain minimal latency
        PaDeviceInfo* device_info = PortAudio.Pa_GetDeviceInfo(output_device_index);
        //Debug.Assert(device_info != null);
        double latency = device_info->defaultLowOutputLatency;

        // step 3: prepare stream
        PaStreamParameters output_params = new PaStreamParameters() {
            channelCount = (int)stream.channels,
            device = output_device_index,
            hostApiSpecificStreamInfo = null,
            sampleFormat = PaSampleFormat.paFloat32,
            suggestedLatency = latency
        };

        PaWasapiStreamInfo stream_info = new PaWasapiStreamInfo() {
            hostApiType = PaHostApiTypeId.paWASAPI,
            version = 1,
            size = (uint)sizeof(PaWasapiStreamInfo),
            flags = PaWasapiFlags.paWinWasapiThreadPriority | PaWasapiFlags.paWinWasapiAutoConvert,
            threadPriority = PaWasapiThreadPriority.eThreadPriorityGames,
            streamCategory = PaWasapiStreamCategory.eAudioCategoryGameMedia
        };
        if (DESIRED_API_ENUM == PaHostApiTypeId.paWASAPI && has_desired) {
            output_params.hostApiSpecificStreamInfo = &stream_info;
        }

        // step 4: check if the format is supported
        err = PortAudio.Pa_IsFormatSupported(null, &output_params, stream.sample_rate);
        if (err != PaError.paNoError) {

            PrintPaError(
                $"Pa_IsFormatSupported() failed. sample_rate={stream.sample_rate} channels={stream.channels} latency={latency} api={DESIRED_API_ENUM}\n",
                err
            );

            if (has_desired) {
                // not supported format, try again using the default API and output device
                has_desired = false;
                output_device_index = PortAudio.Pa_GetDefaultOutputDevice();
                goto L_check_device;
            }

            return;
        }

        // step 5: open the stream
        err = PortAudio.Pa_OpenStream(
            &pastream,
            null, &output_params, stream.sample_rate,
            PortAudio.paFramesPerBufferUnspecified, PaStreamFlags.paNoFlag,
            &read_cb, (nint)stream
        );

        if (err == PaError.paNoError) {
            stream.pastream = pastream;
            PortAudio.Pa_SetStreamFinishedCallback(pastream, &completed_cb);
            return;
        }

        string dev_name = Marshal.PtrToStringUTF8((nint)device_info->name);
        PrintPaError(
            $"Pa_OpenStream() failed. sample_rate={stream.sample_rate} channels={stream.channels} latency={latency} device={dev_name}",
            err
        );

        if (has_desired) {
            // open failed, last attempt with the default audio API and output device
            has_desired = false;
            output_device_index = PortAudio.Pa_GetDefaultOutputDevice();
            goto L_check_device;
        }
    }

    internal static void DisposeBackend() {
        if (SoundBridge.stream_count > 0) return;
        PortAudio.Pa_Terminate();
        SoundBridge.backend_needs_initialize = true;
    }


    public static StreamResult Enqueue(IFileSource ogg_filehandle, out Stream stream) {
        IDecoder oggdecoder = OggUtil.InitOggDecoder(ogg_filehandle);
        if (oggdecoder == null) {
            stream = null;
            return StreamResult.DecoderFailed;
        }

        StreamResult res = SoundBridge.Enqueue(oggdecoder, out stream);

        if (res != StreamResult.Success) {
            oggdecoder.Dispose();
        }

        return res;
    }

    public static unsafe StreamResult Enqueue(IDecoder external_decoder, out Stream stream) {
        stream = null;

        if (backend_needs_initialize) {
            PaError err = PortAudio.Pa_Initialize();
            if (err != PaError.paNoError) return StreamResult.BackendFailed;
            SoundBridge.backend_needs_initialize = false;
        }

        uint sample_rate, channels;
        double duration;
        external_decoder.GetInfo(out sample_rate, out channels, out duration);

        stream = new Stream() {
            decoder = external_decoder,
            bytes_per_sample_per_channel = channels * sizeof(float),
            channels = channels,
            sample_rate = sample_rate,

            pastream = 0x00,

            volume = 1f,
            muted = false,
            looped = false,

            completed = false,
            fetching = false,
            keep_alive = duration < MIN_DURATION_KEEP_ALIVE,
            halt = false,
            callback_running = false,
            looped_needs_time_reset = false,

            last_sample_time = 0.0,
            played_time = 0.0,
            duration_seconds = duration,
            sample_position = 0,

            fade_duration = 0,
            fade_in_or_out = false,
            fade_progress = 0,

            has_loop_points = true,
            loop_end_samples = 0,
            loop_start_samples = 0,
            loop_start_seconds = 0.0,

            buffer_used = 0,
            buffer = (float*)NativeMemory.Alloc(SAMPLE_BUFFER_SIZE * sizeof(float))
        };

        SoundBridge.CreatePaStream(stream);
        if (stream.pastream == 0x00) {
            stream.Dispose();
            return StreamResult.CreateFailed;
        }

        long loop_start, loop_length;
        external_decoder.GetLoopPoints(out loop_start, out loop_length);

        // check if the loop points are valid
        if (loop_start < 0 || loop_start == loop_length) {
            stream.has_loop_points = false;
        } else {
            stream.loop_start_samples = (uint)loop_start;

            if (loop_length < 0)
                stream.loop_end_samples = (uint)(stream.duration_seconds * stream.sample_rate);
            else
                stream.loop_end_samples = (uint)(loop_length + loop_start);
        }

        if (stream.has_loop_points) {
            stream.loop_start_seconds = stream.loop_start_samples / (double)stream.sample_rate;
            stream.looped = true;// enable looping by default
            stream.has_loop_points = true;
        }

        stream.id = STREAM_IDS++;

        return StreamResult.Success;
    }


    public static void SetMasterVolume(float volume) {
        SoundBridge.master_volume = MathF.Max(-1f, MathF.Min(volume, 1f));
    }

    public static void SetMasterMuted(bool muted) {
        SoundBridge.master_mute = muted;
    }


    public static string GetRuntimeInfo() {
        if (backend_needs_initialize) {
            backend_needs_initialize = false;

#pragma warning disable CS0162 // Se detectó código inaccesible
            if (SoundBridge.DESIRED_API_ENUM == PaHostApiTypeId.paWASAPI) {
                WASAPI.TryEnableAutomaticStreamRouting();
            }
#pragma warning restore CS0162 // Se detectó código inaccesible

            PaError err = PortAudio.Pa_Initialize();
            if (err != PaError.paNoError) {
                return Marshal.PtrToStringUTF8(PortAudio.Pa_GetErrorText(err));
            }
        }

        int default_api_index = PortAudio.Pa_GetDefaultHostApi();
        int count = PortAudio.Pa_GetHostApiCount();

        unsafe {
            string desired_api = null;
            char* default_name_ptr = null;
            char* device_name_ptr = null;

            PaHostApiInfo* default_api_info = PortAudio.Pa_GetHostApiInfo(default_api_index);
            if (default_api_info != null) {
                default_name_ptr = default_api_info->name;
                if (default_api_info->defaultOutputDevice != PortAudio.paNoDevice) {
                    device_name_ptr = PortAudio.Pa_GetDeviceInfo(default_api_info->defaultOutputDevice)->name;
                }
            }

            for (int index = 0 ; index < count ; index++) {
                PaHostApiInfo* info = PortAudio.Pa_GetHostApiInfo(index);
                if (info->type != DESIRED_API_ENUM) continue;

                desired_api = DESIRED_API_NAME; // info->name;
                if (info->defaultOutputDevice != PortAudio.paNoDevice) {
                    device_name_ptr = PortAudio.Pa_GetDeviceInfo(info->defaultOutputDevice)->name;
                }
                break;
            }

            string default_name = Marshal.PtrToStringUTF8((nint)default_name_ptr);
            string device_name = Marshal.PtrToStringUTF8((nint)device_name_ptr);
            string version = Marshal.PtrToStringUTF8(PortAudio.Pa_GetVersionText());

            return $"{version}\nAudio API: default={default_name} desired={desired_api} output={device_name}";
        }
    }


#if DEBUG
    public static void Main() {
        Logger.Info(SoundBridge.GetRuntimeInfo());

        Stream stream;
        StreamResult enqueue_ret;

        /*IFileHandle mem = FileHandleUtil.Init("./vorbis.ogg", true);
        Debug.Assert(mem);
        enqueue_ret = SoundBridge.Enqueue(mem);*/

        // FileHandle mem = FileHandleUtil.Init("./vorbis.ogg", true);
        IFileSource mem = FileHandleUtil.Init("./goldgrab.logg", true);
        enqueue_ret = SoundBridge.Enqueue(mem, out stream);

        if (enqueue_ret != StreamResult.Success) return;

        /*
        bool a = true;
        bool b = true;
        bool c = true;
        bool d = true;
        stream.Play();
        while (true) {
            double duration = stream.Duration;
            double position = stream.Position();
            Logger.Log($"duration={duration * 1000}  time={position * 1000}");
            PortAudio.Pa_Sleep(100);
            //if (i == 6) stream.Pause();
            //if (i == 10) stream.Play();
            if (position >= (1.000 + duration)) break;
            if (position >= 5.000 && a) {
                a = false;
                stream.Seek(10.000);
            }
            if (position > 3.000 && b) {
                stream.Pause();
                PortAudio.Pa_Sleep(1000);

                duration = stream.Duration;
                position = stream.Position();
                Logger.Log($"(paused) duration={duration * 1000}  time={position * 1000}");
                stream.Play();
                b = false;
            }
            if (position > 13.000 && c) {
                stream.Pause();
                PortAudio.Pa_Sleep(1000);

                duration = stream.Duration;
                position = stream.Position();
                Logger.Log($"(paused) duration={duration * 1000}  time={position * 1000}");
                stream.Play();
                c = false;
            }
            if (position > 17.000 && d) {
                stream.Stop();
                PortAudio.Pa_Sleep(1000);

                duration = stream.Duration;
                position = stream.Position();
                Logger.Log($"(stop) duration={duration * 1000}  time={position * 1000}");
                stream.Play();
                d = false;
            }
        }
        */

        /*while (true) {
            sndbridge_play(stream_id);
            Pa_Sleep(2000);
            sndbridge_pause(stream_id);
            Pa_Sleep(2000);
            //sndbridge_seek(stream_id, 0.0);
        }*/

        //stream.SetLooped(false);
        //stream.Stop();
        stream.Play();
        //PortAudio.Pa_Sleep(3000);
        //stream.Seek(1.000);


        int total = 0;
        while (total < 50000) {
            double duration = stream.Duration;
            double position = stream.Position;
            Logger.Log($"duration={duration * 1000}  time={position * 1000}");
            PortAudio.Pa_Sleep(100);
            total += 100;
            //if (stream.HasEnded) break;
        }

        Logger.Log("playback done");

        stream.Dispose();
    }

#endif

}

