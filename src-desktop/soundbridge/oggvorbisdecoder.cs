using System;
using System.IO;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Engine.Externals.SoundBridge.Interop;

namespace Engine.Externals.SoundBridge;

internal unsafe class OGGVorbisDecoder : IDecoder {
    private OggVorbis_File* vf;
    private int* current_section;
    private nint src_hnd;
    private double duration_seconds;
    private uint channels, rate;
    private long loop_start, loop_length;

    private OGGVorbisDecoder() { }

    public static OGGVorbisDecoder Init(ISourceHandle src_hnd) {
        OGGVorbisDecoder oggvorbisdecoder = new OGGVorbisDecoder();

        // allocate "OggVorbis_File" and "int" with a single call
        void* ptr = NativeMemory.Alloc((nuint)(sizeof(OggVorbis_File) + sizeof(int)));

        oggvorbisdecoder.current_section = (int*)ptr;
        oggvorbisdecoder.vf = (OggVorbis_File*)(oggvorbisdecoder.current_section + 1);

        *oggvorbisdecoder.current_section = 0;

        ov_callbacks callbacks = new ov_callbacks() {
            read_func = &read_func,
            seek_func = &seek_func,
            tell_func = &tell_func,
            close_func = null
        };

        GCHandle gchandle = GCHandle.Alloc(src_hnd, GCHandleType.Normal);
        oggvorbisdecoder.src_hnd = GCHandle.ToIntPtr(gchandle);

        if (Vorbisfile.ov_open_callbacks(oggvorbisdecoder.src_hnd, oggvorbisdecoder.vf, null, 0, &callbacks) != 0) {
            NativeMemory.Free(ptr);
            gchandle.Free();
            return null;
        }


        vorbis_info* vi = Vorbisfile.ov_info(oggvorbisdecoder.vf, -1);
        oggvorbisdecoder.rate = (uint)vi->rate;
        oggvorbisdecoder.channels = (uint)vi->channels;

        oggvorbisdecoder.duration_seconds = Vorbisfile.ov_time_total(oggvorbisdecoder.vf, -1);

        oggvorbisdecoder.loop_start = 0;
        oggvorbisdecoder.loop_length = 0;

        // find loop points
        vorbis_comment* comments_info = Vorbisfile.ov_comment(oggvorbisdecoder.vf, -1);
        for (int i = 0 ; i < comments_info->comments ; i++) {
            int length = comments_info->comment_lengths[i];
            string comment = Marshal.PtrToStringUTF8((nint)comments_info->user_comments[i], length);

            if (length < 1 || comment == null) continue;

            if (comment.StartsWith(OggUtil.LOOPSTART)) {
                Int64.TryParse(comment.AsSpan(OggUtil.LOOPSTART.Length), out oggvorbisdecoder.loop_start);
            }

            if (comment.StartsWith(OggUtil.LOOPLENGTH)) {
                Int64.TryParse(comment.AsSpan(OggUtil.LOOPLENGTH.Length), out oggvorbisdecoder.loop_length);
            }
        }

        return oggvorbisdecoder;
    }

    public void Dispose() {
        Vorbisfile.ov_clear(this.vf);
        NativeMemory.Free(this.current_section); // this.vf is also free'd too
        GCHandle.FromIntPtr(this.src_hnd).Free();
    }


    public int Read(float* buffer, uint samples_per_channel) {
        uint channels = this.channels;
        float** pcm_channels = null;

        int readed = Vorbisfile.ov_read_float(
            this.vf, &pcm_channels, (int)samples_per_channel, this.current_section
        );

        int offset = 0;
        for (int i = 0 ; i < readed ; i++) {
            for (uint j = 0 ; j < channels ; j++) {
                buffer[offset++] = pcm_channels[j][i];
            }
        }

        return readed;
    }

    public SampleFormat GetInfo(out uint rate, out uint channels, out double duration) {
        rate = this.rate;
        channels = this.channels;
        duration = this.duration_seconds;
        return SampleFormat.FLOAT32;
    }

    public bool Seek(double seconds) {
        return Vorbisfile.ov_pcm_seek(this.vf, (long)(seconds * this.rate)) != 0;
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
    private static nint read_func(void* ptr, nint size, nint nmemb, nint datasource) {
        ISourceHandle src_hnd = recover(datasource);
        return src_hnd.Read(ptr, (int)(size * nmemb));
    }

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static int seek_func(nint datasource, long offset, int whence) {
        ISourceHandle src_hnd = recover(datasource);
        return src_hnd.Seek(offset, (SeekOrigin)whence);
    }

    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    private static int tell_func(nint datasource) {
        ISourceHandle src_hnd = recover(datasource);
        return (int)src_hnd.Tell();
    }

}



