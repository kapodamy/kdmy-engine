using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

///
/// Note: the following structures and enums are not perfectly ported
///
namespace Engine.Externals.FFGraphInterop.Interop;

internal enum AVPictureType { }
internal enum AVColorRange { }
internal enum AVColorPrimaries { }
internal enum AVColorTransferCharacteristic { }
internal enum AVColorSpace { }
internal enum AVChromaLocation { }

internal enum AVPixelFormat {
    AV_PIX_FMT_NONE = -1,
    AV_PIX_FMT_YUV420P,
    AV_PIX_FMT_YUYV422,
    AV_PIX_FMT_RGB24,
    AV_PIX_FMT_BGR24,
    AV_PIX_FMT_YUV422P,
    AV_PIX_FMT_YUV444P,
    AV_PIX_FMT_YUV410P,
    AV_PIX_FMT_YUV411P,
    AV_PIX_FMT_GRAY8,
    AV_PIX_FMT_MONOWHITE,
    AV_PIX_FMT_MONOBLACK,
    AV_PIX_FMT_PAL8,
    AV_PIX_FMT_YUVJ420P,
    AV_PIX_FMT_YUVJ422P,
    AV_PIX_FMT_YUVJ444P,
    AV_PIX_FMT_UYVY422,
    AV_PIX_FMT_UYYVYY411,
    AV_PIX_FMT_BGR8,
    AV_PIX_FMT_BGR4,
    AV_PIX_FMT_BGR4_BYTE,
    AV_PIX_FMT_RGB8,
    AV_PIX_FMT_RGB4,
    AV_PIX_FMT_RGB4_BYTE,
    AV_PIX_FMT_NV12,
    AV_PIX_FMT_NV21,

    AV_PIX_FMT_ARGB,
    AV_PIX_FMT_RGBA,
    AV_PIX_FMT_ABGR,
    AV_PIX_FMT_BGRA,

    AV_PIX_FMT_GRAY16BE,
    AV_PIX_FMT_GRAY16LE,
    AV_PIX_FMT_YUV440P,
    AV_PIX_FMT_YUVJ440P,
    AV_PIX_FMT_YUVA420P,
    AV_PIX_FMT_RGB48BE,
    AV_PIX_FMT_RGB48LE,

    AV_PIX_FMT_RGB565BE,
    AV_PIX_FMT_RGB565LE,
    AV_PIX_FMT_RGB555BE,
    AV_PIX_FMT_RGB555LE,

    AV_PIX_FMT_BGR565BE,
    AV_PIX_FMT_BGR565LE,
    AV_PIX_FMT_BGR555BE,
    AV_PIX_FMT_BGR555LE,

    AV_PIX_FMT_VAAPI,

    AV_PIX_FMT_YUV420P16LE,
    AV_PIX_FMT_YUV420P16BE,
    AV_PIX_FMT_YUV422P16LE,
    AV_PIX_FMT_YUV422P16BE,
    AV_PIX_FMT_YUV444P16LE,
    AV_PIX_FMT_YUV444P16BE,
    AV_PIX_FMT_DXVA2_VLD,

    AV_PIX_FMT_RGB444LE,
    AV_PIX_FMT_RGB444BE,
    AV_PIX_FMT_BGR444LE,
    AV_PIX_FMT_BGR444BE,
    AV_PIX_FMT_YA8,

    AV_PIX_FMT_Y400A = AV_PIX_FMT_YA8,
    AV_PIX_FMT_GRAY8A = AV_PIX_FMT_YA8,

    AV_PIX_FMT_BGR48BE,
    AV_PIX_FMT_BGR48LE,

    AV_PIX_FMT_YUV420P9BE,
    AV_PIX_FMT_YUV420P9LE,
    AV_PIX_FMT_YUV420P10BE,
    AV_PIX_FMT_YUV420P10LE,
    AV_PIX_FMT_YUV422P10BE,
    AV_PIX_FMT_YUV422P10LE,
    AV_PIX_FMT_YUV444P9BE,
    AV_PIX_FMT_YUV444P9LE,
    AV_PIX_FMT_YUV444P10BE,
    AV_PIX_FMT_YUV444P10LE,
    AV_PIX_FMT_YUV422P9BE,
    AV_PIX_FMT_YUV422P9LE,
    AV_PIX_FMT_GBRP,
    AV_PIX_FMT_GBR24P = AV_PIX_FMT_GBRP,
    AV_PIX_FMT_GBRP9BE,
    AV_PIX_FMT_GBRP9LE,
    AV_PIX_FMT_GBRP10BE,
    AV_PIX_FMT_GBRP10LE,
    AV_PIX_FMT_GBRP16BE,
    AV_PIX_FMT_GBRP16LE,
    AV_PIX_FMT_YUVA422P,
    AV_PIX_FMT_YUVA444P,
    AV_PIX_FMT_YUVA420P9BE,
    AV_PIX_FMT_YUVA420P9LE,
    AV_PIX_FMT_YUVA422P9BE,
    AV_PIX_FMT_YUVA422P9LE,
    AV_PIX_FMT_YUVA444P9BE,
    AV_PIX_FMT_YUVA444P9LE,
    AV_PIX_FMT_YUVA420P10BE,
    AV_PIX_FMT_YUVA420P10LE,
    AV_PIX_FMT_YUVA422P10BE,
    AV_PIX_FMT_YUVA422P10LE,
    AV_PIX_FMT_YUVA444P10BE,
    AV_PIX_FMT_YUVA444P10LE,
    AV_PIX_FMT_YUVA420P16BE,
    AV_PIX_FMT_YUVA420P16LE,
    AV_PIX_FMT_YUVA422P16BE,
    AV_PIX_FMT_YUVA422P16LE,
    AV_PIX_FMT_YUVA444P16BE,
    AV_PIX_FMT_YUVA444P16LE,

    AV_PIX_FMT_VDPAU,

    AV_PIX_FMT_XYZ12LE,
    AV_PIX_FMT_XYZ12BE,
    AV_PIX_FMT_NV16,
    AV_PIX_FMT_NV20LE,
    AV_PIX_FMT_NV20BE,

    AV_PIX_FMT_RGBA64BE,
    AV_PIX_FMT_RGBA64LE,
    AV_PIX_FMT_BGRA64BE,
    AV_PIX_FMT_BGRA64LE,

    AV_PIX_FMT_YVYU422,

    AV_PIX_FMT_YA16BE,
    AV_PIX_FMT_YA16LE,

    AV_PIX_FMT_GBRAP,
    AV_PIX_FMT_GBRAP16BE,
    AV_PIX_FMT_GBRAP16LE,
    AV_PIX_FMT_QSV,
    AV_PIX_FMT_MMAL,

    AV_PIX_FMT_D3D11VA_VLD,

    AV_PIX_FMT_CUDA,

    AV_PIX_FMT_0RGB,
    AV_PIX_FMT_RGB0,
    AV_PIX_FMT_0BGR,
    AV_PIX_FMT_BGR0,

    AV_PIX_FMT_YUV420P12BE,
    AV_PIX_FMT_YUV420P12LE,
    AV_PIX_FMT_YUV420P14BE,
    AV_PIX_FMT_YUV420P14LE,
    AV_PIX_FMT_YUV422P12BE,
    AV_PIX_FMT_YUV422P12LE,
    AV_PIX_FMT_YUV422P14BE,
    AV_PIX_FMT_YUV422P14LE,
    AV_PIX_FMT_YUV444P12BE,
    AV_PIX_FMT_YUV444P12LE,
    AV_PIX_FMT_YUV444P14BE,
    AV_PIX_FMT_YUV444P14LE,
    AV_PIX_FMT_GBRP12BE,
    AV_PIX_FMT_GBRP12LE,
    AV_PIX_FMT_GBRP14BE,
    AV_PIX_FMT_GBRP14LE,
    AV_PIX_FMT_YUVJ411P,

    AV_PIX_FMT_BAYER_BGGR8,
    AV_PIX_FMT_BAYER_RGGB8,
    AV_PIX_FMT_BAYER_GBRG8,
    AV_PIX_FMT_BAYER_GRBG8,
    AV_PIX_FMT_BAYER_BGGR16LE,
    AV_PIX_FMT_BAYER_BGGR16BE,
    AV_PIX_FMT_BAYER_RGGB16LE,
    AV_PIX_FMT_BAYER_RGGB16BE,
    AV_PIX_FMT_BAYER_GBRG16LE,
    AV_PIX_FMT_BAYER_GBRG16BE,
    AV_PIX_FMT_BAYER_GRBG16LE,
    AV_PIX_FMT_BAYER_GRBG16BE,

#if FF_API_XVMC
    AV_PIX_FMT_XVMC,
#endif

    AV_PIX_FMT_YUV440P10LE,
    AV_PIX_FMT_YUV440P10BE,
    AV_PIX_FMT_YUV440P12LE,
    AV_PIX_FMT_YUV440P12BE,
    AV_PIX_FMT_AYUV64LE,
    AV_PIX_FMT_AYUV64BE,

    AV_PIX_FMT_VIDEOTOOLBOX,

    AV_PIX_FMT_P010LE,
    AV_PIX_FMT_P010BE,

    AV_PIX_FMT_GBRAP12BE,
    AV_PIX_FMT_GBRAP12LE,

    AV_PIX_FMT_GBRAP10BE,
    AV_PIX_FMT_GBRAP10LE,

    AV_PIX_FMT_MEDIACODEC,

    AV_PIX_FMT_GRAY12BE,
    AV_PIX_FMT_GRAY12LE,
    AV_PIX_FMT_GRAY10BE,
    AV_PIX_FMT_GRAY10LE,

    AV_PIX_FMT_P016LE,
    AV_PIX_FMT_P016BE,

    AV_PIX_FMT_D3D11,

    AV_PIX_FMT_GRAY9BE,
    AV_PIX_FMT_GRAY9LE,

    AV_PIX_FMT_GBRPF32BE,
    AV_PIX_FMT_GBRPF32LE,
    AV_PIX_FMT_GBRAPF32BE,
    AV_PIX_FMT_GBRAPF32LE,

    AV_PIX_FMT_DRM_PRIME,
    AV_PIX_FMT_OPENCL,

    AV_PIX_FMT_GRAY14BE,
    AV_PIX_FMT_GRAY14LE,

    AV_PIX_FMT_GRAYF32BE,
    AV_PIX_FMT_GRAYF32LE,

    AV_PIX_FMT_YUVA422P12BE,
    AV_PIX_FMT_YUVA422P12LE,
    AV_PIX_FMT_YUVA444P12BE,
    AV_PIX_FMT_YUVA444P12LE,

    AV_PIX_FMT_NV24,
    AV_PIX_FMT_NV42,

    AV_PIX_FMT_VULKAN,

    AV_PIX_FMT_Y210BE,
    AV_PIX_FMT_Y210LE,

    AV_PIX_FMT_X2RGB10LE,
    AV_PIX_FMT_X2RGB10BE,
    AV_PIX_FMT_X2BGR10LE,
    AV_PIX_FMT_X2BGR10BE,

    AV_PIX_FMT_P210BE,
    AV_PIX_FMT_P210LE,

    AV_PIX_FMT_P410BE,
    AV_PIX_FMT_P410LE,

    AV_PIX_FMT_P216BE,
    AV_PIX_FMT_P216LE,

    AV_PIX_FMT_P416BE,
    AV_PIX_FMT_P416LE,

    AV_PIX_FMT_VUYA,

    AV_PIX_FMT_RGBAF16BE,
    AV_PIX_FMT_RGBAF16LE,

    AV_PIX_FMT_VUYX,

    AV_PIX_FMT_P012LE,
    AV_PIX_FMT_P012BE,

    AV_PIX_FMT_Y212BE,
    AV_PIX_FMT_Y212LE,

    AV_PIX_FMT_XV30BE,
    AV_PIX_FMT_XV30LE,

    AV_PIX_FMT_XV36BE,
    AV_PIX_FMT_XV36LE,

    AV_PIX_FMT_RGBF32BE,
    AV_PIX_FMT_RGBF32LE,

    AV_PIX_FMT_RGBAF32BE,
    AV_PIX_FMT_RGBAF32LE,

    AV_PIX_FMT_NB
}

internal enum AVMediaType {
    AVMEDIA_TYPE_UNKNOWN = -1,
    AVMEDIA_TYPE_VIDEO,
    AVMEDIA_TYPE_AUDIO,
    AVMEDIA_TYPE_DATA,
    AVMEDIA_TYPE_SUBTITLE,
    AVMEDIA_TYPE_ATTACHMENT,
    AVMEDIA_TYPE_NB
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe readonly struct AVFrameIntPtrArray8 {
    //
    // the amount of fields must be equal to AVFrame.AV_NUM_DATA_POINTERS
    //
    private readonly nint i0, i1, i2, i3, i4, i5, i6, i7;

}


[StructLayout(LayoutKind.Sequential)]
internal unsafe struct AVFrame {
    public const int AV_NUM_DATA_POINTERS = 8;

    public readonly AVFrameIntPtrArray8 data;
    public fixed int linesize[AV_NUM_DATA_POINTERS];
    public readonly byte** extended_data;
    readonly int width, height;
    public readonly int nb_samples;
    readonly int format;
    readonly int key_frame;
    readonly AVPictureType pict_type;
    readonly AVRational sample_aspect_ratio;
    public readonly long pts;
    readonly long pkt_dts;
    readonly AVRational time_base;

#if FF_API_FRAME_PICTURE_NUMBER
    [Obsolete]
    readonly int coded_picture_number;
    [Obsolete]
    readonly int display_picture_number;
#endif

    readonly int quality;
    readonly void* opaque;
    readonly int repeat_pict;
    readonly int interlaced_frame;
    readonly int top_field_first;
    readonly int palette_has_changed;

#if FF_API_REORDERED_OPAQUE
    [Obsolete]
    readonly long reordered_opaque;
#endif

    readonly int sample_rate;

#if FF_API_OLD_CHANNEL_LAYOUT
    [Obsolete]
    readonly ulong channel_layout;
#endif

    readonly AVFrameIntPtrArray8 buf;
    readonly void* extended_buf;
    readonly int nb_extended_buf;
    readonly void* side_data;
    readonly int nb_side_data;
    readonly int flags;
    readonly AVColorRange color_range;
    readonly AVColorPrimaries color_primaries;
    readonly AVColorTransferCharacteristic color_trc;
    readonly AVColorSpace colorspace;
    readonly AVChromaLocation chroma_location;
    readonly long best_effort_timestamp;

#if FF_API_FRAME_PKT
    [Obsolete]
    readonly long pkt_pos;
#endif

#if FF_API_PKT_DURATION
    [Obsolete]
    readonly long pkt_duration;
#endif

    readonly nint metadata;
    readonly int decode_error_flags;

#if FF_API_OLD_CHANNEL_LAYOUT
    [Obsolete]
    readonly int channels;
#endif

#if FF_API_FRAME_PKT
    [Obsolete]
    readonly int pkt_size;
#endif

    readonly nint hw_frames_ctx;
    readonly nint opaque_ref;
    readonly nint crop_top;
    readonly nint crop_bottom;
    readonly nint crop_left;
    readonly nint crop_right;
    readonly nint private_ref;
    readonly AVChannelLayout ch_layout;
    public readonly long duration;
}

[StructLayout(LayoutKind.Sequential)]
internal readonly struct AVRational {
    public readonly int num;
    public readonly int den;
}

internal static unsafe partial class FFmpeg {

    private const string AVUTIL_DLL = "avutil-58";

    public const uint AVERROR_UNKNOWN = 0xb1b4b1ab;
    public const int AVERROR_EOF = -0x20464F45;
    public const int AV_ERROR_MAX_STRING_SIZE = 64;
    public const int AV_TIME_BASE = 1000000;
    public const long AV_NOPTS_VALUE = unchecked((long)0x8000000000000000);
    public const int EAGAIN = 11;


    [DllImport(AVUTIL_DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void av_frame_free(AVFrame** frame);

    [DllImport(AVUTIL_DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void av_free(void* ptr);

    [DllImport(AVUTIL_DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void av_channel_layout_default(AVChannelLayout* ch_layout, int nb_channels);

    [DllImport(AVUTIL_DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void* av_malloc(nuint size);

    [DllImport(AVUTIL_DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int av_strerror(int errnum, byte* errbuf, nint errbuf_size);

    [DllImport(AVUTIL_DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int av_image_get_buffer_size(AVPixelFormat pix_fmt, int width, int height, int align);

    [DllImport(AVUTIL_DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int av_image_fill_arrays(byte** dst_data, int* dst_linesize, byte* src, AVPixelFormat pix_fmt, int width, int height, int align);

    [DllImport(AVUTIL_DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern AVFrame* av_frame_alloc();

    [DllImport(AVUTIL_DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern uint avutil_version();

    [DllImport(AVUTIL_DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern char* av_version_info();


    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static int AVERROR(int err) {
        return -err;
    }

    public static string av_err2str(int errnum) {
        const int errbuf_size = AV_ERROR_MAX_STRING_SIZE;
        byte* errbuf = stackalloc byte[errbuf_size];

        av_strerror(errnum, errbuf, errbuf_size);

        return Marshal.PtrToStringUTF8((nint)errbuf);
    }

}
