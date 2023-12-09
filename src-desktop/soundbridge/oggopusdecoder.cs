using System;
using System.IO;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Engine.Externals.SoundBridge.Interop;

namespace Engine.Externals.SoundBridge;

internal unsafe class OGGOpusDecoder : IDecoder {
    private const int OPUS_RATE = 48000;

    private OggOpusFile* op;
    private uint bytes_per_channel, channels;
    private double duration;
    private long loop_start, loop_length;
    private nint src_hnd;

    private OGGOpusDecoder() { }

    public static OGGOpusDecoder Init(ISourceHandle src_hnd) {
        OGGOpusDecoder oggopusdecoder = new OGGOpusDecoder();

        OpusFileCallbacks callbacks = new OpusFileCallbacks() {
            read = &read_func,
            seek = &seek_func,
            tell = &tell_func,
            close = null
        };

        GCHandle gchandle = GCHandle.Alloc(src_hnd, GCHandleType.Normal);
        oggopusdecoder.src_hnd = GCHandle.ToIntPtr(gchandle);

        OggOpusFile* op = Opusfile.op_open_callbacks(oggopusdecoder.src_hnd, &callbacks, null, 0, null);
        if (op == null) {
            gchandle.Free();
            return null;
        }

        oggopusdecoder.op = op;
        oggopusdecoder.duration = Opusfile.op_pcm_total(oggopusdecoder.op, -1) * OGGOpusDecoder.OPUS_RATE;
        oggopusdecoder.channels = (uint)Opusfile.op_channel_count(oggopusdecoder.op, -1);
        oggopusdecoder.bytes_per_channel = oggopusdecoder.channels * 2;
        oggopusdecoder.loop_start = 0;
        oggopusdecoder.loop_length = 0;

        // find loop points
        OpusTags* comments_info = Opusfile.op_tags(oggopusdecoder.op, -1);
        for (int i = 0 ; i < comments_info->comments ; i++) {
            int length = comments_info->comment_lengths[i];
            string comment = Marshal.PtrToStringUTF8((nint)comments_info->user_comments[i], length);

            if (length < 1 || comment == null) continue;

            if (comment.StartsWith(OggUtil.LOOPSTART)) {
                Int64.TryParse(comment.AsSpan(OggUtil.LOOPSTART.Length), out oggopusdecoder.loop_start);
            }

            if (comment.StartsWith(OggUtil.LOOPLENGTH)) {
                Int64.TryParse(comment.AsSpan(OggUtil.LOOPLENGTH.Length), out oggopusdecoder.loop_length);
            }
        }

        return oggopusdecoder;
    }

    public void Dispose() {
        Opusfile.op_free(this.op);
        GCHandle.FromIntPtr(this.src_hnd).Free();
    }


    public int Read(float* buffer, uint samples_per_channel) {
        uint buf_size = samples_per_channel * this.channels;
        return Opusfile.op_read_float(this.op, buffer, (int)buf_size, null);
    }

    public SampleFormat GetInfo(out uint rate, out uint channels, out double duration) {
        rate = OGGOpusDecoder.OPUS_RATE;
        channels = this.channels;
        duration = this.duration;
        return SampleFormat.FLOAT32;
    }

    public bool Seek(double seconds) {
        long pcm_offset = (long)(seconds / OGGOpusDecoder.OPUS_RATE);
        return Opusfile.op_pcm_seek(this.op, pcm_offset) != 0;
    }

    public void GetLoopPoints(out long loop_start, out long loop_length) {
        loop_start = this.loop_start;
        loop_length = this.loop_length;
    }


    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    private static ISourceHandle recover(nint gchandle_ptr) {
        return (ISourceHandle)GCHandle.FromIntPtr(gchandle_ptr).Target;
    }


    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static int read_func(nint stream, byte* ptr, int nbytes) {
        ISourceHandle src_hnd = recover(stream);
        return src_hnd.Read(ptr, nbytes);
    }

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static int seek_func(nint stream, long offset, int whence) {
        ISourceHandle src_hnd = recover(stream);
        return src_hnd.Seek(offset, (SeekOrigin)whence);
    }

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static long tell_func(nint stream) {
        ISourceHandle src_hnd = recover(stream);
        return src_hnd.Tell();
    }

}

