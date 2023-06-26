using System.Runtime.InteropServices;

///
/// Note: the following structures and enums are not perfectly ported
///
namespace Engine.Externals.FFGraphInterop.Interop;


[StructLayout(LayoutKind.Sequential)]
internal readonly struct SwrContext { }

internal static unsafe partial class FFmpeg {

    private const string SWRESAMPLE_DLL = "swresample-4";


    public const int SWS_FAST_BILINEAR = 1;
    public const int SWS_BILINEAR = 2;
    public const int SWS_BICUBIC = 4;
    public const int SWS_X = 8;
    public const int SWS_POINT = 0x10;
    public const int SWS_AREA = 0x20;
    public const int SWS_BICUBLIN = 0x40;
    public const int SWS_GAUSS = 0x80;
    public const int SWS_SINC = 0x100;
    public const int SWS_LANCZOS = 0x200;
    public const int SWS_SPLINE = 0x400;


    [DllImport(SWRESAMPLE_DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern SwrContext* swr_alloc();

    [DllImport(SWRESAMPLE_DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int swr_alloc_set_opts2(SwrContext** ps, AVChannelLayout* out_ch_layout, AVSampleFormat out_sample_fmt, int out_sample_rate, AVChannelLayout* in_ch_layout, AVSampleFormat in_sample_fmt, int in_sample_rate, int log_offset, void* log_ctx);

    [DllImport(SWRESAMPLE_DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int swr_init(SwrContext* s);

    [DllImport(SWRESAMPLE_DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void swr_free(SwrContext** s);

    [DllImport(SWRESAMPLE_DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int swr_convert(SwrContext* s, byte** @out, int out_count, byte** @in, int in_count);

    [DllImport(SWRESAMPLE_DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern uint swresample_version();

}
