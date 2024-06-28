using System;
using System.Formats.Asn1;
using System.Runtime.InteropServices;

///
/// Note: the following structures and enums are not perfectly ported
///
namespace Engine.Externals.FFGraphInterop.Interop;

internal enum AVCodecID { }
internal enum AVFieldOrder { }
internal enum AVAudioServiceType { }
internal enum AVDiscard { }


[StructLayout(LayoutKind.Sequential)]
internal unsafe struct AVCodecContext {
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
    readonly int flags;
    readonly int flags2;
    readonly byte* extradata;
    readonly int extradata_size;
    readonly AVRational time_base;
    public AVRational pkt_timebase;
    public AVRational framerate;
#if FF_API_TICKS_PER_FRAME
    readonly int ticks_per_frame;
#endif
    readonly int delay;
    public readonly int width, height;
    readonly int coded_width, coded_height;
    readonly AVRational sample_aspect_ratio;
    public readonly AVPixelFormat pix_fmt;
    readonly AVPixelFormat sw_pix_fmt;
    readonly AVColorPrimaries color_primaries;
    readonly AVColorTransferCharacteristic color_trc;
    readonly AVColorSpace colorspace;
    readonly AVColorRange color_range;
    readonly AVChromaLocation chroma_sample_location;
    readonly AVFieldOrder field_order;
    readonly int refs;
    readonly int has_b_frames;
    readonly int slice_flags;
    readonly void* draw_horiz_band;
    readonly void* get_format;
    readonly int max_b_frames;
    readonly float b_quant_factor;
    readonly float b_quant_offset;
    readonly float i_quant_factor;
    readonly float i_quant_offset;
    readonly float lumi_masking;
    readonly float temporal_cplx_masking;
    readonly float spatial_cplx_masking;
    readonly float p_masking;
    readonly float dark_masking;
    readonly int nsse_weight;
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
    readonly int mb_decision;
    readonly ushort* intra_matrix;
    readonly ushort* inter_matrix;
    readonly ushort* chroma_intra_matrix;
    readonly int intra_dc_precision;
    readonly int mb_lmin;
    readonly int mb_lmax;
    readonly int bidir_refine;
    readonly int keyint_min;
    readonly int gop_size;
    readonly int mv0_threshold;
    readonly int slices;
    public readonly int sample_rate;
    public readonly AVSampleFormat sample_fmt;
    public readonly AVChannelLayout ch_layout;
    readonly int frame_size;
    readonly int block_align;
    readonly int cutoff;
    readonly AVAudioServiceType audio_service_type;
    readonly AVSampleFormat request_sample_fmt;
    readonly int initial_padding;
    readonly int trailing_padding;
    readonly int seek_preroll;
    readonly void* get_buffer2;
    readonly int bit_rate_tolerance;
    readonly int global_quality;
    readonly int compression_level;
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
    readonly void* hwaccel;
    readonly void* hwaccel_context;
    readonly void* hw_frames_ctx;
    readonly void* hw_device_ctx;
    readonly int hwaccel_flags;
    readonly int extra_hw_frames;
    fixed ulong error[FFmpeg.AV_NUM_DATA_POINTERS];
    readonly int dct_algo;
    readonly int idct_algo;
    readonly int bits_per_coded_sample;
    readonly int bits_per_raw_sample;
    readonly int thread_count;
    readonly int thread_type;
    readonly int active_thread_type;
    readonly void* execute;
    readonly void* execute2;
    readonly int profile;
    readonly int level;
    readonly uint properties;
    readonly AVDiscard skip_loop_filter;
    readonly AVDiscard skip_idct;
    readonly AVDiscard skip_frame;
    readonly int skip_alpha;
    readonly int skip_top;
    readonly int skip_bottom;
    readonly int lowres;
    readonly nint codec_descriptor;
    readonly char* sub_charenc;
    readonly int sub_charenc_mode;
    readonly int subtitle_header_size;
    readonly byte* subtitle_header;
    readonly byte* dump_separator;
    readonly char* codec_whitelist;
    readonly nint coded_side_data;
    readonly int nb_coded_side_data;
    readonly int export_side_data;
    readonly long max_pixels;
    readonly int apply_cropping;
    readonly int discard_damaged_percentage;
    readonly long max_samples;
    readonly nint get_encode_buffer;
    readonly long frame_num;
    readonly int* side_data_prefer_packet;
    readonly uint nb_side_data_prefer_packet;
    readonly nint decoded_side_data;
    readonly int nb_decoded_side_data;
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

    private const string AVCODEC_DLL = "avcodec-61";


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