using System;
using System.Runtime.InteropServices;

namespace Engine.Externals;

[StructLayout(LayoutKind.Sequential)]
internal struct FFGraphInfo {
    public readonly bool audio_has_stream;
    public readonly bool video_has_stream;

    public readonly int audio_channels;
    public readonly int audio_sample_rate;

    public readonly int video_width;
    public readonly int video_height;

    public readonly double video_seconds_duration;
    public readonly double audio_seconds_duration;
}

internal static class FFgraph {
    private const string DLL = "kdy_FFgraph";


    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr ffgraph_init(IntPtr video_filehandle, IntPtr audio_filehandle);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void ffgraph_destroy(IntPtr ffgraph);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void ffgraph_get_streams_info(IntPtr ffgraph, ref FFGraphInfo output_info);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int ffgraph_read_audio_samples(IntPtr ffgraph, [Out] float[] out_samples, int max_samples_per_channel);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern double ffgraph_read_video_frame(IntPtr ffgraph, out IntPtr out_frame, out int out_frame_size);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern double ffgraph_read_video_frame2(IntPtr ffgraph, IntPtr buffer, int buffer_size);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void ffgraph_seek(IntPtr ffgraph, double time_in_seconds);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void ffgraph_seek2(IntPtr ffgraph, double time_in_seconds, bool audio_or_video);


    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr ffgraph_sndbridge_create_helper(IntPtr ffgraph, bool allow_seek, bool allow_destroy);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void ffgraph_sndbridge_destroy_helper(IntPtr ffgraph_sndbridge);
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl, EntryPoint = "ffgraph_get_runtime_info")]
    private static extern IntPtr __ffgraph_get_runtime_info();

    public static string ffgraph_get_runtime_info() {
        try {
            IntPtr ptr = __ffgraph_get_runtime_info();
            return Marshal.PtrToStringAnsi(ptr);
        } catch (DllNotFoundException) {
            return null;
        } catch (EntryPointNotFoundException) {
            return null;
        }
    }

}

