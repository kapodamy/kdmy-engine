using System.Runtime.InteropServices;

///
/// Note: the following structures and enums are not perfectly ported
///
namespace Engine.Externals.FFGraphInterop.Interop;


internal enum AVDurationEstimationMethod {
    AVFMT_DURATION_FROM_PTS,
    AVFMT_DURATION_FROM_STREAM,
    AVFMT_DURATION_FROM_BITRATE
};

[StructLayout(LayoutKind.Sequential)]
internal unsafe readonly struct AVIOInterruptCB {
    readonly nint callback;
    readonly void* opaque;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct AVFormatContext {
    readonly nint av_class;
    readonly nint iformat;
    readonly nint oformat;
    readonly void* priv_data;
    public AVIOContext* pb;
    readonly int ctx_flags;
    readonly uint nb_streams;
    public readonly AVStream** streams;
    readonly uint nb_stream_groups;
    readonly nint stream_groups;
    readonly uint nb_chapters;
    readonly nint chapters;
    readonly char* url;
    readonly long start_time;
    public readonly long duration;
    readonly long bit_rate;
    readonly uint packet_size;
    readonly int max_delay;
    public int flags;
    readonly long probesize;
    readonly long max_analyze_duration;
    readonly byte* key;
    readonly int keylen;
    readonly uint nb_programs;
    readonly nint programs;
    readonly AVCodecID video_codec_id;
    readonly AVCodecID audio_codec_id;
    readonly AVCodecID subtitle_codec_id;
    readonly AVCodecID data_codec_id;
    readonly nint metadata;
    readonly long start_time_realtime;
    readonly int fps_probe_size;
    readonly int error_recognition;
    readonly AVIOInterruptCB interrupt_callback;
    readonly int debug;
    readonly int max_streams;
    readonly uint max_index_size;
    readonly uint max_picture_buffer;
    readonly long max_interleave_delta;
    readonly int max_ts_probe;
    readonly int max_chunk_duration;
    readonly int max_chunk_size;
    readonly int max_probe_packets;
    readonly int strict_std_compliance;
    readonly int event_flags;
    readonly int avoid_negative_ts;
    readonly int audio_preload;
    readonly int use_wallclock_as_timestamps;
    readonly int skip_estimate_duration_from_pts;
    readonly int avio_flags;
    readonly AVDurationEstimationMethod duration_estimation_method;
    readonly long skip_initial_bytes;
    readonly int correct_ts_overflow;
    readonly int seek2any;
    readonly int flush_packets;
    readonly int probe_score;
    readonly int format_probesize;
    readonly char* codec_whitelist;
    readonly char* format_whitelist;
    readonly char* protocol_whitelist;
    readonly char* protocol_blacklist;
    readonly int io_repositioned;
    readonly AVCodec* video_codec;
    readonly AVCodec* audio_codec;
    readonly AVCodec* subtitle_codec;
    readonly AVCodec* data_codec;
    readonly int metadata_header_padding;
    readonly void* opaque;
    readonly nint control_message_cb;
    readonly long output_ts_offset;
    readonly byte* dump_separator;
    readonly nint io_open;
    readonly nint io_close2;
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe readonly struct AVIOContext {
    readonly nint av_class;
    public readonly byte* buffer;
    //
    //  trucated struct  trucated struct  trucated struct  trucated struct  trucated struct
    //
}

[StructLayout(LayoutKind.Sequential)]
internal unsafe readonly struct AVStream {
    readonly nint av_class;
    readonly int index;
    readonly int id;
    public readonly nint codecpar;
    readonly void* priv_data;
    public readonly AVRational time_base;
    public readonly long start_time;
    public readonly long duration;
    readonly long nb_frames;
    readonly int disposition;
    readonly AVDiscard discard;
    readonly AVRational sample_aspect_ratio;
    readonly nint metadata;
    readonly AVRational avg_frame_rate;
    readonly AVPacket attached_pic;
    readonly int event_flags;
    readonly AVRational r_frame_rate;
    readonly int pts_wrap_bits;
}


internal static unsafe partial class FFmpeg {
    private const string AVFORMAT_DLL = "avformat-61";


    public const int AVSEEK_SIZE = 0x10000;
    public const int AVSEEK_FORCE = 0x20000;
    public const int AVFMT_FLAG_CUSTOM_IO = 0x0080;


    [DllImport(AVFORMAT_DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern AVIOContext* avio_alloc_context(
                  byte* buffer, int buffer_size, int write_flag, nint opaque,
                  delegate* unmanaged[Cdecl]<nint, byte*, int, int> read_packet,
                  delegate* unmanaged[Cdecl]<nint, byte*, int, int> write_packet,
                  delegate* unmanaged[Cdecl]<nint, long, int, long> seek
    );

    [DllImport(AVFORMAT_DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void avio_context_free(AVIOContext** s);

    [DllImport(AVFORMAT_DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern AVFormatContext* avformat_alloc_context();

    [DllImport(AVFORMAT_DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern uint avformat_version();

    [DllImport(AVFORMAT_DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int av_read_frame(AVFormatContext* s, AVPacket* pkt);

    [DllImport(AVFORMAT_DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int avformat_seek_file(AVFormatContext* s, int stream_index, long min_ts, long ts, long max_ts, int flags);

    [DllImport(AVFORMAT_DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int av_find_best_stream(AVFormatContext* ic, AVMediaType type, int wanted_stream_nb, int related_stream, AVCodec** decoder_ret, int flags);

    [DllImport(AVFORMAT_DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int avformat_find_stream_info(AVFormatContext* ic, void* options);

    [DllImport(AVFORMAT_DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern int avformat_open_input(AVFormatContext** ps, char* url, void* fmt, void* options);

    [DllImport(AVFORMAT_DLL, CallingConvention = CallingConvention.Cdecl)]
    public static extern void avformat_free_context(AVFormatContext* s);


}
