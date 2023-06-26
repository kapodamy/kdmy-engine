using System.Runtime.InteropServices;
using System.Security.Cryptography;

///
/// Note: the following structures and enums are not perfectly ported
///
namespace Engine.Externals.FFGraphInterop.Interop;


[StructLayout(LayoutKind.Sequential)]
internal readonly struct SwsContext { }

[StructLayout(LayoutKind.Sequential)]
internal readonly struct SwsFilter { }

internal static unsafe partial class FFmpeg {

    private const string SWSCALE_DLL = "swscale-7";


    [DllImport(SWSCALE_DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int sws_scale(SwsContext* c, byte** srcSlice, int* srcStride, int srcSliceY, int srcSliceH, byte** dst, int* dstStride);

    [DllImport(SWSCALE_DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void sws_freeContext(SwsContext* swsContext);

    [DllImport(SWSCALE_DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern SwsContext* sws_getContext(int srcW, int srcH, AVPixelFormat srcFormat, int dstW, int dstH, AVPixelFormat dstFormat, int flags, SwsFilter* srcFilter, SwsFilter* dstFilter, double* param);

    [DllImport(SWSCALE_DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern uint swscale_version();

}
