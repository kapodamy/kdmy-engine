using System;
using System.Runtime.InteropServices;

namespace Engine.Externals.SoundBridge.Interop;

using ogg_int64_t = Int64;


[StructLayout(LayoutKind.Sequential)]
internal unsafe struct ogg_sync_state {
    readonly byte* data;
    readonly int storage;
    readonly int fill;
    readonly int returned;

    readonly int unsynced;
    readonly int headerbytes;
    readonly int bodybytes;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct ogg_stream_state {
    readonly byte* body_data;
    readonly int body_storage;
    readonly int body_fill;
    readonly int body_returned;


    readonly int* lacing_vals;
    readonly ogg_int64_t* granule_vals;

    readonly int lacing_storage;
    readonly int lacing_fill;
    readonly int lacing_packet;
    readonly int lacing_returned;

    fixed byte header[282];
    readonly int header_fill;

    readonly int e_o_s;
    readonly int b_o_s;
    readonly int serialno;
    readonly int pageno;
    readonly ogg_int64_t packetno;
    readonly ogg_int64_t granulepos;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct oggpack_buffer {
    readonly int endbyte;
    readonly int endbit;

    readonly byte* buffer;
    readonly byte* ptr;
    readonly int storage;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct vorbis_dsp_state {
    readonly int analysisp;
    readonly vorbis_info* vi;

    readonly float** pcm;
    readonly float** pcmret;
    readonly int pcm_storage;
    readonly int pcm_current;
    readonly int pcm_returned;

    readonly int preextrapolate;
    readonly int eofflag;

    readonly int lW;
    readonly int W;
    readonly int nW;
    readonly int centerW;

    readonly ogg_int64_t granulepos;
    readonly ogg_int64_t sequence;

    readonly ogg_int64_t glue_bits;
    readonly ogg_int64_t time_bits;
    readonly ogg_int64_t floor_bits;
    readonly ogg_int64_t res_bits;

    readonly void* backend_state;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct vorbis_block {
    readonly float** pcm;
    readonly oggpack_buffer opb;

    readonly int lW;
    readonly int W;
    readonly int nW;
    readonly int pcmend;
    readonly int mode;

    readonly int eofflag;
    readonly ogg_int64_t granulepos;
    readonly ogg_int64_t sequence;
    readonly vorbis_dsp_state* vd;

    readonly void* localstore;
    readonly int localtop;
    readonly int localalloc;
    readonly int totaluse;
    readonly void* reap;// struct alloc_chain*

    readonly int glue_bits;
    readonly int time_bits;
    readonly int floor_bits;
    readonly int res_bits;

    readonly void* @internal;
}


[StructLayout(LayoutKind.Sequential)]
internal unsafe struct ov_callbacks {
    public delegate* unmanaged[Cdecl]<void*, nint, nint, nint, nint> read_func;
    public delegate* unmanaged[Cdecl]<nint, ogg_int64_t, int, int> seek_func;
    public delegate* unmanaged[Cdecl]<nint, int> close_func;
    public delegate* unmanaged[Cdecl]<nint, int> tell_func;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct vorbis_comment {
    public char** user_comments;
    public int* comment_lengths;
    public int comments;
    public char* vendor;

}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct vorbis_info {
    readonly int version;
    public int channels;
    public int rate;

    readonly int bitrate_upper;
    readonly int bitrate_nominal;
    readonly int bitrate_lower;
    readonly int bitrate_window;

    readonly void* codec_setup;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct OggVorbis_File {
    readonly void* datasource;
    readonly int seekable;
    readonly ogg_int64_t offset;
    readonly ogg_int64_t end;
    readonly ogg_sync_state oy;

    readonly int links;
    readonly ogg_int64_t* offsets;
    readonly ogg_int64_t* dataoffsets;
    readonly int* serialnos;
    readonly ogg_int64_t* pcmlengths;
    readonly vorbis_info* vi;
    readonly vorbis_comment* vc;

    readonly ogg_int64_t pcm_offset;
    readonly int ready_state;
    readonly int current_serialno;
    readonly int current_link;

    readonly double bittrack;
    readonly double samptrack;

    readonly ogg_stream_state os;
    readonly vorbis_dsp_state vd;
    readonly vorbis_block vb;

    readonly ov_callbacks callbacks;
}

internal static unsafe class Vorbisfile {
    public const string DLL = "libvorbisfile-3";


    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int ov_clear(OggVorbis_File* vf);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int ov_open_callbacks(nint datasource, OggVorbis_File* vf, byte* initial, int ibytes, ov_callbacks* callbacks);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern double ov_time_total(OggVorbis_File* vf, int i);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int ov_pcm_seek(OggVorbis_File* vf, ogg_int64_t pos);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern vorbis_info* ov_info(OggVorbis_File* vf, int link);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern vorbis_comment* ov_comment(OggVorbis_File* vf, int link);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int ov_read_float(OggVorbis_File* vf, float*** pcm_channels, int samples, int* bitstream);

}
