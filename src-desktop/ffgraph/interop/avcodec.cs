using System;
using System.Runtime.InteropServices;

///
/// Note: the following structures and enums are not perfectly ported
///
namespace Engine.Externals.FFGraphInterop.Interop;

internal enum AVCodecID { }
internal enum AVFieldOrder { }
internal enum AVAudioServiceType { }
internal enum AVDiscard { }


internal enum AVSampleFormat {
    AV_SAMPLE_FMT_NONE = -1,
    AV_SAMPLE_FMT_U8,
    AV_SAMPLE_FMT_S16,
    AV_SAMPLE_FMT_S32,
    AV_SAMPLE_FMT_FLT,
    AV_SAMPLE_FMT_DBL,

    AV_SAMPLE_FMT_U8P,
    AV_SAMPLE_FMT_S16P,
    AV_SAMPLE_FMT_S32P,
    AV_SAMPLE_FMT_FLTP,
    AV_SAMPLE_FMT_DBLP,
    AV_SAMPLE_FMT_S64,
    AV_SAMPLE_FMT_S64P,

    AV_SAMPLE_FMT_NB
}

internal enum AVChannelOrder {
    AV_CHANNEL_ORDER_UNSPEC,
    AV_CHANNEL_ORDER_NATIVE,
    AV_CHANNEL_ORDER_CUSTOM,
    AV_CHANNEL_ORDER_AMBISONIC
};

[StructLayout(LayoutKind.Sequential)]
internal struct AVChannelLayout {
    public readonly AVChannelOrder order;
    public readonly int nb_channels;
    public readonly U u;
    public nint opaque;

    [StructLayout(LayoutKind.Explicit)]
    public readonly struct U {
        [FieldOffset(0)]
        private readonly ulong mask;
        [FieldOffset(0)]
        private readonly nint map;
    }
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct AVCodecContext {
    private const int AV_NUM_DATA_POINTERS = 8;

    readonly void* av_class;
    readonly int log_level_offset;
    readonly AVMediaType codec_type;
    readonly AVCodec* codec;
    readonly AVCodecID codec_id;
    readonly uint codec_tag;
    readonly void* priv_data;
    readonly void* @internal;
    readonly void* opaque;
    readonly long bit_rate;
    readonly int bit_rate_tolerance;
    readonly int global_quality;
    readonly int compression_level;
    readonly int flags;
    readonly int flags2;
    readonly byte* extradata;
    readonly int extradata_size;
    readonly AVRational time_base;
    readonly int ticks_per_frame;
    readonly int delay;
    public readonly int width, height;
    readonly int coded_width, coded_height;
    readonly int gop_size;
    public readonly AVPixelFormat pix_fmt;
    readonly void* draw_horiz_band;
    readonly void* get_format;
    readonly int max_b_frames;
    readonly float b_quant_factor;
    readonly float b_quant_offset;
    readonly int has_b_frames;
    readonly float i_quant_factor;
    readonly float i_quant_offset;
    readonly float lumi_masking;
    readonly float temporal_cplx_masking;
    readonly float spatial_cplx_masking;
    readonly float p_masking;
    readonly float dark_masking;

#if FF_API_SLICE_OFFSET
    [Obsolete]
    readonly int slice_count;
    [Obsolete]
    readonly int* slice_offset;
#endif

    readonly AVRational sample_aspect_ratio;
    readonly int me_cmp;
    readonly int me_sub_cmp;
    readonly int mb_cmp;
    readonly int ildct_cmp;
    readonly int dia_size;
    readonly int last_predictor_count;
    readonly int me_pre_cmp;
    readonly int pre_dia_size;
    readonly int me_subpel_quality;
    readonly int me_range;
    readonly int slice_flags;
    readonly int mb_decision;
    readonly ushort* intra_matrix;
    readonly ushort* inter_matrix;
    readonly int intra_dc_precision;
    readonly int skip_top;
    readonly int skip_bottom;
    readonly int mb_lmin;
    readonly int mb_lmax;
    readonly int bidir_refine;
    readonly int keyint_min;
    readonly int refs;
    readonly int mv0_threshold;
    readonly AVColorPrimaries color_primaries;
    readonly AVColorTransferCharacteristic color_trc;
    readonly AVColorSpace colorspace;
    readonly AVColorRange color_range;
    readonly AVChromaLocation chroma_sample_location;
    readonly int slices;
    readonly AVFieldOrder field_order;
    public readonly int sample_rate;

#if FF_API_OLD_CHANNEL_LAYOUT
    [Obsolete]
    readonly int channels;
#endif

    public readonly AVSampleFormat sample_fmt;
    readonly int frame_size;

#if FF_API_AVCTX_FRAME_NUMBER
    [Obsolete]
    readonly int frame_number;
#endif

    readonly int block_align;
    readonly int cutoff;

#if FF_API_OLD_CHANNEL_LAYOUT
    [Obsolete]
    readonly ulong channel_layout;
    [Obsolete]
    readonly ulong request_channel_layout;
#endif

    readonly AVAudioServiceType audio_service_type;
    readonly AVSampleFormat request_sample_fmt;
    readonly void* get_buffer2;
    readonly float qcompress;
    readonly float qblur;
    readonly int qmin;
    readonly int qmax;
    readonly int max_qdiff;
    readonly int rc_buffer_size;
    readonly int rc_override_count;
    readonly void* rc_override;
    readonly long rc_max_rate;
    readonly long rc_min_rate;
    readonly float rc_max_available_vbv_use;
    readonly float rc_min_vbv_overflow_use;
    readonly int rc_initial_buffer_occupancy;
    readonly int trellis;
    readonly char* stats_out;
    readonly char* stats_in;
    readonly int workaround_bugs;
    readonly int strict_std_compliance;
    readonly int error_concealment;
    readonly int debug;
    readonly int err_recognition;

#if FF_API_REORDERED_OPAQUE
    [Obsolete]
    readonly long reordered_opaque;
#endif
    readonly void* hwaccel;
    readonly void* hwaccel_context;
    fixed ulong error[AV_NUM_DATA_POINTERS];
    readonly int dct_algo;
    readonly int idct_algo;
    readonly int bits_per_coded_sample;
    readonly int bits_per_raw_sample;
    readonly int lowres;
    readonly int thread_count;
    readonly int thread_type;
    readonly int active_thread_type;
    readonly void* execute;
    readonly void* execute2;
    readonly int nsse_weight;
    readonly int profile;
    readonly int level;
    readonly AVDiscard skip_loop_filter;
    readonly AVDiscard skip_idct;
    readonly AVDiscard skip_frame;
    readonly byte* subtitle_header;
    readonly int subtitle_header_size;
    readonly int initial_padding;
    public readonly AVRational framerate;
    readonly AVPixelFormat sw_pix_fmt;
    public AVRational pkt_timebase;
    readonly nint codec_descriptor;
    readonly long pts_correction_num_faulty_pts;
    readonly long pts_correction_num_faulty_dts;
    readonly long pts_correction_last_pts;
    readonly long pts_correction_last_dts;
    readonly char* sub_charenc;
    readonly int sub_charenc_mode;
    readonly int skip_alpha;
    readonly int seek_preroll;
    readonly ushort* chroma_intra_matrix;
    readonly byte* dump_separator;
    readonly char* codec_whitelist;
    readonly uint properties;
    readonly nint coded_side_data;
    readonly int nb_coded_side_data;
    readonly nint hw_frames_ctx;
    readonly int trailing_padding;
    readonly long max_pixels;
    readonly nint hw_device_ctx;
    readonly int hwaccel_flags;
    readonly int apply_cropping;
    readonly int extra_hw_frames;
    readonly int discard_damaged_percentage;
    readonly long max_samples;
    readonly int export_side_data;
    readonly nint get_encode_buffer;
    public readonly AVChannelLayout ch_layout;
    readonly long frame_num;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct AVPacket {
    readonly nint buf;
    public long pts;
    readonly long dts;
    readonly byte* data;
    public int size;
    public readonly int stream_index;
    readonly int flags;
    readonly nint side_data;
    readonly int side_data_elems;
    public long duration;
    readonly long pos;
    readonly void* opaque;
    readonly nint opaque_ref;
    readonly AVRational time_base;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe readonly struct AVCodec { }

internal static unsafe partial class FFmpeg {

    private const string AVCODEC_DLL = "avcodec-60";


    [DllImport(AVCODEC_DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern uint avcodec_version();

    [DllImport(AVCODEC_DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern AVPacket* av_packet_alloc();

    [DllImport(AVCODEC_DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern AVCodecContext* avcodec_alloc_context3(AVCodec* codec);

    [DllImport(AVCODEC_DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void av_packet_unref(AVPacket* pkt);

    [DllImport(AVCODEC_DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int avcodec_send_packet(AVCodecContext* avctx, AVPacket* avpkt);

    [DllImport(AVCODEC_DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int avcodec_receive_frame(AVCodecContext* avctx, AVFrame* frame);

    [DllImport(AVCODEC_DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void avcodec_flush_buffers(AVCodecContext* avctx);

    [DllImport(AVCODEC_DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int avcodec_open2(AVCodecContext* avctx, AVCodec* codec, void* options);

    [DllImport(AVCODEC_DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int avcodec_parameters_to_context(AVCodecContext* codec, nint par);

    [DllImport(AVCODEC_DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void av_packet_free(AVPacket** pkt);

    [DllImport(AVCODEC_DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void avcodec_free_context(AVCodecContext** avctx);

}