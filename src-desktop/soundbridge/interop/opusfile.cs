using System;
using System.Runtime.InteropServices;

namespace Engine.Externals.SoundBridge.Interop;

using ogg_int64_t = Int64;
using opus_int64 = Int64;


[StructLayout(LayoutKind.Sequential)]
internal unsafe struct OpusTags {
    public char** user_comments;
    public int* comment_lengths;
    public int comments;
    public char* vendor;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct OpusFileCallbacks {
    public delegate* unmanaged[Cdecl]<nint, byte*, int, int> read;
    public delegate* unmanaged[Cdecl]<nint, opus_int64, int, int> seek;
    public delegate* unmanaged[Cdecl]<nint, opus_int64> tell;
    public delegate* unmanaged[Cdecl]<nint, int> close;
}

[StructLayout(LayoutKind.Sequential)]
internal readonly unsafe struct OggOpusFile { }


internal static unsafe class Opusfile {
    private const string DLL = "libopusfile-0";

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int op_pcm_seek(OggOpusFile* _of, ogg_int64_t _pcm_offset);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int op_read_float(OggOpusFile* _of, float* _pcm, int _buf_size, int* _li);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void op_free(OggOpusFile* _of);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern OggOpusFile* op_open_callbacks(nint _stream, OpusFileCallbacks* _cb, byte* _initial_data, nint _initial_bytes, int* _error);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern ogg_int64_t op_pcm_total(OggOpusFile* _of, int _li);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int op_channel_count(OggOpusFile* _of, int _li);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern OpusTags* op_tags(OggOpusFile* _of, int _li);

}
