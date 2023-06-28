using System;
using System.IO;
using System.Runtime.InteropServices;

namespace Engine.Externals.SoundBridge;

internal unsafe class OggUtil {
    public const string LOOPSTART = "LOOPSTART=";
    public const string LOOPLENGTH = "LOOPLENGTH=";
    public const byte CODEC_ERROR = 0;
    public const byte CODEC_VORBIS = 1;
    public const byte CODEC_OPUS = 2;
    public const byte CODEC_UNKNOWN = 3;

    private const int OggS = 0x5367674F;
    private const byte VERSION = 0x00;
    private const byte FLAG_FIRST = 0x02;

    private static readonly byte[] CODEC_OPUS_SIG = { 0x4f, 0x70, 0x75, 0x73, 0x48, 0x65, 0x61, 0x64 }; // OpusHead
    private static readonly byte[] CODEC_VORBIS_SIG = { 0x01, 0x76, 0x6f, 0x72, 0x62, 0x69, 0x73 }; //\x01vorbis


    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    private readonly struct OggPacket {
        public readonly int magic;     // "OggS" binary string in little-endian
        public readonly byte version;    // version
        public readonly byte packetFlag; // type

        public readonly ulong granPos; // granulate position

        public readonly int streamId;      // bitstream serial number
        public readonly int sequenceCount; // page sequence number

        public readonly int checksum; // page checksum

        public readonly byte segmentTableSize; // segment size
        //  public readonly byte* segmentTable;    // segment table
    };


    public static IDecoder InitOggDecoder(IFileSource file_hnd) {
        //Debug.Assert(file_hnd != null);
        //Debug.Assert(file_hnd->read != null);
        //Debug.Assert(file_hnd->seek != null);
        //Debug.Assert(file_hnd->tell != null);

        int result = OggUtil.GetOggCodec(file_hnd);
        IDecoder dec;

        switch (result) {
            case OggUtil.CODEC_VORBIS:
                dec = OGGVorbisDecoder.Init(file_hnd);
                break;
            case OggUtil.CODEC_OPUS:
                dec = OGGOpusDecoder.Init(file_hnd);
                break;
            case OggUtil.CODEC_ERROR:
                Logger.Error("OggUtil::InitOggDecoder() can not identify the audio codec");
                return null;
            case OggUtil.CODEC_UNKNOWN:
                Logger.Error("OggUtil::InitOggDecoder() unknown audio codec");
                return null;
            default:
                return null;
        }

        if (dec == null) {
            string codec = result == OggUtil.CODEC_OPUS ? "opus" : "vorbis";
            Logger.Error($"OggUtil::InitOggDecoder() can not initialize the decoder for: {codec}");
            return null;
        }

        return dec;
    }

    public static byte GetOggCodec(IFileSource ogg_fd) {
        OggPacket oggheader;
        int oggheader_size = sizeof(OggPacket);
        long original_offset = ogg_fd.Tell();

        int max_codec_size = Math.Max(CODEC_VORBIS_SIG.Length, CODEC_OPUS_SIG.Length);

        if (ogg_fd.Read(&oggheader, oggheader_size) != oggheader_size) goto L_error;
        if (oggheader.magic != OggS) {
            Logger.Warn("OggUtil::GetOggCodec() failed, ¿endianess problem?");
            goto L_error;
        }
        if (oggheader.version != VERSION) goto L_error;
        if (oggheader.packetFlag != FLAG_FIRST) goto L_error;
        if (oggheader.segmentTableSize < 1) goto L_error;

        // read segment table
        byte* segmentTable = stackalloc byte[oggheader.segmentTableSize];
        if (ogg_fd.Read(segmentTable, oggheader.segmentTableSize) != oggheader.segmentTableSize) {
            //free(segmentTable);
            goto L_error;
        }

        // parse segment table
        short segment_length = 0;
        for (int i = 0 ; i < oggheader.segmentTableSize ; i++) {
            segment_length += segmentTable[i];
        }
        //free(segmentTable);

        if (segment_length < max_codec_size) goto L_error;

        // read codec data
        byte* codec_data = stackalloc byte[segment_length];
        if (ogg_fd.Read(codec_data, segment_length) != segment_length) {
            //free(codec_data);
            goto L_error;
        }

        // guess if the codec is vorbis
        byte codec = OggUtil.CODEC_VORBIS;
        for (byte i = 0 ; i < CODEC_VORBIS_SIG.Length ; i++) {
            if (codec_data[i] != CODEC_VORBIS_SIG[i]) {
                codec = OggUtil.CODEC_UNKNOWN;
                break;
            }
        }
        if (codec != OggUtil.CODEC_UNKNOWN) goto L_return;

        // guess if the codec is opus
        codec = OggUtil.CODEC_OPUS;
        for (byte i = 0 ; i < CODEC_OPUS_SIG.Length ; i++) {
            if (codec_data[i] != CODEC_OPUS_SIG[i]) {
                codec = OggUtil.CODEC_UNKNOWN;
                break;
            }
        }
        if (codec != OggUtil.CODEC_UNKNOWN) goto L_return;

L_return:
//free(codec_data);
        ogg_fd.Seek(original_offset, SeekOrigin.Begin);
        return codec;
L_error:
        ogg_fd.Seek(original_offset, SeekOrigin.Begin);
        return OggUtil.CODEC_ERROR;
    }

}
